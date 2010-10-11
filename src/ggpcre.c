/*---------------------------------------------------------------------------
 *  ggpcre.c - GSL/regexp package
 *
 *  Generated from ggpcre by ggobjt.gsl using GSL/4.
 *  DO NOT MODIFY THIS FILE.
 *
 *  For documentation and updates see http://www.imatix.com.
 *---------------------------------------------------------------------------*/

#include "sfl.h"
#include "smt3.h"
#include "gsl.h"                        /*  Project header file              */
#include "ggpcre.h"                     /*  Include header file              */

/*- Macros ------------------------------------------------------------------*/

#define REGEXP_NAME "regexp"            /*  Regular Expression Functions     */

#define matches(s,t)    (s ? (ignorecase ? lexcmp (s, t) == 0 : streq (s, t))   : t == NULL)

/*- Function prototypes -----------------------------------------------------*/

static int
regexp_match (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread);

/*- Global variables --------------------------------------------------------*/
static PARM_LIST parm_list_vvr          = { PARM_VALUE,
                                            PARM_VALUE,
                                            PARM_REFERENCE };

static GSL_FUNCTION regexp_functions [] =
{
    {"match",          2, 0, 3, (void *) &parm_list_vvr, 1, regexp_match}};

CLASS_DESCRIPTOR
    regexp_class = {
        "regexp",
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
        regexp_functions, tblsize (regexp_functions) };


#include "pcre.h"


static int
regexp_match (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread)
{
    RESULT_NODE *pattern = argc > 0 ? argv [0] : NULL;
    RESULT_NODE *subject = argc > 1 ? argv [1] : NULL;

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
    if (! subject)
      {
        strcpy (object_error, "Missing argument: subject");
        return -1;
      }
    if (subject-> value. type == TYPE_UNDEFINED)
      {
        result-> culprit = subject-> culprit;
        subject-> culprit = NULL;
        return 0;
      }

  {
    GGCODE_TCB
        *tcb = gsl_thread-> tcb;
    pcre
        *re;
    char
        *error;
    int 
        erroffset;
    int 
        *ovector;
    int
        oveccount,
        rc,
        i,
        start,
        len;
    VALUE
        value;

    re = pcre_compile (string_value (&pattern-> value),
                       0,
                       (const char **) &error,
                       &erroffset,
                       NULL);
    if (! re)
      {
        snprintf (object_error, LINE_MAX,
                  "Regular expression pattern error: %s\n%s\n%*c",
                  error,
                  pattern-> value. s,
                  erroffset + 1, '^');
        return -1;
      }

    rc = pcre_fullinfo (re,
                        NULL,
                        PCRE_INFO_CAPTURECOUNT,
                        &oveccount);
    oveccount = (oveccount + 1) * 3;
    ovector   = mem_alloc (oveccount * sizeof (int));

    string_value (&subject-> value);
    rc = pcre_exec (re,
                    NULL,
                    subject-> value. s,
                    (int) strlen (subject-> value. s),
                    0,
                    0,
                    ovector,
                    oveccount);

    (pcre_free) (re);

    if (rc == PCRE_ERROR_NOMATCH)
        rc = 0;
    else if (rc < 0)
      {
        snprintf (object_error, LINE_MAX,
                 "Regular expression matching error: %d", rc);
        mem_free (ovector);
        return -1;
      }
    else if (rc == 1)
        rc = -1;
    else
        rc -= 1;

    result-> value. type = TYPE_NUMBER;
    result-> value. n    = rc;

    i = 1;
    while (i < argc - 1)
      {
        if (argv [i + 1])
          {
            init_value (& value);
            if (i <= rc)
              {
                start = ovector [i * 2];
                len   = ovector [i * 2 + 1] - start;

                value. type = TYPE_STRING;
                value. s    = mem_alloc (len + 1);
                memcpy (value. s, subject-> value. s + start, len);
                value. s [len] = 0;
              }

            if (! store_symbol_definition (& tcb-> scope_stack,
                                           tcb-> gsl-> ignorecase,
                                           argv [i + 1],
                                           &value,
                                           &error))
              {
                strncpy (object_error, error, LINE_MAX);
                mem_free (value.s);
                mem_free (ovector);
                return -1;
              }
            destroy_value (& value);
          }
        i++;
      }

    mem_free (ovector);
  }

    return 0;  /*  Just in case  */
}

static int regexp_class_init (CLASS_DESCRIPTOR **class, void **item, THREAD *gsl_thread)
{
     *class = & regexp_class;

    return 0;
}

int register_regexp_classes (void)
{
    int
        rc = 0;
    rc |= object_register (regexp_class_init,
                           NULL);
    return rc;
}
