/*===========================================================================*
 *                                                                           *
 *  sflstr.c - String manipulation functions                                 *
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
#include "sfllist.h"                    /*  Linked-list functions            */
#include "sflmem.h"                     /*  Memory handling functions        */
#include "sflsymb.h"                    /*  Symbol-table functions           */
#include "sfltok.h"                     /*  Token-handling functions         */
#include "sflstr.h"                     /*  Prototypes for functions         */

/*  Globals                                                                  */
char *xstrcpy_file = "";                /*  Source file calling xstrcpy      */
long  xstrcpy_line = 0;                 /*  Source line for call             */

/*  ---------------------------------------------------------------------[<]-
    Function: strdupl

    Synopsis:
    Makes a duplicate of string, obtaining space with a call to malloc().
    The allocated space is strlen (string) + 1 bytes long.  The caller is
    responsible for freeing the space allocated by strdup when it is no
    longer needed.  Returns a pointer to the allocated string, which holds
    a copy of the parameter string.  Returns NULL if there was insufficient
    heap storage available to allocate the string, or if the original string
    was itself NULL.

    Use this function in place of the non-portable strdup() function.  You
    may also want to use the more robust _mem_strdup () function.
    ---------------------------------------------------------------------[>]-*/

char *
strdupl (
    const char *string)
{
    char *copy;
    size_t length;

    if (string)
      {
        length = strlen (string) + 1;
        copy = malloc (length);
        if (copy)
            strncpy (copy, string, length);
      }
    else
        copy = NULL;

    return (copy);
}

/*  ---------------------------------------------------------------------[<]-
    Function: strndupl

    Synopsis:
    Makes a duplicate of the first length bytes of string, obtaining space 
    with a call to malloc().  The allocated space is length + 1 bytes.  
    The caller is responsible for freeing the space allocated by strndupl 
    when it is no longer needed.  Returns a pointer to the allocated string, 
    which holds a copy of up to the first length bytes of string plus a NUL 
    terminator.  Returns NULL if there was insufficient heap storage 
    available to allocate the string, or if the original string was 
    itself NULL.

    You may also want to use the more robust _mem_strndup () function.
    ---------------------------------------------------------------------[>]-*/

char *
strndupl (
    const char  *string,
    const size_t length)
{
    char *copy;

    if (string)
      {
        copy = malloc (length + 1);
        if (copy)
          {
        if (length)
                strncpy (copy, string, (length + 1));

        copy [length] = '\0';
          }
      }
    else
        copy = NULL;

    return (copy);
}

/*  ---------------------------------------------------------------------[<]-
    Function: strfree

    Synopsis:
    Releases memory occupied by a string.  Call this function only when you
    previously allocated the string using malloc or strdupl().  You pass the
    address of a char pointer; this function sets the pointer to NULL.  This
    is a safety measure meant to make it safe to try to free non-allocated
    strings.  In your code, initialise all such pointers to NULL.  Returns
    the address of the modified pointer.
    ---------------------------------------------------------------------[>]-*/

char **
strfree (
    char **string)
{
    ASSERT (string);
    if (string && *string)
      {
        free (*string);
        *string = NULL;
      }
    return (string);
}


/*  ---------------------------------------------------------------------[<]-
    Function: strskp

    Synopsis: Skips leading spaces in string, and returns a pointer to the
    first non-blank character.  If this is a null, the end of the string
    was reached.
    ---------------------------------------------------------------------[>]-*/

char *
strskp (
    const char *string)
{
    char
        *skip = (char *) string;

    ASSERT (string);
    while (*skip == ' ')
        skip++;
    return (skip);
}


/*  ---------------------------------------------------------------------[<]-
    Function: strcset

    Synopsis: Sets all characters in string up to but not including the
    final null character to ch.  Returns string.  Use this function instead
    of the equivalent but non-portable strset() function.
    ---------------------------------------------------------------------[>]-*/

char *
strcset (
    char *string,
    char ch)
{
    char *scan;

    ASSERT (string);
    scan = string;
    while (*scan)
        *scan++ = ch;
    return (string);
}


/*  ---------------------------------------------------------------------[<]-
    Function: strpad

    Synopsis: Returns string of length characters, padding with ch or
    truncating if necessary.  String must be at least length + 1 long.
    ---------------------------------------------------------------------[>]-*/

char *
strpad (
    char *string,
    char ch,
    int  length)
{
    int cursize;

    ASSERT (string);
    cursize = strlen (string);          /*  Get current length of string     */
    while (cursize < length)            /*  Pad until at desired length      */
        string [cursize++] = ch;

    string [cursize++] = '\0';          /*  Add terminating null             */
    return (string);                    /*    and return to caller           */
}


/*  ---------------------------------------------------------------------[<]-
    Function: strlwc

    Synopsis: Converts all alphabetic characters in string to lowercase,
    stopping at the final null character.  Returns string.  If string is
    null, returns null.  We do not call this function strlwr because that
    is already provided by some systems (but not by ANSI C).
    ---------------------------------------------------------------------[>]-*/

char *
strlwc (
    char *string)
{
    char *scan;

    if (string)
      {
        scan = string;
        while (*scan)
          {
            *scan = (char) tolower (*scan);
            scan++;
          }
      }
    return (string);
}


/*  ---------------------------------------------------------------------[<]-
    Function: strupc

    Synopsis: Converts all alphabetic characters in string to uppercase,
    stopping at the final null character.  Returns string.  If string is
    null, returns null.  We do not call this function strupr because that
    is already provided by some systems (but not by ANSI C).
    ---------------------------------------------------------------------[>]-*/

char *
strupc (
    char *string)
{
    char *scan;

    if (string)
      {
        scan = string;
        while (*scan)
          {
            *scan = (char) toupper (*scan);
            scan++;
          }
      }
    return (string);
}


/*  ---------------------------------------------------------------------[<]-
    Function: strcrop

    Synopsis: Drops trailing whitespace from string by truncating string
    to the last non-whitespace character.  Returns string.  If string is
    null, returns null.
    ---------------------------------------------------------------------[>]-*/

char *
strcrop (
    char *string)
{
    char *last;

    if (string)
      {
        last = string + strlen (string);
        while (last > string)
          {
            if (!isspace (*(last - 1)))
                break;
            last--;
          }
        *last = 0;
      }
    return (string);
}


/*  ---------------------------------------------------------------------[<]-
    Function: stropen

    Synopsis: Inserts a character at string, and places a blank in the gap.
    If align is TRUE, makes room by reducing the size of the next gap of 2
    or more spaces.  If align is FALSE, extends the size of the string.
    Returns string.
    ---------------------------------------------------------------------[>]-*/

char *
stropen (
    char *string,
    Bool  align)
{
    char *gap;
    int  length;

    ASSERT (string);
    length = strlen (string) + 1;       /*  By default, move string + NULL   */
    if (align)                          /*  If align is TRUE, find gap       */
      {
        gap = strstr (string, "  ");
        if (gap)
            length = (int) (gap - string);
      }
    memmove (string + 1, string, length);
    string [0] = ' ';                   /*  Stick a space into string        */
    return (string);
}


