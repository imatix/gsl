/*===========================================================================*
 *  sysmana.c - system manager agent                                         *
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
 *===========================================================================*/

#include "sfl.h"                        /*  SFL library header file          */
#include "smt3.h"                       /*  SMT kernel header file           */
#include "sysman.h"                     /*  sysman definitions               */


/*- Definitions -------------------------------------------------------------*/

#define AGENT_NAME       "sysmana"      /*  Our public name                  */
#define BUFFER_SIZE      4096           /*  Working buffer (greater than msg)*/

struct _TCB;

typedef struct _TASK {                  /*  Task descriptor                  */
    struct _TASK                        /*                                   */
           *next, *prev;                /*    Doubly-linked list             */
    QID     qid;                        /*    Task thread queue id           */
    QID     slotq;                      /*    Time slot queue id             */
    THREAD *thread;
    THREAD *slot_thread;
    struct _TCB
           *tcb;
    char   *name;                       /*    Name of task                   */
    char   *workdir;                    /*    Working directory, if any      */
    char   *user;                       /*    Desired user name, if any      */
    char   *group;                      /*    Desired group name, if any     */
    char   *std_in;                     /*    Desired stdin device           */
    char   *std_out;                    /*    Desired stdout device          */
    char   *std_err;                    /*    Desired stderr device          */
    char   *run_idle;                   /*    Execute when task idles        */
    char   *run_startup;                /*    Execute to run task            */
    char   *run_cancel;                 /*    Execute when cancelling task   */
    char   **env_list;                  /*    Environment symbol list        */
    Bool   required;                    /*    Is task mandatory              */
    Bool   restart;                     /*    Restart if killed              */
    PROCESS pid;                        /*    Running process ID             */
} TASK;

static NODE
    tasks;                              /*  Task list header                 */

typedef struct _TCB {                   /*  Thread context block:            */
    event_t thread_type;                /*    Thread type indicator          */
    sock_t  handle;                     /*    Handle for i/o, if used        */
    dbyte   msg_id;                     /*    Message identifier code        */
    char    msg_body [BUFFER_SIZE];     /*    Incoming message               */
    dbyte   msg_size;                   /*    Size of incoming message       */
    TASK   *task;                       /*    For task threads               */
} TCB;

/*- Function prototypes -----------------------------------------------------*/

static void    sysmana_term      (void);
static Bool    task_create_sym   (SYMBOL *symbol);
static TASK   *task_create       (char *name);
static void    task_destroy      (TASK *task);
static void    free_task_values  (TASK *task);
static void    canonise_name     (char *name);
static void    read_tcp_block    (THREAD *thread);
static void    put_control_msg   (THREAD *thread, dbyte ident, void *body);
static dbyte   get_control_msg   (THREAD *thread, char *body);
static Bool    address_task      (TCB *tcb);
static PROCESS run_command       (TASK *task, char *command, Bool wait);
static Bool    system_active     (void);


/*- Global variables used in this source file only --------------------------*/

static TCB
    *tcb;                               /*  Address thread context block     */
static QID
    operq,                              /*  Operator console event queue     */
    sockq,                              /*  Socket agent event queue         */
    tranq,                              /*  Transfer agent event queue       */
    logq;                               /*  Logging agent event queue        */
static Bool
    system_active_flag = FALSE;         /*  Global active/disabled state     */
static byte
    smt_msg_body [BUFFER_SIZE];         /*  Message between SMT agents       */
static char
    msg_buffer [BUFFER_SIZE];           /*  Buffer used when display message */


/*- Global variables set from ini file or filled with default values --------*/

static char
    *logfile_name     = NULL,           /*  Main log file                    */
    *startup_command  = NULL,           /*  Run this when going active       */
    *shutdown_command = NULL,           /*  Run this when going down         */
    *default_user     = NULL,           /*  Default User ID to run tasks as  */
    *default_group    = NULL,           /*  Default Group ID to run tasks as */
    *listen_port      = NULL;           /*  Port to listen on                */
SYMTAB
    *tasks_table      = NULL;           /*  Task list from configuration file*/


#include "sysmana.d"                    /*  Include dialog data              */

/********************   INITIALISE AGENT - ENTRY POINT   *********************/

int sysmana_init (
    char *port)                         /*  Port to listen on                */
{
    AGENT   *agent;                     /*  Handle for our agent             */
    THREAD  *thread;                    /*  Handle to various threads        */
#   include "sysmana.i"                 /*  Include dialog interpreter       */

    /*  Shutdown event comes from Kernel                                     */
    declare_smtlib_shutdown      (shutdown_event, SMT_PRIORITY_MAX);

    /*  Reply events from socket agent                                       */
    declare_smtsock_ok           (ok_event,            0);
    declare_smtsock_read_ok      (ok_event,            0);
    declare_smtsock_closed       (sock_closed_event,   0);
    declare_smtsock_error        (sock_error_event,    0);
    declare_smtsock_timeout      (sock_timeout_event,  0);
    declare_smtsock_read_timeout (sock_timeout_event,  0);
    declare_smtsock_read_closed  (sock_error_event,    0);

    /*  Reply events from transfer agent                                     */
    declare_smttran_get_block    (ok_event,            0);
    declare_smttran_put_block    (SMT_NULL_EVENT,      0);
    declare_smttran_putb_ok      (SMT_NULL_EVENT,      0);
    declare_smttran_getb_ok      (ok_event,            0);
    declare_smttran_closed       (sock_closed_event,   0);
    declare_smttran_error        (sock_error_event,    0);

    /*  Reply events from time slot agent                                    */
    declare_smtslot_on           (start_auto_event,    0);
    declare_smtslot_off          (pause_auto_event,    0);

    /*  Reply events from timer agent                                        */
    declare_smttime_reply        (task_alarm_event,    0);

    /*  Private methods used to pass initial thread arguments                */
    method_declare (agent, "_CONTROL_MASTER", control_master_event, 0);
    method_declare (agent, "_CONTROL",        control_event,        0);
    method_declare (agent, "_TASK",           task_event,           0);

    /*  Private methods used between control and task threads                */
    method_declare (agent, "_TASK_STATUS",    status_req_event,     0);
    method_declare (agent, "_TASK_START",     start_req_event,      0);
    method_declare (agent, "_TASK_PAUSE",     pause_req_event,      0);
    method_declare (agent, "_TASK_STOP",      stop_req_event,       0);
    method_declare (agent, "_TASK_FSTOP",     stop_force_event,     0);
    method_declare (agent, "_TASK_FSTART",    start_force_event,    0);
    method_declare (agent, "_TASK_OK",        ok_event,             0);
    method_declare (agent, "_TASK_RUNNING",   running_event,        0);
    method_declare (agent, "_TASK_PAUSED",    paused_event,         0);
    method_declare (agent, "_TASK_STOPPED",   stopped_event,        0);
    method_declare (agent, "_TASK_ERROR",     error_event,          0);

    /*  Set listen port                                                      */
    ASSERT(port);
    listen_port = port;

    /*  Ensure that logging agent is running, and create new thread          */
    smtlog_init ();
    if ((thread = thread_create (SMT_LOGGING, "")) != NULL)
        logq = thread->queue->qid;        /*  Get logging queue id         */
    else
        return (-1);

    /*  Ensure that operator console is running, else start it up            */
    smtoper_init ();
    if ((thread = thread_lookup (SMT_OPERATOR, "")) != NULL)
        operq = thread->queue->qid;
    else
        return (-1);

    /*  Ensure that socket agent is running, else start it up                */
    smtsock_init ();
    if ((thread = thread_lookup (SMT_SOCKET, "")) != NULL)
        sockq = thread->queue->qid;
    else
        return (-1);

    /*  Ensure that transfer agent is running, else start it up              */
    smttran_init ();
    if ((thread = thread_lookup (SMT_TRANSFER, "")) != NULL)
        tranq = thread->queue->qid;
    else
        return (-1);

    /*  Ensure that timer agent is running, else start it up                 */
    if (smttime_init ())
        return (-1);

    /*  Ensure that slot agent is running, else start it up                  */
    smtslot_init ();

    /*  Create initial threads to manage master ports                        */
    thread = thread_create (AGENT_NAME, "");
    SEND (&thread->queue->qid, "_CONTROL_MASTER", "");
    ((TCB *) thread->tcb)->thread_type = control_master_event;
    ((TCB *) thread->tcb)->handle      = 0;

    node_reset (&tasks);                /*  Reset task list                  */
    smt_atexit (sysmana_term);

    /*  Signal okay to caller that we initialised okay                       */
    return (0);
}

