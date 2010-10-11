/*---------------------------------------------------------------------------
 *  ggcodem.h - prototypes for GGCODE messages.
 *
 *  Generated from ggcode.xml by smtmesg.gsl using GSL.
 *  DO NOT MODIFY THIS FILE.
 *
 *  For documentation and updates see http://www.imatix.com.
 *---------------------------------------------------------------------------*/
#ifndef INCLUDE_GGCODEM
#define INCLUDE_GGCODEM

/*---------------------------------------------------------------------------
 *  Definitions and prototypes for ggcode - GSLGen GGCODE agent.
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
 *  Definitions and prototypes for execute - Execute script.
 *---------------------------------------------------------------------------*/

extern char *GGCODE_EXECUTE;

#define declare_ggcode_execute(_event, _priority)                              \
    method_declare (agent, GGCODE_EXECUTE, _event, _priority)

/*  Send event - execute                                                     */

#define lsend_ggcode_execute(_to, _from,                                       \
    _accept, _reject, _expire, _timeout)                                     \
        event_send (_to,                                                     \
                    _from,                                                   \
                    GGCODE_EXECUTE,                                            \
                    NULL, 0,                                                 \
                    _accept, _reject, _expire, _timeout)
#define send_ggcode_execute(_to)                                               \
        event_send (_to,                                                     \
                    &thread-> queue-> qid,                                   \
                    GGCODE_EXECUTE,                                            \
                    NULL, 0,                                                 \
                    NULL, NULL, NULL, 0)


/*---------------------------------------------------------------------------
 *  Definitions and prototypes for start - Start thread.
 *---------------------------------------------------------------------------*/

extern char *GGCODE_START;

#define declare_ggcode_start(_event, _priority)                                \
    method_declare (agent, GGCODE_START, _event, _priority)

/*  Send event - start                                                       */

#define lsend_ggcode_start(_to, _from,                                         \
    _accept, _reject, _expire, _timeout)                                     \
        event_send (_to,                                                     \
                    _from,                                                   \
                    GGCODE_START,                                              \
                    NULL, 0,                                                 \
                    _accept, _reject, _expire, _timeout)
#define send_ggcode_start(_to)                                                 \
        event_send (_to,                                                     \
                    &thread-> queue-> qid,                                   \
                    GGCODE_START,                                              \
                    NULL, 0,                                                 \
                    NULL, NULL, NULL, 0)


/*---------------------------------------------------------------------------
 *  Definitions and prototypes for spawn - Spawn thread.
 *---------------------------------------------------------------------------*/

extern char *GGCODE_SPAWN;

#define declare_ggcode_spawn(_event, _priority)                                \
    method_declare (agent, GGCODE_SPAWN, _event, _priority)

/*  Send event - spawn                                                       */

#define lsend_ggcode_spawn(_to, _from,                                         \
    _accept, _reject, _expire, _timeout)                                     \
        event_send (_to,                                                     \
                    _from,                                                   \
                    GGCODE_SPAWN,                                              \
                    NULL, 0,                                                 \
                    _accept, _reject, _expire, _timeout)
#define send_ggcode_spawn(_to)                                                 \
        event_send (_to,                                                     \
                    &thread-> queue-> qid,                                   \
                    GGCODE_SPAWN,                                              \
                    NULL, 0,                                                 \
                    NULL, NULL, NULL, 0)


/*---------------------------------------------------------------------------
 *  Definitions and prototypes for continue - Continue thread.
 *---------------------------------------------------------------------------*/

extern char *GGCODE_CONTINUE;

#define declare_ggcode_continue(_event, _priority)                             \
    method_declare (agent, GGCODE_CONTINUE, _event, _priority)

/*  Send event - continue                                                    */

#define lsend_ggcode_continue(_to, _from,                                      \
    _accept, _reject, _expire, _timeout)                                     \
        event_send (_to,                                                     \
                    _from,                                                   \
                    GGCODE_CONTINUE,                                           \
                    NULL, 0,                                                 \
                    _accept, _reject, _expire, _timeout)
