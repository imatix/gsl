/*---------------------------------------------------------------------------
 *  ggparsm.h - prototypes for ggpars messages.
 *
 *  Generated from ggpars.xml by smtmesg.gsl using GSL.
 *  DO NOT MODIFY THIS FILE.
 *
 *  For documentation and updates see http://www.imatix.com.
 *---------------------------------------------------------------------------*/
#ifndef INCLUDE_GGPARSM
#define INCLUDE_GGPARSM

/*---------------------------------------------------------------------------
 *  Definitions and prototypes for ggpars - gslgen parser agent.
 *---------------------------------------------------------------------------*/

typedef struct {
    void *job;                          /*  job id                           */
} struct_ggpars_parse;

int
put_ggpars_parse (
          byte **_buffer,
    const void *job);                   /*  job id                           */

int
get_ggpars_parse (
    byte *_buffer,
    struct_ggpars_parse **params);

void
free_ggpars_parse (
    struct_ggpars_parse **params);

/*---------------------------------------------------------------------------
 *  Definitions and prototypes for template - parse template line.
 *---------------------------------------------------------------------------*/

extern char *GGPARS_TEMPLATE;

#define declare_ggpars_template(_event, _priority)                             \
    method_declare (agent, GGPARS_TEMPLATE, _event, _priority)

/*  Send event - template                                                    */

int 
lsend_ggpars_template (
    const QID  *_to,
    const QID  *_from,
          char *_accept,
          char *_reject,
          char *_expire,
          word _timeout,
    const void *job);               /*  job id                           */

#define send_ggpars_template(_to,                                              \
            job)                                                             \
       lsend_ggpars_template(_to,                                              \
            &thread-> queue-> qid,                                           \
            NULL, NULL, NULL, 0,                                             \
            job)

/*---------------------------------------------------------------------------
 *  Definitions and prototypes for gsl - parse gsl line.
 *---------------------------------------------------------------------------*/

extern char *GGPARS_GSL;

#define declare_ggpars_gsl(_event, _priority)                                  \
    method_declare (agent, GGPARS_GSL, _event, _priority)

/*  Send event - gsl                                                         */

int 
lsend_ggpars_gsl (
    const QID  *_to,
    const QID  *_from,
          char *_accept,
          char *_reject,
          char *_expire,
          word _timeout,
    const void *job);               /*  job id                           */

#define send_ggpars_gsl(_to,                                                   \
            job)                                                             \
       lsend_ggpars_gsl(_to,                                                   \
            &thread-> queue-> qid,                                           \
            NULL, NULL, NULL, 0,                                             \
            job)

/*---------------------------------------------------------------------------
 *  Definitions and prototypes for expression - parse expression.
 *---------------------------------------------------------------------------*/

extern char *GGPARS_EXPRESSION;

#define declare_ggpars_expression(_event, _priority)                           \
    method_declare (agent, GGPARS_EXPRESSION, _event, _priority)

/*  Send event - expression                                                  */

int 
lsend_ggpars_expression (
    const QID  *_to,
    const QID  *_from,
          char *_accept,
          char *_reject,
          char *_expire,
          word _timeout,
    const void *job);               /*  job id                           */

#define send_ggpars_expression(_to,                                            \
            job)                                                             \
       lsend_ggpars_expression(_to,                                            \
            &thread-> queue-> qid,                                           \
            NULL, NULL, NULL, 0,                                             \
            job)


typedef struct {
    void *job;                          /*  job id                           */
    void *parse_root;                   /*  parse tree root                  */
    void *parse_memtrn;                 /*  Memory transaction               */
    qbyte size;                         /*  Data size                        */
} struct_ggpars_ok;

int
put_ggpars_ok (
          byte **_buffer,
    const void *job,                    /*  job id                           */
    const void *parse_root,             /*  parse tree root                  */
    const void *parse_memtrn,           /*  Memory transaction               */
    const qbyte size);                  /*  Data size                        */

