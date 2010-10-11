/*===========================================================================*
 *                                                                           *
 *  sflxml.c - XML navigation functions                                      *
 *                                                                           *
 *  Copyright (c) 1991-2010 iMatix Corporation                               *
 *                                                                           *
 *  ------------------ GPL Licensed Source Code ------------------           *
 *  iMatix makes this software available under the GNU General               *
 *  Public License (GPL) license for open source projects.  For              *
 *  details of the GPL license please see www.gnu.org or read the            *
 *  file license.gpl provided in this package.                               *
 *                                                                           *
 *  This program is free software; you can redistribute it and/or            *
 *  modify it under the terms of the GNU General Public License as           *
 *  published by the Free Software Foundation; either version 2 of           *
 *  the License, or (at your option) any later version.                      *
 *                                                                           *
 *  This program is distributed in the hope that it will be useful,          *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of           *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            *
 *  GNU General Public License for more details.                             *
 *                                                                           *
 *  You should have received a copy of the GNU General Public                *
 *  License along with this program in the file 'license.gpl'; if            *
 *  not, write to the Free Software Foundation, Inc., 59 Temple              *
 *  Place - Suite 330, Boston, MA 02111-1307, USA.                           *
 *                                                                           *
 *  You can also license this software under iMatix's General Terms          *
 *  of Business (GTB) for commercial projects.  If you have not              *
 *  explicitly licensed this software under the iMatix GTB you may           *
 *  only use it under the terms of the GNU General Public License.           *
 *                                                                           *
 *  For more information, send an email to info@imatix.com.                  *
 *  --------------------------------------------------------------           *
 *===========================================================================*/

#include "prelude.h"                    /*  Universal header file            */
#include "sflstr.h"                     /*  String functions                 */
#include "sfllist.h"                    /*  List access functions            */
#include "sflmem.h"                     /*  Memory access functions          */
#include "sflfile.h"                    /*  File access functions            */
#include "sfldate.h"                    /*  Date and time functions          */
#include "sflsymb.h"                    /*  Symbol table access functions    */
#include "sflxml.h"                     /*  Prototypes for functions         */
#include "sflxmll.h"                    /*  Loading & saving functions       */


/*  Implementation-dependent type definitions                                */

struct _XML_ITEM {                      /*  Item node definition             */
    struct _XML_ITEM
        *next,                          /*  Next item in list                */
        *prev,                          /*  Previous item in list            */
        *parent;                        /*  Parent if this is a child        */
    char
        *name,                          /*  Item name, allocated string      */
        *value;                         /*  Value node, allocated string     */
    LIST
        attrs,                          /*  List of attributes, 0 or more    */
        children;                       /*  List of children, 0 or more      */
    void
        *data;                          /*  Any other data to attach         */
};

struct _XML_ATTR {                      /*  Attribute node definition        */
    struct _XML_ATTR
        *next,                          /*  Next attr in list                */
        *prev;                          /*  Previous attr in list            */
    struct _XML_ITEM
        *parent;                        /*  Parent item if this attribute    */
    char
        *name,                          /*  Attribute name                   */
        *value;                         /*  Attribute value, may be null     */
};


/*  ---------------------------------------------------------------------[<]-
    Function: xml_create

    Synopsis: Creates and initialises a new XML_ITEM item.  Returns
    the address of the created XML_ITEM item or NULL if there was not
    enough memory.  Sets the new item's name and value as specified; only
    one of these should contain a value, although sflxml will not complain
    if both do.  If the name is non-NULL this is a child node; if the
    value is non-NULL then this is a value node.
    [ph:2004-0213 - it bloody well should complain if both are set]
    ---------------------------------------------------------------------[>]-*/

