/*===========================================================================*
 *                                                                           *
 *  smttran.c - TCP/IP transfer agent                                        *
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
 *  published by the Free Software Foundation; either version 2 of           *
 *  the License, or (at your option) any later version.                      *
 *                                                                           *
 *  This program is distributed in the hope that it will be useful,          *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of           *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            *
 *  GNU General Public License for more details.                             *
 *                                                                           *
 *  You should have received a copy of the GNU General Public                *
 *  License along with this program in the file 'license.gpl'; if            *
 *  not, write to the Free Software Foundation, Inc., 59 Temple              *
 *  Place - Suite 330, Boston, MA 02111-1307, USA.                           *
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

#define AGENT_NAME   SMT_TRANSFER       /*  Our public name                  */
#define BUFFER_SIZE  32768L             /*  Buffer for file transmission     */

#define FILETYPE_BINARY   0
#define FILETYPE_ASCII    1

typedef struct {                        /*  Pipe descriptor                  */
    long    input_rate;                 /*    Max input transfer rate        */
    long    output_rate;                /*    Max output transfer rate       */
    long    input_credit;               /*    Current input credit           */
    long    output_credit;              /*    Current output credit          */
} PIPE;

typedef struct {                        /*  Thread context block:            */
    event_t thread_type;                /*    Thread type indicator          */
    QID     reply_to;                   /*    Queue to reply to              */
    sock_t  handle;                     /*    Socket i/o handle              */
    char   *filename;                   /*    Filename, if used              */
    qbyte   filesize;                   /*    Total file size                */
    int     stream;                     /*    File i/o handle, if used       */
    qbyte   read_size;                  /*    Read size, 0 = GET             */
    qbyte   write_size;                 /*    Write size, 0 = GET            */
    qbyte   block_size;                 /*    Block size, 0 = GET            */
    char   *block_data;                 /*    Block data, or NULL            */
    char   *block_text;                 /*    Block text data, or NULL       */
    long    start_slice;                /*    Start of slice, if any         */
    long    end_slice;                  /*    End slice, if any              */
    dbyte   filetype;                   /*    Type of file (binary, ascii)   */
    Bool    send_text;                  /*    Do we send block data or text  */
    Bool    append;                     /*    Open for output or append?     */
    qbyte   maxsize;                    /*    Max. accepted file size        */
    PIPE   *pipe;                       /*    Transfer pipe, if any          */
} TCB;

typedef struct _TRANREQ {               /*  Request descriptor               */
    struct _TRANREQ                     /*                                   */
           *next, *prev;                /*    Doubly-linked list             */
    QID     reply_to;                   /*    Who sent the request event     */
    Bool    sending;                    /*    True=PUT, false=GET            */
    sock_t  handle;                     /*    Socket for output              */
    qbyte   block_size;                 /*    Block size, 0 = GET            */
    char   *block_data;                 /*    Block data, or NULL            */
    Bool    in_header;                  /*    True when doing header         */
    Bool    in_progress;                /*    Are we processing request?     */
    Bool    huge_block;                 /*    Are we using huge blocks?      */
    qbyte   reqid;                      /*    Unique request ID              */
    PIPE   *pipe;                       /*    Transfer pipe, if any          */
} TRANREQ;


/*- Local function prototypes -----------------------------------------------*/

static TRANREQ *request_create      (THREAD *thread, sock_t handle,
                                     char *pipe_name);
static TRANREQ *request_lookup      (qbyte reqid);
static void     request_destroy     (TRANREQ *request);
static void     destroy_socket_requests (sock_t socket);
static void     signal_fatal_error  (THREAD *thread);
static void     put_request_header  (THREAD *thread, TRANREQ *request);
static void     put_request_body    (THREAD *thread, TRANREQ *request);
static void     get_request_header  (THREAD *thread, TRANREQ *request);
static void     get_request_body    (THREAD *thread, TRANREQ *request);
static void     open_transfer_file  (THREAD *thread, char mode);


/*- Global variables used in this source file only --------------------------*/

static TCB
    *tcb;                               /*  Address thread context block     */
static QID
    operq,                              /*  Operator console event queue     */
    sockq;                              /*  Socket agent event queue         */
static NODE
    requests;                           /*  Request list header              */
static int
    fileio_threads = 0;                 /*  Number of child fileio threads   */
static SYMTAB
    *pipes = NULL;                      /*  Pipe look-up table               */


#include "smttran.d"                    /*  Include dialog data              */


/********************   INITIALISE AGENT - ENTRY POINT   *********************/


/*  ---------------------------------------------------------------------[<]-
    Function: smttran_init

    Synopsis: Initialises the SMT transfer agent.  Returns 0 if initialised
    okay, -1 if there was an error.  The transfer agent reads and writes
    blocks of data, or complete files, through an open TCP/IP socket.  A
    block of data is sent/received as a two-byte length header followed
    by the data block.  Creates an unnamed thread automatically.  Supports
    these public methods:
    <Table>
    PUT_BLOCK    Write a length-specified block to a socket (< 64k)
    GET_BLOCK    Read a length-specified block from a socket (< 64k)
    PUT_HUGE     Write a length-specified block to a socket (< 2Gb)
    GET_HUGE     Read a length-specified block from a socket (< 2Gb)
    PUT_FILE     Write part or all of a file to a socket
    GET_FILE     Read part or all of a file from a socket
    PIPE_CREATE  Create new transfer pipe
    CLEAR_PIPES  Destroy all transfer pipes
    COMMIT       Wait until all transfer requests are finished
    </Table>
    Sends errors to the SMTOPER agent; see the doc for reply events.
    ---------------------------------------------------------------------[>]-*/

