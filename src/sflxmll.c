/*===========================================================================*
 *                                                                           *
 *  sflxmll.c - XML loading and saving functions                             *
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

#include "prelude.h"                    /*  Universal include file           */
#include "sfldate.h"                    /*  Date and time functions          */
#include "sflfile.h"                    /*  File access functions            */
#include "sfllist.h"                    /*  List access functions            */
#include "sflmem.h"                     /*  Memory allocation functions      */
#include "sflstr.h"                     /*  String access functions          */
#include "sflxml.h"                     /*  XML definitions                  */
#include "sflsymb.h"                    /*  Symbol table access functions    */
#include "sflenv.h"                     /*  Environment access functions     */
#include "sfltok.h"                     /*  Token substitution functions     */
#include "sflhttp.h"                    /*  Meta-char encoding/decoding      */
#include "sflcons.h"                    /*  Console reporting functions      */
#include "sfldescr.h"                   /*  Descriptor functions             */
#include "sflxmll.h"                    /*  Include prototype data           */

#define BUFFER_SIZE     16384

typedef struct _XML_BUFFER              /*  API to load XML files/string     */
{
    int   state;                        /*  Most of time : BUF_READY         */
    int   pageln,                       /*  length of current page loaded    */
          cur;                          /*  index in page to last read char  */
    char *page;                         /*  page currently loaded            */
    Bool  page_alloc;                   /*  was page memory allocated?       */
    FILE *fd;
    char *fname,                        /*  file name if from file           */
          cur_line [BUFFER_SIZE + 1];   /*  last line returned so far        */
    int   line_nbr,                     /*  number of \n encountered         */
          char_nbr,                     /*  index for cur_line               */
          vchar_nbr;                    /*  Virtual position we are at       */
    int   tab_spaces;                   /*  Count of spaces left to return   */
} XML_BUFFER;

static char
    last_error[BUFFER_SIZE+1];

/*  Function prototypes                                                      */

static int   xml_save_file_item      (FILE *xmlfile, XML_ITEM *item, int generation);
static void  xml_save_string_item    (char *xml_string, XML_ITEM *item);
static int   xml_item_size           (XML_ITEM *item);
static void  init_charmaps           (void);
static void  build_charmap           (byte flag, char *chars);
static char *make_long_filename      (const char *path, const char *filename);
static void  dispose_xml_buffer      (XML_BUFFER *buf);
static int   buf_seek_next           (XML_BUFFER * buf, char c, Bool *non_white_skipped);
static int   buf_load_page           (XML_BUFFER * buf);
static int   xml_load_item           (XML_ITEM *item, XML_BUFFER *buf);
static int   buf_seek_next_nonwhite  (XML_BUFFER *buf);
static char *buf_get_name            (XML_BUFFER *buf);
static char *buf_get_string          (XML_BUFFER *buf);
static char *buf_get_item_value      (XML_BUFFER *buf);
static char *buf_get_white_char      (XML_BUFFER *buf);
static char *buf_get_until_gt        (XML_BUFFER *buf);
static int   xml_load_attribute      (XML_ITEM *item, XML_BUFFER *buf);
static int   xml_load_item_value     (XML_ITEM *item, XML_BUFFER *buf, char *blanks);
static void  set_error               (XML_BUFFER *buf, char *format, ...);
static int   xml_load_item_string    (XML_BUFFER *buffer, XML_ITEM **item, Bool allow_extended);

static XML_BUFFER *xml_buffer_new              (void);
static XML_BUFFER *init_xml_buffer_from_file   (const char *name);
static XML_BUFFER *init_xml_buffer_from_string (const char *xmlstring);
static XML_BUFFER *init_xml_buffer_from_descr  (const DESCR *descr);


/*  Character classification tables and macros                               */

static char
        *errors[10] = { "unknown error",            /* 0 */
                        "'?' expected",             /* 1 */
                        "char expected",            /* 2 */
                        "'-->' expected",           /* 3 */
                        "'>' expected",             /* 4 */
                        "tag name expected",        /* 5 */
                        "'<' expected",             /* 6 */
                        "Unexpected character: "    /* 7 */

                        };

static byte
    cmap [256];                         /*  Character classification tables  */

#define CMAP_NAME        1              /*  Normal name character            */
#define CMAP_NAME_OPEN   2              /*  Valid character to start name    */
#define CMAP_QUOTE       4              /*  Possible string delimiters       */
#define CMAP_PRINTABLE   8              /*  Valid characters in literal      */
#define CMAP_DECIMAL    16              /*  Decimal digits                   */
#define CMAP_HEX        32              /*  Hexadecimal digits               */
#define CMAP_SPACE      64              /*  White space                      */
#define CMAP_ITEM_END  128              /*  End of item body                 */

                                        /*  Macros for character mapping:    */
#define is_name(ch)      (cmap [(byte) (ch)] & CMAP_NAME)
#define is_name_open(ch) (cmap [(byte) (ch)] & CMAP_NAME_OPEN)
#define is_quote(ch)     (cmap [(byte) (ch)] & CMAP_QUOTE)
#define is_printable(ch) (cmap [(byte) (ch)] & CMAP_PRINTABLE)
#define is_decimal(ch)   (cmap [(byte) (ch)] & CMAP_DECIMAL)
#define is_hex(ch)       (cmap [(byte) (ch)] & CMAP_HEX)
#define is_space(ch)     (cmap [(byte) (ch)] & CMAP_SPACE)
#define is_item_end(ch)  (cmap [(byte) (ch)] & CMAP_ITEM_END)


                                        /*  Macros for XML_BUFFER */
#define BUF_READY               (0)     /*  Possible states */
#define BUF_FILEERROR           (1)
#define BUF_UNEXPECTED_ERROR    (2)
#define BUF_DISPOSED            (3)
#define BUF_END                 (4)
#define MAX_BUF_SIZE          (64*1024) /*  page length when read from file */
#define XML_END_ITEM            (-1)

/*  Note this set of macros is used for reading through a buffer
 *  character by character, keeping track of lines, and when to load
 *  more data from a file (if loading from a file).  The code is mostly
 *  straight forward but for backwards compatibility, we need to expand
 *  tabs properly based on their position on the line.
 *
 *  We do this by returning spaces for the right number of "fake" reads
 *  for the tab character, and then go back to advancing through the
 *  buffer.  When we encounter a tab character we set tab_spaces
 *  to the number of space characters required, and decrement it each
 *  time we read out a space.  We are done when tab_spaces = 0, and
 *  we are asked for the _next_ character.  We always return a space
 *  when the current character is a tab.
 *
 *  In order to track the supposed location along the line, we use
 *  vchar_nbr to track how far along the line we notionally are.
 *  char_nbr tracks the "real" position along the line.
 */

#define CHECK_BUF_STATE                                                       \
    if (buf->state != BUF_READY)                                             \
      {                                                                       \
        res = (buf->state == BUF_END)                                        \
            ? XML_LOADERROR                                                   \
            : XML_FILEERROR;                                                  \
      }

#define CHECK_BUF_STATE_AND_BREAK_ON_ERROR(_ERROR_IDX_)                       \
    CHECK_BUF_STATE;                                                          \
    if (res != XML_NOERROR)                                                   \
      {                                                                       \
        set_error (buf, errors[_ERROR_IDX_]);                                 \
        break;                                                                \
      }

#define buf_page_fault(_BUF_)                                                 \
    ((_BUF_)->cur == (_BUF_)->pageln)