XML_ITEM *
_xml_create (
    const char *name,
    const char *value,
    Bool        duplicate)
{
    XML_ITEM
        *item;

    list_create (item, sizeof (XML_ITEM));
    if (item) {
        list_reset (&item-> attrs);
        list_reset (&item-> children);
        item-> parent = NULL;
        if (duplicate) {
            item-> name   = mem_strdup (name);
            item-> value  = mem_strdup (value);
        }
        else {
            item-> name   = (char*)name;
            item-> value  = (char*)value;
        }
        item-> data = NULL;
        return (item);
    }
    else {
        if (!duplicate) {
            mem_free ((char*)name);
            mem_free ((char*)value);
        }
        return (NULL);
    }
}


/*  ---------------------------------------------------------------------[<]-
    Function: xml_new

    Synopsis: Creates and initialises a new XML_ITEM item with a specified
    parent item.  Returns the address of the created XML_ITEM item or NULL
    if there was not enough memory.  Sets the new item's name and value as
    specified; only one of these should contain a value, although sflxml
    will not complain if both do.  If the name is non-NULL this is a child
    node; if the value is non-NULL then this is a value node.  If the
    parent argument is non-NULL, attaches the new item to the end of the
    parent item list.
    ---------------------------------------------------------------------[>]-*/

XML_ITEM *
_xml_new (
    XML_ITEM   *parent,
    const char *name,
    const char *value,
    Bool        duplicate)
{
    XML_ITEM
        *item;

    if (duplicate)
        item = xml_create (name, value);
    else
        item = xml_create_no_dup (name, value);

    if (item && parent)
        xml_attach_child (parent, item);

    return item;
}


/*  ---------------------------------------------------------------------[<]-
    Function: xml_modify_value

    Synopsis: Modifies an existing XML item's value.
    ---------------------------------------------------------------------[>]-*/

void
xml_modify_value  (XML_ITEM *item, const char *value)
{
    ASSERT (item);

    if (!item-> value)
        item-> value = mem_strdup (value);
    else
        if (! value || (strneq (value, item-> value)))
          {
            mem_free (item-> value);
            item-> value = mem_strdup (value);
          }
}


/*  ---------------------------------------------------------------------[<]-
    Function: xml_rename

    Synopsis: Modifies an existing XML item's name.
    ---------------------------------------------------------------------[>]-*/


void
xml_rename (XML_ITEM *item, const char *name)
{
    ASSERT (item);

    mem_free (item-> name);
    item-> name = mem_strdup (name);
}


/*  ---------------------------------------------------------------------[<]-
    Function: xml_item_name

    Synopsis: Extracts the name of a specified XML item.  The returned string
    should NOT be modified.  To manipulate it, first make a copy first.
    ---------------------------------------------------------------------[>]-*/

char *
xml_item_name (XML_ITEM *item)
{
    ASSERT (item);

    return item-> name;
}


/*  ---------------------------------------------------------------------[<]-
    Function: xml_item_value

    Synopsis: Extracts the value from a value node.  These are recognised
    by their name being NULL.  The returned string should NOT be modified.
    To manipulate it, first make a copy first.
    ---------------------------------------------------------------------[>]-*/

char *
xml_item_value (XML_ITEM *item)
{
    ASSERT (item);

    return item-> value;
}


/*  ---------------------------------------------------------------------[<]-
    Function: xml_item_child_value

    Synopsis: Extracts all child value.  These are recognised
    by their name being NULL.  The returned string is allocated by mem_alloc,
    must be free by mem_free.
    ---------------------------------------------------------------------[>]-*/

char *
xml_item_child_value (XML_ITEM *item)
{
    XML_ITEM
        *child;
    long
        length;
    char
        *buffer = NULL,
        *target,
        *source;

    length = 0;
    FORVALUES (child, item)
      {
        if (xml_item_value (child))
            length += strlen (xml_item_value (child));
       }

    if (length > 0)
      {
        buffer = mem_alloc (length + 1);
        if (buffer != NULL)
          {
            target = buffer;
            FORVALUES (child, item)
              {
                source = xml_item_value (child);
                if (source)
                  {
                    while (*source)
                       *target++ = *source++;
                  }
              }
            *target  = '\0';
          }
      }

    return (buffer);
}

