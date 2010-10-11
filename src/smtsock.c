/*===========================================================================*
 *                                                                           *
 *  smtsock.c - TCP/IP Socket agent                                          *
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

/*  -------------------------------------------------------------------------

    This module has been extensively hand-optimised for both data and
    processing efficiency.  As a result it may also be somewhat difficult to
    follow and prone to errors.  This is an attempt to explain what is
    going on so that the code may make more sense.

    A. DATA
       ----
    1. There are two queues of socket requests: main_list and wait_list.

       main_list contains queued requests grouped/sorted as follows:
           - grouped by socket number in no particular order (in practice
           the order is that in which the first request on the socket
           since the last time all requests on that socket were cleared was
           received).
           - sorted by request type: 'input-type' requests are INPUT, READ
           and READR, while 'output-type' requests are all others, including
           CLOSE.  Output-type requests come before input-type requests.
           - sorted in chronological order of request reception.

       wait_list contains requests simply in the order in which they are
       received.  Every request in wait_list has at least one corresponding
       request on the same socket in main_list.  These requests are held in
       wait_list pending insertion at the appropriate point in main_list.

       Note that requests in both main_list and wait_list are active and must
       be processed as rapidly as posible.

       For the purposes of optimisation, two redundant variables are
       maintained in parallel with wait_list.  fd_waiting is a file
       descriptor bitset containing those socket handles on which there is a
       request in wait_list.  wait_count contains the total number of requests
       in wait_list.  Since fd_waiting may never be required, it is
       dynamically allocated one time for all the first time it is used.

    2. There are six file descriptor bitsets used for calls to select().
       There are two groups of three: the input to and result from the call
       for input, output and error checks respectively.  These are named
       fd_{in/out/err}_{check/result}.  The bitsets fd_{in/out/err}_check
       are maintained permanently, ie at the exit of any module in smtsock
       these three bitsets contain exactly those socket numbers for which
       there are active requests in either main_list or wait_list.
       fd_err_check is always a logical OR of fd_in_check with fd_out_check.

    3. The variable top_socket is maintained so that it is always at
       least as big but preferably no bigger than the highest socket number
       on which there is an active request.  Note that it is non-trivial to
       determine the highest socket number when a request is deleted.  We
       recalculate top_socket whenever we walk the entire list of requests,
       so that  it will be lowered (after a request is delete) following the
       next walk at the latest.  We optimise this process with the flags
       deleted and deleted_walking which indicate, respectively, whether a
       queued request was deleted since the last time top_socket was
       guaranteed up-to-date and whether a queued request was deleted since
       we last began a walk of the requests (since if this is the case then
       we are no longer guaranteed to be up-to-date when the walk is over).
       Note also that non-queued requests don't trigger deleted or
       deleted_walking as they are not reflected in the value of top_socket.

    B. PROCESSING
       ----------
    1. Request reception.  Requests are created before being queued.  We
       attempt to service certain types of requests immediately so that
       they never need to be queued.  READ/H/R requests are attempted if
       there are no currently queued input-type requests on the socket;
       WRITE/H requests are attempted if there are no queued output-type
       requests.  CLOSE requests are attempted if there are no queued
       requests of any type.  While it would be nice to attempt the CLOSE
       as long as there are no output requests pending, we have the problem
       that input-type requests would need to be cancelled immediately and
       we don't have the data structures to locate them efficiently.

    2. Request queuing.  Performed by request_queue.
       When a request is received, fd_err_check is examined to see whether
       there are any existing requests on the same socket.  If not, the
       request is added at the end of main_list and the socket is added to
       fd_err_check; otherwise it is added at the end of wait_list and the
       socket is added to fd_waiting.  In either case the socket is added to
       fd_in_check or fd_out_check as appropriate, and both top_socket and
       new_top_socket are updated if the new socket number is higher than
       their previous value.

    3. Socket watching.  This is handled by check_activity_nonblock and
       wait_for_activity_blocking when SMT is active and idle respectively.
       The dialog manager handles calling these functions.

       check_activity_nonblock calls sock_select with no timeout.  If socket
       activity is detected it sends an ACTIVITY event to the socket queue to
       wake it up and begin processing of the result.

       wait_for_activity_blocking makes use of the fact that it is called when
       SMT is idle to do some housekeeping.  It performs two tasks: merging
       requests in wait_list and recalculating top_socket, optimising using
       wait_count and deleted to avoid redundant processing.

    4. Request merging.  Performed by merge_waiting_requests_same_socket

       There are several things going on at once and the whole thing is quite
       optimised so pay attention.

       # First note that this function must only be called with the argument
       request pointing to the *first* request on a given socket in main_list.
       On return this pointer is updated so that it still points to the
       first request on the socket.

       This function finds all requests in wait_list on the same socket and
       merges them into main_list at the approprate location.  At the same
       time it validates that the new requests being queued are not
       meaningless.  Meaningless requests are: any request following an
       unprocessed CLOSE, any output-type request following an unprocessed
       OUTPUT and any input-type request following an unprocessed INPUT or
       READR.

    5. Checking for socket activity.  Performed by check_next_socket_activity

       This module is called as the requests in main_list are walked following
       socket activity.  When activity is detected, walking stops while the
       activity is processed.  Then walking continues from where it was.

       For each request, the function proceeds as follows:

       It updates new_top_socket if required to the current request socket
       handle.

       If there are any waiting requests on the same socket, they are all
       merged immediately.

       If there was error activity on the socket, this activity is flagged as
       the next to process.

       If there was output activity, this is flagged as the next to process.
       The socket is removed from fd_out_result so that successive output
       requests on the same socket don't get triggered.
       In addition, the following request is examined to see if it is another
       output request on the same socket.  If it isn't then the socket is
       removed from fd_out_check, and fd_err_check if appropriate.

       If the output request was actually a CLOSE, then any remaining input
       requests get sent a CLOSED reply immediately and they are destroyed.
       The socket also gets removed from the input and error check sets as
       well as the input result bitset.

       Then skip further output requests on the same socket.

       If there was input activity, this is flagged as the next to process.
       The socket is removed from fd_in_result so that successive input
       requests on the same socket don't get triggered.
       In addition, the following request is examined to see if it is another
       request on the same socket.  If it isn't then the socket is removed
       from fd_in_check, and fd_err_check if appropriate.

       Then skip all further requests on the same socket.

       If no activity was flagged, proceed to the next request.

       If all requests have been processed, update top_socket to the value
       of new_top_socket.  If no requests were deleted during the walk of the
       requests then we can flag that no requests have been deleted since
       top_socket was last guaranteed correct (deleted).

    6. Request processing is pretty straightforward.  The only request which
       invokes more complicated stuff is FLUSH.

       - FLUSH: There are two versions of the flush request: flush all requests
       or only input-type requests.  For reasons of optimisation, these two
       cases are treated separately.

       When flushing all requests, we can simply walk both main_list and
       wait_list, destroying requests on the given socket as we go.

       When flushing only input requests, we need to maintain the data so that
       any remaining output-type requests are processed cleanly.  We do this
       by merging waiting requests on the same socket, so that we know that
       all requests on our socket are in main_list.  Then we find the
       input-type requests and destroy them.

       The bitsets are maintained appropriately but top_socket is not
       recalculated at this stage.
    -------------------------------------------------------------------------*/


/*- Definitions -------------------------------------------------------------*/

#define AGENT_NAME      SMT_SOCKET      /*  Our public name                  */
#define SINGLE_THREADED TRUE            /*  Single-threaded agent            */
#define WRITE_TIMEOUT   10              /*  Default write timeout            */

typedef enum {
    SOCK_READ,
    SOCK_WRITE,
    SOCK_INPUT,
    SOCK_OUTPUT,
    SOCK_CLOSE,
    SOCK_CONNECT
} REQUEST_TYPE;

#define request_is_input(r)  (r->type == SOCK_READ  || r->type == SOCK_INPUT)
#define request_is_output(r) (! request_is_input(r))

typedef struct _SOCKREQ {               /*  Request descriptor               */
    struct _SOCKREQ                     /*                                   */
           *next, *prev;                /*    Doubly-linked list             */
    long    id;                         /*    Unique internal request ID     */
    REQUEST_TYPE
            type;                       /*    Type of request                */
    QID     reply_to;                   /*    Who sent the request event     */
    sock_t  handle;                     /*    Socket for request             */
    byte   *buffer;                     /*    Buffer for i/o, or NULL        */
    qbyte   max_size;                   /*    Maximum size of buffer         */
    qbyte   cur_size;                   /*    Current size of buffer         */
    qbyte   min_size;                   /*    Minimum data to process        */
    dbyte   timeout;                    /*    Expiry time in seconds         */
    time_t  expires;                    /*    Expiry time, or 0              */
    void   *tag;                        /*    User-defined request tag       */
    int     repeat:1;                   /*    Repeated request?              */
    int     huge_block:1;               /*    Huge blocks?                   */
    int     reply:1;                    /*    Send OK reply afer write?      */
    int     queued:1;                   /*    Is request queued?             */
} SOCKREQ;


