/*===========================================================================*
 *                                                                           *
 *  smtlib.c - SMT kernel functions                                          *
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

#if defined (DEBUG)
#    define SMT_DETECT     1
#    define SMT_LOOP_MAX   8            /*  Detect loops of up to 8 events   */
#    define SMT_LOOP_TRAP  8000         /*  Break after 8000 iterations      */
#endif

#if defined (SMT_LOOP_MAX)
#    define LOOP_EVENT(n)  thread-> loop_event                                \
                               [(n + 2 * SMT_LOOP_MAX) % (2 * SMT_LOOP_MAX)]
#endif

#include "smtpriv.h"                    /*  SMT definitions                  */

#if (defined (SMT_PROFILE))
#    include <sys/times.h>
#endif

/*  -------------------------------------------------------------------------
 *  Local structures
 */

/*  The semaphore descriptor defines all the information for a semaphore.    */
/*  Semaphores are linked into a list that is managed by the kernel.         */

struct _SEMAPH {                        /*  Semaphore block                  */
    /*- Public -------------------------                                     */
    SEMAPH  *next, *prev;               /*    Doubly-linked list             */
    char    *name;                      /*    Name of semaphore              */
    /*- Private ------------------------                                     */
    SYMBOL  *symbol;                    /*    Entry into dictionary          */
    THREAD  threads;                    /*    List of threads                */
    long    value;                      /*    Semaphore value                */
};

/*  Asynchronous functions are called according as close as possible to      */
/*  their scheduled date/time by SMT.                                        */

struct _ASYNC {                         /*  Asynchronous descriptor          */
    ASYNC   *next, *prev;               /*    List sorted by time            */
    ASYNC   *left, *right;              /*    List sorted by priority/time   */
    ASYNCFN *function;                  /*    The function                   */
    void    *arg;                       /*    The function argument          */
    int      priority;                  /*    50=Low, 100=Normal, 200=High   */
    long     next_date,
             next_time;                 /*    Date/time for next execution   */
    MEMTRN  *memtrn;                    /*    Mem tran for fn calls          */
};

/*  Polling functions are called regularly during the normal thread          */
/*  processing, after every interval states changes.  They are maintained in */
/*  an array because they are expected to be mostly static.                  */

struct _POLL {                          /*  Polling function descriptor      */
    POLLFN       *function;             /*    The function                   */
    void         *arg;                  /*    The function argument          */
    volatile int interval;              /*    Interval state changes         */
    volatile int togo;                  /*    State changes to go this time  */
    MEMTRN       *memtrn;               /*    Mem tran for fn calls          */
};

#define MAX_POLLFNS  20                 /*  Max num of pollfns allowed       */

/*  -------------------------------------------------------------------------
 *  Global variables
 */

int
    smt_errno = 0;                      /*  Set when API detects an error    */
char
    *smt_errlist [] = {                 /*  Corresponding error messages     */
        "No errors",
        "Event already declared",
        "Agent had internal error",
        "Method already declared",
        "Unknown event name",
        "Unknown method name",
        "Event queue not defined",
        "Unknown semaphore name",
        "Agent not defined",
        "Thread not declared",
        "SMT API not initialised",
        "Not enough heap memory left",
        "Event queue is empty",
        "Event queue is full",
        "Semaphore already declared",
        "Agent already declared",
        "Agent not initialised",
        "Thread already declared",
        "Too many threads"
    };
event_t
    _the_next_event,                    /*  May be set by thread code        */
    _the_external_event,                /*  Set by event_wait()              */
    _the_exception_event;               /*  May be set by thread code        */
Bool
    _exception_raised,                  /*  May be set by thread code        */
    _io_completed,                      /*  Current lazy I/O finished        */
    _dialog_jump;                       /*  Was module list is broken?       */

/*  The signal_raised flag is true when we've received an interrupt, until   */
/*  the application ends.  The signal_value is one of SMT_SIGNAL_XXXX.       */

/*  The shutdown_request flag is true from the moment a fatal signal (kill,  */
/*  interrupt, or segment violation) is received to the moment the SHUTDOWN  */
/*  events are broadcast.  It's primary use is to halt any ongoing dialog    */
/*  activity.  Basically a SHUTDOWN overrides any other on-going events.     */

Bool
    signal_raised,                      /*  True after interrupt             */
    shutdown_request;                   /*  When kill signal in progress     */
int
    signal_value;                       /*  Value of signal                  */

MEMTRN
    *smt_memtrn_;                       /*  Current transaction              */

LR_LOCATION 
    *LR_location_cache;                 /*  Cache for dialog locations       */

/*  -------------------------------------------------------------------------
 *  Global variables local to this source
 */

typedef struct _PTRITEM {               /*  Reusable list of pointers type   */
    struct _PTRITEM
             *next, *prev;              /*    Doubly-linked list             */
    void     *pointer;                  /*    And the pointers               */
} PTRITEM;

static Bool    smt_alive = FALSE;       /*  Have we been initialised?        */
static SYMTAB *dict;                    /*  Main dictionary                  */
static LIST    exitfcts;                /*  List of exit functions           */
static LIST    agents;                  /*  List of agents                   */
static LIST    semaphs;                 /*  List of semaphores               */
static THREAD  active_threads;          /*  Active threads list              */
static THREAD  waiting_threads;         /*  List of threads waiting for event*/
static int     cur_threads;             /*  Number of threads created        */

static LIST    async_by_time;           /*  List of non-blocking async fun's */
static ASYNC   async_by_priority;       /*  By time and by priority.         */
static ASYNCFN *waitfn;                 /*  There can only be 1 wait funct'n */
static void    *waitarg;                /*     its argument,                 */
static MEMTRN  *waittrn;                /*     and memory transaction.       */
static Bool    async_removed = FALSE;   /*  Flag: some async fun's deleted   */

static POLL    pollfns [MAX_POLLFNS];   /*  Polling functions                */
static int     active_pollfns = 0;      /*     active polling functions      */

static QID     console = { 0, 0 };      /*  Console queue, if any            */
static int     break_wanted;            /*  Break current module list        */
static SEMAPH *break_semaph;            /*  Break for this semaphore         */
#if (defined (SMT_DETECT))
static char   *cur_agent  = "";         /*  For error detection              */
static char   *cur_state  = "";         /*  For error detection              */
static char   *cur_event  = "";         /*  For error detection              */
static char   *cur_module = "";         /*  For error detection              */
static char   *cur_step   = "";         /*  For error detection              */
#endif
#define BREAK_CONTINUE      0           /*  Reasons for breaking the list    */
#define BREAK_RECYCLE       1           /*    of actions for an event        */
#define BREAK_WAIT_EVENT    2           /*    in a state transition          */
#define BREAK_WAIT_SEMAPH   3


/*  -------------------------------------------------------------------------
 *  Local function prototypes and macros
 */

static void    handle_signals   (void);
#if (defined (SMT_PROFILE))
static void    save_profile_results (void);
#endif
static void    handle_signal    (int the_signal);
static char   *get_queue_name   (const QID *qid);
static EVENT  *event_locate     (QUEUE *queue, EVENT *event);
static char   *get_entity_name  (const char *agent, const char *thread);
static char   *get_method_name  (const char *agent_name,
                                 const char *method_name);
static void    thread_relink    (THREAD *left, THREAD *thread, THREAD *right);
static void    activate_thread  (THREAD *thread);
static void    suspend_thread   (THREAD *thread, int state);
static void    async_destroy    (ASYNC *async);
static void    request_all_threads_shutdown (void);
static void    initialise_the_thread (THREAD *thread);
static inline Bool lookup_module_vector (THREAD *thread);
static inline Bool lookup_next_event    (THREAD *thread);
static char   *get_semaph_name  (const char *semaph_name);
static void    handle_atexit    (void);
static void    set_io_completed (int rc);

static int     add_unique_pointer (void *pointer, LIST *ptrlist);
static void    destroy_location_stack (LR_LOCATION *stack);

/*  We can relink threads in various interesting ways                        */
#define thread_relink_after(t,a)   thread_relink ((a), (t), (a)-> right)
#define thread_relink_before(t,b)  thread_relink ((b)-> left, (t), (b))
#define thread_unlink(t)           thread_relink ((t)-> left, (t), (t)-> right)

/*  Order these to include or exclude tracing code...                        */
/*  When tracing is enabled, about 20% of the application's time is spent    */
/*  in the trace() function, so generally we leave this switched off.        */

#define SMT_TRACE
#undef  SMT_TRACE


/*  ---------------------------------------------------------------------[<]-
    Function: smt_init

    Synopsis: Initialises the SMT.  Returns 0 if there were no errors.  Else
    returns -1, after seting smt_errno to one of these values:
    <Table>
    SMT_OUTOFMEMORY          Not enough heap memory left
    </Table>
    ---------------------------------------------------------------------[>]-*/

int
smt_init (void)
{
    int feedback = 0;
#if (defined (SMT_TRACE))
    trace ("smt_init");
#endif
    if (!smt_alive)
      {
        smt_memtrn_ = NULL;             /*  Use default transaction to start */

        dict = sym_create_table ();
        if (dict == NULL)
          {
            smt_errno = SMT_OUTOFMEMORY;
            feedback = -1;
          }
        /*  Initialise list of agents                                        */
        list_reset (&agents);

        /*  Initialise list of exit functions                                */
        list_reset (&exitfcts);

        /*  Initialise list of semaphores                                    */
        list_reset (&semaphs);

        /*  Initialise list of active threads                                */
        active_threads.left  = & active_threads;
        active_threads.right = & active_threads;
        cur_threads = 0;
        LR_location_cache = NULL;

        /*  Initialise list of waiting threads                               */
        waiting_threads.left  = &waiting_threads;
        waiting_threads.right = &waiting_threads;

        /*  Initilise hooks to asyncronous functions                         */
        list_reset (&async_by_time);
        async_by_priority. left  = & async_by_priority;
        async_by_priority. right = & async_by_priority;
        waitfn = NULL;

        /*  Handle interrupt signals                                         */
        handle_signals ();

        /*  On some systems we get a 'broken pipe' when a connection fails   */
#       if defined (SIGPIPE)
        signal (SIGPIPE, SIG_IGN);
#       endif

        /*  We pass through handle_atexit() before closing-down              */
        atexit (handle_atexit);

        smt_alive = TRUE;               /*  SMT kernel is now active         */
      }
    return (feedback);
}

static void
handle_atexit (void)
{
    if (smt_alive)
        smt_term ();
}


/*  -------------------------------------------------------------------------
 *  handle_signals -- internal
 *
 *  Directs all handlable signals to handle_signal.
 */

local
handle_signals (void)
{
    signal_raised    = FALSE;
    shutdown_request = FALSE;

    signal (SIGINT,  handle_signal);
    signal (SIGTERM, handle_signal);
    signal (SIGSEGV, handle_signal); 
}


/*  -------------------------------------------------------------------------
 *  handle_signal -- internal
 *
 *  Sets signal_raised to TRUE, and stores the signal value.  The signal
 *  is handled by the deliver_events() function the next time it is called.
 *  We do not try to handle the signal directly here because it is unsafe
 *  to play with the kernel's linked lists in an interrupt handler.
 */

local
handle_signal (int the_signal)
{
    static time_t
        last_interrupt = 0;             /*  To trap looping code             */

#if (defined (SMT_TRACE))
    trace ("handle_signal: %d", the_signal);
#endif
    /*  A normal shutdown should not take more than X seconds.  If we get    */
    /*  a second signal after that time, we can assume the application is    */
    /*  looping and will not halt normally; we can then abort it.  We do     */
    /*  not abort for alarm signals.                                         */
    if (last_interrupt == 0)
        last_interrupt = time (NULL);
    if (time (NULL) - last_interrupt > SMT_LOOP_DETECTION)
      {
        coprintf ("Application looping - aborted");
        abort ();
      }
    /*  Store value of signal                                                */
    signal_raised = TRUE;
    switch (the_signal)
      {
        case SIGINT:
            signal_value     = SMT_SIGNAL_INT;
            shutdown_request = TRUE;
            break;
        case SIGTERM:
            signal_value     = SMT_SIGNAL_TERM;
            shutdown_request = TRUE;
            break;
        case SIGSEGV:
            /*  In principle it's not correct to do a printf inside a signal
             *  handler, but in this case the program is just about to die.
             *  Hey, how much more trouble can we get into?
             */
            fflush  (stderr);
            fprintf (stderr, "Fatal error in application - aborted\n");
            fprintf (stderr, "%s\n", smt_crash_report ());
            abort ();
      }
}


/*  ---------------------------------------------------------------------[<]-
    Function: smt_term

    Synopsis: Shuts-down the SMT.  Destroys all agents, methods, queues,
    events, threads.  Returns 0 if there were no errors, otherwise returns
    -1 and sets smt_errno to one of these values:
    <Table>
    SMT_NOTREADY             smt_init() was not called, or failed
    SMT_OUTOFMEMORY          Not enough heap memory left
    </Table>
    ---------------------------------------------------------------------[>]-*/

int
smt_term (void)
{
    LR_LOCATION *location;
    PTRITEM *ptritem;                   /*  Exit function address            */

#if (defined (SMT_TRACE))
    trace ("smt_term");
#endif
    if (!smt_alive)                     /*  If SMT API was not correctly     */
      {                                 /*    initialised, forget it         */
        smt_errno = SMT_NOTREADY;
        return (-1);
      }
      
    while (LR_location_cache)
      {
        location = LR_location_cache;
        LR_location_cache = LR_location_cache-> next;
        mem_free (location);
      }

#if (defined (SMT_PROFILE))
    save_profile_results ();
#endif

    while (! list_empty (& async_by_time))  /*  Removed scheduled funct'ns   */
        async_destroy ((ASYNC *) async_by_time. next);

    FORLIST (ptritem, exitfcts)         /*  Execute all exit functions       */
        (* (function) ptritem-> pointer) ();
    list_destroy (&exitfcts);

    agent_destroy (NULL);               /*  Free all agents                  */
    semaph_destroy (NULL);              /*  Free all semaphores              */
    sym_delete_table (dict);

    smt_alive = FALSE;                  /*  SMT is now disactivated          */
    return (0);
}