int
get_ggpars_ok (
    byte *_buffer,
    struct_ggpars_ok **params);

void
free_ggpars_ok (
    struct_ggpars_ok **params);

/*---------------------------------------------------------------------------
 *  Definitions and prototypes for ok - ok reply.
 *---------------------------------------------------------------------------*/

extern char *GGPARS_OK;

#define declare_ggpars_ok(_event, _priority)                                   \
    method_declare (agent, GGPARS_OK, _event, _priority)

/*  Send event - ok                                                          */

int 
lsend_ggpars_ok (
    const QID  *_to,
    const QID  *_from,
          char *_accept,
          char *_reject,
          char *_expire,
          word _timeout,
    const void *job,                /*  job id                           */
    const void *parse_root,         /*  parse tree root                  */
    const void *parse_memtrn,       /*  Memory transaction               */
    const qbyte size);              /*  Data size                        */

#define send_ggpars_ok(_to,                                                    \
            job,                                                             \
            parse_root,                                                      \
            parse_memtrn,                                                    \
            size)                                                            \
       lsend_ggpars_ok(_to,                                                    \
            &thread-> queue-> qid,                                           \
            NULL, NULL, NULL, 0,                                             \
            job,                                                             \
            parse_root,                                                      \
            parse_memtrn,                                                    \
            size)


typedef struct {
    void *job;                          /*  job id                           */
} struct_ggpars_eof;

int
put_ggpars_eof (
          byte **_buffer,
    const void *job);                   /*  job id                           */

int
get_ggpars_eof (
    byte *_buffer,
    struct_ggpars_eof **params);

void
free_ggpars_eof (
    struct_ggpars_eof **params);

/*---------------------------------------------------------------------------
 *  Definitions and prototypes for eof - eof reply.
 *---------------------------------------------------------------------------*/

extern char *GGPARS_EOF;

#define declare_ggpars_eof(_event, _priority)                                  \
    method_declare (agent, GGPARS_EOF, _event, _priority)

/*  Send event - eof                                                         */

int 
lsend_ggpars_eof (
    const QID  *_to,
    const QID  *_from,
          char *_accept,
          char *_reject,
          char *_expire,
          word _timeout,
    const void *job);               /*  job id                           */

#define send_ggpars_eof(_to,                                                   \
            job)                                                             \
       lsend_ggpars_eof(_to,                                                   \
            &thread-> queue-> qid,                                           \
            NULL, NULL, NULL, 0,                                             \
            job)


typedef struct {
    void *job;                          /*  job id                           */
    char *error_text;                   /*  error message text               */
} struct_ggpars_error;

int
put_ggpars_error (
          byte **_buffer,
    const void *job,                    /*  job id                           */
    const char *error_text);            /*  error message text               */

int
get_ggpars_error (
    byte *_buffer,
    struct_ggpars_error **params);

void
free_ggpars_error (
    struct_ggpars_error **params);

/*---------------------------------------------------------------------------
 *  Definitions and prototypes for error - error reply.
 *---------------------------------------------------------------------------*/

extern char *GGPARS_ERROR;

#define declare_ggpars_error(_event, _priority)                                \
    method_declare (agent, GGPARS_ERROR, _event, _priority)

/*  Send event - error                                                       */

int 
lsend_ggpars_error (
    const QID  *_to,
    const QID  *_from,
          char *_accept,
          char *_reject,
          char *_expire,
          word _timeout,
    const void *job,                /*  job id                           */
    const char *error_text);        /*  error message text               */

#define send_ggpars_error(_to,                                                 \
            job,                                                             \
            error_text)                                                      \
       lsend_ggpars_error(_to,                                                 \
            &thread-> queue-> qid,                                           \
            NULL, NULL, NULL, 0,                                             \
            job,                                                             \
            error_text)



#endif                                  /*  Included                         */
