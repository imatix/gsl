/*===========================================================================
 *
 *  ggfunc.inc  GSL Built-In Functions
 *                                                                           
 *  Written:    2000/02/16    iMatix <tools@imatix.com>
 *  Revised:    2002/10/17
 *                                                                            
 *  Copyright (c) 1996-2010 iMatix Corporation 
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or (at
 *  your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful, but
 *  WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 *
 *  For information on alternative licensing for OEMs, please contact
 *  iMatix Corporation.
 *===========================================================================*/

/*- Function prototypes -----------------------------------------------------*/

static int register_inbuilt_functions (void);
static int initialise_inbuilt_functions (CLASS_DESCRIPTOR **class,
                                         void             **item,
                                         THREAD            *gsl_thread);
static SCOPE_BLOCK *lookup_data_scope_from_parameter (RESULT_NODE *node);

static EVAL_FUNCTION
    eval_class,
    eval_count,
    eval_first,
    eval_index,
    eval_item,
    eval_last,
    eval_total,
    eval_name,
    eval_alias,
    eval_defined,
    eval_macro,
    eval_scope,
    eval_which;

/*- Global variables --------------------------------------------------------*/

static PARM_LIST
    one_value_parm_list     = {PARM_VALUE},
    one_scope_parm_list     = {PARM_SIMPLE_SCOPE},
    one_reference_parm_list = {PARM_REFERENCE};

static PARM_LIST
    count_parm_list     = {PARM_EXPRESSION, PARM_EXPRESSION, PARM_SIMPLE_SCOPE};

static GSL_FUNCTION gsl_functions [] =
{
    {"alias",   0, 1, 1, (void *) &one_scope_parm_list,     TRUE, eval_alias},
    {"class",   0, 1, 1, (void *) &one_value_parm_list,     TRUE, eval_class},
    {"count",   0, 3, 3, (void *) &count_parm_list,         TRUE, eval_count},
    {"defined", 1, 1, 1, (void *) &one_value_parm_list,     TRUE, eval_defined},
    {"first",   0, 1, 1, (void *) &one_scope_parm_list,     TRUE, eval_first},
    {"index",   0, 1, 1, (void *) &one_scope_parm_list,     TRUE, eval_index},
    {"item",    0, 1, 1, (void *) &one_scope_parm_list,     TRUE, eval_item},
    {"last",    0, 1, 1, (void *) &one_scope_parm_list,     TRUE, eval_last},
    {"macro",   1, 1, 1, (void *) &one_value_parm_list,     TRUE, eval_macro},
    {"name",    0, 1, 1, (void *) &one_value_parm_list,     TRUE, eval_name},
    {"scope",   1, 1, 1, (void *) &one_scope_parm_list,     TRUE, eval_scope},
    {"total",   0, 1, 1, (void *) &one_scope_parm_list,     TRUE, eval_total},
    {"which",   1, 1, 1, (void *) &one_reference_parm_list, TRUE, eval_which}
};

static CLASS_DESCRIPTOR
    built_in_class = { "$", 
                       NULL, NULL, NULL, NULL, NULL, NULL,
                       NULL, NULL, NULL, NULL, NULL, NULL, 
                       NULL,
                       gsl_functions, tblsize (gsl_functions) };

    
/*- Functions ---------------------------------------------------------------*/

static int register_inbuilt_functions (void)
{
    int
        rc;

    rc = object_register (initialise_inbuilt_functions, NULL);

    return rc;
}


static int initialise_inbuilt_functions (CLASS_DESCRIPTOR **class,
                                         void             **item,
                                         THREAD            *gsl_thread)
{
    *class = & built_in_class;
    return 0;
}


static int
eval_count (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *dummy)
{
    raise_exception (evaluate_count_event);

    return 0;
}


