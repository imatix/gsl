/*---------------------------------------------------------------------------
 *  ggxml.c - GSL/XML package
 *
 *  Generated from ggxml by ggobjt.gsl using GSL/4.
 *  DO NOT MODIFY THIS FILE.
 *
 *  For documentation and updates see http://www.imatix.com.
 *---------------------------------------------------------------------------*/

#include "sfl.h"
#include "smt3.h"
#include "gsl.h"                        /*  Project header file              */
#include "ggxml.h"                      /*  Include header file              */

/*- Macros ------------------------------------------------------------------*/

#define XML_NAME "xml"                  /*  XML                              */
#define XML_ITEM_NAME "xml item"        /*  XML item                         */
#define XML_VALUE_NAME "xml value"      /*  XML value                        */

#define matches(s,t)    (s ? (ignorecase ? lexcmp (s, t) == 0 : streq (s, t))   : t == NULL)

/*- Function prototypes -----------------------------------------------------*/

static int
XML_new (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread);
static int
XML_load_string (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread);
static int
XML_load_file (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread);
static int
XML_item_deleted (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread);
static int
XML_item_prev (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread);
static int
XML_item_string (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread);
static int
XML_item_load_string (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread);
static int
XML_item_load_file (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread);
static int
XML_item_save (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread);
static int
XML_item_name (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread);
static int
XML_item_first (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread);
static int
XML_item_next (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread);
static int
XML_item_parent_function (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread);
static int
XML_item_new (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread);
static int
XML_item_delete_function (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread);
static int
XML_value_name (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread);
static int
XML_value_next (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread);
static int
XML_value_parent_function (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread);
static int
XML_value_delete_function (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread);
static int XML_link (void *item);
static int XML_destroy (void *item);
static VALUE * XML_get_attr (void *item, const char *name, Bool ignorecase);
static int XML_item_link (void *item);
static int XML_item_destroy (void *item);
static const char * XML_item_item_name (void *item);
static VALUE * XML_item_get_attr (void *item, const char *name, Bool ignorecase);
static int XML_item_put_attr (void *item, const char *name, VALUE *value, Bool ignorecase);
static int XML_item_first_child (void *olditem, const char *name, Bool ignorecase, CLASS_DESCRIPTOR **class, void **item);
static int XML_item_next_sibling (void *olditem, const char *name, Bool ignorecase, CLASS_DESCRIPTOR **class, void **item);
static int XML_item_parent (void *olditem, CLASS_DESCRIPTOR **class, void **item, THREAD *gsl_thread);
static int XML_item_create (const char *name, void *parent, void *sibling, CLASS_DESCRIPTOR **class, void **item);
static int XML_item_delete (void *item);
static void * XML_item_copy (void *item, CLASS_DESCRIPTOR *to_class, const char *name, void *parent, void *sibling);
static void * XML_item_move (void *item, CLASS_DESCRIPTOR *to_class, const char *name, void *parent, void *sibling);
static int XML_value_link (void *item);
static int XML_value_destroy (void *item);
static const char * XML_value_item_name (void *item);
static VALUE * XML_value_get_attr (void *item, const char *name, Bool ignorecase);
static int XML_value_put_attr (void *item, const char *name, VALUE *value, Bool ignorecase);
static int XML_value_next_sibling (void *olditem, const char *name, Bool ignorecase, CLASS_DESCRIPTOR **class, void **item);
static int XML_value_parent (void *olditem, CLASS_DESCRIPTOR **class, void **item, THREAD *gsl_thread);
static int XML_value_delete (void *item);
static void * XML_value_copy (void *item, CLASS_DESCRIPTOR *to_class, const char *name, void *parent, void *sibling);
static void * XML_value_move (void *item, CLASS_DESCRIPTOR *to_class, const char *name, void *parent, void *sibling);

/*- Global variables --------------------------------------------------------*/
static PARM_LIST parm_list_vr           = { PARM_VALUE,
                                            PARM_REFERENCE };

static GSL_FUNCTION XML_functions [] =
{
    {"load_file",      1, 2, 2, (void *) &parm_list_vr, 1, XML_load_file},
    {"load_string",    1, 2, 2, (void *) &parm_list_vr, 1, XML_load_string},
    {"new",            0, 1, 1, (void *) &parm_list_vr, 1, XML_new}};

CLASS_DESCRIPTOR
    XML_class = {
        "XML",
        NULL,
        XML_get_attr,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        XML_destroy,
        XML_link,
        NULL,
        NULL,
        NULL,
        NULL,
        XML_functions, tblsize (XML_functions) };

