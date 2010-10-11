/*---------------------------------------------------------------------------
 *  smtsslm.h - prototypes for Smtssl messages.
 *
 *  Generated from smtsslm.xml by smtmesg.gsl using GSL.
 *  DO NOT MODIFY THIS FILE.
 *
 *  For documentation and updates see http://www.imatix.com.
 *---------------------------------------------------------------------------*/
#ifndef INCLUDE_SMTSSLM
#define INCLUDE_SMTSSLM

/*---------------------------------------------------------------------------
 *  Definitions and prototypes for smtssl - SMT SSL agent.
 *---------------------------------------------------------------------------*/

typedef struct {
    char *config;                       /*  Configuration file to use        */
} struct_smtssl_open;

int
put_smtssl_open (
          byte **_buffer,
    const char *config);                /*  Configuration file to use        */

int
get_smtssl_open (
    byte *_buffer,
    struct_smtssl_open **params);

void
free_smtssl_open (
    struct_smtssl_open **params);

/*---------------------------------------------------------------------------
 *  Definitions and prototypes for open - Open SSL port.
 *---------------------------------------------------------------------------*/

extern char *SMTSSL_OPEN;

#define declare_smtssl_open(_event, _priority)                                 \
    method_declare (agent, SMTSSL_OPEN, _event, _priority)

/*  Send event - open                                                        */

int 
lsend_smtssl_open (
    const QID  *_to,
    const QID  *_from,
          char *_accept,
          char *_reject,
          char *_expire,
          word _timeout,
    const char *config);            /*  Configuration file to use        */

#define send_smtssl_open(_to,                                                  \
            config)                                                          \
       lsend_smtssl_open(_to,                                                  \
            &thread-> queue-> qid,                                           \
            NULL, NULL, NULL, 0,                                             \
            config)


typedef struct {
    dbyte port;                         /*  SSL port opened                  */
} struct_smtssl_open_ok;

int
put_smtssl_open_ok (
          byte **_buffer,
    const dbyte port);                  /*  SSL port opened                  */

int
get_smtssl_open_ok (
    byte *_buffer,
    struct_smtssl_open_ok **params);

void
free_smtssl_open_ok (
    struct_smtssl_open_ok **params);

/*---------------------------------------------------------------------------
 *  Definitions and prototypes for open ok - SSL port opened OK.
 *---------------------------------------------------------------------------*/

extern char *SMTSSL_OPEN_OK;

#define declare_smtssl_open_ok(_event, _priority)                              \
    method_declare (agent, SMTSSL_OPEN_OK, _event, _priority)

/*  Send event - open ok                                                     */

int 
lsend_smtssl_open_ok (
    const QID  *_to,
    const QID  *_from,
          char *_accept,
          char *_reject,
          char *_expire,
          word _timeout,
    const dbyte port);              /*  SSL port opened                  */

#define send_smtssl_open_ok(_to,                                               \
            port)                                                            \
       lsend_smtssl_open_ok(_to,                                               \
            &thread-> queue-> qid,                                           \
            NULL, NULL, NULL, 0,                                             \
            port)


/*---------------------------------------------------------------------------
 *  Definitions and prototypes for close - Close SSL port.
 *---------------------------------------------------------------------------*/

extern char *SMTSSL_CLOSE;

#define declare_smtssl_close(_event, _priority)                                \
    method_declare (agent, SMTSSL_CLOSE, _event, _priority)

/*  Send event - close                                                       */

#define lsend_smtssl_close(_to, _from,                                         \
    _accept, _reject, _expire, _timeout)                                     \
        event_send (_to,                                                     \
                    _from,                                                   \
                    SMTSSL_CLOSE,                                              \
                    NULL, 0,                                                 \
                    _accept, _reject, _expire, _timeout)
#define send_smtssl_close(_to)                                                 \
        event_send (_to,                                                     \
                    &thread-> queue-> qid,                                   \
                    SMTSSL_CLOSE,                                              \
                    NULL, 0,                                                 \
                    NULL, NULL, NULL, 0)


/*---------------------------------------------------------------------------
 *  Definitions and prototypes for restart - Restart SSL connection.
 *---------------------------------------------------------------------------*/

extern char *SMTSSL_RESTART;

#define declare_smtssl_restart(_event, _priority)                              \
    method_declare (agent, SMTSSL_RESTART, _event, _priority)

/*  Send event - restart                                                     */