/*  ---------------------------------------------------------------------[<]-
    Function: strclose

    Synopsis: Removes the character at string, and shifts the remainder
    down by one.  If align is TRUE, only shifts up to the next gap of 2 or
    more spaces.  Returns string.
    ---------------------------------------------------------------------[>]-*/

char *
strclose (
    char *string,
    Bool align)
{
    char *gap;
    int  length;

    ASSERT (string);
    length = strlen (string);           /*  By default, move string + NULL   */
    if (align) {                        /*  If align is TRUE, find gap       */
        gap = strstr (string, "  ");
        if (gap && gap != string)
            length = (int) (gap - string);
    }
    memmove (string, string + 1, length);
    return (string);
}


/*  ---------------------------------------------------------------------[<]-
    Function: strunique

    Synopsis: Reduces chains of some character to a single instances.
    For example: replace multiple spaces by one space.  Returns string.
    ---------------------------------------------------------------------[>]-*/

char *
strunique (
    char *string,
    char  unique)
{
    char
        *from,
        *to;

    ASSERT (string);
    if (strnull (string))
        return (string);                /*  Get rid of special cases         */

    from = string + 1;
    to   = string;
    while (*from)
      {
        if (*from == unique && *to == unique)
            from++;
        else
            *++to = *from++;
      }
    *++to = '\0';
    return (string);
}


/*  ---------------------------------------------------------------------[<]-
    Function: strmatch

    Synopsis: Calculates a similarity index for the two strings.  This
    is a value from 0 to 32767 with higher values indicating a closer match.
    The two strings are compared without regard for case.  The algorithm was
    designed by Leif Svalgaard <leif@ibm.net>.
    ---------------------------------------------------------------------[>]-*/

int
strmatch (
    const char *string1,
    const char *string2)
{
    static int
        name_weight [30] = {
            20, 15, 13, 11, 10, 9, 8, 8, 7, 7, 7, 6, 6, 6, 6,
             6,  5,  5,  5,  5, 5, 4, 4, 4, 4, 4, 4, 4, 4, 4
        };
    int
        comp_index,
        name_index,
        start_of_string,
        longest_so_far,
        substring_contribution,
        substring_length,
        compare_length,
        longest_length,
        length_difference,
        name_length,
        char_index,
        similarity_index,
        similarity_weight;
    char
        cur_name_char;

    ASSERT (string1);
    ASSERT (string2);

    name_length    = strlen (string1);
    compare_length = strlen (string2);
    if (name_length > compare_length)
      {
        length_difference = name_length - compare_length;
        longest_length    = name_length;
      }
    else
      {
        length_difference = compare_length - name_length;
        longest_length    = compare_length;
      }
    if (compare_length)
      {
        similarity_weight = 0;
        substring_contribution = 0;

        for (char_index = 0; char_index < name_length; char_index++)
          {
            start_of_string = char_index;
            cur_name_char   = (char) tolower (string1 [char_index]);
            longest_so_far  = 0;
            comp_index      = 0;

            while (comp_index < compare_length)
              {
                while ((comp_index < compare_length)
                &&     (tolower (string2 [comp_index]) != cur_name_char))
                    comp_index++;

                substring_length = 0;
                name_index = start_of_string;

                while ((comp_index < compare_length)
                &&     (tolower (string2 [comp_index])
                     == tolower (string1 [name_index])))
                  {
                    if (comp_index == name_index)
                        substring_contribution++;
                    comp_index++;
                    if (name_index < name_length)
                      {
                        name_index++;
                        substring_length++;
                      }
                  }
                substring_contribution += (substring_length + 1) * 3;
                if (longest_so_far < substring_length)
                    longest_so_far = substring_length;
              }
            similarity_weight += (substring_contribution
                                  + longest_so_far + 1) * 2;
            similarity_weight /= name_length + 1;
          }
        similarity_index  = (name_length < 30? name_weight [name_length]: 3)
                          * longest_length;
        similarity_index /= 10;
        similarity_index += 2 * length_difference / longest_length;
        similarity_index  = 100 * similarity_weight / similarity_index;
      }
    else
        similarity_index = 0;

    return (similarity_index);
}


/*  ---------------------------------------------------------------------[<]-
    Function: strprefixed

    Synopsis: If string starts with specified prefix, returns TRUE.  If
    string does not start with specified prefix, returns FALSE.
    ---------------------------------------------------------------------[>]-*/

Bool
strprefixed (
    const char *string,
    const char *prefix)
{
    ASSERT (string);
    ASSERT (prefix);

    if (*string == *prefix              /*  Check that first letters match   */
    &&  strlen (string) >= strlen (prefix)
    &&  memcmp (string, prefix, strlen (prefix)) == 0)
        return (TRUE);
    else
        return (FALSE);
}


/*  ---------------------------------------------------------------------[<]-
    Function: strprefix

    Synopsis: Looks for one of the delimiter characters in the string; if
    found, returns a string that contains the text up to that delimiter.
    If not found, returns NULL.  The returned string can be zero or more
    characters long followed by a null byte.  It is allocated using the
    mem_alloc() function; you should free it using mem_free() when finished.
    ---------------------------------------------------------------------[>]-*/

char *
strprefix (
    const char *string,
    const char *delims)
{
    const char
        *nextch;
    char
        *token;
    int
        token_size;

    ASSERT (string);
    ASSERT (delims);

    for (nextch = string; *nextch; nextch++)
      {
        if (strchr (delims, *nextch))   /*  Is next character a delimiter    */
          {
            token_size = (int) (nextch - string);
            token = mem_alloc (token_size + 1);
            if (token == NULL)
                return (NULL);          /*  Not enough memory - fail         */
            memcpy (token, string, token_size);
            token [token_size] = 0;
            return (token);
          }
      }
    return (NULL);
}


/*  ---------------------------------------------------------------------[<]-
    Function: strdefix

    Synopsis: If string starts with specified prefix, returns pointer to
    character after prefix.  Null character is not considered part of the
    prefix.  If string does not start with specified prefix, returns NULL.
    ---------------------------------------------------------------------[>]-*/

char *
strdefix (
    const char *string,
    const char *prefix)
{
    ASSERT (string);
    ASSERT (prefix);

    if (strlen (string) >= strlen (prefix)
    &&  memcmp (string, prefix, strlen (prefix)) == 0)
        return ((char *) string + strlen (prefix));
    else
        return (NULL);
}


/*  ---------------------------------------------------------------------[<]-
    Function: strhash

    Synopsis:
    Calculates a 32-bit hash value for the string.  The string must end in
    a null.  To use the result as a hash key, take the modulo over the hash
    table size.  The algorithm was designed by Peter Weinberger.  This
    version was adapted from Dr Dobb's Journal April 1996 page 26.

    Examples:
    int index;
    index = (int) strhash (name) % TABLE_SIZE;
    ---------------------------------------------------------------------[>]-*/