#define send_ggcode_continue(_to)                                              \
        event_send (_to,                                                     \
                    &thread-> queue-> qid,                                   \
                    GGCODE_CONTINUE,                                           \
                    NULL, 0,                                                 \
                    NULL, NULL, NULL, 0)


/*---------------------------------------------------------------------------
 *  Definitions and prototypes for next - Execute next line.
 *---------------------------------------------------------------------------*/

extern char *GGCODE_NEXT;

#define declare_ggcode_next(_event, _priority)                                 \
    method_declare (agent, GGCODE_NEXT, _event, _priority)

/*  Send event - next                                                        */

#define lsend_ggcode_next(_to, _from,                                          \
    _accept, _reject, _expire, _timeout)                                     \
        event_send (_to,                                                     \
                    _from,                                                   \
                    GGCODE_NEXT,                                               \
                    NULL, 0,                                                 \
                    _accept, _reject, _expire, _timeout)
#define send_ggcode_next(_to)                                                  \
        event_send (_to,                                                     \
                    &thread-> queue-> qid,                                   \
                    GGCODE_NEXT,                                               \
                    NULL, 0,                                                 \
                    NULL, NULL, NULL, 0)


/*---------------------------------------------------------------------------
 *  Definitions and prototypes for step - Step next line.
 *---------------------------------------------------------------------------*/

extern char *GGCODE_STEP;

#define declare_ggcode_step(_event, _priority)                                 \
    method_declare (agent, GGCODE_STEP, _event, _priority)

/*  Send event - step                                                        */

#define lsend_ggcode_step(_to, _from,                                          \
    _accept, _reject, _expire, _timeout)                                     \
        event_send (_to,                                                     \
                    _from,                                                   \
                    GGCODE_STEP,                                               \
                    NULL, 0,                                                 \
                    _accept, _reject, _expire, _timeout)
#define send_ggcode_step(_to)                                                  \
        event_send (_to,                                                     \
                    &thread-> queue-> qid,                                   \
                    GGCODE_STEP,                                               \
                    NULL, 0,                                                 \
                    NULL, NULL, NULL, 0)


/*---------------------------------------------------------------------------
 *  Definitions and prototypes for finish - Finish thread.
 *---------------------------------------------------------------------------*/

extern char *GGCODE_FINISH;

#define declare_ggcode_finish(_event, _priority)                               \
    method_declare (agent, GGCODE_FINISH, _event, _priority)

/*  Send event - finish                                                      */

#define lsend_ggcode_finish(_to, _from,                                        \
    _accept, _reject, _expire, _timeout)                                     \
        event_send (_to,                                                     \
                    _from,                                                   \
                    GGCODE_FINISH,                                             \
                    NULL, 0,                                                 \
                    _accept, _reject, _expire, _timeout)
#define send_ggcode_finish(_to)                                                \
        event_send (_to,                                                     \
                    &thread-> queue-> qid,                                   \
                    GGCODE_FINISH,                                             \
                    NULL, 0,                                                 \
                    NULL, NULL, NULL, 0)


typedef struct {
    char *command;                      /*  GSL command line                 */
} struct_ggcode_gsl;

int
put_ggcode_gsl (
          byte **_buffer,
    const char *command);               /*  GSL command line                 */

int
get_ggcode_gsl (
    byte *_buffer,
    struct_ggcode_gsl **params);

void
free_ggcode_gsl (
    struct_ggcode_gsl **params);

/*---------------------------------------------------------------------------
 *  Definitions and prototypes for gsl - Execute command.
 *---------------------------------------------------------------------------*/

extern char *GGCODE_GSL;

#define declare_ggcode_gsl(_event, _priority)                                  \
    method_declare (agent, GGCODE_GSL, _event, _priority)

