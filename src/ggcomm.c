/*===========================================================================*
 *                                                                           *
 *  ggcomm.c - Common functions                                              *
 *                                                                           *
 *  Copyright (c) 1996-2010 iMatix Corporation                               *
 *                                                                           *
 *  This program is free software; you can redistribute it and/or modify     *
 *  it under the terms of the GNU General Public License as published by     *
 *  the Free Software Foundation; either version 3 of the License, or (at    *
 *  your option) any later version.                                          *
 *                                                                           *
 *  This program is distributed in the hope that it will be useful, but      *
 *  WITHOUT ANY WARRANTY; without even the implied warranty of               *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU        *
 *  General Public License for more details.                                 *
 *                                                                           *
 *  For information on alternative licensing for OEMs, please contact        *
 *  iMatix Corporation.                                                      *
 *                                                                           *
 *===========================================================================*/

#include "ggpriv.h"                     /*  Header file                      */

/*- Global variables local to this module -----------------------------------*/

static char
    error_buffer [LINE_MAX + 1];

static RESULT_NODE
    *result_node_cache = NULL;          /*  To cache unused blocks           */

/*- Prototypes --------------------------------------------------------------*/

static void         construct_block        (VALUE *result);
static void         strneat                (char *text);
static char        *strleft                (char *text);
static RESULT_NODE *first_operand          (RESULT_NODE *r, int *indent);
static RESULT_NODE *next_operand           (RESULT_NODE *r, int *indent);

/*- Functions ---------------------------------------------------------------*/

Bool
gsl_file_read (
    FILE *stream,
    char *string)
{
    int
        ch,                             /*  Character read from file         */
        cnbr;                           /*  Index into returned string       */

    ASSERT (stream);
    ASSERT (string);

    cnbr = 0;                           /*  Start at the beginning...        */
    memset (string, ' ', LINE_MAX);     /*    and prepare entire line        */
    for (;;)
      {
        ch = fgetc (stream);            /*  Get next character from file     */
        if (ch == '\r')                 /*  Found carriage-return            */
            file_crlf = TRUE;           /*    Set flag and ignore CR         */
        else
        if (ch == '\n')                 /*  Have end of line                 */
          {
            string [cnbr++] = (char) ch;    /*  Add char to string           */
            string [cnbr] = '\0';       /*  Terminate string                 */
            return (TRUE);              /*  and return TRUE                  */
          }
        else
        if ((ch == EOF)                 /*    or end of file                 */
        ||  (ch == 26))                 /*    or MS-DOS Ctrl-Z               */
          {
            string [cnbr] = '\0';       /*  Terminate string                 */
            return (cnbr > 0);          /*  and return TRUE/FALSE            */
          }
        else
        if (cnbr < LINE_MAX)
            string [cnbr++] = (char) ch;    /*  Else add char to string      */

        if (cnbr >= LINE_MAX)           /*  Return in any case if line is    */
          {                             /*    too long - the line will be    */
            string [LINE_MAX] = '\0';   /*    cut into pieces                */
            return (TRUE);
          }
      }
}


char *
name_the_symbol (RESULT_NODE *node)
{
    char
        *scope_string = "",
        *point_string = "";

    if (node-> scope)
        scope_string = extended_scope_string (node-> scope);

    /*  Print point if scope & name or neither scope nor name is defined.    */
    if ((node-> scope != NULL) == (node-> name != NULL))
        point_string = ".";

    snprintf (error_buffer, LINE_MAX,
              "%s%s%s",
              scope_string,
              point_string,
              node-> name
                ? string_value (& node-> name-> value)
                : "");

    return error_buffer;
}


void
result_node_error (const char *message,
                   RESULT_NODE *node,
                   char **error_text)
{
    if (node && node-> culprit)
        snprintf (error_buffer, LINE_MAX,
                  "%s: %s", message, node-> culprit);
    else
        snprintf (error_buffer, LINE_MAX,
                  "%s", message);

    *error_text = error_buffer;
}