static GSL_FUNCTION XML_item_functions [] =
{
    {"delete",         0, 0, 0, NULL,            1, XML_item_delete_function},
    {"deleted",        0, 0, 0, NULL,            1, XML_item_deleted},
    {"first",          0, 1, 1, (void *) &parm_list_vr, 1, XML_item_first},
    {"load_file",      1, 2, 2, (void *) &parm_list_vr, 1, XML_item_load_file},
    {"load_string",    1, 2, 2, (void *) &parm_list_vr, 1, XML_item_load_string},
    {"name",           0, 0, 0, NULL,            1, XML_item_name},
    {"new",            0, 1, 1, (void *) &parm_list_vr, 1, XML_item_new},
    {"next",           0, 1, 1, (void *) &parm_list_vr, 1, XML_item_next},
    {"parent",         0, 0, 0, NULL,            1, XML_item_parent_function},
    {"prev",           0, 0, 0, NULL,            1, XML_item_prev},
    {"save",           1, 2, 2, (void *) &parm_list_vr, 1, XML_item_save},
    {"string",         0, 0, 0, NULL,            1, XML_item_string}};

CLASS_DESCRIPTOR
    XML_item_class = {
        "XML item",
        XML_item_item_name,
        XML_item_get_attr,
        XML_item_put_attr,
        XML_item_first_child,
        XML_item_next_sibling,
        XML_item_parent,
        XML_item_create,
        XML_item_destroy,
        XML_item_link,
        XML_item_delete,
        XML_item_copy,
        XML_item_move,
        NULL,
        XML_item_functions, tblsize (XML_item_functions) };

static GSL_FUNCTION XML_value_functions [] =
{
    {"delete",         0, 0, 0, NULL,            1, XML_value_delete_function},
    {"name",           0, 0, 0, NULL,            1, XML_value_name},
    {"next",           0, 1, 1, (void *) &parm_list_vr, 1, XML_value_next},
    {"parent",         0, 0, 0, NULL,            1, XML_value_parent_function}};

CLASS_DESCRIPTOR
    XML_value_class = {
        "XML value",
        XML_value_item_name,
        XML_value_get_attr,
        XML_value_put_attr,
        NULL,
        XML_value_next_sibling,
        XML_value_parent,
        NULL,
        XML_value_destroy,
        XML_value_link,
        XML_value_delete,
        XML_value_copy,
        XML_value_move,
        NULL,
        XML_value_functions, tblsize (XML_value_functions) };


/*- Definitions -------------------------------------------------------------*/

#define xml_field(i) (((GSL_XML_ITEM *) i)-> xml_item)

/*- Type definitions --------------------------------------------------------*/

typedef struct {
    int
        links;
    char
        *error_msg;
} XML_CONTEXT;

typedef struct {
    int
        links;
    XML_ITEM
        *xml_item;
} GSL_XML_ITEM;

/*- Global variables used in this source file only --------------------------*/

static XML_ITEM
    *ancestor;

/*- Functions ---------------------------------------------------------------*/

static int
store_xml_error (THREAD      *gsl_thread,
                 XML_CONTEXT *context,
                 RESULT_NODE *error,
                 const char  *error_msg)
{
    GGCODE_TCB
        *gsl_tcb = gsl_thread-> tcb;
    VALUE
        value;
    char
        *error_text;

    if (error_msg)
      {
        if (! context)
            context = get_class_item (gsl_thread, XML_NAME);
        mem_free (context-> error_msg);
        context-> error_msg = memt_strdup (NULL, error_msg);

        if (error)
          {
            init_value (& value);
            assign_string (& value, context-> error_msg);

            if (! store_symbol_definition (& gsl_tcb-> scope_stack,
                                           gsl_tcb-> gsl-> ignorecase,
                                           error,
                                           &value,
                                           &error_text))
              {
                strncpy (object_error, error_text, LINE_MAX);
                return -1;
              }
          }
      }
    return 0;
}

/*  compound_item_value:  Concatentates all the value nodes of an item into  */
/*  a single string and assigns that string to the value of the given node.  */
/*  This is not really XML, since non-value items do not have values, so     */
/*  this value will not appear if the XML is saved.  We need to do this so   */
/*  that the memory allocated to hold the compound string will get           */
/*  deallocated.                                                             */

