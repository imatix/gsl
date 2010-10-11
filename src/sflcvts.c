/*===========================================================================*
 *                                                                           *
 *  sflcvts.c - Convert a string into a time                                 *
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

#include "prelude.h"                    /*  Universal header file            */
#include "sflconv.h"                    /*  Prototypes for functions         */


/*  ---------------------------------------------------------------------[<]-
    Function: conv_time_str

    Synopsis: Converts a time to a string.  The flags and width control the
    resulting string.  You can use one or more of these flags added together:
    <TABLE>
        FLAG_T_HH_AS_H      Suppress leading zeroes on the hours.
        FLAG_T_MM_AS_M      Suppress leading zeroes on the minutes.
        FLAG_T_SS_AS_S      Suppress leading zeroes on the seconds.
        FLAG_T_CC_AS_C      Suppress leading zeroes on the centiseconds.
        FLAG_T_COMPACT      Show without delimiters.
        FLAG_T_12_HOUR      Append am/pm indicator.
    </TABLE>

    Returns a pointer to a static area holding the string, or NULL if there
    was an error.

    If no flags are used, the width argument gives these results (shown
    as a picture, which is how conv_time_str works - see conv_time_pict):
    <TABLE>
        4_or_less       Error.
        5_to_7          "hh:mm"
        8_to_10         "hh:mm:ss"
        11_or_more      "hh:mm:ss:cc"
    </TABLE>

    If FLAG_T_COMPACT is used, width gives these results:
    <TABLE>
        3_or_less       Error.
        4_to_5          "hhmm"
        6_to_7          "hhmmss"
        8_or_more       "hhmmsscc"
    </TABLE>

    If FLAG_T_12_HOUR is used, width gives these results:
    <TABLE>
        5_or_less       Error.
        6_to_8          "hh:mma"
        9_to_11         "hh:mm:ssa"
        12_or_more      "hh:mm:ss:cca"
    </TABLE>

    If FLAG_T_COMPACT and FLAG_T_12_HOUR are used, width gives these results:
    <TABLE>
        4_or_less       Error.
        5_to_6          "hhmma"
        7_to_8          "hhmmssa"
        9_or_more       "hhmmsscca"
    </TABLE>
    ---------------------------------------------------------------------[>]-*/

char *
conv_time_str (
    long time,
    int  flags,
    char timesep,
    int  width)
{
    char
        delim [2],                      /*  Delimiter string, ":" or ""      */
        picture [13];                   /*  Largest picture: hh:mm:ss:cca    */
    int
        delim_len;
    int
        space_left = width;

    conv_reason = 0;                    /*  No conversion errors so far      */
    if (flags & FLAG_T_COMPACT)
      {
        delim [0] = 0;
        delim_len = 0;
      }
    else
      {
        delim [0] = timesep;
        delim [1] = 0;
        delim_len = 1;
      }
    if (flags & FLAG_T_12_HOUR)
        space_left--;                   /*  Subtract 1 if eventual "a"       */

    /*  Build-up date picture components until we run out of space           */
    strcpy (picture, (flags & FLAG_T_HH_AS_H? "h": "hh"));
    space_left -= 2;

    if (space_left >= delim_len + 2)
      {
        strcat (picture, delim);
        strcat (picture, (flags & FLAG_T_MM_AS_M? "m": "mm"));
        space_left -= delim_len + 2;
      }
    else
        return (NULL);                  /*  Error - space_left is too small  */

    if (space_left >= delim_len + 2)
      {
        strcat (picture, delim);
        strcat (picture, (flags & FLAG_T_SS_AS_S? "s": "ss"));
        space_left -= delim_len + 2;
      }

    if (space_left >= delim_len + 2)
      {
        strcat (picture, delim);
        strcat (picture, (flags & FLAG_T_CC_AS_C? "c": "cc"));
        space_left -= delim_len + 2;
      }

    /*  Append "a" (or "aa" if space) if 12-hour clock wanted                */
    if (flags & FLAG_T_12_HOUR)
        strcat (picture, (space_left == 0? "a": "aa"));

    return (conv_time_pict (time, picture));
}
