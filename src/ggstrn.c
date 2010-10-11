/*---------------------------------------------------------------------------
 *  ggstrn.c - GSL/string package
 *
 *  Generated from ggstrn by ggobjt.gsl using GSL/4.
 *  DO NOT MODIFY THIS FILE.
 *
 *  For documentation and updates see http://www.imatix.com.
 *---------------------------------------------------------------------------*/

#include "sfl.h"
#include "smt3.h"
#include "gsl.h"                        /*  Project header file              */
#include "ggstrn.h"                     /*  Include header file              */

/*- Macros ------------------------------------------------------------------*/

#define STRING_NAME "string"            /*  String Functions                 */

#define matches(s,t)    (s ? (ignorecase ? lexcmp (s, t) == 0 : streq (s, t))   : t == NULL)

/*- Function prototypes -----------------------------------------------------*/

static int
string_length (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread);
static int
string_locate (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread);
static int
string_locate_last (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread);
static int
string_substr (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread);
static int
string_trim (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread);
static int
string_justify (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread);
static int
string_certify (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread);
static int
string_replace (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread);
static int
string_match (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread);
static int
string_prefixed (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread);
static int
string_prefix (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread);
static int
string_defix (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread);
static int
string_hash (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread);
static int
string_convch (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread);
static int
string_lexcmp (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread);
static int
string_lexncmp (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread);
static int
string_lexwcmp (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread);
static int
string_matchpat (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread);
static int
string_soundex (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread);
static int
string_cntch (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread);

/*- Global variables --------------------------------------------------------*/
static PARM_LIST parm_list_v            = { PARM_VALUE };

static GSL_FUNCTION string_functions [] =
{
    {"certify",        1, 2, 1, (void *) &parm_list_v, 1, string_certify},
    {"cntch",          2, 2, 1, (void *) &parm_list_v, 1, string_cntch},
    {"convch",         3, 3, 1, (void *) &parm_list_v, 1, string_convch},
    {"defix",          2, 2, 1, (void *) &parm_list_v, 1, string_defix},
    {"hash",           1, 1, 1, (void *) &parm_list_v, 1, string_hash},
    {"justify",        2, 3, 1, (void *) &parm_list_v, 1, string_justify},
    {"length",         1, 1, 1, (void *) &parm_list_v, 1, string_length},
    {"lexcmp",         2, 2, 1, (void *) &parm_list_v, 1, string_lexcmp},
    {"lexncmp",        3, 3, 1, (void *) &parm_list_v, 1, string_lexncmp},
    {"lexwcmp",        2, 2, 1, (void *) &parm_list_v, 1, string_lexwcmp},
    {"locate",         2, 2, 1, (void *) &parm_list_v, 1, string_locate},
    {"locate_last",    2, 2, 1, (void *) &parm_list_v, 1, string_locate_last},
    {"match",          2, 2, 1, (void *) &parm_list_v, 1, string_match},
    {"matchpat",       2, 3, 1, (void *) &parm_list_v, 1, string_matchpat},
    {"prefix",         2, 2, 1, (void *) &parm_list_v, 1, string_prefix},
    {"prefixed",       2, 2, 1, (void *) &parm_list_v, 1, string_prefixed},
    {"replace",        2, 2, 1, (void *) &parm_list_v, 1, string_replace},
    {"soundex",        1, 1, 1, (void *) &parm_list_v, 1, string_soundex},
    {"substr",         1, 4, 1, (void *) &parm_list_v, 1, string_substr},
    {"trim",           1, 1, 1, (void *) &parm_list_v, 1, string_trim}};

CLASS_DESCRIPTOR
    string_class = {
        "string",
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        string_functions, tblsize (string_functions) };


static Bool
node_is_countable (int argn, RESULT_NODE **argv,
                   char *function, RESULT_NODE *result)
{
    if (argv [argn])
      {
        if (argv [argn]-> value. type != TYPE_UNDEFINED)
          {
            number_value (&argv [argn]-> value);
            if ((argv [argn]-> value. type == TYPE_NUMBER)
            && (argv [argn]-> value. n == floor (argv [argn]-> value. n))
            && (argv [argn]-> value. n >= 0))
                return TRUE;
            else
                snprintf (object_error, LINE_MAX,
                          "Illegal argument %u to function %s.",
                          argn + 1, function);
          }
        else
          {
            result-> culprit = argv [argn]-> culprit;
            argv [argn]-> culprit = NULL;
          }
      }

    return FALSE;
}