/*  We always come here when the application shuts down; if there are any
    tasks still running (normally impossible, but we want to be certain)
    then this is a good time to kill them.                                   */

static void
sysmana_term (void)
{
    TASK
        *task;                          /*  Task in list                     */

    for (task = tasks.next; (NODE *) task != &tasks; task = task->next)
      {
        if (task->pid > 0
        &&  process_status (task->pid) == PROCESS_RUNNING)
          {
            printf ("sysman: ATEXIT PROCESSING FOR %s...\n", task->name);
            process_kill (task->pid);
            run_command (task, task->run_idle, TRUE);
          }
      }
}


/*************************   INITIALISE THE THREAD   *************************/

MODULE initialise_the_thread (THREAD *thread)
{
    /*  We don't set the_next_event because we expect an argument event      */
    /*  to supply the initial event for the dialog state machine.            */

    tcb = thread->tcb;                 /*  Point to thread's context        */
}


/***************************  =================  *****************************/
/*                             MASTER HANDLING                               */
/***************************  =================  *****************************/


/***********************   LOAD GLOBAL CONFIGURATION   ***********************/

MODULE load_global_configuration (THREAD *thread)
{
    char
        *keyword,                       /*  Keyword or section name          */
        *value;                         /*  Keyword value                    */
    FILE
        *instream;                      /*  Stream for opened ini file       */

    tcb = thread->tcb;                 /*  Point to thread's context        */

    instream = file_locate ("PATH", "sysman", "ini");
    if (instream == NULL)
      {
        sprintf (msg_buffer, "sysman: can't locate sysman.ini");
        send_smtoper_error (&operq, msg_buffer);
        raise_exception (exception_event);
      }
    else
      {
        /*  Find [Setup] section and parse it                                */
        /*  Each line should contain "keyword=somevalue"                     */

        logfile_name = mem_strdup ("sysman.log");
        if (ini_find_section (instream, "setup", TRUE))
            while (ini_scan_section (instream, &keyword, &value))
              {
                if (streq (keyword, "logfile"))
                  {
                    mem_strfree (&logfile_name);
                    logfile_name = mem_strdup (value);
                  }
                else
                if (streq (keyword, "command")) {
                    if (system (value)) {
                        sprintf (msg_buffer, "sysman: can't execute '%s'", value);
                        send_smtoper_error (&operq, msg_buffer);
                        raise_exception (exception_event);
                    }
                }
                else
                if (streq (keyword, "startup"))
                  {
                    mem_strfree (&startup_command);
                    startup_command = mem_strdup (value);
                  }
                else
                if (streq (keyword, "shutdown"))
                  {
                    mem_strfree (&shutdown_command);
                    shutdown_command = mem_strdup (value);
                  }
                else
                if (streq (keyword, "default-user"))
                  {
                    mem_strfree (&default_user);
                    default_user = mem_strdup (value);
                  }
                else
                if (streq (keyword, "default-group"))
                  {
                    mem_strfree (&default_group);
                    default_group = mem_strdup (value);
                  }
                else
                  {
                    sprintf (msg_buffer,
                             "sysman: bad keyword '%s' in sysman.ini", keyword);
                    send_smtoper_warning (&operq, msg_buffer);
                  }
              }

        /*  Find [Tasks] section and parse it                                */
        /*  Each line should contain "Name=somevalue"                        */

        sym_delete_table (tasks_table);
        tasks_table = sym_create_table ();
        if (ini_find_section (instream, "tasks", TRUE))
            while (ini_scan_section (instream, &keyword, &value))
              {
                if (streq (keyword, "name"))
                    /*  Add each task to run into the tasks_table. The symbol
                     *  name is the task name.
                     */
                    sym_create_symbol (tasks_table, value, "");
                else
                  {
                    sprintf (msg_buffer,
                        "sysman: bad keyword '%s' in sysman.ini", keyword);
                    send_smtoper_warning (&operq, msg_buffer);
                  }
              }
        file_close (instream);
      }
}


/**************************   CREATE TASK THREADS   **************************/

MODULE create_task_threads (THREAD *thread)
{
    tcb = thread->tcb;                 /*  Point to thread's context        */

    /*  Traverse tasks_table, creating a task thread for each symbol in it   */
    ASSERT (tasks_table);
    sym_exec_all (tasks_table, task_create_sym);
}


/*  -------------------------------------------------------------------------
 *  task_create_sym
 *
 *  Internal wrapper function used when calling task_create () from
 *  sym_exec_all().
 */

