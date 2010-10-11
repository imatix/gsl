/*===========================================================================*
 *                                                                           *
 *  smttime.c - Timer pseudo-agent                                           *
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

#ifdef AGENT_NAME
#  undef AGENT_NAME
#endif
#define AGENT_NAME      SMT_TIMER       /*  Our public name                  */
#define SINGLE_THREADED TRUE            /*  Single-threaded agent            */

typedef struct _TIMEREQ {               /*  Request descriptor               */
    struct _TIMEREQ                     /*                                   */
            *next, *prev;               /*    Doubly-linked list             */
    QID     reply_to;                   /*    Who sent the request           */
    qbyte   days;                       /*    Delay in days                  */
    qbyte   csecs;                      /*    Delay in centiseconds          */
    word    cycles;                     /*    How many cycles left           */
    long    date;                       /*    Date and time that the request */
    long    time;                       /*       is due to happen.           */
    void   *tag;                        /*    User-defined request tag       */
} TIMEREQ;


/*- Function prototypes -----------------------------------------------------*/

static void     cancel_or_reschedule_request
                                         (TIMEREQ *request);
static int      sleep_until_time         (long date, long time,
                                          void *dummy);
static int      generate_response_event  (long date, long time,
                                          void *request);
static TIMEREQ *absolute_request_create  (QID *reply_to,
                                          long date, long time,
                                          void *tag);
static TIMEREQ *relative_request_create  (QID *reply_to,
                                          qbyte days, qbyte csecs, word cycles,
                                          void *tag);
static int      put_smttime_tag          (byte **_buffer,
                                          void *tag);
static int      lsend_smttime_reply      (QID *_to, QID *_from,
                                          char *_accept,
                                          char *_reject,
                                          char *_expire,
                                          word _timeout,
                                          void *tag);
static int      put_smttime_error        (byte **_buffer,
                                          char *message);
static int      lsend_smttime_error      (QID *_to, QID *_from,
                                          char *_accept,
                                          char *_reject,
                                          char *_expire,
                                          word _timeout,
                                          char *message);

#if (defined (__WINDOWS__) && !defined (CONSOLE))
VOID CALLBACK handle_timer (HWND hwnd, UINT uMsg, UINT idEvent, DWORD dwTime);
#endif

/*- Global variables used in this source file only --------------------------*/

static LIST
    requests = {0,0};                   /*  Request list header              */

static long
    cur_date = 0,
    cur_time = 0;

/*  ---------------------------------------------------------------------[<]-
    Function: smttime_init

    Synopsis: Initialises the SMT timer agent.  Returns 0 if initialised
    okay, -1 if there was an error.  The timer agent provides timing events
    after a certain delay, at a specific time, or at a specific frequency.
    When you initialise the timer agent it creates an unnamed thread
    automatically.  Send events to this thread.  The timer accuracy is
    1/100th of a second, depending on the system capacity and speed.
    Supports these public methods:
    <Table>
    ALARM       Send an alarm after some delay (use SMT_TIME_ALARM).
    WAKEUP      Send an alarm at some time (use SMT_TIME_ALARM).
    CLOCK       Send an alarm at some frequency (use SMT_TIME_CLOCK).
    FLUSH       Cancel all timing events for a client thread.
    CLEAR       Cancel all timing events matching a request tag for a
                client thread.
    </Table>
    ---------------------------------------------------------------------[>]-*/

int
smttime_init (void)
{
    if (requests.next == 0)            /*  We haven't been started yet.     */
      {
        /*  Initialise requests list                                         */
        list_reset (&requests);

        register_async_blocking (sleep_until_time, NULL);

        smt_atexit (smttime_term);
      }

    /*  Signal okay to caller that we initialised okay                       */
    return (0);
}


static void
cancel_or_reschedule_request (TIMEREQ *request)
{
    if (request->cycles == 1)                /*  Expire if single-shot      */
      {
        list_unlink (request);
        mem_free (request);
      }

    /*  Else set next expiry time to closest future point...  under
     *  normal circumstances we add the delay date and time.  If the
     *  delay is very short (too short), or the clock was moved
     *  forwards, this may not be enough.  We then take the current
     *  date and time and add the delay to that.
     */
    else
      {
        if (request->cycles > 1)
            request->cycles--;

        future_date (&request->date, &request->time,
                     request->days, request->csecs);

        if (request->date <  cur_date
        || (request->date == cur_date
        &&  request->time <  cur_time))
          {
            request->date = cur_date;
            request->time = cur_time;
            future_date (&request->date, &request->time,
                          request->days,  request->csecs);
          }
        schedule_async_nonblock (generate_response_event, request,
                                 SMT_PRIORITY_MAX,
                                 request->date, request->time);
      }
}


