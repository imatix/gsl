/*===========================================================================*
 *                                                                           *
 *  sflmime.c - MIME support functions                                       *
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
#include "sfldate.h"                    /*  Date and time functions          */
#include "sflmime.h"                    /*  Prototypes for functions         */
#include "sflmem.h"                     /*  Memory functions                 */
#include "sflstr.h"                     /*  String functions                 */
#include "sflcons.h"
#include "sflprint.h"                   /*  snprintf functions               */

/*  Definition                                                               */

typedef struct {
    char * ext;
    char * mime;
} MIME_TYPE;

/*  Function prototypes                                                      */

static void init_conversion_tables (void);
static int  find_month             (char *month);


/*  Global variables used in this source file only                           */

static char
    *months [] = {
        "Jan", "Feb", "Mar", "Apr", "May", "Jun",
        "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
        };
static char
    *days [] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};

static MIME_TYPE mime_table [] = {
    {"zip",     "application/zip"},
    {"doc",     "application/msword"},
    {"htm",     "text/html"},
    {"html",    "text/html"},
    {"txt",     "text/plain"},
    {"jpeg",    "image/jpeg"},
    {"jpg",     "image/jpeg"},
    {"gif",     "image/gif"},
    {"ppt",     "application/vnd.ms-powerpoint"},
    {"xls",     "application/vnd.ms-excel"},
    {"csv",     "text/csv"},
    {".",       "ext/plain"},
    {"etx",     "text/x-setext"},
    {"htp",     "text/html"},
    {"rtx",     "text/richtext"},
    {"tsv",     "text/tab-separated-values"},
    {"fh",      "image/x-freehand"},
    {"fh4",     "image/x-freehand"},
    {"fh5",     "image/x-freehand"},
    {"fh7",     "image/x-freehand"},
    {"fhc",     "image/x-freehand"},
    {"ief",     "image/ief"},
    {"jpe",     "image/jpeg"},
    {"pbm",     "image/x-portable-bitmap"},
    {"pgm",     "image/x-portable-graymap"},
    {"png",     "image/png"},
    {"pnm",     "image/x-portable-anymap"},
    {"ppm",     "image/x-portable-pixmap"},
    {"ras",     "image/x-cmu-raster"},
    {"rgb",     "image/x-rgb"},
    {"tif",     "image/tiff"},
    {"tiff",    "image/tiff"},
    {"xbm",     "image/x-xbitmap"},
    {"xpm",     "image/x-xpixmap"},
    {"xwd",     "image/x-xwindowdump"},
    {"avi",     "video/msvideo"},
    {"mov",     "video/quicktime"},
    {"movie",   "video/x-sgi-movie"},
    {"mpe",     "video/mpeg"},
    {"mpeg",    "video/mpeg"},
    {"mpg",     "video/mpeg"},
    {"qt",      "video/quicktime"},
    {"qtv",     "video/quicktime"},
    {"aif",     "audio/x-aiff"},
    {"aifc",    "audio/x-aiff"},
    {"aiff",    "audio/x-aiff"},
    {"au",      "audio/basic"},
    {"m3u",     "audio/mpegurl"},
    {"mid",     "audio/midi"},
    {"mp3",     "audio/mpeg"},
    {"mp3url",  "audio/mpegurl"},
    {"ra",      "audio/x-realaudio"},
    {"ram",     "audio/x-pn-realaudio"},
    {"rm",      "audio/x-pn-realaudio"},
    {"rmi",     "audio/midi"},
    {"rpm",     "audio/x-pn-realaudio-plugin"},
    {"snd",     "audio/basic"},
    {"wav",     "audio/wav"},
    {"aab",     "application/x-authorware-bin"},
    {"aam",     "application/x-authorware-map"},
    {"aas",     "application/x-authorware-seg"},
    {"ai",      "application/postscript"},
    {"bcpio",   "application/x-bcpio"},
    {"bin",     "application/octet-stream"},
    {"cdf",     "application/x-netcdf"},
    {"cpio",    "application/x-cpio"},
    {"csh",     "application/x-csh"},
    {"dcr",     "application/x-director"},
    {"dir",     "application/x-director"},
    {"dvi",     "application/x-dvi"},
    {"dxr",     "application/x-director"},
    {"eps",     "application/postscript"},
    {"exe",     "application/octet-stream"},
    {"gtar",    "application/x-gtar"},
    {"gz",      "application/x-gzip"},
    {"hdf",     "application/x-hdf"},
    {"jar",     "application/java-archive"},
    {"js",      "application/x-javascript"},
    {"latex",   "application/x-latex"},
    {"ltx",     "application/x-latex"},
    {"lzh",     "application/x-lzh"},
    {"man",     "application/x-troff-man"},
    {"me",      "application/x-troff-me"},
    {"ms",      "application/x-troff-ms"},
    {"nc",      "application/x-netcdf"},
    {"oda",     "application/oda"},
    {"pdf",     "application/pdf"},
    {"ps",      "application/postscript"},
    {"roff",    "application/x-troff"},
    {"rtf",     "application/rtf"},
    {"sh",      "application/x-sh"},
    {"shar",    "application/x-shar"},
    {"spl",     "application/futuresplash"},
    {"src",     "application/x-wais-source"},
    {"sv4cpio", "application/x-sv4cpio"},
    {"sv4crc",  "application/x-sv4crc"},
    {"swf",     "application/x-shockwave-flash"},
    {"t",       "application/x-troff"},
    {"tar",     "application/tar"},
    {"tcl",     "application/x-tcl"},
    {"tex",     "application/x-tex"},
    {"texi",    "application/x-texinfo"},
    {"texinfo", "application/x-texinfo"},
    {"tgz",     "application/x-gzip"},
    {"tr",      "application/x-troff"},
    {"txi",     "application/x-texinfo"},
    {"ustar",   "application/x-ustar"},
    {"wrl",     "x-world/x-vrml"},
    {"wrz",     "x-world/x-vrml"},
    {NULL,      NULL}
};

