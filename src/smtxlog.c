/*===========================================================================*
 *                                                                           *
 *  smtxlog.c - SMT extended logging agent                                   *
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
#include "smtxlog.h"

/*  -------------------------------------------------------------------------
    Log file formats are defined as a string:

    log format contain variable, in format '$(variable)'.
    When you log something, all variables values passed in
    parameters are put in format buffer.
    Some system variables are set by default and can be used
    in format string:

        $(datetime) - date/time in NCSA format
        $(yy)       - year as two digits
        $(year)     - year as four digits
        $(mon)      - month
        $(day)      - day
        $(hh)       - hour, using 24-hour clock
        $(mm)       - minutes
        $(ss)       - seconds

    if you put the 'g' character for first character of
    variable, use the UTC date/time
      ex: $(ghh), ...

    Cycling conditions:
        MANUAL  - Manually only
        STARTUP - When log agent start
        HOURLY  - Hourly, at x minutes past the hour
        DAILY   - Daily, at hh:mm
        WEEKLY  - Weekly, at day-of-week, hh:mm
        MONTHLY - Monthly, at day-of-month, hh:mm
        SIZE    - When log file exceeds n kbytes
        LINES   - When log file exceeds n lines

    Cycling methods:
        RENAME  - Rename to path/filename where filename is timestamped
                  Any existing file with same name is overwritten
        DELETE  - Delete old file (no copies left)
        MOVE    - Move to path (not timestamped)
        CONCAT  - Concat to path/filename where filename can be timestamped
        PROCESS - Process using timestamped command, %f is log filename

    Timestamping:
        %y        day of year, 001-366
        %yy       year 2 digits, 00-99
        %yyyy     year 4 digits, 0100-9999
        %mm       month, 01-12
        %mmm      month, Jan
        %mmmm     month, January
        %MMM      month, JAN
        %MMMM     month, JANUARY
        %dd       day, 01-31
        %ddd      day of week, Sun
        %dddd     day of week, Sunday
        %DDD      day of week, SUN
        %DDDD     day of week, SUNDAY
        %w        day of week, 1-7 (1=Sunday)
        %ww       week of year, 01-53
        %q        year quarter, 1-4
        %h        hour, 00-23
        %m        minute, 00-59
        %s        second, 00-59
        %c        centisecond, 00-99
        %%        literal character %
    -------------------------------------------------------------------------*/


/*- Definitions -------------------------------------------------------------*/

#undef  AGENT_NAME
#define AGENT_NAME       SMTXLOG_AGENT   /*  Our public name                  */
#define SMTXLOG_INTERVAL  INTERVAL_MIN   /*  Check log files once per minute  */

#define CYCLE_STARTUP    0              /*  Encoded cycle_when values        */
#define CYCLE_HOURLY     1
#define CYCLE_DAILY      2
#define CYCLE_WEEKLY     3
#define CYCLE_MONTHLY    4
#define CYCLE_MANUAL     5
#define CYCLE_SIZE       6
#define CYCLE_LINES      7

#define CYCLE_RENAME     0              /*  Encoded cycle_how values         */
#define CYCLE_DELETE     1
#define CYCLE_MOVE       2
#define CYCLE_CONCAT     3
#define CYCLE_PROCESS    4


typedef struct                          /*  Thread context block:            */
{
    XLOG_STATS stats;                   /*    Log file statistics            */
    struct_smtxlog_open                 /*                                   */
           *open;                       /*    Logfile OPEN arguments         */
    struct_smtxlog_log                  /*                                   */
           *log;                        /*    Logfile LOG arguments          */
    int     handle;                     /*    Handle for i/o to logfile      */
    char   *filename;                   /*    Full log-file filename         */
    char   *log_format;                 /*    Log format string used         */
    char   *client_host;                /*    Translated client hostname     */
    int     cycle_when;                 /*    Decoded cycle_when             */
    int     cycle_how;                  /*    Decoded cycle_how              */
    long    cycle_date;                 /*    Date and time when log file    */
    long    cycle_time;                 /*      should be cycled again       */
    qbyte   file_limit;                 /*    Max. log file size             */
    PROCESS process_id;                 /*    If we use subprocesses         */
    SYMTAB *symtab;                     /*    Used for symbol substitution   */
    QUEUE  *queue;                      /*    Separate event queue           */
} TCB;