char *
extended_scope_string (RESULT_NODE *scope)
{
    RESULT_NODE
        *member;
    static char
        line [LINE_MAX + 1];
    int
        length = 0;

    line [0] = '\0';
    if (scope)
      {
        member = scope;
        while ((member-> script_node-> type == GG_MEMBER
           ||   member-> script_node-> type == GG_ATTRIBUTE)
           &&   member-> scope)
            member = member-> scope;

        if (member-> script_node-> type != GG_MEMBER
        &&  member-> script_node-> type != GG_ATTRIBUTE)
          {
            strncat (line, string_value (& member-> value),
                           LINE_MAX - length);
            length += strlen (member-> value. s);
            member = member-> parent;
          }
        while (member != scope-> parent && length < LINE_MAX)
          {
            if (member-> script_node-> type == GG_MEMBER)
              {
                strncat (line, "->", LINE_MAX - length);
                length += 2;
              }
            else
              {
                strncat (line, ".", LINE_MAX - length);
                length += 1;
              }
            if (member-> name)
              {
                strncat (line, string_value (& member-> name-> value),
                         LINE_MAX - length);
                length += strlen (member-> name-> value. s);
              }
            if (member-> script_node-> before)
              {
                strncat (line, "(...)", LINE_MAX - length);
                length += 5;
              }
            member = member-> parent;
          }
      }
    return line;
}


void
init_value (VALUE *value)
{
    value-> type = TYPE_UNDEFINED;
    value-> n    = 0;
    value-> s    = NULL;
    value-> b    = NULL;
    value-> c    = NULL;
    value-> i    = NULL;
}

void
destroy_value (VALUE *value)
{
    if (value-> s)
        mem_strfree (& value-> s);
    if (value-> b)
      {
        mem_free (value-> b);
        value-> b = NULL;
      }
    if (value-> i)
      {
        if (value-> c-> destroy)
            value-> c-> destroy (value-> i);
        value-> c = NULL;
        value-> i = NULL;
      }
}

void
copy_value_ (MEMTRN *memtrn, VALUE *dest, VALUE *source)
{
    char
      **blkptr,
       *txtptr;

    dest-> type = source-> type;
    dest-> s    = memtrn
                ? memt_strdup (memtrn, source-> s)
                : mem_strdup  (source-> s);
    dest-> n    = source-> n;

    if (source-> b)
      {
        dest-> b = memtrn
                 ? memt_copy (memtrn, source-> b)
                 : mem_copy  (source-> b);

        blkptr = dest-> b;
        while (*blkptr++) ;
        txtptr = (char *) blkptr;
        blkptr = dest-> b;
        while (*blkptr)
          {
            *blkptr++ = txtptr;
            txtptr += strlen (txtptr) + 1;
          }
      }
    else
        dest-> b = NULL;

    /*  Always link new value before destroying old value.  */
    if (source-> c
    &&  source-> c-> link
    &&  source-> i)
        source-> c-> link (source-> i);

    if (dest-> c
    &&  dest-> c-> destroy
    &&  dest-> i)
        dest-> c-> destroy (dest-> i);

    dest-> c = source-> c;
    dest-> i = source-> i;
}

void
assign_number (VALUE *dest, double n)
{
    dest-> type = TYPE_NUMBER;
    dest-> n    = n;
}

void
assign_string (VALUE *dest, char *s)
{
    if (s)
      {
        dest-> type = TYPE_STRING;
        dest-> s    = s;
      }
}

void
assign_pointer (VALUE *dest, CLASS_DESCRIPTOR *c, void *i)
{
    if (c)
      {
        dest-> type = TYPE_POINTER;
        dest-> c    = c;
        dest-> i    = i;

        if (c-> link)
            c-> link (i);
      }
}

void
free_pointer (CLASS_DESCRIPTOR *c, void *i)
{
    /*  Link then destroy the item.  This is useful for getting rid of an item */
    /*  that has been created but never linked to.                             */
    if (i)
      {
        if (c-> link)
            c-> link (i);
        if (c-> destroy)
            c-> destroy (i);
      }
}

