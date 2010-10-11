/*===========================================================================*
 *                                                                           *
 *  smtlog.c - Logging agent                                                 *
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

#define AGENT_NAME   SMT_LOGGING        /*  Our public name                  */
#define LOG_LINEMAX  4096

typedef struct                          /*  Thread context block:            */
{
    int  handle;                        /*    Handle for i/o                 */
    Bool timestamp;                     /*    By default, we timestamp       */
} TCB;


/*- Function prototypes -----------------------------------------------------*/

static char *logfile_name (THREAD *thread);
static void  open_logfile (THREAD *thread, char mode);
static word  format_text_for_output (THREAD *thread, char *formatted);
static char *time_str     (void);


/*- Global variables used in this source file only --------------------------*/

static TCB
    *tcb;                               /*  Address thread context block     */
static QID
    console;                            /*  Operator console event queue     */

#include "smtlog.d"                     /*  Include dialog data              */


/********************   INITIALISE AGENT - ENTRY POINT    ********************/

/*  ---------------------------------------------------------------------[<]-
    Function: smtlog_init

    Synopsis: Initialises the SMT logging agent.  Returns 0 if initialised
    okay, -1 if there was an error.  The logging agent writes data to log
    files.  Create an unnamed thread for each log file you want to manage,
    then send events to that thread.  Supports these public methods:
    <Table>
    CYCLE       Cycle log file if it already exists.
    OPEN        Start a new logfile as specified by event body.
    APPEND      Append to an existing logfile as specified by event body.
    PUT         Write line to logile, prefixed by date and time.
    PLAIN       Use plain logfile output (no timestamp).
    STAMP       Put timestamp at start of each logged line.
    CLOSE       Close logfile and destroy thread.
    </Table>
    Sends errors to the SMTOPER agent; does not send reply events.
    ---------------------------------------------------------------------[>]-*/

int
smtlog_init (void)
{
    AGENT   *agent;                     /*  Handle for our agent             */
    THREAD  *thread;                    /*  Handle to console thread         */
#   include "smtlog.i"                  /*  Include dialog interpreter       */

    /*  Shutdown event comes from Kernel                                     */
    declare_smtlib_shutdown   (shutdown_event, SMT_PRIORITY_MAX);

    /*  Public methods supported by this agent                               */
    declare_smtlog_cycle      (cycle_event,    0);
    declare_smtlog_open       (open_event,     0);
    declare_smtlog_append     (append_event,   0);
    declare_smtlog_put        (put_event,      0);
    declare_smtlog_plain      (plain_event,    0);
    declare_smtlog_stamp      (stamp_event,    0);
    declare_smtlog_close      (close_event,    0);

    /*  Ensure that operator console is running, else start it up            */
    smtoper_init ();
    if ((thread = thread_lookup (SMT_OPERATOR, "")) != NULL)
        console = thread-> queue-> qid;
    else
        return (-1);

    /*  Signal okay to caller that we initialised okay                       */
    return (0);
}


/*************************   INITIALISE THE THREAD   *************************/

MODULE initialise_the_thread (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */
    tcb-> handle    = 0;                /*  File is not open                 */
    tcb-> timestamp = TRUE;             /*  By default, we timestamp         */
    the_next_event = ok_event;
}


/************************   CYCLE LOGFILE IF EXISTS   ************************/

MODULE cycle_logfile_if_exists (THREAD *thread)
{
    char
        *name;

    if ((name = logfile_name (thread)) != NULL)
      {
        file_cycle (name, CYCLE_ALWAYS);
        mem_free (name);
      }
}

static char *
logfile_name (THREAD *thread)
{
    struct_smtlog_filename
        *message;
    char
        *name;

    tcb = thread-> tcb;                 /*  Point to thread's context        */

    get_smtlog_filename (thread-> event-> body, &message);

    /*  If no filename then use thread name for the log file                 */
    if (message-> filename)
      {
        name = message-> filename;
        message-> filename = NULL;      /*  So it doesn't get deallocated    */
      }
    else
        name = thread-> name;

    if (streq (name, "") || streq (name, "NULL"))
        mem_strfree (&name);

    free_smtlog_filename (&message);

    return (name);
}


/**************************   OPEN THREAD LOGFILE   **************************/

MODULE open_thread_logfile (THREAD *thread)
{
    open_logfile (thread, 'w');
}