static byte char_to_base64 [128];
static char base64_to_char [64];
static Bool tables_initialised = FALSE;


/*  ---------------------------------------------------------------------[<]-
    Function: encode_base64

    Synopsis: Encodes a source buffer in Base 64 and stores the result
    in the target buffer.  The target buffer must be at least 1/3rd plus 1
    byte longer than the amount of data in the source buffer, and no less
    than 4 bytes long.  The base64 data consists of portable printable
    characters as defined in RFC 1521. Returns the number of bytes output
    into the target buffer, NOT including the NULL terminator.
    ---------------------------------------------------------------------[>]-*/

size_t
encode_base64 (const void*source, void *target, size_t source_size)
{
    int
        nb_block;                       /*  Total number of blocks           */
    const byte
        *p_source,                      /*  Pointer to source buffer         */
        *source_start;
    byte
        *p_target,                      /*  Pointer to target buffer         */
        value;                          /*  Value of Base64 byte             */

    ASSERT (source);
    ASSERT (target);

    source_start = (const byte *)source;

    if (source_size == 0)
        return (0);

    if (!tables_initialised)
        init_conversion_tables ();

    /*    Bit positions
                  | byte 1 | byte 2 | byte 3 |
    source block   87654321 87654321 87654321         -> 3 bytes of 8 bits

                  | byte 1 | byte 2 | byte 3 | byte 4 |
    Encoded block  876543   218765   432187   654321  -> 4 bytes of 6 bits
    */

    nb_block = (int) ((source_size + 2) / 3);

    p_source = (const byte *) source;   /*  Point to start of buffers        */
    p_target = (byte *) target;

    while (nb_block--)
      {
        /*  Byte 1                                                           */
        value       = *p_source >> 2;
        *p_target++ = base64_to_char [value];

        /*  Byte 2                                                           */
        value = (*p_source++ & 0x03) << 4;
        if ((size_t) (p_source - source_start) < source_size)
            value |= (*p_source & 0xF0) >> 4;
        *p_target++ = base64_to_char [value];

        /*  Byte 3 - pad the buffer with '=' if block not completed          */
        if ((size_t) (p_source - source_start) < source_size)
          {
            value = (*p_source++ & 0x0F) << 2;
            if ((size_t) (p_source - source_start) < source_size)
                value |= (*p_source & 0xC0) >> 6;
            *p_target++ = base64_to_char [value];
          }
        else
            *p_target++ = '=';

        /*  Byte 4 - pad the buffer with '=' if block not completed          */
        if ((size_t) (p_source - source_start) < source_size)
          {
            value       = *p_source++ & 0x3F;
            *p_target++ = base64_to_char [value];
          }
        else
            *p_target++ = '=';
     }
   *p_target = 0;
   return (size_t) (p_target - (byte *) target);
}


