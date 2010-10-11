/*---------------------------------------------------------------------------
 *  ggmath.c - GSL/math package
 *
 *  Generated from ggmath by ggobjt.gsl using GSL/4.
 *  DO NOT MODIFY THIS FILE.
 *
 *  For documentation and updates see http://www.imatix.com.
 *---------------------------------------------------------------------------*/

#include "sfl.h"
#include "smt3.h"
#include "gsl.h"                        /*  Project header file              */
#include "ggmath.h"                     /*  Include header file              */

/*- Macros ------------------------------------------------------------------*/

#define MATH_NAME "math"                /*  Math Functions                   */

#define matches(s,t)    (s ? (ignorecase ? lexcmp (s, t) == 0 : streq (s, t))   : t == NULL)

/*- Function prototypes -----------------------------------------------------*/

static int
math_abs (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread);
static int
math_ceil (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread);
static int
math_floor (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread);
static int
math_mod (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread);
static int
math_rand (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread);
static int
math_sqrt (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread);
static int
math_exp (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread);
static int
math_log (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread);
static int
math_log10 (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread);
static int
math_pow (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread);
static int
math_sin (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread);
static int
math_cos (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread);
static int
math_tan (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread);
static int
math_sinh (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread);
static int
math_cosh (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread);
static int
math_tanh (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread);
static int
math_asin (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread);
static int
math_acos (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread);
static int
math_atan (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread);
static int
math_atan2 (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread);
static int
math_pi (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread);
static int
math_asinh (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread);
static int
math_acosh (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread);
static int
math_atanh (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread);

/*- Global variables --------------------------------------------------------*/
static PARM_LIST parm_list_v            = { PARM_VALUE };

static GSL_FUNCTION math_functions [] =
{
    {"abs",            1, 1, 1, (void *) &parm_list_v, 1, math_abs},
    {"acos",           1, 1, 1, (void *) &parm_list_v, 1, math_acos},
    {"acosh",          1, 1, 1, (void *) &parm_list_v, 1, math_acosh},
    {"asin",           1, 1, 1, (void *) &parm_list_v, 1, math_asin},
    {"asinh",          1, 1, 1, (void *) &parm_list_v, 1, math_asinh},
    {"atan",           1, 1, 1, (void *) &parm_list_v, 1, math_atan},
    {"atan2",          2, 2, 1, (void *) &parm_list_v, 1, math_atan2},
    {"atanh",          1, 1, 1, (void *) &parm_list_v, 1, math_atanh},
    {"ceil",           1, 1, 1, (void *) &parm_list_v, 1, math_ceil},
    {"cos",            1, 1, 1, (void *) &parm_list_v, 1, math_cos},
    {"cosh",           1, 1, 1, (void *) &parm_list_v, 1, math_cosh},
    {"exp",            1, 1, 1, (void *) &parm_list_v, 1, math_exp},
    {"floor",          1, 1, 1, (void *) &parm_list_v, 1, math_floor},
    {"log",            1, 1, 1, (void *) &parm_list_v, 1, math_log},
    {"log10",          1, 1, 1, (void *) &parm_list_v, 1, math_log10},
    {"mod",            2, 2, 1, (void *) &parm_list_v, 1, math_mod},
    {"pi",             0, 0, 0, NULL,            1, math_pi},
    {"pow",            2, 2, 1, (void *) &parm_list_v, 1, math_pow},
    {"rand",           0, 0, 0, NULL,            1, math_rand},
    {"sin",            1, 1, 1, (void *) &parm_list_v, 1, math_sin},
    {"sinh",           1, 1, 1, (void *) &parm_list_v, 1, math_sinh},
    {"sqrt",           1, 1, 1, (void *) &parm_list_v, 1, math_sqrt},
    {"tan",            1, 1, 1, (void *) &parm_list_v, 1, math_tan},
    {"tanh",           1, 1, 1, (void *) &parm_list_v, 1, math_tanh}};

CLASS_DESCRIPTOR
    math_class = {
        "math",
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
        math_functions, tblsize (math_functions) };



static int
math_abs (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread)
{
    RESULT_NODE *parm    = argc > 0 ? argv [0] : NULL;

    if (! parm)
      {
        strcpy (object_error, "Missing argument: parm");
        return -1;
      }
    if (parm-> value. type == TYPE_UNDEFINED)
      {
        result-> culprit = parm-> culprit;
        parm-> culprit = NULL;
        return 0;
      }

  {
    number_value (&parm-> value);

    if (parm-> value. type == TYPE_NUMBER)
      {
        result-> value. type = TYPE_NUMBER;
        result-> value. n    = fabs (parm-> value. n);
      }
    return 0;
  }

    return 0;  /*  Just in case  */
}