/*- Function prototypes -----------------------------------------------------*/

static void     request_destroy         (SOCKREQ *request);
       int      check_activity_nonblock (int interval, void *dummyarg);
static void     merge_waiting_requests_same_socket
                                        (SOCKREQ **request);
static void     send_ok_reply           (SOCKREQ *request);
static void     send_closed_reply       (SOCKREQ *request);
static void     reply_illegal_sequence_error
                                        (SOCKREQ *request);
       int      wait_for_activity_blocking
                                        (long  wait_date, long  wait_time,
                                         void *dummyarg);
static SOCKREQ *create_small_read_type_request
                                        (THREAD *thread);
static SOCKREQ *request_create          (QID *reply_to,
                                         REQUEST_TYPE request_type,
                                         dbyte timeout,
                                         sock_t handle,
                                         void *tag);
static void     process_read_type_request
                                        (SOCKREQ *request);
static int      read_some_data          (SOCKREQ *request);
static void     request_queue           (SOCKREQ *request);
static void     reply_error             (QID *qid,
                                         char *message,
                                         void *tag);
static SOCKREQ *create_huge_read_type_request
                                        (THREAD *thread);
static void     clear_check_after_input_request
                                        (SOCKREQ *request);
static void     clear_check_after_output_request
                                        (SOCKREQ *request);
static void     handle_partial_io       (SOCKREQ *request, int bytes_done);
static int      write_some_data         (SOCKREQ *request);
static void     prepare_to_cancel_all_requests
                                        (void);
static void     send_error_reply        (SOCKREQ *request);

#if defined (DEBUG)
static void     print_sock_lists        (void);
static void     print_sock_request      (SOCKREQ *request);
static void     print_sock_select       (const char     *intro,
                                         int             num_sockets,
                                         fd_set         *readfds,
                                         fd_set         *writefds,
                                         fd_set         *errorfds,
                                         struct timeval *timeout);
static void     print_sock_select_set   (char           *buffer,
                                         int             num_sockets,
                                         fd_set         *set);
static void     print_sock_check        (void);

#endif

/*- Global variables used in this source file only --------------------------*/

static Bool
    walking,                            /*  Currently walking requests?      */
    trace_flag = FALSE,                 /*  Trace socket activity?           */
    deleted,                            /*  Requests deleted?                */
    deleted_walking;                    /*    ... while walking requests.    */
static NODE                             /*  Two request lists.               */
    main_list,
    wait_list;
static fd_set
    fd_in_check,                        /*  Check bitsets                    */
    fd_out_check,
    fd_err_check,
    fd_in_result,                       /*  Result bitsets                   */
    fd_out_result,
    fd_err_result,
    *fd_waiting = NULL;                 /*  Sockets in waiting requests      */
static sock_t
    top_socket,                         /*  Highest socket number            */
    new_top_socket;                     /*  Temporary during request walk    */
static QID
    sockq,                              /*  Our own (unique) queue           */
    operq;                              /*  Operator console event queue     */
static SOCKREQ
    *current_request,                   /*  Pointer to request (in list)     */
    *active_request;                    /*  Request we're processing         */
static long
    last_id = 0;                        /*  Last used request ID             */
static int
    wait_count = 0;                     /*  Number of requests in wait list  */


#include "smtsock.d"                    /*  Include dialog data              */

/********************   INITIALISE AGENT - ENTRY POINT   *********************/

/*  ---------------------------------------------------------------------[<]-
    Function: smtsock_init

    Synopsis: Initialises the SMT socket agent.  Returns 0 if initialised
    okay, -1 if there was an error.  The socket agent manages all sockets
    (TCP and UPD) used by an SMT application.  Creates an unnamed thread
    automatically: send events to that thread.  Initialises the sflsock
    socket interface automatically.  Supports these public methods:
    <Table>
    READ      Read a specified amount of input data (use SMT_SOCK_READ).
    WRITE     Write a specified amount of output data (use SMT_SOCK_WRITE).
    READR     Read input data, repeatedly (use SMT_SOCK_READ).
    READH     As for READ, but for blocks > 64k (use SMT_SOCK_READH).
    WRITEH    As for WRITE, but for blocks > 64k (use SMT_SOCK_WRITEH).
    READRH    As for READR, but for blocks > 64k (use SMT_SOCK_READH).
    INPUT     Wait for any input ready on socket (use SMT_SOCK_INPUT).
    OUTPUT    Wait for any output ready on socket (use SMT_SOCK_OUTPUT).
    CONNECT   Make socket connection to host & port (use SMT_SOCK_CONNECT).
    FLUSH     Delete all requests for specified socket (use SMT_SOCK_FLUSH).
    </Table>
    Sends errors to the SMTOPER agent; see doc for reply events.
    ---------------------------------------------------------------------[>]-*/

int
smtsock_init (void)
{
    AGENT   *agent;                     /*  Handle for our agent             */
    THREAD  *thread;                    /*  Handle to console thread         */
#   include "smtsock.i"                 /*  Include dialog interpreter       */

    agent->priority = SMT_PRIORITY_HIGH;

    /*  Shutdown event comes from Kernel                                     */
    declare_smtlib_shutdown   (shutdown_event, SMT_PRIORITY_MAX);

    /*  Public methods supported by this agent                               */
    declare_smtsock_read    (read_event,    0);
    declare_smtsock_readr   (readr_event,   0);
    declare_smtsock_readh   (readh_event,   0);
    declare_smtsock_readrh  (readrh_event,  0);
    declare_smtsock_write   (write_event,   0);
    declare_smtsock_writeh  (writeh_event,  0);
    declare_smtsock_input   (input_event,   0);
    declare_smtsock_output  (output_event,  0);
    declare_smtsock_connect (connect_event, 0);
    declare_smtsock_close   (close_event,   0);
    declare_smtsock_flush   (flush_event,   0);

    /*  Alarm event sent by timer to this agent                              */
    declare_smttime_reply   (timeout_event, 0);

    /*  Private method used to cycle on select() call                        */
    method_declare (agent, "_ACTIVITY", activity_event, 0);

    /*  Ensure that operator console is running, else start it up            */
    if (agent_lookup (SMT_OPERATOR) == NULL)
        smtoper_init ();
    if ((thread = thread_lookup (SMT_OPERATOR, "")) != NULL)
        operq = thread->queue->qid;
    else
        return (-1);

    /*  Ensure that timer agent is running, else start it up                 */
    if (smttime_init ())
        return (-1);

    /*  Register blocking asyncronous function.  This must be done after     */
    /*  starting smttime or it will be overridden.                           */
    register_async_blocking (wait_for_activity_blocking, NULL);

    /*  Initialise the socket interface and register smtsock_term()          */
    if (sock_init () == 0)
        smt_atexit ((function) sock_term);
    else
      {
        send_smtoper_error (&operq,
                            "smtsock: could not initialise socket interface");
        send_smtoper_error (&operq,
                            strprintf ("smtsock: %s",
                                       connect_errlist [connect_error ()]));
        return (-1);
      }

    ip_nonblock = TRUE;                  /*  Want nonblocking sockets        */

    /*  Create initial, unnamed thread                                       */
    thread = thread_create (AGENT_NAME, "");
    sockq = thread->queue->qid;

    /*  Start with no sockets needing attention.  */
    FD_ZERO (&fd_in_check);
    FD_ZERO (&fd_out_check);
    FD_ZERO (&fd_err_check);
    top_socket = 0;
    new_top_socket = 0;
    deleted = FALSE;

    /*  Signal okay to caller that we initialised okay                       */
    return (0);
}

/*  ---------------------------------------------------------------------[<]-
    Function: smtsock_trace

    Synopsis: Enables/disables socket tracing: to enable, call with TRUE as
    argument; to disable call with FALSE as argument.  Socket trace data is
    sent to the console.
    ---------------------------------------------------------------------[>]-*/

void smtsock_trace (Bool trace_value)
{
    trace_flag = trace_value;
}


/*  -------------------------------------------------------------------------
 *  request_destroy
 *
 *  Destroys the specified request.
 */

static void
request_destroy (SOCKREQ *request)
{
    /*  Clear timer request, if there was one                                */
    if (request->timeout)
        clear_requests_matching_tag (&sockq, (void *) request->id);

    /*  Only flag request as deleted if it was queued.                       */
    if (request->queued)
      {
        deleted = TRUE;
        deleted_walking = TRUE;

        /*  If this request is the current request in request walk, advance  */
        /*  now to the next request.                                         */
        if (request == current_request)
            current_request = current_request->next;
      }

    /*  Free dynamically-allocated fields in the request block, as reqd.     */
    mem_free (request->buffer);
    mem_free (list_unlink (request));
}


