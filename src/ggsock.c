/*---------------------------------------------------------------------------
 *  ggsock.c - GSL/socket package
 *
 *  Generated from ggsock by ggobjt.gsl using GSL/4.
 *  DO NOT MODIFY THIS FILE.
 *
 *  For documentation and updates see http://www.imatix.com.
 *---------------------------------------------------------------------------*/

#include "sfl.h"
#include "smt3.h"
#include "gsl.h"                        /*  Project header file              */
#include "ggsock.h"                     /*  Include header file              */
#include "ggsock.d"                     /*  Include dialog data              */

#define AGENT_NAME "GGSOCK"

/*- Macros ------------------------------------------------------------------*/

#define SOCK_NAME "sock"                /*  Socket                           */
#define SOCK_HANDLE_NAME "sock handle"  /*  Socket handle                    */

#define matches(s,t)    (s ? (ignorecase ? lexcmp (s, t) == 0 : streq (s, t))   : t == NULL)

/*- Function prototypes -----------------------------------------------------*/

static int
sock_passive (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread);
static int
sock_connect (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread);
static int
sock_handle_accept (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread);
static int
sock_handle_close (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread);
static int
sock_handle_read (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread);
static int
sock_handle_write (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread);
static int sock_link (void *item);
static int sock_destroy (void *item);
static VALUE * sock_get_attr (void *item, const char *name, Bool ignorecase);
static int sock_handle_link (void *item);
static int sock_handle_destroy (void *item);
static VALUE * sock_handle_get_attr (void *item, const char *name, Bool ignorecase);

/*- Global variables --------------------------------------------------------*/
static PARM_LIST parm_list_vr           = { PARM_VALUE,
                                            PARM_REFERENCE };
static PARM_LIST parm_list_vvvr         = { PARM_VALUE,
                                            PARM_VALUE,
                                            PARM_VALUE,
                                            PARM_REFERENCE };
static PARM_LIST parm_list_rvvr         = { PARM_REFERENCE,
                                            PARM_VALUE,
                                            PARM_VALUE,
                                            PARM_REFERENCE };
static PARM_LIST parm_list_vvr          = { PARM_VALUE,
                                            PARM_VALUE,
                                            PARM_REFERENCE };

static GSL_FUNCTION sock_functions [] =
{
    {"connect",        0, 4, 4, (void *) &parm_list_vvvr, 0, sock_connect},
    {"passive",        1, 2, 2, (void *) &parm_list_vr, 1, sock_passive}};

CLASS_DESCRIPTOR
    sock_class = {
        "sock",
        NULL,
        sock_get_attr,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        sock_destroy,
        sock_link,
        NULL,
        NULL,
        NULL,
        NULL,
        sock_functions, tblsize (sock_functions) };

static GSL_FUNCTION sock_handle_functions [] =
{
    {"accept",         0, 2, 2, (void *) &parm_list_vr, 0, sock_handle_accept},
    {"close",          0, 2, 2, (void *) &parm_list_vr, 0, sock_handle_close},
    {"read",           1, 4, 4, (void *) &parm_list_rvvr, 0, sock_handle_read},
    {"write",          1, 3, 3, (void *) &parm_list_vvr, 0, sock_handle_write}};

CLASS_DESCRIPTOR
    sock_handle_class = {
        "sock handle",
        NULL,
        sock_handle_get_attr,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        sock_handle_destroy,
        sock_handle_link,
        NULL,
        NULL,
        NULL,
        NULL,
        sock_handle_functions, tblsize (sock_handle_functions) };


typedef struct {
    int
        links;
    char
        *error_msg;
} SOCK_CONTEXT;

typedef struct {
    int
        links;
    sock_t
        handle;
    char
        *error_msg;
} SOCK_HANDLE_ITEM;

typedef struct {
    THREAD
        *gsl_thread;
    RESULT_NODE
        *result,
        *buffer,
        *error;
    SOCK_HANDLE_ITEM
        *sock_handle;
    SOCK_CONTEXT
        *context;
    sock_t
        handle;
} TCB;

static TCB
    *tcb;                               /*  Address thread context block     */

static QUEUE
    *sockq = NULL;

#define QUEUE_LENGTH     10