static int
math_ceil (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread)
{
    RESULT_NODE *parm    = argc > 0 ? argv [0] : NULL;

    if (! parm)
      {
        strcpy (object_error, "Missing argument: parm");
        return -1;
      }
    if (parm-> value. type == TYPE_UNDEFINED)
      {
        result-> culprit = parm-> culprit;
        parm-> culprit = NULL;
        return 0;
      }

  {
    number_value (&parm-> value);

    if (parm-> value. type == TYPE_NUMBER)
      {
        result-> value. type = TYPE_NUMBER;
        result-> value. n    = ceil (parm-> value. n);
      }
    return 0;
  }

    return 0;  /*  Just in case  */
}


static int
math_floor (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread)
{
    RESULT_NODE *parm    = argc > 0 ? argv [0] : NULL;

    if (! parm)
      {
        strcpy (object_error, "Missing argument: parm");
        return -1;
      }
    if (parm-> value. type == TYPE_UNDEFINED)
      {
        result-> culprit = parm-> culprit;
        parm-> culprit = NULL;
        return 0;
      }

  {
    number_value (&parm-> value);

    if (parm-> value. type == TYPE_NUMBER)
      {
        result-> value. type = TYPE_NUMBER;
        result-> value. n    = floor (parm-> value. n);
      }
    return 0;
  }

    return 0;  /*  Just in case  */
}


static int
math_mod (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread)
{
    RESULT_NODE *x       = argc > 0 ? argv [0] : NULL;
    RESULT_NODE *y       = argc > 1 ? argv [1] : NULL;

    if (! x)
      {
        strcpy (object_error, "Missing argument: x");
        return -1;
      }
    if (x-> value. type == TYPE_UNDEFINED)
      {
        result-> culprit = x-> culprit;
        x-> culprit = NULL;
        return 0;
      }
    if (! y)
      {
        strcpy (object_error, "Missing argument: y");
        return -1;
      }
    if (y-> value. type == TYPE_UNDEFINED)
      {
        result-> culprit = y-> culprit;
        y-> culprit = NULL;
        return 0;
      }

  {
    number_value (&x-> value);
    number_value (&y-> value);

    if ((x-> value. type == TYPE_NUMBER)
    && (y-> value. type == TYPE_NUMBER)
    && (y-> value. n))
      {
        result-> value. type = TYPE_NUMBER;
        result-> value. n    = fmod (number_value (&x-> value),
                                        number_value (&y-> value));
      }
    return 0;
  }

    return 0;  /*  Just in case  */
}


static int
math_rand (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread)
{


  {
    static Bool
        first_time = TRUE;
  
    if (first_time)
      {
        srand ((unsigned) time (NULL));
        first_time = FALSE;
      }

      result-> value. type = TYPE_NUMBER;
      result-> value. n    = (double) rand () / RAND_MAX;
  }

    return 0;  /*  Just in case  */
}


static int
math_sqrt (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread)
{
    RESULT_NODE *parm    = argc > 0 ? argv [0] : NULL;

    if (! parm)
      {
        strcpy (object_error, "Missing argument: parm");
        return -1;
      }
    if (parm-> value. type == TYPE_UNDEFINED)
      {
        result-> culprit = parm-> culprit;
        parm-> culprit = NULL;
        return 0;
      }

  {
    number_value (&parm-> value);

    if ((parm-> value. type == TYPE_NUMBER)
    && (parm-> value. n >= 0))
      {
        result-> value. type = TYPE_NUMBER;
        result-> value. n    = sqrt (parm-> value. n);
      }
    return 0;
  }

    return 0;  /*  Just in case  */
}


static int
math_exp (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread)
{
    RESULT_NODE *parm    = argc > 0 ? argv [0] : NULL;

    if (! parm)
      {
        strcpy (object_error, "Missing argument: parm");
        return -1;
      }
    if (parm-> value. type == TYPE_UNDEFINED)
      {
        result-> culprit = parm-> culprit;
        parm-> culprit = NULL;
        return 0;
      }

  {
    number_value (&parm-> value);

    if (parm-> value. type == TYPE_NUMBER)
      {
        result-> value. type = TYPE_NUMBER;
        result-> value. n    = exp (parm-> value. n);
      }
    return 0;
  }

    return 0;  /*  Just in case  */
}


