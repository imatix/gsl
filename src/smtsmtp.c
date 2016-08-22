/*===========================================================================*
 *                                                                           *
 *  smtsmtp.c - SMTP email agent                                             *
 *                                                                           *
 *  Copyright (c) 1991-2010 iMatix Corporation                               *
 *                                                                           *
 *  ------------------ GPL Licensed Source Code ------------------           *
 *  iMatix makes this software available under the GNU General               *
 *  Public License (GPL) license for open source projects.  For              *
 *  details of the GPL license please see www.gnu.org or read the            *
 *  file license.gpl provided in this package.                               *
 *                                                                           *
 *  This program is free software; you can redistribute it and/or            *
 *  modify it under the terms of the GNU General Public License as           *
 *  published by the Free Software Foundation; either version 3 of           *
 *  the License, or (at your option) any later version.                      *
 *                                                                           *
 *  This program is distributed in the hope that it will be useful,          *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of           *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            *
 *  GNU General Public License for more details.                             *
 *                                                                           *
 *  You should have received a copy of the GNU General Public                *
 *  License along with this program in the file 'license.gpl'; if            *
 *  not, see <http://www.gnu.org/licenses/>.                                 *
 *                                                                           *
 *  You can also license this software under iMatix's General Terms          *
 *  of Business (GTB) for commercial projects.  If you have not              *
 *  explicitly licensed this software under the iMatix GTB you may           *
 *  only use it under the terms of the GNU General Public License.           *
 *                                                                           *
 *  For more information, send an email to info@imatix.com.                  *
 *  --------------------------------------------------------------           *
 *===========================================================================*/

#include "smtpriv.h"                    /*  SMT definitions                  */


/*- Definitions -------------------------------------------------------------*/

#define AGENT_NAME   "smtsmtp"         /*  Our public name                  */

#define SMTP_SERVER_ERROR    400        /*  greater is a server error        */
#define BUFFER_SIZE          512        /* Maximum size of a line            */

/* XXX TMP*/
#define ERROR_CODE            (-1)

#define SOCK_TAG_CONNECT      (1)
#define SOCK_TAG_READ         (2)
#define SOCK_TAG_WRITE        (3)
#define SOCK_TAG_INPUT        (4)
#define SOCK_TAG_OUTPUT       (5)


/* used to make the SMT code more readable */
#define START_BODY                              \
    if (!exception_raised)                      \
      {
             
#define CHECK_EXC                               \
      }                                         \
    if (!exception_raised)                      \
      {
             
#define RAISE_EXC_IF(_condition,_event)         \
        if (_condition)                         \
            raise_exception (_event);           \
      }                                         \
    if (!exception_raised)                      \
      {

#define END_BODY                                \
      }

#define TRACE_VAR(_VAR, _CS)          \
    printf ("%s(%i) - %-10s : [%"#_CS"]\n", __FILE__, __LINE__, #_VAR, _VAR)

#define qid_equals_zero(_qid)      ((_qid.node == 0) && (_qid.ident == 0))


#if (defined (DO_TRACE))
#  define TRACE_SMTP(_string){\
    FILE *file;\
\
    file = file_open ("smtp.log", 'a');\
    if (file)\
      {\
        fprintf (file, _string);\
        file_close (file);\
      }\
}
#else
#  define TRACE_SMTP(_string)
#endif



typedef struct                          /*  Thread context block:            */
{
    event_t
        thread_type;                    /*  Thread type indicator            */
    QID
        reply_to;                       /*  Queue to reply to                */
    struct_smtsmtp_message
        *message;                      
    sock_t
        sock_handle;
    int
        rcpt_cnt,                       /* Used as argument for getstrfld when
                                           sending recipient one by one */
        last_error_code;
    char
        *rcpt_uids,
        *last_error_msg;
    char
        *next_recipient;
    char 
        *message_boundary,
        *plain_text_body_header;
} TCB;



/*- Function prototypes -----------------------------------------------------*/
static void  append_msg_body            (char *buffer, char *body);
static int   extra_characters_required  (char *str);
static void  set_error                  (TCB *ctxt, int code, char *msg, ...);

static char *generate_unique_boundary (void);

static int init_child_context (
    TCB                    *tcb, 
    struct_smtsmtp_message *message, 
    QID                     reply_to,
    event_t                 event_number);

static void clear_context (TCB *ctxt);


/*- Global variables used in this source file only --------------------------*/

static TCB
    *tcb;                               /*  Address thread context block     */

static QID
    sockq;                              /*  Socket agent event queue         */

