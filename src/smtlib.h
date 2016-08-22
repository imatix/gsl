/*===========================================================================*
 *                                                                           *
 *  smtlib.h - SMT kernel prototypes                                         *
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

#ifndef _SMTLIB_INCLUDED                /*  Allow multiple inclusions        */
#define _SMTLIB_INCLUDED



/*---------------------------------------------------------------------------
 *  Structures used by the SMT API
 */

typedef struct _AGENT       AGENT;      /*  Agent descriptor                 */
typedef struct _METHOD      METHOD;     /*  Method descriptor                */
typedef struct _QID         QID;        /*  Queue ID descriptor              */
typedef struct _QUEUE       QUEUE;      /*  Queue descriptor                 */
typedef struct _EVENT       EVENT;      /*  Event descriptor                 */
typedef struct _LR_LOCATION LR_LOCATION;/*  Dialog location descriptor       */
typedef struct _THREAD      THREAD;     /*  Thread descriptor                */
typedef struct _SEMAPH      SEMAPH;     /*  Semaphore descriptor             */
typedef struct _ASYNC       ASYNC;      /*  Asynchronous descriptor          */
typedef struct _POLL        POLL;       /*  Polling descriptor               */


/*---------------------------------------------------------------------------
 *  Type definitions
 */

typedef short int event_t;              /*  Dialog FSM event type            */
typedef short int state_t;              /*  Dialog FSM state type            */
typedef void  (HOOK)  (THREAD *);       /*  Agent thread function            */

/*  Asynchronous functions enable an asynchronous interface to an agent.     */
/*  They are typically used for timeouts, and background processing.         */
/*  They set the arguments next_date and next_time to the moment when this   */
/*  thread next needs asynchronous attention.  If they don't need any such   */
/*  attention at any particular time soon they set the date to 99999999.     */
/*  Threads that require "immediate" attention, for polling for things,      */
/*  should using the polling interface (below).  They return one if a thread */
/*  was activated, -1 if an error occured, zero otherwise.                   */
typedef int   (ASYNCFN) (long wait_date,
                         long wait_time,
                         void *arg);

/*  Polling functions enable an "immediate" asynchronous interface to an     */
/*  agent.  They are used for polling for changes in external data, such as  */
/*  the availability of new data from a socket, which needs to be performed  */
/*  frequently, even while other threads are running.  Threads specify an    */
/*  interval in the number of state changes they should be activated, and    */
/*  the polling function will be called after that number of state changes   */
/*  has occurred, then rescheduled after that number of state changes.  The  */
/*  thread can alter the number of state changes it wishes to wait at any    */
/*  point.  When called these polling functions return one if a thread was   */
/*  activated, -1 if an error occured, and zero otherwise.                   */
typedef int   (POLLFN)  (int  state_interval,
                         void *arg);

/*  Execution types.                                                         */

typedef enum {
    SMT_EXEC_STEP,
    SMT_EXEC_NOBLOCK,
    SMT_EXEC_FULL
} SMT_EXEC_TYPE;

/*---------------------------------------------------------------------------
 *  Global variables
 */

extern int
    smt_errno;                          /*  Set when API detects an error    */
extern char
    *smt_errlist [];                    /*  Corresponding error messages     */
extern event_t
    _the_next_event,                    /*  May be set by thread code        */
    _the_external_event,                /*  Set by event_wait()              */
    _the_exception_event;               /*  May be set by thread code        */
extern Bool
    _exception_raised,                  /*  May be set by thread code        */
    _io_completed,                      /*  Last lazy I/O completed          */
    _repeat_module;                     /*  Repeat current action module     */

extern Bool
    signal_raised,                      /*  True after interrupt             */
    shutdown_pending;                   /*  When kill signal in progress     */
extern int
    signal_value;                       /*  Value of signal                  */
extern MEMTRN
    *smt_memtrn_;                       /*  Current transaction              */