/*  ---------------------------------------------------------------------[<]-
    Function: decode_base64

    Synopsis: Decodes a block of Base 64 data and stores the resulting
    binary data in a target buffer.  The target buffer must be at least
    3/4 the size of the base 64 data.  Returns the number of characters
    output into the target buffer.
    ---------------------------------------------------------------------[>]-*/

size_t
decode_base64 (const void *source, void *target, size_t source_size)
{
    size_t
        target_size = 0;                /*  Length of target buffer          */
    int
        nb_block;                       /*  Total number of block            */
    byte
        value,                          /*  Value of Base64 byte             */
        *p_target,                      /*  Pointer in target buffer         */
    *target_start;
    const byte
        *p_source,                      /*  Pointer in source buffer         */
    *source_start;

    ASSERT (source);
    ASSERT (target);

    if (source_size == 0)
        return (0);

    if (!tables_initialised)
        init_conversion_tables ();

    source_start = (const byte *) source;
    target_start = (byte *) target;

    /*  Bit positions
                  | byte 1 | byte 2 | byte 3 | byte 4 |
    Encoded block  654321   654321   654321   654321  -> 4 bytes of 6 bits
                  | byte 1 | byte 2 | byte 3 |
    Decoded block  65432165 43216543 21654321         -> 3 bytes of 8 bits
    */

    nb_block    = source_size / 4;
    target_size = (size_t) nb_block * 3;
    target_start [target_size] = '\0';

    p_source = source_start;            /*  Point to start of buffers        */
    p_target = target_start;

    while (nb_block--)
      {
        /*  Byte 1                                                           */
        *p_target    = char_to_base64 [(byte) *p_source++ & 0x7f] << 2;
        value        = char_to_base64 [(byte) *p_source++ & 0x7f];
        *p_target++ += ((value & 0x30) >> 4);

        /*  Byte 2                                                           */
        *p_target    = ((value & 0x0F) << 4);
        value        = char_to_base64 [(byte) *p_source++ & 0x7f];
        *p_target++ += ((value & 0x3C) >> 2);

        /*  Byte 3                                                           */
        *p_target    = (value & 0x03) << 6;
        value        = char_to_base64 [(byte) *p_source++ & 0x7f];
        *p_target++ += value;
      }

    /* Ajust target size if source buffer have pad character                 */
    if (source_start [source_size - 1] == '='
    &&  target_start [target_size - 1] == 0)
        target_size--;
    if (source_start [source_size - 2] == '='
    &&  target_start [target_size - 1] == 0)
        target_size--;

   return (target_size);
}


/*  -------------------------------------------------------------------------
    init_conversion_tables function -- internal
    initialise the tables conversion for BASE64 coding
    -----------------------------------------------------------------------*/

static void
init_conversion_tables (void)
{
    byte
        value,                          /*  Value to store in table          */
        offset,
        index;                          /*  Index in table                   */

    /*  Reset the tables                                                     */
    memset (char_to_base64, 0xff, sizeof (char_to_base64));
    memset (base64_to_char, 0,    sizeof (base64_to_char));

    value  = 'A';
    offset = 0;

    for (index = 0; index < 62; index++)
      {
        if (index == 26)
          {
            value  = 'a';
            offset = 26;
          }
        else
        if (index == 52)
          {
            value  = '0';
            offset = 52;
          }
        base64_to_char [index] = value + index - offset;
        char_to_base64 [value + index - offset] = index;
      }
    base64_to_char [62]  = '+';
    base64_to_char [63]  = '/';
    char_to_base64 ['+'] = 62;
    char_to_base64 ['/'] = 63;

    char_to_base64 ['='] = 0;           /*  So that isbase64 ('=') == TRUE   */

    tables_initialised = TRUE;
}

Bool
isbase64 (char c)
{
    if (!tables_initialised)
        init_conversion_tables ();

    return (char_to_base64 [c & 0x7f] != 0xff);
}


/*  ---------------------------------------------------------------------[<]-
    Function: decode_mime_time

    Synopsis: Takes a MIME date and time string in various formats and
    converts to a date and time (both long values).  Returns TRUE if it
    could convert the date and time okay, else returns FALSE.  Accepts
    these formats:
    <TABLE>
    Mon_Jan_12_12:05:01_1995            ctime format
    Monday,_12-Jan-95_12:05:01_GMT      RFC 850
    Monday,_12-Jan-1995_12:05:01_GMT    RFC 850 iMatix extension
    Mon,_12_Jan_1995_12:05:01_GMT       RFC 1123
    </TABLE>
    The returned date and time are in local time, not GMT.
    ---------------------------------------------------------------------[>]-*/

