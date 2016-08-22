/*===========================================================================*
 *                                                                           *
 *  smtserv.c - Windows service wrapper                                      *
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

#if (defined (WIN32))
#include <windowsx.h>
#include <direct.h>                     /*  Directory create functions       */
#endif

#include "smtpriv.h"                    /*  SMT definitions                  */


/* ------------------------------------------------------------------------  */

#define WINDOWS_95        1             /*  Return from get_windows_version  */
#define WINDOWS_NT_4      2
#define WINDOWS_NT_3X     3
#define WINDOWS_2000      4


#define ACTION_ERROR          (-1)      /*  Return from parse_command_line   */
#define ACTION_INSTALL          1
#define ACTION_UNINSTALL        2
#define ACTION_CONSOLE          3
#define ACTION_HELP             4
#define ACTION_NOARG            5
#define ACTION_BACKGROUND       6


#define BUFFER_SIZE            10240


#define DEPENDENCIES      ""
#define REGISTRY_RUN      "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\RunServices"



/* TODO XXX: display iMatix copyright */

#if (defined (WIN32))
# define USAGE\
    strprintf ("\n%s %s Windows service model\n"\
    "\nSyntax: %s action\n"\
    "\nwhere action is ONE of the following:\n"\
    "  -i [usr passwd]  Install Windows service\n"\
    "  -u               Uninstall Windows service\n"\
    "  -d               Run as DOS console program\n"\
    "  -h               Show summary of command-line options.\n"\
    "\nWhen installing the service, you may supply a user/password so the\n"\
    "service will be logged on as the specified user account (Default: \n"\
    "local system account)\n",\
    application_name, application_version, application_name)

#elif (defined (__UNIX__))           /* NOT defined WIN32 */
# define USAGE\
    strprintf ("\n%s %s Unix/Linux model\n"\
    "\nSyntax: %s [option]\n"\
    "\noption:\n"\
    "  -s               Starts %s into background\n"\
    "  -h               Show summary of command-line options.\n",\
    application_name, application_version, application_name, application_name)
#endif
    

#define debug_printf         if (service_debug) log_printf


/* Variables global to this module ---------------------------------------- */

static char 
    *application_name       = NULL,
    *application_config     = NULL,
    *application_version    = NULL,
    *service_name           = NULL,
    *service_display_name   = NULL,
    *service_trace_file     = NULL;

static Bool
    service_debug = FALSE;

static SMT_AGENTS_INIT_FCT 
    *smt_agents_init_fct = NULL;
static SMT_AGENTS_TERM_FCT 
    *smt_agents_term_fct = NULL;


#if (defined(WIN32))
static int
    win_version;                        /*  Windows version                  */

static Bool
    console_mode  = FALSE,              /*  TRUE if console mode             */
    control_break = FALSE;              /*  TRUE if control break            */

static DWORD
    error_code = 0;                     /*  Last error code                  */

static char
    buffer [BUFFER_SIZE],               /*  Working buffer                   */    
    error_buffer [LINE_MAX + 1];        /*  Buffer for error string          */

static SERVICE_STATUS
    service_status;                     /*  current status of the service    */

static SERVICE_STATUS_HANDLE
    service_status_handle;              /*  Service status handle            */

static HANDLE
    server_stop_event = NULL;           /*  Handle for server stop event     */

static char 
    *account = NULL,                    /* account for service registration */
    *password = NULL;                   /* password for service registration */
#endif      /*  defined WIN32 */


/* ------------------------------------------------------------------------  */


#if (defined(WIN32))
static void  add_to_message_log  (char *message);
static void  console_service     (int argc, char ** argv);
static char *date_str            (void);
static char *get_last_error_text (char *buffer, int size);
static int   get_windows_version (void);
static void  hide_window         (void);
static void  install_service     (void);
static void  remove_service      (void);
static int   report_smt_error    (void);
static Bool  report_status       (DWORD state, DWORD exit_code,
                                  DWORD wait_hint);
static void  service_start       (int argc, char **argv);
static void  service_stop        (void);
static long  set_win95_service   (Bool add);
static char *time_str            (void);


BOOL WINAPI  control_handler    (DWORD control_type);
void WINAPI  service_control    (DWORD control_code);
void WINAPI  service_main       (int argc, char **argv);
#endif      /*  defined WIN32 */


