/*---------------------------------------------------------------------------
 *  ggsymb.c - GSL/symb package
 *
 *  Generated from ggsymb by ggobjt.gsl using GSL/4.
 *  DO NOT MODIFY THIS FILE.
 *
 *  For documentation and updates see http://www.imatix.com.
 *---------------------------------------------------------------------------*/

#include "sfl.h"
#include "smt3.h"
#include "gsl.h"                        /*  Project header file              */
#include "ggsymb.h"                     /*  Include header file              */

/*- Macros ------------------------------------------------------------------*/

#define SYMB_NAME "symb"                /*  Symbol                           */

#define matches(s,t)    (s ? (ignorecase ? lexcmp (s, t) == 0 : streq (s, t))   : t == NULL)

/*- Function prototypes -----------------------------------------------------*/

static int
symb_name (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread);
static int
symb_new (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread);
static VALUE * symb_get_attr (void *item, const char *name, Bool ignorecase);
static int symb_put_attr (void *item, const char *name, VALUE *value, Bool ignorecase);
static const char * symb_item_name (void *item);
static int symb_create (const char *name, void *parent, void *sibling, CLASS_DESCRIPTOR **class, void **item);
static void * symb_copy (void *item, CLASS_DESCRIPTOR *to_class, const char *name, void *parent, void *sibling);
static int symb_link (void *item);
static int symb_destroy (void *item);
static void symb_remove (void *item, void *remove);

/*- Global variables --------------------------------------------------------*/
static PARM_LIST parm_list_v            = { PARM_VALUE };

static GSL_FUNCTION symb_functions [] =
{
    {"name",           0, 0, 0, NULL,            1, symb_name},
    {"new",            0, 1, 1, (void *) &parm_list_v, 1, symb_new}};

CLASS_DESCRIPTOR
    symb_class = {
        "symb",
        symb_item_name,
        symb_get_attr,
        symb_put_attr,
        NULL,
        NULL,
        NULL,
        symb_create,
        symb_destroy,
        symb_link,
        NULL,
        symb_copy,
        NULL,
        symb_remove,
        symb_functions, tblsize (symb_functions) };


static Bool
reset_done_flag (SYMTAB_ITEM *symtab_item)
{
    SYMBOL
        *symbol;
    CLASS_ITEM
        *class_item;
        
    if (! symtab_item-> loop)
      {
        symtab_item-> done = FALSE;
        symtab_item-> loop = TRUE;
        if (symtab_item-> symtab)
            for (symbol = symtab_item-> symtab-> symbols; symbol; symbol = symbol-> next)
              {
                class_item = symbol-> data;
            
                if (class_item
                &&  class_item-> class == & symb_class
                &&  class_item-> item)
                    reset_done_flag (class_item-> item);
              }
        symtab_item-> loop = FALSE;
      }
    return FALSE;
}


static Bool
test_unlink_symtab_item (SYMTAB_ITEM *symtab_item, SYMTAB_ITEM *test_item)
{
    SYMBOL
        *symbol;
    CLASS_ITEM
        *class_item;
        
    if (symtab_item == test_item)
        symtab_item-> links--;
    else
    if ((! symtab_item-> done)
    &&     symtab_item-> symtab)
      {
        symtab_item-> done = TRUE;
        if (symtab_item-> symtab)
            for (symbol = symtab_item-> symtab-> symbols; symbol; symbol = symbol-> next)
              {
                class_item = symbol-> data;
            
                if (class_item
                &&  class_item-> class == & symb_class
                &&  class_item-> item)
                    test_unlink_symtab_item (class_item-> item, test_item);
              }
      }
    return test_item-> links > 0;
}

static Bool
free_symbol_data (SYMBOL *symbol)
{
    CLASS_ITEM
        *class_item = symbol-> data;

    if (class_item)
      {
        if (class_item-> class
        &&  class_item-> class-> destroy
        &&  class_item-> item)
            class_item-> class-> destroy (class_item-> item);

        mem_free (class_item);
      }
    return TRUE;
}

static void
delete_symtab_item (SYMTAB_ITEM *symtab_item)
{
    SYMTAB
        *symtab = symtab_item-> symtab;

    if (symtab)
      {
        symtab_item-> symtab = NULL;
        sym_exec_all (symtab, free_symbol_data);
        sym_delete_table (symtab);
      }
    mem_free (symtab_item-> name);
    mem_free (symtab_item);
}

