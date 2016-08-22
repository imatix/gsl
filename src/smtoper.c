/*===========================================================================*
 *                                                                           *
 *  smtoper.c - Operator console agent                                       *
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

#define AGENT_NAME      SMT_OPERATOR    /*  Our public name                  */
#define SINGLE_THREADED TRUE            /*  Single-threaded agent            */


/*- Global variables used in this source file only --------------------------*/

static QID
    logq;                               /*  Logging agent event queue        */

static Bool
    console_enabled = TRUE;             /*  Basic console on/off switch      */


#include "smtoper.d"                    /*  Include dialog data              */

/********************   INITIALISE AGENT - ENTRY POINT   *********************/

/*  ---------------------------------------------------------------------[<]-
    Function: smtoper_init

    Synopsis: Initialises the SMT operator agent. Returns 0 if initialised
    okay, -1 if there was an error.  The operator agent writes messages to
    the standard error device.  More sophisticated implementations could
    send messages to consoles.  Creates one unnamed thread automatically.
    Supports these public methods:
    <Table>
    ERROR       Handle event as a serious error message.
    WARNING     Handle event as a warning message.
    INFO        Handle event as an information message.
    SET_LOG     Send all output to specified thread.
    DISABLE     Console ignores all messages.
    ENABLE      Console handles messages (default).
    </Table>
    Does not send reply events.
    ---------------------------------------------------------------------[>]-*/

int
smtoper_init (void)
{
    AGENT   *agent;                     /*  Handle for our agent             */
    THREAD  *thread;                    /*  Handle for initial thread        */

#   include "smtoper.i"                 /*  Include dialog interpreter       */

    /*  We give this agent a high priority, so that we get to see messages   */
    /*  and errors as soon as possible.                                      */
    agent-> priority = SMT_PRIORITY_HIGH;

    /*  Shutdown event comes from Kernel, normal priority so we can show     */
    /*  incoming messages before we shut down.                               */
    declare_smtlib_shutdown   (shutdown_event, 0);

    /*  Public methods supported by this agent                               */
    declare_smtoper_set_log (log_event,     0);
    declare_smtoper_error   (error_event,   0);
    declare_smtoper_warning (warning_event, 0);
    declare_smtoper_info    (info_event,    0);
    declare_smtoper_disable (disable_event, 0);
    declare_smtoper_enable  (enable_event,  0);


    /*  Create initial, unnamed thread                                       */
    thread = thread_create (AGENT_NAME, "");
    smt_set_console (&thread-> queue-> qid);

    /*  Tell sflcons.c that we want date/time prefix on output               */
    console_set_mode (CONSOLE_DATETIME);

    /*  Signal okay to caller that we initialised okay                       */
    return (0);
}


/*************************   INITIALISE THE THREAD   *************************/

MODULE initialise_the_thread (THREAD *thread)
{
    the_next_event = ok_event;
}


/************************   USE SPECIFIED LOG QUEUE   ************************/

MODULE use_specified_log_queue (THREAD *thread)
{
    THREAD
        *log_thread;
    struct_smtoper_set_log
        *request;

    /*  Look for logging thread with specified name; if found, take that
        thread's queue and save it.  All messages will now go there...       */

    get_smtoper_set_log (thread-> event-> body, &request);
    log_thread = thread_lookup (request-> agent_name, request-> thread_name);
    if (log_thread)
        logq = log_thread-> queue-> qid;

    free_smtoper_set_log (&request);
}


/**************************   SIGNAL ERROR MESSAGE   *************************/

MODULE signal_error_message (THREAD *thread)
{
    if (console_enabled)
      { 
        struct_smtoper_message
            *message;

        get_smtoper_message (thread-> event-> body, &message);
        send_smtlog_put (&logq, message-> text);

        coputs (message-> text);
        free_smtoper_message (&message);
      }
}


/**************************   SIGNAL INFO MESSAGE   **************************/

MODULE signal_info_message (THREAD *thread)
{
    signal_error_message (thread);
}


/*************************   SIGNAL WARNING MESSAGE   ************************/

MODULE signal_warning_message (THREAD *thread)
{
    signal_error_message (thread);
}


/************************   DISABLE CONSOLE MESSAGES   ***********************/

MODULE disable_console_messages (THREAD *thread)
{
    console_enabled = FALSE;            /*  Ignore all messages              */
}


/************************   ENABLE CONSOLE MESSAGES   ************************/

MODULE enable_console_messages (THREAD *thread)
{
    console_enabled = TRUE;             /*  Handle messages                  */
}


/**********************   SIGNAL SHUTDOWN IN PROGRESS   **********************/

MODULE signal_shutdown_in_progress (THREAD *thread)
{
    struct_smtlib_shutdown
        *request;
    char
        *message;

    get_smtlib_shutdown (thread-> event-> body, &request);
    switch (request-> signal)
      {
        case SMT_SIGNAL_INT:  message = "-- interrupted";        break;
        case SMT_SIGNAL_TERM: message = "-- terminated";         break;
        case SMT_SIGNAL_SEGV: message = "-- segment violation";  break;
        case SMT_SIGNAL_USER: message = NULL;                    break;
        default:              message = "-- unknown signal";     break;
      }
    if (message)
        send_smtlog_put (&logq, message);

    free_smtlib_shutdown (&request);
}


/*************************   TERMINATE THE THREAD   **************************/

MODULE terminate_the_thread (THREAD *thread)
{
    the_next_event = terminate_event;
}