static int
string_length (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread)
{
    RESULT_NODE *string  = argc > 0 ? argv [0] : NULL;

    if (! string)
      {
        strcpy (object_error, "Missing argument: string");
        return -1;
      }
    if (string-> value. type == TYPE_UNDEFINED)
      {
        result-> culprit = string-> culprit;
        string-> culprit = NULL;
        return 0;
      }

    result-> value. type = TYPE_NUMBER;
    result-> value. n    = strlen (string_value (&string-> value));

    return 0;  /*  Just in case  */
}


static int
string_locate (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread)
{
    RESULT_NODE *haystack = argc > 0 ? argv [0] : NULL;
    RESULT_NODE *needle  = argc > 1 ? argv [1] : NULL;

    if (! haystack)
      {
        strcpy (object_error, "Missing argument: haystack");
        return -1;
      }
    if (haystack-> value. type == TYPE_UNDEFINED)
      {
        result-> culprit = haystack-> culprit;
        haystack-> culprit = NULL;
        return 0;
      }
    if (! needle)
      {
        strcpy (object_error, "Missing argument: needle");
        return -1;
      }
    if (needle-> value. type == TYPE_UNDEFINED)
      {
        result-> culprit = needle-> culprit;
        needle-> culprit = NULL;
        return 0;
      }

  {
    char
        *strptr = strstr (string_value (&haystack-> value),
                          string_value (&needle  -> value));
    if (strptr)
      {
        result-> value. type = TYPE_NUMBER;
        result-> value. n    = strptr - haystack-> value. s;
      }
  }

    return 0;  /*  Just in case  */
}


static int
string_locate_last (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread)
{
    RESULT_NODE *haystack = argc > 0 ? argv [0] : NULL;
    RESULT_NODE *needle  = argc > 1 ? argv [1] : NULL;

    if (! haystack)
      {
        strcpy (object_error, "Missing argument: haystack");
        return -1;
      }
    if (haystack-> value. type == TYPE_UNDEFINED)
      {
        result-> culprit = haystack-> culprit;
        haystack-> culprit = NULL;
        return 0;
      }
    if (! needle)
      {
        strcpy (object_error, "Missing argument: needle");
        return -1;
      }
    if (needle-> value. type == TYPE_UNDEFINED)
      {
        result-> culprit = needle-> culprit;
        needle-> culprit = NULL;
        return 0;
      }

  {
    char*
        haystackptr = string_value (&haystack-> value);
    char*
        needleptr = string_value (&needle-> value);
    char*
        strptr = strstr (haystackptr, needleptr);
    char*
        lastseenptr = NULL;
    
    while (strptr != NULL) 
      {
        lastseenptr = strptr;
        strptr = strstr (lastseenptr + 1, needleptr);
      }

    if (lastseenptr != NULL)
      {
        result-> value. type = TYPE_NUMBER;
        result-> value. n    = lastseenptr - haystack-> value. s;
      }
  }

    return 0;  /*  Just in case  */
}


static int
string_substr (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread)
{
    RESULT_NODE *string  = argc > 0 ? argv [0] : NULL;
    RESULT_NODE *start   = argc > 1 ? argv [1] : NULL;
    RESULT_NODE *end     = argc > 2 ? argv [2] : NULL;
    RESULT_NODE *length  = argc > 3 ? argv [3] : NULL;

    if (! string)
      {
        strcpy (object_error, "Missing argument: string");
        return -1;
      }
    if (string-> value. type == TYPE_UNDEFINED)
      {
        result-> culprit = string-> culprit;
        string-> culprit = NULL;
        return 0;
      }

  {
    long
        start_n  = 0,
        end_n    = 0,
        length_n = 0;

    if (start  &&start -> value. type == TYPE_UNDEFINED)
        start = NULL;
    if (end    &&end   -> value. type == TYPE_UNDEFINED)
        end = NULL;
    if (length &&length-> value. type == TYPE_UNDEFINED)
        length = NULL;

    if (start &&end &&length)
      {
        strcpy (object_error, "Too many parameters for function 'substr'.");
        return -1;
      }
    if (!(start || end || length))
      {
        strcpy (object_error, "Too few parameters for function 'substr'.");
        return -1;
      }
    if (start)
      {
        if (node_is_countable (1, argv, "substr", result))
            start_n = (long) start-> value. n;
        else
            return -1;
      }
    if (end)
      {
        if (node_is_countable (2, argv, "substr", result))
            end_n = (long) end-> value. n;
        else
            return -1;
      }
    if (length)
      {
        if (node_is_countable (3, argv, "substr", result))
            length_n = (long) length-> value. n;
        else
            return -1;
      }
    if (start &&end &&(end_n < start_n))
      {
        strcpy (object_error, "'End' must be at least 'Start' in 'substr'");
        return -1;
      }
    if (length &&!start)
      {
        if (!end)
            end_n = strlen (string-> value. s) - 1;
        start_n = end_n - length_n + 1;
        if (start_n < 0)
            start_n = 0;
        length_n = end_n - start_n + 1;
      }
    else
      {
        if (!start)
            start_n = 0;
        if (!length)
          {
            if (end)
                length_n = end_n - start_n + 1;
            else
                length_n = strlen (string-> value. s);
          }
      }
    if (start_n >= (long) strlen (string-> value. s))
        result-> value. s = mem_strdup ("");
    else
      {
        result-> value. s = mem_alloc (length_n + 1);
        if (start_n >= 0)
            strncpy (result-> value. s, &string-> value. s [start_n], length_n);
        else
            strncpy (result-> value. s, string-> value. s, length_n);

        (result-> value. s) [length_n] = '\0';
      }
    result-> value. type = TYPE_STRING;
  }

    return 0;  /*  Just in case  */
}