int
smttran_init (void)
{
    AGENT   *agent;                     /*  Handle for our agent             */
    THREAD  *thread;                    /*  Handle to various threads        */
#   include "smttran.i"                 /*  Include dialog interpreter       */

    /*  Shutdown event comes from Kernel                                     */
    declare_smtlib_shutdown   (shutdown_event,    SMT_PRIORITY_MAX);

    /*  Reply events from socket agent                                       */
    declare_smtsock_read_ok       (read_ok_event,           0);
    declare_smtsock_readh_ok      (readh_ok_event,          0);
    declare_smtsock_ok            (write_ok_event,          0);
    declare_smtsock_closed        (sock_closed_event,       0);
    declare_smtsock_read_closed   (sock_read_closed_event,  0);
    declare_smtsock_readh_closed  (sock_readh_closed_event, 0);
    declare_smtsock_error         (sock_error_event,        0);

    /*  Reply events from timer agent                                        */
    declare_smttime_reply        (alarm_event,        0);

    /*  Public methods supported by this agent                               */
    declare_smttran_put_block    (put_block_event,    0);
    declare_smttran_get_block    (get_block_event,    0);
    declare_smttran_put_huge     (put_huge_event,     0);
    declare_smttran_get_huge     (get_huge_event,     0);
    declare_smttran_put_file     (put_file_event,     0);
    declare_smttran_get_file     (get_file_event,     0);
    declare_smttran_pipe_create  (pipe_create_event,  0);
    declare_smttran_clear_pipes  (clear_pipes_event,  0);
    declare_smttran_commit       (commit_event,       SMT_PRIORITY_MIN);

    /*  Ensure that operator console is running, else start it up            */
    smtoper_init ();
    if ((thread = thread_lookup (SMT_OPERATOR, "")) != NULL)
        operq = thread-> queue-> qid;
    else
        return (-1);

    /*  Ensure that socket agent is running, else start it up                */
    smtsock_init ();
    if ((thread = thread_lookup (SMT_SOCKET, "")) != NULL)
        sockq = thread-> queue-> qid;
    else
        return (-1);

    /*  Ensure that timer agent is running, else start it up                 */
    if (smttime_init ())
        return (-1);

    /*  Create initial thread to manage master thread                        */
    if ((thread = thread_create (AGENT_NAME, "")) != NULL)
      {
        ((TCB *) thread-> tcb)-> thread_type = master_event;
        ((TCB *) thread-> tcb)-> filename    = NULL;
      }
    else
        return (-1);

    /*  Create initial thread to manage pipes                                */
    if ((thread = thread_create (AGENT_NAME, "__pipe_manager")) != NULL)
      {
        ((TCB *) thread-> tcb)-> thread_type = pipe_manager_event;
        ((TCB *) thread-> tcb)-> filename    = NULL;
      }
    else
        return (-1);

    /*  Clear & prepare request queue                                        */
    node_reset (&requests);

    /*  Signal okay to caller that we initialised okay                       */
    return (0);
}


/*************************   INITIALISE THE THREAD   *************************/

MODULE initialise_the_thread (THREAD *thread)
{
    /*  We don't set the_next_event because we expect an argument event      */
    /*  to supply these.                                                     */

    tcb = thread-> tcb;                 /*  Point to thread's context        */

    tcb-> stream     = -1;
    tcb-> block_data = NULL;
    tcb-> block_text = NULL;
    tcb-> send_text  = FALSE;

    the_next_event = tcb-> thread_type;
}


/**********************   CREATE TRANSFER PIPES TABLE   **********************/

MODULE create_transfer_pipes_table (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    pipes = sym_create_table ();
}


/***********************   GET NEXT EVENT FROM QUEUE   ***********************/

MODULE get_next_event_from_queue (THREAD *thread)
{
    AGENT   *agent;                     /*  This agent                       */
    QUEUE   *queue;                     /*  Thread's event queue             */
    EVENT   *event;                     /*  Event information block          */
    METHOD  *method;                    /*  Method information block         */

    tcb = thread-> tcb;                 /*  Point to thread's context        */

    /*  Get next event off queue                                             */
    queue = thread-> queue;
    agent = queue-> agent;
    event = event_iterate (queue, NULL);
    if (event)
      {
        method = method_lookup (agent, event-> name);
        if (method == NULL)             /*  Not a method we accept           */
            event_reject (queue, event);
        else
          {
            if (thread-> event)         /*  If thread was sitting on an      */
              {                         /*    event, release it              */
                event_destroy (thread-> event);
                thread-> event = NULL;
              }
            /*  Get event off queue; it now belongs to the thread            */
            thread-> event = event_accept (queue, event);
            the_next_event = method-> event_number;
          }
      }
    else
        the_next_event = empty_event;
}


/**************************   CREATE TRANSFER PIPE   *************************/

