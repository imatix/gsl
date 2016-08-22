/*===========================================================================*
 *                                                                           *
 *  sflcvds.c - Convert a date to a string (hard coded format)               *
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


/*  ---------------------------------------------------------------------[<]-
    Function: conv_date_str

    Synopsis: Converts a date to a string.  The format argument defines how
    the date is shown:
    <TABLE>
        DATE_YMD_COMPACT        ddmmyy
        DATE_YMD_DELIM          dd/mm/yy
        DATE_YMD_SPACE          dd mm yy
        DATE_YMD_COMMA          dd mm, yy  (DM,Y or MD,Y or Y,MD)
        DATE_YM_COMPACT         mmyy
        DATE_YM_SLASH           mm/yy
        DATE_YM_SPACE           mm yy
        DATE_MD_COMPACT         ddmm
        DATE_MD_SLASH           dd/mm
        DATE_MD_SPACE           dd mm
    </TABLE>

    The date order (year/month/day) is normally supplied in the order
    argument.  However, the date flags can override this.  The flags are:
    <TABLE>
        FLAG_D_DD_AS_D          Show day without leading zero
        FLAG_D_MM_AS_M          Show month without leading zero
        FLAG_D_MONTH_ABC        Show month as letters (fullname if width > 16)
        FLAG_D_CENTURY          Show year as four digits
        FLAG_D_UPPERCASE        Month name in uppercase
        FLAG_D_ORDER_DMY        Order is DMY for this date
        FLAG_D_ORDER_MDY        Order is MDY for this date
        FLAG_D_ORDER_YMD        Order is YMD for this date
    </TABLE>

    Returns a pointer to a static area holding the string, or NULL if there
    was an error (for instance, formatted date greater than width).
    ---------------------------------------------------------------------[>]-*/

char *
conv_date_str (
    long date,
    int  flags,
    int  format,
    int  order,
    char datesep,
    int  width)
{
    static char *format_table [] = {
        "ymd",    "dmy",    "mdy",      /*  DATE_YMD_COMPACT                 */
        "y/m/d",  "d/m/y",  "m/d/y",    /*  DATE_YMD_DELIM                   */
        "y m d",  "d m y",  "m d y",    /*  DATE_YMD_SPACE                   */
        "y, m d", "d m, y", "m d, y",   /*  DATE_YMD_COMMA                   */
        "ym",     "my",     "my",       /*  DATE_YM_COMPACT                  */
        "y/m",    "m/y",    "m/y",      /*  DATE_YM_DELIM                    */
        "y m",    "m y",    "m y",      /*  DATE_YM_SPACE                    */
        "md",     "dm",     "md",       /*  DATE_MD_COMPACT                  */
        "m/d",    "d/m",    "m/d",      /*  DATE_MD_DELIM                    */
        "m d",    "d m",    "m d"       /*  DATE_MD_SPACE                    */
    };
    char
        *format_ptr,                    /*  Scan through format string       */
        delim [2],                      /*  Delimiter character              */
        picture [14],                   /*  Largest picture: dd mmmm, yyyy   */
        ch;                             /*  Next char in format string       */
    int
        index,
        date_order = order;             /*  Order to use                     */

    ASSERT (format >= DATE_FORMAT_FIRST && format <= DATE_FORMAT_LAST);
    ASSERT (order  >= DATE_ORDER_FIRST  && order  <= DATE_ORDER_LAST);

    conv_reason = 0;                    /*  No conversion errors so far      */

    if (flags & FLAG_D_ORDER_YMD)
        date_order = DATE_ORDER_YMD;
    else
    if (flags & FLAG_D_ORDER_DMY)
        date_order = DATE_ORDER_DMY;
    else
    if (flags & FLAG_D_ORDER_MDY)
        date_order = DATE_ORDER_MDY;

    /*  Get index into table                                                 */
    index = format * 3 + date_order - 1;

    /*  Now build-up picture according to format string                      */
    strclr (picture);
    for (format_ptr = format_table [index]; *format_ptr; format_ptr++)
      {
        ch = *format_ptr;
        switch (ch)
          {
            case 'y':
                strcat (picture, flags & FLAG_D_CENTURY? "yyyy": "yy");
                break;

            case 'm':
                if (flags & FLAG_D_MONTH_ABC)
                    if (width > 16)
                        strcat (picture, flags & FLAG_D_UPPER? "MMMM": "mmmm");
                    else
                        strcat (picture, flags & FLAG_D_UPPER? "MMM": "mmm");
                else
                    strcat (picture, flags & FLAG_D_MM_AS_M? "m": "mm");
                break;

            case 'd':
                strcat (picture, flags & FLAG_D_DD_AS_D? "d": "dd");
                break;

            case '/':
                ch = datesep;           /*  Use supplied date separator      */
            default:
                delim [0] = ch;
                delim [1] = 0;
                strcat (picture, delim);
          }
      }
    format_ptr = conv_date_pict (date, picture);
    if (strlen (format_ptr) > (unsigned) width)
      {
        conv_reason = CONV_ERR_DATE_OVERFLOW;
        return (NULL);
      }
    else
        return (format_ptr);
}