/*  Send event - gsl                                                         */

int 
lsend_ggcode_gsl (
    const QID  *_to,
    const QID  *_from,
          char *_accept,
          char *_reject,
          char *_expire,
          word _timeout,
    const char *command);           /*  GSL command line                 */

#define send_ggcode_gsl(_to,                                                   \
            command)                                                         \
       lsend_ggcode_gsl(_to,                                                   \
            &thread-> queue-> qid,                                           \
            NULL, NULL, NULL, 0,                                             \
            command)


typedef struct {
    char *expression;                   /*  GSL command line                 */
} struct_ggcode_evaluate;

int
put_ggcode_evaluate (
          byte **_buffer,
    const char *expression);            /*  GSL command line                 */

int
get_ggcode_evaluate (
    byte *_buffer,
    struct_ggcode_evaluate **params);

void
free_ggcode_evaluate (
    struct_ggcode_evaluate **params);

/*---------------------------------------------------------------------------
 *  Definitions and prototypes for evaluate - Execute command.
 *---------------------------------------------------------------------------*/

extern char *GGCODE_EVALUATE;

#define declare_ggcode_evaluate(_event, _priority)                             \
    method_declare (agent, GGCODE_EVALUATE, _event, _priority)

/*  Send event - evaluate                                                    */

int 
lsend_ggcode_evaluate (
    const QID  *_to,
    const QID  *_from,
          char *_accept,
          char *_reject,
          char *_expire,
          word _timeout,
    const char *expression);        /*  GSL command line                 */

#define send_ggcode_evaluate(_to,                                              \
            expression)                                                      \
       lsend_ggcode_evaluate(_to,                                              \
            &thread-> queue-> qid,                                           \
            NULL, NULL, NULL, 0,                                             \
            expression)


/*---------------------------------------------------------------------------
 *  Definitions and prototypes for call - Call GSL function.
 *---------------------------------------------------------------------------*/

extern char *GGCODE_CALL;

#define declare_ggcode_call(_event, _priority)                                 \
    method_declare (agent, GGCODE_CALL, _event, _priority)

/*  Send event - call                                                        */

#define lsend_ggcode_call(_to, _from,                                          \
    _accept, _reject, _expire, _timeout)                                     \
        event_send (_to,                                                     \
                    _from,                                                   \
                    GGCODE_CALL,                                               \
                    NULL, 0,                                                 \
                    _accept, _reject, _expire, _timeout)
#define send_ggcode_call(_to)                                                  \
        event_send (_to,                                                     \
                    &thread-> queue-> qid,                                   \
                    GGCODE_CALL,                                               \
                    NULL, 0,                                                 \
                    NULL, NULL, NULL, 0)


typedef struct {
    void *job;                          /*  Job id                           */
} struct_ggcode_job;

int
put_ggcode_job (
          byte **_buffer,
    const void *job);                   /*  Job id                           */

int
get_ggcode_job (
    byte *_buffer,
    struct_ggcode_job **params);

void
free_ggcode_job (
    struct_ggcode_job **params);

/*---------------------------------------------------------------------------
 *  Definitions and prototypes for ok - OK reply.
 *---------------------------------------------------------------------------*/

extern char *GGCODE_OK;

#define declare_ggcode_ok(_event, _priority)                                   \
    method_declare (agent, GGCODE_OK, _event, _priority)

/*  Send event - ok                                                          */

int 
lsend_ggcode_ok (
    const QID  *_to,
    const QID  *_from,
          char *_accept,
          char *_reject,
          char *_expire,
          word _timeout,
    const void *job);               /*  Job id                           */

#define send_ggcode_ok(_to,                                                    \
            job)                                                             \
       lsend_ggcode_ok(_to,                                                    \
            &thread-> queue-> qid,                                           \
            NULL, NULL, NULL, 0,                                             \
            job)


