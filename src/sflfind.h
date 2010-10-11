/*===========================================================================*
 *                                                                           *
 *  sflfind.h - String and data searching functions                          *
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
    Synopsis:   Searches for a pattern within a string or block of memory
                using a variant of the Boyer-Moore algorithm (improved by
                Horspool and Sunday). As fast or faster than the normal
                Boyer-Moore algorithm for most search strings, and much
                simpler.  Includes a basic function for searching blocks of
                memory with known sizes, plus an envelope that searches
                null-delimited strings.  Provides the option of repeatedly
                searching for the same pattern without re-parsing the pattern
                each time.  Original algorithm published by BOYER, R., and S.
                MOORE 1977, "A Fast String Searching Algorithm." CACM, 20,
                762-72.  Simplifications by HORSPOOL, R. N. 1980, "Practical
                Fast Searching in Strings." Software - Practice and Experience,
                10, 501-06.  More improvements by HUME, A., and D. M. SUNDAY
                1991, "Fast String Searching." AT&T Bell Labs Computing Science
                Technical Report No. 156.  Implemented in C by P. Hintjens.

                strfind_r() and memfind_r(), are reentrant versions of
                strfind() and memfind() for single searches, and
                strfind_rb() and memfind_rb() are reentrant versions of
                strfind() and memfind() supporting repeat searches against
                the same pattern.
 ------------------------------------------------------------------</Prolog>-*/

#ifndef SFLFIND_INCLUDED               /*  Allow multiple inclusions        */
#define SFLFIND_INCLUDED

/*  Function prototypes                                                      */

#ifdef __cplusplus
extern "C" {
#endif

char *strfind    (const char *string, const char *pattern, Bool repeat_find);
char *strfind_r  (const char *string, const char *pattern);
char *strfind_rb (const char *string, const char *pattern,
                  size_t *shift, Bool *repeat_find);

void *memfind    (const void *block,   size_t block_size,
                  const void *pattern, size_t pattern_size, Bool repeat_find);
void *memfind_r  (const void *block,   size_t block_size,
                  const void *pattern, size_t pattern_size);
void *memfind_rb (const void *block,   size_t block_size,
                  const void *pattern, size_t pattern_size,
                  size_t *shift, Bool *repeat_find);

char *txtfind    (const char *string, const char *pattern);

#ifdef __cplusplus
}
#endif

#endif