/*- Function prototypes -----------------------------------------------------*/

static void  open_logfile   (THREAD *thread, char mode);


/*- Global variables used in this source file only --------------------------*/

static TCB
    *tcb;                               /*  Address thread context block     */
static QID
    operq;                              /*  Operator console event queue     */
static AGENT
    *this_agent;                        /*  Pointer to this agent            */

#include "smtxlog.d"                     /*  Include dialog data              */


/********************   INITIALISE AGENT - ENTRY POINT    ********************/

/*  ---------------------------------------------------------------------[<]-
    Function: smtxlog_init

    Synopsis: Initialises the SMT extended logging agent.  Returns 0 if
    initialised okay, -1 if there was an error.  The logging agent writes
    data to log files.  Create an named thread for each log file you want to
    manage, then send events to that thread.  Supports these methods:
    <Table>
    OPEN    Start a new logfile as specified by event body.
    LOG     Write normal log file line
    PUT     Write plain log file line
    CYCLE   Cycle log file if it already exists
    CLEAR   Clear (empty) log file and continue
    CLOSE   Close logfile and prepare for new open event
    </Table>
    Sends errors to the SMTOPER agent; does not send reply events.
    ---------------------------------------------------------------------[>]-*/

int
smtxlog_init (void)
{
    AGENT   *agent;                     /*  Handle for our agent             */
    THREAD  *thread;                    /*  Handle to console thread         */
#   include "smtxlog.i"                  /*  Include dialog interpreter       */

    /*  Shutdown event comes from Kernel                                     */
    declare_smtlib_shutdown   (shutdown_event, SMT_PRIORITY_MAX);

    /*  Public methods supported by this agent                               */
    declare_smtxlog_open         (open_event,     0);
    declare_smtxlog_log          (log_event,      0);
    declare_smtxlog_cycle        (cycle_event,    0);
    declare_smtxlog_clear        (clear_event,    0);
    declare_smtxlog_close        (close_event,    0);
    declare_smtlog_put           (put_event,      0);

    /*  Alarm event sent by timer to this agent                              */
    declare_smttime_reply        (timer_event,    0);


    this_agent = agent;

    /*  Ensure that timer agent is running, else start it up                 */
    if (smttime_init ())
        return (-1);

    /*  Ensure that operator console is running, else start it up            */
    smtoper_init ();
    if ((thread = thread_lookup (SMT_OPERATOR, "")) != NULL)
        operq = thread-> queue-> qid;
    else
        return (-1);

    /*  Signal okay to caller that we initialised okay                       */
    return (0);
}


/*  ---------------------------------------------------------------------[<]-
    Function: smtxlog_log

    Synopsis: log directly a symbol table.
    ---------------------------------------------------------------------[>]-*/

void
smtxlog_log (QID *to, char *log_file, SYMTAB *table)
{
    DESCR
        *log;

    ASSERT (table);
    ASSERT (log_file);

    log = symb2descr (table);
    if (log)
      {
        lsend_smtxlog_log(to, NULL, NULL, NULL, NULL, 0,
                          log_file, (word)log-> size, log-> data);

        mem_free (log);
      }
}


/*************************   INITIALISE THE THREAD   *************************/

MODULE initialise_the_thread (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    tcb-> open        = NULL;           /*  No open parameters allocated     */
    tcb-> log         = NULL;           /*  No log parameters allocated      */
    tcb-> client_host = NULL;           /*  No translated client hostname    */
    tcb-> handle      = 0;              /*  File is not open                 */
    tcb-> filename    = NULL;           /*  And no filename is specified     */
    tcb-> symtab      = NULL;

    the_next_event = ok_event;
}


/***************************   CREATE EVENT QUEUE   **************************/

MODULE create_event_queue (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    tcb-> queue = queue_create (NULL, 100);
}


/***************************   FLUSH EVENT QUEUE   ***************************/

MODULE flush_event_queue (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    queue_flush (tcb-> queue);
}


/**************************   DESTROY EVENT QUEUE   **************************/

MODULE destroy_event_queue (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    queue_destroy (tcb-> queue);
}


/*************************   GET LOGFILE PARAMETERS   ************************/