#define SOCKET_CLOSED_MESSAGE "Socket closed"
#define TIMED_OUT_MESSAGE     "Request timed out"

static int
start_socket_agent (void)
{
    THREAD
        *thread;

    if (! sockq)
      {
        if (agent_lookup (SMT_SOCKET) == NULL)
            smtsock_init ();
        if ( (thread = thread_lookup (SMT_SOCKET, "")) != NULL )
            sockq = thread-> queue;
        else
          {
            strcpy (object_error, "Unable to start SOCK agent.");
            return -1;
          }
      }
    return 0;
}

static int
store_module_error (THREAD       *gsl_thread,
                    SOCK_CONTEXT *context,
                    RESULT_NODE  *error,
                    const char   *error_msg,
                          char   **error_text)
{
    GGCODE_TCB
        *gsl_tcb = gsl_thread-> tcb;
    VALUE
        value;

    if (error_msg)
      {
        if (! context)
            context = get_class_item (gsl_thread, SOCK_NAME);
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

static int
store_sock_error (SOCK_HANDLE_ITEM *sock_handle,
                  THREAD           *gsl_thread,
                  SOCK_CONTEXT     *context,
                  RESULT_NODE      *error,
                  const char       *error_msg,
                        char      **error_text)
{
    if (error_msg && sock_handle)
      {
        mem_free (sock_handle-> error_msg);
        sock_handle-> error_msg = memt_strdup (NULL, error_msg);
      }
    return store_module_error (gsl_thread, context, error,
                               error_msg, error_text);
}

static void
reply_readh_result (byte *body, char *error_msg)
{
    struct_smtsock_readh_reply
        *readh_reply;
    GGCODE_TCB
        *gsl_tcb = tcb-> gsl_thread-> tcb;
    VALUE
        value;
    char
        *error_text;

    /*  Pick up read value  */
    get_smtsock_readh_reply (body, & readh_reply);
    init_value (& value);
    if (readh_reply-> size > 0)
      {
        assign_string (& value, memt_alloc (NULL, readh_reply-> size + 1));
        memcpy (value. s, readh_reply-> data, readh_reply-> size);
        value. s [readh_reply-> size] = '\0';
      }
    /*  Store the value  */
    if (! store_symbol_definition (& gsl_tcb-> scope_stack,
                                   gsl_tcb-> gsl-> ignorecase,
                                   tcb-> buffer,
                                   &value,
                                   &error_text))
      {
        lsend_ggcode_call_error (& tcb-> gsl_thread-> queue-> qid, NULL,
                                 NULL, NULL, NULL, 0,
                                 NULL, 0,
                                 error_text);
        return;
      }
    destroy_value (& value);

    /*  Build return value  */
    assign_number (& tcb-> result-> value, readh_reply-> size);

    free_smtsock_readh_reply (& readh_reply);

    if (store_sock_error (tcb-> sock_handle,
                          tcb-> gsl_thread,
                          tcb-> context,
                          tcb-> error,
                          error_msg,
                          &error_text))
        lsend_ggcode_call_error (& tcb-> gsl_thread-> queue-> qid, NULL,
                                 NULL, NULL, NULL, 0,
                                 NULL, 0,
                                 error_text);
    else
        lsend_ggcode_call_ok (& tcb-> gsl_thread-> queue-> qid, NULL,
                              NULL, NULL, NULL, 0);
}

/*************************   INITIALISE THE THREAD   *************************/

MODULE initialise_the_thread (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */
    the_next_event = ok_event;
}


/****************************   REPLY OK RESULT   ****************************/

MODULE reply_ok_result (THREAD *thread)
{
    SOCK_HANDLE_ITEM
        *socket;
    char
        *error_text;
    int
        rc;

    tcb = thread-> tcb;                 /*  Point to thread's context        */

    /*  We rely here on tcb-> handle being the master socket for an accept   */
    /*  and zero for any other request.                                      */
    if (tcb-> handle)
      {
        socket = memt_alloc (NULL, sizeof (SOCK_HANDLE_ITEM));
        socket-> links     = 0;
        socket-> handle    = accept_socket (tcb-> handle);
        socket-> error_msg = NULL;

        assign_pointer (& tcb-> result-> value, & sock_handle_class, socket);
      }

    if (tcb-> sock_handle)
        rc = store_sock_error (tcb-> sock_handle,
                               tcb-> gsl_thread,
                               tcb-> context,
                               tcb-> error,
                               NULL,
                               &error_text);
    else
        rc = store_module_error (tcb-> gsl_thread,
                                 tcb-> context,
                                 tcb-> error,
                                 NULL,
                                 &error_text);

    if (rc)
        lsend_ggcode_call_error (& tcb-> gsl_thread-> queue-> qid, NULL,
                                 NULL, NULL, NULL, 0,
                                 NULL, 0,
                                 error_text);
    else
        lsend_ggcode_call_ok (& tcb-> gsl_thread-> queue-> qid, NULL,
                              NULL, NULL, NULL, 0);
}


/**************************   TERMINATE THE THREAD   *************************/

MODULE terminate_the_thread (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    the_next_event = terminate_event;
}


/***************************   REPLY ERROR RESULT   **************************/

MODULE reply_error_result (THREAD *thread)
{
    struct_smtsock_error
        *error_reply;
    char
        *error_text;
   int
        rc;

    tcb = thread-> tcb;                 /*  Point to thread's context        */

    get_smtsock_error (thread-> event-> body, & error_reply);

    if (tcb-> sock_handle)
        rc = store_sock_error (tcb-> sock_handle,
                               tcb-> gsl_thread,
                               tcb-> context,
                               tcb-> error,
                               error_reply-> message,
                               &error_text);
    else
        rc = store_module_error (tcb-> gsl_thread,
                                 tcb-> context,
                                 tcb-> error,
                                 error_reply-> message,
                                 &error_text);

    free_smtsock_error (& error_reply);

    if (rc)
        lsend_ggcode_call_error (& tcb-> gsl_thread-> queue-> qid, NULL,
                                 NULL, NULL, NULL, 0,
                                 NULL, 0,
                                 error_text);
    else
        lsend_ggcode_call_ok (& tcb-> gsl_thread-> queue-> qid, NULL,
                              NULL, NULL, NULL, 0);
}


/**************************   REPLY CLOSED RESULT   **************************/

MODULE reply_closed_result (THREAD *thread)
{
    char
        *error_text;

    tcb = thread-> tcb;                 /*  Point to thread's context        */

    if (store_sock_error (tcb-> sock_handle,
                          tcb-> gsl_thread,
                          tcb-> context,
                          tcb-> error,
                          SOCKET_CLOSED_MESSAGE,
                          &error_text))
        lsend_ggcode_call_error (& tcb-> gsl_thread-> queue-> qid, NULL,
                                 NULL, NULL, NULL, 0,
                                 NULL, 0,
                                 error_text);
    else
        lsend_ggcode_call_ok (& tcb-> gsl_thread-> queue-> qid, NULL,
                              NULL, NULL, NULL, 0);
}


/**************************   REPLY TIMEOUT RESULT   *************************/

MODULE reply_timeout_result (THREAD *thread)
{
    char
        *error_text;

    tcb = thread-> tcb;                 /*  Point to thread's context        */

    if (store_sock_error (tcb-> sock_handle,
                          tcb-> gsl_thread,
                          tcb-> context,
                          tcb-> error,
                          TIMED_OUT_MESSAGE,
                          &error_text))
        lsend_ggcode_call_error (& tcb-> gsl_thread-> queue-> qid, NULL,
                                 NULL, NULL, NULL, 0,
                                 NULL, 0,
                                 error_text);
    else
        lsend_ggcode_call_ok (& tcb-> gsl_thread-> queue-> qid, NULL,
                              NULL, NULL, NULL, 0);
}


/************************   REPLY CONNECT OK RESULT   ************************/

MODULE reply_connect_ok_result (THREAD *thread)
{
    struct_smtsock_connect_ok
        *connect_reply;
    SOCK_HANDLE_ITEM
        *socket;
    char
        *error_text;

    tcb = thread-> tcb;                 /*  Point to thread's context        */

    get_smtsock_connect_ok (thread-> event-> body, & connect_reply);

    socket = memt_alloc (NULL, sizeof (SOCK_HANDLE_ITEM));
    socket-> links     = 0;
    socket-> handle    = connect_reply-> socket;
    socket-> error_msg = NULL;

    assign_pointer (& tcb-> result-> value, & sock_handle_class, socket);

    free_smtsock_connect_ok (& connect_reply);

    if (store_module_error (tcb-> gsl_thread,
                            tcb-> context,
                            tcb-> error,
                            NULL,
                            &error_text))
        lsend_ggcode_call_error (& tcb-> gsl_thread-> queue-> qid, NULL,
                                 NULL, NULL, NULL, 0,
                                 NULL, 0,
                                 error_text);
    else
        lsend_ggcode_call_ok (& tcb-> gsl_thread-> queue-> qid, NULL,
                              NULL, NULL, NULL, 0);
}


/*************************   REPLY READH OK RESULT   *************************/

MODULE reply_readh_ok_result (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    reply_readh_result (thread-> event-> body, NULL);
}


/***********************   REPLY READH CLOSED RESULT   ***********************/

MODULE reply_readh_closed_result (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    reply_readh_result (thread-> event-> body, SOCKET_CLOSED_MESSAGE);
}


/***********************   REPLY READH TIMEOUT RESULT   **********************/

MODULE reply_readh_timeout_result (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    reply_readh_result (thread-> event-> body, TIMED_OUT_MESSAGE);
}


static int sock_link (void *item)
{
    
((SOCK_CONTEXT *) item)-> links++;
return 0;
    
}

static int sock_destroy (void *item)
{
    
  {
    SOCK_CONTEXT
        *context = item;

    if (--context-> links == 0)
      {
        mem_free (context-> error_msg);
        mem_free (context);
      }
    return 0;
  }
    
}

static VALUE * sock_get_attr (void *item, const char *name, Bool ignorecase)
{

    SOCK_CONTEXT
        *context = item;
    static VALUE
        value;

    if (! name)
        return NULL;

    init_value (& value);
        
    if (matches (name, "error"))
      {

        if (context-> error_msg)
            assign_string (& value, context-> error_msg);
        
      }

    return & value;
        
}

static int sock_handle_link (void *item)
{
    
    if (item)
        ((SOCK_HANDLE_ITEM *) item)-> links++;
    return 0;
    
}

static int sock_handle_destroy (void *item)
{
    
    SOCK_HANDLE_ITEM
        *socket = item;

    if (socket
    &&  --socket-> links <= 0)
      {
        mem_free (socket-> error_msg);
        mem_free (socket);
      }
    return 0;
    
}

static VALUE * sock_handle_get_attr (void *item, const char *name, Bool ignorecase)
{

    SOCK_HANDLE_ITEM
        *socket = item;
    static VALUE
        value;

    if (! name)
        return NULL;

    init_value (& value);
        
    if (matches (name, "error"))
      {

        if (socket-> error_msg)
            assign_string (& value, socket-> error_msg);
        
      }

    return & value;
        
}


static int
sock_passive (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread)
{
    RESULT_NODE *service = argc > 0 ? argv [0] : NULL;
    RESULT_NODE *error   = argc > 1 ? argv [1] : NULL;

    if (! service)
      {
        strcpy (object_error, "Missing argument: service");
        return -1;
      }
    if (service-> value. type == TYPE_UNDEFINED)
      {
        result-> culprit = service-> culprit;
        service-> culprit = NULL;
        return 0;
      }

  {
    SOCK_HANDLE_ITEM
        *socket;
    sock_t
        handle;
    SOCK_CONTEXT
        *context = item;
    const char
        *error_msg;
    char
        *error_text;

    if (start_socket_agent ())
        return -1;

    handle = passive_TCP (string_value (& service-> value),
                          QUEUE_LENGTH);
    if (handle != INVALID_SOCKET)       /*  Success  */
      {
        socket = memt_alloc (NULL, sizeof (SOCK_HANDLE_ITEM));
        socket-> links     = 0;
        socket-> handle    = handle;
        socket-> error_msg = NULL;

        assign_pointer (& result-> value, & sock_handle_class, socket);

        error_msg = NULL;
      }
    else
        error_msg = connect_errlist [connect_error ()];

    if (store_module_error (gsl_thread,
                            context,
                            error,
                            error_msg,
                            &error_text))

      {
        strncpy (object_error, error_text, LINE_MAX);
        return -1;
      }
    else
        return 0;
  }
        
    return 0;  /*  Just in case  */
}


static int
sock_connect (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread)
{
    RESULT_NODE *host    = argc > 0 ? argv [0] : NULL;
    RESULT_NODE *service = argc > 1 ? argv [1] : NULL;
    RESULT_NODE *timeout = argc > 2 ? argv [2] : NULL;
    RESULT_NODE *error   = argc > 3 ? argv [3] : NULL;

    if (! service)
      {
        strcpy (object_error, "Missing argument: service");
        return -1;
      }
    if (service-> value. type == TYPE_UNDEFINED)
      {
        result-> culprit = service-> culprit;
        service-> culprit = NULL;
        lsend_ggcode_call_ok (& gsl_thread-> queue-> qid, NULL,
                             NULL, NULL, NULL, 0);
        return 0;
      }

  {
    THREAD
        *thread;

    if (start_socket_agent ())
        return -1;

    thread = thread_create (AGENT_NAME, "");
    tcb = thread-> tcb;

    tcb-> gsl_thread  = gsl_thread;
    tcb-> result      = result;
    tcb-> buffer      = NULL;
    tcb-> error       = error;
    tcb-> sock_handle = NULL;
    tcb-> context     = item;
    tcb-> handle      = 0;

    lsend_smtsock_connect (& sockq-> qid,
                           & thread-> queue-> qid,
                           NULL, NULL, NULL, 0,
                           (word) (timeout ? number_value (& timeout-> value) : 0),
                           "tcp",
                           host ? string_value (& host-> value) : "",
                           string_value (& service-> value),
                           0, 0, 0);
    return 0;
  }
        
    return 0;  /*  Just in case  */
}


static int
sock_handle_accept (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread)
{
    RESULT_NODE *timeout = argc > 0 ? argv [0] : NULL;
    RESULT_NODE *error   = argc > 1 ? argv [1] : NULL;


  {
    SOCK_HANDLE_ITEM
        *socket = item;
    THREAD
        *thread;

    if (start_socket_agent ())
        return -1;

    thread = thread_create (AGENT_NAME, "");
    tcb = thread-> tcb;

    tcb-> gsl_thread  = gsl_thread;
    tcb-> result      = result;
    tcb-> buffer      = NULL;
    tcb-> error       = error;
    tcb-> sock_handle = socket;
    tcb-> context     = NULL;
    tcb-> handle      = socket-> handle;

    lsend_smtsock_input (& sockq-> qid,
                         & thread-> queue-> qid,
                         NULL, NULL, NULL, 0,
                         (word) (timeout ? number_value (& timeout-> value) : 0),
                         socket-> handle,
                         0);

    return 0;
  }
        
    return 0;  /*  Just in case  */
}


static int
sock_handle_close (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread)
{
    RESULT_NODE *timeout = argc > 0 ? argv [0] : NULL;
    RESULT_NODE *error   = argc > 1 ? argv [1] : NULL;


  {
    SOCK_HANDLE_ITEM
        *socket = item;
    THREAD
        *thread;

    if (start_socket_agent ())
        return -1;

    thread = thread_create (AGENT_NAME, "");
    tcb = thread-> tcb;

    tcb-> gsl_thread  = gsl_thread;
    tcb-> result      = result;
    tcb-> buffer      = NULL;
    tcb-> error       = error;
    tcb-> sock_handle = socket;
    tcb-> context     = NULL;
    tcb-> handle      = 0;

    lsend_smtsock_close (& sockq-> qid,
                         & thread-> queue-> qid,
                         NULL, NULL, NULL, 0,
                         (word) (timeout ? number_value (& timeout-> value) : 0),
                         socket-> handle,
                         TRUE,
                         0);

    return 0;
  }
        
    return 0;  /*  Just in case  */
}


static int
sock_handle_read (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread)
{
    RESULT_NODE *buffer  = argc > 0 ? argv [0] : NULL;
    RESULT_NODE *minimum = argc > 1 ? argv [1] : NULL;
    RESULT_NODE *timeout = argc > 2 ? argv [2] : NULL;
    RESULT_NODE *error   = argc > 3 ? argv [3] : NULL;

    if (! buffer)
      {
        strcpy (object_error, "Missing argument: buffer");
        return -1;
      }

  {
    SOCK_HANDLE_ITEM
        *socket = item;
    THREAD  
        *thread;

    if (start_socket_agent ())
        return -1;

    thread = thread_create (AGENT_NAME, "");
    tcb = thread-> tcb;

    tcb-> gsl_thread  = gsl_thread;
    tcb-> result      = result;
    tcb-> buffer      = buffer;
    tcb-> error       = error;
    tcb-> sock_handle = socket;
    tcb-> context     = NULL;
    tcb-> handle      = 0;

    lsend_smtsock_readh (& sockq-> qid,
                         & thread-> queue-> qid,
                         NULL, NULL, NULL, 0,
                         (word) (timeout ? number_value (&timeout-> value): 0),
                         socket-> handle,
                         (dbyte) 0xFFFF,                   /*  Maximum */
                         minimum ? (word) number_value (& minimum-> value) : 1,
                         0);

    return 0;
  }
        
    return 0;  /*  Just in case  */
}


static int
sock_handle_write (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread)
{
    RESULT_NODE *buffer  = argc > 0 ? argv [0] : NULL;
    RESULT_NODE *timeout = argc > 1 ? argv [1] : NULL;
    RESULT_NODE *error   = argc > 2 ? argv [2] : NULL;

    if (! buffer)
      {
        strcpy (object_error, "Missing argument: buffer");
        return -1;
      }
    if (buffer-> value. type == TYPE_UNDEFINED)
      {
        result-> culprit = buffer-> culprit;
        buffer-> culprit = NULL;
        lsend_ggcode_call_ok (& gsl_thread-> queue-> qid, NULL,
                             NULL, NULL, NULL, 0);
        return 0;
      }

  {
    SOCK_HANDLE_ITEM
        *socket = item;
    THREAD  
        *thread;

    if (start_socket_agent ())
        return -1;

    thread = thread_create (AGENT_NAME, "");
    tcb = thread-> tcb;

    tcb-> gsl_thread  = gsl_thread;
    tcb-> result      = result;
    tcb-> buffer      = NULL;
    tcb-> error       = error;
    tcb-> sock_handle = socket;
    tcb-> context     = NULL;
    tcb-> handle      = 0;

    lsend_smtsock_writeh (& sockq-> qid,
                          & thread-> queue-> qid,
                          NULL, NULL, NULL, 0,
                          (word) (timeout ? number_value (& timeout-> value) : 0),
                          socket-> handle,
                          (qbyte) strlen (string_value (& buffer-> value)),
                          (byte *) buffer-> value. s,
                          TRUE,
                          0);
    return 0;
  }
        
    return 0;  /*  Just in case  */
}

static int sock_class_init (CLASS_DESCRIPTOR **class, void **item, THREAD *gsl_thread)
{
     *class = & sock_class;


  {
    SOCK_CONTEXT
        *context;

    context = memt_alloc (NULL, sizeof (SOCK_CONTEXT));
    context-> links     = 0;
    context-> error_msg = NULL;

    *item = context;
  }
    
    return 0;
}

int register_sock_classes (void)
{
    int
        rc = 0;
    AGENT   *agent;                     /*  Handle for our agent             */
#include "ggsock.i"                     /*  Include dialog interpreter       */


    declare_smtlib_shutdown       (shutdown_event,      0);

    declare_smtsock_readh_ok      (readh_ok_event,      0);
    declare_smtsock_readh_closed  (readh_closed_event,  0);
    declare_smtsock_readh_timeout (readh_timeout_event, 0);

    declare_smtsock_connect_ok    (connect_ok_event,    0);

    declare_smtsock_ok            (ok_event,      0);
    declare_smtsock_closed        (closed_event,  0);
    declare_smtsock_timeout       (timeout_event, 0);
    declare_smtsock_error         (error_event,   0);

    rc |= object_register (sock_class_init,
                           NULL);
    return rc;
}
