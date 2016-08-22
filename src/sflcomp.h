/*===========================================================================*
 *                                                                           *
 *  sflcomp.h - Compression functions                                        *
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
    Synopsis:   Various compression/decompression functions.  The LZ-type
                algorith (LZRW1/KH) was originally written by Kurt Haenen
                <ghgaea8@blekul11> and made portable by P. Hintjens. This
                is a reasonable LZ/RLE algorithm, very fast, but about 30%
                less efficient than a ZIP-type algorithm in terms of space.
                The RLE algorithms are better suited to compressing sparse
                data.  The nulls variant is specifically tuned to data that
                consists mostly of binary zeroes.  The bits variant is
                tuned for compressing sparse bitmaps.
 ------------------------------------------------------------------</Prolog>-*/

#ifndef SFLCOMP_INCLUDED               /*  Allow multiple inclusions        */
#define SFLCOMP_INCLUDED

/*  Function prototypes                                                      */

#ifdef __cplusplus
extern "C" {
#endif

word compress_block (const byte *source, byte *dest, word source_size);
word expand_block   (const byte *source, byte *dest, word source_size);
word compress_rle   (      byte *source, byte *dest, word source_size);
word expand_rle     (const byte *source, byte *dest, word source_size);
word compress_nulls (      byte *source, byte *dest, word source_size);
word expand_nulls   (const byte *source, byte *dest, word source_size);
word compress_bits  (      byte *source, byte *dest, word source_size);
word expand_bits    (const byte *source, byte *dest, word source_size);

#ifdef __cplusplus
}
#endif

#endif