extern LR_LOCATION 
    *LR_location_cache;                 /*  Cache for dialog locations       */
    

/*  A agent corresponds to one finite-state-machine, i.e. one program.       */
/*  All agents are linked into a list that is managed by the kernel.         */
/*  For each agent we must:                                                  */
/*      - declare the methods that the agent accepts                         */
/*      - create one or more threads to do the work                          */
/*      - tell the kernel how these threads use their event queues           */

struct _AGENT {                         /*  Agent descriptor                 */
    /*- Public -------------------------                                     */
    AGENT  *next, *prev;                /*    Doubly-linked list             */
    LIST    methods;                    /*    Methods accepted by agent      */
    LIST    queues;                     /*    Queues defined for agent       */
    char   *name;                       /*    Agent's name                   */
    Bool    router;                     /*    True if multi-thread/queue     */
    int     priority;                   /*    50=Low, 100=Normal, 200=High   */
    long    max_threads;                /*    Max. permitted threads, or 0   */
    long    cur_threads;                /*    Current number of threads      */
    long    top_threads;                /*    Max. number threads we had     */
    long    thread_tally;               /*    How many threads created       */
    long    switch_tally;               /*    How many context switches      */
    /*- Private ------------------------                                     */
    SYMBOL *symbol;                     /*    Entry into dictionary          */
    size_t  tcb_size;                   /*    Common tcb size for agent      */
    HOOK   *initialise;                 /*    Initialise-the-thread          */
    word   *LR_nextst;                  /*    Dialog next state table        */
    word   *LR_action;                  /*    Dialog action table            */
    word   *LR_offset;                  /*    Dialog vector offset table     */
    word   *LR_vector;                  /*    Dialog vector table            */
#if (defined (SMT_PROFILE))
    word   *LR_count;                   /*    Module execution count         */
    clock_t *LR_time;                   /*    Total module execution time    */
    word   *LR_flow;                    /*    Dialog flow count              */
#endif
    HOOK  **LR_module;                  /*    Dialog module table            */
    char  **LR_mname;                   /*    Module names, or null          */
    char  **LR_sname;                   /*    State names, or null           */
    char  **LR_ename;                   /*    Event names, or null           */
    event_t LR_defaults;                /*    Defaults state, or zero        */
    int     maxevent;                   /*    Number of events defined       */
    int     maxmodule;                  /*    Number of modules defined      */
    int     maxstate;                   /*    Number of states defined       */
    Bool    animate;                    /*    Animation permitted            */
};

/*  The method descriptor defines all the information for a method declared  */
/*  for an agent.  Methods are linked into a list attached to one agent.     */
/*  Each method points to its parent agent.   The method priority is used    */
/*  to order method event delivery to a agent thread.                        */

struct _METHOD {                        /*  Method descriptor                */
    /*- Public -------------------------                                     */
    METHOD *next, *prev;                /*    Doubly-linked list             */
    AGENT  *agent;                      /*    Parent agent descriptor        */
    char   *name;                       /*    Name of method                 */
    int     priority;                   /*    50=Low, 100=Normal, 200=High   */
    event_t event_number;               /*    Internal event number          */
    /*- Private ------------------------                                     */
    SYMBOL *symbol;                     /*    Entry into dictionary          */
};

/*  The queue ID block is a portable definition of a queue.  We can send     */
/*  this to other parties.  The lookup_queue() function lets us access a     */
/*  queue descriptor given the queue ID.                                     */

struct _QID {                           /*  Queue ID block                   */
    /*- Public -------------------------                                     */
    long node;                          /*    Location of queue (zero)       */
    long ident;                         /*    Queue ID number (1..n)         */
};

/*  The queue descriptor defines all the information for a queue.  Queues    */
/*  are linked into a list attached to one agent.  Each queue points to      */
/*  its parent agent.  Each queue holds a list of events stored in the       */
/*  queue, and a list of threads that can process the queue.  When this      */
/*  list holds more than one thread, events are routed depending on which    */
/*  threads are ready to handle them.                                        */

