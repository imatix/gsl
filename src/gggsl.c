/*---------------------------------------------------------------------------
 *  gggsl.c - GSL/gsl control package
 *
 *  Generated from gggsl by ggobjt.gsl using GSL/4.
 *  DO NOT MODIFY THIS FILE.
 *
 *  For documentation and updates see http://www.imatix.com.
 *---------------------------------------------------------------------------*/

#include "sfl.h"
#include "smt3.h"
#include "gsl.h"                        /*  Project header file              */
#include "gggsl.h"                      /*  Include header file              */
#include "gggsl.d"                      /*  Include dialog data              */

#define AGENT_NAME "GGGSL"

/*- Macros ------------------------------------------------------------------*/

#define GSL_NAME "gsl"                  /*  GSL Control Class                */

#define matches(s,t)    (s ? (ignorecase ? lexcmp (s, t) == 0 : streq (s, t))   : t == NULL)

/*- Function prototypes -----------------------------------------------------*/

static int
gsl_include (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread);
static int
gsl_exec (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread);
static int
gsl_name (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread);
static int gsl_link (void *item);
static int gsl_destroy (void *item);
static const char * gsl_item_name (void *item);
static VALUE * gsl_get_attr (void *item, const char *name, Bool ignorecase);
static int gsl_put_attr (void *item, const char *name, VALUE *value, Bool ignorecase);

/*- Global variables --------------------------------------------------------*/
static PARM_LIST parm_list_v            = { PARM_VALUE };

static GSL_FUNCTION gsl_functions [] =
{
    {"exec",           1, 2, 1, (void *) &parm_list_v, 0, gsl_exec},
    {"include",        1, 2, 1, (void *) &parm_list_v, 0, gsl_include},
    {"name",           0, 0, 0, NULL,            1, gsl_name}};

CLASS_DESCRIPTOR
    gsl_class = {
        "gsl",
        gsl_item_name,
        gsl_get_attr,
        gsl_put_attr,
        NULL,
        NULL,
        NULL,
        NULL,
        gsl_destroy,
        gsl_link,
        NULL,
        NULL,
        NULL,
        NULL,
        gsl_functions, tblsize (gsl_functions) };


#include "ggscrp.h"                    /*  File module for 'script' item     */

/*- Macros ------------------------------------------------------------------*/

#define BUFFER_SIZE 4096

/*- Types -------------------------------------------------------------------*/

typedef struct {
    event_t
        thread_type_event;
    VDESCR
        *buffer;
    RESULT_NODE
        *result,
        *eval_result;
    THREAD
        *slave_thread;
    QID
        replyqid;
} TCB;

/*- Global variables used in this source file only --------------------------*/

static TCB
    *tcb;                               /*  Address thread context block     */

/*- Global variables --------------------------------------------------------*/

char
    *me,                               /*  Module name                       */
    *version;                          /*  GSL version                       */


/*- Functions ---------------------------------------------------------------*/

static VDESCR *
output_catch_start (void)
{
    VDESCR
        *vdescr;
        
    vdescr = mem_alloc (sizeof (VDESCR));
    vdescr-> max_size = 0;
    vdescr-> cur_size = 0;
    vdescr-> data     = NULL;
    
    return vdescr;
}

static void
output_catch (JOBID job, const char *text)
{
    VDESCR 
        *buffer = (VDESCR *) job;
    size_t
        text_length = strlen (text);
    
    if (text_length)
      {
        if (buffer-> cur_size + text_length > buffer-> max_size)
          {
            if (buffer-> data == NULL)
                buffer-> data = mem_alloc (BUFFER_SIZE);
            else
                buffer-> data = mem_realloc (buffer-> data,
                                             buffer-> max_size + BUFFER_SIZE);
            buffer-> max_size += BUFFER_SIZE;
          }
        memcpy (buffer-> data + buffer-> cur_size,
                text, text_length);
        buffer-> cur_size += text_length;
      }
}


/*************************   INITIALISE THE THREAD   *************************/

MODULE initialise_the_thread (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */
    
    the_next_event = tcb-> thread_type_event;
}


/******************************   RETURN DONE   ******************************/

MODULE return_done (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    if (tcb-> buffer)
      {
        tcb-> result-> value. type = TYPE_UNKNOWN;
        tcb-> result-> value. s    = mem_alloc (tcb-> buffer-> cur_size + 1);
        memcpy (tcb-> result-> value. s, 
                tcb-> buffer-> data, tcb-> buffer-> cur_size);
        tcb-> result-> value. s [tcb-> buffer-> cur_size] = 0;
      }    
    lsend_ggcode_call_ok (& tcb-> replyqid, NULL,
                          NULL, NULL, NULL, 0);
}


/********************   FORWARD MESSAGE AS CALL MESSAGE   ********************/