int
wait_for_activity_blocking (long wait_date, long wait_time, void *dummyarg)
{
    SOCKREQ
        *request,
        *next_request;
    long
        current_date,
        current_time,
        days,
        csecs;
    struct timeval
        timeout;
    struct timeval
        *timeoutptr;
    int
        rc;                                /*  Return code from select()     */

    if (trace_flag)
        coprintf ("Blocking wait until %lu %lu.", wait_date, wait_time);

    /*  Since this is idle time, we start by doing a request merge and       */
    /*  recalculate top_socket as required.  We optimise here is several     */
    /*  ways: If no request has been deleted since top_socket was calculated */
    /*  or if all waiting requests have been merged then the loop can stop.  */
    /*  Careful not to update top_socket if the loop was terminated early.   */
    /*  Another simple optimisation is to find the first request on a        */
    /*  different socket before doing the merge, since the merge can make    */
    /*  the finding process longer.                                          */
    request = (SOCKREQ *) main_list.next;
    new_top_socket = 0;
    while ((wait_count || deleted)
       &&  ((NODE *) request != &main_list))
      {
        next_request = request;
        while ((NODE *) next_request != &main_list
           &&  next_request->handle == request->handle)
            next_request = next_request->next;
        if (wait_count
        &&  FD_ISSET ((int) request->handle, fd_waiting))
            merge_waiting_requests_same_socket (&request);
        request = next_request;

        /*  Since the original request may have been deleted during the      */
        /*  merge, we adjust new_top_socket to the handle of the previous    */
        /*  remaining request.                                               */
        if ((NODE *) request->prev != &main_list)
            new_top_socket = max (request->prev->handle, new_top_socket);
      }

    if (deleted)
      {
        ASSERT (new_top_socket <= top_socket);
        if (trace_flag
        &&  new_top_socket < top_socket)
            coprintf ("smtsock: -- setting top_socket to %d",
                      new_top_socket) ;

        top_socket = new_top_socket;
        deleted = FALSE;
      }

    if (wait_date < 99999999)
      {
        timeoutptr = &timeout;          /*  Work out how long to wait.       */
        get_date_time_now(&current_date, &current_time);
        date_diff (wait_date, wait_time,
                   current_date, current_time,
                   &days, &csecs);
        csecs++;                        /*  Round up to avoid busy waiting.  */

        if ((days >= 0) && (csecs >= 0))   /*  wait_date/time is in future.  */
          {
            timeout.tv_sec  = days * 86400 + csecs / 100;
            timeout.tv_usec = (csecs % 100) * 10000;
          }
        else
          {
            /*  If time is in the past, do the select without blocking  */
            timeout.tv_sec  = 0;
            timeout.tv_usec = 0;
          }
      }
    else
      {
        if (top_socket)
            timeoutptr = NULL;          /*  Block indefinitely.              */
        else
            return 0;                   /*  Don't wait forever on no socket. */
      }

    /*  First copy sets to check because select () uses the same data        */
    /*  space for its result.                                                */
    memcpy (&fd_in_result,  &fd_in_check,  sizeof (fd_set));
    memcpy (&fd_out_result, &fd_out_check, sizeof (fd_set));
    memcpy (&fd_err_result, &fd_err_check, sizeof (fd_set));
#   if defined (DEBUG)
    if (trace_flag)
        print_sock_select (
            "smtsock: called blocking select",
            (int) top_socket + 1,
            &fd_in_result,
            &fd_out_result,
            &fd_err_result,
            timeoutptr);
#   endif
    rc = sock_select (
            (int) top_socket + 1,       /*  Handles to check                 */
            &fd_in_result,              /*  Check/result for input           */
            &fd_out_result,             /*  Check/result for output          */
            &fd_err_result,             /*  Check/result for error           */
            timeoutptr);

    if (rc > 0)
      {
#   if defined (DEBUG)
    if (trace_flag)
        print_sock_select (
            "smtsock: returned activity",
            (int) top_socket + 1,
            &fd_in_result,
            &fd_out_result,
            &fd_err_result,
            timeoutptr);
#   endif
        event_send (
            &sockq,                     /*  Send to ourselves                */
            NULL,                       /*  No queue for reply               */
            "_ACTIVITY",                /*  Name of event to send            */
            NULL, 0,                    /*  No event body                    */
            NULL, NULL, NULL,           /*  No response events               */
            0);                         /*  No timeout                       */

        walking = TRUE;
        return 1;
      }

    if (rc < 0)
      {
         int errorvalue = errno;

         trace("smtsock: Error in (blocking) select: %d (%d)\n",
               rc, errorvalue);
         fprintf(stderr, "smtsock: Error in (blocking) select: "
               "%d (%d)\n", rc, errorvalue);
      }

    return rc;
}


static void
merge_waiting_requests_same_socket (SOCKREQ **request)
{
    SOCKREQ
        *waiting_request,
        *next_waiting_request,
        *after_output_request,
        *after_input_request;
    Bool
        disallow_input,
        disallow_output;
    sock_t
        handle;

    disallow_input  = FALSE;
    disallow_output = FALSE;
    handle = (*request)->handle;

    if (trace_flag)
        coprintf ("smtsock: -- merging requests on %d", handle) ;

    FD_CLR ((int) handle, fd_waiting);

    after_output_request = NULL;
    after_input_request  = NULL;
    waiting_request = (SOCKREQ *) wait_list.next;
    FOREVER
      {
        /*  Find waiting request on same socket as current request.      */
        while (((NODE *) waiting_request != &wait_list)
           &&  (waiting_request->handle != handle))
                waiting_request = waiting_request->next;

        if ((NODE *) waiting_request == &wait_list)
            break;

        next_waiting_request = waiting_request->next;
        list_unlink (waiting_request);
        wait_count--;

        /*  Find place to merge request as required.  This code is optimised */
        /*  and delicate.  Note that requests in the main list are grouped   */
        /*  by socket number and within those groups are sorted so that      */
        /*  output requests precede input requests.  Within a specific       */
        /*  request type they are in the order in which they were received.  */
        /*  At the same time, this loop rejects illegal request sequences:   */
        /*  anything after a close, output after an OUTPUT or input after an */
        /*  INPUT or READR.                                                  */

        if (! after_output_request)
          {
            after_output_request = *request;
            while (((NODE *) after_output_request !=  &main_list)
               &&  (after_output_request->handle == handle)
               &&  (request_is_output (after_output_request)))
              {
                disallow_output
                     = disallow_output
                    || after_output_request->type == SOCK_OUTPUT
                    || after_output_request->type == SOCK_CLOSE;

                disallow_input
                     = disallow_input
                    || after_output_request->type == SOCK_CLOSE;

               after_output_request = after_output_request->next;
              }
          }

        if (request_is_output (waiting_request))
          {
            if (disallow_output)
                reply_illegal_sequence_error (waiting_request);
            else
              {
                list_relink_before (waiting_request, after_output_request);

                /*  If there were previously no output requests, set request */
                /*  to point to the new request, now the first request on    */
                /*  the socket.                                              */
                if (after_output_request == *request)
                    *request = waiting_request;
              }
          }
        else
          {
            if (! after_input_request)
              {
                after_input_request = after_output_request;
                while ((NODE *) after_input_request != &main_list
                   &&  after_input_request->handle == handle)
                  {
                    disallow_input
                         = disallow_input
                        || after_input_request->type == SOCK_INPUT
                        || after_input_request->repeat;

                    after_input_request = after_input_request->next;
                  }
              }

            if (disallow_input)
                reply_illegal_sequence_error (waiting_request);
            else
              {
                list_relink_before (waiting_request, after_input_request);

                /*  If there were previously no input requests then the new  */
                /*  request is the first request after output requests.      */
                if (after_input_request == after_output_request)
                    after_output_request = waiting_request;
              }
          }

        waiting_request = next_waiting_request;
      }
}


static void
send_ok_reply (SOCKREQ *request)
{
    if (request->reply)
      {
        if (request->type == SOCK_READ)
            if (request->huge_block)
                lsend_smtsock_readh_ok (&request->reply_to, &sockq,
                                        NULL, NULL, NULL, 0,
                                        request->cur_size,
                                        request->buffer,
                                        request->tag);
            else
                lsend_smtsock_read_ok (&request->reply_to, &sockq,
                                       NULL, NULL, NULL, 0,
                                       (dbyte) request->cur_size,
                                       request->buffer,
                                       request->tag);
        else
        if (request->type == SOCK_CONNECT)
            lsend_smtsock_connect_ok (&request->reply_to, &sockq,
                                      NULL, NULL, NULL, 0,
                                      request->handle,
                                      request->tag);
        else
            lsend_smtsock_ok (&request->reply_to, &sockq,
                              NULL, NULL, NULL, 0,
                              request->tag);
      }
    if (request->repeat)
        request->cur_size = 0;
    else
        request_destroy (request);
}