static int
math_log (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread)
{
    RESULT_NODE *parm    = argc > 0 ? argv [0] : NULL;

    if (! parm)
      {
        strcpy (object_error, "Missing argument: parm");
        return -1;
      }
    if (parm-> value. type == TYPE_UNDEFINED)
      {
        result-> culprit = parm-> culprit;
        parm-> culprit = NULL;
        return 0;
      }

  {
    number_value (&parm-> value);

    if ((parm-> value. type == TYPE_NUMBER)
    && (parm-> value.n > 0))
      {
        result-> value. type = TYPE_NUMBER;
        result-> value. n    = log (parm-> value. n);
      }
    return 0;
  }

    return 0;  /*  Just in case  */
}


static int
math_log10 (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread)
{
    RESULT_NODE *parm    = argc > 0 ? argv [0] : NULL;

    if (! parm)
      {
        strcpy (object_error, "Missing argument: parm");
        return -1;
      }
    if (parm-> value. type == TYPE_UNDEFINED)
      {
        result-> culprit = parm-> culprit;
        parm-> culprit = NULL;
        return 0;
      }

  {
    number_value (&parm-> value);

    if ((parm-> value. type == TYPE_NUMBER)
    && (parm-> value.n > 0))
      {
        result-> value. type = TYPE_NUMBER;
        result-> value. n    = log10 (parm-> value. n);
      }
    return 0;
  }

    return 0;  /*  Just in case  */
}


static int
math_pow (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread)
{
    RESULT_NODE *x       = argc > 0 ? argv [0] : NULL;
    RESULT_NODE *y       = argc > 1 ? argv [1] : NULL;

    if (! x)
      {
        strcpy (object_error, "Missing argument: x");
        return -1;
      }
    if (x-> value. type == TYPE_UNDEFINED)
      {
        result-> culprit = x-> culprit;
        x-> culprit = NULL;
        return 0;
      }
    if (! y)
      {
        strcpy (object_error, "Missing argument: y");
        return -1;
      }
    if (y-> value. type == TYPE_UNDEFINED)
      {
        result-> culprit = y-> culprit;
        y-> culprit = NULL;
        return 0;
      }

  {
    number_value (&x-> value);
    number_value (&y-> value);

    if ((x-> value. type == TYPE_NUMBER)
    && (y-> value. type == TYPE_NUMBER))
      {
        if (((x-> value.n != 0)
        ||   (y-> value.n > 0))
        && ((x-> value.n >= 0)
        ||   (y-> value.n == floor (y-> value.n))))
          {
            result-> value. type = TYPE_NUMBER;
            result-> value. n    = pow (number_value (&x-> value),
                                           number_value (&y-> value));
          }
      }
    return 0;
  }

    return 0;  /*  Just in case  */
}


static int
math_sin (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread)
{
    RESULT_NODE *parm    = argc > 0 ? argv [0] : NULL;

    if (! parm)
      {
        strcpy (object_error, "Missing argument: parm");
        return -1;
      }
    if (parm-> value. type == TYPE_UNDEFINED)
      {
        result-> culprit = parm-> culprit;
        parm-> culprit = NULL;
        return 0;
      }

  {
    number_value (&parm-> value);

    if (parm-> value. type == TYPE_NUMBER)
      {
        result-> value. type = TYPE_NUMBER;
        result-> value. n    = sin (parm-> value. n);
      }
    return 0;
  }

    return 0;  /*  Just in case  */
}


static int
math_cos (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread)
{
    RESULT_NODE *parm    = argc > 0 ? argv [0] : NULL;

    if (! parm)
      {
        strcpy (object_error, "Missing argument: parm");
        return -1;
      }
    if (parm-> value. type == TYPE_UNDEFINED)
      {
        result-> culprit = parm-> culprit;
        parm-> culprit = NULL;
        return 0;
      }

  {
    number_value (&parm-> value);

    if (parm-> value. type == TYPE_NUMBER)
      {
        result-> value. type = TYPE_NUMBER;
        result-> value. n    = cos (parm-> value. n);
      }
    return 0;
  }

    return 0;  /*  Just in case  */
}