#if (defined (SMT_PROFILE))
static void
save_profile_results (void)
{
    AGENT   *agent;
    state_t state;
    event_t event;
    word    index;
    word    module;
    char    buffer [LINE_MAX + 1];

    XML_ITEM
        *xml_root,
        *xml_agent,
        *xml_state,
        *xml_event,
        *xml_module;

    xml_root = xml_new (NULL, "root", "");
    FORLIST (agent, agents)
      {
        xml_agent = xml_new (xml_root, "agent", "");
        xml_put_attr (xml_agent, "name", agent-> name);
        for (state = 0; state < agent-> maxstate; state++)
          {
            xml_state = xml_new (xml_agent, "state", "");
            xml_put_attr (xml_state, "name", agent-> LR_sname [state]);
            for (event = 0; event < agent-> maxevent; event++)
              {
                index = agent-> LR_action [state * agent-> maxevent + event];
                if (index)
                  {
                    xml_event = xml_new (xml_state, "event", "");
                    xml_put_attr (xml_event, "name", agent-> LR_ename [event]);
                    sprintf (buffer, "%u", agent-> LR_flow [state * agent-> maxevent + event]);
                    xml_put_attr (xml_event, "count", buffer);
                  }
              }
          }
        for (module = 0; module < agent-> maxmodule; module++)
          {
            xml_module = xml_new (xml_agent, "module", "");
            xml_put_attr (xml_module, "name", agent-> LR_mname [module]);

            sprintf (buffer, "%u", agent-> LR_count [module]);
            xml_put_attr (xml_module, "count",  buffer);
            sprintf (buffer, "%lu", agent-> LR_time [module]);
            xml_put_attr (xml_module, "time",  buffer);
          }
      }
    xml_save_file (xml_root, "profile.xml");
    xml_free (xml_root);
}
#endif


/*  ---------------------------------------------------------------------[<]-
    Function: smt_exec_full

    Synopsis: Executes the current set of threads until there is nothing
    left to do.

    This function is deprecated.  Use 'smt_exec (SMT_EXEC_FULL)'
    ---------------------------------------------------------------------[>]-*/
int
smt_exec_full (void)
{
    smt_exec (SMT_EXEC_FULL);

    return 0;
}


/*  ---------------------------------------------------------------------[<]-
    Function: smt_exec_step

    Synopsis: Executes the current set of threads for one step.

    This function is deprecated.  Use 'smt_exec (SMT_EXEC_STEP)'
    ---------------------------------------------------------------------[>]-*/

Bool
smt_exec_step (void)
{
    return smt_exec (SMT_EXEC_STEP);
}


/*  ---------------------------------------------------------------------[<]-
    Function: smt_exec

    Synopsis: Executes the current set of threads.  Depending on value of
    argument 'exec_type', it stops after 1 step, after a blocking function
    would be called, or after there is nothing else left to do.

    Returns TRUE if there is more to do, FALSE otherwise.

    This function runs in a tight loop; for performance reasons it is thus
    long rather than split into smaller functions.
    ---------------------------------------------------------------------[>]-*/

Bool smt_exec (SMT_EXEC_TYPE exec_type)
{
    Bool
        break_for_async = FALSE;
    long
        cur_date,
        cur_time,
        first_date,
        first_time;
    ASYNC
        *async;                         /*  Asynchronous function info       */
    THREAD
        *thread;
    AGENT
        *agent;
    word
        module;
    int
#if (defined (SMT_LOOP_MAX))
        i,
#endif
        rc;
#if (defined (SMT_PROFILE))
    long
        time;
    struct tms                          /*  Warning: non-portable  */
        proctimes;
#endif

    /*  Initialise a few things so it doesn't have to be done inside         */
    /*  the tight loop.                                                      */
    _exception_raised = FALSE;
    _dialog_jump = FALSE;
    break_wanted      = BREAK_CONTINUE;

    /*  The big loop...  */
    FOREVER
      {

#if (defined (SMT_TRACE))
        trace ("smt_exec_step");
#endif

        /*  Look at polling functions.  We update the "togo" count on each   */
        /*  polling function and call the ones that are due to be run.  If   */
        /*  any are flagged to be removed, we tidy the list to remove them.  */
        if (active_pollfns > 0)
          {
            int i;

            for (i = 0; i < active_pollfns; ++i)
              {
                if (pollfns [i]. function == NULL)
                  {
                    /*  Function pointer is null; function has been flagged  */
                    /*  to be removed.  We move last one to this position.   */
                    if (i < (active_pollfns - 1))
                      {
                        memcpy (&(pollfns [i]),
                                &(pollfns [active_pollfns - 1]),
                                sizeof (POLL));
                      }

                    --active_pollfns;
                    --i;                    /*  Process this position again */
                    continue;
                  }

                if (pollfns [i]. togo > 0)
                  {
                    if (--pollfns [i]. togo <= 0)
                      {
                        /*  Function is due to be actived; has reached 0.    */
                        /*  Reset its togo counter, and then call it.        */
                        pollfns [i]. togo = pollfns [i]. interval;
                    
                        smt_memtrn_ = pollfns [i]. memtrn;
                        rc = pollfns [i]. function (pollfns [i]. interval, 
                                                    pollfns [i]. arg);
                        smt_memtrn_ = NULL;

                        if (rc < 0)
                          {
                            coprintf ("Polling function error.");
                            request_all_threads_shutdown ();
                            break;
                          }
                      }
                  }
              }
          }
        /*  Handle shutdown, if requested.  */
        if (shutdown_request)
            request_all_threads_shutdown ();

        /*  Look at asynchronous functions.  This involves several steps:    */
        /*      1. Remove de-scheduled functions from list.                  */
        if (async_removed)
          {
            FORLIST (async, async_by_time)
                if (async-> function == NULL)
                  {
                    async = async-> prev;
                    async_destroy (async-> next);
                  }
        
            async_removed = FALSE;
          }

        /*      2. Execute and deschedule those functions whose scheduled    */
        /*         time has come and which have a priority no less than that */
        /*         of the executing thread.  This includes those with a      */
        /*         scheduled time of zero.                                   */
        cur_date = 0;
        FORLIST (async, async_by_time)
          {
            /*  Optimise on time calls.  */
            if ((async-> next_date)
            &&  (async-> next_date != 99999999)
            &&  (cur_date == 0))
                get_date_time_now (& cur_date, & cur_time);

            if ((async-> next_date != 99999999)
            &&  (timege (cur_date, cur_time,
                         async-> next_date, async-> next_time)))
              {
                /*  We do this inside the loop because the function calls    */
                /*  may change the active thread.                            */
                thread = active_threads.right;
                if (thread == & active_threads
                ||  thread-> queue-> agent-> priority <= async-> priority)
                  {
                    smt_memtrn_ = async-> memtrn;
                    rc = async-> function (0, 0, async-> arg);
                    smt_memtrn_ = NULL;
                    async = async-> prev;
                    async_destroy (async-> next);

                    if (rc < 0)
                      {
                        coprintf ("Asynchronous function error.");
                        request_all_threads_shutdown ();
                        break;
                      }
                  }
              }
            else
                break;
          }

        /*  3. If no active threads, execute and deschedule those functions  */
        /*     with a scheduled date of 99999999.                            */
        if (active_threads.right == & active_threads)
          {
            for ( ; (void *) async != & async_by_time; async = async-> next)
                if (async-> next_date == 99999999)
                  {
                    smt_memtrn_ = async-> memtrn;
                    rc = async-> function (0, 0, async-> arg);
                    smt_memtrn_ = NULL;
                    async = async-> prev;
                    async_destroy (async-> next);

                    if (rc < 0)
                      {
                        coprintf ("Asynchronous function error.");
                        request_all_threads_shutdown ();
                        break;
                      }

                    /*  Again check in loop in case active thread changes.   */
                    if (active_threads.right != & active_threads)
                        break;
                  }
          }
        /*  Handle shutdown, if requested.  */
        if (shutdown_request)
            request_all_threads_shutdown ();

        /*  Having dealt with asynchronous processing, do syncronous         */
        /*  (dialog) processing.                                             */

        /*  Find thread to execute.  Skip processing if none is active.  */
        thread = active_threads.right;
        if (thread != &active_threads)
          {
            agent = thread-> queue-> agent;

#if (defined (SMT_DETECT))
            cur_agent = agent-> name;       /*  Agent name for error tracking    */
#endif

            /*  If first time, initialise_the_thread for initial event   */
            if (thread-> LR_state == SMT_NULL_STATE)
              {
                initialise_the_thread (thread);
                if (! lookup_next_event (thread))
                    continue;

                thread-> LR_event  = thread-> the_next_event;
                thread-> LR_number = 0;
                if (! lookup_module_vector (thread))
                    suspend_thread (thread, SMT_THREAD_WAIT_EVENT);
              }

#if (defined (SMT_DETECT))
            /*  Keep track of current agent/state/event for err tracking */
            cur_state  = agent-> LR_sname [thread-> LR_state];
            cur_event  = agent-> LR_ename [thread-> LR_event];
#endif

            while (thread == active_threads.right)
              {
                module = agent-> LR_vector
                    [agent-> LR_offset [thread-> LR_index]
                                      + thread-> LR_number];

                /*  If at end of module list, change state, and thread.  */
                if (module == SMT_ACTION_STOP)
                  {
                    if (! lookup_next_event (thread))
                        break;

                    thread-> LR_state = agent-> LR_nextst
                        [ thread-> LR_state * agent-> maxevent
                        + thread-> LR_event];
                    thread-> LR_savest = thread-> LR_state;
                    thread-> LR_event  = thread-> the_next_event;
                    thread-> LR_number = 0;

#if (defined (SMT_LOOP_MAX))
                    thread-> loop_event [thread-> loop_index]
                        = thread-> LR_event;
                    if (thread-> loop_length)     /*  Already found loop     */
                      {
                        if (LOOP_EVENT (thread-> loop_index
                                      - thread-> loop_length)
                        ==  LOOP_EVENT (thread-> loop_index))
                          {
                            thread-> loop_count++;
                            if (thread-> loop_count
                            >=  SMT_LOOP_TRAP * thread-> loop_length)
                              {
                                coprintf ("Agent %s appears to be looping with events:",
                                          thread->queue->agent->name);
                                for (i = 0; i < thread-> loop_length; i++)
                                    coprintf ("   %s",
                                              thread-> queue-> agent-> LR_ename [LOOP_EVENT (thread-> loop_index - thread-> loop_length + i)]);
                                abort ();
                              }
                          }

                        else
                          {
                            thread-> loop_length = 0;
                            thread-> loop_count  = 0;
                          }
                      }
                    if (! thread-> loop_length)  /*  Try to find a loop  */
                      {
                        thread-> loop_length = 1;
                        while (thread-> loop_length <= SMT_LOOP_MAX)
                          {
                            for (i = 0; i < thread-> loop_length; i++)
                              {
                                if (LOOP_EVENT (thread-> loop_index - i - thread-> loop_length)
                                !=  LOOP_EVENT (thread-> loop_index - i))
                                    break;
                              }
                            if (i == thread-> loop_length)
                                break;

                            thread-> loop_length++;
                          }
                        if (thread-> loop_length > SMT_LOOP_MAX)
                            thread-> loop_length = 0;      /*  No loop  */
                      }

                    thread-> loop_index = (thread-> loop_index + 1)
                                        % (2 * SMT_LOOP_MAX);
#endif

                    if (lookup_module_vector (thread))
                        activate_thread (thread);
                    else
                        suspend_thread (thread, SMT_THREAD_WAIT_EVENT);

                    break;
                  }

                thread-> LR_number++;

                /*  Take events from thread context, for duration of function*/
                _the_next_event      = thread-> the_next_event;
                _the_external_event  = thread-> the_external_event;

                /*  Prepare to call module  */
                _io_completed     = TRUE;

#if (defined (SMT_DETECT))
                /*  Keep track of current agent/state/event for err tracking */
                cur_module = agent-> LR_mname [module];
                cur_step   = "";
#endif

                if (agent-> animate && thread-> animate)
                    coprintf ("%-10s %4ld>           + %s",
                              agent-> name,
                              thread-> thread_id,
                              agent-> LR_mname [module]);

#if (defined (SMT_PROFILE))
                times (&proctimes);
                time = proctimes. tms_utime;
#endif

                /*  Execute the module  */
                smt_memtrn_ = thread-> memtrn;
                (agent-> LR_module [module]) (thread);
                smt_memtrn_ = NULL;
#if (defined (SMT_PROFILE))
                times (&proctimes);
                time = proctimes. tms_utime - time;
        
                agent-> LR_count [module]++;
                agent-> LR_time  [module] += time;
#endif

                /*  Replace thread copy of the events.  */
                thread-> the_next_event      = _the_next_event;
                thread-> the_external_event  = _the_external_event;
        
                /*  Now handle whatever happened during the module call.  */
                if (_exception_raised)
                  {
                    _exception_raised = FALSE;

                    thread-> LR_event = _the_exception_event;

                    if (agent-> animate && thread-> animate)
                        coprintf ("%-10s %4ld>     (=>) %s",
                                  agent-> name, thread-> thread_id,
                                  agent-> LR_ename [_the_exception_event]);

                    /*  Restore state in case we were in the defaults state  */
                    thread-> LR_state = thread-> LR_savest;
                    thread-> LR_number = 0;

                    _the_exception_event = SMT_NULL_EVENT;

                    /*  Prepare new module list */
                    if (! lookup_module_vector (thread))
                        suspend_thread (thread, SMT_THREAD_WAIT_EVENT);

                    break;
                  }
                else
                if (break_wanted != BREAK_CONTINUE)
                  {
                    switch (break_wanted)
                      {
                        case BREAK_WAIT_SEMAPH:
                            suspend_thread (thread, SMT_THREAD_WAIT_SEMAPH);
                            thread_relink_before (thread,
                                                  &break_semaph-> threads);
                            break;
                        case BREAK_WAIT_EVENT:
                            suspend_thread (thread, SMT_THREAD_WAIT_EVENT);
                            break;
                        case BREAK_RECYCLE:
                            thread-> LR_number--;
                            activate_thread (thread);
                            break;
                        default:
                            break;
                      }
                    break_wanted = BREAK_CONTINUE;
                  }
                else
                if (_dialog_jump)
                  {
                    _dialog_jump = FALSE;
                    if (! lookup_module_vector (thread))
                        suspend_thread (thread, SMT_THREAD_WAIT_EVENT);
                    break;
                  }
                else
                  {
                    async = async_by_priority. right;
                    cur_date = 0;
                    while (async != & async_by_priority
                       &&  async-> function
                       &&  async-> priority > agent-> priority)
                      {
                        /*  Optimise on time calls.  */
                        if (async-> next_date
                        && (cur_date == 0))
                            get_date_time_now (& cur_date, & cur_time);

                        if (timege (cur_date, cur_time,
                                    async-> next_date, async-> next_time))
                          {
                            break_for_async = TRUE;
                            break;
                          }

                        async = async-> right;
                      }

                    if (break_for_async)
                      {
                        break_for_async = FALSE;
                        break;
                      }
                  }
              }
          }

        /*  Handle shutdown, if requested.  */
        if (shutdown_request)
            request_all_threads_shutdown ();

        /*      4. If execution would stop, call blocking function with      */
        /*         date/time of first scheduled function.                    */
        if (active_threads.right == &active_threads)
          {
            first_date = 99999999;
            first_time = 99999999;
            FORLIST (async, async_by_time)
                if (async-> function && async-> next_date)
                  {
                    first_date = async-> next_date;
                    first_time = async-> next_time;
                    break;
                  }

            if (waitfn)
              {
                if (exec_type == SMT_EXEC_NOBLOCK)
                    return TRUE;

                smt_memtrn_ = waittrn;
                rc = waitfn (first_date,
                             first_time,
                             waitarg);

                smt_memtrn_ = NULL;

                /*  Handle shutdown, if requested.  */
                if (shutdown_request)
                    request_all_threads_shutdown ();
                else
                /*  Catch the condition where nothing is happening.  */
                if (rc == 0
                &&  first_date == 99999999)
                    return FALSE;

                if (rc < 0)
                  {
                    coprintf ("Blocking function error.");
                    request_all_threads_shutdown ();
                  }
              }
            else
                /*  No active threads, no wait function -> stop.  */
                return FALSE;
          }
        if (exec_type == SMT_EXEC_STEP)
            return TRUE;
      }
}


