/*---------------------------------------------------------------------------
 *  ggthrd.c - GSL/thrd package
 *
 *  Generated from ggthrd by ggobjt.gsl using GSL/4.
 *  DO NOT MODIFY THIS FILE.
 *
 *  For documentation and updates see http://www.imatix.com.
 *---------------------------------------------------------------------------*/

#include "sfl.h"
#include "smt3.h"
#include "gsl.h"                        /*  Project header file              */
#include "ggthrd.h"                     /*  Include header file              */
#include "ggthrd.d"                     /*  Include dialog data              */

#define AGENT_NAME "GGTHRD"

/*- Macros ------------------------------------------------------------------*/

#define THREAD_NAME "thread"            /*  Thread                           */
#define REMOTE_THREAD_NAME "remote thread"  /*  Thread                       */
#define CHILD_THREAD_NAME "child thread"  /*  Thread                         */
#define PARSED_ITEM_NAME "parsed item"  /*  Thread                           */

#define matches(s,t)    (s ? (ignorecase ? lexcmp (s, t) == 0 : streq (s, t))   : t == NULL)

/*- Function prototypes -----------------------------------------------------*/

static int
thread_parse (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread);
static int
thread_new (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread);
static int
thread_sleep (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread);
static int
thread_receive (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread);
static int
remote_thread_send (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread);
static int
child_thread_interrupt (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread);
static int
child_thread_send (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread);
static int
parsed_item_run (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread);
static int thread_link (void *item);
static int threaddestroy (void *item);
static VALUE * thread_get_attr (void *item, const char *name, Bool ignorecase);
static int remote_thread_link (void *item);
static int remote_thread_destroy (void *item);
static VALUE * remote_thread_get_attr (void *item, const char *name, Bool ignorecase);
static int child_thread_link (void *item);
static int child_thread_destroy (void *item);
static VALUE * child_thread_get_attr (void *item, const char *name, Bool ignorecase);
static int parsed_item_link (void *item);
static int parsed_item_destroy (void *item);
static VALUE * parsed_item_get_attr (void *item, const char *name, Bool ignorecase);

/*- Global variables --------------------------------------------------------*/
static PARM_LIST parm_list_vr           = { PARM_VALUE,
                                            PARM_REFERENCE };
static PARM_LIST parm_list_r            = { PARM_REFERENCE };

static GSL_FUNCTION thread_functions [] =
{
    {"new",            1, 2, 2, (void *) &parm_list_vr, 0, thread_new},
    {"parse",          1, 2, 2, (void *) &parm_list_vr, 0, thread_parse},
    {"receive",        0, 0, 1, (void *) &parm_list_r, 0, thread_receive},
    {"sleep",          1, 1, 1, (void *) &parm_list_vr, 0, thread_sleep}};

CLASS_DESCRIPTOR
    thread_class = {
        "thread",
        NULL,
        thread_get_attr,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        threaddestroy,
        thread_link,
        NULL,
        NULL,
        NULL,
        NULL,
        thread_functions, tblsize (thread_functions) };

static GSL_FUNCTION remote_thread_functions [] =
{
    {"send",           0, 0, 0, (void *) &parm_list_r, 1, remote_thread_send}};

CLASS_DESCRIPTOR
    remote_thread_class = {
        "remote thread",
        NULL,
        remote_thread_get_attr,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        remote_thread_destroy,
        remote_thread_link,
        NULL,
        NULL,
        NULL,
        NULL,
        remote_thread_functions, tblsize (remote_thread_functions) };

static GSL_FUNCTION child_thread_functions [] =
{
    {"interrupt",      0, 0, 0, NULL,            1, child_thread_interrupt},
    {"send",           0, 0, 0, (void *) &parm_list_r, 1, child_thread_send}};

CLASS_DESCRIPTOR
    child_thread_class = {
        "child thread",
        NULL,
        child_thread_get_attr,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        child_thread_destroy,
        child_thread_link,
        NULL,
        NULL,
        NULL,
        NULL,
        child_thread_functions, tblsize (child_thread_functions) };