static void
send_closed_reply (SOCKREQ *request)
{
    if (request->type == SOCK_READ)
        if (request->huge_block)
            lsend_smtsock_readh_closed (&request->reply_to, &sockq,
                                        NULL, NULL, NULL, 0,
                                        request->cur_size,
                                        request->buffer,
                                        request->tag);
        else
            lsend_smtsock_read_closed (&request->reply_to, &sockq,
                                       NULL, NULL, NULL, 0,
                                       (dbyte) request->cur_size,
                                       request->buffer,
                                       request->tag);
    else
        lsend_smtsock_closed (&request->reply_to, &sockq,
                              NULL, NULL, NULL, 0,
                              request->tag);

    request_destroy (request);
}


static void
reply_illegal_sequence_error (SOCKREQ *request)
{
    reply_error (&request->reply_to,
                 "Illegal request sequence.",
                 request->tag);

    request_destroy (request);
}


#if defined (DEBUG)

void
print_sock_check (void)
{
    struct timeval
        no_block = { 0, 0 };            /*  Timeout for select = don't block */

    coprintf ("-------------------------------------------------------");
    print_sock_lists ();
    print_sock_select (
            "smtsock: Handle checked",
                (int) top_socket,
                &fd_in_check,
                &fd_out_check,
                &fd_err_check,
                &no_block);
    coprintf ("-------------------------------------------------------");
}

static void
print_sock_lists (void)
{
    SOCKREQ
        *current;

    coprintf ("Main list content");
    current = main_list.next;
    while ((NODE *) current != &main_list)
      {
        print_sock_request (current);
        current = current->next;
      }

    coprintf ("Wait list content");
    current = wait_list.next;
    while ((NODE *) current != &wait_list)
      {
        print_sock_request (current);
        current = current->next;
      }
}

static void
print_sock_request (SOCKREQ *request)
{
    char
        *sock_type = "?";

    switch (request->type)
      {
        case SOCK_READ:    sock_type = "READ   "; break;
        case SOCK_WRITE:   sock_type = "WRITE  "; break;
        case SOCK_INPUT:   sock_type = "INPUT  "; break;
        case SOCK_OUTPUT:  sock_type = "OUTPUT "; break;
        case SOCK_CLOSE:   sock_type = "CLOSE  "; break;
        case SOCK_CONNECT: sock_type = "CONNECT"; break;
      }
    coprintf ("%s  id=%ld handle=%d size=%ld min=%ld max=%ld reply=%d",
              sock_type,
              request->id,
              request->handle,
              request->cur_size,
              request->min_size,
              request->max_size,
              request->reply);
}


static void
print_sock_select (const char *intro, int num_sockets,
                   fd_set *readfds, fd_set *writefds, fd_set *errorfds,
                   struct timeval *timeout)
{
    static char buffer [1024 + 1];

    buffer [0] = 0;
    strncat (buffer, strprintf ("%s: top=%d, read=(", intro, num_sockets), 1024);
    print_sock_select_set (buffer, num_sockets, readfds);
    strncat (buffer, "), write=(", 1024);
    print_sock_select_set (buffer, num_sockets, writefds);
    strncat (buffer, "), error=(", 1024);
    print_sock_select_set (buffer, num_sockets, errorfds);
    if (timeout)
        strncat (buffer, strprintf ("), timeout=%ld.%03d",
                 timeout->tv_sec, (long) (timeout->tv_usec / 1000)),
                 1024);
    else
        strncat (buffer, "], NULL)", 1024);

    coprintf ("%s", buffer);
}

static void
print_sock_select_set (char *buffer, int num_sockets, fd_set *set)
{
    int i;
    Bool first = TRUE,
        second = FALSE,
        third = FALSE;

    for (i = 0; i < num_sockets; ++i)
        if (FD_ISSET (i, set))
          {
            if (first)
              {
                strncat (buffer, strprintf ("%d", i), 1024);
                first = FALSE;
              }
            else
                if (second)
                    third = TRUE;
                else
                    strncat (buffer, strprintf (", %d", i), 1024);

            if (! second)
              {
                second = TRUE;
                third = FALSE;
              }
          }
        else
          {
            second = FALSE;
            if (third)
              {
                third = FALSE;
                strncat (buffer, strprintf (" .. %d", i-1), 1024);
              }
          }

    if (third)
        strncat (buffer, strprintf (" .. %d", i-1), 1024);
}
#endif


/*************************   INITIALISE THE THREAD   *************************/

MODULE initialise_the_thread (THREAD *thread)
{
    list_reset (&main_list);            /*  Initialise requests lists        */
    list_reset (&wait_list);
    wait_count = 0;

    the_next_event = ok_event;
}


/**********************   SCHEDULE NON BLOCKING SELECT   *********************/

MODULE schedule_non_blocking_select (THREAD *thread)
{
    /*  Ensure our polling function is scheduled to run after the next       */
    /*  state change.                                                        */
#if defined (DEBUG)
    if (trace_flag)
        print_sock_check();
#endif
    schedule_polling_function (check_activity_nonblock, NULL, 1);
    walking = FALSE;
}

/*  Maximum number of thread state changes between polling for activity      */
#define MAX_POLLING_INTERVAL   (10)

int
check_activity_nonblock (int interval, void *dummyarg)
{
    struct timeval
        no_block = { 0, 0 };            /*  Timeout for select = don't block */
    int
        rc;                             /*  Return code from select()        */

    /*  First copy sets to check because select () uses the same data        */
    /*  space for its result.                                                */
    memcpy (&fd_in_result,  &fd_in_check,  sizeof (fd_set));
    memcpy (&fd_out_result, &fd_out_check, sizeof (fd_set));
    memcpy (&fd_err_result, &fd_err_check, sizeof (fd_set));
    if (top_socket)
      {
#if defined (DEBUG)
        if (trace_flag)
          {
            print_sock_lists ();
            print_sock_select (
                "smtsock: called non-blocking select",
                (int) top_socket + 1,
                &fd_in_result,
                &fd_out_result,
                &fd_err_result,
                &no_block);
          }
#endif

        rc = sock_select ((int) top_socket + 1, /*  Handles to check         */
                          &fd_in_result,        /*  Check/result for input   */
                          &fd_out_result,       /*  Check/result for output  */
                          &fd_err_result,       /*  Check/result for error   */
                          &no_block);           /*  Timeout                  */

        if (rc > 0)
          {
#if defined (DEBUG)
            if (trace_flag)
                print_sock_select (
                    "smtsock: returned activity",
                    (int) top_socket + 1,
                    &fd_in_result,
                    &fd_out_result,
                    &fd_err_result,
                    &no_block);
#endif
            /*  Don't send another ACTIVITY event if we are already walking  */
            /*  the request list.                                            */
            if (! walking)
              {
                event_send (
                    &sockq,             /*  Send to ourselves                */
                    NULL,               /*  No queue for reply               */
                    "_ACTIVITY",        /*  Name of event to send            */
                    NULL, 0,            /*  No event body                    */
                    NULL, NULL, NULL,   /*  No response events               */
                    0);                 /*  No timeout                       */

                walking = TRUE;
              }
            /*  Temporarily disable calls to ourselves; the main thread  */
            /*  will reschedule us when required.                        */
            schedule_polling_function (check_activity_nonblock, NULL, 0);

            return 1;
          }
        else
          {
            if (rc == 0)
              {
                /*  Nothing happened, reschedule ourselves to run less   */
                /*  often; once activity happens we'll be scheduled back */
                /*  up again.                                            */
                if (interval < MAX_POLLING_INTERVAL)
                    schedule_polling_function (check_activity_nonblock,
                                               NULL, (interval + 1));
              }
            else /* (rc < 0) */
              {
                int errorvalue = errno;

                trace("smtsock: Error in (non-blocking) select: "
                      "%d (%d)\n", rc, errorvalue);
                coprintf ("smtsock: Error in (non-blocking) select: "
                          "%d (%d)\n", rc, errorvalue);
              }

            return rc;
          }
      }
    return 0;
}


/**************************   CREATE READ REQUEST   **************************/

MODULE create_read_request (THREAD *thread)
{
    SOCKREQ
        *request;

    request = create_small_read_type_request (thread);
    if (request)
        process_read_type_request (request);
}


static SOCKREQ *
create_small_read_type_request (THREAD *thread)
{
    SOCKREQ
        *request = NULL;
    struct_smtsock_read
        *message;

    /*  Get arguments from message                                           */
    if (get_smtsock_read (
            thread->event->body,
            &message))
      {
        raise_exception (fatal_event);
        return NULL;
      }

    if (trace_flag)
        coprintf ("smtsock: READ min=%d max=%d socket=%ld timeout=%d",
                  message->min_size,
                  message->max_size,
                  message->socket,
                  message->timeout) ;

    if (message->max_size == 0 || message->socket == 0)
        reply_error (&thread->event->sender,
                     "Null read request",
                     message->tag);
    else
      {
        if ((request = request_create (&thread->event->sender,
                                       SOCK_READ,
                                       message->timeout,
                                       message->socket,
                                       message->tag)) != NULL)
          {
            request->max_size = message->max_size;
            request->min_size = message->min_size
                               ? message->min_size
                               : message->max_size;

            if (! socket_is_alive (message->socket))
              {
                send_closed_reply (request);
                request = NULL;
              }
          }
      }
    free_smtsock_read (&message);
    return request;
}