/*  ---------------------------------------------------------------------[<]-
    Function: xml_get_data

    Synopsis: Returns the (void *) data field.
    ---------------------------------------------------------------------[>]-*/

void *
xml_get_data (
    XML_ITEM *item)
{
    return item-> data;
}

/*  ---------------------------------------------------------------------[<]-
    Function: xml_set_data

    Synopsis: Sets the (void *) data field.
    ---------------------------------------------------------------------[>]-*/

void
xml_set_data (
    XML_ITEM *item,
    void *data)
{
    item-> data = data;
}

/*  ---------------------------------------------------------------------[<]-
    Function: xml_free

    Synopsis: Frees all memory used by an XML_ITEM item and its children.
    ---------------------------------------------------------------------[>]-*/

void
xml_free (
    XML_ITEM *item)
{
    if (item) {
        /*  Free attribute nodes for the item                                */
        while (!list_empty (&item-> attrs))
            xml_free_attr (item-> attrs.next);

        /*  Free child nodes for the item                                    */
        while (!list_empty (&item-> children))
            xml_free (item-> children.next);

        /*  Now free this item itself                                        */
        list_unlink (item);             /*  Unlink from its parent list      */
        mem_free (item-> name);         /*  Free strings, if not null        */
        mem_free (item-> value);
        mem_free (item);                /*  And free the item itself         */
    }
}


/*  ---------------------------------------------------------------------[<]-
    Function: xml_attach_child

    Synopsis: Attaches an XML item as the last child of a given parent.
    If the item is already attached to a parent, it is first removed.
    ---------------------------------------------------------------------[>]-*/

void
xml_attach_child (
    XML_ITEM *parent,
    XML_ITEM *item)
{
    if (item-> parent)
        xml_detach (item);

    item-> parent = parent;
    if (parent)
        list_relink_before (item, &parent-> children);
}


/*  ---------------------------------------------------------------------[<]-
    Function: xml_attach_sibling

    Synopsis: Attaches an XML item as the sibling preceeding a given item.
    If the item is already attached to a parent, it is first removed.
    ---------------------------------------------------------------------[>]-*/

void
xml_attach_sibling (
    XML_ITEM *sibling,
    XML_ITEM *item)
{
    if (item-> parent)
        xml_detach (item);

    item-> parent = xml_parent (sibling);
    list_relink_before (item, sibling);
}


/*  ---------------------------------------------------------------------[<]-
    Function: xml_detach

    Synopsis: Removes an XML item from the tree.
    ---------------------------------------------------------------------[>]-*/

void
xml_detach (
    XML_ITEM *item)
{
    item-> parent = NULL;
    list_unlink (item);
}


/*  ---------------------------------------------------------------------[<]-
    Function: xml_copy

    Synopsis: Recursively makes a copy of all the attributes and children
              of node 'from', attaching them to node 'to'.
    ---------------------------------------------------------------------[>]-*/

int
xml_copy (XML_ITEM *to, XML_ITEM *from)
{
    XML_ATTR
        *attr;
    XML_ITEM
        *to_child,
        *from_child;

    FORATTRIBUTES (attr, from)
        xml_put_attr (to,
                      xml_attr_name  (attr),
                      xml_attr_value (attr));

    for (from_child  = xml_first_child (from);
         from_child != NULL;
         from_child  = xml_next_sibling (from_child))
      {
        to_child = xml_new (to,
                            xml_item_name  (from_child),
                            xml_item_value (from_child));
        if (! to_child)
            return -1;

        xml_copy (to_child, from_child);
      }
    return 0;
}




/*  ---------------------------------------------------------------------[<]-
    Function: xml_first_child

    Synopsis: Returns the first child node of the specified item, or NULL
    if there are none.
    ---------------------------------------------------------------------[>]-*/

XML_ITEM *
xml_first_child (XML_ITEM *item)
{
    if (item && !list_empty (&item-> children))
        return (item-> children.next);
    else
        return (NULL);
}


/*  ---------------------------------------------------------------------[<]-
    Function: xml_last_child

    Synopsis: Returns the last child node of the specified item, or NULL
    if there are none.
    ---------------------------------------------------------------------[>]-*/