static void
remove_item_from_symtab (SYMTAB *symtab, void *remove)
{
    SYMBOL
        *symbol,
        *next;
    CLASS_ITEM
        *class_item;
        
    for (symbol = symtab-> symbols; symbol; symbol = next)
      {
        next = symbol-> next;           /*  In case symbol gets deleted      */
        class_item = symbol-> data;
    
        if (class_item
        &&  class_item-> class == & symb_class
        &&  class_item-> item)
            remove_item_from_symtab (((SYMTAB_ITEM *) (class_item-> item))-> symtab,
                                     remove);
      
        if (class_item
        &&  class_item-> item == remove)
          {
            if (class_item-> class == & symb_class)
                delete_symtab_item (class_item-> item);
            else
            if (class_item-> class-> destroy)
                class_item-> class-> destroy (class_item-> item);
    
            mem_free (symbol-> data);
            sym_delete_symbol (symtab, symbol);
          }
      }
}

Bool
copy_symbol (SYMBOL *symbol, CLASS_DESCRIPTOR *class, void *item)
{
    void
        *new_item;
    VALUE
        value;
    CLASS_ITEM
        *child;

    if (symbol-> value)
      {
        init_value (& value);
        value. type = TYPE_UNKNOWN;
        value. s    = symbol-> value;

        if (! class-> put_attr (item, symbol-> name, & value, FALSE))
            return TRUE;                    /*  Normal */
        else
            return FALSE;                   /*  Error  */
      }
    else
      {
        child = symbol-> data;
        if (child-> class-> copy
        && (new_item = child-> class-> copy (child-> item,
                                             class,
                                             NULL,
                                             item,
                                             NULL)))
          {
            /*  Destroy new item; otherwise it'll be left dangling  */
            if (class-> destroy)
                class-> destroy (new_item);
            return TRUE;
          }
        else
            return FALSE;
      }
}

static VALUE * symb_get_attr (void *item, const char *name, Bool ignorecase)
{

    char
        *name_lwc;
    SYMTAB_ITEM
        *symtab_item = item;
    SYMBOL
        *symbol = NULL;
    static VALUE
        value;
    CLASS_ITEM
        *class_item;

    ASSERT (symtab_item);
    if (! symtab_item-> symtab)         /*  Item is deleted.  */
        return NULL;

    name_lwc = memt_strdup (NULL, name);
    strlwc (name_lwc);

    init_value (& value);

    if (name_lwc && name_lwc [0])
        symbol = sym_lookup_symbol (symtab_item-> symtab,
                                    name_lwc);
    mem_free (name_lwc);

    if (symbol)
      {
        if (symbol-> value)
          {
            value. s    = symbol-> value;
            value. type = TYPE_UNKNOWN;
            return & value;
          }
        else
        if (symbol-> data)
          {
            class_item  = symbol-> data;

            /*  Build value manually rather than use assign_pointer because   */
            /*  don't want a link made.                                       */
            value. type = TYPE_POINTER;
            value. c    = class_item-> class;
            value. i    = class_item-> item;

            return & value;
          }
      }
    return NULL;
    
}

static int symb_put_attr (void *item, const char *name, VALUE *value, Bool ignorecase)
{

    char
        *name_lwc;
    SYMTAB_ITEM
        *symtab_item = item;
    SYMBOL
        *symbol;
    CLASS_DESCRIPTOR
        *old_class = NULL;
    void
        *old_item = NULL;

    ASSERT (symtab_item);
    if (! symtab_item-> symtab)         /*  Item is deleted.  */
        return -1;

    if (name && name [0])
      {
        name_lwc = memt_strdup (NULL, name);
        strlwc (name_lwc);

        symbol = sym_lookup_symbol (symtab_item-> symtab, name_lwc);
        if (symbol && symbol-> data)
          {
            old_class = ((CLASS_ITEM *) symbol-> data)-> class;
            old_item  = ((CLASS_ITEM *) symbol-> data)-> item;
          }

        if (value && value-> type == TYPE_POINTER)
          {
            if (value-> c-> link
            &&  value-> i)
                value-> c-> link (value-> i);

            if (symbol)
                sym_set_value (symbol, NULL);
            else
                symbol = sym_assume_symbol (symtab_item-> symtab,
                                            name_lwc, NULL);

            if (! symbol-> data)
                symbol-> data = memt_alloc (NULL, sizeof (CLASS_ITEM));

            ((CLASS_ITEM *) symbol-> data)-> class = value-> c;
            ((CLASS_ITEM *) symbol-> data)-> item  = value-> i;
          }
        else
        if ((! value) || value-> type == TYPE_UNDEFINED)
          {
            if (symbol)
              {
                mem_free (symbol-> data);
                sym_delete_symbol (symtab_item-> symtab, symbol);
              }
          }
        else
          {
            if (symbol)
              {
                if ((! symbol-> value)
                ||  (! streq (symbol-> value, string_value (value))))
                    sym_set_value (symbol, value-> s);

                if (symbol-> data)
                  {
                    mem_free (symbol-> data);
                    symbol-> data = NULL;
                  }
              }
            else
                sym_assume_symbol (symtab_item-> symtab, name_lwc,
                                   string_value (value));
          }

        if (old_class
        &&  old_class-> destroy
        &&  old_item)
            old_class-> destroy (old_item);

        mem_free (name_lwc);
        return 0;
      }
    return -1;
    
}