qbyte
strhash (
    const char *string)
{
    qbyte
        high_bits,
        hash_value = 0;

    ASSERT (string);
    while (*string)
      {
        hash_value = (hash_value << 4) + *string++;
        if ((high_bits = hash_value & 0xF0000000L) != 0)
            hash_value ^= high_bits >> 24;
        hash_value &= ~high_bits;
      }
    return (hash_value);
}


/*  ---------------------------------------------------------------------[<]-
    Function: strconvch

    Synopsis: Converts all instances of one character in a string to some
    other character.  Returns string.  Does nothing if the string is NULL.
    ---------------------------------------------------------------------[>]-*/

char *
strconvch (
    char *string,
    char from,
    char to)
{
    char *scan;

    if (string)
      {
        scan = string;
        while (*scan)
          {
            if (*scan == from)
               *scan = to;
            scan++;
          }
      }
    return (string);
}


/*  ---------------------------------------------------------------------[<]-
    Function: strconvchs

    Synopsis: Converts all instances of one character in a string to a
    string of one or more characters.  Allocates a fresh string for the
    result.  Does nothing if the supplied string is null.
    ---------------------------------------------------------------------[>]-*/

char *
strconvchs (
    char *string,
    char from,
    char *to)
{
    size_t
        index,
        from_count;
    char
        *result;

    from_count = 0;
    for (index = 0; string [index]; index++)
        if (string [index] == from)
            from_count++;
            
    result = mem_alloc (strlen (string) + (strlen (to) - 1) * from_count + 1);
    index  = 0;
    while (*string)
	  {
        if (*string == from)
		  {
            strcpy (result + index, to);
            index += strlen (to);
          }
        else
            result [index++] = *string;
        string++;
      }
    result [index] = '\0';
    return (result);
}


/*  ---------------------------------------------------------------------[<]-
    Function: xstrcat

    Synopsis: Concatenates multiple strings into a single result.  Eg.
    xstrcat (buffer, "A", "B", NULL) stores "AB" in buffer.  Returns dest.
    Append the string to any existing contents of dest.
    From DDJ Nov 1992 p. 155, with adaptions.
    ---------------------------------------------------------------------[>]-*/

char *
xstrcat (
    char *dest,
    const char *src, ...)
{
    char
        *feedback = dest;
    va_list
        va;

    ASSERT (dest);
    while (*dest)                       /*  Find end of dest string          */
        dest++;

    va_start (va, src);
    while (src)
      {
        while (*src)
            *dest++ = *src++;
        src = va_arg (va, char *);
      }
    *dest = '\0';                       /*  Append a null character          */
    va_end (va);
    return (feedback);
}


/*  ---------------------------------------------------------------------[<]-
    Function: xstrcpy

    Synopsis: Concatenates multiple strings into a single result.  Eg.
    xstrcpy (buffer, "A", "B", NULL) stores "AB" in buffer.  Returns dest.
    Any existing contents of dest are cleared.  If the dest buffer is NULL,
    allocates a new buffer with the required length and returns that.  The
    buffer is allocated using mem_alloc(), and should eventually be freed
    using mem_free() or mem_strfree().  Returns NULL if there was too little
    memory to allocate the new string.  We can't define macros with variable
    argument lists, so we pass the file and line number through two globals,
    xstrcpy_file and xstrcpy_line, which are reset to empty values after
    each call to xstrcpy.
    ---------------------------------------------------------------------[>]-*/

char *
xstrcpy (
    char *dest,
    const char *src, ...)
{
    const char
        *src_ptr;
    va_list
        va;
    size_t
        dest_size;                      /*  Size of concatenated strings     */

    /*  Allocate new buffer if necessary                                     */
    if (dest == NULL)
      {
        va_start (va, src);             /*  Start variable args processing   */
        src_ptr   = src;
        dest_size = 1;                  /*  Allow for trailing null char     */
        while (src_ptr)
          {
            dest_size += strlen (src_ptr);
            src_ptr = va_arg (va, char *);
          }
        va_end (va);                    /*  End variable args processing     */

        /*  Allocate by going directly to mem_alloc_ function                */
        dest = mem_alloc_ (NULL, dest_size, xstrcpy_file, xstrcpy_line);
        xstrcpy_file = "";
        xstrcpy_line = 0;
        if (dest == NULL)
            return (NULL);              /*  Not enough memory                */
      }

    /*  Now copy strings into destination buffer                             */
    va_start (va, src);                 /*  Start variable args processing   */
    src_ptr  = src;
    dest [0] = '\0';
    while (src_ptr)
      {
        strcat (dest, src_ptr);
        src_ptr = va_arg (va, char *);
      }
    va_end (va);                        /*  End variable args processing     */
    return (dest);
}


/*  ---------------------------------------------------------------------[<]-
    Function: lexcmp

    Synopsis: Performs an unsigned comparison of two strings without regard
    to the case of any letters in the strings.  Returns a value that is
    <TABLE>
        <_0     if string1 is less than string2
        ==_0    if string1 is equal to string2
        >_0     if string1 is greater than string2
    </TABLE>
    ---------------------------------------------------------------------[>]-*/

int
lexcmp (
    const char *string1,
    const char *string2)
{
    int cmp;

    ASSERT (string1);
    ASSERT (string2);

    do
      {
        cmp = (byte) tolower (*string1) - (byte) tolower (*string2);
      }
    while (*string1++ && *string2++ && cmp == 0);
    return (cmp);
}


/*  ---------------------------------------------------------------------[<]-
    Function: lexncmp

    Synopsis: Performs an unsigned comparison of two strings without regard
    to the case of specified number of letters in the strings.
    Returns a value that is
    <TABLE>
        <_0     if string1 is less than string2
        ==_0    if string1 is equal to string2
        >_0     if string1 is greater than string2
    </TABLE>
    ---------------------------------------------------------------------[>]-*/

int
lexncmp (
    const char *string1,
    const char *string2,
    const int   count)
{
    int
        cmp = 0;
    char
        *end;

    ASSERT (string1);
    ASSERT (string2);

    end = (char *)string1 + count;

    do
      {
        cmp = (byte) tolower (*string1) - (byte) tolower (*string2);
      }
    while (*string1++ && *string2++ && cmp == 0 && string1 < end);

    return (cmp);
}


/*  ---------------------------------------------------------------------[<]-
    Function: lexwcmp

    Synopsis: Compares two strings ignoring case, and allowing wildcards
    in the second string (the pattern).  Two special characters are
    recognised in the pattern: '?' matches any character in the string,
    and '*' matches the remainder of the string.
    Returns a value that is:
    <TABLE>
        <_0     if string1 is less than pattern
        ==_0    if string1 is equal to pattern
        >_0     if string1 is greater than pattern
    </TABLE>
    ---------------------------------------------------------------------[>]-*/