struct _QUEUE {                         /*  Queue descriptor                 */
    /*- Public -------------------------                                     */
    QUEUE  *next, *prev;                /*    Doubly-linked list             */
    AGENT  *agent;                      /*    Parent agent descriptor        */
    LIST    events;                     /*    Events in queue                */
    LIST    threads;                    /*    Threads for queue              */
    QID     qid;                        /*    Queue ID descriptor            */
    int     max_events;                 /*    Maximum allowed events         */
    int     cur_events;                 /*    Current number of events       */
    /*- Private ------------------------                                     */
    SYMBOL *symbol;                     /*    Entry into dictionary          */
    int     timed_events;               /*    Nbr of events with timeout     */
    Bool    shutdown;                   /*    Shutdown event sent?           */
};

/*  The queued event descriptor defines all the information for a queued     */
/*  event.  Events are linked into a list attached to one queue.  Each       */
/*  event points to its parent queue.                                        */

struct _EVENT {                         /*  Event descriptor                 */
    /*- Public -------------------------                                     */
    EVENT  *next, *prev;                /*    Doubly-linked list             */
    QUEUE  *queue;                      /*    Parent queue descriptor        */
    QID     sender;                     /*    Replies come back here         */
    char   *name;                       /*    Name of event                  */
    size_t  body_size;                  /*    Size of event body in bytes    */
    byte   *body;                       /*    Event body                     */
    char   *accept_event;               /*    Reply if we accept event       */
    char   *reject_event;               /*    Reply if we reject event       */
    char   *expire_event;               /*    Reply if we expire event       */
    time_t  timeout;                    /*    Expires at this time (or 0)    */
    /*- Private ------------------------                                     */
    size_t  size;                       /*    Size of complete event block   */
    int     priority;                   /*    When known from method         */
    event_t event_number;               /*    When known from method         */
};


/*  The location descriptor defines a place in the dialog.  Thread           */
/*  descriptors contain a stack of these.                                    */

struct _LR_LOCATION {                   /*   To store position for           */
    LR_LOCATION *next;                  /*   Dialog-Call/Dialog-Return       */
    state_t state;
    event_t event;
    int     number;
};


/*  The thread descriptor defines all the information for a thread.          */
/*  Threads are linked into a list attached to one queue.  Each thread       */
/*  points to its parent queue.                                              */

struct _THREAD {                        /*  Thread block                     */
    /*- Public -------------------------                                     */
    THREAD  *next, *prev;               /*    Doubly-linked list             */
    QUEUE   *queue;                     /*    Parent queue descriptor        */
    long     thread_id;                 /*    Thread identifier number       */
    char    *name;                      /*    Name of thread                 */
    Bool     animate;                   /*    Animate this thread            */
    void    *tcb;                       /*    Thread context block (TCB)     */
    EVENT   *event;                     /*    Last-received event            */
    MEMTRN  *memtrn;                    /*    Transaction for mem allocat'n  */
    HOOK    *atdestroy;                 /*    Called when thread destroyed   */
    /*- Private ------------------------                                     */
    SYMBOL  *symbol;                    /*    Entry into dictionary          */
    THREAD  *left, *right;              /*    Active/passive linked list     */
    int      state;                     /*    Thread activity state          */
    LR_LOCATION
            *LR_location_stack;         /*    Subdialog stack comes here     */
    event_t  LR_event;                  /*    Event for state transition     */
    state_t  LR_state;                  /*    Current dialog state           */
    word     LR_index;                  /*    Index into LR vector           */
    word     LR_number;                 /*    Number of next module          */
    state_t  LR_savest;                 /*    Saved dialog state             */
    event_t  the_next_event;            /*    Saved the_next_event           */
    event_t  the_external_event;        /*    Saved the_external_event       */
    /* - Loop monitoring stuff --------                                      */
    event_t *loop_event;                /*  Preceding events                 */
    int      loop_index,                /*    Current position in list       */
             loop_length;               /*    Current loop length            */
    long     loop_count;                /*    Event count in current loop    */
};