#define lsend_smtssl_restart(_to, _from,                                       \
    _accept, _reject, _expire, _timeout)                                     \
        event_send (_to,                                                     \
                    _from,                                                   \
                    SMTSSL_RESTART,                                            \
                    NULL, 0,                                                 \
                    _accept, _reject, _expire, _timeout)
#define send_smtssl_restart(_to)                                               \
        event_send (_to,                                                     \
                    &thread-> queue-> qid,                                   \
                    SMTSSL_RESTART,                                            \
                    NULL, 0,                                                 \
                    NULL, NULL, NULL, 0)


typedef struct {
    qbyte socket;                       /*  Socket handle for SSL connection  */
    char *user;                         /*  User name                        */
    char *cipher;                       /*  Cipher used                      */
    dbyte verify;                       /*  Level of user verification       */
} struct_smtssl_accepted;

int
put_smtssl_accepted (
          byte **_buffer,
    const qbyte socket,                 /*  Socket handle for SSL connection  */
    const char *user,                   /*  User name                        */
    const char *cipher,                 /*  Cipher used                      */
    const dbyte verify);                /*  Level of user verification       */

int
get_smtssl_accepted (
    byte *_buffer,
    struct_smtssl_accepted **params);

void
free_smtssl_accepted (
    struct_smtssl_accepted **params);

/*---------------------------------------------------------------------------
 *  Definitions and prototypes for accepted - Accept SSL connection.
 *---------------------------------------------------------------------------*/

extern char *SMTSSL_ACCEPTED;

#define declare_smtssl_accepted(_event, _priority)                             \
    method_declare (agent, SMTSSL_ACCEPTED, _event, _priority)

/*  Send event - accepted                                                    */

int 
lsend_smtssl_accepted (
    const QID  *_to,
    const QID  *_from,
          char *_accept,
          char *_reject,
          char *_expire,
          word _timeout,
    const qbyte socket,             /*  Socket handle for SSL connection  */
    const char *user,               /*  User name                        */
    const char *cipher,             /*  Cipher used                      */
    const dbyte verify);            /*  Level of user verification       */

#define send_smtssl_accepted(_to,                                              \
            socket,                                                          \
            user,                                                            \
            cipher,                                                          \
            verify)                                                          \
       lsend_smtssl_accepted(_to,                                              \
            &thread-> queue-> qid,                                           \
            NULL, NULL, NULL, 0,                                             \
            socket,                                                          \
            user,                                                            \
            cipher,                                                          \
            verify)


typedef struct {
    qbyte size;                         /*  Maximum size to read             */
} struct_smtssl_read_request;

int
put_smtssl_read_request (
          byte **_buffer,
    const qbyte size);                  /*  Maximum size to read             */

int
get_smtssl_read_request (
    byte *_buffer,
    struct_smtssl_read_request **params);

void
free_smtssl_read_request (
    struct_smtssl_read_request **params);

/*---------------------------------------------------------------------------
 *  Definitions and prototypes for read request - Request read.
 *---------------------------------------------------------------------------*/

extern char *SMTSSL_READ_REQUEST;

#define declare_smtssl_read_request(_event, _priority)                         \
    method_declare (agent, SMTSSL_READ_REQUEST, _event, _priority)

/*  Send event - read request                                                */

int 
lsend_smtssl_read_request (
    const QID  *_to,
    const QID  *_from,
          char *_accept,
          char *_reject,
          char *_expire,
          word _timeout,
    const qbyte size);              /*  Maximum size to read             */

#define send_smtssl_read_request(_to,                                          \
            size)                                                            \
       lsend_smtssl_read_request(_to,                                          \
            &thread-> queue-> qid,                                           \
            NULL, NULL, NULL, 0,                                             \
            size)


typedef struct {
    dbyte timeout;                      /*  Timeout in seconds, zero = none  */
    qbyte socket;                       /*  Socket to write to               */
    word  size;                         /*  Amount of data to write          */
    void *data;                         /*  Block of data to write           */
    Bool  reply;                        /*  Whether OK reply is required     */
    qbyte tag;                          /*  User-defined request tag         */
} struct_smtssl_write_request;

int
put_smtssl_write_request (
          byte **_buffer,
    const dbyte timeout,                /*  Timeout in seconds, zero = none  */
    const qbyte socket,                 /*  Socket to write to               */
    const word  size,                   /*  Amount of data to write          */
    const void *data,                   /*  Block of data to write           */
    const Bool  reply,                  /*  Whether OK reply is required     */
    const qbyte tag);                   /*  User-defined request tag         */