static Bool
task_create_sym (SYMBOL *symbol)
{
    char
        *name;

    /*  task_create () modifies name, must create a temporary copy           */
    name = mem_strdup (symbol ->name);
    task_create (name);
    mem_strfree (&name);
    return (TRUE);
}


/*  -------------------------------------------------------------------------
 *  task_create
 *
 *  Creates a new task entry and attaches it to the tasks list.  The
 *  new entry is initialised with the task name and other arguments.
 *  Creates a new thread to manage the task, and sends a _TASK event
 *  to the new thread.  Returns a pointer to the fresh task entry, or
 *  NULL if there was an error.  In the latter case, sends an error message
 *  to the console.  Task names are canonised as follows:
 *    - whitespace is replaced by underlines.
 */

static TASK *
task_create (char *name)
{
    TCB
        *tcb;                           /*  Address thread context block     */
    TASK
        *task;                          /*  Freshly created task             */
    THREAD
        *task_thread,                   /*  Thread that manages task         */
        *slot_thread;                   /*  Thread managing time slots       */

    canonise_name (name);
    if ((task = node_create (tasks.prev, sizeof (TASK))) != NULL)
      {
        if (thread_lookup (SMT_SLOT, name) != NULL)
          raise_exception (exception_event);
        if (thread_lookup (AGENT_NAME, name) != NULL)
          raise_exception (exception_event);

        slot_thread = thread_create (SMT_SLOT,   name);
        task_thread = thread_create (AGENT_NAME, name);
        if (slot_thread && task_thread)
          {
            /*  Send initial event to task thread and initialise task TCB    */
            SEND (&task_thread->queue->qid, "_TASK", "");
            tcb = task_thread->tcb;

            tcb->thread_type  = task_event;
            tcb->handle       = 0;
            tcb->task         = task;

            task->name        = mem_strdup (name);
            task->qid         = task_thread->queue->qid;
            task->slotq       = slot_thread->queue->qid;
            task->thread      = task_thread;
            task->slot_thread = slot_thread;
            task->tcb         = tcb;
            task->workdir     = NULL;
            task->user        = NULL;
            task->group       = NULL;
            task->std_in      = NULL;
            task->std_out     = NULL;
            task->std_err     = NULL;
            task->run_idle    = NULL;
            task->run_startup = NULL;
            task->run_cancel  = NULL;
            task->env_list    = NULL;
            task->required    = FALSE;
            task->restart     = FALSE;
            task->pid         = 0;     /*  Not running                      */
          }
        else
          {
            task_destroy (task);
            task = NULL;                /*  Could not create thread          */
          }
      }
    if (!task)
      {
        sprintf (msg_buffer, "sysman: couldn't create thread for task '%s'",
                             name);
        lsend_smtoper_error (&operq, &task->qid, NULL, NULL, NULL, 0,
                             msg_buffer);
        raise_exception (exception_event);
      }
    return (task);
}


/*  -------------------------------------------------------------------------
 *  task_destroy
 *
 *  Destroys the specified task.
 */

static void
task_destroy (TASK *task)
{
    free_task_values (task);
    mem_strfree (&task->name);
    node_destroy (task);
}

/*  Free any allocated fields in the task block                              */
/*  except name, which is not reloaded from task .ini file                   */

static void
free_task_values (TASK *task)
{
    mem_strfree (&task->workdir);
    mem_strfree (&task->user);
    mem_strfree (&task->group);
    mem_strfree (&task->std_in);
    mem_strfree (&task->std_out);
    mem_strfree (&task->std_err);
    mem_strfree (&task->run_idle);
    mem_strfree (&task->run_startup);
    mem_strfree (&task->run_cancel);
    strtfree (task->env_list);
    task->env_list = NULL;
}


static void
canonise_name (char *name)
{
    while (*name) {
        if (isspace (*name))
            *name = '_';
        name++;
    }
}


/**************************   OPEN MASTER LOG FILE   *************************/

MODULE open_master_log_file (THREAD *thread)
{
    tcb = thread->tcb;                 /*  Point to thread's context        */
    send_smtlog_append   (&logq, logfile_name);
    send_smtoper_set_log (&operq, SMT_LOGGING, "");
}


/***********************   OPEN CONTROL MASTER SOCKET   **********************/

MODULE open_control_master_socket (THREAD *thread)
{
    tcb = thread->tcb;                 /*  Point to thread's context        */

    /*  Listen for connects to local machine only                            */
    ip_passive = inet_addr ("127.0.0.1");
    tcb->handle = passive_TCP (listen_port, 5);
    if (tcb->handle == INVALID_SOCKET)
      {
        sprintf (msg_buffer, "sysman: couldn't open control port %s",
                             listen_port);
        send_smtoper_error (&operq, msg_buffer);

        sprintf (msg_buffer, "sysman: %s",
                             connect_errlist [connect_error ()]);
        send_smtoper_error (&operq, msg_buffer);

        raise_exception (fatal_event);
      }
    else
      {
        sprintf (msg_buffer, "sysman: listening on port %s", listen_port);
        send_smtoper_info (&operq, msg_buffer);
      }
}


/*************************   WAIT FOR SOCKET INPUT   *************************/

MODULE wait_for_socket_input (THREAD *thread)
{
    tcb = thread->tcb;                 /*  Point to thread's context        */

    /*  Wait for input to arrive on socket, 30-second timeout                */
    send_smtsock_input (&sockq, 30, tcb->handle, 0);
}


/***********************   ACCEPT CONTROL CONNECTION   ***********************/

MODULE accept_control_connection (THREAD *thread)
{
    sock_t
        slave_socket;                   /*  Connected socket                 */
    THREAD
        *child_thread;                  /*  Handle to child threads          */

    tcb = thread->tcb;                 /*  Point to thread's context        */

    slave_socket = accept_socket (tcb->handle);
    if (slave_socket != INVALID_SOCKET)
      {
        child_thread = thread_create (AGENT_NAME, "");
        if (child_thread)
          {
            SEND (&child_thread->queue->qid, "_CONTROL", "");
            ((TCB *) child_thread->tcb)->thread_type = control_event;
            ((TCB *) child_thread->tcb)->handle      = slave_socket;
          }
      }
    else
    if (errno != EAGAIN)
      {
        sprintf (msg_buffer, "sysman: could not accept connection: %s",
                             sockmsg ());
        send_smtoper_error (&operq, msg_buffer);

        raise_exception (exception_event);
      }
}


/**********************   REFRESH LOGICAL SYSTEM STATE   *********************/

