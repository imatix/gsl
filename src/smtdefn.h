/*===========================================================================*
 *                                                                           *
 *  smtdefn.h - SMT agents and definitions                                   *
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

#ifndef _SMTDEFN_INCLUDED               /*  Allow multiple inclusions        */
#define _SMTDEFN_INCLUDED


/*---------------------------------------------------------------------------
 *  Standard SMT agents
 */

/*  Prototypes                                                               */

#ifdef __cplusplus
extern "C" {
#endif

int  smtecho_init    (void);            /*  TCP/IP echo agent                */
int  smtlog_init     (void);            /*  Logging agent                    */
int  smtoper_init    (void);            /*  Operator agent                   */
int  smtslot_init    (void);            /*  Time slot agent                  */
int  smtsock_init    (void);            /*  Socket i/o agent                 */
void smtsock_trace   (Bool trace);      /*  Socket i/o agent trace on/off    */
void smtsock_process_asynchronously
                     (void);
Bool smtsock_wait_forever
                     (void);
void smttime_process_asynchronously
                     (void);
int  smttran_init    (void);            /*  TCP/IP transfer agent            */
int  smtrdns_init    (void);            /*  Reverse DNS agent                */
int  smtpipe_init    (char *pipefile);  /*  Transfer pipe agent              */
int  smtsmtp_init    (void);            /*  SMTP agent                       */
int  smtpop_init     (void);            /*  POP3 agent                       */


#ifdef __cplusplus
}
#endif


/*---------------------------------------------------------------------------
 *  SMTLOG - Logging Agent
 *      One thread per log file; create thread with log file name, or supply
 *      log file name in CYCLE, OPEN, APPEND events.  Filename "" or "NULL"
 *      means discard all log file output.  Errors are sent to SMTOPER.
 *
 *  Method:                         Body:           Replies:
 *  CYCLE    Cycle log file         [file name]     -
 *  OPEN     Open new log file      [file name]     -
 *  APPEND   Append to existing     [file name]     -
 *  PUT      Write line to log      Line of text    -
 *  PLAIN    Do not timestamp log   -               -
 *  STAMP    Timestamp each line    -               -
 *  CLOSE    Close log file         -               -
 */

#define SMT_LOGGING     "smtlog"        /*  Name of logging agent            */


/*---------------------------------------------------------------------------
 *  SMTOPER - Operator Console Agent
 *      Single unnamed thread created automatically when agent initialises.
 *      Send messages to console device or log file if specified.
 *
 *  Method:                         Body:             Replies:
 *  ERROR    Error message          Message           -
 *  WARNING  Warning message        Message           -
 *  INFO     Information message    Message           -
 *  LOG      Redirect to log file   SMTLOG thread name  -
 */

#define SMT_OPERATOR    "smtoper"       /*  Name of operator agent           */


/*---------------------------------------------------------------------------
 *  SMTSLOT - Time Slot Agent
 *      One thread per application; create thread with application name.
 *      Send SPECIFY events, then ON or OFF to set timer initial state.
 *      Specification is "name values..."; name is "mon" to "sun", or date
 *      in MM/DD, YY/MM/DD, or YYYY/MM/DD format.  Values are hh:mm[-hh:mm]
 *      in 24-hour clock format, delimited by spaces.
 *
 *  Method:                         Body:             Replies:
 *  SPECIFY  Specify time slot      Specification     SLOT_ERROR, if error
 *  ON       Error message          -                 SWITCH_ON, SWITCH_OFF
 *  OFF      Error message          -                 SWITCH_ON, SWITCH_OFF
 */

#define SMT_SLOT        "smtslot"       /*  Name of time slot agent          */


