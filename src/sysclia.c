/*===========================================================================*
 *  syscli.c - system manager client agent                                   *
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
 *===========================================================================*/

#include "sfl.h"                        /*  SFL library header file          */
#include "smt3.h"                       /*  SMT kernel header file           */
#include "sysman.h"                     /*  Sysman definitions               */


/*- Definitions -------------------------------------------------------------*/

#define AGENT_NAME       "sysclia"      /*  Name of our agent                */
#define SINGLE_THREADED  TRUE


/*- Function prototypes -----------------------------------------------------*/

static void   put_control_msg  (THREAD *thread, dbyte ident, char *body);
static dbyte  get_control_msg  (THREAD *thread, char *body);


/*- Global variables used in this source file only --------------------------*/

static char
    *command,                           /*  Command from user                */
    *port;                              /*  Connect to Sysman on this port   */

static sock_t
    control_socket = 0;                 /*  Socket to SYSMAN daemon          */
static QID
    sockq,                              /*  Socket agent event queue         */
    tranq;                              /*  Transfer agent event queue       */
static char
    **token_list = NULL,                /*  Parsed command line              */
    control_body [LINE_MAX],            /*  Message from SYSMAN daemon       */
    input_line [LINE_MAX + 1];
static dbyte
    control_id;                         /*  SYSMAN message id                */

#include "sysclia.d"                    /*  Include dialog data              */


/********************   INITIALISE AGENT - ENTRY POINT   *********************/

int
sysclia_init (char *p_command, char *p_port)
{
    AGENT   *agent;                     /*  Handle for our agent             */
    THREAD  *thread;                    /*  Handle to console thread         */
#   include "sysclia.i"                 /*  Include dialog interpreter       */

    /*  Shutdown event comes from Kernel                                     */
    declare_smtlib_shutdown    (shutdown_event, SMT_PRIORITY_MAX);

    /*  Reply events from socket agent                                       */
    declare_smtsock_ok         (ok_event,            0);
    declare_smtsock_read_ok    (ok_event,            0);
    declare_smtsock_connect_ok (ok_event,            0);
    declare_smtsock_closed     (sock_closed_event,   0);
    declare_smtsock_error      (sock_error_event,    0);
    declare_smtsock_timeout    (sock_error_event,    0);

    /*  Reply events from transfer agent                                     */
    declare_smttran_get_block  (ok_event,            0);
    declare_smttran_put_block  (SMT_NULL_EVENT,      0);
    declare_smttran_putb_ok    (SMT_NULL_EVENT,      0);
    declare_smttran_getb_ok    (ok_event,            0);
    declare_smttran_closed     (sock_closed_event,   0);
    declare_smttran_error      (sock_error_event,    0);

    /*  Create initial, unnamed thread                                       */
    thread_create (AGENT_NAME, "");

    /*  Ensure that socket agent is running, else start it up                */
    smtsock_init ();
    if ((thread = thread_lookup (SMT_SOCKET, "")) != NULL)
        sockq = thread-> queue-> qid;
    else
        return (-1);

    /*  Ensure that transfer agent is running, else start it up              */
    smttran_init ();
    if ((thread = thread_lookup (SMT_TRANSFER, "")) != NULL)
        tranq = thread-> queue-> qid;
    else
        return (-1);

    /*  Signal okay to caller that we initialised okay                       */
    command = p_command;                /*  Get command from user            */
    port = p_port;                      /*  Get port to connect to           */
    return (0);
}


/*************************   INITIALISE THE THREAD   *************************/

MODULE initialise_the_thread (THREAD *thread)
{
    the_next_event = ok_event;
}


/*************************   CONNECT TO SYSMAN DAEMON   **********************/

MODULE connect_to_sysman_daemon (THREAD *thread)
{
    printf ("syscli> 100- Connecting to SYSMAN on port %s...\n", port);
   
    send_smtsock_connect(&sockq, 0, "TCP", "", port, 0, 0, 0);
}


/*************************   STORE CONNECTION DATA   *************************/

MODULE store_connection_data (THREAD *thread)
{
    struct_smtsock_connect_ok
        *msg = NULL;

    get_smtsock_connect_ok (thread-> event-> body, &msg);
    if (msg)
      {
        control_socket = msg-> socket;
        free_smtsock_connect_ok (&msg);
      }
}


