/*===========================================================================*
 *                                                                           *
 *  ggscop.c - Scope functions                                               *
 *                                                                           *
 *  Copyright (c) 1996-2010 iMatix Corporation                               *
 *                                                                           *
 *  This program is free software; you can redistribute it and/or modify     *
 *  it under the terms of the GNU General Public License as published by     *
 *  the Free Software Foundation; either version 2 of the License, or (at    *
 *  your option) any later version.                                          *
 *                                                                           *
 *  This program is distributed in the hope that it will be useful, but      *
 *  WITHOUT ANY WARRANTY; without even the implied warranty of               *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU        *
 *  General Public License for more details.                                 *
 *                                                                           *
 *  For information on alternative licensing for OEMs, please contact        *
 *  iMatix Corporation.                                                      *
 *                                                                           *
 *===========================================================================*/

#include "ggpriv.h"                     /*  Header file                      */

/*- Global variables local to this module -----------------------------------*/

static char
    error_buffer [LINE_MAX + 1];

/*- Prototypes --------------------------------------------------------------*/

static SCOPE_BLOCK *lookup_scope_by_number (LIST *scope_stack,
                                            int   scope_number);

/*- Functions ---------------------------------------------------------------*/

static SCOPE_BLOCK *
lookup_scope_by_number (LIST *scope_stack,
                        int   scope_number)
{
    SCOPE_BLOCK
        *scope_block = NULL;

    if (scope_number <= 0)  /*  Count from inner to outer   */
      {
        FORLIST (scope_block, *scope_stack)
            if (scope_block-> stacked)
                if (scope_number++ == 0)
                    return scope_block;
      }
    else  /*  Count from outer to inner  */
        for (scope_block = scope_stack-> prev;
             (void *) scope_block != (void *) scope_stack;
             scope_block = scope_block-> prev)
            if (scope_block-> stacked)
                if (--scope_number == 0)
                    return scope_block;

    return NULL;
}

SCOPE_BLOCK *
lookup_simple_scope (LIST *scope_stack,
                     VALUE *scope_value,
                     Bool ignorecase,
                     CLASS_DESCRIPTOR **class,
                     void **item,
                     char **error_text)
{
    int
        scope_number;
    SCOPE_BLOCK
        *scope_block = NULL;
    VALUE
        *value;

    *class = NULL;
    *item  = NULL;

    if (error_text)
        *error_text = NULL;             /*  No errors yet  */

    if (scope_value)
      {
        /*  If value is already a pointer then that's the result  */
        if (scope_value-> type == TYPE_POINTER)
          {
            *class = scope_value-> c;
            *item  = scope_value-> i;
            return NULL;
          }

        scope_number = (int) number_value (scope_value);
        if (scope_value-> type == TYPE_NUMBER)
            scope_block = lookup_scope_by_number (scope_stack,
                                                  scope_number);

        if (! scope_block)
          {
            string_value (scope_value);
            if (scope_value-> s && scope_value-> s [0])
              {
                FORLIST (scope_block, *scope_stack)
                    if (scope_block-> name
                    && (ignorecase ? lexcmp (scope_block-> name, scope_value-> s) == 0
                                   : streq  (scope_block-> name, scope_value-> s) ))
                      {
                        *class = scope_block-> class;
                        *item  = scope_block-> item;
                        return scope_block;
                      }

                FORLIST (scope_block, *scope_stack)
                    if (scope_block-> stacked)
                      {
                        value = scope_block-> class-> get_attr (scope_block-> item,
                                                                scope_value-> s,
                                                                ignorecase);
                        if (value && value-> type == TYPE_POINTER)
                          {
                            *class = value-> c;
                            *item  = value-> i;
                            return NULL;
                          }
                      }
                return NULL;
              }
          }
      }
    if (scope_block)
      {
        *class = scope_block-> class;
        *item  = scope_block-> item;
      }
    return scope_block;
}

int
symbol_item (LIST *scope_stack,
             RESULT_NODE *symbol,
             Bool ignorecase,
             CLASS_DESCRIPTOR **class,
             void **item,
             char **error_text)
{
    RESULT_NODE
        *name;
    SCOPE_BLOCK
        *scope_block;
    VALUE
        *value;
    int
        scope_number = 0;

    if (error_text)
        *error_text = NULL;             /*  No errors yet  */

    if (symbol-> scope
    &&  symbol-> scope-> value. type == TYPE_POINTER)
      {
        *class = symbol-> scope-> value. c;
        *item  = symbol-> scope-> value. i;
      }
    else
      {
        *class = NULL;
        *item  = NULL;

        name = symbol-> name;
        if (name)
          {
            /*  Look for a stacked scope that defines this identifier.       */
            FORLIST (scope_block, *scope_stack)
                if (scope_block-> stacked && scope_block-> item)
                  {
                    *class = scope_block-> class;
                    *item  = scope_block-> item;
                    if ((*class)-> get_attr)
                      {
                        value = (*class)-> get_attr (
                                    scope_block-> item,
                                    string_value (& name-> value),
                                    ignorecase);
                        if (value && value-> type != TYPE_UNDEFINED)
                            break;
                      }
                    *class = NULL;
                    *item  = NULL;
                    scope_number--;
                  }
          }
      }
    return scope_number;
}


