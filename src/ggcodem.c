/*---------------------------------------------------------------------------
 *  ggcodem.c - functions for GGCODE messages.
 *
 *  Generated from ggcode.xml by smtmesg.gsl using GSL
 *  DO NOT MODIFY THIS FILE.
 *
 *  For documentation and updates see http://www.imatix.com.
 *---------------------------------------------------------------------------*/

#include "sfl.h"                        /*  SFL header file                  */
#include "smtlib.h"                     /*  SMT header file                  */
#include "ggcodem.h"                    /*  Definitions & prototypes         */

/*---------------------------------------------------------------------------
 *  Message functions for ggcode - GSLGen GGCODE agent.
 *---------------------------------------------------------------------------*/

char *GGCODE_EXECUTE = "GGCODE EXECUTE";

char *GGCODE_START = "GGCODE START";

char *GGCODE_SPAWN = "GGCODE SPAWN";

char *GGCODE_CONTINUE = "GGCODE CONTINUE";

char *GGCODE_NEXT = "GGCODE NEXT";

char *GGCODE_STEP = "GGCODE STEP";

char *GGCODE_FINISH = "GGCODE FINISH";


/*  ---------------------------------------------------------------------[<]-
    Function: put_ggcode_gsl

    Synopsis: Formats a gsl message, allocates a new buffer,
    and returns the formatted message in the buffer.  You should free the
    buffer using mem_free() when finished.  Returns the size of the buffer
    in bytes.
    ---------------------------------------------------------------------[>]-*/

int
put_ggcode_gsl (
    byte **_buffer,
    const char *command)                /*  GSL command line                 */
{
    struct_ggcode_gsl
        *_struct_ptr;
    int
        _total_size = sizeof (struct_ggcode_gsl);
    char
        *_ptr;

    _total_size += command ? strlen (command) + 1 : 0;
    _struct_ptr = mem_alloc (_total_size);
    *_buffer = (byte *) _struct_ptr;
    if (_struct_ptr)
      {
        _ptr = (char *) _struct_ptr + sizeof (struct_ggcode_gsl);
        if (command)
          {
            _struct_ptr-> command           = (char *) _ptr;
            strcpy ((char *) _ptr, command);
            _ptr += strlen (command) + 1;
          }
        else
            _struct_ptr-> command           = NULL;

        return _total_size;
      }
    else
        return 0;
}


/*  ---------------------------------------------------------------------[<]-
    Function: get_ggcode_gsl

    Synopsis: Accepts a buffer containing a gsl message,
    and unpacks it into a new struct_ggcode_gsl structure. Free the
    structure using free_ggcode_gsl() when finished.
    ---------------------------------------------------------------------[>]-*/

int
get_ggcode_gsl (
    byte *_buffer,
    struct_ggcode_gsl **params)
{
    struct_ggcode_gsl
        *_struct_ptr;
    char
        *_ptr;

    _struct_ptr = (struct_ggcode_gsl *) _buffer;
    *params = mem_alloc (sizeof (struct_ggcode_gsl));
    if (*params)
      {
        _ptr = (char *) _struct_ptr + sizeof (struct_ggcode_gsl);
        if (_struct_ptr-> command)
          {
            (* params)-> command            = mem_strdup (_ptr);
            _ptr += strlen ((* params)-> command) + 1;
          }
        else
            (* params)-> command            = NULL;
        return 0;
      }
    else
        return -1;
}


/*  ---------------------------------------------------------------------[<]-
    Function: free_ggcode_gsl

    Synopsis: frees a structure allocated by get_ggcode_gsl().
    ---------------------------------------------------------------------[>]-*/

void
free_ggcode_gsl (
    struct_ggcode_gsl **params)
{
    mem_free ((*params)-> command);
    mem_free (*params);
    *params = NULL;
}

char *GGCODE_GSL = "GGCODE GSL";

/*  ---------------------------------------------------------------------[<]-
    Function: lsend_ggcode_gsl

    Synopsis: Sends a gsl - Execute command
              event to the ggcode agent
    ---------------------------------------------------------------------[>]-*/

