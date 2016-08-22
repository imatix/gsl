/*===========================================================================*
 *                                                                           *
 *  smttst2.c - Test timeslot agent                                          *
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

#define AGENT_NAME      "smttst2"       /*  Our public name                  */
#define SINGLE_THREADED TRUE


/*- Function prototypes -----------------------------------------------------*/

static char *time_str         (void);


/*- Global variables used in this source file only --------------------------*/

static QID
    slotq;                              /*  Operator console event queue     */


#include "smttst2.d"                    /*  Include dialog data              */

/********************   INITIALISE AGENT - ENTRY POINT   *********************/

int smttst2_init (void)
{
    AGENT   *agent;                     /*  Handle for our agent             */
    THREAD  *thread;                    /*  Handle to console thread         */
#   include "smttst2.i"                 /*  Include dialog interpreter       */

    /*  Shutdown event comes from Kernel                                     */
    declare_smtlib_shutdown   (shutdown_event, SMT_PRIORITY_MAX);

    declare_smtslot_switch_on  (switch_on_event,   0);
    declare_smtslot_switch_off (switch_off_event,  0);

    /*  Create initial thread - all threads are unnamed                      */
    thread_create (AGENT_NAME, "");

    /*  Ensure that slot agent is running, else start it up                  */
    smtslot_init ();
    thread = thread_create (SMT_SLOT, "");
    slotq  = thread-> queue-> qid;

    /*  Signal okay to caller that we initialised okay                       */
    return (0);
}


/*************************   INITIALISE THE THREAD   *************************/

MODULE initialise_the_thread (THREAD *thread)
{
    the_next_event = ok_event;
}


/***********************   SEND TIME SLOT SPECIFIERS   ***********************/

MODULE send_time_slot_specifiers (THREAD *thread)
{
    send_smtslot_specify (
        &slotq,
        "Tue 22:10-23");

    send_smtslot_specify (
        &slotq,
        "Mon 6:50-9 12:00-15");

    send_smtslot_specify (
        &slotq,
        "Thu 12:30-20:00");
}


/*************************   SEND TIME SWITCHED OFF   ************************/

MODULE send_time_switched_off (THREAD *thread)
{
    send_smtslot_off (&slotq);
}


/***************************   SIGNAL SWITCHED ON   **************************/

MODULE signal_switched_on (THREAD *thread)
{
    printf ("%s: +++ ON\n", time_str ());
}


/*  -------------------------------------------------------------------------
 *  time_str
 *
 *  Returns the current date and time formatted as: "yy/mm/dd hh:mm:ss".
 *  The formatted time is in a static string that each call overwrites.
 */

static char *
time_str (void)
{
    static char
        formatted_time [18];
    time_t
        time_secs;
    struct tm
        *time_struct;

    time_secs   = time (NULL);
    time_struct = localtime (&time_secs);

    sprintf (formatted_time, "%2d/%02d/%02d %2d:%02d:%02d",
                              time_struct-> tm_year % 100,
                              time_struct-> tm_mon + 1,
                              time_struct-> tm_mday,
                              time_struct-> tm_hour,
                              time_struct-> tm_min,
                              time_struct-> tm_sec);
    return (formatted_time);
}


/**************************   SIGNAL SWITCHED OFF   **************************/

MODULE signal_switched_off (THREAD *thread)
{
    printf ("%s: --- OFF\n", time_str ());
}


/*************************   TERMINATE THE THREAD   **************************/

MODULE terminate_the_thread (THREAD *thread)
{
    the_next_event = terminate_event;
}