static int
sleep_until_time (long date, long time, void *voidreq)
{
    int
        rc = 0;
    long
        days,
        csecs;
    dword
        msecs;
    TIMEREQ
        *loopreq;

    if (date < 99999999)
      {
        get_date_time_now(&cur_date, &cur_time);
        date_diff (date, time,
                   cur_date, cur_time,
                   &days, &csecs);
        csecs++;                        /*  Round up to avoid busy waiting.  */

        if (days > 48)                  /*  Don't wait more than 48 days!    */
            return -1;

        if ((days >= 0) && (csecs >= 0))   /*  wait_date/time is in future.  */
          {
            msecs = csecs * 10 + days * 86400000;
            process_sleep (msecs);
          }

        get_date_time_now (&cur_date, &cur_time);
        FORLIST (loopreq, requests)
          {
            if (loopreq->date <  cur_date
            || (loopreq->date == cur_date
            &&  loopreq->time <  cur_time))
              {
                lsend_smttime_reply (&loopreq->reply_to,
                                     NULL,
                                     NULL, NULL, NULL, 0,
                                     loopreq->tag);
                cancel_or_reschedule_request (loopreq);
                rc = 1;
              }
          }
      }
    return rc;
}


static int
generate_response_event (long date, long time, void *voidreq)
{
    long
        delay_days,                     /*  Which is in how many days        */
        delay_csecs;                    /*    and how many centiseconds      */
    TIMEREQ
        *request = (TIMEREQ *) voidreq,
        *loopreq;

    /*  Handle a backwards clock adjustment by estimating the difference     */
    /*  and applying this to all time requests.                             */
    /*  JS: Why/when do this at all? */
    smt_set_step ("handle clock adjustment");
    if (date_is_future (cur_date, cur_time))
      {
        long
            clock_date,
            clock_time;

        get_date_time_now (&clock_date, &clock_time);

        date_diff (cur_date,    cur_time,
                   clock_date,  clock_time,
                   &delay_days, &delay_csecs);
        FORLIST (loopreq, requests)
            past_date (&loopreq->date, &loopreq->time,
                       delay_days, delay_csecs);
      }

    lsend_smttime_reply (&request->reply_to, /*  Send to specified queue    */
                         NULL,                /*  No reply queue             */
                         NULL, NULL, NULL, 0, /*  No accept, reject or expire*/
                         request->tag);

    cancel_or_reschedule_request (request);

    return 1;                           /*  Because we delivered an event.   */
}


int
create_single_alarm_request (
    QID  *reply_to,                     /*  Queue to receive reply           */
    qbyte days,                         /*  Time/delay in days               */
    qbyte csecs,                        /*  Time/delay in 1/100th seconds    */
    void *tag)                          /*  User-defined request tag         */
{
    TIMEREQ
        *request;

    request = relative_request_create (reply_to, days, csecs, 1, tag);
    if (request)
      {
        schedule_async_nonblock (generate_response_event, request,
                                 SMT_PRIORITY_MAX,
                                 request->date, request->time);
        return 0;
      }
    else
        return -1;
}


/*  -------------------------------------------------------------------------
 *  flush_requests_for_client
 *
 *  Deletes all timer requests originating from a given queue.
 */

void
flush_requests_for_client (QID *clientq)
{
    TIMEREQ
        *request,                       /*  Pointer to request in list       */
        *reqprev;

    /*  Destroy any requests originally created by the sending thread        */
    for (request  = (TIMEREQ *) requests.next;
         request != (TIMEREQ *) &requests;
         request  = (TIMEREQ *) request->next)
      {
        if (memcmp (&request->reply_to, clientq, sizeof (QID)) == 0)
          {
            deschedule_async_nonblock (generate_response_event, request);
            reqprev = request->prev;
            list_unlink (request);
            mem_free (request);
            request = reqprev;
          }
      }
}


/*  -------------------------------------------------------------------------
 *  relative_request_create
 *
 *  Creates a new request, and initialises it to empty.  If the request
 *  could not be created, returns null.  Otherwise returns the address of
 *  the created request.  Sets the expiry time as specified by the delay.
 *  Attaches the user-defined body to the request; this is returned with the
 *  alarm event.
 */