#define STROUT_SIZE     (LINE_MAX)

static char
    strout[STROUT_SIZE]                                 = "";

static struct_smtsmtp_message
    *smtp_msg = NULL;


static const char *plain_text_body_footer = "\r\n";

static char operation[32];


#include "smtsmtp.d"                   /*  Include dialog data              */

/********************   INITIALISE AGENT - ENTRY POINT   *********************/

int smtsmtp_init (void)
{
    AGENT  *agent;                      /*  Handle for our agent             */
    THREAD *thread;
#   include "smtsmtp.i"                /*  Include dialog interpreter       */

    /*  Change any of the agent properties that you need to                  */
    agent-> router      = FALSE;        /*  FALSE = default                  */
    agent-> max_threads = 0;            /*  0 = default                      */

    /*                      Method name     Event value      Priority        */
    /*  Shutdown event comes from Kernel                                     */
    declare_smtlib_shutdown   (shutdown_event,    SMT_PRIORITY_MAX);

    /*  Public methods supported by this agent                               */
    declare_smtsmtp_send_message   (send_mail_event,   0);
    declare_smtsmtp_open_message   (open_mail_event,   0);
    declare_smtsmtp_message_chunk  (mail_chunk_event,  0);
    declare_smtsmtp_close_message  (close_mail_event,  0);

    /*  Reply events from socket agent                                       */
    declare_smtsock_connect_ok   (sock_connect_ok_event,  SMT_PRIORITY_HIGH);
    declare_smtsock_read_ok      (sock_read_ok_event,     SMT_PRIORITY_HIGH);
    declare_smtsock_closed       (sock_closed_event,      SMT_PRIORITY_HIGH);
    declare_smtsock_read_closed  (sock_read_closed_event, SMT_PRIORITY_HIGH);
    declare_smtsock_timeout      (sock_timeout_event,     SMT_PRIORITY_HIGH);
    declare_smtsock_error        (sock_error_event,       SMT_PRIORITY_HIGH);
    declare_smtsock_read_timeout (sock_timeout_event,     SMT_PRIORITY_HIGH);
    declare_smtsock_ok           (sock_ok_event,          SMT_PRIORITY_HIGH);

    /*  Ensure that socket i/o agent is running, else start it up            */
    if (agent_lookup (SMT_SOCKET) == NULL)
        smtsock_init ();
    if ((thread = thread_lookup (SMT_SOCKET, "")) != NULL)
        sockq = thread-> queue-> qid;
    else
        return (-1);

    /*  Create master thread */
    if ((thread = thread_create (AGENT_NAME, "master")) != NULL)
      {
        tcb = thread-> tcb;
        clear_context (tcb);
        tcb-> thread_type = master_event;
      }
    else
        return (-1);

    /*  We need random number to generate message boundaries                 */
    randomize ();

    /*  Signal okay to caller that we initialised okay                       */
    return (0);
}

static void clear_context (TCB *tcb)
{ 
    ASSERT (tcb);

    tcb-> sock_handle = INVALID_SOCKET;
    tcb-> message = NULL;
    tcb-> last_error_code = 0;

    tcb-> rcpt_uids = NULL;
    tcb-> last_error_msg = NULL;
    tcb-> next_recipient = NULL;

    tcb-> message_boundary = NULL;

}


/*************************   INITIALISE THE THREAD   *************************/

MODULE initialise_the_thread (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    ASSERT ( (tcb-> thread_type == master_event)
          || (tcb-> thread_type == send_mail_event)
          || (tcb-> thread_type == open_mail_event) );

    if (tcb-> thread_type != master_event)
      {
        ASSERT (tcb-> message);
      }

    the_next_event = tcb-> thread_type;
}


/*************************   TERMINATE THE THREAD   **************************/

MODULE terminate_the_thread (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    the_next_event = terminate_event;

    if (tcb->message)
        free_smtsmtp_message (&tcb-> message);

    close_connection (thread);

    mem_free (tcb-> rcpt_uids);
    mem_free (tcb-> last_error_msg);
    mem_free (tcb-> next_recipient);
    mem_free (tcb-> message_boundary);
    mem_free (tcb-> plain_text_body_header);
}


#define SOCK_TIMEOUT           10

/**************************   CREATE CHILD THREAD   **************************/