static const char * symb_item_name (void *item)
{
    
ASSERT (item);
if (! ((SYMTAB_ITEM *) item)-> symtab)
    return NULL;

return ((SYMTAB_ITEM *) item)-> name;
    
}

static int symb_create (const char *name, void *parent, void *sibling, CLASS_DESCRIPTOR **class, void **item)
{
    
SYMTAB_ITEM
    *symtab_item;

symtab_item = memt_alloc (NULL, sizeof (SYMTAB_ITEM));
symtab_item-> links   = 0;
symtab_item-> name    = memt_strdup (NULL, name);
symtab_item-> symtab  = sym_create_table ();
symtab_item-> loop    = FALSE;

*class = & symb_class;
*item  = symtab_item;

return 0;
    
}

static void * symb_copy (void *item, CLASS_DESCRIPTOR *to_class, const char *name, void *parent, void *sibling)
{
    
    SYMBOL
        *symbol;
    SYMTAB_ITEM
        *symtab_item = item;
    CLASS_DESCRIPTOR
        *new_class;
    void
        *new_item = NULL;
    int
        rc = -1;

    if (to_class-> create)
        rc = to_class-> create (name ? name : symtab_item-> name,
                                parent, sibling,
                                &new_class, &new_item);

    if ((! rc)
    &&  new_item
    &&  new_class-> put_attr)
        for (symbol = symtab_item-> symtab-> symbols; symbol; symbol = symbol-> next)
            copy_symbol (symbol, new_class, new_item);
            
    if (rc >= 0)
        return new_item;
    else
      {
        if (new_class-> destroy)
            new_class-> destroy (new_item);

        return NULL;
      }
    
}

static int symb_link (void *item)
{
    
if (item)
    ((SYMTAB_ITEM *) item)-> links++;

return 0;
    
}

static int symb_destroy (void *item)
{
    
SYMTAB_ITEM
    *symtab_item = item;
int
    save_links;
SYMBOL
    *symbol;
CLASS_ITEM
    *class_item;

if (symtab_item)
  {
    /*  If all links are gone then delete now.  */
    symtab_item-> links--;
    save_links = symtab_item-> links;

    if (symtab_item-> links > 0)
      {
        /*  Try to find a loop that will remove all links  */
        reset_done_flag (symtab_item);
        
        for (symbol = symtab_item-> symtab-> symbols; symbol; symbol = symbol-> next)
            {
            class_item = symbol-> data;
        
            if (class_item
            &&  class_item-> class == & symb_class
            &&  class_item-> item)
                test_unlink_symtab_item (class_item-> item, symtab_item);
            }
      }
    if (symtab_item-> links == 0)
        delete_symtab_item (symtab_item);
    else
        symtab_item-> links = save_links;
  }
return 0;
    
}

static void symb_remove (void *item, void *remove)
{
    
    SYMTAB_ITEM
        *symtab_item = item;
        
    remove_item_from_symtab (symtab_item-> symtab, remove);
    
}


static int
symb_name (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread)
{


  {
    char
        *item_name;

    item_name = mem_strdup ((char *) symb_item_name
                                         (item));

    if (item_name)
        assign_string (& result-> value, item_name);

    return 0;
  }
        
    return 0;  /*  Just in case  */
}


static int
symb_new (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread)
{
    RESULT_NODE *name    = argc > 0 ? argv [0] : NULL;


  {
    int
        rc;
    CLASS_DESCRIPTOR
        *returnclass;
    void
        *returnitem;

    rc = symb_create
             (name ? string_value (& name-> value) : NULL,
              item,
              NULL,
              & returnclass,
              & returnitem);

    if ((! rc)
    &&  item)
        assign_pointer (& result-> value, returnclass, returnitem);

    return rc;
  }
        
    return 0;  /*  Just in case  */
}

int register_symb_classes (void)
{
    int
        rc = 0;
    return rc;
}
