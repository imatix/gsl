/*===========================================================================*
 *                                                                           *
 *  smtpop.c - POP3 email agent                                              *
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

#define AGENT_NAME   "smtpop"          /*  Our public name                  */

#define CONNECT_TIMEOUT      (0)
#define CHUNK_REALLOC_SIZE   (64)

#define is_lwsp(CH)     ( ((CH) == ' ') || ((CH) == '\t') )


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


/* defines loops on enumeration type, usind IDX as 'iterator'                */
#define for_enum(ENUM_TYPE, IDX)        \
  for (IDX=(ENUM_TYPE)0; IDX<ENUM_TYPE##_SIZE; IDX++)

#define is_enum_value_valid(ENUM_TYPE, VALUE)                                \
    ( ((VALUE) >= (ENUM_TYPE)0) && ((VALUE) < ENUM_TYPE##_SIZE) )

#define TRACE_VAR(_VAR, _CS)          \
    printf ("%s(%i) - %-10s : [%"#_CS"]\n", __FILE__, __LINE__, #_VAR, _VAR)



#define XXX_ERROR_CODE      10

typedef enum {
    HF_UNDEFINED       = -1,
    HF_MEDIA_TYPE,
    HF_FROM,
    HF_TO,
    HF_CC,
    HF_DATE,
    HF_SUBJECT,
    HF_SENDER,
    header_field_enum_SIZE
} header_field_enum;

typedef enum {
    MT_UNDEFINED   = -1,   /* don't change or 'for_enum' macro will be wrong */
    MT_TEXT,
    MT_IMAGE,
    MT_AUDIO,
    MT_VIDEO,
    MT_APPLICATION,
    MT_MULTIPART,
    MT_MESSAGE,
    media_type_enum_SIZE
  } media_type_enum;


typedef enum {
    PHF_UNDEFINED       = -1,
    PHF_MEDIA_TYPE,
    PHF_TRANSFER_ENCODING,
    PHF_DISPOSITION,
    part_header_field_enum_SIZE
} part_header_field_enum;


typedef enum {
    NONE                = 0,
    MULTI_LINE,
    SINGLE_LINE
} response_type;

typedef struct
{
    byte *data;
    int   size;
} chunk_struct;


typedef struct
{
    response_type   type;
    chunk_struct   *chunks;             /* array                             */
    int             chunk_count;        /* used array slots                  */
    int             array_size;         /* allocated array slots             */
} chunked_response_struct;


typedef struct
{
    /* ptr means it points to an ALREADY allocated string. Don't free it !! */
    const char
        *ptr_header_fields_value[header_field_enum_SIZE];
    const char
        *ptr_body;                      /* ie body AND attachment as a (large) string */
} local_message_struct;


typedef struct                          /*  Thread context block:            */
{
    event_t
        thread_type;                    /*  Thread type indicator            */
    QID
        reply_to;                       /* Queue to reply to                 */
    sock_t
        sock_handle;                    /* connected socket                  */
    char
        *password,                      /* from connection request           */
        *user,                          /* from connection request           */
        *server,                        /* from connection request           */
        *attach_dir,                    /* for MSG request, dir where attachments */
                                        /* will be saved (if any).           */
        *multipart_boundary,
        *str_resp;                      /* large buffer containing last message */
                                        /* or message header. msg_struct fields */
                                        /* may point directly to it at several pos */
    int
        message_count,
        msg_id;                         /* msg id related to last request    */
    int
        total_size;
    chunked_response_struct
        chunked_resp;
    local_message_struct
        msg_struct;                     /* fields are pointers to null-terminated
                                         * strings, each located in str_resp,
                                         * none of them overlaps */
    media_type_enum
        media_type;
} TCB;


static void reset_chunked_resp (response_type expected);
static Bool get_lattest_chars  (char *buf, word count);

static void clear_message_struct    (void);
static void default_message_struct  (void);

static const char *remove_crlflwp (const char *header_field);
static char *get_boundary_value   (const char *src);

static const char *handle_message_body (char **attach_list);



/*- Function prototypes -----------------------------------------------------*/


/*- Global variables used in this source file only --------------------------*/

static TCB
    *tcb;                               /*  Address thread context block     */

static QID
    sockq;                              /*  Socket agent event queue         */


#define STROUT_SIZE       (514)
static char
    strout[STROUT_SIZE] = "";

static const char
    *media_types[media_type_enum_SIZE] = {
        "text",                     /* MT_TEXT                           */
        "image",                    /* MT_IMAGE                          */
        "audio",                    /* MT_AUDIO                          */
        "video",                    /* MT_VIDEO                          */
        "application",              /* MT_APPLICATION                    */
        "multipart",                /* MT_MULTIPART                      */
        "message"                   /* MT_MESSAGE                        */
      };

static const char
    *default_media_type = "text/plain; charset=us-ascii";


static const char
    *header_field_names[header_field_enum_SIZE] = {
        "Content-Type:",
        "From:",
        "To:",
        "Cc:",
        "Date:",
        "Subject:",
        "Sender:"
    };



static const char
    *part_header_field_names[part_header_field_enum_SIZE] = {
        "Content-Type:",
        "Content-Transfer-Encoding:",
        "Content-Disposition:"
    };


#include "smtpop.d"                    /*  Include dialog data              */

/********************   INITIALISE AGENT - ENTRY POINT   *********************/

int smtpop_init (void)
{
    AGENT  *agent;                      /*  Handle for our agent             */
    THREAD *thread;
#   include "smtpop.i"                 /*  Include dialog interpreter       */

    /*  Change any of the agent properties that you need to                  */
    agent-> router      = FALSE;        /*  FALSE = default                  */
    agent-> max_threads = 0;            /*  0 = default                      */

    /*                      Method name     Event value      Priority        */
    /*  Shutdown event comes from Kernel                                     */
    declare_smtlib_shutdown   (shutdown_event,    SMT_PRIORITY_MAX);

    /*  Public methods supported by this agent                               */
    declare_smtpop_connect              (connection_request_event,      0);
    declare_smtpop_get_session_info     (session_info_request_event,    0);
    declare_smtpop_get_msg_info         (message_info_request_event,    0);
    declare_smtpop_quit                 (quit_request_event,            0);
    declare_smtpop_get_msg              (message_request_event,         0);
    declare_smtpop_get_msg_header       (header_request_event,          0);
    declare_smtpop_delete_msg           (del_request_event,             0);

    /*  Reply events from socket agent                                       */
    declare_smtsock_connect_ok (sock_connect_ok_event, 0);
    declare_smtsock_read_ok    (sock_read_ok_event,    0);
    declare_smtsock_closed      (sock_closed_event,     0);
    declare_smtsock_read_closed (sock_closed_event,     0);
    declare_smtsock_timeout     (sock_timeout_event,    0);
    declare_smtsock_error       (sock_error_event,      0);

    /* private method for Master - Child  management                         */
    method_declare (agent, "_MASTER",     master_event,         0);
    method_declare (agent, "_CHILD",      client_event,         0);

    /*  Create master thread */
    if ((thread = thread_create (AGENT_NAME, "")) != NULL)
      {
        SEND (&thread-> queue-> qid, "_MASTER", "");
        tcb = thread-> tcb;
        tcb-> thread_type = master_event;
      }
    else
        return (-1);

    /*  Ensure that socket i/o agent is running, else start it up            */
    smtsock_init ();
    if ((thread = thread_lookup (SMT_SOCKET, "")) != NULL)
        sockq = thread-> queue-> qid;
    else
        return (-1);

    /*  Signal okay to caller that we initialised okay                       */
    return (0);
}


/*************************   INITIALISE THE THREAD   *************************/

MODULE initialise_the_thread (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    ASSERT ( (tcb-> thread_type == client_event)
          || (tcb-> thread_type == master_event) );

    tcb-> sock_handle         = 0;
    tcb-> message_count       = 0;
    tcb-> total_size          = 0;
    tcb-> msg_id              = 0;
    tcb-> attach_dir          = NULL;
    tcb-> str_resp            = NULL;
    tcb-> media_type          = MT_UNDEFINED;
    tcb-> multipart_boundary  = NULL;

    memset (&tcb-> chunked_resp, 0, sizeof (tcb-> chunked_resp));
    clear_message_struct ();

    if (tcb-> thread_type == master_event)
      {
        tcb-> password      = NULL;
        tcb-> user          = NULL;
        tcb-> server        = NULL;
      }
}


/*************************   TERMINATE THE THREAD   **************************/

MODULE terminate_the_thread (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */
    the_next_event = terminate_event;

    if (tcb-> password != NULL)
      {
        mem_free (tcb-> password);
        tcb-> password = NULL;
      }
    if (tcb-> user)
      {
        mem_free (tcb-> user);
        tcb-> user = NULL;
      }
    if (tcb-> server)
      {
        mem_free (tcb-> server);
        tcb-> server = NULL;
      }
    if (tcb-> attach_dir)
      {
        mem_free (tcb-> attach_dir);
        tcb-> attach_dir = NULL;
      }

    release_response (thread);

    if (tcb-> chunked_resp. chunks != NULL)
      {
        mem_free (tcb-> chunked_resp. chunks);
        tcb-> chunked_resp. chunks = NULL;
      }

    do_close_socket (thread);

    the_next_event = terminate_event;
}


/**************************   CREATE CHILD THREAD   **************************/

MODULE create_child_thread (THREAD *thread)
{
    THREAD *child = NULL;
    char   *child_name = NULL;
    struct_smtpop_connection *connect_param = NULL;

    tcb = thread-> tcb;                 /*  Point to thread's context        */

    get_smtpop_connection (thread-> event-> body, &connect_param);

    /* build the string "user@server" */
    child_name = xstrcpy (NULL, connect_param-> user,
                          "@",
                          connect_param-> server, NULL);

    if (thread_lookup (AGENT_NAME, child_name) != NULL)
        /* a connection has already been established for that user */
        send_smtpop_error (
            &thread-> event-> sender,
            "mailbox is locked by another process",
            XXX_ERROR_CODE
          );
    else
    if ((child = thread_create (AGENT_NAME, child_name)) == NULL)
          raise_exception (memory_error_event);

    else                                /* GENERAL CASE */
      {
        SEND (&child-> queue-> qid, "_CHILD", "");
        tcb = child-> tcb;
        memset (tcb, 0, sizeof (TCB));
        /* Here is the only place where master can give info to child */
        tcb-> thread_type = client_event;
        tcb-> reply_to = thread-> event-> sender;

        tcb-> server = connect_param-> server;
        tcb-> user = connect_param-> user;
        tcb-> password = connect_param-> password;

        /* we'll take care of freeing these strings
         * ---> we set fields to NULL for free_smtpop_connection to not
         * free them now*/
        connect_param-> server   = NULL;
        connect_param-> user     = NULL;
        connect_param-> password = NULL;
      }

    mem_free (child_name);
    if (connect_param)
      free_smtpop_connection (&connect_param);
}


/***************************   DO CONNECT SOCKET   ***************************/

MODULE do_connect_socket (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    reset_chunked_resp (SINGLE_LINE);
    send_smtsock_connect(
      &sockq,
      CONNECT_TIMEOUT,                  /* timeout                           */
      "tcp",                            /* protocol type                     */
      tcb-> server,                     /* host                              */
      "110",                            /* service                           */
      0,                                /* 16-bit host port, or 0            */
      0,                                /* 32-bit host address, or 0         */
      0);                               /* User-defined request tag          */
}

/***************************   GET SOCKET HANDLE   ***************************/

MODULE get_socket_handle (THREAD *thread)
{
    struct_smtsock_connect_ok *sock_ok = NULL;

    tcb = thread-> tcb;                 /*  Point to thread's context        */

    get_smtsock_connect_ok  (thread-> event-> body, &sock_ok);

    ASSERT (tcb-> sock_handle == 0);  /* will be init ONCE !!! */
    tcb-> sock_handle = sock_ok-> socket;

    free_smtsock_connect_ok (&sock_ok);
}


/**************************   READ SERVER RESPONSE   *************************/

MODULE read_repeatedly_server_response (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    send_smtsock_readr(&sockq,
               0,                    /*  Timeout in seconds, zero = none  */
               tcb-> sock_handle,    /*  Socket to read from              */
               512,                  /*  Size of receiving buffer         */
               1,                    /*  Minimum data to read, zero = all */
               0);                   /*  User-defined request tag         */
}


/*****************************     BUILD RESPONSE   ****************************/

MODULE build_response (THREAD *thread)
{
    struct_smtsock_read_reply
        *sock_read_ok = NULL;
    chunked_response_struct
        *resp = NULL;
    char
        first_char;

    tcb = thread-> tcb;                 /*  Point to thread's context        */
    resp = &tcb-> chunked_resp;        /*  only for readability             */

    get_smtsock_read_reply (thread-> event-> body, &sock_read_ok);
    
    if (resp-> chunk_count == resp-> array_size)
      {
        /* realloc chunk array if necessary */
        resp-> array_size += CHUNK_REALLOC_SIZE;
        if (resp-> chunk_count == 0)
          resp-> chunks = mem_alloc (resp-> array_size * sizeof (chunk_struct));
        else
          resp-> chunks = mem_realloc (resp-> chunks, resp-> array_size * sizeof (chunk_struct));

        if (resp-> chunks == NULL)
          {
            raise_exception (memory_error_event);
            free_smtsock_read_reply (&sock_read_ok);
            return ;
          }
      }

    ASSERT (sock_read_ok->size > 0);
    ASSERT (sock_read_ok->data != NULL);
    resp-> chunks[resp-> chunk_count]. size = sock_read_ok-> size;
    resp-> chunks[resp-> chunk_count]. data = sock_read_ok-> data;
    resp-> chunk_count++;

    if (resp-> chunk_count == 1)
      {
        first_char = ((char *)sock_read_ok-> data)[0];

        /* first piece of the response, we check whether response is ok */
        /* we're expecting a minus (-) or plus (+) as first char of response */
        if (first_char == '-')
            resp-> type = SINGLE_LINE;    /* error response are always a single line */
        else
        if (first_char != '+')
            raise_exception (server_response_error_event);
      }

    sock_read_ok-> data = NULL;           /* the data is part of chunk struct now */
                                          /* we'll manage desallocation within the agent*/
    free_smtsock_read_reply (&sock_read_ok);
}


/*************************   CHECK SERVER RESPONSE   *************************/

MODULE check_server_response (THREAD *thread)
{
    chunked_response_struct
        *resp;
    char buf[6];

    tcb = thread-> tcb;                 /*  Point to thread's context        */
    resp = &tcb-> chunked_resp;        /*  only for readability             */

    /* we must here find out :
     * 1) is the response complete ? (ie all chunks had been read)
     * 2) positive or negative response
     */

    if (resp-> type == SINGLE_LINE)
      {
        /* we look at the 2 lattest received char : CRLF ?                   */
        if (! get_lattest_chars (buf, 2))
            return;

        if (strcmp(buf, "\r\n") != 0)
            return;

        if (resp-> chunks[0].data[0] == '+')
            the_next_event = server_positive_response_event;
        else
        if (resp-> chunks[0].data[0] == '-')
            the_next_event = server_negative_response_event;
        else
          {
            raise_exception (server_response_error_event);
            return;
          }
      }
    else
    if (resp-> type == MULTI_LINE)
      {
        /* we look at the 5 lattest received char : CRLF . CRLF ?            */
        if (! get_lattest_chars (buf, 5))
            return;
        if (strcmp(buf, "\r\n.\r\n") != 0)
            return;
        /* \r\n.\r\n has been received, meaning the response is complete */
        if (resp-> chunks[0].data[0] == '+')
            the_next_event = server_positive_response_event;
        else
        if (resp-> chunks[0].data[0] == '-')
            the_next_event = server_negative_response_event;
        else
          {
            raise_exception (server_response_error_event);
            return;
          }
      }
    else
      {
        raise_exception (undefined_error_event);  /* should not happen */
        return;
      }

    ASSERT ( (the_next_event == server_positive_response_event)
          || (the_next_event == server_negative_response_event) );

    /* response is complete, we stop reading socket for response chunks */
    send_smtsock_flush (&sockq, tcb-> sock_handle, TRUE);
}


/***************************   SEND USER COMMAND   ***************************/

MODULE send_user_command (THREAD *thread)
{
    int
        rc;

    tcb = thread-> tcb;                 /*  Point to thread's context        */

  START_BODY
    ASSERT (tcb-> user != NULL);
    rc = snprintf (strout, STROUT_SIZE, "USER %s\r\n", tcb-> user);
    RAISE_EXC_IF (rc <= 0, snprintf_error_event);

    reset_chunked_resp (SINGLE_LINE);

    send_smtsock_write(
        &sockq,
        0,                              /* Timeout in seconds, zero = none */
        tcb-> sock_handle,              /* Socket to write to */
        (word) strlen (strout),         /* Amount of data to write */
        strout,                         /* Block of data to write */
        FALSE,                          /* Whether to always reply           */
        0);                             /* User-defined request tag */
  END_BODY
}


/*************************   SEND PASSWORD COMMAND   *************************/

MODULE send_password_command (THREAD *thread)
{
    int
        rc;

    tcb = thread-> tcb;                 /*  Point to thread's context        */

  START_BODY
    ASSERT (tcb-> password != NULL);
    rc = snprintf (strout, STROUT_SIZE, "PASS %s\r\n", tcb-> password);
    RAISE_EXC_IF (rc <= 0, snprintf_error_event);

    reset_chunked_resp (SINGLE_LINE);
    send_smtsock_write(
        &sockq,
        0,                              /* Timeout in seconds, zero = none */
        tcb-> sock_handle,              /* Socket to write to */
        (word) strlen (strout),         /* Amount of data to write */
        strout,                         /* Block of data to write */
        FALSE,                          /* Whether to always reply           */
        0);                             /* User-defined request tag */
  END_BODY
}


/********************* GET MESSAGE COUNT AND SIZE ****************************/

MODULE get_message_count_and_size (THREAD *thread)
{
    char *stat = NULL;
    char *ptr = NULL;

    tcb = thread-> tcb;                 /*  Point to thread's context        */

    ASSERT (tcb-> str_resp != NULL);

  START_BODY
    stat = tcb-> str_resp;

    ASSERT (strlen (stat) > 4);
    ASSERT (strncmp (stat, "+OK ", 4) == 0);

    ptr = &stat[4];
    tcb-> message_count = atoi (ptr);

    ptr = strchr (ptr, ' ');
    RAISE_EXC_IF (ptr == NULL, server_response_error_event);

    ptr++;
    RAISE_EXC_IF (!isdigit(*ptr), server_response_error_event);

    tcb-> total_size = atoi (ptr);

    /*  count == 0  ==>  size == 0
        count != 0  ==>  size != 0 */
    RAISE_EXC_IF (tcb-> message_count == 0 ? tcb-> total_size != 0
                                           : tcb-> total_size == 0,
                  server_response_error_event);
  END_BODY
}


/*******************   SEND CONNECTION ACCEPTED TO CLIENT   ******************/

MODULE send_connection_accepted_to_client (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    send_smtpop_connect_ok (&tcb-> reply_to,
                            tcb-> message_count,
                            tcb-> total_size);
}


/***************************   SEND STAT COMMAND   ***************************/

MODULE send_stat_command (THREAD *thread)
{
    int
        rc;
    tcb = thread-> tcb;                 /*  Point to thread's context        */

  START_BODY
    rc = snprintf (strout, sizeof(strout), "STAT\r\n");
    RAISE_EXC_IF (rc <= 0, snprintf_error_event);

    reset_chunked_resp (SINGLE_LINE);
    send_smtsock_write(
        &sockq,
        0,                              /* Timeout in seconds, zero = none */
        tcb-> sock_handle,              /* Socket to write to */
        (word) strlen (strout),         /* Amount of data to write */
        strout,                         /* Block of data to write */
        FALSE,                          /* Whether to always reply           */
        0);                             /* User-defined request tag */
  END_BODY
}


/***************************   SEND QUIT COMMAND   ***************************/

MODULE send_quit_command (THREAD *thread)
{
    int
        rc;
    tcb = thread-> tcb;                 /*  Point to thread's context        */

  START_BODY
    rc = snprintf (strout, STROUT_SIZE, "QUIT\r\n");
    RAISE_EXC_IF (rc <= 0, snprintf_error_event);

    reset_chunked_resp (SINGLE_LINE);
    send_smtsock_write(
        &sockq,
        0,                              /* Timeout in seconds, zero = none */
        tcb-> sock_handle,              /* Socket to write to */
        (word) strlen (strout),         /* Amount of data to write */
        strout,                         /* Block of data to write */
        FALSE,                          /* Whether to always reply           */
        0);                             /* User-defined request tag */
  END_BODY
}


/**********************   SEND SESSION INFO TO CLIENT   **********************/

MODULE send_session_info_to_client (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    send_smtpop_session_info (&tcb-> reply_to,
                          tcb-> message_count,
                          tcb-> total_size);
}


/******************************   SEND QUIT OK   *****************************/

MODULE send_quit_ok_to_client (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    send_smtpop_quit_ok(&tcb-> reply_to);
}


/***************************   SEND RETR COMMAND   ***************************/

MODULE send_retr_command (THREAD *thread)
{
    int
        rc;
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    ASSERT (tcb-> msg_id > 0);
    ASSERT (tcb-> msg_id <= tcb-> message_count);

  START_BODY
    rc = snprintf (strout, STROUT_SIZE, "RETR %i\r\n", tcb-> msg_id);
    RAISE_EXC_IF (rc <= 0, snprintf_error_event);

    reset_chunked_resp (MULTI_LINE);
    send_smtsock_write(
        &sockq,
        0,                              /* Timeout in seconds, zero = none */
        tcb-> sock_handle,              /* Socket to write to */
        (word) strlen (strout),         /* Amount of data to write */
        strout,                         /* Block of data to write */
        FALSE,                          /* Whether to always reply           */
        0);                             /* User-defined request tag */
   END_BODY
}


/************************   STORE MSG ID PARAMETERS   ************************/

MODULE store_msg_id_parameters (THREAD *thread)
{
    struct_smtpop_msg_id *param;

    tcb = thread-> tcb;                 /*  Point to thread's context        */

    get_smtpop_msg_id (thread-> event-> body, &param);

    tcb-> msg_id = param-> msg_id;
    tcb-> attach_dir = param-> attach_dir;
    param-> attach_dir = NULL;          /* we'll take care of freeing it     */

    free_smtpop_msg_id (&param);
}


/***********************   SEND TOP COMMAND TO SERVER   **********************/

MODULE send_top_command_to_server (THREAD *thread)
{
    int
        rc;
    tcb = thread-> tcb;                 /*  Point to thread's context        */

  START_BODY
    rc = snprintf (strout, STROUT_SIZE, "TOP %i 0\r\n", tcb-> msg_id);
    RAISE_EXC_IF (rc <= 0, snprintf_error_event);

    reset_chunked_resp (MULTI_LINE);
    send_smtsock_write(
        &sockq,
        0,                              /* Timeout in seconds, zero = none */
        tcb-> sock_handle,              /* Socket to write to */
        (word) strlen (strout),         /* Amount of data to write */
        strout,                         /* Block of data to write */
        FALSE,                          /* Whether to always reply           */
        0);                             /* User-defined request tag */
  END_BODY
}


/*********************   SEND MESSAGE HEADER TO CLIENT   *********************/

MODULE send_message_header_to_client (THREAD *thread)
{
    local_message_struct
        *msg;                           /* for readability only              */
    char
        *from;

    tcb = thread-> tcb;                 /*  Point to thread's context        */

    msg = &tcb-> msg_struct;
    ASSERT (msg != NULL);

    from = (char *)msg-> ptr_header_fields_value[HF_FROM]?
           (char *)msg-> ptr_header_fields_value[HF_FROM]:
           (char *)msg-> ptr_header_fields_value[HF_SENDER];

    send_smtpop_msg_header (
        &tcb-> reply_to,
        tcb-> msg_id,
        from,
        (char *)msg-> ptr_header_fields_value[HF_TO],
        (char *)msg-> ptr_header_fields_value[HF_CC],
        (char *)msg-> ptr_header_fields_value[HF_DATE],
        (char *)msg-> ptr_header_fields_value[HF_SUBJECT]
      );
}


/*************************   SEND MESSAGE TO CLIENT   ************************/

MODULE send_message_to_client (THREAD *thread)
{
    local_message_struct
        *msg;                           /* for readability only              */
    const char
        *body = NULL;
    char
        *from,
        *attach_list = NULL;

    tcb = thread-> tcb;                 /*  Point to thread's context        */

    msg = &tcb-> msg_struct;
    ASSERT (msg != NULL);

    body = handle_message_body(&attach_list);
    if (exception_raised)
        return;

    from = (char *)msg-> ptr_header_fields_value[HF_FROM]?
           (char *)msg-> ptr_header_fields_value[HF_FROM]:
           (char *)msg-> ptr_header_fields_value[HF_SENDER];

    send_smtpop_msg (
        &tcb-> reply_to,
        tcb-> msg_id,
        from,
        (char *)msg-> ptr_header_fields_value[HF_TO],
        (char *)msg-> ptr_header_fields_value[HF_CC],
        (char *)msg-> ptr_header_fields_value[HF_DATE],
        (char *)msg-> ptr_header_fields_value[HF_SUBJECT],
        (char *)body,
        attach_list,
        tcb-> attach_dir   /* XXX TMP voir si on ne remplace pas NULL et ""
                            * par "." ie working dir*/
      );

    if (attach_list != NULL)
      {
        mem_free (attach_list);
        attach_list = NULL;
      }
}



/************************   CONVERT CHUNKS TO STRING   ***********************/

MODULE convert_chunks_to_string (THREAD *thread)
{
    char *src, *dst;
    word
        chunk_idx, idx;
    dword
        size;

    tcb = thread-> tcb;                 /*  Point to thread's context        */

    size = 0;
    for (idx=0; idx<tcb-> chunked_resp.chunk_count; idx++)
        size += tcb-> chunked_resp.chunks[idx].size;

    ASSERT (tcb-> str_resp == NULL);
    tcb-> str_resp = mem_alloc (size+1);

    if (tcb-> str_resp == NULL)
      {
        raise_exception (memory_error_event);
        return;
      }

    strclr (tcb-> str_resp);
    dst = tcb-> str_resp;

    for (chunk_idx = 0;
         chunk_idx < tcb-> chunked_resp.chunk_count;
         chunk_idx++)
      {
        src = (char *) tcb-> chunked_resp.chunks[chunk_idx].data;
        for (idx=0; idx<tcb-> chunked_resp.chunks[chunk_idx].size; idx++)
          {
            ASSERT (*src);
            *dst++ = *src++;
          }
      }
    *dst = 0;

    ASSERT (strlen (tcb-> str_resp) == size); /* chunks are made of non-zero char */
    reset_chunked_resp (NONE);              /* chunks are not needed anymore --> we free them */
}


/***********************   CONVERT STRING TO STRUCT   **********************/

MODULE convert_string_to_struct (THREAD *thread)
{
    char
        *p_line;
    char
        *header_to_trim;
    header_field_enum
        idx;

    tcb = thread-> tcb;                 /*  Point to thread's context        */

    clear_message_struct ();

    ASSERT (tcb-> str_resp != NULL);

    p_line = tcb-> str_resp;
    ASSERT (strncmp (tcb-> str_resp , "+OK", 3) == 0);   /* first line of msg is server response */
    ASSERT (strstr (tcb-> str_resp , "\r\n\r\n") != NULL);
    ASSERT (strstr (tcb-> str_resp , "\r\n.\r\n") != NULL);

    p_line = strstr (p_line, "\r\n");
    p_line += 2;                        /* points now to start of a header line */


    FOREVER
      {
        if (strncmp (p_line, "\r\n", 2) == 0)
            break;                      /* end of header fields */

        header_to_trim = NULL;
        for_enum (header_field_enum, idx)
            if (strncmp (p_line,
                         header_field_names[idx],
                         strlen (header_field_names[idx])) == 0)
              {
                tcb-> msg_struct.ptr_header_fields_value[idx]
                        = p_line + strlen (header_field_names[idx]);
                header_to_trim = (char *)tcb-> msg_struct.ptr_header_fields_value[idx];
                break;
              }

        /* now, we want to null-terminate the header field body
         * --> we search next line NOT starting with linear white space (RFC 822) */
        do
          {
            p_line = strstr (p_line, "\r\n");
            ASSERT (p_line != NULL);
            p_line += 2;                /* skips CRLF                        */
          } while (is_lwsp (*p_line));

        ASSERT (*(p_line-2) == '\r');
        *(p_line - 2) = 0;              /* we null terminate the string */

        if (header_to_trim)
            remove_crlflwp (header_to_trim);
      }

    p_line += 2;                        /* skips 2nd CR LF (last header field
                                         * ends with CRLF CRLF */
    tcb-> msg_struct.ptr_body = p_line;

    default_message_struct ();
}



/****************************   RELEASE RESPONSE   ***************************/

MODULE release_response (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    reset_chunked_resp (NONE);
    clear_message_struct ();
    if (tcb-> str_resp != NULL)
      {
        mem_free (tcb-> str_resp);
        tcb-> str_resp = NULL;
      }
    if (tcb-> multipart_boundary != NULL)
      {
        mem_free (tcb-> multipart_boundary);
        tcb-> multipart_boundary = NULL;
      }
    if (tcb-> attach_dir != NULL)
      {
        mem_free (tcb-> attach_dir);
        tcb-> attach_dir = NULL;
      }
}


/******************************   CHECK MSG ID   *****************************/

MODULE check_msg_id (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    if ( (tcb-> msg_id > 0) && (tcb-> msg_id <= tcb-> message_count) )
        return;

    raise_exception (bad_message_id_event);
}

/*****************************   GET MEDIA TYPE   ****************************/

MODULE get_media_type_and_boundary (THREAD *thread)
{
    const char
        *media_header;
    media_type_enum
        idx;

    tcb = thread-> tcb;                 /*  Point to thread's context        */

    media_header = tcb-> msg_struct.ptr_header_fields_value[HF_MEDIA_TYPE];
    ASSERT (media_header);
    ASSERT (tcb-> multipart_boundary == NULL);

    for_enum (media_type_enum, idx)
      {
        if (lexncmp (media_header,
                     media_types[idx],
                     strlen (media_types[idx])) == 0)
          {
            tcb-> media_type = idx;
            break;
          }
      }

    if (idx == media_type_enum_SIZE)
      {
        raise_exception (undefined_error_event);  /* XXX TODO */
        return;
      }

    if (tcb-> media_type == MT_MULTIPART)
      {
        /* we'll need the 'boundary' value... */
        ASSERT (tcb-> multipart_boundary == NULL);
        tcb-> multipart_boundary = get_boundary_value (media_header);

        if (tcb-> multipart_boundary == NULL)
          {
            ASSERT (exception_raised);
            return ;
          }
      }
}


/**********************   SEND DELE COMMAND TO SERVER   **********************/

MODULE send_dele_command_to_server (THREAD *thread)
{
    int
        rc;
    tcb = thread-> tcb;                 /*  Point to thread's context        */

  START_BODY
    ASSERT (tcb-> user != NULL);
    rc = snprintf (strout, STROUT_SIZE, "DELE %i\r\n", tcb-> msg_id);
    RAISE_EXC_IF (rc <= 0, snprintf_error_event);

    reset_chunked_resp (SINGLE_LINE);
    send_smtsock_write(
        &sockq,
        0,                              /* Timeout in seconds, zero = none */
        tcb-> sock_handle,              /* Socket to write to */
        (word) strlen (strout),         /* Amount of data to write */
        strout,                         /* Block of data to write */
        FALSE,                          /* Whether to always reply           */
        0);                             /* User-defined request tag */
  END_BODY
}


/***********************   SEND REQUEST OK TO CLIENT   ***********************/

MODULE send_request_ok_to_client (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    send_smtpop_delete_ok (&tcb-> reply_to, tcb-> msg_id, NULL);
}





/* Local function definitions ---------------------------------------------- */

static void reset_chunked_resp (response_type expected)
{
    chunked_response_struct *resp;      /* for readabililty */
    word idx;

    resp = &tcb-> chunked_resp;
    ASSERT (resp-> chunk_count <= resp-> array_size);

    resp-> type = expected;
    for (idx=0; idx<resp-> chunk_count; idx++)
        if (resp-> chunks[idx].data != NULL)
            mem_free (resp-> chunks[idx].data);

    resp-> chunk_count = 0;
    for (idx=0; idx<resp-> array_size; idx++)
      {
        resp-> chunks[idx].data = NULL;
        resp-> chunks[idx].size = 0;
      }
}

/* fills buf with the 'count' lattest received char
 * assumes buf size is at least >= count + 1 (res is NULL terminated string)
 * returns FALSE if less than count char are currently stored in resp */
static Bool get_lattest_chars (char *buf, word count)
{
    chunked_response_struct *resp;      /* for readabililty */
    int
        chunk_idx,
        data_idx;

    ASSERT (buf  != NULL);
    ASSERT (count > 0);

    resp = &tcb-> chunked_resp;
    buf[count] = 0;                     /* on success, buf is NULL terminated */

    for (chunk_idx=resp-> chunk_count-1; chunk_idx>=0; chunk_idx--)
        for (data_idx=resp-> chunks[chunk_idx].size - 1; data_idx>=0; data_idx--)
          {
            buf[count-1] = resp-> chunks[chunk_idx]. data[data_idx];
            count--;
            if (count==0)
                return TRUE;
          }

    buf[0] = 0;
    return FALSE;
}

static
void clear_message_struct (void)
{
    memset (&tcb-> msg_struct, 0, sizeof (tcb-> msg_struct));
}

static
void default_message_struct (void)
{
    if (tcb-> msg_struct.ptr_header_fields_value[HF_MEDIA_TYPE] == NULL)
        tcb-> msg_struct.ptr_header_fields_value[HF_MEDIA_TYPE] = default_media_type;
}

static
const char *remove_crlflwp (const char *header_field)
{
    int
        size;
    char
        *src,
        *dst;

    ASSERT (header_field);
    size = strlen (header_field) + 1;

    src = (char *)header_field;
    dst = src;

    while (*src == ' ')
        src++;

    FOREVER
      {
        while ((*src != 0) && (*src != '\r'))
            *dst++ = *src++;

        if (*src == 0)
            break;

        src += 2;
        ASSERT (*src != 0);

        while (is_lwsp(*src))
            src++;
      }

    *dst = 0;

    return header_field;
}

static
char *get_boundary_value (const char *src)
{
    static const char
        *boundary_header = "boundary=\"";
    char
        *start,
        *end,
        *res;
    int
        size;

    ASSERT (src != NULL);

    start = strstr (src, boundary_header);

    if (start == NULL)
      {
        raise_exception (badly_formed_msg_error_event);
        return NULL;
      }

    start += strlen (boundary_header);
    end = start;
    while ((*end != 0) && (*end != '\"'))
        end++;

    if (*end == 0)
      {
        raise_exception (badly_formed_msg_error_event);
        return NULL;
      }

    size = end - start;
    ASSERT (size > 0);

    size += 2;                           /* "--" is prefixed */
    res = mem_alloc (size + 1);
    if (res == NULL)
      {
        raise_exception (memory_error_event);
        return NULL;
      }

    res[0] = '-';
    res[1] = '-';
    memcpy (res+2, start, size-2);
    res[size] = 0;

    return res;
}




static
Bool test_attach_dir (void)
{
  /* if no dir specified, attach are saved in current directory, which exists */
  if ((tcb-> attach_dir == NULL) || !strused(tcb->attach_dir))
      return TRUE;

  return file_exists(tcb-> attach_dir);
}



typedef struct {
    media_type_enum
        media_type;
    const char
        *ptr_header_value[part_header_field_enum_SIZE];
    char
        *filename,
        *ptr_body;
} part_info_struct;



static
part_info_struct *get_part_info (char *part)
{
    static part_info_struct
        part_info;
    media_type_enum
        mt_idx;
    part_header_field_enum
        idx;
    char
        *header_to_trim,
        *p_line;

    ASSERT (part);
    memset (&part_info, 0, sizeof(part_info));

    p_line = part;

    FOREVER
      {
        if (strncmp (p_line, "\r\n", 2) == 0)
            break;                      /* end of header fields */

        header_to_trim = NULL;

        for_enum (part_header_field_enum, idx)
            if (strncmp (p_line,
                         part_header_field_names[idx],
                         strlen (part_header_field_names[idx])) == 0)
              {
                part_info.ptr_header_value[idx]
                        = p_line + strlen (part_header_field_names[idx]);
                header_to_trim = (char *)part_info.ptr_header_value[idx];
                break;
              }

        /* now, we want to null-terminate the part header field body
         * --> we search next line NOT starting with linear white space (RFC 822) */
        do
          {
            p_line = strstr (p_line, "\r\n");
            ASSERT (p_line != NULL);
            p_line += 2;                /* skips CRLF                        */
          } while (is_lwsp (*p_line));

        ASSERT (*(p_line-2) == '\r');
        *(p_line - 2) = 0;              /* we null terminate the string */

        if (header_to_trim != NULL)
            remove_crlflwp (header_to_trim);
      }


      p_line += 2;                        /* skips 2nd CR LF (last header field
                                           * ends with CRLF CRLF */

      part_info.ptr_body = p_line;



    if (part_info.ptr_header_value[HF_MEDIA_TYPE] == NULL)
        part_info.ptr_header_value[HF_MEDIA_TYPE] = default_media_type;

    for_enum (media_type_enum, mt_idx)
      {
        if (lexncmp (part_info.ptr_header_value[HF_MEDIA_TYPE],
                     media_types[mt_idx],
                     strlen (media_types[mt_idx])) == 0)
          {
            part_info.media_type = mt_idx;
            break;
          }
      }


    return &part_info;
}


static
const char *close_text_body (const char *body, const char *delim)
{
    size_t
        size;
    char
        *end;

    ASSERT (body);

    /* we close the string at the delimiter position */
    if (delim)
      {
        end = strstr (body, delim);
        ASSERT (end != NULL);               /* delim must be present in body */
        *end = 0;
        end += strlen (delim);
      }
    else
        end = (char *) &body[strlen(body)];

    searchreplace ((char *) body, "\r\n..", "\r\n.", strlen (body));
    size = strlen(body);

    /* XXX Following CRLF removal should be check in some RFC */
    ASSERT (strncmp (&body[strlen(body) - 2], "\r\n", 2) == 0);

    ((char *)body)[strlen(body) - 2] = 0;

    return end;
}



#define FILENAME_LIST_INC_SIZE        1024

static char
    *filename_list = NULL;


/* concat name at the end of global variable filename_list
 * on success, result points to the pos 'name' as been copied. */
static
const char *add_filename_to_attach_list (const char *name)
{
    static int
        list_size    = 0,               /* allocated size */
        list_length  = 0;               /* used size */
    int
        len;
    Bool
        separator = FALSE;
    const
        char *res;

    ASSERT (name != NULL);
    len = strlen (name);
    ASSERT (len > 0);

    if (filename_list == NULL)
      {
        list_size     = FILENAME_LIST_INC_SIZE;
        list_length   = 0;
        filename_list = mem_alloc (list_size);

        if (filename_list == NULL)
          {
            raise_exception (memory_error_event);
            return NULL;
          }

        strclr(filename_list);
      }

    res = &filename_list[list_length];

    if (list_length > 0)
      {
        len++;                          /* it is not the first name added in
                                         * list --> we need a separator ';' */
        separator = TRUE;
      }
    list_length += len;

    if (list_length >= list_size )
      {
        list_size = list_length + FILENAME_LIST_INC_SIZE;
        filename_list = mem_realloc (filename_list, list_size);
        if (filename_list == NULL)
          {
            raise_exception (memory_error_event);
            return NULL;
          }
      }

    if (separator)
        strcat (filename_list, ";");
    strcat (filename_list, name);

    if (separator)
        res++;
    ASSERT (streq (name, res));

    return res;
}

static
const char *retrieve_filename (const char *src)
{
    const char
        *key = "filename=\"";
    char
        *found,
        *end,
        save;
    const char
        *res;

    ASSERT (src);
    ASSERT (strncmp (src, "attachment", 10) == 0); /* XXX TO BE CHECKED */

    found = strstr (src, key);

    if (!found)
      {
        raise_exception (badly_formed_msg_error_event);
        return NULL;
      }

    found += strlen (key);
    end = found;
    FOREVER
      {
        if (*end == '"')
            break;

        if (*end == 0)
          {
            raise_exception (badly_formed_msg_error_event);
            return NULL;
          }

        end++;
      }

    save = *end;
    *end = 0;

    res = add_filename_to_attach_list (found);
    *end = save;

    return res;
}

char *
removechars (
    char *strbuf,
    char *chrstorm)
{
   char *offset;

   ASSERT (strbuf);
   ASSERT (chrstorm);

   offset = (char *)NULL;

   while (*strbuf)
      {
         offset = strpbrk (strbuf, chrstorm);
         if (offset)
             strcpy (offset, (offset + 1));                    /* NO OVERRUN */
         else
             break;
      }

   return strbuf;
}



/* returns pointer to text message body, well formatted and null terminated.
 * returns NULL if a REAL problem occured */
static
const char *handle_message_body (char **attach_list)
{
    const char
        *fname,
        *filename,
        *res = NULL,
        *attach_disp = "attachment",
        *text_plain = "text/plain";
    char
        *body,
        *part,
        *next_part;
    Bool
        attach_dir_exists;
    word
        new_size,
        written,
        boundary_length;
    part_info_struct
        *part_info;
    FILE
        *file = NULL;
    char *tmp_buf;

    ASSERT (attach_list != NULL);

    body = (char *)tcb-> msg_struct. ptr_body;
    ASSERT (body != NULL);

    if (tcb-> media_type == MT_TEXT)
      {
        close_text_body (body, "\r\n.\r\n");
        return body;
      }

    if (tcb-> media_type != MT_MULTIPART)
      {
        ASSERT (FALSE);  /* XXX TODO */
        return NULL;
      }


    /* MULTIPART MANAGEMENT */
    ASSERT (tcb-> multipart_boundary != NULL);
    boundary_length = strlen (tcb-> multipart_boundary);
    ASSERT (boundary_length > 0);
    attach_dir_exists = test_attach_dir ();

    part = strstr (body, tcb-> multipart_boundary);
    if (part == NULL)
      {
        raise_exception (badly_formed_msg_error_event);
        return NULL;
      }
    part += boundary_length;

    FOREVER
      {
        if (strncmp (part, "--", 2) == 0)    /* end of message body */
            break;

        ASSERT (strncmp (part, "\r\n", 2) == 0);
        part +=2;

        next_part = strstr (part, tcb-> multipart_boundary);
        if (next_part == NULL)
          {
            raise_exception (badly_formed_msg_error_event);
            return NULL;
          }
        ASSERT (*(next_part-2) == '\r');
        *(next_part-2) = 0;  /* we NULL terminate the part for easier management */

        part_info = get_part_info (part);
        ASSERT (part_info != NULL);

        if ( (part_info-> ptr_header_value[PHF_DISPOSITION] != NULL)
          && (strncmp (part_info-> ptr_header_value[PHF_DISPOSITION],
                       attach_disp,
                       strlen(attach_disp)) == 0) )
          {
             /* part body is an attachment */
            if (! attach_dir_exists)
              {
                ASSERT (FALSE);
                raise_exception (undefined_error_event);
                return NULL;
              }

            fname = retrieve_filename (part_info-> ptr_header_value[PHF_DISPOSITION]);
            if (exception_raised)
                return NULL;
            if (fname == NULL)
              {
                raise_exception (undefined_error_event);
                return NULL;
              }

            if (part_info-> media_type == MT_TEXT)
              {
                filename = file_where ('w', tcb-> attach_dir, fname, NULL);
                file = filename ? file_open(filename, 'w')
                                : NULL;

                if (!file)
                  {
                    raise_exception (undefined_error_event);
                    return NULL;
                  }

                body = part_info->ptr_body;

                written = fwrite (part_info->ptr_body,
                                  strlen (part_info->ptr_body),
                                  1,
                                  file);
                file_close (file);
                if (!written)
                  {
                    raise_exception (undefined_error_event);
                    return NULL;
                  }
              }
            else
            if (part_info-> media_type == MT_APPLICATION)
              {
                removechars (part_info->ptr_body, "\r\n");

                /* it seems decode_base64 can't deal when in and out buffer
                 * are the same ---> we use a tmp buffer */
                tmp_buf = strdup (part_info->ptr_body);
                ASSERT (tmp_buf);
                new_size = decode_base64 (part_info->ptr_body,
                                          tmp_buf,
                                          strlen (part_info->ptr_body));

                file = fopen (fname, "wb");
                if (file != NULL)
                  {
                    written = fwrite (tmp_buf,
                                      new_size,
                                      1,
                                      file);
                    file_close (file);
                  }

                free (tmp_buf);
              }
          }

        else
        if ( (part_info-> media_type == MT_TEXT)
          && (strncmp (part_info-> ptr_header_value[PHF_MEDIA_TYPE],
                       text_plain,
                       10) == 0) )
          {
            /* we just found the message text body */
            if (res != NULL)
              {
                /* Sergio: we already found the text body. The mailbox maybe 
                 * corrupted. We noticed this problem with Jonathan while
                 * sending too long lines as emails (2002/01/29) */
                raise_exception (badly_formed_msg_error_event);
                return NULL;
              }
            
            close_text_body (part_info->ptr_body, NULL);
            res = part_info->ptr_body;
          }

        part = next_part += boundary_length;
      }

    *attach_list = filename_list;
    filename_list = NULL;

    return res;
}

/***********************   SEND MESSAGE INFO REQUEST   ***********************/

MODULE send_message_info_request (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    snprintf (strout, STROUT_SIZE, "LIST %i\r\n", tcb-> msg_id);
    reset_chunked_resp (SINGLE_LINE);

    send_smtsock_write(
        &sockq,
        0,                              /* Timeout in seconds, zero = none */
        tcb-> sock_handle,              /* Socket to write to */
        (word) strlen (strout),         /* Amount of data to write */
        strout,                         /* Block of data to write */
        FALSE,                          /* Whether to always reply           */
        0);                             /* User-defined request tag */
}


/**********************   SEND MESSAGE INFO TO CLIENT   **********************/

MODULE send_message_info_to_client (THREAD *thread)
{
    char *ptr;
    char buf[64];
    int  size;

    tcb = thread-> tcb;                 /*  Point to thread's context        */

    ASSERT (tcb-> str_resp != NULL);
    ASSERT (lexncmp (tcb-> str_resp, "+OK ", 4) == 0);

    ASSERT (atoi (getstrfld (tcb-> str_resp, 1, 0, " ", buf)) == tcb-> msg_id);

    ptr = getstrfld (tcb-> str_resp, 2, 0, " ", buf);
    ASSERT (ptr != NULL);

    size = atoi (buf);
    send_smtpop_msg_session_info(
        &tcb-> reply_to,
        tcb-> msg_id,
        size);
}


/****************************   DO CLOSE SOCKET   ****************************/

MODULE do_close_socket (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    if (tcb-> sock_handle && socket_is_alive(tcb-> sock_handle))
      {
        send_smtsock_close(
            &sockq,
            0,                              /*  timeout */
            tcb-> sock_handle,              /*  socket */
            FALSE,                          /*  reply: we don't need a reply */
            (qbyte) 0);                     /*  User-defined request tag     */

        tcb-> sock_handle = (sock_t) 0;
      }
}



/**********************   SEND SOCKET ERROR TO CLIENT   **********************/

MODULE send_error_to_client (THREAD *thread)
{
    struct_smtsock_error
        *params = NULL;
    const char 
        *reason;
    dbyte 
        code = XXX_ERROR_CODE;
  
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    switch (thread-> LR_event)
      {
        case sock_closed_event:
            reason = strprintf ("Socket closed");
            break;
        case sock_timeout_event:
            reason = strprintf ("Socket timeout");
            break;
        case sock_error_event:
            get_smtsock_error (thread-> event-> body, &params);
            reason = strprintf ("Socket error: %s", params-> message);
            free_smtsock_error (&params);
            break;
        case server_negative_response_event:
            ASSERT (tcb-> str_resp);
            reason = strprintf ("Server error: %s", tcb-> str_resp);
            break;
        case bad_message_id_event:
            reason = strprintf ("Bad message id (%i)", tcb-> msg_id);
            break;
        case server_response_error_event:
            ASSERT (tcb-> str_resp);
            reason = strprintf (
                "Unconsistent server response: %s", 
                tcb-> str_resp
              ); 
            break;
        case memory_error_event:
            reason = strprintf ("Fatal: Out of memory");
            break;
        case io_error_event:
            reason = strprintf ("Fatal: I/O failed");
            break;
        case badly_formed_msg_error_event:
            reason = strprintf (
                "Badly formed message. Mail box may be corrupted");
            break;
        default:
            reason = strprintf (
                "Unknown error [%s]", 
                _LR_ename[thread-> LR_event]
              );
            break;
      }

    /* 'reason' is a static var from strprintf. Remove CR-LF will reduce the
     * size --> it is not dangerous to cast it to (char *) */
    removechars ((char*)reason, "\r\n");

    send_smtpop_error (&tcb-> reply_to, (char*)reason, code);
}