Bool
decode_mime_time (const char *mime_string, long *date, long *time)
{
    int
        cent  = 0,
        year  = 0,
        month = 0,
        day   = 0,
        hour  = 0,
        min   = 0,
        sec   = 0;
    char
        mime_safe  [50 + 1],
        month_name [50 + 1],
        buffer     [50 + 1],
        *p_char;

    ASSERT (mime_string);
    ASSERT (date);
    ASSERT (time);

    /*  Copy input to safe string for security                               */
    strncpy (mime_safe, mime_string, 49);
    mime_safe [50] = 0;

    /*  Whatever format we're looking at, it will start with weekday.        */
    /*  Skip to first space.                                                 */
    if (!(p_char = strchr (mime_safe, ' ')))
        return FALSE;
    else
        while (isspace (*p_char))
            ++p_char;

    if (isalpha (*p_char))
        /*  ctime                                                            */
        sscanf (p_char, "%s %d %d:%d:%d %d",
                month_name, &day, &hour, &min, &sec, &year);
    else
    if (p_char [2] == '-')
      {
        /*  RFC 850                                                          */
        sscanf (p_char, "%s %d:%d:%d",
                buffer, &hour, &min, &sec);
        buffer [2] = '\0';
        day        = atoi (buffer);
        buffer [6] = '\0';
        strcpy (month_name, &buffer [3]);
        year = atoi (&buffer [7]);

        /*  Use windowing at 1970 if century is missing                      */
        if (year < 70)
            cent = 20;
        else
            cent = 19;
      }
    else
        /*  RFC 1123                                                         */
        sscanf (p_char, "%d %s %d %d:%d:%d",
                &day, month_name, &year, &hour, &min, &sec);

    if (year > 100)
      {
        cent = (int) year / 100;
        year -= cent * 100;
      }
    month = find_month (month_name);
    *date = MAKE_DATE (cent, year, month, day);
    *time = MAKE_TIME (hour, min,  sec,   0  );

    gmt_to_local (*date, *time, date, time);
    return (TRUE);
}


/*  -------------------------------------------------------------------------
    find_month function -- internal
    Converts a 3-letter month into a value 0 to 11, or -1 if the month
    name is not valid.
    -----------------------------------------------------------------------*/

static int
find_month (char *month)
{
    int
        index;

    for (index = 0; index < 12; index++)
        if (!strcmp (months [index], month))
            return (index + 1);

    return (-1);
}


/*  ---------------------------------------------------------------------[<]-
    Function: encode_mime_time

    Synopsis: Encode date and time (in long format) in Mime RFC1123 date
    format, e.g. Mon, 12 Jan 1995 12:05:01 GMT.  The supplied date and time
    are in local time.  Returns the date/time string if the date was legal,
    else returns "?".  Returned string is in a static buffer.
    ---------------------------------------------------------------------[>]-*/

char *
encode_mime_time (long date, long time)
{
    int
        day_week,                       /*  Day of week number (0 is sunday) */
        month;                          /*  Month number                     */
    static char
        buffer [50];

    local_to_gmt (date, time, &date, &time);
    day_week = day_of_week (date);
    month    = GET_MONTH   (date);
    if (day_week >= 0 && day_week < 7 && month > 0 && month < 13)
      {
        snprintf (buffer, sizeof (buffer),
                          "%s, %02d %s %04d %02d:%02d:%02d GMT",
                          days       [day_week],
                          GET_DAY    (date),
                          months     [month - 1],
                          GET_CCYEAR (date),
                          GET_HOUR   (time),
                          GET_MINUTE (time),
                          GET_SECOND (time)
                 );
        return (buffer);
      }
    else
        return ("?");
}

/*  ---------------------------------------------------------------------[<]-
    Function: get_mime_type

    Synopsis: Get mime type in table from extension. Return null if not found.
    ---------------------------------------------------------------------[>]-*/

char *
get_mime_type (char *extension)
{
    int
        index = 0;
    char
        *feedback = NULL;
    while (mime_table [index].ext)
      {
        if (strcmp (mime_table [index].ext, extension) == 0)
          {
            feedback = mime_table [index].mime;
            break;
          }
        index++;
      }
    return (feedback);
}


/*  ---------------------------------------------------------------------[<]-
    Function: encode_quoted_string

    Synopsis: Quoted-Printable Content-Transfer-Encoding function (rfc2045)
              used in mime to encode non ascii value.
              If target is NULL, the return value is allocated and must be
              free by mem_free.
    ---------------------------------------------------------------------[>]-*/

