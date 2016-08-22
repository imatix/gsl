/*===========================================================================*
 *                                                                           *
 *  sflini.h - Configuration file processing                                 *
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
    Synopsis:   Provides functions to read an initialisation file that follows
                the MS-Windows style, i.e. consists of [Sections] followed by
                keyword = value lines.
 ------------------------------------------------------------------</Prolog>-*/

#ifndef SLFINI_INCLUDED                /*  Allow multiple inclusions        */
#define SLFINI_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

Bool    ini_find_section  (FILE *inifile, char *section, Bool top);
Bool    ini_scan_section  (FILE *inifile, char **keyword, char **value);
SYMTAB *ini_dyn_load      (SYMTAB *symtab, const char *filename);
SYMTAB *ini_dyn_loade     (SYMTAB *symtab, const char *filename);
int     ini_dyn_save      (SYMTAB *symtab, const char *filename);
Bool    ini_dyn_changed   (SYMTAB *symtab);
Bool    ini_dyn_refresh   (SYMTAB *symtab);
char   *ini_dyn_value     (SYMTAB *symtab, const char *section,
                           const char *keyword, const char *default_value);
char  **ini_dyn_values    (SYMTAB *symtab, const char *section,
                           const char *keyword, const char *default_value);
char   *ini_dyn_assume    (SYMTAB *symtab, const char *section,
                           const char *keyword, const char *default_value);

#ifdef __cplusplus
}
#endif

#endif