int
lexwcmp (
    const char *string1,
    const char *pattern)
{
    int cmp = 0;

    ASSERT (string1);
    ASSERT (pattern);

    do
      {
        if (*pattern != '?' && *pattern != '*')
            cmp = (byte) tolower (*string1) - (byte) tolower (*pattern);
      }
    while (*string1++ && *pattern++ && cmp == 0 && *pattern != '*');
    return (cmp);
}



/*  ---------------------------------------------------------------------[<]-
    Function: match_pattern

    Synopsis: Compares two strings and allowing wildcards
    in the second string (the pattern).  Two special characters are
    recognised in the pattern: '?' matches any character in the string,
    and '*' matches the remainder of the string, you can use a range of
    character in pattern between bracket character like
    '[aeioy]' or '[a-z0-9]'.
    Returns a value that is TRUE if string match the pattern.
    ---------------------------------------------------------------------[>]-*/


Bool
match_pattern (
    char *string,
    char *pattern,
    Bool ignore_case)
{
    char
        string_char,
        old_pattern_char,
        pattern_char;


    FOREVER
      {
        pattern_char = *pattern++;        
        switch (pattern_char)
          {
            case '\0':
                return (*string? FALSE: TRUE);                

            case '*':
                while (*string)
                  {
                    if (match_pattern (string++, pattern, ignore_case))
                        return (TRUE);
                  }
                return (match_pattern (string++, pattern, ignore_case));

            case '?':
                if (*string++ == '\0')
                    return (FALSE);
                break;

            case '[':
                string_char = *string++;
                if (string_char == '\0')
                    return (FALSE);                
                if (ignore_case)
                    string_char  = toupper (string_char);
                old_pattern_char = '\0';

                while ((pattern_char = *pattern++) != '\0')
                  {
                    if (pattern_char == ']')
                        return (FALSE);

                    if (ignore_case)
                        pattern_char = toupper (pattern_char);

                    /* Check a range of char                                 */
                    if (pattern_char == '-')
                      {
                        pattern_char = *pattern++;
                        if (pattern_char == '\0'
                        ||  pattern_char == ']')
                            return (FALSE);

                        if (string_char >= old_pattern_char
                        &&  string_char <= pattern_char)
                            break;

                      }
                    old_pattern_char = pattern_char;
                    if (string_char == pattern_char)
                        break;
                  }
                while (pattern_char != '\0'
                &&     pattern_char != ']')
                    pattern_char = *pattern++;
                break;
            default:
                string_char = *string++;
                if (ignore_case)
                  {
                    string_char  = toupper (string_char);
                    pattern_char = toupper (pattern_char);
                  }
                if (string_char != pattern_char)
                    return (FALSE);
                break;
          }
       }
}

/*  ---------------------------------------------------------------------[<]-
    Function: soundex

    Synopsis: Calculates the SOUNDEX code for the string.  Returns the
    address of a static area that holds the code.  This area is overwritten
    by each call to the soundex function.  The SOUNDEX encoding converts
    letters to uppercase, and translates each letter according to this
    table: A0 B1 C2 D3 E0 F1 G2 H0 I0 J2 K2 L4 M5 N5 O0 P1 Q2 R6 S2 T3
    U0 V1 W0 X2 Y0 Z2.  Non-letters are ignored, letters that translate
    to zero, and multiple occurences of the same value are also ignored.
    This function always returns a 4-letter encoding: the first letter of
    the string followed by the first three significant digits.

    Examples:
    printf ("Soundex of %s = %s\n", argv [1], soundex (argv [1]));
    ---------------------------------------------------------------------[>]-*/

char *
soundex (
    const char *string)
{
    ASSERT (string);
    return (soundexn (string, 4, FALSE));
}


/*  ---------------------------------------------------------------------[<]-
    Function: soundexn

    Synopsis: Calculates the SOUNDEX code for the string.  Returns the
    address of a static area that holds the code.  This area is overwritten
    by each call to the soundex function.  The SOUNDEX encoding converts
    letters to uppercase, and translates each letter according to this
    table: A0 B1 C2 D3 E0 F1 G2 H0 I0 J2 K2 L4 M5 N5 O0 P1 Q2 R6 S2 T3
    U0 V1 W0 X2 Y0 Z2.  Non-letters are ignored, letters that translate
    to zero, and multiple occurences of the same value are also ignored.
    This function returns a N-letter encoding: the first letter of the
    string followed by the first N-1 significant digits.  N may not be
    greater than SOUNDEX_MAX (100).  If the fold argument is true, includes
    the first letter in the calculated digits, else starts with the second
    letter.
    ---------------------------------------------------------------------[>]-*/

char *
soundexn (
    const char *string, int size, Bool fold)
{
#   define SOUNDEX_MAX  100
#   define SOUNDEX_TABLE                   \
        "00000000000000000000000000000000" \
        "00000000000000000000000000000000" \
        "00123012002245501262301020200000" \
        "00123012002245501262301020200000" \
        "00000000000000000000000000000000" \
        "00000000000000000000000000000000" \
        "00000000000000000000000000000000" \
        "00000000000000000000000000000000"

    static char
       *soundex_table = SOUNDEX_TABLE,  /*  ASCII-SOUNDEX conversion         */
        soundex_code [SOUNDEX_MAX + 1]; /*  Letter + 3 digits                */
    int
        index;
    char
        last_value = 0,
        this_value;

    ASSERT (string);
    ASSERT (size > 0 && size <= SOUNDEX_MAX);

    /*  Initialise the soundex code to a string of zeroes                    */
    memset (soundex_code, '0', size);
    soundex_code [size] = '\0';

    soundex_code [0] = toupper (*string);
    last_value = fold? 0: soundex_table [(byte) *string];
    string++;
    index = 1;                          /*  Store results at [index]         */
    while (*string && isalpha (*string))
      {
        /*  Letters H and W don't act as separator between letters having
         *  same code value                                                  */
        this_value = soundex_table [(byte) *string];
        while ((   *string == 'H' 
                || *string == 'W'
                || *string == 'h'
                || *string == 'w'
               ) && *(string + 1))
             this_value = soundex_table [(byte) *++string];

        if (this_value == last_value    /*  Ignore doubles                   */
        ||  this_value == '0')          /*    and 'quiet' letters            */
          {
            string++;
            last_value = this_value;
            continue;
          }
        string++;
        last_value = this_value;
        soundex_code [index++] = this_value;
        if (index == size)              /*  Up to size result characters     */
            break;
      }
    return (soundex_code);
}