MODULE create_transfer_pipe (THREAD *thread)
{
    SYMBOL
        *pipe_sym;
    PIPE
        *pipe;
    struct_smttran_pipe_create
        *args;

    tcb = thread-> tcb;                 /*  Point to thread's context        */

    if (pipes)
      {
        get_smttran_pipe_create (thread-> event-> body, &args);
        pipe_sym = sym_lookup_symbol (pipes, args-> name);
        if (pipe_sym)
            pipe = (PIPE *) pipe_sym-> value;
        else
          {
            pipe_sym = sym_create_symbol (pipes, args-> name, NULL);
            pipe = mem_alloc (sizeof (PIPE));
            pipe_sym-> value = (char *) pipe;
          } 
        pipe-> input_rate    = args-> input_rate;
        pipe-> output_rate   = args-> output_rate;
        pipe-> input_credit  = 0;
        pipe-> output_credit = 0;
        free_smttran_pipe_create (&args);
      }
}


/*************************   WAIT FOR PIPE REFRESH   *************************/

MODULE wait_for_pipe_refresh (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */
/*  Alarm events leaking in this version of SMT - disabled for now           */
#if 0
    /*  Ask timer to send us an event after one second                       */
    smttime_request_flush ();
    smttime_request_alarm (0, 
                           100,     /*  No centiseconds  */
                           empty_event);
#endif
}


/***************************   REFRESH ALL PIPES   ***************************/

MODULE refresh_all_pipes (THREAD *thread)
{
    SYMBOL
        *pipe_sym;
    PIPE
        *pipe;
        
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    for (pipe_sym = pipes-> symbols; pipe_sym; pipe_sym = pipe_sym-> next)
      {
        pipe = (PIPE *) pipe_sym-> value;

        /*  Credits can be negative, but never greater than the rate         */
        pipe-> input_credit += pipe-> input_rate;
        if (pipe-> input_credit > pipe-> input_rate)
            pipe-> input_credit = pipe-> input_rate;

        pipe-> output_credit += pipe-> output_rate;
        if (pipe-> output_credit > pipe-> output_rate)
            pipe-> output_credit = pipe-> output_rate;
      }
}


/***********************   DESTROY ALL TRANSFER PIPES   **********************/

MODULE destroy_all_transfer_pipes (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    sym_delete_table (pipes);
    pipes = NULL;                       /*  Pipes no longer available        */
}


/************************   CREATE PUT BLOCK REQUEST   ***********************/

MODULE create_put_block_request (THREAD *thread)
{
    struct_smttran_putb
        *message;
    TRANREQ
        *request;                       /*  Newly-created request            */

    tcb = thread-> tcb;                 /*  Point to thread's context        */

    /*  Get arguments from message                                           */
    get_smttran_putb (thread-> event-> body, &message);

    if ((request = request_create (thread, 
                                   message-> socket,
                                   message-> pipe)) != NULL)
      {
        request-> block_size = message-> size;
        request-> block_data = mem_copy (message-> data);
        request-> sending    = TRUE;
      }

    free_smttran_putb (&message);
}


/*  -------------------------------------------------------------------------
 *  request_create
 *
 *  Creates a new request, and initialises it to empty.
 */

static TRANREQ *
request_create (THREAD *thread, sock_t handle, char *pipe_name)
{
    static qbyte
        reqid = 0;                      /*  Cyclic request id                */
    TRANREQ
        *request;                       /*  Request we create                */
    SYMBOL
        *pipe_sym;

    request = node_create (requests.prev, sizeof (TRANREQ));
    if (request)
      {
        /*  Initialise the request with default values                       */
        request-> reply_to    = thread-> event-> sender;
        request-> handle      = handle;
        request-> block_size  = 0;
        request-> block_data  = NULL;
        request-> in_header   = TRUE;
        request-> in_progress = FALSE;
        request-> reqid       = reqid++;
        request-> huge_block  = FALSE;
        request-> pipe        = NULL;

        /*  Get pipe to use, if any                                          */
        if (pipes 
        &&  pipe_name
        && *pipe_name)
          {
            pipe_sym = sym_lookup_symbol (pipes, pipe_name);
            if (pipe_sym)
                request-> pipe = (PIPE *) pipe_sym-> value;
          }
      }
    return (request);
}


/************************   CREATE GET BLOCK REQUEST   ***********************/

MODULE create_get_block_request (THREAD *thread)
{
    struct_smttran_getb
        *message;
    TRANREQ
        *request;                       /*  Newly-created request            */

    tcb = thread-> tcb;                 /*  Point to thread's context        */

    /*  Get arguments from message                                           */
    get_smttran_getb (thread-> event-> body, &message);
    if ((request = request_create (thread, 
                                   message-> socket,
                                   message-> pipe)) != NULL)
        request-> sending = FALSE;

    free_smttran_getb (&message);
}


/************************   CREATE PUT HUGE REQUEST   ************************/

MODULE create_put_huge_request (THREAD *thread)
{
    struct_smttran_puth
        *message;
    TRANREQ
        *request;                       /*  Newly-created request            */

    tcb = thread-> tcb;                 /*  Point to thread's context        */

    /*  Get arguments from message                                           */
    get_smttran_puth (thread-> event-> body, &message);

    if ((request = request_create (thread, 
                                   message-> socket, 
                                   message-> pipe)) != NULL)
      {
        request-> block_size = message-> size;
        request-> block_data = mem_alloc (message-> size);
        memcpy (request-> block_data, message-> data, message-> size);
        request-> sending    = TRUE;
        request-> huge_block = TRUE;
      }

    free_smttran_puth (&message);
}


/************************   CREATE GET HUGE REQUEST   ************************/

