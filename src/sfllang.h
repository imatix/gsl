/*===========================================================================*
 *                                                                           *
 *  sfllang.h - Multilingual date/time/number representation                 *
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
    Synopsis:   Provides hard-coded multilanguage dictionaries for dates and
                numbers,  The hard-coded dictionaries work with most European
                languages.
 ------------------------------------------------------------------</Prolog>-*/

#ifndef SFLLANG_INCLUDED               /*  Allow multiple inclusions        */
#define SFLLANG_INCLUDED


/*  Function prototypes                                                      */

#ifdef __cplusplus
extern "C" {
#endif

int   set_userlang        (int language);
int   set_userlang_str    (const char *language);
int   get_userlang        (void);
char *get_userlang_str    (void);
int   set_accents         (Bool accents);
Bool  get_accents         (void);
char *get_units_name      (int units);
char *get_tens_name       (int tens);
char *get_day_name        (int day);
char *get_day_abbrev      (int day, Bool upper);
char *get_month_name      (int month);
char *get_month_abbrev    (int month, Bool upper);
char *timestamp_string    (char *buffer, const char *pattern);
char *certify_the_number  (char *buffer, int buffer_size, long number,
                           char *language, int code_page);

#ifdef __cplusplus
}
#endif


/*  Constant definitions                                                     */

enum {
    USERLANG_DEFAULT = 0,               /*  Default language                 */
    USERLANG_DA,                        /*  Danish                           */
    USERLANG_DE,                        /*  German                           */
    USERLANG_EN,                        /*  English                          */
    USERLANG_ES,                        /*  Castillian Spanish               */
    USERLANG_FB,                        /*  Belgian or Swiss French          */
    USERLANG_FR,                        /*  French                           */
    USERLANG_IS,                        /*  Icelandic                        */
    USERLANG_IT,                        /*  Italian                          */
    USERLANG_NL,                        /*  Dutch                            */
    USERLANG_NO,                        /*  Norwegian                        */
    USERLANG_PO,                        /*  Portuguese                       */
    USERLANG_SV                         /*  Swedish                          */
};

#define USERLANG_TOP     USERLANG_SV + 1

#endif