XML_ITEM *
xml_last_child (XML_ITEM *item)
{
    if (item && !list_empty (&item-> children))
        return (item-> children.prev);
    else
        return (NULL);
}


/*  ---------------------------------------------------------------------[<]-
    Function: xml_next_sibling

    Synopsis: Returns the next sibling of the specified item, or NULL if there
    if are none.
    ---------------------------------------------------------------------[>]-*/

XML_ITEM *
xml_next_sibling (XML_ITEM *item)
{
    if (item && (LIST *) item-> next != & item-> parent-> children)
        return (item-> next);
    else
        return (NULL);
}


/*  ---------------------------------------------------------------------[<]-
    Function: xml_prev_sibling

    Synopsis: Returns the previous sibling of the specified item, or NULL if
    there if are none.
    ---------------------------------------------------------------------[>]-*/

XML_ITEM *
xml_prev_sibling (XML_ITEM *item)
{
    if (item && (LIST *) item-> prev != & item-> parent-> children)
        return (item-> prev);
    else
        return (NULL);
}


/*  ---------------------------------------------------------------------[<]-
    Function: xml_parent

    Synopsis: Returns the parent of the specified item, or NULL if this is
    the root item.
    ---------------------------------------------------------------------[>]-*/

XML_ITEM *
xml_parent (XML_ITEM *item)
{
    if (item)
        return (item-> parent);
    else
        return (NULL);
}

/*  ---------------------------------------------------------------------[<]-
    Function: xml_xml_all

    Synopsis: Traverses the XML tree, executing the specified function for
    every item in the tree.  Continues so long as the function returns
    TRUE; halts when every item has been processed, or when the function
    returns FALSE.  Returns the number of items processed, or negative the
    number of items processed if processing stopped early due to the
    function returning FALSE or other errors.  The items are processed from
    the leaves back towards the root; it is thus permissible for the
    function to delete the item.
    ---------------------------------------------------------------------[>]-*/

int
xml_exec_all (
    XML_ITEM *item,                     /*  XML tree to process              */
    xmlfunc handler                     /*  Function to call                 */
)
{
    XML_ITEM
        *child,                         /*  Pointer to item                  */
        *next = NULL;                   /*    and to next in list            */
    int
        rc,
        count = 0;                      /*  Number of symbols processed ok   */

    ASSERT (item);

    child = xml_first_child (item);
    while (child && count >= 0)
      {
        next = xml_next_sibling (child);   /*  In case function deletes item */
        rc = xml_exec_all (child, handler);
        if (rc > 0)
            count += rc;
        else
            count = -count - rc;

        child = next;
      }
    if (count >= 0)
      {
        if ((*handler) (item))
            count += 1;
        else
            count = -count - 1;
      }
    return count;
}

/*  ---------------------------------------------------------------------[<]-
    Function: xml_find_item

    Synopsis: Finds a specific item in the XML Tree.  Path is an
    XPath-like specification where items are separated by slashes. 
    Returns the item found or NULL if no item matching path was found.

    Example:
    Given the following XML:
    <root>
      <one>
        <two/>
        <three></three>
      </one>
    </root>
    the following code 
    xml_item = xml_find_item (xml_root, "one/three");
    will return an xml_item pointing to <three>.
    ---------------------------------------------------------------------[>]-*/

XML_ITEM *xml_find_item (XML_ITEM *xml_root, const char *p_path)
{
    XML_ITEM
        *xml_item,                      /*  Current position in XML Tree     */
        *xml_child,                     /*  Candidate child                  */
        *xml_found;                     /*  XML item found                   */
    char
        *path,                          /*  Path to item we are searching for*/
        *cur_path,                      /*  Current position in path         */
        *next_path;                     /*  Candidate path                   */

    path = mem_strdup (p_path);
    cur_path = path;
    xml_item = xml_root;
    xml_child = NULL;
    xml_found = NULL;

    /*  Traverse the XML Tree, starting at xml_root passed to us, keeping 
     *  track of our current position in the path passed to us and in the 
     *  XML Tree.  Stop when we have run out of either. */
    while (cur_path) {
        next_path = strchr (cur_path, '/');
        if (next_path)
            *next_path++ = '\0';

        FORCHILDREN (xml_child, xml_item) {
            if (strcmp (xml_item_name (xml_child), cur_path) == 0) {
                xml_item = xml_child;
                if (next_path == NULL)
                    xml_found = xml_child;
            }
        }

        cur_path = next_path;
    }

    mem_strfree (&path);
    return (xml_found);
}