static int
eval_index (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *dummy)
{
    SCOPE_BLOCK
        *scope_block;
    RESULT_NODE
        *scope = argc > 0 ? argv [0] : NULL;

    if (! arguments_are_defined (argc, argv, result))
        return 0;
    scope_block = lookup_data_scope_from_parameter (scope);
    if (scope_block
    && (scope_block-> index))
      {
        result-> value. type = TYPE_NUMBER;
        result-> value. n    = scope_block-> index;
      }

    return 0;
}


SCOPE_BLOCK *
lookup_data_scope_from_parameter (RESULT_NODE *node)
{
    SCOPE_BLOCK
        *scope_block;
    CLASS_DESCRIPTOR
        *class;
    void
        *item;
    char
        *error_text;

    if (node)
      {
        scope_block = lookup_simple_scope (& tcb-> scope_stack,
                                           & node-> value,
                                           tcb-> gsl-> ignorecase,
                                           & class,
                                           & item,
                                           &error_text);
        if ((! scope_block)
        &&   ( error_text))
            report_error (error_event, "%s", error_text);
      }
    else
      {
        FORLIST (scope_block, tcb-> scope_stack)
            if (scope_block-> stacked)
                break;

        if ((void *) scope_block == & tcb-> scope_stack)
            scope_block = NULL;
      }
    return scope_block;
}


static int
eval_item (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *dummy)
{
    SCOPE_BLOCK
        *scope_block;
    RESULT_NODE
        *scope = argc > 0 ? argv [0] : NULL;

    if (! arguments_are_defined (argc, argv, result))
        return 0;
    scope_block = lookup_data_scope_from_parameter (scope);
    if (scope_block
    && (scope_block-> scope_item))
      {
        result-> value. type = TYPE_NUMBER;
        result-> value. n    = scope_block-> scope_item-> item_num;
      }

    return 0;
}


static int
eval_first (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *dummy)
{
    SCOPE_BLOCK
        *scope_block;
    RESULT_NODE
        *scope = argc > 0 ? argv [0] : NULL;

    if (! arguments_are_defined (argc, argv, result))
        return 0;
    scope_block = lookup_data_scope_from_parameter (scope);
    if (scope_block
    && (scope_block-> index))
      {
        result-> value. type = TYPE_NUMBER;
        result-> value. n    = scope_block-> index == 1 ? 1 : 0;
      }

    return 0;
}


static int
eval_last (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *dummy)
{
    SCOPE_BLOCK
        *scope_block;
    RESULT_NODE
        *scope = argc > 0 ? argv [0] : NULL;

    if (! arguments_are_defined (argc, argv, result))
        return 0;
    scope_block = lookup_data_scope_from_parameter (scope);
    if (scope_block
    && (scope_block-> index))
      {
        result-> value. type = TYPE_NUMBER;
        result-> value. n    = scope_block-> index == scope_block-> total
                             ? 1 : 0;
      }

    return 0;
}


static int
eval_total (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *dummy)
{
    SCOPE_BLOCK
        *scope_block;
    RESULT_NODE
        *scope = argc > 0 ? argv [0] : NULL;

    if (! arguments_are_defined (argc, argv, result))
        return 0;
    scope_block = lookup_data_scope_from_parameter (scope);
    if (scope_block
    && (scope_block-> total))
      {
        result-> value. type = TYPE_NUMBER;
        result-> value. n    = scope_block-> total;
      }

    return 0;
}


static int
eval_name (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *dummy)
{
    RESULT_NODE
        *object = argc > 0 ? argv [0] : NULL;
    const char
        *name = NULL;
    SCOPE_BLOCK
        *scope_block;

    if (! arguments_are_defined (argc, argv, result))
        return 0;

    if (object && object-> value. type == TYPE_POINTER)
      {
        if (object-> value. c-> item_name)
            name = object-> value. c-> item_name (object-> value. i);
      }
    else
      {
        scope_block = lookup_data_scope_from_parameter (object);
        if (scope_block
        &&  scope_block-> item
        &&  scope_block-> class-> item_name)
            name = scope_block-> class-> item_name (scope_block-> item);
      }

    if (name)
        assign_string (& result-> value, mem_strdup (name));
        
    return 0;
}