MODULE refresh_logical_system_state (THREAD *thread)
{
    char
        *command;                       /*  Command we want to run           */

    tcb = thread->tcb;                 /*  Point to thread's context        */
    if (system_active_flag != system_active ()) {
        system_active_flag = system_active ();
        command = system_active_flag? startup_command: shutdown_command;
        if (command && *command) {
            sprintf (msg_buffer, "sysman: system command '%s'", command);
            send_smtoper_info (&operq, msg_buffer);
            if (command && process_create (
                command,                    /*  Command to execute               */
                NULL,                       /*  Arguments for command            */
                NULL,                       /*  Use current working directory    */
                NULL, NULL, NULL,           /*  Don't redirect stdin/out/err     */
                NULL,                       /*  User current environment         */
                TRUE)                       /*  And wait until it finished       */
              == 0) {
                sprintf (msg_buffer,
                    "sysman: cannot execute '%s': %s", command, strerror (errno));
                send_smtoper_error (&operq, msg_buffer);
            }
        }
    }
}


/*  Returns TRUE if any required task is running; FALSE if all required
 *  tasks are stopped.
 */

static Bool
system_active (void)
{
    TASK
        *task;                          /*  Task in list                     */

    for (task = tasks.next; (NODE *) task != &tasks; task = task->next)
        if (task->required && task->pid)
            return (TRUE);              /*  At least one task is up          */

    return (FALSE);
}


/***************************  ==================  ****************************/
/*                             CONTROL HANDLING                              */
/***************************  ==================  ****************************/


/**************************   READ CONTROL MESSAGE   *************************/

MODULE read_control_message (THREAD *thread)
{
    read_tcp_block (thread);
}

static void
read_tcp_block (THREAD *thread)
{
    tcb = thread->tcb;                 /*  Point to thread's context        */

    /*  Ask the transfer agent to read a block from the socket               */
    send_smttran_get_block (&tranq, tcb->handle, NULL);
    event_wait ();                      /*  Wait for reply event             */
}


/*************************   CHECK CONTROL MESSAGE   *************************/

MODULE check_control_message (THREAD *thread)
{
    tcb = thread->tcb;                 /*  Point to thread's context        */

    if (the_external_event == ok_event)
      {
        tcb->msg_id = get_control_msg (thread, tcb->msg_body);
        switch (tcb->msg_id)
          {
            case SYSMAN_LIST:    the_next_event = list_event;    break;
            case SYSMAN_HALT:    the_next_event = halt_event;    break;
            case SYSMAN_STATUS:  the_next_event = status_event;  break;
            case SYSMAN_START:   the_next_event = start_event;   break;
            case SYSMAN_PAUSE:   the_next_event = pause_event;   break;
            case SYSMAN_STOP:    the_next_event = stop_event;    break;
            case SYSMAN_REFRESH: the_next_event = refresh_event; break;
            default:             the_next_event = error_event;   break;
          }
      }
    else
        raise_exception (the_external_event);
}


/*  -------------------------------------------------------------------------
 *  get_control_msg -- local
 *
 *  Decodes the thread body to give a message id and a message body.  Puts
 *  the message body in the specified string and returns the message id.
 */

static dbyte
get_control_msg (THREAD *thread, char *body)
{
    struct_smttran_getb_ok
        *msg = NULL;
    dbyte
        control_ident = 0;

    get_smttran_getb_ok (thread->event->body, &msg);
    if (msg)
      {
        memcpy (smt_msg_body, msg->data, msg->size);

        /*  Decode SYSMAN message                                            */
        exdr_read (smt_msg_body, SYSMAN_MESSAGE, &control_ident, &body);
        free_smttran_getb_ok (&msg);
      }
    return (control_ident);
}


/************************   GET FIRST AFFECTED TASK   ************************/

MODULE get_first_affected_task (THREAD *thread)
{
    tcb = thread->tcb;                 /*  Point to thread's context        */

    if (address_task (tcb))
        the_next_event = ok_event;
    else
        the_next_event = no_more_event;
}

/*  Looks for first task with name that matches tcb->msg_body; returns
 *  TRUE if found, else returns FALSE.
 */

static Bool
address_task (TCB *tcb)
{
    /*  The tcb->msg_body contains the task name or "ALL"; put it           */
    /*  into a canonical form (upper case and no whitespace).                */
    canonise_name (tcb->msg_body);
    if (tcb->msg_body [0] == '\0')
        strcpy (tcb->msg_body, "ALL");

    for (tcb->task = tasks.next; (NODE *) tcb->task != &tasks; tcb->task = tcb->task->next)
      {
        if (streq (tcb->msg_body, tcb->task->name)
        ||  streq (tcb->msg_body, "ALL"))
            return (TRUE);              /*  tcb->task points to task        */
      }
    return (FALSE);                     /*  tcb->task is undefined          */
}


/*************************   GET NEXT AFFECTED TASK   ************************/

MODULE get_next_affected_task (THREAD *thread)
{
    tcb = thread->tcb;                 /*  Point to thread's context        */

    the_next_event = no_more_event;     /*  Assume no tasks found            */
    for (tcb->task = tasks.next; (NODE *) tcb->task != &tasks; tcb->task = tcb->task->next)
      {
        if (streq (tcb->msg_body, tcb->task->name)
        ||  streq (tcb->msg_body, "ALL"))
          {
            the_next_event = ok_event;
            break;
          }
      }
}


/***********************   PASS STATUS TO TASK THREAD   **********************/

MODULE pass_status_to_task_thread (THREAD *thread)
{
    tcb = thread->tcb;                 /*  Point to thread's context        */

    event_send (
        &tcb->task->qid,              /*  Send to task's thread queue      */
        &thread->queue->qid,          /*  Queue for reply                  */
        "_TASK_STATUS",                 /*  Name of event to send            */
        NULL, 0,                        /*  No event body                    */
        NULL, NULL, NULL,               /*  No response events               */
        0);                             /*  No timeout                       */
}


/***********************   PASS START TO TASK THREAD   ***********************/

MODULE pass_start_to_task_thread (THREAD *thread)
{
    tcb = thread->tcb;                 /*  Point to thread's context        */

    event_send (
        &tcb->task->qid,              /*  Send to task's thread queue      */
        &thread->queue->qid,          /*  Queue for reply                  */
        "_TASK_START",                  /*  Name of event to send            */
        NULL, 0,                        /*  No event body                    */
        NULL, NULL, NULL,               /*  No response events               */
        0);                             /*  No timeout                       */
}


/***********************   PASS PAUSE TO TASK THREAD   ***********************/