static void
async_destroy (ASYNC *async)
{
    async-> left-> right = async-> right;
    async-> right-> left = async-> left;
    mem_free (list_unlink (async));
}


static void
request_all_threads_shutdown (void)
{
    AGENT   *agent;                     /*  Agent information block          */
    QUEUE   *queue;                     /*  Queue information block          */

    FORLIST (agent, agents)
        /*  Process each queue in the agent                                  */
        FORLIST (queue, agent-> queues)
            if (!queue-> shutdown)
              {
                queue-> shutdown = TRUE;
                lsend_smtlib_shutdown (
                    &queue-> qid,
                    NULL,               /*  No queue for reply               */
                    NULL, NULL, NULL, 0,
                    (word) signal_value);
              }

    /*  Get rid of scheduled asynchronous functions.  */
    while (! list_empty (& async_by_time))  /*  Removed scheduled funct'ns   */
        async_destroy ((ASYNC *) async_by_time. next);

    waitfn = NULL;

    shutdown_request = FALSE;           /*  Any SHUTDOWNs were delivered     */
}


static void
initialise_the_thread (THREAD *thread)
{
#if (defined (SMT_LOOP_MAX))
    int i;
#endif
    /*  Take events from thread context, for duration of function            */
    _the_next_event      = thread-> the_next_event;
    _the_external_event  = thread-> the_external_event;

#if (defined (SMT_DETECT))
    /*  Keep track of current agent/state/event for error tracking           */
    cur_state  = "Initialising";
    cur_event  = "(None)";
    cur_module = "Initialise-The-Thread";
#endif

    _the_next_event = SMT_NULL_EVENT;
    (thread-> queue-> agent-> initialise) (thread);
    thread-> LR_state = 0;              /*  First state is zero              */
    thread-> LR_index = 0;              /*  No module list yet               */
    thread-> LR_event = SMT_NULL_EVENT;

    thread-> LR_savest = thread-> LR_state;

    /*  Replace thread copy of the events                                    */
    thread-> the_next_event     = _the_next_event;
    thread-> the_external_event = _the_external_event;

#if (defined (SMT_LOOP_MAX))
    /*  Initialise loop detection variables.                                 */
    if (! thread-> loop_event)
        thread-> loop_event = mem_alloc (2 * SMT_LOOP_MAX * sizeof (event_t *));
    for (i = 0; i < 2 * SMT_LOOP_MAX; i++)
        thread-> loop_event [i] = SMT_NULL_EVENT;
    thread-> loop_index  = 0;
    thread-> loop_length = 0;
    thread-> loop_count  = 0;
#endif

}


static inline Bool
lookup_module_vector (THREAD *thread)
{
    AGENT
        *agent = thread-> queue-> agent;
    int
        i, j;

    /*  Look up module list  */
    i = thread-> LR_state * agent-> maxevent + thread-> LR_event;

    thread-> LR_index = agent-> LR_action [i];

    /*  If no action for this event, try the defaults state.  At the same    */
    /*  timme copy the action and next state info from the defaults state    */
    /*  to the current state, so next time through this will go faster.      */
    if ((thread-> LR_index == 0)
    &&  ( agent-> LR_defaults != SMT_NULL_STATE))
      {
        j = agent-> LR_defaults * agent-> maxevent + thread-> LR_event;
        thread-> LR_index = agent-> LR_action [j];
        agent-> LR_action [i] = thread-> LR_index;
        /*  Be careful to detect undefined next state in defaults state.     */
        if (agent-> LR_nextst [j] == agent-> LR_defaults)
            agent-> LR_nextst [i] = thread-> LR_state;
        else
            agent-> LR_nextst [i] = agent-> LR_nextst [j];
      }

    /*  If we still did not get a valid index, event is bad      */
    if (thread-> LR_index == 0)
      {
        coprintf ("%s: state %s - event %s is not accepted",
                  agent-> name,
                  agent-> LR_sname [thread-> LR_savest],
                  agent-> LR_ename [thread-> LR_event]);

        shutdown_request = TRUE;
        return FALSE;
      }

    if (agent-> animate && thread-> animate)
      {
        coprintf ("%-10s %4ld> %s:",
                  agent-> name,
                  thread-> thread_id,
                  agent-> LR_sname [thread-> LR_state]);
        coprintf ("%-10s %4ld>     (%s) %s",
                  agent-> name,
                  thread-> thread_id,
                  thread-> LR_number == 0 ? "--" : "..",
                  agent-> LR_ename [thread-> LR_event]);
      }

#if (defined (SMT_PROFILE))
    agent-> LR_flow [i]++;
#endif

    thread-> the_next_event = SMT_NULL_EVENT;

    return TRUE;
}


static inline Bool
lookup_next_event (THREAD *thread)
{
    AGENT
        *agent = thread-> queue-> agent;

    /*  If no event was supplied see if there's one waiting.     */
    /*  If note, suspend.                                        */
    if (thread-> the_next_event == SMT_NULL_EVENT)
      {
        if (! queue_deliver (thread-> queue, thread))
          {
            suspend_thread (thread, SMT_THREAD_PASSIVE);
            return FALSE;
          }
      }
    else
    /*  Thread gets deleted if it terminated                 */
    if (thread-> the_next_event == SMT_TERM_EVENT)
      {
        thread_destroy (thread, TRUE);
        return FALSE;
      }

    /*  Check that event is OK                                   */
    if ((thread-> the_next_event > agent-> maxevent)
    ||  (thread-> the_next_event < 0))
      {
        coprintf ("%s: state %s - event is out of range",
                  agent-> name,
                  agent-> LR_sname [thread-> LR_state]);
        
        shutdown_request = TRUE;
        return FALSE;
      }

    return TRUE;
}


/*  -------------------------------------------------------------------------
 *  activate_thread -- internal
 *
 *  Moves the thread onto the active list, according to the priority of the
 *  thread agents.   Sets the thread state to SMT_THREAD_ACTIVE.
 */

static void
activate_thread (THREAD *thread)
{
    THREAD  *active;                    /*  Thread in active list            */
    int     priority;                   /*  Agent priority                   */

    /*  Find first thread with lower priority                                */
    /*  We may end-up with active pointing to the list head                  */
    priority = thread-> queue-> agent-> priority;
    for (
    active  = active_threads.right;
    active != &active_threads;
    active  = active-> right)
        if (priority > active-> queue-> agent-> priority)
            break;

    thread_unlink (thread);
    thread_relink_before (thread, active);
    thread-> state = SMT_THREAD_ACTIVE;
    thread-> queue-> agent-> switch_tally++;
}


/*  -------------------------------------------------------------------------
 *  suspend_thread -- internal
 *
 *  Moves the thread off the active list, frees any allocated event, and
 *  sets a new state as specified.
 */

static void
suspend_thread (THREAD *thread, int state)
{
#if (defined (SMT_LOOP_MAX))
    int i;
#endif
    if (thread-> event)                 /*  If thread was sitting on an      */
      {                                 /*    event, release it              */
        event_destroy (thread-> event);
        thread-> event = NULL;
      }
    thread-> state = state;
#if (defined (SMT_LOOP_MAX))
    /*  If thread goes inactive, all loops are off  */
    for (i = 0; i < 2 * SMT_LOOP_MAX; i++)
        thread-> loop_event [i] = SMT_NULL_EVENT;
    thread-> loop_index  = 0;
    thread-> loop_length = 0;
    thread-> loop_count  = 0;
#endif
    thread_unlink (thread);             /*  Remove thread from active list   */

    if (state == SMT_THREAD_WAIT_EVENT)
        thread_relink_before (thread, &waiting_threads);
}


/*  ---------------------------------------------------------------------[<]-
    Function: smt_set_console

    Synopsis: Tells the SMT kernel to send error events to the specified
    console queue.  There can be just one console queue.  If you do not
    specify a console queue, error events are discarded.
    ---------------------------------------------------------------------[>]-*/

void
smt_set_console (const QID *qid)
{
#if (defined (SMT_TRACE))
    trace ("smt_set_console");
#endif
    console = *qid;
}


/*  ---------------------------------------------------------------------[<]-
    Function: smt_atexit

    Synopsis: Registers a termination function.  The function is defined as
    a void function without arguments.  The termination functions are called
    in the order that they are declared.  Multiple instances of the same
    function are ignored.  Returns 0 if okay, -1 if there was an error.
    In the case of an error, sets smt_errno to one of:
    <Table>
    SMT_NOTREADY             smt_init() was not called, or failed
    SMT_OUTOFMEMORY          Not enough heap memory left
    </Table>
    The kernel executes termination functions before destroying agents and
    other objects.  Thus, termination functions can access the object symbol
    table ('lookup' functions), but not send or receive events.
    ---------------------------------------------------------------------[>]-*/

int
smt_atexit (function handler)
{
#if (defined (SMT_TRACE))
    trace ("smt_atexit");
#endif
    return add_unique_pointer ((void *) handler, &exitfcts);
}


int
add_unique_pointer (void *pointer, LIST *ptrlist)
{
    PTRITEM *ptritem;

    ASSERT (pointer);
    if (!smt_alive)                     /*  If SMT API was not correctly     */
      {                                 /*    initialised, forget it         */
        smt_errno = SMT_NOTREADY;
        return (-1);
      }

    /*  Check that exit function is not already defined; if so, ignore it    */
    FORLIST (ptritem, *ptrlist)
        if (ptritem-> pointer == pointer)
            return (0);

    /*  Allocate an PTRITEM block and attach it to the list                  */
    list_create (ptritem, sizeof (PTRITEM));
    if (ptritem == NULL)
      {
        smt_errno = SMT_OUTOFMEMORY;
        return (-1);
      }
    list_relink_before (ptritem, ptrlist);

    ptritem-> pointer = pointer;
    return (0);
}


/*  ---------------------------------------------------------------------[<]-
    Function: smt_shutdown

    Synopsis: Ends the current SMT application.  Use this if you detect a
    fatal error in a thread.  Sends a SHUTDOWN event to every thread, so
    halting the application.
    ---------------------------------------------------------------------[>]-*/

void
smt_shutdown (void)
{
    shutdown_request = TRUE;
    signal_value  = SMT_SIGNAL_USER;    /*  Kernel takes it from here        */
}


/*  ---------------------------------------------------------------------[<]-
    Function: schedule_async_nonblock

    Synopsis: Schedules a non-blocking asynchronous function call.  This
    is used to specify a function to be called  asynchronously by the dialog
    manager.  It can be used to send events, thus providing an asynchronous 
    way of communicating with a dialog.  The specified function may not block.

    Many different nonblocking asynchronous functions may be scheduled.

    The function will be called after the specified date and time has been
    reached.  If the specified date is zero, the function will be called as
    often as possible (the meaning of this depends on the priority - see
    below).  If the specified date is 99999999, the function will only be
    called when all threads are idle.

    The function will not be called until the executing thread (ie the active
    thread with the greatest priority) has priority less than or equal to that
    of the asyncronous function.  If the function has greater priority than
    the executing thread the date/time is checked after each module.  If the
    priorities are equal, testing occurs when the thread changes state.

    Returns 0 if there were no errors, or -1, after seting smt_errno to one
    of these values:
    <Table>
    SMT_OUTOFMEMORY          Not enough heap memory left
    </Table>
    ---------------------------------------------------------------------[>]-*/

int
schedule_async_nonblock (ASYNCFN *asyncfn, void *arg,
                         int priority, long date, long time)
{
    ASYNC
        *async,
        *next,
        *right;

#if (defined (SMT_TRACE))
    trace ("register_async_nonblock");
#endif

    ASSERT (asyncfn);
    if (!smt_alive)                     /*  If SMT API was not correctly     */
      {                                 /*    initialised, forget it         */
        smt_errno = SMT_NOTREADY;
        return (-1);
      }

    /*  Sort scheduled list by date/time.  */
    FORLIST (next, async_by_time)
        if (timege (next-> next_date, next-> next_time,
                    date, time))
            break;

    /*  Sort scheduled list by priority/date/time.  */
    for (right = async_by_priority. right;
         right != & async_by_priority;
         right = right-> right)
      {
        if (right-> priority  <  priority
        || (right-> priority  == priority
        &&  timege (right-> next_date, right-> next_time,
                    date, time)))
            break;
      }

    list_create (async, sizeof (ASYNC));
    if (async == NULL)
      {
        smt_errno = SMT_OUTOFMEMORY;
        return -1;
      }

    /*  Link async function in two lists.  */
    list_relink_before (async, next);

    async-> left = right-> left;
    right-> left = async;
    async-> right = async-> left-> right;
    async-> left-> right = async;

    async-> function  = asyncfn;
    async-> arg       = arg;
    async-> priority  = priority;
    async-> next_date = date;
    async-> next_time = time;
    async-> memtrn    = smt_memtrn_;

    return 0;
}

/*  ---------------------------------------------------------------------[<]-
    Function: deschedule_async_nonblock

    Synopsis: Deschedules non-blocking asynchronous function calls.  This
    flags any entries on the async list which match the function pointer _and_
    argument to be ignored, and removed on the next processing of the async
    list.  We do not modify the list at this stage so that we can be safely
    called from an function called by the async processing without completely
    messing up the list (due to aliasing issues).

    This function should be used when an async call back is no longer required
    for some reason.

    Returns the number of async callbacks that were flaged; 0 means "none
    found", and 1 is the expected value, more may indicate errors.

    ---------------------------------------------------------------------[>]-*/

int
deschedule_async_nonblock (ASYNCFN *asyncfn, void *arg)
{
    ASYNC
        *async;
    int
         flagged = 0;

    FORLIST (async, async_by_time)
      {
        if (async-> function == asyncfn && async-> arg == arg)
          {
            async-> function = NULL;       /*  Flag: not active              */
            async_removed    = TRUE;       /*  Do delete processing later    */
            ++flagged;
          }
      }

    return (flagged);
}

/*  ---------------------------------------------------------------------[<]-
    Function: register_async_blocking

    Synopsis: Registers a blocking asynchronous function call.  This is used
    to specify a function to be called by the dialog manager when no other
    activiy is occurring.  If a null pointer is specified, no function is
    registered, otherwise the specified function is registerd and any
    previously registered function is no longer registered.

    Returns 0 if there were no errors.
    ---------------------------------------------------------------------[>]-*/

void
register_async_blocking (ASYNCFN *asyncfn, void *arg)
{
    waitfn  = asyncfn;
    waitarg = arg;
    waittrn = smt_memtrn_;
}

