/*---------------------------------------------------------------------------
 *  ggparsm.c - functions for ggpars messages.
 *
 *  Generated from ggpars.xml by smtmesg.gsl using GSL
 *  DO NOT MODIFY THIS FILE.
 *
 *  For documentation and updates see http://www.imatix.com.
 *---------------------------------------------------------------------------*/

#include "sfl.h"                        /*  SFL header file                  */
#include "smtlib.h"                     /*  SMT header file                  */
#include "ggparsm.h"                    /*  Definitions & prototypes         */

/*---------------------------------------------------------------------------
 *  Message functions for ggpars - gslgen parser agent.
 *---------------------------------------------------------------------------*/


/*  ---------------------------------------------------------------------[<]-
    Function: put_ggpars_parse

    Synopsis: Formats a parse message, allocates a new buffer,
    and returns the formatted message in the buffer.  You should free the
    buffer using mem_free() when finished.  Returns the size of the buffer
    in bytes.
    ---------------------------------------------------------------------[>]-*/

int
put_ggpars_parse (
    byte **_buffer,
    const void *job)                    /*  job id                           */
{
    struct_ggpars_parse
        *_struct_ptr;

    _struct_ptr = mem_alloc (sizeof (struct_ggpars_parse));
    *_buffer = (byte *) _struct_ptr;
    if (_struct_ptr)
      {
        _struct_ptr-> job               = (void *) job;

        return sizeof (*_struct_ptr);
      }
    else
        return 0;
}


/*  ---------------------------------------------------------------------[<]-
    Function: get_ggpars_parse

    Synopsis: Accepts a buffer containing a parse message,
    and unpacks it into a new struct_ggpars_parse structure. Free the
    structure using free_ggpars_parse() when finished.
    ---------------------------------------------------------------------[>]-*/

int
get_ggpars_parse (
    byte *_buffer,
    struct_ggpars_parse **params)
{
    struct_ggpars_parse
        *_struct_ptr;

    _struct_ptr = (struct_ggpars_parse *) _buffer;
    *params = mem_alloc (sizeof (struct_ggpars_parse));
    if (*params)
      {
        (* params)-> job                = _struct_ptr-> job;
        return 0;
      }
    else
        return -1;
}


/*  ---------------------------------------------------------------------[<]-
    Function: free_ggpars_parse

    Synopsis: frees a structure allocated by get_ggpars_parse().
    ---------------------------------------------------------------------[>]-*/

void
free_ggpars_parse (
    struct_ggpars_parse **params)
{
    mem_free (*params);
    *params = NULL;
}

char *GGPARS_TEMPLATE = "GGPARS TEMPLATE";

/*  ---------------------------------------------------------------------[<]-
    Function: lsend_ggpars_template

    Synopsis: Sends a template - parse template line
              event to the ggpars agent
    ---------------------------------------------------------------------[>]-*/

int 
lsend_ggpars_template (
    const QID  *_to,
    const QID  *_from,
          char *_accept,
          char *_reject,
          char *_expire,
          word _timeout,
    const void *job)                /*  job id                           */
{
    byte *_body;
    int   _size,
          _rc;
    _size = put_ggpars_parse
                (&_body,
                 job);
    if (_size)
      {
        _rc = event_send (_to, _from, GGPARS_TEMPLATE,
                          _body, _size,
                          _accept, _reject, _expire, _timeout);
        mem_free (_body);
        return _rc;
      }
    else
        return -1;
}

char *GGPARS_GSL = "GGPARS GSL";

/*  ---------------------------------------------------------------------[<]-
    Function: lsend_ggpars_gsl

    Synopsis: Sends a gsl - parse gsl line
              event to the ggpars agent
    ---------------------------------------------------------------------[>]-*/

int 
lsend_ggpars_gsl (
    const QID  *_to,
    const QID  *_from,
          char *_accept,
          char *_reject,
          char *_expire,
          word _timeout,
    const void *job)                /*  job id                           */
{
    byte *_body;
    int   _size,
          _rc;
    _size = put_ggpars_parse
                (&_body,
                 job);
    if (_size)
      {
        _rc = event_send (_to, _from, GGPARS_GSL,
                          _body, _size,
                          _accept, _reject, _expire, _timeout);
        mem_free (_body);
        return _rc;
      }
    else
        return -1;
}

char *GGPARS_EXPRESSION = "GGPARS EXPRESSION";

/*  ---------------------------------------------------------------------[<]-
    Function: lsend_ggpars_expression

    Synopsis: Sends a expression - parse expression
              event to the ggpars agent
    ---------------------------------------------------------------------[>]-*/

int 
lsend_ggpars_expression (
    const QID  *_to,
    const QID  *_from,
          char *_accept,
          char *_reject,
          char *_expire,
          word _timeout,
    const void *job)                /*  job id                           */
{
    byte *_body;
    int   _size,
          _rc;
    _size = put_ggpars_parse
                (&_body,
                 job);
    if (_size)
      {
        _rc = event_send (_to, _from, GGPARS_EXPRESSION,
                          _body, _size,
                          _accept, _reject, _expire, _timeout);
        mem_free (_body);
        return _rc;
      }
    else
        return -1;
}


