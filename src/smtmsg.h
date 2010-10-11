/*---------------------------------------------------------------------------
 *  smtmsg.h - prototypes for SMT standard messages.
 *
 *  Generated from smtmsg.xml by smtmesg.gsl using GSL.
 *  DO NOT MODIFY THIS FILE.
 *
 *  For documentation and updates see http://www.imatix.com.
 *---------------------------------------------------------------------------*/
#ifndef INCLUDE_SMTMSG
#define INCLUDE_SMTMSG

/*---------------------------------------------------------------------------
 *  Definitions and prototypes for smtlib - SMT kernel.
 *---------------------------------------------------------------------------*/

typedef struct {
    word  signal;                       /*  Signal that provoked shutdown    */
} struct_smtlib_shutdown;

int
put_smtlib_shutdown (
          byte **_buffer,
    const word  signal);                /*  Signal that provoked shutdown    */

int
get_smtlib_shutdown (
    byte *_buffer,
    struct_smtlib_shutdown **params);

void
free_smtlib_shutdown (
    struct_smtlib_shutdown **params);

/*---------------------------------------------------------------------------
 *  Definitions and prototypes for shutdown - .
 *---------------------------------------------------------------------------*/

extern char *SMTLIB_SHUTDOWN;

#define declare_smtlib_shutdown(_event, _priority)                             \
    method_declare (agent, SMTLIB_SHUTDOWN, _event, _priority)

/*  Send event - shutdown                                                    */

int 
lsend_smtlib_shutdown (
    const QID  *_to,
    const QID  *_from,
          char *_accept,
          char *_reject,
          char *_expire,
          word _timeout,
    const word  signal);            /*  Signal that provoked shutdown    */

#define send_smtlib_shutdown(_to,                                              \
            signal)                                                          \
       lsend_smtlib_shutdown(_to,                                              \
            &thread-> queue-> qid,                                           \
            NULL, NULL, NULL, 0,                                             \
            signal)


/*---------------------------------------------------------------------------
 *  Definitions and prototypes for smtlog - SMT Log agent.
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
 *  Definitions and prototypes for plain - .
 *---------------------------------------------------------------------------*/

extern char *SMTLOG_PLAIN;

#define declare_smtlog_plain(_event, _priority)                                \
    method_declare (agent, SMTLOG_PLAIN, _event, _priority)

/*  Send event - plain                                                       */

#define lsend_smtlog_plain(_to, _from,                                         \
    _accept, _reject, _expire, _timeout)                                     \
        event_send (_to,                                                     \
                    _from,                                                   \
                    SMTLOG_PLAIN,                                              \
                    NULL, 0,                                                 \
                    _accept, _reject, _expire, _timeout)
#define send_smtlog_plain(_to)                                                 \
        event_send (_to,                                                     \
                    &thread-> queue-> qid,                                   \
                    SMTLOG_PLAIN,                                              \
                    NULL, 0,                                                 \
                    NULL, NULL, NULL, 0)


/*---------------------------------------------------------------------------
 *  Definitions and prototypes for stamp - .
 *---------------------------------------------------------------------------*/

extern char *SMTLOG_STAMP;

#define declare_smtlog_stamp(_event, _priority)                                \
    method_declare (agent, SMTLOG_STAMP, _event, _priority)

/*  Send event - stamp                                                       */

#define lsend_smtlog_stamp(_to, _from,                                         \
    _accept, _reject, _expire, _timeout)                                     \
        event_send (_to,                                                     \
                    _from,                                                   \
                    SMTLOG_STAMP,                                              \
                    NULL, 0,                                                 \
                    _accept, _reject, _expire, _timeout)
#define send_smtlog_stamp(_to)                                                 \
        event_send (_to,                                                     \
                    &thread-> queue-> qid,                                   \
                    SMTLOG_STAMP,                                              \
                    NULL, 0,                                                 \
                    NULL, NULL, NULL, 0)


/*---------------------------------------------------------------------------
 *  Definitions and prototypes for close - .
 *---------------------------------------------------------------------------*/

extern char *SMTLOG_CLOSE;

#define declare_smtlog_close(_event, _priority)                                \
    method_declare (agent, SMTLOG_CLOSE, _event, _priority)

/*  Send event - close                                                       */

#define lsend_smtlog_close(_to, _from,                                         \
    _accept, _reject, _expire, _timeout)                                     \
        event_send (_to,                                                     \
                    _from,                                                   \
                    SMTLOG_CLOSE,                                              \
                    NULL, 0,                                                 \
                    _accept, _reject, _expire, _timeout)
#define send_smtlog_close(_to)                                                 \
        event_send (_to,                                                     \
                    &thread-> queue-> qid,                                   \
                    SMTLOG_CLOSE,                                              \
                    NULL, 0,                                                 \
                    NULL, NULL, NULL, 0)


typedef struct {
    char *filename;                     /*                                   */
} struct_smtlog_filename;

int
put_smtlog_filename (
          byte **_buffer,
    const char *filename);              /*                                   */

int
get_smtlog_filename (
    byte *_buffer,
    struct_smtlog_filename **params);

void
free_smtlog_filename (
    struct_smtlog_filename **params);

/*---------------------------------------------------------------------------
 *  Definitions and prototypes for cycle - .
 *---------------------------------------------------------------------------*/

extern char *SMTLOG_CYCLE;

#define declare_smtlog_cycle(_event, _priority)                                \
    method_declare (agent, SMTLOG_CYCLE, _event, _priority)

/*  Send event - cycle                                                       */

int 
lsend_smtlog_cycle (
    const QID  *_to,
    const QID  *_from,
          char *_accept,
          char *_reject,
          char *_expire,
          word _timeout,
    const char *filename);          /*                                   */

#define send_smtlog_cycle(_to,                                                 \
            filename)                                                        \
       lsend_smtlog_cycle(_to,                                                 \
            &thread-> queue-> qid,                                           \
            NULL, NULL, NULL, 0,                                             \
            filename)

/*---------------------------------------------------------------------------
 *  Definitions and prototypes for open - .
 *---------------------------------------------------------------------------*/

extern char *SMTLOG_OPEN;

#define declare_smtlog_open(_event, _priority)                                 \
    method_declare (agent, SMTLOG_OPEN, _event, _priority)

/*  Send event - open                                                        */

int 
lsend_smtlog_open (
    const QID  *_to,
    const QID  *_from,
          char *_accept,
          char *_reject,
          char *_expire,
          word _timeout,
    const char *filename);          /*                                   */

#define send_smtlog_open(_to,                                                  \
            filename)                                                        \
       lsend_smtlog_open(_to,                                                  \
            &thread-> queue-> qid,                                           \
            NULL, NULL, NULL, 0,                                             \
            filename)

/*---------------------------------------------------------------------------
 *  Definitions and prototypes for append - .
 *---------------------------------------------------------------------------*/

extern char *SMTLOG_APPEND;

#define declare_smtlog_append(_event, _priority)                               \
    method_declare (agent, SMTLOG_APPEND, _event, _priority)

/*  Send event - append                                                      */

int 
lsend_smtlog_append (
    const QID  *_to,
    const QID  *_from,
          char *_accept,
          char *_reject,
          char *_expire,
          word _timeout,
    const char *filename);          /*                                   */

#define send_smtlog_append(_to,                                                \
            filename)                                                        \
       lsend_smtlog_append(_to,                                                \
            &thread-> queue-> qid,                                           \
            NULL, NULL, NULL, 0,                                             \
            filename)


typedef struct {
    char *text;                         /*                                   */
} struct_smtlog_text;

int
put_smtlog_text (
          byte **_buffer,
    const char *text);                  /*                                   */

int
get_smtlog_text (
    byte *_buffer,
    struct_smtlog_text **params);

void
free_smtlog_text (
    struct_smtlog_text **params);

/*---------------------------------------------------------------------------
 *  Definitions and prototypes for put - .
 *---------------------------------------------------------------------------*/

extern char *SMTLOG_PUT;

#define declare_smtlog_put(_event, _priority)                                  \
    method_declare (agent, SMTLOG_PUT, _event, _priority)

/*  Send event - put                                                         */

int 
lsend_smtlog_put (
    const QID  *_to,
    const QID  *_from,
          char *_accept,
          char *_reject,
          char *_expire,
          word _timeout,
    const char *text);              /*                                   */

#define send_smtlog_put(_to,                                                   \
            text)                                                            \
       lsend_smtlog_put(_to,                                                   \
            &thread-> queue-> qid,                                           \
            NULL, NULL, NULL, 0,                                             \
            text)


/*---------------------------------------------------------------------------
 *  Definitions and prototypes for smtxlog - SMT Extended Logging Agent.
 *---------------------------------------------------------------------------*/

typedef struct {
    char *log_path;                     /*  Path for logfiles, or empty      */
    char *log_file;                     /*  Name of logfile                  */
    char *log_format;                   /*  Desired logging format           */
    char *cycle_when;                   /*  When to cycle logfile            */
    char *cycle_how;                    /*  How to cycle logfile             */
    char *cycle_time;                   /*  For time-based cycling           */
    char *cycle_date;                   /*  For date-based cycling           */
    char *cycle_size;                   /*  For size-based cycling           */
    char *cycle_lines;                  /*  For size-based cycling           */
    char *cycle_argument;               /*  For other cycle methods          */
} struct_smtxlog_open;

int
put_smtxlog_open (
          byte **_buffer,
    const char *log_path,               /*  Path for logfiles, or empty      */
    const char *log_file,               /*  Name of logfile                  */
    const char *log_format,             /*  Desired logging format           */
    const char *cycle_when,             /*  When to cycle logfile            */
    const char *cycle_how,              /*  How to cycle logfile             */
    const char *cycle_time,             /*  For time-based cycling           */
    const char *cycle_date,             /*  For date-based cycling           */
    const char *cycle_size,             /*  For size-based cycling           */
    const char *cycle_lines,            /*  For size-based cycling           */
    const char *cycle_argument);        /*  For other cycle methods          */

int
get_smtxlog_open (
    byte *_buffer,
    struct_smtxlog_open **params);

void
free_smtxlog_open (
    struct_smtxlog_open **params);

/*---------------------------------------------------------------------------
 *  Definitions and prototypes for open - .
 *---------------------------------------------------------------------------*/

extern char *SMTXLOG_OPEN;

#define declare_smtxlog_open(_event, _priority)                                \
    method_declare (agent, SMTXLOG_OPEN, _event, _priority)

/*  Send event - open                                                        */

int 
lsend_smtxlog_open (
    const QID  *_to,
    const QID  *_from,
          char *_accept,
          char *_reject,
          char *_expire,
          word _timeout,
    const char *log_path,           /*  Path for logfiles, or empty      */
    const char *log_file,           /*  Name of logfile                  */
    const char *log_format,         /*  Desired logging format           */
    const char *cycle_when,         /*  When to cycle logfile            */
    const char *cycle_how,          /*  How to cycle logfile             */
    const char *cycle_time,         /*  For time-based cycling           */
    const char *cycle_date,         /*  For date-based cycling           */
    const char *cycle_size,         /*  For size-based cycling           */
    const char *cycle_lines,        /*  For size-based cycling           */
    const char *cycle_argument);    /*  For other cycle methods          */

#define send_smtxlog_open(_to,                                                 \
            log_path,                                                        \
            log_file,                                                        \
            log_format,                                                      \
            cycle_when,                                                      \
            cycle_how,                                                       \
            cycle_time,                                                      \
            cycle_date,                                                      \
            cycle_size,                                                      \
            cycle_lines,                                                     \
            cycle_argument)                                                  \
       lsend_smtxlog_open(_to,                                                 \
            &thread-> queue-> qid,                                           \
            NULL, NULL, NULL, 0,                                             \
            log_path,                                                        \
            log_file,                                                        \
            log_format,                                                      \
            cycle_when,                                                      \
            cycle_how,                                                       \
            cycle_time,                                                      \
            cycle_date,                                                      \
            cycle_size,                                                      \
            cycle_lines,                                                     \
            cycle_argument)


typedef struct {
    char *file_name;                    /*  Filename used for request        */
    word  value_size;                   /*  Value size                       */
    void *value;                        /*  Value to log                     */
} struct_smtxlog_log;

int
put_smtxlog_log (
          byte **_buffer,
    const char *file_name,              /*  Filename used for request        */
    const word  value_size,             /*  Value size                       */
    const void *value);                 /*  Value to log                     */

int
get_smtxlog_log (
    byte *_buffer,
    struct_smtxlog_log **params);

void
free_smtxlog_log (
    struct_smtxlog_log **params);

/*---------------------------------------------------------------------------
 *  Definitions and prototypes for log - .
 *---------------------------------------------------------------------------*/

extern char *SMTXLOG_LOG;

#define declare_smtxlog_log(_event, _priority)                                 \
    method_declare (agent, SMTXLOG_LOG, _event, _priority)

/*  Send event - log                                                         */

int 
lsend_smtxlog_log (
    const QID  *_to,
    const QID  *_from,
          char *_accept,
          char *_reject,
          char *_expire,
          word _timeout,
    const char *file_name,          /*  Filename used for request        */
    const word  value_size,         /*  Value size                       */
    const void *value);             /*  Value to log                     */

#define send_smtxlog_log(_to,                                                  \
            file_name,                                                       \
            value_size,                                                      \
            value)                                                           \
       lsend_smtxlog_log(_to,                                                  \
            &thread-> queue-> qid,                                           \
            NULL, NULL, NULL, 0,                                             \
            file_name,                                                       \
            value_size,                                                      \
            value)


typedef struct {
    char *message;                      /*  Line of text to log              */
} struct_smtxlog_put;

int
put_smtxlog_put (
          byte **_buffer,
    const char *message);               /*  Line of text to log              */

int
get_smtxlog_put (
    byte *_buffer,
    struct_smtxlog_put **params);

void
free_smtxlog_put (
    struct_smtxlog_put **params);

/*---------------------------------------------------------------------------
 *  Definitions and prototypes for put - .
 *---------------------------------------------------------------------------*/

extern char *SMTXLOG_PUT;

#define declare_smtxlog_put(_event, _priority)                                 \
    method_declare (agent, SMTXLOG_PUT, _event, _priority)

/*  Send event - put                                                         */

int 
lsend_smtxlog_put (
    const QID  *_to,
    const QID  *_from,
          char *_accept,
          char *_reject,
          char *_expire,
          word _timeout,
    const char *message);           /*  Line of text to log              */

#define send_smtxlog_put(_to,                                                  \
            message)                                                         \
       lsend_smtxlog_put(_to,                                                  \
            &thread-> queue-> qid,                                           \
            NULL, NULL, NULL, 0,                                             \
            message)


/*---------------------------------------------------------------------------
 *  Definitions and prototypes for cycle - .
 *---------------------------------------------------------------------------*/

extern char *SMTXLOG_CYCLE;

#define declare_smtxlog_cycle(_event, _priority)                               \
    method_declare (agent, SMTXLOG_CYCLE, _event, _priority)

/*  Send event - cycle                                                       */

#define lsend_smtxlog_cycle(_to, _from,                                        \
    _accept, _reject, _expire, _timeout)                                     \
        event_send (_to,                                                     \
                    _from,                                                   \
                    SMTXLOG_CYCLE,                                             \
                    NULL, 0,                                                 \
                    _accept, _reject, _expire, _timeout)
#define send_smtxlog_cycle(_to)                                                \
        event_send (_to,                                                     \
                    &thread-> queue-> qid,                                   \
                    SMTXLOG_CYCLE,                                             \
                    NULL, 0,                                                 \
                    NULL, NULL, NULL, 0)


/*---------------------------------------------------------------------------
 *  Definitions and prototypes for clear - .
 *---------------------------------------------------------------------------*/

extern char *SMTXLOG_CLEAR;

#define declare_smtxlog_clear(_event, _priority)                               \
    method_declare (agent, SMTXLOG_CLEAR, _event, _priority)

/*  Send event - clear                                                       */

#define lsend_smtxlog_clear(_to, _from,                                        \
    _accept, _reject, _expire, _timeout)                                     \
        event_send (_to,                                                     \
                    _from,                                                   \
                    SMTXLOG_CLEAR,                                             \
                    NULL, 0,                                                 \
                    _accept, _reject, _expire, _timeout)
#define send_smtxlog_clear(_to)                                                \
        event_send (_to,                                                     \
                    &thread-> queue-> qid,                                   \
                    SMTXLOG_CLEAR,                                             \
                    NULL, 0,                                                 \
                    NULL, NULL, NULL, 0)


/*---------------------------------------------------------------------------
 *  Definitions and prototypes for close - .
 *---------------------------------------------------------------------------*/

extern char *SMTXLOG_CLOSE;

#define declare_smtxlog_close(_event, _priority)                               \
    method_declare (agent, SMTXLOG_CLOSE, _event, _priority)

/*  Send event - close                                                       */

#define lsend_smtxlog_close(_to, _from,                                        \
    _accept, _reject, _expire, _timeout)                                     \
        event_send (_to,                                                     \
                    _from,                                                   \
                    SMTXLOG_CLOSE,                                             \
                    NULL, 0,                                                 \
                    _accept, _reject, _expire, _timeout)
#define send_smtxlog_close(_to)                                                \
        event_send (_to,                                                     \
                    &thread-> queue-> qid,                                   \
                    SMTXLOG_CLOSE,                                             \
                    NULL, 0,                                                 \
                    NULL, NULL, NULL, 0)