static int
eval_class (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *dummy)
{
    SCOPE_BLOCK
        *scope_block;
    RESULT_NODE
        *object = argc > 0 ? argv [0] : NULL;
    const char
        *classname = NULL;

    if (! arguments_are_defined (argc, argv, result))
        return 0;

    if (object
    &&  object-> value. type == TYPE_POINTER
    &&  object-> value. c-> name)
        classname = object-> value. c-> name;
    else
      {
        scope_block = lookup_data_scope_from_parameter (object);
        if (scope_block
        &&  scope_block-> class)
            classname = scope_block-> class-> name;
      }

    if (classname)
      {
        result-> value. type = TYPE_STRING;
        result-> value. s    = mem_strdup (classname);
      }

    return 0;
}


static int
eval_alias (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *dummy)
{
    SCOPE_BLOCK
        *scope_block;
    RESULT_NODE
        *scope = argc > 0 ? argv [0] : NULL;

    if (! arguments_are_defined (argc, argv, result))
        return 0;

    scope_block = lookup_data_scope_from_parameter (scope);
    if (scope_block
    && (scope_block-> name))
      {
        result-> value. type = TYPE_STRING;
        result-> value. s    = mem_strdup (scope_block-> name);
      }

    return 0;
}


static int
eval_defined (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *dummy)
{
    RESULT_NODE
        *expr = argc > 0 ? argv [0] : NULL;

    ASSERT (expr);

    result-> value. n    = expr-> value. type != TYPE_UNDEFINED ? 1 : 0;
    result-> value. type = TYPE_NUMBER;
    return 0;
}


static int
eval_macro (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *dummy)
{
    RESULT_NODE
        *name = argc > 0 ? argv [0] : NULL,
        pseudo = {NULL,
                  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
                  NULL, NULL, NULL, NULL,
                  0, NULL, NULL,
                  0, 0, 0,
                  {TYPE_UNDEFINED, NULL, 0},
                  FALSE };

    if (! arguments_are_defined (argc, argv, result))
        return 0;

    pseudo. name = name;

    result-> value. type = TYPE_NUMBER;
    if (macro_value (& tcb-> scope_stack,
                     & pseudo))
        result-> value. n = 1;
    else
        result-> value. n = 0;

    return 0;
}


static int
eval_scope (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *dummy)
{
    RESULT_NODE
        *scope = argc > 0 ? argv [0] : NULL;
    SCOPE_BLOCK
        *scope_block;
    CLASS_DESCRIPTOR
        *dummy_class;
    void
        *dummy_item;

    if (! arguments_are_defined (argc, argv, result))
        return 0;

    scope_block = lookup_simple_scope (& tcb-> scope_stack,
                                       & scope-> value,
                                       tcb-> gsl-> ignorecase,
                                       & dummy_class,
                                       & dummy_item,
                                       NULL);

    result-> value. type = TYPE_NUMBER;
    if (scope_block)
        result-> value. n = 1;
    else
        result-> value. n = 0;

    return 0;
}

static int
eval_which (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *dummy)
{
    RESULT_NODE
        *attr = argc > 0 ? argv [0] : NULL;
    CLASS_DESCRIPTOR
        *attr_class;
    void
        *attr_item;

    if (attr-> scope)
      {
        strcpy (object_error,
                "No scope permitted in parameter to function 'which'.");
        return -1;
      }

    result-> value. n = symbol_item (& tcb-> scope_stack,
                                     attr,
                                     tcb-> gsl-> ignorecase,
                                     & attr_class,
                                     & attr_item,
                                     NULL);

    if (attr_item)
        result-> value. type = TYPE_NUMBER;

    return 0;
}