/*---------------------------------------------------------------------------
*  Function prototypes
 */

#ifdef __cplusplus
extern "C" {
#endif

int      smt_init           (void);
int      smt_term           (void);
int      smt_exec_full      (void);
Bool     smt_exec_step      (void);
Bool     smt_exec           (SMT_EXEC_TYPE exec_type);
Bool     smt_active         (void);
void     smt_set_console    (const QID *qid);
void     smt_set_timer      (const QID *qid);
int      smt_atexit         (function exit_function);
void     smt_shutdown       (void);
char    *smt_crash_report   (void);
void     smt_set_step       (const char *step);

int      schedule_async_nonblock   (ASYNCFN *asyncfn, void *arg, 
                                    int priority, long date, long time);
int      deschedule_async_nonblock (ASYNCFN *asyncfn, void *arg);
void     register_async_blocking   (ASYNCFN *asyncfn, void *arg);

int      schedule_polling_function   (POLLFN  *pollfn,  void *arg,
                                      int poll_interval);
int      deschedule_polling_function (POLLFN *pollfn, void *arg);

AGENT   *agent_declare      (const char *agent_name);
AGENT   *agent_lookup       (const char *agent);
int      agent_destroy      (AGENT *agent);

METHOD  *method_declare     (AGENT *agent, const char *method_name,
                             event_t number, int priority);
METHOD  *method_lookup      (const AGENT *agent, const char *method);
int      method_destroy     (METHOD *method);

QUEUE   *queue_create       (AGENT *agent, int max_events);
QUEUE   *queue_lookup       (const QID *qid);
int      queue_destroy      (QUEUE *queue);
int      queue_flush        (QUEUE *queue);
int      queue_deliver      (QUEUE *queue, THREAD *thread);

int      event_send         (const QID *to, const QID *from, char *name,
                             void *body, size_t body_size, char *accept,
                             char *reject, char *expire, word timeout);
EVENT   *event_accept       (QUEUE *queue, EVENT *event);
int      event_reject       (QUEUE *queue, EVENT *event);
int      event_expire       (QUEUE *queue, EVENT *event);
int      event_discard      (QUEUE *queue, EVENT *event);
EVENT   *event_iterate      (QUEUE *queue, EVENT *event);
int      event_destroy      (EVENT *event);
void     event_wait         (void);

THREAD  *thread_create_     (const char *agent, const char *thread);
/* On some platforms thread_create is an existing system call                */
#define thread_create(a,t) thread_create_(a,t)
THREAD  *thread_lookup      (const char *agent, const char *thread);
int      thread_destroy     (THREAD *thread, Bool cleanup);

SEMAPH  *semaph_create      (const char *name, int value);
SEMAPH  *semaph_lookup      (const char *name);
int      semaph_destroy     (SEMAPH *semaph);
int      semaph_wait        (SEMAPH *semaph);
int      semaph_signal      (SEMAPH *semaph);

int      lazy_creat         (char *path, int mode);
int      lazy_creat_text    (char *path, int mode);
int      lazy_open          (char *path, int flags);
int      lazy_open_text     (char *path, int flags);
int      lazy_read          (int handle, char *buffer, size_t count);
int      lazy_write         (int handle, char *buffer, size_t count);
int      lazy_close         (int handle);

void     raise_exception    (event_t event);
void     _call_exception    (THREAD *thread, event_t event);
void     recycle_module     (Bool wanted);
void     _dialog_push       (THREAD *thread);
void     _dialog_return     (THREAD *thread);
void     _dialog_pop        (THREAD *thread);

#ifdef __cplusplus
}
#endif


/*---------------------------------------------------------------------------
 *  Macros
 */