/*  ---------------------------------------------------------------------[<]-
    Function: strt2descr

    Synopsis: Converts a table of strings into a single block of memory.
    The input table consists of an array of null-terminated strings,
    terminated in a null pointer.  Returns the address of a DESCR block
    defined as: "typedef struct {size_t size; byte *data} DESCR;".
    Allocates the descriptor block using the mem_alloc() function; you must
    free it using mem_free() when you are finished with it. The strings are
    packed into the descriptor data field, each terminated by a null byte.
    The final string is terminated by two nulls.  The total size of the
    descriptor is descr->size + sizeof (DESCR).  Note that if you omit the
    last null pointer in the input table, you will probably get an addressing
    error.  Returns NULL if there was insufficient memory to allocate the
    descriptor block.
    ---------------------------------------------------------------------[>]-*/

DESCR *
strt2descr (
    char **table)
{
    DESCR
        *descr;                         /*  Allocated descriptor             */
    char
        *descr_ptr;                     /*  Pointer into block               */
    size_t
        descr_size;                     /*  Size of table                    */
    int
        string_nbr;                     /*  Index into string table          */

    ASSERT (table);

    /*  Calculate the size of the descriptor                                 */
    descr_size = 1;                     /*  Allow for final null byte        */
    for (string_nbr = 0; table [string_nbr]; string_nbr++)
        descr_size += strlen (table [string_nbr]) + 1;

    /*  Allocate a descriptor and fill it with the strings                   */
    descr = mem_alloc (descr_size + sizeof (DESCR));
    if (descr)
      {
        descr->size = descr_size;
        descr->data = (byte *) descr + sizeof (DESCR);
        descr_ptr    = (char *) descr->data;

        for (string_nbr = 0; table [string_nbr]; string_nbr++)
          {
            size_t descr_len = strlen (table [string_nbr]) + 1;
            strncpy (descr_ptr, table [string_nbr], descr_len);
            descr_ptr += descr_len;
          }
        *descr_ptr = '\0';              /*  Add a null string                */
      }
    return (descr);
}


/*  ---------------------------------------------------------------------[<]-
    Function: descr2strt

    Synopsis: Takes a descriptor prepared by strt2descr() and returns an
    array of strings pointers, terminated in a null pointer.  The array is
    allocated using the mem_alloc() function.  Each string is individually
    allocated.  Thus, to free the string table you must call mem_free() for
    each entry in the table, except the last one, and then for the table.
    You can also call strtfree() to destroy the table in a single operation.
    Returns NULL if there was insufficient memory to allocate the table of
    strings.
    ---------------------------------------------------------------------[>]-*/

char **
descr2strt (
    const DESCR *descr)
{
    char
        **table;
    int
        string_count,
        string_nbr;                     /*  Index into string table          */
    char
        *descr_ptr;                     /*  Pointer into block               */

    ASSERT (descr);

    /*  Count the number of strings in the table                             */
    descr_ptr = (char *) descr->data;
    string_count = 0;
    while (*descr_ptr)                  /*  Loop until we hit null string    */
      {
        string_count++;
        descr_ptr += strlen (descr_ptr) + 1;
      }

    /*  Allocate a table and fill it with the strings                        */
    table = mem_alloc ((string_count + 1) * sizeof (char *));
    if (table)
      {
        descr_ptr = (char *) descr->data;
        for (string_nbr = 0; string_nbr < string_count; string_nbr++)
          {
            table [string_nbr] = mem_strdup (descr_ptr);
            descr_ptr += strlen (descr_ptr) + 1;
          }
        table [string_count] = NULL;    /*  Store final null pointer         */
      }
    return (table);
}


/*  ---------------------------------------------------------------------[<]-
    Function: strtfree

    Synopsis: Releases a table of strings as created by descr2strt() or a
    similar function.  If the argument is null, does nothing.
    ---------------------------------------------------------------------[>]-*/

void
strtfree (
    char **table)
{
    int
        string_nbr;                     /*  Index into string array          */

    if (table)
      {
        for (string_nbr = 0; table [string_nbr]; string_nbr++)
            mem_free (table [string_nbr]);
        mem_free (table);
      }
}

/*  ---------------------------------------------------------------------[<]-
    Function: strcntch

    Synopsis: Returns number of instances of a character in a string.
    ---------------------------------------------------------------------[>]-*/

int
strcntch (const char *string, char value)
{
    int
        count = 0;

    ASSERT (string);

    while (*string)
        if (*string++ == value)
            count++;

    return (count);
}


/*  ---------------------------------------------------------------------[<]-
    Function: strlookup

    Synopsis: Searches the specified lookup table, defined as an array of
    LOOKUP items, for the specified string key, and returns a lookup value.
    You are REQUIRED to terminate the table with a null key: if the key is
    not found in the table, returns the value for the last, null key.
    ---------------------------------------------------------------------[>]-*/

int
strlookup (const LOOKUP *lookup, const char *key)
{
    int
        index;

    ASSERT (lookup);
    ASSERT (key);

    for (index = 0; lookup [index].key; index++)
        if (streq (lookup [index].key, key))
            break;
    
    return (lookup [index].value);
}

/*  ---------------------------------------------------------------------[<]-
    Function: strjustify

    Synopsis: Justifies a string to fit within lines of the specified width.
    Allocates a fresh block of memory to contain the newly justified string.
    If the width is zero, will reformat the string into a single line.
    ---------------------------------------------------------------------[>]-*/
char *
strjustify (const char *source, size_t width)
{
    size_t
        token_len;                      /*  Size of current token            */
    char
        **tokens,                       /*  String broken into words         */
        *token,                         /*  Current token                    */
        *buffer,                        /*  Target multiline buffer          */
        *bufptr;                        /*  Next position in buffer          */
    int
        cur_width,                      /*  Current line width incl. prefix  */
        token_nbr;                      /*  Token number, 0..n               */

    ASSERT (source);
    if (source == NULL)
        return NULL;
    
    tokens = tok_split (source);
    buffer = mem_alloc (tok_text_size (tokens) + 1);
    cur_width = 0;
    bufptr = buffer;
    for (token_nbr = 0; tokens [token_nbr]; token_nbr++)
      {
        token = tokens [token_nbr];
        token_len = strlen (token);

        if (token_nbr > 0)
          {
            /*  Decide if next token will fit on line or not                 */
            if (width > 0 && token_len + cur_width + 1 > width)
              {
                *bufptr++ = '\n';       /*  Start new line                   */
                cur_width = 0;
              }
            /*  Otherwise append a space                                     */
            else
              {
                *bufptr++ = ' ';
                cur_width++;
              }
          }
        /*  Now append token to line                                         */
        memcpy (bufptr, token, token_len);
        bufptr    += token_len;
        cur_width += token_len;
      }
    *bufptr = '\0';                     /*  Terminate the last line          */
    tok_free (tokens);
    return (buffer);
}

/*  ---------------------------------------------------------------------[<]-
    Function: strprefixlines

    Synopsis: Inserts a prefix at the start of each line in a string.  Alocates
    a fresh block for the result.
    ---------------------------------------------------------------------[>]-*/

