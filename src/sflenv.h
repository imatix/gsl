/*===========================================================================*
 *                                                                           *
 *  sflenv.h - Environment manipulation                                      *
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
    Synopsis:   Provides functions to read environment variables (also called
                shell variables or logical variables.)  Provides translation
                into numeric and Boolean values.  Provides functions to work
                with the environment block.
 ------------------------------------------------------------------</Prolog>-*/

#ifndef _SFLENV_INCLUDED                /*  Allow multiple inclusions        */
#define _SFLENV_INCLUDED


/*  Function prototypes                                                      */

#ifdef __cplusplus
extern "C" {
#endif

char    *env_get_string  (const char *name, const char *default_value);
long     env_get_number  (const char *name, long default_value);
Bool     env_get_boolean (const char *name, Bool default_value);
DESCR   *env2descr       (void);
char   **descr2env       (const DESCR *descr);
SYMTAB  *env2symb        (void);
char   **symb2env        (const SYMTAB *symtab);
char   **env_copy        (char **environment);
int      env_set         (const char *name, const char *value, int overwrite);
void     env_clear       (const char *name);

#ifdef __cplusplus
}
#endif

#endif