MODULE pass_pause_to_task_thread (THREAD *thread)
{
    tcb = thread->tcb;                 /*  Point to thread's context        */

    event_send (
        &tcb->task->qid,              /*  Send to task's thread queue      */
        &thread->queue->qid,          /*  Queue for reply                  */
        "_TASK_PAUSE",                  /*  Name of event to send            */
        NULL, 0,                        /*  No event body                    */
        NULL, NULL, NULL,               /*  No response events               */
        0);                             /*  No timeout                       */
}


/************************   PASS STOP TO TASK THREAD   ***********************/

MODULE pass_stop_to_task_thread (THREAD *thread)
{
    tcb = thread->tcb;                 /*  Point to thread's context        */

    event_send (
        &tcb->task->qid,              /*  Send to task's thread queue      */
        &thread->queue->qid,          /*  Queue for reply                  */
        "_TASK_STOP",                   /*  Name of event to send            */
        NULL, 0,                        /*  No event body                    */
        NULL, NULL, NULL,               /*  No response events               */
        0);                             /*  No timeout                       */
}


/************************   COMMIT OUTGOING REPLIES   ************************/

MODULE commit_outgoing_replies (THREAD *thread)
{
    tcb = thread->tcb;                 /*  Point to thread's context        */

    event_send (
        &tranq,                         /*  Send to transfer agent           */
        &thread->queue->qid,          /*  Queue for reply                  */
        "COMMIT",                       /*  Name of event to send            */
        NULL, 0,                        /*  Event body and size              */
        NULL, NULL, NULL,               /*  No response events               */
        0);                             /*  No timeout                       */

    event_wait ();                      /*  ...and wait for reply            */
}


/***********************   CONTROL READY FOR COMMAND   ***********************/

MODULE control_ready_for_command (THREAD *thread)
{
    put_control_msg (thread, SYSMAN_READY, SYSMAN_VERSION);
}


static void
put_control_msg (THREAD *thread, dbyte ident, void *body)
{
    static byte
        control_body [BUFFER_SIZE];     /*  Message to SYSMAN client agent   */
    static DESCR                        /*  Descriptor for exdr_writed       */
        control = { BUFFER_SIZE, control_body };
    int
        control_size;                   /*  Size of formatted control_body   */

    tcb = thread->tcb;                 /*  Point to thread's context        */

    control_size = exdr_writed (&control, SYSMAN_MESSAGE, ident, body);
    send_smttran_put_block (&tranq, tcb->handle, (dbyte)control_size,
                            (byte *)control_body, NULL);
}


/*************************   CONTROL LIST OF TASKS   *************************/

MODULE control_list_of_tasks (THREAD *thread)
{
    TASK
        *task;                          /*  Task in list                     */

    /*  Send a SYSMAN_TASK_ID message for each task in the list              */
    for (task = tasks.next; (NODE *) task != &tasks; task = task->next)
        put_control_msg (thread, SYSMAN_TASK_ID, task->name);
}


/************************   CONTROL TASK STARTED OK   ************************/

MODULE control_task_started_ok (THREAD *thread)
{
    put_control_msg (thread, SYSMAN_START_OK, thread->event->body);
}


/**********************   CONTROL TASK STARTED ERROR   ***********************/

MODULE control_task_started_error (THREAD *thread)
{
    /*  We've received an error from the task thread; the event body is      */
    /*  the error message.  Send it through to the client.                   */
    put_control_msg (thread, SYSMAN_START_ERROR, thread->event->body);
}


/*************************   CONTROL TASK PAUSED OK   ************************/

MODULE control_task_paused_ok (THREAD *thread)
{
    put_control_msg (thread, SYSMAN_PAUSE_OK, thread->event->body);
}


/***********************   CONTROL TASK PAUSED ERROR   ***********************/

MODULE control_task_paused_error (THREAD *thread)
{
    put_control_msg (thread, SYSMAN_PAUSE_ERROR, thread->event->body);
}


/************************   CONTROL TASK STOPPED OK   ************************/

MODULE control_task_stopped_ok (THREAD *thread)
{
    put_control_msg (thread, SYSMAN_STOP_OK, thread->event->body);
}


/**********************   CONTROL TASK STOPPED ERROR   ***********************/

MODULE control_task_stopped_error (THREAD *thread)
{
    /*  We've received an error from the task thread; the event body is      */
    /*  the error message.  Send it through to the client.                   */
    put_control_msg (thread, SYSMAN_STOP_ERROR, thread->event->body);
}


/*************************   CONTROL TASK RUNNING   **************************/

MODULE control_task_running (THREAD *thread)
{
    put_control_msg (thread, SYSMAN_TASK_RUNNING, thread->event->body);
}


/**************************   CONTROL TASK PAUSED   **************************/

MODULE control_task_paused (THREAD *thread)
{
    put_control_msg (thread, SYSMAN_TASK_PAUSED, thread->event->body);
}


/*************************   CONTROL TASK STOPPED   **************************/

MODULE control_task_stopped (THREAD *thread)
{
    put_control_msg (thread, SYSMAN_TASK_STOPPED, thread->event->body);
}


/****************************   CONTROL HALTING   ****************************/

MODULE control_halting (THREAD *thread)
{
    put_control_msg (thread, SYSMAN_HALTING, "");
}


/*********************   CONTROL UNRECOGNISED CONTROL   **********************/

MODULE control_unrecognised_control (THREAD *thread)
{
    put_control_msg (thread, SYSMAN_ERROR, "Invalid control command");
}


/*****************************  ===============  *****************************/
/*                               TASK HANDLING                               */
/*****************************  ===============  *****************************/

/*************************   CHECK IF REQUIRED TASK   ************************/

MODULE check_if_required_task (THREAD *thread)
{
    TASK
        *task;                          /*  Task control block               */

    tcb = thread->tcb;                 /*  Point to thread's context        */
    task = tcb->task;

    if (task->required)
        the_next_event = required_event;
    else
        the_next_event = normal_event;
}


/***********************   CHECK IF RESTARTABLE TASK   ***********************/

MODULE check_if_restartable_task (THREAD *thread)
{
    TASK
        *task;                          /*  Task control block               */

    tcb = thread->tcb;                 /*  Point to thread's context        */
    task = tcb->task;

    if (task->restart)
        the_next_event = restart_event;
    else
    if (task->required)
        the_next_event = required_event;
    else
        the_next_event = normal_event;
}


/**************************   LOAD TASK TIME SLOTS   *************************/