static void  free_resources      (void);
static int   init_resources      (const char           *binary_name,
                                  const char           *appl_version,
                                  SMT_AGENTS_INIT_FCT  *init_fct, 
                                  SMT_AGENTS_TERM_FCT  *term_fct);
static int   load_service_config (const char *filename);
static int   parse_command_line  (int argc, char **argv);
static char *service_get_config_filename (const char *binary_name);


#define log_printf      coprintf


/* ------------------------------------------------------------------------ */
/* ------------------------ PUBLIC FUNCTIONS ------------------------------ */
/* ------------------------------------------------------------------------ */

/*  ---------------------------------------------------------------------[<]-
    Function: service_begin

    Synopsis: depending on arguments (from the command line):
      -i: install service               (windows)
      -u: remove  service               (windows)
      -d: runs service in console mode  (windows)
      -h: basic help information        (windows)
      -d: runs service in background    (UNIX / Linux)

      if no arguments, the service is actually started.

    NOTE: with Windows, the working directory is set to the one where the
    binary program stands.

    Returns: 0 is everything is OK, negative error code otherwise
    ---------------------------------------------------------------------[>]-*/
int
service_begin (
    int                   argc, 
    char                **argv,
    SMT_AGENTS_INIT_FCT  *init_fct, 
    SMT_AGENTS_TERM_FCT  *term_fct,
    const char           *appl_version)
{  
    int
        action;
    int
        rc = 0;

#if (defined(WIN32)) 
    static char
        buffer [LINE_MAX];
    char
        *p_char;

    SERVICE_TABLE_ENTRYA 
        dispatch_table [] = {
        { NULL, (LPSERVICE_MAIN_FUNCTIONA) service_main },
        { NULL, NULL }
    };

    /*  Change to the correct working directory, where config file stands    */
    GetModuleFileNameA (NULL, buffer, LINE_MAX);
    if ((p_char = strrchr (buffer, '\\')) != NULL)
        *p_char = '\0';
    SetCurrentDirectoryA (buffer);
#endif

    rc = init_resources (argv[0], appl_version, init_fct, term_fct);
    if (rc != 0)
        return (1);

    ASSERT (application_config);        /* init_resources post condition     */
    if (load_service_config (application_config) != 0)
      {
        free_resources ();
        return (1);
      }

    ASSERT (service_trace_file);        /* load_service_config postcondition */
    console_set_mode (CONSOLE_DATETIME);
    console_capture (service_trace_file, 'a');

#if (defined(WIN32)) 
    dispatch_table [0].lpServiceName = service_name;
    win_version = get_windows_version ();
#endif

    action = parse_command_line (argc, argv);

    if (action == ACTION_HELP)
      {
        puts (USAGE);
      }
#if (defined(WIN32)) 
    else
    if (action == ACTION_INSTALL)
      {
        if (win_version == WINDOWS_95)
            set_win95_service (TRUE);
        else
            install_service ();
      }
    else 
    if (action == ACTION_UNINSTALL)
      {
        if (win_version == WINDOWS_95)
            set_win95_service (FALSE);
        else
            remove_service ();
      }
    else
    if (action == ACTION_CONSOLE)
      {
        console_mode  = TRUE;
        console_service (argc, argv);
      }
    else
    if (action == ACTION_NOARG)
      {
        console_send (NULL, FALSE);

        if (win_version == WINDOWS_95)
          {
            hide_window ();
            console_mode = TRUE;
            console_service (argc, argv);
          }
        else
        if (win_version == WINDOWS_NT_3X
        ||  win_version == WINDOWS_NT_4
        ||  win_version == WINDOWS_2000)
          {
            log_printf ("%s: initialising service ...", application_name);
            if (!StartServiceCtrlDispatcherA (dispatch_table))
                add_to_message_log ("StartServiceCtrlDispatcher failed");
          }
      }
#elif (defined(__UNIX__))
    else
    if (action == ACTION_BACKGROUND)
      {
        const char
           *background_args [] = { "-s", NULL };

        log_printf ("Moving into the background");
        if (process_server (NULL, NULL, argc, argv, background_args) != 0)
          {
            log_printf ("Backgrounding failed.  Giving up.");
            rc = -1;
          }
        else
          action = ACTION_NOARG;
      }
    if (action == ACTION_NOARG)
      {
        rc = smt_init ();
        if (!rc && (init_fct != NULL))
            rc = (*init_fct)(application_config);
        if (!rc)
            smt_exec_full ();
        if (term_fct != NULL)
            (*term_fct)();
        smt_term ();
      }
#endif
    else
    if (action == ACTION_ERROR)
        puts (USAGE);
    
    free_resources ();
    return rc;
}