MODULE get_logfile_parameters (THREAD *thread)
{
    static LOOKUP
        lookup_when [] = {
            { XLOG_CYCLE_STARTUP, CYCLE_STARTUP },
            { XLOG_CYCLE_HOURLY,  CYCLE_HOURLY  },
            { XLOG_CYCLE_DAILY,   CYCLE_DAILY   },
            { XLOG_CYCLE_WEEKLY,  CYCLE_WEEKLY  },
            { XLOG_CYCLE_MONTHLY, CYCLE_MONTHLY },
            { XLOG_CYCLE_MANUAL,  CYCLE_MANUAL  },
            { XLOG_CYCLE_SIZE,    CYCLE_SIZE    },
            { XLOG_CYCLE_LINES,   CYCLE_LINES   },
            { "0",                CYCLE_STARTUP },
            { "1",                CYCLE_HOURLY  },
            { "2",                CYCLE_DAILY   },
            { "3",                CYCLE_WEEKLY  },
            { "4",                CYCLE_MONTHLY },
            { "5",                CYCLE_MANUAL  },
            { "6",                CYCLE_SIZE    },
            { "7",                CYCLE_LINES   },
            { NULL,               -1            }
        },
        lookup_how [] = {
            { XLOG_CYCLE_RENAME,  CYCLE_RENAME  },
            { XLOG_CYCLE_DELETE,  CYCLE_DELETE  },
            { XLOG_CYCLE_MOVE,    CYCLE_MOVE    },
            { XLOG_CYCLE_CONCAT,  CYCLE_CONCAT  },
            { XLOG_CYCLE_PROCESS, CYCLE_PROCESS },
            { "0",                CYCLE_RENAME  },
            { "1",                CYCLE_DELETE  },
            { "2",                CYCLE_MOVE    },
            { "3",                CYCLE_CONCAT  },
            { "4",                CYCLE_PROCESS },
            { NULL,               -1            }
        };
    char
        *format,
        *when,
        *how;

    tcb = thread-> tcb;                 /*  Point to thread's context        */

    /*  We free the smtxlog_open structure only when we're done with it       */
    if (tcb-> open)
        free_smtxlog_open (&tcb-> open);

    /*  Load log file parameters into open block                             */
    get_smtxlog_open (thread-> event-> body, &tcb-> open);

    /*  Use some short names; we reference these a lot                       */
    format = tcb-> open-> log_format;
    when   = tcb-> open-> cycle_when;
    how    = tcb-> open-> cycle_how;

    /*  Get actual log format to use                                         */
    tcb-> log_format = format;

    /*  Parse cycle_when argument, which maps over the standard options in
     *  Xitami, and which can be numeric as well as a string in up/low case.
     */
    tcb-> cycle_when = strlookup (lookup_when, strlwc (when));
    if (tcb-> cycle_when == -1)
      {
        send_smtoper_error (
            &operq,
            strprintf ("Invalid cycle condition: %s", when));
        raise_exception (exception_event);
      }
    tcb-> cycle_how = strlookup (lookup_how, strlwc (how));
    if (tcb-> cycle_how == -1)
      {
        send_smtoper_error (
            &operq,
            strprintf ("Invalid cycle method: %s", how));
        raise_exception (exception_event);
      }
}


/***************************   BUILD LOG FILENAME   **************************/

MODULE build_log_filename (THREAD *thread)
{
    char
        *formatted;                     /*  Formatted file name              */

    tcb = thread-> tcb;                 /*  Point to thread's context        */

    /*  Build a filename as specified from the path and file pattern         */
    if (strnull (tcb-> open-> log_path)
    || file_is_directory (tcb-> open-> log_path))
      {
        formatted = timestamp_string (NULL, tcb-> open-> log_file);
        mem_strfree (&tcb-> filename);
        tcb-> filename = mem_strdup (
            file_where ('s', tcb-> open-> log_path, formatted, NULL));
        mem_free (formatted);
        ASSERT (tcb-> filename);
      }
    else
      {
        send_smtoper_error (
            &operq,
            strprintf ("Log path '%s' is not accessible",
                       tcb-> open-> log_path));
        raise_exception (exception_event);
      }
}


/***********************   CHECK IF CYCLE BEFORE OPEN   **********************/

