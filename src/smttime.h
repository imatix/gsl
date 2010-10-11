/*===========================================================================*
 *                                                                           *
 *  smttime.h - Timer pseudo-agent                                           *
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
/*  ----------------------------------------------------------------<Prolog>-
    Synopsis:   Defines the functions to communicate with the SMT timer
                agent.  These include the deprecated functions which to
                keep compatibility with the old dialog-based timer agent.
 ------------------------------------------------------------------</Prolog>-*/

#ifndef _SMTTIME_INCLUDED               /*  Allow multiple inclusions        */
#define _SMTTIME_INCLUDED


int  smttime_init (void);
int
create_single_alarm_request (
    QID  *reply_to,                     /*  Queue to receive reply           */
    qbyte days,                         /*  Time/delay in days               */
    qbyte csecs,                        /*  Time/delay in 1/100th seconds    */
    void *tag);                         /*  User-defined request tag         */
void
flush_requests_for_client (QID *clientq);
int
create_cycled_clock_request (
    QID  *reply_to,                     /*  Queue to receive reply           */
    qbyte days,                         /*  Time/delay in days               */
    qbyte csecs,                        /*  Time/delay in 1/100th seconds    */
    word  cycles,                       /*  Cycle count; zero = forever      */
    void *tag);                         /*  User-defined request tag         */
int
create_single_wakeup_request (
    QID  *reply_to,                     /*  Queue to receive reply           */
    long  date,                         /*  Date of request                  */
    long  time,                         /*  Time of request                  */
    void *tag);                         /*  User-defined request tag         */
void
clear_requests_matching_tag (
    QID  *reply_to,                     /*  Queue to receive reply           */
    void *tag);                         /*  User-defined request tag         */
void smttime_term (void);


/*---------------------------------------------------------------------------
 *  Definitions and prototypes for smttime - Timer agent.
 *
 *  The send_....() and lsend_.....() macros are provided for backwards
 *  compatibility, and are deprecated.  Use of the smttime_request_....() 
 *  macros is encouraged in new code. 
 *
 *  NOTE: Use of the send_....() and lsend_....() macros may result in
 *  compiler warnings about unused variables, as the _to parameter to
 *  the macros is unused (but retained for backwards compatibility).
 *
 *---------------------------------------------------------------------------*/

/*  Send event - alarm                                                       */

#define lsend_smttime_alarm(_to, _from,                                      \
        _accept, _reject, _expire, _timeout,                                 \
        days,                                                                \
        csecs,                                                               \
        tag)                                                                 \
    if (! create_single_alarm_request (_from, days, csecs, tag))             \
      {                                                                      \
        if (_accept)                                                         \
            event_send (_from, NULL, _accept, NULL, 0, NULL, NULL, NULL, 0); \
      }                                                                      \
    else                                                                     \
      {                                                                      \
        if (_reject)                                                         \
            event_send (_from, NULL, _reject, NULL, 0, NULL, NULL, NULL, 0); \
      }

#define send_smttime_alarm(_to,                                              \
        days,                                                                \
        csecs,                                                               \
        tag)                                                                 \
    create_single_alarm_request (&thread-> queue-> qid, days, csecs, tag)

#define smttime_request_alarm(                                               \
        days,                                                                \
        csecs,                                                               \
        tag)                                                                 \
    create_single_alarm_request (&thread-> queue-> qid, days, csecs, tag)

#define smttime_request_alarm_with_reply(                                    \
        _reply_to, _accept, _reject, _expire, _timeout,                      \
        days,                                                                \
        csecs,                                                               \
        tag)                                                                 \
    if (! create_single_alarm_request (_reply_to, days, csecs, tag))         \
      {                                                                      \
        if (_accept)                                                         \
            event_send (_reply_to, NULL, _accept,                            \
		        NULL, 0, NULL, NULL, NULL, 0);                       \
      }                                                                      \
    else                                                                     \
      {                                                                      \
        if (_reject)                                                         \
            event_send (_reply_to, NULL, _reject,                            \
		        NULL, 0, NULL, NULL, NULL, 0);                       \
      }

/*  Send event - wakeup                                                      */

#define lsend_smttime_wakeup(_to, _from,                                     \
        _accept, _reject, _expire, _timeout,                                 \
        date,                                                                \
        time,                                                                \
        tag)                                                                 \
    if (! create_single_wakeup_request (_from, date, time, tag))             \
      {                                                                      \
        if (_accept)                                                         \
            event_send (_from, NULL, _accept, NULL, 0, NULL, NULL, NULL, 0); \
      }                                                                      \
    else                                                                     \
      {                                                                      \
        if (_reject)                                                         \
            event_send (_from, NULL, _reject, NULL, 0, NULL, NULL, NULL, 0); \
      }

#define send_smttime_wakeup(_to,                                             \
        date,                                                                \
        time,                                                                \
        tag)                                                                 \
    create_single_wakeup_request (&thread-> queue-> qid, date, time, tag)

#define smttime_request_wakeup(                                              \
        date,                                                                \
        time,                                                                \
        tag)                                                                 \
    create_single_wakeup_request (&thread-> queue-> qid, date, time, tag)