/* ------------------------------------------------------------------------ */
/* ------------------------ STATIC FUNCTIONS ------------------------------ */
/* ------------------------------------------------------------------------ */

/*  ---------------------------------------------------------------------[<]-
    Function: add_to_message_log

    Synopsis: Allows any thread to log an error message.
    ---------------------------------------------------------------------[>]-*/

static void
add_to_message_log (char *message)
{
#if (defined(WIN32))
    static char
        *strings       [2],
        message_buffer [LINE_MAX + 1];
    HANDLE
        event_source_handle;

    if (!console_mode)
      {
        error_code = GetLastError();

        /* Use event logging to log the error.                               */
        event_source_handle = RegisterEventSourceA (NULL, service_name);

        sprintf (message_buffer, "%s error: %d", service_name, error_code);
        strings [0] = message_buffer;
        strings [1] = message;

        if (event_source_handle)
          {
            ReportEventA (event_source_handle,/* handle of event source       */
                EVENTLOG_ERROR_TYPE,          /* event type                   */
                0,                            /* event category               */
                0,                            /* event ID                     */
                NULL,                         /* current user's SID           */
                2,                            /* strings in variable strings  */
                0,                            /* no bytes of raw data         */
                strings,                      /* array of error strings       */
                NULL);                        /* no raw data                  */

            DeregisterEventSource (event_source_handle);
          }

        /* we also log to console (STDOUT and/or FILE) the error messages */
        log_printf ("Error(WIN): %s", strings[0]);
        log_printf ("Error(APP): %s", strings[1]);
      }
#elif (defined(__UNIX__))

    log_printf ("Error: %s", message);
#endif
}



#if (defined(WIN32))
/*  ---------------------------------------------------------------------[<]-
    Function: console_service

    Synopsis: Runs the service as a console application
    ---------------------------------------------------------------------[>]-*/

static void
console_service (int argc, char ** argv)
{
    log_printf ("%s %s: starting in console mode",
            application_name, application_version);
    SetConsoleCtrlHandler (control_handler, TRUE);
    service_start (argc, argv);
    control_break = FALSE;
}

/*  ---------------------------------------------------------------------[<]-
    Function: control_handler

    Synopsis: Handled console control events
    ---------------------------------------------------------------------[>]-*/

BOOL WINAPI
control_handler (DWORD control_type)
{
    switch (control_type)
      {
        /* Use Ctrl+C or Ctrl+Break to simulate SERVICE_CONTROL_STOP in      */
        /* console mode                                                      */
        case CTRL_BREAK_EVENT:
        case CTRL_C_EVENT:
            service_stop ();
            log_printf ("%s: stopping service", application_name);
            control_break = TRUE;
            return (TRUE);

      }
    return (FALSE);
}
#endif     /* defined WIN32 */



/*  ---------------------------------------------------------------------[<]-
    Function: free_resource

    Synopsis: Free all allocated resources
    ---------------------------------------------------------------------[>]-*/

static void  
free_resources (void)
{
    FILE
        *trace_f;

#if (defined (WIN32))
    mem_free (account);
    mem_free (password);
    account  = NULL;
    password = NULL;
#endif

    mem_free (application_config);
    mem_free (application_name);
    mem_free (application_version);
    mem_free (service_name);
    mem_free (service_display_name);
    mem_free (service_trace_file);
    /* resetting global variable to NULL insures there will be no problem if */
    /* free_resources is invoked more than once                              */
    application_config   = NULL;
    application_name     = NULL;
    application_version  = NULL;
    service_name         = NULL;
    service_display_name = NULL;
    service_trace_file   = NULL;

    /*  Check that all memory was cleanly released                           */
    if (mem_used ())
      {
        add_to_message_log ("Memory leak error, see 'memtrace.lst'");
        trace_f = fopen ("memtrace.lst", "w");
        if (trace_f)
          {
            mem_display (trace_f);
            fclose (trace_f);
          }
      }
}

#if (defined(WIN32))
  
