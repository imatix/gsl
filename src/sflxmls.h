/*===========================================================================*
 *                                                                           *
 *  sflxmls.h - XML Store functions                                          *
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
    Synopsis:   Provides functions to Manage XML data.

*/
#ifndef SLFXMLS_INCLUDED               /*  Allow multiple inclusions        */
#define SLFXMLS_INCLUDED

/*- Definition ------------------------------------------------------------- */

typedef struct _XML_STORE XML_STORE;

/*- Function prototypes ---------------------------------------------------- */

#ifdef __cplusplus
extern "C" {
#endif


/*  Function prototypes  */

XML_STORE *xmls_new         (char *xml_buffer);
XML_STORE *xmls_load        (char *file_name);
int        xmls_save        (XML_STORE *store, char *file_name);
char      *xmls_save_string (XML_STORE *store);
Bool       xmls_free        (XML_STORE *store);
void       xmls_index       (XML_STORE *store, int tree_level);
char      *xmls_get_value   (XML_STORE *store, char *path, char *default_value);
Bool       xmls_set_value   (XML_STORE *store, char *path, char *value);
long       xmls_count_item  (XML_STORE *store, char *path);
Bool       xmls_add         (XML_STORE *store, char *path, char *value);
Bool       xmls_delete      (XML_STORE *store, char *path);

#ifdef __cplusplus
}
#endif

#endif                                  /*  Included                         */
