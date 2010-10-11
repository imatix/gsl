/*---------------------------------------------------------------------------
 *  ggtime.c - GSL/time package
 *
 *  Generated from ggtime by ggobjt.gsl using GSL/4.
 *  DO NOT MODIFY THIS FILE.
 *
 *  For documentation and updates see http://www.imatix.com.
 *---------------------------------------------------------------------------*/

#include "sfl.h"
#include "smt3.h"
#include "gsl.h"                        /*  Project header file              */
#include "ggtime.h"                     /*  Include header file              */

/*- Macros ------------------------------------------------------------------*/

#define TIME_NAME "time"                /*  Time Functions                   */
#define DATE_NAME "date"                /*  Date Functions                   */

#define matches(s,t)    (s ? (ignorecase ? lexcmp (s, t) == 0 : streq (s, t))   : t == NULL)

/*- Function prototypes -----------------------------------------------------*/

static int
time_picture (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread);
static int
time_number (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread);
static int
gsl_now (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread);
static int
time_diff (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread);
static int
date_picture (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread);
static int
date_number (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread);
static VALUE * time_get_attr (void *item, const char *name, Bool ignorecase);
static VALUE * date_get_attr (void *item, const char *name, Bool ignorecase);

/*- Global variables --------------------------------------------------------*/
static PARM_LIST parm_list_v            = { PARM_VALUE };
static PARM_LIST parm_list_r            = { PARM_REFERENCE };

static GSL_FUNCTION time_functions [] =
{
    {"diff",           4, 4, 1, (void *) &parm_list_v, 1, time_diff},
    {"now",            0, 2, 1, (void *) &parm_list_r, 1, gsl_now},
    {"number",         1, 1, 1, (void *) &parm_list_v, 1, time_number},
    {"picture",        0, 2, 1, (void *) &parm_list_v, 1, time_picture}};

CLASS_DESCRIPTOR
    time_class = {
        "time",
        NULL,
        time_get_attr,
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
        time_functions, tblsize (time_functions) };

static GSL_FUNCTION date_functions [] =
{
    {"number",         1, 1, 1, (void *) &parm_list_v, 1, date_number},
    {"picture",        0, 2, 1, (void *) &parm_list_v, 1, date_picture}};

CLASS_DESCRIPTOR
    date_class = {
        "date",
        NULL,
        date_get_attr,
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
        date_functions, tblsize (date_functions) };


static VALUE * time_get_attr (void *item, const char *name, Bool ignorecase)
{

    static VALUE
        value;
        
    if (name == NULL || name [0] == 0)
      {

    assign_string (& value, conv_time_pict (time_now (), "hh:mm:ss"));
        
      }

    return & value;
        
}

static VALUE * date_get_attr (void *item, const char *name, Bool ignorecase)
{

    static VALUE
        value;
        
    if (name == NULL || name [0] == 0)
      {

    assign_string (& value, conv_date_pict (date_now (), "yyyy/mm/dd"));
        
      }

    return & value;
        
}


static int
time_picture (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread)
{
    RESULT_NODE *time    = argc > 0 ? argv [0] : NULL;
    RESULT_NODE *picture = argc > 1 ? argv [1] : NULL;


  {
    char
        *strptr;

    if (! arguments_are_defined (argc, argv, result))
        return 0;

    strptr = conv_time_pict (
                 time    ? (long) number_value (&time-> value)    : time_now (),
                 picture ?        string_value (&picture-> value) : "hh:mm:ss");

    if (strptr)
        assign_string (& result-> value, mem_strdup (strptr));
  }
        
    return 0;  /*  Just in case  */
}


static int
time_number (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread)
{
    RESULT_NODE *time    = argc > 0 ? argv [0] : NULL;

    if (! time)
      {
        strcpy (object_error, "Missing argument: time");
        return -1;
      }
    if (time-> value. type == TYPE_UNDEFINED)
      {
        result-> culprit = time-> culprit;
        time-> culprit = NULL;
        return 0;
      }

  {
    long
        ltime;

    if (! arguments_are_defined (argc, argv, result))
        return 0;

    ltime = conv_str_time (string_value (&time-> value));
    assign_number (& result-> value, ltime);
  }
        
    return 0;  /*  Just in case  */
}


