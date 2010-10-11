/*===========================================================================*
 *                                                                           *
 *  sflscrit.c   Parse search criteria for sflsearch                         *
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
#include "sfllist.h"                    /*  Linked-list functions            */
#include "sflmem.h"                     /*  Memory handling functions        */
#include "sflstr.h"                     /*  String-handling functions        */
#include "sflcons.h"                    /*  Console i/o functions            */
#include "sflbits.h"                    /*  Large bitstring functions        */
#include "sflprint.h"                   /*  Virtual printing functions       */
#include "sflsearch.h"                  /*  Search constants                 */

#include "sflscrit.d"                   /*  Include dialog data              */

/*  Local function prototypes                                                */

static char   toalpha                  (char ansichar);
static SCRIT *scrit_add                (char type, char *value);
static void   set_previous_token_scope (char scope);

#define SCRIT_LIMIT     10              /*  Max. scrits per token            */


/*- Global variables used in this source file only --------------------------*/

static LIST
    *feedback;                          /*  Feedback for calling program     */

static SCRIT
    *cur_scrit [SCRIT_LIMIT];           /*  Previous SCRITs for ranges       */
static char
    *string,                            /*  String to parse                  */
    decimal,                            /*  Caller's decimal point           */
    cur_char,                           /*  Current character from string    */
    comma,                              /*  Comma character                  */
    method,                             /*  Criteria method                  */
    scope,                              /*  Criteria scope                   */
    *scanptr,                           /*  Scan through string              */
    *token;                             /*  Current token being collected    */
static int
    scrit_count,                        /*  Size of cur_scrit table          */
    tok_size;                           /*  Size of token                    */


/********************************   M A I N   ********************************/

/*  ---------------------------------------------------------------------[<]-
    Function: search_build_scrit

    Synopsis: Accepts a data string and splits it into criteria that can be
    indexed.  Returns a SCRIT list containing the resulting search
    criteria.  The caller must free each criterion value and item in the
    search criterion list.  If the string contained no valid search criteria
    returns a list with no attached items.

    National character set treatment: assumes that the character set is
    ISO-8859-1 and folds 8-bit accented characters into isalpha equivalents.

    When the search option is true, takes special characters in the string
    as various search options: +abc means the criteria is mandatory; -abc
    means it is excluded.  nnnn> means greater-equal nnnn, and >nnnn means
    less than or equal to nnnn.  nnnn>mmmm defines a range between two
    criteria.  These options are set in the method and scope for each
    scrit item.  When the search option is false these special characters
    are ignored and the method and scope properties are set to spaces.
    ---------------------------------------------------------------------[>]-*/

LIST *search_build_scrit (char *p_string, char p_decimal)
{
    string  = p_string;                 /*  Local copy of parameters         */
    decimal = p_decimal;

#   include "sflscrit.i"                /*  Include dialog interpreter       */
}


/*************************   INITIALISE THE PROGRAM   ************************/

MODULE initialise_the_program (void)
{
    /*  Reset returned list                                                  */
    feedback = mem_alloc (sizeof (LIST));
    list_reset (feedback);

    /*  Token is at most the same length as the string                       */
    token   = mem_alloc (strlen (string) + 1);
    scanptr = string;

    /*  By default, numbers are handled as US/UK decimals                    */
    if (decimal == ',')
        comma = '.';
    else {
        decimal = '.';
        comma   = ',';
    }
    the_next_event = ok_event;
}


/***************************   GET NEXT CHARACTER   **************************/

MODULE get_next_character (void)
{
    cur_char = toupper (toalpha (*scanptr));
    scanptr++;

    if (isalpha (cur_char))
        the_next_event = alpha_event;
    else
    if (isdigit (cur_char))
        the_next_event = digit_event;
    else
    if (cur_char == '+')
        the_next_event = plus_event;
    else
    if (cur_char == '-')
        the_next_event = minus_event;
    else
    if (cur_char == '>')
        the_next_event = range_event;
    else
    if (cur_char == decimal)
        the_next_event = decimal_event;
    else
    if (cur_char == comma)
        the_next_event = comma_event;
    else
    if (cur_char == '\0')
        the_next_event = finished_event;
    else
        the_next_event = delimiter_event;
}

