/*===========================================================================*
 *                                                                           *
 *  sflxmls.c - XML Store functions                                          *
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
 *  published by the Free Software Foundation; either version 3 of           *
 *  the License, or (at your option) any later version.                      *
 *                                                                           *
 *  This program is distributed in the hope that it will be useful,          *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of           *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            *
 *  GNU General Public License for more details.                             *
 *                                                                           *
 *  You should have received a copy of the GNU General Public                *
 *  License along with this program in the file 'license.gpl'; if            *
 *  not, see <http://www.gnu.org/licenses/>.                                 *
 *                                                                           *
 *  You can also license this software under iMatix's General Terms          *
 *  of Business (GTB) for commercial projects.  If you have not              *
 *  explicitly licensed this software under the iMatix GTB you may           *
 *  only use it under the terms of the GNU General Public License.           *
 *                                                                           *
 *  For more information, send an email to info@imatix.com.                  *
 *  --------------------------------------------------------------           *
 *===========================================================================*/
/*
    Synopsis:   Provides functions to Manage XML data like dictionary.

    The path parameter use the partial syntax of XPath standard (www.w3c.org)

    To have a specific item you can use the syntax:

    /root_item/item/sub_item

    If the first character is '/', you use a absolute position from the root
    item. If you use without the first slash, you go from the relative position
    (position from the last call)

    you can also go to a item like a table access
    Ex:
        /root_item/item [2]/sub_item

    The first iteration of item have index 1
        /root_item/item [1]/sub_item  <=> /root_item/item/sub_item

    To have value of a attribute, use the character '@' for the first
    characters of attributes name

    Ex:
       /root_item/item/sub_item/@attribute_1

    When you count items, you can use '/root_item/item/sub_item' to
    count the number of 'sub_item' child of 'item' or you can
    use '*' at end of path to have the count of all child items
    of 'item'.

*/
#include "prelude.h"                    /*  Universal include file           */
#include "sfldate.h"                    /*  Date and time functions          */
#include "sflfile.h"                    /*  File access functions            */
#include "sfllist.h"                    /*  List access functions            */
#include "sflmem.h"                     /*  Memory allocation functions      */
#include "sflstr.h"                     /*  String access functions          */
#include "sflxml.h"                     /*  XML definitions                  */
#include "sflsymb.h"
#include "sflhttp.h"                    /*  Meta-char encoding/decoding      */
#include "sflxmll.h"                    /*  XML Load/save functions          */
#include "sflxmls.h"                    /*  Include prototype data           */

/*  Definitions                                                              */

struct _XML_STORE {
    XML_ITEM *root;                     /*  Root item                        */
    XML_ITEM *item;                     /*  Current item                     */
    XML_ATTR *attr;                     /*  Current attribute                */
    char      path  [512];              /*  Current path for item            */
    char      error [512];              /*  Error buffer                     */
    char     *value;                    /*  Current value                    */
    Bool      is_attr;                  /*  TRUE if path for attribute       */
    Bool      all_item;                 /*  TRUE if path for all items       */
    SYMTAB   *cache;                    /*  Cache of path to better access   */
    int       index_tree_level;         /*  Index level in xml tree          */
    Bool      need_reindex;             /*  TRUE when xml data changed       */
};

#define ADD2CACHE(key, pdata)                                                  \
    symbol = sym_assume_symbol (store-> cache, strlwc ((key)), NULL);         \
    if (symbol)                                                               \
        symbol-> data = (void *)(pdata);

/* Global variable                                                           */

static char
    error_buffer [512];

/*  Function prototypes                                                      */
static Bool go_to_path        (XML_STORE *store, char *path,
                               Bool create_missing);
static Bool set_path_position (XML_STORE *store, char *path,
                               Bool create_missing);
static void index_xml_child   (XML_STORE *store, XML_ITEM *item,
                               char *parent_path, int tree_level);
static Bool search_by_index   (XML_STORE *store);

/*  ---------------------------------------------------------------------[<]-
    Function: xmls_new

    Synopsis: Create a new xml_strore. Initialise with xml data if not null.
    ---------------------------------------------------------------------[>]-*/

XML_STORE *
xmls_new (char *xml_buffer)
{
    XML_STORE
        *feedback = NULL;

    feedback = mem_alloc (sizeof (XML_STORE));
    if (feedback)
      {
        memset (feedback, 0, sizeof (XML_STORE));
        if (xml_buffer && *xml_buffer)
          {
            if (xml_load_string (&feedback-> root, xml_buffer, FALSE) 
                != XML_NOERROR)
              {
                strcpy (error_buffer, xml_error ());
                if (feedback-> root)
                    xml_free (feedback-> root);
                mem_free (feedback);
                feedback = NULL;
              }
            else
              {
                strcpy (feedback-> path, "/");
                feedback-> item = xml_first_child (feedback-> root);
              }
          }
      }
    return (feedback);
}