int
get_smtssl_write_request (
    byte *_buffer,
    struct_smtssl_write_request **params);

void
free_smtssl_write_request (
    struct_smtssl_write_request **params);

/*---------------------------------------------------------------------------
 *  Definitions and prototypes for write request - Write SSL data.
 *---------------------------------------------------------------------------*/

extern char *SMTSSL_WRITE_REQUEST;

#define declare_smtssl_write_request(_event, _priority)                        \
    method_declare (agent, SMTSSL_WRITE_REQUEST, _event, _priority)

/*  Send event - write request                                               */

int 
lsend_smtssl_write_request (
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
    const qbyte tag);               /*  User-defined request tag         */

#define send_smtssl_write_request(_to,                                         \
            timeout,                                                         \
            socket,                                                          \
            size,                                                            \
            data,                                                            \
            reply,                                                           \
            tag)                                                             \
       lsend_smtssl_write_request(_to,                                         \
            &thread-> queue-> qid,                                           \
            NULL, NULL, NULL, 0,                                             \
            timeout,                                                         \
            socket,                                                          \
            size,                                                            \
            data,                                                            \
            reply,                                                           \
            tag)


typedef struct {
    qbyte socket;                       /*  Socket for output                */
    char *filename;                     /*  Name of file to send             */
    qbyte start;                        /*  Starting offset; 0 = start       */
    qbyte end;                          /*  Ending offset; 0 = end           */
} struct_smtssl_put_slice;

int
put_smtssl_put_slice (
          byte **_buffer,
    const qbyte socket,                 /*  Socket for output                */
    const char *filename,               /*  Name of file to send             */
    const qbyte start,                  /*  Starting offset; 0 = start       */
    const qbyte end);                   /*  Ending offset; 0 = end           */

int
get_smtssl_put_slice (
    byte *_buffer,
    struct_smtssl_put_slice **params);

void
free_smtssl_put_slice (
    struct_smtssl_put_slice **params);

/*---------------------------------------------------------------------------
 *  Definitions and prototypes for put slice - Write file slice to SSL socket.
 *---------------------------------------------------------------------------*/

extern char *SMTSSL_PUT_SLICE;

#define declare_smtssl_put_slice(_event, _priority)                            \
    method_declare (agent, SMTSSL_PUT_SLICE, _event, _priority)

/*  Send event - put slice                                                   */

int 
lsend_smtssl_put_slice (
    const QID  *_to,
    const QID  *_from,
          char *_accept,
          char *_reject,
          char *_expire,
          word _timeout,
    const qbyte socket,             /*  Socket for output                */
    const char *filename,           /*  Name of file to send             */
    const qbyte start,              /*  Starting offset; 0 = start       */
    const qbyte end);               /*  Ending offset; 0 = end           */

#define send_smtssl_put_slice(_to,                                             \
            socket,                                                          \
            filename,                                                        \
            start,                                                           \
            end)                                                             \
       lsend_smtssl_put_slice(_to,                                             \
            &thread-> queue-> qid,                                           \
            NULL, NULL, NULL, 0,                                             \
            socket,                                                          \
            filename,                                                        \
            start,                                                           \
            end)


typedef struct {
    qbyte code;                         /*  Error code                       */
} struct_smtssl_error;

int
put_smtssl_error (
          byte **_buffer,
    const qbyte code);                  /*  Error code                       */

int
get_smtssl_error (
    byte *_buffer,
    struct_smtssl_error **params);

void
free_smtssl_error (
    struct_smtssl_error **params);

/*---------------------------------------------------------------------------
 *  Definitions and prototypes for error - Signal SSL error.
 *---------------------------------------------------------------------------*/

extern char *SMTSSL_ERROR;

#define declare_smtssl_error(_event, _priority)                                \
    method_declare (agent, SMTSSL_ERROR, _event, _priority)

/*  Send event - error                                                       */

int 
lsend_smtssl_error (
    const QID  *_to,
    const QID  *_from,
          char *_accept,
          char *_reject,
          char *_expire,
          word _timeout,
    const qbyte code);              /*  Error code                       */

#define send_smtssl_error(_to,                                                 \
            code)                                                            \
       lsend_smtssl_error(_to,                                                 \
            &thread-> queue-> qid,                                           \
            NULL, NULL, NULL, 0,                                             \
            code)