static char *
compound_item_value (XML_ITEM *item)
{
    XML_ITEM
        *child;
    char
        *name,
        *value,
        *compound = NULL;
    size_t
        length = 0;

    if (xml_item_name (item))
      {
        child = xml_first_child (item);
        while (child)
          {
            name = xml_item_name (child);
            if (! name)           /*  This is a value node  */
              {
                value = xml_item_value (child);
                if (value)
                  {
                    if (compound)
                        compound = mem_realloc (compound,
                                                length + strlen (value) + 1);
                    else
                        compound = memt_alloc (NULL, strlen (value) + 1);

                    ASSERT (compound);
                    strcpy (compound + length, value);

                    length += strlen (value);
                  }
              }
            child = xml_next_sibling (child);
          }
        xml_modify_value (item, compound);
        mem_free (compound);
      }
    return xml_item_value (item);
}


void *
get_gsl_xml_item (XML_ITEM *xml_item)
{
    GSL_XML_ITEM
        *gsl_xml_item;

    gsl_xml_item = xml_get_data (xml_item);
    if (! gsl_xml_item)
      {
        gsl_xml_item = mem_alloc (sizeof (GSL_XML_ITEM));
        gsl_xml_item-> links    = 0;
        gsl_xml_item-> xml_item = xml_item;

        xml_set_data (xml_item, gsl_xml_item);
      }
    return gsl_xml_item;
}

XML_ITEM *
get_xml_item (void *gsl_xml_item)
{
    return xml_field (gsl_xml_item);
}

static void
get_matching_item (XML_ITEM          *xml_item,
                   const char        *name,
                   Bool               ignorecase,
                   CLASS_DESCRIPTOR **class,
                   void             **item)
{
    const char
        *item_name;

    while (xml_item)
      {
        item_name = xml_item_name (xml_item);
        if (name && name [0])
          {
            if (ignorecase)
              {
                if (item_name
                &&  lexcmp (item_name, name) == 0)
                    break;
              }
            else
              {
                if (item_name
                &&  streq (item_name, name))
                    break;
              }
          }
        else
            /*  If no item name specified, take all children  */
            break;
            
        xml_item = xml_next_sibling (xml_item);
      }

    if (xml_item)
      {
        if (item_name)
            *class = & XML_item_class;
        else
            *class = & XML_value_class;
            
        *item  = get_gsl_xml_item (xml_item);
      }
    else
      {
        *class = NULL;
        *item  = NULL;
      }
}

static Bool
delete_xml_item (XML_ITEM *xml_item)
{
    GSL_XML_ITEM
        *gsl_xml_item = xml_get_data (xml_item);

    /*  Remove link to deleted XML item  */
    if (gsl_xml_item)
        gsl_xml_item-> xml_item = NULL;

    /*  And really delete the item  */
    xml_free (xml_item);

    return TRUE;    /*  Keep on traversing the tree  */
}


static void *
copy_xml_item (XML_ITEM         *xml_item,
               CLASS_DESCRIPTOR *to_class,
               const char       *name,
               void             *parent,
               void             *sibling)
{
    int
        rc = -1;
    CLASS_DESCRIPTOR
        *new_class;
    void
        *new_item = NULL;
    XML_ATTR
        *xml_attr;
    XML_ITEM
        *xml_child;
    VALUE
        value;

    /*  Be sure XML hasn't been deleted.  */
    if (! xml_item)
       return NULL;

    /*  Simple case - copying from XML to XML  */
    if (to_class == &XML_item_class
    || (sibling
    &&  to_class == &XML_value_class))
    
      {
        new_item = xml_create (name ? name
                                    : xml_item_name (xml_item),
                               xml_item_value (xml_item));
        xml_copy (new_item, xml_item);
        if (sibling)
            xml_attach_sibling (xml_field (sibling), new_item);
        else
            xml_attach_child   (xml_field (parent),  new_item);

        return get_gsl_xml_item (new_item);
      }
    /*  General case - copying to another class  */
    else
      {
        if (to_class-> create)
            rc = to_class-> create (name ? name : xml_item_name (xml_item),
                                    parent, sibling,
                                    &new_class, &new_item);
    
        init_value (& value);
        value. type = TYPE_UNKNOWN;
        
        if ((! rc)
        &&  new_item
        &&  new_class-> put_attr)
          {
            /*  Copy XML value  */
            value. s = xml_item_value (xml_item);
            if (value. s)
                rc = new_class-> put_attr (new_item,
                                           NULL, & value,
                                           FALSE);
                                           
            /*  Copy XML attributes  */
            FORATTRIBUTES (xml_attr, xml_item)
              {
                value. s = xml_attr_value (xml_attr);;
                rc = new_class-> put_attr (new_item,
                                           xml_attr_name (xml_attr), & value,
                                           FALSE);
              }
              
            /*  Copy XML children  */
            if (! rc)
                for (xml_child  = xml_first_child (xml_item);
                     xml_child != NULL;
                     xml_child  = xml_next_sibling (xml_child))
                    if (! copy_xml_item (xml_child,
                                         new_class,
                                         NULL,
                                         new_item, NULL))
                      {
                        rc = -1;
                        break;
                      }
    
            if (rc)
              {
                if (new_class-> destroy)
                    new_class-> destroy (new_item);
                new_item = NULL;
              }
          }
        return new_item;
      }
}