/*  ---------------------------------------------------------------------[<]-
    Function: get_last_error_text

    Synopsis: copies error message text to string
    ---------------------------------------------------------------------[>]-*/

static char *
get_last_error_text (char *buffer, int size)
{
    DWORD
        last_error,
        return_code;
    char
        *temp = NULL;
    
    last_error = GetLastError ();
    
    return_code = FormatMessage (FORMAT_MESSAGE_ALLOCATE_BUFFER |
                                 FORMAT_MESSAGE_FROM_SYSTEM     |
                                 FORMAT_MESSAGE_ARGUMENT_ARRAY,
                                 NULL,
                                 last_error,
                                 LANG_NEUTRAL,
                                 (LPTSTR) &temp,
                                 0,
                                 NULL );

    /*  Supplied buffer is not long enough                                    */
    if (return_code == 0 || ((long) size < (long) return_code + 14))
        buffer [0] = '\0';
    else
      {
        temp [lstrlenA (temp) - 2] = '\0'; /*remove cr and newline character   */
        sprintf (buffer, "%s (0x%x)", temp, last_error);
      }
    if (temp)
        LocalFree ((HLOCAL) temp);

    return (buffer);
}


/*  ---------------------------------------------------------------------[<]-
    Function: get_windows_version

    Synopsis: Return the windows version
    <TABLE>
    WINDOWS_95       Windows 95 or later
    WINDOWS_NT_3X    Windows NT 3.x
    WINDOWS_NT_4     Windows NT 4.0
    WINDOWS_2000     Windows 2000
    </TABLE>
    ---------------------------------------------------------------------[>]-*/
static int
get_windows_version (void)
{
    static int
        version = 0;
    OSVERSIONINFO
        version_info;

    version_info.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
    if (GetVersionEx (&version_info))
      {
        if (version_info.dwMajorVersion < 4)
            version = WINDOWS_NT_3X;
        else
        if (version_info.dwPlatformId == VER_PLATFORM_WIN32_NT)
          {
            if (version_info.dwMajorVersion = 4)
                version = WINDOWS_NT_4;
            else
                version = WINDOWS_2000;
          }
        else
            version = WINDOWS_95;
      }
    return (version);
}



/*  ---------------------------------------------------------------------[<]-
    Function: hide_window

    Synopsis: Hidden console window.
    ---------------------------------------------------------------------[>]-*/

static void
hide_window (void)
{
    char
        title [255];
    HWND
        win;
    GetConsoleTitleA  (title, 254);
    win = FindWindowA (NULL,title);
    if (win)
        SetWindowPos (win, NULL, 0, 0, 0, 0, SWP_HIDEWINDOW);
}
#endif


/*  ---------------------------------------------------------------------[<]-
    Function: init_resources

    Synopsis: stores needed parameters for the service and application
      - bin_name and appl_version will be used for display only. Can be NULL
      - init_fct: a function starting all the SMT agents needed for the service
      - term_fct: a function disposing resources allocated by init_fct

    Returns: 0 is everything is OK, negative error code otherwise
    ---------------------------------------------------------------------[>]-*/
static int 
init_resources(
    const char *binary_name,
    const char *appl_version,
    SMT_AGENTS_INIT_FCT *init_fct, 
    SMT_AGENTS_TERM_FCT *term_fct)
{
    if (!binary_name)
        return 1;

    application_name = mem_strdup (binary_name);
    if (application_name)
        strip_file_path (application_name);
    application_version = mem_strdup (appl_version ? appl_version : "???");
    application_config = service_get_config_filename (application_name);
    smt_agents_init_fct = init_fct;
    smt_agents_term_fct = term_fct;

    if (!application_name || !application_config || !application_version)
      {
        free_resources();
        return 1;
      }

    return 0;
}

#if (defined(WIN32))
/*  ---------------------------------------------------------------------[<]-
    Function: install_service

    Synopsis: Installs the service
    ---------------------------------------------------------------------[>]-*/