static int
string_trim (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread)
{
    RESULT_NODE *string  = argc > 0 ? argv [0] : NULL;

    if (! string)
      {
        strcpy (object_error, "Missing argument: string");
        return -1;
      }
    if (string-> value. type == TYPE_UNDEFINED)
      {
        result-> culprit = string-> culprit;
        string-> culprit = NULL;
        return 0;
      }

{
    char *
        scanptr;
        
    if (string-> value. type == TYPE_STRING
    ||  string-> value. type == TYPE_NUMBER
    ||  string-> value. type == TYPE_UNKNOWN)
        result-> value. type = string-> value. type;

    if (string-> value. s)
      {
        scanptr = string-> value. s;
        while (*scanptr == '\n')
            scanptr++;
        result-> value. s = mem_strdup (strcrop (scanptr));
        ASSERT (result-> value. s);
      }
    result-> value. n = string-> value. n;
}

    return 0;  /*  Just in case  */
}


static int
string_justify (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread)
{
    RESULT_NODE *string  = argc > 0 ? argv [0] : NULL;
    RESULT_NODE *width   = argc > 1 ? argv [1] : NULL;
    RESULT_NODE *prefix  = argc > 2 ? argv [2] : NULL;

    if (! string)
      {
        strcpy (object_error, "Missing argument: string");
        return -1;
      }
    if (string-> value. type == TYPE_UNDEFINED)
      {
        result-> culprit = string-> culprit;
        string-> culprit = NULL;
        return 0;
      }
    if (! width)
      {
        strcpy (object_error, "Missing argument: width");
        return -1;
      }
    if (width-> value. type == TYPE_UNDEFINED)
      {
        result-> culprit = width-> culprit;
        width-> culprit = NULL;
        return 0;
      }

{
    unsigned long
        width_n;

    if (node_is_countable (1, argv, "justify", result))
        width_n = (unsigned long) width-> value. n;
    else
        return -1;

    result-> value. s = strreformat (string_value (&string-> value),
                                     width_n,
                                     string_result (prefix));
    result-> value. type = TYPE_STRING;
  }

    return 0;  /*  Just in case  */
}


static int
string_certify (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread)
{
    RESULT_NODE *number  = argc > 0 ? argv [0] : NULL;
    RESULT_NODE *language = argc > 1 ? argv [1] : NULL;

    if (! number)
      {
        strcpy (object_error, "Missing argument: number");
        return -1;
      }
    if (number-> value. type == TYPE_UNDEFINED)
      {
        result-> culprit = number-> culprit;
        number-> culprit = NULL;
        return 0;
      }

  {
    #define MAX_CHARS           512
    #define DEFAULT_LANGUAGE    "en-gb"

    static char
        buffer [MAX_CHARS + 1];

    if (language  &&language -> value. type == TYPE_UNDEFINED)
        language = NULL;

    certify_the_number (
        buffer,
        MAX_CHARS,
        (long) number_value (&number-> value),
        language ? string_value (&language-> value) : DEFAULT_LANGUAGE,
        850
    );

    result-> value. type = TYPE_STRING;
    result-> value. s    = mem_strdup (buffer);
  }

    return 0;  /*  Just in case  */
}