MODULE create_child_thread (THREAD *thread)
{
    THREAD
        *child = NULL;
    TCB
        *child_tcb = NULL;
    struct_smtsmtp_message
        *event = NULL;

    tcb = thread-> tcb;                 /*  Point to thread's context        */

  START_BODY
    get_smtsmtp_message (thread-> event-> body, &event);

    child = thread_create (AGENT_NAME, "");
    RAISE_EXC_IF (child == NULL, undefined_event);
    
    child_tcb = child-> tcb;
    clear_context (child_tcb);
    
    init_child_context (child_tcb, event, 
                        thread-> event-> sender,
                        thread-> event-> event_number);
    CHECK_EXC;
    event = NULL;

  END_BODY

    if (event)
        free_smtsmtp_message (&event);
}


static int init_child_context (
    TCB                    *tcb, 
    struct_smtsmtp_message *message, 
    QID                     reply_to,
    event_t                 event_number)
{
    ASSERT (tcb);

  START_BODY
    tcb-> thread_type = event_number;
    tcb-> message = message;
    tcb-> reply_to = reply_to;

    tcb-> message_boundary = generate_unique_boundary ();
    RAISE_EXC_IF (!tcb-> message_boundary, memory_error_event);
    tcb-> plain_text_body_header = xstrcpy (
        NULL, 
        "\r\n--", 
        tcb-> message_boundary, 
        "\r\n"
        "Content-Type: text/plain; charset=US-ASCII\r\n"
        "Content-Transfer-Encoding: 7BIT\r\n"
        "Content-description: Body of message\r\n\r\n",
        NULL
      );
    RAISE_EXC_IF (!tcb-> plain_text_body_header, memory_error_event);
  END_BODY
      
    return 0;
}

static char *
generate_unique_boundary (void)
{
    char 
        *res = NULL;

    res = mem_strdup (strprintf (
        "----=_NextPart_%03d_%04d_%08X.%08X",
        random (1000),
        random (1000),
        random (ULONG_MAX),
        random (ULONG_MAX)
      ));

    return res;
}


/****************************   BUILD RECIPIENT   ****************************/

MODULE build_recipients (THREAD *thread)
{
    int
        rcpt_uids_len = 0;
    char
        *p_buffer,
        *data;

    tcb = thread-> tcb;                 /*  Point to thread's context        */

    mem_free (tcb-> rcpt_uids);
    tcb-> rcpt_uids = NULL;

    if (tcb-> message-> dest_uids)
        rcpt_uids_len += strlen (tcb-> message-> dest_uids) + 1;

    tcb-> rcpt_uids = (char *) mem_alloc (rcpt_uids_len);

    p_buffer = tcb-> rcpt_uids;
    if (tcb-> message-> dest_uids != NULL)
      {
        data = tcb-> message-> dest_uids;
        while (*data)
            *p_buffer++ = *data++;
      }
    *p_buffer = '\0';

    tcb-> rcpt_cnt = 0;
}


/**************************   READ SERVER RESPONSE   *************************/

MODULE read_server_response (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    send_smtsock_read(
        &sockq,
        SOCK_TIMEOUT,                   /*  Timeout in seconds, zero = none  */
        tcb-> sock_handle,              /*  Socket to read from              */
        512,                            /*  Size of receiving buffer         */
        5,                              /*  Minimum data to read, zero = all */
                                        /*  5 = <d><d><d><CR><LF>            */
        (void *) SOCK_TAG_READ);        /*  User-defined request tag         */
}


/*************************   CHECK SERVER RESPONSE   *************************/

MODULE check_server_response (THREAD *thread)
{
    struct_smtsock_read_reply 
        *sock_read_ok = NULL;
    int 
        error_code;
    byte 
        saved = 0;
    char 
        *sock_read_ok_data = NULL;

    tcb = thread-> tcb;                 /*  Point to thread's context        */

  START_BODY
    get_smtsock_read_reply (thread-> event-> body, &sock_read_ok);

    if (sock_read_ok-> size <= 0)
        set_error (tcb, ERROR_CODE, "Out of memory", NULL);
    RAISE_EXC_IF (sock_read_ok-> size <= 0, memory_error_event);

    sock_read_ok_data = (char *) sock_read_ok-> data;

    saved = sock_read_ok_data[sock_read_ok-> size];
    sock_read_ok_data[sock_read_ok-> size] = 0;
    strcrop (sock_read_ok_data);

    if ((error_code = atoi (sock_read_ok_data)) > SMTP_SERVER_ERROR)
      {
         set_error (tcb, ERROR_CODE, 
                    "REFUSED by SMTP server: ", sock_read_ok_data, NULL);
         raise_exception (server_response_error_event);
      }
    CHECK_EXC;

    the_next_event = server_response_ok_event;
    sock_read_ok_data[sock_read_ok-> size] = saved;
  END_BODY

    if (sock_read_ok)
        free_smtsock_read_reply (&sock_read_ok);
}


