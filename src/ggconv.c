/*---------------------------------------------------------------------------
 *  ggconv.c - GSL/conv package
 *
 *  Generated from ggconv by ggobjt.gsl using GSL/4.
 *  DO NOT MODIFY THIS FILE.
 *
 *  For documentation and updates see http://www.imatix.com.
 *---------------------------------------------------------------------------*/

#include "sfl.h"
#include "smt3.h"
#include "gsl.h"                        /*  Project header file              */
#include "ggconv.h"                     /*  Include header file              */

/*- Macros ------------------------------------------------------------------*/

#define CONV_NAME "conv"                /*  Conversion Functions             */

#define matches(s,t)    (s ? (ignorecase ? lexcmp (s, t) == 0 : streq (s, t))   : t == NULL)

/*- Function prototypes -----------------------------------------------------*/

static int
conv_chr (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread);
static int
conv_number (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread);
static int
conv_ord (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread);
static int
conv_string (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread);

/*- Global variables --------------------------------------------------------*/
static PARM_LIST parm_list_v            = { PARM_VALUE };

static GSL_FUNCTION conv_functions [] =
{
    {"chr",            1, 1, 1, (void *) &parm_list_v, 1, conv_chr},
    {"number",         1, 1, 1, (void *) &parm_list_v, 1, conv_number},
    {"ord",            1, 1, 1, (void *) &parm_list_v, 1, conv_ord},
    {"string",         1, 1, 1, (void *) &parm_list_v, 1, conv_string}};

CLASS_DESCRIPTOR
    conv_class = {
        "conv",
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
        conv_functions, tblsize (conv_functions) };



static int
conv_chr (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread)
{
    RESULT_NODE *arg     = argc > 0 ? argv [0] : NULL;

    if (! arg)
      {
        strcpy (object_error, "Missing argument: arg");
        return -1;
      }
    if (arg-> value. type == TYPE_UNDEFINED)
      {
        result-> culprit = arg-> culprit;
        arg-> culprit = NULL;
        return 0;
      }

  {
    number_value (&arg-> value);
    
    if (arg-> value. type == TYPE_NUMBER)
      {
        result-> value. type  = TYPE_STRING;
        result-> value. s = mem_alloc (2);

        ASSERT (result-> value. s);
        
        if (arg-> value. n > 0
        && arg-> value. n < 256)
            result-> value. s [0] = (char) arg-> value. n;
        else
            result-> value. s [0] = '\0';

        result-> value. s [1] = '\0';
      }
    else
      {
        result-> culprit = arg-> culprit;
        arg-> culprit = NULL;
      }
    return 0;
  }

    return 0;  /*  Just in case  */
}


static int
conv_number (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread)
{
    RESULT_NODE *arg     = argc > 0 ? argv [0] : NULL;

    if (! arg)
      {
        strcpy (object_error, "Missing argument: arg");
        return -1;
      }
    if (arg-> value. type == TYPE_UNDEFINED)
      {
        result-> culprit = arg-> culprit;
        arg-> culprit = NULL;
        return 0;
      }

  {
    number_value (&arg-> value);
    
    if (arg-> value. type == TYPE_NUMBER)
        copy_value (&result-> value, &arg-> value);

    return 0;
  }

    return 0;  /*  Just in case  */
}


static int
conv_ord (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread)
{
    RESULT_NODE *arg     = argc > 0 ? argv [0] : NULL;

    if (! arg)
      {
        strcpy (object_error, "Missing argument: arg");
        return -1;
      }
    if (arg-> value. type == TYPE_UNDEFINED)
      {
        result-> culprit = arg-> culprit;
        arg-> culprit = NULL;
        return 0;
      }

  {
    string_value (&arg-> value);
    
    if (arg-> value. type == TYPE_STRING)
      {
        result-> value. type = TYPE_NUMBER;
        result-> value. n    = arg-> value. s [0];
      }
    else
      {
        result-> culprit = arg-> culprit;
        arg-> culprit = NULL;
      }
    return 0;
  }

    return 0;  /*  Just in case  */
}


static int
conv_string (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread)
{
    RESULT_NODE *arg     = argc > 0 ? argv [0] : NULL;

    if (! arg)
      {
        strcpy (object_error, "Missing argument: arg");
        return -1;
      }
    if (arg-> value. type == TYPE_UNDEFINED)
      {
        result-> culprit = arg-> culprit;
        arg-> culprit = NULL;
        return 0;
      }

  {
    if (arg-> value. type != TYPE_UNDEFINED)
      {
        string_value (&arg-> value);
        copy_value (&result-> value, &arg-> value);
      }

    return 0;
  }

    return 0;  /*  Just in case  */
}

static int conv_class_init (CLASS_DESCRIPTOR **class, void **item, THREAD *gsl_thread)
{
     *class = & conv_class;

    return 0;
}

int register_conv_classes (void)
{
    int
        rc = 0;
    rc |= object_register (conv_class_init,
                           NULL);
    return rc;
}
