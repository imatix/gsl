/*---------------------------------------------------------------------------
 *  ggenvt.c - GSL/environment package
 *
 *  Generated from ggenvt by ggobjt.gsl using GSL/4.
 *  DO NOT MODIFY THIS FILE.
 *
 *  For documentation and updates see http://www.imatix.com.
 *---------------------------------------------------------------------------*/

#include "sfl.h"
#include "smt3.h"
#include "gsl.h"                        /*  Project header file              */
#include "ggenvt.h"                     /*  Include header file              */

/*- Macros ------------------------------------------------------------------*/

#define ENV_NAME "env"                  /*  Environment Functions            */

#define matches(s,t)    (s ? (ignorecase ? lexcmp (s, t) == 0 : streq (s, t))   : t == NULL)

/*- Function prototypes -----------------------------------------------------*/

static int
env_get (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread);
static int
envset (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread);

/*- Global variables --------------------------------------------------------*/
static PARM_LIST parm_list_v            = { PARM_VALUE };

static GSL_FUNCTION env_functions [] =
{
    {"get",            1, 1, 1, (void *) &parm_list_v, 1, env_get},
    {"set",            1, 2, 1, (void *) &parm_list_v, 1, envset}};

CLASS_DESCRIPTOR
    env_class = {
        "env",
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
        env_functions, tblsize (env_functions) };



static int
env_get (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread)
{
    RESULT_NODE *name    = argc > 0 ? argv [0] : NULL;

    if (! name)
      {
        strcpy (object_error, "Missing argument: name");
        return -1;
      }
    if (name-> value. type == TYPE_UNDEFINED)
      {
        result-> culprit = name-> culprit;
        name-> culprit = NULL;
        return 0;
      }

  {
    result-> value. s = mem_strdup (getenv (string_value (& name-> value)));
    if (result-> value. s)
        result-> value. type = TYPE_STRING;

    return 0;
  }

    return 0;  /*  Just in case  */
}


static int
envset (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread)
{
    RESULT_NODE *name    = argc > 0 ? argv [0] : NULL;
    RESULT_NODE *value   = argc > 1 ? argv [1] : NULL;

    if (! name)
      {
        strcpy (object_error, "Missing argument: name");
        return -1;
      }
    if (name-> value. type == TYPE_UNDEFINED)
      {
        result-> culprit = name-> culprit;
        name-> culprit = NULL;
        return 0;
      }

  {
    result-> value. type = TYPE_NUMBER;
    if (value
    &&  value-> value. type != TYPE_UNDEFINED)
        result-> value. n = env_set (string_value (& name-> value),
                                     string_value (& value-> value),
                                     1);
    else
      {
        env_clear (string_value (& name-> value));
        result-> value. n = 0;
      }
    return 0;
  }

    return 0;  /*  Just in case  */
}

static int env_class_init (CLASS_DESCRIPTOR **class, void **item, THREAD *gsl_thread)
{
     *class = & env_class;

    return 0;
}

int register_env_classes (void)
{
    int
        rc = 0;
    rc |= object_register (env_class_init,
                           NULL);
    return rc;
}