static int
math_tan (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread)
{
    RESULT_NODE *parm    = argc > 0 ? argv [0] : NULL;

    if (! parm)
      {
        strcpy (object_error, "Missing argument: parm");
        return -1;
      }
    if (parm-> value. type == TYPE_UNDEFINED)
      {
        result-> culprit = parm-> culprit;
        parm-> culprit = NULL;
        return 0;
      }

  {
    number_value (&parm-> value);

    if (parm-> value. type == TYPE_NUMBER)
      {
        result-> value. type = TYPE_NUMBER;
        result-> value. n    = tan (parm-> value. n);
      }
    return 0;
  }

    return 0;  /*  Just in case  */
}


static int
math_sinh (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread)
{
    RESULT_NODE *parm    = argc > 0 ? argv [0] : NULL;

    if (! parm)
      {
        strcpy (object_error, "Missing argument: parm");
        return -1;
      }
    if (parm-> value. type == TYPE_UNDEFINED)
      {
        result-> culprit = parm-> culprit;
        parm-> culprit = NULL;
        return 0;
      }

  {
    number_value (&parm-> value);

    if (parm-> value. type == TYPE_NUMBER)
      {
        result-> value. type = TYPE_NUMBER;
        result-> value. n    = sinh (parm-> value. n);
      }
    return 0;
  }

    return 0;  /*  Just in case  */
}


static int
math_cosh (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread)
{
    RESULT_NODE *parm    = argc > 0 ? argv [0] : NULL;

    if (! parm)
      {
        strcpy (object_error, "Missing argument: parm");
        return -1;
      }
    if (parm-> value. type == TYPE_UNDEFINED)
      {
        result-> culprit = parm-> culprit;
        parm-> culprit = NULL;
        return 0;
      }

  {
    number_value (&parm-> value);

    if (parm-> value. type == TYPE_NUMBER)
      {
        result-> value. type = TYPE_NUMBER;
        result-> value. n    = cosh (parm-> value. n);
      }
    return 0;
  }

    return 0;  /*  Just in case  */
}


static int
math_tanh (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread)
{
    RESULT_NODE *parm    = argc > 0 ? argv [0] : NULL;

    if (! parm)
      {
        strcpy (object_error, "Missing argument: parm");
        return -1;
      }
    if (parm-> value. type == TYPE_UNDEFINED)
      {
        result-> culprit = parm-> culprit;
        parm-> culprit = NULL;
        return 0;
      }

  {
    number_value (&parm-> value);

    if (parm-> value. type == TYPE_NUMBER)
      {
        result-> value. type = TYPE_NUMBER;
        result-> value. n    = tanh (parm-> value. n);
      }
    return 0;
  }

    return 0;  /*  Just in case  */
}


static int
math_asin (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread)
{
    RESULT_NODE *parm    = argc > 0 ? argv [0] : NULL;

    if (! parm)
      {
        strcpy (object_error, "Missing argument: parm");
        return -1;
      }
    if (parm-> value. type == TYPE_UNDEFINED)
      {
        result-> culprit = parm-> culprit;
        parm-> culprit = NULL;
        return 0;
      }

  {
    number_value (&parm-> value);

    if ((parm-> value. type == TYPE_NUMBER)
    && (parm-> value. n >= -1)
    && (parm-> value. n <=  1))
      {
        result-> value. type = TYPE_NUMBER;
        result-> value. n    = asin (parm-> value. n);
      }
    return 0;
  }

    return 0;  /*  Just in case  */
}


static int
math_acos (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread)
{
    RESULT_NODE *parm    = argc > 0 ? argv [0] : NULL;

    if (! parm)
      {
        strcpy (object_error, "Missing argument: parm");
        return -1;
      }
    if (parm-> value. type == TYPE_UNDEFINED)
      {
        result-> culprit = parm-> culprit;
        parm-> culprit = NULL;
        return 0;
      }

  {
    number_value (&parm-> value);

    if ((parm-> value. type == TYPE_NUMBER)
    && (parm-> value. n >= -1)
    && (parm-> value. n <=  1))
      {
        result-> value. type = TYPE_NUMBER;
        result-> value. n    = acos (parm-> value. n);
      }
    return 0;
  }

    return 0;  /*  Just in case  */
}