static GSL_FUNCTION parsed_item_functions [] =
{
    {"run",            0, 1, 1, (void *) &parm_list_r, 1, parsed_item_run}};

CLASS_DESCRIPTOR
    parsed_item_class = {
        "parsed item",
        NULL,
        parsed_item_get_attr,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        parsed_item_destroy,
        parsed_item_link,
        NULL,
        NULL,
        NULL,
        NULL,
        parsed_item_functions, tblsize (parsed_item_functions) };


#define THREAD_INTERRUPT "thread_interrupt"

#define THREAD_PARSED      0
#define THREAD_RUNNING     1
#define THREAD_FINISHED    2
#define THREAD_INTERRUPTED 3
#define THREAD_ERROR      -1

typedef struct {
    int
        links;
    int
        error_line;                      /*  Line number of error.            */
    char
        *error_text,                     /*  Error message.                   */
        *error_file;                     /*  Source file name of error.       */
    LIST
        messages;                        /*  Message queue for this thread    */
    Bool
        waiting;                         /*  Waiting on a message arrival?    */
    THREAD
        *gsl_thread;                     /*  The thread - messy               */
    QUEUE
        *replyq;
    int
        argc;
    RESULT_NODE
        **argv;
} THREAD_CONTEXT;

typedef struct _MESSAGE {
    struct _MESSAGE
        *next,
        *prev;
    THREAD_CONTEXT
        *sender;
    int
        val_count;
    VALUE
        *val;
} MESSAGE;

typedef struct {
    int
        links;
    THREAD_CONTEXT
        *context;
    THREAD
        *controller_thread;              /*  The GGTHRD thread                */
    SCRIPT_HANDLE
        *script_handle;
    int
        status,
        error_line;                      /*  Line number of error.            */
    char
        *error_text,                     /*  Error message.                   */
        *error_file;                     /*  Source file name of error.       */
} THREAD_HANDLE_ITEM;

typedef struct {
    event_t
        thread_type;
    THREAD
        *parent_thread,                  /*  The GSL thread that created us   */
        *child_thread;                   /*  The GSL thread that we created   */
    RESULT_NODE
        *error,                          /*  For GSL error parameter          */
        *result;                         /*  For GSL function return code     */
    THREAD_HANDLE_ITEM
        *thread_handle;
    THREAD_CONTEXT
        *parent_context;
} TCB;

static TCB
    *tcb;                               /*  Address thread context block     */

/*****************************************************************************/

static THREAD_HANDLE_ITEM*
create_thread_handle (void)
{
    THREAD_HANDLE_ITEM
        *handle;

    handle = memt_alloc (NULL, sizeof (THREAD_HANDLE_ITEM));

    handle-> links             = 0;
    handle-> context           = NULL;
    handle-> controller_thread = NULL;
    handle-> script_handle     = NULL;
    handle-> status            = THREAD_PARSED;
    handle-> error_line        = 0;
    handle-> error_text        = NULL;
    handle-> error_file        = NULL;

    return handle;
}

static THREAD*
create_controller_thread (THREAD         *gsl_thread,
                          THREAD_CONTEXT *context,
                          char           *command,
                          event_t         thread_type,
                          RESULT_NODE    *result,
                          RESULT_NODE    *error)
{
    GGCODE_TCB
        *gsl_tcb = gsl_thread-> tcb;
    THREAD
        *thread;

    thread = thread_create (AGENT_NAME, "");
    tcb = thread-> tcb;

    tcb-> thread_type    = thread_type;
    tcb-> parent_thread  = gsl_thread;
    tcb-> child_thread   = NULL;
    tcb-> error          = error;
    tcb-> result         = result;
    tcb-> parent_context = context;
    context-> links++;
    context-> replyq     = gsl_tcb-> replyq;

    tcb-> thread_handle                     = create_thread_handle ();
    tcb-> thread_handle-> controller_thread = thread;
    tcb-> thread_handle-> links++;       /*  Keep this link  */

    if (command)
        script_load_string (strprintf ("(%s %u)",
                                       gsl_cur_script (gsl_thread),
                                       gsl_cur_line (gsl_thread)),
                            command,
                            gsl_tcb-> gsl-> line-> template,
                            (Bool) (gsl_tcb-> execute_full == 0),
                            tcb-> result,
                            thread-> queue);

    return thread;
}