static void
load_xml_item (XML_ITEM *parent,
               XML_ITEM *new_root,
               VALUE    *value)
{
    XML_ITEM
        *xml_item,
        *xml_result,
        *xml_next;

    xml_result = xml_first_child (new_root);
    xml_item = xml_result;
    while (xml_item)
      {
        xml_next = xml_next_sibling (xml_item);
        xml_detach (xml_item);

        if (parent)
            xml_attach_child (parent, xml_item);
        else
        /*  If no parent then all items apart from first one are dropped */
        if (xml_item != xml_result)
            xml_free (xml_item);

        xml_item = xml_next;
      }
    assign_pointer (value,
                    & XML_item_class,
                    get_gsl_xml_item (xml_result));
}

static int
load_xml_string (THREAD      *gsl_thread,
                 XML_CONTEXT *context,
                 XML_ITEM    *parent,
                 const char  *string,
                 RESULT_NODE *result,
                 RESULT_NODE *error)
{
    XML_ITEM
        *new_root = NULL;
    int
        rc = 0;

    if (xml_load_string (& new_root,
                         string,
                         TRUE) == XML_NOERROR)
        load_xml_item (parent,
                       new_root,
                       & result-> value);
    else
        rc = store_xml_error (gsl_thread,
                              context,
                              error,
                              xml_error ());

    xml_free (new_root);
    return rc;
}

static int
load_xml_file (THREAD      *gsl_thread,
               XML_CONTEXT *context,
               XML_ITEM    *parent,
               const char  *filename,
               RESULT_NODE *result,
               RESULT_NODE *error)
{
    XML_ITEM
        *new_root = NULL;
    int
        rc = 0;

    if (xml_load_file (& new_root,
                       PATH,
                       filename,
                       TRUE) == XML_NOERROR)
        load_xml_item (parent,
                       new_root,
                       & result-> value);
    else
        rc = store_xml_error (gsl_thread,
                              context,
                              error,
                              xml_error ());

    xml_free (new_root);
    return rc;
}

static void
report_xml_deleted_error (void)
{
    strcpy (object_error, "XML structure has been deleted.");
}

static int XML_link (void *item)
{
    
((XML_CONTEXT *) item)-> links++;
return 0;
    
}

static int XML_destroy (void *item)
{
    
  {
    XML_CONTEXT
        *context = item;

    if (--context-> links == 0)
      {
        mem_free (context-> error_msg);
        mem_free (context);
      }
    return 0;
  }
    
}

static VALUE * XML_get_attr (void *item, const char *name, Bool ignorecase)
{

    XML_CONTEXT
        *context = item;
    static VALUE
        value;

    if (! name)
        return NULL;

    init_value (& value);
        
    if (matches (name, "error"))
      {

        if (context-> error_msg)
            assign_string (& value, context-> error_msg);
        
      }

    return & value;
        
}

static int XML_item_link (void *item)
{
    
    if (item)
        ((GSL_XML_ITEM *) item)-> links++;

    return 0;
    
}

static int XML_item_destroy (void *item)
{
    
    GSL_XML_ITEM
        *gsl_xml_item = item;

    ASSERT (gsl_xml_item-> links > 0);

    if (gsl_xml_item
    &&  --gsl_xml_item-> links <= 0)
      {
        /*  Erase link in XML data  */
        if (gsl_xml_item-> xml_item)
            xml_set_data (gsl_xml_item-> xml_item, NULL);
        mem_free (gsl_xml_item);
      }
    return 0;
    
}

static const char * XML_item_item_name (void *item)
{
    
    return xml_field (item) ? xml_item_name (xml_field (item)) : NULL;
    
}

static VALUE * XML_item_get_attr (void *item, const char *name, Bool ignorecase)
{

    static VALUE
        value;

    init_value (& value);

    /*  Be sure XML hasn't been deleted.  */
    if (xml_field (item))
      {
        if (name && name [0])
            value. s = xml_get_attr_ic (xml_field (item),
                                        name,
                                        NULL,
                                        ignorecase);
        else
            value. s = compound_item_value (xml_field (item));
      }
    if (value. s)
      {
        value. type = TYPE_UNKNOWN;
        return & value;
      }
    else
        return NULL;
    
}