MODULE check_if_cycle_before_open (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    /*  If the log file already exists when we start we cycle it if
     *  cycling is manual, or size-based and the file is too large.
     */
    if (file_exists (tcb-> filename))
      {
        if (tcb-> cycle_when == CYCLE_SIZE)
          {
            if (get_file_size (tcb-> filename) / 1024
            > atol (tcb-> open-> cycle_size))
                raise_exception (cycle_now_event);
          }
        else
        if (tcb-> cycle_when == CYCLE_LINES)
          {
            if (file_lines (tcb-> filename)
            > atol (tcb-> open-> cycle_lines))
                raise_exception (cycle_now_event);
          }
        else
        if (tcb-> cycle_when == CYCLE_STARTUP)
            raise_exception (cycle_now_event);
      }
}


/************************   RECALCULATE CYCLE TIMER   ************************/

MODULE recalculate_cycle_timer (THREAD *thread)
{
    long
        arg_time,                       /*  Log file parameters              */
        arg_day,                        /*    for timing-based cycling       */
        cycle_time,                     /*  Calculated time and date         */
        cycle_date,                     /*    for next cycle operation       */
        cycle_days,                     /*  Julian date for next cycle       */
        cycle_hh,                       /*  Hour for next cycle              */
        cycle_mm,                       /*  Minute for next cycle            */
        cycle_year,                     /*  Year for next cycle              */
        cycle_month,                    /*  Month for next cycle             */
        cycle_day;                      /*  Day for next cycle               */
    Bool
        want_alarm = TRUE;              /*  Do we use timer alarms?          */

    tcb = thread-> tcb;                 /*  Point to thread's context        */

    /*  Calculate the time for the next cycling operation, if cycle_when is
     *  time-based.
     */
    arg_time = conv_str_time (tcb-> open-> cycle_time);
    arg_day  = atol          (tcb-> open-> cycle_date);

    /*  Start with current time as base point                                */
    cycle_date = date_now ();
    cycle_time = time_now ();
    cycle_days = date_to_days (cycle_date);
    cycle_hh   = GET_HOUR     (cycle_time);
    cycle_mm   = GET_MINUTE   (cycle_time);

    switch (tcb-> cycle_when)
      {
        case CYCLE_HOURLY:
            /*  At mm minutes past the hour                                  */
            if (cycle_mm >= GET_MINUTE (arg_time))
                if (++cycle_hh == 24)
                  {
                    cycle_hh = 0;
                    cycle_days++;
                  }
            cycle_mm = GET_MINUTE (arg_time);
            break;

        case CYCLE_DAILY:
            if (cycle_time >= arg_time)
                cycle_days++;
            cycle_hh = GET_HOUR   (arg_time);
            cycle_mm = GET_MINUTE (arg_time);
            break;

        case CYCLE_WEEKLY:
            /*  First calculate cycle day, in same week as today, 0=Sunday   */
            cycle_days = cycle_days - day_of_week (cycle_date) + arg_day;

            /*  If that date has passed, move to next week                   */
            if ((cycle_days <  date_to_days (cycle_date))
            ||  (cycle_days == date_to_days (cycle_date)
            &&   cycle_time >= arg_time))
                cycle_days += 7;
            cycle_hh = GET_HOUR   (arg_time);
            cycle_mm = GET_MINUTE (arg_time);
            break;

        case CYCLE_MONTHLY:
            cycle_year  = GET_CCYEAR (cycle_date);
            cycle_month = GET_MONTH  (cycle_date);
            cycle_day   = GET_DAY    (cycle_date);
            if (arg_day < 1)
                arg_day = 1;
            if (cycle_day >  arg_day
            || (cycle_day == arg_day && cycle_time >= arg_time))
                if (++cycle_month == 13)
                  {
                    cycle_month = 1;
                    cycle_year++;
                  }
            cycle_hh   = GET_HOUR   (arg_time);
            cycle_mm   = GET_MINUTE (arg_time);
            cycle_date = cycle_year * 10000 + cycle_month * 100 + arg_day;
            cycle_days = date_to_days (cycle_date);
            break;

        case CYCLE_STARTUP:
        case CYCLE_MANUAL:
            want_alarm = FALSE;
            break;

        case CYCLE_SIZE:
            tcb-> file_limit = atol (tcb-> open-> cycle_size);
            break;

        case CYCLE_LINES:
            tcb-> file_limit = atol (tcb-> open-> cycle_lines);
            break;

        default:
            send_smtoper_error (&operq,
                                "smtxlog: internal error 001");
            raise_exception (exception_event);
      }
    tcb-> cycle_time = MAKE_TIME    (cycle_hh, cycle_mm, 0, 0);
    tcb-> cycle_date = days_to_date (cycle_days);

    /*  Check the log file every SMTXLOG_INTERVAL centiseconds                */
    if (want_alarm)
        smttime_request_alarm (0, SMTXLOG_INTERVAL, 0);
}