MODULE create_get_huge_request (THREAD *thread)
{
    struct_smttran_geth
        *message;
    TRANREQ
        *request;                       /*  Newly-created request            */

    tcb = thread-> tcb;                 /*  Point to thread's context        */

    /*  Get arguments from message                                           */
    get_smttran_geth (thread-> event-> body, &message);
    if ((request = request_create (thread, 
                                   message-> socket,
                                   message-> pipe)) != NULL)
      {
        request-> sending    = FALSE;
        request-> huge_block = TRUE;
      }

    free_smttran_geth (&message);
}


/***********************   PROCESS TRANSFER REQUESTS   ***********************/

MODULE process_transfer_requests (THREAD *thread)
{
    TRANREQ
        *request;                       /*  Request we process               */
    Bool
        did_io = FALSE;                /*  True if we did a put or get      */

    tcb = thread-> tcb;                 /*  Point to thread's context        */

    for (request = requests.next;
         (NODE *) request != &requests;
         request = request-> next)
      {
        if (request-> in_progress)
            continue;                   /*  Ignore requests in progress      */

        if (request-> sending)
          {
            if (request-> pipe == NULL
            ||  request-> pipe-> output_credit > 0)
              {
                /*  Send header and body one after the other                 */
                put_request_header (thread, request);
                put_request_body   (thread, request);

                request-> in_progress = TRUE;
                did_io = TRUE;
              }  
          }
        else                            /*  Otherwise we want GET            */
            if (request-> pipe == NULL
            ||  request-> pipe-> input_credit > 0)
              {
                if (request-> in_header)
                    get_request_header (thread, request);
                else
                    get_request_body   (thread, request);

                request-> in_progress = TRUE;
                did_io = TRUE;
              }
      }
    /*  All i/o is waiting for a pipe to be freed - loop slowly              */
    if (!did_io)
      {
/*  Alarm events leaking in this version of SMT - disabled for now           */
#if 0
        smttime_request_flush ();
        smttime_request_alarm (0, 
                               100,     /*  No centiseconds  */
                               empty_event);
#endif
      }
}


static void
put_request_header (THREAD *thread, TRANREQ *request)
{
    dbyte
        dsize;                          /*  Block size in network order      */
    qbyte
        qsize;                          /*  Huge size in network order       */

    /*  Write 2/4-byte block header in network format                        */
    if (request-> huge_block)
      {
        qsize = htonl (request-> block_size);
        send_smtsock_writeh (&sockq, 0, request-> handle, 4,
                             (byte *) &qsize, TRUE,
                             (void *) (long) request-> reqid);
        if (request-> pipe)
            request-> pipe-> output_credit -= 4;
      }
    else
      {
        dsize = htons ((dbyte) request-> block_size);
        send_smtsock_write (&sockq, 0, request-> handle, 2,
                   (byte *) &dsize, TRUE, (void *) (long) request-> reqid);
        if (request-> pipe)
            request-> pipe-> output_credit -= 2;
      }
}


static void
put_request_body (THREAD *thread, TRANREQ *request)
{
    if (request-> huge_block)
        send_smtsock_writeh (
            &sockq,                         /*  Socket agent queue           */
            0,                              /*  Timeout for transmission     */
            request-> handle,               /*  Socket to send to            */
            request-> block_size,           /*  Amount of data to send       */
            (byte *) request-> block_data,  /*  Address of data to send      */
            TRUE,                           /*  We do want a reply           */
            (void *) (long) request-> reqid);  /*  Request tag               */
    else
        send_smtsock_write (
            &sockq,
            0,
            request-> handle,
            (dbyte)  request-> block_size,
            (byte *) request-> block_data,
            TRUE,                           /*  We do want a reply           */
            (void *) (long) request-> reqid);

    if (request-> pipe)
        request-> pipe-> output_credit -= request-> block_size;
}


static void
get_request_header (THREAD *thread, TRANREQ *request)
{
    if (request-> huge_block)
        send_smtsock_readh (
            &sockq,                     /*  Socket agent queue               */
            0,                          /*  Timeout for transmission         */
            request-> handle,           /*  Socket to read from              */
            4, 4,                       /*  Read exactly 4 bytes             */
            (void *) (long) request-> reqid);  /*  Request tag               */
    else
        send_smtsock_read (
            &sockq,                     /*  Socket agent queue               */
            0,                          /*  Timeout for transmission         */
            request-> handle,           /*  Socket to read from              */
            2, 2,                       /*  Read exactly 2 bytes             */
            (void *) (long) request-> reqid);  /*  Request tag               */
}


static void
get_request_body (THREAD *thread, TRANREQ *request)
{
    if (request-> huge_block)
        send_smtsock_readh (
            &sockq,                     /*  Socket agent queue               */
            0,                          /*  Timeout for transmission         */
            request-> handle,           /*  Socket to read from              */
            request-> block_size,       /*  Max amount of data to read       */
            request-> block_size,       /*  Min amount of data to read       */
            (void *) (long) request-> reqid);  /*  Request tag               */
    else
        send_smtsock_read (
            &sockq,
            0,
            request-> handle,
            (dbyte) request-> block_size,
            (dbyte) request-> block_size,
            (void *) (long) request-> reqid);
}


/***********************   UPDATE REQUEST AFTER READ   ***********************/

