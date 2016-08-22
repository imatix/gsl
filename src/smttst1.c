/*===========================================================================*
 *                                                                           *
 *  smttst1.c - Test socket agent                                            *
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

#define AGENT_NAME   "smttst1"          /*  Our public name                  */

typedef struct                          /*  Thread context block:            */
{
    qbyte delay;                        /*    Delay for this thread          */
} TCB;


/*- Function prototypes -----------------------------------------------------*/


/*- Global variables used in this source file only --------------------------*/

static TCB
    *tcb;                               /*  Address thread context block     */
static QID
    console;                            /*  Operator console event queue     */


#include "smttst1.d"                    /*  Include dialog data              */

/********************   INITIALISE AGENT - ENTRY POINT   *********************/

int smttst1_init (void)
{
    AGENT   *agent;                     /*  Handle for our agent             */
    THREAD  *thread;                    /*  Handle to console thread         */
#   include "smttst1.i"                 /*  Include dialog interpreter       */

    /*  Shutdown event comes from Kernel                                     */
    declare_smtlib_shutdown   (shutdown_event, SMT_PRIORITY_MAX);

    /*  Alarm event sent by kernel to this agent                             */
    declare_smttime_reply     (alarm_event,    0);
    declare_smttime_error     (error_event,    0);

    /*  Create initial thread - all threads are unnamed                      */
    thread_create (AGENT_NAME, "");

    /*  Ensure that operator console is running, else start it up            */
    smtoper_init ();
    if ((thread = thread_lookup (SMT_OPERATOR, "")) != NULL)
        console = thread-> queue-> qid;
    else
        return (-1);

    /*  Ensure that timer is running, else start it up                       */
    if (smttime_init ())
        return (-1);

    /*  Signal okay to caller that we initialised okay                       */
    return (0);
}


/*************************   INITIALISE THE THREAD   *************************/

MODULE initialise_the_thread (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */
    the_next_event = ok_event;
}


/***************************   SET CLOCK TICKING   ***************************/

MODULE set_clock_ticking (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    /*  Request clock messages with 1 second delay                           */
    tcb-> delay = 100;                  /*  Measured in centiseconds         */

    smttime_request_clock (
        (qbyte) 0,
        tcb-> delay,
        0,
        0);
}


/*************************   TERMINATE THE THREAD   **************************/

MODULE terminate_the_thread (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */
    the_next_event = terminate_event;
}