/*  Send an simple event: just a name, and string body                       */
#define SEND(q,name,s)      event_send ((q), NULL, (name), (byte *) (s), \
                                        strlen(s) + 1, NULL, NULL, NULL, 0)


/*---------------------------------------------------------------------------
 *  Constants
 */

/*  Possible values for smt_errno                                            */

#define SMT_NOERRORS        0           /*  No errors                        */
#define SMT_EVENTEXISTS     1           /*  Event already declared           */
#define SMT_INTERNALERROR   2           /*  Agent had internal error         */
#define SMT_METHODEXISTS    3           /*  Method already declared          */
#define SMT_NOSUCHEVENT     4           /*  Unknown event name               */
#define SMT_NOSUCHMETHOD    5           /*  Unknown method name              */
#define SMT_NOSUCHQUEUE     6           /*  Event queue not defined          */
#define SMT_NOSUCHSEMAPH    7           /*  Unknown semaphore name           */
#define SMT_NOSUCHAGENT     8           /*  Agent not defined                */
#define SMT_NOSUCHTHREAD    9           /*  Thread not declared              */
#define SMT_NOTREADY        10          /*  SMT API not initialised          */
#define SMT_OUTOFMEMORY     11          /*  Not enough heap memory left      */
#define SMT_QUEUEISEMPTY    12          /*  Event queue is empty             */
#define SMT_QUEUEISFULL     13          /*  Event queue is full              */
#define SMT_SEMAPHEXISTS    14          /*  Semaphore already exists         */
#define SMT_AGENTEXISTS     15          /*  Agent already declared           */
#define SMT_AGENTNOTREADY   16          /*  Agent not initialised            */
#define SMT_THREADEXISTS    17          /*  Thread already declared          */
#define SMT_TOOMANYTHREADS  18          /*  Too many threads                 */

/*  Values for priority arguments                                            */

#define SMT_PRIORITY_NULL   0           /*  Illegal (null) priority          */
#define SMT_PRIORITY_MIN    1           /*  Lowest priority                  */
#define SMT_PRIORITY_LOW    50          /*  Low priority                     */
#define SMT_PRIORITY_NORMAL 100         /*  Default priority                 */
#define SMT_PRIORITY_HIGH   200         /*  High priority                    */
#define SMT_PRIORITY_MAX    255         /*  Highest priority                 */

/*  Values for agent_declare () type argument                                */

#define SMT_AGENT_NORMAL    0           /*  1 queue for 1 thread             */
#define SMT_AGENT_ROUTER    1           /*  1 queue for n threads            */

/*  Dialog constants (event is signed integer)                               */

#define SMT_NULL_STATE      -1          /*  Thread not initialised           */
#define SMT_TERM_EVENT      -1          /*  Terminate the thread             */
#define SMT_NULL_EVENT      -2          /*  No event provided                */
#define SMT_ACTION_STOP     0xFFFFL     /*  End of module list               */

/*  Values for thread activity state                                         */

#define SMT_THREAD_ACTIVE       1       /*  Active and executing             */
#define SMT_THREAD_PASSIVE      2       /*  Passive, between states          */
#define SMT_THREAD_WAIT_EVENT   3       /*  Waiting for an event             */
#define SMT_THREAD_WAIT_SEMAPH  4       /*  Waiting for a semaphore          */

/*  Maximum time, in seconds, allowed for a normal shutdown                  */

#define SMT_LOOP_DETECTION      2       /*  After this, we abort             */

#define SMT_SIGNAL_INT      1           /*  SIGINT - interrupted             */
#define SMT_SIGNAL_TERM     2           /*  SIGTERM - cancelled              */
#define SMT_SIGNAL_SEGV     3           /*  SIGSEGV - segment violation      */
#define SMT_SIGNAL_ALRM     4           /*  SIGALRM - timer signal           */
#define SMT_SIGNAL_USER     5           /*  User shutdown request            */

#define call_exception(event) _call_exception (thread, event)

#endif                                  /*  Include smtlib.h                 */