/**************************   SEND ERROR TO CLIENT   *************************/

MODULE send_error_to_client (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    if (!qid_equals_zero(tcb-> reply_to))
        send_smtsmtp_error (&tcb-> reply_to,
                             tcb-> last_error_code,
                             tcb-> last_error_msg);
}


/***********************   BUILD SOCKET CLOSED ERROR   ***********************/

MODULE build_socket_closed_error (THREAD *thread)
{
#if 0
    STRUCT_SMTSOCK_CLOSED
        *sock_read_closed = NULL;
    int
        rc;

    tcb = thread-> tcb;                 /*  Point to thread's context        */

  START_BODY

    GET_SMTSOCK_CLOSED  (thread-> event-> body, &sock_read_closed);

    switch (sock_read_closed-> tag)
      {
        case SOCK_TAG_CONNECT:      strcpy (operation, "Connect"); break;
/*      case SOCK_TAG_READ:         strcpy (operation, "Read");    break; */
        case SOCK_TAG_WRITE:        strcpy (operation, "Write");   break;
        case SOCK_TAG_INPUT:        strcpy (operation, "Connect"); break;
        case SOCK_TAG_OUTPUT:       strcpy (operation, "Output");  break;
      }

    rc = snprintf (
        strout,
        STROUT_SIZE,
        "%s on socket[%li] failed: socket closed by peer",
        operation,
        tcb-> sock_handle);
    RAISE_EXC_IF (rc <= 0, snprintf_error_event);

    set_error (tcb, ERROR_CODE, strout, NULL);

  END_BODY

    FREE_SMTSOCK_CLOSED (&sock_read_closed);
#endif
}


/*********************   BUILD SOCKET READ CLOSED ERROR   ********************/

MODULE build_socket_read_closed_error (THREAD *thread)
{
#if 0
    struct_smtsock_read_closed
        *sock_read_read_closed = NULL;
    int
        rc;

    tcb = thread-> tcb;                 /*  Point to thread's context        */

  START_BODY
    get_smtsock_read_closed  (thread-> event-> body, &sock_read_read_closed);

    strcpy (operation, "Read");

    rc = snprintf (
        strout,
        STROUT_SIZE,
        "%s on socket[%li] failed: socket closed by peer",
        operation,
        tcb-> sock_handle);
    RAISE_EXC_IF (rc <= 0, snprintf_error_event);

    set_error (tcb, ERROR_CODE, strout, NULL);
  END_BODY

    if (sock_read_read_closed)
        free_smtsock_read_closed (&sock_read_read_closed);
#endif
}


/***********************   BUILD SOCKET TIMEOUT ERROR   **********************/

MODULE build_socket_timeout_error (THREAD *thread)
{
#if 0
    STRUCT_SMTSOCK_TIMEOUT
        *sock_read_timeout = NULL;
    int
        rc;

    tcb = thread-> tcb;                 /*  Point to thread's context        */

  START_BODY
    GET_SMTSOCK_TIMEOUT  (thread-> event-> body, &sock_read_timeout);

    /*  JS: Note that the only socket request that can timeout in this       */
    /*      is connect.                                                      */
    switch (sock_read_timeout-> tag)
      {
        case SOCK_TAG_CONNECT:      strcpy (operation, "Connect"); break;
/*          case SOCK_TAG_READ:         strcpy (operation, "Read");    break; */
        case SOCK_TAG_WRITE:        strcpy (operation, "Write");   break;
        case SOCK_TAG_INPUT:        strcpy (operation, "Connect"); break;
        case SOCK_TAG_OUTPUT:       strcpy (operation, "Output");  break;
      }

    rc = snprintf (
        strout,
        STROUT_SIZE,
        "%s failed on socket [%li]: timeout expired",
        operation,
        tcb-> sock_handle);
    RAISE_EXC_IF (rc <= 0, snprintf_error_event);

    set_error (tcb, ERROR_CODE, strout, NULL);
  END_BODY

    if (sock_read_timeout)
        FREE_SMTSOCK_TIMEOUT (&sock_read_timeout);
#endif
}


/***************************   BUILD SOCKET ERROR   **************************/