MODULE forward_message_as_call_message (THREAD *thread)
{
    struct_ggcode_error_reply
        *error_reply;

    tcb = thread-> tcb;                 /*  Point to thread's context        */

    get_ggcode_error_reply (thread-> event-> body, & error_reply);
    lsend_ggcode_call_message (& tcb-> replyqid, NULL,
                               NULL, NULL, NULL, 0,
                               error_reply-> error_name, 
                               error_reply-> error_line,
                               error_reply-> error_text);
    free_ggcode_error_reply (& error_reply);
}


/*********************   FORWARD MESSAGE AS CALL ERROR   *********************/

MODULE forward_message_as_call_error (THREAD *thread)
{
    struct_ggcode_error_reply
        *error_reply;

    tcb = thread-> tcb;                 /*  Point to thread's context        */

    get_ggcode_error_reply (thread-> event-> body, & error_reply);
    lsend_ggcode_call_error (& tcb-> replyqid, NULL,
                             NULL, NULL, NULL, 0,
                             error_reply-> error_name, 
                             error_reply-> error_line,
                             error_reply-> error_text);
    free_ggcode_error_reply (& error_reply);
}


/**************************   TERMINATE THE THREAD   *************************/

MODULE terminate_the_thread (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    if (tcb-> buffer)
      {
        mem_free (tcb-> buffer-> data);
        mem_free (tcb-> buffer);
      }
    gsl_finish (tcb-> slave_thread);
    the_next_event = terminate_event;
}


static int gsl_link (void *item)
{
    
    ((GSL_CONTROL *) item)-> links++;
    return 0;
    
}

static int gsl_destroy (void *item)
{
    
    GSL_CONTROL
        *gsl = item;
    int
        i;

    if (--gsl-> links == 0)
      {
        mem_free (gsl-> terminator);
        for (i = 0; i < gsl-> argc; i++)
            mem_free (gsl-> argv [i]);
        mem_free (gsl-> argv);
        mem_free (gsl-> output_file);
        mem_free (gsl-> input_file);
        symb_class. destroy (gsl-> switches);
        mem_free (gsl);
      }
    return 0;
    
}

static const char * gsl_item_name (void *item)
{
    
    return "gsl";
    
}

static VALUE * gsl_get_attr (void *item, const char *name, Bool ignorecase)
{

    GSL_CONTROL
        *gsl = item;
    static VALUE
        value;
    char
        *args,
        *endptr;
    int
        argn;

    init_value (& value);
        
    if (matches (name, "ignorecase"))
      {

    assign_number (& value, gsl-> ignorecase);
        
      }
    else
    if (matches (name, "cobol"))
      {

    assign_number (& value, gsl-> cobol);
        
      }
    else
    if (matches (name, "script"))
      {

    if (gsl-> line)
      {
        /*  Build value manually rather than use assign_pointer because   */
        /*  don't want a link made.                                       */
        value. type = TYPE_POINTER;
        value. c    = & script_line_class;
        value. i    = gsl-> line;
      }
        
      }
    else
    if (matches (name, "shuffle"))
      {

    assign_number (& value, gsl-> shuffle);
        
      }
    else
    if (matches (name, "terminator"))
      {

    assign_string (& value, gsl-> terminator);
        
      }
    else
    if (matches (name, "switches"))
      {

    /*  Build value manually rather than use assign_pointer because   */
    /*  don't want a link made.                                       */
    value. type = TYPE_POINTER;
    value. c    = & symb_class;
    value. i    = gsl-> switches;
        
      }
    else
    if (matches (name, "me"))
      {

    assign_string (& value, me);
        
      }
    else
    if (matches (name, "version"))
      {

    assign_string (& value, version);
        
      }
    else
    if (matches (name, "argc"))
      {

    assign_number (& value, gsl-> argc);
        
      }
    else
    if (matches (name, "outfile"))
      {

    assign_string (& value, gsl-> output_file);
        
      }
    else
    if (matches (name, "filename"))
      {

    assign_string (& value, gsl-> input_file);
        
      }
    else
    if (matches (name, "line"))
      {

    assign_number (& value, gsl-> output_line);
        
      }

    else
    if (name && name [0])
      {
        args = mem_strdup (name);
        strlwc (args);
        if (strlen (args) >= 4
        &&  args [0] == 'a' && args [1] == 'r' && args [2] == 'g')
          {
            argn = (int) strtol (& args [3], & endptr, 10);
            if (*endptr == 0            /*  Complete conversion  */
            &&  argn <= gsl-> argc)     /*  Within range         */
              {
                value. type = TYPE_UNKNOWN;
                value. s    = gsl-> argv [argn - 1];
              }
          }
        mem_free (args);
      }

    return & value;
        
}

static int gsl_put_attr (void *item, const char *name, VALUE *value, Bool ignorecase)
{

    GSL_CONTROL
        *gsl = item;
        
    if (matches (name, "ignorecase"))
      {

    gsl-> ignorecase = (Bool) number_value (value);
        
      }
    else
    if (matches (name, "cobol"))
      {

    gsl-> cobol      = (Bool) number_value (value);
        
      }
    else
    if (matches (name, "shuffle"))
      {

    gsl-> shuffle    = (Bool) number_value (value);
        
      }
    else
    if (matches (name, "terminator"))
      {

    mem_free (gsl-> terminator);
    gsl-> terminator = memt_strdup (NULL, string_value (value));
        
      }

    else
        return -1;

    return 0;
        
}