/*  ---------------------------------------------------------------------[<]-
    Function: schedule_polling_function

    Synopsis: Schedules a polling function to be called asynchronously 
    after every poll_interval static changes have occured.  It can be used
    to send events, thus providing an asynchronous way of communicating with
    a dialog.  The specified function may not block.

    Many different nonblocking polling functions may be scheduled.

    The function will be called every poll_interval state changes.  It may
    alter the poll_interval state changes by reschueduling itself.  (This
    will change the scheduled interval, not schedule an additional call.)

    Returns 0 if there were no errors, or -1, after seting smt_errno to one
    of these values:
    <Table>
    SMT_OUTOFMEMORY          Not enough room left in the list
    </Table>
    ---------------------------------------------------------------------[>]-*/

int
schedule_polling_function (POLLFN *pollfn, void *arg, 
                           int poll_interval)
{
    int
            i;

#if (defined (SMT_TRACE))
    trace ("schedule_polling_function");
#endif

    ASSERT (pollfn);
    if (! pollfn)
      {
        smt_errno = SMT_AGENTNOTREADY;  /*  No polling function=> cannot call*/
        return -1;
      }

    if (!smt_alive)                     /*  If SMT API was not correctly     */
      {                                 /*    initialised, forget it         */
        smt_errno = SMT_NOTREADY;
        return (-1);
      }

    /*  Search the polling functions list for a function with the same       */
    /*  function pointer, and argument.  If found we update the interval.    */
    for (i = 0; i < active_pollfns; ++i) 
      {
        if (pollfns [i]. function == pollfn
        &&  pollfns [i]. arg      == arg)
          {
            pollfns [i]. interval = poll_interval;
            pollfns [i]. togo     = poll_interval;
            return 0;
          }
      }

    /*  If we reach this point, it is a new polling function, so we schedule */
    /*  it at the end of the list, if there is still room.                   */
    if (active_pollfns < (MAX_POLLFNS - 1))
      {
        pollfns [active_pollfns]. function = pollfn;
        pollfns [active_pollfns]. arg      = arg;
        pollfns [active_pollfns]. interval = poll_interval;
        pollfns [active_pollfns]. togo     = poll_interval;
        pollfns [active_pollfns]. memtrn   = smt_memtrn_;
        ++active_pollfns;
      }
    else
      {
        smt_errno = SMT_OUTOFMEMORY;
        return -1;
      }

    return 0;
}

/*  ---------------------------------------------------------------------[<]-
    Function: deschedule_polling_function

    Synopsis: Deschedules a polling_function, so that it will not be called
    again.  This flags a entry on the polling list which matches the function 
    pointer _and_ argument to be ignored, and removed on the next processing 
    of the polling list.  We do not modify the list at this stage so that 
    we can be safely called from an function called by the polling processing 
    without messing up the list (due to aliasing issues).

    This function should be used when an polling function call back is no 
    longer required at all for some reason (eg, thread that was using it 
    ending).

    Returns 1 if the function was found, and 0 if it was not.

    ---------------------------------------------------------------------[>]-*/

int
deschedule_polling_function (POLLFN *pollfn, void *arg)
{
    int
        i,
        flagged = 0;

#if (defined (SMT_TRACE))
    trace ("deschedule_polling_function");
#endif

    for (i = 0; i < active_pollfns; ++i)
      {
        if (pollfns [i]. function == pollfn 
        &&  pollfns [i]. arg      == arg)
          {
            pollfns [i]. function = NULL;  /*  Flag: not active              */
            ++flagged;
          }
      }

    return (flagged);
}

/*  ---------------------------------------------------------------------[<]-
    Function: agent_declare

    Synopsis: Declares a new agent. Typically you'll do this when you are
    initialising a agent. You must declare the agent before you can
    create queues, threads, or methods for that agent.

    The agent name is an arbitrary text, unique within the application.
    Returns the address of the created AGENT block. If there was an error,
    returns NULL and sets smt_errno to one of these values:
    <Table>
    SMT_NOTREADY             smt_init() was not called, or failed
    SMT_OUTOFMEMORY          Not enough heap memory left
    SMT_AGENTEXISTS          A agent with this name is already declared
    </Table>

    Agents and threads are stored in the dictionary as follows:
    a name is built of three parts: s~agent~[thread].   This name
    is the key into the dictionary and lets us find a agent, or
    thread by name.  Then, all agents are chained into a linked list that
    is attached to the agents list. Each agent has a sublist of queues,
    and each queue has a sublist of threads.  Each thread
    has a pointer to the parent queue respectively.  This
    cross-linking lets us browse the list of agents/threads from
    any point.  Names are always stored in lower-case.

    Sets agent priority to SMT_PRIORITY_NORMAL; sets router flag to FALSE,
    and max_threads to 0.
    ---------------------------------------------------------------------[>]-*/

AGENT *
agent_declare (
    const char *agent_name              /*  Name of agent to declare         */
)
{
    SYMBOL  *dict_entry;                /*  Dictionary symbol                */
    AGENT   *agent;                     /*  Agent information block          */
    char    *full_name;                 /*  Full agent name                  */

#if (defined (SMT_TRACE))
    trace ("agent_declare: agent=%s", agent_name);
#endif
    ASSERT (agent_name);
    if (!smt_alive)                     /*  If SMT API was not correctly     */
      {                                 /*    initialised, forget it         */
        smt_errno = SMT_NOTREADY;
        return (NULL);
      }

    /*  Check that agent is not already declared                             */
    full_name = get_entity_name (agent_name, NULL);
    if (sym_lookup_symbol (dict, full_name))
      {
        smt_errno = SMT_AGENTEXISTS;
        return (NULL);
      }

    /*  Now create entry for the agent                                       */
    dict_entry = sym_create_symbol (dict, full_name, NULL);
    if (dict_entry == NULL)
      {
        smt_errno = SMT_OUTOFMEMORY;
        return (NULL);
      }

    /*  Allocate an AGENT block and attach it to the agent list              */
    list_create (agent, sizeof (AGENT));
    if (agent == NULL)
      {
        sym_delete_symbol (dict, dict_entry);
        smt_errno = SMT_OUTOFMEMORY;
        return (NULL);
      }
    list_relink_before (agent, &agents);

    /*  Point the dictionary entry to the agent information block            */
    dict_entry-> data = agent;

    /*  Now initialise the agent - all fields are already set to zero        */
    list_reset (&agent-> methods);
    list_reset (&agent-> queues);
    agent-> name         = mem_strdup (agent_name);
    agent-> router       = FALSE;
    agent-> priority     = SMT_PRIORITY_NORMAL;
    agent-> max_threads  = 0;
    agent-> cur_threads  = 0;
    agent-> top_threads  = 0;
    agent-> thread_tally = 0;
    agent-> switch_tally = 0;
    agent-> symbol       = dict_entry;

    /*  These fields must be set by the calling program                      */
    agent-> tcb_size     = 0;           /*  Size of thread context block     */
    agent-> initialise   = NULL;        /*  Initialise-the-thread            */
    agent-> LR_nextst    = NULL;        /*  Next state table                 */
    agent-> LR_action    = NULL;        /*  Action table                     */
    agent-> LR_offset    = NULL;        /*  Vector offset table              */
    agent-> LR_vector    = NULL;        /*  Vector table                     */
    agent-> LR_module    = NULL;        /*  Module table                     */
    agent-> LR_defaults = 0;            /*  Defaults state                   */
    agent-> maxevent     = 0;           /*  Number of events defined         */
    agent-> maxmodule    = 0;           /*  Number of modules defined        */
    agent-> maxstate     = 0;           /*  Number of states defined         */

    /*  These fields may be changed by the calling program                   */
    agent-> LR_mname     = NULL;        /*  Module name table (if animated)  */
    agent-> LR_sname     = NULL;        /*  State name table (if animated)   */
    agent-> LR_ename     = NULL;        /*  Event name table (if animated)   */
    agent-> animate      = FALSE;       /*  Agent animation enabled          */

    return (agent);
}


/*  -------------------------------------------------------------------------
 *  get_entity_name -- internal
 *
 *  Returns a formatted agent/thread name given the two parts
 *  of the name.  This is used for lookups into the dictionary.  The
 *  returned name is in a static area that is overwritten by each call.
 *  The name is always converted to lower-case.
 */

static char
*get_entity_name (const char *agent_name, const char *thread_name)
{
    static
      char  full_name [LINE_MAX + 1];

    if (thread_name)                    /*  Thread name specified            */
      {
        ASSERT ((strlen (agent_name) + strlen (thread_name) + 3) <= LINE_MAX);
        sprintf (full_name, "s~%s~%s", agent_name, thread_name);
      }
    else                                /*  Just the agent name              */
      {
        ASSERT ((strlen (agent_name) + 2) <= LINE_MAX);
        sprintf (full_name, "s~%s", agent_name);
      }
    strlwc  (full_name);
    return  (full_name);
}


/*  ---------------------------------------------------------------------[<]-
    Function: agent_lookup

    Synopsis: Checks whether a specific agent exists; returns the address
    of the agent information block, or NULL if there was an error, setting
    smt_errno to one of these values:
    <Table>
    SMT_NOTREADY             smt_init() was not called, or failed
    SMT_NOSUCHAGENT          Specified agent was not declared
    </Table>
    ---------------------------------------------------------------------[>]-*/

AGENT *
agent_lookup (
    const char *agent_name              /*  Name of agent to look for        */
)
{
    SYMBOL  *dict_entry;                /*  Dictionary symbol                */

#if (defined (SMT_TRACE))
    trace ("agent_lookup: agent=%s", agent_name);
#endif
    if (!smt_alive)                     /*  If SMT API was not correctly     */
      {                                 /*    initialised, forget it         */
        smt_errno = SMT_NOTREADY;
        return (NULL);
      }

    dict_entry = sym_lookup_symbol (dict, get_entity_name (agent_name, NULL));
    if (dict_entry == NULL)
      {
         smt_errno = SMT_NOSUCHAGENT;
         return (NULL);
      }
    return (dict_entry-> data);         /*  Return pointer to AGENT          */
}


/*  ---------------------------------------------------------------------[<]-
    Function: agent_destroy

    Synopsis: Destroys the agent. Returns 0 when completed.   The agent
    argument points to an agent block, or is null. If null, all agents
    are destroyed.  Returns 0 when completed normally, else returns -1
    and sets smt_errno to one of these values:
    <Table>
    SMT_NOTREADY             smt_init() was not called, or failed
    </Table>
    ---------------------------------------------------------------------[>]-*/

int
agent_destroy (
    AGENT *agent                        /*  Agent to destroy; null = all     */
)
{
#if (defined (SMT_TRACE))
    trace ("agent_destroy: agent=%s", agent? agent-> name: "ALL");
#endif
    if (!smt_alive)                     /*  If SMT API was not correctly     */
      {                                 /*    initialised, forget it         */
        smt_errno = SMT_NOTREADY;
        return (-1);
      }

    if (agent == NULL)                  /*  Destroy all agents if wanted     */
        while (agents.next != &agents)
            agent_destroy (agents.next);
    else                                /*  Else destroy this agent          */
      {
        /*   Destroy all queues and methods declared for the agent           */
        while (agent-> queues.next != &agent-> queues)
            queue_destroy (agent-> queues.next);
        while (agent-> methods.next != &agent-> methods)
            method_destroy (agent-> methods.next);

        /*  We have to be a little careful or sym_delete_symbol () will      */
        /*  try to release the symbol's data area; the data area points      */
        /*  to our node, which we want to release ourselves.                 */
        agent-> symbol-> data = NULL;
        sym_delete_symbol (dict, agent-> symbol);

        /*  Now delete the agent                                             */
        mem_strfree (&agent-> name);    /*  First we take its name           */
        mem_free (list_unlink (agent)); /*  ... then we take its life        */
      }
    return (0);
}


/*  ---------------------------------------------------------------------[<]-
    Function: method_declare

    Synopsis: Declares a new method for an agent. All external events that
    an agent is prepared to method are declared as methods. The agent must
    already have been declared using agent_declare(). The method name is an
    arbitrary text, unique within the agent. The event number is the number
    of the event assigned by the dialog code generator; if you specify the
    event number as SMT_NULL_EVENT, the method is ignored.  This discards
    any incoming events with that name.  The priority may be 0 (meaning
    normal priority), SMT_PRIORITY_LOW, SMT_PRIORITY_HIGH, or another
    suitable value.  Returns the address of the created METHOD block.
    If there was an error, returns NULL and sets smt_errno to one of:
    <Table>
    SMT_NOTREADY             smt_init() was not called, or failed
    SMT_OUTOFMEMORY          Not enough heap memory left
    SMT_NOSUCHAGENT          Specified agent was not declared
    SMT_METHODEXISTS         Method is already declared
    </Table>
    ---------------------------------------------------------------------[>]-*/

METHOD *
method_declare (
    AGENT *agent,                       /*  Create method in this agent      */
    const char *method_name,            /*  Name of method to declare        */
    event_t event_number,               /*  Method number from dialog        */
    int     priority                    /*  Priority for the method, or 0    */
)
{
    SYMBOL  *dict_entry;                /*  Dictionary symbol                */
    METHOD  *method;                    /*  Method information block         */
    char    *full_name;                 /*  Full method name                 */

#if (defined (SMT_TRACE))
    trace ("method_declare: agent=%s method=%s", agent-> name, method_name);
#endif
    ASSERT (agent);
    ASSERT (method_name);
    if (!smt_alive)                     /*  If SMT API was not correctly     */
      {                                 /*    initialised, forget it         */
        smt_errno = SMT_NOTREADY;
        return (NULL);
      }

    /*  Check that method is not already declared                            */
    full_name = get_method_name (agent-> name, method_name);
    if (sym_lookup_symbol (dict, full_name))
      {
        smt_errno = SMT_METHODEXISTS;
        return (NULL);
      }

    /*  Now create entry for the method                                      */
    dict_entry = sym_create_symbol (dict, full_name, NULL);
    if (dict_entry == NULL)
      {
        smt_errno = SMT_OUTOFMEMORY;
        return (NULL);
      }

    /*  Allocate a METHOD block and attach it to the method list             */
    list_create (method, sizeof (METHOD));
    if (method == NULL)
      {
        sym_delete_symbol (dict, dict_entry);
        smt_errno = SMT_OUTOFMEMORY;
        return (NULL);
      }
    list_relink_before (method, &agent-> methods);

    /*  Point the dictionary entry to the method information block           */
    dict_entry-> data = method;

    /*  Now initialise the method - all fields are already set to zero       */
    method-> agent        = agent;
    method-> name         = mem_strdup (method_name);
    method-> priority     = priority? priority: SMT_PRIORITY_NORMAL;
    method-> event_number = event_number;
    method-> symbol       = dict_entry;

    return (method);
}


/*  -------------------------------------------------------------------------
 *  get_method_name -- internal
 *
 *  Returns a formatted method name for the specified agent method.
 *  The method name is used for lookups into the dictionary.
 *  The name is always converted to lower-case.
 */