static TIMEREQ *
relative_request_create (
    QID   *reply_to,                    /*  Who sent this timing request     */
    qbyte  days,                        /*  Days to delay                    */
    qbyte  csecs,                       /*  Csecs to delay                   */
    word   cycles,                      /*  Number of alarm cycles           */
    void  *tag)                         /*  Request tag                      */
{
    TIMEREQ
        *request;                       /*  Request we create                */
    long
        current_date,
        current_time;

    get_date_time_now (&current_date, &current_time);

    if ((request = absolute_request_create (reply_to, 
                                            current_date, current_time,
                                            tag)) != NULL)
      {
        /*  A zero time is treated as the smallest possible unit             */
        if (days == 0 && csecs == 0)
            csecs = 1;

        /*  Fill-in request fields                                           */
        request->days   = days;
        request->csecs  = csecs;
        request->cycles = cycles;

        /*  Set request expiry date and time                                 */
        future_date (&request->date, &request->time,
                      request->days,  request->csecs);
      }
    return (request);
}


/*  -------------------------------------------------------------------------
 *  absolute_request_create
 *
 *  Creates a new request, and initialises it to empty.  If the request
 *  could not be created, sends a TIME_ERROR to the caller, and returns
 *  null.  Otherwise returns the address of the created request.  Sets
 *  the expiry time as specified by the delay.  Attaches the user-defined
 *  body to the request; this is returned with the alarm event.
 */

static TIMEREQ *
absolute_request_create (QID *reply_to,
                         long date, long time,
                         void *tag)
{
    TIMEREQ
        *request;                       /*  Request we create                */

    list_create (request, sizeof (TIMEREQ));
    if (! request)
        lsend_smttime_error (
            reply_to,
            NULL,                       /*  No reply queue                   */
            NULL, NULL, NULL, 0,        /*  No accept, reject or expire      */
            "Out of memory");
    else
      {
        list_relink_before (request, &requests);

        /*  Fill-in request fields                                           */
        request->reply_to  = *reply_to;
        request->tag       = tag;
        request->cycles    = 1;
        request->date  = date;
        request->time  = time;
      }

    return (request);
}


int
create_cycled_clock_request (
    QID  *reply_to,                     /*  Queue to receive reply           */
    qbyte days,                         /*  Time/delay in days               */
    qbyte csecs,                        /*  Time/delay in 1/100th seconds    */
    word  cycles,                       /*  Cycle count; zero = forever      */
    void *tag)                          /*  User-defined request tag         */
{
    TIMEREQ
        *request;

    request = relative_request_create (reply_to, days, csecs, cycles, tag);
    if (request)
      {
        schedule_async_nonblock (generate_response_event, request,
                                 SMT_PRIORITY_MAX,
                                 request->date, request->time);
        return 0;
      }
    else
        return -1;
}


int
create_single_wakeup_request (
    QID  *reply_to,                     /*  Queue to receive reply           */
    long  date,                         /*  Date of request                  */
    long  time,                         /*  Time of request                  */
    void *tag)                          /*  User-defined request tag         */
{
    TIMEREQ
        *request;

    request = absolute_request_create (reply_to, date, time, tag);
    if (request)
      {
        schedule_async_nonblock (generate_response_event, request,
                                 SMT_PRIORITY_MAX,
                                 request->date, request->time);
        return 0;
      }
    else
        return -1;
}


void
clear_requests_matching_tag (
    QID  *reply_to,                     /*  Queue to receive reply           */
    void *tag)                          /*  User-defined request tag         */
{
    TIMEREQ
        *request,                       /*  Pointer to request in list       */
        *reqprev;

    /*  Destroy any requests originally created by the sending thread        */
    for (request  = (TIMEREQ *) requests.next;
         request != (TIMEREQ *) &requests;
         request  = (TIMEREQ *) request->next)
      {
        if ((memcmp (&request->reply_to, reply_to, sizeof (QID)) == 0)
        && (request->tag == tag))
          {
            deschedule_async_nonblock (generate_response_event, request);
            reqprev = request->prev;
            list_unlink (request);
            mem_free (request);
            request = reqprev;
          }
      }
}


void
smttime_term (void)
{
    list_destroy (&requests);
}


/*---------------------------------------------------------------------------
 *  Message functions for smttime - Timer agent.
 *---------------------------------------------------------------------------*/

#define SMTTIME_TAG_FORMAT "q"

/*  ---------------------------------------------------------------------[<]-
    Function: put_smttime_tag                        

    Synopsis: Formats a tag message, allocates a new buffer,
    and returns the formatted message in the buffer.  You should free the
    buffer using mem_free() when finished.  Returns the size of the buffer
    in bytes.
    ---------------------------------------------------------------------[>]-*/

static int
put_smttime_tag (
    byte **_buffer,
    void *tag)                          /*  User-defined request tag         */
{
    int _size;

    _size = exdr_write (NULL,
                        SMTTIME_TAG_FORMAT,
                        tag);
    *_buffer = mem_alloc (_size);
    if (*_buffer)
        exdr_write (*_buffer,
                    SMTTIME_TAG_FORMAT,
                    tag);
    else
        _size = 0;
    return (_size);
}