/*  ---------------------------------------------------------------------[<]-
    Function: xml_put_attr_ic

    Synopsis: Sets, modifies, or deletes an attribute for the
    specified item.  The attribute name must be supplied.  If the value is
    NULL,  the first attribute with the specified name is deleted.  Otherwise
    it is either created or modified accordingly.  If the parameter
    'ignore_case' is TRUE, the case of the attribute name is insignificant.
    Returns the number  of attribute nodes created (-1, 0, or 1).
    ---------------------------------------------------------------------[>]-*/

int
xml_put_attr_ic (
    XML_ITEM   *item,
    const char *name,
    const char *value,
    Bool        ignore_case,
    Bool        duplicate)
{
    int
        feedback = 0;
    XML_ATTR
        *attr;

    ASSERT (item);
    ASSERT (name);

    attr = xml_attr_ic (item, name, ignore_case);
    if (attr)
        if (value)                      /*  Value specified - update attr    */
          {
            mem_free (attr-> value);
            if (duplicate) {
                attr-> value = mem_strdup (value);
            }
            else {
                attr-> value = (char*) value;
                mem_free ((char*) name);
            }
          }
        else
          {
            xml_free_attr (attr);       /*  No value - delete attribute      */
            feedback = -1;
            if (!duplicate ) {
                mem_free ((char*) name);
                mem_free ((char*) value);
            }
          }
    else
        if (value)                      /*  Value specified - update attr    */
          {
            list_create (attr, sizeof (XML_ATTR));
            if (attr)
              {
                if (duplicate) {
                    attr-> name   = mem_strdup (name);
                    attr-> value  = mem_strdup (value);
                }
                else {
                    attr-> name   = (char*) name;
                    attr-> value  = (char*) value;
                }
                attr-> parent = item;
                list_relink_before (attr, &item-> attrs);
                feedback = 1;
              }
            else if (!duplicate)
              {
                mem_free ((char*) name);
                mem_free ((char*) value);
              }
          }
        else if (!duplicate)
          {
            mem_free ((char*) name);
            mem_free ((char*) value);
          }
    return (feedback);
}


/*  ---------------------------------------------------------------------[<]-
    Function: xml_attr_ic

    Synopsis: Searches for the attribute with the specified name; if
    found, returns the address of the attribute node, otherwise  returns
    NULL.  If the paramater 'ignore_case' is TRUE, the case of the
    attribute name is insignificant.
    ---------------------------------------------------------------------[>]-*/

XML_ATTR *
xml_attr_ic (
    XML_ITEM   *item,
    const char *name,
    Bool        ignore_case)
{
    XML_ATTR
        *attr;

    ASSERT (item);
    ASSERT (name);

    if (ignore_case)
      {
        FORLIST (attr, item-> attrs)
            if (attr-> name ? lexcmp (attr-> name, name) == 0 : FALSE)
                return (attr);
      }
    else
      {
        FORLIST (attr, item-> attrs)
            if (attr-> name ? streq (attr-> name, name) : FALSE)
                return (attr);
      }
    return (NULL);
}


/*  ---------------------------------------------------------------------[<]-
    Function: xml_attr_name

    Synopsis: Extracts the name of a specified XML attr.  The returned string
    should NOT be modified.  To manipulate it, first make a copy first.
    ---------------------------------------------------------------------[>]-*/

char *
xml_attr_name (XML_ATTR *attr)
{
    ASSERT (attr);

    return (attr-> name);
}