/*  -------------------------------------------------------------------------
 *  request_create
 *
 *  Creates a new request, and initialises it to empty.  If the request
 *  could not be created, sends an SOCK_ERROR event to the caller, and
 *  returns null.  Otherwise returns the address of the created request.
 */

static SOCKREQ *
request_create (QID *reply_to,
                REQUEST_TYPE request_type,
                dbyte timeout,
                sock_t handle,
                void *tag)
{
    SOCKREQ
        *request;                       /*  Request we create                */

    if (handle == 0 || handle == INVALID_SOCKET)
      {
        reply_error (reply_to ,
                     "Invalid socket handle",
                     tag);
        return NULL;
      }

    list_create (request, sizeof (SOCKREQ));
    if (! request)
        reply_error (reply_to,
                     "Out of memory",
                     tag);
    else
      {
        /*  Initialise the request with default values                       */
        request->id         = ++last_id;
        request->type       = request_type;
        request->reply_to   = *reply_to;
        request->handle     = handle;
        request->buffer     = NULL;
        request->max_size   = 0;
        request->cur_size   = 0;
        request->min_size   = 0;
        request->tag        = tag;
        request->repeat     = FALSE;
        request->huge_block = FALSE;
        request->timeout    = timeout;
        request->reply      = TRUE;
        request->queued     = FALSE;

        /*  It's really not correct ANSI C to compute with timevals; this    */
        /*  will just have to do for now.  It may break on weird systems.    */
        request->expires = timeout ? time (NULL) + timeout : 0;
      }
    return (request);
}


static void
process_read_type_request (SOCKREQ *request)
{
    Bool
        queue = TRUE;

    /*  If no input requests are currently queued we can try to              */
    /*  read data before queuing the request.  If enough data are            */
    /*  available, we reply immediately to the request.  Otherwise,          */
    /*  or if this request is repeated, we queue it.  We check for           */
    /*  a closed socket here but other errors can wait for regular           */
    /*  processing.                                                          */
    if (! FD_ISSET ((int) request->handle, &fd_in_check))
      {
        read_some_data (request);

        if (request->cur_size >= request->min_size)
          {
            queue = request->repeat;
            send_ok_reply (request);
          }
        else
        if (sockerrno == EPIPE || sockerrno == ECONNRESET)
          {
            send_closed_reply (request);
            queue = FALSE;
          }
        else
            queue = TRUE;
      }

    if (queue)
        request_queue (request);
}


static int
read_some_data (SOCKREQ *request)
{
    int
        rc;

    if (! request->buffer)
      {
        request->buffer = mem_alloc (request->max_size);
        if (request->buffer == NULL)
          {
            raise_exception (fatal_event);
            return 0;
          }
      }

    rc = read_TCP (request->handle,
                   request->buffer   + request->cur_size,
         (size_t) (request->max_size - request->cur_size));

    if (trace_flag)
      {
        byte *ptr = (byte *) request->buffer + request->cur_size;
        coprintf ("smtsock: reading %d bytes '%02x %02x %02x %02x %02x %02x %02x %02x...'",
                  rc, ptr [0], ptr [1], ptr [2], ptr [3],
                      ptr [4], ptr [5], ptr [6], ptr [7]);
      }

    if (rc > 0)
        request->cur_size += rc;

    return rc;
}


static void
request_queue (SOCKREQ *request)
{
    if (request->timeout)
      {
        smttime_request_alarm_with_reply (
            &sockq,
            NULL, NULL, NULL, 0,
            0, request->timeout * 100,
            (void *) request->id);  /*  Tag data     */
      }
    if (! FD_ISSET ((int) request->handle, &fd_err_check))
      {                             /*  Not a duplicated request         */
        FD_SET ((int) request->handle, &fd_err_check);
        node_relink_before (request, &main_list);
      }
    else                            /*  Duplicated request               */
      {
        if (! fd_waiting)
          {
            fd_waiting = mem_alloc (sizeof (*fd_waiting));

            FD_ZERO (fd_waiting);
          }
        FD_SET ((int) request->handle, fd_waiting);
        node_relink_before (request, &wait_list);
        wait_count++;
      }

    FD_SET ((int) request->handle,
            request_is_input (request) ? &fd_in_check : &fd_out_check);

    /*  Recalculate both top_socket and new_top_socket.  The latter          */
    /*  because we are potentially in the process of recalculating           */
    /*  top_socket in new_top_socket.  Subtle.                               */
    top_socket     = max (request->handle, top_socket);
    new_top_socket = max (request->handle, new_top_socket);

    /*  And flag that request is queued.                                     */
    request->queued = TRUE;
}


/*  -------------------------------------------------------------------------
 *  reply_error
 *
 *  Formats and sends a message containing the socket number and an error
 *  message.
 */

static void
reply_error (QID *qid, char *message, void *tag)
{
    lsend_smtsock_error (qid,           /*  Send to specified queue          */
                         &sockq,        /*  No queue for reply               */
                         NULL, NULL, NULL, 0,
                         message,
                         (void *) tag);
}


/***********************   CREATE READ REPEAT REQUEST   **********************/

MODULE create_read_repeat_request (THREAD *thread)
{
    SOCKREQ
        *request;

    request = create_small_read_type_request (thread);
    if (request)
      {
        request->repeat = TRUE;
        process_read_type_request (request);
      }
}


/************************   CREATE HUGE READ REQUEST   ***********************/

MODULE create_huge_read_request (THREAD *thread)
{
    SOCKREQ
        *request;

    request = create_huge_read_type_request (thread);
    if (request)
        process_read_type_request (request);
}


static SOCKREQ *
create_huge_read_type_request (THREAD *thread)
{
    SOCKREQ
        *request = NULL;
    struct_smtsock_readh
        *message;

    /*  Get arguments from message                                           */
    if (get_smtsock_readh (
            thread->event->body,
            &message))
      {
        raise_exception (fatal_event);
        return NULL;
      }

    if (trace_flag)
        coprintf ("smtsock: READH min=%ld max=%ld socket=%ld timeout=%d",
                  message->min_size,
                  message->max_size,
                  message->socket,
                  message->timeout);

    if (message->max_size == 0 || message->socket == 0)
        reply_error (&thread->event->sender,
                     "Null read request",
                     message->tag);
    else
    if (message->max_size > UINT_MAX)
        reply_error (&thread->event->sender,
                     "Read request too large for memory model",
                     message->tag);
    else
    if ((request = request_create (&thread->event->sender,
                                   SOCK_READ,
                                   message->timeout,
                                   message->socket,
                                   message->tag)) != NULL)
      {
        request->huge_block = TRUE;
        request->max_size   = message->max_size;
        request->min_size   = message->min_size
                             ? message->min_size
                             : message->max_size;

        if (! socket_is_alive (message->socket))
          {
            send_closed_reply (request);
            request = NULL;
          }
      }
    free_smtsock_readh (&message);
    return request;
}


/********************   CREATE HUGE READ REPEAT REQUEST   ********************/

MODULE create_huge_read_repeat_request (THREAD *thread)
{
    SOCKREQ
        *request;

    request = create_huge_read_type_request (thread);
    if (request)
      {
        request->repeat = TRUE;
        process_read_type_request (request);
      }
}


/**************************   CREATE WRITE REQUEST   *************************/

MODULE create_write_request (THREAD *thread)
{
    SOCKREQ
        *request;
    struct_smtsock_write
        *message;

    /*  Get arguments from message                                           */
    if (get_smtsock_write (
            thread->event->body,
            &message))
      {
        raise_exception (fatal_event);
        return;
      }

    /*  For write requests we do not want to allow a zero timeout, it makes
        no sense and on some bogus OSes (Solaris) it can cause socket leaks.
     */
    if (message->timeout == 0)
        message->timeout = WRITE_TIMEOUT;

    if (trace_flag)
        coprintf (
            "smtsock: WRITE size=%d socket=%ld timeout=%d data=%x %x %x %x",
            message->size, message->socket, message->timeout,
            ((byte *) message->data) [0],
            ((byte *) message->data) [1],
            ((byte *) message->data) [2],
            ((byte *) message->data) [3]);

    if (message->size == 0 || message->socket == 0)
        reply_error (&thread->event->sender,
                     "Null write request",
                     message->tag);
    else
      {
        if ((request = request_create (&thread->event->sender,
                                       SOCK_WRITE,
                                       message->timeout,
                                       message->socket,
                                       message->tag)) != NULL)
          {
            request->max_size = message->size;
            request->min_size = message->size;
            request->buffer   = message->data;
            request->reply    = message->reply;

            message->data = NULL;      /*  Avoids deallocation of data      */

            if (socket_is_alive (message->socket))
                request_queue (request);
            else
                send_closed_reply (request);
          }
      }

    free_smtsock_write (&message);
}