typedef struct {
    void *job;                          /*  Job id                           */
    char *error_name;                   /*  Error file name                  */
    qbyte error_line;                   /*  Error line                       */
    char *error_text;                   /*  Error message text               */
} struct_ggcode_error_reply;

int
put_ggcode_error_reply (
          byte **_buffer,
    const void *job,                    /*  Job id                           */
    const char *error_name,             /*  Error file name                  */
    const qbyte error_line,             /*  Error line                       */
    const char *error_text);            /*  Error message text               */

int
get_ggcode_error_reply (
    byte *_buffer,
    struct_ggcode_error_reply **params);

void
free_ggcode_error_reply (
    struct_ggcode_error_reply **params);

/*---------------------------------------------------------------------------
 *  Definitions and prototypes for message - Error reply.
 *---------------------------------------------------------------------------*/

extern char *GGCODE_MESSAGE;

#define declare_ggcode_message(_event, _priority)                              \
    method_declare (agent, GGCODE_MESSAGE, _event, _priority)

/*  Send event - message                                                     */

int 
lsend_ggcode_message (
    const QID  *_to,
    const QID  *_from,
          char *_accept,
          char *_reject,
          char *_expire,
          word _timeout,
    const void *job,                /*  Job id                           */
    const char *error_name,         /*  Error file name                  */
    const qbyte error_line,         /*  Error line                       */
    const char *error_text);        /*  Error message text               */

#define send_ggcode_message(_to,                                               \
            job,                                                             \
            error_name,                                                      \
            error_line,                                                      \
            error_text)                                                      \
       lsend_ggcode_message(_to,                                               \
            &thread-> queue-> qid,                                           \
            NULL, NULL, NULL, 0,                                             \
            job,                                                             \
            error_name,                                                      \
            error_line,                                                      \
            error_text)

/*---------------------------------------------------------------------------
 *  Definitions and prototypes for error - Error reply.
 *---------------------------------------------------------------------------*/

extern char *GGCODE_ERROR;

#define declare_ggcode_error(_event, _priority)                                \
    method_declare (agent, GGCODE_ERROR, _event, _priority)

/*  Send event - error                                                       */

int 
lsend_ggcode_error (
    const QID  *_to,
    const QID  *_from,
          char *_accept,
          char *_reject,
          char *_expire,
          word _timeout,
    const void *job,                /*  Job id                           */
    const char *error_name,         /*  Error file name                  */
    const qbyte error_line,         /*  Error line                       */
    const char *error_text);        /*  Error message text               */

#define send_ggcode_error(_to,                                                 \
            job,                                                             \
            error_name,                                                      \
            error_line,                                                      \
            error_text)                                                      \
       lsend_ggcode_error(_to,                                                 \
            &thread-> queue-> qid,                                           \
            NULL, NULL, NULL, 0,                                             \
            job,                                                             \
            error_name,                                                      \
            error_line,                                                      \
            error_text)

/*---------------------------------------------------------------------------
 *  Definitions and prototypes for fatal - Fatal reply.
 *---------------------------------------------------------------------------*/

extern char *GGCODE_FATAL;

#define declare_ggcode_fatal(_event, _priority)                                \
    method_declare (agent, GGCODE_FATAL, _event, _priority)

/*  Send event - fatal                                                       */

int 
lsend_ggcode_fatal (
    const QID  *_to,
    const QID  *_from,
          char *_accept,
          char *_reject,
          char *_expire,
          word _timeout,
    const void *job,                /*  Job id                           */
    const char *error_name,         /*  Error file name                  */
    const qbyte error_line,         /*  Error line                       */
    const char *error_text);        /*  Error message text               */

#define send_ggcode_fatal(_to,                                                 \
            job,                                                             \
            error_name,                                                      \
            error_line,                                                      \
            error_text)                                                      \
       lsend_ggcode_fatal(_to,                                                 \
            &thread-> queue-> qid,                                           \
            NULL, NULL, NULL, 0,                                             \
            job,                                                             \
            error_name,                                                      \
            error_line,                                                      \
            error_text)