/*---------------------------------------------------------------------------
 *  Definitions and prototypes for smtoper - Operator agent.
 *---------------------------------------------------------------------------*/

typedef struct {
    char *agent_name;                   /*  Name of logging agent            */
    char *thread_name;                  /*  Name of logging thread           */
} struct_smtoper_set_log;

int
put_smtoper_set_log (
          byte **_buffer,
    const char *agent_name,             /*  Name of logging agent            */
    const char *thread_name);           /*  Name of logging thread           */

int
get_smtoper_set_log (
    byte *_buffer,
    struct_smtoper_set_log **params);

void
free_smtoper_set_log (
    struct_smtoper_set_log **params);

/*---------------------------------------------------------------------------
 *  Definitions and prototypes for set log - .
 *---------------------------------------------------------------------------*/

extern char *SMTOPER_SET_LOG;

#define declare_smtoper_set_log(_event, _priority)                             \
    method_declare (agent, SMTOPER_SET_LOG, _event, _priority)

/*  Send event - set log                                                     */

int 
lsend_smtoper_set_log (
    const QID  *_to,
    const QID  *_from,
          char *_accept,
          char *_reject,
          char *_expire,
          word _timeout,
    const char *agent_name,         /*  Name of logging agent            */
    const char *thread_name);       /*  Name of logging thread           */

#define send_smtoper_set_log(_to,                                              \
            agent_name,                                                      \
            thread_name)                                                     \
       lsend_smtoper_set_log(_to,                                              \
            &thread-> queue-> qid,                                           \
            NULL, NULL, NULL, 0,                                             \
            agent_name,                                                      \
            thread_name)


typedef struct {
    char *text;                         /*  Text of message                  */
} struct_smtoper_message;

int
put_smtoper_message (
          byte **_buffer,
    const char *text);                  /*  Text of message                  */

int
get_smtoper_message (
    byte *_buffer,
    struct_smtoper_message **params);

void
free_smtoper_message (
    struct_smtoper_message **params);

/*---------------------------------------------------------------------------
 *  Definitions and prototypes for error - .
 *---------------------------------------------------------------------------*/

extern char *SMTOPER_ERROR;

#define declare_smtoper_error(_event, _priority)                               \
    method_declare (agent, SMTOPER_ERROR, _event, _priority)

/*  Send event - error                                                       */

int 
lsend_smtoper_error (
    const QID  *_to,
    const QID  *_from,
          char *_accept,
          char *_reject,
          char *_expire,
          word _timeout,
    const char *text);              /*  Text of message                  */

#define send_smtoper_error(_to,                                                \
            text)                                                            \
       lsend_smtoper_error(_to,                                                \
            &thread-> queue-> qid,                                           \
            NULL, NULL, NULL, 0,                                             \
            text)

/*---------------------------------------------------------------------------
 *  Definitions and prototypes for warning - .
 *---------------------------------------------------------------------------*/

extern char *SMTOPER_WARNING;

#define declare_smtoper_warning(_event, _priority)                             \
    method_declare (agent, SMTOPER_WARNING, _event, _priority)

/*  Send event - warning                                                     */

int 
lsend_smtoper_warning (
    const QID  *_to,
    const QID  *_from,
          char *_accept,
          char *_reject,
          char *_expire,
          word _timeout,
    const char *text);              /*  Text of message                  */

#define send_smtoper_warning(_to,                                              \
            text)                                                            \
       lsend_smtoper_warning(_to,                                              \
            &thread-> queue-> qid,                                           \
            NULL, NULL, NULL, 0,                                             \
            text)

/*---------------------------------------------------------------------------
 *  Definitions and prototypes for info - .
 *---------------------------------------------------------------------------*/

extern char *SMTOPER_INFO;

#define declare_smtoper_info(_event, _priority)                                \
    method_declare (agent, SMTOPER_INFO, _event, _priority)

/*  Send event - info                                                        */

int 
lsend_smtoper_info (
    const QID  *_to,
    const QID  *_from,
          char *_accept,
          char *_reject,
          char *_expire,
          word _timeout,
    const char *text);              /*  Text of message                  */

#define send_smtoper_info(_to,                                                 \
            text)                                                            \
       lsend_smtoper_info(_to,                                                 \
            &thread-> queue-> qid,                                           \
            NULL, NULL, NULL, 0,                                             \
            text)


/*---------------------------------------------------------------------------
 *  Definitions and prototypes for enable - .
 *---------------------------------------------------------------------------*/

extern char *SMTOPER_ENABLE;

#define declare_smtoper_enable(_event, _priority)                              \
    method_declare (agent, SMTOPER_ENABLE, _event, _priority)

/*  Send event - enable                                                      */

#define lsend_smtoper_enable(_to, _from,                                       \
    _accept, _reject, _expire, _timeout)                                     \
        event_send (_to,                                                     \
                    _from,                                                   \
                    SMTOPER_ENABLE,                                            \
                    NULL, 0,                                                 \
                    _accept, _reject, _expire, _timeout)
#define send_smtoper_enable(_to)                                               \
        event_send (_to,                                                     \
                    &thread-> queue-> qid,                                   \
                    SMTOPER_ENABLE,                                            \
                    NULL, 0,                                                 \
                    NULL, NULL, NULL, 0)


/*---------------------------------------------------------------------------
 *  Definitions and prototypes for disable - .
 *---------------------------------------------------------------------------*/

extern char *SMTOPER_DISABLE;

#define declare_smtoper_disable(_event, _priority)                             \
    method_declare (agent, SMTOPER_DISABLE, _event, _priority)

/*  Send event - disable                                                     */

#define lsend_smtoper_disable(_to, _from,                                      \
    _accept, _reject, _expire, _timeout)                                     \
        event_send (_to,                                                     \
                    _from,                                                   \
                    SMTOPER_DISABLE,                                           \
                    NULL, 0,                                                 \
                    _accept, _reject, _expire, _timeout)
#define send_smtoper_disable(_to)                                              \
        event_send (_to,                                                     \
                    &thread-> queue-> qid,                                   \
                    SMTOPER_DISABLE,                                           \
                    NULL, 0,                                                 \
                    NULL, NULL, NULL, 0)


/*---------------------------------------------------------------------------
 *  Definitions and prototypes for smtrdns - Reverse DNS agent.
 *---------------------------------------------------------------------------*/

typedef struct {
    qbyte ip_address;                   /*  IP address in network order      */
    char *ip_string;                    /*  Alternative address in string format  */
    qbyte request_tag;                  /*  User-defined request tag         */
} struct_smtrdns_get_host_name;

int
put_smtrdns_get_host_name (
          byte **_buffer,
    const qbyte ip_address,             /*  IP address in network order      */
    const char *ip_string,              /*  Alternative address in string format  */
    const qbyte request_tag);           /*  User-defined request tag         */

int
get_smtrdns_get_host_name (
    byte *_buffer,
    struct_smtrdns_get_host_name **params);

void
free_smtrdns_get_host_name (
    struct_smtrdns_get_host_name **params);

/*---------------------------------------------------------------------------
 *  Definitions and prototypes for get host name - .
 *---------------------------------------------------------------------------*/

extern char *SMTRDNS_GET_HOST_NAME;

#define declare_smtrdns_get_host_name(_event, _priority)                       \
    method_declare (agent, SMTRDNS_GET_HOST_NAME, _event, _priority)

/*  Send event - get host name                                               */

int 
lsend_smtrdns_get_host_name (
    const QID  *_to,
    const QID  *_from,
          char *_accept,
          char *_reject,
          char *_expire,
          word _timeout,
    const qbyte ip_address,         /*  IP address in network order      */
    const char *ip_string,          /*  Alternative address in string format  */
    const qbyte request_tag);       /*  User-defined request tag         */

#define send_smtrdns_get_host_name(_to,                                        \
            ip_address,                                                      \
            ip_string,                                                       \
            request_tag)                                                     \
       lsend_smtrdns_get_host_name(_to,                                        \
            &thread-> queue-> qid,                                           \
            NULL, NULL, NULL, 0,                                             \
            ip_address,                                                      \
            ip_string,                                                       \
            request_tag)


typedef struct {
    char *host_name;                    /*  Host name to look-up             */
    qbyte request_tag;                  /*  User-defined request tag         */
} struct_smtrdns_get_host_ip;

int
put_smtrdns_get_host_ip (
          byte **_buffer,
    const char *host_name,              /*  Host name to look-up             */
    const qbyte request_tag);           /*  User-defined request tag         */

int
get_smtrdns_get_host_ip (
    byte *_buffer,
    struct_smtrdns_get_host_ip **params);

void
free_smtrdns_get_host_ip (
    struct_smtrdns_get_host_ip **params);

/*---------------------------------------------------------------------------
 *  Definitions and prototypes for get host ip - .
 *---------------------------------------------------------------------------*/

extern char *SMTRDNS_GET_HOST_IP;

#define declare_smtrdns_get_host_ip(_event, _priority)                         \
    method_declare (agent, SMTRDNS_GET_HOST_IP, _event, _priority)

/*  Send event - get host ip                                                 */

int 
lsend_smtrdns_get_host_ip (
    const QID  *_to,
    const QID  *_from,
          char *_accept,
          char *_reject,
          char *_expire,
          word _timeout,
    const char *host_name,          /*  Host name to look-up             */
    const qbyte request_tag);       /*  User-defined request tag         */

#define send_smtrdns_get_host_ip(_to,                                          \
            host_name,                                                       \
            request_tag)                                                     \
       lsend_smtrdns_get_host_ip(_to,                                          \
            &thread-> queue-> qid,                                           \
            NULL, NULL, NULL, 0,                                             \
            host_name,                                                       \
            request_tag)


typedef struct {
    qbyte ip_address;                   /*  IP address in network order      */
    char *host_name;                    /*  Host name                        */
    qbyte request_tag;                  /*  User-defined request tag         */
} struct_smtrdns_host_name;

int
put_smtrdns_host_name (
          byte **_buffer,
    const qbyte ip_address,             /*  IP address in network order      */
    const char *host_name,              /*  Host name                        */
    const qbyte request_tag);           /*  User-defined request tag         */

int
get_smtrdns_host_name (
    byte *_buffer,
    struct_smtrdns_host_name **params);

void
free_smtrdns_host_name (
    struct_smtrdns_host_name **params);

/*---------------------------------------------------------------------------
 *  Definitions and prototypes for host name - .
 *---------------------------------------------------------------------------*/

extern char *SMTRDNS_HOST_NAME;

#define declare_smtrdns_host_name(_event, _priority)                           \
    method_declare (agent, SMTRDNS_HOST_NAME, _event, _priority)

/*  Send event - host name                                                   */

int 
lsend_smtrdns_host_name (
    const QID  *_to,
    const QID  *_from,
          char *_accept,
          char *_reject,
          char *_expire,
          word _timeout,
    const qbyte ip_address,         /*  IP address in network order      */
    const char *host_name,          /*  Host name                        */
    const qbyte request_tag);       /*  User-defined request tag         */

#define send_smtrdns_host_name(_to,                                            \
            ip_address,                                                      \
            host_name,                                                       \
            request_tag)                                                     \
       lsend_smtrdns_host_name(_to,                                            \
            &thread-> queue-> qid,                                           \
            NULL, NULL, NULL, 0,                                             \
            ip_address,                                                      \
            host_name,                                                       \
            request_tag)


typedef struct {
    qbyte ip_address;                   /*  IP address in network order      */
    char *host_name;                    /*  Host name                        */
    qbyte request_tag;                  /*  User-defined request tag         */
} struct_smtrdns_host_ip;

int
put_smtrdns_host_ip (
          byte **_buffer,
    const qbyte ip_address,             /*  IP address in network order      */
    const char *host_name,              /*  Host name                        */
    const qbyte request_tag);           /*  User-defined request tag         */

int
get_smtrdns_host_ip (
    byte *_buffer,
    struct_smtrdns_host_ip **params);

void
free_smtrdns_host_ip (
    struct_smtrdns_host_ip **params);

/*---------------------------------------------------------------------------
 *  Definitions and prototypes for host ip - .
 *---------------------------------------------------------------------------*/

extern char *SMTRDNS_HOST_IP;

#define declare_smtrdns_host_ip(_event, _priority)                             \
    method_declare (agent, SMTRDNS_HOST_IP, _event, _priority)

/*  Send event - host ip                                                     */

int 
lsend_smtrdns_host_ip (
    const QID  *_to,
    const QID  *_from,
          char *_accept,
          char *_reject,
          char *_expire,
          word _timeout,
    const qbyte ip_address,         /*  IP address in network order      */
    const char *host_name,          /*  Host name                        */
    const qbyte request_tag);       /*  User-defined request tag         */

#define send_smtrdns_host_ip(_to,                                              \
            ip_address,                                                      \
            host_name,                                                       \
            request_tag)                                                     \
       lsend_smtrdns_host_ip(_to,                                              \
            &thread-> queue-> qid,                                           \
            NULL, NULL, NULL, 0,                                             \
            ip_address,                                                      \
            host_name,                                                       \
            request_tag)


/*---------------------------------------------------------------------------
 *  Definitions and prototypes for host error - .
 *---------------------------------------------------------------------------*/

extern char *SMTRDNS_HOST_ERROR;

#define declare_smtrdns_host_error(_event, _priority)                          \
    method_declare (agent, SMTRDNS_HOST_ERROR, _event, _priority)

/*  Send event - host error                                                  */

#define lsend_smtrdns_host_error(_to, _from,                                   \
    _accept, _reject, _expire, _timeout)                                     \
        event_send (_to,                                                     \
                    _from,                                                   \
                    SMTRDNS_HOST_ERROR,                                        \
                    NULL, 0,                                                 \
                    _accept, _reject, _expire, _timeout)
#define send_smtrdns_host_error(_to)                                           \
        event_send (_to,                                                     \
                    &thread-> queue-> qid,                                   \
                    SMTRDNS_HOST_ERROR,                                        \
                    NULL, 0,                                                 \
                    NULL, NULL, NULL, 0)


/*---------------------------------------------------------------------------
 *  Definitions and prototypes for host end - .
 *---------------------------------------------------------------------------*/

extern char *SMTRDNS_HOST_END;

#define declare_smtrdns_host_end(_event, _priority)                            \
    method_declare (agent, SMTRDNS_HOST_END, _event, _priority)

/*  Send event - host end                                                    */

#define lsend_smtrdns_host_end(_to, _from,                                     \
    _accept, _reject, _expire, _timeout)                                     \
        event_send (_to,                                                     \
                    _from,                                                   \
                    SMTRDNS_HOST_END,                                          \
                    NULL, 0,                                                 \
                    _accept, _reject, _expire, _timeout)
#define send_smtrdns_host_end(_to)                                             \
        event_send (_to,                                                     \
                    &thread-> queue-> qid,                                   \
                    SMTRDNS_HOST_END,                                          \
                    NULL, 0,                                                 \
                    NULL, NULL, NULL, 0)


/*---------------------------------------------------------------------------
 *  Definitions and prototypes for host timeout - .
 *---------------------------------------------------------------------------*/

extern char *SMTRDNS_HOST_TIMEOUT;

#define declare_smtrdns_host_timeout(_event, _priority)                        \
    method_declare (agent, SMTRDNS_HOST_TIMEOUT, _event, _priority)

/*  Send event - host timeout                                                */

#define lsend_smtrdns_host_timeout(_to, _from,                                 \
    _accept, _reject, _expire, _timeout)                                     \
        event_send (_to,                                                     \
                    _from,                                                   \
                    SMTRDNS_HOST_TIMEOUT,                                      \
                    NULL, 0,                                                 \
                    _accept, _reject, _expire, _timeout)
#define send_smtrdns_host_timeout(_to)                                         \
        event_send (_to,                                                     \
                    &thread-> queue-> qid,                                   \
                    SMTRDNS_HOST_TIMEOUT,                                      \
                    NULL, 0,                                                 \
                    NULL, NULL, NULL, 0)


/*---------------------------------------------------------------------------
 *  Definitions and prototypes for smtslot - Time slot agent.
 *---------------------------------------------------------------------------*/

typedef struct {
    char *times;                        /*  Time slot specification          */
} struct_smtslot_specification;

int
put_smtslot_specification (
          byte **_buffer,
    const char *times);                 /*  Time slot specification          */

int
get_smtslot_specification (
    byte *_buffer,
    struct_smtslot_specification **params);

void
free_smtslot_specification (
    struct_smtslot_specification **params);

/*---------------------------------------------------------------------------
 *  Definitions and prototypes for specify - .
 *---------------------------------------------------------------------------*/

extern char *SMTSLOT_SPECIFY;

#define declare_smtslot_specify(_event, _priority)                             \
    method_declare (agent, SMTSLOT_SPECIFY, _event, _priority)

/*  Send event - specify                                                     */

int 
lsend_smtslot_specify (
    const QID  *_to,
    const QID  *_from,
          char *_accept,
          char *_reject,
          char *_expire,
          word _timeout,
    const char *times);             /*  Time slot specification          */

#define send_smtslot_specify(_to,                                              \
            times)                                                           \
       lsend_smtslot_specify(_to,                                              \
            &thread-> queue-> qid,                                           \
            NULL, NULL, NULL, 0,                                             \
            times)


/*---------------------------------------------------------------------------
 *  Definitions and prototypes for reset - .
 *---------------------------------------------------------------------------*/

extern char *SMTSLOT_RESET;