MODULE update_request_after_read (THREAD *thread)
{
    struct_smtsock_read_reply
        *message;
    TRANREQ
        *request;                       /*  Request we process               */

    tcb = thread-> tcb;                 /*  Point to thread's context        */

    get_smtsock_read_reply (thread-> event-> body, &message);
    if ((request = request_lookup ((qbyte) (long) message-> tag)) == NULL)
      {
        free_smtsock_read_reply (&message);
        return;                         /*  Something garbage - ignore it    */
      }

    /*  We either just received the header or the body                       */
    if (request-> in_header)
      {
        byte *
            message_data = (byte *) message-> data;
        request-> block_size  = ntohs (*((dbyte *) message_data));
        request-> in_header   = FALSE;
        request-> in_progress = FALSE;
      }
    else
      {
        request-> block_size = message-> size;

        /*  Send it back to the calling program                              */
        send_smttran_getb_ok (&request-> reply_to,
                              (dbyte) request-> block_size,
                              message-> data);

        request_destroy (request);
      }
    free_smtsock_read_reply (&message);
}


/*  Returns in-progress request for handle, or NULL if none found            */

static TRANREQ
*request_lookup (qbyte reqid)
{
    TRANREQ
        *request;                       /*  Request we process               */

    for (request = requests.next;
         (NODE *) request != &requests;
         request = request-> next)
      {
        if (request-> reqid == reqid)
            return (request);           /*  We found the request             */
      }
    return (NULL);
}

static void
request_destroy (TRANREQ *request)
{
    mem_free (request-> block_data);
    node_destroy (request);
}


/***********************   UPDATE REQUEST AFTER READH   **********************/

MODULE update_request_after_readh (THREAD *thread)
{
    struct_smtsock_readh_reply
        *message;
    TRANREQ
        *request;                       /*  Request we process               */

    tcb = thread-> tcb;                 /*  Point to thread's context        */

    get_smtsock_readh_reply (thread-> event-> body, &message);
    if ((request = request_lookup ((qbyte) (long) message-> tag)) == NULL)
      {
        free_smtsock_readh_reply (&message);
        return;                         /*  Something garbage - ignore it    */
      }

    /*  We either just received the header or the body                       */
    if (request-> in_header)
      {
        request-> block_size  = ntohl (*((qbyte *) message-> data));
        request-> in_header   = FALSE;
        request-> in_progress = FALSE;
      }
    else
      {
        request-> block_size = message-> size;

        /*  Send it back to the calling program                              */
        send_smttran_geth_ok (&request-> reply_to,
                              request-> block_size, 
                              message-> data);

        request_destroy (request);
      }
    free_smtsock_readh_reply (&message);
}


/***********************   UPDATE REQUEST AFTER WRITE   **********************/

MODULE update_request_after_write (THREAD *thread)
{
    struct_smtsock_reply
        *message;
    TRANREQ
        *request;                       /*  Request we process               */

    tcb = thread-> tcb;                 /*  Point to thread's context        */

    /*  Pick up reqid from event body, encoded as SMT_SOCK_OK                */
    get_smtsock_reply (thread-> event-> body, &message);
    if ((request = request_lookup ((qbyte) (long) message-> tag)) == NULL)
      {
        free_smtsock_reply (&message);
        return;                         /*  Something garbage - ignore it    */
      }

    /*  We either just sent the header or the body                           */
    if (request-> in_header)
        request-> in_header = FALSE;
    else
      {
        /*  Send confirmation back to the calling program                    */
        if (request-> huge_block)
            send_smttran_puth_ok (&request-> reply_to, 
                                  request-> block_size);
        else
            send_smttran_putb_ok (&request-> reply_to,
                                  (dbyte) request-> block_size);
        request_destroy (request);
      }
    free_smtsock_reply (&message);
}


/**********************   UPDATE REQUEST AFTER CLOSED   **********************/

MODULE update_request_after_closed (THREAD *thread)
{
    struct_smtsock_reply
        *message;
    TRANREQ
        *request;                       /*  Request we process               */

    tcb = thread-> tcb;                 /*  Point to thread's context        */

    get_smtsock_reply (thread-> event-> body, &message);
    if ((request = request_lookup ((qbyte) (long) message-> tag)) == NULL)
      {
        free_smtsock_reply (&message);
        return;                         /*  Something garbage - ignore it    */
      }

    send_smttran_closed (&request-> reply_to);
    destroy_socket_requests (request-> handle);
    free_smtsock_reply (&message);
}


static void
destroy_socket_requests (sock_t socket)
{
    TRANREQ
        *request;

    for (request = requests.next;
         (NODE *) request != &requests;
         request = request-> next)
      {
        /*  Destroy all requests that refer to this (closed) socket          */
        if (request-> handle == socket)
          {
            request = request-> prev;
            request_destroy (request-> next);
          }
      }
}


/********************   UPDATE REQUEST AFTER READ CLOSED   *******************/

MODULE update_request_after_read_closed (THREAD *thread)
{
    struct_smtsock_read_reply
        *message;
    TRANREQ
        *request;                       /*  Request we process               */

    tcb = thread-> tcb;                 /*  Point to thread's context        */

    get_smtsock_read_reply (thread-> event-> body, &message);
    if ((request = request_lookup ((qbyte) (long) message-> tag)) == NULL)
      {
        free_smtsock_read_reply (&message);
        return;                         /*  Something garbage - ignore it    */
      }

    send_smttran_closed (&request-> reply_to);
    destroy_socket_requests (request-> handle);
    free_smtsock_read_reply (&message);
}


/*******************   UPDATE REQUEST AFTER READH CLOSED   *******************/