static int XML_item_put_attr (void *item, const char *name, VALUE *value, Bool ignorecase)
{

    XML_ITEM
        *child_item,
        *value_item;
    char
        *item_name;

    if (value)
      {
        if (value-> type == TYPE_POINTER)
           return -1;

        string_value (value);
      }
    /*  Be sure XML hasn't been deleted.  */
    if (!xml_field (item))
       return -1;

    if (name && name [0])
        xml_put_attr_ic (xml_field (item),
                         name,
                         value ? value-> s : NULL,
                         ignorecase,
                         TRUE);
    else
      {
        /*  Modifying an item's value means getting rid of any existing  */
        /*  value nodes then adding one value node with the new value.   */
        child_item = xml_first_child (xml_field (item));
        while (child_item)
          {
            item_name = xml_item_name (child_item);
            value_item = child_item;
            child_item = xml_next_sibling (child_item);
            if (! item_name)           /*  This is a value node  */
                xml_free (value_item);
          }
        if (value && value-> s [0])
            xml_new (xml_field (item),
                     NULL,       /*  Value node  */
                     value-> s);
      }
    return 0;
    
}

static int XML_item_first_child (void *olditem, const char *name, Bool ignorecase, CLASS_DESCRIPTOR **class, void **item)
{
    
    /*  Be sure XML hasn't been deleted.  */
    if (! xml_field (olditem))
       return -1;

    get_matching_item (xml_first_child (xml_field (olditem)),
                       name,
                       ignorecase,
                       class,
                       item);
    return 0;
    
}

static int XML_item_next_sibling (void *olditem, const char *name, Bool ignorecase, CLASS_DESCRIPTOR **class, void **item)
{
    
    /*  Be sure XML hasn't been deleted.  */
    if (!xml_field (olditem))
       return -1;

    get_matching_item (xml_next_sibling (xml_field (olditem)),
                       name,
                       ignorecase,
                       class,
                       item);
    return 0;
    
}

static int XML_item_parent (void *olditem, CLASS_DESCRIPTOR **class, void **item, THREAD *gsl_thread)
{
    
  {
    XML_ITEM
        *xml_item;

    /*  Be sure XML hasn't been deleted.  */
    if (!xml_field (olditem))
       return -1;

    xml_item = xml_parent (xml_field (olditem));
    if (xml_item)
      {
        *class = & XML_item_class;
        *item  = get_gsl_xml_item (xml_item);
      }
    else
      {
        *class = NULL;
        *item  = NULL;
      }
    return 0;
  }
    
}

static int XML_item_create (const char *name, void *parent, void *sibling, CLASS_DESCRIPTOR **class, void **item)
{
    
    XML_ITEM
        *xml_item;

    /*  Be sure XML hasn't been deleted.  */
    if (sibling
    && (! xml_field (sibling)))
        return -1;
    if (parent
    && (! xml_field (parent)))
        return -1;

    xml_item = xml_create (name, NULL);
    if (sibling)
        xml_attach_sibling (xml_field (sibling), xml_item);
    else
        xml_attach_child   (xml_field (parent),  xml_item);

    *class = & XML_item_class;
    *item  = get_gsl_xml_item (xml_item);

    return 0;
    
}

static int XML_item_delete (void *item)
{
    
    /*  Be sure XML hasn't been deleted.  */
    if (!xml_field (item))
       return -1;

    xml_exec_all (xml_field (item), delete_xml_item);
    return 0;
    
}

static void * XML_item_copy (void *item, CLASS_DESCRIPTOR *to_class, const char *name, void *parent, void *sibling)
{
    
    return copy_xml_item (xml_field (item),
                          to_class,
                          name,
                          parent,
                          sibling);
    
}

static void * XML_item_move (void *item, CLASS_DESCRIPTOR *to_class, const char *name, void *parent, void *sibling)
{
    
    if (! (to_class == &XML_item_class
    ||    (sibling
    &&    (to_class == &XML_value_class))))
        return NULL;

    /*  Be sure XML hasn't been deleted.  */
    if (!xml_field (item))
       return NULL;

    xml_detach (xml_field (item));
    if (name)
        xml_rename (xml_field (item),
                    name);

    if (sibling)
        xml_attach_sibling (xml_field (sibling), xml_field (item));
    else
        xml_attach_child   (xml_field (parent),  xml_field (item));

    return item;
    
}

static int XML_value_link (void *item)
{
    
    if (item)
        ((GSL_XML_ITEM *) item)-> links++;

    return 0;
    
}