/***********************   CHECK IF CYCLE WHILE OPEN   ***********************/

MODULE check_if_cycle_while_open (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    switch (tcb-> cycle_when)
      {
        case CYCLE_HOURLY:
        case CYCLE_DAILY:
        case CYCLE_WEEKLY:
        case CYCLE_MONTHLY:
            if (date_now () >  tcb-> cycle_date
            || (date_now () == tcb-> cycle_date
             && time_now () >= tcb-> cycle_time))
                raise_exception (cycle_now_event);
            break;

        case CYCLE_SIZE:
            if (tcb-> file_limit      > 0
            &&  (tcb-> stats.file_size / 1024) > tcb-> file_limit)
                raise_exception (cycle_now_event);
            break;

        case CYCLE_LINES:
            if (tcb-> stats.file_lines > tcb-> file_limit)
                raise_exception (cycle_now_event);
            break;

        default:
            send_smtoper_error (&operq,
                                "smtxlog: internal error 002");
            raise_exception (exception_event);
      }
}


/***************************   CHECK CYCLE METHOD   **************************/

MODULE check_cycle_method (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    switch (tcb-> cycle_how)
      {
        case CYCLE_RENAME:
            the_next_event = rename_old_event;
            break;
        case CYCLE_DELETE:
            the_next_event = delete_old_event;
            break;
        case CYCLE_MOVE:
            the_next_event = move_old_event;
            break;
        case CYCLE_CONCAT:
            the_next_event = concat_old_event;
            break;
        case CYCLE_PROCESS:
            the_next_event = process_old_event;
            break;
        default:
            send_smtoper_error (
                &operq,
                "smtxlog: internal error 003");
            raise_exception (exception_event);
      }
}


/**************************   OPEN LOGFILE APPEND   **************************/

MODULE open_logfile_append (THREAD *thread)
{
    open_logfile (thread, 'a');
}

static void
open_logfile (THREAD *thread, char mode)
{
    int
        open_mode;

    tcb = thread-> tcb;                 /*  Point to thread's context        */

    /*  First get size of file, if necessary                                 */
    if (mode == 'a'
    &&  file_exists (tcb-> filename))
      {
        tcb-> stats.file_size  = get_file_size  (tcb-> filename);
        tcb-> stats.file_lines = get_file_lines (tcb-> filename);
      }
    else
      {
        /*  Zero, since we're going to create an new file                    */
        tcb-> stats.file_size  = 0;
        tcb-> stats.file_lines = 0;
      }
    if (mode == 'w')
        open_mode = O_WRONLY | O_CREAT | O_TRUNC;
    else
        open_mode = O_WRONLY | O_CREAT | O_APPEND;

    /*  Open file as specified                                               */
    tcb-> handle = lazy_open_text (tcb-> filename, open_mode);

    /*  If file is already opened by another application, don't panic        */
    /*  We'll set the last character of the filename to '$' and try again    */
    if (io_completed && tcb-> handle < 0
    &&  errno == EACCES)
      {
        strlast (tcb-> filename) = '$';
        tcb-> handle = lazy_open_text (tcb-> filename, open_mode);
      }
    if (io_completed && tcb-> handle < 0)
      {
        /*  If the open failed, send error to console, and terminate         */
        send_smtoper_error (
            &operq,
            strprintf ("Could not open %s", tcb-> filename));
        send_smtoper_error (
            &operq,
            strerror (errno));
        raise_exception (exception_event);
      }
}


/**************************   OPEN LOGFILE CREATE   **************************/

MODULE open_logfile_create (THREAD *thread)
{
    open_logfile (thread, 'w');
}


/*****************************   CLOSE LOGFILE   *****************************/