static int
store_module_error (THREAD_CONTEXT *context,
                    RESULT_NODE    *error,
                    GGCODE_TCB     *gsl_tcb,
                    const char     *error_text,
                    const char     *error_file,
                    int             error_line,
                    char          **store_error_text)
{
    VALUE
        value;

    init_value (& value);
    mem_free (context-> error_text);
    mem_free (context-> error_file);
    context-> error_text = memt_strdup (NULL, error_text);
    context-> error_file = memt_strdup (NULL, error_file);
    context-> error_line = error_line;

    assign_string (& value, strprintf ("(%s %u) %s",
                                       error_file,
                                       error_line,
                                       error_text));

    if (error)
        if (! store_symbol_definition (& gsl_tcb-> scope_stack,
                                       gsl_tcb-> gsl-> ignorecase,
                                       error,
                                       &value,
                                       store_error_text))
            return -1;

    return 0;
}

static int
store_thread_error (THREAD_HANDLE_ITEM *thread_handle,
                    THREAD_CONTEXT     *context,
                    RESULT_NODE        *error,
                    GGCODE_TCB         *gsl_tcb,
                    const char         *error_text,
                    const char         *error_file,
                    int                 error_line,
                    char              **store_error_text)
{
    thread_handle-> status = THREAD_ERROR;
    if (error_text)
      {
        mem_free (thread_handle-> error_text);
        mem_free (thread_handle-> error_file);
        thread_handle-> error_text = memt_strdup (NULL, error_text);
        thread_handle-> error_file = memt_strdup (NULL, error_file);
        thread_handle-> error_line = error_line;

        return store_module_error (context,
                                   error,
                                   gsl_tcb,
                                   error_text,
                                   error_file,
                                   error_line,
                                   store_error_text);
      }
    return 0;
}

static int
wakeup (long dummy_date, long dummy_time, void *gsl_thread)
{
    lsend_ggcode_call_ok (
        & ((THREAD *) gsl_thread)-> queue-> qid, NULL,
        NULL, NULL, NULL, 0);
    return 1;
}

static void
message_destroy (MESSAGE *message)
{
    int
        i;

    for (i = 0; i < message-> val_count; i++)
      {
        mem_free (message-> val[i]. s);
        mem_free (message-> val[i]. b);
        if (message-> val[i]. type == TYPE_POINTER
        &&  message-> val[i]. c-> destroy)
            message-> val[i]. c-> destroy (message-> val[i]. i);
      }
    mem_free (message-> val);
    list_unlink (message);
    mem_free (message);
}


