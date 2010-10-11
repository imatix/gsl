/*===========================================================================*
 *                                                                           *
 *  sfldescr.c - String descriptor manipulation functions                    *
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
 *===========================================================================*/

#include "prelude.h"                    /*  Universal header file            */
#include "sflcons.h"
#include "sflmem.h"
#include "sfldescr.h"                   /*  Prototypes for functions         */


/*  ---------------------------------------------------------------------[<]-
    Function: descr_str

    Synopsis: Return descriptor data as null-terminated string, limited
    to a maximum of 255 characters.
    ---------------------------------------------------------------------[>]-*/

char *
descr_str (const DESCR *descr)
{
    static char
        string [256];
    int
        size;

    if (descr) {
        size = min (descr-> size, 255);
        memcpy (string, descr-> data, size);
        string [size] = 0;
        return (string);
    }
    else
        return ("");
}


/*  ---------------------------------------------------------------------[<]-
    Function: descr_streq

    Synopsis: Compare descriptor data with specified string, return TRUE
    if the strings are identical.
    ---------------------------------------------------------------------[>]-*/

Bool
descr_streq (const DESCR *descr, const char *string)
{
    int
        string_size,
        descr_size;

    ASSERT (string);
    string_size = strlen (string);
    descr_size  = descr->size;

    /*  Descriptor may include string terminator, if so, ignore that         */
    if (descr_size > 0 && descr->data [descr->size - 1] == 0)
        descr_size--;

    if (descr_size == string_size
    &&  memcmp (descr->data, string, string_size) == 0)
        return (TRUE);
    else
        return (FALSE);
}