/*  ---------------------------------------------------------------------[<]-
    Function: put_ggpars_ok

    Synopsis: Formats a ok message, allocates a new buffer,
    and returns the formatted message in the buffer.  You should free the
    buffer using mem_free() when finished.  Returns the size of the buffer
    in bytes.
    ---------------------------------------------------------------------[>]-*/

int
put_ggpars_ok (
    byte **_buffer,
    const void *job,                    /*  job id                           */
    const void *parse_root,             /*  parse tree root                  */
    const void *parse_memtrn,           /*  Memory transaction               */
    const qbyte size)                   /*  Data size                        */
{
    struct_ggpars_ok
        *_struct_ptr;

    _struct_ptr = mem_alloc (sizeof (struct_ggpars_ok));
    *_buffer = (byte *) _struct_ptr;
    if (_struct_ptr)
      {
        _struct_ptr-> job               = (void *) job;
        _struct_ptr-> parse_root        = (void *) parse_root;
        _struct_ptr-> parse_memtrn      = (void *) parse_memtrn;
        _struct_ptr-> size              = size;

        return sizeof (*_struct_ptr);
      }
    else
        return 0;
}


/*  ---------------------------------------------------------------------[<]-
    Function: get_ggpars_ok

    Synopsis: Accepts a buffer containing a ok message,
    and unpacks it into a new struct_ggpars_ok structure. Free the
    structure using free_ggpars_ok() when finished.
    ---------------------------------------------------------------------[>]-*/

int
get_ggpars_ok (
    byte *_buffer,
    struct_ggpars_ok **params)
{
    struct_ggpars_ok
        *_struct_ptr;

    _struct_ptr = (struct_ggpars_ok *) _buffer;
    *params = mem_alloc (sizeof (struct_ggpars_ok));
    if (*params)
      {
        (* params)-> job                = _struct_ptr-> job;
        (* params)-> parse_root         = _struct_ptr-> parse_root;
        (* params)-> parse_memtrn       = _struct_ptr-> parse_memtrn;
        (* params)-> size               = _struct_ptr-> size;
        return 0;
      }
    else
        return -1;
}


/*  ---------------------------------------------------------------------[<]-
    Function: free_ggpars_ok

    Synopsis: frees a structure allocated by get_ggpars_ok().
    ---------------------------------------------------------------------[>]-*/

void
free_ggpars_ok (
    struct_ggpars_ok **params)
{
    mem_free (*params);
    *params = NULL;
}

char *GGPARS_OK = "GGPARS OK";

/*  ---------------------------------------------------------------------[<]-
    Function: lsend_ggpars_ok

    Synopsis: Sends a ok - ok reply
              event to the ggpars agent
    ---------------------------------------------------------------------[>]-*/

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
    const qbyte size)               /*  Data size                        */
{
    byte *_body;
    int   _size,
          _rc;
    _size = put_ggpars_ok
                (&_body,
                 job,
                 parse_root,
                 parse_memtrn,
                 size);
    if (_size)
      {
        _rc = event_send (_to, _from, GGPARS_OK,
                          _body, _size,
                          _accept, _reject, _expire, _timeout);
        mem_free (_body);
        return _rc;
      }
    else
        return -1;
}


/*  ---------------------------------------------------------------------[<]-
    Function: put_ggpars_eof

    Synopsis: Formats a eof message, allocates a new buffer,
    and returns the formatted message in the buffer.  You should free the
    buffer using mem_free() when finished.  Returns the size of the buffer
    in bytes.
    ---------------------------------------------------------------------[>]-*/

int
put_ggpars_eof (
    byte **_buffer,
    const void *job)                    /*  job id                           */
{
    struct_ggpars_eof
        *_struct_ptr;

    _struct_ptr = mem_alloc (sizeof (struct_ggpars_eof));
    *_buffer = (byte *) _struct_ptr;
    if (_struct_ptr)
      {
        _struct_ptr-> job               = (void *) job;

        return sizeof (*_struct_ptr);
      }
    else
        return 0;
}


/*  ---------------------------------------------------------------------[<]-
    Function: get_ggpars_eof

    Synopsis: Accepts a buffer containing a eof message,
    and unpacks it into a new struct_ggpars_eof structure. Free the
    structure using free_ggpars_eof() when finished.
    ---------------------------------------------------------------------[>]-*/

int
get_ggpars_eof (
    byte *_buffer,
    struct_ggpars_eof **params)
{
    struct_ggpars_eof
        *_struct_ptr;

    _struct_ptr = (struct_ggpars_eof *) _buffer;
    *params = mem_alloc (sizeof (struct_ggpars_eof));
    if (*params)
      {
        (* params)-> job                = _struct_ptr-> job;
        return 0;
      }
    else
        return -1;
}