static void
message_receive (MESSAGE *message,
                 int argc, RESULT_NODE **argv,
                 THREAD *gsl_thread)
{
    GGCODE_TCB
        *gsl_tcb = gsl_thread-> tcb;
    VALUE
        value;
    char
        *error_text;
    int
        i;

    if (argc > 0
    &&  argv [0])                        /*  Was a 'sender' reference given?  */
      {
        init_value (& value);

        /*  Build value manually rather than use assign_pointer because   */
        /*  don't want a link made.                                       */
        value. type = TYPE_POINTER;
        value. c    = &remote_thread_class;
        value. i    = message-> sender;

        if (! store_symbol_definition (& gsl_tcb-> scope_stack,
                                       gsl_tcb-> gsl-> ignorecase,
                                       argv [0],
                                       &value,
                                       &error_text))
          {
            lsend_ggcode_call_error (
                & gsl_thread-> queue-> qid, NULL,
                NULL, NULL, NULL, 0,
                NULL, 0,
                error_text);
            return;
          }
      }
    for (i = 0; i < message-> val_count; i++)
      {
        if (i + 1 < argc
        &&  argv [i + 1])
            if (! store_symbol_definition (& gsl_tcb-> scope_stack,
                                           gsl_tcb-> gsl-> ignorecase,
                                           argv [i + 1],
                                           & message-> val [i],
                                           &error_text))
              {
                lsend_ggcode_call_error (
                    & gsl_thread-> queue-> qid, NULL,
                    NULL, NULL, NULL, 0,
                    NULL, 0,
                    error_text);
                return;
              }
      }

    message_destroy (message);

    lsend_ggcode_call_ok (
        & gsl_thread-> queue-> qid, NULL,
        NULL, NULL, NULL, 0);
}


static void
message_create (THREAD_CONTEXT *local_context,
                THREAD_CONTEXT *remote_context,
                int argc, RESULT_NODE **argv)
{
    MESSAGE
        *message;
    int
        i;

    list_create (message, sizeof (MESSAGE));
    message-> sender    = local_context;
    message-> val_count = argc;
    message-> val = memt_alloc (NULL, argc * sizeof (VALUE));
    for (i = 0; i < argc; i++)
      {
        init_value (& message-> val [i]);
        copy_value (& message-> val [i], & argv [i]-> value);
      }
    list_relink_before (message, & remote_context-> messages);

    if (remote_context-> waiting)
      {
        remote_context-> waiting = FALSE;
        message_receive (message,
                         remote_context-> argc,
                         remote_context-> argv,
                         remote_context-> gsl_thread);
        remote_context-> gsl_thread = NULL;   /*  No longer valid  */
      }
}


/*************************   INITIALISE THE THREAD   *************************/

MODULE initialise_the_thread (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    the_next_event = tcb-> thread_type;
}


/******************************   LOG MESSAGE   ******************************/

MODULE log_message (THREAD *thread)
{
    struct_ggcode_call_error
        *error;
    char
        *store_error_text;

    tcb = thread-> tcb;                 /*  Point to thread's context        */

    get_ggcode_call_error (thread-> event-> body, & error);

    if (store_thread_error (tcb-> thread_handle,
                            tcb-> parent_context,
                            tcb-> error,
                            tcb-> parent_thread-> tcb,
                            error-> error_text,
                            error-> error_name,
                            error-> error_line,
                            &store_error_text))
        lsend_ggcode_call_error (& tcb-> parent_thread-> queue-> qid, NULL,
                                 NULL, NULL, NULL, 0,
                                 NULL, 0,
                                 store_error_text);
    else
        lsend_ggcode_call_message (& tcb-> parent_thread-> queue-> qid, NULL,
                                   NULL, NULL, NULL, 0,
                                   error-> error_name,
                                   error-> error_line,
                                   error-> error_text);

    free_ggcode_call_error (& error);
    tcb = thread-> tcb;                 /*  Point to thread's context        */
}


/*************************   REPLY PARSED OK RESULT   ************************/

MODULE reply_parsed_ok_result (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    tcb-> thread_handle-> script_handle = tcb-> result-> value. i;

    /*  Set return value for function call */
    init_value (& tcb-> result-> value);
    assign_pointer (& tcb-> result-> value,
                    & parsed_item_class, tcb-> thread_handle);

    lsend_ggcode_call_ok (& tcb-> parent_thread-> queue-> qid, NULL,
                          NULL, NULL, NULL, 0);
}


/************************   REPLY RUNNING OK RESULT   ************************/