/*  Update our record of the current line.
 *  NOTE: We record the characters as they come in, and notice a new
 *  line if the _previous_ character we recorded was a '\n'.  char_nbr
 *  records the number of characters in our line; vchar_nbr
 *  records our offset from the start of the line including the offset
 *  gained by expanding tabs.
 */
#define buf_update_cur_line(_BUF_)                                            \
  {                                                                           \
    if ((_BUF_)->state == BUF_READY )                                        \
      {                                                                       \
        ASSERT ((_BUF_)->char_nbr > 0);                                      \
        if ((_BUF_)->cur_line[(_BUF_)->char_nbr-1] == '\n')                 \
          {                                                                   \
            (_BUF_)->char_nbr  = 0;                                          \
            (_BUF_)->vchar_nbr = -1;                                         \
            (_BUF_)->line_nbr++;                                             \
          }                                                                   \
        (_BUF_)->cur_line[(_BUF_)->char_nbr] = (_BUF_)->page[(_BUF_)->cur];\
        (_BUF_)->char_nbr++;                                                 \
        (_BUF_)->vchar_nbr++;                                                \
        if ((_BUF_)->char_nbr >= BUFFER_SIZE)                                \
            (_BUF_)->char_nbr = 1;                                           \
      }                                                                       \
    else                                                                      \
      {                                                                       \
        (_BUF_)->cur_line[0] = 0;                                            \
        (_BUF_)->line_nbr    = -1;                                           \
        (_BUF_)->char_nbr    = 0;                                            \
        (_BUF_)->vchar_nbr   = 0;                                            \
      }                                                                       \
  }

/*  Check if we have started a tab character -- to get the count right, we
 *  must be called _after_ the position is updated, so we get correct offset.
 */
#define buf_check_for_tab_start(_BUF_)                                        \
    if (((_BUF_)->page[(_BUF_)->cur]) == '\t')                              \
        (_BUF_)->tab_spaces = 8 - ((_BUF_)->vchar_nbr & 7);

#define buf_next(_BUF_)                                                       \
  {                                                                           \
    if ((_BUF_)->tab_spaces)                                                 \
      {                                                                       \
        (_BUF_)->tab_spaces--;                                               \
        (_BUF_)->vchar_nbr++;                                                \
      }                                                                       \
    else                                                                      \
      {                                                                       \
        _BUF_->cur++;                                                        \
        if (buf_page_fault (_BUF_))                                           \
            buf_load_page (_BUF_);                                            \
        buf_update_cur_line (_BUF_);                                          \
        buf_check_for_tab_start (_BUF_);                                      \
        if ((_BUF_)->tab_spaces)                                             \
            (_BUF_)->tab_spaces--;                                           \
     }                                                                        \
  }

#define buf_get_next(_BUF_, _RES_)                                            \
  {                                                                           \
    if ((_BUF_)->tab_spaces)                                                 \
      {                                                                       \
        (_BUF_)->tab_spaces--;                                               \
        (_BUF_)->vchar_nbr++;                                                \
        _RES_ = ' ';                                                          \
      }                                                                       \
    else                                                                      \
      {                                                                       \
        _BUF_->cur++;                                                        \
        if ((buf_page_fault(_BUF_)) && (buf_load_page(_BUF_) != BUF_READY))   \
            _RES_ = 0;                                                        \
        else                                                                  \
            _RES_ = (_BUF_)->page[(_BUF_)->cur];                            \
        buf_update_cur_line (_BUF_);                                          \
        buf_check_for_tab_start (_BUF_);                                      \
        if ((_BUF_)->tab_spaces)                                             \
          {                                                                   \
            (_BUF_)->tab_spaces--;                                           \
            _RES_ = ' ';                                                      \
          }                                                                   \
      }                                                                       \
  }

/*  If the current character is a tab, we return a space; otherwise as is    */
#define buf_get_current(_BUF_)                                                \
    (((_BUF_)->page[(_BUF_)->cur]) == '\t'                                  \
     ? ' ' : ((_BUF_)->page[(_BUF_)->cur]))

/*  This is a hack which means that if the '<' is the last byte of the       */
/*  buffer then it will always look like the end of a value item.            */
/*  Workaround is to add a space or use &lt; instead of '<'.  OTHT it's      */
/*  better than missing a close tag completely.                              */
#define buf_peek_next(_BUF_)                                                  \
    ((_BUF_)->cur + 1 < (_BUF_)->pageln? (_BUF_)->page[(_BUF_)->cur + 1]: '/')

#ifdef _DEBUG_
    #define XML_BUF_INVARIANT(_BUF_)    invariant (_BUF_, __LINE__)
#else
    #define XML_BUF_INVARIANT(_BUF_)
#endif

/*  ---------------------------------------------------------------------[<]-
    Function: xml_save_file

    Synopsis: Saves an XML tree to the specified file.  Returns XML_NOERROR
    or XML_FILEERROR.
    ---------------------------------------------------------------------[>]-*/

int
xml_save_file (
    XML_ITEM   *item,
    const char *filename)
{
    FILE
        *xmlfile;                       /*  XML output stream                */
    int
        count;                          /*  How many symbols did we save?    */

    ASSERT (item);
    ASSERT (filename);
    init_charmaps ();                   /*  Initialise character maps        */

    if ((xmlfile = file_open (filename, 'w')) == NULL)
        return XML_FILEERROR;           /*  No permission to write file      */

    /*  Write XML file header                                                */
    fprintf (xmlfile, "<?xml version=\"1.0\"?>\n");

    /*  Output XML root                                                      */
    count = xml_save_file_item (xmlfile, item, 0);

    /*  Output a final carriage return  */
    fprintf (xmlfile, "\n");

    file_close (xmlfile);
    return XML_NOERROR;
}


/*  -------------------------------------------------------------------------
 *  init_charmaps
 *
 *  Initialise character map bit tables.  These are used to speed-up
 *  token recognition and collection.
 */

static void
init_charmaps (void)
{
    memset (cmap, 0, sizeof (cmap));    /*  Clear all bitmaps                */

    /*  Name     ::= (Letter | '_' | ':') (NameChar)*                        */
    /*  NameChar ::= Letter | Digit | MiscName                               */

    /*  Map fixed character sets to various bitmaps                          */
    build_charmap (CMAP_NAME, "abcdefghijklmnopqrstuvwxyz");
    build_charmap (CMAP_NAME, "ABCDEFGHIJKLMNOPQRSTUVWXYZ");
    build_charmap (CMAP_NAME, "0123456789");
    build_charmap (CMAP_NAME, "_:.-");

    build_charmap (CMAP_NAME_OPEN, "abcdefghijklmnopqrstuvwxyz");
    build_charmap (CMAP_NAME_OPEN, "ABCDEFGHIJKLMNOPQRSTUVWXYZ");
    build_charmap (CMAP_NAME_OPEN, "_:");

    build_charmap (CMAP_QUOTE, "\"'");

    /*  Printable characters.  ???                                           */
    build_charmap (CMAP_PRINTABLE, "abcdefghijklmnopqrstuvwxyz");
    build_charmap (CMAP_PRINTABLE, "ABCDEFGHIJKLMNOPQRSTUVWXYZ");
    build_charmap (CMAP_PRINTABLE, "0123456789");
    build_charmap (CMAP_PRINTABLE, "!@#$%^&*()-_=+[]{}\\|;:'\"<>,./?`~ ");

    build_charmap (CMAP_DECIMAL, "0123456789");

    build_charmap (CMAP_HEX, "abcdefghijklmnopqrstuvwxyz");
    build_charmap (CMAP_HEX, "ABCDEFGHIJKLMNOPQRSTUVWXYZ");
    build_charmap (CMAP_HEX, "0123456789");

    build_charmap (CMAP_SPACE, " \t\r\n");

    build_charmap (CMAP_ITEM_END, "abcdefghijklmnopqrstuvwxyz");
    build_charmap (CMAP_ITEM_END, "ABCDEFGHIJKLMNOPQRSTUVWXYZ");
    build_charmap (CMAP_ITEM_END, "0123456789");
    build_charmap (CMAP_ITEM_END, "_:.-/!");
}


