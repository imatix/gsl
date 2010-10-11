/*===========================================================================*
 *                                                                           *
 *  sflsort.c - Sorting functions                                            *
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
#include "sfllist.h"                    /*  List declarations                */
#include "sflmem.h"                     /*  Memory management                */
#include "sflsymb.h"                    /*  Symbol table management          */
#include "sflxml.h"                     /*  XML manipulation                 */
#include "sflxmll.h"                    /*  XML parsing                      */
#include "sflstr.h"                     /*  String functions                 */
#include "sfltok.h"                     /*  Token splitting functions        */
#include "sflsort.h"                    /*  Prototypes for functions         */


#define KEY_MAX_SIZE        4096        /*  Maximum size of composed key     */

/*  Local function prototypes                                                */

static char *get_next_range   (char *format, int *range_from, int *range_to);
static void  add_field_to_key (char *key, int *key_length, char *value);



/*  ---------------------------------------------------------------------[<]-
    Function: comb_sort

    Synopsis: Sorts a list using the "comb-sort" algorithm.  Note that this
    function is deprecated and simply passes through to list_sort().
    ---------------------------------------------------------------------[>]-*/

void comb_sort (void *list, LIST_COMPARE comp)
{
    list_sort (list, (NODE_COMPARE) comp);
}


/*  ---------------------------------------------------------------------[<]-
    Function: build_sort_key

    Synopsis: Builds a sort key from some data.  This function provides a
    number of ways of building the sort key, using a 'format' that specifies
    which parts of the data to use.  There are various possibilities.

    Firstly, by specifying columns of data.  This is appropriate for data
    that follows a fixed column format.  To specify one or more columns,
    use this syntax: c:n[-m]{,n[-m]}...  For example: c:1-5,10-15.  The
    first column in the data is numbered 1 (not 0).

    Secondly, by specifying data fields separated by some delimiter.  This
    is appropriate for printable data in a simple delimited format.  To
    specify one or more fields, use this syntax: fx:n{,m}...  For example,
    f,:3,6.  Note that the delimiter character must be specified after the
    initial 'f'.  The first field in the data is numbered 1.

    Lastly, by specifying XML attributes.  This is appropriate when the
    table data is a valid XML item.  The attributes must be in the root
    item.  Use this syntax: x:name{,name}...  For example: x:city,country.

    This function allocates a block of memory and places the built sort key
    in this - the calling code must release this block using mem_free().
    If there was a syntax error in the format, the function returns NULL.
    If the function could not build a sort key successfully (e.g. the 'x'
    format type was used, but the data was not valid XML), it returns an
    empty string, "".

    Note that the data, the format, and the resulting key, are all ANSI C
    strings and may not contain binary zeroes.  The sort keys built for the
    'f' and 'x' format types are automagically formatted to sort correctly
    for variable-length field values, through the insertion of a low binary
    value (\1) after each field.  Thus a key built from two fields, "auto"
    and "mobile", will sort before a key built from one field, "automobile".
    
    If the data is NULL, simply validates the format and returns NULL if it
    is invalid, "OK" if it is valid.  In this case, the caller does not need
    to free the returned pointer.
    ---------------------------------------------------------------------[>]-*/