VALUE *
symbol_value (LIST *scope_stack,
              RESULT_NODE *symbol,
              Bool ignorecase,
              char **error_text)
{
    CLASS_DESCRIPTOR
        *class;
    void
        *item;
    SCOPE_BLOCK
        *scope_block;
    RESULT_NODE
        *name;
    int
        name_number;
    static VALUE
        value;
    VALUE
        *value_ptr;

    if (error_text)
        *error_text = NULL;             /*  No errors yet  */

    init_value (& value);

    /*  There are four cases here:                                           */
    /*     1. General: scope is specified and has been resolved to an object */
    /*     2. No scope/name is numeric: Treat name as scope.  This occurs in */
    /*        a construct like 'item = $(3)'                                 */
    /*     3. No scope/name is non-numeric: Look for match on attribute or   */
    /*        scope name.                                                    */
    /*     4. Empty scope: Look for match on attribute only.                 */

    name = symbol-> name;

    /*  Case 1  */
    if (symbol-> scope
    &&  symbol-> scope-> value. type == TYPE_POINTER)
      {
        class = symbol-> scope-> value. c;
        item  = symbol-> scope-> value. i;

        if (name)
          {
            if (class-> get_attr)
              {
                value_ptr = class-> get_attr (item,
                                              name-> value. s,
                                              ignorecase);
                if (value_ptr && value_ptr-> type == TYPE_UNDEFINED)
                    value_ptr = NULL;

                return value_ptr;
              }
          }
        else
          {
            value. c = class;
            value. i = item;
            if (value. i)
                value. type = TYPE_POINTER;
          }
        return & value;
      }
    else
      {
        if (name)
          {
            name_number = (int) number_value (& name-> value);

            /*  Case 2  */
            if (name-> value. type == TYPE_NUMBER)
              {
                scope_block = lookup_scope_by_number (scope_stack,
                                                      name_number);
                if (scope_block)
                  {
                    value. c = scope_block-> class;
                    value. i = scope_block-> item;
                    if (value. i)
                        value. type = TYPE_POINTER;

                    return & value;
                  }
                else
                    return NULL;
              }
            string_value (& name-> value);
          }

         /*  Case 3  */
        if ((! symbol-> scope) && name)
            FORLIST (scope_block, *scope_stack)
                if (scope_block-> name
                && (ignorecase ? (lexcmp (name-> value.s,
                                          scope_block-> name) == 0)
                               :  streq  (name-> value.s,
                                          scope_block-> name)))
                  {
                    value. c = scope_block-> class;
                    value. i = scope_block-> item;
                    if (value. i)
                        value. type = TYPE_POINTER;

                    return & value;
                  }

        /*  Case 3/4  */
        FORLIST (scope_block, *scope_stack)
            if ((item = scope_block-> item) != NULL)
              {
                if (scope_block-> stacked
                &&  scope_block-> class-> get_attr)
                  {
                    value_ptr = scope_block-> class-> get_attr
                                    (item,
                                     name ? name-> value. s : NULL,
                                     ignorecase);
                    if (value_ptr && value_ptr-> type != TYPE_UNDEFINED)
                        return value_ptr;
                  }

              }
      }

    return NULL;
}


Bool
store_symbol_definition (LIST *scope_stack,
                         Bool  ignorecase,
                         RESULT_NODE *symbol,
                         VALUE *value,
                         char **error_text)
{
    CLASS_DESCRIPTOR
        *class;
    void
        *item;
    int
        rc = 0;
    char
        *name = NULL;
    SCOPE_BLOCK
        *scope_block;

    if (error_text)
        *error_text = NULL;             /*  No errors yet  */

    if (symbol-> scope
    &&  symbol-> scope-> value. type == TYPE_UNDEFINED)
      {
        result_node_error ("Undefined scope", symbol-> scope, error_text);
        return FALSE;
      }
    if (symbol-> name
    &&  symbol-> name-> value. type != TYPE_UNDEFINED)
        name = string_value (& symbol-> name-> value);

    /*  Find symbol already defined.  */
    symbol_item (scope_stack,
                 symbol,
                 ignorecase,
                 &class,
                 &item,
                 error_text);

    if ((! value)
    ||  value-> type == TYPE_UNDEFINED)
      {
        if (item)
            rc =  ((! class-> put_attr)
               ||  (  class-> put_attr (item, name, NULL, ignorecase)));
      }
    else
      {
        if (! item)
          {
            /*  Use outermost stacked scope block.  */
            scope_block = first_scope_block (scope_stack);
            while (((void *) scope_block != (void *) scope_stack)
               && (! scope_block-> stacked))
                scope_block = scope_block-> prev;

            if ((void *) scope_block != (void *) scope_stack)
              {
                class = scope_block-> class;
                item  = scope_block-> item;
              }
          } 


        if (item)
            rc =  ((! class-> put_attr)
               ||  (  class-> put_attr (item, name,
                                        value,
                                        ignorecase)));
        else
            rc = -1;
      }

    if (rc)
      {
        strcpy (error_buffer,
                strprintf ("Assignment failed to attribute: %s of class: %s",
                           name_the_symbol (symbol),
                           class-> name));
        *error_text = error_buffer;
        return FALSE;
      }

    return TRUE;
}