/*  -------------------------------------------------------------------------
 *  build_charmap
 *
 *  Encode character string and flag into character map table.  Flag should
 *  be a 1-bit value from 1 to 128 (character map is 8 bits wide).
 */

static void
build_charmap (byte flag, char *string)
{
    for (; *string; string++)
        cmap [(byte) *string] |= flag;
}


static int
xml_save_file_item (FILE *xmlfile, XML_ITEM *item, int generation)
{
    int
        count = 1;                      /*  Count 1 for current item         */
    XML_ITEM
        *child,
        *sibling;
    XML_ATTR
        *attr;
    char
        *token,
        *item_name,
        *attr_name,
        *ptr;
    Bool
        pretty;

    /*  First output item name and attributes                                */
    token = mem_alloc (BUFFER_SIZE + 1);
    item_name  = xml_item_name (item);
    if (item_name)
      {
        fprintf (xmlfile, "<%s", item_name);
        FORATTRIBUTES (attr, item)
          {
            attr_name  = xml_attr_name  (attr);
            ptr        = xml_attr_value (attr);
            http_encode_meta (token, &ptr, BUFFER_SIZE, FALSE);
            fprintf (xmlfile, "\n%*s%s = \"%s", (generation + 1) * 4, "",
                                              attr_name, token);
            while (*ptr)
              {
                http_encode_meta (token, &ptr, BUFFER_SIZE, FALSE);
                fprintf (xmlfile, "\n%s", token);
              }
            fprintf (xmlfile, "\"");
          }

        /*  If value or children exist, use long form, otherwise short form  */
        if ((child = xml_first_child (item)))
          {
            fprintf (xmlfile, ">");

            pretty = TRUE;
            for (sibling = child ;
                 sibling != NULL ;
                 sibling = xml_next_sibling (sibling))
                if (! xml_item_name (sibling))
                    pretty = FALSE;
                else
                    break;

            for (; child != NULL; child  = xml_next_sibling (child))
              {
                if (pretty)
                    fprintf (xmlfile, "\n%*s", (generation + 1) * 4, "");

                count += xml_save_file_item (xmlfile, child, generation + 1);
                if (xml_item_name (child))
                  {
                    pretty = TRUE;
                    for (sibling = xml_next_sibling (child) ;
                         sibling != NULL ;
                         sibling = xml_next_sibling (sibling))
                        if (! xml_item_name (sibling))
                            pretty = FALSE;
                        else
                            break;
                  }
              }

            if (pretty)
                fprintf (xmlfile, "\n%*s", generation * 4, "");

            fprintf (xmlfile, "</%s>", item_name);
          }
        else
            fprintf (xmlfile, "/>");
      }
    else            /*  Not name => this is a value node  */
      {
        ptr = xml_item_value (item);
        if (ptr)
            while (*ptr)
              {
                http_encode_meta (token, &ptr, BUFFER_SIZE, FALSE);
                fprintf (xmlfile, "%s", token);
              }
      }
    mem_free (token);
    return (count);
}


/*  ---------------------------------------------------------------------[<]-
    Function: xml_save_string

    Synopsis: Saves an XML tree to a string.  Returns a freshly-allocated
    string containing the XML tree, or NULL if there was insufficient memory
    to allocate a new string.
    ---------------------------------------------------------------------[>]-*/

char *
xml_save_string (
    XML_ITEM *item)
{
    size_t
        xml_size;
    char
        *xml_string;

    ASSERT (item);
    init_charmaps ();                   /*  Initialise character maps        */
    xml_size   = xml_item_size (item);
    xml_string = mem_alloc (xml_size + 1);
    if (xml_string) {
        xml_save_string_item (xml_string, item);
        /*  JS 2001/11/28 */
        if (strlen (xml_string) != xml_size)
            printf ("BUG in sflxmll:\n%s", xml_string);
    }
    return (xml_string);
}


/*  ---------------------------------------------------------------------[<]-
    Function: xml_save_descr

    Synopsis: Saves an XML tree to a descriptor.  Returns a freshly-allocated
    DESCR containing the XML tree, or NULL if there was insufficient memory
    to allocate a new descriptor.
    ---------------------------------------------------------------------[>]-*/

DESCR *
xml_save_descr (
    XML_ITEM *item)
{
    size_t
        xml_size;
    DESCR
        *xml_descr;

    ASSERT (item);
    init_charmaps ();                   /*  Initialise character maps        */
    xml_size  = xml_item_size (item);
    xml_descr = mem_descr (NULL, xml_size + 1);
    if (xml_descr)
        xml_save_string_item ((char *) xml_descr->data, item);

    return (xml_descr);
}


/*  Return string size of XML item and all its children                      */

static int
xml_item_size (XML_ITEM *item)
{
    int
        item_size;
    XML_ITEM
        *child = NULL;
    XML_ATTR
        *attr;
    char
        *token,
        *item_name,
        *attr_name,
        *ptr;

    item_size  = 0;
    token = mem_alloc (BUFFER_SIZE + 1);

    item_name = xml_item_name (item);
    if (item_name)
      {
        item_size++;
        item_size += strlen (item_name);
        FORATTRIBUTES (attr, item)
          {
            attr_name  = xml_attr_name  (attr);
            ptr        = xml_attr_value (attr);
            item_size += 3 + strlen (attr_name);
            while (*ptr)
              {
                http_encode_meta (token, &ptr, BUFFER_SIZE, FALSE);
                item_size += strlen (token);
              }
            item_size++;
          }

        /*  If value or children exist, use long form, otherwise short form  */
        if ((child = xml_first_child (item)))
          {
            item_size++;
            for (child  = xml_first_child (item);
                 child != NULL;
                 child  = xml_next_sibling (child))
              {
                item_size += xml_item_size (child);
              }
            item_size += strlen (item_name) + 3;
          }
        else
            item_size += 2;
      }
    else            /*  No name => this is a value node                      */
      {
        ptr = xml_item_value (item);
        if (ptr)
            while (*ptr)
              {
                http_encode_meta (token, &ptr, BUFFER_SIZE, FALSE);
                item_size += strlen (token);
              }
      }
    mem_free (token);
    return (item_size);
}