static int XML_value_destroy (void *item)
{
    
    GSL_XML_ITEM
        *gsl_xml_item = item;

    ASSERT (gsl_xml_item-> links > 0);

    if (gsl_xml_item
    &&  --gsl_xml_item-> links <= 0)
      {
        /*  Erase link in XML data  */
        if (gsl_xml_item-> xml_item)
          {
            xml_set_data (gsl_xml_item-> xml_item, NULL);
            
            /*  If no text then delete item  */
            if (! xml_item_value (gsl_xml_item-> xml_item))
              {
                xml_free (gsl_xml_item-> xml_item);
                gsl_xml_item-> xml_item = NULL;
                  }
          }
        mem_free (gsl_xml_item);
      }
    return 0;
    
}

static const char * XML_value_item_name (void *item)
{
    
    return NULL;
    
}

static VALUE * XML_value_get_attr (void *item, const char *name, Bool ignorecase)
{

    static VALUE
        value;

    init_value (& value);

    /*  Be sure XML hasn't been deleted and request is for value.  */
    if (xml_field (item)
    && (! (name && name [0])))
      {
        value. s = xml_item_value (xml_field (item));
        value. type = TYPE_UNKNOWN;
        return & value;
      }
    else
        return NULL;
    
}

static int XML_value_put_attr (void *item, const char *name, VALUE *value, Bool ignorecase)
{

    if (value)
      {
        if (value-> type == TYPE_POINTER)
           return -1;

        string_value (value);
      }
    /*  Be sure XML hasn't been deleted.  */
    if (!xml_field (item))
       return -1;
    
    /*  Only value assignment is allowed  */
    if (name && name [0])
        return -1;
        
    xml_modify_value (xml_field (item), value ? value-> s : NULL);
    
    return 0;
    
}

static int XML_value_next_sibling (void *olditem, const char *name, Bool ignorecase, CLASS_DESCRIPTOR **class, void **item)
{
    
    /*  Be sure XML hasn't been deleted.  */
    if (!xml_field (olditem))
       return -1;

    get_matching_item (xml_next_sibling (xml_field (olditem)),
                       name,
                       ignorecase,
                       class,
                       item);
    return 0;
    
}

static int XML_value_parent (void *olditem, CLASS_DESCRIPTOR **class, void **item, THREAD *gsl_thread)
{
    
  {
    XML_ITEM
        *xml_item;

    /*  Be sure XML hasn't been deleted.  */
    if (!xml_field (olditem))
       return -1;

    xml_item = xml_parent (xml_field (olditem));
    if (xml_item)
      {
        *class = & XML_item_class;
        *item  = get_gsl_xml_item (xml_item);
      }
    else
      {
        *class = NULL;
        *item  = NULL;
      }
    return 0;
  }
    
}

static int XML_value_delete (void *item)
{
    
    /*  Be sure XML hasn't been deleted.  */
    if (!xml_field (item))
       return -1;

    delete_xml_item (xml_field (item));
    return 0;
    
}

static void * XML_value_copy (void *item, CLASS_DESCRIPTOR *to_class, const char *name, void *parent, void *sibling)
{
    
    return copy_xml_item (xml_field (item),
                          to_class,
                          name,
                          parent,
                          sibling);
    
}

static void * XML_value_move (void *item, CLASS_DESCRIPTOR *to_class, const char *name, void *parent, void *sibling)
{
    
    if (! (to_class == &XML_item_class
    ||    (sibling
    &&    (to_class == &XML_value_class))))
        return NULL;

    /*  Be sure XML hasn't been deleted.  */
    if (!xml_field (item))
       return NULL;

    /*  Renaming a value item is disallowed.  */
    if (name)
        return NULL;
    
    xml_detach (xml_field (item));
    if (sibling)
        xml_attach_sibling (xml_field (sibling), xml_field (item));
    else
        xml_attach_child   (xml_field (parent),  xml_field (item));

    return item;
    
}


static int
XML_new (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread)
{
    RESULT_NODE *name    = argc > 0 ? argv [0] : NULL;


  {
    XML_ITEM
        *xml_item;

    xml_item = xml_new (ancestor,
                        name ? string_value (& name-> value) : NULL,
                        NULL);
    assign_pointer (& result-> value,
                    & XML_item_class,
                    get_gsl_xml_item (xml_item));
  }
        
    return 0;  /*  Just in case  */
}


static int
XML_load_string (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread)
{
    RESULT_NODE *string  = argc > 0 ? argv [0] : NULL;
    RESULT_NODE *error   = argc > 1 ? argv [1] : NULL;

    if (! string)
      {
        strcpy (object_error, "Missing argument: string");
        return -1;
      }
    if (string-> value. type == TYPE_UNDEFINED)
      {
        result-> culprit = string-> culprit;
        string-> culprit = NULL;
        return 0;
      }

    return load_xml_string (gsl_thread,
                            item,
                            ancestor,
                            string_value (& string-> value),
                            result,
                            error);
        
    return 0;  /*  Just in case  */
}