/***********************   CREATE HUGE WRITE REQUEST   ***********************/

MODULE create_huge_write_request (THREAD *thread)
{
    SOCKREQ
        *request;
    struct_smtsock_writeh
        *message;

    /*  Get arguments from message                                           */
    if (get_smtsock_writeh (
            thread->event->body,
            &message))
      {
        raise_exception (fatal_event);
        return;
      }

    /*  For write requests we do not want to allow a zero timeout, it makes
        no sense and on some bogus OSes (Solaris) it can cause socket leaks.
     */
    if (message->timeout == 0)
        message->timeout = WRITE_TIMEOUT;

    if (trace_flag)
        coprintf (
            "smtsock: WRITEH size=%ld socket=%ld time=%d data=%x%x%x%x",
            message->size, message->socket, message->timeout,
            (byte) message->data [0], (byte) message->data [1],
            (byte) message->data [2], (byte) message->data [3]) ;

    if (message->size == 0 || message->socket == 0)
        reply_error (&thread->event->sender,
                     "Null write request",
                     message->tag);
    else
    if (message->size > UINT_MAX)
        reply_error (&thread->event->sender,
                     "Write request too large for memory model",
                     message->tag);
    else
      {
        if ((request = request_create (&thread->event->sender,
                                       SOCK_WRITE,
                                       message->timeout,
                                       message->socket,
                                       message->tag)) != NULL)
          {
            request->huge_block = TRUE;
            request->max_size   = message->size;
            request->min_size   = message->size;
            request->buffer     = message->data;
            request->reply      = message->reply;

            message->data = NULL;      /*  Avoids deallocation of data      */

            if (socket_is_alive (message->socket))
                request_queue (request);
            else
                send_closed_reply (request);
          }
      }

    free_smtsock_writeh (&message);
}


/**************************   CREATE INPUT REQUEST   *************************/

MODULE create_input_request (THREAD *thread)
{
    SOCKREQ
        *request;
    struct_smtsock_input
        *message;

    /*  Get arguments from message                                           */
    if (get_smtsock_input (
            thread->event->body,
            &message))
      {
        raise_exception (fatal_event);
        return;
      }

    if (trace_flag)
        /* send_smtoper_info (&operq, */coprintf ("%s",
                           strprintf ("smtsock: INPUT socket=%ld timeout=%d",
                                      message->socket, message->timeout));

    if (message->socket == 0)
        reply_error (&thread->event->sender,
                     "Null input request",
                     message->tag);
    else
      {
        if ((request = request_create (&thread->event->sender,
                                       SOCK_INPUT,
                                       message->timeout,
                                       message->socket,
                                       message->tag)) != NULL)
          {
            if (socket_is_alive (message->socket))
                request_queue (request);
            else
                send_closed_reply (request);
          }
      }
    free_smtsock_input (&message);
}


/*************************   CREATE OUTPUT REQUEST   *************************/

MODULE create_output_request (THREAD *thread)
{
    SOCKREQ
        *request;
    struct_smtsock_output
        *message;

    /*  Get arguments from message                                           */
    if (get_smtsock_output (
            thread->event->body,
            &message))
      {
        raise_exception (fatal_event);
        return;
      }

    if (trace_flag)
        /* send_smtoper_info (&operq, */coprintf ("%s",
                           strprintf ("smtsock: OUTPUT socket=%ld timeout=%d",
                                      message->socket, message->timeout));

    if (message->socket == 0)
        reply_error (&thread->event->sender,
                     "Null output request",
                     message->tag);
    else
      {
        if ((request = request_create (&thread->event->sender,
                                       SOCK_OUTPUT,
                                       message->timeout,
                                       message->socket,
                                       message->tag)) != NULL)
          {
            if (socket_is_alive (message->socket))
                request_queue (request);
          }
      }
    free_smtsock_output (&message);
}


/*************************   CREATE CONNECT REQUEST   ************************/

MODULE create_connect_request (THREAD *thread)
{
    SOCKREQ
        *request;
    struct_smtsock_connect
        *message;
    struct sockaddr_in
        host_addr;                      /*  Structure for connection         */
    sock_t
        handle;                         /*  Handle for connection            */

    /*  Get arguments from message                                           */
    if (get_smtsock_connect (
            thread->event->body,
            &message))
      {
        raise_exception (fatal_event);
        return;
      }

    if (trace_flag)
        coprintf ("smtsock: CONNECT type=%s to=%s/%s nbr=%lx/%d timeout=%d",
                  message->type,
                  message->host,
                  message->service,
                  message->address,
                  message->port,
                  message->timeout);

    /*  Build socket address structure and connect to host.  Either of the   */
    /*  information pairs (host, service) (address, port) will be used       */
    /*  by the connect function.                                             */
    build_sockaddr (&host_addr, message->address, message->port);
    handle = connect_socket (message->host,
                             message->service,
                             message->type,
                             &host_addr, 3, 0);

    /*  The connect call can fail, in which case we return the connect error */
    /*  message.  If the call succeeds, we need to wait until the socket is  */
    /*  ready for use, since we use non-blocking sockets.  We generate a     */
    /*  write request; when this is true we'll send an ok event plus the     */
    /*  socket handle to the calling program.                                */

    if (handle == INVALID_SOCKET)
        reply_error (&thread->event->sender,
                     connect_errlist [connect_error ()],
                     message->tag);
    else
    if (handle > 0)                     /*  Else wait until ready to write   */
      {
        if ((request = request_create (&thread->event->sender,
                                       SOCK_CONNECT,
                                       message->timeout,
                                       handle,
                                       message->tag)) != NULL)
            request_queue (request);
      }
    free_smtsock_connect (&message);
}


/**************************   CREATE CLOSE REQUEST   *************************/

MODULE create_close_request (THREAD *thread)
{
    SOCKREQ
        *request;
    struct_smtsock_close
        *message;
    int
        rc;
    Bool
        queue;

    /*  Get arguments from message                                           */
    if (get_smtsock_close (
            thread->event->body,
            &message))
      {
        raise_exception (fatal_event);
        return;
      }

    if (trace_flag)
        coprintf ("smtsock: CLOSE socket=%ld",
                  message->socket);

    if (message->socket == 0)
        reply_error (&thread->event->sender,
                     "Null close request",
                     message->tag);
    else
      {
        /*  If there are no pending requests, try just closing the socket    */
        /*  with a non-blocking call.                                        */
        if (! FD_ISSET ((int) message->socket, &fd_err_check))
          {
            queue = FALSE;
            rc = close_socket (message->socket);
            if (! rc)
              {
                if (message->reply)
                    lsend_smtsock_ok (&thread->event->sender, &sockq,
                                      NULL, NULL, NULL, 0,
                                      message->tag);
              }
            else
            if (sockerrno == EPIPE || sockerrno == ECONNRESET)
                lsend_smtsock_closed (&thread->event->sender, &sockq,
                                      NULL, NULL, NULL, 0,
                                      message->tag);
            else
                queue = TRUE;
          }
        else
            queue = TRUE;

        if (queue)
            if ((request = request_create (&thread->event->sender,
                                           SOCK_CLOSE,
                                           message->timeout,
                                           message->socket,
                                           message->tag)) != NULL)
              {
                request->reply = message->reply;
                request_queue (request);
              }
      }

    free_smtsock_close (&message);
}


/*************************   FLUSH SOCKET REQUESTS   *************************/