char *
build_sort_key (char *data, char *format)
{
    static struct {
        char    identifier;             /*  Letter that identifies format    */
        size_t  min_length;             /*  Minimum length for this letter   */
        int     colon_posn;             /*  Expected position of ':'         */
        Bool    want_digit;             /*  Expect digit after colon         */
    } format_table [] = {
        { 'f', 4, 2, TRUE  },           /*  fx:n{,m}                         */
        { 'c', 3, 1, TRUE  },           /*  c:n[-m]{,n[-m]}                  */
        { 'x', 3, 1, FALSE }            /*  x:name{,name}                    */
    };
    Bool
        format_valid = FALSE;           /*  Until proven innocent            */
    int
        format_nbr,
        split_count,                    /*  Count of data fields             */
        field_nbr,                      /*  Index into split data            */
        char_nbr,                       /*  Index into original data         */
        key_length,                     /*  Length of key so far             */
        range_from,                     /*  Range of numbers from format     */
        range_to;
    char
        *key_value = NULL,              /*  Composed key                     */
        **split_values,                 /*  Tokenised string table           */
        *format_ptr;                    /*  Points into format string        */
    XML_ITEM
        *data_root,                     /*  Data broken into XML tree        */
        *xml_item;                      /*  Actual XML data is here          */

    /*  Validate format for basic correctness                                */
    if (format && *format) {
        for (format_nbr = 0; format_nbr < tblsize (format_table); format_nbr++) {
            if (format [0]      == format_table [format_nbr].identifier
            &&  strlen (format) >= format_table [format_nbr].min_length
            &&  format [format_table [format_nbr].colon_posn] == ':') {
                format_valid = TRUE;    /*  At least it looks okay           */
                if (format_table [format_nbr].want_digit
                && !isdigit (format [format_table [format_nbr].colon_posn + 1]))
                    format_valid = FALSE;   /*  Ah, no, sorry!               */
                break;
            }
        }
    }
    /*  Special case for NULL data - just validate the format                */
    if (!data)
        return (format_valid? "OK": NULL);
        
    key_length = 0;
    if (format_valid) {
        key_value = mem_alloc (KEY_MAX_SIZE + 1);
        strclr (key_value);
        
        format_ptr = format + format_table [format_nbr].colon_posn + 1;
        switch (format [0]) {
            case 'f':
                format_ptr = format + 3;
                split_values = tok_split_ex (data, format [1]);
                for (split_count = 0; split_values [split_count]; split_count++);
                while (*format_ptr) {
                    format_ptr = get_next_range (format_ptr, &range_from, &range_to);
                    if (!format_ptr) {
                        format_valid = FALSE;
                        break;          /*  Invalid format                   */
                    }
                    if (range_from == 0)
                        break;          /*  No more field specifiers         */
                    if (range_to == -1 || range_to > split_count)
                        range_to = split_count;
                    /*  Now collect field values, indexed from 0 (not 1)     */
                    for (field_nbr = range_from - 1; field_nbr < range_to; field_nbr++)
                        add_field_to_key (key_value, &key_length, split_values [field_nbr]);
                }
                tok_free (split_values);
                if (key_value [key_length - 1] == '\1')
                    key_value [key_length - 1] = '\0';
                break;

            case 'c':
                format_ptr = format + 2;
                while (*format_ptr) {
                    format_ptr = get_next_range (format_ptr, &range_from, &range_to);
                    if (!format_ptr) {
                        format_valid = FALSE;
                        break;          /*  Invalid format                   */
                    }
                    if (range_from == 0)
                        break;          /*  No more field specifiers         */
                    if (range_to == -1 || range_to > (int) strlen (data))
                        range_to = strlen (data);
                    /*  Now collect column values, indexed from 0 (not 1)    */
                    for (char_nbr = range_from - 1; char_nbr < range_to; char_nbr++)
                        key_value [key_length++] = data [char_nbr];
                }
                key_value [key_length] = '\0';
                break;
                
            case 'x':
                data_root = xml_create (NULL, NULL);
                if (xml_load_string (&data_root, data, FALSE))
                    format_valid = FALSE;
                else {
                    /*  Split format into field names                        */
                    xml_item = xml_first_child (data_root);
                    split_values = tok_split_ex (format + 2, ',');
                    for (split_count = 0; split_values [split_count]; split_count++)
                        add_field_to_key (key_value, &key_length,
                            xml_get_attr (xml_item, split_values [split_count], ""));
                    tok_free (split_values);
                    xml_free (data_root);
                }                
                if (key_value [key_length - 1] == '\1')
                    key_value [key_length - 1] = '\0';
                break;
        }
    }
    if (format_valid)
        return (key_value);
    else {
        mem_strfree (&key_value);
        return (NULL);
    }
}


/*  Pick-up next field number or range from format.  Returns updated format  */
/*  pointer if okay, else returns NULL.                                      */

static char *
get_next_range (char *format, int *from, int *to)
{
    *from = *to = 0;                    /*  Unless something found           */
    while (isdigit (*format))           /*  Pick-up first numeric value      */
        *from = *from * 10 + *format++ - '0';
    
    if (*format == '-') {               /*  n-m means a range                */
        format++;                       /*  Skip hyphen                      */
        if (isdigit (*format)) {
            while (isdigit (*format))
                *to = *to * 10 + *format++ - '0';
        }
        else
            *to = -1;                   /*  n- means from n to end           */
    }
    if (*format == ',')                 /*  , is a separator, just skip it   */
        format++;

    /*  To keep matters simple, we always return a range, even of 1 field    */
    if (*to == 0)
        *to = *from;

    /*  Now check that what we picked-up makes some kind of sense            */
    if ((*to == -1 || *to >= *from)
    &&  (*format == '\0' || isdigit (*format)))
        return (format);
    else
        return (NULL);
}

/*  Append a value to the key, and update the key length.  We put a binary
    1 at the end of the value, so that the key will sort correctly whatever
    the variation in field values.  The key is left correctly terminated.
    If the resulting key would be too large, the value is not added.
 */
static void
add_field_to_key (char *key, int *key_length, char *value)
{
    if (*key_length + strlen (value) < KEY_MAX_SIZE) {
        strcat (key + *key_length, value);
        *key_length += strlen (value);
        key [*key_length] = '\1';
        *key_length += 1;
        key [*key_length] = '\0';
    }
}