MODULE load_task_time_slots (THREAD *thread)
{
    static char
        buffer [BUFFER_SIZE];           /*  Input line from file             */
    FILE
        *slotfile;                      /*  Stream for opened slots file     */
    TASK
        *task;                          /*  Task control block               */

    tcb  = thread->tcb;                /*  Point to thread's context        */
    task = tcb->task;

    /*  Look for time slot file; if found, load specified values             */
    /*  A slot specification is a string, in the format: "name value ...".   */
    /*  The name field is a day name ("mon"-"sun"), a date in MD order       */
    /*  ("12/31") or a date in YMD order ("95/12/31").  The value is a list  */
    /*  of times in 24 hour HH:MM-HH:MM format ("7:30-12:30 13:30-17:30").   */
    /*  The time slot accuracy is SLOT_TICK csecs.  Any day that does not    */
    /*  have specified values is switched 'off'                              */

    slotfile = file_locate ("PATH", task->name, "tim");
    if (slotfile)
      {
        while (file_read (slotfile, buffer))
          {
            strcrop (buffer);           /*  Remove trailing spaces           */
            if (*buffer == '#' || *buffer == '\0')
                continue;               /*  Ignore comments & blank lines    */

            /*  Send specification to slot agent                             */
            send_smtslot_specify (&task->slotq, buffer);
          }
       file_close (slotfile);
      }
    /*  Tell slot thread that we are switched off now                        */
    send_smtslot_off (&task->slotq);
}


/****************************   LOAD TASK VALUES   ***************************/

MODULE load_task_values (THREAD *thread)
{
    char
        *keyword,                       /*  Keyword or section name          */
        *value;                         /*  Keyword value                    */
    FILE
        *instream;                      /*  Stream for opened ini file       */
    TASK
        *task;                          /*  Task control block               */
    int
        section;                        /*  Current section                  */
    SYMTAB
        *symtab;                        /*  Environment variables            */

#   define SECTION_NONE         0       /*  Values for section               */
#   define SECTION_SETUP        1
#   define SECTION_ENVIRONMENT  2
#   define SECTION_RUN          3

    tcb  = thread->tcb;                /*  Point to thread's context        */
    task = tcb->task;

    instream = file_locate ("PATH", task->name, "ini");
    if (instream == NULL)
      {
        /*  Send message to console, and copy to tcb->msg_body              */
        sprintf (tcb->msg_body, "can't locate '%s.ini'", task->name);
        sprintf (msg_buffer, "sysman: %s", tcb->msg_body);
        send_smtoper_error (&operq, msg_buffer);
        raise_exception (task_failed_event);
      }
    else
      {
        free_task_values (task);
        symtab  = env2symb ();          /*  Load current environment         */
        section = SECTION_NONE;

        FOREVER
          {
            if (ini_scan_section (instream, &keyword, &value))
              {
                if (section == SECTION_SETUP)
                  {
                    if (streq (keyword, "workdir"))
                        task->workdir = mem_strdup (value);
                    else
                    if (streq (keyword, "user"))
                        task->user = mem_strdup (value);
                    else
                    if (streq (keyword, "group"))
                        task->group = mem_strdup (value);
                    else
                    if (streq (keyword, "stdin"))
                        task->std_in = mem_strdup (value);
                    else
                    if (streq (keyword, "stdout"))
                        task->std_out = mem_strdup (value);
                    else
                    if (streq (keyword, "stderr"))
                        task->std_err = mem_strdup (value);
                    else
                    if (streq (keyword, "required"))
                        task->required = streq (value, "1");
                    else
                    if (streq (keyword, "restart"))
                        task->restart = streq (value, "1");
                    else
                      {
                        sprintf (msg_buffer,
                                 "sysman: bad keyword '%s=%s' in '%s.ini'",
                                  keyword, value, task->name);
                        send_smtoper_warning (&operq, msg_buffer);
                      }
                  }
                else
                if (section == SECTION_ENVIRONMENT)
                  {
                    strupc (keyword);
                    sym_assume_symbol (symtab, keyword, value);
                  }
                else
                if (section == SECTION_RUN)
                  {
                    if (streq (keyword, "idle"))
                        task->run_idle = mem_strdup (value);
                    else
                    if (streq (keyword, "startup"))
                        task->run_startup = mem_strdup (value);
                    else
                    if (streq (keyword, "cancel"))
                        task->run_cancel = mem_strdup (value);
                    else
                      {
                        sprintf (msg_buffer,
                                 "sysman: bad keyword '%s=%s' in '%s.ini'",
                                  keyword, value, task->name);
                        send_smtoper_warning (&operq, msg_buffer);
                      }
                  }
              }
            else
            if (keyword == NULL)
                break;                  /*  End of file                      */
            else
            if (streq (keyword, "setup"))
                section = SECTION_SETUP;
            else
            if (streq (keyword, "environment"))
                section = SECTION_ENVIRONMENT;
            else
            if (streq (keyword, "run"))
                section = SECTION_RUN;
            else
                section = SECTION_NONE;
          }
        task->env_list = symb2strt (symtab);
        sym_delete_table (symtab);

        /*  Task startup command is mandatory                                */
        if (!task->run_startup)
          {
            sprintf (tcb->msg_body, "no [Run] startup= in '%s.ini'",
                     task->name);
            sprintf (msg_buffer, "sysman: %s", tcb->msg_body);
            send_smtoper_error (&operq, msg_buffer);
            raise_exception (task_failed_event);
          }
       file_close (instream);
      }
}


/*************************   RUN TASK IDLE COMMAND   *************************/

MODULE run_task_idle_command (THREAD *thread)
{
    tcb = thread->tcb;                 /*  Point to thread's context        */

    /*  Run the idle command.  Any errors are sent to the console but for    */
    /*  the rest we ignore them.                                             */
    run_command (tcb->task, tcb->task->run_idle, TRUE);
}


/*  -------------------------------------------------------------------------
 *  run_command -- local
 *
 *  Runs a command for the current thread task, and returns the PROCESS id
 *  for the created command.  Optionally waits until the command has
 *  completed.  If there is an error, formats an error message in the tcb
 *  message_body, and sends it to the console.  Returns -1 if there was an
 *  error.  If the command argument is NULL, returns 0.
 */