MODULE update_request_after_readh_closed (THREAD *thread)
{
    struct_smtsock_readh_reply
        *message;
    TRANREQ
        *request;                       /*  Request we process               */

    tcb = thread-> tcb;                 /*  Point to thread's context        */

    get_smtsock_readh_reply (thread-> event-> body, &message);
    if ((request = request_lookup ((qbyte) (long) message-> tag)) == NULL)
      {
        free_smtsock_readh_reply (&message);
        return;                         /*  Something garbage - ignore it    */
      }

    send_smttran_closed (&request-> reply_to);
    destroy_socket_requests (request-> handle);
    free_smtsock_readh_reply (&message);
}


/***********************   UPDATE REQUEST AFTER ERROR   **********************/

MODULE update_request_after_error (THREAD *thread)
{
    struct_smtsock_error
        *message;
    TRANREQ
        *request;                       /*  Request we process               */

    tcb = thread-> tcb;                 /*  Point to thread's context        */

    /*  Pick up reqid from event body, encoded as SMT_SOCK_ERROR             */
    get_smtsock_error (thread-> event-> body, &message);
    if ((request = request_lookup ((qbyte) (long) message-> tag)) == NULL)
      {
        free_smtsock_error (&message);
        return;                         /*  Something garbage - ignore it    */
      }

    send_smttran_error (&request-> reply_to,
                        message-> message);
    request_destroy (request);

    free_smtsock_error (&message);
}


/*************************   CREATE PUT FILE THREAD   ************************/

MODULE create_put_file_thread (THREAD *thread)
{
    struct_smttran_putf
        *params;
    THREAD
        *child;                         /*  Handle to child thread           */
    SYMBOL
        *pipe_sym;
        
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    get_smttran_putf (thread-> event-> body, &params);

    child = thread_create (AGENT_NAME, "");
    ASSERT (child);
    tcb = child-> tcb;

    tcb-> thread_type = put_file_event;
    tcb-> filename    = mem_strdup (clean_path (params-> filename));
    tcb-> start_slice = params-> start;
    tcb-> end_slice   = params-> end;
    tcb-> filetype    = params-> filetype;
    tcb-> handle      = params-> socket;
    tcb-> append      = FALSE;
    tcb-> maxsize     = 0xFFFFFFFFUL;
    tcb-> reply_to    = thread-> event-> sender;
    tcb-> pipe        = NULL;

    /*  Get pipe to use, if any                                          */
    if (pipes && *params-> pipe)
      {
        pipe_sym = sym_lookup_symbol (pipes, params-> pipe);
        if (pipe_sym)
            tcb-> pipe = (PIPE *) pipe_sym-> value;
      }

#if (defined (GATES_FILESYSTEM))
    /*  On DOS-based file systems, text files already contain CRLFs, and
     *  can thus be handled as binary files.
     */   
    tcb-> filetype = FILETYPE_BINARY;
#endif

    fileio_threads++;
    free_smttran_putf (&params);
}


/*************************   CREATE GET FILE THREAD   ************************/

MODULE create_get_file_thread (THREAD *thread)
{
    struct_smttran_getf
        *params;
    THREAD
        *child;                         /*  Handle to child thread           */
    SYMBOL
        *pipe_sym;
        
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    get_smttran_getf (thread-> event-> body, &params);

    child = thread_create (AGENT_NAME, "");
    ASSERT (child);
    tcb = child-> tcb;

    tcb-> thread_type = get_file_event;
    tcb-> filename    = mem_strdup (clean_path (params-> filename));
    tcb-> start_slice = params-> start;
    tcb-> end_slice   = params-> end;
    tcb-> filetype    = params-> filetype;
    tcb-> handle      = params-> socket;
    tcb-> append      = params-> append;
    tcb-> maxsize     = params-> maxsize;
    tcb-> reply_to    = thread-> event-> sender;
    tcb-> pipe        = NULL;

    /*  Get pipe to use, if any                                          */
    if (pipes && *params-> pipe)
      {
        pipe_sym = sym_lookup_symbol (pipes, params-> pipe);
        if (pipe_sym)
            tcb-> pipe = (PIPE *) pipe_sym-> value;
      }

    fileio_threads++;
    free_smttran_getf (&params);
}


/**************************   OPEN FILE FOR OUTPUT   *************************/

MODULE open_file_for_output (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    if (tcb-> append)
        open_transfer_file (thread, 'a');
    else
        open_transfer_file (thread, 'o');
}

static void
open_transfer_file (THREAD *thread, char mode)
{
    int
        open_mode;
    
    tcb = thread-> tcb;                 /*  Point to thread's context        */
    if (mode == 'i')                    /*  Input                            */
        open_mode = O_RDONLY;
    else

    if (mode == 'o')                    /*  Output                           */
        open_mode = O_WRONLY | O_CREAT | O_TRUNC;
    else
    if (mode == 'a')                    /*  Append                           */
        open_mode = O_WRONLY | O_CREAT | O_APPEND;
    else
        return;                         /*  Error                            */
        
    tcb-> filesize = 0;
    tcb-> stream   = lazy_open (tcb-> filename, open_mode);
    if (io_completed)
      {
        if (tcb-> stream < 0)           /*  If the open failed, send error   */
          {                             /*    to console, and terminate      */
            send_smtoper_error (
                &operq,
                strprintf ("smttran: cannot open '%s' for '%c'", 
                           tcb-> filename, mode));
            signal_fatal_error (thread);
          }
        else
        if (tcb-> start_slice > 0)      /*  Position at start of slice?      */
            _lseek (tcb-> stream, tcb-> start_slice, SEEK_SET);
      }
}