/*---------------------------------------------------------------------------
 *  SMTSOCK - Socket I/O Agent
 *      Single unnamed thread created automatically when agent initialises.
 *      Multiple writes are processed in order; multiple reads are collapsed.
 *
 *  Method:                         Body:             Replies:
 *  READ     Read socket data       SMT_SOCK_READ     SOCK_READ_OK, ...
 *  READR    Read, repeat for ever  SMT_SOCK_READ     SOCK_READ_OK, ...
 *  WRITE    Write socket data      SMT_SOCK_WRITE    SOCK_WRITE_OK, ...
 *  INPUT    Wait for input         SMT_SOCK_INPUT    SOCK_INPUT_OK, ...
 *  INPUTR   Wait input, repeat     SMT_SOCK_INPUT    SOCK_INPUT_OK, ...
 *  OUTPUT   Wait for output        SMT_SOCK_OUTPUT   SOCK_OUTPUT_OK, ...
 *  CONNECT  Connect to host        SMT_SOCK_CONNECT  SOCK_WRITE_OK, ...
 *  FLUSH    Flush requests         SMT_SOCK_FLUSH    -
 *
 *  Replies:                                    Body:
 *  SOCK_READ_OK    Data read okay              SMT_SOCK_READ_OK
 *  SOCK_WRITE_OK   Data written okay           SMT_SOCK_OK
 *  SOCK_INPUT_OK   Socket has input ready      SMT_SOCK_OK
 *  SOCK_OUTPUT_OK  Socket ready for output     SMT_SOCK_OK
 *  SOCK_CLOSED     Socket was closed           SMT_SOCK_READ_OK
 *  SOCK_TIMEOUT    Request timed-out           SMT_SOCK_READ_OK
 *  SOCK_ERROR      Socket error during read    SMT_SOCK_ERROR
 */

#define SMT_SOCKET      "smtsock"       /*  Name of socket i/o agent         */


/*---------------------------------------------------------------------------
 *  SMTTIME - Timer Agent
 *      Single unnamed thread created automatically when agent initialises.
 *      Accurate to 1/100th second.
 *
 *  Method:                         Body:             Replies:
 *  ALARM    Alarm after delay      SMT_TIME_ALARM    TIME_ALARM, TIME_ERROR
 *  WAKEUP   Alarm at some time     SMT_TIME_ALARM    TIME_ALARM, TIME_ERROR
 *  CLOCK    Alarm at intervals     SMT_TIME_CLOCK    TIME_ALARM, TIME_ERROR
 *
 *  Replies:                                    Body:
 *  TIME_ALARM      Alarm went off              SMT_TIME_REPLY
 *  TIME_ERROR      Insufficient memory         Error message
 */

#define SMT_TIMER       "smttime"       /*  Name of timer agent              */


/*---------------------------------------------------------------------------
 *  SMTTRAN - Transfer Agent
 *      Single unnamed thread created automatically when agent initialises.
 *      Transfers fixed-size blocks or complete files.
 *
 *  Method:                         Body:             Replies:
 *  PUT_BLOCK   Write block         SMT_TRAN_PUTB     TRAN_PUTB_OK, ...
 *  GET_BLOCK   Read block          SMT_TRAN_GETB     TRAN_GETB_OK, ...
 *  PUT_FILE    Write file          SMT_TRAN_PUTF     TRAN_PUTF_OK, ...
 *  GET_FILE    Read file           SMT_TRAN_GETF     TRAN_GETF_OK, ...
 *  PUT_TEXT    Write text file     SMT_TRAN_PUTT     TRAN_PUTF_OK, ...
 *  GET_TEXT    Read text file      SMT_TRAN_GETT     TRAN_GETF_OK, ...
 *  PUT_SLICE   Write file slice    SMT_TRAN_PUTS     TRAN_PUTF_OK, ...
 *  GET_SLICE   Read file slice     SMT_TRAN_GETS     TRAN_GETF_OK, ...
 *  COMMIT      Finish transfers    -                 -
 */

#define SMT_TRANSFER    "smttran"       /*  Name of transfer agent           */


/*---------------------------------------------------------------------------
 *  SMTRDNS - Reverse DNS Agent
 *      One thread per request.  A main thread creates child threads with
 *      the type of request.
 *
 *  Method:                         Body:             Replies:
 *  GET_HOST Get host name          SMT_GET_HOST_NAME RDNS_OK,...
 *
 *  Replies:
 *  HOST_NAME  Host name
 *  HOST_IP    Host IP address
 */

#define SMT_RDNS        "smtrdns"       /*  Name of reverse dns agent        */


/*---------------------------------------------------------------------------
 *  SMTPOP - POP 3 agent
 *      One thread per request.  A main thread creates child threads to handle
 *      _one_ connection.
 *      
 *  Method:                         Body:             Replies:
 *      XXX TODO
 *  Replies:
 *      XXX TODO
 */

#define SMT_POP        "smtpop"       /*  Name of the POP 3 agent            */
#define SMT_SMTP       "smtsmtp"      /*  Name of the SMTP agent             */


#define SMTXLOG_AGENT  "smtxlog"        /* Name of extended log agent        */
/*  Other Agents                                                             */

#define SMT_ECHO        "smtecho"       /*  Name of TCP/IP echo agent        */
#define SMT_ECHO_PORT   "7"             /*  Port for echo agent              */

#endif                                  /*  Include smtdefn.h                */