MODULE build_socket_error (THREAD *thread)
{
    struct_smtsock_error
        *sock_error = NULL;
    int
        rc;

    tcb = thread-> tcb;                 /*  Point to thread's context        */

  START_BODY
    get_smtsock_error (thread-> event-> body, &sock_error);

    switch ((long) sock_error-> tag)
      {
        case SOCK_TAG_CONNECT:      strcpy (operation, "Connect"); break;
        case SOCK_TAG_READ:         strcpy (operation, "Read");    break;
        case SOCK_TAG_WRITE:        strcpy (operation, "Write");   break;
        case SOCK_TAG_INPUT:        strcpy (operation, "Connect"); break;
        case SOCK_TAG_OUTPUT:       strcpy (operation, "Output");  break;
      }

    rc = snprintf (
        strout,
        STROUT_SIZE,
        "%s on socket[%li] failed: %s",
        operation,
        (long) tcb-> sock_handle,
        sock_error-> message);
    RAISE_EXC_IF (rc <= 0, snprintf_error_event);

    set_error (tcb, ERROR_CODE, strout, NULL);
  END_BODY

    if (sock_error)
        free_smtsock_error (&sock_error);
}

/*- Local Functions ---------------------------------------------------------*/

/* concatenates msg_body to buffer, single dot characters (.) at beginning of
   a line are duplicated and line terminators are all set to "\r\n".         */
static
void append_msg_body (char *buffer, char *body)
{
    char
        *p_end_line;

    ASSERT (buffer);
    ASSERT (body);

    buffer += strlen (buffer);          /*  Point to end of string.  */
    if (*body == '\n')
      {
        *buffer++ = '\r';
        *buffer++ = '\n';
        body++;
      }
    while (strused (body))
      {
        /* is line starting with a single DOT character ? */
        if (*body == '.')
            /* --> an extra dot is appended to buffer */
            *buffer++ = '.';

        /* line can now be appended to buffer */
        p_end_line = strchr (body, '\n');
        if (p_end_line)                 /* Was line terminator found?        */
          {
            memcpy (buffer, body, p_end_line - body);
            buffer += p_end_line - body;
            if (* (p_end_line - 1) != '\r')
                *buffer++ = '\r';
            *buffer++ = '\n';
            body = p_end_line + 1;      /* sets body to next line          */
          }
        else
          {
            strcpy (buffer, body);
            buffer += strlen (body);
            body   += strlen (body);
          }
      }
    *buffer = '\0';      /*  Add final string terminator  */
}


/* Returns the number of extra characters required to send this body.  This  */
/* is the number of '.' at start of line plus the number of lines ending in  */
/* only \n with no \r.                                                       */
static
int extra_characters_required (char *str)
{
    int 
        res = 0;
    char
        *ptr;

    if ( !str || strnull (str) )
        return 0;

    ptr = str;
    if (*ptr == '\n')
      {
        res++;
        ptr++;
      }
    if (*ptr == '.')
      {
        res++;
        ptr++;
      }

    while ( (ptr = strchr (ptr, '\n')) != NULL )
      {
        if (*(ptr - 1) != '\r')
            res++;
        ptr++;
        if (*ptr == '.')
            res++;
      }

    return res;
}

/* stores an error code and message in thread context. */
static
void set_error (TCB *ctxt, int code, char *msg, ...)
{
    const char
        *src_ptr;
    va_list
        va;
    size_t
        msg_size;                       /*  Size of concatenated strings     */

    ctxt-> last_error_code = code;

    if (ctxt-> last_error_msg)
      {
        mem_free(ctxt-> last_error_msg);
        ctxt-> last_error_msg = NULL;
      }
    va_start (va, msg);                 /*  Start variable args processing   */
    src_ptr   = msg;
    msg_size = 0;
    while (src_ptr)
      {
        msg_size += strlen (src_ptr);
        src_ptr = va_arg (va, char *);
      }
    va_end (va);                        /*  End variable args processing     */

    ctxt-> last_error_msg = (char *) mem_alloc (msg_size+1);
    if (ctxt-> last_error_msg == NULL)
      {
        raise_exception (memory_error_event);
        return;                         /*  Not enough memory                */
      }

    /*  Now copy strings into destination buffer                             */
    va_start (va, msg);                 /*  Start variable args processing   */
    src_ptr  = msg;
    strclr (ctxt-> last_error_msg);
    while (src_ptr)
      {
        strcat (ctxt-> last_error_msg, src_ptr);
        src_ptr = va_arg (va, char *);
      }
    va_end (va);                        /*  End variable args processing     */

    ctxt-> last_error_msg[msg_size] = 0;
}



/****************************   CLOSE CONNECTION   ***************************/