#define declare_smtslot_reset(_event, _priority)                               \
    method_declare (agent, SMTSLOT_RESET, _event, _priority)

/*  Send event - reset                                                       */

#define lsend_smtslot_reset(_to, _from,                                        \
    _accept, _reject, _expire, _timeout)                                     \
        event_send (_to,                                                     \
                    _from,                                                   \
                    SMTSLOT_RESET,                                             \
                    NULL, 0,                                                 \
                    _accept, _reject, _expire, _timeout)
#define send_smtslot_reset(_to)                                                \
        event_send (_to,                                                     \
                    &thread-> queue-> qid,                                   \
                    SMTSLOT_RESET,                                             \
                    NULL, 0,                                                 \
                    NULL, NULL, NULL, 0)


/*---------------------------------------------------------------------------
 *  Definitions and prototypes for on - .
 *---------------------------------------------------------------------------*/

extern char *SMTSLOT_ON;

#define declare_smtslot_on(_event, _priority)                                  \
    method_declare (agent, SMTSLOT_ON, _event, _priority)

/*  Send event - on                                                          */

#define lsend_smtslot_on(_to, _from,                                           \
    _accept, _reject, _expire, _timeout)                                     \
        event_send (_to,                                                     \
                    _from,                                                   \
                    SMTSLOT_ON,                                                \
                    NULL, 0,                                                 \
                    _accept, _reject, _expire, _timeout)
#define send_smtslot_on(_to)                                                   \
        event_send (_to,                                                     \
                    &thread-> queue-> qid,                                   \
                    SMTSLOT_ON,                                                \
                    NULL, 0,                                                 \
                    NULL, NULL, NULL, 0)


/*---------------------------------------------------------------------------
 *  Definitions and prototypes for off - .
 *---------------------------------------------------------------------------*/

extern char *SMTSLOT_OFF;

#define declare_smtslot_off(_event, _priority)                                 \
    method_declare (agent, SMTSLOT_OFF, _event, _priority)

/*  Send event - off                                                         */

#define lsend_smtslot_off(_to, _from,                                          \
    _accept, _reject, _expire, _timeout)                                     \
        event_send (_to,                                                     \
                    _from,                                                   \
                    SMTSLOT_OFF,                                               \
                    NULL, 0,                                                 \
                    _accept, _reject, _expire, _timeout)
#define send_smtslot_off(_to)                                                  \
        event_send (_to,                                                     \
                    &thread-> queue-> qid,                                   \
                    SMTSLOT_OFF,                                               \
                    NULL, 0,                                                 \
                    NULL, NULL, NULL, 0)


/*---------------------------------------------------------------------------
 *  Definitions and prototypes for finish - .
 *---------------------------------------------------------------------------*/

extern char *SMTSLOT_FINISH;

#define declare_smtslot_finish(_event, _priority)                              \
    method_declare (agent, SMTSLOT_FINISH, _event, _priority)

/*  Send event - finish                                                      */

#define lsend_smtslot_finish(_to, _from,                                       \
    _accept, _reject, _expire, _timeout)                                     \
        event_send (_to,                                                     \
                    _from,                                                   \
                    SMTSLOT_FINISH,                                            \
                    NULL, 0,                                                 \
                    _accept, _reject, _expire, _timeout)
#define send_smtslot_finish(_to)                                               \
        event_send (_to,                                                     \
                    &thread-> queue-> qid,                                   \
                    SMTSLOT_FINISH,                                            \
                    NULL, 0,                                                 \
                    NULL, NULL, NULL, 0)


typedef struct {
    char *message;                      /*  Error message                    */
} struct_smtslot_error;

int
put_smtslot_error (
          byte **_buffer,
    const char *message);               /*  Error message                    */

int
get_smtslot_error (
    byte *_buffer,
    struct_smtslot_error **params);

void
free_smtslot_error (
    struct_smtslot_error **params);

/*---------------------------------------------------------------------------
 *  Definitions and prototypes for error - .
 *---------------------------------------------------------------------------*/

extern char *SMTSLOT_ERROR;

#define declare_smtslot_error(_event, _priority)                               \
    method_declare (agent, SMTSLOT_ERROR, _event, _priority)

/*  Send event - error                                                       */

int 
lsend_smtslot_error (
    const QID  *_to,
    const QID  *_from,
          char *_accept,
          char *_reject,
          char *_expire,
          word _timeout,
    const char *message);           /*  Error message                    */

#define send_smtslot_error(_to,                                                \
            message)                                                         \
       lsend_smtslot_error(_to,                                                \
            &thread-> queue-> qid,                                           \
            NULL, NULL, NULL, 0,                                             \
            message)


/*---------------------------------------------------------------------------
 *  Definitions and prototypes for switch on - .
 *---------------------------------------------------------------------------*/

extern char *SMTSLOT_SWITCH_ON;

#define declare_smtslot_switch_on(_event, _priority)                           \
    method_declare (agent, SMTSLOT_SWITCH_ON, _event, _priority)

/*  Send event - switch on                                                   */

#define lsend_smtslot_switch_on(_to, _from,                                    \
    _accept, _reject, _expire, _timeout)                                     \
        event_send (_to,                                                     \
                    _from,                                                   \
                    SMTSLOT_SWITCH_ON,                                         \
                    NULL, 0,                                                 \
                    _accept, _reject, _expire, _timeout)
#define send_smtslot_switch_on(_to)                                            \
        event_send (_to,                                                     \
                    &thread-> queue-> qid,                                   \
                    SMTSLOT_SWITCH_ON,                                         \
                    NULL, 0,                                                 \
                    NULL, NULL, NULL, 0)


/*---------------------------------------------------------------------------
 *  Definitions and prototypes for switch off - .
 *---------------------------------------------------------------------------*/

extern char *SMTSLOT_SWITCH_OFF;

#define declare_smtslot_switch_off(_event, _priority)                          \
    method_declare (agent, SMTSLOT_SWITCH_OFF, _event, _priority)

/*  Send event - switch off                                                  */

#define lsend_smtslot_switch_off(_to, _from,                                   \
    _accept, _reject, _expire, _timeout)                                     \
        event_send (_to,                                                     \
                    _from,                                                   \
                    SMTSLOT_SWITCH_OFF,                                        \
                    NULL, 0,                                                 \
                    _accept, _reject, _expire, _timeout)
#define send_smtslot_switch_off(_to)                                           \
        event_send (_to,                                                     \
                    &thread-> queue-> qid,                                   \
                    SMTSLOT_SWITCH_OFF,                                        \
                    NULL, 0,                                                 \
                    NULL, NULL, NULL, 0)


/*---------------------------------------------------------------------------
 *  Definitions and prototypes for smtsock - Socket i/o agent.
 *---------------------------------------------------------------------------*/

typedef struct {
    dbyte timeout;                      /*  Timeout in seconds, zero = none  */
    qbyte socket;                       /*  Socket to read from              */
    dbyte max_size;                     /*  Size of receiving buffer         */
    dbyte min_size;                     /*  Minimum data to read, zero = all  */
    void *tag;                          /*  User-defined request tag         */
} struct_smtsock_read;

int
put_smtsock_read (
          byte **_buffer,
    const dbyte timeout,                /*  Timeout in seconds, zero = none  */
    const qbyte socket,                 /*  Socket to read from              */
    const dbyte max_size,               /*  Size of receiving buffer         */
    const dbyte min_size,               /*  Minimum data to read, zero = all  */
    const void *tag);                   /*  User-defined request tag         */

int
get_smtsock_read (
    byte *_buffer,
    struct_smtsock_read **params);

void
free_smtsock_read (
    struct_smtsock_read **params);

/*---------------------------------------------------------------------------
 *  Definitions and prototypes for read - Read from socket.
 *---------------------------------------------------------------------------*/

extern char *SMTSOCK_READ;

#define declare_smtsock_read(_event, _priority)                                \
    method_declare (agent, SMTSOCK_READ, _event, _priority)

/*  Send event - read                                                        */

int 
lsend_smtsock_read (
    const QID  *_to,
    const QID  *_from,
          char *_accept,
          char *_reject,
          char *_expire,
          word _timeout,
    const dbyte timeout,            /*  Timeout in seconds, zero = none  */
    const qbyte socket,             /*  Socket to read from              */
    const dbyte max_size,           /*  Size of receiving buffer         */
    const dbyte min_size,           /*  Minimum data to read, zero = all  */
    const void *tag);               /*  User-defined request tag         */

#define send_smtsock_read(_to,                                                 \
            timeout,                                                         \
            socket,                                                          \
            max_size,                                                        \
            min_size,                                                        \
            tag)                                                             \
       lsend_smtsock_read(_to,                                                 \
            &thread-> queue-> qid,                                           \
            NULL, NULL, NULL, 0,                                             \
            timeout,                                                         \
            socket,                                                          \
            max_size,                                                        \
            min_size,                                                        \
            tag)

/*---------------------------------------------------------------------------
 *  Definitions and prototypes for readr - Repeated read from socket.
 *---------------------------------------------------------------------------*/

extern char *SMTSOCK_READR;

#define declare_smtsock_readr(_event, _priority)                               \
    method_declare (agent, SMTSOCK_READR, _event, _priority)

/*  Send event - readr                                                       */

int 
lsend_smtsock_readr (
    const QID  *_to,
    const QID  *_from,
          char *_accept,
          char *_reject,
          char *_expire,
          word _timeout,
    const dbyte timeout,            /*  Timeout in seconds, zero = none  */
    const qbyte socket,             /*  Socket to read from              */
    const dbyte max_size,           /*  Size of receiving buffer         */
    const dbyte min_size,           /*  Minimum data to read, zero = all  */
    const void *tag);               /*  User-defined request tag         */

#define send_smtsock_readr(_to,                                                \
            timeout,                                                         \
            socket,                                                          \
            max_size,                                                        \
            min_size,                                                        \
            tag)                                                             \
       lsend_smtsock_readr(_to,                                                \
            &thread-> queue-> qid,                                           \
            NULL, NULL, NULL, 0,                                             \
            timeout,                                                         \
            socket,                                                          \
            max_size,                                                        \
            min_size,                                                        \
            tag)


typedef struct {
    dbyte timeout;                      /*  Timeout in seconds, zero = none  */
    qbyte socket;                       /*  Socket to write to               */
    word  size;                         /*  Amount of data to write          */
    void *data;                         /*  Block of data to write           */
    Bool  reply;                        /*  Whether OK reply is required     */
    void *tag;                          /*  User-defined request tag         */
} struct_smtsock_write;

int
put_smtsock_write (
          byte **_buffer,
    const dbyte timeout,                /*  Timeout in seconds, zero = none  */
    const qbyte socket,                 /*  Socket to write to               */
    const word  size,                   /*  Amount of data to write          */
    const void *data,                   /*  Block of data to write           */
    const Bool  reply,                  /*  Whether OK reply is required     */
    const void *tag);                   /*  User-defined request tag         */

int
get_smtsock_write (
    byte *_buffer,
    struct_smtsock_write **params);

void
free_smtsock_write (
    struct_smtsock_write **params);

/*---------------------------------------------------------------------------
 *  Definitions and prototypes for write - Write to socket.
 *---------------------------------------------------------------------------*/

extern char *SMTSOCK_WRITE;

#define declare_smtsock_write(_event, _priority)                               \
    method_declare (agent, SMTSOCK_WRITE, _event, _priority)

/*  Send event - write                                                       */

int 
lsend_smtsock_write (
    const QID  *_to,
    const QID  *_from,
          char *_accept,
          char *_reject,
          char *_expire,
          word _timeout,
    const dbyte timeout,            /*  Timeout in seconds, zero = none  */
    const qbyte socket,             /*  Socket to write to               */
    const word  size,               /*  Amount of data to write          */
    const void *data,               /*  Block of data to write           */
    const Bool  reply,              /*  Whether OK reply is required     */
    const void *tag);               /*  User-defined request tag         */

#define send_smtsock_write(_to,                                                \
            timeout,                                                         \
            socket,                                                          \
            size,                                                            \
            data,                                                            \
            reply,                                                           \
            tag)                                                             \
       lsend_smtsock_write(_to,                                                \
            &thread-> queue-> qid,                                           \
            NULL, NULL, NULL, 0,                                             \
            timeout,                                                         \
            socket,                                                          \
            size,                                                            \
            data,                                                            \
            reply,                                                           \
            tag)


typedef struct {
    dbyte timeout;                      /*  Timeout in seconds, zero = none  */
    qbyte socket;                       /*  Socket to write to               */
    Bool  reply;                        /*  Whether OK reply is required     */
    void *tag;                          /*  User-defined request tag         */
} struct_smtsock_close;

int
put_smtsock_close (
          byte **_buffer,
    const dbyte timeout,                /*  Timeout in seconds, zero = none  */
    const qbyte socket,                 /*  Socket to write to               */
    const Bool  reply,                  /*  Whether OK reply is required     */
    const void *tag);                   /*  User-defined request tag         */

int
get_smtsock_close (
    byte *_buffer,
    struct_smtsock_close **params);

void
free_smtsock_close (
    struct_smtsock_close **params);

/*---------------------------------------------------------------------------
 *  Definitions and prototypes for close - Close socket.
 *---------------------------------------------------------------------------*/

extern char *SMTSOCK_CLOSE;

#define declare_smtsock_close(_event, _priority)                               \
    method_declare (agent, SMTSOCK_CLOSE, _event, _priority)

/*  Send event - close                                                       */

int 
lsend_smtsock_close (
    const QID  *_to,
    const QID  *_from,
          char *_accept,
          char *_reject,
          char *_expire,
          word _timeout,
    const dbyte timeout,            /*  Timeout in seconds, zero = none  */
    const qbyte socket,             /*  Socket to write to               */
    const Bool  reply,              /*  Whether OK reply is required     */
    const void *tag);               /*  User-defined request tag         */

#define send_smtsock_close(_to,                                                \
            timeout,                                                         \
            socket,                                                          \
            reply,                                                           \
            tag)                                                             \
       lsend_smtsock_close(_to,                                                \
            &thread-> queue-> qid,                                           \
            NULL, NULL, NULL, 0,                                             \
            timeout,                                                         \
            socket,                                                          \
            reply,                                                           \
            tag)


typedef struct {
    dbyte timeout;                      /*  Timeout in seconds, zero = none  */
    qbyte socket;                       /*  Socket to read from              */
    qbyte max_size;                     /*  Size of receiving buffer         */
    qbyte min_size;                     /*  Minimum data to read, zero = all  */
    void *tag;                          /*  User-defined request tag         */
} struct_smtsock_readh;

int
put_smtsock_readh (
          byte **_buffer,
    const dbyte timeout,                /*  Timeout in seconds, zero = none  */
    const qbyte socket,                 /*  Socket to read from              */
    const qbyte max_size,               /*  Size of receiving buffer         */
    const qbyte min_size,               /*  Minimum data to read, zero = all  */
    const void *tag);                   /*  User-defined request tag         */

int
get_smtsock_readh (
    byte *_buffer,
    struct_smtsock_readh **params);

void
free_smtsock_readh (
    struct_smtsock_readh **params);

/*---------------------------------------------------------------------------
 *  Definitions and prototypes for readh - Huge read from socket.
 *---------------------------------------------------------------------------*/

extern char *SMTSOCK_READH;

#define declare_smtsock_readh(_event, _priority)                               \
    method_declare (agent, SMTSOCK_READH, _event, _priority)

/*  Send event - readh                                                       */

int 
lsend_smtsock_readh (
    const QID  *_to,
    const QID  *_from,
          char *_accept,
          char *_reject,
          char *_expire,
          word _timeout,
    const dbyte timeout,            /*  Timeout in seconds, zero = none  */
    const qbyte socket,             /*  Socket to read from              */
    const qbyte max_size,           /*  Size of receiving buffer         */
    const qbyte min_size,           /*  Minimum data to read, zero = all  */
    const void *tag);               /*  User-defined request tag         */

#define send_smtsock_readh(_to,                                                \
            timeout,                                                         \
            socket,                                                          \
            max_size,                                                        \
            min_size,                                                        \
            tag)                                                             \
       lsend_smtsock_readh(_to,                                                \
            &thread-> queue-> qid,                                           \
            NULL, NULL, NULL, 0,                                             \
            timeout,                                                         \
            socket,                                                          \
            max_size,                                                        \
            min_size,                                                        \
            tag)

/*---------------------------------------------------------------------------
 *  Definitions and prototypes for readrh - Repeated huge read from socket.
 *---------------------------------------------------------------------------*/

extern char *SMTSOCK_READRH;

#define declare_smtsock_readrh(_event, _priority)                              \
    method_declare (agent, SMTSOCK_READRH, _event, _priority)

/*  Send event - readrh                                                      */

int 
lsend_smtsock_readrh (
    const QID  *_to,
    const QID  *_from,
          char *_accept,
          char *_reject,
          char *_expire,
          word _timeout,
    const dbyte timeout,            /*  Timeout in seconds, zero = none  */
    const qbyte socket,             /*  Socket to read from              */
    const qbyte max_size,           /*  Size of receiving buffer         */
    const qbyte min_size,           /*  Minimum data to read, zero = all  */
    const void *tag);               /*  User-defined request tag         */

