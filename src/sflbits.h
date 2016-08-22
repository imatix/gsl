/*===========================================================================*
 *                                                                           *
 *  sflbits.h - Large bitmap manipulation functions                          *
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
    Synopsis:   Provides operations to manipulate large bitstrings.  The
                bitstrings are compressed.  Intended for bit-based index
                techniques, where bitstrings can be millions of bits long.
                These functions are still in development; this is an early
                version that provides basic functionality.  Simple tests
                on large bitmaps with random filling show a cost of about
                3 bytes per bit, after compression.  This includes all the
                indexing information.
 ------------------------------------------------------------------</Prolog>-*/

#ifndef SFLBITS_INCLUDED               /*  Allow multiple inclusions        */
#define SFLBITS_INCLUDED

/*  Definitions                                                              */

#define BIT_DATASIZE    500             /*  Size of block data part          */
#define BIT_INDEXSIZE   BIT_DATASIZE/2  /*  Size of block index part         */
#define BIT_SECTSIZE    8192            /*  Size of one bitstring section    */
#define BIT_MAXBLOCKS   1024            /*  Max. size of bitstring           */
#define BIT_MAXBITS     16384000L       /*  Max. possible bit number         */

typedef struct {                        /*  Bitstring block                  */
    union {
        byte  data  [BIT_DATASIZE];     /*    Data record part               */
        dbyte index [BIT_INDEXSIZE];    /*    Index record part              */
    } block;
    dbyte right;                        /*    Pointer to right (data too)    */
    int   size;                         /*    Size of data part              */
} BITBLOCK;

typedef struct {                        /*  Bitstring object                 */
    BITBLOCK
       *block [BIT_MAXBLOCKS];          /*  Table of allocated blocks        */
    int
        block_count;                    /*  How many allocated blocks        */
    dbyte
        free_list;                      /*  Block free list                  */
} BITS;

#ifdef __cplusplus
extern "C" {
#endif

extern long bits_free_count;            /*  We count free() and malloc()     */
extern long bits_alloc_count;

/*  Function prototypes                                                      */

BITS *bits_create     (void);
void  bits_destroy    (BITS *bits);

int   bits_set        (BITS *bits, long bit);
int   bits_clear      (BITS *bits, long bit);

int   bits_fput       (FILE *file, const BITS *bits);
BITS *bits_fget       (FILE *file);
char *bits_save       (BITS *bits);
BITS *bits_load       (const char *buffer);

int   bits_test       (const BITS *bits, long bit);
long  bits_search_set (const BITS *bits, long bit, Bool reverse);
long  bits_set_count  (const BITS *bits);

BITS *bits_and        (const BITS *bits1, const BITS *bits2);
BITS *bits_or         (const BITS *bits1, const BITS *bits2);
BITS *bits_xor        (const BITS *bits1, const BITS *bits2);
BITS *bits_invert     (const BITS *bits);

#ifdef __cplusplus
}
#endif

#endif