static int
string_replace (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread)
{
    RESULT_NODE *strbuf  = argc > 0 ? argv [0] : NULL;
    RESULT_NODE *strpattern = argc > 1 ? argv [1] : NULL;

    if (! strbuf)
      {
        strcpy (object_error, "Missing argument: strbuf");
        return -1;
      }
    if (strbuf-> value. type == TYPE_UNDEFINED)
      {
        result-> culprit = strbuf-> culprit;
        strbuf-> culprit = NULL;
        return 0;
      }
    if (! strpattern)
      {
        strcpy (object_error, "Missing argument: strpattern");
        return -1;
      }
    if (strpattern-> value. type == TYPE_UNDEFINED)
      {
        result-> culprit = strpattern-> culprit;
        strpattern-> culprit = NULL;
        return 0;
      }

  {
    char
        *original,
        *copy;
    size_t
        max_length;

    original = string_value (&strbuf->value);
    max_length = strlen (original) * 4;    //  Some random factor
    copy = mem_alloc (max_length + 1);
    strcpy (copy, original);

    stringreplace (copy, string_value (&strpattern->value), max_length);
    result-> value. s    = copy;
    result-> value. type = TYPE_STRING;
  }

    return 0;  /*  Just in case  */
}


static int
string_match (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread)
{
    RESULT_NODE *string1 = argc > 0 ? argv [0] : NULL;
    RESULT_NODE *string2 = argc > 1 ? argv [1] : NULL;

    if (! string1)
      {
        strcpy (object_error, "Missing argument: string1");
        return -1;
      }
    if (string1-> value. type == TYPE_UNDEFINED)
      {
        result-> culprit = string1-> culprit;
        string1-> culprit = NULL;
        return 0;
      }
    if (! string2)
      {
        strcpy (object_error, "Missing argument: string2");
        return -1;
      }
    if (string2-> value. type == TYPE_UNDEFINED)
      {
        result-> culprit = string2-> culprit;
        string2-> culprit = NULL;
        return 0;
      }

  {
    result-> value. n = strmatch (string_value (&string1-> value),
                                     string_value (&string2-> value));
    result-> value. type = TYPE_NUMBER;
  }

    return 0;  /*  Just in case  */
}


static int
string_prefixed (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread)
{
    RESULT_NODE *string  = argc > 0 ? argv [0] : NULL;
    RESULT_NODE *prefix  = argc > 1 ? argv [1] : NULL;

    if (! string)
      {
        strcpy (object_error, "Missing argument: string");
        return -1;
      }
    if (string-> value. type == TYPE_UNDEFINED)
      {
        result-> culprit = string-> culprit;
        string-> culprit = NULL;
        return 0;
      }
    if (! prefix)
      {
        strcpy (object_error, "Missing argument: prefix");
        return -1;
      }
    if (prefix-> value. type == TYPE_UNDEFINED)
      {
        result-> culprit = prefix-> culprit;
        prefix-> culprit = NULL;
        return 0;
      }

  {
    result-> value. n = strprefixed (string_value (&string-> value),
                                        string_value (&prefix-> value));
    result-> value. type = TYPE_NUMBER;
  }

    return 0;  /*  Just in case  */
}


static int
string_prefix (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread)
{
    RESULT_NODE *string  = argc > 0 ? argv [0] : NULL;
    RESULT_NODE *delims  = argc > 1 ? argv [1] : NULL;

    if (! string)
      {
        strcpy (object_error, "Missing argument: string");
        return -1;
      }
    if (string-> value. type == TYPE_UNDEFINED)
      {
        result-> culprit = string-> culprit;
        string-> culprit = NULL;
        return 0;
      }
    if (! delims)
      {
        strcpy (object_error, "Missing argument: delims");
        return -1;
      }
    if (delims-> value. type == TYPE_UNDEFINED)
      {
        result-> culprit = delims-> culprit;
        delims-> culprit = NULL;
        return 0;
      }

  {
    char
        *strptr = strprefix (string_value (&string-> value),
                             string_value (&delims-> value));
    result-> value. s    = strptr;           /*  strprefix does mem_alloc  */
    result-> value. type = TYPE_STRING;
  }

    return 0;  /*  Just in case  */
}


