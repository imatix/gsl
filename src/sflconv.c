/*===========================================================================*
 *                                                                           *
 *  sflconv.c - Global variables for conversion functions                    *
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

#include "prelude.h"                    /*  Universal header file            */
#include "sflconv.h"                    /*  Prototypes for functions         */

/*  The conv_reason details the last error returned by a conv_ function.     */
/*  Constants for the various possible errors are defined in ifconv.h.       */

int conv_reason = 0;

/*  The conv_reason_text array provides an error message text for each       */
/*  value of conv_reason.                                                    */

char *conv_reason_text [] =
  {
    "No errors",
    "Unrecognised char in input",
    "Value out of valid range",
    "Not a yes/no or true/false value",
    "More than one 'am' or 'pm'",
    "Result too large for output",
    "Too few or too many digits",
    "Too many delimiters",
    "Unknown month name",
    "3/5 digits in a row not allowed",
    "More than one month name",
    "Not enough decimals supplied",
    "Result too large for output",
    "More than one sign character",
    "Sign not allowed if unsigned",
    "Malformed financial negative",
    "More than one decimal point",
    "Decimals not allowed if integer",
    "Decimals not allowed if hidden",
    "Too many decimal positions",
    "Too many digits for number"
  };