local
signal_fatal_error (THREAD *thread)
{
    raise_exception (exception_event);
    send_smttran_error (&tcb-> reply_to, strerror (errno));
}


/**************************   OPEN FILE FOR INPUT   **************************/

MODULE open_file_for_input (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    open_transfer_file (thread, 'i');
}


/*************************   READ FILE DATA BUFFER   *************************/

MODULE read_file_data_buffer (THREAD *thread)
{
    long
        read_size;                      /*  Amount of data read/to read      */

    tcb = thread-> tcb;                 /*  Point to thread's context        */

    if (tcb-> end_slice > 0)            /*  End of slice defined?            */
      {
        read_size = tcb-> end_slice - tcb-> filesize + 1;
        if (read_size < 0)
            read_size = 0;
        else
        if (read_size > BUFFER_SIZE)
            read_size = BUFFER_SIZE;
      }
    else
        read_size = BUFFER_SIZE;

    /*  If read_size is 0, we're finished reading                            */
    if (read_size > 0)
      {
        mem_strfree (&tcb-> block_data);
        tcb-> block_size = read_size;
        tcb-> block_data = mem_alloc ((dbyte) read_size);
        read_size = lazy_read (tcb-> stream, tcb-> block_data, (int) read_size);
      }
    if (io_completed)
      {
        if (read_size == 0)
            raise_exception (end_of_file_event);
        else
        if (read_size == -1)            /*  If the read failed, send error   */
          {                             /*    to console, and terminate      */
            send_smtoper_error (
                &operq,
                strprintf ("smttran: could not read from %s", 
                           tcb-> filename));
            signal_fatal_error (thread);
          }
        else
            tcb-> read_size = read_size;
      }
}


/**********************   CONVERT ASCII TO NET IF REQD   *********************/

MODULE convert_ascii_to_net_if_reqd (THREAD *thread)
{
    qbyte
        text_size;                      /*  Size of text block               */

    tcb = thread-> tcb;                 /*  Point to thread's context        */

    if (tcb-> filetype == FILETYPE_BINARY)
      {
        tcb-> write_size = tcb-> read_size;
        tcb-> send_text  = FALSE;
      }
    else
      {
        /*  Allocate text block, in the worst case = all LFs -> CRLFs        */
        mem_strfree (&tcb-> block_text);
        text_size = tcb-> read_size * 2;    
        tcb-> block_text = mem_alloc ((size_t) text_size);
        if (tcb-> block_text)
          {
            /*  Convert data to 'text'                                       */
            tcb-> write_size = file_set_eoln (tcb-> block_text,
                                              tcb-> block_data,
                                              (dbyte) tcb-> read_size, TRUE);
            tcb-> send_text = TRUE;
           }
        else
          {
            send_smtoper_error (
                &operq,
                strprintf ("smttran: could not allocate text buffer (%ld bytes)",
                           text_size));
            signal_fatal_error (thread);
          }
      }
}


/**************************   PUT BUFFER TO SOCKET   *************************/

MODULE put_buffer_to_socket (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    if (tcb-> pipe == NULL
    ||  tcb-> pipe-> output_credit > 0)
      {
        tcb-> filesize += tcb-> write_size;
        send_smtsock_write (&sockq, 0, tcb-> handle, (word) tcb-> write_size,
                   (byte *) (tcb-> send_text?
                             tcb-> block_text: tcb-> block_data), TRUE, 0);
        if (tcb-> pipe)
            tcb-> pipe-> output_credit -= tcb-> write_size;
      }
    else
      {
/*  Alarm events leaking in this version of SMT - disabled for now           */
#if 0
        smttime_request_flush ();
        smttime_request_alarm (0, 
                               100,     /*  No centiseconds  */
                               empty_event);
#endif
      }
}


/*************************   GET BUFFER FROM SOCKET   ************************/

MODULE get_buffer_from_socket (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    if (tcb-> pipe == NULL
    ||  tcb-> pipe-> input_credit > 0)
        /*  Read buffer from socket, we expect up to BUFFER_MAX bytes,       */
        /*  but anything will do.                                            */
        send_smtsock_read (&sockq, 0, tcb-> handle, BUFFER_SIZE, 1, 0);
    else
      {
/*  Alarm events leaking in this version of SMT - disabled for now           */
#if 0
        smttime_request_flush ();
        smttime_request_alarm (0, 
                               100,     /*  No centiseconds  */
                               empty_event);
#endif
      }
}


/**********************   CONVERT NET TO ASCII IF REQD   *********************/

