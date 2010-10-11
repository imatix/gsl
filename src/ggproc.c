/*---------------------------------------------------------------------------
 *  ggproc.c - GSL/process management
package
 *
 *  Generated from ggproc by ggobjt.gsl using GSL/4.
 *  DO NOT MODIFY THIS FILE.
 *
 *  For documentation and updates see http://www.imatix.com.
 *---------------------------------------------------------------------------*/

#include "sfl.h"
#include "smt3.h"
#include "gsl.h"                        /*  Project header file              */
#include "ggproc.h"                     /*  Include header file              */

/*- Macros ------------------------------------------------------------------*/

#define PROC_NAME "proc"                /*  Process                          */
#define PROC_HANDLE_NAME "proc handle"  /*  Process handle                   */

#define matches(s,t)    (s ? (ignorecase ? lexcmp (s, t) == 0 : streq (s, t))   : t == NULL)

/*- Function prototypes -----------------------------------------------------*/

static int
proc_new (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread);
static int
proc_handle_setenv (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread);
static int
proc_handle_getenv (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread);
static int
proc_handle_run (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread);
static int proc_link (void *item);
static int proc_destroy (void *item);
static VALUE * proc_get_attr (void *item, const char *name, Bool ignorecase);
static int proc_handle_link (void *item);
static int proc_handle_destroy (void *item);
static VALUE * proc_handle_get_attr (void *item, const char *name, Bool ignorecase);
static int proc_handle_put_attr (void *item, const char *name, VALUE *value, Bool ignorecase);

/*- Global variables --------------------------------------------------------*/
static PARM_LIST parm_list_v            = { PARM_VALUE };
static PARM_LIST parm_list_r            = { PARM_REFERENCE };

static GSL_FUNCTION proc_functions [] =
{
    {"new",            1, 5, 1, (void *) &parm_list_v, 1, proc_new}};

CLASS_DESCRIPTOR
    proc_class = {
        "proc",
        NULL,
        proc_get_attr,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        proc_destroy,
        proc_link,
        NULL,
        NULL,
        NULL,
        NULL,
        proc_functions, tblsize (proc_functions) };

static GSL_FUNCTION proc_handle_functions [] =
{
    {"getenv",         1, 1, 1, (void *) &parm_list_v, 1, proc_handle_getenv},
    {"run",            0, 1, 1, (void *) &parm_list_r, 0, proc_handle_run},
    {"setenv",         1, 2, 1, (void *) &parm_list_v, 1, proc_handle_setenv}};

CLASS_DESCRIPTOR
    proc_handle_class = {
        "proc handle",
        NULL,
        proc_handle_get_attr,
        proc_handle_put_attr,
        NULL,
        NULL,
        NULL,
        NULL,
        proc_handle_destroy,
        proc_handle_link,
        NULL,
        NULL,
        NULL,
        NULL,
        proc_handle_functions, tblsize (proc_handle_functions) };



#define POLL_INTERVAL        (10)        /*  Poll interval in csecs           */

#define PROCESS_NOT_STARTED  (-1)        /*  Extends macros  in sflproc.h     */

#define PROC_ERROR           (-1)

typedef struct {
    int
        links;
    char
        *error_msg;
} PROC_CONTEXT;

typedef struct _PROC_HANDLE_ITEM {
    struct _PROC_HANDLE_ITEM
        *next,
        *prev;
    int
        links;
    char
        *command,
        *workdir,
        *inname,
        *outname,
        *errname;
    SYMTAB
        *envt;
    PROCESS
        handle;
    int
        status;
    Bool
        waiting;                        /*  Is GSL script waiting?            */
    RESULT_NODE
        *error,                         /*  For GSL error parameter           */
        *result;                        /*  For GSL function return code      */
    THREAD
        *gsl_thread;
    char
        *error_msg;
} PROC_HANDLE_ITEM;

static LIST
    handle_list = { & handle_list, & handle_list };

static int
    running_count = 0;


static int
store_proc_error (PROC_HANDLE_ITEM *proc_handle,
                  THREAD           *gsl_thread,
                  RESULT_NODE      *error,
                  const char       *error_msg,
                        char      **error_text)
{
    GGCODE_TCB
        *gsl_tcb = gsl_thread-> tcb;
    PROC_CONTEXT
        *context;
    VALUE
        value;

    if (error_msg)
      {
        mem_free (proc_handle-> error_msg);
        proc_handle-> error_msg = memt_strdup (NULL, error_msg);

        context = get_class_item (gsl_thread, PROC_NAME);
        mem_free (context-> error_msg);
        context-> error_msg = memt_strdup (NULL, error_msg);

        if (error)
          {
            init_value (& value);
            assign_string (& value, context-> error_msg);

            if (! store_symbol_definition (& gsl_tcb-> scope_stack,
                                           gsl_tcb-> gsl-> ignorecase,
                                           error,
                                           &value,
                                           error_text))
                return -1;
          }
      }
    return 0;
}