#define send_smtsock_readrh(_to,                                               \
            timeout,                                                         \
            socket,                                                          \
            max_size,                                                        \
            min_size,                                                        \
            tag)                                                             \
       lsend_smtsock_readrh(_to,                                               \
            &thread-> queue-> qid,                                           \
            NULL, NULL, NULL, 0,                                             \
            timeout,                                                         \
            socket,                                                          \
            max_size,                                                        \
            min_size,                                                        \
            tag)


typedef struct {
    dbyte timeout;                      /*  Timeout in seconds, zero = none  */
    qbyte socket;                       /*  Socket to write to               */
    qbyte size;                         /*  Amount of data to write          */
    byte *data;                         /*  Block of data to write           */
    Bool  reply;                        /*  Whether OK reply is required     */
    void *tag;                          /*  User-defined request tag         */
} struct_smtsock_writeh;

int
put_smtsock_writeh (
          byte **_buffer,
    const dbyte timeout,                /*  Timeout in seconds, zero = none  */
    const qbyte socket,                 /*  Socket to write to               */
    const qbyte size,                   /*  Amount of data to write          */
    const byte *data,                   /*  Block of data to write           */
    const Bool  reply,                  /*  Whether OK reply is required     */
    const void *tag);                   /*  User-defined request tag         */

int
get_smtsock_writeh (
    byte *_buffer,
    struct_smtsock_writeh **params);

void
free_smtsock_writeh (
    struct_smtsock_writeh **params);

/*---------------------------------------------------------------------------
 *  Definitions and prototypes for writeh - Huge write to socket.
 *---------------------------------------------------------------------------*/

extern char *SMTSOCK_WRITEH;

#define declare_smtsock_writeh(_event, _priority)                              \
    method_declare (agent, SMTSOCK_WRITEH, _event, _priority)

/*  Send event - writeh                                                      */

int 
lsend_smtsock_writeh (
    const QID  *_to,
    const QID  *_from,
          char *_accept,
          char *_reject,
          char *_expire,
          word _timeout,
    const dbyte timeout,            /*  Timeout in seconds, zero = none  */
    const qbyte socket,             /*  Socket to write to               */
    const qbyte size,               /*  Amount of data to write          */
    const byte *data,               /*  Block of data to write           */
    const Bool  reply,              /*  Whether OK reply is required     */
    const void *tag);               /*  User-defined request tag         */

#define send_smtsock_writeh(_to,                                               \
            timeout,                                                         \
            socket,                                                          \
            size,                                                            \
            data,                                                            \
            reply,                                                           \
            tag)                                                             \
       lsend_smtsock_writeh(_to,                                               \
            &thread-> queue-> qid,                                           \
            NULL, NULL, NULL, 0,                                             \
            timeout,                                                         \
            socket,                                                          \
            size,                                                            \
            data,                                                            \
            reply,                                                           \
            tag)


typedef struct {
    dbyte timeout;                      /*  Timeout in seconds, zero = none  */
    qbyte socket;                       /*  Socket to wait on                */
    void *tag;                          /*  User-defined request tag         */
} struct_smtsock_input;

int
put_smtsock_input (
          byte **_buffer,
    const dbyte timeout,                /*  Timeout in seconds, zero = none  */
    const qbyte socket,                 /*  Socket to wait on                */
    const void *tag);                   /*  User-defined request tag         */

int
get_smtsock_input (
    byte *_buffer,
    struct_smtsock_input **params);

void
free_smtsock_input (
    struct_smtsock_input **params);

/*---------------------------------------------------------------------------
 *  Definitions and prototypes for input - Wait for socket input.
 *---------------------------------------------------------------------------*/

extern char *SMTSOCK_INPUT;

#define declare_smtsock_input(_event, _priority)                               \
    method_declare (agent, SMTSOCK_INPUT, _event, _priority)

/*  Send event - input                                                       */

int 
lsend_smtsock_input (
    const QID  *_to,
    const QID  *_from,
          char *_accept,
          char *_reject,
          char *_expire,
          word _timeout,
    const dbyte timeout,            /*  Timeout in seconds, zero = none  */
    const qbyte socket,             /*  Socket to wait on                */
    const void *tag);               /*  User-defined request tag         */

#define send_smtsock_input(_to,                                                \
            timeout,                                                         \
            socket,                                                          \
            tag)                                                             \
       lsend_smtsock_input(_to,                                                \
            &thread-> queue-> qid,                                           \
            NULL, NULL, NULL, 0,                                             \
            timeout,                                                         \
            socket,                                                          \
            tag)


typedef struct {
    dbyte timeout;                      /*  Timeout in seconds, zero = none  */
    qbyte socket;                       /*  Socket to wait on                */
    void *tag;                          /*  User-defined request tag         */
} struct_smtsock_output;

int
put_smtsock_output (
          byte **_buffer,
    const dbyte timeout,                /*  Timeout in seconds, zero = none  */
    const qbyte socket,                 /*  Socket to wait on                */
    const void *tag);                   /*  User-defined request tag         */

int
get_smtsock_output (
    byte *_buffer,
    struct_smtsock_output **params);

void
free_smtsock_output (
    struct_smtsock_output **params);

/*---------------------------------------------------------------------------
 *  Definitions and prototypes for output - Wait for socket output.
 *---------------------------------------------------------------------------*/

extern char *SMTSOCK_OUTPUT;

#define declare_smtsock_output(_event, _priority)                              \
    method_declare (agent, SMTSOCK_OUTPUT, _event, _priority)

/*  Send event - output                                                      */

int 
lsend_smtsock_output (
    const QID  *_to,
    const QID  *_from,
          char *_accept,
          char *_reject,
          char *_expire,
          word _timeout,
    const dbyte timeout,            /*  Timeout in seconds, zero = none  */
    const qbyte socket,             /*  Socket to wait on                */
    const void *tag);               /*  User-defined request tag         */

#define send_smtsock_output(_to,                                               \
            timeout,                                                         \
            socket,                                                          \
            tag)                                                             \
       lsend_smtsock_output(_to,                                               \
            &thread-> queue-> qid,                                           \
            NULL, NULL, NULL, 0,                                             \
            timeout,                                                         \
            socket,                                                          \
            tag)


typedef struct {
    dbyte timeout;                      /*  Timeout in seconds, zero = none  */
    char *type;                         /*  Type, UDP or TCP                 */
    char *host;                         /*  Host, name or dotted address, or NULL  */
    char *service;                      /*  Service, as name or port in ASCII, or NULL  */
    dbyte port;                         /*  16-bit host port, or 0           */
    qbyte address;                      /*  32-bit host address, or 0        */
    void *tag;                          /*  User-defined request tag         */
} struct_smtsock_connect;

int
put_smtsock_connect (
          byte **_buffer,
    const dbyte timeout,                /*  Timeout in seconds, zero = none  */
    const char *type,                   /*  Type, UDP or TCP                 */
    const char *host,                   /*  Host, name or dotted address, or NULL  */
    const char *service,                /*  Service, as name or port in ASCII, or NULL  */
    const dbyte port,                   /*  16-bit host port, or 0           */
    const qbyte address,                /*  32-bit host address, or 0        */
    const void *tag);                   /*  User-defined request tag         */

int
get_smtsock_connect (
    byte *_buffer,
    struct_smtsock_connect **params);

void
free_smtsock_connect (
    struct_smtsock_connect **params);

/*---------------------------------------------------------------------------
 *  Definitions and prototypes for connect - Connect to socket.
 *---------------------------------------------------------------------------*/

extern char *SMTSOCK_CONNECT;

#define declare_smtsock_connect(_event, _priority)                             \
    method_declare (agent, SMTSOCK_CONNECT, _event, _priority)

/*  Send event - connect                                                     */

int 
lsend_smtsock_connect (
    const QID  *_to,
    const QID  *_from,
          char *_accept,
          char *_reject,
          char *_expire,
          word _timeout,
    const dbyte timeout,            /*  Timeout in seconds, zero = none  */
    const char *type,               /*  Type, UDP or TCP                 */
    const char *host,               /*  Host, name or dotted address, or NULL  */
    const char *service,            /*  Service, as name or port in ASCII, or NULL  */
    const dbyte port,               /*  16-bit host port, or 0           */
    const qbyte address,            /*  32-bit host address, or 0        */
    const void *tag);               /*  User-defined request tag         */

#define send_smtsock_connect(_to,                                              \
            timeout,                                                         \
            type,                                                            \
            host,                                                            \
            service,                                                         \
            port,                                                            \
            address,                                                         \
            tag)                                                             \
       lsend_smtsock_connect(_to,                                              \
            &thread-> queue-> qid,                                           \
            NULL, NULL, NULL, 0,                                             \
            timeout,                                                         \
            type,                                                            \
            host,                                                            \
            service,                                                         \
            port,                                                            \
            address,                                                         \
            tag)


typedef struct {
    qbyte socket;                       /*  Socket for operation             */
    Bool  alltypes;                     /*  All request types, or just read?  */
} struct_smtsock_flush;

int
put_smtsock_flush (
          byte **_buffer,
    const qbyte socket,                 /*  Socket for operation             */
    const Bool  alltypes);              /*  All request types, or just read?  */

int
get_smtsock_flush (
    byte *_buffer,
    struct_smtsock_flush **params);

void
free_smtsock_flush (
    struct_smtsock_flush **params);

/*---------------------------------------------------------------------------
 *  Definitions and prototypes for flush - Flush socket.
 *---------------------------------------------------------------------------*/

extern char *SMTSOCK_FLUSH;

#define declare_smtsock_flush(_event, _priority)                               \
    method_declare (agent, SMTSOCK_FLUSH, _event, _priority)

/*  Send event - flush                                                       */

int 
lsend_smtsock_flush (
    const QID  *_to,
    const QID  *_from,
          char *_accept,
          char *_reject,
          char *_expire,
          word _timeout,
    const qbyte socket,             /*  Socket for operation             */
    const Bool  alltypes);          /*  All request types, or just read?  */

#define send_smtsock_flush(_to,                                                \
            socket,                                                          \
            alltypes)                                                        \
       lsend_smtsock_flush(_to,                                                \
            &thread-> queue-> qid,                                           \
            NULL, NULL, NULL, 0,                                             \
            socket,                                                          \
            alltypes)


typedef struct {
    word  size;                         /*  Amount of data read              */
    void *data;                         /*  Block of data read               */
    void *tag;                          /*  User-defined request tag         */
} struct_smtsock_read_reply;

int
put_smtsock_read_reply (
          byte **_buffer,
    const word  size,                   /*  Amount of data read              */
    const void *data,                   /*  Block of data read               */
    const void *tag);                   /*  User-defined request tag         */

int
get_smtsock_read_reply (
    byte *_buffer,
    struct_smtsock_read_reply **params);

void
free_smtsock_read_reply (
    struct_smtsock_read_reply **params);

/*---------------------------------------------------------------------------
 *  Definitions and prototypes for read ok - .
 *---------------------------------------------------------------------------*/

extern char *SMTSOCK_READ_OK;

#define declare_smtsock_read_ok(_event, _priority)                             \
    method_declare (agent, SMTSOCK_READ_OK, _event, _priority)

/*  Send event - read ok                                                     */

int 
lsend_smtsock_read_ok (
    const QID  *_to,
    const QID  *_from,
          char *_accept,
          char *_reject,
          char *_expire,
          word _timeout,
    const word  size,               /*  Amount of data read              */
    const void *data,               /*  Block of data read               */
    const void *tag);               /*  User-defined request tag         */

#define send_smtsock_read_ok(_to,                                              \
            size,                                                            \
            data,                                                            \
            tag)                                                             \
       lsend_smtsock_read_ok(_to,                                              \
            &thread-> queue-> qid,                                           \
            NULL, NULL, NULL, 0,                                             \
            size,                                                            \
            data,                                                            \
            tag)

/*---------------------------------------------------------------------------
 *  Definitions and prototypes for read closed - .
 *---------------------------------------------------------------------------*/

extern char *SMTSOCK_READ_CLOSED;

#define declare_smtsock_read_closed(_event, _priority)                         \
    method_declare (agent, SMTSOCK_READ_CLOSED, _event, _priority)

/*  Send event - read closed                                                 */

int 
lsend_smtsock_read_closed (
    const QID  *_to,
    const QID  *_from,
          char *_accept,
          char *_reject,
          char *_expire,
          word _timeout,
    const word  size,               /*  Amount of data read              */
    const void *data,               /*  Block of data read               */
    const void *tag);               /*  User-defined request tag         */

#define send_smtsock_read_closed(_to,                                          \
            size,                                                            \
            data,                                                            \
            tag)                                                             \
       lsend_smtsock_read_closed(_to,                                          \
            &thread-> queue-> qid,                                           \
            NULL, NULL, NULL, 0,                                             \
            size,                                                            \
            data,                                                            \
            tag)

/*---------------------------------------------------------------------------
 *  Definitions and prototypes for read timeout - .
 *---------------------------------------------------------------------------*/

extern char *SMTSOCK_READ_TIMEOUT;

#define declare_smtsock_read_timeout(_event, _priority)                        \
    method_declare (agent, SMTSOCK_READ_TIMEOUT, _event, _priority)

/*  Send event - read timeout                                                */

int 
lsend_smtsock_read_timeout (
    const QID  *_to,
    const QID  *_from,
          char *_accept,
          char *_reject,
          char *_expire,
          word _timeout,
    const word  size,               /*  Amount of data read              */
    const void *data,               /*  Block of data read               */
    const void *tag);               /*  User-defined request tag         */

#define send_smtsock_read_timeout(_to,                                         \
            size,                                                            \
            data,                                                            \
            tag)                                                             \
       lsend_smtsock_read_timeout(_to,                                         \
            &thread-> queue-> qid,                                           \
            NULL, NULL, NULL, 0,                                             \
            size,                                                            \
            data,                                                            \
            tag)


typedef struct {
    qbyte size;                         /*  Amount of data read              */
    byte *data;                         /*  Block of data read               */
    void *tag;                          /*  User-defined request tag         */
} struct_smtsock_readh_reply;

int
put_smtsock_readh_reply (
          byte **_buffer,
    const qbyte size,                   /*  Amount of data read              */
    const byte *data,                   /*  Block of data read               */
    const void *tag);                   /*  User-defined request tag         */

int
get_smtsock_readh_reply (
    byte *_buffer,
    struct_smtsock_readh_reply **params);

void
free_smtsock_readh_reply (
    struct_smtsock_readh_reply **params);

/*---------------------------------------------------------------------------
 *  Definitions and prototypes for readh ok - .
 *---------------------------------------------------------------------------*/

extern char *SMTSOCK_READH_OK;

#define declare_smtsock_readh_ok(_event, _priority)                            \
    method_declare (agent, SMTSOCK_READH_OK, _event, _priority)

/*  Send event - readh ok                                                    */

int 
lsend_smtsock_readh_ok (
    const QID  *_to,
    const QID  *_from,
          char *_accept,
          char *_reject,
          char *_expire,
          word _timeout,
    const qbyte size,               /*  Amount of data read              */
    const byte *data,               /*  Block of data read               */
    const void *tag);               /*  User-defined request tag         */

#define send_smtsock_readh_ok(_to,                                             \
            size,                                                            \
            data,                                                            \
            tag)                                                             \
       lsend_smtsock_readh_ok(_to,                                             \
            &thread-> queue-> qid,                                           \
            NULL, NULL, NULL, 0,                                             \
            size,                                                            \
            data,                                                            \
            tag)

/*---------------------------------------------------------------------------
 *  Definitions and prototypes for readh closed - .
 *---------------------------------------------------------------------------*/

extern char *SMTSOCK_READH_CLOSED;

#define declare_smtsock_readh_closed(_event, _priority)                        \
    method_declare (agent, SMTSOCK_READH_CLOSED, _event, _priority)

/*  Send event - readh closed                                                */

int 
lsend_smtsock_readh_closed (
    const QID  *_to,
    const QID  *_from,
          char *_accept,
          char *_reject,
          char *_expire,
          word _timeout,
    const qbyte size,               /*  Amount of data read              */
    const byte *data,               /*  Block of data read               */
    const void *tag);               /*  User-defined request tag         */

#define send_smtsock_readh_closed(_to,                                         \
            size,                                                            \
            data,                                                            \
            tag)                                                             \
       lsend_smtsock_readh_closed(_to,                                         \
            &thread-> queue-> qid,                                           \
            NULL, NULL, NULL, 0,                                             \
            size,                                                            \
            data,                                                            \
            tag)

/*---------------------------------------------------------------------------
 *  Definitions and prototypes for readh timeout - .
 *---------------------------------------------------------------------------*/

extern char *SMTSOCK_READH_TIMEOUT;

#define declare_smtsock_readh_timeout(_event, _priority)                       \
    method_declare (agent, SMTSOCK_READH_TIMEOUT, _event, _priority)

/*  Send event - readh timeout                                               */

int 
lsend_smtsock_readh_timeout (
    const QID  *_to,
    const QID  *_from,
          char *_accept,
          char *_reject,
          char *_expire,
          word _timeout,
    const qbyte size,               /*  Amount of data read              */
    const byte *data,               /*  Block of data read               */
    const void *tag);               /*  User-defined request tag         */

#define send_smtsock_readh_timeout(_to,                                        \
            size,                                                            \
            data,                                                            \
            tag)                                                             \
       lsend_smtsock_readh_timeout(_to,                                        \
            &thread-> queue-> qid,                                           \
            NULL, NULL, NULL, 0,                                             \
            size,                                                            \
            data,                                                            \
            tag)


