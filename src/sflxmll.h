/*===========================================================================*
 *                                                                           *
 *  sflxmll.h - XML loading and saving functions                             *
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

/*  ----------------------------------------------------------------<Prolog>-
    Synopsis:   Provides functions to load and save XML files.  An XML file
                is held in memory as a tree of nodes, of type XML_ITEM.  The
                XML functions do not currently accept DTDs in the XML data.
 ------------------------------------------------------------------</Prolog>-*/

#ifndef SLFXMLL_INCLUDED               /*  Allow multiple inclusions        */
#define SLFXMLL_INCLUDED


/*- Function prototypes ---------------------------------------------------- */

#ifdef __cplusplus
extern "C" {
#endif

/*  Error values                                                             */

#define XML_NOERROR         0           /*  No errors                        */
#define XML_FILEERROR       1           /*  Error in file i/o                */
#define XML_LOADERROR       2           /*  Error loading XML                */


/*  Function prototypes                                                      */

int     xml_seems_to_be (const char *path, const char *filename);
char   *xml_error       (void);

int     xml_load_file   (XML_ITEM **item, const char *path, const char *filename, Bool extend);
int     xml_save_file   (XML_ITEM  *item, const char *filename);

int     xml_load_string (XML_ITEM **item, const char *xmlstring, Bool extend);
char   *xml_save_string (XML_ITEM  *item);

int     xml_load_descr  (XML_ITEM **item, const DESCR *descr, Bool extend);
DESCR  *xml_save_descr  (XML_ITEM  *item);

SYMTAB *xml_load_symtab      (SYMTAB *load_symtab, XML_ITEM *xml_root, const char *path);
SYMTAB *xml_load_symtab_file (SYMTAB *load_symtab, const char *filename, const char *path);


/*  Macros                                                                   */

#define xml_load_extended(item, path, filename)  xml_load_file (item, path, filename, TRUE)
#define xml_load(item, path, filename)           xml_load_file (item, path, filename, FALSE)


#ifdef __cplusplus
}
#endif

#endif                                  /*  Included                         */