int 
lsend_ggcode_gsl (
    const QID  *_to,
    const QID  *_from,
          char *_accept,
          char *_reject,
          char *_expire,
          word _timeout,
    const char *command)            /*  GSL command line                 */
{
    byte *_body;
    int   _size,
          _rc;
    _size = put_ggcode_gsl
                (&_body,
                 command);
    if (_size)
      {
        _rc = event_send (_to, _from, GGCODE_GSL,
                          _body, _size,
                          _accept, _reject, _expire, _timeout);
        mem_free (_body);
        return _rc;
      }
    else
        return -1;
}


/*  ---------------------------------------------------------------------[<]-
    Function: put_ggcode_evaluate

    Synopsis: Formats a evaluate message, allocates a new buffer,
    and returns the formatted message in the buffer.  You should free the
    buffer using mem_free() when finished.  Returns the size of the buffer
    in bytes.
    ---------------------------------------------------------------------[>]-*/

int
put_ggcode_evaluate (
    byte **_buffer,
    const char *expression)             /*  GSL command line                 */
{
    struct_ggcode_evaluate
        *_struct_ptr;
    int
        _total_size = sizeof (struct_ggcode_evaluate);
    char
        *_ptr;

    _total_size += expression ? strlen (expression) + 1 : 0;
    _struct_ptr = mem_alloc (_total_size);
    *_buffer = (byte *) _struct_ptr;
    if (_struct_ptr)
      {
        _ptr = (char *) _struct_ptr + sizeof (struct_ggcode_evaluate);
        if (expression)
          {
            _struct_ptr-> expression        = (char *) _ptr;
            strcpy ((char *) _ptr, expression);
            _ptr += strlen (expression) + 1;
          }
        else
            _struct_ptr-> expression        = NULL;

        return _total_size;
      }
    else
        return 0;
}


/*  ---------------------------------------------------------------------[<]-
    Function: get_ggcode_evaluate

    Synopsis: Accepts a buffer containing a evaluate message,
    and unpacks it into a new struct_ggcode_evaluate structure. Free the
    structure using free_ggcode_evaluate() when finished.
    ---------------------------------------------------------------------[>]-*/

int
get_ggcode_evaluate (
    byte *_buffer,
    struct_ggcode_evaluate **params)
{
    struct_ggcode_evaluate
        *_struct_ptr;
    char
        *_ptr;

    _struct_ptr = (struct_ggcode_evaluate *) _buffer;
    *params = mem_alloc (sizeof (struct_ggcode_evaluate));
    if (*params)
      {
        _ptr = (char *) _struct_ptr + sizeof (struct_ggcode_evaluate);
        if (_struct_ptr-> expression)
          {
            (* params)-> expression         = mem_strdup (_ptr);
            _ptr += strlen ((* params)-> expression) + 1;
          }
        else
            (* params)-> expression         = NULL;
        return 0;
      }
    else
        return -1;
}


/*  ---------------------------------------------------------------------[<]-
    Function: free_ggcode_evaluate

    Synopsis: frees a structure allocated by get_ggcode_evaluate().
    ---------------------------------------------------------------------[>]-*/

void
free_ggcode_evaluate (
    struct_ggcode_evaluate **params)
{
    mem_free ((*params)-> expression);
    mem_free (*params);
    *params = NULL;
}

char *GGCODE_EVALUATE = "GGCODE EVALUATE";

/*  ---------------------------------------------------------------------[<]-
    Function: lsend_ggcode_evaluate

    Synopsis: Sends a evaluate - Execute command
              event to the ggcode agent
    ---------------------------------------------------------------------[>]-*/

int 
lsend_ggcode_evaluate (
    const QID  *_to,
    const QID  *_from,
          char *_accept,
          char *_reject,
          char *_expire,
          word _timeout,
    const char *expression)         /*  GSL command line                 */
{
    byte *_body;
    int   _size,
          _rc;
    _size = put_ggcode_evaluate
                (&_body,
                 expression);
    if (_size)
      {
        _rc = event_send (_to, _from, GGCODE_EVALUATE,
                          _body, _size,
                          _accept, _reject, _expire, _timeout);
        mem_free (_body);
        return _rc;
      }
    else
        return -1;
}