MODULE close_logfile (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    if (tcb-> handle)                   /*  Close log file, if opened        */
      {
        lazy_close (tcb-> handle);
        tcb-> handle = 0;
      }
}


/***************************   DELETE OLD LOGFILE   **************************/

MODULE delete_old_logfile (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    file_delete (tcb-> filename);
}


/***************************   RENAME OLD LOGFILE   **************************/

MODULE rename_old_logfile (THREAD *thread)
{
    char
        *formatted,
        *new_name;                      /*  Target file name                 */

    tcb = thread-> tcb;                 /*  Point to thread's context        */

    /*  Rename log file to new name using supplied path and pattern          */
    formatted = timestamp_string (NULL, tcb-> open-> cycle_argument);
    new_name  = file_where ('s', tcb-> open-> log_path, formatted, NULL);

    if (streq (new_name, tcb-> filename))
        send_smtoper_error (
            &operq,
            strprintf ("Can't rename '%s' to '%s'",
                       tcb-> filename, new_name));
    else
      {
        if (file_exists (new_name))
          {
            mem_free (formatted);
            formatted = get_new_filename (new_name);
            new_name  = formatted;
          }
        if (file_rename (tcb-> filename, new_name) == 0)
            file_delete (tcb-> filename);
        else
          {
            send_smtoper_error (
                &operq,
                strprintf ("Could not rename '%s' to '%s'",
                           tcb-> filename, new_name));
            send_smtoper_error (
                &operq,
                strerror (errno));
          }
      }
    mem_free (formatted);
}


/****************************   MOVE OLD LOGFILE   ***************************/

MODULE move_old_logfile (THREAD *thread)
{
    char
        *rename_to,                     /*  New location and name for file   */
        *new_filename = NULL;           /*  In case of duplicates            */

    tcb = thread-> tcb;                 /*  Point to thread's context        */

    /*  Move log file to specified directory                                 */
    rename_to = file_where ('s', tcb-> open-> cycle_argument,
                                 tcb-> filename, NULL);
    ASSERT (rename_to);
    if (file_exists (rename_to))
      {
        new_filename = get_new_filename (rename_to);
        rename_to = new_filename;
      }
    if (file_rename (tcb-> filename, rename_to) == 0)
        file_delete (tcb-> filename);
    else
      {
        send_smtoper_error (
            &operq,
            strprintf ("Could not move '%s' to '%s'",
                       tcb-> filename, rename_to));
        send_smtoper_error (
            &operq,
            strerror (errno));
      }
    mem_free (new_filename);            /*  If this was allocated            */
}


/***************************   CONCAT OLD LOGFILE   **************************/

MODULE concat_old_logfile (THREAD *thread)
{
    char
        *target;                        /*  Target file name                 */

    tcb = thread-> tcb;                 /*  Point to thread's context        */

    /*  Concat log file to specified file                                    */
    target = timestamp_string (NULL, tcb-> open-> cycle_argument);
    if (file_concat (tcb-> filename, target) == 0)
        file_delete (tcb-> filename);
    else
      {
        send_smtoper_error (
            &operq,
            strprintf ("Could not write to '%s'", target));
        send_smtoper_error (
            &operq,
            strerror (errno));
      }
    mem_free (target);
}


/**************************   PROCESS OLD LOGFILE   **************************/

MODULE process_old_logfile (THREAD *thread)
{
    char
        *insert_mark,                   /*  Position of %f in command        */
        *command,                       /*  Command to execute               */
        *copy_command;                  /*  With inserted filename           */

    tcb = thread-> tcb;                 /*  Point to thread's context        */

    /*  Execute command on specified file                                    */
    /*  First look for %f, and replace by filename, if necessary             */
    command = timestamp_string (NULL, tcb-> open-> cycle_argument);
    while ((insert_mark = strstr (command, "%f")) != NULL)
      {
        *insert_mark = '\0';
        copy_command = xstrcpy (NULL, command, tcb-> filename,
                                insert_mark + 2, NULL);
        mem_free (command);
        command = copy_command;
      }
    tcb-> process_id = process_create (
        command, NULL, ".", NULL, NULL, NULL, NULL, FALSE);

    if (tcb-> process_id == NULL_PROCESS)
      {
        send_smtoper_error (
            &operq,
            strprintf ("Can't run '%s'", command));
        send_smtoper_error (
            &operq,
            strerror (errno));
      }
    mem_free (command);
}


