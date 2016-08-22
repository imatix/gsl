/*===========================================================================*
 *                                                                           *
 *  sflstr.h - String manipulation functions                                 *
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
    Synopsis:   Provides various string-handling functions.  Some of these
                functions are available on some but not all platforms; others
                are useful tools for string handling.
 ------------------------------------------------------------------</Prolog>-*/

#ifndef SFLSTR_INCLUDED                /*  Allow multiple inclusions        */
#define SFLSTR_INCLUDED

/*  Macro to prepare call to xstrcpy                                         */
#if (defined (DEBUG))
#  define xstrcpy_debug() { xstrcpy_file = __FILE__; xstrcpy_line = __LINE__;}
#else
#  define xstrcpy_debug()
#endif

extern char *xstrcpy_file;
extern long xstrcpy_line;

/*  Structure defines simple unsorted lookup table                           */
typedef struct {
    char *key;
    int   value;
} LOOKUP;

/*  Function prototypes                                                      */

#ifdef __cplusplus
extern "C" {
#endif

char   *strdupl            (const char *string);
char   *strndupl           (const char *string, const size_t length);
char  **strfree            (char **string);
char   *strskp             (const char *string);
char   *strcset            (char *string, char ch);
char   *strpad             (char *string, char ch, int length);
char   *strlwc             (char *string);
char   *strupc             (char *string);
char   *strcrop            (char *string);
char   *stropen            (char *string, Bool align);
char   *strclose           (char *string, Bool align);
char   *strunique          (char *string, char unique);
int     strmatch           (const char *string1, const char *string2);
qbyte   strhash            (const char *string);
Bool    strprefixed        (const char *string, const char *prefix);
char   *strprefix          (const char *string, const char *delims);
char   *strdefix           (const char *string, const char *prefix);
char   *strconvch          (char *string, char from, char to);
char   *strconvchs         (char *string, char from, char *to);
char   *xstrcat            (char *dest, const char *src, ...);
char   *xstrcpy            (char *dest, const char *src, ...);
int     lexcmp             (const char *string1, const char *string2);
int     lexncmp            (const char *string1, const char *string2,
                            const int count);
int     lexwcmp            (const char *string1, const char *string2);
char   *soundex            (const char *string);
char   *soundexn           (const char *string, int size, Bool fold);
DESCR  *strt2descr         (char **strings);
char  **descr2strt         (const DESCR *descr);
void    strtfree           (char **strings);
int     strcntch           (const char *string, char value);
int     strlookup          (const LOOKUP *lookup, const char *key);
char   *strjustify         (const char *source, size_t width);
char   *strprefixlines     (const char *source, const char *prefix);
char   *strreformat        (const char *source, size_t width, const char *prefix);
size_t  xstrlcat           (char *dst, const char *src, size_t siz);
size_t  xstrlcpy           (char *dst, const char *src, size_t siz);

char   *replacechrswith    (char *str, char *chrs, char ctorlcwth);
char   *insertstring       (char *str, char *chrstoins, int pos);
char   *insertchar         (char *str, char chrtoins, int pos);
char   *leftfill           (char *str, char chrtofill, unsigned len);
char   *rightfill          (char *str, char chrtofill, unsigned len);
char   *searchreplace      (char *str, char *strtofnd, char *strtoins, size_t max_length);
char   *deletestring       (char *str, char *strtodel, int ignorecase);
char   *getstrfld          (char *str, int fln, int ofset,
                            char *sep, char *ret);
char   *setstrfld          (char *str, int fln, int ofset,
                            char *sep, char *ins);
int     getstrfldlen       (char *str, int fln, int ofset, char *sep);
int     matchtable         (char *str, char *strmatch,
                            char *strsept, int ncse);
char   *stringreplace      (char *strbuf, char *strpattern, size_t max_length);
char   *wordwrapstr        (char *strbuff, int iwid);
char   *stricstr           (const char *str1, const char *str2);
int     strtempcmp         (const char *str1, const char *strPat);
Bool    match_pattern      (char *string, char *pattern, Bool ignore_case);
Bool    is_text            (const byte characters);

#ifdef __cplusplus
}
#endif

/*  Macros & defines                                                         */
#define IGNORECASE 0
#define SENSECASE  1

#define lexeq(s1,s2)      (!lexcmp ((s1), (s2)))
#define lexneq(s1,s2)     (lexcmp ((s1), (s2)))

#endif