void
init_result (RESULT_NODE *node)
{
    node-> next         = NULL;
    node-> parent       = NULL;
    node-> scope        = NULL;
    node-> name         = NULL;
    node-> op1          = NULL;
    node-> op2          = NULL;
    node-> as           = NULL;
    node-> to           = NULL;
    node-> before       = NULL;
    node-> after        = NULL;
    node-> operand      = NULL;
    node-> script_node  = NULL;
    node-> result_node  = NULL;
    node-> macro        = NULL;
    node-> gsl_function = NULL;
    node-> argc         = 0;
    node-> argv         = NULL;
    node-> culprit      = NULL;
    node-> indent       = 0;
    node-> width        = 0;
    node-> constant     = FALSE;
    init_value (& node-> value);
}

RESULT_NODE *
new_result_node (void)
{
    RESULT_NODE
        *node;

    if (result_node_cache)
      {
        node = result_node_cache;
        result_node_cache = node-> next;
        node-> next = NULL;
      }
    else
        node = memt_alloc (NULL, sizeof (RESULT_NODE));

    return node;
}

void
destroy_result (RESULT_NODE *node)
{
    int
        argc;

    if (node)
      {
        if (node-> scope)
            destroy_result (node-> scope);
        if (node-> name)
            destroy_result (node-> name);
        if (node-> op1)
            destroy_result (node-> op1);
        if (node-> op2)
            destroy_result (node-> op2);
        if (node-> as)
            destroy_result (node-> as);
        if (node-> to)
            destroy_result (node-> to);
        if (node-> before)
            destroy_result (node-> before);
        if (node-> after)
            destroy_result (node-> after);
        if (node-> operand)
            destroy_result (node-> operand);

        if (node-> culprit)
            mem_free (node-> culprit);

        argc = node-> argc;
        if (argc)
          {
            while (argc--)
                if (node-> argv [argc])
                     destroy_result (node-> argv [argc]);

            mem_free (node-> argv);
          }

        if (node-> value. s)
            mem_free (node-> value. s);
        if (node-> value. b)
            mem_free (node-> value. b);
        if (node-> value. c
        &&  node-> value. c-> destroy)
            node-> value. c-> destroy (node-> value. i);

        /*  Clean up link from parent to avoid multiple frees  */
        if (node-> parent)
          {
            if (node == node-> parent-> scope)
                node-> parent-> scope = NULL;
            if (node == node-> parent-> name)
                node-> parent-> name = NULL;
            if (node == node-> parent-> op1)
                node-> parent-> op1 = NULL;
            if (node == node-> parent-> op2)
                node-> parent-> op2 = NULL;
            if (node == node-> parent-> as)
                node-> parent-> as = NULL;
            if (node == node-> parent-> to)
                node-> parent-> to = NULL;
            if (node == node-> parent-> before)
                node-> parent-> before = NULL;
            if (node == node-> parent-> after)
                node-> parent-> after = NULL;
            if (node == node-> parent-> operand)
                node-> parent-> operand = NULL;

            argc = node-> parent-> argc;
            while (argc--)
                if (node == node-> parent-> argv [argc])
                    node-> parent-> argv [argc] = NULL;
          }
        node-> next = result_node_cache;
        result_node_cache = node;
      }
}

void
copy_result (RESULT_NODE *dest, RESULT_NODE *source)
{
    ASSERT (dest);

    if (source)
      {
        copy_value_ (NULL, & dest-> value, & source-> value);
        dest-> culprit  = source-> culprit;
        source-> culprit = NULL;
      }
}


/*- Type Conversion Functions -----------------------------------------------*/