MODULE reply_running_ok_result (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    tcb-> thread_handle-> script_handle = tcb-> result-> value. i;

    /*  Set return value for function call */
    assign_pointer (& tcb-> result-> value,
                    & child_thread_class, tcb-> thread_handle);

    lsend_ggcode_call_ok (& tcb-> parent_thread-> queue-> qid, NULL,
                          NULL, NULL, NULL, 0);
}


/************************   REPLY PARSE ERROR RESULT   ***********************/

MODULE reply_parse_error_result (THREAD *thread)
{
    struct_ggcode_call_error
        *error;
    char
        *store_error_text;

    tcb = thread-> tcb;                 /*  Point to thread's context        */

    get_ggcode_call_error (thread-> event-> body, & error);

    if (store_thread_error (tcb-> thread_handle,
                            tcb-> parent_context,
                            tcb-> error,
                            tcb-> parent_thread-> tcb,
                            error-> error_text,
                            error-> error_name,
                            error-> error_line,
                            &store_error_text))
        lsend_ggcode_call_error (& tcb-> parent_thread-> queue-> qid, NULL,
                                 NULL, NULL, NULL, 0,
                                 NULL, 0,
                                 store_error_text);
    else
        lsend_ggcode_call_ok (& tcb-> parent_thread-> queue-> qid, NULL,
                              NULL, NULL, NULL, 0);

    free_ggcode_call_error (& error);
}


/***************************   DESTROY THE THREAD   **************************/

MODULE destroy_the_thread (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    mem_free (tcb-> thread_handle-> error_text);
    mem_free (tcb-> thread_handle-> error_file);
    mem_free (tcb-> thread_handle);
    tcb-> thread_handle = NULL;
}


/***********************   SET THREAD FINISHED STATUS   **********************/

MODULE set_thread_finished_status (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    tcb-> child_thread           = NULL;
    tcb-> thread_handle-> status = THREAD_FINISHED;
}


/************************   SET THREAD ERROR STATUS   ************************/

MODULE set_thread_error_status (THREAD *thread)
{
    struct_ggcode_error_reply
        *error_reply;

    tcb = thread-> tcb;                 /*  Point to thread's context        */

    tcb-> child_thread = NULL;

    event_send (& tcb-> parent_context-> replyq-> qid,
                NULL,
                thread-> event-> name,
                thread-> event-> body,
                thread-> event-> body_size,
                NULL, NULL, NULL, 0);

    get_ggcode_error_reply (thread-> event-> body, & error_reply);
    store_thread_error (tcb-> thread_handle,
                        tcb-> parent_context,
                        NULL,
                        NULL,
                        error_reply-> error_text,
                        error_reply-> error_name,
                        error_reply-> error_line,
                        NULL);

    free_ggcode_error_reply (& error_reply);
}


/****************************   SPAWN GSL THREAD   ***************************/

MODULE spawn_gsl_thread (THREAD *thread)
{
    THREAD_CONTEXT
        *context;

    tcb = thread-> tcb;                 /*  Point to thread's context        */

    /*  Bump up the number of links so source doesn't get deleted when the   */
    /*  spawned script finishes.                                             */
    script_handle_link (tcb-> thread_handle-> script_handle);

    tcb-> thread_handle-> status = THREAD_RUNNING;

    tcb-> child_thread = gsl_spawn
                           (tcb-> parent_thread,
                            thread-> queue,
                            tcb-> thread_handle-> script_handle);

    context = get_class_item (tcb-> child_thread, THREAD_NAME);

    tcb-> thread_handle-> context = context;
    tcb-> thread_handle-> context-> links++;

    tcb-> parent_thread = NULL;          /*  Assume parent thread finished   */
}


/**********************   SHUTDOWN RUNNING GSL THREAD   **********************/

MODULE shutdown_running_gsl_thread (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    if (tcb-> child_thread)
      {
        if (! tcb-> child_thread-> queue-> shutdown)
          {
            tcb-> thread_handle-> status = THREAD_INTERRUPTED;

            tcb-> child_thread-> queue-> shutdown = TRUE;
            lsend_smtlib_shutdown (& tcb-> child_thread-> queue-> qid,
                                   NULL,
                                   NULL, NULL, NULL, 0,
                                   0);      /*  Don't specify a signal           
*/          }
      }
}