static char *
get_method_name (const char *agent_name, const char *method_name)
{
    static
      char full_name [LINE_MAX + 1];

    ASSERT ((strlen (agent_name) + strlen (method_name) + 3) <= LINE_MAX);

    sprintf (full_name, "m~%s~%s", agent_name, method_name);
    strlwc  (full_name);
    return  (full_name);
}


/*  ---------------------------------------------------------------------[<]-
    Function: method_lookup

    Synopsis: Checks whether a specific method exists; returns the address
    of the method information block, or NULL if there was an error, setting
    smt_errno to one of these values:
    <Table>
    SMT_NOTREADY             smt_init() was not called, or failed
    SMT_NOSUCHMETHOD         Specified method was not declared
    </Table>
    ---------------------------------------------------------------------[>]-*/

METHOD *
method_lookup (
    const AGENT *agent,                 /*  Agent to look at                 */
    const char *method_name             /*  Name of method to look for       */
)
{
    SYMBOL  *dict_entry;                /*  Dictionary symbol                */
    char    *full_name;                 /*  Full agent/method name           */

#if (defined (SMT_TRACE))
    trace ("method_lookup: agent=%s", agent-> name, method_name);
#endif
    ASSERT (agent);
    ASSERT (method_name);
    if (!smt_alive)                     /*  If SMT API was not correctly     */
      {                                 /*    initialised, forget it         */
        smt_errno = SMT_NOTREADY;
        return (NULL);
      }

    full_name  = get_method_name (agent-> name, method_name);
    dict_entry = sym_lookup_symbol (dict, full_name);
    if (dict_entry == NULL)
      {
         smt_errno = SMT_NOSUCHMETHOD;
         return (NULL);
      }
    return (dict_entry-> data);         /*  Return pointer to METHOD         */
}


/*  ---------------------------------------------------------------------[<]-
    Function: method_destroy

    Synopsis: Destroys the method.  Returns 0 when completed.
    In case of error, returns -1 and sets smt_errno to one of these values:
    <Table>
    SMT_NOTREADY             smt_init() was not called, or failed
    </Table>
    ---------------------------------------------------------------------[>]-*/

int
method_destroy (
    METHOD *method                      /*  Method to destroy                */
)
{
#if (defined (SMT_TRACE))
    trace ("method_destroy: method=%s", method-> name);
#endif
    ASSERT (method);
    if (!smt_alive)                     /*  If SMT API was not correctly     */
      {                                 /*    initialised, forget it         */
        smt_errno = SMT_NOTREADY;
        return (-1);
      }

    /*  We have to be a little careful or sym_delete_symbol () will          */
    /*  try to release the symbol's data area; the data area points          */
    /*  to our node, which we want to release ourselves.                     */
    method-> symbol-> data = NULL;
    sym_delete_symbol (dict, method-> symbol);

    /*  Delete the method                                                    */
    mem_strfree (&method-> name);
    mem_free (list_unlink (method));
    return (0);
}


/*  ---------------------------------------------------------------------[<]-
    Function: queue_create

    Synopsis: Creates an event queue, and returns a handle to the created
    queue.  Event queues are unnamed but attached to a agent within an
    agent. Queue can also be 'floating', i.e. not attached to a agent.
    This is useful for foreign programs.  If you specify a agent, the
    queue is attached to that agent.   If the agent argument is null,
    the queue is left floating.  You always refer to a queue using its
    address (within the owning process) or QID handle (within any process).
    The current implementation uses a linked list in heap memory, so QID
    handles are only valid within the process.  Future implementations may
    use other types of shared memory including connections across a
    communications protocol.  Returns a pointer to the created QUEUE block.
    In case of error, returns null and sets smt_errno to one of these values:
    <Table>
    SMT_NOTREADY             smt_init() was not called, or failed
    SMT_OUTOFMEMORY          Not enough heap memory left
    </Table>
    ---------------------------------------------------------------------[>]-*/

QUEUE *
queue_create (
    AGENT *agent,                       /*  Parent agent block, or null      */
    int    max_events                   /*  Max. events; 0 = no limit        */
)
{
    static
      long  top_id = 0;                 /*  We number queues from 1 up       */
    QID     qid;                        /*  Created queue                    */
    SYMBOL  *dict_entry;                /*  Dictionary symbol                */
    QUEUE   *queue;                     /*  Queue information block          */

#if (defined (SMT_TRACE))
    trace ("queue_create: agent=%s", agent? agent-> name: "<none>");
#endif
    if (!smt_alive)                     /*  If SMT API was not correctly     */
      {                                 /*    initialised, forget it         */
        smt_errno = SMT_NOTREADY;
        return (NULL);
      }

    qid.node  = 0;                      /*  Queues are local for now         */
    qid.ident = ++top_id;               /*  First queue has id = 1           */
    dict_entry = sym_create_symbol (dict, get_queue_name (&qid), NULL);
    if (dict_entry == NULL)
      {
        smt_errno = SMT_OUTOFMEMORY;
        return (NULL);
      }

    /*  Allocate a QUEUE block and attach it to the queue list               */
    list_create (queue, sizeof (QUEUE));
    if (queue == NULL)
      {
        sym_delete_symbol (dict, dict_entry);
        smt_errno = SMT_OUTOFMEMORY;
        return (NULL);
      }
    if (agent)
        list_relink_before (queue, &agent-> queues);

    /*  Point the dictionary entry to the queue information block            */
    dict_entry-> data = queue;

    /*  Now initialise the queue info block fields and list heads            */
    queue-> agent        = agent;
    list_reset (&queue-> events);
    list_reset (&queue-> threads);
    queue-> qid          = qid;
    queue-> max_events   = max_events;
    queue-> cur_events   = 0;
    queue-> symbol       = dict_entry;
    queue-> timed_events = 0;
    queue-> shutdown     = FALSE;
    return (queue);
}


/*  -------------------------------------------------------------------------
 *  get_queue_name -- internal
 *
 *  Returns a formatted queue name for the specified queue handle.  The queue
 *  name is used for lookups into the dictionary.
 */

static char *
get_queue_name (const QID *qid)
{
    static
      char queue_name [15];           /*  Queue number is 32-bit value     */

    sprintf (queue_name, "q~%09ld", qid-> ident);
    return (queue_name);
}


/*  ---------------------------------------------------------------------[<]-
    Function: queue_lookup

    Synopsis: Returns a pointer to the queue information block for the
    specified queue handle.  You can change the max_events field, but you
    should not change the other fields in the queue information block.
    In case of error, returns NULL and sets smt_errno to one of these values:
    <Table>
    SMT_NOTREADY             smt_init() was not called, or failed
    SMT_NOSUCHQUEUE          The event queue is not defined
    </Table>
    ---------------------------------------------------------------------[>]-*/

QUEUE *
queue_lookup (
    const QID *qid                      /*  Queue to find                    */
)
{
    SYMBOL  *dict_entry;                /*  Queue entry in dictionary        */

#if (defined (SMT_TRACE))
    trace ("queue_lookup");
#endif
    ASSERT (qid);
    if (!smt_alive)                     /*  If SMT API was not correctly     */
      {                                 /*    initialised, forget it         */
        smt_errno = SMT_NOTREADY;
        return (NULL);
      }

    dict_entry = sym_lookup_symbol (dict, get_queue_name (qid));
    if (dict_entry)
        return (dict_entry-> data);
    else
      {
        smt_errno = SMT_NOSUCHQUEUE;
        return (NULL);
      }
}


/*  ---------------------------------------------------------------------[<]-
    Function: queue_destroy

    Synopsis: Deletes any events in the event queue and then destroys the
    queue and all its threads.  Returns 0 when successfully completed.
    In case of error, returns -1 and sets smt_errno to one of these values:
    <Table>
    SMT_NOTREADY             smt_init() was not called, or failed
    SMT_NOSUCHQUEUE          The event queue is not defined
    </Table>
    ---------------------------------------------------------------------[>]-*/

int
queue_destroy (
    QUEUE *queue                        /*  Queue to destroy                 */
)
{
#if (defined (SMT_TRACE))
    trace ("queue_destroy");
#endif
    ASSERT (queue);
    if (!smt_alive)                     /*  If SMT API was not correctly     */
      {                                 /*    initialised, forget it         */
        smt_errno = SMT_NOTREADY;
        return (-1);
      }

    /*  Delete all events in the queue                                       */
    while (event_discard (queue, NULL) == 0);

    /*  Destroy all threads defined for the queue                            */
    while (queue-> threads.next != &queue-> threads)
        thread_destroy (queue-> threads.next, FALSE);

    /*  We have to be a little careful or sym_delete_symbol () will          */
    /*  try to release the symbol's data area; the data area points          */
    /*  to our node, which we want to release ourselves.                     */
    queue-> symbol-> data = NULL;
    sym_delete_symbol (dict, queue-> symbol);

    /*  Now delete the queue itself                                          */
    mem_free (list_unlink (queue));
    return (0);
}


/*  ---------------------------------------------------------------------[<]-
    Function: queue_deliver

    Synopsis: Tries to deliver an event from a queue to an agent thread.
    The event, if deliverable, is removed from the queue and stored in the
    thread-> event block.  Returns 1 if an event was delivered, else 0.
    ---------------------------------------------------------------------[>]-*/

int
queue_deliver (
    QUEUE  *queue,                      /*  Queue containing events          */
    THREAD *thread)                     /*  Agent thread to receive event    */
{
    AGENT   *agent;                     /*  Agent to receive event           */
    EVENT   *event;                     /*  Event information block          */
    EVENT   *deliver_event;             /*  Event to deliver                 */
    EVENT   *null_event;                /*  Event with NULL method           */
    METHOD  *method;                    /*  Method information block         */
    int     top_priority;               /*  Highest event priority so far    */

#if (defined (SMT_TRACE))
    trace ("queue_deliver: thread=%s in %s", thread-> name,
                                             thread-> queue-> agent-> name);
#endif
    /*  Get event to deliver - find event with highest method priority       */
    top_priority  = -1;
    deliver_event = NULL;
    agent = thread-> queue-> agent;
    event = queue-> events.next;
    while ((LIST *) event != &queue-> events)
      {
        if (event-> priority == SMT_PRIORITY_NULL)
          {
            /*  Lookup method; if method is not declared, reject event      */
            method = method_lookup (agent, event-> name);
            if (method == NULL)
              {
                /*  Reject this event, but keep our place in the queue...   */
                send_smtoper_error (&console,
                                    strprintf ("Event %s not declared by %s",
                                               event-> name, agent-> name));
                event = event-> next;
                event_reject (queue, event-> prev);
                continue;
              }
            else
            /*  If null method, accept event but discard it                 */
            if (method-> event_number == SMT_NULL_EVENT)
              {
                event = event-> next;
                null_event = event_accept (queue, event-> prev);
                if (null_event)
                  {
                    if (null_event-> body)
                        mem_free (null_event-> body);
                    mem_free (null_event);
                  }
                continue;
              }
            /*  Update the event values, to save a lookup next time         */
            event-> priority     = method-> priority;
            event-> event_number = method-> event_number;
          }
        if (event-> priority > top_priority)
          {
            top_priority  = event-> priority;
            deliver_event = event;
          }
        event = event-> next;
      }
    if (deliver_event)
      {
        if (thread-> event)           
            event_destroy (thread-> event);

        thread-> the_next_event     = deliver_event-> event_number;
        thread-> the_external_event = deliver_event-> event_number;
        thread-> event = event_accept (queue, deliver_event);
        return (1);                     /*  We delivered the event           */
      }
    else
        return (0);                     /*  We did not find an event         */
}


/*  ---------------------------------------------------------------------[<]-
    Function: queue_flush

    Synopsis: Expires any out-of-date events in the queue: calls
    event_expire() for each event who's timeout has passed.  Returns the
    number of events expired.
    In case of error, returns -1 and sets smt_errno to one of these values:
    <Table>
    SMT_NOTREADY             smt_init() was not called, or failed
    SMT_NOSUCHQUEUE          The event queue is not defined
    </Table>
    ---------------------------------------------------------------------[>]-*/

int
queue_flush (
    QUEUE *queue                        /*  Queue to flush                   */
)
{
    time_t  time_now;                   /*  Current time                     */
    int     feedback = 0;               /*  Number of events we delivered    */
    EVENT   *event;                     /*  Event information block          */

#if (defined (SMT_TRACE))
    trace ("queue_flush");
#endif
    ASSERT (queue);
    if (!smt_alive)                     /*  If SMT API was not correctly     */
      {                                 /*    initialised, forget it         */
        smt_errno = SMT_NOTREADY;
        return (-1);
      }

    time_now = time (NULL);
    FORLIST (event, queue-> events)
        if (event-> timeout && event-> timeout < time_now)
          {
            event_expire (queue, event);
            feedback++;
          }

    return (feedback);
}


/*  ---------------------------------------------------------------------[<]-
    Function: event_send

    Synopsis: Sends an event to an event queue.  The event body - if not
    null or empty - is always copied, crossing memory boundaries.  The
    accept_event, reject_event, and timeout_events are sent back to the
    sender event queue as required.  These events may be specified as null
    or empty strings.  The timeout may be 0 for none, or a value in
    milliseconds.  Returns 0 when successfully completed.  The current
    implementation provides timeouts accurate to a second only.  The
    target queue may be null, in which case the event is ignored, and
    not sent.  This lets you reply to an event without always checking
    that the reply queue was specified.  The event name and reply event
    names are always stored in uppercase.

    In case of error, returns -1 and sets smt_errno to one of these values:
    <Table>
    SMT_NOTREADY             smt_init() was not called, or failed
    SMT_OUTOFMEMORY          Not enough heap memory left
    SMT_NOSUCHQUEUE          The target event queue is not known
    SMT_QUEUEISFULL          The target event queue is full
    </Table>
    ---------------------------------------------------------------------[>]-*/