MODULE flush_socket_requests (THREAD *thread)
{
    struct_smtsock_flush
        *message;
    SOCKREQ
        *request;

    /*  Get arguments from message                                           */
    if (get_smtsock_flush (
            thread->event->body,
            &message))
      {
        raise_exception (fatal_event);
        return;
      }

    if (trace_flag)
        coprintf ("smtsock: FLUSH socket=%ld alltypes=%s",
                  message->socket,
                  message->alltypes ? "TRUE" : "FALSE");

    /*  Find any requests on the specified socket.  We optimise by looking   */
    /*  at the check bitsets to see if there are any requests to delete.     */
    if (message->alltypes
        ? FD_ISSET ((int) message->socket, &fd_err_check)
        : FD_ISSET ((int) message->socket, &fd_in_check))
      {
        /*  We know there must be a request so don't bother testing for the  */
        /*  end of the list in this loop.                                    */
        request = (SOCKREQ *) main_list.next;
        while (request->handle != message->socket)
            request = request->next;

        /*  We separate flush processing into two cases:                     */
        /*     1. Brute force when all requests can be deleted, ie either    */
        /*        alltypes is TRUE or there are no pending output requests.  */
        /*     2. More delicate if some requests are going to remain         */
        /*        following the flush.                                       */
        if (message->alltypes
        || (! FD_ISSET ((int) message->socket, &fd_out_check)))
          {
            /*  Clear output bitset here, rest of bitset processing later.   */
            FD_CLR ((int) message->socket, &fd_out_check);

            /*  Destroy requests in main_list.                               */
            while (((NODE *) request != &main_list)
               &&  (request->handle == message->socket))
              {
                request = request->next;
                request_destroy (request->prev);
              }

            /*  Destroy requests in wait_list.                               */
            if (wait_count
            &&  FD_ISSET ((int) message->socket, fd_waiting))
              {
                FD_CLR ((int) message->socket, fd_waiting);
                request = (SOCKREQ *) wait_list.next;
                while (wait_count
                   && ((NODE *) request != &wait_list))
                  {
                    request = request->next;
                    if (request->prev->handle == message->socket)
                      {
                        request_destroy (request->prev);
                        wait_count--;
                      }
                  }
              }
          }
        else
          {
            /*  Now deal with the more complicated case of just deleting     */
            /*  input requests.  Although more optimisation is possible, we  */
            /*  draw the line in the sand here and just do a merge before    */
            /*  destroying all input requests.  Note that any merged         */
            /*  requests which go before the current request must be output  */
            /*  requests, so can be ignored here.                            */
            if (wait_count
            &&  FD_ISSET ((int) message->socket, fd_waiting))
                merge_waiting_requests_same_socket (&request);

            /*  Skip past output-type requests.                              */
            while (((NODE *) request != &main_list)
               &&  (request_is_output (request))
               &&  (request->handle == message->socket))
                request = request->next;

            /*  Now we can destroy all requests on this socket.              */
            while (((NODE *) request != &main_list)
               &&  (request->handle == message->socket))
              {
                request = request->next;
                request_destroy (request->prev);
              }
          }

        /*  Now clear input bitset.                                          */
        FD_CLR ((int) message->socket, &fd_in_check);

        /*  And error checkset as necessary.                                 */
        if (! FD_ISSET ((int) message->socket, &fd_out_check))
            FD_CLR ((int) message->socket, &fd_err_check);
      }

    free_smtsock_flush (&message);
}


/*************************   FIND TIMED OUT REQUEST   ************************/

MODULE find_timed_out_request (THREAD *thread)
{
    struct_smttime_tag
        *timer_reply;
    long
        timeout_id;
    SOCKREQ
        *request;

    /*  Get request from event body                                          */
    get_smttime_tag (thread->event->body, &timer_reply);
    timeout_id = (long) timer_reply->tag;
    free_smttime_tag (&timer_reply);

    active_request = NULL;
    FORLIST (request, main_list)
      {
        /*  Merge as we look for timed-out request - it could still be in    */
        /*  wait_list.                                                       */
        if (wait_count
        &&  FD_ISSET ((int) request->handle, fd_waiting))
            merge_waiting_requests_same_socket (&request);

        /*  Identify time-out request by its ID.  */
        if (timeout_id == request->id)
          {
            active_request = request;
            break;
          }
      }
}


/************************   REPLY TIMEOUT TO REQUEST   ***********************/

MODULE reply_timeout_to_request (THREAD *thread)
{
    if (active_request)
      {
        if (active_request->type == SOCK_READ)
            if (active_request->huge_block)
                send_smtsock_readh_timeout (&active_request->reply_to,
                                            active_request->cur_size,
                                            active_request->buffer,
                                            active_request->tag);
            else
                send_smtsock_read_timeout (&active_request->reply_to,
                                           (dbyte) active_request->cur_size,
                                           active_request->buffer,
                                           active_request->tag);
        else
            send_smtsock_timeout (&active_request->reply_to,
                                  active_request->tag);

        /*  Since timeouts don't necessarily occur in the same order as      */
        /*  requests are made, test that there are no preceding requests of  */
        /*  the same type before clearing the corresponding check bits.      */
        if (request_is_input (active_request))
          {
            if (((NODE *) active_request->prev == &main_list)
            ||  (active_request->prev->handle != active_request->handle)
            ||  (request_is_output (active_request->prev)))
                clear_check_after_input_request (active_request);
          }
        else
            if (((NODE *) active_request->prev == &main_list)
            ||  (active_request->prev->handle != active_request->handle))
                clear_check_after_output_request (active_request);

        request_destroy (active_request);
      }
}


static void
clear_check_after_input_request (SOCKREQ *request)
{
    sock_t
        handle = request->handle;

    if (((NODE *) request->next == &main_list)
    ||  (request->next->handle != handle))
      {
        FD_CLR ((int) handle, &fd_in_check);
        if (! FD_ISSET ((int) handle, &fd_out_check))
            FD_CLR ((int) handle, &fd_err_check);
      }
}


static void
clear_check_after_output_request (SOCKREQ *request)
{
    sock_t
        handle = request->handle;

    if (((NODE *) request->next == &main_list)
    ||  (request_is_input (request->next))
    ||  (request->next->handle != handle))
      {
        FD_CLR ((int) handle, &fd_out_check);
        if (! FD_ISSET ((int) handle, &fd_in_check))
            FD_CLR ((int) handle, &fd_err_check);
      }
}


/**********************   CHECK FIRST SOCKET ACTIVITY   **********************/

MODULE check_first_socket_activity (THREAD *thread)
{
    current_request = (SOCKREQ *) main_list.next;
    new_top_socket = 0;
    deleted_walking = FALSE;
    check_next_socket_activity (thread);
}


/***********************   CHECK NEXT SOCKET ACTIVITY   **********************/

MODULE check_next_socket_activity (THREAD *thread)
{
    sock_t
        handle;

#if defined (DEBUG)
    if (trace_flag)
        print_sock_check();
#endif

    active_request = NULL;
    while ((NODE *) current_request != &main_list)
      {
        handle = current_request->handle;

        if (wait_count
        &&  FD_ISSET ((int) handle, fd_waiting))
            merge_waiting_requests_same_socket (&current_request);

        new_top_socket = max (handle, new_top_socket);

        /*  Now check for any kind of activity on the socket.  Error         */
        /*  activity applies to all requests on that socket.  Other activty  */
        /*  only applies to one request of the given type (input or output)  */
        /*  so we clear the activity bit in the relevant result set.         */
        if (FD_ISSET ((int) handle, &fd_err_result))
          {
            active_request = current_request;
            current_request = current_request->next;
          }
        else
        if (request_is_output (current_request))
          {
            if (FD_ISSET ((int) handle, &fd_out_result))
              {
                active_request = current_request;

                /*  Clear output result immediately.                         */
                FD_CLR ((int) handle, &fd_out_result);

                /*  If request is a close, reply closed to any other         */
                /*  requests on the same socket immediately.                 */
                if (active_request->type == SOCK_CLOSE)
                  {
                    FD_CLR ((int) handle, &fd_out_check);
                    FD_CLR ((int) handle, &fd_in_check);
                    FD_CLR ((int) handle, &fd_in_result);
                    FD_CLR ((int) handle, &fd_err_check);
                    while (((NODE *) current_request->next != &main_list)
                       &&  (current_request->next->handle == handle))
                        send_closed_reply (current_request->next);
                  }
                else
                    clear_check_after_output_request (active_request);
              }

            /*  Skip past other output requests on this socket.              */
            do current_request = current_request->next;
            while (((NODE *) current_request != &main_list)
               &&  (current_request->handle == handle)
               &&  (request_is_output (current_request)));
          }
        else                            /*  Request is input-type.           */
          {
            if (FD_ISSET ((int) handle, &fd_in_result))
              {
                active_request = current_request;

                /*  Clear input result immediately and input check if no     */
                /*  further input requests remain.                           */
                FD_CLR ((int) handle, &fd_in_result);
                if (! active_request->repeat)
                    clear_check_after_input_request (active_request);
              }
            /*  Skip past other requests on this socket.                     */
            do current_request = current_request->next;
            while (((NODE *) current_request != &main_list)
               &&  (current_request->handle == handle));
          }

        /*  If any event got set, we can access it as active_request         */
        if (active_request)
          {
            if (trace_flag)
                coprintf ("smtsock: -- activity on %d",
                          active_request->handle);

            break;
          }
      }

    /*  End-of-loop processing.  Replace top_socket with new_top_socket.     */
    if ((NODE *) current_request == &main_list)
      {
        if (trace_flag
        &&  new_top_socket < top_socket)
            if (trace_flag)
                coprintf ("smtsock: -- setting top_socket to %d",
                          new_top_socket);

        top_socket = new_top_socket;

        /*  And if no requests were deleted while the request walk was in    */
        /*  progress then we can flag no deleted requests ie top_socket is   */
        /*  guaranteed to be up-to-date.                                     */
        if (! deleted_walking)
            deleted = FALSE;
      }
#   if defined (DEBUG)
    if (trace_flag)
        print_sock_check();
#   endif
}