static int
XML_load_file (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread)
{
    RESULT_NODE *filename = argc > 0 ? argv [0] : NULL;
    RESULT_NODE *error   = argc > 1 ? argv [1] : NULL;

    if (! filename)
      {
        strcpy (object_error, "Missing argument: filename");
        return -1;
      }
    if (filename-> value. type == TYPE_UNDEFINED)
      {
        result-> culprit = filename-> culprit;
        filename-> culprit = NULL;
        return 0;
      }

    return load_xml_file (gsl_thread,
                          item,
                          ancestor,
                          string_value (& filename-> value),
                          result,
                          error);
        
    return 0;  /*  Just in case  */
}


static int
XML_item_deleted (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread)
{


    assign_number (& result-> value, (! xml_field (item)) ? 1 : 0);
        
    return 0;  /*  Just in case  */
}


static int
XML_item_prev (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread)
{


    XML_ITEM
        *xml_item;

    /*  Be sure XML hasn't been deleted.  */
    if (! (xml_item = xml_field (item)))
       return 0;

    xml_item = xml_prev_sibling (xml_item);

    if (xml_item)
        assign_pointer (& result-> value,
                        & XML_item_class, get_gsl_xml_item (xml_item));

    return 0;
        
    return 0;  /*  Just in case  */
}


static int
XML_item_string (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread)
{


    /*  Be sure XML hasn't been deleted.  */
    if (!xml_field (item))
       return 0;

    assign_string (& result-> value, xml_save_string (xml_field (item)));
    return 0;
        
    return 0;  /*  Just in case  */
}


static int
XML_item_load_string (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread)
{
    RESULT_NODE *string  = argc > 0 ? argv [0] : NULL;
    RESULT_NODE *error   = argc > 1 ? argv [1] : NULL;

    if (! string)
      {
        strcpy (object_error, "Missing argument: string");
        return -1;
      }
    if (string-> value. type == TYPE_UNDEFINED)
      {
        result-> culprit = string-> culprit;
        string-> culprit = NULL;
        return 0;
      }

    /*  Be sure XML hasn't been deleted.  */
    if (!xml_field (item))
      {
        report_xml_deleted_error ();
        return -1;
      }
    return load_xml_string (gsl_thread,
                            NULL,
                            xml_field (item),
                            string_value (& string-> value),
                            result,
                            error);
        
    return 0;  /*  Just in case  */
}


static int
XML_item_load_file (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread)
{
    RESULT_NODE *filename = argc > 0 ? argv [0] : NULL;
    RESULT_NODE *error   = argc > 1 ? argv [1] : NULL;

    if (! filename)
      {
        strcpy (object_error, "Missing argument: filename");
        return -1;
      }
    if (filename-> value. type == TYPE_UNDEFINED)
      {
        result-> culprit = filename-> culprit;
        filename-> culprit = NULL;
        return 0;
      }

    /*  Be sure XML hasn't been deleted.  */
    if (!xml_field (item))
      {
        report_xml_deleted_error ();
        return -1;
      }
    return load_xml_file (gsl_thread,
                          NULL,
                          xml_field (item),
                          string_value (& filename-> value),
                          result,
                          error);
        
    return 0;  /*  Just in case  */
}


static int
XML_item_save (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread)
{
    RESULT_NODE *filename = argc > 0 ? argv [0] : NULL;
    RESULT_NODE *error   = argc > 1 ? argv [1] : NULL;

    if (! filename)
      {
        strcpy (object_error, "Missing argument: filename");
        return -1;
      }
    if (filename-> value. type == TYPE_UNDEFINED)
      {
        result-> culprit = filename-> culprit;
        filename-> culprit = NULL;
        return 0;
      }

  {
    int
        rc;

    /*  Be sure XML hasn't been deleted.  */
    if (!xml_field (item))
      {
        report_xml_deleted_error ();
        return -1;
      }
    rc = xml_save_file (xml_field (item), string_value (& filename-> value));
    assign_number (& result-> value, 0);
    if (rc == XML_FILEERROR)
      {
        result-> value. n = errno;
        return store_xml_error (gsl_thread,
                                NULL,
                                error,
                                strerror (errno));
      }
  }
        
    return 0;  /*  Just in case  */
}