static void
install_service (void)
{
    SC_HANDLE
        service,
        manager;
    static char
        path [512];

    if (GetModuleFileNameA ( NULL, path, 512 ) == 0)
      {
        log_printf ("%s: cannot install '%s': %s",
                 application_name, service_name, 
                 get_last_error_text (error_buffer, LINE_MAX));
        return;
      }

    manager = OpenSCManager(
                    NULL,                      /* machine  (NULL == local)   */
                    NULL,                      /* database (NULL == default) */
                    SC_MANAGER_ALL_ACCESS      /* access required            */
                );
    if (manager)
      {
        service = CreateServiceA (
                    manager,                   /* SCManager database         */
                    service_name,              /* short name for service     */
                    service_display_name,      /* name to display            */
                    SERVICE_ALL_ACCESS,        /* desired access             */
                    SERVICE_WIN32_OWN_PROCESS, /* service type               */
                    SERVICE_AUTO_START,        /* start type                 */
                    SERVICE_ERROR_NORMAL,      /* error control type         */
                    path,                      /* service's binary           */
                    NULL,                      /* no load ordering group     */
                    NULL,                      /* no tag identifier          */
                    DEPENDENCIES,              /* dependencies               */
                    account,
                    password);

        if (service)
          {
            log_printf ("%s: service '%s' installed",
                     application_name, service_name);
            CloseServiceHandle (service);
          }
        else
            log_printf ("%s: CreateService '%s' failed: %s", 
                    application_name, service_name,
                    get_last_error_text (error_buffer, LINE_MAX));

        CloseServiceHandle (manager);
      }
    else
        log_printf ("%s: OpenSCManager failed: %s",
                application_name, 
                get_last_error_text (error_buffer, LINE_MAX));
}
#endif

static int 
load_service_config (const char *binary_name)
{
    XML_ITEM 
        *root = NULL,
        *item,
        *child;
    int
        res = 0,
        rc;

    if (! file_exists (application_config))
      {
        log_printf ("ERROR: cannot find config file '%s'", application_config);
        return -1;
      }

    rc = xml_load_file (&root, NULL, application_config, FALSE);
    if (rc != XML_NOERROR)
      {
        log_printf ("ERROR: cannot load XML file '%s' (%s)", application_config, xml_error());
        return -1;
      }

    item = xml_first_child (root);
    if (item)
      {
        FORCHILDREN (child, item)
          {
            if (streq (xml_item_name(child), "service"))
              {
                service_name =  
                    mem_strdup (xml_get_attr (child, "name", NULL));
                service_display_name = 
                    mem_strdup (xml_get_attr (child, "display_name", NULL));
                service_trace_file =
                    mem_strdup (xml_get_attr (child, "trace_file", "smt_service.log"));
                service_debug = 
                    atoi (xml_get_attr(child, "debug", "0"));
                break;
              }
          }
      }

#if (defined(WIN32))
    /* service_name and service_display_name are only used with windows     */
    /* services, when registering or removing the service.                  */
    /* these fields are mandatory in Windows service configuration, but     */
    /* not used in UNIX daemons                                             */
    if (!service_name)
      {
        log_printf (
            "ERROR: item 'service_name' is missing in XML file '%s'", 
            application_config);
        res = -1;
      }
    if (!service_display_name)
      {
        log_printf (
            "ERROR: item 'service_text' is missing in XML file '%s'", 
            application_config);
        res = -1;
      }
#endif    

    xml_free (root);

    if (!res)
      {
        debug_printf ("Service configuration successfully loaded");
      }
    else
        free_resources ();

    return res;
}


/*  ---------------------------------------------------------------------[<]-
    Function: parse_command_line

    Synopsis: 

    Returns
    ---------------------------------------------------------------------[>]-*/
static int 
parse_command_line (int argc, char **argv)
{
    int
        res = ACTION_NOARG;

    if (argc <= 1)
        return ACTION_NOARG;

    /* there is at least ONE argument */
    if (argv[1][0] != '-')
      {
        log_printf ("ERROR: invalid argument");
        return ACTION_ERROR;
      }

    /* first argument is starting with '-' */
    switch (argv[1][1])
      {
        case 'h':
            res = ACTION_HELP;
            break;
#if (defined (WIN32))    
        case 'i':
            res = ACTION_INSTALL;
            if (argc >= 3)
                account = xstrcpy (NULL, ".\\", argv[2], NULL);
            if (argc >= 4)
                password = mem_strdup (argv[3]);
            break;
        case 'u':
            res = ACTION_UNINSTALL;
            break;
        case 'd':
            res = ACTION_CONSOLE;
            break;
#elif (defined (__UNIX__))
        case 's':
            res = ACTION_BACKGROUND;
            break;
#endif        /* defined WIN32 */
        default:
            log_printf ("ERROR: invalid action (%s)", argv[1]);
            return ACTION_ERROR;
      }

    /* we have a valid argument */
    if (res != ACTION_INSTALL)
      {
        if (argc > 2)
          {
            log_printf ("ERROR: too many arguments");
            return ACTION_ERROR;
          }
      }
    else
      {
        /* we collect optional user / password */
        if (argc > 4)
          {
            log_printf ("ERROR: too many arguments");
            return ACTION_ERROR;
          }

      }

    return res;
}