/***********************   GET LOG REQUEST ARGUMENTS   ***********************/

MODULE get_log_request_arguments (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    ASSERT (tcb-> log == NULL);

    get_smtxlog_log (thread-> event-> body, &tcb-> log);

}


/***********************   FREE LOG REQUEST ARGUMENTS   **********************/

MODULE free_log_request_arguments (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    free_smtxlog_log (&tcb-> log);
    mem_strfree (&tcb-> client_host);
}




/************************   WRITE REQUEST LOG ENTRY   ************************/

MODULE write_request_log_entry (THREAD *thread)
{
#   define STORE(name,value)                                                 \
        sym_assume_symbol (tcb-> symtab, (name), (value))
#   define STFMT(name,format,value)                                          \
        sprintf (buffer, (format), (value));                                 \
        sym_assume_symbol (tcb-> symtab, (name), buffer)
#   define NOTNULL(x)  (x) && *(x)? (x): "-"

    time_t
        time_secs;
    struct tm
        *utc_time_struct,
        *time_struct;
    static char
        buffer [20];                    /*  Formatted values                 */
    char
        *log_line;
    DESCR
        log;

    if (tcb-> symtab)
        sym_delete_table (tcb->symtab);

    log.size = tcb-> log-> value_size;
    log.data = tcb-> log-> value;

    tcb-> symtab = descr2symb (&log);

    /*  Get current date and time                                            */
    time_secs       = time (NULL);
    time_struct     = safe_localtime (&time_secs);
    utc_time_struct = gmtime         (&time_secs);

    tcb = thread-> tcb;                 /*  Point to thread's context        */


    STORE ("datetime", http_time_str ());
    STFMT ("yy"      , "%02d", time_struct-> tm_year);
    STFMT ("year"    , "%04d", time_struct-> tm_year + 1900);
    STFMT ("mon"     , "%02d", time_struct-> tm_mon + 1);
    STFMT ("day"     , "%02d", time_struct-> tm_mday);
    STFMT ("hh"      , "%02d", time_struct-> tm_hour);
    STFMT ("mm"      , "%02d", time_struct-> tm_min);
    STFMT ("ss"      , "%02d", time_struct-> tm_sec);

    STFMT ("gyy"     , "%02d", utc_time_struct-> tm_year);
    STFMT ("gyear"   , "%04d", utc_time_struct-> tm_year + 1900);
    STFMT ("gmon"    , "%02d", utc_time_struct-> tm_mon + 1);
    STFMT ("gday"    , "%02d", utc_time_struct-> tm_mday);
    STFMT ("ghh"     , "%02d", utc_time_struct-> tm_hour);
    STFMT ("gmm"     , "%02d", utc_time_struct-> tm_min);
    STFMT ("gss"     , "%02d", utc_time_struct-> tm_sec);

    log_line = tok_subst (tcb-> log_format, tcb-> symtab);
    if (tcb-> handle)
      {
        ASSERT (write (tcb-> handle, log_line, strlen (log_line)));
        ASSERT (write (tcb-> handle, "\n", 1));
      }
    tcb-> stats.file_size  += strlen (log_line) + 1;
    tcb-> stats.file_lines += 1;
    mem_strfree (&log_line);
}


/*************************   WRITE PLAIN LOG ENTRY   *************************/

MODULE write_plain_log_entry (THREAD *thread)
{
    char
        *log_line;

    tcb = thread-> tcb;                 /*  Point to thread's context        */

    log_line = (char *) thread-> event-> body;
    if (tcb-> handle)
      {
        lazy_write (tcb-> handle, log_line, strlen (log_line));
        lazy_write (tcb-> handle, "\n",     1);
      }
}


/***********************   CHECK QUEUED EVENTS IF ANY   **********************/

MODULE check_queued_events_if_any (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    queue_deliver (tcb-> queue, thread);
}


/*************************   TERMINATE THE THREAD   **************************/

MODULE terminate_the_thread (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    if (tcb-> open)
        free_smtxlog_open (&tcb-> open);
    sym_delete_table (tcb-> symtab);
    mem_strfree (&tcb-> filename);

    the_next_event = terminate_event;
}