char *
string_value (VALUE *value)
{
    char
        buffer [LINE_MAX + 1];
    int
        width,
        lines,
        length,
        i;
    char
      **blkptr,
       *txtptr,
       *strptr;
    VALUE
        *pointer_value;

    if (! value)
        return NULL;

    if (! value-> s)
      {
        if (value-> type == TYPE_NUMBER)
          {
            snprintf (buffer, LINE_MAX, "%.9f", value-> n);
            i = strlen (buffer) - 1;
            /*  Strip trailing zeroes  */
            while (buffer [i] == '0')
                buffer [i--] = '\0';
            if (buffer [i] == '.')
                buffer [i] = '\0';

            value-> s = mem_strdup (buffer);
          }
        else
        if (value-> type == TYPE_BLOCK)
          {
            blkptr = value-> b;
            width = 0;
            lines = 0;
            while (*blkptr)
              {
                lines++;
                txtptr = *blkptr++;
                length = strlen (txtptr);
                if (length > width)
                    width = length;
              }
            value-> s = mem_alloc (lines * (width + 1));

            blkptr = value-> b;
            strptr = value-> s;
            while (*blkptr)
              {
                strcpy (strptr, *blkptr++);
                length = strlen (strptr);
                memset (strptr + length, ' ', width - length);
                strptr [width] = '\n';
                strptr += width + 1;
              }
            *(strptr - 1) = 0;
          }
        else
        if (value-> type == TYPE_POINTER
        &&  value-> c-> get_attr)
          {
            pointer_value = value-> c-> get_attr (value-> i,
                                                  NULL,
                                                  FALSE);
            value-> s = mem_strdup (pointer_value && pointer_value-> s
                                    ? pointer_value-> s
                                    : NULL);
          }

        if (! value-> s)
            value-> s = mem_strdup ("");
      }
    return value-> s;
}


double
number_value (VALUE *value)
{
    double
        place = 0.1,
        n = 0;
    int
        sign = 1;
    char
        *ch;

    if (! value)
        return 0;

    if (value-> type == TYPE_NUMBER)
        return value-> n;
    else
      {
        if ((value-> s) && (*value-> s))
          {
            ch = value-> s;
            if (*ch == '-')
              {
                sign = -1;
                ch++;
              }
            else
            if (*ch == '+')
                ch++;

            while (isdigit (*ch))
                n = n * 10 + *ch++ - '0';

            if (*ch)
                if (*ch == '.')
                  {
                    ch++;
                    while (isdigit (*ch))
                      {
                        n += (*ch++ - '0') * place;
                        place /= 10;
                      }
                  }

            if (! *ch)
              {
                value-> type = TYPE_NUMBER;
                value-> n    = sign * n;
                return value-> n;
              }
            else
              {
                value-> type = TYPE_STRING;
                return 0;
              }
          }
      }

    return 0;
}


/*- Error Functions ---------------------------------------------------------*/

/*- Print format string -----------------------------------------------------*/


int
parse_format (char *format, size_t format_max,
              char *conversion,
              RESULT_NODE *node,
              char **error_text)
{
    char
        *in,
        *out = format;
    int
        width = 0;

    if (error_text)
        *error_text = NULL;             /*  No errors yet  */

    *out++ = '%';
    if (node-> as)
      {
        /*  Rough and ready way of rejecting overly long format strings.  */
        if (strlen (string_value (& node-> as-> value))
                                  + node-> as-> script_node-> spaces
                                  > format_max - 2)
          {
            if (error_text)
              {
                snprintf (error_buffer, LINE_MAX,
                          "Format string too long: %s",
                          node-> as-> value. s);
                *error_text = error_buffer;
              }

            return -1;
          }

        in = node-> as-> value. s;

        if (node-> as-> script_node-> spaces)     /*  Spaces  */
            *out++ = ' ';
        while (strchr ("#0- +", *in))   /*  Optional flags  */
            *out++ = *in++;
        while (isdigit (*in))           /*  Optional width  */
          {
            width = (10 * width) + (*in - '0');
            *out++ = *in++;
         }
        if (*in == '.')                 /*  Skip optional precision  */
          {
            *out++ = *in++;
            while (isdigit (*in))
                *out++ = *in++;
          }
        *conversion = *in++;
        if (*in == '\0')
          {
            if (strchr ("diouxX", *conversion))
              {
                *out++ = 'l';     /*  All integers are long  */
                *out++ = *conversion;
                *out = '\0';
                return width;
              }
            else
            if (strchr ("eEfg", *conversion))
              {
                *out++ = *conversion;
                *out = '\0';
                return width;
              }
            else
            if (*conversion == 'c')
              {
                *out++ = *conversion;
                *out = '\0';
                return width;
              }
            else
            if (*conversion == 's')
              {
                *out++ = *conversion;
                *out = '\0';
                return width;
              }
          }
      }
    else
      {
        *conversion = 's';
        *out++ = *conversion;
        *out = '\0';
        return 0;
      }

    if (error_text)
      {
        snprintf (error_buffer, LINE_MAX,
                  "Invalid format string: %s",
                  node-> as-> value. s);
        *error_text = error_buffer;
      }
    return -1;
}