#define smttime_request_wakekup_with_reply(                                  \
        _reply_to, _accept, _reject, _expire, _timeout,                      \
        date,                                                                \
        time,                                                                \
        tag)                                                                 \
    if (! create_single_wakeup_request (_reply_to, date, time, tag))         \
      {                                                                      \
        if (_accept)                                                         \
            event_send (_reply_to, NULL, _accept,                            \
		        NULL, 0, NULL, NULL, NULL, 0);                       \
      }                                                                      \
    else                                                                     \
      {                                                                      \
        if (_reject)                                                         \
            event_send (_reply_to, NULL, _reject,                            \
		        NULL, 0, NULL, NULL, NULL, 0);                       \
      }

/*  Send event - clock                                                       */

#define lsend_smttime_clock(_to, _from,                                      \
        _accept, _reject, _expire, _timeout,                                 \
        days,                                                                \
        csecs,                                                               \
        cycles,                                                              \
        tag)                                                                 \
    if (! create_cycled_clock_request (_from, days, csecs, cycles tag))      \
      {                                                                      \
        if (_accept)                                                         \
            event_send (_from, NULL, _accept, NULL, 0, NULL, NULL, NULL, 0); \
      }                                                                      \
    else                                                                     \
      {                                                                      \
        if (_reject)                                                         \
            event_send (_from, NULL, _reject, NULL, 0, NULL, NULL, NULL, 0); \
      }

#define send_smttime_clock(_to,                                              \
        days,                                                                \
        csecs,                                                               \
        cycles,                                                              \
        tag)                                                                 \
    create_cycled_clock_request (&thread-> queue-> qid,                      \
                                 days, csecs, cycles, tag)

#define smttime_request_clock(                                               \
        days,                                                                \
        csecs,                                                               \
        cycles,                                                              \
        tag)                                                                 \
    create_cycled_clock_request (&thread-> queue-> qid,                      \
                                 days, csecs, cycles, tag)

#define smttime_request_clock_with_reply(                                    \
        _reply_to, _accept, _reject, _expire, _timeout,                      \
        days,                                                                \
        csecs,                                                               \
        cycles,                                                              \
        tag)                                                                 \
    if (! create_cycled_clock_request (_reply_to, days, csecs, cycles tag))  \
      {                                                                      \
        if (_accept)                                                         \
            event_send (_reply_to, NULL, _accept,                            \
		        NULL, 0, NULL, NULL, NULL, 0);                       \
      }                                                                      \
    else                                                                     \
      {                                                                      \
        if (_reject)                                                         \
            event_send (_reply_to, NULL, _reject,                            \
		        NULL, 0, NULL, NULL, NULL, 0);                       \
      }

/*  Send event - flush                                                       */

#define lsend_smttime_flush(_to, _from,                                      \
        _accept, _reject, _expire, _timeout)                                 \
    flush_requests_for_client (_from);                                       \
    if (_accept)                                                             \
        event_send (_from, NULL, _accept, NULL, 0, NULL, NULL, NULL, 0);

#define send_smttime_flush(_to)                                              \
    flush_requests_for_client (&thread-> queue-> qid)

#define smttime_request_flush()                                              \
    flush_requests_for_client (&thread-> queue-> qid)

#define smttime_request_flush_with_reply(                                    \
        _reply_to, _accept, _reject, _expire, _timeout)                      \
    flush_requests_for_client (_reply_to);                                   \
    if (_accept)                                                             \
        event_send (_reply_to, NULL, _accept, NULL, 0, NULL, NULL, NULL, 0);

/*  Send event - clear                                                       */

#define lsend_smttime_clear(_to, _from,                                      \
        _accept, _reject, _expire, _timeout,                                 \
        tag)                                                                 \
    clear_requests_matching_tag (_from, tag);                                \
    if (_accept)                                                             \
        event_send (_from, NULL, _accept, NULL, 0, NULL, NULL, NULL, 0);

#define send_smttime_clear(_to,                                              \
        tag)                                                                 \
    clear_requests_matching_tag (&thread-> queue-> qid,                      \
                                 tag)

#define smttime_request_clear(                                               \
        tag)                                                                 \
    clear_requests_matching_tag (&thread-> queue-> qid, tag)

#define smttime_request_clear_with_reply(                                    \
        _reply_to, _accept, _reject, _expire, _timeout,                      \
        tag)                                                                 \
    clear_requests_matching_tag (_reply_to, tag);                            \
    if (_accept)                                                             \
        event_send (_reply_to, NULL, _accept, NULL, 0, NULL, NULL, NULL, 0);

/*  Structure for reply                                                      */

typedef struct {
    void *tag;                          /*  User-defined request tag         */
} struct_smttime_tag;

int  get_smttime_tag                       (byte *_buffer,
                                            struct_smttime_tag **params);

void free_smttime_tag                      (struct_smttime_tag **params);
extern char *SMTTIME_REPLY;

#define declare_smttime_reply(_event, _priority)                             \
    method_declare (agent, SMTTIME_REPLY, _event, _priority)

typedef struct {
    char *message;                      /*  Error message                    */
} struct_smttime_error;

int  get_smttime_error                     (byte *_buffer,
                                            struct_smttime_error **params);
void free_smttime_error                    (struct_smttime_error **params);

extern char *SMTTIME_ERROR;

#define declare_smttime_error(_event, _priority)                             \
    method_declare (agent, SMTTIME_ERROR, _event, _priority)

#endif                                  /*  Include smttime.h                */
