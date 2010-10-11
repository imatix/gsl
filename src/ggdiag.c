/*---------------------------------------------------------------------------
 *  ggdiag.c - GSL/diag package
 *
 *  Generated from ggdiag by ggobjt.gsl using GSL/4.
 *  DO NOT MODIFY THIS FILE.
 *
 *  For documentation and updates see http://www.imatix.com.
 *---------------------------------------------------------------------------*/

#include "sfl.h"
#include "smt3.h"
#include "gsl.h"                        /*  Project header file              */
#include "ggdiag.h"                     /*  Include header file              */

/*- Macros ------------------------------------------------------------------*/

#define DIAG_NAME "diag"                /*  Diagnostic Functions             */

#define matches(s,t)    (s ? (ignorecase ? lexcmp (s, t) == 0 : streq (s, t))   : t == NULL)

/*- Function prototypes -----------------------------------------------------*/

static int
diag_used (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread);
static int
diag_allocs (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread);
static int
diag_frees (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread);
static int
diag_display (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread);
static int
diag_checkall (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread);
static int
diag_raise (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread);
static int
diag_animate (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread);
static int
diag_console_set_mode (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread);

/*- Global variables --------------------------------------------------------*/
static PARM_LIST parm_list_v            = { PARM_VALUE };

static GSL_FUNCTION diag_functions [] =
{
    {"allocs",         0, 0, 0, NULL,            1, diag_allocs},
    {"animate",        1, 1, 1, (void *) &parm_list_v, 1, diag_animate},
    {"checkall",       0, 0, 0, NULL,            1, diag_checkall},
    {"console_set_mode", 1, 1, 1, (void *) &parm_list_v, 1, diag_console_set_mode},
    {"display",        1, 1, 1, (void *) &parm_list_v, 1, diag_display},
    {"frees",          0, 0, 0, NULL,            1, diag_frees},
    {"raise",          1, 1, 1, (void *) &parm_list_v, 1, diag_raise},
    {"used",           0, 0, 0, NULL,            1, diag_used}};

CLASS_DESCRIPTOR
    diag_class = {
        "diag",
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
        diag_functions, tblsize (diag_functions) };



static int
diag_used (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread)
{


  {
    result-> value. type = TYPE_NUMBER;
    result-> value. n    = mem_used ();

    return 0;
  }
    
    return 0;  /*  Just in case  */
}


static int
diag_allocs (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread)
{


  {
    result-> value. type = TYPE_NUMBER;
    result-> value. n    = mem_allocs ();

    return 0;
  }
    
    return 0;  /*  Just in case  */
}


static int
diag_frees (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread)
{


  {
    result-> value. type = TYPE_NUMBER;
    result-> value. n    = mem_frees ();

    return 0;
  }
    
    return 0;  /*  Just in case  */
}


static int
diag_display (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread)
{
    RESULT_NODE *filename = argc > 0 ? argv [0] : NULL;

    if (! filename)
      {
        strcpy (object_error, "Missing argument: filename");
        return -1;
      }
    if (filename-> value. type == TYPE_UNDEFINED)
      {
        result-> culprit = filename-> culprit;
        filename-> culprit = NULL;
        return 0;
      }

  {
    FILE
        *file;

    file = file_open (string_value (& filename-> value), 'w');
    if (! file)
      {
        strcpy (object_error, strerror (errno));
        return -1;
      }
    mem_display (file);
    file_close (file);

    return 0;
  }
    
    return 0;  /*  Just in case  */
}


static int
diag_checkall (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread)
{


    mem_checkall ();
    
    return 0;  /*  Just in case  */
}


static int
diag_raise (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread)
{
    RESULT_NODE *signal  = argc > 0 ? argv [0] : NULL;

    if (! signal)
      {
        strcpy (object_error, "Missing argument: signal");
        return -1;
      }
    if (signal-> value. type == TYPE_UNDEFINED)
      {
        result-> culprit = signal-> culprit;
        signal-> culprit = NULL;
        return 0;
      }

    raise ((int) number_value (& signal-> value));
    
    return 0;  /*  Just in case  */
}


static int
diag_animate (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread)
{
    RESULT_NODE *value   = argc > 0 ? argv [0] : NULL;

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

    gsl_thread-> animate = (Bool) number_value (& value-> value);
    
    return 0;  /*  Just in case  */
}


static int
diag_console_set_mode (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread)
{
    RESULT_NODE *mode    = argc > 0 ? argv [0] : NULL;

    if (! mode)
      {
        strcpy (object_error, "Missing argument: mode");
        return -1;
      }
    if (mode-> value. type == TYPE_UNDEFINED)
      {
        result-> culprit = mode-> culprit;
        mode-> culprit = NULL;
        return 0;
      }

    console_set_mode ((int) number_value (& mode-> value));
    
    return 0;  /*  Just in case  */
}

static int diag_class_init (CLASS_DESCRIPTOR **class, void **item, THREAD *gsl_thread)
{
     *class = & diag_class;

    return 0;
}

int register_diag_classes (void)
{
    int
        rc = 0;
    rc |= object_register (diag_class_init,
                           NULL);
    return rc;
}