/*  ---------------------------------------------------------------------[<]-
    Function: xmls_load

    Synopsis: Load a xml file a store to a xml_store.
    ---------------------------------------------------------------------[>]-*/

XML_STORE *
xmls_load (char *file_name)
{
    XML_STORE
        *feedback = NULL;

    feedback = mem_alloc (sizeof (XML_STORE));
    if (feedback)
      {
        memset (feedback, 0, sizeof (XML_STORE));
        if (file_name && *file_name)
          {
            if (xml_load (&feedback-> root, ".", file_name) 
                != XML_NOERROR)
              {
                strcpy (error_buffer, xml_error ());
                if (feedback-> root)
                    xml_free (feedback-> root);
                mem_free (feedback);
                feedback = NULL;
              }
            else
              {
                strcpy (feedback-> path, "/");
                feedback-> item = xml_first_child (feedback-> root);
              }
          }
      }
    return (feedback);
}


/*  ---------------------------------------------------------------------[<]-
    Function: xmls_save 

    Synopsis: Save xml data to a file.
    ---------------------------------------------------------------------[>]-*/

int 
xmls_save (XML_STORE *store, char *file_name)
{
    int
        feedback = 0;

    if (store
    &&  store-> root)
        feedback = xml_save_file (xml_first_child (store-> root), file_name);

    return (feedback);
}


/*  ---------------------------------------------------------------------[<]-
    Function: xmls_save_sting

    Synopsis: Store xml data to a string. This string must be free by mem_free.
    ---------------------------------------------------------------------[>]-*/

char *
xmls_save_string (XML_STORE *store)
{
    char
        *feedback = NULL;

    if (store
    &&  store-> root)
        feedback = xml_save_string (xml_first_child (store-> root));

    return (feedback);
}

/*  ---------------------------------------------------------------------[<]-
    Function: xmls_free 

    Synopsis: Free all allocated data.
    ---------------------------------------------------------------------[>]-*/

Bool
xmls_free (XML_STORE *store)
{
    Bool
        feedback = TRUE;

    if (store)
      {
         if (store-> root)
             xml_free (store-> root);

         if (store-> value)
             mem_free (store-> value);

         if (store-> cache)
             sym_delete_table (store-> cache);

         memset (store, 0, sizeof (XML_STORE));
         mem_free (store);
      }
    return (feedback);
}


/*  ---------------------------------------------------------------------[<]-
    Function: xmls_get_value 

    Synopsis: Get a xml value stored at position specified by the path.
              If path is wrong or have no value, return the default value.
    ---------------------------------------------------------------------[>]-*/

char *
xmls_get_value (XML_STORE *store, char *path, char *default_value)
{
    char
        *value    = NULL,
        *feedback = NULL;
    Bool
        have_position;

    if (store == NULL)
        return (default_value);

    have_position = go_to_path (store, path, FALSE);
    if (have_position)
      {
        if (store-> value)
          {
            mem_free (store-> value);
            store-> value = NULL;
          }
        if (store-> attr)
          {
            value = xml_attr_value (store-> attr);
            if (value == NULL && default_value)
                value = default_value;
            if (value)
              {
                store-> value = mem_strdup (value);
                feedback = store-> value;
              }
          }
        else
        if (store-> item)
          {
            value = xml_item_child_value (store-> item);
            if (value)
                store-> value = value;
            else
            if (default_value)  
                store-> value = mem_strdup (default_value);
            feedback = store-> value;
          }
      }
    else
        feedback = default_value;

    return (feedback);
}


/*  ---------------------------------------------------------------------[<]-
    Function: xmls_set_value  

    Synopsis: Save the value at the position specified by the path.
              Return TRUE if no problem.
    ---------------------------------------------------------------------[>]-*/

Bool
xmls_set_value  (XML_STORE *store, char *path, char *value)
{
    Bool
        have_position,
        feedback = FALSE;
    XML_ITEM
        *next,
        *child = NULL;

    if (store == NULL)
        return (feedback);

    have_position = go_to_path (store, path, FALSE);
    if (have_position)
      {
        if (store-> value)
          {
            mem_free (store-> value);
            store-> value = NULL;
          }

        if (store-> attr)
          {
            feedback = TRUE;
            xml_put_attr (store-> item, store-> path, value);
          }
        else
        if (store-> item)
          {
            child  = xml_first_child (store-> item);
            while (child)
              {
                next = xml_next_sibling (child);
                if (!xml_item_name (child))
                    xml_free (child);
                child = next;
              }

            if (value)
                xml_new (store-> item, NULL, value);
            feedback = TRUE;
          }
      }

    return (feedback);
}


/*  ---------------------------------------------------------------------[<]-
    Function: xmls_count_item 

    Synopsis: Count number of item specified by last value in path.
              If you need count all items, use '*' characters.
    ---------------------------------------------------------------------[>]-*/