static void
xml_save_string_item (char *xml_string, XML_ITEM *item)
{
    XML_ITEM
        *child = NULL;
    XML_ATTR
        *attr;
    char
        *token,
        *item_name,
        *attr_name,
        *ptr;

    /*  First output item name and attributes                                */
    token = mem_alloc (BUFFER_SIZE + 1);
    xml_string [0] = 0;
    item_name = xml_item_name (item);
    if (item_name)
      {
        xstrcat (xml_string, "<", item_name, NULL);
        FORATTRIBUTES (attr, item)
          {
            attr_name  = xml_attr_name  (attr);
            ptr        = xml_attr_value (attr);
            xstrcat (xml_string, " ", attr_name, "=\"", NULL);
            while (*ptr)
              {
                http_encode_meta (token, &ptr, BUFFER_SIZE, FALSE);
                strcat (xml_string, token);
              }
            strcat (xml_string, "\"");
          }

        /*  If value or children exist, use long form, otherwise short form  */
        if ((child = xml_first_child (item)))
          {
            strcat (xml_string, ">");
            for (child  = xml_first_child (item);
                 child != NULL;
                 child  = xml_next_sibling (child))
              {
                xml_string += strlen (xml_string);
                xml_save_string_item (xml_string, child);
              }
            xstrcat (xml_string, "</", item_name, ">", NULL);
          }
        else
            strcat (xml_string, "/>");
      }
    else            /*  No name => this is a value node                      */
      {
        ptr = xml_item_value (item);
        if (ptr)
            while (*ptr)
              {
                xml_string += strlen (xml_string);
                http_encode_meta (token, &ptr, BUFFER_SIZE, FALSE);
                strcat (xml_string, token);
              }
      }
    mem_free (token);
}


/*  ---------------------------------------------------------------------[<]-
    Function: xml_error

    Synopsis: Returns the last error message generated by xml_load.
    ---------------------------------------------------------------------[>]-*/

char *
xml_error (void)
{
    return (last_error);
}


/*  ---------------------------------------------------------------------[<]-
    Function: xml_seems_to_be

    Synopsis: Reads the first line of a file.  Returns XML_NOERROR if file
    is found and has a valid XML header, XML_FILEERROR if the file could not
    be opened and XML_LOADERROR if the first line is not an XML header.
    ---------------------------------------------------------------------[>]-*/

int
xml_seems_to_be (const char *path,
                 const char *filename)
{
    XML_BUFFER
            *buf = 0;
    int
            res = XML_NOERROR;
    char
            c,
            *name;
    Bool
            non_white_skipped;

    name = make_long_filename (path, filename);
    if (!name)
        res = XML_FILEERROR;

    if (res == XML_NOERROR)
      {
        buf = init_xml_buffer_from_file (name);
        mem_free (name);
        if (!buf)
            res = XML_FILEERROR;
      }

    if (res == XML_NOERROR)
      {
        buf_seek_next (buf, '<', &non_white_skipped);
        if (non_white_skipped)
            res = XML_LOADERROR;        /*  unexpected char were found       */
        else if (buf->state != BUF_READY)
            res = XML_LOADERROR;
      }

    if (res == XML_NOERROR)
      {
        buf_get_next(buf, c);
        if (c != '?')
            res = XML_LOADERROR;
      }


    if (res == XML_NOERROR)
      {
        buf_next(buf);
        do
          {
            res = buf_seek_next (buf, '?', NULL);
            CHECK_BUF_STATE_AND_BREAK_ON_ERROR(1);          /* ? expected */
            buf_get_next(buf, c);
            CHECK_BUF_STATE_AND_BREAK_ON_ERROR(2);          /* char expected */
            buf_next(buf)
          } while (c != '>');
      }

    if (buf)
        dispose_xml_buffer (buf);

    return res;
}


/*  ---------------------------------------------------------------------[<]-
    Function: xml_load_file

    Synopsis: Loads the contents of an XML file into a given XML tree,
    or a new XML tree if none was specified.  The XML data is not checked
    against a DTD.  Returns one of XML_NOERROR, XML_FILEERROR,
    XML_LOADERROR.  If the parameter `allow_extended' is TRUE then the file
    may contain more than one XML item.  The following attributes are
    defined in the root item of the XML tree.
    <TABLE>
    filename        Name of the XML input file
    filetime        Modification time of the file, "HHMMSSCC"
    filedate        Modification date of input file, "YYYYMMDD"
    </TABLE>
    Looks for the XML file on the specified path symbol, or in the current
    directory if the path argument is null.  Adds the extension ".xml"
    to the file name if there is none already included.
    ---------------------------------------------------------------------[>]-*/

int
xml_load_file (XML_ITEM   **item,
               const char  *path,
               const char  *filename,
               Bool         allow_extended)
{
    XML_BUFFER
           *buf = NULL;
    int
            res = XML_NOERROR;
    Bool
            non_white_skipped;
    char
           *name,
            buffer [60];

    ASSERT (filename);

    name = make_long_filename (path, filename);
    if (!name)
      {
        set_error (buf, "Could not find XML file: %s", filename);
        res = XML_FILEERROR;
      }

    if (res == XML_NOERROR)
      {
        init_charmaps ();
        buf = init_xml_buffer_from_file (name);
        if (!buf)
          {
            set_error (buf, "Could not open XML file: %s", filename);
            res = XML_FILEERROR;
          }
        else
          buf->fname = strdupl (filename);
      }
    if ((!*item) && (res == XML_NOERROR))
      {
        *item = xml_new (NULL, "root", "");
        ASSERT (*item);
        /*  SM : should this init be done even when root item is NOT created ? */
      }

    if ((res == XML_NOERROR) && (!allow_extended))
      {
        xml_put_attr (*item, "filename", name);
        sprintf (buffer, "%ld", timer_to_date (get_file_time (name)));
        xml_put_attr (*item, "filedate", buffer);
        sprintf (buffer, "%ld", timer_to_time (get_file_time (name)));
        xml_put_attr (*item, "filetime", buffer);
      }

    while (res == XML_NOERROR)
      {
        buf_seek_next (buf, '<', &non_white_skipped);
        if (non_white_skipped)
          {
            res = XML_LOADERROR;        /*  unexpected char were found       */
            break;
          }
        if (buf->state == BUF_END)     /*  we processed the entire file     */
            break;
        CHECK_BUF_STATE_AND_BREAK_ON_ERROR(7);
        if (!allow_extended && (xml_first_child(*item) != NULL))
          {
            res = XML_LOADERROR;        /* root has children                 */
            break;
          }
        buf_next(buf);
        res = xml_load_item (*item, buf); /* we try to load it               */
        if (res == XML_END_ITEM)
          {
            set_error (buf, "Unexpected character: /");
            res = XML_LOADERROR;
          }
      }

    if (buf)
        dispose_xml_buffer (buf);
    if (name)
        mem_free (name);

    return res;
}


/*  ---------------------------------------------------------------------[<]-
    Function: xml_load_string

    Synopsis: Loads an XML string into a new XML tree.  The XML data is not
    checked against a DTD.  See xml_load() for details.
    ---------------------------------------------------------------------[>]-*/

int
xml_load_string (
    XML_ITEM  **item,
    const char *xmlstring,
    Bool        allow_extended)
{
    XML_BUFFER
        *buffer;

    ASSERT (xmlstring);
    buffer = init_xml_buffer_from_string (xmlstring);
    return (xml_load_item_string (buffer, item, allow_extended));
}


/*  ---------------------------------------------------------------------[<]-
    Function: xml_load_descr

    Synopsis: Loads a DESCR string into a new XML tree. The XML data is not
    checked against a DTD.  See xml_load() for details.
    ---------------------------------------------------------------------[>]-*/

int
xml_load_descr (
    XML_ITEM   **item,
    const DESCR *descr,
    Bool         allow_extended)
{
    XML_BUFFER
        *buffer;

    ASSERT (descr);
    buffer = init_xml_buffer_from_descr (descr);
    return (xml_load_item_string (buffer, item, allow_extended));
}