typedef struct {
    qbyte socket;                       /*  New socket                       */
    void *tag;                          /*  User-defined request tag         */
} struct_smtsock_connect_ok;

int
put_smtsock_connect_ok (
          byte **_buffer,
    const qbyte socket,                 /*  New socket                       */
    const void *tag);                   /*  User-defined request tag         */

int
get_smtsock_connect_ok (
    byte *_buffer,
    struct_smtsock_connect_ok **params);

void
free_smtsock_connect_ok (
    struct_smtsock_connect_ok **params);

/*---------------------------------------------------------------------------
 *  Definitions and prototypes for connect ok - .
 *---------------------------------------------------------------------------*/

extern char *SMTSOCK_CONNECT_OK;

#define declare_smtsock_connect_ok(_event, _priority)                          \
    method_declare (agent, SMTSOCK_CONNECT_OK, _event, _priority)

/*  Send event - connect ok                                                  */

int 
lsend_smtsock_connect_ok (
    const QID  *_to,
    const QID  *_from,
          char *_accept,
          char *_reject,
          char *_expire,
          word _timeout,
    const qbyte socket,             /*  New socket                       */
    const void *tag);               /*  User-defined request tag         */

#define send_smtsock_connect_ok(_to,                                           \
            socket,                                                          \
            tag)                                                             \
       lsend_smtsock_connect_ok(_to,                                           \
            &thread-> queue-> qid,                                           \
            NULL, NULL, NULL, 0,                                             \
            socket,                                                          \
            tag)


typedef struct {
    void *tag;                          /*  User-defined request tag         */
} struct_smtsock_reply;

int
put_smtsock_reply (
          byte **_buffer,
    const void *tag);                   /*  User-defined request tag         */

int
get_smtsock_reply (
    byte *_buffer,
    struct_smtsock_reply **params);

void
free_smtsock_reply (
    struct_smtsock_reply **params);

/*---------------------------------------------------------------------------
 *  Definitions and prototypes for ok - .
 *---------------------------------------------------------------------------*/

extern char *SMTSOCK_OK;

#define declare_smtsock_ok(_event, _priority)                                  \
    method_declare (agent, SMTSOCK_OK, _event, _priority)

/*  Send event - ok                                                          */

int 
lsend_smtsock_ok (
    const QID  *_to,
    const QID  *_from,
          char *_accept,
          char *_reject,
          char *_expire,
          word _timeout,
    const void *tag);               /*  User-defined request tag         */

#define send_smtsock_ok(_to,                                                   \
            tag)                                                             \
       lsend_smtsock_ok(_to,                                                   \
            &thread-> queue-> qid,                                           \
            NULL, NULL, NULL, 0,                                             \
            tag)

/*---------------------------------------------------------------------------
 *  Definitions and prototypes for closed - .
 *---------------------------------------------------------------------------*/

extern char *SMTSOCK_CLOSED;

#define declare_smtsock_closed(_event, _priority)                              \
    method_declare (agent, SMTSOCK_CLOSED, _event, _priority)

/*  Send event - closed                                                      */

int 
lsend_smtsock_closed (
    const QID  *_to,
    const QID  *_from,
          char *_accept,
          char *_reject,
          char *_expire,
          word _timeout,
    const void *tag);               /*  User-defined request tag         */

#define send_smtsock_closed(_to,                                               \
            tag)                                                             \
       lsend_smtsock_closed(_to,                                               \
            &thread-> queue-> qid,                                           \
            NULL, NULL, NULL, 0,                                             \
            tag)

/*---------------------------------------------------------------------------
 *  Definitions and prototypes for timeout - .
 *---------------------------------------------------------------------------*/

extern char *SMTSOCK_TIMEOUT;

#define declare_smtsock_timeout(_event, _priority)                             \
    method_declare (agent, SMTSOCK_TIMEOUT, _event, _priority)

/*  Send event - timeout                                                     */

int 
lsend_smtsock_timeout (
    const QID  *_to,
    const QID  *_from,
          char *_accept,
          char *_reject,
          char *_expire,
          word _timeout,
    const void *tag);               /*  User-defined request tag         */

#define send_smtsock_timeout(_to,                                              \
            tag)                                                             \
       lsend_smtsock_timeout(_to,                                              \
            &thread-> queue-> qid,                                           \
            NULL, NULL, NULL, 0,                                             \
            tag)


typedef struct {
    char *message;                      /*  Error message                    */
    void *tag;                          /*  User-defined request tag         */
} struct_smtsock_error;

int
put_smtsock_error (
          byte **_buffer,
    const char *message,                /*  Error message                    */
    const void *tag);                   /*  User-defined request tag         */

int
get_smtsock_error (
    byte *_buffer,
    struct_smtsock_error **params);

void
free_smtsock_error (
    struct_smtsock_error **params);

/*---------------------------------------------------------------------------
 *  Definitions and prototypes for error - .
 *---------------------------------------------------------------------------*/

extern char *SMTSOCK_ERROR;

#define declare_smtsock_error(_event, _priority)                               \
    method_declare (agent, SMTSOCK_ERROR, _event, _priority)

/*  Send event - error                                                       */

int 
lsend_smtsock_error (
    const QID  *_to,
    const QID  *_from,
          char *_accept,
          char *_reject,
          char *_expire,
          word _timeout,
    const char *message,            /*  Error message                    */
    const void *tag);               /*  User-defined request tag         */

#define send_smtsock_error(_to,                                                \
            message,                                                         \
            tag)                                                             \
       lsend_smtsock_error(_to,                                                \
            &thread-> queue-> qid,                                           \
            NULL, NULL, NULL, 0,                                             \
            message,                                                         \
            tag)


/*---------------------------------------------------------------------------
 *  Definitions and prototypes for smttran - Transfer agent.
 *---------------------------------------------------------------------------*/

typedef struct {
    qbyte socket;                       /*  Socket for output                */
    word  size;                         /*  Amount of data to send           */
    void *data;                         /*  Block of data to send            */
    char *pipe;                         /*  Transfer pipe, if any            */
} struct_smttran_putb;

int
put_smttran_putb (
          byte **_buffer,
    const qbyte socket,                 /*  Socket for output                */
    const word  size,                   /*  Amount of data to send           */
    const void *data,                   /*  Block of data to send            */
    const char *pipe);                  /*  Transfer pipe, if any            */

int
get_smttran_putb (
    byte *_buffer,
    struct_smttran_putb **params);

void
free_smttran_putb (
    struct_smttran_putb **params);

/*---------------------------------------------------------------------------
 *  Definitions and prototypes for put block - .
 *---------------------------------------------------------------------------*/

extern char *SMTTRAN_PUT_BLOCK;

#define declare_smttran_put_block(_event, _priority)                           \
    method_declare (agent, SMTTRAN_PUT_BLOCK, _event, _priority)

/*  Send event - put block                                                   */

int 
lsend_smttran_put_block (
    const QID  *_to,
    const QID  *_from,
          char *_accept,
          char *_reject,
          char *_expire,
          word _timeout,
    const qbyte socket,             /*  Socket for output                */
    const word  size,               /*  Amount of data to send           */
    const void *data,               /*  Block of data to send            */
    const char *pipe);              /*  Transfer pipe, if any            */

#define send_smttran_put_block(_to,                                            \
            socket,                                                          \
            size,                                                            \
            data,                                                            \
            pipe)                                                            \
       lsend_smttran_put_block(_to,                                            \
            &thread-> queue-> qid,                                           \
            NULL, NULL, NULL, 0,                                             \
            socket,                                                          \
            size,                                                            \
            data,                                                            \
            pipe)


typedef struct {
    qbyte socket;                       /*  Socket for input                 */
    char *pipe;                         /*  Transfer pipe, if any            */
} struct_smttran_getb;

int
put_smttran_getb (
          byte **_buffer,
    const qbyte socket,                 /*  Socket for input                 */
    const char *pipe);                  /*  Transfer pipe, if any            */

int
get_smttran_getb (
    byte *_buffer,
    struct_smttran_getb **params);

void
free_smttran_getb (
    struct_smttran_getb **params);

/*---------------------------------------------------------------------------
 *  Definitions and prototypes for get block - .
 *---------------------------------------------------------------------------*/

extern char *SMTTRAN_GET_BLOCK;

#define declare_smttran_get_block(_event, _priority)                           \
    method_declare (agent, SMTTRAN_GET_BLOCK, _event, _priority)

/*  Send event - get block                                                   */

int 
lsend_smttran_get_block (
    const QID  *_to,
    const QID  *_from,
          char *_accept,
          char *_reject,
          char *_expire,
          word _timeout,
    const qbyte socket,             /*  Socket for input                 */
    const char *pipe);              /*  Transfer pipe, if any            */

#define send_smttran_get_block(_to,                                            \
            socket,                                                          \
            pipe)                                                            \
       lsend_smttran_get_block(_to,                                            \
            &thread-> queue-> qid,                                           \
            NULL, NULL, NULL, 0,                                             \
            socket,                                                          \
            pipe)


typedef struct {
    qbyte socket;                       /*  Socket for output                */
    qbyte size;                         /*  Amount of data to send           */
    byte *data;                         /*  Block of data to send            */
    char *pipe;                         /*  Transfer pipe, if any            */
} struct_smttran_puth;

int
put_smttran_puth (
          byte **_buffer,
    const qbyte socket,                 /*  Socket for output                */
    const qbyte size,                   /*  Amount of data to send           */
    const byte *data,                   /*  Block of data to send            */
    const char *pipe);                  /*  Transfer pipe, if any            */

int
get_smttran_puth (
    byte *_buffer,
    struct_smttran_puth **params);

void
free_smttran_puth (
    struct_smttran_puth **params);

/*---------------------------------------------------------------------------
 *  Definitions and prototypes for put huge - .
 *---------------------------------------------------------------------------*/

extern char *SMTTRAN_PUT_HUGE;

#define declare_smttran_put_huge(_event, _priority)                            \
    method_declare (agent, SMTTRAN_PUT_HUGE, _event, _priority)

/*  Send event - put huge                                                    */

int 
lsend_smttran_put_huge (
    const QID  *_to,
    const QID  *_from,
          char *_accept,
          char *_reject,
          char *_expire,
          word _timeout,
    const qbyte socket,             /*  Socket for output                */
    const qbyte size,               /*  Amount of data to send           */
    const byte *data,               /*  Block of data to send            */
    const char *pipe);              /*  Transfer pipe, if any            */

#define send_smttran_put_huge(_to,                                             \
            socket,                                                          \
            size,                                                            \
            data,                                                            \
            pipe)                                                            \
       lsend_smttran_put_huge(_to,                                             \
            &thread-> queue-> qid,                                           \
            NULL, NULL, NULL, 0,                                             \
            socket,                                                          \
            size,                                                            \
            data,                                                            \
            pipe)


typedef struct {
    qbyte socket;                       /*  Socket for input                 */
    char *pipe;                         /*  Transfer pipe, if any            */
} struct_smttran_geth;

int
put_smttran_geth (
          byte **_buffer,
    const qbyte socket,                 /*  Socket for input                 */
    const char *pipe);                  /*  Transfer pipe, if any            */

int
get_smttran_geth (
    byte *_buffer,
    struct_smttran_geth **params);

void
free_smttran_geth (
    struct_smttran_geth **params);

/*---------------------------------------------------------------------------
 *  Definitions and prototypes for get huge - .
 *---------------------------------------------------------------------------*/

extern char *SMTTRAN_GET_HUGE;

#define declare_smttran_get_huge(_event, _priority)                            \
    method_declare (agent, SMTTRAN_GET_HUGE, _event, _priority)

/*  Send event - get huge                                                    */

int 
lsend_smttran_get_huge (
    const QID  *_to,
    const QID  *_from,
          char *_accept,
          char *_reject,
          char *_expire,
          word _timeout,
    const qbyte socket,             /*  Socket for input                 */
    const char *pipe);              /*  Transfer pipe, if any            */

#define send_smttran_get_huge(_to,                                             \
            socket,                                                          \
            pipe)                                                            \
       lsend_smttran_get_huge(_to,                                             \
            &thread-> queue-> qid,                                           \
            NULL, NULL, NULL, 0,                                             \
            socket,                                                          \
            pipe)


typedef struct {
    qbyte socket;                       /*  Socket for output                */
    char *filename;                     /*  Name of file to send             */
    dbyte filetype;                     /*  0=binary, 1=ASCII                */
    qbyte start;                        /*  Starting offset; 0 = start       */
    qbyte end;                          /*  Ending offset; 0 = end           */
    char *pipe;                         /*  Transfer pipe, if any            */
} struct_smttran_putf;

int
put_smttran_putf (
          byte **_buffer,
    const qbyte socket,                 /*  Socket for output                */
    const char *filename,               /*  Name of file to send             */
    const dbyte filetype,               /*  0=binary, 1=ASCII                */
    const qbyte start,                  /*  Starting offset; 0 = start       */
    const qbyte end,                    /*  Ending offset; 0 = end           */
    const char *pipe);                  /*  Transfer pipe, if any            */

int
get_smttran_putf (
    byte *_buffer,
    struct_smttran_putf **params);

void
free_smttran_putf (
    struct_smttran_putf **params);

/*---------------------------------------------------------------------------
 *  Definitions and prototypes for put file - .
 *---------------------------------------------------------------------------*/

extern char *SMTTRAN_PUT_FILE;

#define declare_smttran_put_file(_event, _priority)                            \
    method_declare (agent, SMTTRAN_PUT_FILE, _event, _priority)

/*  Send event - put file                                                    */

int 
lsend_smttran_put_file (
    const QID  *_to,
    const QID  *_from,
          char *_accept,
          char *_reject,
          char *_expire,
          word _timeout,
    const qbyte socket,             /*  Socket for output                */
    const char *filename,           /*  Name of file to send             */
    const dbyte filetype,           /*  0=binary, 1=ASCII                */
    const qbyte start,              /*  Starting offset; 0 = start       */
    const qbyte end,                /*  Ending offset; 0 = end           */
    const char *pipe);              /*  Transfer pipe, if any            */

#define send_smttran_put_file(_to,                                             \
            socket,                                                          \
            filename,                                                        \
            filetype,                                                        \
            start,                                                           \
            end,                                                             \
            pipe)                                                            \
       lsend_smttran_put_file(_to,                                             \
            &thread-> queue-> qid,                                           \
            NULL, NULL, NULL, 0,                                             \
            socket,                                                          \
            filename,                                                        \
            filetype,                                                        \
            start,                                                           \
            end,                                                             \
            pipe)


typedef struct {
    qbyte socket;                       /*  Socket for input                 */
    char *filename;                     /*  Name of file to receive          */
    dbyte filetype;                     /*  0=binary, 1=ASCII                */
    qbyte start;                        /*  Starting offset; 0 = start       */
    qbyte end;                          /*  Ending offset; 0 = end           */
    Bool  append;                       /*  1 = append existing              */
    qbyte maxsize;                      /*  Max. size, -1 = no limit         */
    char *pipe;                         /*  Transfer pipe, if any            */
} struct_smttran_getf;

int
put_smttran_getf (
          byte **_buffer,
    const qbyte socket,                 /*  Socket for input                 */
    const char *filename,               /*  Name of file to receive          */
    const dbyte filetype,               /*  0=binary, 1=ASCII                */
    const qbyte start,                  /*  Starting offset; 0 = start       */
    const qbyte end,                    /*  Ending offset; 0 = end           */
    const Bool  append,                 /*  1 = append existing              */
    const qbyte maxsize,                /*  Max. size, -1 = no limit         */
    const char *pipe);                  /*  Transfer pipe, if any            */

int
get_smttran_getf (
    byte *_buffer,
    struct_smttran_getf **params);

void
free_smttran_getf (
    struct_smttran_getf **params);

/*---------------------------------------------------------------------------
 *  Definitions and prototypes for get file - .
 *---------------------------------------------------------------------------*/

extern char *SMTTRAN_GET_FILE;

#define declare_smttran_get_file(_event, _priority)                            \
    method_declare (agent, SMTTRAN_GET_FILE, _event, _priority)

/*  Send event - get file                                                    */

int 
lsend_smttran_get_file (
    const QID  *_to,
    const QID  *_from,
          char *_accept,
          char *_reject,
          char *_expire,
          word _timeout,
    const qbyte socket,             /*  Socket for input                 */
    const char *filename,           /*  Name of file to receive          */
    const dbyte filetype,           /*  0=binary, 1=ASCII                */
    const qbyte start,              /*  Starting offset; 0 = start       */
    const qbyte end,                /*  Ending offset; 0 = end           */
    const Bool  append,             /*  1 = append existing              */
    const qbyte maxsize,            /*  Max. size, -1 = no limit         */
    const char *pipe);              /*  Transfer pipe, if any            */

#define send_smttran_get_file(_to,                                             \
            socket,                                                          \
            filename,                                                        \
            filetype,                                                        \
            start,                                                           \
            end,                                                             \
            append,                                                          \
            maxsize,                                                         \
            pipe)                                                            \
       lsend_smttran_get_file(_to,                                             \
            &thread-> queue-> qid,                                           \
            NULL, NULL, NULL, 0,                                             \
            socket,                                                          \
            filename,                                                        \
            filetype,                                                        \
            start,                                                           \
            end,                                                             \
            append,                                                          \
            maxsize,                                                         \
            pipe)


typedef struct {
    dbyte size;                         /*  Amount of transmitted data       */
} struct_smttran_putb_ok;