char *GGCODE_CALL = "GGCODE CALL";


/*  ---------------------------------------------------------------------[<]-
    Function: put_ggcode_job

    Synopsis: Formats a job message, allocates a new buffer,
    and returns the formatted message in the buffer.  You should free the
    buffer using mem_free() when finished.  Returns the size of the buffer
    in bytes.
    ---------------------------------------------------------------------[>]-*/

int
put_ggcode_job (
    byte **_buffer,
    const void *job)                    /*  Job id                           */
{
    struct_ggcode_job
        *_struct_ptr;

    _struct_ptr = mem_alloc (sizeof (struct_ggcode_job));
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
    Function: get_ggcode_job

    Synopsis: Accepts a buffer containing a job message,
    and unpacks it into a new struct_ggcode_job structure. Free the
    structure using free_ggcode_job() when finished.
    ---------------------------------------------------------------------[>]-*/

int
get_ggcode_job (
    byte *_buffer,
    struct_ggcode_job **params)
{
    struct_ggcode_job
        *_struct_ptr;

    _struct_ptr = (struct_ggcode_job *) _buffer;
    *params = mem_alloc (sizeof (struct_ggcode_job));
    if (*params)
      {
        (* params)-> job                = _struct_ptr-> job;
        return 0;
      }
    else
        return -1;
}


/*  ---------------------------------------------------------------------[<]-
    Function: free_ggcode_job

    Synopsis: frees a structure allocated by get_ggcode_job().
    ---------------------------------------------------------------------[>]-*/

void
free_ggcode_job (
    struct_ggcode_job **params)
{
    mem_free (*params);
    *params = NULL;
}

char *GGCODE_OK = "GGCODE OK";

/*  ---------------------------------------------------------------------[<]-
    Function: lsend_ggcode_ok

    Synopsis: Sends a ok - OK reply
              event to the ggcode agent
    ---------------------------------------------------------------------[>]-*/

int 
lsend_ggcode_ok (
    const QID  *_to,
    const QID  *_from,
          char *_accept,
          char *_reject,
          char *_expire,
          word _timeout,
    const void *job)                /*  Job id                           */
{
    byte *_body;
    int   _size,
          _rc;
    _size = put_ggcode_job
                (&_body,
                 job);
    if (_size)
      {
        _rc = event_send (_to, _from, GGCODE_OK,
                          _body, _size,
                          _accept, _reject, _expire, _timeout);
        mem_free (_body);
        return _rc;
      }
    else
        return -1;
}


/*  ---------------------------------------------------------------------[<]-
    Function: put_ggcode_error_reply

    Synopsis: Formats a error reply message, allocates a new buffer,
    and returns the formatted message in the buffer.  You should free the
    buffer using mem_free() when finished.  Returns the size of the buffer
    in bytes.
    ---------------------------------------------------------------------[>]-*/

int
put_ggcode_error_reply (
    byte **_buffer,
    const void *job,                    /*  Job id                           */
    const char *error_name,             /*  Error file name                  */
    const qbyte error_line,             /*  Error line                       */
    const char *error_text)             /*  Error message text               */
{
    struct_ggcode_error_reply
        *_struct_ptr;
    int
        _total_size = sizeof (struct_ggcode_error_reply);
    char
        *_ptr;

    _total_size += error_name ? strlen (error_name) + 1 : 0;
    _total_size += error_text ? strlen (error_text) + 1 : 0;
    _struct_ptr = mem_alloc (_total_size);
    *_buffer = (byte *) _struct_ptr;
    if (_struct_ptr)
      {
        _ptr = (char *) _struct_ptr + sizeof (struct_ggcode_error_reply);
        _struct_ptr-> job               = (void *) job;
        if (error_name)
          {
            _struct_ptr-> error_name        = (char *) _ptr;
            strcpy ((char *) _ptr, error_name);
            _ptr += strlen (error_name) + 1;
          }
        else
            _struct_ptr-> error_name        = NULL;
        _struct_ptr-> error_line        = error_line;
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
    Function: get_ggcode_error_reply

    Synopsis: Accepts a buffer containing a error reply message,
    and unpacks it into a new struct_ggcode_error_reply structure. Free the
    structure using free_ggcode_error_reply() when finished.
    ---------------------------------------------------------------------[>]-*/

int
get_ggcode_error_reply (
    byte *_buffer,
    struct_ggcode_error_reply **params)
{
    struct_ggcode_error_reply
        *_struct_ptr;
    char
        *_ptr;

    _struct_ptr = (struct_ggcode_error_reply *) _buffer;
    *params = mem_alloc (sizeof (struct_ggcode_error_reply));
    if (*params)
      {
        _ptr = (char *) _struct_ptr + sizeof (struct_ggcode_error_reply);
        (* params)-> job                = _struct_ptr-> job;
        if (_struct_ptr-> error_name)
          {
            (* params)-> error_name         = mem_strdup (_ptr);
            _ptr += strlen ((* params)-> error_name) + 1;
          }
        else
            (* params)-> error_name         = NULL;
        (* params)-> error_line         = _struct_ptr-> error_line;
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
    Function: free_ggcode_error_reply

    Synopsis: frees a structure allocated by get_ggcode_error_reply().
    ---------------------------------------------------------------------[>]-*/

void
free_ggcode_error_reply (
    struct_ggcode_error_reply **params)
{
    mem_free ((*params)-> error_name);
    mem_free ((*params)-> error_text);
    mem_free (*params);
    *params = NULL;
}

char *GGCODE_MESSAGE = "GGCODE MESSAGE";

/*  ---------------------------------------------------------------------[<]-
    Function: lsend_ggcode_message

    Synopsis: Sends a message - Error reply
              event to the ggcode agent
    ---------------------------------------------------------------------[>]-*/

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
    const char *error_text)         /*  Error message text               */
{
    byte *_body;
    int   _size,
          _rc;
    _size = put_ggcode_error_reply
                (&_body,
                 job,
                 error_name,
                 error_line,
                 error_text);
    if (_size)
      {
        _rc = event_send (_to, _from, GGCODE_MESSAGE,
                          _body, _size,
                          _accept, _reject, _expire, _timeout);
        mem_free (_body);
        return _rc;
      }
    else
        return -1;
}

char *GGCODE_ERROR = "GGCODE ERROR";

/*  ---------------------------------------------------------------------[<]-
    Function: lsend_ggcode_error

    Synopsis: Sends a error - Error reply
              event to the ggcode agent
    ---------------------------------------------------------------------[>]-*/

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
    const char *error_text)         /*  Error message text               */
{
    byte *_body;
    int   _size,
          _rc;
    _size = put_ggcode_error_reply
                (&_body,
                 job,
                 error_name,
                 error_line,
                 error_text);
    if (_size)
      {
        _rc = event_send (_to, _from, GGCODE_ERROR,
                          _body, _size,
                          _accept, _reject, _expire, _timeout);
        mem_free (_body);
        return _rc;
      }
    else
        return -1;
}

char *GGCODE_FATAL = "GGCODE FATAL";

/*  ---------------------------------------------------------------------[<]-
    Function: lsend_ggcode_fatal

    Synopsis: Sends a fatal - Fatal reply
              event to the ggcode agent
    ---------------------------------------------------------------------[>]-*/

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
    const char *error_text)         /*  Error message text               */
{
    byte *_body;
    int   _size,
          _rc;
    _size = put_ggcode_error_reply
                (&_body,
                 job,
                 error_name,
                 error_line,
                 error_text);
    if (_size)
      {
        _rc = event_send (_to, _from, GGCODE_FATAL,
                          _body, _size,
                          _accept, _reject, _expire, _timeout);
        mem_free (_body);
        return _rc;
      }
    else
        return -1;
}

char *GGCODE_CALL_OK = "GGCODE CALL OK";


/*  ---------------------------------------------------------------------[<]-
    Function: put_ggcode_call_error

    Synopsis: Formats a call error message, allocates a new buffer,
    and returns the formatted message in the buffer.  You should free the
    buffer using mem_free() when finished.  Returns the size of the buffer
    in bytes.
    ---------------------------------------------------------------------[>]-*/

int
put_ggcode_call_error (
    byte **_buffer,
    const char *error_name,             /*  Error file name                  */
    const qbyte error_line,             /*  Error line                       */
    const char *error_text)             /*  Error message text               */
{
    struct_ggcode_call_error
        *_struct_ptr;
    int
        _total_size = sizeof (struct_ggcode_call_error);
    char
        *_ptr;

    _total_size += error_name ? strlen (error_name) + 1 : 0;
    _total_size += error_text ? strlen (error_text) + 1 : 0;
    _struct_ptr = mem_alloc (_total_size);
    *_buffer = (byte *) _struct_ptr;
    if (_struct_ptr)
      {
        _ptr = (char *) _struct_ptr + sizeof (struct_ggcode_call_error);
        if (error_name)
          {
            _struct_ptr-> error_name        = (char *) _ptr;
            strcpy ((char *) _ptr, error_name);
            _ptr += strlen (error_name) + 1;
          }
        else
            _struct_ptr-> error_name        = NULL;
        _struct_ptr-> error_line        = error_line;
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
    Function: get_ggcode_call_error

    Synopsis: Accepts a buffer containing a call error message,
    and unpacks it into a new struct_ggcode_call_error structure. Free the
    structure using free_ggcode_call_error() when finished.
    ---------------------------------------------------------------------[>]-*/

int
get_ggcode_call_error (
    byte *_buffer,
    struct_ggcode_call_error **params)
{
    struct_ggcode_call_error
        *_struct_ptr;
    char
        *_ptr;

    _struct_ptr = (struct_ggcode_call_error *) _buffer;
    *params = mem_alloc (sizeof (struct_ggcode_call_error));
    if (*params)
      {
        _ptr = (char *) _struct_ptr + sizeof (struct_ggcode_call_error);
        if (_struct_ptr-> error_name)
          {
            (* params)-> error_name         = mem_strdup (_ptr);
            _ptr += strlen ((* params)-> error_name) + 1;
          }
        else
            (* params)-> error_name         = NULL;
        (* params)-> error_line         = _struct_ptr-> error_line;
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
    Function: free_ggcode_call_error

    Synopsis: frees a structure allocated by get_ggcode_call_error().
    ---------------------------------------------------------------------[>]-*/

void
free_ggcode_call_error (
    struct_ggcode_call_error **params)
{
    mem_free ((*params)-> error_name);
    mem_free ((*params)-> error_text);
    mem_free (*params);
    *params = NULL;
}

char *GGCODE_CALL_MESSAGE = "GGCODE CALL MESSAGE";

/*  ---------------------------------------------------------------------[<]-
    Function: lsend_ggcode_call_message

    Synopsis: Sends a call message - Message reply
              event to the ggcode agent
    ---------------------------------------------------------------------[>]-*/

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
    const char *error_text)         /*  Error message text               */
{
    byte *_body;
    int   _size,
          _rc;
    _size = put_ggcode_call_error
                (&_body,
                 error_name,
                 error_line,
                 error_text);
    if (_size)
      {
        _rc = event_send (_to, _from, GGCODE_CALL_MESSAGE,
                          _body, _size,
                          _accept, _reject, _expire, _timeout);
        mem_free (_body);
        return _rc;
      }
    else
        return -1;
}

char *GGCODE_CALL_ERROR = "GGCODE CALL ERROR";

/*  ---------------------------------------------------------------------[<]-
    Function: lsend_ggcode_call_error

    Synopsis: Sends a call error - Error reply
              event to the ggcode agent
    ---------------------------------------------------------------------[>]-*/

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
    const char *error_text)         /*  Error message text               */
{
    byte *_body;
    int   _size,
          _rc;
    _size = put_ggcode_call_error
                (&_body,
                 error_name,
                 error_line,
                 error_text);
    if (_size)
      {
        _rc = event_send (_to, _from, GGCODE_CALL_ERROR,
                          _body, _size,
                          _accept, _reject, _expire, _timeout);
        mem_free (_body);
        return _rc;
      }
    else
        return -1;
}