static int
xml_load_item_string (XML_BUFFER *buf, XML_ITEM **item, Bool allow_extended)
{
    int
        res = XML_NOERROR;
    Bool
        non_white_skipped;

    if (buf == NULL)
        return (XML_FILEERROR);

    init_charmaps ();
    if ((!*item) && (res == XML_NOERROR)) {
        *item = xml_new (NULL, "root", "");
        ASSERT (*item);
    }
    while (res == XML_NOERROR) {
        buf_seek_next (buf, '<', &non_white_skipped);
        if (non_white_skipped) {
            res = XML_LOADERROR;
            break;
        }
        if (buf->state == BUF_END)  /*  we processed the entire string   */
            break;
        CHECK_BUF_STATE_AND_BREAK_ON_ERROR(7);

        if (!allow_extended && (xml_first_child (*item) != NULL)) {
            res = XML_LOADERROR;   /* root has children                 */
            break;
        }
        buf_next (buf);
        res = xml_load_item (*item, buf);
        if (res == XML_END_ITEM) {
            set_error (buf, "Unexpected character: /");
            res = XML_LOADERROR;
        }
    }
    if (buf)
        dispose_xml_buffer (buf);

    return (res);
}


/*  ---------------------------------------------------------------------[<]-
    Function: xml_load_symtab

    Synopsis: Loads the contents of an XML tree into a symbol table.  If no
    symbol table is specified, creates a new symbol table.  The XML data is
    loaded as a set of symbols and values, where the symbol name is built
    from the section (enclosing) item name and child item name like this:
    "section:name".  Ignores any nested items.  If the same item occurs
    several times in an enclosing item, earlier symbols are overwritten.
    Performs environment substitution on symbol values.  If path is
    specified, loads the subtree that it points to or returns an empty
    table if it was not found.  Returns NULL if there is not enough memory.
    Stores these control variables in the symbol table if they exist as
    attributes of the root of the XML tree and the symbol table was freshly
    created:
    <TABLE>
    filename        Name of input file
    filetime        Time of input file, as 8-digit string "HHMMSSCC"
    filedate        Date of input file, as 8-digit string "YYYYMMDD"
    </TABLE>
    Also creates a symbol for each section item, with name equal to the
    item name, and value equal to an empty string.  Looks for the .ini file
    on the current PATH.  The table is sorted after loading.

    ---------------------------------------------------------------------[>]-*/


SYMTAB *xml_load_symtab (SYMTAB *load_symtab, XML_ITEM *xml_root, const char *path)
{
    XML_ITEM
        *xml_subtree = NULL,            /*  Subtree to load into symtab      */
        *xml_section = NULL,            /*  Current section item             */
        *xml_item    = NULL;            /*  Current item                     */
    SYMTAB
        *symtab,                        /*  Symbol table to populate         */
        *envtab;                        /*  Environment, as symbol table     */
    char
        *xml_value = NULL,              /*  Filled as we traverse            */
        *name      = NULL,              /*    the subtree                    */
        *value     = NULL;

    ASSERT (xml_root);
    if (load_symtab)                    /*  Use specified symbol table       */
        symtab = load_symtab;           /*    or create a new one            */
    else {
        symtab = sym_create_table ();
        if (symtab == NULL)
            return (NULL);              /*  Quit if insufficient memory      */
    }

    /*  Add the file information to the symbol table                         */
    sym_assume_symbol (symtab, "filename", 
                       xml_get_attr (xml_root, "filename", NULL));
    sym_assume_symbol (symtab, "filedate", 
                       xml_get_attr (xml_root, "filedate", NULL));
    sym_assume_symbol (symtab, "filetime", 
                       xml_get_attr (xml_root, "filetime", NULL));

    /*  Load the subtree determined by path                                  */
    if (path)
        xml_subtree = xml_find_item (xml_root, path);
    else
        xml_subtree = xml_root;
    if (xml_subtree == NULL) 
        return (symtab);                /*  Path not found; empty table      */

    /*  Traverse the subtree. Each item in the subtree becomes a section,
     *  each item in a section gets loaded into the symbol table. */
    envtab = env2symb ();
    FORCHILDREN (xml_section, xml_subtree) {
        sym_assume_symbol (symtab, xml_item_name (xml_section), "");

        FORCHILDREN (xml_item, xml_section) {
            name = xstrcpy (NULL, xml_item_name (xml_section), ":",
                            xml_item_name (xml_item), NULL);
            xml_value = xml_item_child_value (xml_item);
            if (xml_value == NULL)      /*  Empty item                       */
                xml_value = mem_strdup ("");
            /*  Perform environment substitution on the item value           */
            value = tok_subst (xml_value, envtab);
            ASSERT (name);
            ASSERT (value);
            sym_assume_symbol (symtab, name, value);
            mem_free (name);
            mem_free (value);
            mem_free (xml_value);
        }
    }

    sym_delete_table (envtab);
    sym_sort_table (symtab, NULL);      /*  Sort table by symbol name        */
    return (symtab);
}


/*  ---------------------------------------------------------------------[<]-
    Function: xml_load_symtab_file

    Synopsis: Loads the contents of an XML file into a symbol table. This
    function is a wrapper around xml_load_symtab(), and should be used as
    a drop-in replacement for ini_dyn_load(). See the documentation for
    xml_load_symtab() for more information.

    ---------------------------------------------------------------------[>]-*/


SYMTAB *xml_load_symtab_file (SYMTAB *load_symtab, const char *filename, const char *path)
{
    XML_ITEM
        *xml_root    = NULL;            /*  XML loaded from file             */
    SYMTAB
        *symtab;                        /*  Symbol table to populate         */
    int
        result;                         /*  Result of loading the XML file   */

    ASSERT (filename);
    if (load_symtab)                    /*  Use specified symbol table       */
        symtab = load_symtab;           /*    or create a new one            */
    else {
        symtab = sym_create_table ();
        if (symtab == NULL)
            return (NULL);              /*  Quit if insufficient memory      */
    }

    /*  Attempt to load XML from file                                        */
    result = xml_load (&xml_root, NULL, filename);
    if (result != XML_NOERROR) {
        if (result == XML_FILEERROR) {
            return (symtab);            /*  File not found; empty table      */
        } else {
            sym_delete_table (symtab);  /*  Other error                      */
            return NULL;
        }
    }

    /*  Populate symbol table                                                */
    symtab = xml_load_symtab (symtab, xml_root, path);

    xml_free (xml_root);
    return (symtab);
}


static char *
make_long_filename (const char *path,
                    const char *filename)
{
    char *
        fullpath = NULL;

    fullpath = file_where ('r', path, filename, "xml");
    if (fullpath)
        return mem_strdup (fullpath);
    else
        return NULL;
}

static XML_BUFFER *xml_buffer_new (void)
{
    XML_BUFFER
        *buffer;

    buffer = mem_alloc (sizeof (XML_BUFFER));
    if (buffer) {
        buffer->fd           = NULL;
        buffer->fname        = NULL;
        buffer->cur          = 0;
        buffer->page         = NULL;
        buffer->page_alloc   = FALSE;
        buffer->pageln       = 0;
        buffer->state        = BUF_READY;
        buffer->cur_line [0] = '\n';
        buffer->line_nbr     = 0;
        buffer->char_nbr     = 1;
        buffer->vchar_nbr    = 0;
        buffer->fname        = NULL;
        buffer->tab_spaces   = 0;
    }
    return (buffer);
}