static PROCESS
run_command (TASK *task, char *command, Bool wait)
{
    PROCESS_DATA
        procinfo = PROCESS_DATA_INIT;
    PROCESS
        pid;                            /*  ID of created process            */

    tcb = task->tcb;                   /*  Point to task's context          */
    if (command == NULL)
        return (0);

    sprintf (msg_buffer, "sysman: task command '%s'", command);
    lsend_smtoper_info (&operq, &task->qid, NULL, NULL, NULL, 0,
                        msg_buffer);

    procinfo.filename  = command;
    procinfo.workdir   = task->workdir;
    procinfo.wait      = wait;
    procinfo.envv      = task->env_list;
    procinfo.username  = task->user;
    if (default_user && procinfo.username == NULL)
        procinfo.username = default_user;
    procinfo.groupname = task->group;
    if (default_group && procinfo.groupname == NULL)
        procinfo.groupname = default_group;

    if (process_setinfo (&procinfo, task->std_in,
                         task->std_out, FALSE,
                         task->std_err, FALSE) == 0) {
        pid = process_create_full (&procinfo);
        process_close_io (&procinfo);
        if (pid == NULL_PROCESS) {
            sprintf (msg_buffer, "sysman: can't start process: %s", strerror (procinfo.error));
            lsend_smtoper_info (&operq, &task->qid, NULL, NULL, NULL, 0, msg_buffer);
        }
    }
    else {
        sprintf (msg_buffer, "sysman: can't redirect i/o: %s", strerror (errno));
        lsend_smtoper_info (&operq, &task->qid, NULL, NULL, NULL, 0, msg_buffer);
        pid = NULL_PROCESS;
    }

    if (pid == NULL_PROCESS)
      {
        /*  Send message to console, and copy to tcb->msg_body              */
        sprintf (tcb->msg_body, "cannot run '%s': %s", command, strerror (errno));
        sprintf (msg_buffer, "sysman: %s", tcb->msg_body);
        lsend_smtoper_info (&operq, &task->qid, NULL, NULL, NULL, 0, msg_buffer);
      }
    else
      {
        switch (process_status (pid))
          {
            case PROCESS_ENDED_OK:
                if (wait)               /*  We did not expect the process    */
                  {                     /*    to end quite so soon!          */
                    sprintf (tcb->msg_body,
                             "'%s' ended prematurely", command);
                    pid = 0;            /*  Process ended                    */
                  }
                break;
            case PROCESS_ENDED_ERROR:
                sprintf (tcb->msg_body, "'%s' failed: %s", command,
                         strerror (process_errno));
                pid = 0;                /*  Process ended                    */
                break;
            case PROCESS_INTERRUPTED:
                sprintf (tcb->msg_body, "'%s' was killed", command);
                pid = 0;                /*  Process ended                    */
                break;
          }
        if (pid == 0)
          {
            sprintf (msg_buffer, "sysman: %s", tcb->msg_body);
            lsend_smtoper_error (&operq, &task->qid, NULL, NULL, NULL, 0,
                                 msg_buffer);
          }
      }
    return (pid);
}


/************************   RUN TASK STARTUP COMMAND   ***********************/

MODULE run_task_startup_command (THREAD *thread)
{
    TASK
        *task;                          /*  Task control block               */

    tcb = thread->tcb;                 /*  Point to thread's context        */
    task = tcb->task;

    /*  Run startup command; if there is an error, send to the console and   */
    /*  copy the message to tcb->msg_body.                                  */
    ASSERT (task->run_startup);
    task->pid = run_command (task, task->run_startup, FALSE);
    if (task->pid)
      {
        sprintf (msg_buffer, "sysman: started process %s in %s",
                 task->run_startup, task->workdir);
        send_smtoper_info (&operq, msg_buffer);
      }
    else
        raise_exception (task_failed_event);
}


/************************   RUN TASK CANCEL COMMAND   ************************/

MODULE run_task_cancel_command (THREAD *thread)
{
    tcb = thread->tcb;                 /*  Point to thread's context        */

    /*  Run the cancel command.  Any errors are sent to the console but      */
    /*  but the rest we ignore them.                                         */
    run_command (tcb->task, tcb->task->run_cancel, TRUE);
}


/***********************   MONITOR ACTIVE TASK STATUS   **********************/

MODULE monitor_active_task_status (THREAD *thread)
{
    tcb = thread->tcb;                 /*  Point to thread's context        */

#   define MONITOR_RATE  6000           /*  Centiseconds                     */
    /*  Ask timer to send us an event after a short delay                    */
    send_smttime_alarm (&timeq, 0, MONITOR_RATE, 0);
}


/************************   CHECK TASK STILL RUNNING   ***********************/

MODULE check_task_still_running (THREAD *thread)
{
    tcb = thread->tcb;                 /*  Point to thread's context        */

    if (process_status (tcb->task->pid) != PROCESS_RUNNING)
      {
        sprintf (msg_buffer,
                 "sysman: process %s halted", tcb->task->run_startup);
        send_smtoper_info (&operq, msg_buffer);
        raise_exception (task_halted_event);
        tcb->task->pid = 0;           /*  Not running                      */
      }
}


/************************   BROADCAST STOP ALL TASKS   ***********************/

MODULE broadcast_stop_all_tasks (THREAD *thread)
{
    TASK
        *task;                          /*  Task control block               */

    tcb = thread->tcb;                 /*  Point to thread's context        */

    for (task = tasks.next; (NODE *) task != &tasks; task = task->next)
        SEND (&task->qid, "_TASK_FSTOP", "");
}


/**************************   SHUT DOWN CHILD TASK   *************************/

MODULE shut_down_child_task (THREAD *thread)
{
    TASK
        *task;                          /*  Task control block               */

    tcb = thread->tcb;                 /*  Point to thread's context        */
    task = tcb->task;

    if (task->pid > 0                  /*  Process must be running          */
    &&  process_status (task->pid) == PROCESS_RUNNING
    &&  process_kill   (task->pid))
      {
        /*  Send message to console, and copy to tcb->msg_body              */
        sprintf (tcb->msg_body, "cannot halt process %u: %s",
                 task->pid, strerror (errno));
        sprintf (msg_buffer, "sysman: %s", tcb->msg_body);
        send_smtoper_error (&operq, msg_buffer);
        raise_exception (exception_event);
      }
    else
      {
        sprintf (msg_buffer,
                 "sysman: stopped process %s", tcb->task->run_startup);
        send_smtoper_info (&operq, msg_buffer);
      }
    task->pid = 0;                     /*  Not running                      */
}


/**************************   SIGNAL TASK RUNNING   **************************/

MODULE signal_task_running (THREAD *thread)
{
    tcb = thread->tcb;                 /*  Point to thread's context        */

    /*  Tell control thread that task is running                             */
    SEND (&thread->event->sender, "_TASK_RUNNING", tcb->task->name);
}


/***************************   SIGNAL TASK PAUSED   **************************/