int
event_send (
    const QID *to_queue,                /*  Queue to receive event, or NULL  */
    const QID *from_queue,              /*  Queue to receive reply, or NULL  */
    char   *name,                       /*  Name of event to send            */
    void   *body,                       /*  Body of message or NULL          */
    size_t  body_size,                  /*  Size of body >= 0                */
    char   *accept_event,               /*  Accept event or NULL             */
    char   *reject_event,               /*  Reject event or NULL             */
    char   *expire_event,               /*  Expire event or NULL             */
    word    timeout                     /*  Timeout in seconds: 0 = none     */
)
{
    static QID
        null_queue = {0, 0};            /*  Indicates a null sender queue    */
    QUEUE   *queue;                     /*  Queue where we will place event  */
    EVENT   *event;                     /*  Allocated event block            */
    size_t  size;                       /*  Total size of event block        */
    char    *string;                    /*  For storing event strings        */
    THREAD  *thread;

#if (defined (SMT_TRACE))
    trace ("event_send: event=%s", name);
#endif
    ASSERT (name && name [0]);
    if (!smt_alive)                     /*  If SMT API was not correctly     */
      {                                 /*    initialised, forget it         */
        smt_errno = SMT_NOTREADY;
        return (-1);
      }

    /*  If the called did not specify a target queue, ignore                 */
    if (to_queue == NULL || to_queue-> ident == 0)
        return (0);

    if (from_queue == NULL)             /*  If the caller did not specify    */
      {                                 /*    a reply queue, then we can     */
        accept_event =                  /*    ignore the reply events in     */
        reject_event =                  /*    any case.                      */
        expire_event = NULL;
        from_queue   = &null_queue;
      }
    if ((queue = queue_lookup (to_queue)) == NULL)
      {
        smt_errno = SMT_NOSUCHQUEUE;
        return (-1);
      }

    /*  Check that we're allowed to create a new event                       */
    if (queue-> max_events > 0
    &&  queue-> max_events == queue-> cur_events)
      {
        smt_errno = SMT_QUEUEISFULL;
        return (-1);
      }

    /*  We allocate the event, and return events as a single                 */
    /*  block, to reduce access to the heap and make cleaning-up easier.     */
    size = sizeof (EVENT) + strlen (name) + 1;
    if (accept_event)
        size += strlen (accept_event) + 1;
    if (reject_event)
        size += strlen (reject_event) + 1;
    if (expire_event)
        size += strlen (expire_event) + 1;

    /*  Allocate an EVENT block and attach it to the event list              */
    list_create (event, size);
    if (event == NULL)
      {
        smt_errno = SMT_OUTOFMEMORY;
        return (-1);
      }
    list_relink_before (event, &queue-> events);

    event-> queue        = queue;       /*  Set parent queue address         */
    event-> sender       = *from_queue; /*    and sender queue               */
    event-> body_size    = body_size;   /*  Store body size                  */
    event-> timeout      = timeout? time (NULL) + timeout: 0;
    event-> size         = size;        /*  Event is self-contained          */
    event-> priority     = SMT_PRIORITY_NULL;
    event-> event_number = SMT_NULL_EVENT;

    /*  Store variable-length parts after main event structure               */
    string = (char *) event + sizeof (EVENT);
    event-> name = string;
    strcpy (string, name);
    strupc (string);
    string += strlen (string) + 1;

    if (body_size > 0)
      {
        /*  Store event body                                                 */
        event-> body = (byte *) mem_alloc (body_size);
        if (event-> body)
            memcpy (event-> body, (byte *) body, body_size);
      }
    else
        event-> body = NULL;

    if (accept_event)
      {
        event-> accept_event = string;
        strcpy (string, accept_event);
        strupc (string);
        string += strlen (string) + 1;
      }
    else
        event-> accept_event = NULL;

    if (reject_event)
      {
        event-> reject_event = string;
        strcpy (string, reject_event);
        strupc (string);
        string += strlen (string) + 1;
      }
    else
        event-> reject_event = NULL;

    if (expire_event)
      {
        event-> expire_event = string;
        strcpy (string, expire_event);
        strupc (string);
        string += strlen (string) + 1;
      }
    else
        event-> expire_event = NULL;

    if (timeout)
        queue-> timed_events++;         /*  Count event if timed             */
    queue-> cur_events++;               /*  Count the event                  */

    /*  Deliver events, if any, to passive threads, if any           */
    thread = queue-> threads.next;
    while ((queue-> cur_events > 0)
       && ((LIST *) thread != &queue-> threads))
      {
        if (thread-> state == SMT_THREAD_PASSIVE
        ||  thread-> state == SMT_THREAD_WAIT_EVENT)
          {
            if (queue_deliver (queue, thread))
                activate_thread (thread);

            /*  We can break after delivering because there can only be one  */
            /*  event to deliver.                                            */
            break;
          }

        thread = thread-> next;
      }

    return (0);                         /*  No errors                        */
}


/*  ---------------------------------------------------------------------[<]-
    Function: event_accept

    Synopsis: Takes an event off an event queue, and sends an 'accept'
    reply to the original sender, if required.  If the specified event is
    null, takes the first (oldest) event in the queue.  Otherwise takes the
    specified event.  Returns the address of the event.  You should call
    event_destroy() when you have finished processing the event.  In case
    of error, returns NULL and sets smt_errno to one of these values:
    <Table>
    SMT_NOTREADY             smt_init() was not called, or failed
    SMT_QUEUEISEMPTY         The queue was empty
    </Table>
    ---------------------------------------------------------------------[>]-*/

EVENT *
event_accept (
    QUEUE *queue,                       /*  Queue to take event from         */
    EVENT *event                        /*  Event, or null for first         */
)
{
#if (defined (SMT_TRACE))
    trace ("event_accept");
#endif
    ASSERT (queue);
    if (!smt_alive)                     /*  If SMT API was not correctly     */
      {                                 /*    initialised, forget it         */
        smt_errno = SMT_NOTREADY;
        return (NULL);
      }

    if ((event = event_locate (queue, event)) == NULL)
        return (NULL);

    /*  Reply to original sender if necessary                                */
    if (event-> accept_event)
        event_send (
            &event-> sender,            /*  Send back to original sender     */
            NULL,                       /*  No queue for reply               */
            event-> accept_event,       /*  Name of event to send            */
            NULL, 0,                    /*  Body is empty, size is 0         */
            NULL, NULL, NULL,           /*  No response events               */
            0);                         /*  No timeout                       */

    if (event-> timeout)
        queue-> timed_events--;         /*  One less timed event, maybe      */
    queue-> cur_events--;               /*  One less event in queue          */

    list_unlink (event);                /*  Unlink from queue                */
    return (event);
}


/*  -------------------------------------------------------------------------
 *  event_locate -- internal
 *
 *  Finds the specified or first event in the event queue.  If the queue
 *  is empty, sets smt_errno to SMT_QUEUEISEMPTY and returns NULL.  Else
 *  returns the address of the event wanted.
 */

static EVENT *
event_locate (QUEUE *queue, EVENT *event)
{
    if (event == NULL)                  /*  If no event was specified,       */
        event = queue-> events.next;    /*    get first event in queue       */

    if (event == (EVENT *) &queue-> events)
      {
        smt_errno = SMT_QUEUEISEMPTY;
        return (NULL);
      }
    return (event);
}


/*  ---------------------------------------------------------------------[<]-
    Function: event_reject

    Synopsis: Rejects the next event or a specific event on an event queue.
    Sends a 'rejected' event to the original sender if required, then
    destroys the event.  You can use this to reject one specific message,
    or in a loop to cancel the entire queue.  Returns 0 if the event was
    successfully rejected, else returns -1 and sets smt_errno to one of:
    <Table>
    SMT_NOTREADY             smt_init() was not called, or failed
    SMT_QUEUEISEMPTY         The queue was empty
    </Table>
    ---------------------------------------------------------------------[>]-*/

int
event_reject (
    QUEUE *queue,                       /*  Queue to take event from         */
    EVENT *event                        /*  Event, or null for first         */
)
{
#if (defined (SMT_TRACE))
    trace ("event_reject");
#endif
    ASSERT (queue);
    if (!smt_alive)                     /*  If SMT API was not correctly     */
      {                                 /*    initialised, forget it         */
        smt_errno = SMT_NOTREADY;
        return (-1);
      }

    if ((event = event_locate (queue, event)) == NULL)
        return (-1);

    /*  Reply to original sender if necessary                                */
    if (event-> reject_event)
        event_send (
            &event-> sender,            /*  Send back to original sender     */
            NULL,                       /*  No queue for reply               */
            event-> reject_event,       /*  Name of event to send            */
            NULL, 0,                    /*  Body is empty, size is 0         */
            NULL, NULL, NULL,           /*  No response events               */
            0);                         /*  No timeout                       */

    /*  Unlink and destroy event                                             */
    return (event_discard (queue, event));
}


/*  ---------------------------------------------------------------------[<]-
    Function: event_expire

    Synopsis: Expires the next event or a specific event on an event queue.
    Sends a 'expired' event to the original sender if required, then
    destroys the event.  You can use this to expire one specific message,
    or in a loop to cancel the entire queue.  Returns 0 if the event was
    successfully expired, else returns -1 and sets smt_errno to one of:
    <Table>
    SMT_NOTREADY             smt_init() was not called, or failed
    SMT_QUEUEISEMPTY         The queue was empty
    </Table>
    ---------------------------------------------------------------------[>]-*/

int
event_expire (
    QUEUE *queue,                       /*  Queue to take event from         */
    EVENT *event                        /*  Event, or null for first         */
)
{
#if (defined (SMT_TRACE))
    trace ("event_expire");
#endif
    ASSERT (queue);
    if (!smt_alive)                     /*  If SMT API was not correctly     */
      {                                 /*    initialised, forget it         */
        smt_errno = SMT_NOTREADY;
        return (-1);
      }

    if ((event = event_locate (queue, event)) == NULL)
        return (-1);

    /*  Reply to original sender if necessary                                */
    if (event-> expire_event)
        event_send (
            &event-> sender,            /*  Send back to original sender     */
            NULL,                       /*  No queue for reply               */
            event-> expire_event,       /*  Name of event to send            */
            NULL, 0,                    /*  Body is empty, size is 0         */
            NULL, NULL, NULL,           /*  No response events               */
            0);                         /*  No timeout                       */

    /*  Unlink and destroy event                                             */
    return (event_discard (queue, event));
}


/*  ---------------------------------------------------------------------[<]-
    Function: event_discard

    Synopsis: Discards the specified event in the specified queue.  The
    event_iterate (), event_accept () and event_discard () calls let a
    thread manipulate its event queue directly.  In such cases the thread
    takes responsibility for event delivery and acknowledgement.  Returns 0
    when successfully completed.

    In case of error, returns -1 and sets smt_errno to one of these values:
    <Table>
    SMT_NOTREADY             smt_init() was not called, or failed
    SMT_QUEUEISEMPTY         The queue was empty
    </Table>
    ---------------------------------------------------------------------[>]-*/

int
event_discard (
    QUEUE *queue,                       /*  Queue to take event from         */
    EVENT *event                        /*  Event, or null                   */
)
{
#if (defined (SMT_TRACE))
    trace ("event_discard");
#endif
    ASSERT (queue);
    if (!smt_alive)                     /*  If SMT API was not correctly     */
      {                                 /*    initialised, forget it         */
        smt_errno = SMT_NOTREADY;
        return (-1);
      }

    if ((event = event_locate (queue, event)) == NULL)
        return (-1);

    queue-> cur_events--;               /*  Count the event                  */
    event_destroy (event);              /*  Unlink and destroy event         */
    return (0);
}


/*  ---------------------------------------------------------------------[<]-
    Function: event_iterate

    Synopsis: Returns the first or next event in the queue.  If the 'after'
    argument is null, returns the first event, else returns the next event.
    You should not 'walk' the event queue directly, since the implementation
    may change arbitrarily.  Returns a pointer to the next event, or null if
    no (further) events were found.  May set smt_errno to one of:
    <Table>
    SMT_NOTREADY             smt_init() was not called, or failed
    SMT_QUEUEISEMPTY         The queue was empty
    </Table>
    ---------------------------------------------------------------------[>]-*/

EVENT *
event_iterate (
    QUEUE *queue,                       /*  Queue to search                  */
    EVENT *event                        /*  Event, or null for first         */
)
{
#if (defined (SMT_TRACE))
    trace ("event_iterate");
#endif
    ASSERT (queue);
    if (!smt_alive)                     /*  If SMT API was not correctly     */
      {                                 /*    initialised, forget it         */
        smt_errno = SMT_NOTREADY;
        return (NULL);
      }

    /*  If no event specified, get first event in queue                      */
    if (event == NULL)
        event = (EVENT *) &queue-> events;

    event = event-> next;               /*  Get next event in queue          */
    if (event == (EVENT *) &queue-> events)
      {
        smt_errno = SMT_QUEUEISEMPTY;
        return (NULL);
      }
    return (event);
}


/*  ---------------------------------------------------------------------[<]-
    Function: event_destroy

    Synopsis: Destroys the specified event, which is assumed not to be
    linked into any queue.  Typically you'll call this after processing an
    event you received with event_accept().  Returns 0 if okay.
    ---------------------------------------------------------------------[>]-*/

int
event_destroy (
    EVENT *event                        /*  Event block to destroy           */
)
{
#if (defined (SMT_TRACE))
    trace ("event_destroy: event=%s", event-> name);
#endif
    if (event-> body)
      {
        mem_free (event-> body);
        event-> body = NULL;
      }
    mem_free (list_unlink (event));     /*  Unlink and destroy event         */
    return (0);
}


/*  ---------------------------------------------------------------------[<]-
    Function: event_wait

    Synopsis: Suspends processing of an action module list, and waits for an
    incoming event.  The event is received in 'the_external_event' by the
    next dialog module.  When called in the last module in a list, has no
    effect.  This call has no effect if you raise an exception or supply a
    value in the_next_event.
    ---------------------------------------------------------------------[>]-*/

void
event_wait (void)
{
#if (defined (SMT_TRACE))
    trace ("event_wait");
#endif
    break_wanted = BREAK_WAIT_EVENT;
}


/*  ---------------------------------------------------------------------[<]-
    Function: thread_create_

    Synopsis: Creates a new thread, and possibly an event queue for the
    thread.  The caller specifies the agent and thread name.   The
    agent must already be declared using agent_declare().  If the agent
    was defined as a router, you can create multiple threads with the same
    name.  These threads then share the same event queue on an anonymous
    basis.  If the agent was not defined as a router, it is illegal to
    create multiple threads with the same name unless the name is empty
    (meaning "" or NULL).
    The function automatically creates an event queue for the thread when
    required.  Returns a pointer to the created THREAD block, or null if
    there was an error.  In that case, sets smt_errno to one of:
    <Table>
    SMT_NOTREADY             smt_init() was not called, or failed
    SMT_OUTOFMEMORY          Not enough heap memory left
    SMT_NOSUCHAGENT          The agent was not declared
    SMT_AGENTNOTREADY        The agent is not initialised
    SMT_TOOMANYTHREADS       Tried to exceed the maximum permitted threads
    SMT_THREADEXISTS         The thread already exists
    </Table>
    Attaches the thread to the active thread list.
    ---------------------------------------------------------------------[>]-*/