static int
gsl_include (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread)
{
    RESULT_NODE *filename = argc > 0 ? argv [0] : NULL;
    RESULT_NODE *template = argc > 1 ? argv [1] : NULL;

    if (! filename)
      {
        strcpy (object_error, "Missing argument: filename");
        return -1;
      }
    if (filename-> value. type == TYPE_UNDEFINED)
      {
        result-> culprit = filename-> culprit;
        filename-> culprit = NULL;
        lsend_ggcode_call_ok (& gsl_thread-> queue-> qid, NULL,
                             NULL, NULL, NULL, 0);
        return 0;
      }

  {
    THREAD
        *thread;
    GGCODE_TCB
        *new_gsl_tcb;
    
    thread = thread_create (AGENT_NAME, "");
    tcb = thread-> tcb;
    tcb-> thread_type_event = execute_event;
    tcb-> replyqid          = gsl_thread-> queue-> qid;
    tcb-> result            = result;
    tcb-> buffer            = output_catch_start ();
    
    tcb-> slave_thread = gsl_copy (gsl_thread,
                                   thread-> queue,
                                   tcb-> buffer);
    new_gsl_tcb = tcb-> slave_thread-> tcb;
    new_gsl_tcb-> script_name = mem_strdup (string_value (& filename-> value));
    if (template)
        new_gsl_tcb-> gsl-> template = (Bool) number_value (& template-> value);
        
    gsl_continue (tcb-> slave_thread,
                  FALSE,
                  thread-> queue);
  }
        
    return 0;  /*  Just in case  */
}


static int
gsl_exec (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread)
{
    RESULT_NODE *command = argc > 0 ? argv [0] : NULL;
    RESULT_NODE *template = argc > 1 ? argv [1] : NULL;

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

  {
    THREAD
        *thread;
    GGCODE_TCB
        *new_gsl_tcb;
    
    thread = thread_create (AGENT_NAME, "");
    tcb = thread-> tcb;
    tcb-> thread_type_event = execute_event;
    tcb-> replyqid          = gsl_thread-> queue-> qid;
    tcb-> result            = result;
    tcb-> buffer            = output_catch_start ();
    
    tcb-> slave_thread = gsl_copy (gsl_thread,
                                   thread-> queue,
                                   tcb-> buffer);
    new_gsl_tcb = tcb-> slave_thread-> tcb;
    new_gsl_tcb-> script_name = mem_strdup (strprintf ("(%s %u)",
                                                       gsl_cur_script (gsl_thread),
                                                       gsl_cur_line (gsl_thread)));
    gg_send_output (tcb-> slave_thread, output_catch, FALSE);
    if (template)
        new_gsl_tcb-> gsl-> template = (Bool) number_value (& template-> value);
    
    gsl_command (tcb-> slave_thread,
                 string_value (& command-> value),
                 FALSE,
                 thread-> queue);
  }
        
    return 0;  /*  Just in case  */
}


static int
gsl_name (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread)
{


  {
    char
        *item_name;

    item_name = mem_strdup ((char *) gsl_item_name
                                         (item));

    if (item_name)
        assign_string (& result-> value, item_name);

    return 0;
  }
        
    return 0;  /*  Just in case  */
}

static int gsl_class_init (CLASS_DESCRIPTOR **class, void **item, THREAD *gsl_thread)
{
     *class = & gsl_class;


  {
    GSL_CONTROL
        *gsl;

    gsl = mem_alloc (sizeof (GSL_CONTROL));

    gsl-> links       = 0;
    gsl-> ignorecase  = TRUE;
    gsl-> cobol       = FALSE;
    gsl-> template    = FALSE;
    gsl-> line        = NULL;
    gsl-> shuffle     = 2;
    gsl-> terminator  = NULL;
    gsl-> argc        = 0;
    gsl-> argv        = NULL;
    gsl-> switches    = NULL;
    gsl-> output_file = NULL;
    gsl-> input_file  = NULL;
    gsl-> output_line = 1;

    *item = gsl;
    ((GGCODE_TCB *) gsl_thread-> tcb)-> gsl = gsl;
  }
    
    return 0;
}

int register_gsl_classes (void)
{
    int
        rc = 0;
    AGENT   *agent;                     /*  Handle for our agent             */
#include "gggsl.i"                      /*  Include dialog interpreter       */


    /*  Shutdown event comes from Kernel                                     */
    declare_smtlib_shutdown   (shutdown_event, SMT_PRIORITY_MAX);

    /*  GSL interpreter reply events                                         */
    declare_ggcode_ok      (ok_event,      0);
    declare_ggcode_message (message_event, 0);
    declare_ggcode_error   (error_event,   0);
    declare_ggcode_fatal   (fatal_event,   0);


    rc |= object_register (gsl_class_init,
                           NULL);
    return rc;
}