MODULE close_connection (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    if (tcb-> sock_handle != INVALID_SOCKET 
          && socket_is_alive(tcb-> sock_handle))
      {
        send_smtsock_close(
            &sockq,
            0,                              /*  timeout */
            tcb-> sock_handle,              /*  socket */
            FALSE,                          /*  reply: we don't need a reply */
            (qbyte) 0);                     /*  User-defined request tag     */

        tcb-> sock_handle = (sock_t) INVALID_SOCKET;
      }
}


    
/**************************   CONNECT SMTP SERVER   **************************/

MODULE connect_smtp_server (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    ASSERT (tcb-> message);

    send_smtsock_connect(
          &sockq,
          SOCK_TIMEOUT,                 /* timeout                           */
          "tcp",                        /* protocol type                     */
          tcb-> message-> smtp_server,  /* host                              */
          "smtp",                       /* service                           */
          0,                            /*  16-bit host port, or 0           */
          0,                            /*  32-bit host address, or 0        */
          (void *) SOCK_TAG_CONNECT);   /*  User-defined request tag         */
}


/**************************   STORE SOCKET HANDLE   **************************/

MODULE store_socket_handle (THREAD *thread)
{
    struct_smtsock_connect_ok 
        *sock_ok = NULL;

    tcb = thread-> tcb;                 /*  Point to thread's context        */

    /* the socket handle is in the sock_ok replied event
       --> we unpack the event body into pending struct, get the handle,
           and free the struct (allocated by the get_... method */
    get_smtsock_connect_ok  (thread-> event-> body, &sock_ok);

    ASSERT (tcb-> sock_handle == INVALID_SOCKET);
    ASSERT (sock_ok-> socket != 0);
    tcb-> sock_handle = sock_ok-> socket;

    free_smtsock_connect_ok (&sock_ok);
}


/*******************************   WRITE HELO   ******************************/

MODULE write_helo (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    TRACE_SMTP ("\n\nNEW_MESSAGE:\n");

    /* HELO <SP> <domain> <CRLF> */
    xstrcpy (strout, "HELO ", get_hostname (), "\r\n", NULL);

    /* Just say hello to the mail server.                                    */
    send_smtsock_write(
        &sockq,
        0,                              /* Timeout in seconds, zero = none   */
        tcb-> sock_handle,              /* Socket to write to                */
        (word) strlen (strout),         /* Amount of data to write           */
        strout,                         /* Block of data to write            */
        FALSE,                          /* Whether to always reply           */
        (void *) SOCK_TAG_WRITE);       /* User-defined request tag          */

    TRACE_SMTP (strout);
}




/***************************   WRITE FROM MAILER   ***************************/

MODULE write_from_mailer (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    ASSERT (tcb-> message);

    xstrcpy (strout, "MAIL FROM:<", tcb-> message-> sender_uid, ">\r\n", NULL);

    send_smtsock_write(
        &sockq,
        0,                              /* Timeout in seconds, zero = none */
        tcb-> sock_handle,              /* Socket to write to */
        (word) strlen (strout),         /* Amount of data to write */
        strout,                         /* Block of data to write */
        FALSE,                          /* Whether to always reply           */
        (void *) SOCK_TAG_WRITE);       /* User-defined request tag */

    TRACE_SMTP (strout);
}


/**************************   CHECK RECIPIENT LEFT   *************************/

MODULE check_recipient_left (THREAD *thread)
{
    static char
        buffer[512];
  
    tcb = thread-> tcb;                 /*  Point to thread's context        */
    
    mem_free (tcb-> next_recipient);
    tcb-> next_recipient = NULL;
    
    getstrfld (tcb-> rcpt_uids,
               tcb-> rcpt_cnt++,
               0,
               ",;",
               buffer);

    if (strnull (buffer))
        the_next_event = no_event;
    else
      {
        the_next_event = yes_event;
        tcb-> next_recipient = mem_strdup (buffer);
      }
}


/**************************   WRITE NEXT RECIPIENT   *************************/

MODULE write_next_recipient (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    ASSERT (tcb-> next_recipient != NULL);

    /* RCPT <SP> TO:<forward-path> <CRLF> */
    xstrcpy (strout, "RCPT TO:<", tcb-> next_recipient, ">\r\n", NULL);
    send_smtsock_write(
        &sockq,
        0,                              /* Timeout in seconds, zero = none */
        tcb-> sock_handle,              /* Socket to write to */
        (word) strlen (strout),         /* Amount of data to write */
        strout,                         /* Block of data to write */
        FALSE,                           /* Whether to always reply     */
        (void *) SOCK_TAG_WRITE);        /* User-defined request tag */
    
    TRACE_SMTP (strout);
}