static int
string_defix (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread)
{
    RESULT_NODE *string  = argc > 0 ? argv [0] : NULL;
    RESULT_NODE *delims  = argc > 1 ? argv [1] : NULL;

    if (! string)
      {
        strcpy (object_error, "Missing argument: string");
        return -1;
      }
    if (string-> value. type == TYPE_UNDEFINED)
      {
        result-> culprit = string-> culprit;
        string-> culprit = NULL;
        return 0;
      }
    if (! delims)
      {
        strcpy (object_error, "Missing argument: delims");
        return -1;
      }
    if (delims-> value. type == TYPE_UNDEFINED)
      {
        result-> culprit = delims-> culprit;
        delims-> culprit = NULL;
        return 0;
      }

  {
    char
        *strptr = strdefix (string_value (&string-> value),
                            string_value (&delims-> value));
    result-> value. s    = mem_strdup (strptr);
    result-> value. type = TYPE_STRING;
  }

    return 0;  /*  Just in case  */
}


static int
string_hash (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread)
{
    RESULT_NODE *string  = argc > 0 ? argv [0] : NULL;

    if (! string)
      {
        strcpy (object_error, "Missing argument: string");
        return -1;
      }
    if (string-> value. type == TYPE_UNDEFINED)
      {
        result-> culprit = string-> culprit;
        string-> culprit = NULL;
        return 0;
      }

  {
    result-> value. n = strhash (string_value (&string-> value));
    result-> value. type = TYPE_NUMBER;
  }

    return 0;  /*  Just in case  */
}


static int
string_convch (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread)
{
    RESULT_NODE *string  = argc > 0 ? argv [0] : NULL;
    RESULT_NODE *from    = argc > 1 ? argv [1] : NULL;
    RESULT_NODE *to      = argc > 2 ? argv [2] : NULL;

    if (! string)
      {
        strcpy (object_error, "Missing argument: string");
        return -1;
      }
    if (string-> value. type == TYPE_UNDEFINED)
      {
        result-> culprit = string-> culprit;
        string-> culprit = NULL;
        return 0;
      }
    if (! from)
      {
        strcpy (object_error, "Missing argument: from");
        return -1;
      }
    if (from-> value. type == TYPE_UNDEFINED)
      {
        result-> culprit = from-> culprit;
        from-> culprit = NULL;
        return 0;
      }
    if (! to)
      {
        strcpy (object_error, "Missing argument: to");
        return -1;
      }
    if (to-> value. type == TYPE_UNDEFINED)
      {
        result-> culprit = to-> culprit;
        to-> culprit = NULL;
        return 0;
      }

  {
    char
        *strptr = strconvch (string_value (&string-> value),
                            *string_value (&from  -> value),
                            *string_value (&to    -> value));
    result-> value. s    = mem_strdup (strptr);
    result-> value. type = TYPE_STRING;
  }

    return 0;  /*  Just in case  */
}


static int
string_lexcmp (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread)
{
    RESULT_NODE *string1 = argc > 0 ? argv [0] : NULL;
    RESULT_NODE *string2 = argc > 1 ? argv [1] : NULL;

    if (! string1)
      {
        strcpy (object_error, "Missing argument: string1");
        return -1;
      }
    if (string1-> value. type == TYPE_UNDEFINED)
      {
        result-> culprit = string1-> culprit;
        string1-> culprit = NULL;
        return 0;
      }
    if (! string2)
      {
        strcpy (object_error, "Missing argument: string2");
        return -1;
      }
    if (string2-> value. type == TYPE_UNDEFINED)
      {
        result-> culprit = string2-> culprit;
        string2-> culprit = NULL;
        return 0;
      }

  {
    result-> value. n = lexcmp (string_value (&string1-> value),
                                   string_value (&string2-> value));
    result-> value. type = TYPE_NUMBER;
  }

    return 0;  /*  Just in case  */
}


static int
string_lexncmp (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread)
{
    RESULT_NODE *string1 = argc > 0 ? argv [0] : NULL;
    RESULT_NODE *string2 = argc > 1 ? argv [1] : NULL;
    RESULT_NODE *count   = argc > 2 ? argv [2] : NULL;

    if (! string1)
      {
        strcpy (object_error, "Missing argument: string1");
        return -1;
      }
    if (string1-> value. type == TYPE_UNDEFINED)
      {
        result-> culprit = string1-> culprit;
        string1-> culprit = NULL;
        return 0;
      }
    if (! string2)
      {
        strcpy (object_error, "Missing argument: string2");
        return -1;
      }
    if (string2-> value. type == TYPE_UNDEFINED)
      {
        result-> culprit = string2-> culprit;
        string2-> culprit = NULL;
        return 0;
      }
    if (! count)
      {
        strcpy (object_error, "Missing argument: count");
        return -1;
      }
    if (count-> value. type == TYPE_UNDEFINED)
      {
        result-> culprit = count-> culprit;
        count-> culprit = NULL;
        return 0;
      }

  {
    result-> value. n = lexncmp (string_value (&string1-> value),
                                    string_value (&string2-> value),
                              (int) number_value (&count  -> value));
    result-> value. type = TYPE_NUMBER;
  }

    return 0;  /*  Just in case  */
}