static int
gsl_now (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread)
{
    RESULT_NODE *date    = argc > 0 ? argv [0] : NULL;
    RESULT_NODE *time    = argc > 1 ? argv [1] : NULL;


  {
    GGCODE_TCB
        *gsl_tcb = gsl_thread-> tcb;
    long
        cur_date,
        cur_time;
    VALUE
        value;
    int
        rc = 0;
    char
        *error_text;

    get_date_time_now (& cur_date, & cur_time);

    if (date)
      {
        init_value (& value);
        assign_number (& value, cur_date);
        rc = ! store_symbol_definition (& gsl_tcb-> scope_stack,
                                        gsl_tcb-> gsl-> ignorecase,
                                        date,
                                        &value,
                                        &error_text);
        destroy_value (& value);
      }
    if (time && (! rc))
      {
        init_value (& value);
        assign_number (& value, cur_time);
        rc = ! store_symbol_definition (& gsl_tcb-> scope_stack,
                                        gsl_tcb-> gsl-> ignorecase,
                                        time,
                                        &value,
                                        &error_text);
        destroy_value (& value);
      }

    if (rc)
      {
        strncpy (object_error, error_text, LINE_MAX);
        return rc;
      }
    else
        assign_number (& result-> value, cur_time);
  }
        
    return 0;  /*  Just in case  */
}


static int
time_diff (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread)
{
    RESULT_NODE *date1   = argc > 0 ? argv [0] : NULL;
    RESULT_NODE *time1   = argc > 1 ? argv [1] : NULL;
    RESULT_NODE *date2   = argc > 2 ? argv [2] : NULL;
    RESULT_NODE *time2   = argc > 3 ? argv [3] : NULL;

    if (! date1)
      {
        strcpy (object_error, "Missing argument: date1");
        return -1;
      }
    if (date1-> value. type == TYPE_UNDEFINED)
      {
        result-> culprit = date1-> culprit;
        date1-> culprit = NULL;
        return 0;
      }
    if (! time1)
      {
        strcpy (object_error, "Missing argument: time1");
        return -1;
      }
    if (time1-> value. type == TYPE_UNDEFINED)
      {
        result-> culprit = time1-> culprit;
        time1-> culprit = NULL;
        return 0;
      }
    if (! date2)
      {
        strcpy (object_error, "Missing argument: date2");
        return -1;
      }
    if (date2-> value. type == TYPE_UNDEFINED)
      {
        result-> culprit = date2-> culprit;
        date2-> culprit = NULL;
        return 0;
      }
    if (! time2)
      {
        strcpy (object_error, "Missing argument: time2");
        return -1;
      }
    if (time2-> value. type == TYPE_UNDEFINED)
      {
        result-> culprit = time2-> culprit;
        time2-> culprit = NULL;
        return 0;
      }

  {
    long
        days,
        csecs;

    if (! arguments_are_defined (argc, argv, result))
        return 0;

    date_diff ((long) number_value (& date1-> value),
               (long) number_value (& time1-> value),
               (long) number_value (& date2-> value),
               (long) number_value (& time2-> value),
               & days, &csecs);

    assign_number (& result-> value, days * INTERVAL_DAY + csecs);
  }
        
    return 0;  /*  Just in case  */
}


static int
date_picture (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread)
{
    RESULT_NODE *date    = argc > 0 ? argv [0] : NULL;
    RESULT_NODE *picture = argc > 1 ? argv [1] : NULL;


  {
    char
        *strptr;

    if (! arguments_are_defined (argc, argv, result))
        return 0;

    strptr = conv_date_pict (
                 date    ? (long) number_value (&date-> value)    : date_now (),
                 picture ?        string_value (&picture-> value) : "yyyy/mm/dd");
    if (strptr)
        assign_string (& result-> value, mem_strdup (strptr));
  }
    
    return 0;  /*  Just in case  */
}


static int
date_number (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread)
{
    RESULT_NODE *date    = argc > 0 ? argv [0] : NULL;

    if (! date)
      {
        strcpy (object_error, "Missing argument: date");
        return -1;
      }
    if (date-> value. type == TYPE_UNDEFINED)
      {
        result-> culprit = date-> culprit;
        date-> culprit = NULL;
        return 0;
      }

  {
    long
        ldate;

    if (! arguments_are_defined (argc, argv, result))
        return 0;

    ldate = conv_str_date (string_value (&date-> value),
                           FLAG_D_ORDER_YMD, DATE_YMD_COMPACT, DATE_ORDER_YMD);
    assign_number (& result-> value, ldate);
  }
    
    return 0;  /*  Just in case  */
}

static int time_class_init (CLASS_DESCRIPTOR **class, void **item, THREAD *gsl_thread)
{
     *class = & time_class;

    return 0;
}

static int date_class_init (CLASS_DESCRIPTOR **class, void **item, THREAD *gsl_thread)
{
     *class = & date_class;

    return 0;
}

int register_time_classes (void)
{
    int
        rc = 0;
    rc |= object_register (time_class_init,
                           NULL);
    rc |= object_register (date_class_init,
                           NULL);
    return rc;
}