static int
math_atan (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread)
{
    RESULT_NODE *parm    = argc > 0 ? argv [0] : NULL;

    if (! parm)
      {
        strcpy (object_error, "Missing argument: parm");
        return -1;
      }
    if (parm-> value. type == TYPE_UNDEFINED)
      {
        result-> culprit = parm-> culprit;
        parm-> culprit = NULL;
        return 0;
      }

  {
    number_value (&parm-> value);

    if (parm-> value. type == TYPE_NUMBER)
      {
        result-> value. type = TYPE_NUMBER;
        result-> value. n    = atan (parm-> value. n);
      }
    return 0;
  }

    return 0;  /*  Just in case  */
}


static int
math_atan2 (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread)
{
    RESULT_NODE *x       = argc > 0 ? argv [0] : NULL;
    RESULT_NODE *y       = argc > 1 ? argv [1] : NULL;

    if (! x)
      {
        strcpy (object_error, "Missing argument: x");
        return -1;
      }
    if (x-> value. type == TYPE_UNDEFINED)
      {
        result-> culprit = x-> culprit;
        x-> culprit = NULL;
        return 0;
      }
    if (! y)
      {
        strcpy (object_error, "Missing argument: y");
        return -1;
      }
    if (y-> value. type == TYPE_UNDEFINED)
      {
        result-> culprit = y-> culprit;
        y-> culprit = NULL;
        return 0;
      }

  {
    number_value (&x-> value);
    number_value (&y-> value);

    if ((x-> value. type == TYPE_NUMBER)
    && (y-> value. type == TYPE_NUMBER))
      {
        result-> value. type = TYPE_NUMBER;
        result-> value. n    = atan2 (x-> value. n, y-> value. n);
      }
    return 0;
  }

    return 0;  /*  Just in case  */
}


static int
math_pi (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread)
{


  {
    result-> value. type = TYPE_NUMBER;
    result-> value. n    = atan (1) * 4;
    
    return 0;
  }

    return 0;  /*  Just in case  */
}


static int
math_asinh (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread)
{
    RESULT_NODE *parm    = argc > 0 ? argv [0] : NULL;

    if (! parm)
      {
        strcpy (object_error, "Missing argument: parm");
        return -1;
      }
    if (parm-> value. type == TYPE_UNDEFINED)
      {
        result-> culprit = parm-> culprit;
        parm-> culprit = NULL;
        return 0;
      }

  {
    number_value (&parm-> value);

    if (parm-> value. type == TYPE_NUMBER)
      {
        result-> value. type = TYPE_NUMBER;
        result-> value. n    = -log (
            sqrt (parm-> value. n * parm-> value. n + 1)
            - parm-> value. n
        );
      }
    return 0;
  }

    return 0;  /*  Just in case  */
}


static int
math_acosh (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread)
{
    RESULT_NODE *parm    = argc > 0 ? argv [0] : NULL;

    if (! parm)
      {
        strcpy (object_error, "Missing argument: parm");
        return -1;
      }
    if (parm-> value. type == TYPE_UNDEFINED)
      {
        result-> culprit = parm-> culprit;
        parm-> culprit = NULL;
        return 0;
      }

  {
    number_value (&parm-> value);

    if ((parm-> value. type == TYPE_NUMBER)
    && (parm-> value. n >= 1))
      {
        result-> value. type = TYPE_NUMBER;
        result-> value. n    = log (
            sqrt (parm-> value. n * parm-> value. n - 1)
            + parm-> value. n
        );
      }
    return 0;
  }

    return 0;  /*  Just in case  */
}


static int
math_atanh (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread)
{
    RESULT_NODE *parm    = argc > 0 ? argv [0] : NULL;

    if (! parm)
      {
        strcpy (object_error, "Missing argument: parm");
        return -1;
      }
    if (parm-> value. type == TYPE_UNDEFINED)
      {
        result-> culprit = parm-> culprit;
        parm-> culprit = NULL;
        return 0;
      }

  {
    number_value (&parm-> value);

    if ((parm-> value. type == TYPE_NUMBER)
    && (parm-> value. n < 1))
      {
        result-> value. type = TYPE_NUMBER;
        result-> value. n    = log (
            (1 + parm-> value. n) /
            (1 - parm-> value. n)
        ) / 2;
      }
    return 0;
  }

    return 0;  /*  Just in case  */
}

static int math_class_init (CLASS_DESCRIPTOR **class, void **item, THREAD *gsl_thread)
{
     *class = & math_class;

    return 0;
}

int register_math_classes (void)
{
    int
        rc = 0;
    rc |= object_register (math_class_init,
                           NULL);
    return rc;
}