/*- For Stack Functions -----------------------------------------------------*/

SCOPE_BLOCK *
create_scope_block (LIST *scope_stack,
                    SCRIPT_NODE_TYPE scope_type,
                    const char *alias,
                    CLASS_DESCRIPTOR *class)
{
    SCOPE_BLOCK
        *scope_block;

    list_create (scope_block, sizeof (SCOPE_BLOCK));
    ASSERT (scope_block);
    list_relink_after (scope_block, scope_stack);

    scope_block-> scope_type    = scope_type;
    scope_block-> name          = mem_strdup (alias);
    scope_block-> scope_item    = NULL;
    scope_block-> class         = class;
    scope_block-> item          = NULL;
    scope_block-> total         = 0;
    scope_block-> index         = 0;
    scope_block-> stacked       = TRUE;
    scope_block-> macros        = NULL;
    list_reset (& scope_block-> item_list);

    return scope_block;
}


void 
destroy_scope_block (SCOPE_BLOCK *scope_block)
{
    list_unlink (scope_block);

    if (! list_empty (& scope_block-> item_list))
        while (! list_empty (& scope_block-> item_list))
            destroy_scope_item (scope_block-> item_list.prev);
    else
        if (scope_block-> class
        &&  scope_block-> class-> destroy)
            scope_block-> class-> destroy (scope_block-> item);

    mem_free (scope_block-> name);

    if (scope_block-> macros)
        macro_table_destroy (scope_block-> macros);

    mem_free (scope_block);
}


SCOPE_BLOCK *
first_scope_block (LIST *scope_stack)
{
    if (!list_empty (scope_stack))
        return scope_stack-> prev;
    else
        return NULL;
}


SCOPE_BLOCK *
last_scope_block (LIST *scope_stack)
{
    if (!list_empty (scope_stack))
        return scope_stack-> next;
    else
        return NULL;
}


SCOPE_ITEM *
create_scope_item (SCOPE_BLOCK *scope_block,
                   CLASS_DESCRIPTOR *class,
                   void *item,
                   long item_num)
{
    SCOPE_ITEM
        *scope_item;

    list_create (scope_item, sizeof (SCOPE_ITEM));
    ASSERT (scope_item);
    list_relink_before (scope_item, &scope_block-> item_list);

    scope_item-> class    = class;
    scope_item-> item     = item;
    scope_item-> item_num = item_num;
    init_value (& scope_item-> sort_key);

    if (class-> link)
        class-> link (item);

    return scope_item;
}


void
destroy_scope_item (SCOPE_ITEM *scope_item)
{
    list_unlink (scope_item);
    destroy_value (& scope_item-> sort_key);

    if (scope_item-> class-> destroy)
        scope_item-> class-> destroy (scope_item-> item);

    mem_free (scope_item);
}


Bool
first_scope_item (SCOPE_BLOCK *scope_block)
{
    if (! list_empty (& scope_block-> item_list))
      {
        scope_block-> scope_item = scope_block-> item_list. next;
        scope_block-> class      = scope_block-> scope_item-> class;
        scope_block-> item       = scope_block-> scope_item-> item;
        scope_block-> index      = 1;
        return TRUE;
      }
    else
      {
        scope_block-> scope_item = NULL;
        scope_block-> class      = NULL;
        scope_block-> item       = NULL;
        return FALSE;
      }
}


Bool
next_scope_item (SCOPE_BLOCK *scope_block)
{
    if ((void *) scope_block-> scope_item-> next != & scope_block-> item_list)
      {
        scope_block-> scope_item = scope_block-> scope_item-> next;
        scope_block-> class      = scope_block-> scope_item-> class;
        scope_block-> item       = scope_block-> scope_item-> item;
        scope_block-> index++;
        return TRUE;
      }
    else
      {
        scope_block-> scope_item = NULL;
        scope_block-> class      = NULL;
        scope_block-> item       = NULL;
        return FALSE;
      }
}


void
copy_scope_stack (LIST *to,
                  LIST *from)
{
    SCOPE_BLOCK
        *new_block,
        *old_block;

    for (old_block = from-> prev;
         (void *) old_block != from;
         old_block = old_block-> prev)
      {
        new_block = create_scope_block (to,
                                        old_block-> scope_type,
                                        old_block-> name,
                                        old_block-> class);
        new_block-> item     = old_block-> item;
        new_block-> total    = old_block-> total;
        new_block-> index    = old_block-> index;
        new_block-> stacked  = old_block-> stacked;
        
        /*  Need a macro table or child macros may become orphaned.  */
        if (! old_block-> macros)
          {
            old_block-> macros = macro_table_create ();
            macro_table_link (old_block-> macros);
          }
        new_block-> macros = old_block-> macros;
        macro_table_link (new_block-> macros);

        if (new_block-> class-> link)
            new_block-> class-> link (new_block-> item);
      }
}