/***************************   WRITE DATA HEADER   ***************************/

MODULE write_data_header (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    /* DATA <CRLF> */
    xstrcpy (strout, "DATA\r\n", NULL);
    send_smtsock_write(
        &sockq,
        0,                              /* Timeout in seconds, zero = none */
        tcb-> sock_handle,              /* Socket to write to */
        (word) strlen (strout),         /* Amount of data to write */
        strout,                         /* Block of data to write */
        FALSE,                          /* Whether to always reply           */
        (void *) SOCK_TAG_WRITE);       /* User-defined request tag */

    TRACE_SMTP (strout);
}


/***************************   WRITE BODY HEADER   ***************************/

MODULE write_body_header (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */
    smtp_msg = tcb-> message;           /*  only for readability             */

    *strout = '\0';                     /*   clears out buffer */

    /* Set the date and time of the message.                                  */
    xstrcat ( strout, "Date: ", encode_mime_time (date_now (), time_now ()),
              " \r\n", NULL);

    replacechrswith (smtp_msg-> dest_uids, ";", ',');
    xstrcat (strout, "To: ", smtp_msg-> dest_uids, "\r\n", NULL);
    
    if ( strstr( smtp_msg-> sender_uid, "<" ) != NULL &&
         strstr( smtp_msg-> sender_uid, ">" ) != NULL )
        xstrcat (strout, "Reply-To:",  smtp_msg-> sender_uid, "\r\n", NULL);
    else
        xstrcat (strout, "Reply-To:<", smtp_msg-> sender_uid, ">\r\n", NULL);

    xstrcat (strout, "Sender:", smtp_msg-> sender_uid, "\r\n", NULL);
    xstrcat (strout, "From:",   smtp_msg-> sender_uid, "\r\n", NULL);

    xstrcat (strout, "X-Mailer: sflmail function\r\n", NULL);

    /* Set the mime version. */
    xstrcat (strout, "MIME-Version: 1.0\r\n",
            "Content-Type: Multipart/Mixed; boundary=\"",
            tcb-> message_boundary,
            "\"\r\n",
            NULL);

    /* Send the subject and message body. */
    ASSERT (smtp_msg-> subject != NULL); /* XXX I'm not too sure */
    xstrcat (strout, "Subject:", smtp_msg-> subject, "\r\n\r\n", NULL);

    /* Send the plain text body header */
    xstrcat (strout, tcb-> plain_text_body_header, NULL);

    send_smtsock_write(
        &sockq,
        0,                              /* Timeout in seconds, zero = none */
        tcb-> sock_handle,              /* Socket to write to */
        (word) strlen (strout),         /* Amount of data to write */
        strout,                         /* Block of data to write */
        FALSE,                          /* Whether to always reply           */
        (void *) SOCK_TAG_WRITE);       /* User-defined request tag */
    TRACE_SMTP (strout);
}


/***********************   GENERATE THREAD TYPE EVENT   **********************/

MODULE generate_thread_type_event (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    the_next_event = tcb-> thread_type;
}


/****************************   WRITE TEXT BODY   ****************************/

MODULE write_text_body (THREAD *thread)
{
    char
        *body_buffer;
    int
        body_size;

    tcb = thread-> tcb;                 /*  Point to thread's context        */
    smtp_msg = tcb-> message;           /*  only for readability             */


    if (smtp_msg-> msg_body && *smtp_msg-> msg_body)
      {
  START_BODY
        body_size = strlen (smtp_msg-> msg_body)
                  + extra_characters_required (smtp_msg-> msg_body);

        body_buffer = (char *) mem_alloc (body_size + 1);
        RAISE_EXC_IF (!body_buffer, memory_error_event);

        strclr (body_buffer);
        append_msg_body (body_buffer, smtp_msg-> msg_body);

        send_smtsock_writeh(
            &sockq,
            0,                              /* Timeout in sec, zero = none */
            tcb-> sock_handle,              /* Socket to write to */
            (qbyte)strlen(body_buffer),     /* Amount of data to write */
            (byte*)body_buffer,             /* Block of data to write */
            FALSE,                          /* Whether to always reply */
            (void *) SOCK_TAG_WRITE);       /* User-defined request tag */
        TRACE_SMTP (body_buffer);
        mem_free (body_buffer);
  END_BODY
      }
    /*  Free up this piece of the message now to save memory.  */
    mem_strfree (& smtp_msg-> msg_body);
}