int
format_output (char *format, char conversion, int width,
               RESULT_NODE *node)
{
    char
        local_buffer [LINE_MAX + 1],
        *line_ptr,
        *line_end,
        *buffer,
        *buf_ptr;
    int
        total_length,
        line_length,
        out_length,
        rc = 0;

    if (conversion == 's')
      {
        string_value (& node-> value);
        total_length = 0;
        line_ptr = node-> value. s;
        while (line_ptr && *line_ptr)
          {
            line_end = strchr (line_ptr, '\n');
            if (line_end)
                line_length = line_end - line_ptr;
            else
                line_length = strlen (line_ptr);

            if (line_length < width)
                line_length = width;

            total_length += line_length;
            if (line_end)
              {
                line_ptr = line_end + 1;
                total_length += 1;
              }
            else
                line_ptr = NULL;

          }

        buffer = mem_alloc (total_length + 1);
        buf_ptr = buffer;

        line_ptr = node-> value. s;
        out_length = 0;
        while (line_ptr && *line_ptr)
          {
            line_end = strchr (line_ptr, '\n');
            if (line_end)
                *line_end = 0;

            rc = sprintf (buf_ptr, format, line_ptr);
            buf_ptr += rc;
            out_length += rc;

            if (line_end)
              {
                *line_end = '\n';
                *buf_ptr  = '\n';
                buf_ptr += 1;
                out_length += 1;
                line_ptr = line_end + 1;
              }
            else
                break;
          }
        *buf_ptr = 0;

        ASSERT (out_length == total_length);

        mem_free (node-> value. s);
        node-> value. s = buffer;
      }
    else
      {
        // The following conversion is only valid for doubles in the range
        // (0 .. 2^32)
        if (strchr ("diouxX", conversion))
            rc = snprintf (local_buffer, LINE_MAX, format,
                          (unsigned long) number_value (& node-> value));
        else
        if (strchr ("eEfg", conversion))
            rc = snprintf (local_buffer, LINE_MAX, format,
                           number_value (& node-> value));
        else
        if (conversion == 'c')
            rc = snprintf (local_buffer, LINE_MAX, format,
                          (int) number_value (& node-> value));

        mem_free (node-> value. s);
        node-> value. s = mem_strdup (local_buffer);
      }

    if (node-> value. type == TYPE_BLOCK)
        construct_block (& node-> value);
    else
        node-> value. type = TYPE_UNKNOWN;

    return rc;
}


static void
construct_block (VALUE *result)
{
    char
        *startptr,
        *endptr,
        *txtptr;
    char
        **blkptr;
    int
        length,
        lines,
        blksize,
        txtsize;

    /*  Calculate number of lines and total size of block.  */
    lines   = 0;
    txtsize = 0;
    startptr = result-> s;
    while (startptr)
      {
        lines++;
        endptr = strchr (startptr, '\n');
        if (! endptr)
            endptr = startptr + strlen (startptr);
        txtsize += endptr - startptr + 1;

        if (*endptr)
            startptr = endptr + 1;
        else
            startptr = 0;
      }
    blksize = (lines + 1) * sizeof (char *);

    result-> b  = mem_alloc (blksize + txtsize);
    blkptr  = result-> b;
    txtptr  = ((char *) result-> b) + blksize;
    startptr = result-> s;
    while (startptr)
      {
        endptr = strchr (startptr, '\n');
        if (! endptr)
            endptr = startptr + strlen (startptr);
        length = endptr - startptr;
        strncpy (txtptr, startptr, length);
        txtptr [length] = 0;
        *blkptr++ = txtptr;
        txtptr += length + 1;
        if (*endptr)
            startptr = endptr + 1;
        else
            startptr = 0;
      }
    *blkptr = 0;

    mem_free (result-> s);
    result-> s    = NULL;

    /*  JS temporary just to make sure I haven't blundered.  */
    ASSERT (txtptr - (char *) result-> b == blksize + txtsize);
}