THREAD *
thread_create_ (
    const char *agent_name,             /*  Name of agent                    */
    const char *thread_name             /*  Create thread with this name     */
)
{
    SYMBOL  *dict_entry;                /*  Dictionary symbol                */
    AGENT   *agent;                     /*  Agent information block          */
    QUEUE   *queue;                     /*  Queue information block          */
    THREAD  *thread;                    /*  Created thread block             */
    char    *full_name;                 /*  Full thread name                 */

#if (defined (SMT_TRACE))
    trace ("thread_create: agent=%s thread=%s", agent_name, thread_name);
#endif
    ASSERT (agent_name);
    if (!smt_alive)                     /*  If SMT API was not correctly     */
      {                                 /*    initialised, forget it         */
        smt_errno = SMT_NOTREADY;
        return (NULL);
      }

    /*  Find agent, quit if there was an error                               */
    if ((agent = agent_lookup (agent_name)) == NULL)
        return (NULL);

    /*  Check that we're allowed to create a new thread                      */
    if (agent-> max_threads > 0
    &&  agent-> max_threads == agent-> cur_threads)
      {
        smt_errno = SMT_TOOMANYTHREADS;
        return (NULL);
      }

    /*  Simple check that agent has been initialised                         */
    if (agent-> initialise == NULL)
      {
        smt_errno = SMT_AGENTNOTREADY;
        return (NULL);
      }

    /*  Treat a NULL thread name as an empty string                          */
    if (thread_name == NULL)
        thread_name = "";

    /*  Check if the thread already exists                                   */
    full_name  = get_entity_name (agent_name, thread_name);
    dict_entry = sym_lookup_symbol (dict, full_name);
    queue      = NULL;                  /*  No queue created yet             */
    if (dict_entry)
      {
        /*  If it's a router, we'll use the same queue                       */
        if (agent-> router)
            queue = ((THREAD *) (dict_entry-> data))-> queue;
        else
        if (thread_name [0])            /*  Otherwise it's an error if the   */
          {                             /*    thread was given a name        */
            smt_errno = SMT_THREADEXISTS;
            return (NULL);
          }
      }
    if (!queue)                        /*   Create new queue in agent        */
        queue = queue_create (agent, 0);

    /*  Now create entry for the thread                                      */
    dict_entry = sym_create_symbol (dict, full_name, NULL);
    if (dict_entry == NULL)
      {
        if (queue)                      /*  Clean-up nicely                  */
            queue_destroy (queue);
        smt_errno = SMT_OUTOFMEMORY;
        return (NULL);
      }

    /*  Allocate a THREAD block and attach it to the queue's thread list     */
    /*  We also allocate the TCB and subdialog stack if that is required     */
    list_create (thread, sizeof (THREAD));
    if (! thread)
        smt_errno = SMT_OUTOFMEMORY;
    else
      {
        list_relink_before (thread, &queue-> threads);
        smt_errno = SMT_NOERRORS;
        if (agent-> tcb_size > 0)
          {
            thread-> tcb = mem_alloc (agent-> tcb_size);
            if (! thread-> tcb)
                smt_errno = SMT_OUTOFMEMORY;
            else
                memset (thread-> tcb, 0, agent-> tcb_size);
          }
        else
            thread-> tcb = NULL;
      }
       
    if (smt_errno)
      {
        if (thread)
            mem_free (thread);

        sym_delete_symbol (dict, dict_entry);
        if (queue)                      /*  Clean-up nicely                  */
            queue_destroy (queue);

        return (NULL);
      }

    /*  Point the dictionary entry to the thread information block           */
    dict_entry-> data = thread;

    /*  Now initialise the thread fields and list heads                      */
    thread-> queue             = queue;
    thread-> thread_id         = agent-> thread_tally++;
    thread-> name              = mem_strdup (thread_name);
    thread-> animate           = agent-> animate;
    thread-> event             = NULL;      /*  Last event for thread            */
    thread-> memtrn            = mem_new_trans ();
    thread-> atdestroy         = NULL;
    thread-> symbol            = dict_entry;
    thread-> left              = thread;
    thread-> right             = thread;
    thread-> loop_event        = NULL;
    thread-> LR_location_stack = NULL;

    /*  Setting LR_state to SMT_NULL_STATE signals to the dialog manager     */
    /*  this is a fresh thread.  It will initialise the other thread data.   */
    thread-> LR_state          = SMT_NULL_STATE;

    cur_threads++;                      /*  Keep count of threads            */
    agent-> cur_threads++;
    if (agent-> top_threads < agent-> cur_threads)
        agent-> top_threads = agent-> cur_threads;

    activate_thread (thread);           /*  Move thread to active list       */
    return (thread);
}


/*  -------------------------------------------------------------------------
 *  thread_relink -- internal
 *
 *  General-purpose function to attach and remove threads from the active
 *  list.  Use via the macros thread_unlink, thread_link_after, and
 *  thread_link_before.  Basically unlinks a node if it is linked, or links
 *  a node if it is unlinked.
 */

static void
thread_relink (THREAD *left, THREAD *thread, THREAD *right)
{
    static THREAD *swap;

    swap = left-> right;                /*  Exchange left pointers           */
           left-> right = thread-> right;
                         thread-> right = swap;

    swap = right-> left;                /*  Exchange right pointers          */
           right-> left = thread-> left;
                          thread-> left = swap;
}


/*  ---------------------------------------------------------------------[<]-
    Function: thread_lookup

    Synopsis: Checks whether a specific thread exists; returns the address
    of the thread information block, or NULL if there was an error, setting
    smt_errno to one of these values:
    <Table>
    SMT_NOTREADY             smt_init() was not called, or failed
    SMT_NOSUCHTHREAD         The agent/thread does not exist
    </Table>
    If there are multiple threads (routers) with the same name, returns the
    earliest thread that was defined.
    ---------------------------------------------------------------------[>]-*/

THREAD *
thread_lookup (
    const char *agent_name,             /*  Name of agent                    */
    const char *thread_name             /*  Create thread with this name     */
)
{
    SYMBOL  *dict_entry;                /*  Dictionary symbol                */
    char    *full_name;                 /*  Full thread name                 */

#if (defined (SMT_TRACE))
    trace ("thread_lookup: agent=%s thread=%s", agent_name, thread_name);
#endif
    ASSERT (agent_name);
    ASSERT (thread_name);
    if (!smt_alive)                     /*  If SMT API was not correctly     */
      {                                 /*    initialised, forget it         */
        smt_errno = SMT_NOTREADY;
        return (NULL);
      }

    /*  Check if the thread already exists                                   */
    full_name  = get_entity_name (agent_name, thread_name);
    dict_entry = sym_lookup_symbol (dict, full_name);
    if (dict_entry == NULL)
      {
         smt_errno = SMT_NOSUCHTHREAD;
         return (NULL);
      }

    /*  Get address of thread block, then find first thread defined for      */
    /*  this queue.  Usually it will be the same thread; when there are      */
    /*  multiple threads (routers) it may be a different thread.             */
    return (((THREAD *) (dict_entry-> data))-> queue-> threads.next);
}


/*  ---------------------------------------------------------------------[<]-
    Function: thread_destroy

    Synopsis: Destroys the thread.  If this was the last instance of a
    router thread, destroys the parent queue as well, if the cleanup
    argument is TRUE.  Returns 0 if successfully completed, else returns
    -1 and sets smt_errno to one of these values:
    <Table>
    SMT_NOTREADY             smt_init() was not called, or failed
    </Table>
    Removes the thread from the active list if it was attached there.
    Destroys any event allocated for the thread.
    ---------------------------------------------------------------------[>]-*/

int
thread_destroy (
    THREAD *thread,                     /*  Thread to destroy                */
    Bool    cleanup                     /*  Delete queue if last thread      */
)
{
    AGENT   *agent;                     /*  Agent information block          */
    QUEUE   *queue;                     /*  Queue information block          */

#if (defined (SMT_TRACE))
    trace ("thread_destroy: thread=%s", thread-> name);
#endif
    ASSERT (thread);
    if (!smt_alive)                     /*  If SMT API was not correctly     */
      {                                 /*    initialised, forget it         */
        smt_errno = SMT_NOTREADY;
        return (-1);
      }

    queue = thread-> queue;             /*  Get parents for thread           */
    agent = queue-> agent;

    /*  We have to be a little careful or sym_delete_symbol () will          */
    /*  try to release the symbol's data area; the data area points          */
    /*  to our node, which we want to release ourselves.                     */
    thread-> symbol-> data = NULL;
    sym_delete_symbol (dict, thread-> symbol);

    /*  Destroy event for the thread, if we still need to                    */
    if (thread-> event)                 /*  NULL = no event for thread       */
        event_destroy (thread-> event);

    /*  Call the destroy function for the thread.                            */
    if (thread-> atdestroy)
        (thread-> atdestroy) (thread);

    /*  Assert that all thread's memory has been freed.                      */
    memt_assert  (thread-> memtrn);
    mem_rollback (thread-> memtrn);

    /*  Delete the thread                                                    */
    thread_unlink (thread);             /*  Remove thread from active        */
    mem_free (thread-> tcb);            /*  Free allocated TCB,              */
    destroy_location_stack (thread-> LR_location_stack); /*  & dialog stack  */
    mem_strfree (&thread-> name);
    mem_free (thread-> loop_event);
    mem_free (list_unlink (thread));

    /*  Destroy queue if last thread, and we are asked to clean-up           */
    if (queue-> threads.next == &queue-> threads && cleanup)
        queue_destroy (queue);

    ASSERT (agent-> cur_threads > 0);
    agent-> cur_threads--;
    cur_threads--;                      /*  Keep count of threads            */
    return (0);
}


/*  ---------------------------------------------------------------------[<]-
    Function: semaph_create

    Synopsis: Creates a new semaphore.  You must create a semaphore before
    you can use it.  The value argument specifies the number of parties
    that can access the semaphore (or its related resources) at once.  The
    value must be greated than zero.  A 'normal' binary semaphore has an
    initial value of 1.

    The semaph name is an arbitrary text, unique within the application.
    Returns the address of the created SEMAPH block. If there was an error,
    returns NULL and sets smt_errno to one of these values:
    <Table>
    SMT_NOTREADY             smt_init() was not called, or failed
    SMT_OUTOFMEMORY          Not enough heap memory left
    SMT_SEMAPHEXISTS         A semaphore with this name is already declared
    </Table>
    ---------------------------------------------------------------------[>]-*/

SEMAPH *
semaph_create (
    const char *semaph_name,            /*  Name of semaph to create         */
    int   value                         /*  Initial semaphore value          */
)
{
    SYMBOL  *dict_entry;                /*  Dictionary symbol                */
    SEMAPH  *semaph;                    /*  Agent information block          */
    char    *full_name;                 /*  Full semaph name                 */

#if (defined (SMT_TRACE))
    trace ("semaph_create: semaph=%s", semaph_name);
#endif
    ASSERT (semaph_name);
    ASSERT (value > 0);
    if (!smt_alive)                     /*  If SMT API was not correctly     */
      {                                 /*    initialised, forget it         */
        smt_errno = SMT_NOTREADY;
        return (NULL);
      }

    /*  Check that semaphore is not already declared                         */
    full_name = get_semaph_name (semaph_name);
    if (sym_lookup_symbol (dict, full_name))
      {
        smt_errno = SMT_SEMAPHEXISTS;
        return (NULL);
      }

    /*  Now create entry for the semaphore                                   */
    dict_entry = sym_create_symbol (dict, full_name, NULL);
    if (dict_entry == NULL)
      {
        smt_errno = SMT_OUTOFMEMORY;
        return (NULL);
      }

    /*  Allocate an SEMAPH block and attach it to the semaphore list         */
    list_create (semaph, sizeof (SEMAPH));
    if (semaph == NULL)
      {
        sym_delete_symbol (dict, dict_entry);
        smt_errno = SMT_OUTOFMEMORY;
        return (NULL);
      }
    list_relink_before (semaph, &semaphs);

    /*  Point the dictionary entry to the semaph information block           */
    dict_entry-> data = semaph;

    /*  Now initialise the semaph - all fields are already set to zero       */
    semaph-> name          = mem_strdup (semaph_name);
    semaph-> symbol        = dict_entry;
    semaph-> threads.left  = &semaph-> threads;
    semaph-> threads.right = &semaph-> threads;
    semaph-> value         = value;

    return (semaph);
}


/*  -------------------------------------------------------------------------
 *  get_semaph_name -- internal
 *
 *  Returns a formatted name for the specified semaphore.
 *  The semaphore name is used for lookups into the dictionary.
 *  The name is always converted to lower-case.
 */

static char *
get_semaph_name (const char *semaph_name)
{
    static
      char  full_name [LINE_MAX + 1];

    ASSERT ((strlen (semaph_name) + 2) <= LINE_MAX);

    sprintf (full_name, "x~%s", semaph_name);
    strlwc  (full_name);
    return  (full_name);
}


/*  ---------------------------------------------------------------------[<]-
    Function: semaph_lookup

    Synopsis: Checks whether a specific semaphore exists; returns the
    address of the semaphore information block, or NULL if there was an
    error, setting smt_errno to one of these values:
    <Table>
    SMT_NOTREADY             smt_init() was not called, or failed
    SMT_NOSUCHSEMAPH         Specified semaphore was not declared
    </Table>
    ---------------------------------------------------------------------[>]-*/

SEMAPH *
semaph_lookup (
    const char *semaph_name             /*  Name of semaph to look for       */
)
{
    SYMBOL  *dict_entry;                /*  Dictionary symbol                */

#if (defined (SMT_TRACE))
    trace ("semaph_lookup: semaph=%s", semaph_name);
#endif
    if (!smt_alive)                     /*  If SMT API was not correctly     */
      {                                 /*    initialised, forget it         */
        smt_errno = SMT_NOTREADY;
        return (NULL);
      }

    dict_entry = sym_lookup_symbol (dict, get_semaph_name (semaph_name));
    if (dict_entry == NULL)
      {
         smt_errno = SMT_NOSUCHSEMAPH;
         return (NULL);
      }
    return (dict_entry-> data);         /*  Return pointer to SEMAPH         */
}


/*  ---------------------------------------------------------------------[<]-
    Function: semaph_destroy

    Synopsis: Destroys the semaphore.  Returns 0 when completed.  The
    semaph argument points to an semaph block, or is null.  If null, all
    semaphores are destroyed.  Returns 0 when completed normally, else
    returns -1 and sets smt_errno to one of these values:
    <Table>
    SMT_NOTREADY             smt_init() was not called, or failed
    </Table>
    ---------------------------------------------------------------------[>]-*/

int
semaph_destroy (
    SEMAPH *semaph                      /*  Semaph to destroy; null = all    */
)
{
    THREAD  *thread;

#if (defined (SMT_TRACE))
    trace ("semaph_destroy: semaph=%s", semaph? semaph-> name: "ALL");
#endif
    if (!smt_alive)                     /*  If SMT API was not correctly     */
      {                                 /*    initialised, forget it         */
        smt_errno = SMT_NOTREADY;
        return (-1);
      }

    if (semaph == NULL)                 /*  Destroy all semaphs if wanted    */
        while (semaphs.next != &semaphs)
            semaph_destroy (semaphs.next);
    else                                /*  Else destroy this semaph         */
      {
        /*  We have to be a little careful or sym_delete_symbol () will      */
        /*  try to release the symbol's data area; the data area points      */
        /*  to our node, which we want to release ourselves.                 */
        semaph-> symbol-> data = NULL;
        sym_delete_symbol (dict, semaph-> symbol);

        /*  Reactivate any threads which are waiting on this semaphore       */
        while (semaph-> threads.right != &semaph-> threads)
          {
            thread = semaph-> threads.right;
            ASSERT (thread-> state == SMT_THREAD_WAIT_SEMAPH);
            activate_thread (thread);
          }

        /*  Now delete the semaph                                            */
        mem_strfree (&semaph-> name);   /*  First we take its name           */
        mem_free (list_unlink (semaph));/*  ... then we take its life        */
      }
    return (0);
}