char *
strprefixlines (const char *source, const char *prefix)
{
    size_t
        prefix_len,                     /*  Size of prefix string            */
        line_len;                       /*  Length of line in string         */
    int
        lines = 1;
    char
        *buffer,                        /*  Target buffer                    */
        *bufptr;                        /*  Next position in buffer          */
    const char
        *sourceptr,                     /*  Pointer to source line           */
        *nextptr;                       /*  Pointer to next source line      */

    ASSERT (source);

    /*  For consistence, work if prefix is NULL.                             */
    if (prefix == NULL)
        return mem_strdup (source);

    prefix_len = strlen (prefix);

    sourceptr = source - 1;
    while (sourceptr)
      {
        sourceptr = strchr (sourceptr + 1, '\n');
        lines++;
      }

    buffer = mem_alloc (strlen (source) + (strlen (prefix) * lines) + 1);
    bufptr = buffer;
    nextptr = source;
    while (nextptr)
      {
        sourceptr = nextptr;
        nextptr   = strchr (sourceptr, '\n');
        
        strcpy  (bufptr, prefix);
        bufptr += prefix_len;
        if (nextptr)
          {
            nextptr++;
            line_len = nextptr - sourceptr;
            memcpy (bufptr, sourceptr, line_len);
            bufptr += line_len;
          }
        else
            strcpy  (bufptr, sourceptr);
      }
    return (buffer);
}

/*  ---------------------------------------------------------------------[<]-
    Function: strreformat

    Synopsis: Reformats a string to fit within lines of the specified width.
    Prefixes each line with some optional text (included in the width).
    Allocates a fresh block of memory to contain the newly formatted line.
    If the width is zero, will reformat the string into a single line.
    ---------------------------------------------------------------------[>]-*/

char *
strreformat (const char *source, size_t width, const char *prefix)
{
    char
        *justify,
        *result;

    justify = strjustify (source, width - (prefix ? strlen (prefix) : 0));
    result  = strprefixlines (justify, prefix);
    mem_free (justify);
    return result;
}


/*  ---------------------------------------------------------------------[<]-
    Function: replacechrswith

    Synopsis: Subsitutes known char(s)in a string with another. Returns
    pointer to head of the buffer.
    Submitted by Scott Beasley <jscottb@infoave.com>
    ---------------------------------------------------------------------[>]-*/

char *
replacechrswith (
    char *strbuf,
    char *chrstorm,
    char chartorlcwith)
{
   char *offset;

   ASSERT (strbuf);
   ASSERT (chrstorm);

   offset = (char *)NULL;

   while (*strbuf)
      {
         offset = strpbrk (strbuf, chrstorm);
         if (offset)
           {
             *(offset)= chartorlcwith;
           }

         else
             break;
      }

   return strbuf;
}

/*  ---------------------------------------------------------------------[<]-
    Function: insertstring

    Synopsis: Inserts a string into another string.  Returns a pointer
    to head of the buffer.
    Submitted by Scott Beasley <jscottb@infoave.com>
    ---------------------------------------------------------------------[>]-*/

char *
insertstring (
    char *strbuf,
    char *chrstoins,
    int pos)
{
   ASSERT (strbuf);
   ASSERT (chrstoins);

   memmove (((strbuf + pos) + strlen (chrstoins)),
            (strbuf + pos), (strlen ((strbuf + pos)) + 1));
   memcpy ((strbuf + pos), chrstoins, strlen (chrstoins));

   return strbuf;
}

/*  ---------------------------------------------------------------------[<]-
    Function: insertchar

    Synopsis: Inserts a char into a string.  Returns a pointer to head of
    the buffer.  Submitted by Scott Beasley <jscottb@infoave.com>
    ---------------------------------------------------------------------[>]-*/

char *
insertchar (
    char *strbuf,
    char chrtoins,
    int pos)
{
   ASSERT (strbuf);

   memmove ((strbuf + pos)+ 1, (strbuf + pos), strlen ((strbuf + pos))+ 1);
   *(strbuf + pos)= chrtoins;

   return strbuf;
}

/*  ---------------------------------------------------------------------[<]-
    Function: leftfill

    Synopsis: Pads a string to the left, to a know length, with the
    given char value. Returns a pointer to head of the buffer.
    Submitted by Scott Beasley <jscottb@infoave.com>
    ---------------------------------------------------------------------[>]-*/

char *
leftfill (
    char *strbuf,
    char chrtofill,
    unsigned len)
{
   ASSERT (strbuf);

   while (strlen (strbuf)< len)
     {
       insertchar (strbuf, chrtofill, 0);
     }

   return strbuf;
}

/*  ---------------------------------------------------------------------[<]-
    Function: rightfill

    Synopsis: Pads a string to the right, to a known length, with the
    given char value. Returns a pointer to head of the buffer.
    Submitted by Scott Beasley <jscottb@infoave.com>
    ---------------------------------------------------------------------[>]-*/

char *
rightfill (
    char *strbuf,
    char chrtofill,
    unsigned len)
{
   ASSERT (strbuf);

   while (strlen (strbuf)< len)
     {
       insertchar (strbuf, chrtofill, strlen (strbuf));
     }

   return strbuf;
}


/*  ---------------------------------------------------------------------[<]-
    Function: searchreplace

    Synopsis: A case insensitive search and replace. Searches for all
    occurances of a string, and replaces it with another string.
    Returns a pointer
    to head of the buffer.  Submitted by Scott Beasley <jscottb@infoave.com>

    Revision: totally rewritten by SM to improve performance (2001/06/22)
    ---------------------------------------------------------------------[>]-*/