/**************************   TERMINATE THE THREAD   *************************/

MODULE terminate_the_thread (THREAD *thread)
{
    tcb = thread-> tcb;                  /*  Point to thread's context       */


    if (tcb-> thread_handle)
      {
        tcb-> thread_handle-> controller_thread = NULL;
        child_thread_destroy (tcb-> thread_handle);    /*  Remove this link  */
      }

    threaddestroy (tcb-> parent_context);

    the_next_event = terminate_event;
}

static int thread_link (void *item)
{
    
((THREAD_CONTEXT *) item)-> links++;
return 0;
    
}

static int threaddestroy (void *item)
{
    
  {
    THREAD_CONTEXT
        *context = item;

    if (--context-> links == 0)
      {
        mem_free (context-> error_text);
        mem_free (context-> error_file);
        while (! list_empty (& context-> messages))
            message_destroy ((MESSAGE *) context-> messages. next);

        mem_free (context);
      }
    return 0;
  }
    
}

static VALUE * thread_get_attr (void *item, const char *name, Bool ignorecase)
{

    THREAD_CONTEXT
        *context = item;
    static VALUE
        value;

    init_value (& value);
        
    if (matches (name, "error_text"))
      {

        if (context-> error_text)
            assign_string (& value, context-> error_text);
        
      }
    else
    if (matches (name, "error_file"))
      {

        if (context-> error_text)
            assign_string (& value, context-> error_file);
        
      }
    else
    if (matches (name, "error_line"))
      {

        if (context-> error_text)
            assign_number (& value, context-> error_line);
        
      }
    else
    if (matches (name, "error"))
      {

        if (context-> error_text)
            assign_string (& value, strprintf ("(%s %u) %s",
                                               context-> error_file,
                                               context-> error_line,
                                               context-> error_text));
        
      }

    return & value;
        
}

static int remote_thread_link (void *item)
{
    
    return thread_link (item);
    
}

static int remote_thread_destroy (void *item)
{
    
    return threaddestroy (item);
    
}

static VALUE * remote_thread_get_attr (void *item, const char *name, Bool ignorecase)
{

    return thread_get_attr (item, name, ignorecase);
    
}

static int child_thread_link (void *item)
{
    
((THREAD_HANDLE_ITEM *) item)-> links++;
return 0;
    
}

static int child_thread_destroy (void *item)
{
    
  {
    THREAD_HANDLE_ITEM
        *thread_handle = item;

    if (--thread_handle-> links == 0)
      {
        if (thread_handle-> script_handle)
            script_handle_destroy (thread_handle-> script_handle);
        if (thread_handle-> context)
            threaddestroy (thread_handle-> context);
        mem_free (thread_handle-> error_text);
        mem_free (thread_handle-> error_file);
        mem_free (thread_handle);
      }
    return 0;
  }
    
}

static VALUE * child_thread_get_attr (void *item, const char *name, Bool ignorecase)
{

    THREAD_HANDLE_ITEM
        *thread_handle = item;
    static VALUE
        value;

    init_value (& value);
        
    if (matches (name, "status"))
      {

        assign_number (& value, thread_handle-> status);
        
      }
    else
    if (matches (name, "error_text"))
      {

        if (thread_handle-> error_text)
            assign_string (& value, thread_handle-> error_text);
        
      }
    else
    if (matches (name, "error_file"))
      {

        if (thread_handle-> error_text)
            assign_string (& value, thread_handle-> error_file);
        
      }
    else
    if (matches (name, "error_line"))
      {

        if (thread_handle-> error_text)
            assign_number (& value, thread_handle-> error_line);
        
      }
    else
    if (matches (name, "error"))
      {

        if (thread_handle-> error_text)
            assign_string (& value, strprintf ("(%s %u) %s",
                                               thread_handle-> error_file,
                                               thread_handle-> error_line,
                                               thread_handle-> error_text));
        
      }
    else
    if (matches (name, "status"))
      {

        assign_number (& value, thread_handle-> status);
        
      }

    return & value;
        
}