/*  ---------------------------------------------------------------------[<]-
    Function: semaph_wait

    Synopsis: When the semaphore value is > 0, subtracts 1 from the semaphore
    value.  If necessary, suspends the thread until this happens.  Threads are
    re-started on a FIFO basis.  Call as last statement in an action module.
    Returns 0 when completed normally, else returns -1 and sets smt_errno to
    one of these values:
    <Table>
    SMT_NOTREADY             smt_init() was not called, or failed
    </Table>
    ---------------------------------------------------------------------[>]-*/

int
semaph_wait (
    SEMAPH *semaph                      /*  Semaph to wait for               */
)
{
#if (defined (SMT_TRACE))
    trace ("semaph_wait");
#endif
    ASSERT (semaph);
    if (!smt_alive)                     /*  If SMT API was not correctly     */
      {                                 /*    initialised, forget it         */
        smt_errno = SMT_NOTREADY;
        return (-1);
      }

    if (semaph-> value > 0)             /*  If semaphore is > 0              */
        semaph-> value--;               /*    then we can continue           */
    else
      {                                 /*  Else break on semaphore          */
        break_wanted = BREAK_WAIT_SEMAPH;
        break_semaph = semaph;
      }
    return (0);
}


/*  ---------------------------------------------------------------------[<]-
    Function: semaph_signal

    Synopsis: Adds 1 to the semaphore value.
    Returns 0 when completed normally, else returns -1 and sets smt_errno to
    one of these values:
    <Table>
    SMT_NOTREADY             smt_init() was not called, or failed
    </Table>
    ---------------------------------------------------------------------[>]-*/

int
semaph_signal (
    SEMAPH *semaph                      /*  Semaph to signal                 */
)
{
#if (defined (SMT_TRACE))
    trace ("semaph_signal");
#endif
    ASSERT (semaph);
    if (!smt_alive)                     /*  If SMT API was not correctly     */
      {                                 /*    initialised, forget it         */
        smt_errno = SMT_NOTREADY;
        return (-1);
      }

    semaph-> value++;                   /*  Bump semaphore value             */
    if ((semaph-> value > 0)            /*  And re-activate thread if ready  */
    &&  (semaph-> threads.right != &semaph-> threads))
      {
        semaph-> value--;
        activate_thread (semaph-> threads.right);
      }

    return (0);                         /*    initialised, forget it         */
}


/*  ---------------------------------------------------------------------[<]-
    Function: lazy_creat

    Synopsis: Calls the standard creat() function without blocking.
    (Actually, calls the open() function, but with the O_CREAT flag.)
    Returns a file handle when the call succeeds, else returns -1 and sets
    errno to the cause of the error.  The file is always opened in binary
    mode, and you must process control characters yourself.  We use open()
    so that we can force O_NONBLOCK.
    If the call would block, returns -1, sets errno to EAGAIN, and calls
    recycle_module() to re-execute the current dialog module automatically.
    You can override this behavious by calling recycle_module (FALSE) after
    the return.  Sets the global variable "io_completed" to TRUE if the
    i/o access completed, with or without an error.
    ---------------------------------------------------------------------[>]-*/

int
lazy_creat (char *path, int mode)
{
    int rc;

#if (defined (SMT_TRACE))
    trace ("lazy_create: path=%s", path);
#endif
    rc = open (path, O_CREAT | O_WRONLY | O_TRUNC | O_NONBLOCK | O_BINARY,
               mode);

    set_io_completed (rc);
    return (rc);
}

/*  -------------------------------------------------------------------------
 *  set_io_completed -- internal
 *
 *  If the previous i/o operation completed okay or with a permanent error,
 *  sets the _io_completed flag to TRUE.  If the error was EWOULDBLOCK or
 *  EAGAIN (which we get depends on the system) sets _io_completed FALSE
 *  and calls recycle_module().
 */

local
set_io_completed (int rc)
{
    if (rc >= 0)
        _io_completed = TRUE;
    else
    if (errno == EAGAIN || errno == EWOULDBLOCK)
      {
        _io_completed = FALSE;
        errno = EAGAIN;                 /*  Always export EAGAIN to caller   */
      }
    else
        _io_completed = TRUE;

    if (!_io_completed)
        recycle_module (TRUE);
}


/*  ---------------------------------------------------------------------[<]-
    Function: lazy_creat_text

    Synopsis: Calls the standard creat() function without blocking.
    (Actually, calls the open() function, but with the O_CREAT flag.)
    Returns a file handle when the call succeeds, else returns -1 and sets
    errno to the cause of the error.  The file is always opened in text
    mode.  We use open() so that we can force O_NONBLOCK.
    If the call would block, returns -1, sets errno to EAGAIN, and calls
    recycle_module() to re-execute the current dialog module automatically.
    You can override this behavious by calling recycle_module (FALSE) after
    the return.  Sets the global variable "io_completed" to TRUE if the
    i/o access completed, with or without an error.
    ---------------------------------------------------------------------[>]-*/

int
lazy_creat_text (char *path, int mode)
{
    int rc;

#if (defined (SMT_TRACE))
    trace ("lazy_create_text: path=%s", path);
#endif
    rc = open (path, O_CREAT | O_WRONLY | O_TRUNC | O_NONBLOCK, mode);
    set_io_completed (rc);
    return (rc);
}


/*  ---------------------------------------------------------------------[<]-
    Function: lazy_open

    Synopsis: Calls the standard open() function without blocking.  Returns
    a file handle when the call succeeds, else returns -1 and sets errno to
    the cause of the error.  The file is always opened in binary mode, and
    you must process control characters yourself.
    If the call would block, returns -1, sets errno to EAGAIN, and calls
    recycle_module() to re-execute the current dialog module automatically.
    You can override this behavious by calling recycle_module (FALSE) after
    the return.  Sets the global variable "io_completed" to TRUE if the
    i/o access completed, with or without an error.

    Examples:
    handle_input  = lazy_open (filename, O_RDONLY);
    handle_output = lazy_open (filename, O_WRONLY | O_CREAT | O_TRUNC);
    handle_append = lazy_open (filename, O_WRONLY | O_CREAT | O_APPEND);
    if (io_completed && handle < 0)
        have error on file
    ---------------------------------------------------------------------[>]-*/

int
lazy_open (char *path, int flags)
{
    int rc;

#if (defined (SMT_TRACE))
    trace ("lazy_open: path=%s", path);
#endif
    rc = open (path, flags | O_NONBLOCK | O_BINARY, S_IREAD | S_IWRITE);
    set_io_completed (rc);
    return (rc);
}


/*  ---------------------------------------------------------------------[<]-
    Function: lazy_open_text

    Synopsis: As lazy_open(), but opens the file in text mode, on those
    platforms where this makes a difference.

    Examples:
    handle_input  = lazy_open (filename, O_RDONLY);
    handle_output = lazy_open (filename, O_WRONLY | O_CREAT | O_TRUNC);
    handle_append = lazy_open (filename, O_WRONLY | O_CREAT | O_APPEND);
    if (io_completed && handle < 0)
        have error on file
    ---------------------------------------------------------------------[>]-*/

int
lazy_open_text (char *path, int flags)
{
    int rc;

#if (defined (SMT_TRACE))
    trace ("lazy_open_text: path=%s", path);
#endif
    rc = open (path, flags | O_NONBLOCK, S_IREAD | S_IWRITE);
    set_io_completed (rc);
    return (rc);
}


#if (!defined __CYGNUS__)

/*  ---------------------------------------------------------------------[<]-
    Function: lazy_read

    Synopsis: Calls the standard read() function without blocking.  Returns
    the number of bytes read when the call succeeds, else returns -1 and sets
    errno to the cause of the error.
    If the call would block, returns -1, sets errno to EAGAIN, and calls
    recycle_module() to re-execute the current dialog module automatically.
    You can override this behavious by calling recycle_module (FALSE) after
    the return.  Sets the global variable "io_completed" to TRUE if the
    i/o access completed, with or without an error.
    ---------------------------------------------------------------------[>]-*/

int
lazy_read (int handle, char *buffer, size_t count)
{
    int rc;

#if (defined (SMT_TRACE))
    trace ("lazy_read: handle=%d bytes=%d", handle, count);
#endif
    rc = read (handle, buffer, count);
    set_io_completed (rc);
    return (rc);
}


/*  ---------------------------------------------------------------------[<]-
    Function: lazy_write

    Synopsis: Calls the standard write() function without blocking.  Returns
    the number of bytes written when the call succeeds, else returns -1 and
    sets errno to the cause of the error.
    If the call would block, returns -1, sets errno to EAGAIN, and calls
    recycle_module() to re-execute the current dialog module automatically.
    You can override this behavious by calling recycle_module (FALSE) after
    the return.  Sets the global variable "io_completed" to TRUE if the
    i/o access completed, with or without an error.
    ---------------------------------------------------------------------[>]-*/

int
lazy_write (int handle, char *buffer, size_t count)
{
    int rc;

#if (defined (SMT_TRACE))
    trace ("lazy_write: handle=%d bytes=%d", handle, count);
#endif
    rc = write (handle, buffer, count);
    set_io_completed (rc);
    return (rc);
}


/*  ---------------------------------------------------------------------[<]-
    Function: lazy_close

    Synopsis: Calls the standard close() function without blocking.
    Returns 0 when the call succeeds, else returns -1 and sets errno to
    the cause of the error.
    If the call would block, returns -1, sets errno to EAGAIN, and calls
    recycle_module() to re-execute the current dialog module automatically.
    You can override this behavious by calling recycle_module (FALSE) after
    the return.  Sets the global variable "io_completed" to TRUE if the
    i/o access completed, with or without an error.
    ---------------------------------------------------------------------[>]-*/

int
lazy_close (int handle)
{
    int rc;

#if (defined (SMT_TRACE))
    trace ("lazy_close: handle=%d", handle);
#endif
    rc = close (handle);
    set_io_completed (rc);
    return (rc);
}

#endif  /*  !__CYGNUS__  */

/*  ---------------------------------------------------------------------[<]-
    Function: raise_exception

    Synopsis: Sets the exception_raised flag to TRUE and sets the
    exception event as specified.  To raise an exception without setting
    the exception event, just do this: 'exception_raised = TRUE;'.
    ---------------------------------------------------------------------[>]-*/

void
raise_exception (event_t event)
{
#if (defined (SMT_TRACE))
    trace ("raise_exception");
#endif
    _exception_raised    = TRUE;
    _the_exception_event = event;
}


/*  ---------------------------------------------------------------------[<]-
    Function: call_exception

    Synopsis: Stacks the current position in the dialog and raises an
    exception.  dialog_return will cause the dialog to continue at the
    module following the one which called call_exception.  dialog_pop
    pops a dialog position but doesn't return there.
    ---------------------------------------------------------------------[>]-*/

void
_call_exception (THREAD *thread, event_t event)
{
#if (defined (SMT_TRACE))
    trace ("call_exception");
#endif
    _exception_raised    = TRUE;
    _the_exception_event = event;

    _dialog_push (thread);
}


void _dialog_push (THREAD *thread)
{
    LR_LOCATION
        *location;

        
    if (LR_location_cache)
      {
        location = LR_location_cache;
        LR_location_cache = location-> next;
        location-> next = NULL;
      }
    else
        location = memt_alloc (NULL, sizeof (LR_LOCATION));
        
    location-> state  = thread-> LR_savest;
    location-> event  = thread-> LR_event;
    location-> number = thread-> LR_number;
    location-> next   = thread-> LR_location_stack;
    thread-> LR_location_stack = location;
}


void _dialog_return (THREAD *thread)
{
    LR_LOCATION
        *location;

    if (thread-> LR_location_stack)
      {
        location = thread-> LR_location_stack;
        thread-> LR_location_stack = thread-> LR_location_stack-> next;
        
        thread-> LR_state  = location-> state;
        thread-> LR_event  = location-> event;
        thread-> LR_number = location-> number;

        thread-> LR_savest = thread-> LR_state;
        _dialog_jump = TRUE;
        
        location-> next = LR_location_cache;
        LR_location_cache = location;
      }
    else
      {
        fprintf (stdout,
            "State %d (%s) - Dialog-Return underflow\n", thread-> LR_state,
            thread-> queue-> agent-> LR_sname [thread-> LR_state]);
        exit (EXIT_FAILURE);
      }
}


void _dialog_pop (THREAD *thread)
{
    LR_LOCATION
        *location;

    if (thread-> LR_location_stack)
      {
        location = thread-> LR_location_stack;
        thread-> LR_location_stack = thread-> LR_location_stack-> next;
        
        location-> next = LR_location_cache;
        LR_location_cache = location;
      }
    else
      {
        fprintf (stdout,
            "State %d (%s) - Dialog-Pop underflow\n", thread-> LR_state,
            thread-> queue-> agent-> LR_sname [thread-> LR_state]);
        exit (EXIT_FAILURE);
      }
}

static void destroy_location_stack (LR_LOCATION *stack)
{
    LR_LOCATION
        *location;

    while (stack)
      {
        location = stack;
        stack = stack-> next;
        
        location-> next = LR_location_cache;
        LR_location_cache = location;
      }
}

/*  ---------------------------------------------------------------------[<]-
    Function: recycle_module

    Synopsis: Tells the SMT kernel to repeat the current action module.
    This is a simple way to re-attempt an i/o that returned a 'EAGAIN' or
    'EWOULDBLOCK' code.  The lazy file access functions automatically call
    this function in case they failed.  If you call this function with the
    wanted argument as FALSE, any previous recycle request is cancelled.
    ---------------------------------------------------------------------[>]-*/

void
recycle_module (Bool wanted)
{
#if (defined (SMT_TRACE))
    trace ("recycle_module");
#endif
    if (wanted)
        break_wanted = BREAK_RECYCLE;
    else
        break_wanted = BREAK_CONTINUE;
}


/*  ---------------------------------------------------------------------[<]-
    Function: smt_set_step

    Synopsis: Sets the current module 'step'.  Used to debug applications:
    suspect modules can set 'steps': the last of which is displayed when
    there is a SEGV crash.  Must be called with a string (constant) value.
    ---------------------------------------------------------------------[>]-*/

void
smt_set_step (const char *step)
{
#if (defined (SMT_DETECT))
    cur_step = (char *) step;
#endif
}


/*  ---------------------------------------------------------------------[<]-
    Function: smt_crash_report

    Synopsis: Returns a formatted string showing the current agent, state,
    module, step, event, and catch report, if any.
    ---------------------------------------------------------------------[>]-*/

char *
smt_crash_report (void)
{
    static char
        report [512];
    
#if (defined (SMT_DETECT))
    sprintf (report, "Abort at %s:%s:%s (%s, %s)",
                      cur_agent, cur_module, cur_step,
                      cur_state, cur_event);
#else
    sprintf (report, "Abort.  Compile with SMT_DETECT for more info.");
#endif
    return (report);
}