/*  pretty_print: Reformats a string according to a format string consisting */
/*  of a comma-separated series of specifiers.  Currently accepted formats:  */
/*         lower - lower case                                                */
/*         UPPER - UPPER CASE                                                */
/*         Neat  - Neat Case                                                 */
/*         Camel - CamelCase                                                 */
/*         no    - as-is                                                     */
/*         c     - Valid_c_identifier                                        */
/*         COBOL - VALID-COBOL-IDENTIFIER                                    */
/*         justify - Left justify, moving words                              */
/*         left - Remove spaces from all lines so that at least one line     */
/*                has no spaces at the start.                                */
/*         block - Turn multi-line into a block                              */
/*                                                                           */
/*  If no case modifier is provided and pretty is not empty and the expr-    */
/*  is a single identifier, its case is used as an example to match for      */
/*  lower, upper or neat case.                                               */

int
pretty_print (VALUE *result,
              RESULT_NODE *pretty,
              char *example,
              int space,
              char **error_text)
{
    char
        *tokens,
        *token,
        *c,
        *newstring,
        *oldpos,
        *newpos;
    Bool
        use_example,
        new_word;

    if (error_text)
        *error_text = NULL;             /*  No errors yet  */

    string_value (result);

    use_example = (example != NULL);

    if (pretty) {
        if (strlen (string_value (& pretty-> value)) == 0)
            use_example = FALSE;

        tokens = mem_strdup (pretty-> value. s);
        token = strtok (tokens, ", ");
        while (token) {
            strlwc (token);
            if (streq (token, "lower")) {
                use_example = FALSE;
                strlwc (result-> s);
            }
            else
            if (streq (token, "upper")) {
                use_example = FALSE;
                strupc (result-> s);
            }
            else
            if (streq (token, "camel") || streq (token, "pascal")) {
                use_example = FALSE;
                oldpos = result->s;
                newpos = result->s;
                new_word = streq (token, "pascal") ? TRUE: FALSE;
                while (*oldpos) {
                    if (!isalnum (*oldpos))
                        new_word = TRUE;
                    else
                    if (new_word) {
                        *newpos = toupper (*oldpos);
                        newpos++;
                        new_word = FALSE;
                    }
                    else {
                        *newpos = *oldpos;
                        newpos++;
                    }
                    oldpos++;
                }
                *newpos = 0;
            }
            else
            if (streq (token, "no"))
                use_example = FALSE;
            else
            if (streq (token, "neat")) {
                use_example = FALSE;
                strneat (result-> s);
            }
            else
            if (streq (token, "c")) {
                c = result-> s;
                if (*c && !isalpha (*c))
                    *c = '_';

                while (*c) {
                    if (!(isalpha (*c) || isdigit (*c)))
                        *c = '_';
                    c++;
                }
            }
            else
            if (streq (token, "cobol")) {
                c = result-> s;
                if (*c && !isalpha (*c))
                    *c = '-';

                while (*c) {
                    if (!(isalpha (*c) || isdigit (*c)))
                        *c = '-';
                    c++;
                }
            }
            else
            if (streq (token, "justify")) {
                newstring = strjustify (result-> s, space);
                mem_free (result-> s);
                result-> s = newstring;
            }
            else
            if (streq (token, "left")) {
                newstring = strleft (result-> s);
                mem_free (result-> s);
                result-> s = newstring;
            }
            else
            if (streq (token, "block"))
                result-> type = TYPE_BLOCK;
            else
            if (error_text) {
                snprintf (error_buffer, LINE_MAX,
                          "Unknown pretty-print modifier: %s", token);
                mem_free (tokens);
                *error_text = error_buffer;
                return -1;
            }
            token = strtok (NULL, ", ");
        }
        mem_free (tokens);
    }

    if ((use_example)
    &&  (strlen (example) > 1))
      {
        c = example;

        if (isupper (*c))
            while ((isupper (*c) || !isalpha (*c)) && (*c))
                c++;

        if (*c == 0)
            strupc (result-> s);
        else
        if (c == example + 1)
          {
            if (islower (*c))
              {
                while ((islower (*c) || !isalpha (*c)) && (*c))
                    c++;
                if (!isupper (*c))
                    strneat (result-> s);
              }
          }
        else
            if (c == example)
              {
                if (islower (*c))
                    while ((islower (*c) || !isalpha (*c)) && (*c))
                        c++;

                if (*c == 0)
                    strlwc (result-> s);
              }
      }
    return 0;
}