long
xmls_count_item (XML_STORE *store, char *path)
{
    char
        *name = NULL;
    long
        feedback = 0;
    Bool
        have_position;
    XML_ITEM
        *parent,
        *child;

    if (store == NULL)
        return (0);

    have_position = go_to_path (store, path, FALSE);
    if (have_position)
      {
        if (store-> item)
          {
            if (store-> all_item == FALSE)
              {
                name = xml_item_name (store-> item);
                parent = xml_parent (store-> item);
              }
            else
                parent = store-> item;
            FORCHILDREN (child, parent)
              {
                if (name)
                  {
                    if (lexcmp (name, xml_item_name (child)) == 0)
                        feedback++;
                  }
                else
                    feedback++;
              }
          }
      }
    return (feedback);
}


/*  ---------------------------------------------------------------------[<]-
    Function: xmls_add 

    Synopsis: Add item and value in xml tree. All missing items in path
              are created.
    ---------------------------------------------------------------------[>]-*/

Bool
xmls_add (XML_STORE *store, char *path, char *value)
{
    Bool
        have_position,
        feedback = FALSE;
    XML_ITEM
        *child;

    have_position = go_to_path (store, path, TRUE);
    if (have_position)
      {
        if (store-> item)
            store-> item = xml_parent (store-> item);
      }

    if (store-> is_attr)
      {
        if (store-> item)
            xml_put_attr (store-> item, store-> path, value);       
      }
    else
      {
        child = xml_new (store-> item, store-> path, NULL);
        if (child && value)
            xml_new (child, NULL, value);
      }
    if (store-> cache)
        store-> need_reindex = TRUE;

    return (feedback);
}


/*  ---------------------------------------------------------------------[<]-
    Function: xmls_delete 

    Synopsis: Delete item specified by the path.
    ---------------------------------------------------------------------[>]-*/

Bool
xmls_delete (XML_STORE *store, char *path)
{
    Bool
        have_position,
        feedback = FALSE;

    have_position = go_to_path (store, path, FALSE);
    if (have_position)
      {
        if (store-> attr)
          {
            xml_free_attr (store-> attr);
            store-> attr = NULL;
            feedback = TRUE;
          }
        else
        if (store-> item)
          {
            xml_free (store-> item);
            store-> item = NULL;
            feedback = TRUE;

            /* Delete from cache if required                                 */
            if (store-> cache)
                store-> need_reindex = TRUE;
          }
      }
    return (feedback);
}

/*  ---------------------------------------------------------------------[<]-
    Function: xml_index 

    Synopsis: Index all xml tree to give better access with path value.
    ---------------------------------------------------------------------[>]-*/

void 
xmls_index (XML_STORE *store, int tree_level)
{
    XML_ITEM
        *item;
    char
        path [256];
    SYMBOL
        *symbol;

    if (store == NULL || store-> root == NULL)
        return;

    if (store-> cache)
        sym_delete_table (store-> cache);

    store-> cache            = sym_create_table ();
    store-> index_tree_level = tree_level;
    store-> need_reindex     = FALSE;

    item = xml_first_child (store-> root);
    if (item)    
      {
        sprintf (path, "/%s", xml_item_name (item));
        ADD2CACHE (path, item);
        index_xml_child (store, item, path, tree_level - 1);
      }
}

/*---------------------------------------------------------------------------*/
/*------------------- L O C A L  F U N C T I O N S --------------------------*/
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* This function get item of attribute for the specified path.               */
/*                                                                           */
/* Initialise All parameters, get the first item and call the recursive      */
/* function 'set_path_position'                                              */

static Bool
go_to_path (XML_STORE *store, char *path, Bool create_missing)
{
    char
        *begin_path;
    Bool
        feedback = FALSE;
    if (path == NULL)
        return (TRUE);


    strcpy (store-> path, path);
    store-> all_item = FALSE;
    store-> attr     = NULL;
    store-> is_attr  = FALSE;

    feedback = search_by_index (store);
    if (feedback == FALSE)
      {
        if (*store-> path == '/')
          {
            store-> item = store-> root;
            begin_path = &store-> path [1];        
          }
        else
            begin_path = store-> path;

        feedback = set_path_position (store, begin_path, create_missing);
      }
    return (feedback);
}


/*---------------------------------------------------------------------------*/
/* Recursive function to select specified data by the path.                  */