#if (defined(WIN32))
/*  ---------------------------------------------------------------------[<]-
    Function: remove_service

    Synopsis: Stops and removes the service
    ---------------------------------------------------------------------[>]-*/

static void 
remove_service (void)
{
    SC_HANDLE
        service,
        manager;

    manager = OpenSCManagerA (
                        NULL,                  /* machine (NULL == local)    */
                        NULL,                  /* database (NULL == default) */
                        SC_MANAGER_ALL_ACCESS  /* access required            */
                        );
    if (manager)
      {
        service = OpenServiceA (manager, service_name, SERVICE_ALL_ACCESS);
        if (service)
          {
            /*  Try to stop the service                                      */
            if (ControlService (service, SERVICE_CONTROL_STOP, 
                &service_status))
              {
                log_printf ("%s: stopping service '%s'...",
                         application_name, service_name);
                sleep (1);

                while (QueryServiceStatus (service, &service_status))
                    if (service_status.dwCurrentState == SERVICE_STOP_PENDING)
                      {
                        printf (".");
                        sleep  (1);
                      }
                    else
                        break;

                if (service_status.dwCurrentState == SERVICE_STOPPED)
                    log_printf (" Ok");
                else
                    log_printf (" Failed");
              }

            /*  Now remove the service                                       */
            if (DeleteService (service))
                log_printf ("%s: service '%s' removed",
                         application_name, service_name);
            else
                log_printf ("%s: DeleteService '%s' failed: %s",
                         application_name, service_name,
                         get_last_error_text (error_buffer, LINE_MAX));

            CloseServiceHandle (service);
          }
        else
            log_printf ("%s: OpenService '%s' failed: %s",
                     application_name, service_name,
                     get_last_error_text (error_buffer, LINE_MAX));

        CloseServiceHandle (manager);
    }
    else
        log_printf ("%s: OpenSCManager failed: %s",
                 application_name,
                 get_last_error_text (error_buffer, LINE_MAX));
}


/*  ---------------------------------------------------------------------[<]-
    Function: report_smt_error

    Synopsis: Pops-up a window; returns 0 if the user asked to debug the
    error, 1 if the user asked to continue.
    ---------------------------------------------------------------------[>]-*/

static int
report_smt_error (void)
{
    char
        *message;
    FILE
        *crash_log;

    message = xstrcpy (NULL,
        http_time_str (), " ", application_version, "\r\n",
        smt_crash_report (), 
        NULL);
    add_to_message_log (message);

    /*  Log error to crash.log file                                          */
    crash_log = fopen ("crash.log", "a");
    if (crash_log)
      {
        fprintf (crash_log, message);
        fclose (crash_log);
      }
    /*  Recover silently if necessary                                        */
    return (EXCEPTION_EXECUTE_HANDLER);
}


/*  ---------------------------------------------------------------------[<]-
    Function: report_status

    Synopsis: Sets the current status of the service and
              reports it to the Service Control Manager.
    ---------------------------------------------------------------------[>]-*/
static Bool
report_status (DWORD state, DWORD exit_code, DWORD wait_hint)
{
    static DWORD
        check_point = 1;
    Bool
       result       = TRUE;

    /*  when debugging we don't report to the SCM                            */
    if (!console_mode)
      {
        if (state == SERVICE_START_PENDING)
            service_status.dwControlsAccepted = 0;
        else
            service_status.dwControlsAccepted = SERVICE_ACCEPT_STOP;

        service_status.dwCurrentState  = state;
        service_status.dwWin32ExitCode = exit_code;
        service_status.dwWaitHint      = wait_hint;

        if (state == SERVICE_RUNNING
        ||  state == SERVICE_STOPPED)
            service_status.dwCheckPoint = 0;
        else
            service_status.dwCheckPoint = check_point++;


        /* Report the status of the service to the service control manager.  */
        result = SetServiceStatus (service_status_handle, &service_status);
        if (!result)
            add_to_message_log ("SetServiceStatus failed");
      }
    return result;
}