char *
searchreplace (
    char *buf,
    char *to_fnd,
    char *to_ins,
    size_t max_length)
{
    int
        delta,
        count = 0;
    char
        *replace,
        *unchanged,
        *found,
        *strbase;
    size_t
        buf_len,
        unchanged_len,
        fnd_len,
        ins_len;
    typedef struct _FINDLIST FINDLIST;
    struct _FINDLIST {
        struct _FINDLIST
            *next,                     /*  Next attr in list                */
            *prev;                     /*  Previous attr in list            */
        char
            *str;                      /*  Filename                         */
    };
    FINDLIST 
        *list = NULL, 
        *node = NULL;

    ASSERT (buf);
    ASSERT (to_fnd);
    ASSERT (to_ins);

    buf_len = strlen (buf);
    fnd_len = strlen (to_fnd);
    ins_len = strlen (to_ins);

    list_create (list, sizeof (FINDLIST));

    strbase = buf;
    while (*strbase)
      {
        found = stricstr (strbase, to_fnd);
        if (!found)
            break;

        /* we save pointer to this instance that will be replaced */
        node = mem_alloc (sizeof (FINDLIST));
        if (!node)
          {
            ASSERT (FALSE);     /* XXX TODO: what if malloc fails */
            break;
          }

        list_reset (node);
        node->str = found;
        list_relink_after (list, node);

        count++;
        strbase = found + fnd_len;
    }

    if (count > 0)
	  {
        delta = ins_len - fnd_len;
        if (delta == 0)
		  {
            /* easier case: we can just replace to_fnd by to_ins             */
            FORLIST (node, *list)
                strncpy (node->str, to_ins, ins_len);
          }
        else
        if (delta > 0)
		  {
            /* buf length will increase                                      */
            for (node = list->prev; node != list; node = node->prev)
			  {
                unchanged = node->str + fnd_len;
                unchanged_len = (node->next == list)
                              ? &strterm (buf) - unchanged  + 1
                              : node->next->str - unchanged;
                ASSERT (count);
                count--;
                if (buf_len + delta < max_length)
				  {
                    replace = node->str + (count * delta);
                    memmove (replace + ins_len, unchanged, unchanged_len);
                    strncpy (replace, to_ins, ins_len);
                    buf_len += delta;
                  }
              }
          }
        else {
            /* buf length will decrease                                      */
            replace = list->next->str;
            FORLIST (node, *list) 
			  {
                unchanged = node->str + fnd_len;
                unchanged_len = (node->next == list)
                              ? &strterm (buf) - unchanged + 1
                              : node->next->str - unchanged;
                strncpy (replace, to_ins, ins_len);
                memmove (replace + ins_len, unchanged, unchanged_len);
                replace += ins_len + unchanged_len;
                buf_len -= delta;
              }
          }
      }
    /* otherwise (found_count == 0), there is nothing to do */
    
    list_destroy (list);
    mem_free (list);

    return buf;
}


/*  ---------------------------------------------------------------------[<]-
    Function: deletestring

    Synopsis: Deletes all occurances of one string, in another string.
    Returns a pointer to head of the buffer.
    Submitted by Scott Beasley <jscottb@infoave.com>
    ---------------------------------------------------------------------[>]-*/

char *
deletestring (
    char *strbuf,
    char *strtodel,
    int ignorecase)
{
   char *offset;

   ASSERT (strbuf);
   ASSERT (strtodel);

   offset = (char *)NULL;

   while (*strbuf)
     {
        if (!ignorecase)
            offset = stricstr (strbuf, strtodel);
        else
            offset = strstr (strbuf, strtodel);
        if (offset)
          {
            strcpy (offset, (offset + strlen (strtodel)));    /* NO OVERRUN */
          }
        else
            break;
     }

   return strbuf;
}

/*  ---------------------------------------------------------------------[<]-
    Function: getstrfld

    Synopsis: Gets a sub-string from a formated string. nice strtok
    replacement.

    usage:
      char strarray[] = { "123,456,789,abc" };
      char strretbuff[4];
      getstrfld (strarray, 2, 0, ",", strretbuff);

    This would return the string "789" and place it also in strretbuff.
    Returns a NULL if fldno is out of range, else returns a pointer to
    head of the buffer.  Submitted by Scott Beasley <jscottb@infoave.com>
    ---------------------------------------------------------------------[>]-*/

char *
getstrfld (
    char *strbuf,
    int fldno,
    int ofset,
    char *sep,
    char *retstr)
{
   char *offset, *strptr;
   int curfld;

   ASSERT (strbuf);
   ASSERT (sep);
   ASSERT (retstr);

   offset = strptr = (char *)NULL;
   curfld = 0;

   strbuf += ofset;

   while (*strbuf)
     {
       strptr = !offset ? strbuf : offset;
       offset = strpbrk ((!offset ? strbuf : offset), sep);

       if (offset)
          offset++;
       else if (curfld != fldno)
         {
           *retstr = 0;
           break;
         }

       if (curfld == fldno)
         {
           strncpy (retstr, strptr,
              (int)(!offset ? strlen (strptr)+ 1 :
              (int)(offset - strptr)));
           if (offset)
              retstr[offset - strptr - 1] = 0;

           break;
         }
       curfld++;
     }
   return retstr;
}

/*  ---------------------------------------------------------------------[<]-
    Function: setstrfld

    Synopsis: Inserts a string into a fomated string.

    usage:
       char strsrray[26] = { "this is a test." };
       setstrfld (strsrray, 2, 0, " ", "big ");

       result: this is a big test.

    Does nothing if fldno is out of range, else returns pointer to head
    of the buffer.  Returns a pointer to head of the buffer.
    Submitted by Scott Beasley <jscottb@infoave.com>
    ---------------------------------------------------------------------[>]-*/

char *
setstrfld (
    char *strbuf,
    int fldno,
    int ofset,
    char *sep,
    char *strtoins)
{
   char *offset, *strptr, *strhead;
   int curfld;

   ASSERT (strbuf);
   ASSERT (sep);
   ASSERT (strtoins);

   offset = strptr = (char *)NULL;
   curfld = 0;

   strhead = strbuf;
   strbuf += ofset;

   while (*strbuf)
     {
       strptr = !offset ? strbuf : offset;
       offset = strpbrk ((!offset ? strbuf : offset), sep);

       if (offset)
          offset++;

       if (curfld == fldno)
          {
            insertstring (strptr, strtoins,
               (int)(!offset ? strlen (strptr):
               (int)(offset - strptr)));
            break;
          }
       curfld++;
     }

   return strhead;
}

/*  ---------------------------------------------------------------------[<]-
    Function: getstrfldlen

    Synopsis: Get the length of as a field in a string.  Used mainly
    for getting the len to malloc mem to call getstrfld with.  Returns
    the length of the field.
    Submitted by Scott Beasley <jscottb@infoave.com>
    ---------------------------------------------------------------------[>]-*/

int
getstrfldlen (
    char *strbuf,
    int fldno,
    int ofset,
    char *sep)
{
   char *offset, *strptr;
   int curfld, retlen = 0;

   ASSERT (strbuf);
   ASSERT (sep);

   offset = strptr = (char *)NULL;
   curfld = 0;

   strbuf += ofset;

   while (*strbuf)
     {
       strptr = !offset ? strbuf : offset;
       offset = strpbrk ((!offset ? strbuf : offset), sep);

       if (offset)
          offset++;
       else if (curfld != fldno)
         {
           retlen = 0;
           break;
         }
       if (curfld == fldno)
         {
           retlen = (int)(!offset ? strlen (strptr) + 1 :
                    (int)(offset - strptr));
           break;
         }
       curfld++;
     }
   return retlen;
}


/*  ---------------------------------------------------------------------[<]-
    Function: stringreplace

    Synopsis:
    This function searches for known strings, and replaces them with
    another string.

    Examples:
       stringreplace (strfilename, "sqv|sqr,ruv|run,h_v|h");

    This example would replace all occurences of sqv, with sqr, ruv with
    run and h_v with h. Returns pointer to head of the return buffer.
    Submitted by Scott Beasley <jscottb@infoave.com>
    ---------------------------------------------------------------------[>]-*/