static int parsed_item_link (void *item)
{
    
((THREAD_HANDLE_ITEM *) item)-> links++;
return 0;
    
}

static int parsed_item_destroy (void *item)
{
    
  {
    THREAD_HANDLE_ITEM
        *thread_handle = item;

    if (--thread_handle-> links == 0)
      {
        script_handle_destroy (thread_handle-> script_handle);
        mem_free (thread_handle-> error_text);
        mem_free (thread_handle-> error_file);
        mem_free (thread_handle);
      }
    return 0;
  }
    
}

static VALUE * parsed_item_get_attr (void *item, const char *name, Bool ignorecase)
{

    return child_thread_get_attr (item, name, ignorecase);
    
}


static int
thread_parse (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread)
{
    RESULT_NODE *command = argc > 0 ? argv [0] : NULL;
    RESULT_NODE *error   = argc > 1 ? argv [1] : NULL;

    if (! command)
      {
        strcpy (object_error, "Missing argument: command");
        return -1;
      }
    if (command-> value. type == TYPE_UNDEFINED)
      {
        result-> culprit = command-> culprit;
        command-> culprit = NULL;
        lsend_ggcode_call_ok (& gsl_thread-> queue-> qid, NULL,
                             NULL, NULL, NULL, 0);
        return 0;
      }

    create_controller_thread (gsl_thread,
                         item,
                         string_value (&command-> value),
                         parse_event,
                         result,
                         error);
    return 0;
        
    return 0;  /*  Just in case  */
}


static int
thread_new (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread)
{
    RESULT_NODE *command = argc > 0 ? argv [0] : NULL;
    RESULT_NODE *error   = argc > 1 ? argv [1] : NULL;

    if (! command)
      {
        strcpy (object_error, "Missing argument: command");
        return -1;
      }
    if (command-> value. type == TYPE_UNDEFINED)
      {
        result-> culprit = command-> culprit;
        command-> culprit = NULL;
        lsend_ggcode_call_ok (& gsl_thread-> queue-> qid, NULL,
                             NULL, NULL, NULL, 0);
        return 0;
      }

    create_controller_thread (gsl_thread,
                              item,
                              string_value (&command-> value),
                              parse_and_run_event,
                              result,
                              error);
    return 0;
        
    return 0;  /*  Just in case  */
}


static int
thread_sleep (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread)
{
    RESULT_NODE *csecs   = argc > 0 ? argv [0] : NULL;

    if (! csecs)
      {
        strcpy (object_error, "Missing argument: csecs");
        return -1;
      }
    if (csecs-> value. type == TYPE_UNDEFINED)
      {
        result-> culprit = csecs-> culprit;
        csecs-> culprit = NULL;
        lsend_ggcode_call_ok (& gsl_thread-> queue-> qid, NULL,
                             NULL, NULL, NULL, 0);
        return 0;
      }

  {
    long
        date = 0,
        time = 0;

    number_value (& csecs-> value);
    if (csecs-> value. type != TYPE_NUMBER
    || (csecs-> value. n < 0
    ||  csecs-> value. n > ULONG_MAX))
      {
        strprintf (object_error,
                   "Illegal delay value for proc.sleep: %s",
                   string_value (& csecs-> value));
        return -1;
      }
    future_date (& date, & time, 0, (qbyte) csecs-> value. n);
    schedule_async_nonblock (wakeup, gsl_thread,
                             SMT_PRIORITY_NORMAL, date, time);
  }
        
    return 0;  /*  Just in case  */
}