int
put_smttran_putb_ok (
          byte **_buffer,
    const dbyte size);                  /*  Amount of transmitted data       */

int
get_smttran_putb_ok (
    byte *_buffer,
    struct_smttran_putb_ok **params);

void
free_smttran_putb_ok (
    struct_smttran_putb_ok **params);

/*---------------------------------------------------------------------------
 *  Definitions and prototypes for putb ok - .
 *---------------------------------------------------------------------------*/

extern char *SMTTRAN_PUTB_OK;

#define declare_smttran_putb_ok(_event, _priority)                             \
    method_declare (agent, SMTTRAN_PUTB_OK, _event, _priority)

/*  Send event - putb ok                                                     */

int 
lsend_smttran_putb_ok (
    const QID  *_to,
    const QID  *_from,
          char *_accept,
          char *_reject,
          char *_expire,
          word _timeout,
    const dbyte size);              /*  Amount of transmitted data       */

#define send_smttran_putb_ok(_to,                                              \
            size)                                                            \
       lsend_smttran_putb_ok(_to,                                              \
            &thread-> queue-> qid,                                           \
            NULL, NULL, NULL, 0,                                             \
            size)


typedef struct {
    word  size;                         /*  Amount of data received          */
    void *data;                         /*  Block of data received           */
} struct_smttran_getb_ok;

int
put_smttran_getb_ok (
          byte **_buffer,
    const word  size,                   /*  Amount of data received          */
    const void *data);                  /*  Block of data received           */

int
get_smttran_getb_ok (
    byte *_buffer,
    struct_smttran_getb_ok **params);

void
free_smttran_getb_ok (
    struct_smttran_getb_ok **params);

/*---------------------------------------------------------------------------
 *  Definitions and prototypes for getb ok - .
 *---------------------------------------------------------------------------*/

extern char *SMTTRAN_GETB_OK;

#define declare_smttran_getb_ok(_event, _priority)                             \
    method_declare (agent, SMTTRAN_GETB_OK, _event, _priority)

/*  Send event - getb ok                                                     */

int 
lsend_smttran_getb_ok (
    const QID  *_to,
    const QID  *_from,
          char *_accept,
          char *_reject,
          char *_expire,
          word _timeout,
    const word  size,               /*  Amount of data received          */
    const void *data);              /*  Block of data received           */

#define send_smttran_getb_ok(_to,                                              \
            size,                                                            \
            data)                                                            \
       lsend_smttran_getb_ok(_to,                                              \
            &thread-> queue-> qid,                                           \
            NULL, NULL, NULL, 0,                                             \
            size,                                                            \
            data)


typedef struct {
    qbyte size;                         /*  Amount of transmitted data       */
} struct_smttran_puth_ok;

int
put_smttran_puth_ok (
          byte **_buffer,
    const qbyte size);                  /*  Amount of transmitted data       */

int
get_smttran_puth_ok (
    byte *_buffer,
    struct_smttran_puth_ok **params);

void
free_smttran_puth_ok (
    struct_smttran_puth_ok **params);

/*---------------------------------------------------------------------------
 *  Definitions and prototypes for puth ok - .
 *---------------------------------------------------------------------------*/

extern char *SMTTRAN_PUTH_OK;

#define declare_smttran_puth_ok(_event, _priority)                             \
    method_declare (agent, SMTTRAN_PUTH_OK, _event, _priority)

/*  Send event - puth ok                                                     */

int 
lsend_smttran_puth_ok (
    const QID  *_to,
    const QID  *_from,
          char *_accept,
          char *_reject,
          char *_expire,
          word _timeout,
    const qbyte size);              /*  Amount of transmitted data       */

#define send_smttran_puth_ok(_to,                                              \
            size)                                                            \
       lsend_smttran_puth_ok(_to,                                              \
            &thread-> queue-> qid,                                           \
            NULL, NULL, NULL, 0,                                             \
            size)


typedef struct {
    qbyte size;                         /*  Amount of data received          */
    byte *data;                         /*  Block of data received           */
} struct_smttran_geth_ok;

int
put_smttran_geth_ok (
          byte **_buffer,
    const qbyte size,                   /*  Amount of data received          */
    const byte *data);                  /*  Block of data received           */

int
get_smttran_geth_ok (
    byte *_buffer,
    struct_smttran_geth_ok **params);

void
free_smttran_geth_ok (
    struct_smttran_geth_ok **params);

/*---------------------------------------------------------------------------
 *  Definitions and prototypes for geth ok - .
 *---------------------------------------------------------------------------*/

extern char *SMTTRAN_GETH_OK;

#define declare_smttran_geth_ok(_event, _priority)                             \
    method_declare (agent, SMTTRAN_GETH_OK, _event, _priority)

/*  Send event - geth ok                                                     */

int 
lsend_smttran_geth_ok (
    const QID  *_to,
    const QID  *_from,
          char *_accept,
          char *_reject,
          char *_expire,
          word _timeout,
    const qbyte size,               /*  Amount of data received          */
    const byte *data);              /*  Block of data received           */

#define send_smttran_geth_ok(_to,                                              \
            size,                                                            \
            data)                                                            \
       lsend_smttran_geth_ok(_to,                                              \
            &thread-> queue-> qid,                                           \
            NULL, NULL, NULL, 0,                                             \
            size,                                                            \
            data)


typedef struct {
    qbyte size;                         /*  Amount of transmitted data       */
} struct_smttran_putf_ok;

int
put_smttran_putf_ok (
          byte **_buffer,
    const qbyte size);                  /*  Amount of transmitted data       */

int
get_smttran_putf_ok (
    byte *_buffer,
    struct_smttran_putf_ok **params);

void
free_smttran_putf_ok (
    struct_smttran_putf_ok **params);

/*---------------------------------------------------------------------------
 *  Definitions and prototypes for putf ok - .
 *---------------------------------------------------------------------------*/

extern char *SMTTRAN_PUTF_OK;

#define declare_smttran_putf_ok(_event, _priority)                             \
    method_declare (agent, SMTTRAN_PUTF_OK, _event, _priority)

/*  Send event - putf ok                                                     */

int 
lsend_smttran_putf_ok (
    const QID  *_to,
    const QID  *_from,
          char *_accept,
          char *_reject,
          char *_expire,
          word _timeout,
    const qbyte size);              /*  Amount of transmitted data       */

#define send_smttran_putf_ok(_to,                                              \
            size)                                                            \
       lsend_smttran_putf_ok(_to,                                              \
            &thread-> queue-> qid,                                           \
            NULL, NULL, NULL, 0,                                             \
            size)


typedef struct {
    qbyte size;                         /*  Amount of transmitted data       */
} struct_smttran_getf_ok;

int
put_smttran_getf_ok (
          byte **_buffer,
    const qbyte size);                  /*  Amount of transmitted data       */

int
get_smttran_getf_ok (
    byte *_buffer,
    struct_smttran_getf_ok **params);

void
free_smttran_getf_ok (
    struct_smttran_getf_ok **params);

/*---------------------------------------------------------------------------
 *  Definitions and prototypes for getf ok - .
 *---------------------------------------------------------------------------*/

extern char *SMTTRAN_GETF_OK;

#define declare_smttran_getf_ok(_event, _priority)                             \
    method_declare (agent, SMTTRAN_GETF_OK, _event, _priority)

/*  Send event - getf ok                                                     */

int 
lsend_smttran_getf_ok (
    const QID  *_to,
    const QID  *_from,
          char *_accept,
          char *_reject,
          char *_expire,
          word _timeout,
    const qbyte size);              /*  Amount of transmitted data       */

#define send_smttran_getf_ok(_to,                                              \
            size)                                                            \
       lsend_smttran_getf_ok(_to,                                              \
            &thread-> queue-> qid,                                           \
            NULL, NULL, NULL, 0,                                             \
            size)


typedef struct {
    char *name;                         /*  Name of pipe                     */
    qbyte input_rate;                   /*  Input rate, bytes/s              */
    qbyte output_rate;                  /*  Output rate, bytes/s             */
} struct_smttran_pipe_create;

int
put_smttran_pipe_create (
          byte **_buffer,
    const char *name,                   /*  Name of pipe                     */
    const qbyte input_rate,             /*  Input rate, bytes/s              */
    const qbyte output_rate);           /*  Output rate, bytes/s             */

int
get_smttran_pipe_create (
    byte *_buffer,
    struct_smttran_pipe_create **params);

void
free_smttran_pipe_create (
    struct_smttran_pipe_create **params);

/*---------------------------------------------------------------------------
 *  Definitions and prototypes for pipe create - .
 *---------------------------------------------------------------------------*/

extern char *SMTTRAN_PIPE_CREATE;

#define declare_smttran_pipe_create(_event, _priority)                         \
    method_declare (agent, SMTTRAN_PIPE_CREATE, _event, _priority)

/*  Send event - pipe create                                                 */

int 
lsend_smttran_pipe_create (
    const QID  *_to,
    const QID  *_from,
          char *_accept,
          char *_reject,
          char *_expire,
          word _timeout,
    const char *name,               /*  Name of pipe                     */
    const qbyte input_rate,         /*  Input rate, bytes/s              */
    const qbyte output_rate);       /*  Output rate, bytes/s             */

#define send_smttran_pipe_create(_to,                                          \
            name,                                                            \
            input_rate,                                                      \
            output_rate)                                                     \
       lsend_smttran_pipe_create(_to,                                          \
            &thread-> queue-> qid,                                           \
            NULL, NULL, NULL, 0,                                             \
            name,                                                            \
            input_rate,                                                      \
            output_rate)


/*---------------------------------------------------------------------------
 *  Definitions and prototypes for clear pipes - .
 *---------------------------------------------------------------------------*/

extern char *SMTTRAN_CLEAR_PIPES;

#define declare_smttran_clear_pipes(_event, _priority)                         \
    method_declare (agent, SMTTRAN_CLEAR_PIPES, _event, _priority)

/*  Send event - clear pipes                                                 */

#define lsend_smttran_clear_pipes(_to, _from,                                  \
    _accept, _reject, _expire, _timeout)                                     \
        event_send (_to,                                                     \
                    _from,                                                   \
                    SMTTRAN_CLEAR_PIPES,                                       \
                    NULL, 0,                                                 \
                    _accept, _reject, _expire, _timeout)
#define send_smttran_clear_pipes(_to)                                          \
        event_send (_to,                                                     \
                    &thread-> queue-> qid,                                   \
                    SMTTRAN_CLEAR_PIPES,                                       \
                    NULL, 0,                                                 \
                    NULL, NULL, NULL, 0)


/*---------------------------------------------------------------------------
 *  Definitions and prototypes for commit - .
 *---------------------------------------------------------------------------*/

extern char *SMTTRAN_COMMIT;

#define declare_smttran_commit(_event, _priority)                              \
    method_declare (agent, SMTTRAN_COMMIT, _event, _priority)

/*  Send event - commit                                                      */

#define lsend_smttran_commit(_to, _from,                                       \
    _accept, _reject, _expire, _timeout)                                     \
        event_send (_to,                                                     \
                    _from,                                                   \
                    SMTTRAN_COMMIT,                                            \
                    NULL, 0,                                                 \
                    _accept, _reject, _expire, _timeout)
#define send_smttran_commit(_to)                                               \
        event_send (_to,                                                     \
                    &thread-> queue-> qid,                                   \
                    SMTTRAN_COMMIT,                                            \
                    NULL, 0,                                                 \
                    NULL, NULL, NULL, 0)


/*---------------------------------------------------------------------------
 *  Definitions and prototypes for closed - .
 *---------------------------------------------------------------------------*/

extern char *SMTTRAN_CLOSED;

#define declare_smttran_closed(_event, _priority)                              \
    method_declare (agent, SMTTRAN_CLOSED, _event, _priority)

/*  Send event - closed                                                      */

#define lsend_smttran_closed(_to, _from,                                       \
    _accept, _reject, _expire, _timeout)                                     \
        event_send (_to,                                                     \
                    _from,                                                   \
                    SMTTRAN_CLOSED,                                            \
                    NULL, 0,                                                 \
                    _accept, _reject, _expire, _timeout)
#define send_smttran_closed(_to)                                               \
        event_send (_to,                                                     \
                    &thread-> queue-> qid,                                   \
                    SMTTRAN_CLOSED,                                            \
                    NULL, 0,                                                 \
                    NULL, NULL, NULL, 0)


typedef struct {
    char *reason;                       /*  Error message                    */
} struct_smttran_error;

int
put_smttran_error (
          byte **_buffer,
    const char *reason);                /*  Error message                    */

int
get_smttran_error (
    byte *_buffer,
    struct_smttran_error **params);

void
free_smttran_error (
    struct_smttran_error **params);

/*---------------------------------------------------------------------------
 *  Definitions and prototypes for error - .
 *---------------------------------------------------------------------------*/

extern char *SMTTRAN_ERROR;

#define declare_smttran_error(_event, _priority)                               \
    method_declare (agent, SMTTRAN_ERROR, _event, _priority)

/*  Send event - error                                                       */

int 
lsend_smttran_error (
    const QID  *_to,
    const QID  *_from,
          char *_accept,
          char *_reject,
          char *_expire,
          word _timeout,
    const char *reason);            /*  Error message                    */

#define send_smttran_error(_to,                                                \
            reason)                                                          \
       lsend_smttran_error(_to,                                                \
            &thread-> queue-> qid,                                           \
            NULL, NULL, NULL, 0,                                             \
            reason)


/*---------------------------------------------------------------------------
 *  Definitions and prototypes for smtupm - Unattended Process Monitor agent.
 *---------------------------------------------------------------------------*/

typedef struct {
    dbyte ident;                        /*                                   */
    char *string;                       /*                                   */
} struct_smtupm_message;

int
put_smtupm_message (
          byte **_buffer,
    const dbyte ident,                  /*                                   */
    const char *string);                /*                                   */

int
get_smtupm_message (
    byte *_buffer,
    struct_smtupm_message **params);

void
free_smtupm_message (
    struct_smtupm_message **params);

/*---------------------------------------------------------------------------
 *  Definitions and prototypes for put block - .
 *---------------------------------------------------------------------------*/

extern char *SMTUPM_PUT_BLOCK;

#define declare_smtupm_put_block(_event, _priority)                            \
    method_declare (agent, SMTUPM_PUT_BLOCK, _event, _priority)

/*  Send event - put block                                                   */

int 
lsend_smtupm_put_block (
    const QID  *_to,
    const QID  *_from,
          char *_accept,
          char *_reject,
          char *_expire,
          word _timeout,
    const dbyte ident,              /*                                   */
    const char *string);            /*                                   */

#define send_smtupm_put_block(_to,                                             \
            ident,                                                           \
            string)                                                          \
       lsend_smtupm_put_block(_to,                                             \
            &thread-> queue-> qid,                                           \
            NULL, NULL, NULL, 0,                                             \
            ident,                                                           \
            string)


/*---------------------------------------------------------------------------
 *  Definitions and prototypes for smtpop - POP3 agent.
 *---------------------------------------------------------------------------*/

typedef struct {
    char *server;                       /*  pop3 server name                 */
    char *user;                         /*  user name                        */
    char *password;                     /*  user password                    */
} struct_smtpop_connection;

int
put_smtpop_connection (
          byte **_buffer,
    const char *server,                 /*  pop3 server name                 */
    const char *user,                   /*  user name                        */
    const char *password);              /*  user password                    */

int
get_smtpop_connection (
    byte *_buffer,
    struct_smtpop_connection **params);

void
free_smtpop_connection (
    struct_smtpop_connection **params);

/*---------------------------------------------------------------------------
 *  Definitions and prototypes for connect - .
 *---------------------------------------------------------------------------*/

extern char *SMTPOP_CONNECT;

#define declare_smtpop_connect(_event, _priority)                              \
    method_declare (agent, SMTPOP_CONNECT, _event, _priority)

/*  Send event - connect                                                     */

int 
lsend_smtpop_connect (
    const QID  *_to,
    const QID  *_from,
          char *_accept,
          char *_reject,
          char *_expire,
          word _timeout,
    const char *server,             /*  pop3 server name                 */
    const char *user,               /*  user name                        */
    const char *password);          /*  user password                    */

#define send_smtpop_connect(_to,                                               \
            server,                                                          \
            user,                                                            \
            password)                                                        \
       lsend_smtpop_connect(_to,                                               \
            &thread-> queue-> qid,                                           \
            NULL, NULL, NULL, 0,                                             \
            server,                                                          \
            user,                                                            \
            password)


typedef struct {
    qbyte msg_cnt;                      /*  count of new messages on server  */
    qbyte msg_size;                     /*  messages total size (bytes)      */
} struct_smtpop_connect_ok;

int
put_smtpop_connect_ok (
          byte **_buffer,
    const qbyte msg_cnt,                /*  count of new messages on server  */
    const qbyte msg_size);              /*  messages total size (bytes)      */

int
get_smtpop_connect_ok (
    byte *_buffer,
    struct_smtpop_connect_ok **params);

void
free_smtpop_connect_ok (
    struct_smtpop_connect_ok **params);

/*---------------------------------------------------------------------------
 *  Definitions and prototypes for connect_ok - .
 *---------------------------------------------------------------------------*/

extern char *SMTPOP_CONNECT_OK;

#define declare_smtpop_connect_ok(_event, _priority)                           \
    method_declare (agent, SMTPOP_CONNECT_OK, _event, _priority)