static int
XML_item_name (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread)
{


  {
    char
        *item_name;

    item_name = mem_strdup ((char *) XML_item_item_name
                                         (item));

    if (item_name)
        assign_string (& result-> value, item_name);

    return 0;
  }
        
    return 0;  /*  Just in case  */
}


static int
XML_item_first (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread)
{
    RESULT_NODE *name    = argc > 0 ? argv [0] : NULL;


  {
    int
        rc;
    CLASS_DESCRIPTOR
        *returnclass;
    void
        *returnitem;

    rc = XML_item_first_child
             (item,
              name ? string_value (& name-> value) : NULL,
              ((GGCODE_TCB *) gsl_thread-> tcb)-> gsl-> ignorecase,
              & returnclass,
              & returnitem);

    if ((! rc)
    &&  returnitem)
        assign_pointer (& result-> value, returnclass, returnitem);

    return rc;
  }
        
    return 0;  /*  Just in case  */
}


static int
XML_item_next (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread)
{
    RESULT_NODE *name    = argc > 0 ? argv [0] : NULL;


  {
    int
        rc;
    CLASS_DESCRIPTOR
        *returnclass;
    void
        *returnitem;

    rc = XML_item_next_sibling
             (item,
              name ? string_value (& name-> value) : NULL,
              ((GGCODE_TCB *) gsl_thread-> tcb)-> gsl-> ignorecase,
              & returnclass,
              & returnitem);

    if ((! rc)
    &&  item)
        assign_pointer (& result-> value, returnclass, returnitem);

    return rc;
}
        
    return 0;  /*  Just in case  */
}


static int
XML_item_parent_function (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread)
{


  {
    int
        rc;
    CLASS_DESCRIPTOR
        *returnclass;
    void
        *returnitem;

    rc = XML_item_parent
             (item,
              & returnclass,
              & returnitem,
              gsl_thread);

    if ((! rc)
    &&  item)
        assign_pointer (& result-> value, returnclass, returnitem);

    return rc;
  }
        
    return 0;  /*  Just in case  */
}


static int
XML_item_new (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread)
{
    RESULT_NODE *name    = argc > 0 ? argv [0] : NULL;


  {
    int
        rc;
    CLASS_DESCRIPTOR
        *returnclass;
    void
        *returnitem;

    rc = XML_item_create
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


static int
XML_item_delete_function (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread)
{


  {
    int
        rc;

    rc = XML_item_delete
             (item);

    return rc;
  }
        
    return 0;  /*  Just in case  */
}


static int
XML_value_name (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread)
{


  {
    char
        *item_name;

    item_name = mem_strdup ((char *) XML_value_item_name
                                         (item));

    if (item_name)
        assign_string (& result-> value, item_name);

    return 0;
  }
        
    return 0;  /*  Just in case  */
}


static int
XML_value_next (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread)
{
    RESULT_NODE *name    = argc > 0 ? argv [0] : NULL;


  {
    int
        rc;
    CLASS_DESCRIPTOR
        *returnclass;
    void
        *returnitem;

    rc = XML_value_next_sibling
             (item,
              name ? string_value (& name-> value) : NULL,
              ((GGCODE_TCB *) gsl_thread-> tcb)-> gsl-> ignorecase,
              & returnclass,
              & returnitem);

    if ((! rc)
    &&  item)
        assign_pointer (& result-> value, returnclass, returnitem);

    return rc;
}
        
    return 0;  /*  Just in case  */
}


static int
XML_value_parent_function (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread)
{


  {
    int
        rc;
    CLASS_DESCRIPTOR
        *returnclass;
    void
        *returnitem;

    rc = XML_value_parent
             (item,
              & returnclass,
              & returnitem,
              gsl_thread);

    if ((! rc)
    &&  item)
        assign_pointer (& result-> value, returnclass, returnitem);

    return rc;
  }
        
    return 0;  /*  Just in case  */
}


static int
XML_value_delete_function (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread)
{


  {
    int
        rc;

    rc = XML_value_delete
             (item);

    return rc;
  }
        
    return 0;  /*  Just in case  */
}

int shutdown_XML_classes (void)
{

    xml_free (ancestor);


    return 0;
}

static int XML_class_init (CLASS_DESCRIPTOR **class, void **item, THREAD *gsl_thread)
{
     *class = & XML_class;


  {
    XML_CONTEXT
        *context;

    context = memt_alloc (NULL, sizeof (XML_CONTEXT));
    context-> links     = 0;
    context-> error_msg = NULL;

    *item = context;
  }
    
    return 0;
}

int register_XML_classes (void)
{
    int
        rc = 0;

    ancestor = xml_create ("ancestor", NULL);

    rc |= object_register (XML_class_init,
                           NULL);
    return rc;
}