char *
encode_quoted_string (char *target, size_t target_size, const byte *source,
                      const char *charset)
{
    Bool
        have_high_value = FALSE;
    byte
        *p_source;
    char
        *p_target,
        *feedback,
        header [50];
    short
        header_length,
        target_length,
        word_length,
        source_length = 0;
    static char
        mime_specials [] = "@.,;<>[]\\\"()?/= \t";
    static char
        hex_char      [] = "0123456789ABCDEF";

#define ADD_NEW_HEADER {                               \
                if (target_length < header_length + 5) \
                    break;                             \
                strcpy (p_target, "=?=\n ");           \
                target_length -= 5;                    \
                p_target      += 5;                    \
                strcpy (p_target, header);             \
                p_target      += header_length;        \
                target_length -= header_length;        \
                word_length    = header_length;        \
              }


    if (source == NULL)
        return (NULL);

    /* Check if have a character greater than 127 in source buffer           */
    p_source = (byte *)source;
    while (*p_source)
      {
        if (*p_source & 0x80)
            have_high_value = TRUE;

        source_length++;
        p_source++;
      }
    if (have_high_value && charset)
        sprintf (header, "=?%s?Q?", charset);
    else
        strcpy  (header, "=?US-ASCII?Q?");

    header_length = strlen (header);

    if (target == NULL)
      {
         target_length =  (source_length * 3) +
                        (((source_length /72) + 1) * (header_length + 5)) + 10;
         feedback = (char *)mem_alloc (target_length);
      }
    else
      {
         feedback = target;
         target_length = target_size;
      }

    if (feedback)
      {
        strcpy (feedback, header);
        p_target       = feedback + header_length;
        word_length    = header_length;
        target_length -= header_length;
        p_source       = (byte *)source;

        while (*p_source && target_length > 3)
          {
            if (word_length >= 72)
                ADD_NEW_HEADER;
            if (*p_source == ' ')
              {
                *p_target++ = '_';
                word_length++;
                target_length--;
              }
            else
            if ((*p_source & 0x80)
            ||   *p_source == '\t'
            ||   *p_source == '_'
            ||   strchr (mime_specials, *p_source))
              {
                if (word_length >= 70)
                    ADD_NEW_HEADER;
                if (target_length < 3)
                    break;
                *p_target++ = '=';
                *p_target++ = hex_char [*p_source >> 4];
                *p_target++ = hex_char [*p_source & 15];
                word_length   += 3;
                target_length -= 3;
              }
            else
              {
                *p_target++ = *p_source;
                word_length++;
                target_length--;
              }
            p_source++;
          }
        strcpy (p_target, "?=");
      }

    return (feedback);
}

/*  ---------------------------------------------------------------------[<]-
    Function: encode_mimeb_string

    Synopsis: Base64 Content-Transfer-Encoding function (rfc2045)
              used in mime (mail)to encode non ascii value.
              If target is NULL, the return value is allocated and must be
              free by mem_free.
    ---------------------------------------------------------------------[>]-*/

char *
encode_mimeb_string (char *target, size_t target_size, const byte *source,
                      const char *charset)
{
    char
        *encoded_data;
    short
        target_length,
        encoded_length,
        header_length,
        source_length;
    char
        *p_target,
        *feedback = NULL,
        header [50];

    if (source == NULL)
        return (NULL);

    if (charset)
        sprintf (header, "=?%s?B?", charset);
    else
        strcpy  (header, "=?US-ASCII?B?");

    header_length  = strlen (header);
    source_length  = strlen ((char *) source);
    encoded_length = source_length * 2;
    encoded_data   = mem_alloc (encoded_length + 1);
    if (encoded_data)
      {
        encoded_length = encode_base64 (source, encoded_data, source_length);

        if (target == NULL)
          {
            target_length = encoded_length + header_length + 3;
            feedback = mem_alloc (target_length + 1);
          }
        else
          {
            feedback = target;
            target_length = target_size;
          }
        if (feedback)
          {
            if (target_length > header_length + 3)
              {
                p_target = feedback;
                strcpy (p_target, header);
                p_target += header_length;
                source_length = target_length - header_length - 3;
                if (source_length > encoded_length)
                    source_length = encoded_length;
                strncpy (p_target, encoded_data, source_length);
                p_target += source_length;
                strcpy (p_target, "?=");
              }
          }
        mem_free (encoded_data);
      }
   return (feedback);
}