static XML_BUFFER *
init_xml_buffer_from_file (const char * name)
{
    FILE
        *file;
    XML_BUFFER
        *buffer = NULL;

    ASSERT (name);
    file = fopen (name, "rt");
    if (file) {
        buffer = xml_buffer_new ();
        if (buffer) {
            buffer->fd = file;
            buf_load_page     (buffer);
            XML_BUF_INVARIANT (buffer);
        }
    }
    return buffer;
}

static XML_BUFFER *
init_xml_buffer_from_string (const char *xml_string)
{
    XML_BUFFER
        *buffer = NULL;

    buffer = xml_buffer_new ();
    if (buffer) {
        buffer->page   = mem_strdup (xml_string);
        buffer->pageln = strlen (xml_string);
        buffer->page_alloc = TRUE;
        XML_BUF_INVARIANT (buffer);
    }
    return buffer;
}

static XML_BUFFER *
init_xml_buffer_from_descr (const DESCR *descr)
{
    XML_BUFFER
        *buffer = NULL;

    buffer = xml_buffer_new ();
    if (buffer) {
        buffer->page   = (char *) descr->data;
        buffer->pageln = descr->size;
        XML_BUF_INVARIANT (buffer);
    }
    return buffer;
}

static void
dispose_xml_buffer (XML_BUFFER * buf)
{
    ASSERT (buf);

    if (buf->fd)
      {
        fclose (buf->fd);
        buf->fd = NULL;
      }
    if (buf->page && buf->page_alloc)
        mem_free (buf->page);
    buf->page = NULL;

    strfree (&buf->fname);

    mem_free (buf);
}

/*  ----------------------------------------------------------------------
 *  make buf to point on the next occurence of char c in buffer
 *  returns XML_NOERROR on success;
 *          XML_LOADERROR if char c was not found
 *          XML_FILEERROR
 *  if NonWhiteSkipped, on exit, *NonWhiteSkipped is TRUE if at least one
 *  non white character has been lost because of seek.
 *  ---------------------------------------------------------------------- */
static int
buf_seek_next (XML_BUFFER *buf,
               char        wanted,
               Bool       *non_white_skipped)
{
    char
            ch    = '\0';

    ASSERT (buf);
    XML_BUF_INVARIANT(buf);

    if (buf->state != BUF_READY)
	return buf->state;

    if (non_white_skipped)
        *non_white_skipped = FALSE;

    while ((ch = buf_get_current (buf)) && ch != wanted)
      {
        if (non_white_skipped
        && !(*non_white_skipped)
        && ! is_space (buf_get_current (buf)))
          {
            set_error (buf, "Unexpected character: %c", buf_get_current (buf));
            *non_white_skipped = TRUE;
          }

	buf_next (buf);
	if (buf->state != BUF_READY)
	    break;
      }

    if (buf->state != BUF_READY)
	return buf->state;

    if (ch == wanted)
	return BUF_READY;

    /*  In theory this code is unreachable, but just in case... */
    buf->state = BUF_UNEXPECTED_ERROR;
    set_error (buf, "Something strange happened in buf_seek_next");
    return buf->state;
}

static int
buf_load_page (XML_BUFFER * buf)
{
    ASSERT (buf);

    if (buf->state != BUF_READY)
        return buf->state;

    if (buf->fd)
      {
        if (feof (buf->fd))
            buf->state = BUF_END;
        else
          {
            if (!buf->page)
              {
                /*  It's the first load. We allocate a memory page */
                buf->page = (char *) mem_alloc (MAX_BUF_SIZE);
                if (buf->page) {
                    memset (buf->page, 0, MAX_BUF_SIZE);
                    buf->page_alloc = TRUE;
                }
                else {
                    buf->state = BUF_UNEXPECTED_ERROR;
                    return BUF_UNEXPECTED_ERROR;
                  }
              }
            buf->pageln = fread(buf->page, 1, MAX_BUF_SIZE, buf->fd);

            if (ferror(buf->fd))
                buf->state = BUF_FILEERROR;
            else
              {
                ASSERT (!feof(buf->fd) ? buf->pageln == MAX_BUF_SIZE : TRUE);
                buf->cur = 0;
              }
          }
      }
    else
         buf->state = BUF_END;

    buf_update_cur_line(buf);                     /* XXX: Is this correct?   */

    return buf->state;
}

/*-----------------28/07/00 2:25--------------------
 * loads an xml item from buffer and attachs it to Item.
 * assumes that buffer currently points to '<'
 * returns XML_NOERROR on success
 *         XML_FILEERROR on io error
 *         XML_LOADERROR on xml syntax error
 * on success, buffer is currently pointing to the next non white char after the end of the previously loaded item
 * --------------------------------------------------*/