/***************************   GET DAEMON MESSAGE   **************************/

MODULE get_daemon_message (THREAD *thread)
{
    /*  Ask the transfer agent to read a block from the socket               */
    send_smttran_get_block (&tranq, control_socket, NULL);
    event_wait ();                      /*  Wait for reply event             */
}


/**************************   CHECK DAEMON MESSAGE   *************************/

MODULE check_daemon_message (THREAD *thread)
{
    /*  This table converts a SYSMAN message id into a dialog event          */
    static struct {
        dbyte   id;
        event_t event;
    } idents [] = {
        { SYSMAN_READY,        ready_event         },
        { SYSMAN_ERROR,        error_event         },
        { SYSMAN_HALTING,      halting_event       },
        { SYSMAN_TASK_ID,      task_id_event       },
        { SYSMAN_TASK_NF,      task_nf_event       },
        { SYSMAN_TASK_RUNNING, task_running_event  },
        { SYSMAN_TASK_PAUSED,  task_paused_event   },
        { SYSMAN_TASK_STOPPED, task_stopped_event  },
        { SYSMAN_START_OK,     start_ok_event      },
        { SYSMAN_START_ERROR,  start_error_event   },
        { SYSMAN_PAUSE_OK,     pause_ok_event      },
        { SYSMAN_PAUSE_ERROR,  pause_error_event   },
        { SYSMAN_STOP_OK,      stop_ok_event       },
        { SYSMAN_STOP_ERROR,   stop_error_event    },
        { 0,                   0                   }
    };
    int
        ident_nbr;

    if (the_external_event == ok_event)
      {
        /*  Get arguments from message                                       */
        control_id = get_control_msg (thread, control_body);
        for (ident_nbr = 0; idents [ident_nbr].id; ident_nbr++)
          {
            if (idents [ident_nbr].id == control_id)
              {
                the_next_event = idents [ident_nbr].event;
                break;
              }
          }
        if (idents [ident_nbr].id == 0)
          {
            signal_unexpected_message (thread);
            raise_exception (exception_event);
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

    /*  Get buffer from transfer agent                                       */
    get_smttran_getb_ok (thread-> event-> body, &msg);
    if (msg && msg-> size > 0)
      {
        /*  Decode SYSMAN message                                                */
        exdr_read (msg-> data, SYSMAN_MESSAGE, &control_ident, &body);
        free_smttran_getb_ok (&msg);
      }

    return (control_ident);
}


/*************************   GET USER COMMAND INPUT   ************************/

MODULE get_user_command_input (THREAD *thread)
{
    static struct {
        char   *name;
        event_t event;
    } keywords [] = {
        { "LIST",    list_event     },
        { "STATUS",  status_event   },
        { "START",   start_event    },
        { "PAUSE",   pause_event    },
        { "STOP",    stop_event     },
        { "HALT",    halt_event     },
        { "EXIT",    exit_event     },
        { "QUIT",    exit_event     },
        { "HELP",    help_event     },
        { "VERSION", version_event  },
        { "REFRESH", refresh_event  },
        {  NULL,     0              }
    };
    int
        keyword_nbr;

    /*  If we got a command from the user, use it, else prompt               */
    if (command && strused (command))
      {
        strncpy (input_line, command, LINE_MAX);
        input_line [LINE_MAX] = '\0';   /*  Ensure delimited, if looong      */
        command = "exit";               /*  Next time treat as Exit          */
      }
    else
      {
        /*  Show syscli prompt and wait for user command                     */
        printf ("syscli> ");
        fflush (stdout);
        if (fgets (input_line, LINE_MAX, stdin) == NULL)
            strclr (input_line);        /*  Treat EOF as empty               */
      }
    if (token_list)
        tok_free (token_list);
    token_list = tok_split (input_line);

    /*  Get event corresponding to user command                              */
    if (token_list [0] && *token_list [0])
      {
        the_next_event = error_event;
        strupc (token_list [0]);
        for (keyword_nbr = 0; keywords [keyword_nbr].name; keyword_nbr++)
            if (streq (token_list [0], keywords [keyword_nbr].name))
              {
                the_next_event = keywords [keyword_nbr].event;
                break;
              }
        /*  Specific handling for stop command without arguments             */
        if (the_next_event == stop_event && token_list [1] == NULL)
            the_next_event = stop_all_event;
      }
    else
        the_next_event = empty_event;
}


/***********************   GET CONFIRMATION FOR HALT   ***********************/

MODULE get_confirmation_for_halt (THREAD *thread)
{
    printf ("Do you really want to halt sysman (n)? ");
    fflush (stdout);
    if (fgets (input_line, LINE_MAX, stdin) == NULL)
        strclr (input_line);            /*  Treat EOF as empty               */
    strlwc (input_line);

    if (input_line [0] == 'y')
        the_next_event = ok_event;
    else
      {
        the_next_event = cancel_event;
        printf ("syscli> 211 - cancelled\n");
     }
}


/***********************   GET CONFIRMATION FOR STOP   ***********************/

MODULE get_confirmation_for_stop (THREAD *thread)
{
    printf ("Do you really want to stop all tasks (n)? ");
    fflush (stdout);
    if (fgets (input_line, LINE_MAX, stdin) == NULL)
        strclr (input_line);            /*  Treat EOF as empty               */
    strlwc (input_line);

    if (input_line [0] == 'y')
        the_next_event = ok_event;
    else
      {
        the_next_event = cancel_event;
        printf ("syscli> 211 - cancelled\n");
     }
}


/************************   SEND DAEMON LIST COMMAND   ***********************/

MODULE send_daemon_list_command (THREAD *thread)
{
    put_control_msg (thread, SYSMAN_LIST, "");
}


static void
put_control_msg (THREAD *thread, dbyte ident, char *body)
{
    static byte
        control_body [LINE_MAX];        /*  Message to SYSMAN agent          */
    static DESCR                        /*  Descriptor for exdr_writed       */
        control = { LINE_MAX, control_body };
    int
        control_size;                   /*  Size of formatted control_body   */

    control_size = exdr_writed (&control, SYSMAN_MESSAGE, ident, body);

    send_smttran_put_block (&tranq, control_socket, (dbyte)control_size,
                            (byte *)control_body, NULL);
}


/**********************   SEND DAEMON REFRESH COMMAND   **********************/

MODULE send_daemon_refresh_command (THREAD *thread)
{
    put_control_msg (thread, SYSMAN_REFRESH, token_list [1]);
}


/***********************   SEND DAEMON STATUS COMMAND   **********************/

MODULE send_daemon_status_command (THREAD *thread)
{
    put_control_msg (thread, SYSMAN_STATUS, token_list [1]);
}


/***********************   SEND DAEMON START COMMAND   ***********************/

MODULE send_daemon_start_command (THREAD *thread)
{
    put_control_msg (thread, SYSMAN_START, token_list [1]);
}


/***********************   SEND DAEMON PAUSE COMMAND   ***********************/

MODULE send_daemon_pause_command (THREAD *thread)
{
    put_control_msg (thread, SYSMAN_PAUSE, token_list [1]);
}


/************************   SEND DAEMON STOP COMMAND   ***********************/

MODULE send_daemon_stop_command (THREAD *thread)
{
    put_control_msg (thread, SYSMAN_STOP, token_list [1]);
}


/************************   SEND DAEMON HALT COMMAND   ***********************/

MODULE send_daemon_halt_command (THREAD *thread)
{
    put_control_msg (thread, SYSMAN_HALT, "");
}


/************************   SHOW VERSION INFORMATION   ***********************/

MODULE show_version_information (THREAD *thread)
{
    printf ("syscli> 101- SYSMAN client version %s\n", SYSCLI_VERSION);
}


/*************************   SHOW HELP INFORMATION   *************************/

MODULE show_help_information (THREAD *thread)
{
#   define HELP1                                                              \
    "Commands and arguments can be in any case.  Commands are:\n"            \
    "LIST                 - list all known tasks\n"                          \
    "REFRESH              - reload SYSMAN config\n"                          \
    "STATUS [task | ALL]  - show status for specified task (default all)\n"  \
    "START [task | ALL]   - start specified task (default all)\n"            \
    "PAUSE [task | ALL]   - pause specified task (default all)\n"
    
#    define HELP2                                                            \
    "STOP [task | ALL]    - stop specified task (default all)\n"             \
    "HALT                 - halt SYSMAN\n"                                   \
    "EXIT                 - end this SYSMAN client session\n"                \
    "QUIT                 - end this SYSMAN client session\n"                \
    "HELP                 - show this information\n"                         \
    "VERSION              - show SYSMAN client version\n"

    puts (HELP1);
    puts (HELP2);
}


/*************************   SIGNAL CONNECTED OKAY   *************************/

MODULE signal_connected_okay (THREAD *thread)
{
    printf ("syscli> 102- Connected to SYSMAN version %s\n", control_body);
}


/*****************************   SHOW TASK NAME   ****************************/

MODULE show_task_name (THREAD *thread)
{
    printf ("syscli> 200- %s\n", control_body);
}


/*************************   SIGNAL TASK STARTED OK   ************************/

MODULE signal_task_started_ok (THREAD *thread)
{
    printf ("syscli> 201- task started successfully: %s\n", control_body);
}


/************************   SIGNAL TASK NOT STARTED   ************************/

MODULE signal_task_not_started (THREAD *thread)
{
    printf ("syscli> 202- task not started: %s\n", control_body);
}


/*************************   SIGNAL TASK PAUSED OK   *************************/

MODULE signal_task_paused_ok (THREAD *thread)
{
    printf ("syscli> 203- task paused successfully: %s\n", control_body);
}


/*************************   SIGNAL TASK STOPPED OK   ************************/

MODULE signal_task_stopped_ok (THREAD *thread)
{
    printf ("syscli> 204- task stopped successfully: %s\n", control_body);
}


/*************************   SIGNAL TASK NOT PAUSED   ************************/

MODULE signal_task_not_paused (THREAD *thread)
{
    printf ("syscli> 205- task not paused: %s\n", control_body);
}


/************************   SIGNAL TASK NOT STOPPED   ************************/

MODULE signal_task_not_stopped (THREAD *thread)
{
    printf ("syscli> 206- task not stopped: %s\n", control_body);
}


/*************************   SIGNAL TASK NOT KNOWN   *************************/

MODULE signal_task_not_known (THREAD *thread)
{
    printf ("syscli> 207- task not defined: %s\n", control_body);
}


/**************************   SIGNAL TASK RUNNING   **************************/

MODULE signal_task_running (THREAD *thread)
{
    printf ("syscli> 208- %-10s * running\n", control_body);
}


/***************************   SIGNAL TASK PAUSED   **************************/

MODULE signal_task_paused (THREAD *thread)
{
    printf ("syscli> 209- %-10s . paused\n", control_body);
}


/**************************   SIGNAL TASK STOPPED   **************************/

MODULE signal_task_stopped (THREAD *thread)
{
    printf ("syscli> 210- %-10s   stopped\n", control_body);
}


/**************************   SIGNAL SOCKET CLOSED   *************************/

MODULE signal_socket_closed (THREAD *thread)
{
    printf ("syscli> 300- SYSMAN closed connection\n");
}


/**************************   SIGNAL SOCKET ERROR   **************************/

MODULE signal_socket_error (THREAD *thread)
{
    printf ("syscli> 301- SYSMAN connection failed\n");
}


/*************************   SIGNAL INVALID COMMAND   ************************/

MODULE signal_invalid_command (THREAD *thread)
{
    printf ("syscli> 302- Invalid command - 'help' shows possible commands\n");
}


/***********************   SIGNAL UNEXPECTED MESSAGE   ***********************/

MODULE signal_unexpected_message (THREAD *thread)
{
    printf ("syscli> 303- Unexpected message from SYSMAN: %d\n", control_id);
}


/***********************   SIGNAL SYSMAN FATAL ERROR   ***********************/

MODULE signal_sysman_fatal_error (THREAD *thread)
{
    printf ("syscli> 304- Fatal error from SYSMAN: %s\n", control_body);
}


/*************************   TERMINATE THE THREAD   **************************/

MODULE terminate_the_thread (THREAD *thread)
{
    if (control_socket)
        close_socket (control_socket);
    if (token_list)
        tok_free (token_list);          /*  Free-up allocated memory         */

    smt_shutdown ();                    /*  End the entire application       */
    the_next_event = terminate_event;
}