/*  ---------------------------------------------------------------------[<]-
    Function: service_control

    Synopsis: This function is called by the service control manager whenever
              ControlService() is called on this service.
    ---------------------------------------------------------------------[>]-*/

void WINAPI
service_control (DWORD control_code)
{
    /* Handle the requested control code.                                    */
    switch (control_code)
      {
        case SERVICE_CONTROL_STOP:      /*  Stop the service                 */
            service_status.dwCurrentState = SERVICE_STOP_PENDING;
            service_stop ();
            break;

        case SERVICE_CONTROL_INTERROGATE:/* Update the service status        */
            break;
        default:                        /*  Invalid control code             */
            break;
      }
    report_status (service_status.dwCurrentState, NO_ERROR, 0);
}
#endif    /* defined WIN32 */



/*  ---------------------------------------------------------------------[<]-
    Function: service_main

    Synopsis: This routine performs the service initialization and then calls
              the user defined service_start() routine to perform majority
              of the work.
    ---------------------------------------------------------------------[>]-*/
static char *
service_get_config_filename (const char *binary_name)
{
    char 
        *res = NULL;

    if (!binary_name || strnull(binary_name))
        return NULL;

    /* we allocate 4 extra char to append the ".cfg" extension */
    res = mem_alloc (strlen(binary_name) + 4 + 1);
    if (res)
      {
        strcpy (res, binary_name);
        strip_file_path (res);
        res = fixed_extension (res, res, "cfg");
      }

    return res;
}



#if (defined(WIN32))
/*  ---------------------------------------------------------------------[<]-
    Function: service_main

    Synopsis: This routine performs the service initialization and then calls
              the user defined service_start() routine to perform majority
              of the work.
    ---------------------------------------------------------------------[>]-*/

void WINAPI
service_main (int argc, char **argv)
{
    /* Register our service control handler:                                 */
    service_status_handle 
        = RegisterServiceCtrlHandlerA (service_name, service_control);
    if (!service_status_handle)
        return;

    service_status.dwServiceType             = SERVICE_WIN32_OWN_PROCESS;
    service_status.dwServiceSpecificExitCode = 0;

    /* report the status to the service control manager.                     */
    if (report_status (
            SERVICE_START_PENDING,      /*  Service state                    */
            NO_ERROR,                   /*  Exit code                        */
            3000))                      /*  Wait Hint                        */
        service_start (argc, argv);

    /* Try to report the stopped status to the service control manager.      */
    if (service_status_handle)
        report_status (SERVICE_STOPPED, error_code, 0);
}




/*  ---------------------------------------------------------------------[<]-
    Function: service_start

    Synopsis: Main routine for xitami web server. The service stops when
    server_stop_event is signaled.
    ---------------------------------------------------------------------[>]-*/