static Bool
set_path_position (XML_STORE *store, char *path, Bool create_missing)
{
    char
        *item_index,
        *end;
    XML_ITEM
        *item;
    Bool
        feedback = FALSE;
    int
        cur_position = 1,
        req_position = 1;
     
    if (store == NULL 
    ||  path  == NULL 
    || *path  == 0)
        return  (FALSE);


    end = strchr (path, '/');
    if (end)
        *end = '\0';

    /* If attribute                                                          */
    if (*path == '@')
      {
        store-> is_attr = TRUE;
        end             = NULL;
        path++;
        if (store-> item)
          {
            FORATTRIBUTES (store-> attr, store-> item)
              {
                if (lexcmp (xml_attr_name (store-> attr), path) == 0)
                  {
                    feedback = TRUE;
                    break;
                  }
              }
            if (feedback == FALSE)
                store-> attr = NULL;

            strcpy (store-> path, path);
          }
      }
    /* Else item                                                             */
    else
    if (*path == '*')
      {
        store-> all_item = TRUE;
        feedback = TRUE;
      }
    else
      {
        item_index = strchr (path, '[');
        if (item_index)
          {
            *item_index++ = '\0';
            req_position = atoi (item_index);
            strcrop (path);
          }

        FORCHILDREN (item, store-> item)
          {
            if (lexcmp (xml_item_name (item), path) == 0)
              {
                if (cur_position == req_position)
                  {
                    feedback = TRUE;
                    store-> item = item;
                    break;
                  }
                else
                    cur_position++;
              }
          }
        
        if (feedback == TRUE)           /* If we found item                  */
          {
            if (end)                    /* If not the end of the path        */
              {
                end++;
                feedback = set_path_position (store, end, create_missing);
              }
            else
                strcpy (store-> path, path);
          }
        else
        if (end == '\0')
            strcpy (store-> path, path);
        else
        if (create_missing)            /* Create missing item if required    */
          {
            store-> item = xml_new (store-> item, path, NULL);
            end++;
            feedback = set_path_position (store, end, create_missing);
          }
      }    
    return (feedback);
}


/*---------------------------------------------------------------------------*/
/* Recursive function to index xml tree                                      */

static void
index_xml_child (XML_STORE *store, XML_ITEM *item, char *parent_path,
                 int tree_level)
{
    XML_ITEM
        *child;
    char
        item_name [256],
        path      [1024];
    SYMTAB
        *index_cache;
    SYMBOL
        *symbol;
    int
        *index;

    if (tree_level == 0)
        return;

    index_cache = sym_create_table ();

    if (index_cache == NULL)
        return;
    FORCHILDREN (child, item)
       {
         strcpy (item_name, xml_item_name (child));
         symbol = sym_lookup_symbol (index_cache, item_name);
         if (symbol)
           {           
             index = (int *)(symbol-> data);
             (*index)++;
           }
         else
           {
             index = mem_alloc (sizeof (int));
             if (index)
               {
                 *index = 1;
                 symbol = sym_assume_symbol (index_cache, item_name, NULL);
                 if (symbol)
                     symbol-> data = (void *)index;
               }
           }
         if (*index == 1)
           {
             sprintf (path,"%s/%s", parent_path, item_name);
             ADD2CACHE (path, child);
             index_xml_child (store, child, path, tree_level - 1);
           }
         sprintf (path,"%s/%s [%d]", parent_path, item_name, *index);
         ADD2CACHE (path, child);
         index_xml_child (store, child, path, tree_level - 1);
       }

    for (symbol = index_cache-> symbols; symbol; symbol = symbol-> next)
      {
        if (symbol-> data)
            mem_free (symbol-> data);
      }

    sym_delete_table (index_cache);
}

/*---------------------------------------------------------------------------*/
/* Analyse path value and try to find item in cache                          */

static Bool
search_by_index (XML_STORE *store)
{
    char
        *p,
        path [1024];
    Bool
        is_attr,
        is_all,
        is_complete,
        feedback;
    int
        level;
    SYMBOL
        *symbol;

    if (store-> cache == NULL)
        return (FALSE);

    if (store-> need_reindex)
        xmls_index (store, store-> index_tree_level);

    is_attr     = FALSE;
    is_all      = FALSE;
    is_complete = FALSE;
    feedback    = FALSE;
    level       = 0;

    strcpy (path, store-> path);
    p = path;
    while (*p)
     {
       if ((*p) == '/')
         {
           level++;
           if (*(p + 1))
             {
               if (*(p + 1) == '@')
                   is_attr = TRUE;
               if (*(p + 1) == '*')
                   is_all = TRUE;              
              }
           if (level > store-> index_tree_level || is_attr || is_all)
             {
               *p++ = '\0';
               break;
             }
         }
       p++;
     } 

    if (level  <= store-> index_tree_level
    &&  is_all  == FALSE
    &&  is_attr == FALSE)
        is_complete = TRUE;

    symbol = sym_lookup_symbol (store-> cache, path);
    if (symbol)
      {
        store-> item = (XML_ITEM *)symbol-> data;
        if (is_complete)
            feedback = TRUE;
        else
        if (p && *p)
            strcpy (store-> path, p);
            
      }
    return (feedback);
}