/*  ---------------------------------------------------------------------[<]-
    Function: xml_attr_value

    Synopsis: Extracts the value of a specified XML attr.  The returned string
    should NOT be modified.  To manipulate it, first make a copy first.
    ---------------------------------------------------------------------[>]-*/

char *
xml_attr_value (XML_ATTR *attr)
{
    ASSERT (attr);

    return (attr-> value);
}


/*  ---------------------------------------------------------------------[<]-
    Function: xml_get_attr_ic

    Synopsis: Returns the value for the specified attribute, if it exists.
    Otherwise returns the default value.  If the paramater 'ignore_case'
    is TRUE, the case of the attribute name is insignificant.
    ---------------------------------------------------------------------[>]-*/

char *
xml_get_attr_ic (
    XML_ITEM   *item,
    const char *name,
    const char *deflt,
    Bool        ignore_case)
{
    XML_ATTR
        *attr;

    ASSERT (item);
    ASSERT (name);

    attr = xml_attr_ic (item, name, ignore_case);
    if (attr)
        return (attr-> value);
    else
        return ((char *) deflt);
}


/*  ---------------------------------------------------------------------[<]-
    Function: xml_free_attr

    Synopsis: Frees all memory used by an XML_ATTR node.
    ---------------------------------------------------------------------[>]-*/

void
xml_free_attr (
    XML_ATTR *attr)
{
    ASSERT (attr);

    list_unlink (attr);
    mem_free (attr-> name);
    mem_free (attr-> value);
    mem_free (attr);
}


/*  ---------------------------------------------------------------------[<]-
    Function: xml_first_attr

    Synopsis: Returns the first attribute of a specified XML item, or NULL
    if there are none.
    ---------------------------------------------------------------------[>]-*/

XML_ATTR *
xml_first_attr (XML_ITEM *item)
{
    if (item && !list_empty (&item-> attrs))
        return (item-> attrs.next);
    else
        return (NULL);
}


/*  ---------------------------------------------------------------------[<]-
    Function: xml_last_attr

    Synopsis: Returns the last attribute of a specified XML item, or NULL
    if there are none.
    ---------------------------------------------------------------------[>]-*/

XML_ATTR *
xml_last_attr (XML_ITEM *item)
{
    if (item && !list_empty (&item-> attrs))
        return (item-> attrs.prev);
    else
        return (NULL);
}


/*  ---------------------------------------------------------------------[<]-
    Function: xml_next_attr

    Synopsis: Returns the next attribute following the specified attribute,
    or NULL if there are none.
    ---------------------------------------------------------------------[>]-*/

XML_ATTR *
xml_next_attr (XML_ATTR *attr)
{
    if (attr && (LIST *) attr-> next != & attr-> parent-> attrs)
        return (attr-> next);
    else
        return (NULL);
}


/*  ---------------------------------------------------------------------[<]-
    Function: xml_prev_attr

    Synopsis: Returns the previous attribute following the specified
    attribute, or NULL if there are none.
    ---------------------------------------------------------------------[>]-*/

XML_ATTR *
xml_prev_attr (XML_ATTR *attr)
{
    if (attr && (LIST *) attr-> prev != & attr-> parent-> attrs)
        return (attr-> prev);
    else
        return (NULL);
}


/*  ---------------------------------------------------------------------[<]-
    Function: xml_changed

    Synopsis: Returns TRUE if the XML file loaded into the specified list
    has in the meantime been changed.  Returns FALSE if not.
    ---------------------------------------------------------------------[>]-*/

Bool
xml_changed (
    XML_ITEM *item)
{
    char
        *filename;

    ASSERT (item);

    /*  Date, time, and name of original XML file are in the list            */
    filename = xml_get_attr (item, "filename", NULL);
    if (filename
    &&  file_has_changed (filename,
                          atol (xml_get_attr (item, "filedate", "0")),
                          atol (xml_get_attr (item, "filetime", "0"))))
        return (TRUE);
    else
        return (FALSE);
}