static void
strneat (char *text)
{
    char
        *c;

    c = text;
    while (*c)
      {
        while ((!isalpha (*c)) && (*c))
            c++;

    if (! *c)
        break;

        *c = toupper (*c);
        c++;

        while (isalpha (*c) && (*c))
          {
            *c = tolower (*c);
            c++;
          }
      }
}


static char *
strleft (char *text)
{
    int
        line_spaces,
        min_spaces,
        total_line_count,
        char_count,
        line_length,
        blank_line_count,
        new_size;
    char
        *line_start,
        *line_end,
        *new_string,
        *new_line;

    total_line_count = 0;
    blank_line_count = 0;
    char_count = 0;
    min_spaces = INT_MAX;
    line_start = text;
    while (line_start)
      {
        total_line_count++;
        line_spaces = strspn (line_start, " ");
        line_end    = strchr (line_start, '\n');
        if (line_end)
            line_length = line_end - line_start;
        else
            line_length = strlen (line_start);

        if (line_spaces < line_length)            /*  Not a blank line       */
          {
            if (line_spaces < min_spaces)
                min_spaces = line_spaces;

            if (line_end)
                char_count += line_end - line_start;
            else
                char_count += strlen (line_start);
          }
        else
            blank_line_count++;

        if (line_end)
            line_start = line_end + 1;
        else
            line_start = NULL;
      }

    /*  Total size of result string is char_count less min_spaces for each   */
    /*  non-blank line plus one character for each line.   Note that the     */
    /*  final line may be null.                                              */
    new_size = char_count
             - min_spaces * (total_line_count - blank_line_count)
             + total_line_count;
    new_string = mem_alloc (new_size);
    new_line = new_string;
    line_start = text;
    while (line_start)
      {
        line_spaces = strspn (line_start, " ");
        line_end = strchr (line_start, '\n');
        if (line_end)
            line_length = line_end - line_start;
        else
            line_length = strlen (line_start);

        if (line_spaces < line_length)            /*  Not a blank line       */
          {
            if (line_end)
              {
                strncpy (new_line, line_start + min_spaces,
                         line_end - (line_start + min_spaces) + 1);
                new_line += line_end - (line_start + min_spaces) + 1;
              }
            else
              {
                strcpy (new_line, line_start + min_spaces);
                new_line += strlen (line_start + min_spaces) + 1;
              }
          }
        else
          {
            if (line_end)
                *new_line++ = '\n';
            else
                *new_line++ = 0;
          }

        if (line_end)
            line_start = line_end + 1;
        else
            line_start = NULL;
      }
    ASSERT (new_line - new_string == new_size);
    return new_string;
}


size_t
strllen (const char *s)
{
    const char
        *c;

    c = strrchr (s, '\n');
    if (c)
        c++;
    else
        c = s;

    return strlen (c);
}


