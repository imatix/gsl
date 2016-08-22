/*===========================================================================*
 *                                                                           *
 *  sflxml.h - XML navigation functions                                      *
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
/*  ----------------------------------------------------------------<Prolog>-
    Synopsis:   Provides functions to read and write XML files, and manipulate
                XML data in memory as list structures.  XML is the Extensible
                Markup Language.  Accepts this XML syntax:
                <item [attr=["]value["]]...>value [child]</item>
 ------------------------------------------------------------------</Prolog>-*/

#ifndef SLFXML_INCLUDED                /*  Allow multiple inclusions        */
#define SLFXML_INCLUDED

/* -------------------------------------------------------------------------
    An XML tree is built as the following recursive structure:

                   .---------.    .----------.
                 .-:  Attr   :<-->:   0..n   :  Attributes are not sorted.
    .----------. : :  Head   :    :   attrs  :
    :   Item   :-' `---------'    `----------'
    :   node   :-. .---------.    .----------.
    `----------' : :  Child/ :<-->:   0..n   :  Each child node is the root
                 `-:  Value  :    : children :  of its own tree of nodes.
                   `---------'    `----------'
   ------------------------------------------------------------------------- */


/*- Structure definitions -------------------------------------------------- */

typedef struct _XML_ITEM XML_ITEM;
typedef struct _XML_ATTR XML_ATTR;


/*- Function prototypes ---------------------------------------------------- */

#ifdef __cplusplus
extern "C" {
#endif

/*  XML item functions */
XML_ITEM *_xml_new          (XML_ITEM *parent,
                            const char *name,
                            const char *value,
                            Bool duplicate);
#define xml_new(parent, name, value)            _xml_new(parent, name, value, TRUE)
#define xml_new_no_dup(parent, name, value)     _xml_new(parent, name, value, FALSE)
#define xml_new_child(parent, name)             _xml_new(parent, name, NULL,  TRUE)
#define xml_new_value(parent, value)            _xml_new(parent, NULL, value, TRUE)

XML_ITEM *_xml_create       (const char *name,
                            const char *value,
                            Bool duplicate);
#define xml_create(name, value)                 _xml_create(name, value, TRUE)
#define xml_create_no_dup(name, value)          _xml_create(name, value, FALSE)


void      xml_modify_value (XML_ITEM *item,
                            const char *value);
void      xml_rename       (XML_ITEM *item,
                            const char *name);
char     *xml_item_name    (XML_ITEM *item);
char     *xml_item_value   (XML_ITEM *item);
char     *xml_item_child_value (XML_ITEM *item);
void     *xml_get_data     (XML_ITEM *item);
void      xml_set_data     (XML_ITEM *item,
                            void *data);
void      xml_free         (XML_ITEM *item);

/*  XML tree manipulation  */
void      xml_attach_child   (XML_ITEM *parent,  XML_ITEM *item);
void      xml_attach_sibling (XML_ITEM *sibling, XML_ITEM *item);
void      xml_detach         (XML_ITEM *item);
int       xml_copy           (XML_ITEM *to, XML_ITEM *from);

/*  XML family navigation  */
XML_ITEM *xml_first_child  (XML_ITEM *item);
XML_ITEM *xml_last_child   (XML_ITEM *item);
XML_ITEM *xml_next_sibling (XML_ITEM *item);
XML_ITEM *xml_prev_sibling (XML_ITEM *item);
XML_ITEM *xml_parent       (XML_ITEM *item);

/*  XML tree traversal  */
typedef Bool (*xmlfunc) (XML_ITEM *);
int xml_exec_all (XML_ITEM *item, xmlfunc handler);

/*  XML path navigation    */
XML_ITEM *xml_find_item    (XML_ITEM *xml_root, 
                            const char *p_path);

/*  XML attribute functions  */
int       xml_put_attr_ic  (XML_ITEM   *item,
                            const char *name,
                            const char *value,
                            Bool        ignore_case,
                            Bool        duplicate);
XML_ATTR *xml_attr_ic      (XML_ITEM   *item,
                            const char *name,
                            Bool        ignore_case);
char     *xml_attr_name    (XML_ATTR   *item);
char     *xml_attr_value   (XML_ATTR   *item);
char     *xml_get_attr_ic  (XML_ITEM   *item,
                            const char *name,
                            const char *deflt,
                            Bool        ignore_case);
void      xml_free_attr    (XML_ATTR   *attr);

#define  xml_put_attr(item, name, value)                                      \
         xml_put_attr_ic (item, name, value, FALSE, TRUE)
#define  xml_put_attr_no_dup(item, name, value)                               \
         xml_put_attr_ic (item, name, value, FALSE, FALSE)

#define  xml_attr(item, name)                                                 \
         xml_attr_ic (item, name, FALSE)
#define  xml_get_attr(item, name, dflt)                                       \
         xml_get_attr_ic (item, name, dflt, FALSE)

/*  XML attribute navigation  */
XML_ATTR *xml_first_attr   (XML_ITEM *item);
XML_ATTR *xml_last_attr    (XML_ITEM *item);
XML_ATTR *xml_next_attr    (XML_ATTR *attr);
XML_ATTR *xml_prev_attr    (XML_ATTR *attr);

/*  XML housekeeping functions  */
Bool     xml_changed       (XML_ITEM *item);
Bool     xml_refresh       (XML_ITEM **item);

/*  Macros to treat all children and all attributes                          */

#define FORCHILDREN(child,item)    for (child  = xml_first_child (item);      \
                                        child != NULL;                        \
                                        child  = xml_next_sibling (child))    \
                                       if (xml_item_name (child))

#define FORVALUES(child,item)      for (child  = xml_first_child (item);      \
                                        child != NULL;                        \
                                        child  = xml_next_sibling (child))    \
                                       if (!xml_item_name (child))

#define FORATTRIBUTES(attr,item)   for (attr  = xml_first_attr (item);        \
                                        attr != NULL;                         \
                                        attr  = xml_next_attr (attr))



/* functions for generated XML parser.                                       */

XML_ITEM *alloc_xml_item          (void);
Bool      set_xml_item_name       (XML_ITEM *item, char *name);
Bool      set_xml_item_value      (XML_ITEM *item, char *value);
Bool      link_xml_child          (XML_ITEM *parent, XML_ITEM *item);
Bool      check_xml_closing_name  (XML_ITEM *item, char *name);

XML_ATTR *alloc_xml_attr      (void);
Bool      set_xml_attr_name   (XML_ATTR *attr, char *name);
Bool      set_xml_attr_value  (XML_ATTR *attr, char *value);
Bool      link_xml_attr       (XML_ITEM *item, XML_ATTR *attr);


#ifdef __cplusplus
}
#endif

#endif