static int
thread_receive (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread)
{


  {
    THREAD_CONTEXT
        *context = item;

    if (list_empty (& context-> messages))
      {
        context-> waiting    = TRUE;
        context-> gsl_thread = gsl_thread;
        context-> argc       = argc;
        context-> argv       = argv;
        return 0;
      }

    message_receive ((MESSAGE *) context-> messages. next,
                     argc, argv,
                     gsl_thread);
    context-> gsl_thread = NULL;         /*  No longer valid  */
  }
        
    return 0;  /*  Just in case  */
}


static int
remote_thread_send (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread)
{


  {
    THREAD_CONTEXT
        *remote_context = item,
        *local_context;

    local_context = get_class_item (gsl_thread, THREAD_NAME);

    message_create (local_context, remote_context, argc, argv);
  }
        
    return 0;  /*  Just in case  */
}


static int
child_thread_interrupt (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread)
{


  {
    THREAD_HANDLE_ITEM
        *thread_handle = item;

    if (thread_handle-> status == THREAD_RUNNING)
        event_send (& thread_handle-> controller_thread-> queue-> qid,
                    NULL,
                    THREAD_INTERRUPT,
                    NULL, 0,
                    NULL, NULL, NULL, 0);
  }
        
    return 0;  /*  Just in case  */
}


static int
child_thread_send (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread)
{


  {
    THREAD_HANDLE_ITEM
        *thread_handle = item;
    THREAD_CONTEXT
        *local_context,
        *remote_context = thread_handle-> context;

    local_context = get_class_item (gsl_thread, THREAD_NAME);

    message_create (local_context, remote_context, argc, argv);
  }
        
    return 0;  /*  Just in case  */
}


static int
parsed_item_run (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread)
{
    RESULT_NODE *error   = argc > 0 ? argv [0] : NULL;


  {
    THREAD
        *thread;
    THREAD_HANDLE_ITEM
        *handle = item;
    THREAD_CONTEXT
        *local_context;

    local_context = get_class_item (gsl_thread, THREAD_NAME);

    thread = create_controller_thread (gsl_thread,
                                       local_context,
                                       NULL,
                                       run_event,
                                       result,
                                       error);
    tcb = thread-> tcb;

    tcb-> thread_handle-> script_handle = handle-> script_handle;
    script_handle_link (tcb-> thread_handle-> script_handle);

    assign_pointer (& tcb-> result-> value,
                    & child_thread_class,
                    tcb-> thread_handle);

    return 0;
  }
        
    return 0;  /*  Just in case  */
}

static int thread_class_init (CLASS_DESCRIPTOR **class, void **item, THREAD *gsl_thread)
{
     *class = & thread_class;


  {
    THREAD_CONTEXT
        *context;

    context = memt_alloc (NULL, sizeof (THREAD_CONTEXT));
    context-> links      = 0;
    context-> error_text = NULL;
    context-> error_file = NULL;
    context-> error_line = 0;
    context-> waiting    = FALSE;
    context-> gsl_thread = NULL;         /*  Gets filled in in thread.receive */
    context-> replyq     = NULL;
    list_reset (& context-> messages);

    *item = context;
  }
    
    return 0;
}

int register_thread_classes (void)
{
    int
        rc = 0;
    AGENT   *agent;                     /*  Handle for our agent             */
#include "ggthrd.i"                     /*  Include dialog interpreter       */


    declare_smtlib_shutdown       (shutdown_event,      0);

    /*  Reply messages from GSL interpreter                                  */
    declare_ggcode_call_ok      (ok_event,      0);
    declare_ggcode_call_message (message_event, 0);
    declare_ggcode_call_error   (error_event,   0);

    declare_ggcode_ok           (ok_event,      0);
    declare_ggcode_error        (error_event,   0);
    declare_ggcode_fatal        (error_event,   0);

    /*  Declare internal interrupt message  */
    method_declare (agent, THREAD_INTERRUPT, interrupt_event, 0);

    rc |= object_register (thread_class_init,
                           NULL);
    return rc;
}