static int
string_lexwcmp (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread)
{
    RESULT_NODE *string1 = argc > 0 ? argv [0] : NULL;
    RESULT_NODE *pattern = argc > 1 ? argv [1] : NULL;

    if (! string1)
      {
        strcpy (object_error, "Missing argument: string1");
        return -1;
      }
    if (string1-> value. type == TYPE_UNDEFINED)
      {
        result-> culprit = string1-> culprit;
        string1-> culprit = NULL;
        return 0;
      }
    if (! pattern)
      {
        strcpy (object_error, "Missing argument: pattern");
        return -1;
      }
    if (pattern-> value. type == TYPE_UNDEFINED)
      {
        result-> culprit = pattern-> culprit;
        pattern-> culprit = NULL;
        return 0;
      }

  {
    result-> value. n = lexwcmp (string_value (&string1-> value),
                                    string_value (&pattern-> value));
    result-> value. type = TYPE_NUMBER;
  }

    return 0;  /*  Just in case  */
}


static int
string_matchpat (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread)
{
    RESULT_NODE *string1 = argc > 0 ? argv [0] : NULL;
    RESULT_NODE *pattern = argc > 1 ? argv [1] : NULL;
    RESULT_NODE *ic      = argc > 2 ? argv [2] : NULL;

    if (! string1)
      {
        strcpy (object_error, "Missing argument: string1");
        return -1;
      }
    if (string1-> value. type == TYPE_UNDEFINED)
      {
        result-> culprit = string1-> culprit;
        string1-> culprit = NULL;
        return 0;
      }
    if (! pattern)
      {
        strcpy (object_error, "Missing argument: pattern");
        return -1;
      }
    if (pattern-> value. type == TYPE_UNDEFINED)
      {
        result-> culprit = pattern-> culprit;
        pattern-> culprit = NULL;
        return 0;
      }

  {
    Bool
        ignore_case;

    if (ic)
        ignore_case = (Bool) number_value (&ic-> value);
    else
        ignore_case = 0;                /*  If mode unspecified, use FALSE   */

    result-> value. n = match_pattern (string_value (&string1-> value),
                                          string_value (&pattern-> value),
                                          ignore_case);
    result-> value. type = TYPE_NUMBER;
  }

    return 0;  /*  Just in case  */
}


static int
string_soundex (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread)
{
    RESULT_NODE *string  = argc > 0 ? argv [0] : NULL;

    if (! string)
      {
        strcpy (object_error, "Missing argument: string");
        return -1;
      }
    if (string-> value. type == TYPE_UNDEFINED)
      {
        result-> culprit = string-> culprit;
        string-> culprit = NULL;
        return 0;
      }

  {
    char
        *strptr = soundex (string_value (&string-> value));
    result-> value. s    = mem_strdup (strptr);
    result-> value. type = TYPE_STRING;
  }

    return 0;  /*  Just in case  */
}


static int
string_cntch (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread)
{
    RESULT_NODE *string  = argc > 0 ? argv [0] : NULL;
    RESULT_NODE *value   = argc > 1 ? argv [1] : NULL;

    if (! string)
      {
        strcpy (object_error, "Missing argument: string");
        return -1;
      }
    if (string-> value. type == TYPE_UNDEFINED)
      {
        result-> culprit = string-> culprit;
        string-> culprit = NULL;
        return 0;
      }
    if (! value)
      {
        strcpy (object_error, "Missing argument: value");
        return -1;
      }
    if (value-> value. type == TYPE_UNDEFINED)
      {
        result-> culprit = value-> culprit;
        value-> culprit = NULL;
        return 0;
      }

  {
    result-> value. n = strcntch (string_value (&string-> value),
                                    *string_value (&value -> value));
    result-> value. type = TYPE_NUMBER;
  }

    return 0;  /*  Just in case  */
}

static int string_class_init (CLASS_DESCRIPTOR **class, void **item, THREAD *gsl_thread)
{
     *class = & string_class;

    return 0;
}

int register_string_classes (void)
{
    int
        rc = 0;
    rc |= object_register (string_class_init,
                           NULL);
    return rc;
}