/*  Send event - connect_ok                                                  */

int 
lsend_smtpop_connect_ok (
    const QID  *_to,
    const QID  *_from,
          char *_accept,
          char *_reject,
          char *_expire,
          word _timeout,
    const qbyte msg_cnt,            /*  count of new messages on server  */
    const qbyte msg_size);          /*  messages total size (bytes)      */

#define send_smtpop_connect_ok(_to,                                            \
            msg_cnt,                                                         \
            msg_size)                                                        \
       lsend_smtpop_connect_ok(_to,                                            \
            &thread-> queue-> qid,                                           \
            NULL, NULL, NULL, 0,                                             \
            msg_cnt,                                                         \
            msg_size)


typedef struct {
    char *reason;                       /*  why connection failed            */
    dbyte code;                         /*                                   */
} struct_smtpop_error;

int
put_smtpop_error (
          byte **_buffer,
    const char *reason,                 /*  why connection failed            */
    const dbyte code);                  /*                                   */

int
get_smtpop_error (
    byte *_buffer,
    struct_smtpop_error **params);

void
free_smtpop_error (
    struct_smtpop_error **params);

/*---------------------------------------------------------------------------
 *  Definitions and prototypes for error - .
 *---------------------------------------------------------------------------*/

extern char *SMTPOP_ERROR;

#define declare_smtpop_error(_event, _priority)                                \
    method_declare (agent, SMTPOP_ERROR, _event, _priority)

/*  Send event - error                                                       */

int 
lsend_smtpop_error (
    const QID  *_to,
    const QID  *_from,
          char *_accept,
          char *_reject,
          char *_expire,
          word _timeout,
    const char *reason,             /*  why connection failed            */
    const dbyte code);              /*                                   */

#define send_smtpop_error(_to,                                                 \
            reason,                                                          \
            code)                                                            \
       lsend_smtpop_error(_to,                                                 \
            &thread-> queue-> qid,                                           \
            NULL, NULL, NULL, 0,                                             \
            reason,                                                          \
            code)


/*---------------------------------------------------------------------------
 *  Definitions and prototypes for get_session_info - ask for message count on server.
 *---------------------------------------------------------------------------*/

extern char *SMTPOP_GET_SESSION_INFO;

#define declare_smtpop_get_session_info(_event, _priority)                     \
    method_declare (agent, SMTPOP_GET_SESSION_INFO, _event, _priority)

/*  Send event - get_session_info                                            */

#define lsend_smtpop_get_session_info(_to, _from,                              \
    _accept, _reject, _expire, _timeout)                                     \
        event_send (_to,                                                     \
                    _from,                                                   \
                    SMTPOP_GET_SESSION_INFO,                                   \
                    NULL, 0,                                                 \
                    _accept, _reject, _expire, _timeout)
#define send_smtpop_get_session_info(_to)                                      \
        event_send (_to,                                                     \
                    &thread-> queue-> qid,                                   \
                    SMTPOP_GET_SESSION_INFO,                                   \
                    NULL, 0,                                                 \
                    NULL, NULL, NULL, 0)

/*---------------------------------------------------------------------------
 *  Definitions and prototypes for quit - quit command.
 *---------------------------------------------------------------------------*/

extern char *SMTPOP_QUIT;

#define declare_smtpop_quit(_event, _priority)                                 \
    method_declare (agent, SMTPOP_QUIT, _event, _priority)

/*  Send event - quit                                                        */

#define lsend_smtpop_quit(_to, _from,                                          \
    _accept, _reject, _expire, _timeout)                                     \
        event_send (_to,                                                     \
                    _from,                                                   \
                    SMTPOP_QUIT,                                               \
                    NULL, 0,                                                 \
                    _accept, _reject, _expire, _timeout)
#define send_smtpop_quit(_to)                                                  \
        event_send (_to,                                                     \
                    &thread-> queue-> qid,                                   \
                    SMTPOP_QUIT,                                               \
                    NULL, 0,                                                 \
                    NULL, NULL, NULL, 0)

/*---------------------------------------------------------------------------
 *  Definitions and prototypes for quit_ok - quit response.
 *---------------------------------------------------------------------------*/

extern char *SMTPOP_QUIT_OK;

#define declare_smtpop_quit_ok(_event, _priority)                              \
    method_declare (agent, SMTPOP_QUIT_OK, _event, _priority)

/*  Send event - quit_ok                                                     */

#define lsend_smtpop_quit_ok(_to, _from,                                       \
    _accept, _reject, _expire, _timeout)                                     \
        event_send (_to,                                                     \
                    _from,                                                   \
                    SMTPOP_QUIT_OK,                                            \
                    NULL, 0,                                                 \
                    _accept, _reject, _expire, _timeout)
#define send_smtpop_quit_ok(_to)                                               \
        event_send (_to,                                                     \
                    &thread-> queue-> qid,                                   \
                    SMTPOP_QUIT_OK,                                            \
                    NULL, 0,                                                 \
                    NULL, NULL, NULL, 0)


typedef struct {
    qbyte count;                        /*  message count                    */
    qbyte size;                         /*  messages total size in bytes     */
} struct_smtpop_session_info;

int
put_smtpop_session_info (
          byte **_buffer,
    const qbyte count,                  /*  message count                    */
    const qbyte size);                  /*  messages total size in bytes     */

int
get_smtpop_session_info (
    byte *_buffer,
    struct_smtpop_session_info **params);

void
free_smtpop_session_info (
    struct_smtpop_session_info **params);

/*---------------------------------------------------------------------------
 *  Definitions and prototypes for session_info - .
 *---------------------------------------------------------------------------*/

extern char *SMTPOP_SESSION_INFO;

#define declare_smtpop_session_info(_event, _priority)                         \
    method_declare (agent, SMTPOP_SESSION_INFO, _event, _priority)

/*  Send event - session_info                                                */

int 
lsend_smtpop_session_info (
    const QID  *_to,
    const QID  *_from,
          char *_accept,
          char *_reject,
          char *_expire,
          word _timeout,
    const qbyte count,              /*  message count                    */
    const qbyte size);              /*  messages total size in bytes     */

#define send_smtpop_session_info(_to,                                          \
            count,                                                           \
            size)                                                            \
       lsend_smtpop_session_info(_to,                                          \
            &thread-> queue-> qid,                                           \
            NULL, NULL, NULL, 0,                                             \
            count,                                                           \
            size)


typedef struct {
    qbyte msg_id;                       /*  requested message id             */
    qbyte size;                         /*  messages total size in bytes     */
} struct_smtpop_msg_session_info;

int
put_smtpop_msg_session_info (
          byte **_buffer,
    const qbyte msg_id,                 /*  requested message id             */
    const qbyte size);                  /*  messages total size in bytes     */

int
get_smtpop_msg_session_info (
    byte *_buffer,
    struct_smtpop_msg_session_info **params);

void
free_smtpop_msg_session_info (
    struct_smtpop_msg_session_info **params);

/*---------------------------------------------------------------------------
 *  Definitions and prototypes for msg_session_info - .
 *---------------------------------------------------------------------------*/

extern char *SMTPOP_MSG_SESSION_INFO;

#define declare_smtpop_msg_session_info(_event, _priority)                     \
    method_declare (agent, SMTPOP_MSG_SESSION_INFO, _event, _priority)

/*  Send event - msg_session_info                                            */

int 
lsend_smtpop_msg_session_info (
    const QID  *_to,
    const QID  *_from,
          char *_accept,
          char *_reject,
          char *_expire,
          word _timeout,
    const qbyte msg_id,             /*  requested message id             */
    const qbyte size);              /*  messages total size in bytes     */

#define send_smtpop_msg_session_info(_to,                                      \
            msg_id,                                                          \
            size)                                                            \
       lsend_smtpop_msg_session_info(_to,                                      \
            &thread-> queue-> qid,                                           \
            NULL, NULL, NULL, 0,                                             \
            msg_id,                                                          \
            size)


typedef struct {
    qbyte msg_id;                       /*  message id, zero=all             */
    char *attach_dir;                   /*  directory where attchment will be stored  */
} struct_smtpop_msg_id;

int
put_smtpop_msg_id (
          byte **_buffer,
    const qbyte msg_id,                 /*  message id, zero=all             */
    const char *attach_dir);            /*  directory where attchment will be stored  */

int
get_smtpop_msg_id (
    byte *_buffer,
    struct_smtpop_msg_id **params);

void
free_smtpop_msg_id (
    struct_smtpop_msg_id **params);

/*---------------------------------------------------------------------------
 *  Definitions and prototypes for get_msg_header - ask for message header.
 *---------------------------------------------------------------------------*/

extern char *SMTPOP_GET_MSG_HEADER;

#define declare_smtpop_get_msg_header(_event, _priority)                       \
    method_declare (agent, SMTPOP_GET_MSG_HEADER, _event, _priority)

/*  Send event - get_msg_header                                              */

int 
lsend_smtpop_get_msg_header (
    const QID  *_to,
    const QID  *_from,
          char *_accept,
          char *_reject,
          char *_expire,
          word _timeout,
    const qbyte msg_id,             /*  message id, zero=all             */
    const char *attach_dir);        /*  directory where attchment will be stored  */

#define send_smtpop_get_msg_header(_to,                                        \
            msg_id,                                                          \
            attach_dir)                                                      \
       lsend_smtpop_get_msg_header(_to,                                        \
            &thread-> queue-> qid,                                           \
            NULL, NULL, NULL, 0,                                             \
            msg_id,                                                          \
            attach_dir)

/*---------------------------------------------------------------------------
 *  Definitions and prototypes for get_msg - ask for entire message.
 *---------------------------------------------------------------------------*/

extern char *SMTPOP_GET_MSG;

#define declare_smtpop_get_msg(_event, _priority)                              \
    method_declare (agent, SMTPOP_GET_MSG, _event, _priority)

/*  Send event - get_msg                                                     */

int 
lsend_smtpop_get_msg (
    const QID  *_to,
    const QID  *_from,
          char *_accept,
          char *_reject,
          char *_expire,
          word _timeout,
    const qbyte msg_id,             /*  message id, zero=all             */
    const char *attach_dir);        /*  directory where attchment will be stored  */

#define send_smtpop_get_msg(_to,                                               \
            msg_id,                                                          \
            attach_dir)                                                      \
       lsend_smtpop_get_msg(_to,                                               \
            &thread-> queue-> qid,                                           \
            NULL, NULL, NULL, 0,                                             \
            msg_id,                                                          \
            attach_dir)

/*---------------------------------------------------------------------------
 *  Definitions and prototypes for delete_msg - delete message from server.
 *---------------------------------------------------------------------------*/

extern char *SMTPOP_DELETE_MSG;

#define declare_smtpop_delete_msg(_event, _priority)                           \
    method_declare (agent, SMTPOP_DELETE_MSG, _event, _priority)

/*  Send event - delete_msg                                                  */

int 
lsend_smtpop_delete_msg (
    const QID  *_to,
    const QID  *_from,
          char *_accept,
          char *_reject,
          char *_expire,
          word _timeout,
    const qbyte msg_id,             /*  message id, zero=all             */
    const char *attach_dir);        /*  directory where attchment will be stored  */

#define send_smtpop_delete_msg(_to,                                            \
            msg_id,                                                          \
            attach_dir)                                                      \
       lsend_smtpop_delete_msg(_to,                                            \
            &thread-> queue-> qid,                                           \
            NULL, NULL, NULL, 0,                                             \
            msg_id,                                                          \
            attach_dir)

/*---------------------------------------------------------------------------
 *  Definitions and prototypes for delete_ok - delete ok response.
 *---------------------------------------------------------------------------*/

extern char *SMTPOP_DELETE_OK;

#define declare_smtpop_delete_ok(_event, _priority)                            \
    method_declare (agent, SMTPOP_DELETE_OK, _event, _priority)

/*  Send event - delete_ok                                                   */

int 
lsend_smtpop_delete_ok (
    const QID  *_to,
    const QID  *_from,
          char *_accept,
          char *_reject,
          char *_expire,
          word _timeout,
    const qbyte msg_id,             /*  message id, zero=all             */
    const char *attach_dir);        /*  directory where attchment will be stored  */

#define send_smtpop_delete_ok(_to,                                             \
            msg_id,                                                          \
            attach_dir)                                                      \
       lsend_smtpop_delete_ok(_to,                                             \
            &thread-> queue-> qid,                                           \
            NULL, NULL, NULL, 0,                                             \
            msg_id,                                                          \
            attach_dir)

/*---------------------------------------------------------------------------
 *  Definitions and prototypes for get_msg_info - ask for message info.
 *---------------------------------------------------------------------------*/

extern char *SMTPOP_GET_MSG_INFO;

#define declare_smtpop_get_msg_info(_event, _priority)                         \
    method_declare (agent, SMTPOP_GET_MSG_INFO, _event, _priority)

/*  Send event - get_msg_info                                                */

int 
lsend_smtpop_get_msg_info (
    const QID  *_to,
    const QID  *_from,
          char *_accept,
          char *_reject,
          char *_expire,
          word _timeout,
    const qbyte msg_id,             /*  message id, zero=all             */
    const char *attach_dir);        /*  directory where attchment will be stored  */

#define send_smtpop_get_msg_info(_to,                                          \
            msg_id,                                                          \
            attach_dir)                                                      \
       lsend_smtpop_get_msg_info(_to,                                          \
            &thread-> queue-> qid,                                           \
            NULL, NULL, NULL, 0,                                             \
            msg_id,                                                          \
            attach_dir)


typedef struct {
    qbyte msg_id;                       /*  message id requested             */
    char *from;                         /*  message sender                   */
    char *to;                           /*  people the message was addressed to  */
    char *cc;                           /*  people the message is carbon copied to  */
    char *date;                         /*  date the message was received    */
    char *subject;                      /*  guess what                       */
} struct_smtpop_msg_header;

int
put_smtpop_msg_header (
          byte **_buffer,
    const qbyte msg_id,                 /*  message id requested             */
    const char *from,                   /*  message sender                   */
    const char *to,                     /*  people the message was addressed to  */
    const char *cc,                     /*  people the message is carbon copied to  */
    const char *date,                   /*  date the message was received    */
    const char *subject);               /*  guess what                       */

int
get_smtpop_msg_header (
    byte *_buffer,
    struct_smtpop_msg_header **params);

void
free_smtpop_msg_header (
    struct_smtpop_msg_header **params);

/*---------------------------------------------------------------------------
 *  Definitions and prototypes for msg_header - mail info.
 *---------------------------------------------------------------------------*/

extern char *SMTPOP_MSG_HEADER;

#define declare_smtpop_msg_header(_event, _priority)                           \
    method_declare (agent, SMTPOP_MSG_HEADER, _event, _priority)

/*  Send event - msg_header                                                  */

int 
lsend_smtpop_msg_header (
    const QID  *_to,
    const QID  *_from,
          char *_accept,
          char *_reject,
          char *_expire,
          word _timeout,
    const qbyte msg_id,             /*  message id requested             */
    const char *from,               /*  message sender                   */
    const char *to,                 /*  people the message was addressed to  */
    const char *cc,                 /*  people the message is carbon copied to  */
    const char *date,               /*  date the message was received    */
    const char *subject);           /*  guess what                       */

#define send_smtpop_msg_header(_to,                                            \
            msg_id,                                                          \
            from,                                                            \
            to,                                                              \
            cc,                                                              \
            date,                                                            \
            subject)                                                         \
       lsend_smtpop_msg_header(_to,                                            \
            &thread-> queue-> qid,                                           \
            NULL, NULL, NULL, 0,                                             \
            msg_id,                                                          \
            from,                                                            \
            to,                                                              \
            cc,                                                              \
            date,                                                            \
            subject)


typedef struct {
    qbyte msg_id;                       /*  message id requested             */
    char *from;                         /*  message sender                   */
    char *to;                           /*  people the message was addressed to  */
    char *cc;                           /*  people the message is carbon copied to  */
    char *date;                         /*  date the message was received    */
    char *subject;                      /*  guess what                       */
    char *body;                         /*  the text body                    */
    char *attachments;                  /*  attachment names, separated by semi-colon  */
    char *attach_dir;                   /*  directory where attach have been stored  */
} struct_smtpop_msg;

int
put_smtpop_msg (
          byte **_buffer,
    const qbyte msg_id,                 /*  message id requested             */
    const char *from,                   /*  message sender                   */
    const char *to,                     /*  people the message was addressed to  */
    const char *cc,                     /*  people the message is carbon copied to  */
    const char *date,                   /*  date the message was received    */
    const char *subject,                /*  guess what                       */
    const char *body,                   /*  the text body                    */
    const char *attachments,            /*  attachment names, separated by semi-colon  */
    const char *attach_dir);            /*  directory where attach have been stored  */

int
get_smtpop_msg (
    byte *_buffer,
    struct_smtpop_msg **params);

void
free_smtpop_msg (
    struct_smtpop_msg **params);

/*---------------------------------------------------------------------------
 *  Definitions and prototypes for msg - complete message.
 *---------------------------------------------------------------------------*/

extern char *SMTPOP_MSG;

#define declare_smtpop_msg(_event, _priority)                                  \
    method_declare (agent, SMTPOP_MSG, _event, _priority)

/*  Send event - msg                                                         */