typedef struct {
    qbyte socket;                       /*  Socket used for i/o, or new socket  */
    qbyte tag;                          /*  User-defined request tag         */
} struct_smtssl_write_ok;

int
put_smtssl_write_ok (
          byte **_buffer,
    const qbyte socket,                 /*  Socket used for i/o, or new socket  */
    const qbyte tag);                   /*  User-defined request tag         */

int
get_smtssl_write_ok (
    byte *_buffer,
    struct_smtssl_write_ok **params);

void
free_smtssl_write_ok (
    struct_smtssl_write_ok **params);

/*---------------------------------------------------------------------------
 *  Definitions and prototypes for write ok - Write OK reply.
 *---------------------------------------------------------------------------*/

extern char *SMTSSL_WRITE_OK;

#define declare_smtssl_write_ok(_event, _priority)                             \
    method_declare (agent, SMTSSL_WRITE_OK, _event, _priority)

/*  Send event - write ok                                                    */

int 
lsend_smtssl_write_ok (
    const QID  *_to,
    const QID  *_from,
          char *_accept,
          char *_reject,
          char *_expire,
          word _timeout,
    const qbyte socket,             /*  Socket used for i/o, or new socket  */
    const qbyte tag);               /*  User-defined request tag         */

#define send_smtssl_write_ok(_to,                                              \
            socket,                                                          \
            tag)                                                             \
       lsend_smtssl_write_ok(_to,                                              \
            &thread-> queue-> qid,                                           \
            NULL, NULL, NULL, 0,                                             \
            socket,                                                          \
            tag)


typedef struct {
    word  size;                         /*  Size of result                   */
    void *data;                         /*  Read data                        */
} struct_smtssl_read_ok;

int
put_smtssl_read_ok (
          byte **_buffer,
    const word  size,                   /*  Size of result                   */
    const void *data);                  /*  Read data                        */

int
get_smtssl_read_ok (
    byte *_buffer,
    struct_smtssl_read_ok **params);

void
free_smtssl_read_ok (
    struct_smtssl_read_ok **params);

/*---------------------------------------------------------------------------
 *  Definitions and prototypes for read ok - Read result data.
 *---------------------------------------------------------------------------*/

extern char *SMTSSL_READ_OK;

#define declare_smtssl_read_ok(_event, _priority)                              \
    method_declare (agent, SMTSSL_READ_OK, _event, _priority)

/*  Send event - read ok                                                     */

int 
lsend_smtssl_read_ok (
    const QID  *_to,
    const QID  *_from,
          char *_accept,
          char *_reject,
          char *_expire,
          word _timeout,
    const word  size,               /*  Size of result                   */
    const void *data);              /*  Read data                        */

#define send_smtssl_read_ok(_to,                                               \
            size,                                                            \
            data)                                                            \
       lsend_smtssl_read_ok(_to,                                               \
            &thread-> queue-> qid,                                           \
            NULL, NULL, NULL, 0,                                             \
            size,                                                            \
            data)


typedef struct {
    qbyte size;                         /*  Amount of transmitted data       */
} struct_smtssl_put_slice_ok;

int
put_smtssl_put_slice_ok (
          byte **_buffer,
    const qbyte size);                  /*  Amount of transmitted data       */

int
get_smtssl_put_slice_ok (
    byte *_buffer,
    struct_smtssl_put_slice_ok **params);

void
free_smtssl_put_slice_ok (
    struct_smtssl_put_slice_ok **params);

/*---------------------------------------------------------------------------
 *  Definitions and prototypes for put slice ok - File written okay.
 *---------------------------------------------------------------------------*/

extern char *SMTSSL_PUT_SLICE_OK;

#define declare_smtssl_put_slice_ok(_event, _priority)                         \
    method_declare (agent, SMTSSL_PUT_SLICE_OK, _event, _priority)

/*  Send event - put slice ok                                                */

int 
lsend_smtssl_put_slice_ok (
    const QID  *_to,
    const QID  *_from,
          char *_accept,
          char *_reject,
          char *_expire,
          word _timeout,
    const qbyte size);              /*  Amount of transmitted data       */

#define send_smtssl_put_slice_ok(_to,                                          \
            size)                                                            \
       lsend_smtssl_put_slice_ok(_to,                                          \
            &thread-> queue-> qid,                                           \
            NULL, NULL, NULL, 0,                                             \
            size)



#endif                                  /*  Included                         */