static void
return_process_status (PROC_HANDLE_ITEM *process)
{
    assign_number (& process-> result-> value, process-> status);
    lsend_ggcode_call_ok (
        & process-> gsl_thread-> queue-> qid, NULL,
        NULL, NULL, NULL, 0);
}

static void
schedule_check_processes_status (void);

static int
check_processes_status (long dummy_date, long dummy_time, void *dummy_arg)
{
    PROC_HANDLE_ITEM
        *process;
    int
        status,
        rc = 0;

    FORLIST (process, handle_list)
      {
        if (process-> handle)
          {
            status = process_status (process-> handle);
            if (status != process-> status)
              {
                ASSERT (process-> status == PROCESS_RUNNING);
                running_count--;
                process-> status = status;
                if (process-> waiting)
                  {
                    return_process_status (process);
                    process_kill (process-> handle);
                    process-> handle = 0;
                    rc = 1;
                  }
              }
          }
      }
    if (running_count > 0)
        schedule_check_processes_status ();

    return rc;
}

static void
schedule_check_processes_status (void)
{
    long
        date = 0,
        time = 0;

    future_date (& date, & time, 0, POLL_INTERVAL);
    schedule_async_nonblock (check_processes_status, NULL,
                             SMT_PRIORITY_NORMAL, date, time);
}

static void
return_with_error (PROC_HANDLE_ITEM *process,
                   const char       *error_msg)
{
    char
        *error_text;

    assign_number (& process-> result-> value, PROC_ERROR);

    if (store_proc_error (process,
                          process-> gsl_thread,
                          process-> error,
                          error_msg,
                          &error_text))
        lsend_ggcode_call_error (& process-> gsl_thread-> queue-> qid, NULL,
                                 NULL, NULL, NULL, 0,
                                 NULL,
                                 0,
                                 error_text);
    else
        lsend_ggcode_call_ok (& process-> gsl_thread-> queue-> qid, NULL,
                              NULL, NULL, NULL, 0);
}


static int proc_link (void *item)
{
    
((PROC_CONTEXT *) item)-> links++;
return 0;
    
}

static int proc_destroy (void *item)
{
    
  {
    PROC_CONTEXT
        *context = item;

    if (--context-> links == 0)
      {
        mem_free (context-> error_msg);
        mem_free (context);
      }
    return 0;
  }
    
}

static VALUE * proc_get_attr (void *item, const char *name, Bool ignorecase)
{

    PROC_CONTEXT
        *context = item;
    static VALUE
        value;

    init_value (& value);
        
    if (matches (name, "error"))
      {

        if (context-> error_msg)
            assign_string (& value, context-> error_msg);
        
      }

    return & value;
        
}

static int proc_handle_link (void *item)
{
    
    if (item)
        ((PROC_HANDLE_ITEM *) item)-> links++;
    return 0;
    
}

static int proc_handle_destroy (void *item)
{
    
    PROC_HANDLE_ITEM
        *process = item;

    if (process
    &&  --process-> links <= 0)
      {
        if (process-> status == PROCESS_RUNNING)
            running_count--;
        mem_free (process-> command);
        mem_free (process-> workdir);
        mem_free (process-> inname);
        mem_free (process-> outname);
        mem_free (process-> errname);
        sym_delete_table (process-> envt);
        mem_free (process-> error_msg);
        list_unlink (process);
        mem_free (process);
      }
    return 0;
    
}

static VALUE * proc_handle_get_attr (void *item, const char *name, Bool ignorecase)
{

    PROC_HANDLE_ITEM
        *process = item;
    static VALUE
        value;

    init_value (& value);
        
    if (matches (name, "wait"))
      {

        assign_number (& value, process-> waiting);
        
      }
    else
    if (matches (name, "status"))
      {

        assign_number (& value, process-> status);
        
      }
    else
    if (matches (name, "error"))
      {

        if (process-> error_msg)
            assign_string (& value, process-> error_msg);
        
      }

    return & value;
        
}

static int proc_handle_put_attr (void *item, const char *name, VALUE *value, Bool ignorecase)
{

    PROC_HANDLE_ITEM
        *process = item;
    int
        rc = 0;

    number_value (value);
        
    if (matches (name, "wait"))
      {

        if (value-> type == TYPE_NUMBER
        &&  value-> n    >= 0
        &&  value-> n    <= 1)
            process-> waiting = (Bool) value-> n;
        else
            rc = -1;
        
      }

    else
        rc = -1;

    return rc;
        
}