char *
concatenate_results (RESULT_NODE *r,
                     int shuffle,
                     Bool convert_indent,
                     char **error_text)
{
    RESULT_NODE
       *op;
    int
        indent = 0,
        shuffle_cnt,
        oplines,
        totlines,
        linewidth,
        opwidth,
        totwidth,
        totlinelength,
        runlength,
        line;
    char
      **blk,
       *txt,
       *rc;

    if (error_text)
        *error_text = NULL;             /*  No errors yet  */

    totlines      = 0;
    totwidth      = 0;
    totlinelength = 0;
    shuffle_cnt   = 0;
    op = first_operand (r, & indent);
    if (convert_indent)
        r-> indent = 0;                 /*  Coz indent has been migrated.    */
    else
        indent = 0;                     /*  Forget about initial indentation */
    while (op)
      {
        op-> indent = indent;           /*  Copy indent value to operand.    */
        /*  Perform shuffle  */
        if (shuffle > 0
        &&  op-> indent >= shuffle)
          {
            if (op-> indent - shuffle_cnt >= shuffle)
                op-> indent -= shuffle_cnt;
            else
                op-> indent  = shuffle;
          }

        /*  Calculate length & width of operand for shuffle.  */
        if (op-> value. type == TYPE_BLOCK)
          {
            /*  Catch undefined operand.  */
            if (! op-> value. b)
                return NULL;

            oplines  = 0;
            opwidth  = 0;
            blk      = op-> value. b;
            while (*blk)
              {
                oplines++;
                linewidth = strlen (*blk);
                if (linewidth > opwidth)
                    opwidth = linewidth;
                blk++;
              }
            opwidth       += op-> indent;
            totwidth      += opwidth;
            totlinelength += opwidth;
          }
        else
          {
            if (op-> value. type != TYPE_UNDEFINED)
                string_value (& op-> value);

            /*  Catch undefined operand.  */
            if (op-> value. type == TYPE_UNDEFINED
            || (! op-> value. s))
              {
                /*  Pass node result coz that's where culprit is stored  */
                result_node_error ("Undefined expression", r, error_text);
                return NULL;
              }

            string_value (& op-> value);
            oplines = 1;
            opwidth = op-> indent + strllen (op-> value. s);
            if (strchr (op-> value. s, '\n'))
                totwidth  = opwidth;
            else
                totwidth += opwidth;
            totlinelength += op-> indent + strlen (op-> value. s);
          }
        if (oplines > totlines)
            totlines = oplines;

        if (op-> value. s && strchr (op-> value. s, '\n'))
            shuffle_cnt = 0;
        else
        if (op-> script_node-> extend)
            shuffle_cnt  = totwidth;
        else
            shuffle_cnt += opwidth - (op-> script_node-> width
                                   +  indent);

        op = next_operand (op, & indent);
      }

    /*  Now build the result  */
    rc = mem_alloc (totlines * totlinelength + 1);
    memset (rc, ' ',   totlines * totlinelength + 1);

    op = first_operand (r, NULL);
    runlength = 0;
    while (op)
      {
        line = 0;
        if (op-> value. type == TYPE_BLOCK)
          {
            opwidth  = 0;
            blk = op-> value. b;
            while (*blk)
              {
                linewidth = strlen (*blk);
                if (linewidth > opwidth)
                    opwidth = linewidth;

                txt = rc
                    + line * totlinelength + runlength
                    + op-> indent;
                memcpy (txt, *blk, strlen (*blk));
                blk++;
                line++;
              }
            opwidth   += op-> indent;
            runlength += opwidth;
          }
        else
          {
            for (line = 0; line < totlines; line++)
              {
                txt = rc
                    + line * totlinelength + runlength
                    + op-> indent;
                memcpy (txt, op-> value. s, strlen (op-> value. s));
              }
            runlength += op-> indent + strlen (op-> value. s);
          }
        op = next_operand (op, NULL);
      }

    rc [totlines * totlinelength] = 0;

    /*  JS Blunder check  */
    ASSERT (runlength == totlinelength);

    return rc;
}


static RESULT_NODE *
first_operand (RESULT_NODE *r, int *indent)
{
    if (! r)
        return NULL;

    if (indent)
        *indent = r-> indent;

    while ((! r-> script_node)
       ||   r-> script_node-> type == GG_OPERATOR)
        r = r-> op1;
    return r;
}


static RESULT_NODE *
next_operand (RESULT_NODE *r, int *indent)
{
    RESULT_NODE
        *p;

    p = r-> parent;
    if (!p)
        return NULL;
    
    while (p-> parent
       && ((! p-> script_node)
       ||  (  p-> script_node-> type     == GG_OPERATOR
       &&     p-> script_node-> operator == OP_UNDEFINED))
       &&  r == p-> op2)
      {
        r = p;
        p = r-> parent;
      }

    if (((! p-> script_node)
    ||  (  p-> script_node-> type     == GG_OPERATOR
    &&     p-> script_node-> operator == OP_UNDEFINED))
    &&  r != p-> op2)
        return first_operand (p-> op2, indent);
    else
        return NULL;
}

void
destroy_caches (void)
{
    RESULT_NODE
        *node;

    while (result_node_cache)
      {
        node = result_node_cache;
        result_node_cache = result_node_cache-> next;
        mem_free (node);
      }
}