MODULE convert_net_to_ascii_if_reqd (THREAD *thread)
{
    struct_smtsock_read_reply
        *message;
    
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    get_smtsock_read_reply (thread-> event-> body, &message);
    if (tcb-> filetype == FILETYPE_BINARY)
      {
        mem_strfree (&tcb-> block_data);
        if (message-> size > 0)
          {
            tcb-> block_data = mem_alloc (message-> size);
            memcpy (tcb-> block_data, message-> data, message-> size);
          }
        else
            tcb-> block_data = NULL;

        tcb-> write_size = message-> size;
      }
    else
      {
        mem_strfree (&tcb-> block_text);
        mem_strfree (&tcb-> block_data);
        if (message-> size > 0)
          {
            tcb-> block_text = mem_alloc (message-> size);
            memcpy (tcb-> block_text, message-> data, message-> size);
            tcb-> block_data = mem_alloc (message-> size);
            if (tcb-> block_text && tcb-> block_data)
                tcb-> write_size = file_set_eoln (tcb-> block_data,
                                                  tcb-> block_text,
                                                  message-> size, FALSE);
            else
              {
                send_smtoper_error (
                    &operq,
                    strprintf ("smttran: could not allocate text buffer "
                               "(%ld bytes)", message-> size));
                signal_fatal_error (thread);
              }
          }
        else 
          {
            tcb-> block_text = NULL;
            tcb-> block_data = NULL;
            tcb-> write_size = message->size;
          }
      }

    if (tcb-> pipe)
        tcb-> pipe-> input_credit -= message-> size;

    free_smtsock_read_reply (&message);
}

/*************************   WRITE FILE DATA BUFFER   ************************/

MODULE write_file_data_buffer (THREAD *thread)
{
    int
        rc = 0,                         /*  Return code from write           */
        write_size;                     /*  Actual amount of data to write   */

    tcb = thread-> tcb;                 /*  Point to thread's context        */

    write_size = (int) tcb-> write_size;
    if (write_size + tcb-> filesize > tcb-> maxsize)
        write_size = (int) (tcb-> maxsize - tcb-> filesize);

    if (write_size > 0)                 /*  Don't exceed limits              */ 
        rc = lazy_write (tcb-> stream, tcb-> block_data, (size_t) write_size);
    if (io_completed)
      {
        tcb-> filesize += tcb-> write_size;
        if (rc < 0)                     /*  If the read failed, send error   */
          {                             /*    to console, and terminate      */
            send_smtoper_error (
                &operq,
                strprintf ("smttran: could not write to %s", 
                           tcb-> filename));
            signal_fatal_error (thread);
          }
      }
}


/**************************   SIGNAL PUT FILE OKAY   *************************/

MODULE signal_put_file_okay (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    send_smttran_putf_ok (&tcb-> reply_to, tcb-> filesize);
    fileio_threads--;                   /*  Request is finished              */
}


/**************************   SIGNAL GET FILE OKAY   *************************/

MODULE signal_get_file_okay (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    send_smttran_getf_ok (&tcb-> reply_to, tcb-> filesize);
    fileio_threads--;                   /*  Request is finished              */
}


/**************************   SIGNAL SOCKET CLOSED   *************************/

MODULE signal_socket_closed (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    send_smttran_closed (&tcb-> reply_to);
    fileio_threads--;                   /*  Request is finished              */
}


/**************************   SIGNAL SOCKET ERROR   **************************/

MODULE signal_socket_error (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    send_smttran_error (&tcb-> reply_to, (char *) thread-> event-> body);
    fileio_threads--;                  /*  Request is finished              */
}


/************************   OKAY IF NO REQUESTS LEFT   ***********************/

MODULE okay_if_no_requests_left (THREAD *thread)
{
    static QID
        commit_requestor = { 0, 0 };

    tcb = thread-> tcb;                 /*  Point to thread's context        */

    /*  Since commit events come from various places, we will store the QID  */
    /*  of the first one to come in, and use that for reply events.  When    */
    /*  we send an ok event, we'll reset the requestor queue id.  This is    */
    /*  not the best solution, but will have to do.                          */

    if (commit_requestor.ident == 0)
        commit_requestor = thread-> event-> sender;

    /*  If no requests left, signal all closed to sender, else send a        */
    /*  timer alarm event to ourselves to cause the dialog to loop.          */
    if (requests.next  == &requests
    &&  fileio_threads == 0)
      {
        send_smttran_closed (&commit_requestor);
        commit_requestor.ident = 0;
      }
    else
      {
/*  Alarm events leaking in this version of SMT - disabled for now           */
#if 0
        /*  Ask timer to send us an event after one second                   */
        smttime_request_flush ();
        smttime_request_alarm (0, 100, commit_event);
#endif
      }
}


/*************************   REGENERATE ALARM EVENT   ************************/

MODULE regenerate_alarm_event (THREAD *thread)
{
    struct_smttime_tag
        *reply;                         /*  Reply from timer                 */
        
    tcb = thread-> tcb;                 /*  Point to thread's context        */
    get_smttime_tag (thread-> event-> body, &reply);
    if (reply-> tag == (void *) commit_event)
        the_next_event = commit_event;
    else
        the_next_event = empty_event; 
        
    free_smttime_tag (&reply);
}


/***************************   CHECK THREAD TYPE   ***************************/

MODULE check_thread_type (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */
    the_next_event = tcb-> thread_type;
}


/**************************   DESTROY ALL REQUESTS   *************************/

MODULE destroy_all_requests (THREAD *thread)
{
    while (requests.next != &requests)
        request_destroy (requests.next);
}


/*************************   TERMINATE THE THREAD   **************************/

MODULE terminate_the_thread (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    if (tcb-> stream >= 0)
      {
        lazy_close (tcb-> stream);
        if (io_completed)
            tcb-> stream = -1;
      }

    if (tcb->stream < 0)
      {
        mem_strfree (&tcb-> filename);
        mem_strfree (&tcb-> block_data);
        mem_strfree (&tcb-> block_text);
        the_next_event = terminate_event;
      }
}