/****************************   WRITE TEXT CHUNK   ***************************/

MODULE write_text_chunk (THREAD *thread)
{
    struct_smtsmtp_chunk
        *event = NULL;
    char
        *body_buffer;
    int
        body_size;

    tcb = thread-> tcb;                 /*  Point to thread's context        */

    get_smtsmtp_chunk (thread-> event-> body, &event);

    if (event-> chunk && *event-> chunk)
      {
  START_BODY
        body_size = strlen (event-> chunk)
                  + extra_characters_required (event-> chunk);

        body_buffer = (char *) mem_alloc (body_size + 1);
        RAISE_EXC_IF (!body_buffer, memory_error_event);

        strclr (body_buffer);
        append_msg_body (body_buffer, event-> chunk);

        send_smtsock_writeh(
            &sockq,
            0,                              /* Timeout in sec, zero = none */
            tcb-> sock_handle,              /* Socket to write to */
            (qbyte)strlen(body_buffer),     /* Amount of data to write */
            (byte*)body_buffer,             /* Block of data to write */
            FALSE,                          /* Whether to always reply */
            (void *) SOCK_TAG_WRITE);       /* User-defined request tag */
        TRACE_SMTP (body_buffer);
        mem_free (body_buffer);
  END_BODY
      }

    if (event)
        free_smtsmtp_chunk (&event);
}


/**********************   WAIT SOCKET READY FOR OUTPUT   *********************/

MODULE wait_socket_ready_for_output (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    send_smtsock_output(
        &sockq, 
            0,                              /* Timeout in sec, zero = none */
            tcb-> sock_handle,              /* Socket to write to */
            (void *) SOCK_TAG_OUTPUT);      /* User-defined request tag */
}


/*******************************   WRITE END   *******************************/

MODULE write_end (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    ASSERT (tcb-> message_boundary != NULL);

    /* This ends the message. */
    xstrcpy (strout,
             plain_text_body_footer, 
             "\r\n--", tcb-> message_boundary, "--\r\n\r\n.\r\n", NULL);

    send_smtsock_write(
        &sockq,
        0,                              /* Timeout in seconds, zero = none */
        tcb-> sock_handle,              /* Socket to write to */
        (word) strlen (strout),         /* Amount of data to write */
        strout,                         /* Block of data to write */
        FALSE,                          /* Whether to always reply           */
        (void *) SOCK_TAG_WRITE);       /* User-defined request tag */

    TRACE_SMTP (strout);
}


/*******************************   WRITE QUIT   ******************************/

MODULE write_quit (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    /* This ends the message. */
    xstrcpy (strout, "QUIT\r\n", NULL);

    send_smtsock_write(
        &sockq,
        0,                              /* Timeout in seconds, zero = none */
        tcb-> sock_handle,              /* Socket to write to */
        (word) strlen (strout),         /* Amount of data to write */
        strout,                         /* Block of data to write */
        FALSE,                          /* Whether to always reply           */
        (void *) SOCK_TAG_WRITE);       /* User-defined request tag */

    TRACE_SMTP (strout);
}


/**************************   BUILD TIMEOUT ERROR   **************************/

MODULE build_timeout_error (THREAD *thread)
{
    struct_smtsock_read_reply
        *event = NULL;
    int
        rc;

    tcb = thread-> tcb;                 /*  Point to thread's context        */

  START_BODY
    get_smtsock_read_reply  (thread-> event-> body, &event);

    /*  JS: Note that the only socket request that can timeout in this       */
    /*      is connect.                                                      */
    switch ((long) event-> tag)
      {
        case SOCK_TAG_CONNECT:      strcpy (operation, "Connect"); break;
        case SOCK_TAG_READ:         strcpy (operation, "Read");    break;
        case SOCK_TAG_WRITE:        strcpy (operation, "Write");   break;
      }

    rc = snprintf (
        strout,
        STROUT_SIZE,
        "%s failed on socket [%li]: timeout expired",
        operation,
        (long) tcb-> sock_handle);
    RAISE_EXC_IF (rc <= 0, snprintf_error_event);

    set_error (tcb, ERROR_CODE, strout, NULL);
  END_BODY

    if (event)
        free_smtsock_read_reply (&event);
}


/***************************   SEND OK TO CLIENT   ***************************/

MODULE send_ok_to_client (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    send_smtsmtp_ok (&tcb-> reply_to, 0, "Mail successfully sent");
}