/*  toalpha -- local
    Converts the supplied ANSI character to an (approximate) alpha character
 */
static char
toalpha (char ansichar)
{
    static struct {
        int from;
        int to;
    }
    conversion [] = {
        { 192, 'A' }, 	/*  capital A grave                À  */
        { 193, 'A' }, 	/*  capital A acute                Á  */
        { 194, 'A' }, 	/*  capital A circumflex           Â  */
        { 195, 'A' }, 	/*  capital A tilde                Ã  */
        { 196, 'A' }, 	/*  capital A dieresis or umlaut   Ä  */
        { 197, 'A' }, 	/*  capital A ring                 Å  */
        { 198, 'A' }, 	/*  capital AE ligature            Æ  */
        { 199, 'C' }, 	/*  capital C cedilla              Ç  */
        { 200, 'E' }, 	/*  capital E grave                È  */
        { 201, 'E' }, 	/*  capital E acute                É  */
        { 202, 'E' }, 	/*  capital E circumflex           Ê  */
        { 203, 'E' }, 	/*  capital E dieresis or umlaut   Ë  */
        { 204, 'I' }, 	/*  capital I grave                Ì  */
        { 205, 'I' }, 	/*  capital I acute                Í  */
        { 206, 'I' }, 	/*  capital I circumflex           Î  */
        { 207, 'I' }, 	/*  capital I dieresis or umlaut   Ï  */
        { 208, 'O' }, 	/*  capital ETH                    Ð  */
        { 209, 'N' }, 	/*  capital N tilde                Ñ  */
        { 210, 'O' }, 	/*  capital O grave                Ò  */
        { 211, 'O' }, 	/*  capital O acute                Ó  */
        { 212, 'O' }, 	/*  capital O circumflex           Ô  */
        { 213, 'O' }, 	/*  capital O tilde                Õ  */
        { 214, 'O' }, 	/*  capital O dieresis or umlaut   Ö  */
        { 216, 'O' }, 	/*  capital O slash                Ø  */
        { 217, 'U' }, 	/*  capital U grave                Ù  */
        { 218, 'U' }, 	/*  capital U acute                Ú  */
        { 219, 'U' }, 	/*  capital U circumflex           Û  */
        { 220, 'U' }, 	/*  capital U dieresis or umlaut   Ü  */
        { 221, 'Y' }, 	/*  capital Y acute                Ý  */
        { 222, 'P' }, 	/*  capital THORN                  Þ  */
        { 223, 's' }, 	/*  small sharp s, sz ligature     ß  */
        { 224, 'a' }, 	/*  small a grave                  à  */
        { 225, 'a' }, 	/*  small a acute                  á  */
        { 226, 'a' }, 	/*  small a circumflex             â  */
        { 227, 'a' }, 	/*  small a tilde                  ã  */
        { 228, 'a' }, 	/*  small a dieresis or umlaut     ä  */
        { 229, 'a' }, 	/*  small a ring                   å  */
        { 230, 'a' }, 	/*  small ae ligature              æ  */
        { 231, 'c' }, 	/*  small c cedilla                ç  */
        { 232, 'e' }, 	/*  small e grave                  è  */
        { 233, 'e' }, 	/*  small e acute                  é  */
        { 234, 'e' }, 	/*  small e circumflex             ê  */
        { 235, 'e' }, 	/*  small e dieresis or umlaut     ë  */
        { 236, 'i' }, 	/*  small i grave                  ì  */
        { 237, 'i' }, 	/*  small i acute                  í  */
        { 238, 'i' }, 	/*  small i circumflex             î  */
        { 239, 'i' }, 	/*  small i dieresis or umlaut     ï  */
        { 240, 'o' }, 	/*  small eth                      ð  */
        { 241, 'n' }, 	/*  small n tilde                  ñ  */
        { 242, 'o' }, 	/*  small o grave                  ò  */
        { 243, 'o' }, 	/*  small o acute                  ó  */
        { 244, 'o' }, 	/*  small o circumflex             ô  */
        { 245, 'o' }, 	/*  small o tilde                  õ  */
        { 246, 'o' }, 	/*  small o dieresis or umlaut     ö  */
        { 248, 'o' }, 	/*  small o slash                  ø  */
        { 249, 'u' }, 	/*  small u grave                  ù  */
        { 250, 'u' }, 	/*  small u acute                  ú  */
        { 251, 'u' }, 	/*  small u circumflex             û  */
        { 252, 'u' }, 	/*  small u dieresis or umlaut     ü  */
        { 253, 'y' }, 	/*  small y acute                  ý  */
        { 254, 'p' }, 	/*  small thorn                    þ  */
        { 255, 'y' }  	/*  small y dieresis or umlaut     ÿ  */
    };
    static int
        table [256];
    static Bool
        initialized = FALSE;
    int
        index;

    if (!initialized) {
        /*  Default translation is from character to itself                  */
        for (index = 0; index < 256; index++)
            table [index] = index;
        /*  Now punch in our conversions                                     */
        for (index = 0; index < tblsize (conversion); index++)
            table [conversion [index].from] = (unsigned int) conversion [index].to;
        initialized = TRUE;
    }
    return ((char) table [(byte) ansichar]);
}