static void
open_logfile (THREAD *thread, char mode)
{
    char
        *name;

    tcb = thread-> tcb;                 /*  Point to thread's context        */

    if ((name = logfile_name (thread)) == NULL)
        tcb-> handle = 0;
    else
      {
        tcb-> handle = lazy_open_text (name,
                       mode == 'w'? O_WRONLY | O_CREAT | O_TRUNC:
                       mode == 'a'? O_WRONLY | O_CREAT | O_APPEND:
                       /*  else  */ O_WRONLY | O_CREAT);
        if (io_completed)
            if (tcb-> handle < 0)       /*  If the open failed, send error   */
              {                         /*    to console, and terminate      */
                send_smtoper_error (&console, 
                                    strprintf ("Could not open %s", name));
                send_smtoper_error (&console, 
                                    strerror (errno));
                raise_exception (exception_event);
              }

        mem_free (name);
      }
}


/*************************   APPEND THREAD LOGFILE   *************************/

MODULE append_thread_logfile (THREAD *thread)
{
    open_logfile (thread, 'a');
}


/************************   LOG FILE OUTPUT IS PLAIN   ***********************/

MODULE log_file_output_is_plain (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */
    tcb-> timestamp = FALSE;            /*  No timestamp in data             */
}


/************************   LOG FILE OUTPUT IS TIMED   ***********************/

MODULE log_file_output_is_timed (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */
    tcb-> timestamp = TRUE;             /*  Add timestamp to data            */
}


/**************************   WRITE TEXT TO STDOUT   *************************/

MODULE write_text_to_stdout (THREAD *thread)
{
    word
        size;                           /*  Length of string                 */
    static char
        formatted [LOG_LINEMAX + 2];    /*  Formatted string                 */

    tcb = thread-> tcb;                 /*  Point to thread's context        */

    size = format_text_for_output (thread, formatted);
    formatted [size] = '\0';            /*  Add null terminator              */

    /*  Write to the log file, but do not try to signal errors - if we       */
    /*  send a message to the console now, we could create an infinite       */
    /*  loop... (the console may send it right back to us, see?)             */

    puts (formatted);
}


static word
format_text_for_output (THREAD *thread, char *formatted)
{
    struct_smtlog_text
        *message;
    word
        size,                           /*  Size of event body               */
        fmtsize;                        /*  Size of formatted string         */

    tcb = thread-> tcb;                 /*  Point to thread's context        */

    if (tcb-> timestamp)
      {                                 /*  Start with date and time         */
        strcpy (formatted, time_str ());
        strcat (formatted, ": ");       /*  Add a colon and a space          */
        fmtsize = strlen (formatted);   /*    and add event body text        */
      }
    else
        fmtsize = 0;                    /*  No date/time stamp               */

    get_smtlog_text (thread-> event-> body, &message);
    size = strlen (message-> text);

    if (size + fmtsize >= LOG_LINEMAX)
        size = LOG_LINEMAX - fmtsize;

    strncpy (formatted + fmtsize, message-> text, size);
    fmtsize += size;

    free_smtlog_text (&message);

    return fmtsize;
}


/*  -------------------------------------------------------------------------
 *  time_str
 *
 *  Returns the current date and time formatted as: "yyyy/mm/dd hh:mm:ss".
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
    time_struct = safe_localtime (&time_secs);

    sprintf (formatted_time, "%04d/%02d/%02d %2d:%02d:%02d",
                              time_struct-> tm_year + 1900,
                              time_struct-> tm_mon + 1,
                              time_struct-> tm_mday,
                              time_struct-> tm_hour,
                              time_struct-> tm_min,
                              time_struct-> tm_sec);
    return (formatted_time);
}


/*************************   WRITE TEXT TO LOGFILE   *************************/

MODULE write_text_to_logfile (THREAD *thread)
{
    word
        size;                           /*  Length of string                 */
    static char
        formatted [LOG_LINEMAX + 1];    /*  Formatted string                 */

    tcb = thread-> tcb;                 /*  Point to thread's context        */

    size = format_text_for_output (thread, formatted);

    /*  Write to the log file, but do not try to signal errors - if we       */
    /*  send a message to the console now, we could create an infinite       */
    /*  loop... (the console may send it right back to us, see?)             */

    formatted [size++] = '\n';          /*  Add a newline                    */
    lazy_write (tcb-> handle, formatted, size);
}


/**************************   CLOSE THREAD LOGFILE   *************************/

MODULE close_thread_logfile (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    if (tcb-> handle)                   /*  Close log file, if opened        */
        lazy_close (tcb-> handle);

    tcb-> handle = 0;                   /*  File is not open                 */
}


/*************************   TERMINATE THE THREAD   **************************/

MODULE terminate_the_thread (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */
    the_next_event = terminate_event;
}