static int
proc_new (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread)
{
    RESULT_NODE *command = argc > 0 ? argv [0] : NULL;
    RESULT_NODE *workdir = argc > 1 ? argv [1] : NULL;
    RESULT_NODE *inname  = argc > 2 ? argv [2] : NULL;
    RESULT_NODE *outname = argc > 3 ? argv [3] : NULL;
    RESULT_NODE *errname = argc > 4 ? argv [4] : NULL;

    if (! command)
      {
        strcpy (object_error, "Missing argument: command");
        return -1;
      }
    if (command-> value. type == TYPE_UNDEFINED)
      {
        result-> culprit = command-> culprit;
        command-> culprit = NULL;
        return 0;
      }

  {
    PROC_HANDLE_ITEM
        *process;

    list_create (process, sizeof (PROC_HANDLE_ITEM));
    list_relink_before (process, & handle_list);

    process-> links      = 0;
    process-> command    = memt_strdup (NULL, string_result (command));
    process-> workdir    = memt_strdup (NULL, string_result (workdir));
    process-> inname     = memt_strdup (NULL, string_result (inname));
    process-> outname    = memt_strdup (NULL, string_result (outname));
    process-> errname    = memt_strdup (NULL, string_result (errname));
    process-> envt       = sym_create_table ();
    process-> handle     = 0;
    process-> status     = PROCESS_NOT_STARTED;
    process-> waiting    = TRUE;
    process-> error      = NULL;
    process-> result     = NULL;
    process-> gsl_thread = NULL;
    process-> error_msg  = NULL;

    assign_pointer (& result-> value, & proc_handle_class, process);

    return 0;
  }
        
    return 0;  /*  Just in case  */
}


static int
proc_handle_setenv (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread)
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
    SYMBOL
        *symbol;

    if (((PROC_HANDLE_ITEM *) item)-> status != PROCESS_NOT_STARTED)
      {
        strcpy (object_error,
               "Cannot set environment variable on running process.");
        return -1;
      }
    if (value && value-> value. type != TYPE_UNDEFINED)
        sym_assume_symbol (((PROC_HANDLE_ITEM *) item)-> envt,
                           string_value (& name-> value),
                           string_value (& value-> value));
    else
      {
        symbol = sym_lookup_symbol (((PROC_HANDLE_ITEM *) item)-> envt,
                                    string_value (& name-> value));
        if (symbol)
            sym_delete_symbol (((PROC_HANDLE_ITEM *) item)-> envt,
                               symbol);
      }
  }
        
    return 0;  /*  Just in case  */
}


static int
proc_handle_getenv (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread)
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
    char
        *value;

    if (((PROC_HANDLE_ITEM *) item)-> status != PROCESS_NOT_STARTED)
      {
        strcpy (object_error,
               "Cannot set environment variable on running process.");
        return -1;
      }
    value = sym_get_value (((PROC_HANDLE_ITEM *) item)-> envt,
                           string_value (& name-> value), NULL);
    assign_string (& result-> value, memt_strdup (NULL, value));
  }
        
    return 0;  /*  Just in case  */
}


static int
proc_handle_run (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread)
{
    RESULT_NODE *error   = argc > 0 ? argv [0] : NULL;


  {
    PROCESS_DATA
        procinfo = PROCESS_DATA_INIT;
    PROC_HANDLE_ITEM
        *process = item;
    int
        rc;

    process-> error      = error;
    process-> result     = result;
    process-> gsl_thread = gsl_thread;

    rc = process_setinfo (& procinfo,
                          process-> inname,
                          process-> outname,
                          TRUE,
                          process-> errname,
                          TRUE);
    if (rc)
      {
        return_with_error (process,
                           "Error redirecting stdin, stdout or stderrn");
        return 0;
      }

    procinfo. filename = process-> command;
    procinfo. wait     = FALSE;
    procinfo. workdir  = process-> workdir;
    procinfo. envv     = symb2strt (process-> envt);

    process-> handle = process_create_full (& procinfo);

    strtfree (procinfo.envv);

    if (process-> handle == 0)          /*  Process wasn't created  */
      {
        return_with_error (process,
                           strerror (procinfo. error));
        return 0;
      }
    process-> status = PROCESS_RUNNING;
    if (running_count == 0)              /*  Otherwise already scheduled      */
        schedule_check_processes_status ();

    running_count++;

    if (! process-> waiting)
        lsend_ggcode_call_ok (& process-> gsl_thread-> queue-> qid, NULL,
                              NULL, NULL, NULL, 0);

    /*  Build return value  */
    assign_number (& result-> value, 0);
}
        
    return 0;  /*  Just in case  */
}

static int proc_class_init (CLASS_DESCRIPTOR **class, void **item, THREAD *gsl_thread)
{
     *class = & proc_class;


  {
    PROC_CONTEXT
        *context;

    context = memt_alloc (NULL, sizeof (PROC_CONTEXT));
    context-> links     = 0;
    context-> error_msg = NULL;

    *item = context;

    /*  Ensure that timer agent is running, else start it up                 */
    if (smttime_init ())
        return (-1);
}
    
    return 0;
}

int register_proc_classes (void)
{
    int
        rc = 0;
    rc |= object_register (proc_class_init,
                           NULL);
    return rc;
}