/*  ---------------------------------------------------------------------[<]-
    Function: xml_refresh

    Synopsis: Refreshes an XML tree created by xml_load ().  If the original
    file (as specified by the 'filename' attribute of the root item) has
    been modified, reloads the whole XML file.  Returns TRUE if the XML file
    was actually reloaded, or FALSE if the file had not changed or could not
    be accessed, or if the XML tree was incorrectly created.
    ---------------------------------------------------------------------[>]-*/

Bool
xml_refresh (
    XML_ITEM **item)
{
    char
        *filename,
        *pathsym;
    int
        rc;

    ASSERT (item);
    ASSERT (*item);
    if (xml_changed (*item))
      {
        pathsym  = mem_strdup (xml_get_attr (*item, "pathsym",  NULL));
        filename = mem_strdup (xml_get_attr (*item, "filename", NULL));
        xml_free (*item);               /*  Delete previous XML tree         */
        rc = xml_load (item, pathsym, filename);
        mem_free (pathsym);
        mem_free (filename);
        return (rc == XML_NOERROR);
      }
    return (FALSE);
}



/* Functions for generated XML parser.                                       */
/* These functions should not be used by module other than sflxmll.c         */
/* 'xml_' prefix has been discarded to distinguish them from the 'regular'   */
/* interface                                                                 */


XML_ITEM *alloc_xml_item (void)
{
    XML_ITEM
            *item;

    list_create (item, sizeof (XML_ITEM));
    if (item)
      {
        list_reset (&item-> attrs);
        list_reset (&item-> children);
        item-> parent = NULL;
        item-> name   = NULL;
        item-> value  = NULL;
        item-> data   = NULL;
      }

    return (item);
}

Bool set_xml_item_name (XML_ITEM *item, char *name)
{
    ASSERT (item);
    ASSERT (!item-> name);
    item-> name = name;

    return TRUE;
}

Bool set_xml_item_value (XML_ITEM *item, char *value)
{
    XML_ITEM
        *child;
    char
        *cur;
    ASSERT (item);

    for (cur=value; *cur; cur++)
      {
        if ( !strchr (" \t\r\n", *cur) )
          break;
      }

    if (!*cur)                          /* value is only made of blanks */
      {
        mem_free (value);
        return TRUE;
      }

    child = alloc_xml_item ();
    ASSERT (child);
    child-> value = value;
    xml_attach_child (item, child);

    return TRUE;
}


Bool link_xml_child (XML_ITEM *parent, XML_ITEM *item)
{
    if (item-> parent)
        xml_detach (item);

    item-> parent = parent;
    if (parent)
        list_relink_before (item, &parent-> children);

    return TRUE;
}


Bool check_xml_closing_name (XML_ITEM *item, char *name)
{
    Bool
        res;

    ASSERT (item);
    ASSERT (name);

    res = strcmp (xml_item_name(item), name) == 0 ? TRUE : FALSE;
    mem_free (name);

    return res;
}

 /* ------------------------------------------------------------------------- */


XML_ATTR *alloc_xml_attr (void)
{
    XML_ATTR
            *attr;

    list_create (attr, sizeof (XML_ATTR));
    if (attr)
      {
        attr-> parent = NULL;
        attr-> name   = NULL;
        attr-> value  = NULL;
      }

    return (attr);
}

Bool set_xml_attr_name (XML_ATTR *attr, char *name)
{
    ASSERT (attr);
    ASSERT (!attr-> name);
    attr-> name = name;

    return TRUE;
}

Bool set_xml_attr_value (XML_ATTR *attr, char *value)
{
    ASSERT (attr);
    ASSERT (!attr-> value);

    attr-> value = value;
    return TRUE;
}

Bool link_xml_attr (XML_ITEM *item, XML_ATTR *attr)
{
    ASSERT (item);
    ASSERT (attr);

    if (xml_get_attr(item, attr-> name, NULL) != NULL)
        return FALSE;

    attr-> parent = item;
    list_relink_before (attr, &item-> attrs);
    return TRUE;
}