MODULE signal_task_paused (THREAD *thread)
{
    tcb = thread->tcb;                 /*  Point to thread's context        */

    /*  Tell control thread that task is not running                         */
    SEND (&thread->event->sender, "_TASK_PAUSED", tcb->task->name);
}


/**************************   SIGNAL TASK STOPPED   **************************/

MODULE signal_task_stopped (THREAD *thread)
{
    tcb = thread->tcb;                 /*  Point to thread's context        */

    /*  Tell control thread that task is not running                         */
    SEND (&thread->event->sender, "_TASK_STOPPED", tcb->task->name);
}


/************************   SIGNAL TASK STARTED OKAY   ***********************/

MODULE signal_task_started_okay (THREAD *thread)
{
    tcb = thread->tcb;                 /*  Point to thread's context        */

    /*  Tell control thread that task was started okay                       */
    SEND (&thread->event->sender, "_TASK_OK", tcb->task->name);
}


/************************   SIGNAL TASK PAUSED OKAY   ************************/

MODULE signal_task_paused_okay (THREAD *thread)
{
    tcb = thread->tcb;                 /*  Point to thread's context        */

    /*  Tell control thread that task was paused okay                        */
    SEND (&thread->event->sender, "_TASK_OK", tcb->task->name);
}


/************************   SIGNAL TASK STOPPED OKAY   ***********************/

MODULE signal_task_stopped_okay (THREAD *thread)
{
    tcb = thread->tcb;                 /*  Point to thread's context        */

    /*  Tell control thread that task was stopped okay                       */
    SEND (&thread->event->sender, "_TASK_OK", tcb->task->name);
}


/***********************   SIGNAL TASK STARTED ERROR   ***********************/

MODULE signal_task_started_error (THREAD *thread)
{
    tcb = thread->tcb;                 /*  Point to thread's context        */

    /*  Tell control thread that task was not stopped due to an error        */
    SEND (&thread->event->sender, "_TASK_ERROR", tcb->msg_body);
}


/************************   SIGNAL TASK NOT RUNNING   ************************/

MODULE signal_task_not_running (THREAD *thread)
{
    tcb = thread->tcb;                 /*  Point to thread's context        */

    /*  Tell control thread that task was not stopped because it is idle     */
    sprintf (msg_buffer, "Task '%s' is not running", tcb->task->name);

    event_send (&thread->event->sender,/*  Send to specified queue         */
                NULL,                    /*  No queue for reply              */
                "_TASK_ERROR",           /*  Name of event to send           */
                (byte *) msg_buffer,     /*  Event body to send              */
                strlen (msg_buffer) + 1, /*  Event body size, including null */
                NULL, NULL, NULL,        /*  No response events              */
                0);
}


/**********************   SIGNAL TASK ALREADY RUNNING   **********************/

MODULE signal_task_already_running (THREAD *thread)
{
    tcb = thread->tcb;                 /*  Point to thread's context        */

    /*  Tell control thread that task was not started since it is running    */
    sprintf (msg_buffer, "Task '%s' is already running", tcb->task->name);

    event_send (&thread->event->sender,/*  Send to specified queue         */
                NULL,                    /*  No queue for reply              */
                "_TASK_ERROR",           /*  Name of event to send           */
                (byte *) msg_buffer,     /*  Event body to send              */
                strlen (msg_buffer) + 1, /*  Event body size, including null */
                NULL, NULL, NULL,        /*  No response events              */
                0);
}


/****************************   DESTROY THE TASK   ***************************/

MODULE destroy_the_task (THREAD *thread)
{
    TASK
        *task;                          /*  Task control block               */

    tcb = thread->tcb;                 /*  Point to thread's context        */
    task = tcb->task;

    send_smtslot_finish (&task->slotq);/*  Tell time slot thread to end     */
    task_destroy (task);                /*  Destroy task block               */
}


/**************************   SIGNAL SOCKET ERROR   **************************/

MODULE signal_socket_error (THREAD *thread)
{
    tcb = thread->tcb;                 /*  Point to thread's context        */

    sprintf (msg_buffer,
             "sysman: error on socket: %s", thread->event->body);
    send_smtoper_error (&operq, msg_buffer);
}


/***************************   CHECK THREAD TYPE   ***************************/

MODULE check_thread_type (THREAD *thread)
{
    tcb = thread->tcb;                 /*  Point to thread's context        */

    the_next_event = tcb->thread_type;
}


/*************************   CLOSE MANAGER LOG FILE   ************************/

MODULE close_manager_log_file (THREAD *thread)
{
    tcb = thread->tcb;                 /*  Point to thread's context        */

    send_smtlog_close (&logq);
    mem_strfree (&logfile_name);
    mem_strfree (&startup_command);
    mem_strfree (&shutdown_command);
    mem_strfree (&default_user);
    mem_strfree (&default_group);

    if (tasks_table)
        sym_delete_table (tasks_table);
}


/************************   LOG INVALID DIALOG EVENT   ***********************/

MODULE log_invalid_dialog_event (THREAD *thread)
{
    tcb = thread->tcb;                 /*  Point to thread's context        */

    sprintf (msg_buffer, "sysman: invalid dialog event %d",
             thread->LR_event);
    send_smtoper_error (&operq, msg_buffer);
}


/************************   SHUTDOWN THE APPLICATION   ***********************/

MODULE shutdown_the_application (THREAD *thread)
{
    tcb = thread->tcb;                 /*  Point to thread's context        */

    smt_shutdown ();                    /*  Halt the application             */
}


/*************************   TERMINATE THE THREAD   **************************/

MODULE terminate_the_thread (THREAD *thread)
{
    tcb = thread->tcb;                 /*  Point to thread's context        */

    if (tcb->handle)
        close_socket (tcb->handle);

    the_next_event = terminate_event;
}


/**************************   DESTROY TASK THREADS   *************************/

MODULE destroy_task_threads (THREAD *thread)
{
    TASK
        *next,
        *task;                          /*  Task in list                     */

    tcb = thread->tcb;                 /*  Point to thread's context        */

    for (task = tasks.next; (NODE *) task != &tasks; task = task->next)
      {
        next = task->next;
        if (task->thread != NULL)
            thread_destroy (task->thread, TRUE);  /* XXX cleanup or not ??? */

        if (task->slot_thread != NULL)
            thread_destroy (task->slot_thread, TRUE);

        task_destroy (task);                /*  Destroy task block           */
      }
}

/****************************   FORCE ALL TASKS   ****************************/

MODULE force_all_tasks (THREAD *thread)
{
    tcb = thread->tcb;                 /*  Point to thread's context        */

    ASSERT (tcb->msg_body);
    tcb->msg_body [0] = '\0';
}