/*---------------------------------------------------------------------------
 *  Definitions and prototypes for call ok - .
 *---------------------------------------------------------------------------*/

extern char *GGCODE_CALL_OK;

#define declare_ggcode_call_ok(_event, _priority)                              \
    method_declare (agent, GGCODE_CALL_OK, _event, _priority)

/*  Send event - call ok                                                     */

#define lsend_ggcode_call_ok(_to, _from,                                       \
    _accept, _reject, _expire, _timeout)                                     \
        event_send (_to,                                                     \
                    _from,                                                   \
                    GGCODE_CALL_OK,                                            \
                    NULL, 0,                                                 \
                    _accept, _reject, _expire, _timeout)
#define send_ggcode_call_ok(_to)                                               \
        event_send (_to,                                                     \
                    &thread-> queue-> qid,                                   \
                    GGCODE_CALL_OK,                                            \
                    NULL, 0,                                                 \
                    NULL, NULL, NULL, 0)


typedef struct {
    char *error_name;                   /*  Error file name                  */
    qbyte error_line;                   /*  Error line                       */
    char *error_text;                   /*  Error message text               */
} struct_ggcode_call_error;

int
put_ggcode_call_error (
          byte **_buffer,
    const char *error_name,             /*  Error file name                  */
    const qbyte error_line,             /*  Error line                       */
    const char *error_text);            /*  Error message text               */

int
get_ggcode_call_error (
    byte *_buffer,
    struct_ggcode_call_error **params);

void
free_ggcode_call_error (
    struct_ggcode_call_error **params);

/*---------------------------------------------------------------------------
 *  Definitions and prototypes for call message - Message reply.
 *---------------------------------------------------------------------------*/

extern char *GGCODE_CALL_MESSAGE;

#define declare_ggcode_call_message(_event, _priority)                         \
    method_declare (agent, GGCODE_CALL_MESSAGE, _event, _priority)

/*  Send event - call message                                                */

int 
lsend_ggcode_call_message (
    const QID  *_to,
    const QID  *_from,
          char *_accept,
          char *_reject,
          char *_expire,
          word _timeout,
    const char *error_name,         /*  Error file name                  */
    const qbyte error_line,         /*  Error line                       */
    const char *error_text);        /*  Error message text               */

#define send_ggcode_call_message(_to,                                          \
            error_name,                                                      \
            error_line,                                                      \
            error_text)                                                      \
       lsend_ggcode_call_message(_to,                                          \
            &thread-> queue-> qid,                                           \
            NULL, NULL, NULL, 0,                                             \
            error_name,                                                      \
            error_line,                                                      \
            error_text)

/*---------------------------------------------------------------------------
 *  Definitions and prototypes for call error - Error reply.
 *---------------------------------------------------------------------------*/

extern char *GGCODE_CALL_ERROR;

#define declare_ggcode_call_error(_event, _priority)                           \
    method_declare (agent, GGCODE_CALL_ERROR, _event, _priority)

/*  Send event - call error                                                  */

int 
lsend_ggcode_call_error (
    const QID  *_to,
    const QID  *_from,
          char *_accept,
          char *_reject,
          char *_expire,
          word _timeout,
    const char *error_name,         /*  Error file name                  */
    const qbyte error_line,         /*  Error line                       */
    const char *error_text);        /*  Error message text               */

#define send_ggcode_call_error(_to,                                            \
            error_name,                                                      \
            error_line,                                                      \
            error_text)                                                      \
       lsend_ggcode_call_error(_to,                                            \
            &thread-> queue-> qid,                                           \
            NULL, NULL, NULL, 0,                                             \
            error_name,                                                      \
            error_line,                                                      \
            error_text)



#endif                                  /*  Included                         */