static int
xml_load_item (XML_ITEM  *item, XML_BUFFER *buf)
{
    char
            c = 0,
           *comment,
           *blanks,
           *item_name     = NULL,
           *end_item_name = NULL;
    unsigned int
            size;
    Bool
            found;
    XML_ITEM
           *new_item;
    int
            res = XML_NOERROR;

    ASSERT (item);
    ASSERT (buf);
    XML_BUF_INVARIANT(buf);

    buf_seek_next_nonwhite(buf);
    CHECK_BUF_STATE;
    if (res == XML_NOERROR)
      {
        c = buf_get_current(buf);
        if (c == '/')
          {
            buf_next(buf);
            return XML_END_ITEM;
          }
        if (c == '!')
          {
            /*  This is either a comment or a DOCTYPE declaration.           */
            buf_get_next (buf, c);
            if (c == '-') {
                buf_get_next (buf, c);
                if (c != '-') {
                    set_error (buf, "'--' expected");
                    return XML_LOADERROR;
                }

                /*  It is a comment. Skip everything until the next occurence
                 *  of '-->'
                 */
                found = FALSE;
                while (!found)
                  {
                    buf_next(buf);
                    CHECK_BUF_STATE_AND_BREAK_ON_ERROR(3); /* '-->' expected */
                    comment = buf_get_until_gt(buf);
                    CHECK_BUF_STATE_AND_BREAK_ON_ERROR(3);
                    ASSERT (comment);
                    size = strlen(comment);
                    found = ((size >= 2)
                        && (comment[size-1] == '-')
                        && (comment[size-2] == '-')) ? TRUE : FALSE;
                    mem_free (comment);
                  }
                if (found)
                    buf_next(buf);
            }
            else if (c == 'D') {
                item_name = buf_get_name (buf);
                if (!item_name || !strcmp (item_name, "OCTYPE")) {
                    set_error (buf, "'--' or DOCTYPE declaration expected");
                    return XML_LOADERROR;
                }
                mem_free (item_name);

                /*  It is a DOCTYPE declaration. Skip everything until the
                 *  next '>'.
                 */
                buf_seek_next (buf, '>', NULL);
                CHECK_BUF_STATE;
                if (res != XML_NOERROR) {
                    set_error (buf, "'>' expected");
                    return res;
                }
                buf_next (buf);
            }
            else {
                set_error (buf, "'--' or DOCTYPE declaration expected");
                return XML_LOADERROR;
            }
            return res;
          }
        if (c == '?')
          {
            /*  processing instruction. We skip it. syntax is: <? blahblah ?>*/
            /*  -->we search next '?' until next char after '?' is '>'      */
            buf_next(buf);
            do
              {
                res = buf_seek_next (buf, '?', NULL);
                CHECK_BUF_STATE_AND_BREAK_ON_ERROR(1);  /* ? expected */
                buf_get_next(buf, c);
                CHECK_BUF_STATE_AND_BREAK_ON_ERROR(4);  /* > expected */
                buf_next(buf)
              } while (c != '>');

            return res;
          }

        if (! is_name_open(c))
          {
            set_error (buf, "item name should begin with an alphanumeric char");
            return XML_LOADERROR;
          }
      }

    /*  here, we're sure we are trying to actually load a new item           */
    ASSERT (is_name_open(c));
    ASSERT (res == XML_NOERROR);

    item_name = buf_get_name (buf);
    if (! item_name)
        return XML_LOADERROR;

    /*  we create an item and we'll set the name later                       */
    new_item = xml_create_no_dup(item_name, NULL);
    if (!new_item)
        return XML_FILEERROR;           /*  memory exhausted ?               */

    /*  so far, we've create the item, with its name.                        */
    /*  !!! if an error occurs, we'll have to free the item !!!              */
    /*  We now manage its attributes                                         */
    /*  buf is currently pointing to the next char after the item name       */
    while (res == XML_NOERROR)
      {
        buf_seek_next_nonwhite(buf);
        CHECK_BUF_STATE_AND_BREAK_ON_ERROR(7);

        c = buf_get_current(buf);
        ASSERT (c);
        if ((c == '/') || (c == '>'))
            break;                      /*  we're done with item attributes  */

        /*  we have an attribute. We try to load it.                         */
        res = xml_load_attribute (new_item, buf);
        if (res != XML_NOERROR)
            break;
      }

    if (res == XML_NOERROR)
      {
        ASSERT (c == buf_get_current(buf));
        ASSERT ((c == '/') || (c == '>'));
        if (c == '/')
          {
            /*  CASE 1. : <item attributes... />                             */
            /*  the item has no child item nor value. We check for "/>"      */
            buf_next(buf);
            buf_seek_next_nonwhite(buf);
            CHECK_BUF_STATE;
            if (res == XML_NOERROR)
              {
                if (buf_get_current(buf) == '>')
                    buf_next(buf)
                else
                  {
                    set_error (buf, "Unexpected token at end of item: %c", buf_get_current(buf));
                    res = XML_LOADERROR;
                  }
              }
          }
        else
          {
            /*  CASE 2. : <item attributes...> child and/or values </item>   */
            /*  we now look for closing item tag, ie </item>                 */
            buf_next(buf);
            FOREVER
              {
                /*  we handle children items/values while closing tag is not found */
                blanks = buf_get_white_char(buf);
                CHECK_BUF_STATE_AND_BREAK_ON_ERROR(7);
                c = buf_get_current(buf);
                if (c == '<')
                  {
                    mem_free (blanks);  /* blanks are unsignificant          */
                    buf_next(buf);
                    CHECK_BUF_STATE_AND_BREAK_ON_ERROR(5);
                    res = xml_load_item (new_item, buf);
                    if (res != XML_NOERROR)
                      {
                        if (res == XML_END_ITEM)
                          {
                            /*  not an error. end tag has been found.        */
                            end_item_name = buf_get_name (buf);

                            if (strcmp (item_name, end_item_name) == 0) {
                                res = XML_NOERROR;

                                /* Skip optional white space at end of name */
                                /* Then check for '>" at end of end tag.    */
                                            if (buf_seek_next_nonwhite(buf) == BUF_READY)
                                {
                                    if (buf_get_current(buf) == '>') {
                                            buf_next(buf);
                                    }
                                else
                                {
                                    res = XML_LOADERROR;
                                    set_error (buf, "Expected '>' to close end tag, got '%c'", buf_get_current(buf));
                                }
                                }
                                else
                                {
                                res = XML_LOADERROR;
                                                set_error (buf, "Expected '>' to close end tag, ran out of data");
                                }
                            }
                            else
                            {
                                res = XML_LOADERROR;
                                set_error (buf, "Incorrect closing tag name: %s (expected %s)", end_item_name, item_name);
                              }
                            mem_free (end_item_name);
                          }
                        break;
                      }
                  }
                else
                  {
                    /* we have found an item value                           */
                    res = xml_load_item_value (new_item, buf, blanks);
                    mem_free (blanks);
                  }
              }
          }
      }

    if (res == XML_NOERROR)
        xml_attach_child (item, new_item);
    else
      {
        /*  we mem_free allocated items                                      */
        if (new_item)
            xml_free (new_item);
      }

    return res;
}

/*  XXX: This routine _MUST_ use the macros to walk through the buffer
 *  in order for our tab expansion to work.  It has been hacked to do
 *  so, but this probably means at least some of the code is now
 *  redundant, so this needs to be optimised.
 */
static int
buf_seek_next_nonwhite (XML_BUFFER * buf)
{
    char
        ch = '\0';

    ASSERT (buf);
    XML_BUF_INVARIANT(buf);

    if (buf->state != BUF_READY)
	return buf->state;

    while ((ch = buf_get_current (buf)) && is_space (ch))
      {
	buf_next (buf);
	if (buf->state != BUF_READY)
	    break;
      }

    if (buf->state != BUF_READY)
	return buf->state;

    if (! is_space (ch))
	return BUF_READY;

    /*  In theory this code is unreachable, but just in case... */
    buf->state = BUF_UNEXPECTED_ERROR;
    set_error (buf, "Something strange happened in buf_seek_next_nonwhite");
    return buf->state;
}

#define STRING_BUF_SIZE             (64)
#define APPEND(_c_)                                                           \
  {                                                                           \
    ASSERT (res_size <= res_alloc_size);                                      \
    if (res_size >= res_alloc_size)                                           \
      {                                                                       \
        /* res is, or could become, full (or NULL) -->we increase its size*/ \
        if (!res_alloc_size)                                                  \
          {                                                                   \
            res_alloc_size = STRING_BUF_SIZE;                                 \
            res = (char*) mem_alloc (res_alloc_size);                         \
          }                                                                   \
        else                                                                  \
          {                                                                   \
            res_alloc_size += STRING_BUF_SIZE;                                \
            res = (char*) mem_realloc (res, res_alloc_size);                  \
          }                                                                   \
        if (!res)                                                             \
            return NULL;     /* reallocation failed. NULL is returned */      \
      }                                                                       \
    res[res_size++] = (_c_);                                                  \
  }