int 
lsend_smtpop_msg (
    const QID  *_to,
    const QID  *_from,
          char *_accept,
          char *_reject,
          char *_expire,
          word _timeout,
    const qbyte msg_id,             /*  message id requested             */
    const char *from,               /*  message sender                   */
    const char *to,                 /*  people the message was addressed to  */
    const char *cc,                 /*  people the message is carbon copied to  */
    const char *date,               /*  date the message was received    */
    const char *subject,            /*  guess what                       */
    const char *body,               /*  the text body                    */
    const char *attachments,        /*  attachment names, separated by semi-colon  */
    const char *attach_dir);        /*  directory where attach have been stored  */

#define send_smtpop_msg(_to,                                                   \
            msg_id,                                                          \
            from,                                                            \
            to,                                                              \
            cc,                                                              \
            date,                                                            \
            subject,                                                         \
            body,                                                            \
            attachments,                                                     \
            attach_dir)                                                      \
       lsend_smtpop_msg(_to,                                                   \
            &thread-> queue-> qid,                                           \
            NULL, NULL, NULL, 0,                                             \
            msg_id,                                                          \
            from,                                                            \
            to,                                                              \
            cc,                                                              \
            date,                                                            \
            subject,                                                         \
            body,                                                            \
            attachments,                                                     \
            attach_dir)


/*---------------------------------------------------------------------------
 *  Definitions and prototypes for smtsmtp - SMTP agent.
 *---------------------------------------------------------------------------*/

typedef struct {
    char *smtp_server;                  /*                                   */
    char *msg_body;                     /*                                   */
    char *sender_uid;                   /*                                   */
    char *dest_uids;                    /*                                   */
    char *subject;                      /*                                   */
} struct_smtsmtp_message;

int
put_smtsmtp_message (
          byte **_buffer,
    const char *smtp_server,            /*                                   */
    const char *msg_body,               /*                                   */
    const char *sender_uid,             /*                                   */
    const char *dest_uids,              /*                                   */
    const char *subject);               /*                                   */

int
get_smtsmtp_message (
    byte *_buffer,
    struct_smtsmtp_message **params);

void
free_smtsmtp_message (
    struct_smtsmtp_message **params);

/*---------------------------------------------------------------------------
 *  Definitions and prototypes for send_message - .
 *---------------------------------------------------------------------------*/

extern char *SMTSMTP_SEND_MESSAGE;

#define declare_smtsmtp_send_message(_event, _priority)                        \
    method_declare (agent, SMTSMTP_SEND_MESSAGE, _event, _priority)

/*  Send event - send_message                                                */

int 
lsend_smtsmtp_send_message (
    const QID  *_to,
    const QID  *_from,
          char *_accept,
          char *_reject,
          char *_expire,
          word _timeout,
    const char *smtp_server,        /*                                   */
    const char *msg_body,           /*                                   */
    const char *sender_uid,         /*                                   */
    const char *dest_uids,          /*                                   */
    const char *subject);           /*                                   */

#define send_smtsmtp_send_message(_to,                                         \
            smtp_server,                                                     \
            msg_body,                                                        \
            sender_uid,                                                      \
            dest_uids,                                                       \
            subject)                                                         \
       lsend_smtsmtp_send_message(_to,                                         \
            &thread-> queue-> qid,                                           \
            NULL, NULL, NULL, 0,                                             \
            smtp_server,                                                     \
            msg_body,                                                        \
            sender_uid,                                                      \
            dest_uids,                                                       \
            subject)

/*---------------------------------------------------------------------------
 *  Definitions and prototypes for open_message - .
 *---------------------------------------------------------------------------*/

extern char *SMTSMTP_OPEN_MESSAGE;

#define declare_smtsmtp_open_message(_event, _priority)                        \
    method_declare (agent, SMTSMTP_OPEN_MESSAGE, _event, _priority)

/*  Send event - open_message                                                */

int 
lsend_smtsmtp_open_message (
    const QID  *_to,
    const QID  *_from,
          char *_accept,
          char *_reject,
          char *_expire,
          word _timeout,
    const char *smtp_server,        /*                                   */
    const char *msg_body,           /*                                   */
    const char *sender_uid,         /*                                   */
    const char *dest_uids,          /*                                   */
    const char *subject);           /*                                   */

#define send_smtsmtp_open_message(_to,                                         \
            smtp_server,                                                     \
            msg_body,                                                        \
            sender_uid,                                                      \
            dest_uids,                                                       \
            subject)                                                         \
       lsend_smtsmtp_open_message(_to,                                         \
            &thread-> queue-> qid,                                           \
            NULL, NULL, NULL, 0,                                             \
            smtp_server,                                                     \
            msg_body,                                                        \
            sender_uid,                                                      \
            dest_uids,                                                       \
            subject)


typedef struct {
    char *chunk;                        /*                                   */
} struct_smtsmtp_chunk;

int
put_smtsmtp_chunk (
          byte **_buffer,
    const char *chunk);                 /*                                   */

int
get_smtsmtp_chunk (
    byte *_buffer,
    struct_smtsmtp_chunk **params);

void
free_smtsmtp_chunk (
    struct_smtsmtp_chunk **params);

/*---------------------------------------------------------------------------
 *  Definitions and prototypes for message chunk - .
 *---------------------------------------------------------------------------*/

extern char *SMTSMTP_MESSAGE_CHUNK;

#define declare_smtsmtp_message_chunk(_event, _priority)                       \
    method_declare (agent, SMTSMTP_MESSAGE_CHUNK, _event, _priority)

/*  Send event - message chunk                                               */

int 
lsend_smtsmtp_message_chunk (
    const QID  *_to,
    const QID  *_from,
          char *_accept,
          char *_reject,
          char *_expire,
          word _timeout,
    const char *chunk);             /*                                   */

#define send_smtsmtp_message_chunk(_to,                                        \
            chunk)                                                           \
       lsend_smtsmtp_message_chunk(_to,                                        \
            &thread-> queue-> qid,                                           \
            NULL, NULL, NULL, 0,                                             \
            chunk)


/*---------------------------------------------------------------------------
 *  Definitions and prototypes for close_message - .
 *---------------------------------------------------------------------------*/

extern char *SMTSMTP_CLOSE_MESSAGE;

#define declare_smtsmtp_close_message(_event, _priority)                       \
    method_declare (agent, SMTSMTP_CLOSE_MESSAGE, _event, _priority)

/*  Send event - close_message                                               */

#define lsend_smtsmtp_close_message(_to, _from,                                \
    _accept, _reject, _expire, _timeout)                                     \
        event_send (_to,                                                     \
                    _from,                                                   \
                    SMTSMTP_CLOSE_MESSAGE,                                     \
                    NULL, 0,                                                 \
                    _accept, _reject, _expire, _timeout)
#define send_smtsmtp_close_message(_to)                                        \
        event_send (_to,                                                     \
                    &thread-> queue-> qid,                                   \
                    SMTSMTP_CLOSE_MESSAGE,                                     \
                    NULL, 0,                                                 \
                    NULL, NULL, NULL, 0)


typedef struct {
    qbyte code;                         /*  error code                       */
    char *msg;                          /*  error description                */
} struct_smtsmtp_reply;

int
put_smtsmtp_reply (
          byte **_buffer,
    const qbyte code,                   /*  error code                       */
    const char *msg);                   /*  error description                */

int
get_smtsmtp_reply (
    byte *_buffer,
    struct_smtsmtp_reply **params);

void
free_smtsmtp_reply (
    struct_smtsmtp_reply **params);

/*---------------------------------------------------------------------------
 *  Definitions and prototypes for ok - .
 *---------------------------------------------------------------------------*/

extern char *SMTSMTP_OK;

#define declare_smtsmtp_ok(_event, _priority)                                  \
    method_declare (agent, SMTSMTP_OK, _event, _priority)

/*  Send event - ok                                                          */

int 
lsend_smtsmtp_ok (
    const QID  *_to,
    const QID  *_from,
          char *_accept,
          char *_reject,
          char *_expire,
          word _timeout,
    const qbyte code,               /*  error code                       */
    const char *msg);               /*  error description                */

#define send_smtsmtp_ok(_to,                                                   \
            code,                                                            \
            msg)                                                             \
       lsend_smtsmtp_ok(_to,                                                   \
            &thread-> queue-> qid,                                           \
            NULL, NULL, NULL, 0,                                             \
            code,                                                            \
            msg)

/*---------------------------------------------------------------------------
 *  Definitions and prototypes for error - .
 *---------------------------------------------------------------------------*/

extern char *SMTSMTP_ERROR;

#define declare_smtsmtp_error(_event, _priority)                               \
    method_declare (agent, SMTSMTP_ERROR, _event, _priority)

/*  Send event - error                                                       */

int 
lsend_smtsmtp_error (
    const QID  *_to,
    const QID  *_from,
          char *_accept,
          char *_reject,
          char *_expire,
          word _timeout,
    const qbyte code,               /*  error code                       */
    const char *msg);               /*  error description                */

#define send_smtsmtp_error(_to,                                                \
            code,                                                            \
            msg)                                                             \
       lsend_smtsmtp_error(_to,                                                \
            &thread-> queue-> qid,                                           \
            NULL, NULL, NULL, 0,                                             \
            code,                                                            \
            msg)


/*---------------------------------------------------------------------------
 *  Definitions and prototypes for smttmtp - TMTP agent.
 *---------------------------------------------------------------------------*/

typedef struct {
    char *host;                         /*  Remote host                      */
    char *port;                         /*                                   */
    word  size;                         /*  Amount of data to write          */
    void *data;                         /*  Block of data to write           */
} struct_smttmtp_write;

int
put_smttmtp_write (
          byte **_buffer,
    const char *host,                   /*  Remote host                      */
    const char *port,                   /*                                   */
    const word  size,                   /*  Amount of data to write          */
    const void *data);                  /*  Block of data to write           */

int
get_smttmtp_write (
    byte *_buffer,
    struct_smttmtp_write **params);

void
free_smttmtp_write (
    struct_smttmtp_write **params);

/*---------------------------------------------------------------------------
 *  Definitions and prototypes for write - .
 *---------------------------------------------------------------------------*/

extern char *SMTTMTP_WRITE;

#define declare_smttmtp_write(_event, _priority)                               \
    method_declare (agent, SMTTMTP_WRITE, _event, _priority)

/*  Send event - write                                                       */

int 
lsend_smttmtp_write (
    const QID  *_to,
    const QID  *_from,
          char *_accept,
          char *_reject,
          char *_expire,
          word _timeout,
    const char *host,               /*  Remote host                      */
    const char *port,               /*                                   */
    const word  size,               /*  Amount of data to write          */
    const void *data);              /*  Block of data to write           */

#define send_smttmtp_write(_to,                                                \
            host,                                                            \
            port,                                                            \
            size,                                                            \
            data)                                                            \
       lsend_smttmtp_write(_to,                                                \
            &thread-> queue-> qid,                                           \
            NULL, NULL, NULL, 0,                                             \
            host,                                                            \
            port,                                                            \
            size,                                                            \
            data)


typedef struct {
    char *port;                         /*                                   */
} struct_smttmtp_listen;

int
put_smttmtp_listen (
          byte **_buffer,
    const char *port);                  /*                                   */

int
get_smttmtp_listen (
    byte *_buffer,
    struct_smttmtp_listen **params);

void
free_smttmtp_listen (
    struct_smttmtp_listen **params);

/*---------------------------------------------------------------------------
 *  Definitions and prototypes for listen - .
 *---------------------------------------------------------------------------*/

extern char *SMTTMTP_LISTEN;

#define declare_smttmtp_listen(_event, _priority)                              \
    method_declare (agent, SMTTMTP_LISTEN, _event, _priority)

/*  Send event - listen                                                      */

int 
lsend_smttmtp_listen (
    const QID  *_to,
    const QID  *_from,
          char *_accept,
          char *_reject,
          char *_expire,
          word _timeout,
    const char *port);              /*                                   */

#define send_smttmtp_listen(_to,                                               \
            port)                                                            \
       lsend_smttmtp_listen(_to,                                               \
            &thread-> queue-> qid,                                           \
            NULL, NULL, NULL, 0,                                             \
            port)


/*---------------------------------------------------------------------------
 *  Definitions and prototypes for write ok - .
 *---------------------------------------------------------------------------*/

extern char *SMTTMTP_WRITE_OK;

#define declare_smttmtp_write_ok(_event, _priority)                            \
    method_declare (agent, SMTTMTP_WRITE_OK, _event, _priority)

/*  Send event - write ok                                                    */

#define lsend_smttmtp_write_ok(_to, _from,                                     \
    _accept, _reject, _expire, _timeout)                                     \
        event_send (_to,                                                     \
                    _from,                                                   \
                    SMTTMTP_WRITE_OK,                                          \
                    NULL, 0,                                                 \
                    _accept, _reject, _expire, _timeout)
#define send_smttmtp_write_ok(_to)                                             \
        event_send (_to,                                                     \
                    &thread-> queue-> qid,                                   \
                    SMTTMTP_WRITE_OK,                                          \
                    NULL, 0,                                                 \
                    NULL, NULL, NULL, 0)


/*---------------------------------------------------------------------------
 *  Definitions and prototypes for listen ok - .
 *---------------------------------------------------------------------------*/

extern char *SMTTMTP_LISTEN_OK;

#define declare_smttmtp_listen_ok(_event, _priority)                           \
    method_declare (agent, SMTTMTP_LISTEN_OK, _event, _priority)

/*  Send event - listen ok                                                   */

#define lsend_smttmtp_listen_ok(_to, _from,                                    \
    _accept, _reject, _expire, _timeout)                                     \
        event_send (_to,                                                     \
                    _from,                                                   \
                    SMTTMTP_LISTEN_OK,                                         \
                    NULL, 0,                                                 \
                    _accept, _reject, _expire, _timeout)
#define send_smttmtp_listen_ok(_to)                                            \
        event_send (_to,                                                     \
                    &thread-> queue-> qid,                                   \
                    SMTTMTP_LISTEN_OK,                                         \
                    NULL, 0,                                                 \
                    NULL, NULL, NULL, 0)


typedef struct {
    char *message;                      /*                                   */
} struct_smttmtp_error;

int
put_smttmtp_error (
          byte **_buffer,
    const char *message);               /*                                   */

int
get_smttmtp_error (
    byte *_buffer,
    struct_smttmtp_error **params);

void
free_smttmtp_error (
    struct_smttmtp_error **params);

/*---------------------------------------------------------------------------
 *  Definitions and prototypes for write error - .
 *---------------------------------------------------------------------------*/

extern char *SMTTMTP_WRITE_ERROR;

#define declare_smttmtp_write_error(_event, _priority)                         \
    method_declare (agent, SMTTMTP_WRITE_ERROR, _event, _priority)

/*  Send event - write error                                                 */

int 
lsend_smttmtp_write_error (
    const QID  *_to,
    const QID  *_from,
          char *_accept,
          char *_reject,
          char *_expire,
          word _timeout,
    const char *message);           /*                                   */

#define send_smttmtp_write_error(_to,                                          \
            message)                                                         \
       lsend_smttmtp_write_error(_to,                                          \
            &thread-> queue-> qid,                                           \
            NULL, NULL, NULL, 0,                                             \
            message)

/*---------------------------------------------------------------------------
 *  Definitions and prototypes for listen error - .
 *---------------------------------------------------------------------------*/

extern char *SMTTMTP_LISTEN_ERROR;

#define declare_smttmtp_listen_error(_event, _priority)                        \
    method_declare (agent, SMTTMTP_LISTEN_ERROR, _event, _priority)

/*  Send event - listen error                                                */

int 
lsend_smttmtp_listen_error (
    const QID  *_to,
    const QID  *_from,
          char *_accept,
          char *_reject,
          char *_expire,
          word _timeout,
    const char *message);           /*                                   */

#define send_smttmtp_listen_error(_to,                                         \
            message)                                                         \
       lsend_smttmtp_listen_error(_to,                                         \
            &thread-> queue-> qid,                                           \
            NULL, NULL, NULL, 0,                                             \
            message)


typedef struct {
    word  size;                         /*  Amount of data to write          */
    void *data;                         /*  Block of data read               */
} struct_smttmtp_incoming_message;

int
put_smttmtp_incoming_message (
          byte **_buffer,
    const word  size,                   /*  Amount of data to write          */
    const void *data);                  /*  Block of data read               */

int
get_smttmtp_incoming_message (
    byte *_buffer,
    struct_smttmtp_incoming_message **params);

void
free_smttmtp_incoming_message (
    struct_smttmtp_incoming_message **params);

/*---------------------------------------------------------------------------
 *  Definitions and prototypes for incoming data - .
 *---------------------------------------------------------------------------*/

extern char *SMTTMTP_INCOMING_DATA;

#define declare_smttmtp_incoming_data(_event, _priority)                       \
    method_declare (agent, SMTTMTP_INCOMING_DATA, _event, _priority)

/*  Send event - incoming data                                               */

int 
lsend_smttmtp_incoming_data (
    const QID  *_to,
    const QID  *_from,
          char *_accept,
          char *_reject,
          char *_expire,
          word _timeout,
    const word  size,               /*  Amount of data to write          */
    const void *data);              /*  Block of data read               */

#define send_smttmtp_incoming_data(_to,                                        \
            size,                                                            \
            data)                                                            \
       lsend_smttmtp_incoming_data(_to,                                        \
            &thread-> queue-> qid,                                           \
            NULL, NULL, NULL, 0,                                             \
            size,                                                            \
            data)



#endif                                  /*  Included                         */