static void
service_start (int argc, char **argv)
{
    int
        argn;                           /*  Argument number                  */
    Bool
        args_ok     = TRUE,             /*  Were the arguments okay?         */
        quite_mode  = FALSE;            /*  -q means suppress all output     */
    char
        **argparm = NULL;               /*  Argument parameter to pick-up    */
    DWORD
        wait;

    argparm = NULL;
    for (argn = 1; argn < argc; argn++)
      {
        /*  If argparm is set, we have to collect an argument parameter      */
        if (argparm)
          {
            if (*argv [argn] != '-')    /*  Parameter can't start with '-'   */
              {
                free (*argparm);
                *argparm = strdupl (argv [argn]);
                argparm = NULL;
              }
            else
              {
                args_ok = FALSE;
                break;
              }
          }
        else
        if (*argv [argn] == '-')
          {
            switch (argv [argn][1])
              {
                /*  These switches have an immediate effect                  */
                case 'q':
                    quite_mode = TRUE;
                    break;
                /*  Used only for service                                    */
                case 'i':
                case 'd':
                case 'u':
                case 'v':
                case 'h':
                    break;
                /*  Anything else is an error                                */
                default:
                    args_ok = FALSE;
              }
          }
        else
          {
            args_ok = FALSE;
            break;
          }
      }


    /*  If there was a missing parameter or an argument error, quit          */
    if (argparm)
      {
        add_to_message_log (
            strprintf (
                "Argument missing - type '%s -h' for help",
                application_name));
        return;
      }
    else
    if (!args_ok)
      {
        add_to_message_log (
            strprintf (
                "Invalid arguments - type '%s -h' for help",
                application_name));
        return;
      }
    /* Service initialization                                                */

    /* Report the status to the service control manager.                     */
    if (!report_status (
            SERVICE_START_PENDING,      /* Service state                     */
            NO_ERROR,                   /* Exit code                         */
            3000))                      /* wait hint                         */
        return;

    /* Create the event object. The control handler function signals         */
    /* this event when it receives the "stop" control code.                  */
    server_stop_event = CreateEvent (
                            NULL,       /* no security attributes            */
                            TRUE,       /* manual reset event                */
                            FALSE,      /* not-signalled                     */
                            NULL);      /* no name                           */

    if (server_stop_event == NULL)
        return;

    /* report the status to the service control manager.                     */
    if (!report_status (
            SERVICE_START_PENDING,      /* Service state                     */
            NO_ERROR,                   /* Exit code                         */
            3000))                      /* wait hint                         */
      {
        CloseHandle (server_stop_event);
        return;
      }
    if (quite_mode)
      {
        fclose (stdout);                /*  Kill standard output             */
        fclose (stderr);                /*   and standard error              */
      }
    /*  Report the status to the service control manager.                    */
    if (!report_status (SERVICE_RUNNING, NO_ERROR, 0))                
      {
        CloseHandle (server_stop_event);
        return;
      }


    smt_init ();
    if (smt_agents_init_fct != NULL)
        control_break = (*smt_agents_init_fct)(application_config);

    while (!control_break)
      {
        FOREVER 
          {
            __try 
              {
                if (!smt_exec_step ())  /*  Run just one step                */
                  {
                    control_break = TRUE;
                    break;
                  }
              }
            __except (report_smt_error ()) 
              {
                /*  Fatal error handling                                     */
                smt_term  ();           /*  Shut-down SMT kernel             */
                mem_freeall ();         /*  Free ALL allocated memory        */
                break;                  /*  We'll recover control            */
              }
            wait = WaitForSingleObject (server_stop_event, 0);
            if (wait != WAIT_TIMEOUT)
                smt_shutdown ();        /*  Shut down the HTTP server        */
          }
        if (control_break)
            break;
      }

    if (smt_agents_term_fct != NULL)
        (*smt_agents_term_fct)();
    smt_term ();

    CloseHandle (server_stop_event);

    free_resources ();
}


/*  ---------------------------------------------------------------------[<]-
    Function: service_stop

    Synopsis: Stops the service. If a service_stop procedure is going to
    take longer than 3 seconds to execute, it should spawn a thread to
    execute the stop code, and return.  Otherwise, ServiceControlManager
    will believe that the service has stopped responding.
    ---------------------------------------------------------------------[>]-*/

void
service_stop (void)
{
    if (server_stop_event)
        SetEvent (server_stop_event);
}


/*  ---------------------------------------------------------------------[<]-
    Function: set_win95_service

    Synopsis: Add or remove from the windows registry the value to run
              the web server on startup.
    ---------------------------------------------------------------------[>]-*/
static long
set_win95_service (Bool add)
{
    HKEY
        key;
    DWORD
        disp;
    long
        feedback;
    static char
        path [LINE_MAX + 1];

    feedback = RegCreateKeyExA (HKEY_LOCAL_MACHINE, REGISTRY_RUN, 
        0, REG_NONE, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &key, &disp);

    if (feedback == ERROR_SUCCESS)
      {
        if (add)
          {
            GetModuleFileNameA (NULL, path, LINE_MAX);
            feedback = RegSetValueExA (key, "kserver", 0, REG_SZ,
                                      (CONST BYTE *) path, strlen (path) + 1);
            log_printf ("kserver: service '%s' installed", service_name);
            coputs   ("Restart windows to run service...");
          }
        else
          {
            feedback = RegDeleteValueA (key, "kserver");
            log_printf ("Updater: service '%s' uninstalled", service_name);
          }
        RegCloseKey (key);
      }
    return (feedback);
}

#endif          /* defined WIN32 */