/****************************   START NEW TOKEN   ****************************/

MODULE start_new_token (void)
{
    tok_size = 0;
    scrit_count = 0;
}


/****************************   APPEND TO TOKEN   ****************************/

MODULE append_to_token (void)
{
    token [tok_size++] = cur_char;
}


/***************************   STORE ALPHA TOKEN   ***************************/

MODULE store_alpha_token (void)
{
    /*  We store a prefix+phonetic textual token                             */
    if (tok_size == 1)
        token [tok_size++] = '-';
        
    token [tok_size] = '\0';
    cur_scrit [scrit_count++] = scrit_add ('t',
        strprintf ("%c%c%s", token [0], token [1], soundex (token) + 1));
}

/*  scrit_add -- local
    Adds the specified scrit to the list.
    Returns the address of the created SCRIT.
    We do not try to eliminate duplicates, this can be done elsewhere.
 */
static SCRIT *
scrit_add (char type, char *value)
{
    SCRIT
        scrit = { NULL, '?', ' ', ' ' };

    scrit.value  = mem_strdup (value);
    scrit.type   = type;
    scrit.method = method;
    scrit.scope  = scope;
    list_queue (feedback, scrit);
    return ((SCRIT *) ((char *) feedback->prev + sizeof (LIST)));
}


/**************************   STORE NUMERIC TOKEN   **************************/

MODULE store_numeric_token (void)
{
    /*  We add one text criteria with the full string value                  */
    token [tok_size] = '\0';
    cur_scrit [scrit_count++] = scrit_add ('n', token);
}


/*****************************   USE METHOD OR   *****************************/

MODULE use_method_or (void)
{
    method = SEARCH_METHOD_OR;
}


/*****************************   USE METHOD AND   ****************************/

MODULE use_method_and (void)
{
    method = SEARCH_METHOD_AND;
}


/*****************************   USE METHOD NOT   ****************************/

MODULE use_method_not (void)
{
    method = SEARCH_METHOD_NOT;
}


/****************************   USE SCOPE EQUAL   ****************************/

MODULE use_scope_equal (void)
{
    scope = SEARCH_SCOPE_EQ;
}


/************************   USE SCOPE GREATER EQUAL   ************************/

MODULE use_scope_greater_equal (void)
{
    scope = SEARCH_SCOPE_GE;
}


/**************************   USE SCOPE LESS EQUAL   *************************/

MODULE use_scope_less_equal (void)
{
    set_previous_token_scope (SEARCH_SCOPE_LE);
}

static void
set_previous_token_scope (char scope)
{
    int
        index;

    for (index = 0; index < scrit_count; index++)
        cur_scrit [index]->scope = scope;
}


/***************************   USE SCOPE FROM TO   ***************************/

MODULE use_scope_from_to (void)
{
    set_previous_token_scope (SEARCH_SCOPE_FROM);
    scope = SEARCH_SCOPE_TO;
}


/***************************   GET EXTERNAL EVENT   **************************/

MODULE get_external_event (void)
{
}


/*************************   TERMINATE THE PROGRAM    ************************/

MODULE terminate_the_program (void)
{
    mem_free (token);
    the_next_event = terminate_event;
}