#define BUF_GET_UNTIL(buf)                                                    \
    char * res = NULL;                                                        \
    unsigned int res_alloc_size = 0;                                          \
    unsigned int res_size = 0;                                                \
    char metachar[7];                                                         \
    int metasize;                                                             \
    int idx;                                                                  \
    char c;                                                                   \
                                                                              \
    /* if meta char must be escaped, & can not be a terminator, */            \
    /* otherwise mechanism can be wrong (?) */                                \
    ASSERT (_ESCAPEMETA_ ? !TERMINATOR('&') : TRUE);                          \
    ASSERT (buf);                                                             \
    c = buf_get_current(buf);                                                 \
    FOREVER                                                                   \
      {                                                                       \
        if (_ESCAPEMETA_ && (c == '&'))                                       \
          {                                                                   \
            /* perhaps a metachar. We handle the case and continue. */        \
            metasize=0;                                                       \
            buf_get_next(buf, c);                                             \
            while ((metasize < 7) && c && isalnum(c))                         \
              {                                                               \
                metachar[metasize++] = c;                                     \
                buf_get_next(buf, c);                                         \
              }                                                               \
            if (c == ';')                                                     \
              {                                                               \
                c = decode_meta_charn (metachar, metasize);                   \
                if (c)                                                        \
                    APPEND (c)                                                \
                else                                                          \
                  {                                                           \
                    APPEND ('&');                                             \
                    for (idx=0; idx<metasize; idx++) {                        \
                        APPEND (metachar[idx]);                               \
                    }                                                         \
                    APPEND (';');                                             \
                  }                                                           \
                buf_get_next(buf, c);                                         \
              }                                                               \
            else                                                              \
              {                                                               \
                APPEND ('&');                                                 \
                for (idx=0; idx<metasize; idx++)                              \
                    APPEND (metachar[idx])                                    \
              }                                                               \
            continue;                                                         \
          }                                                                   \
                                                                              \
        if (TERMINATOR(c))                                                    \
          {                                                                   \
            /* we've found what we search and exit the loop */                \
            APPEND (0);                                                       \
            break;                                                            \
          }                                                                   \
                                                                              \
        if (buf->state != BUF_READY)                                         \
          {                                                                   \
            /* an error occured or end of the file was reached. */            \
            /* We exit the loop */                                            \
            ASSERT (!c);                                                      \
            if (res)                                                          \
              {                                                               \
                mem_free (res);                                               \
                res = NULL;                                                   \
                res_size = 0;                                                 \
              }                                                               \
            break;                                                            \
          }                                                                   \
                                                                              \
        /* we add char and read next */                                       \
        APPEND (c);                                                           \
        buf_get_next(buf, c);                                                 \
      }                                                                       \
                                                                              \
    ASSERT ((res != NULL) ? (strlen(res)==(res_size-1)) : (res_size == 0));   \
                                                                              \
    return res;                                                               \


static char *
buf_get_name(XML_BUFFER * buf)
{
    #define _ESCAPEMETA_                    (FALSE)
    #define TERMINATOR(_C_)                 (! is_name (_C_))
    BUF_GET_UNTIL(buf)
    #undef _ESCAPEMETA_
    #undef TERMINATOR
}

static char *
buf_get_string(XML_BUFFER * buf)
{
    #define _ESCAPEMETA_                    (TRUE)
    #define TERMINATOR(_C_)                 (_C_ == '"')
    BUF_GET_UNTIL(buf)
    #undef _ESCAPEMETA_
    #undef TERMINATOR
}

static char *
buf_get_item_value(XML_BUFFER * buf)
{
    #define _ESCAPEMETA_                    (TRUE)
    #define TERMINATOR(_C_)                 (_C_ == '<' && is_item_end (buf_peek_next (buf)))
    BUF_GET_UNTIL(buf)
    #undef _ESCAPEMETA_
    #undef TERMINATOR
}

static char *
buf_get_white_char(XML_BUFFER * buf)
{
    #define _ESCAPEMETA_                    (FALSE)
    #define TERMINATOR(_C_)                 (! strchr(" \t\r\n", c))
    BUF_GET_UNTIL(buf)
    #undef _ESCAPEMETA_
    #undef TERMINATOR
}

static char *
buf_get_until_gt(XML_BUFFER * buf)
{
    #define _ESCAPEMETA_                    (FALSE)
    #define TERMINATOR(_C_)                 (_C_ == '>')
    BUF_GET_UNTIL(buf)
    #undef _ESCAPEMETA_
    #undef TERMINATOR
}

static int
xml_load_attribute (XML_ITEM * item, XML_BUFFER * buf)
{
    char * attr_name;
    char * attr_value;
    char c;

    ASSERT (buf);
    XML_BUF_INVARIANT(buf);
    if (! is_name_open (buf_get_current(buf)))
      {
        set_error (buf, "Unrecognized token : %c", buf_get_current(buf));
        return XML_LOADERROR;
      }

    attr_name = buf_get_name(buf);

    if (!attr_name)
        return XML_LOADERROR;
    ASSERT (strlen(attr_name) > 0);

    /*  we expect equal sign. buf points to a non alphanumeric char          */
    buf_seek_next_nonwhite (buf);
    c = buf_get_current(buf);
    if (c != '=')
      {
        mem_free (attr_name);
        set_error (buf, "Unexpected token loading attribute : %c",
        buf_get_current(buf));
        return XML_LOADERROR;
      }

    /*  now, we expect a double-quoted string                                */
    buf_next(buf);
    buf_seek_next_nonwhite (buf);
    c = buf_get_current(buf);
    if (c != '\"')
      {
        set_error (buf, "Invalid literal opening quote: %c", c);
        mem_free (attr_name);
        return XML_LOADERROR;
      }

    buf_next(buf);
    attr_value = buf_get_string(buf);
    if (!attr_value)
      {
        set_error (buf, "File end inside literal.");
        mem_free (attr_name);
        return XML_LOADERROR;
      }

    /*  we've found attribute name and value                                 */
    ASSERT (attr_name);
    ASSERT (attr_value);
    xml_put_attr_no_dup (item, attr_name, attr_value);

    buf_next(buf);

    return XML_NOERROR;
}

static int
xml_load_item_value (XML_ITEM   *item,
                     XML_BUFFER *buf,
                     char       *blanks)
{
    char * value;
    char * str = NULL;

    ASSERT (buf);
    ASSERT (blanks);

    value = buf_get_item_value(buf);
    if (!value)
        return XML_LOADERROR;

    ASSERT (value);
    str = (char*) mem_alloc (strlen(blanks) +  strlen(value) + 2);
    strcpy (str, blanks);
    strcat (str, value);

    /*  an item value has been found -->we create a new xml item and attach it to item */
    xml_new_no_dup (item, NULL, str);
    mem_free (value);

    return XML_NOERROR;
}

static void
set_error (XML_BUFFER *buf,
           char       *format, ...)
{
    char
        *mess_ptr;
    int
        offset;
    va_list
        argptr;

    mess_ptr = last_error;
    mess_ptr[0] = 0;
    if (buf && (buf->line_nbr >= 0))
      {
        ASSERT (buf->char_nbr >= 0);
        buf->cur_line [buf->char_nbr] = 0;
        if (buf->fname)
          {
            offset = sprintf (mess_ptr, "%s ", buf->fname);
            mess_ptr += offset;
          }
        offset = sprintf (mess_ptr, "%d: %s\n",
                          buf->line_nbr,
                          buf->cur_line);
        mess_ptr += offset;
      }

    va_start (argptr, format);          /*  Start variable arguments list    */
    offset = vsprintf (mess_ptr, format, argptr);
    va_end   (argptr);                  /*  End variable arguments list      */
    mess_ptr += offset;
}


#ifdef _DEBUG_
    #define my_assert(_COND_)                                       \
        if (! (_COND_)) {                                           \
            printf ("\nmy_ASSERT (%d)\n", LineNr);                  \
            ASSERT (_COND_);                                        \
        }

    void invariant (XML_BUFFER * buf, int LineNr)
    {
        my_assert (buf);

        if (buf->State != BUF_READY)
          {
            my_assert (buf->Page != NULL);
            my_assert (buf->PageLen > 0);
            my_assert (buf->Cur < buf->PageLen);
          }
    }
#endif