/**********************   GENERATE REQUEST TYPE EVENT   **********************/

MODULE generate_request_type_event (THREAD *thread)
{
    static event_t request_type_event [] = {
        read_event,
        write_event,
        input_event,
        output_event,
        close_event,
        connect_event
    };

    if (! active_request)
        the_next_event = no_active_request_event;
    else
        if (FD_ISSET ((int) active_request->handle, &fd_err_result))
            the_next_event = exception_event;
        else
            the_next_event = request_type_event [active_request->type];
}


/*************************   READ DATA FROM SOCKET   *************************/

MODULE read_data_from_socket (THREAD *thread)
{
    /*  Read as much data as we can from the request's socket, then          */
    /*  update the request appropriately.                                    */
    if (trace_flag)
        coprintf ("smtsock: reading %d bytes from %ld",
                  active_request->max_size - active_request->cur_size,
                  active_request->handle);

    handle_partial_io (active_request, read_some_data (active_request));
}


/*  -------------------------------------------------------------------------
 *  handle_partial_io
 *
 *  Handles the return code from a socket read or write, to update the
 *  request size indicators and set the next event.
 */

static void
handle_partial_io (SOCKREQ *request, int bytes_done)
{
    /*  If we read something, update the request cur_size, and check if      */
    /*  we got everything.  If so, we can signal 'finished'.  Else we loop.  */
    if (bytes_done > 0)
      {
        if (request->cur_size >= request->min_size)
            the_next_event = finished_event;
        else
            the_next_event = incomplete_event;
      }

    /*  If the return code was zero, the socket got closed.  Whatever we     */
    /*  got, we'll send back.  Some systems return EPIPE or ECONNRESET.      */
    else
    if (bytes_done == 0 || sockerrno == EPIPE || sockerrno == ECONNRESET)
        the_next_event = closed_event;
    else
    /*  In principle we can't get an EAGAIN, since we waited until the       */
    /*  socket was ready, but you never know.  We'll just try again...       */
    if (sockerrno == EAGAIN || sockerrno == EWOULDBLOCK)
        the_next_event = incomplete_event;

    /*  Anything else, that's an error                                       */
    else
        the_next_event = error_event;

    /*  If request wasn't completed, set the check bits again.               */
    if (the_next_event == incomplete_event)
      {
        FD_SET ((int) request->handle, &fd_err_check);
        FD_SET ((int) request->handle,
                request_is_input (request) ? &fd_in_check : &fd_out_check);
      }
}


/**************************   WRITE DATA TO SOCKET   *************************/

MODULE write_data_to_socket (THREAD *thread)
{
    /*  Write as much data as we can to the request's socket, then           */
    /*  update the request appropriately.                                    */
#if defined (DEBUG)
    if (trace_flag)
        print_sock_check();
#endif
    if (trace_flag)
        coprintf ("smtsock: writing %d bytes to %ld",
                  active_request->max_size - active_request->cur_size,
                  active_request->handle) ;

    handle_partial_io (active_request, write_some_data (active_request));
#if defined (DEBUG)
    if (trace_flag)
        print_sock_check();
#endif

}


static int
write_some_data (SOCKREQ *request)
{
    int
        rc;

    rc = write_TCP (request->handle,
                    request->buffer   + request->cur_size,
          (size_t) (request->max_size - request->cur_size));

    if (trace_flag)
      {
        byte *ptr = (byte *) request->buffer + request->cur_size;
        coprintf ("smtsock: writing %02x %02x %02x %02x %02x %02x %02x %02x...",
                  ptr [0], ptr [1], ptr [2], ptr [3],
                  ptr [4], ptr [5], ptr [6], ptr [7]);
      }

    request->cur_size += rc;

    return rc;
}


/************************   CONFIRM SOCKET CONNECTED   ***********************/

MODULE confirm_socket_connected (THREAD *thread)
{
    int
        rc;

    rc = socket_error (active_request->handle);
    if (rc)
        the_next_event = error_event;
    else
        the_next_event = finished_event;
}


/**************************   REPLY OK TO REQUEST   **************************/

MODULE reply_ok_to_request (THREAD *thread)
{
#if defined (DEBUG)
    if (trace_flag)
        print_sock_check();
#endif
    send_ok_reply (active_request);
#if defined (DEBUG)
    if (trace_flag)
      {
        coprintf ("After send ok");
        print_sock_check();
      }
#endif
}


/****************************   CLOSE THE SOCKET   ***************************/

MODULE close_the_socket (THREAD *thread)
{
    int
        rc;

    if (trace_flag)
        coprintf ("smtsock: closing socket %ld",
                  active_request->handle);

    rc = close_socket (active_request->handle);
    if (! rc)
        the_next_event = finished_event;
    else
    if (sockerrno == EPIPE || sockerrno == ECONNRESET)
        the_next_event = closed_event;
    /*  Anything else, that's an error  */
    else
        the_next_event = error_event;
}


/************************   PROCESS SOCKET EXCEPTION   ***********************/

MODULE process_socket_exception (THREAD *thread)
{
  /*  This function needs replacing in order for socket exception handling
      to be done properly.  */
  /*  PH: what the heck does this mean?  */

    sock_t
        active_handle;
    SOCKREQ
        *request;

    prepare_to_cancel_all_requests ();

    /*  Now reply exception and destroy each request.  */
    active_handle = active_request->handle;
    request = active_request;
    while (((NODE *) request != &main_list)
       &&  (request->handle == active_handle))
      {
        reply_error (&request->reply_to,
                     "Socket exception",
                     request->tag);
        request = request->next;
        request_destroy (request->prev);
      }

    /*  And close the socket.  */
    close_socket (active_handle);
}


/*  prepare_to_cancel_all_requests is called following an error condition on */
/*  a socket.  It merges waiting requests on the same socket and clears the  */
/*  socket from the bitset.  All that remains to be done is to reply to the  */
/*  requests and destroy them.                                               */
static void
prepare_to_cancel_all_requests (void)
{
    sock_t
        active_handle;
    SOCKREQ
        *request;

    /*  First backtrack to the first request on this socket.                 */
    active_handle = active_request->handle;
    request = active_request;
    while (((NODE *) request->prev != &main_list)
       &&  (request->prev->handle == active_handle))
        request = request->prev;

    /*  Then merge any waiting requests.                                     */
    if (wait_count
    &&  FD_ISSET ((int) request->handle, fd_waiting))
        merge_waiting_requests_same_socket (&request);

    /*  And set the active request to first request on the socket.  */
    active_request = request;

    /*  And drop the socket from bitsets  */
    FD_CLR ((int) active_handle, &fd_in_check);
    FD_CLR ((int) active_handle, &fd_out_check);
    FD_CLR ((int) active_handle, &fd_err_check);
}


/**********************   REPLY CLOSED TO ALL REQUESTS   *********************/

MODULE reply_closed_to_all_requests (THREAD *thread)
{
    sock_t
        active_handle;
    SOCKREQ
        *request;

    prepare_to_cancel_all_requests ();

    /*  Now reply closed and destroy each request.                           */
    active_handle = active_request->handle;
    request = active_request;
    while (((NODE *) request != &main_list)
       &&  (request->handle == active_handle))
      {
        request = request->next;
        send_closed_reply (request->prev);
      }
}


/**********************   REPLY ERROR TO ALL REQUESTS   **********************/

MODULE reply_error_to_all_requests (THREAD *thread)
{
    sock_t
        active_handle;
    SOCKREQ
        *request;

    prepare_to_cancel_all_requests ();

    /*  Now reply error and destroy each request.  */
    active_handle = active_request->handle;
    request = active_request;
    while (((NODE *) request != &main_list)
       &&  (request->handle == active_handle))
      {
        request = request->next;
        send_error_reply (request->prev);
      }

    /*  And close the socket.  */
    close_socket (active_handle);
}


static void
send_error_reply (SOCKREQ *request)
{
    reply_error (&request->reply_to,
                 (char *) sockmsg (),
                 request->tag);

    request_destroy (request);
}


/****************************   REQUEST SHUTDOWN   ***************************/

MODULE request_shutdown (THREAD *thread)
{
    smt_shutdown ();
}


/**************************   DESTROY ALL REQUESTS   *************************/

MODULE destroy_all_requests (THREAD *thread)
{
    while (main_list.next != &main_list)
        request_destroy (main_list.next);

    while (wait_list.next != &wait_list)
      {
        request_destroy (wait_list.next);
        wait_count--;
      }
}


/*************************   TERMINATE THE THREAD   **************************/

MODULE terminate_the_thread (THREAD *thread)
{
    if (fd_waiting)
        mem_free (fd_waiting);

    the_next_event = terminate_event;
    deschedule_polling_function (check_activity_nonblock, NULL);
}