/*  ---------------------------------------------------------------------[<]-
    Function: get_smttime_tag                        

    Synopsis: Accepts a buffer containing a tag message,
    and unpacks it into a new struct_smttime_tag structure. Free the
    structure using free_smttime_tag() when finished.
    ---------------------------------------------------------------------[>]-*/

int
get_smttime_tag (
    byte *_buffer,
    struct_smttime_tag **params)
{
    *params = mem_alloc (sizeof (struct_smttime_tag));
    if (*params)
      {
        return (exdr_read (_buffer, SMTTIME_TAG_FORMAT,
                   &(*params)->tag));
      }
    else
        return -1;
}


/*  ---------------------------------------------------------------------[<]-
    Function: free_smttime_tag                        

    Synopsis: frees a structure allocated by get_smttime_tag().
    ---------------------------------------------------------------------[>]-*/

void
free_smttime_tag (
    struct_smttime_tag **params)
{
    mem_free (*params);
    *params = NULL;
}

char *SMTTIME_REPLY = "SMTTIME REPLY";

/*  ---------------------------------------------------------------------[<]-
    Function: lsend_smttime_reply              

    Synopsis: Sends a reply - 
              event to the smttime agent
    ---------------------------------------------------------------------[>]-*/

int
lsend_smttime_reply (QID *_to, QID *_from,
    char *_accept,
    char *_reject,
    char *_expire,
    word _timeout,
    void *tag)                          /*  User-defined request tag         */
{
    byte *_body;
    int   _size,
          _rc;
    _size = put_smttime_tag
                (&_body,
                 tag);
    if (_size)
      {
        _rc = event_send (_to, _from, SMTTIME_REPLY,
                          _body, _size,
                          _accept, _reject, _expire, _timeout);
        mem_free (_body);
        return _rc;
      }
    else
        return -1;
}

#define SMTTIME_ERROR_FORMAT "s"

/*  ---------------------------------------------------------------------[<]-
    Function: put_smttime_error                      

    Synopsis: Formats a error message, allocates a new buffer,
    and returns the formatted message in the buffer.  You should free the
    buffer using mem_free() when finished.  Returns the size of the buffer
    in bytes.
    ---------------------------------------------------------------------[>]-*/

static int
put_smttime_error (
    byte **_buffer,
    char *message)                      /*  Error message                    */
{
    int _size;

    _size = exdr_write (NULL,
                        SMTTIME_ERROR_FORMAT,
                        message);
    *_buffer = mem_alloc (_size);
    if (*_buffer)
        exdr_write (*_buffer,
                    SMTTIME_ERROR_FORMAT,
                    message);
    else
        _size = 0;
    return (_size);
}


/*  ---------------------------------------------------------------------[<]-
    Function: get_smttime_error                      

    Synopsis: Accepts a buffer containing a error message,
    and unpacks it into a new struct_smttime_error structure. Free the
    structure using free_smttime_error() when finished.
    ---------------------------------------------------------------------[>]-*/

int
get_smttime_error (
    byte *_buffer,
    struct_smttime_error **params)
{
    *params = mem_alloc (sizeof (struct_smttime_error));
    if (*params)
      {
        (*params)->message = NULL;
        return (exdr_read (_buffer, SMTTIME_ERROR_FORMAT,
                   &(*params)->message));
      }
    else
        return -1;
}


/*  ---------------------------------------------------------------------[<]-
    Function: free_smttime_error                      

    Synopsis: frees a structure allocated by get_smttime_error().
    ---------------------------------------------------------------------[>]-*/

void
free_smttime_error (
    struct_smttime_error **params)
{
    mem_free ((*params)->message);
    mem_free (*params);
    *params = NULL;
}

char *SMTTIME_ERROR = "SMTTIME ERROR";

/*  ---------------------------------------------------------------------[<]-
    Function: lsend_smttime_error              

    Synopsis: Sends a error - 
              event to the smttime agent
    ---------------------------------------------------------------------[>]-*/

static int
lsend_smttime_error (QID *_to, QID *_from,
    char *_accept,
    char *_reject,
    char *_expire,
    word _timeout,
    char *message)                      /*  Error message                    */
{
    byte *_body;
    int   _size,
          _rc;
    _size = put_smttime_error
                (&_body,
                 message);
    if (_size)
      {
        _rc = event_send (_to, _from, SMTTIME_ERROR,
                          _body, _size,
                          _accept, _reject, _expire, _timeout);
        mem_free (_body);
        return _rc;
      }
    else
        return -1;
}