char *
stringreplace (
    char *strbuf,
    char *strpattern,
    size_t max_length)
{
   int ilen, ifld = 0;
   char *strsrch, *strrpl, *strpat;

   ASSERT (strbuf);
   ASSERT (strpattern);

   if (!strpattern)
       return strbuf;

   while (1)
     {
       ilen = getstrfldlen (strpattern, ifld, 0, ",");
       if (!ilen)
           break;
       strpat = (char *)malloc (ilen + 1);
       getstrfld (strpattern, ifld, 0, ",", strpat);
       ifld++;

       ilen = getstrfldlen (strpat, 0, 0, "|");
       strsrch = (char *)malloc (ilen + 1);
       getstrfld (strpat, 0, 0, "|", strsrch);

       ilen = getstrfldlen (strpat, 1, 0, "|");
       strrpl = (char *)malloc (ilen + 1);
       getstrfld (strpat, 1, 0, "|", strrpl);

       searchreplace (strbuf, strsrch, strrpl, max_length);

       free (strsrch);
       free (strrpl);
       free (strpat);
     }

   return strbuf;
}


/*  ---------------------------------------------------------------------[<]-
    Function: stricstr

    Synopsis:
    A case insensitive strstr.  Returns a pointer to head of the str1.
    Submitted by Scott Beasley <jscottb@infoave.com>
    ---------------------------------------------------------------------[>]-*/

char *
stricstr (
    const char *str1,
    const char *str2)
{
   char 
       *res = NULL,
       *strtmp = (char *)str1;
   size_t
       length_2;

   ASSERT (str1);
   ASSERT (str2);

   length_2 = strlen (str2);

   while (*strtmp)
     {
       if (lexncmp (strtmp, str2, length_2) == 0)
         {
           res = strtmp;
           break;
         }

       strtmp++;
     }

   return res;
}


/*  ---------------------------------------------------------------------[<]-
    Function: strtempcmp

    Synopsis:
    Compares a string to a template.
    Template chars and there functions:
      # or 9 = Number.
      A or _ = Alpha.
      @      = Alphanumeric
      \char  = Literal.  Char would be the literal to use. ie: "\%" -
               looks for a % in that postion
    Returns 0 if == to the template and 1 if != to the template.
    Submitted by Scott Beasley <jscottb@infoave.com>
    ---------------------------------------------------------------------[>]-*/

int
strtempcmp (
    const char *str1,
    const char *strPat)
{
   int ires = 1;

   ASSERT (str1);
   ASSERT (strPat);

   while (*str1 && *strPat)
     {
       switch ((int)*strPat)
         {
           case '#':
           case '9':
              ires = isdigit ((int)*str1);
              break;

           case 'A':
           case '_':
              ires = isalpha ((int)*str1);
              break;

           case '@':
              ires = isalnum ((int)*str1);
              break;

           case ' ':
              ires = isspace ((int)*str1);
              break;

           case '\\':
              strPat++;
              if (*str1 != *strPat)
                 {
                   ires = 1;
                 }
              break;

           default:
              break;
         }

       if (!ires)
         {
           break;
         }

       str1++;
       strPat++;
     }

   return ires ? 0 : 1;
}


/*  ---------------------------------------------------------------------[<]-
    Function: is_text 

    Synopsis: check if character are binary or textual
    (based on characters defined by ISO 8859-1 and control characters).
    ---------------------------------------------------------------------[>]-*/

Bool
is_text (const byte characters)
{
    static char
        is_ascii [256] = {
    0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 1, 0, 1, 1, 0, 0,  /* 0x00 - 0x0F          */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  /* 0x10 - 0x1F          */
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  /* 0x20 - 0x2F          */
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  /* 0x30 - 0x3F          */
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  /* 0x40 - 0x4F          */
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  /* 0x50 - 0x5F          */
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  /* 0x60 - 0x6F          */
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,  /* 0x70 - 0x7F          */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  /* 0x80 - 0x8F          */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  /* 0x90 - 0x9F          */
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  /* 0xA0 - 0xAF          */
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  /* 0xB0 - 0xBF          */
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  /* 0xC0 - 0xCF          */
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  /* 0xD0 - 0xDF          */
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  /* 0xE0 - 0xEF          */
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1   /* 0xF0 - 0xFF          */
    };

    return ((Bool) is_ascii [characters]);
}

/* The following copyright notice applies to strlcat() and strlcpy(), which
 * were taken from the OpenBSD CVS and are used here as xstrlcat() and 
 * xstrlcpy() */

/*
 * Copyright (c) 1998 Todd C. Miller <Todd.Miller@courtesan.com>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/* From $OpenBSD: strlcat.c,v 1.11 2003/06/17 21:56:24 millert Exp $ */

/*  ---------------------------------------------------------------------[<]-
    Function: xstrlcat 

    Synopsis: Appends src to string dst of size siz (unlike strncat, siz is
    the full size of dst, not space left).  At most siz-1 characters will
    be copied.  Always NUL terminates (unless siz <= strlen(dst)).  Returns
    strlen(src) + MIN(siz, strlen(initial dst)).  If retval >= siz,
    truncation occurred.

    Use this function in place of the insecure strcat() and strncat() 
    functions.
    ---------------------------------------------------------------------[>]-*/

size_t
xstrlcat(char *dst, const char *src, size_t siz)
{
    register char *d = dst;
    register const char *s = src;
    register size_t n = siz;
    size_t dlen;

    /*  Find the end of dst and adjust bytes left but don't go past end      */
    while (n-- != 0 && *d != '\0')
        d++;
    dlen = d - dst;
    n = siz - dlen;

    if (n == 0)
        return(dlen + strlen(s));
    while (*s != '\0') {
        if (n != 1) {
            *d++ = *s;
            n--;
        }
        s++;
    }
    *d = '\0';

    return(dlen + (s - src));           /*  count does not include NUL       */
}


/* From $OpenBSD: strlcpy.c,v 1.8 2003/06/17 21:56:24 millert Exp $ */

/*  ---------------------------------------------------------------------[<]-
    Function: xstrlcpy

    Synopsis: Copy src to string dst of size siz.  At most siz-1 characters
    will be copied.  Always NUL terminates (unless siz == 0).  Returns
    strlen(src); if retval >= siz, truncation occurred.
 
    Use this function in place of the insecure strcpy() and strncpy() 
    functions.
    ---------------------------------------------------------------------[>]-*/

size_t
xstrlcpy(char *dst, const char *src, size_t siz)
{
    register char *d = dst;
    register const char *s = src;
    register size_t n = siz;

    /*  Copy as many bytes as will fit                                       */
    if (n != 0 && --n != 0)
	  {
        do
		  {
            if ((*d++ = *s++) == 0)
                break;
          } while (--n != 0);
      }

    /*  Not enough room in dst, add NUL and traverse rest of src             */
    if (n == 0)
	  {
        if (siz != 0)
            *d = '\0';                  /*  NUL-terminate dst                */
        while (*s++)
            ;
      }

    return(s - src - 1);                /*  count does not include NUL       */
}