/*  ---------------------------------------------------------------------[<]-
    Function: free_ggpars_eof

    Synopsis: frees a structure allocated by get_ggpars_eof().
    ---------------------------------------------------------------------[>]-*/

void
free_ggpars_eof (
    struct_ggpars_eof **params)
{
    mem_free (*params);
    *params = NULL;
}

char *GGPARS_EOF = "GGPARS EOF";

/*  ---------------------------------------------------------------------[<]-
    Function: lsend_ggpars_eof

    Synopsis: Sends a eof - eof reply
              event to the ggpars agent
    ---------------------------------------------------------------------[>]-*/

int 
lsend_ggpars_eof (
    const QID  *_to,
    const QID  *_from,
          char *_accept,
          char *_reject,
          char *_expire,
          word _timeout,
    const void *job)                /*  job id                           */
{
    byte *_body;
    int   _size,
          _rc;
    _size = put_ggpars_eof
                (&_body,
                 job);
    if (_size)
      {
        _rc = event_send (_to, _from, GGPARS_EOF,
                          _body, _size,
                          _accept, _reject, _expire, _timeout);
        mem_free (_body);
        return _rc;
      }
    else
        return -1;
}


/*  ---------------------------------------------------------------------[<]-
    Function: put_ggpars_error

    Synopsis: Formats a error message, allocates a new buffer,
    and returns the formatted message in the buffer.  You should free the
    buffer using mem_free() when finished.  Returns the size of the buffer
    in bytes.
    ---------------------------------------------------------------------[>]-*/

int
put_ggpars_error (
    byte **_buffer,
    const void *job,                    /*  job id                           */
    const char *error_text)             /*  error message text               */
{
    struct_ggpars_error
        *_struct_ptr;
    int
        _total_size = sizeof (struct_ggpars_error);
    char
        *_ptr;

    _total_size += error_text ? strlen (error_text) + 1 : 0;
    _struct_ptr = mem_alloc (_total_size);
    *_buffer = (byte *) _struct_ptr;
    if (_struct_ptr)
      {
        _ptr = (char *) _struct_ptr + sizeof (struct_ggpars_error);
        _struct_ptr-> job               = (void *) job;
        if (error_text)
          {
            _struct_ptr-> error_text        = (char *) _ptr;
            strcpy ((char *) _ptr, error_text);
            _ptr += strlen (error_text) + 1;
          }
        else
            _struct_ptr-> error_text        = NULL;

        return _total_size;
      }
    else
        return 0;
}


/*  ---------------------------------------------------------------------[<]-
    Function: get_ggpars_error

    Synopsis: Accepts a buffer containing a error message,
    and unpacks it into a new struct_ggpars_error structure. Free the
    structure using free_ggpars_error() when finished.
    ---------------------------------------------------------------------[>]-*/

int
get_ggpars_error (
    byte *_buffer,
    struct_ggpars_error **params)
{
    struct_ggpars_error
        *_struct_ptr;
    char
        *_ptr;

    _struct_ptr = (struct_ggpars_error *) _buffer;
    *params = mem_alloc (sizeof (struct_ggpars_error));
    if (*params)
      {
        _ptr = (char *) _struct_ptr + sizeof (struct_ggpars_error);
        (* params)-> job                = _struct_ptr-> job;
        if (_struct_ptr-> error_text)
          {
            (* params)-> error_text         = mem_strdup (_ptr);
            _ptr += strlen ((* params)-> error_text) + 1;
          }
        else
            (* params)-> error_text         = NULL;
        return 0;
      }
    else
        return -1;
}


/*  ---------------------------------------------------------------------[<]-
    Function: free_ggpars_error

    Synopsis: frees a structure allocated by get_ggpars_error().
    ---------------------------------------------------------------------[>]-*/

void
free_ggpars_error (
    struct_ggpars_error **params)
{
    mem_free ((*params)-> error_text);
    mem_free (*params);
    *params = NULL;
}

char *GGPARS_ERROR = "GGPARS ERROR";

/*  ---------------------------------------------------------------------[<]-
    Function: lsend_ggpars_error

    Synopsis: Sends a error - error reply
              event to the ggpars agent
    ---------------------------------------------------------------------[>]-*/

int 
lsend_ggpars_error (
    const QID  *_to,
    const QID  *_from,
          char *_accept,
          char *_reject,
          char *_expire,
          word _timeout,
    const void *job,                /*  job id                           */
    const char *error_text)         /*  error message text               */
{
    byte *_body;
    int   _size,
          _rc;
    _size = put_ggpars_error
                (&_body,
                 job,
                 error_text);
    if (_size)
      {
        _rc = event_send (_to, _from, GGPARS_ERROR,
                          _body, _size,
                          _accept, _reject, _expire, _timeout);
        mem_free (_body);
        return _rc;
      }
    else
        return -1;
}


