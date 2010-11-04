/*---------------------------------------------------------------------------
 *  smtmsg.c - functions for SMT standard messages.
 *
 *  Generated from smtmsg.xml by smtmesg.gsl using GSL
 *  DO NOT MODIFY THIS FILE.
 *
 *  For documentation and updates see http://www.imatix.com.
 *---------------------------------------------------------------------------*/

#include "sfl.h"                        /*  SFL header file                  */
#include "smtlib.h"                     /*  SMT header file                  */
#include "smtmsg.h"                     /*  Definitions & prototypes         */

/*---------------------------------------------------------------------------
 *  Message functions for smtlib - SMT kernel.
 *---------------------------------------------------------------------------*/


/*  ---------------------------------------------------------------------[<]-
    Function: put_smtlib_shutdown

    Synopsis: Formats a shutdown message, allocates a new buffer,
    and returns the formatted message in the buffer.  You should free the
    buffer using mem_free() when finished.  Returns the size of the buffer
    in bytes.
    ---------------------------------------------------------------------[>]-*/

int
put_smtlib_shutdown (
    byte **_buffer,
    const word  signal)                 /*  Signal that provoked shutdown    */
{
    struct_smtlib_shutdown
        *_struct_ptr;

    _struct_ptr = mem_alloc (sizeof (struct_smtlib_shutdown));
    *_buffer = (byte *) _struct_ptr;
    if (_struct_ptr)
      {
        _struct_ptr-> signal            = signal;

        return sizeof (*_struct_ptr);
      }
    else
        return 0;
}


/*  ---------------------------------------------------------------------[<]-
    Function: get_smtlib_shutdown

    Synopsis: Accepts a buffer containing a shutdown message,
    and unpacks it into a new struct_smtlib_shutdown structure. Free the
    structure using free_smtlib_shutdown() when finished.
    ---------------------------------------------------------------------[>]-*/

int
get_smtlib_shutdown (
    byte *_buffer,
    struct_smtlib_shutdown **params)
{
    struct_smtlib_shutdown
        *_struct_ptr;

    _struct_ptr = (struct_smtlib_shutdown *) _buffer;
    *params = mem_alloc (sizeof (struct_smtlib_shutdown));
    if (*params)
      {
        (* params)-> signal             = _struct_ptr-> signal;
        return 0;
      }
    else
        return -1;
}


/*  ---------------------------------------------------------------------[<]-
    Function: free_smtlib_shutdown

    Synopsis: frees a structure allocated by get_smtlib_shutdown().
    ---------------------------------------------------------------------[>]-*/

void
free_smtlib_shutdown (
    struct_smtlib_shutdown **params)
{
    mem_free (*params);
    *params = NULL;
}

char *SMTLIB_SHUTDOWN = "SMTLIB SHUTDOWN";

/*  ---------------------------------------------------------------------[<]-
    Function: lsend_smtlib_shutdown

    Synopsis: Sends a shutdown -
              event to the smtlib agent
    ---------------------------------------------------------------------[>]-*/

int
lsend_smtlib_shutdown (
    const QID  *_to,
    const QID  *_from,
          char *_accept,
          char *_reject,
          char *_expire,
          word _timeout,
    const word  signal)             /*  Signal that provoked shutdown    */
{
    byte *_body;
    int   _size,
          _rc;
    _size = put_smtlib_shutdown
                (&_body,
                 signal);
    if (_size)
      {
        _rc = event_send (_to, _from, SMTLIB_SHUTDOWN,
                          _body, _size,
                          _accept, _reject, _expire, _timeout);
        mem_free (_body);
        return _rc;
      }
    else
        return -1;
}

/*---------------------------------------------------------------------------
 *  Message functions for smtlog - SMT Log agent.
 *---------------------------------------------------------------------------*/

char *SMTLOG_PLAIN = "SMTLOG PLAIN";

char *SMTLOG_STAMP = "SMTLOG STAMP";

char *SMTLOG_CLOSE = "SMTLOG CLOSE";


/*  ---------------------------------------------------------------------[<]-
    Function: put_smtlog_filename

    Synopsis: Formats a filename message, allocates a new buffer,
    and returns the formatted message in the buffer.  You should free the
    buffer using mem_free() when finished.  Returns the size of the buffer
    in bytes.
    ---------------------------------------------------------------------[>]-*/

int
put_smtlog_filename (
    byte **_buffer,
    const char *filename)               /*                                   */
{
    struct_smtlog_filename
        *_struct_ptr;
    int
        _total_size = sizeof (struct_smtlog_filename);
    char
        *_ptr;

    _total_size += filename ? strlen (filename) + 1 : 0;
    _struct_ptr = mem_alloc (_total_size);
    *_buffer = (byte *) _struct_ptr;
    if (_struct_ptr)
      {
        _ptr = (char *) _struct_ptr + sizeof (struct_smtlog_filename);
        if (filename)
          {
            _struct_ptr-> filename          = (char *) _ptr;
            strcpy ((char *) _ptr, filename);
            _ptr += strlen (filename) + 1;
          }
        else
            _struct_ptr-> filename          = NULL;

        return _total_size;
      }
    else
        return 0;
}


/*  ---------------------------------------------------------------------[<]-
    Function: get_smtlog_filename

    Synopsis: Accepts a buffer containing a filename message,
    and unpacks it into a new struct_smtlog_filename structure. Free the
    structure using free_smtlog_filename() when finished.
    ---------------------------------------------------------------------[>]-*/

int
get_smtlog_filename (
    byte *_buffer,
    struct_smtlog_filename **params)
{
    struct_smtlog_filename
        *_struct_ptr;
    char
        *_ptr;

    _struct_ptr = (struct_smtlog_filename *) _buffer;
    *params = mem_alloc (sizeof (struct_smtlog_filename));
    if (*params)
      {
        _ptr = (char *) _struct_ptr + sizeof (struct_smtlog_filename);
        if (_struct_ptr-> filename)
          {
            (* params)-> filename           = mem_strdup (_ptr);
            _ptr += strlen ((* params)-> filename) + 1;
          }
        else
            (* params)-> filename           = NULL;
        return 0;
      }
    else
        return -1;
}


/*  ---------------------------------------------------------------------[<]-
    Function: free_smtlog_filename

    Synopsis: frees a structure allocated by get_smtlog_filename().
    ---------------------------------------------------------------------[>]-*/

void
free_smtlog_filename (
    struct_smtlog_filename **params)
{
    mem_free ((*params)-> filename);
    mem_free (*params);
    *params = NULL;
}

char *SMTLOG_CYCLE = "SMTLOG CYCLE";

/*  ---------------------------------------------------------------------[<]-
    Function: lsend_smtlog_cycle

    Synopsis: Sends a cycle -
              event to the smtlog agent
    ---------------------------------------------------------------------[>]-*/

int
lsend_smtlog_cycle (
    const QID  *_to,
    const QID  *_from,
          char *_accept,
          char *_reject,
          char *_expire,
          word _timeout,
    const char *filename)           /*                                   */
{
    byte *_body;
    int   _size,
          _rc;
    _size = put_smtlog_filename
                (&_body,
                 filename);
    if (_size)
      {
        _rc = event_send (_to, _from, SMTLOG_CYCLE,
                          _body, _size,
                          _accept, _reject, _expire, _timeout);
        mem_free (_body);
        return _rc;
      }
    else
        return -1;
}

char *SMTLOG_OPEN = "SMTLOG OPEN";

/*  ---------------------------------------------------------------------[<]-
    Function: lsend_smtlog_open

    Synopsis: Sends a open -
              event to the smtlog agent
    ---------------------------------------------------------------------[>]-*/

int
lsend_smtlog_open (
    const QID  *_to,
    const QID  *_from,
          char *_accept,
          char *_reject,
          char *_expire,
          word _timeout,
    const char *filename)           /*                                   */
{
    byte *_body;
    int   _size,
          _rc;
    _size = put_smtlog_filename
                (&_body,
                 filename);
    if (_size)
      {
        _rc = event_send (_to, _from, SMTLOG_OPEN,
                          _body, _size,
                          _accept, _reject, _expire, _timeout);
        mem_free (_body);
        return _rc;
      }
    else
        return -1;
}

char *SMTLOG_APPEND = "SMTLOG APPEND";

/*  ---------------------------------------------------------------------[<]-
    Function: lsend_smtlog_append

    Synopsis: Sends a append -
              event to the smtlog agent
    ---------------------------------------------------------------------[>]-*/

int
lsend_smtlog_append (
    const QID  *_to,
    const QID  *_from,
          char *_accept,
          char *_reject,
          char *_expire,
          word _timeout,
    const char *filename)           /*                                   */
{
    byte *_body;
    int   _size,
          _rc;
    _size = put_smtlog_filename
                (&_body,
                 filename);
    if (_size)
      {
        _rc = event_send (_to, _from, SMTLOG_APPEND,
                          _body, _size,
                          _accept, _reject, _expire, _timeout);
        mem_free (_body);
        return _rc;
      }
    else
        return -1;
}


/*  ---------------------------------------------------------------------[<]-
    Function: put_smtlog_text

    Synopsis: Formats a text message, allocates a new buffer,
    and returns the formatted message in the buffer.  You should free the
    buffer using mem_free() when finished.  Returns the size of the buffer
    in bytes.
    ---------------------------------------------------------------------[>]-*/

int
put_smtlog_text (
    byte **_buffer,
    const char *text)                   /*                                   */
{
    struct_smtlog_text
        *_struct_ptr;
    int
        _total_size = sizeof (struct_smtlog_text);
    char
        *_ptr;

    _total_size += text ? strlen (text) + 1 : 0;
    _struct_ptr = mem_alloc (_total_size);
    *_buffer = (byte *) _struct_ptr;
    if (_struct_ptr)
      {
        _ptr = (char *) _struct_ptr + sizeof (struct_smtlog_text);
        if (text)
          {
            _struct_ptr-> text              = (char *) _ptr;
            strcpy ((char *) _ptr, text);
            _ptr += strlen (text) + 1;
          }
        else
            _struct_ptr-> text              = NULL;

        return _total_size;
      }
    else
        return 0;
}


/*  ---------------------------------------------------------------------[<]-
    Function: get_smtlog_text

    Synopsis: Accepts a buffer containing a text message,
    and unpacks it into a new struct_smtlog_text structure. Free the
    structure using free_smtlog_text() when finished.
    ---------------------------------------------------------------------[>]-*/

int
get_smtlog_text (
    byte *_buffer,
    struct_smtlog_text **params)
{
    struct_smtlog_text
        *_struct_ptr;
    char
        *_ptr;

    _struct_ptr = (struct_smtlog_text *) _buffer;
    *params = mem_alloc (sizeof (struct_smtlog_text));
    if (*params)
      {
        _ptr = (char *) _struct_ptr + sizeof (struct_smtlog_text);
        if (_struct_ptr-> text)
          {
            (* params)-> text               = mem_strdup (_ptr);
            _ptr += strlen ((* params)-> text) + 1;
          }
        else
            (* params)-> text               = NULL;
        return 0;
      }
    else
        return -1;
}


/*  ---------------------------------------------------------------------[<]-
    Function: free_smtlog_text

    Synopsis: frees a structure allocated by get_smtlog_text().
    ---------------------------------------------------------------------[>]-*/

void
free_smtlog_text (
    struct_smtlog_text **params)
{
    mem_free ((*params)-> text);
    mem_free (*params);
    *params = NULL;
}

char *SMTLOG_PUT = "SMTLOG PUT";

/*  ---------------------------------------------------------------------[<]-
    Function: lsend_smtlog_put

    Synopsis: Sends a put -
              event to the smtlog agent
    ---------------------------------------------------------------------[>]-*/

int
lsend_smtlog_put (
    const QID  *_to,
    const QID  *_from,
          char *_accept,
          char *_reject,
          char *_expire,
          word _timeout,
    const char *text)               /*                                   */
{
    byte *_body;
    int   _size,
          _rc;
    _size = put_smtlog_text
                (&_body,
                 text);
    if (_size)
      {
        _rc = event_send (_to, _from, SMTLOG_PUT,
                          _body, _size,
                          _accept, _reject, _expire, _timeout);
        mem_free (_body);
        return _rc;
      }
    else
        return -1;
}

/*---------------------------------------------------------------------------
 *  Message functions for smtxlog - SMT Extended Logging Agent.
 *---------------------------------------------------------------------------*/


/*  ---------------------------------------------------------------------[<]-
    Function: put_smtxlog_open

    Synopsis: Formats a open message, allocates a new buffer,
    and returns the formatted message in the buffer.  You should free the
    buffer using mem_free() when finished.  Returns the size of the buffer
    in bytes.
    ---------------------------------------------------------------------[>]-*/

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
    const char *cycle_argument)         /*  For other cycle methods          */
{
    struct_smtxlog_open
        *_struct_ptr;
    int
        _total_size = sizeof (struct_smtxlog_open);
    char
        *_ptr;

    _total_size += log_path ? strlen (log_path) + 1 : 0;
    _total_size += log_file ? strlen (log_file) + 1 : 0;
    _total_size += log_format ? strlen (log_format) + 1 : 0;
    _total_size += cycle_when ? strlen (cycle_when) + 1 : 0;
    _total_size += cycle_how ? strlen (cycle_how) + 1 : 0;
    _total_size += cycle_time ? strlen (cycle_time) + 1 : 0;
    _total_size += cycle_date ? strlen (cycle_date) + 1 : 0;
    _total_size += cycle_size ? strlen (cycle_size) + 1 : 0;
    _total_size += cycle_lines ? strlen (cycle_lines) + 1 : 0;
    _total_size += cycle_argument ? strlen (cycle_argument) + 1 : 0;
    _struct_ptr = mem_alloc (_total_size);
    *_buffer = (byte *) _struct_ptr;
    if (_struct_ptr)
      {
        _ptr = (char *) _struct_ptr + sizeof (struct_smtxlog_open);
        if (log_path)
          {
            _struct_ptr-> log_path          = (char *) _ptr;
            strcpy ((char *) _ptr, log_path);
            _ptr += strlen (log_path) + 1;
          }
        else
            _struct_ptr-> log_path          = NULL;
        if (log_file)
          {
            _struct_ptr-> log_file          = (char *) _ptr;
            strcpy ((char *) _ptr, log_file);
            _ptr += strlen (log_file) + 1;
          }
        else
            _struct_ptr-> log_file          = NULL;
        if (log_format)
          {
            _struct_ptr-> log_format        = (char *) _ptr;
            strcpy ((char *) _ptr, log_format);
            _ptr += strlen (log_format) + 1;
          }
        else
            _struct_ptr-> log_format        = NULL;
        if (cycle_when)
          {
            _struct_ptr-> cycle_when        = (char *) _ptr;
            strcpy ((char *) _ptr, cycle_when);
            _ptr += strlen (cycle_when) + 1;
          }
        else
            _struct_ptr-> cycle_when        = NULL;
        if (cycle_how)
          {
            _struct_ptr-> cycle_how         = (char *) _ptr;
            strcpy ((char *) _ptr, cycle_how);
            _ptr += strlen (cycle_how) + 1;
          }
        else
            _struct_ptr-> cycle_how         = NULL;
        if (cycle_time)
          {
            _struct_ptr-> cycle_time        = (char *) _ptr;
            strcpy ((char *) _ptr, cycle_time);
            _ptr += strlen (cycle_time) + 1;
          }
        else
            _struct_ptr-> cycle_time        = NULL;
        if (cycle_date)
          {
            _struct_ptr-> cycle_date        = (char *) _ptr;
            strcpy ((char *) _ptr, cycle_date);
            _ptr += strlen (cycle_date) + 1;
          }
        else
            _struct_ptr-> cycle_date        = NULL;
        if (cycle_size)
          {
            _struct_ptr-> cycle_size        = (char *) _ptr;
            strcpy ((char *) _ptr, cycle_size);
            _ptr += strlen (cycle_size) + 1;
          }
        else
            _struct_ptr-> cycle_size        = NULL;
        if (cycle_lines)
          {
            _struct_ptr-> cycle_lines       = (char *) _ptr;
            strcpy ((char *) _ptr, cycle_lines);
            _ptr += strlen (cycle_lines) + 1;
          }
        else
            _struct_ptr-> cycle_lines       = NULL;
        if (cycle_argument)
          {
            _struct_ptr-> cycle_argument    = (char *) _ptr;
            strcpy ((char *) _ptr, cycle_argument);
            _ptr += strlen (cycle_argument) + 1;
          }
        else
            _struct_ptr-> cycle_argument    = NULL;

        return _total_size;
      }
    else
        return 0;
}


/*  ---------------------------------------------------------------------[<]-
    Function: get_smtxlog_open

    Synopsis: Accepts a buffer containing a open message,
    and unpacks it into a new struct_smtxlog_open structure. Free the
    structure using free_smtxlog_open() when finished.
    ---------------------------------------------------------------------[>]-*/

int
get_smtxlog_open (
    byte *_buffer,
    struct_smtxlog_open **params)
{
    struct_smtxlog_open
        *_struct_ptr;
    char
        *_ptr;

    _struct_ptr = (struct_smtxlog_open *) _buffer;
    *params = mem_alloc (sizeof (struct_smtxlog_open));
    if (*params)
      {
        _ptr = (char *) _struct_ptr + sizeof (struct_smtxlog_open);
        if (_struct_ptr-> log_path)
          {
            (* params)-> log_path           = mem_strdup (_ptr);
            _ptr += strlen ((* params)-> log_path) + 1;
          }
        else
            (* params)-> log_path           = NULL;
        if (_struct_ptr-> log_file)
          {
            (* params)-> log_file           = mem_strdup (_ptr);
            _ptr += strlen ((* params)-> log_file) + 1;
          }
        else
            (* params)-> log_file           = NULL;
        if (_struct_ptr-> log_format)
          {
            (* params)-> log_format         = mem_strdup (_ptr);
            _ptr += strlen ((* params)-> log_format) + 1;
          }
        else
            (* params)-> log_format         = NULL;
        if (_struct_ptr-> cycle_when)
          {
            (* params)-> cycle_when         = mem_strdup (_ptr);
            _ptr += strlen ((* params)-> cycle_when) + 1;
          }
        else
            (* params)-> cycle_when         = NULL;
        if (_struct_ptr-> cycle_how)
          {
            (* params)-> cycle_how          = mem_strdup (_ptr);
            _ptr += strlen ((* params)-> cycle_how) + 1;
          }
        else
            (* params)-> cycle_how          = NULL;
        if (_struct_ptr-> cycle_time)
          {
            (* params)-> cycle_time         = mem_strdup (_ptr);
            _ptr += strlen ((* params)-> cycle_time) + 1;
          }
        else
            (* params)-> cycle_time         = NULL;
        if (_struct_ptr-> cycle_date)
          {
            (* params)-> cycle_date         = mem_strdup (_ptr);
            _ptr += strlen ((* params)-> cycle_date) + 1;
          }
        else
            (* params)-> cycle_date         = NULL;
        if (_struct_ptr-> cycle_size)
          {
            (* params)-> cycle_size         = mem_strdup (_ptr);
            _ptr += strlen ((* params)-> cycle_size) + 1;
          }
        else
            (* params)-> cycle_size         = NULL;
        if (_struct_ptr-> cycle_lines)
          {
            (* params)-> cycle_lines        = mem_strdup (_ptr);
            _ptr += strlen ((* params)-> cycle_lines) + 1;
          }
        else
            (* params)-> cycle_lines        = NULL;
        if (_struct_ptr-> cycle_argument)
          {
            (* params)-> cycle_argument     = mem_strdup (_ptr);
            _ptr += strlen ((* params)-> cycle_argument) + 1;
          }
        else
            (* params)-> cycle_argument     = NULL;
        return 0;
      }
    else
        return -1;
}


/*  ---------------------------------------------------------------------[<]-
    Function: free_smtxlog_open

    Synopsis: frees a structure allocated by get_smtxlog_open().
    ---------------------------------------------------------------------[>]-*/

void
free_smtxlog_open (
    struct_smtxlog_open **params)
{
    mem_free ((*params)-> log_path);
    mem_free ((*params)-> log_file);
    mem_free ((*params)-> log_format);
    mem_free ((*params)-> cycle_when);
    mem_free ((*params)-> cycle_how);
    mem_free ((*params)-> cycle_time);
    mem_free ((*params)-> cycle_date);
    mem_free ((*params)-> cycle_size);
    mem_free ((*params)-> cycle_lines);
    mem_free ((*params)-> cycle_argument);
    mem_free (*params);
    *params = NULL;
}

char *SMTXLOG_OPEN = "SMTXLOG OPEN";

/*  ---------------------------------------------------------------------[<]-
    Function: lsend_smtxlog_open

    Synopsis: Sends a open -
              event to the smtxlog agent
    ---------------------------------------------------------------------[>]-*/

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
    const char *cycle_argument)     /*  For other cycle methods          */
{
    byte *_body;
    int   _size,
          _rc;
    _size = put_smtxlog_open
                (&_body,
                 log_path,
                 log_file,
                 log_format,
                 cycle_when,
                 cycle_how,
                 cycle_time,
                 cycle_date,
                 cycle_size,
                 cycle_lines,
                 cycle_argument);
    if (_size)
      {
        _rc = event_send (_to, _from, SMTXLOG_OPEN,
                          _body, _size,
                          _accept, _reject, _expire, _timeout);
        mem_free (_body);
        return _rc;
      }
    else
        return -1;
}


/*  ---------------------------------------------------------------------[<]-
    Function: put_smtxlog_log

    Synopsis: Formats a log message, allocates a new buffer,
    and returns the formatted message in the buffer.  You should free the
    buffer using mem_free() when finished.  Returns the size of the buffer
    in bytes.
    ---------------------------------------------------------------------[>]-*/

int
put_smtxlog_log (
    byte **_buffer,
    const char *file_name,              /*  Filename used for request        */
    const word  value_size,             /*  Value size                       */
    const void *value)                  /*  Value to log                     */
{
    struct_smtxlog_log
        *_struct_ptr;
    int
        _total_size = sizeof (struct_smtxlog_log);
    char
        *_ptr;

    _total_size += file_name ? strlen (file_name) + 1 : 0;
    _total_size += value_size;
    _struct_ptr = mem_alloc (_total_size);
    *_buffer = (byte *) _struct_ptr;
    if (_struct_ptr)
      {
        _ptr = (char *) _struct_ptr + sizeof (struct_smtxlog_log);
        if (file_name)
          {
            _struct_ptr-> file_name         = (char *) _ptr;
            strcpy ((char *) _ptr, file_name);
            _ptr += strlen (file_name) + 1;
          }
        else
            _struct_ptr-> file_name         = NULL;
        _struct_ptr-> value_size        = value_size;
        _struct_ptr-> value             = (byte *) _ptr;
        memcpy (_ptr, value, value_size);
        _ptr += value_size;

        return _total_size;
      }
    else
        return 0;
}


/*  ---------------------------------------------------------------------[<]-
    Function: get_smtxlog_log

    Synopsis: Accepts a buffer containing a log message,
    and unpacks it into a new struct_smtxlog_log structure. Free the
    structure using free_smtxlog_log() when finished.
    ---------------------------------------------------------------------[>]-*/

int
get_smtxlog_log (
    byte *_buffer,
    struct_smtxlog_log **params)
{
    struct_smtxlog_log
        *_struct_ptr;
    char
        *_ptr;

    _struct_ptr = (struct_smtxlog_log *) _buffer;
    *params = mem_alloc (sizeof (struct_smtxlog_log));
    if (*params)
      {
        _ptr = (char *) _struct_ptr + sizeof (struct_smtxlog_log);
        if (_struct_ptr-> file_name)
          {
            (* params)-> file_name          = mem_strdup (_ptr);
            _ptr += strlen ((* params)-> file_name) + 1;
          }
        else
            (* params)-> file_name          = NULL;
        (* params)-> value_size         = _struct_ptr-> value_size;
        (* params)-> value              = mem_alloc (_struct_ptr-> value_size + 1);
        memcpy ((* params)-> value, _ptr, _struct_ptr-> value_size);
        *((byte *)(* params)-> value + _struct_ptr-> value_size) = 0;
        _ptr += _struct_ptr-> value_size;
        return 0;
      }
    else
        return -1;
}


/*  ---------------------------------------------------------------------[<]-
    Function: free_smtxlog_log

    Synopsis: frees a structure allocated by get_smtxlog_log().
    ---------------------------------------------------------------------[>]-*/

void
free_smtxlog_log (
    struct_smtxlog_log **params)
{
    mem_free ((*params)-> file_name);
    mem_free ((*params)-> value);
    mem_free (*params);
    *params = NULL;
}

char *SMTXLOG_LOG = "SMTXLOG LOG";

/*  ---------------------------------------------------------------------[<]-
    Function: lsend_smtxlog_log

    Synopsis: Sends a log -
              event to the smtxlog agent
    ---------------------------------------------------------------------[>]-*/

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
    const void *value)              /*  Value to log                     */
{
    byte *_body;
    int   _size,
          _rc;
    _size = put_smtxlog_log
                (&_body,
                 file_name,
                 value_size,
                 value);
    if (_size)
      {
        _rc = event_send (_to, _from, SMTXLOG_LOG,
                          _body, _size,
                          _accept, _reject, _expire, _timeout);
        mem_free (_body);
        return _rc;
      }
    else
        return -1;
}


/*  ---------------------------------------------------------------------[<]-
    Function: put_smtxlog_put

    Synopsis: Formats a put message, allocates a new buffer,
    and returns the formatted message in the buffer.  You should free the
    buffer using mem_free() when finished.  Returns the size of the buffer
    in bytes.
    ---------------------------------------------------------------------[>]-*/

int
put_smtxlog_put (
    byte **_buffer,
    const char *message)                /*  Line of text to log              */
{
    struct_smtxlog_put
        *_struct_ptr;
    int
        _total_size = sizeof (struct_smtxlog_put);
    char
        *_ptr;

    _total_size += message ? strlen (message) + 1 : 0;
    _struct_ptr = mem_alloc (_total_size);
    *_buffer = (byte *) _struct_ptr;
    if (_struct_ptr)
      {
        _ptr = (char *) _struct_ptr + sizeof (struct_smtxlog_put);
        if (message)
          {
            _struct_ptr-> message           = (char *) _ptr;
            strcpy ((char *) _ptr, message);
            _ptr += strlen (message) + 1;
          }
        else
            _struct_ptr-> message           = NULL;

        return _total_size;
      }
    else
        return 0;
}


/*  ---------------------------------------------------------------------[<]-
    Function: get_smtxlog_put

    Synopsis: Accepts a buffer containing a put message,
    and unpacks it into a new struct_smtxlog_put structure. Free the
    structure using free_smtxlog_put() when finished.
    ---------------------------------------------------------------------[>]-*/

int
get_smtxlog_put (
    byte *_buffer,
    struct_smtxlog_put **params)
{
    struct_smtxlog_put
        *_struct_ptr;
    char
        *_ptr;

    _struct_ptr = (struct_smtxlog_put *) _buffer;
    *params = mem_alloc (sizeof (struct_smtxlog_put));
    if (*params)
      {
        _ptr = (char *) _struct_ptr + sizeof (struct_smtxlog_put);
        if (_struct_ptr-> message)
          {
            (* params)-> message            = mem_strdup (_ptr);
            _ptr += strlen ((* params)-> message) + 1;
          }
        else
            (* params)-> message            = NULL;
        return 0;
      }
    else
        return -1;
}


/*  ---------------------------------------------------------------------[<]-
    Function: free_smtxlog_put

    Synopsis: frees a structure allocated by get_smtxlog_put().
    ---------------------------------------------------------------------[>]-*/

void
free_smtxlog_put (
    struct_smtxlog_put **params)
{
    mem_free ((*params)-> message);
    mem_free (*params);
    *params = NULL;
}

char *SMTXLOG_PUT = "SMTXLOG PUT";

/*  ---------------------------------------------------------------------[<]-
    Function: lsend_smtxlog_put

    Synopsis: Sends a put -
              event to the smtxlog agent
    ---------------------------------------------------------------------[>]-*/

int
lsend_smtxlog_put (
    const QID  *_to,
    const QID  *_from,
          char *_accept,
          char *_reject,
          char *_expire,
          word _timeout,
    const char *message)            /*  Line of text to log              */
{
    byte *_body;
    int   _size,
          _rc;
    _size = put_smtxlog_put
                (&_body,
                 message);
    if (_size)
      {
        _rc = event_send (_to, _from, SMTXLOG_PUT,
                          _body, _size,
                          _accept, _reject, _expire, _timeout);
        mem_free (_body);
        return _rc;
      }
    else
        return -1;
}

char *SMTXLOG_CYCLE = "SMTXLOG CYCLE";

char *SMTXLOG_CLEAR = "SMTXLOG CLEAR";

char *SMTXLOG_CLOSE = "SMTXLOG CLOSE";

/*---------------------------------------------------------------------------
 *  Message functions for smtoper - Operator agent.
 *---------------------------------------------------------------------------*/


/*  ---------------------------------------------------------------------[<]-
    Function: put_smtoper_set_log

    Synopsis: Formats a set log message, allocates a new buffer,
    and returns the formatted message in the buffer.  You should free the
    buffer using mem_free() when finished.  Returns the size of the buffer
    in bytes.
    ---------------------------------------------------------------------[>]-*/

int
put_smtoper_set_log (
    byte **_buffer,
    const char *agent_name,             /*  Name of logging agent            */
    const char *thread_name)            /*  Name of logging thread           */
{
    struct_smtoper_set_log
        *_struct_ptr;
    int
        _total_size = sizeof (struct_smtoper_set_log);
    char
        *_ptr;

    _total_size += agent_name ? strlen (agent_name) + 1 : 0;
    _total_size += thread_name ? strlen (thread_name) + 1 : 0;
    _struct_ptr = mem_alloc (_total_size);
    *_buffer = (byte *) _struct_ptr;
    if (_struct_ptr)
      {
        _ptr = (char *) _struct_ptr + sizeof (struct_smtoper_set_log);
        if (agent_name)
          {
            _struct_ptr-> agent_name        = (char *) _ptr;
            strcpy ((char *) _ptr, agent_name);
            _ptr += strlen (agent_name) + 1;
          }
        else
            _struct_ptr-> agent_name        = NULL;
        if (thread_name)
          {
            _struct_ptr-> thread_name       = (char *) _ptr;
            strcpy ((char *) _ptr, thread_name);
            _ptr += strlen (thread_name) + 1;
          }
        else
            _struct_ptr-> thread_name       = NULL;

        return _total_size;
      }
    else
        return 0;
}


/*  ---------------------------------------------------------------------[<]-
    Function: get_smtoper_set_log

    Synopsis: Accepts a buffer containing a set log message,
    and unpacks it into a new struct_smtoper_set_log structure. Free the
    structure using free_smtoper_set_log() when finished.
    ---------------------------------------------------------------------[>]-*/

int
get_smtoper_set_log (
    byte *_buffer,
    struct_smtoper_set_log **params)
{
    struct_smtoper_set_log
        *_struct_ptr;
    char
        *_ptr;

    _struct_ptr = (struct_smtoper_set_log *) _buffer;
    *params = mem_alloc (sizeof (struct_smtoper_set_log));
    if (*params)
      {
        _ptr = (char *) _struct_ptr + sizeof (struct_smtoper_set_log);
        if (_struct_ptr-> agent_name)
          {
            (* params)-> agent_name         = mem_strdup (_ptr);
            _ptr += strlen ((* params)-> agent_name) + 1;
          }
        else
            (* params)-> agent_name         = NULL;
        if (_struct_ptr-> thread_name)
          {
            (* params)-> thread_name        = mem_strdup (_ptr);
            _ptr += strlen ((* params)-> thread_name) + 1;
          }
        else
            (* params)-> thread_name        = NULL;
        return 0;
      }
    else
        return -1;
}


/*  ---------------------------------------------------------------------[<]-
    Function: free_smtoper_set_log

    Synopsis: frees a structure allocated by get_smtoper_set_log().
    ---------------------------------------------------------------------[>]-*/

void
free_smtoper_set_log (
    struct_smtoper_set_log **params)
{
    mem_free ((*params)-> agent_name);
    mem_free ((*params)-> thread_name);
    mem_free (*params);
    *params = NULL;
}

char *SMTOPER_SET_LOG = "SMTOPER SET LOG";

/*  ---------------------------------------------------------------------[<]-
    Function: lsend_smtoper_set_log

    Synopsis: Sends a set log -
              event to the smtoper agent
    ---------------------------------------------------------------------[>]-*/

int
lsend_smtoper_set_log (
    const QID  *_to,
    const QID  *_from,
          char *_accept,
          char *_reject,
          char *_expire,
          word _timeout,
    const char *agent_name,         /*  Name of logging agent            */
    const char *thread_name)        /*  Name of logging thread           */
{
    byte *_body;
    int   _size,
          _rc;
    _size = put_smtoper_set_log
                (&_body,
                 agent_name,
                 thread_name);
    if (_size)
      {
        _rc = event_send (_to, _from, SMTOPER_SET_LOG,
                          _body, _size,
                          _accept, _reject, _expire, _timeout);
        mem_free (_body);
        return _rc;
      }
    else
        return -1;
}


/*  ---------------------------------------------------------------------[<]-
    Function: put_smtoper_message

    Synopsis: Formats a message message, allocates a new buffer,
    and returns the formatted message in the buffer.  You should free the
    buffer using mem_free() when finished.  Returns the size of the buffer
    in bytes.
    ---------------------------------------------------------------------[>]-*/

int
put_smtoper_message (
    byte **_buffer,
    const char *text)                   /*  Text of message                  */
{
    struct_smtoper_message
        *_struct_ptr;
    int
        _total_size = sizeof (struct_smtoper_message);
    char
        *_ptr;

    _total_size += text ? strlen (text) + 1 : 0;
    _struct_ptr = mem_alloc (_total_size);
    *_buffer = (byte *) _struct_ptr;
    if (_struct_ptr)
      {
        _ptr = (char *) _struct_ptr + sizeof (struct_smtoper_message);
        if (text)
          {
            _struct_ptr-> text              = (char *) _ptr;
            strcpy ((char *) _ptr, text);
            _ptr += strlen (text) + 1;
          }
        else
            _struct_ptr-> text              = NULL;

        return _total_size;
      }
    else
        return 0;
}


/*  ---------------------------------------------------------------------[<]-
    Function: get_smtoper_message

    Synopsis: Accepts a buffer containing a message message,
    and unpacks it into a new struct_smtoper_message structure. Free the
    structure using free_smtoper_message() when finished.
    ---------------------------------------------------------------------[>]-*/

int
get_smtoper_message (
    byte *_buffer,
    struct_smtoper_message **params)
{
    struct_smtoper_message
        *_struct_ptr;
    char
        *_ptr;

    _struct_ptr = (struct_smtoper_message *) _buffer;
    *params = mem_alloc (sizeof (struct_smtoper_message));
    if (*params)
      {
        _ptr = (char *) _struct_ptr + sizeof (struct_smtoper_message);
        if (_struct_ptr-> text)
          {
            (* params)-> text               = mem_strdup (_ptr);
            _ptr += strlen ((* params)-> text) + 1;
          }
        else
            (* params)-> text               = NULL;
        return 0;
      }
    else
        return -1;
}


/*  ---------------------------------------------------------------------[<]-
    Function: free_smtoper_message

    Synopsis: frees a structure allocated by get_smtoper_message().
    ---------------------------------------------------------------------[>]-*/

void
free_smtoper_message (
    struct_smtoper_message **params)
{
    mem_free ((*params)-> text);
    mem_free (*params);
    *params = NULL;
}

char *SMTOPER_ERROR = "SMTOPER ERROR";

/*  ---------------------------------------------------------------------[<]-
    Function: lsend_smtoper_error

    Synopsis: Sends a error -
              event to the smtoper agent
    ---------------------------------------------------------------------[>]-*/

int
lsend_smtoper_error (
    const QID  *_to,
    const QID  *_from,
          char *_accept,
          char *_reject,
          char *_expire,
          word _timeout,
    const char *text)               /*  Text of message                  */
{
    byte *_body;
    int   _size,
          _rc;
    _size = put_smtoper_message
                (&_body,
                 text);
    if (_size)
      {
        _rc = event_send (_to, _from, SMTOPER_ERROR,
                          _body, _size,
                          _accept, _reject, _expire, _timeout);
        mem_free (_body);
        return _rc;
      }
    else
        return -1;
}

char *SMTOPER_WARNING = "SMTOPER WARNING";

/*  ---------------------------------------------------------------------[<]-
    Function: lsend_smtoper_warning

    Synopsis: Sends a warning -
              event to the smtoper agent
    ---------------------------------------------------------------------[>]-*/

int
lsend_smtoper_warning (
    const QID  *_to,
    const QID  *_from,
          char *_accept,
          char *_reject,
          char *_expire,
          word _timeout,
    const char *text)               /*  Text of message                  */
{
    byte *_body;
    int   _size,
          _rc;
    _size = put_smtoper_message
                (&_body,
                 text);
    if (_size)
      {
        _rc = event_send (_to, _from, SMTOPER_WARNING,
                          _body, _size,
                          _accept, _reject, _expire, _timeout);
        mem_free (_body);
        return _rc;
      }
    else
        return -1;
}

char *SMTOPER_INFO = "SMTOPER INFO";

/*  ---------------------------------------------------------------------[<]-
    Function: lsend_smtoper_info

    Synopsis: Sends a info -
              event to the smtoper agent
    ---------------------------------------------------------------------[>]-*/

int
lsend_smtoper_info (
    const QID  *_to,
    const QID  *_from,
          char *_accept,
          char *_reject,
          char *_expire,
          word _timeout,
    const char *text)               /*  Text of message                  */
{
    byte *_body;
    int   _size,
          _rc;
    _size = put_smtoper_message
                (&_body,
                 text);
    if (_size)
      {
        _rc = event_send (_to, _from, SMTOPER_INFO,
                          _body, _size,
                          _accept, _reject, _expire, _timeout);
        mem_free (_body);
        return _rc;
      }
    else
        return -1;
}

char *SMTOPER_ENABLE = "SMTOPER ENABLE";

char *SMTOPER_DISABLE = "SMTOPER DISABLE";

/*---------------------------------------------------------------------------
 *  Message functions for smtrdns - Reverse DNS agent.
 *---------------------------------------------------------------------------*/


/*  ---------------------------------------------------------------------[<]-
    Function: put_smtrdns_get_host_name

    Synopsis: Formats a get host name message, allocates a new buffer,
    and returns the formatted message in the buffer.  You should free the
    buffer using mem_free() when finished.  Returns the size of the buffer
    in bytes.
    ---------------------------------------------------------------------[>]-*/

int
put_smtrdns_get_host_name (
    byte **_buffer,
    const qbyte ip_address,             /*  IP address in network order      */
    const char *ip_string,              /*  Alternative address in string format  */
    const qbyte request_tag)            /*  User-defined request tag         */
{
    struct_smtrdns_get_host_name
        *_struct_ptr;
    int
        _total_size = sizeof (struct_smtrdns_get_host_name);
    char
        *_ptr;

    _total_size += ip_string ? strlen (ip_string) + 1 : 0;
    _struct_ptr = mem_alloc (_total_size);
    *_buffer = (byte *) _struct_ptr;
    if (_struct_ptr)
      {
        _ptr = (char *) _struct_ptr + sizeof (struct_smtrdns_get_host_name);
        _struct_ptr-> ip_address        = ip_address;
        if (ip_string)
          {
            _struct_ptr-> ip_string         = (char *) _ptr;
            strcpy ((char *) _ptr, ip_string);
            _ptr += strlen (ip_string) + 1;
          }
        else
            _struct_ptr-> ip_string         = NULL;
        _struct_ptr-> request_tag       = request_tag;

        return _total_size;
      }
    else
        return 0;
}


/*  ---------------------------------------------------------------------[<]-
    Function: get_smtrdns_get_host_name

    Synopsis: Accepts a buffer containing a get host name message,
    and unpacks it into a new struct_smtrdns_get_host_name structure. Free the
    structure using free_smtrdns_get_host_name() when finished.
    ---------------------------------------------------------------------[>]-*/

int
get_smtrdns_get_host_name (
    byte *_buffer,
    struct_smtrdns_get_host_name **params)
{
    struct_smtrdns_get_host_name
        *_struct_ptr;
    char
        *_ptr;

    _struct_ptr = (struct_smtrdns_get_host_name *) _buffer;
    *params = mem_alloc (sizeof (struct_smtrdns_get_host_name));
    if (*params)
      {
        _ptr = (char *) _struct_ptr + sizeof (struct_smtrdns_get_host_name);
        (* params)-> ip_address         = _struct_ptr-> ip_address;
        if (_struct_ptr-> ip_string)
          {
            (* params)-> ip_string          = mem_strdup (_ptr);
            _ptr += strlen ((* params)-> ip_string) + 1;
          }
        else
            (* params)-> ip_string          = NULL;
        (* params)-> request_tag        = _struct_ptr-> request_tag;
        return 0;
      }
    else
        return -1;
}


/*  ---------------------------------------------------------------------[<]-
    Function: free_smtrdns_get_host_name

    Synopsis: frees a structure allocated by get_smtrdns_get_host_name().
    ---------------------------------------------------------------------[>]-*/

void
free_smtrdns_get_host_name (
    struct_smtrdns_get_host_name **params)
{
    mem_free ((*params)-> ip_string);
    mem_free (*params);
    *params = NULL;
}

char *SMTRDNS_GET_HOST_NAME = "SMTRDNS GET HOST NAME";

/*  ---------------------------------------------------------------------[<]-
    Function: lsend_smtrdns_get_host_name

    Synopsis: Sends a get host name -
              event to the smtrdns agent
    ---------------------------------------------------------------------[>]-*/

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
    const qbyte request_tag)        /*  User-defined request tag         */
{
    byte *_body;
    int   _size,
          _rc;
    _size = put_smtrdns_get_host_name
                (&_body,
                 ip_address,
                 ip_string,
                 request_tag);
    if (_size)
      {
        _rc = event_send (_to, _from, SMTRDNS_GET_HOST_NAME,
                          _body, _size,
                          _accept, _reject, _expire, _timeout);
        mem_free (_body);
        return _rc;
      }
    else
        return -1;
}


/*  ---------------------------------------------------------------------[<]-
    Function: put_smtrdns_get_host_ip

    Synopsis: Formats a get host ip message, allocates a new buffer,
    and returns the formatted message in the buffer.  You should free the
    buffer using mem_free() when finished.  Returns the size of the buffer
    in bytes.
    ---------------------------------------------------------------------[>]-*/

int
put_smtrdns_get_host_ip (
    byte **_buffer,
    const char *host_name,              /*  Host name to look-up             */
    const qbyte request_tag)            /*  User-defined request tag         */
{
    struct_smtrdns_get_host_ip
        *_struct_ptr;
    int
        _total_size = sizeof (struct_smtrdns_get_host_ip);
    char
        *_ptr;

    _total_size += host_name ? strlen (host_name) + 1 : 0;
    _struct_ptr = mem_alloc (_total_size);
    *_buffer = (byte *) _struct_ptr;
    if (_struct_ptr)
      {
        _ptr = (char *) _struct_ptr + sizeof (struct_smtrdns_get_host_ip);
        if (host_name)
          {
            _struct_ptr-> host_name         = (char *) _ptr;
            strcpy ((char *) _ptr, host_name);
            _ptr += strlen (host_name) + 1;
          }
        else
            _struct_ptr-> host_name         = NULL;
        _struct_ptr-> request_tag       = request_tag;

        return _total_size;
      }
    else
        return 0;
}


/*  ---------------------------------------------------------------------[<]-
    Function: get_smtrdns_get_host_ip

    Synopsis: Accepts a buffer containing a get host ip message,
    and unpacks it into a new struct_smtrdns_get_host_ip structure. Free the
    structure using free_smtrdns_get_host_ip() when finished.
    ---------------------------------------------------------------------[>]-*/

int
get_smtrdns_get_host_ip (
    byte *_buffer,
    struct_smtrdns_get_host_ip **params)
{
    struct_smtrdns_get_host_ip
        *_struct_ptr;
    char
        *_ptr;

    _struct_ptr = (struct_smtrdns_get_host_ip *) _buffer;
    *params = mem_alloc (sizeof (struct_smtrdns_get_host_ip));
    if (*params)
      {
        _ptr = (char *) _struct_ptr + sizeof (struct_smtrdns_get_host_ip);
        if (_struct_ptr-> host_name)
          {
            (* params)-> host_name          = mem_strdup (_ptr);
            _ptr += strlen ((* params)-> host_name) + 1;
          }
        else
            (* params)-> host_name          = NULL;
        (* params)-> request_tag        = _struct_ptr-> request_tag;
        return 0;
      }
    else
        return -1;
}


/*  ---------------------------------------------------------------------[<]-
    Function: free_smtrdns_get_host_ip

    Synopsis: frees a structure allocated by get_smtrdns_get_host_ip().
    ---------------------------------------------------------------------[>]-*/

void
free_smtrdns_get_host_ip (
    struct_smtrdns_get_host_ip **params)
{
    mem_free ((*params)-> host_name);
    mem_free (*params);
    *params = NULL;
}

char *SMTRDNS_GET_HOST_IP = "SMTRDNS GET HOST IP";

/*  ---------------------------------------------------------------------[<]-
    Function: lsend_smtrdns_get_host_ip

    Synopsis: Sends a get host ip -
              event to the smtrdns agent
    ---------------------------------------------------------------------[>]-*/

int
lsend_smtrdns_get_host_ip (
    const QID  *_to,
    const QID  *_from,
          char *_accept,
          char *_reject,
          char *_expire,
          word _timeout,
    const char *host_name,          /*  Host name to look-up             */
    const qbyte request_tag)        /*  User-defined request tag         */
{
    byte *_body;
    int   _size,
          _rc;
    _size = put_smtrdns_get_host_ip
                (&_body,
                 host_name,
                 request_tag);
    if (_size)
      {
        _rc = event_send (_to, _from, SMTRDNS_GET_HOST_IP,
                          _body, _size,
                          _accept, _reject, _expire, _timeout);
        mem_free (_body);
        return _rc;
      }
    else
        return -1;
}


/*  ---------------------------------------------------------------------[<]-
    Function: put_smtrdns_host_name

    Synopsis: Formats a host name message, allocates a new buffer,
    and returns the formatted message in the buffer.  You should free the
    buffer using mem_free() when finished.  Returns the size of the buffer
    in bytes.
    ---------------------------------------------------------------------[>]-*/

int
put_smtrdns_host_name (
    byte **_buffer,
    const qbyte ip_address,             /*  IP address in network order      */
    const char *host_name,              /*  Host name                        */
    const qbyte request_tag)            /*  User-defined request tag         */
{
    struct_smtrdns_host_name
        *_struct_ptr;
    int
        _total_size = sizeof (struct_smtrdns_host_name);
    char
        *_ptr;

    _total_size += host_name ? strlen (host_name) + 1 : 0;
    _struct_ptr = mem_alloc (_total_size);
    *_buffer = (byte *) _struct_ptr;
    if (_struct_ptr)
      {
        _ptr = (char *) _struct_ptr + sizeof (struct_smtrdns_host_name);
        _struct_ptr-> ip_address        = ip_address;
        if (host_name)
          {
            _struct_ptr-> host_name         = (char *) _ptr;
            strcpy ((char *) _ptr, host_name);
            _ptr += strlen (host_name) + 1;
          }
        else
            _struct_ptr-> host_name         = NULL;
        _struct_ptr-> request_tag       = request_tag;

        return _total_size;
      }
    else
        return 0;
}


/*  ---------------------------------------------------------------------[<]-
    Function: get_smtrdns_host_name

    Synopsis: Accepts a buffer containing a host name message,
    and unpacks it into a new struct_smtrdns_host_name structure. Free the
    structure using free_smtrdns_host_name() when finished.
    ---------------------------------------------------------------------[>]-*/

int
get_smtrdns_host_name (
    byte *_buffer,
    struct_smtrdns_host_name **params)
{
    struct_smtrdns_host_name
        *_struct_ptr;
    char
        *_ptr;

    _struct_ptr = (struct_smtrdns_host_name *) _buffer;
    *params = mem_alloc (sizeof (struct_smtrdns_host_name));
    if (*params)
      {
        _ptr = (char *) _struct_ptr + sizeof (struct_smtrdns_host_name);
        (* params)-> ip_address         = _struct_ptr-> ip_address;
        if (_struct_ptr-> host_name)
          {
            (* params)-> host_name          = mem_strdup (_ptr);
            _ptr += strlen ((* params)-> host_name) + 1;
          }
        else
            (* params)-> host_name          = NULL;
        (* params)-> request_tag        = _struct_ptr-> request_tag;
        return 0;
      }
    else
        return -1;
}


/*  ---------------------------------------------------------------------[<]-
    Function: free_smtrdns_host_name

    Synopsis: frees a structure allocated by get_smtrdns_host_name().
    ---------------------------------------------------------------------[>]-*/

void
free_smtrdns_host_name (
    struct_smtrdns_host_name **params)
{
    mem_free ((*params)-> host_name);
    mem_free (*params);
    *params = NULL;
}

char *SMTRDNS_HOST_NAME = "SMTRDNS HOST NAME";

/*  ---------------------------------------------------------------------[<]-
    Function: lsend_smtrdns_host_name

    Synopsis: Sends a host name -
              event to the smtrdns agent
    ---------------------------------------------------------------------[>]-*/

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
    const qbyte request_tag)        /*  User-defined request tag         */
{
    byte *_body;
    int   _size,
          _rc;
    _size = put_smtrdns_host_name
                (&_body,
                 ip_address,
                 host_name,
                 request_tag);
    if (_size)
      {
        _rc = event_send (_to, _from, SMTRDNS_HOST_NAME,
                          _body, _size,
                          _accept, _reject, _expire, _timeout);
        mem_free (_body);
        return _rc;
      }
    else
        return -1;
}


/*  ---------------------------------------------------------------------[<]-
    Function: put_smtrdns_host_ip

    Synopsis: Formats a host ip message, allocates a new buffer,
    and returns the formatted message in the buffer.  You should free the
    buffer using mem_free() when finished.  Returns the size of the buffer
    in bytes.
    ---------------------------------------------------------------------[>]-*/

int
put_smtrdns_host_ip (
    byte **_buffer,
    const qbyte ip_address,             /*  IP address in network order      */
    const char *host_name,              /*  Host name                        */
    const qbyte request_tag)            /*  User-defined request tag         */
{
    struct_smtrdns_host_ip
        *_struct_ptr;
    int
        _total_size = sizeof (struct_smtrdns_host_ip);
    char
        *_ptr;

    _total_size += host_name ? strlen (host_name) + 1 : 0;
    _struct_ptr = mem_alloc (_total_size);
    *_buffer = (byte *) _struct_ptr;
    if (_struct_ptr)
      {
        _ptr = (char *) _struct_ptr + sizeof (struct_smtrdns_host_ip);
        _struct_ptr-> ip_address        = ip_address;
        if (host_name)
          {
            _struct_ptr-> host_name         = (char *) _ptr;
            strcpy ((char *) _ptr, host_name);
            _ptr += strlen (host_name) + 1;
          }
        else
            _struct_ptr-> host_name         = NULL;
        _struct_ptr-> request_tag       = request_tag;

        return _total_size;
      }
    else
        return 0;
}


/*  ---------------------------------------------------------------------[<]-
    Function: get_smtrdns_host_ip

    Synopsis: Accepts a buffer containing a host ip message,
    and unpacks it into a new struct_smtrdns_host_ip structure. Free the
    structure using free_smtrdns_host_ip() when finished.
    ---------------------------------------------------------------------[>]-*/

int
get_smtrdns_host_ip (
    byte *_buffer,
    struct_smtrdns_host_ip **params)
{
    struct_smtrdns_host_ip
        *_struct_ptr;
    char
        *_ptr;

    _struct_ptr = (struct_smtrdns_host_ip *) _buffer;
    *params = mem_alloc (sizeof (struct_smtrdns_host_ip));
    if (*params)
      {
        _ptr = (char *) _struct_ptr + sizeof (struct_smtrdns_host_ip);
        (* params)-> ip_address         = _struct_ptr-> ip_address;
        if (_struct_ptr-> host_name)
          {
            (* params)-> host_name          = mem_strdup (_ptr);
            _ptr += strlen ((* params)-> host_name) + 1;
          }
        else
            (* params)-> host_name          = NULL;
        (* params)-> request_tag        = _struct_ptr-> request_tag;
        return 0;
      }
    else
        return -1;
}


/*  ---------------------------------------------------------------------[<]-
    Function: free_smtrdns_host_ip

    Synopsis: frees a structure allocated by get_smtrdns_host_ip().
    ---------------------------------------------------------------------[>]-*/

void
free_smtrdns_host_ip (
    struct_smtrdns_host_ip **params)
{
    mem_free ((*params)-> host_name);
    mem_free (*params);
    *params = NULL;
}

char *SMTRDNS_HOST_IP = "SMTRDNS HOST IP";

/*  ---------------------------------------------------------------------[<]-
    Function: lsend_smtrdns_host_ip

    Synopsis: Sends a host ip -
              event to the smtrdns agent
    ---------------------------------------------------------------------[>]-*/

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
    const qbyte request_tag)        /*  User-defined request tag         */
{
    byte *_body;
    int   _size,
          _rc;
    _size = put_smtrdns_host_ip
                (&_body,
                 ip_address,
                 host_name,
                 request_tag);
    if (_size)
      {
        _rc = event_send (_to, _from, SMTRDNS_HOST_IP,
                          _body, _size,
                          _accept, _reject, _expire, _timeout);
        mem_free (_body);
        return _rc;
      }
    else
        return -1;
}

char *SMTRDNS_HOST_ERROR = "SMTRDNS HOST ERROR";

char *SMTRDNS_HOST_END = "SMTRDNS HOST END";

char *SMTRDNS_HOST_TIMEOUT = "SMTRDNS HOST TIMEOUT";

/*---------------------------------------------------------------------------
 *  Message functions for smtslot - Time slot agent.
 *---------------------------------------------------------------------------*/


/*  ---------------------------------------------------------------------[<]-
    Function: put_smtslot_specification

    Synopsis: Formats a specification message, allocates a new buffer,
    and returns the formatted message in the buffer.  You should free the
    buffer using mem_free() when finished.  Returns the size of the buffer
    in bytes.
    ---------------------------------------------------------------------[>]-*/

int
put_smtslot_specification (
    byte **_buffer,
    const char *times)                  /*  Time slot specification          */
{
    struct_smtslot_specification
        *_struct_ptr;
    int
        _total_size = sizeof (struct_smtslot_specification);
    char
        *_ptr;

    _total_size += times ? strlen (times) + 1 : 0;
    _struct_ptr = mem_alloc (_total_size);
    *_buffer = (byte *) _struct_ptr;
    if (_struct_ptr)
      {
        _ptr = (char *) _struct_ptr + sizeof (struct_smtslot_specification);
        if (times)
          {
            _struct_ptr-> times             = (char *) _ptr;
            strcpy ((char *) _ptr, times);
            _ptr += strlen (times) + 1;
          }
        else
            _struct_ptr-> times             = NULL;

        return _total_size;
      }
    else
        return 0;
}


/*  ---------------------------------------------------------------------[<]-
    Function: get_smtslot_specification

    Synopsis: Accepts a buffer containing a specification message,
    and unpacks it into a new struct_smtslot_specification structure. Free the
    structure using free_smtslot_specification() when finished.
    ---------------------------------------------------------------------[>]-*/

int
get_smtslot_specification (
    byte *_buffer,
    struct_smtslot_specification **params)
{
    struct_smtslot_specification
        *_struct_ptr;
    char
        *_ptr;

    _struct_ptr = (struct_smtslot_specification *) _buffer;
    *params = mem_alloc (sizeof (struct_smtslot_specification));
    if (*params)
      {
        _ptr = (char *) _struct_ptr + sizeof (struct_smtslot_specification);
        if (_struct_ptr-> times)
          {
            (* params)-> times              = mem_strdup (_ptr);
            _ptr += strlen ((* params)-> times) + 1;
          }
        else
            (* params)-> times              = NULL;
        return 0;
      }
    else
        return -1;
}


/*  ---------------------------------------------------------------------[<]-
    Function: free_smtslot_specification

    Synopsis: frees a structure allocated by get_smtslot_specification().
    ---------------------------------------------------------------------[>]-*/

void
free_smtslot_specification (
    struct_smtslot_specification **params)
{
    mem_free ((*params)-> times);
    mem_free (*params);
    *params = NULL;
}

char *SMTSLOT_SPECIFY = "SMTSLOT SPECIFY";

/*  ---------------------------------------------------------------------[<]-
    Function: lsend_smtslot_specify

    Synopsis: Sends a specify -
              event to the smtslot agent
    ---------------------------------------------------------------------[>]-*/

int
lsend_smtslot_specify (
    const QID  *_to,
    const QID  *_from,
          char *_accept,
          char *_reject,
          char *_expire,
          word _timeout,
    const char *times)              /*  Time slot specification          */
{
    byte *_body;
    int   _size,
          _rc;
    _size = put_smtslot_specification
                (&_body,
                 times);
    if (_size)
      {
        _rc = event_send (_to, _from, SMTSLOT_SPECIFY,
                          _body, _size,
                          _accept, _reject, _expire, _timeout);
        mem_free (_body);
        return _rc;
      }
    else
        return -1;
}

char *SMTSLOT_RESET = "SMTSLOT RESET";

char *SMTSLOT_ON = "SMTSLOT ON";

char *SMTSLOT_OFF = "SMTSLOT OFF";

char *SMTSLOT_FINISH = "SMTSLOT FINISH";


/*  ---------------------------------------------------------------------[<]-
    Function: put_smtslot_error

    Synopsis: Formats a error message, allocates a new buffer,
    and returns the formatted message in the buffer.  You should free the
    buffer using mem_free() when finished.  Returns the size of the buffer
    in bytes.
    ---------------------------------------------------------------------[>]-*/

int
put_smtslot_error (
    byte **_buffer,
    const char *message)                /*  Error message                    */
{
    struct_smtslot_error
        *_struct_ptr;
    int
        _total_size = sizeof (struct_smtslot_error);
    char
        *_ptr;

    _total_size += message ? strlen (message) + 1 : 0;
    _struct_ptr = mem_alloc (_total_size);
    *_buffer = (byte *) _struct_ptr;
    if (_struct_ptr)
      {
        _ptr = (char *) _struct_ptr + sizeof (struct_smtslot_error);
        if (message)
          {
            _struct_ptr-> message           = (char *) _ptr;
            strcpy ((char *) _ptr, message);
            _ptr += strlen (message) + 1;
          }
        else
            _struct_ptr-> message           = NULL;

        return _total_size;
      }
    else
        return 0;
}


/*  ---------------------------------------------------------------------[<]-
    Function: get_smtslot_error

    Synopsis: Accepts a buffer containing a error message,
    and unpacks it into a new struct_smtslot_error structure. Free the
    structure using free_smtslot_error() when finished.
    ---------------------------------------------------------------------[>]-*/

int
get_smtslot_error (
    byte *_buffer,
    struct_smtslot_error **params)
{
    struct_smtslot_error
        *_struct_ptr;
    char
        *_ptr;

    _struct_ptr = (struct_smtslot_error *) _buffer;
    *params = mem_alloc (sizeof (struct_smtslot_error));
    if (*params)
      {
        _ptr = (char *) _struct_ptr + sizeof (struct_smtslot_error);
        if (_struct_ptr-> message)
          {
            (* params)-> message            = mem_strdup (_ptr);
            _ptr += strlen ((* params)-> message) + 1;
          }
        else
            (* params)-> message            = NULL;
        return 0;
      }
    else
        return -1;
}


/*  ---------------------------------------------------------------------[<]-
    Function: free_smtslot_error

    Synopsis: frees a structure allocated by get_smtslot_error().
    ---------------------------------------------------------------------[>]-*/

void
free_smtslot_error (
    struct_smtslot_error **params)
{
    mem_free ((*params)-> message);
    mem_free (*params);
    *params = NULL;
}

char *SMTSLOT_ERROR = "SMTSLOT ERROR";

/*  ---------------------------------------------------------------------[<]-
    Function: lsend_smtslot_error

    Synopsis: Sends a error -
              event to the smtslot agent
    ---------------------------------------------------------------------[>]-*/

int
lsend_smtslot_error (
    const QID  *_to,
    const QID  *_from,
          char *_accept,
          char *_reject,
          char *_expire,
          word _timeout,
    const char *message)            /*  Error message                    */
{
    byte *_body;
    int   _size,
          _rc;
    _size = put_smtslot_error
                (&_body,
                 message);
    if (_size)
      {
        _rc = event_send (_to, _from, SMTSLOT_ERROR,
                          _body, _size,
                          _accept, _reject, _expire, _timeout);
        mem_free (_body);
        return _rc;
      }
    else
        return -1;
}

char *SMTSLOT_SWITCH_ON = "SMTSLOT SWITCH ON";

char *SMTSLOT_SWITCH_OFF = "SMTSLOT SWITCH OFF";

/*---------------------------------------------------------------------------
 *  Message functions for smtsock - Socket i/o agent.
 *---------------------------------------------------------------------------*/


/*  ---------------------------------------------------------------------[<]-
    Function: put_smtsock_read

    Synopsis: Formats a read message, allocates a new buffer,
    and returns the formatted message in the buffer.  You should free the
    buffer using mem_free() when finished.  Returns the size of the buffer
    in bytes.
    ---------------------------------------------------------------------[>]-*/

int
put_smtsock_read (
    byte **_buffer,
    const dbyte timeout,                /*  Timeout in seconds, zero = none  */
    const qbyte socket,                 /*  Socket to read from              */
    const dbyte max_size,               /*  Size of receiving buffer         */
    const dbyte min_size,               /*  Minimum data to read, zero = all  */
    const void *tag)                    /*  User-defined request tag         */
{
    struct_smtsock_read
        *_struct_ptr;

    _struct_ptr = mem_alloc (sizeof (struct_smtsock_read));
    *_buffer = (byte *) _struct_ptr;
    if (_struct_ptr)
      {
        _struct_ptr-> timeout           = timeout;
        _struct_ptr-> socket            = socket;
        _struct_ptr-> max_size          = max_size;
        _struct_ptr-> min_size          = min_size;
        _struct_ptr-> tag               = (void *) tag;

        return sizeof (*_struct_ptr);
      }
    else
        return 0;
}


/*  ---------------------------------------------------------------------[<]-
    Function: get_smtsock_read

    Synopsis: Accepts a buffer containing a read message,
    and unpacks it into a new struct_smtsock_read structure. Free the
    structure using free_smtsock_read() when finished.
    ---------------------------------------------------------------------[>]-*/

int
get_smtsock_read (
    byte *_buffer,
    struct_smtsock_read **params)
{
    struct_smtsock_read
        *_struct_ptr;

    _struct_ptr = (struct_smtsock_read *) _buffer;
    *params = mem_alloc (sizeof (struct_smtsock_read));
    if (*params)
      {
        (* params)-> timeout            = _struct_ptr-> timeout;
        (* params)-> socket             = _struct_ptr-> socket;
        (* params)-> max_size           = _struct_ptr-> max_size;
        (* params)-> min_size           = _struct_ptr-> min_size;
        (* params)-> tag                = _struct_ptr-> tag;
        return 0;
      }
    else
        return -1;
}


/*  ---------------------------------------------------------------------[<]-
    Function: free_smtsock_read

    Synopsis: frees a structure allocated by get_smtsock_read().
    ---------------------------------------------------------------------[>]-*/

void
free_smtsock_read (
    struct_smtsock_read **params)
{
    mem_free (*params);
    *params = NULL;
}

char *SMTSOCK_READ = "SMTSOCK READ";

/*  ---------------------------------------------------------------------[<]-
    Function: lsend_smtsock_read

    Synopsis: Sends a read - Read from socket
              event to the smtsock agent
    ---------------------------------------------------------------------[>]-*/

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
    const void *tag)                /*  User-defined request tag         */
{
    byte *_body;
    int   _size,
          _rc;
    _size = put_smtsock_read
                (&_body,
                 timeout,
                 socket,
                 max_size,
                 min_size,
                 tag);
    if (_size)
      {
        _rc = event_send (_to, _from, SMTSOCK_READ,
                          _body, _size,
                          _accept, _reject, _expire, _timeout);
        mem_free (_body);
        return _rc;
      }
    else
        return -1;
}

char *SMTSOCK_READR = "SMTSOCK READR";

/*  ---------------------------------------------------------------------[<]-
    Function: lsend_smtsock_readr

    Synopsis: Sends a readr - Repeated read from socket
              event to the smtsock agent
    ---------------------------------------------------------------------[>]-*/

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
    const void *tag)                /*  User-defined request tag         */
{
    byte *_body;
    int   _size,
          _rc;
    _size = put_smtsock_read
                (&_body,
                 timeout,
                 socket,
                 max_size,
                 min_size,
                 tag);
    if (_size)
      {
        _rc = event_send (_to, _from, SMTSOCK_READR,
                          _body, _size,
                          _accept, _reject, _expire, _timeout);
        mem_free (_body);
        return _rc;
      }
    else
        return -1;
}


/*  ---------------------------------------------------------------------[<]-
    Function: put_smtsock_write

    Synopsis: Formats a write message, allocates a new buffer,
    and returns the formatted message in the buffer.  You should free the
    buffer using mem_free() when finished.  Returns the size of the buffer
    in bytes.
    ---------------------------------------------------------------------[>]-*/

int
put_smtsock_write (
    byte **_buffer,
    const dbyte timeout,                /*  Timeout in seconds, zero = none  */
    const qbyte socket,                 /*  Socket to write to               */
    const word  size,                   /*  Amount of data to write          */
    const void *data,                   /*  Block of data to write           */
    const Bool  reply,                  /*  Whether OK reply is required     */
    const void *tag)                    /*  User-defined request tag         */
{
    struct_smtsock_write
        *_struct_ptr;
    int
        _total_size = sizeof (struct_smtsock_write);
    char
        *_ptr;

    _total_size += size;
    _struct_ptr = mem_alloc (_total_size);
    *_buffer = (byte *) _struct_ptr;
    if (_struct_ptr)
      {
        _ptr = (char *) _struct_ptr + sizeof (struct_smtsock_write);
        _struct_ptr-> timeout           = timeout;
        _struct_ptr-> socket            = socket;
        _struct_ptr-> size              = size;
        _struct_ptr-> data              = (byte *) _ptr;
        memcpy (_ptr, data, size);
        _ptr += size;
        _struct_ptr-> reply             = reply;
        _struct_ptr-> tag               = (void *) tag;

        return _total_size;
      }
    else
        return 0;
}


/*  ---------------------------------------------------------------------[<]-
    Function: get_smtsock_write

    Synopsis: Accepts a buffer containing a write message,
    and unpacks it into a new struct_smtsock_write structure. Free the
    structure using free_smtsock_write() when finished.
    ---------------------------------------------------------------------[>]-*/

int
get_smtsock_write (
    byte *_buffer,
    struct_smtsock_write **params)
{
    struct_smtsock_write
        *_struct_ptr;
    char
        *_ptr;

    _struct_ptr = (struct_smtsock_write *) _buffer;
    *params = mem_alloc (sizeof (struct_smtsock_write));
    if (*params)
      {
        _ptr = (char *) _struct_ptr + sizeof (struct_smtsock_write);
        (* params)-> timeout            = _struct_ptr-> timeout;
        (* params)-> socket             = _struct_ptr-> socket;
        (* params)-> size               = _struct_ptr-> size;
        (* params)-> data               = mem_alloc (_struct_ptr-> size + 1);
        memcpy ((* params)-> data, _ptr, _struct_ptr-> size);
        *((byte *)(* params)-> data + _struct_ptr-> size) = 0;
        _ptr += _struct_ptr-> size;
        (* params)-> reply              = _struct_ptr-> reply;
        (* params)-> tag                = _struct_ptr-> tag;
        return 0;
      }
    else
        return -1;
}


/*  ---------------------------------------------------------------------[<]-
    Function: free_smtsock_write

    Synopsis: frees a structure allocated by get_smtsock_write().
    ---------------------------------------------------------------------[>]-*/

void
free_smtsock_write (
    struct_smtsock_write **params)
{
    mem_free ((*params)-> data);
    mem_free (*params);
    *params = NULL;
}

char *SMTSOCK_WRITE = "SMTSOCK WRITE";

/*  ---------------------------------------------------------------------[<]-
    Function: lsend_smtsock_write

    Synopsis: Sends a write - Write to socket
              event to the smtsock agent
    ---------------------------------------------------------------------[>]-*/

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
    const void *tag)                /*  User-defined request tag         */
{
    byte *_body;
    int   _size,
          _rc;
    _size = put_smtsock_write
                (&_body,
                 timeout,
                 socket,
                 size,
                 data,
                 reply,
                 tag);
    if (_size)
      {
        _rc = event_send (_to, _from, SMTSOCK_WRITE,
                          _body, _size,
                          _accept, _reject, _expire, _timeout);
        mem_free (_body);
        return _rc;
      }
    else
        return -1;
}


/*  ---------------------------------------------------------------------[<]-
    Function: put_smtsock_close

    Synopsis: Formats a close message, allocates a new buffer,
    and returns the formatted message in the buffer.  You should free the
    buffer using mem_free() when finished.  Returns the size of the buffer
    in bytes.
    ---------------------------------------------------------------------[>]-*/

int
put_smtsock_close (
    byte **_buffer,
    const dbyte timeout,                /*  Timeout in seconds, zero = none  */
    const qbyte socket,                 /*  Socket to write to               */
    const Bool  reply,                  /*  Whether OK reply is required     */
    const void *tag)                    /*  User-defined request tag         */
{
    struct_smtsock_close
        *_struct_ptr;

    _struct_ptr = mem_alloc (sizeof (struct_smtsock_close));
    *_buffer = (byte *) _struct_ptr;
    if (_struct_ptr)
      {
        _struct_ptr-> timeout           = timeout;
        _struct_ptr-> socket            = socket;
        _struct_ptr-> reply             = reply;
        _struct_ptr-> tag               = (void *) tag;

        return sizeof (*_struct_ptr);
      }
    else
        return 0;
}


/*  ---------------------------------------------------------------------[<]-
    Function: get_smtsock_close

    Synopsis: Accepts a buffer containing a close message,
    and unpacks it into a new struct_smtsock_close structure. Free the
    structure using free_smtsock_close() when finished.
    ---------------------------------------------------------------------[>]-*/

int
get_smtsock_close (
    byte *_buffer,
    struct_smtsock_close **params)
{
    struct_smtsock_close
        *_struct_ptr;

    _struct_ptr = (struct_smtsock_close *) _buffer;
    *params = mem_alloc (sizeof (struct_smtsock_close));
    if (*params)
      {
        (* params)-> timeout            = _struct_ptr-> timeout;
        (* params)-> socket             = _struct_ptr-> socket;
        (* params)-> reply              = _struct_ptr-> reply;
        (* params)-> tag                = _struct_ptr-> tag;
        return 0;
      }
    else
        return -1;
}


/*  ---------------------------------------------------------------------[<]-
    Function: free_smtsock_close

    Synopsis: frees a structure allocated by get_smtsock_close().
    ---------------------------------------------------------------------[>]-*/

void
free_smtsock_close (
    struct_smtsock_close **params)
{
    mem_free (*params);
    *params = NULL;
}

char *SMTSOCK_CLOSE = "SMTSOCK CLOSE";

/*  ---------------------------------------------------------------------[<]-
    Function: lsend_smtsock_close

    Synopsis: Sends a close - Close socket
              event to the smtsock agent
    ---------------------------------------------------------------------[>]-*/

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
    const void *tag)                /*  User-defined request tag         */
{
    byte *_body;
    int   _size,
          _rc;
    _size = put_smtsock_close
                (&_body,
                 timeout,
                 socket,
                 reply,
                 tag);
    if (_size)
      {
        _rc = event_send (_to, _from, SMTSOCK_CLOSE,
                          _body, _size,
                          _accept, _reject, _expire, _timeout);
        mem_free (_body);
        return _rc;
      }
    else
        return -1;
}


/*  ---------------------------------------------------------------------[<]-
    Function: put_smtsock_readh

    Synopsis: Formats a readh message, allocates a new buffer,
    and returns the formatted message in the buffer.  You should free the
    buffer using mem_free() when finished.  Returns the size of the buffer
    in bytes.
    ---------------------------------------------------------------------[>]-*/

int
put_smtsock_readh (
    byte **_buffer,
    const dbyte timeout,                /*  Timeout in seconds, zero = none  */
    const qbyte socket,                 /*  Socket to read from              */
    const qbyte max_size,               /*  Size of receiving buffer         */
    const qbyte min_size,               /*  Minimum data to read, zero = all  */
    const void *tag)                    /*  User-defined request tag         */
{
    struct_smtsock_readh
        *_struct_ptr;

    _struct_ptr = mem_alloc (sizeof (struct_smtsock_readh));
    *_buffer = (byte *) _struct_ptr;
    if (_struct_ptr)
      {
        _struct_ptr-> timeout           = timeout;
        _struct_ptr-> socket            = socket;
        _struct_ptr-> max_size          = max_size;
        _struct_ptr-> min_size          = min_size;
        _struct_ptr-> tag               = (void *) tag;

        return sizeof (*_struct_ptr);
      }
    else
        return 0;
}


/*  ---------------------------------------------------------------------[<]-
    Function: get_smtsock_readh

    Synopsis: Accepts a buffer containing a readh message,
    and unpacks it into a new struct_smtsock_readh structure. Free the
    structure using free_smtsock_readh() when finished.
    ---------------------------------------------------------------------[>]-*/

int
get_smtsock_readh (
    byte *_buffer,
    struct_smtsock_readh **params)
{
    struct_smtsock_readh
        *_struct_ptr;

    _struct_ptr = (struct_smtsock_readh *) _buffer;
    *params = mem_alloc (sizeof (struct_smtsock_readh));
    if (*params)
      {
        (* params)-> timeout            = _struct_ptr-> timeout;
        (* params)-> socket             = _struct_ptr-> socket;
        (* params)-> max_size           = _struct_ptr-> max_size;
        (* params)-> min_size           = _struct_ptr-> min_size;
        (* params)-> tag                = _struct_ptr-> tag;
        return 0;
      }
    else
        return -1;
}


/*  ---------------------------------------------------------------------[<]-
    Function: free_smtsock_readh

    Synopsis: frees a structure allocated by get_smtsock_readh().
    ---------------------------------------------------------------------[>]-*/

void
free_smtsock_readh (
    struct_smtsock_readh **params)
{
    mem_free (*params);
    *params = NULL;
}

char *SMTSOCK_READH = "SMTSOCK READH";

/*  ---------------------------------------------------------------------[<]-
    Function: lsend_smtsock_readh

    Synopsis: Sends a readh - Huge read from socket
              event to the smtsock agent
    ---------------------------------------------------------------------[>]-*/

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
    const void *tag)                /*  User-defined request tag         */
{
    byte *_body;
    int   _size,
          _rc;
    _size = put_smtsock_readh
                (&_body,
                 timeout,
                 socket,
                 max_size,
                 min_size,
                 tag);
    if (_size)
      {
        _rc = event_send (_to, _from, SMTSOCK_READH,
                          _body, _size,
                          _accept, _reject, _expire, _timeout);
        mem_free (_body);
        return _rc;
      }
    else
        return -1;
}

char *SMTSOCK_READRH = "SMTSOCK READRH";

/*  ---------------------------------------------------------------------[<]-
    Function: lsend_smtsock_readrh

    Synopsis: Sends a readrh - Repeated huge read from socket
              event to the smtsock agent
    ---------------------------------------------------------------------[>]-*/

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
    const void *tag)                /*  User-defined request tag         */
{
    byte *_body;
    int   _size,
          _rc;
    _size = put_smtsock_readh
                (&_body,
                 timeout,
                 socket,
                 max_size,
                 min_size,
                 tag);
    if (_size)
      {
        _rc = event_send (_to, _from, SMTSOCK_READRH,
                          _body, _size,
                          _accept, _reject, _expire, _timeout);
        mem_free (_body);
        return _rc;
      }
    else
        return -1;
}


/*  ---------------------------------------------------------------------[<]-
    Function: put_smtsock_writeh

    Synopsis: Formats a writeh message, allocates a new buffer,
    and returns the formatted message in the buffer.  You should free the
    buffer using mem_free() when finished.  Returns the size of the buffer
    in bytes.
    ---------------------------------------------------------------------[>]-*/

int
put_smtsock_writeh (
    byte **_buffer,
    const dbyte timeout,                /*  Timeout in seconds, zero = none  */
    const qbyte socket,                 /*  Socket to write to               */
    const qbyte size,                   /*  Amount of data to write          */
    const byte *data,                   /*  Block of data to write           */
    const Bool  reply,                  /*  Whether OK reply is required     */
    const void *tag)                    /*  User-defined request tag         */
{
    struct_smtsock_writeh
        *_struct_ptr;
    int
        _total_size = sizeof (struct_smtsock_writeh);
    char
        *_ptr;

    _total_size += size;
    _struct_ptr = mem_alloc (_total_size);
    *_buffer = (byte *) _struct_ptr;
    if (_struct_ptr)
      {
        _ptr = (char *) _struct_ptr + sizeof (struct_smtsock_writeh);
        _struct_ptr-> timeout           = timeout;
        _struct_ptr-> socket            = socket;
        _struct_ptr-> size              = size;
        _struct_ptr-> data              = (byte *) _ptr;
        memcpy (_ptr, data, size);
        _ptr += size;
        _struct_ptr-> reply             = reply;
        _struct_ptr-> tag               = (void *) tag;

        return _total_size;
      }
    else
        return 0;
}


/*  ---------------------------------------------------------------------[<]-
    Function: get_smtsock_writeh

    Synopsis: Accepts a buffer containing a writeh message,
    and unpacks it into a new struct_smtsock_writeh structure. Free the
    structure using free_smtsock_writeh() when finished.
    ---------------------------------------------------------------------[>]-*/

int
get_smtsock_writeh (
    byte *_buffer,
    struct_smtsock_writeh **params)
{
    struct_smtsock_writeh
        *_struct_ptr;
    char
        *_ptr;

    _struct_ptr = (struct_smtsock_writeh *) _buffer;
    *params = mem_alloc (sizeof (struct_smtsock_writeh));
    if (*params)
      {
        _ptr = (char *) _struct_ptr + sizeof (struct_smtsock_writeh);
        (* params)-> timeout            = _struct_ptr-> timeout;
        (* params)-> socket             = _struct_ptr-> socket;
        (* params)-> size               = _struct_ptr-> size;
        (* params)-> data               = mem_alloc (_struct_ptr-> size + 1);
        memcpy ((* params)-> data, _ptr, _struct_ptr-> size);
        *((byte *)(* params)-> data + _struct_ptr-> size) = 0;
        _ptr += _struct_ptr-> size;
        (* params)-> reply              = _struct_ptr-> reply;
        (* params)-> tag                = _struct_ptr-> tag;
        return 0;
      }
    else
        return -1;
}


/*  ---------------------------------------------------------------------[<]-
    Function: free_smtsock_writeh

    Synopsis: frees a structure allocated by get_smtsock_writeh().
    ---------------------------------------------------------------------[>]-*/

void
free_smtsock_writeh (
    struct_smtsock_writeh **params)
{
    mem_free ((*params)-> data);
    mem_free (*params);
    *params = NULL;
}

char *SMTSOCK_WRITEH = "SMTSOCK WRITEH";

/*  ---------------------------------------------------------------------[<]-
    Function: lsend_smtsock_writeh

    Synopsis: Sends a writeh - Huge write to socket
              event to the smtsock agent
    ---------------------------------------------------------------------[>]-*/

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
    const void *tag)                /*  User-defined request tag         */
{
    byte *_body;
    int   _size,
          _rc;
    _size = put_smtsock_writeh
                (&_body,
                 timeout,
                 socket,
                 size,
                 data,
                 reply,
                 tag);
    if (_size)
      {
        _rc = event_send (_to, _from, SMTSOCK_WRITEH,
                          _body, _size,
                          _accept, _reject, _expire, _timeout);
        mem_free (_body);
        return _rc;
      }
    else
        return -1;
}


/*  ---------------------------------------------------------------------[<]-
    Function: put_smtsock_input

    Synopsis: Formats a input message, allocates a new buffer,
    and returns the formatted message in the buffer.  You should free the
    buffer using mem_free() when finished.  Returns the size of the buffer
    in bytes.
    ---------------------------------------------------------------------[>]-*/

int
put_smtsock_input (
    byte **_buffer,
    const dbyte timeout,                /*  Timeout in seconds, zero = none  */
    const qbyte socket,                 /*  Socket to wait on                */
    const void *tag)                    /*  User-defined request tag         */
{
    struct_smtsock_input
        *_struct_ptr;

    _struct_ptr = mem_alloc (sizeof (struct_smtsock_input));
    *_buffer = (byte *) _struct_ptr;
    if (_struct_ptr)
      {
        _struct_ptr-> timeout           = timeout;
        _struct_ptr-> socket            = socket;
        _struct_ptr-> tag               = (void *) tag;

        return sizeof (*_struct_ptr);
      }
    else
        return 0;
}


/*  ---------------------------------------------------------------------[<]-
    Function: get_smtsock_input

    Synopsis: Accepts a buffer containing a input message,
    and unpacks it into a new struct_smtsock_input structure. Free the
    structure using free_smtsock_input() when finished.
    ---------------------------------------------------------------------[>]-*/

int
get_smtsock_input (
    byte *_buffer,
    struct_smtsock_input **params)
{
    struct_smtsock_input
        *_struct_ptr;

    _struct_ptr = (struct_smtsock_input *) _buffer;
    *params = mem_alloc (sizeof (struct_smtsock_input));
    if (*params)
      {
        (* params)-> timeout            = _struct_ptr-> timeout;
        (* params)-> socket             = _struct_ptr-> socket;
        (* params)-> tag                = _struct_ptr-> tag;
        return 0;
      }
    else
        return -1;
}


/*  ---------------------------------------------------------------------[<]-
    Function: free_smtsock_input

    Synopsis: frees a structure allocated by get_smtsock_input().
    ---------------------------------------------------------------------[>]-*/

void
free_smtsock_input (
    struct_smtsock_input **params)
{
    mem_free (*params);
    *params = NULL;
}

char *SMTSOCK_INPUT = "SMTSOCK INPUT";

/*  ---------------------------------------------------------------------[<]-
    Function: lsend_smtsock_input

    Synopsis: Sends a input - Wait for socket input
              event to the smtsock agent
    ---------------------------------------------------------------------[>]-*/

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
    const void *tag)                /*  User-defined request tag         */
{
    byte *_body;
    int   _size,
          _rc;
    _size = put_smtsock_input
                (&_body,
                 timeout,
                 socket,
                 tag);
    if (_size)
      {
        _rc = event_send (_to, _from, SMTSOCK_INPUT,
                          _body, _size,
                          _accept, _reject, _expire, _timeout);
        mem_free (_body);
        return _rc;
      }
    else
        return -1;
}


/*  ---------------------------------------------------------------------[<]-
    Function: put_smtsock_output

    Synopsis: Formats a output message, allocates a new buffer,
    and returns the formatted message in the buffer.  You should free the
    buffer using mem_free() when finished.  Returns the size of the buffer
    in bytes.
    ---------------------------------------------------------------------[>]-*/

int
put_smtsock_output (
    byte **_buffer,
    const dbyte timeout,                /*  Timeout in seconds, zero = none  */
    const qbyte socket,                 /*  Socket to wait on                */
    const void *tag)                    /*  User-defined request tag         */
{
    struct_smtsock_output
        *_struct_ptr;

    _struct_ptr = mem_alloc (sizeof (struct_smtsock_output));
    *_buffer = (byte *) _struct_ptr;
    if (_struct_ptr)
      {
        _struct_ptr-> timeout           = timeout;
        _struct_ptr-> socket            = socket;
        _struct_ptr-> tag               = (void *) tag;

        return sizeof (*_struct_ptr);
      }
    else
        return 0;
}


/*  ---------------------------------------------------------------------[<]-
    Function: get_smtsock_output

    Synopsis: Accepts a buffer containing a output message,
    and unpacks it into a new struct_smtsock_output structure. Free the
    structure using free_smtsock_output() when finished.
    ---------------------------------------------------------------------[>]-*/

int
get_smtsock_output (
    byte *_buffer,
    struct_smtsock_output **params)
{
    struct_smtsock_output
        *_struct_ptr;

    _struct_ptr = (struct_smtsock_output *) _buffer;
    *params = mem_alloc (sizeof (struct_smtsock_output));
    if (*params)
      {
        (* params)-> timeout            = _struct_ptr-> timeout;
        (* params)-> socket             = _struct_ptr-> socket;
        (* params)-> tag                = _struct_ptr-> tag;
        return 0;
      }
    else
        return -1;
}


/*  ---------------------------------------------------------------------[<]-
    Function: free_smtsock_output

    Synopsis: frees a structure allocated by get_smtsock_output().
    ---------------------------------------------------------------------[>]-*/

void
free_smtsock_output (
    struct_smtsock_output **params)
{
    mem_free (*params);
    *params = NULL;
}

char *SMTSOCK_OUTPUT = "SMTSOCK OUTPUT";

/*  ---------------------------------------------------------------------[<]-
    Function: lsend_smtsock_output

    Synopsis: Sends a output - Wait for socket output
              event to the smtsock agent
    ---------------------------------------------------------------------[>]-*/

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
    const void *tag)                /*  User-defined request tag         */
{
    byte *_body;
    int   _size,
          _rc;
    _size = put_smtsock_output
                (&_body,
                 timeout,
                 socket,
                 tag);
    if (_size)
      {
        _rc = event_send (_to, _from, SMTSOCK_OUTPUT,
                          _body, _size,
                          _accept, _reject, _expire, _timeout);
        mem_free (_body);
        return _rc;
      }
    else
        return -1;
}


/*  ---------------------------------------------------------------------[<]-
    Function: put_smtsock_connect

    Synopsis: Formats a connect message, allocates a new buffer,
    and returns the formatted message in the buffer.  You should free the
    buffer using mem_free() when finished.  Returns the size of the buffer
    in bytes.
    ---------------------------------------------------------------------[>]-*/

int
put_smtsock_connect (
    byte **_buffer,
    const dbyte timeout,                /*  Timeout in seconds, zero = none  */
    const char *type,                   /*  Type, UDP or TCP                 */
    const char *host,                   /*  Host, name or dotted address, or NULL  */
    const char *service,                /*  Service, as name or port in ASCII, or NULL  */
    const dbyte port,                   /*  16-bit host port, or 0           */
    const qbyte address,                /*  32-bit host address, or 0        */
    const void *tag)                    /*  User-defined request tag         */
{
    struct_smtsock_connect
        *_struct_ptr;
    int
        _total_size = sizeof (struct_smtsock_connect);
    char
        *_ptr;

    _total_size += type ? strlen (type) + 1 : 0;
    _total_size += host ? strlen (host) + 1 : 0;
    _total_size += service ? strlen (service) + 1 : 0;
    _struct_ptr = mem_alloc (_total_size);
    *_buffer = (byte *) _struct_ptr;
    if (_struct_ptr)
      {
        _ptr = (char *) _struct_ptr + sizeof (struct_smtsock_connect);
        _struct_ptr-> timeout           = timeout;
        if (type)
          {
            _struct_ptr-> type              = (char *) _ptr;
            strcpy ((char *) _ptr, type);
            _ptr += strlen (type) + 1;
          }
        else
            _struct_ptr-> type              = NULL;
        if (host)
          {
            _struct_ptr-> host              = (char *) _ptr;
            strcpy ((char *) _ptr, host);
            _ptr += strlen (host) + 1;
          }
        else
            _struct_ptr-> host              = NULL;
        if (service)
          {
            _struct_ptr-> service           = (char *) _ptr;
            strcpy ((char *) _ptr, service);
            _ptr += strlen (service) + 1;
          }
        else
            _struct_ptr-> service           = NULL;
        _struct_ptr-> port              = port;
        _struct_ptr-> address           = address;
        _struct_ptr-> tag               = (void *) tag;

        return _total_size;
      }
    else
        return 0;
}


/*  ---------------------------------------------------------------------[<]-
    Function: get_smtsock_connect

    Synopsis: Accepts a buffer containing a connect message,
    and unpacks it into a new struct_smtsock_connect structure. Free the
    structure using free_smtsock_connect() when finished.
    ---------------------------------------------------------------------[>]-*/

int
get_smtsock_connect (
    byte *_buffer,
    struct_smtsock_connect **params)
{
    struct_smtsock_connect
        *_struct_ptr;
    char
        *_ptr;

    _struct_ptr = (struct_smtsock_connect *) _buffer;
    *params = mem_alloc (sizeof (struct_smtsock_connect));
    if (*params)
      {
        _ptr = (char *) _struct_ptr + sizeof (struct_smtsock_connect);
        (* params)-> timeout            = _struct_ptr-> timeout;
        if (_struct_ptr-> type)
          {
            (* params)-> type               = mem_strdup (_ptr);
            _ptr += strlen ((* params)-> type) + 1;
          }
        else
            (* params)-> type               = NULL;
        if (_struct_ptr-> host)
          {
            (* params)-> host               = mem_strdup (_ptr);
            _ptr += strlen ((* params)-> host) + 1;
          }
        else
            (* params)-> host               = NULL;
        if (_struct_ptr-> service)
          {
            (* params)-> service            = mem_strdup (_ptr);
            _ptr += strlen ((* params)-> service) + 1;
          }
        else
            (* params)-> service            = NULL;
        (* params)-> port               = _struct_ptr-> port;
        (* params)-> address            = _struct_ptr-> address;
        (* params)-> tag                = _struct_ptr-> tag;
        return 0;
      }
    else
        return -1;
}


/*  ---------------------------------------------------------------------[<]-
    Function: free_smtsock_connect

    Synopsis: frees a structure allocated by get_smtsock_connect().
    ---------------------------------------------------------------------[>]-*/

void
free_smtsock_connect (
    struct_smtsock_connect **params)
{
    mem_free ((*params)-> type);
    mem_free ((*params)-> host);
    mem_free ((*params)-> service);
    mem_free (*params);
    *params = NULL;
}

char *SMTSOCK_CONNECT = "SMTSOCK CONNECT";

/*  ---------------------------------------------------------------------[<]-
    Function: lsend_smtsock_connect

    Synopsis: Sends a connect - Connect to socket
              event to the smtsock agent
    ---------------------------------------------------------------------[>]-*/

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
    const void *tag)                /*  User-defined request tag         */
{
    byte *_body;
    int   _size,
          _rc;
    _size = put_smtsock_connect
                (&_body,
                 timeout,
                 type,
                 host,
                 service,
                 port,
                 address,
                 tag);
    if (_size)
      {
        _rc = event_send (_to, _from, SMTSOCK_CONNECT,
                          _body, _size,
                          _accept, _reject, _expire, _timeout);
        mem_free (_body);
        return _rc;
      }
    else
        return -1;
}


/*  ---------------------------------------------------------------------[<]-
    Function: put_smtsock_flush

    Synopsis: Formats a flush message, allocates a new buffer,
    and returns the formatted message in the buffer.  You should free the
    buffer using mem_free() when finished.  Returns the size of the buffer
    in bytes.
    ---------------------------------------------------------------------[>]-*/

int
put_smtsock_flush (
    byte **_buffer,
    const qbyte socket,                 /*  Socket for operation             */
    const Bool  alltypes)               /*  All request types, or just read?  */
{
    struct_smtsock_flush
        *_struct_ptr;

    _struct_ptr = mem_alloc (sizeof (struct_smtsock_flush));
    *_buffer = (byte *) _struct_ptr;
    if (_struct_ptr)
      {
        _struct_ptr-> socket            = socket;
        _struct_ptr-> alltypes          = alltypes;

        return sizeof (*_struct_ptr);
      }
    else
        return 0;
}


/*  ---------------------------------------------------------------------[<]-
    Function: get_smtsock_flush

    Synopsis: Accepts a buffer containing a flush message,
    and unpacks it into a new struct_smtsock_flush structure. Free the
    structure using free_smtsock_flush() when finished.
    ---------------------------------------------------------------------[>]-*/

int
get_smtsock_flush (
    byte *_buffer,
    struct_smtsock_flush **params)
{
    struct_smtsock_flush
        *_struct_ptr;

    _struct_ptr = (struct_smtsock_flush *) _buffer;
    *params = mem_alloc (sizeof (struct_smtsock_flush));
    if (*params)
      {
        (* params)-> socket             = _struct_ptr-> socket;
        (* params)-> alltypes           = _struct_ptr-> alltypes;
        return 0;
      }
    else
        return -1;
}


/*  ---------------------------------------------------------------------[<]-
    Function: free_smtsock_flush

    Synopsis: frees a structure allocated by get_smtsock_flush().
    ---------------------------------------------------------------------[>]-*/

void
free_smtsock_flush (
    struct_smtsock_flush **params)
{
    mem_free (*params);
    *params = NULL;
}

char *SMTSOCK_FLUSH = "SMTSOCK FLUSH";

/*  ---------------------------------------------------------------------[<]-
    Function: lsend_smtsock_flush

    Synopsis: Sends a flush - Flush socket
              event to the smtsock agent
    ---------------------------------------------------------------------[>]-*/

int
lsend_smtsock_flush (
    const QID  *_to,
    const QID  *_from,
          char *_accept,
          char *_reject,
          char *_expire,
          word _timeout,
    const qbyte socket,             /*  Socket for operation             */
    const Bool  alltypes)           /*  All request types, or just read?  */
{
    byte *_body;
    int   _size,
          _rc;
    _size = put_smtsock_flush
                (&_body,
                 socket,
                 alltypes);
    if (_size)
      {
        _rc = event_send (_to, _from, SMTSOCK_FLUSH,
                          _body, _size,
                          _accept, _reject, _expire, _timeout);
        mem_free (_body);
        return _rc;
      }
    else
        return -1;
}


/*  ---------------------------------------------------------------------[<]-
    Function: put_smtsock_read_reply

    Synopsis: Formats a read reply message, allocates a new buffer,
    and returns the formatted message in the buffer.  You should free the
    buffer using mem_free() when finished.  Returns the size of the buffer
    in bytes.
    ---------------------------------------------------------------------[>]-*/

int
put_smtsock_read_reply (
    byte **_buffer,
    const word  size,                   /*  Amount of data read              */
    const void *data,                   /*  Block of data read               */
    const void *tag)                    /*  User-defined request tag         */
{
    struct_smtsock_read_reply
        *_struct_ptr;
    int
        _total_size = sizeof (struct_smtsock_read_reply);
    char
        *_ptr;

    _total_size += size;
    _struct_ptr = mem_alloc (_total_size);
    *_buffer = (byte *) _struct_ptr;
    if (_struct_ptr)
      {
        _ptr = (char *) _struct_ptr + sizeof (struct_smtsock_read_reply);
        _struct_ptr-> size              = size;
        _struct_ptr-> data              = (byte *) _ptr;
        memcpy (_ptr, data, size);
        _ptr += size;
        _struct_ptr-> tag               = (void *) tag;

        return _total_size;
      }
    else
        return 0;
}


/*  ---------------------------------------------------------------------[<]-
    Function: get_smtsock_read_reply

    Synopsis: Accepts a buffer containing a read reply message,
    and unpacks it into a new struct_smtsock_read_reply structure. Free the
    structure using free_smtsock_read_reply() when finished.
    ---------------------------------------------------------------------[>]-*/

int
get_smtsock_read_reply (
    byte *_buffer,
    struct_smtsock_read_reply **params)
{
    struct_smtsock_read_reply
        *_struct_ptr;
    char
        *_ptr;

    _struct_ptr = (struct_smtsock_read_reply *) _buffer;
    *params = mem_alloc (sizeof (struct_smtsock_read_reply));
    if (*params)
      {
        _ptr = (char *) _struct_ptr + sizeof (struct_smtsock_read_reply);
        (* params)-> size               = _struct_ptr-> size;
        (* params)-> data               = mem_alloc (_struct_ptr-> size + 1);
        memcpy ((* params)-> data, _ptr, _struct_ptr-> size);
        *((byte *)(* params)-> data + _struct_ptr-> size) = 0;
        _ptr += _struct_ptr-> size;
        (* params)-> tag                = _struct_ptr-> tag;
        return 0;
      }
    else
        return -1;
}


/*  ---------------------------------------------------------------------[<]-
    Function: free_smtsock_read_reply

    Synopsis: frees a structure allocated by get_smtsock_read_reply().
    ---------------------------------------------------------------------[>]-*/

void
free_smtsock_read_reply (
    struct_smtsock_read_reply **params)
{
    mem_free ((*params)-> data);
    mem_free (*params);
    *params = NULL;
}

char *SMTSOCK_READ_OK = "SMTSOCK READ OK";

/*  ---------------------------------------------------------------------[<]-
    Function: lsend_smtsock_read_ok

    Synopsis: Sends a read ok -
              event to the smtsock agent
    ---------------------------------------------------------------------[>]-*/

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
    const void *tag)                /*  User-defined request tag         */
{
    byte *_body;
    int   _size,
          _rc;
    _size = put_smtsock_read_reply
                (&_body,
                 size,
                 data,
                 tag);
    if (_size)
      {
        _rc = event_send (_to, _from, SMTSOCK_READ_OK,
                          _body, _size,
                          _accept, _reject, _expire, _timeout);
        mem_free (_body);
        return _rc;
      }
    else
        return -1;
}

char *SMTSOCK_READ_CLOSED = "SMTSOCK READ CLOSED";

/*  ---------------------------------------------------------------------[<]-
    Function: lsend_smtsock_read_closed

    Synopsis: Sends a read closed -
              event to the smtsock agent
    ---------------------------------------------------------------------[>]-*/

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
    const void *tag)                /*  User-defined request tag         */
{
    byte *_body;
    int   _size,
          _rc;
    _size = put_smtsock_read_reply
                (&_body,
                 size,
                 data,
                 tag);
    if (_size)
      {
        _rc = event_send (_to, _from, SMTSOCK_READ_CLOSED,
                          _body, _size,
                          _accept, _reject, _expire, _timeout);
        mem_free (_body);
        return _rc;
      }
    else
        return -1;
}

char *SMTSOCK_READ_TIMEOUT = "SMTSOCK READ TIMEOUT";

/*  ---------------------------------------------------------------------[<]-
    Function: lsend_smtsock_read_timeout

    Synopsis: Sends a read timeout -
              event to the smtsock agent
    ---------------------------------------------------------------------[>]-*/

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
    const void *tag)                /*  User-defined request tag         */
{
    byte *_body;
    int   _size,
          _rc;
    _size = put_smtsock_read_reply
                (&_body,
                 size,
                 data,
                 tag);
    if (_size)
      {
        _rc = event_send (_to, _from, SMTSOCK_READ_TIMEOUT,
                          _body, _size,
                          _accept, _reject, _expire, _timeout);
        mem_free (_body);
        return _rc;
      }
    else
        return -1;
}


/*  ---------------------------------------------------------------------[<]-
    Function: put_smtsock_readh_reply

    Synopsis: Formats a readh reply message, allocates a new buffer,
    and returns the formatted message in the buffer.  You should free the
    buffer using mem_free() when finished.  Returns the size of the buffer
    in bytes.
    ---------------------------------------------------------------------[>]-*/

int
put_smtsock_readh_reply (
    byte **_buffer,
    const qbyte size,                   /*  Amount of data read              */
    const byte *data,                   /*  Block of data read               */
    const void *tag)                    /*  User-defined request tag         */
{
    struct_smtsock_readh_reply
        *_struct_ptr;
    int
        _total_size = sizeof (struct_smtsock_readh_reply);
    char
        *_ptr;

    _total_size += size;
    _struct_ptr = mem_alloc (_total_size);
    *_buffer = (byte *) _struct_ptr;
    if (_struct_ptr)
      {
        _ptr = (char *) _struct_ptr + sizeof (struct_smtsock_readh_reply);
        _struct_ptr-> size              = size;
        _struct_ptr-> data              = (byte *) _ptr;
        memcpy (_ptr, data, size);
        _ptr += size;
        _struct_ptr-> tag               = (void *) tag;

        return _total_size;
      }
    else
        return 0;
}


/*  ---------------------------------------------------------------------[<]-
    Function: get_smtsock_readh_reply

    Synopsis: Accepts a buffer containing a readh reply message,
    and unpacks it into a new struct_smtsock_readh_reply structure. Free the
    structure using free_smtsock_readh_reply() when finished.
    ---------------------------------------------------------------------[>]-*/

int
get_smtsock_readh_reply (
    byte *_buffer,
    struct_smtsock_readh_reply **params)
{
    struct_smtsock_readh_reply
        *_struct_ptr;
    char
        *_ptr;

    _struct_ptr = (struct_smtsock_readh_reply *) _buffer;
    *params = mem_alloc (sizeof (struct_smtsock_readh_reply));
    if (*params)
      {
        _ptr = (char *) _struct_ptr + sizeof (struct_smtsock_readh_reply);
        (* params)-> size               = _struct_ptr-> size;
        (* params)-> data               = mem_alloc (_struct_ptr-> size + 1);
        memcpy ((* params)-> data, _ptr, _struct_ptr-> size);
        *((byte *)(* params)-> data + _struct_ptr-> size) = 0;
        _ptr += _struct_ptr-> size;
        (* params)-> tag                = _struct_ptr-> tag;
        return 0;
      }
    else
        return -1;
}


/*  ---------------------------------------------------------------------[<]-
    Function: free_smtsock_readh_reply

    Synopsis: frees a structure allocated by get_smtsock_readh_reply().
    ---------------------------------------------------------------------[>]-*/

void
free_smtsock_readh_reply (
    struct_smtsock_readh_reply **params)
{
    mem_free ((*params)-> data);
    mem_free (*params);
    *params = NULL;
}

char *SMTSOCK_READH_OK = "SMTSOCK READH OK";

/*  ---------------------------------------------------------------------[<]-
    Function: lsend_smtsock_readh_ok

    Synopsis: Sends a readh ok -
              event to the smtsock agent
    ---------------------------------------------------------------------[>]-*/

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
    const void *tag)                /*  User-defined request tag         */
{
    byte *_body;
    int   _size,
          _rc;
    _size = put_smtsock_readh_reply
                (&_body,
                 size,
                 data,
                 tag);
    if (_size)
      {
        _rc = event_send (_to, _from, SMTSOCK_READH_OK,
                          _body, _size,
                          _accept, _reject, _expire, _timeout);
        mem_free (_body);
        return _rc;
      }
    else
        return -1;
}

char *SMTSOCK_READH_CLOSED = "SMTSOCK READH CLOSED";

/*  ---------------------------------------------------------------------[<]-
    Function: lsend_smtsock_readh_closed

    Synopsis: Sends a readh closed -
              event to the smtsock agent
    ---------------------------------------------------------------------[>]-*/

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
    const void *tag)                /*  User-defined request tag         */
{
    byte *_body;
    int   _size,
          _rc;
    _size = put_smtsock_readh_reply
                (&_body,
                 size,
                 data,
                 tag);
    if (_size)
      {
        _rc = event_send (_to, _from, SMTSOCK_READH_CLOSED,
                          _body, _size,
                          _accept, _reject, _expire, _timeout);
        mem_free (_body);
        return _rc;
      }
    else
        return -1;
}

char *SMTSOCK_READH_TIMEOUT = "SMTSOCK READH TIMEOUT";

/*  ---------------------------------------------------------------------[<]-
    Function: lsend_smtsock_readh_timeout

    Synopsis: Sends a readh timeout -
              event to the smtsock agent
    ---------------------------------------------------------------------[>]-*/

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
    const void *tag)                /*  User-defined request tag         */
{
    byte *_body;
    int   _size,
          _rc;
    _size = put_smtsock_readh_reply
                (&_body,
                 size,
                 data,
                 tag);
    if (_size)
      {
        _rc = event_send (_to, _from, SMTSOCK_READH_TIMEOUT,
                          _body, _size,
                          _accept, _reject, _expire, _timeout);
        mem_free (_body);
        return _rc;
      }
    else
        return -1;
}


/*  ---------------------------------------------------------------------[<]-
    Function: put_smtsock_connect_ok

    Synopsis: Formats a connect ok message, allocates a new buffer,
    and returns the formatted message in the buffer.  You should free the
    buffer using mem_free() when finished.  Returns the size of the buffer
    in bytes.
    ---------------------------------------------------------------------[>]-*/

int
put_smtsock_connect_ok (
    byte **_buffer,
    const qbyte socket,                 /*  New socket                       */
    const void *tag)                    /*  User-defined request tag         */
{
    struct_smtsock_connect_ok
        *_struct_ptr;

    _struct_ptr = mem_alloc (sizeof (struct_smtsock_connect_ok));
    *_buffer = (byte *) _struct_ptr;
    if (_struct_ptr)
      {
        _struct_ptr-> socket            = socket;
        _struct_ptr-> tag               = (void *) tag;

        return sizeof (*_struct_ptr);
      }
    else
        return 0;
}


/*  ---------------------------------------------------------------------[<]-
    Function: get_smtsock_connect_ok

    Synopsis: Accepts a buffer containing a connect ok message,
    and unpacks it into a new struct_smtsock_connect_ok structure. Free the
    structure using free_smtsock_connect_ok() when finished.
    ---------------------------------------------------------------------[>]-*/

int
get_smtsock_connect_ok (
    byte *_buffer,
    struct_smtsock_connect_ok **params)
{
    struct_smtsock_connect_ok
        *_struct_ptr;

    _struct_ptr = (struct_smtsock_connect_ok *) _buffer;
    *params = mem_alloc (sizeof (struct_smtsock_connect_ok));
    if (*params)
      {
        (* params)-> socket             = _struct_ptr-> socket;
        (* params)-> tag                = _struct_ptr-> tag;
        return 0;
      }
    else
        return -1;
}


/*  ---------------------------------------------------------------------[<]-
    Function: free_smtsock_connect_ok

    Synopsis: frees a structure allocated by get_smtsock_connect_ok().
    ---------------------------------------------------------------------[>]-*/

void
free_smtsock_connect_ok (
    struct_smtsock_connect_ok **params)
{
    mem_free (*params);
    *params = NULL;
}

char *SMTSOCK_CONNECT_OK = "SMTSOCK CONNECT OK";

/*  ---------------------------------------------------------------------[<]-
    Function: lsend_smtsock_connect_ok

    Synopsis: Sends a connect ok -
              event to the smtsock agent
    ---------------------------------------------------------------------[>]-*/

int
lsend_smtsock_connect_ok (
    const QID  *_to,
    const QID  *_from,
          char *_accept,
          char *_reject,
          char *_expire,
          word _timeout,
    const qbyte socket,             /*  New socket                       */
    const void *tag)                /*  User-defined request tag         */
{
    byte *_body;
    int   _size,
          _rc;
    _size = put_smtsock_connect_ok
                (&_body,
                 socket,
                 tag);
    if (_size)
      {
        _rc = event_send (_to, _from, SMTSOCK_CONNECT_OK,
                          _body, _size,
                          _accept, _reject, _expire, _timeout);
        mem_free (_body);
        return _rc;
      }
    else
        return -1;
}


/*  ---------------------------------------------------------------------[<]-
    Function: put_smtsock_reply

    Synopsis: Formats a reply message, allocates a new buffer,
    and returns the formatted message in the buffer.  You should free the
    buffer using mem_free() when finished.  Returns the size of the buffer
    in bytes.
    ---------------------------------------------------------------------[>]-*/

int
put_smtsock_reply (
    byte **_buffer,
    const void *tag)                    /*  User-defined request tag         */
{
    struct_smtsock_reply
        *_struct_ptr;

    _struct_ptr = mem_alloc (sizeof (struct_smtsock_reply));
    *_buffer = (byte *) _struct_ptr;
    if (_struct_ptr)
      {
        _struct_ptr-> tag               = (void *) tag;

        return sizeof (*_struct_ptr);
      }
    else
        return 0;
}


/*  ---------------------------------------------------------------------[<]-
    Function: get_smtsock_reply

    Synopsis: Accepts a buffer containing a reply message,
    and unpacks it into a new struct_smtsock_reply structure. Free the
    structure using free_smtsock_reply() when finished.
    ---------------------------------------------------------------------[>]-*/

int
get_smtsock_reply (
    byte *_buffer,
    struct_smtsock_reply **params)
{
    struct_smtsock_reply
        *_struct_ptr;

    _struct_ptr = (struct_smtsock_reply *) _buffer;
    *params = mem_alloc (sizeof (struct_smtsock_reply));
    if (*params)
      {
        (* params)-> tag                = _struct_ptr-> tag;
        return 0;
      }
    else
        return -1;
}


/*  ---------------------------------------------------------------------[<]-
    Function: free_smtsock_reply

    Synopsis: frees a structure allocated by get_smtsock_reply().
    ---------------------------------------------------------------------[>]-*/

void
free_smtsock_reply (
    struct_smtsock_reply **params)
{
    mem_free (*params);
    *params = NULL;
}

char *SMTSOCK_OK = "SMTSOCK OK";

/*  ---------------------------------------------------------------------[<]-
    Function: lsend_smtsock_ok

    Synopsis: Sends a ok -
              event to the smtsock agent
    ---------------------------------------------------------------------[>]-*/

int
lsend_smtsock_ok (
    const QID  *_to,
    const QID  *_from,
          char *_accept,
          char *_reject,
          char *_expire,
          word _timeout,
    const void *tag)                /*  User-defined request tag         */
{
    byte *_body;
    int   _size,
          _rc;
    _size = put_smtsock_reply
                (&_body,
                 tag);
    if (_size)
      {
        _rc = event_send (_to, _from, SMTSOCK_OK,
                          _body, _size,
                          _accept, _reject, _expire, _timeout);
        mem_free (_body);
        return _rc;
      }
    else
        return -1;
}

char *SMTSOCK_CLOSED = "SMTSOCK CLOSED";

/*  ---------------------------------------------------------------------[<]-
    Function: lsend_smtsock_closed

    Synopsis: Sends a closed -
              event to the smtsock agent
    ---------------------------------------------------------------------[>]-*/

int
lsend_smtsock_closed (
    const QID  *_to,
    const QID  *_from,
          char *_accept,
          char *_reject,
          char *_expire,
          word _timeout,
    const void *tag)                /*  User-defined request tag         */
{
    byte *_body;
    int   _size,
          _rc;
    _size = put_smtsock_reply
                (&_body,
                 tag);
    if (_size)
      {
        _rc = event_send (_to, _from, SMTSOCK_CLOSED,
                          _body, _size,
                          _accept, _reject, _expire, _timeout);
        mem_free (_body);
        return _rc;
      }
    else
        return -1;
}

char *SMTSOCK_TIMEOUT = "SMTSOCK TIMEOUT";

/*  ---------------------------------------------------------------------[<]-
    Function: lsend_smtsock_timeout

    Synopsis: Sends a timeout -
              event to the smtsock agent
    ---------------------------------------------------------------------[>]-*/

int
lsend_smtsock_timeout (
    const QID  *_to,
    const QID  *_from,
          char *_accept,
          char *_reject,
          char *_expire,
          word _timeout,
    const void *tag)                /*  User-defined request tag         */
{
    byte *_body;
    int   _size,
          _rc;
    _size = put_smtsock_reply
                (&_body,
                 tag);
    if (_size)
      {
        _rc = event_send (_to, _from, SMTSOCK_TIMEOUT,
                          _body, _size,
                          _accept, _reject, _expire, _timeout);
        mem_free (_body);
        return _rc;
      }
    else
        return -1;
}


/*  ---------------------------------------------------------------------[<]-
    Function: put_smtsock_error

    Synopsis: Formats a error message, allocates a new buffer,
    and returns the formatted message in the buffer.  You should free the
    buffer using mem_free() when finished.  Returns the size of the buffer
    in bytes.
    ---------------------------------------------------------------------[>]-*/

int
put_smtsock_error (
    byte **_buffer,
    const char *message,                /*  Error message                    */
    const void *tag)                    /*  User-defined request tag         */
{
    struct_smtsock_error
        *_struct_ptr;
    int
        _total_size = sizeof (struct_smtsock_error);
    char
        *_ptr;

    _total_size += message ? strlen (message) + 1 : 0;
    _struct_ptr = mem_alloc (_total_size);
    *_buffer = (byte *) _struct_ptr;
    if (_struct_ptr)
      {
        _ptr = (char *) _struct_ptr + sizeof (struct_smtsock_error);
        if (message)
          {
            _struct_ptr-> message           = (char *) _ptr;
            strcpy ((char *) _ptr, message);
            _ptr += strlen (message) + 1;
          }
        else
            _struct_ptr-> message           = NULL;
        _struct_ptr-> tag               = (void *) tag;

        return _total_size;
      }
    else
        return 0;
}


/*  ---------------------------------------------------------------------[<]-
    Function: get_smtsock_error

    Synopsis: Accepts a buffer containing a error message,
    and unpacks it into a new struct_smtsock_error structure. Free the
    structure using free_smtsock_error() when finished.
    ---------------------------------------------------------------------[>]-*/

int
get_smtsock_error (
    byte *_buffer,
    struct_smtsock_error **params)
{
    struct_smtsock_error
        *_struct_ptr;
    char
        *_ptr;

    _struct_ptr = (struct_smtsock_error *) _buffer;
    *params = mem_alloc (sizeof (struct_smtsock_error));
    if (*params)
      {
        _ptr = (char *) _struct_ptr + sizeof (struct_smtsock_error);
        if (_struct_ptr-> message)
          {
            (* params)-> message            = mem_strdup (_ptr);
            _ptr += strlen ((* params)-> message) + 1;
          }
        else
            (* params)-> message            = NULL;
        (* params)-> tag                = _struct_ptr-> tag;
        return 0;
      }
    else
        return -1;
}


/*  ---------------------------------------------------------------------[<]-
    Function: free_smtsock_error

    Synopsis: frees a structure allocated by get_smtsock_error().
    ---------------------------------------------------------------------[>]-*/

void
free_smtsock_error (
    struct_smtsock_error **params)
{
    mem_free ((*params)-> message);
    mem_free (*params);
    *params = NULL;
}

char *SMTSOCK_ERROR = "SMTSOCK ERROR";

/*  ---------------------------------------------------------------------[<]-
    Function: lsend_smtsock_error

    Synopsis: Sends a error -
              event to the smtsock agent
    ---------------------------------------------------------------------[>]-*/

int
lsend_smtsock_error (
    const QID  *_to,
    const QID  *_from,
          char *_accept,
          char *_reject,
          char *_expire,
          word _timeout,
    const char *message,            /*  Error message                    */
    const void *tag)                /*  User-defined request tag         */
{
    byte *_body;
    int   _size,
          _rc;
    _size = put_smtsock_error
                (&_body,
                 message,
                 tag);
    if (_size)
      {
        _rc = event_send (_to, _from, SMTSOCK_ERROR,
                          _body, _size,
                          _accept, _reject, _expire, _timeout);
        mem_free (_body);
        return _rc;
      }
    else
        return -1;
}

/*---------------------------------------------------------------------------
 *  Message functions for smttran - Transfer agent.
 *---------------------------------------------------------------------------*/


/*  ---------------------------------------------------------------------[<]-
    Function: put_smttran_putb

    Synopsis: Formats a putb message, allocates a new buffer,
    and returns the formatted message in the buffer.  You should free the
    buffer using mem_free() when finished.  Returns the size of the buffer
    in bytes.
    ---------------------------------------------------------------------[>]-*/

int
put_smttran_putb (
    byte **_buffer,
    const qbyte socket,                 /*  Socket for output                */
    const word  size,                   /*  Amount of data to send           */
    const void *data,                   /*  Block of data to send            */
    const char *pipe)                   /*  Transfer pipe, if any            */
{
    struct_smttran_putb
        *_struct_ptr;
    int
        _total_size = sizeof (struct_smttran_putb);
    char
        *_ptr;

    _total_size += size;
    _total_size += pipe ? strlen (pipe) + 1 : 0;
    _struct_ptr = mem_alloc (_total_size);
    *_buffer = (byte *) _struct_ptr;
    if (_struct_ptr)
      {
        _ptr = (char *) _struct_ptr + sizeof (struct_smttran_putb);
        _struct_ptr-> socket            = socket;
        _struct_ptr-> size              = size;
        _struct_ptr-> data              = (byte *) _ptr;
        memcpy (_ptr, data, size);
        _ptr += size;
        if (pipe)
          {
            _struct_ptr-> pipe              = (char *) _ptr;
            strcpy ((char *) _ptr, pipe);
            _ptr += strlen (pipe) + 1;
          }
        else
            _struct_ptr-> pipe              = NULL;

        return _total_size;
      }
    else
        return 0;
}


/*  ---------------------------------------------------------------------[<]-
    Function: get_smttran_putb

    Synopsis: Accepts a buffer containing a putb message,
    and unpacks it into a new struct_smttran_putb structure. Free the
    structure using free_smttran_putb() when finished.
    ---------------------------------------------------------------------[>]-*/

int
get_smttran_putb (
    byte *_buffer,
    struct_smttran_putb **params)
{
    struct_smttran_putb
        *_struct_ptr;
    char
        *_ptr;

    _struct_ptr = (struct_smttran_putb *) _buffer;
    *params = mem_alloc (sizeof (struct_smttran_putb));
    if (*params)
      {
        _ptr = (char *) _struct_ptr + sizeof (struct_smttran_putb);
        (* params)-> socket             = _struct_ptr-> socket;
        (* params)-> size               = _struct_ptr-> size;
        (* params)-> data               = mem_alloc (_struct_ptr-> size + 1);
        memcpy ((* params)-> data, _ptr, _struct_ptr-> size);
        *((byte *)(* params)-> data + _struct_ptr-> size) = 0;
        _ptr += _struct_ptr-> size;
        if (_struct_ptr-> pipe)
          {
            (* params)-> pipe               = mem_strdup (_ptr);
            _ptr += strlen ((* params)-> pipe) + 1;
          }
        else
            (* params)-> pipe               = NULL;
        return 0;
      }
    else
        return -1;
}


/*  ---------------------------------------------------------------------[<]-
    Function: free_smttran_putb

    Synopsis: frees a structure allocated by get_smttran_putb().
    ---------------------------------------------------------------------[>]-*/

void
free_smttran_putb (
    struct_smttran_putb **params)
{
    mem_free ((*params)-> data);
    mem_free ((*params)-> pipe);
    mem_free (*params);
    *params = NULL;
}

char *SMTTRAN_PUT_BLOCK = "SMTTRAN PUT BLOCK";

/*  ---------------------------------------------------------------------[<]-
    Function: lsend_smttran_put_block

    Synopsis: Sends a put block -
              event to the smttran agent
    ---------------------------------------------------------------------[>]-*/

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
    const char *pipe)               /*  Transfer pipe, if any            */
{
    byte *_body;
    int   _size,
          _rc;
    _size = put_smttran_putb
                (&_body,
                 socket,
                 size,
                 data,
                 pipe);
    if (_size)
      {
        _rc = event_send (_to, _from, SMTTRAN_PUT_BLOCK,
                          _body, _size,
                          _accept, _reject, _expire, _timeout);
        mem_free (_body);
        return _rc;
      }
    else
        return -1;
}


/*  ---------------------------------------------------------------------[<]-
    Function: put_smttran_getb

    Synopsis: Formats a getb message, allocates a new buffer,
    and returns the formatted message in the buffer.  You should free the
    buffer using mem_free() when finished.  Returns the size of the buffer
    in bytes.
    ---------------------------------------------------------------------[>]-*/

int
put_smttran_getb (
    byte **_buffer,
    const qbyte socket,                 /*  Socket for input                 */
    const char *pipe)                   /*  Transfer pipe, if any            */
{
    struct_smttran_getb
        *_struct_ptr;
    int
        _total_size = sizeof (struct_smttran_getb);
    char
        *_ptr;

    _total_size += pipe ? strlen (pipe) + 1 : 0;
    _struct_ptr = mem_alloc (_total_size);
    *_buffer = (byte *) _struct_ptr;
    if (_struct_ptr)
      {
        _ptr = (char *) _struct_ptr + sizeof (struct_smttran_getb);
        _struct_ptr-> socket            = socket;
        if (pipe)
          {
            _struct_ptr-> pipe              = (char *) _ptr;
            strcpy ((char *) _ptr, pipe);
            _ptr += strlen (pipe) + 1;
          }
        else
            _struct_ptr-> pipe              = NULL;

        return _total_size;
      }
    else
        return 0;
}


/*  ---------------------------------------------------------------------[<]-
    Function: get_smttran_getb

    Synopsis: Accepts a buffer containing a getb message,
    and unpacks it into a new struct_smttran_getb structure. Free the
    structure using free_smttran_getb() when finished.
    ---------------------------------------------------------------------[>]-*/

int
get_smttran_getb (
    byte *_buffer,
    struct_smttran_getb **params)
{
    struct_smttran_getb
        *_struct_ptr;
    char
        *_ptr;

    _struct_ptr = (struct_smttran_getb *) _buffer;
    *params = mem_alloc (sizeof (struct_smttran_getb));
    if (*params)
      {
        _ptr = (char *) _struct_ptr + sizeof (struct_smttran_getb);
        (* params)-> socket             = _struct_ptr-> socket;
        if (_struct_ptr-> pipe)
          {
            (* params)-> pipe               = mem_strdup (_ptr);
            _ptr += strlen ((* params)-> pipe) + 1;
          }
        else
            (* params)-> pipe               = NULL;
        return 0;
      }
    else
        return -1;
}


/*  ---------------------------------------------------------------------[<]-
    Function: free_smttran_getb

    Synopsis: frees a structure allocated by get_smttran_getb().
    ---------------------------------------------------------------------[>]-*/

void
free_smttran_getb (
    struct_smttran_getb **params)
{
    mem_free ((*params)-> pipe);
    mem_free (*params);
    *params = NULL;
}

char *SMTTRAN_GET_BLOCK = "SMTTRAN GET BLOCK";

/*  ---------------------------------------------------------------------[<]-
    Function: lsend_smttran_get_block

    Synopsis: Sends a get block -
              event to the smttran agent
    ---------------------------------------------------------------------[>]-*/

int
lsend_smttran_get_block (
    const QID  *_to,
    const QID  *_from,
          char *_accept,
          char *_reject,
          char *_expire,
          word _timeout,
    const qbyte socket,             /*  Socket for input                 */
    const char *pipe)               /*  Transfer pipe, if any            */
{
    byte *_body;
    int   _size,
          _rc;
    _size = put_smttran_getb
                (&_body,
                 socket,
                 pipe);
    if (_size)
      {
        _rc = event_send (_to, _from, SMTTRAN_GET_BLOCK,
                          _body, _size,
                          _accept, _reject, _expire, _timeout);
        mem_free (_body);
        return _rc;
      }
    else
        return -1;
}


/*  ---------------------------------------------------------------------[<]-
    Function: put_smttran_puth

    Synopsis: Formats a puth message, allocates a new buffer,
    and returns the formatted message in the buffer.  You should free the
    buffer using mem_free() when finished.  Returns the size of the buffer
    in bytes.
    ---------------------------------------------------------------------[>]-*/

int
put_smttran_puth (
    byte **_buffer,
    const qbyte socket,                 /*  Socket for output                */
    const qbyte size,                   /*  Amount of data to send           */
    const byte *data,                   /*  Block of data to send            */
    const char *pipe)                   /*  Transfer pipe, if any            */
{
    struct_smttran_puth
        *_struct_ptr;
    int
        _total_size = sizeof (struct_smttran_puth);
    char
        *_ptr;

    _total_size += size;
    _total_size += pipe ? strlen (pipe) + 1 : 0;
    _struct_ptr = mem_alloc (_total_size);
    *_buffer = (byte *) _struct_ptr;
    if (_struct_ptr)
      {
        _ptr = (char *) _struct_ptr + sizeof (struct_smttran_puth);
        _struct_ptr-> socket            = socket;
        _struct_ptr-> size              = size;
        _struct_ptr-> data              = (byte *) _ptr;
        memcpy (_ptr, data, size);
        _ptr += size;
        if (pipe)
          {
            _struct_ptr-> pipe              = (char *) _ptr;
            strcpy ((char *) _ptr, pipe);
            _ptr += strlen (pipe) + 1;
          }
        else
            _struct_ptr-> pipe              = NULL;

        return _total_size;
      }
    else
        return 0;
}


/*  ---------------------------------------------------------------------[<]-
    Function: get_smttran_puth

    Synopsis: Accepts a buffer containing a puth message,
    and unpacks it into a new struct_smttran_puth structure. Free the
    structure using free_smttran_puth() when finished.
    ---------------------------------------------------------------------[>]-*/

int
get_smttran_puth (
    byte *_buffer,
    struct_smttran_puth **params)
{
    struct_smttran_puth
        *_struct_ptr;
    char
        *_ptr;

    _struct_ptr = (struct_smttran_puth *) _buffer;
    *params = mem_alloc (sizeof (struct_smttran_puth));
    if (*params)
      {
        _ptr = (char *) _struct_ptr + sizeof (struct_smttran_puth);
        (* params)-> socket             = _struct_ptr-> socket;
        (* params)-> size               = _struct_ptr-> size;
        (* params)-> data               = mem_alloc (_struct_ptr-> size + 1);
        memcpy ((* params)-> data, _ptr, _struct_ptr-> size);
        *((byte *)(* params)-> data + _struct_ptr-> size) = 0;
        _ptr += _struct_ptr-> size;
        if (_struct_ptr-> pipe)
          {
            (* params)-> pipe               = mem_strdup (_ptr);
            _ptr += strlen ((* params)-> pipe) + 1;
          }
        else
            (* params)-> pipe               = NULL;
        return 0;
      }
    else
        return -1;
}


/*  ---------------------------------------------------------------------[<]-
    Function: free_smttran_puth

    Synopsis: frees a structure allocated by get_smttran_puth().
    ---------------------------------------------------------------------[>]-*/

void
free_smttran_puth (
    struct_smttran_puth **params)
{
    mem_free ((*params)-> data);
    mem_free ((*params)-> pipe);
    mem_free (*params);
    *params = NULL;
}

char *SMTTRAN_PUT_HUGE = "SMTTRAN PUT HUGE";

/*  ---------------------------------------------------------------------[<]-
    Function: lsend_smttran_put_huge

    Synopsis: Sends a put huge -
              event to the smttran agent
    ---------------------------------------------------------------------[>]-*/

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
    const char *pipe)               /*  Transfer pipe, if any            */
{
    byte *_body;
    int   _size,
          _rc;
    _size = put_smttran_puth
                (&_body,
                 socket,
                 size,
                 data,
                 pipe);
    if (_size)
      {
        _rc = event_send (_to, _from, SMTTRAN_PUT_HUGE,
                          _body, _size,
                          _accept, _reject, _expire, _timeout);
        mem_free (_body);
        return _rc;
      }
    else
        return -1;
}


/*  ---------------------------------------------------------------------[<]-
    Function: put_smttran_geth

    Synopsis: Formats a geth message, allocates a new buffer,
    and returns the formatted message in the buffer.  You should free the
    buffer using mem_free() when finished.  Returns the size of the buffer
    in bytes.
    ---------------------------------------------------------------------[>]-*/

int
put_smttran_geth (
    byte **_buffer,
    const qbyte socket,                 /*  Socket for input                 */
    const char *pipe)                   /*  Transfer pipe, if any            */
{
    struct_smttran_geth
        *_struct_ptr;
    int
        _total_size = sizeof (struct_smttran_geth);
    char
        *_ptr;

    _total_size += pipe ? strlen (pipe) + 1 : 0;
    _struct_ptr = mem_alloc (_total_size);
    *_buffer = (byte *) _struct_ptr;
    if (_struct_ptr)
      {
        _ptr = (char *) _struct_ptr + sizeof (struct_smttran_geth);
        _struct_ptr-> socket            = socket;
        if (pipe)
          {
            _struct_ptr-> pipe              = (char *) _ptr;
            strcpy ((char *) _ptr, pipe);
            _ptr += strlen (pipe) + 1;
          }
        else
            _struct_ptr-> pipe              = NULL;

        return _total_size;
      }
    else
        return 0;
}


/*  ---------------------------------------------------------------------[<]-
    Function: get_smttran_geth

    Synopsis: Accepts a buffer containing a geth message,
    and unpacks it into a new struct_smttran_geth structure. Free the
    structure using free_smttran_geth() when finished.
    ---------------------------------------------------------------------[>]-*/

int
get_smttran_geth (
    byte *_buffer,
    struct_smttran_geth **params)
{
    struct_smttran_geth
        *_struct_ptr;
    char
        *_ptr;

    _struct_ptr = (struct_smttran_geth *) _buffer;
    *params = mem_alloc (sizeof (struct_smttran_geth));
    if (*params)
      {
        _ptr = (char *) _struct_ptr + sizeof (struct_smttran_geth);
        (* params)-> socket             = _struct_ptr-> socket;
        if (_struct_ptr-> pipe)
          {
            (* params)-> pipe               = mem_strdup (_ptr);
            _ptr += strlen ((* params)-> pipe) + 1;
          }
        else
            (* params)-> pipe               = NULL;
        return 0;
      }
    else
        return -1;
}


/*  ---------------------------------------------------------------------[<]-
    Function: free_smttran_geth

    Synopsis: frees a structure allocated by get_smttran_geth().
    ---------------------------------------------------------------------[>]-*/

void
free_smttran_geth (
    struct_smttran_geth **params)
{
    mem_free ((*params)-> pipe);
    mem_free (*params);
    *params = NULL;
}

char *SMTTRAN_GET_HUGE = "SMTTRAN GET HUGE";

/*  ---------------------------------------------------------------------[<]-
    Function: lsend_smttran_get_huge

    Synopsis: Sends a get huge -
              event to the smttran agent
    ---------------------------------------------------------------------[>]-*/

int
lsend_smttran_get_huge (
    const QID  *_to,
    const QID  *_from,
          char *_accept,
          char *_reject,
          char *_expire,
          word _timeout,
    const qbyte socket,             /*  Socket for input                 */
    const char *pipe)               /*  Transfer pipe, if any            */
{
    byte *_body;
    int   _size,
          _rc;
    _size = put_smttran_geth
                (&_body,
                 socket,
                 pipe);
    if (_size)
      {
        _rc = event_send (_to, _from, SMTTRAN_GET_HUGE,
                          _body, _size,
                          _accept, _reject, _expire, _timeout);
        mem_free (_body);
        return _rc;
      }
    else
        return -1;
}


/*  ---------------------------------------------------------------------[<]-
    Function: put_smttran_putf

    Synopsis: Formats a putf message, allocates a new buffer,
    and returns the formatted message in the buffer.  You should free the
    buffer using mem_free() when finished.  Returns the size of the buffer
    in bytes.
    ---------------------------------------------------------------------[>]-*/

int
put_smttran_putf (
    byte **_buffer,
    const qbyte socket,                 /*  Socket for output                */
    const char *filename,               /*  Name of file to send             */
    const dbyte filetype,               /*  0=binary, 1=ASCII                */
    const qbyte start,                  /*  Starting offset; 0 = start       */
    const qbyte end,                    /*  Ending offset; 0 = end           */
    const char *pipe)                   /*  Transfer pipe, if any            */
{
    struct_smttran_putf
        *_struct_ptr;
    int
        _total_size = sizeof (struct_smttran_putf);
    char
        *_ptr;

    _total_size += filename ? strlen (filename) + 1 : 0;
    _total_size += pipe ? strlen (pipe) + 1 : 0;
    _struct_ptr = mem_alloc (_total_size);
    *_buffer = (byte *) _struct_ptr;
    if (_struct_ptr)
      {
        _ptr = (char *) _struct_ptr + sizeof (struct_smttran_putf);
        _struct_ptr-> socket            = socket;
        if (filename)
          {
            _struct_ptr-> filename          = (char *) _ptr;
            strcpy ((char *) _ptr, filename);
            _ptr += strlen (filename) + 1;
          }
        else
            _struct_ptr-> filename          = NULL;
        _struct_ptr-> filetype          = filetype;
        _struct_ptr-> start             = start;
        _struct_ptr-> end               = end;
        if (pipe)
          {
            _struct_ptr-> pipe              = (char *) _ptr;
            strcpy ((char *) _ptr, pipe);
            _ptr += strlen (pipe) + 1;
          }
        else
            _struct_ptr-> pipe              = NULL;

        return _total_size;
      }
    else
        return 0;
}


/*  ---------------------------------------------------------------------[<]-
    Function: get_smttran_putf

    Synopsis: Accepts a buffer containing a putf message,
    and unpacks it into a new struct_smttran_putf structure. Free the
    structure using free_smttran_putf() when finished.
    ---------------------------------------------------------------------[>]-*/

int
get_smttran_putf (
    byte *_buffer,
    struct_smttran_putf **params)
{
    struct_smttran_putf
        *_struct_ptr;
    char
        *_ptr;

    _struct_ptr = (struct_smttran_putf *) _buffer;
    *params = mem_alloc (sizeof (struct_smttran_putf));
    if (*params)
      {
        _ptr = (char *) _struct_ptr + sizeof (struct_smttran_putf);
        (* params)-> socket             = _struct_ptr-> socket;
        if (_struct_ptr-> filename)
          {
            (* params)-> filename           = mem_strdup (_ptr);
            _ptr += strlen ((* params)-> filename) + 1;
          }
        else
            (* params)-> filename           = NULL;
        (* params)-> filetype           = _struct_ptr-> filetype;
        (* params)-> start              = _struct_ptr-> start;
        (* params)-> end                = _struct_ptr-> end;
        if (_struct_ptr-> pipe)
          {
            (* params)-> pipe               = mem_strdup (_ptr);
            _ptr += strlen ((* params)-> pipe) + 1;
          }
        else
            (* params)-> pipe               = NULL;
        return 0;
      }
    else
        return -1;
}


/*  ---------------------------------------------------------------------[<]-
    Function: free_smttran_putf

    Synopsis: frees a structure allocated by get_smttran_putf().
    ---------------------------------------------------------------------[>]-*/

void
free_smttran_putf (
    struct_smttran_putf **params)
{
    mem_free ((*params)-> filename);
    mem_free ((*params)-> pipe);
    mem_free (*params);
    *params = NULL;
}

char *SMTTRAN_PUT_FILE = "SMTTRAN PUT FILE";

/*  ---------------------------------------------------------------------[<]-
    Function: lsend_smttran_put_file

    Synopsis: Sends a put file -
              event to the smttran agent
    ---------------------------------------------------------------------[>]-*/

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
    const char *pipe)               /*  Transfer pipe, if any            */
{
    byte *_body;
    int   _size,
          _rc;
    _size = put_smttran_putf
                (&_body,
                 socket,
                 filename,
                 filetype,
                 start,
                 end,
                 pipe);
    if (_size)
      {
        _rc = event_send (_to, _from, SMTTRAN_PUT_FILE,
                          _body, _size,
                          _accept, _reject, _expire, _timeout);
        mem_free (_body);
        return _rc;
      }
    else
        return -1;
}


/*  ---------------------------------------------------------------------[<]-
    Function: put_smttran_getf

    Synopsis: Formats a getf message, allocates a new buffer,
    and returns the formatted message in the buffer.  You should free the
    buffer using mem_free() when finished.  Returns the size of the buffer
    in bytes.
    ---------------------------------------------------------------------[>]-*/

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
    const char *pipe)                   /*  Transfer pipe, if any            */
{
    struct_smttran_getf
        *_struct_ptr;
    int
        _total_size = sizeof (struct_smttran_getf);
    char
        *_ptr;

    _total_size += filename ? strlen (filename) + 1 : 0;
    _total_size += pipe ? strlen (pipe) + 1 : 0;
    _struct_ptr = mem_alloc (_total_size);
    *_buffer = (byte *) _struct_ptr;
    if (_struct_ptr)
      {
        _ptr = (char *) _struct_ptr + sizeof (struct_smttran_getf);
        _struct_ptr-> socket            = socket;
        if (filename)
          {
            _struct_ptr-> filename          = (char *) _ptr;
            strcpy ((char *) _ptr, filename);
            _ptr += strlen (filename) + 1;
          }
        else
            _struct_ptr-> filename          = NULL;
        _struct_ptr-> filetype          = filetype;
        _struct_ptr-> start             = start;
        _struct_ptr-> end               = end;
        _struct_ptr-> append            = append;
        _struct_ptr-> maxsize           = maxsize;
        if (pipe)
          {
            _struct_ptr-> pipe              = (char *) _ptr;
            strcpy ((char *) _ptr, pipe);
            _ptr += strlen (pipe) + 1;
          }
        else
            _struct_ptr-> pipe              = NULL;

        return _total_size;
      }
    else
        return 0;
}


/*  ---------------------------------------------------------------------[<]-
    Function: get_smttran_getf

    Synopsis: Accepts a buffer containing a getf message,
    and unpacks it into a new struct_smttran_getf structure. Free the
    structure using free_smttran_getf() when finished.
    ---------------------------------------------------------------------[>]-*/

int
get_smttran_getf (
    byte *_buffer,
    struct_smttran_getf **params)
{
    struct_smttran_getf
        *_struct_ptr;
    char
        *_ptr;

    _struct_ptr = (struct_smttran_getf *) _buffer;
    *params = mem_alloc (sizeof (struct_smttran_getf));
    if (*params)
      {
        _ptr = (char *) _struct_ptr + sizeof (struct_smttran_getf);
        (* params)-> socket             = _struct_ptr-> socket;
        if (_struct_ptr-> filename)
          {
            (* params)-> filename           = mem_strdup (_ptr);
            _ptr += strlen ((* params)-> filename) + 1;
          }
        else
            (* params)-> filename           = NULL;
        (* params)-> filetype           = _struct_ptr-> filetype;
        (* params)-> start              = _struct_ptr-> start;
        (* params)-> end                = _struct_ptr-> end;
        (* params)-> append             = _struct_ptr-> append;
        (* params)-> maxsize            = _struct_ptr-> maxsize;
        if (_struct_ptr-> pipe)
          {
            (* params)-> pipe               = mem_strdup (_ptr);
            _ptr += strlen ((* params)-> pipe) + 1;
          }
        else
            (* params)-> pipe               = NULL;
        return 0;
      }
    else
        return -1;
}


/*  ---------------------------------------------------------------------[<]-
    Function: free_smttran_getf

    Synopsis: frees a structure allocated by get_smttran_getf().
    ---------------------------------------------------------------------[>]-*/

void
free_smttran_getf (
    struct_smttran_getf **params)
{
    mem_free ((*params)-> filename);
    mem_free ((*params)-> pipe);
    mem_free (*params);
    *params = NULL;
}

char *SMTTRAN_GET_FILE = "SMTTRAN GET FILE";

/*  ---------------------------------------------------------------------[<]-
    Function: lsend_smttran_get_file

    Synopsis: Sends a get file -
              event to the smttran agent
    ---------------------------------------------------------------------[>]-*/

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
    const char *pipe)               /*  Transfer pipe, if any            */
{
    byte *_body;
    int   _size,
          _rc;
    _size = put_smttran_getf
                (&_body,
                 socket,
                 filename,
                 filetype,
                 start,
                 end,
                 append,
                 maxsize,
                 pipe);
    if (_size)
      {
        _rc = event_send (_to, _from, SMTTRAN_GET_FILE,
                          _body, _size,
                          _accept, _reject, _expire, _timeout);
        mem_free (_body);
        return _rc;
      }
    else
        return -1;
}


/*  ---------------------------------------------------------------------[<]-
    Function: put_smttran_putb_ok

    Synopsis: Formats a putb ok message, allocates a new buffer,
    and returns the formatted message in the buffer.  You should free the
    buffer using mem_free() when finished.  Returns the size of the buffer
    in bytes.
    ---------------------------------------------------------------------[>]-*/

int
put_smttran_putb_ok (
    byte **_buffer,
    const dbyte size)                   /*  Amount of transmitted data       */
{
    struct_smttran_putb_ok
        *_struct_ptr;

    _struct_ptr = mem_alloc (sizeof (struct_smttran_putb_ok));
    *_buffer = (byte *) _struct_ptr;
    if (_struct_ptr)
      {
        _struct_ptr-> size              = size;

        return sizeof (*_struct_ptr);
      }
    else
        return 0;
}


/*  ---------------------------------------------------------------------[<]-
    Function: get_smttran_putb_ok

    Synopsis: Accepts a buffer containing a putb ok message,
    and unpacks it into a new struct_smttran_putb_ok structure. Free the
    structure using free_smttran_putb_ok() when finished.
    ---------------------------------------------------------------------[>]-*/

int
get_smttran_putb_ok (
    byte *_buffer,
    struct_smttran_putb_ok **params)
{
    struct_smttran_putb_ok
        *_struct_ptr;

    _struct_ptr = (struct_smttran_putb_ok *) _buffer;
    *params = mem_alloc (sizeof (struct_smttran_putb_ok));
    if (*params)
      {
        (* params)-> size               = _struct_ptr-> size;
        return 0;
      }
    else
        return -1;
}


/*  ---------------------------------------------------------------------[<]-
    Function: free_smttran_putb_ok

    Synopsis: frees a structure allocated by get_smttran_putb_ok().
    ---------------------------------------------------------------------[>]-*/

void
free_smttran_putb_ok (
    struct_smttran_putb_ok **params)
{
    mem_free (*params);
    *params = NULL;
}

char *SMTTRAN_PUTB_OK = "SMTTRAN PUTB OK";

/*  ---------------------------------------------------------------------[<]-
    Function: lsend_smttran_putb_ok

    Synopsis: Sends a putb ok -
              event to the smttran agent
    ---------------------------------------------------------------------[>]-*/

int
lsend_smttran_putb_ok (
    const QID  *_to,
    const QID  *_from,
          char *_accept,
          char *_reject,
          char *_expire,
          word _timeout,
    const dbyte size)               /*  Amount of transmitted data       */
{
    byte *_body;
    int   _size,
          _rc;
    _size = put_smttran_putb_ok
                (&_body,
                 size);
    if (_size)
      {
        _rc = event_send (_to, _from, SMTTRAN_PUTB_OK,
                          _body, _size,
                          _accept, _reject, _expire, _timeout);
        mem_free (_body);
        return _rc;
      }
    else
        return -1;
}


/*  ---------------------------------------------------------------------[<]-
    Function: put_smttran_getb_ok

    Synopsis: Formats a getb ok message, allocates a new buffer,
    and returns the formatted message in the buffer.  You should free the
    buffer using mem_free() when finished.  Returns the size of the buffer
    in bytes.
    ---------------------------------------------------------------------[>]-*/

int
put_smttran_getb_ok (
    byte **_buffer,
    const word  size,                   /*  Amount of data received          */
    const void *data)                   /*  Block of data received           */
{
    struct_smttran_getb_ok
        *_struct_ptr;
    int
        _total_size = sizeof (struct_smttran_getb_ok);
    char
        *_ptr;

    _total_size += size;
    _struct_ptr = mem_alloc (_total_size);
    *_buffer = (byte *) _struct_ptr;
    if (_struct_ptr)
      {
        _ptr = (char *) _struct_ptr + sizeof (struct_smttran_getb_ok);
        _struct_ptr-> size              = size;
        _struct_ptr-> data              = (byte *) _ptr;
        memcpy (_ptr, data, size);
        _ptr += size;

        return _total_size;
      }
    else
        return 0;
}


/*  ---------------------------------------------------------------------[<]-
    Function: get_smttran_getb_ok

    Synopsis: Accepts a buffer containing a getb ok message,
    and unpacks it into a new struct_smttran_getb_ok structure. Free the
    structure using free_smttran_getb_ok() when finished.
    ---------------------------------------------------------------------[>]-*/

int
get_smttran_getb_ok (
    byte *_buffer,
    struct_smttran_getb_ok **params)
{
    struct_smttran_getb_ok
        *_struct_ptr;
    char
        *_ptr;

    _struct_ptr = (struct_smttran_getb_ok *) _buffer;
    *params = mem_alloc (sizeof (struct_smttran_getb_ok));
    if (*params)
      {
        _ptr = (char *) _struct_ptr + sizeof (struct_smttran_getb_ok);
        (* params)-> size               = _struct_ptr-> size;
        (* params)-> data               = mem_alloc (_struct_ptr-> size + 1);
        memcpy ((* params)-> data, _ptr, _struct_ptr-> size);
        *((byte *)(* params)-> data + _struct_ptr-> size) = 0;
        _ptr += _struct_ptr-> size;
        return 0;
      }
    else
        return -1;
}


/*  ---------------------------------------------------------------------[<]-
    Function: free_smttran_getb_ok

    Synopsis: frees a structure allocated by get_smttran_getb_ok().
    ---------------------------------------------------------------------[>]-*/

void
free_smttran_getb_ok (
    struct_smttran_getb_ok **params)
{
    mem_free ((*params)-> data);
    mem_free (*params);
    *params = NULL;
}

char *SMTTRAN_GETB_OK = "SMTTRAN GETB OK";

/*  ---------------------------------------------------------------------[<]-
    Function: lsend_smttran_getb_ok

    Synopsis: Sends a getb ok -
              event to the smttran agent
    ---------------------------------------------------------------------[>]-*/

int
lsend_smttran_getb_ok (
    const QID  *_to,
    const QID  *_from,
          char *_accept,
          char *_reject,
          char *_expire,
          word _timeout,
    const word  size,               /*  Amount of data received          */
    const void *data)               /*  Block of data received           */
{
    byte *_body;
    int   _size,
          _rc;
    _size = put_smttran_getb_ok
                (&_body,
                 size,
                 data);
    if (_size)
      {
        _rc = event_send (_to, _from, SMTTRAN_GETB_OK,
                          _body, _size,
                          _accept, _reject, _expire, _timeout);
        mem_free (_body);
        return _rc;
      }
    else
        return -1;
}


/*  ---------------------------------------------------------------------[<]-
    Function: put_smttran_puth_ok

    Synopsis: Formats a puth ok message, allocates a new buffer,
    and returns the formatted message in the buffer.  You should free the
    buffer using mem_free() when finished.  Returns the size of the buffer
    in bytes.
    ---------------------------------------------------------------------[>]-*/

int
put_smttran_puth_ok (
    byte **_buffer,
    const qbyte size)                   /*  Amount of transmitted data       */
{
    struct_smttran_puth_ok
        *_struct_ptr;

    _struct_ptr = mem_alloc (sizeof (struct_smttran_puth_ok));
    *_buffer = (byte *) _struct_ptr;
    if (_struct_ptr)
      {
        _struct_ptr-> size              = size;

        return sizeof (*_struct_ptr);
      }
    else
        return 0;
}


/*  ---------------------------------------------------------------------[<]-
    Function: get_smttran_puth_ok

    Synopsis: Accepts a buffer containing a puth ok message,
    and unpacks it into a new struct_smttran_puth_ok structure. Free the
    structure using free_smttran_puth_ok() when finished.
    ---------------------------------------------------------------------[>]-*/

int
get_smttran_puth_ok (
    byte *_buffer,
    struct_smttran_puth_ok **params)
{
    struct_smttran_puth_ok
        *_struct_ptr;

    _struct_ptr = (struct_smttran_puth_ok *) _buffer;
    *params = mem_alloc (sizeof (struct_smttran_puth_ok));
    if (*params)
      {
        (* params)-> size               = _struct_ptr-> size;
        return 0;
      }
    else
        return -1;
}


/*  ---------------------------------------------------------------------[<]-
    Function: free_smttran_puth_ok

    Synopsis: frees a structure allocated by get_smttran_puth_ok().
    ---------------------------------------------------------------------[>]-*/

void
free_smttran_puth_ok (
    struct_smttran_puth_ok **params)
{
    mem_free (*params);
    *params = NULL;
}

char *SMTTRAN_PUTH_OK = "SMTTRAN PUTH OK";

/*  ---------------------------------------------------------------------[<]-
    Function: lsend_smttran_puth_ok

    Synopsis: Sends a puth ok -
              event to the smttran agent
    ---------------------------------------------------------------------[>]-*/

int
lsend_smttran_puth_ok (
    const QID  *_to,
    const QID  *_from,
          char *_accept,
          char *_reject,
          char *_expire,
          word _timeout,
    const qbyte size)               /*  Amount of transmitted data       */
{
    byte *_body;
    int   _size,
          _rc;
    _size = put_smttran_puth_ok
                (&_body,
                 size);
    if (_size)
      {
        _rc = event_send (_to, _from, SMTTRAN_PUTH_OK,
                          _body, _size,
                          _accept, _reject, _expire, _timeout);
        mem_free (_body);
        return _rc;
      }
    else
        return -1;
}


/*  ---------------------------------------------------------------------[<]-
    Function: put_smttran_geth_ok

    Synopsis: Formats a geth ok message, allocates a new buffer,
    and returns the formatted message in the buffer.  You should free the
    buffer using mem_free() when finished.  Returns the size of the buffer
    in bytes.
    ---------------------------------------------------------------------[>]-*/

int
put_smttran_geth_ok (
    byte **_buffer,
    const qbyte size,                   /*  Amount of data received          */
    const byte *data)                   /*  Block of data received           */
{
    struct_smttran_geth_ok
        *_struct_ptr;
    int
        _total_size = sizeof (struct_smttran_geth_ok);
    char
        *_ptr;

    _total_size += size;
    _struct_ptr = mem_alloc (_total_size);
    *_buffer = (byte *) _struct_ptr;
    if (_struct_ptr)
      {
        _ptr = (char *) _struct_ptr + sizeof (struct_smttran_geth_ok);
        _struct_ptr-> size              = size;
        _struct_ptr-> data              = (byte *) _ptr;
        memcpy (_ptr, data, size);
        _ptr += size;

        return _total_size;
      }
    else
        return 0;
}


/*  ---------------------------------------------------------------------[<]-
    Function: get_smttran_geth_ok

    Synopsis: Accepts a buffer containing a geth ok message,
    and unpacks it into a new struct_smttran_geth_ok structure. Free the
    structure using free_smttran_geth_ok() when finished.
    ---------------------------------------------------------------------[>]-*/

int
get_smttran_geth_ok (
    byte *_buffer,
    struct_smttran_geth_ok **params)
{
    struct_smttran_geth_ok
        *_struct_ptr;
    char
        *_ptr;

    _struct_ptr = (struct_smttran_geth_ok *) _buffer;
    *params = mem_alloc (sizeof (struct_smttran_geth_ok));
    if (*params)
      {
        _ptr = (char *) _struct_ptr + sizeof (struct_smttran_geth_ok);
        (* params)-> size               = _struct_ptr-> size;
        (* params)-> data               = mem_alloc (_struct_ptr-> size + 1);
        memcpy ((* params)-> data, _ptr, _struct_ptr-> size);
        *((byte *)(* params)-> data + _struct_ptr-> size) = 0;
        _ptr += _struct_ptr-> size;
        return 0;
      }
    else
        return -1;
}


/*  ---------------------------------------------------------------------[<]-
    Function: free_smttran_geth_ok

    Synopsis: frees a structure allocated by get_smttran_geth_ok().
    ---------------------------------------------------------------------[>]-*/

void
free_smttran_geth_ok (
    struct_smttran_geth_ok **params)
{
    mem_free ((*params)-> data);
    mem_free (*params);
    *params = NULL;
}

char *SMTTRAN_GETH_OK = "SMTTRAN GETH OK";

/*  ---------------------------------------------------------------------[<]-
    Function: lsend_smttran_geth_ok

    Synopsis: Sends a geth ok -
              event to the smttran agent
    ---------------------------------------------------------------------[>]-*/

int
lsend_smttran_geth_ok (
    const QID  *_to,
    const QID  *_from,
          char *_accept,
          char *_reject,
          char *_expire,
          word _timeout,
    const qbyte size,               /*  Amount of data received          */
    const byte *data)               /*  Block of data received           */
{
    byte *_body;
    int   _size,
          _rc;
    _size = put_smttran_geth_ok
                (&_body,
                 size,
                 data);
    if (_size)
      {
        _rc = event_send (_to, _from, SMTTRAN_GETH_OK,
                          _body, _size,
                          _accept, _reject, _expire, _timeout);
        mem_free (_body);
        return _rc;
      }
    else
        return -1;
}


/*  ---------------------------------------------------------------------[<]-
    Function: put_smttran_putf_ok

    Synopsis: Formats a putf ok message, allocates a new buffer,
    and returns the formatted message in the buffer.  You should free the
    buffer using mem_free() when finished.  Returns the size of the buffer
    in bytes.
    ---------------------------------------------------------------------[>]-*/

int
put_smttran_putf_ok (
    byte **_buffer,
    const qbyte size)                   /*  Amount of transmitted data       */
{
    struct_smttran_putf_ok
        *_struct_ptr;

    _struct_ptr = mem_alloc (sizeof (struct_smttran_putf_ok));
    *_buffer = (byte *) _struct_ptr;
    if (_struct_ptr)
      {
        _struct_ptr-> size              = size;

        return sizeof (*_struct_ptr);
      }
    else
        return 0;
}


/*  ---------------------------------------------------------------------[<]-
    Function: get_smttran_putf_ok

    Synopsis: Accepts a buffer containing a putf ok message,
    and unpacks it into a new struct_smttran_putf_ok structure. Free the
    structure using free_smttran_putf_ok() when finished.
    ---------------------------------------------------------------------[>]-*/

int
get_smttran_putf_ok (
    byte *_buffer,
    struct_smttran_putf_ok **params)
{
    struct_smttran_putf_ok
        *_struct_ptr;

    _struct_ptr = (struct_smttran_putf_ok *) _buffer;
    *params = mem_alloc (sizeof (struct_smttran_putf_ok));
    if (*params)
      {
        (* params)-> size               = _struct_ptr-> size;
        return 0;
      }
    else
        return -1;
}


/*  ---------------------------------------------------------------------[<]-
    Function: free_smttran_putf_ok

    Synopsis: frees a structure allocated by get_smttran_putf_ok().
    ---------------------------------------------------------------------[>]-*/

void
free_smttran_putf_ok (
    struct_smttran_putf_ok **params)
{
    mem_free (*params);
    *params = NULL;
}

char *SMTTRAN_PUTF_OK = "SMTTRAN PUTF OK";

/*  ---------------------------------------------------------------------[<]-
    Function: lsend_smttran_putf_ok

    Synopsis: Sends a putf ok -
              event to the smttran agent
    ---------------------------------------------------------------------[>]-*/

int
lsend_smttran_putf_ok (
    const QID  *_to,
    const QID  *_from,
          char *_accept,
          char *_reject,
          char *_expire,
          word _timeout,
    const qbyte size)               /*  Amount of transmitted data       */
{
    byte *_body;
    int   _size,
          _rc;
    _size = put_smttran_putf_ok
                (&_body,
                 size);
    if (_size)
      {
        _rc = event_send (_to, _from, SMTTRAN_PUTF_OK,
                          _body, _size,
                          _accept, _reject, _expire, _timeout);
        mem_free (_body);
        return _rc;
      }
    else
        return -1;
}


/*  ---------------------------------------------------------------------[<]-
    Function: put_smttran_getf_ok

    Synopsis: Formats a getf ok message, allocates a new buffer,
    and returns the formatted message in the buffer.  You should free the
    buffer using mem_free() when finished.  Returns the size of the buffer
    in bytes.
    ---------------------------------------------------------------------[>]-*/

int
put_smttran_getf_ok (
    byte **_buffer,
    const qbyte size)                   /*  Amount of transmitted data       */
{
    struct_smttran_getf_ok
        *_struct_ptr;

    _struct_ptr = mem_alloc (sizeof (struct_smttran_getf_ok));
    *_buffer = (byte *) _struct_ptr;
    if (_struct_ptr)
      {
        _struct_ptr-> size              = size;

        return sizeof (*_struct_ptr);
      }
    else
        return 0;
}


/*  ---------------------------------------------------------------------[<]-
    Function: get_smttran_getf_ok

    Synopsis: Accepts a buffer containing a getf ok message,
    and unpacks it into a new struct_smttran_getf_ok structure. Free the
    structure using free_smttran_getf_ok() when finished.
    ---------------------------------------------------------------------[>]-*/

int
get_smttran_getf_ok (
    byte *_buffer,
    struct_smttran_getf_ok **params)
{
    struct_smttran_getf_ok
        *_struct_ptr;

    _struct_ptr = (struct_smttran_getf_ok *) _buffer;
    *params = mem_alloc (sizeof (struct_smttran_getf_ok));
    if (*params)
      {
        (* params)-> size               = _struct_ptr-> size;
        return 0;
      }
    else
        return -1;
}


/*  ---------------------------------------------------------------------[<]-
    Function: free_smttran_getf_ok

    Synopsis: frees a structure allocated by get_smttran_getf_ok().
    ---------------------------------------------------------------------[>]-*/

void
free_smttran_getf_ok (
    struct_smttran_getf_ok **params)
{
    mem_free (*params);
    *params = NULL;
}

char *SMTTRAN_GETF_OK = "SMTTRAN GETF OK";

/*  ---------------------------------------------------------------------[<]-
    Function: lsend_smttran_getf_ok

    Synopsis: Sends a getf ok -
              event to the smttran agent
    ---------------------------------------------------------------------[>]-*/

int
lsend_smttran_getf_ok (
    const QID  *_to,
    const QID  *_from,
          char *_accept,
          char *_reject,
          char *_expire,
          word _timeout,
    const qbyte size)               /*  Amount of transmitted data       */
{
    byte *_body;
    int   _size,
          _rc;
    _size = put_smttran_getf_ok
                (&_body,
                 size);
    if (_size)
      {
        _rc = event_send (_to, _from, SMTTRAN_GETF_OK,
                          _body, _size,
                          _accept, _reject, _expire, _timeout);
        mem_free (_body);
        return _rc;
      }
    else
        return -1;
}


/*  ---------------------------------------------------------------------[<]-
    Function: put_smttran_pipe_create

    Synopsis: Formats a pipe create message, allocates a new buffer,
    and returns the formatted message in the buffer.  You should free the
    buffer using mem_free() when finished.  Returns the size of the buffer
    in bytes.
    ---------------------------------------------------------------------[>]-*/

int
put_smttran_pipe_create (
    byte **_buffer,
    const char *name,                   /*  Name of pipe                     */
    const qbyte input_rate,             /*  Input rate, bytes/s              */
    const qbyte output_rate)            /*  Output rate, bytes/s             */
{
    struct_smttran_pipe_create
        *_struct_ptr;
    int
        _total_size = sizeof (struct_smttran_pipe_create);
    char
        *_ptr;

    _total_size += name ? strlen (name) + 1 : 0;
    _struct_ptr = mem_alloc (_total_size);
    *_buffer = (byte *) _struct_ptr;
    if (_struct_ptr)
      {
        _ptr = (char *) _struct_ptr + sizeof (struct_smttran_pipe_create);
        if (name)
          {
            _struct_ptr-> name              = (char *) _ptr;
            strcpy ((char *) _ptr, name);
            _ptr += strlen (name) + 1;
          }
        else
            _struct_ptr-> name              = NULL;
        _struct_ptr-> input_rate        = input_rate;
        _struct_ptr-> output_rate       = output_rate;

        return _total_size;
      }
    else
        return 0;
}


/*  ---------------------------------------------------------------------[<]-
    Function: get_smttran_pipe_create

    Synopsis: Accepts a buffer containing a pipe create message,
    and unpacks it into a new struct_smttran_pipe_create structure. Free the
    structure using free_smttran_pipe_create() when finished.
    ---------------------------------------------------------------------[>]-*/

int
get_smttran_pipe_create (
    byte *_buffer,
    struct_smttran_pipe_create **params)
{
    struct_smttran_pipe_create
        *_struct_ptr;
    char
        *_ptr;

    _struct_ptr = (struct_smttran_pipe_create *) _buffer;
    *params = mem_alloc (sizeof (struct_smttran_pipe_create));
    if (*params)
      {
        _ptr = (char *) _struct_ptr + sizeof (struct_smttran_pipe_create);
        if (_struct_ptr-> name)
          {
            (* params)-> name               = mem_strdup (_ptr);
            _ptr += strlen ((* params)-> name) + 1;
          }
        else
            (* params)-> name               = NULL;
        (* params)-> input_rate         = _struct_ptr-> input_rate;
        (* params)-> output_rate        = _struct_ptr-> output_rate;
        return 0;
      }
    else
        return -1;
}


/*  ---------------------------------------------------------------------[<]-
    Function: free_smttran_pipe_create

    Synopsis: frees a structure allocated by get_smttran_pipe_create().
    ---------------------------------------------------------------------[>]-*/

void
free_smttran_pipe_create (
    struct_smttran_pipe_create **params)
{
    mem_free ((*params)-> name);
    mem_free (*params);
    *params = NULL;
}

char *SMTTRAN_PIPE_CREATE = "SMTTRAN PIPE CREATE";

/*  ---------------------------------------------------------------------[<]-
    Function: lsend_smttran_pipe_create

    Synopsis: Sends a pipe create -
              event to the smttran agent
    ---------------------------------------------------------------------[>]-*/

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
    const qbyte output_rate)        /*  Output rate, bytes/s             */
{
    byte *_body;
    int   _size,
          _rc;
    _size = put_smttran_pipe_create
                (&_body,
                 name,
                 input_rate,
                 output_rate);
    if (_size)
      {
        _rc = event_send (_to, _from, SMTTRAN_PIPE_CREATE,
                          _body, _size,
                          _accept, _reject, _expire, _timeout);
        mem_free (_body);
        return _rc;
      }
    else
        return -1;
}

char *SMTTRAN_CLEAR_PIPES = "SMTTRAN CLEAR PIPES";

char *SMTTRAN_COMMIT = "SMTTRAN COMMIT";

char *SMTTRAN_CLOSED = "SMTTRAN CLOSED";


/*  ---------------------------------------------------------------------[<]-
    Function: put_smttran_error

    Synopsis: Formats a error message, allocates a new buffer,
    and returns the formatted message in the buffer.  You should free the
    buffer using mem_free() when finished.  Returns the size of the buffer
    in bytes.
    ---------------------------------------------------------------------[>]-*/

int
put_smttran_error (
    byte **_buffer,
    const char *reason)                 /*  Error message                    */
{
    struct_smttran_error
        *_struct_ptr;
    int
        _total_size = sizeof (struct_smttran_error);
    char
        *_ptr;

    _total_size += reason ? strlen (reason) + 1 : 0;
    _struct_ptr = mem_alloc (_total_size);
    *_buffer = (byte *) _struct_ptr;
    if (_struct_ptr)
      {
        _ptr = (char *) _struct_ptr + sizeof (struct_smttran_error);
        if (reason)
          {
            _struct_ptr-> reason            = (char *) _ptr;
            strcpy ((char *) _ptr, reason);
            _ptr += strlen (reason) + 1;
          }
        else
            _struct_ptr-> reason            = NULL;

        return _total_size;
      }
    else
        return 0;
}


/*  ---------------------------------------------------------------------[<]-
    Function: get_smttran_error

    Synopsis: Accepts a buffer containing a error message,
    and unpacks it into a new struct_smttran_error structure. Free the
    structure using free_smttran_error() when finished.
    ---------------------------------------------------------------------[>]-*/

int
get_smttran_error (
    byte *_buffer,
    struct_smttran_error **params)
{
    struct_smttran_error
        *_struct_ptr;
    char
        *_ptr;

    _struct_ptr = (struct_smttran_error *) _buffer;
    *params = mem_alloc (sizeof (struct_smttran_error));
    if (*params)
      {
        _ptr = (char *) _struct_ptr + sizeof (struct_smttran_error);
        if (_struct_ptr-> reason)
          {
            (* params)-> reason             = mem_strdup (_ptr);
            _ptr += strlen ((* params)-> reason) + 1;
          }
        else
            (* params)-> reason             = NULL;
        return 0;
      }
    else
        return -1;
}


/*  ---------------------------------------------------------------------[<]-
    Function: free_smttran_error

    Synopsis: frees a structure allocated by get_smttran_error().
    ---------------------------------------------------------------------[>]-*/

void
free_smttran_error (
    struct_smttran_error **params)
{
    mem_free ((*params)-> reason);
    mem_free (*params);
    *params = NULL;
}

char *SMTTRAN_ERROR = "SMTTRAN ERROR";

/*  ---------------------------------------------------------------------[<]-
    Function: lsend_smttran_error

    Synopsis: Sends a error -
              event to the smttran agent
    ---------------------------------------------------------------------[>]-*/

int
lsend_smttran_error (
    const QID  *_to,
    const QID  *_from,
          char *_accept,
          char *_reject,
          char *_expire,
          word _timeout,
    const char *reason)             /*  Error message                    */
{
    byte *_body;
    int   _size,
          _rc;
    _size = put_smttran_error
                (&_body,
                 reason);
    if (_size)
      {
        _rc = event_send (_to, _from, SMTTRAN_ERROR,
                          _body, _size,
                          _accept, _reject, _expire, _timeout);
        mem_free (_body);
        return _rc;
      }
    else
        return -1;
}

/*---------------------------------------------------------------------------
 *  Message functions for smtupm - Unattended Process Monitor agent.
 *---------------------------------------------------------------------------*/


/*  ---------------------------------------------------------------------[<]-
    Function: put_smtupm_message

    Synopsis: Formats a message message, allocates a new buffer,
    and returns the formatted message in the buffer.  You should free the
    buffer using mem_free() when finished.  Returns the size of the buffer
    in bytes.
    ---------------------------------------------------------------------[>]-*/

int
put_smtupm_message (
    byte **_buffer,
    const dbyte ident,                  /*                                   */
    const char *string)                 /*                                   */
{
    struct_smtupm_message
        *_struct_ptr;
    int
        _total_size = sizeof (struct_smtupm_message);
    char
        *_ptr;

    _total_size += string ? strlen (string) + 1 : 0;
    _struct_ptr = mem_alloc (_total_size);
    *_buffer = (byte *) _struct_ptr;
    if (_struct_ptr)
      {
        _ptr = (char *) _struct_ptr + sizeof (struct_smtupm_message);
        _struct_ptr-> ident             = ident;
        if (string)
          {
            _struct_ptr-> string            = (char *) _ptr;
            strcpy ((char *) _ptr, string);
            _ptr += strlen (string) + 1;
          }
        else
            _struct_ptr-> string            = NULL;

        return _total_size;
      }
    else
        return 0;
}


/*  ---------------------------------------------------------------------[<]-
    Function: get_smtupm_message

    Synopsis: Accepts a buffer containing a message message,
    and unpacks it into a new struct_smtupm_message structure. Free the
    structure using free_smtupm_message() when finished.
    ---------------------------------------------------------------------[>]-*/

int
get_smtupm_message (
    byte *_buffer,
    struct_smtupm_message **params)
{
    struct_smtupm_message
        *_struct_ptr;
    char
        *_ptr;

    _struct_ptr = (struct_smtupm_message *) _buffer;
    *params = mem_alloc (sizeof (struct_smtupm_message));
    if (*params)
      {
        _ptr = (char *) _struct_ptr + sizeof (struct_smtupm_message);
        (* params)-> ident              = _struct_ptr-> ident;
        if (_struct_ptr-> string)
          {
            (* params)-> string             = mem_strdup (_ptr);
            _ptr += strlen ((* params)-> string) + 1;
          }
        else
            (* params)-> string             = NULL;
        return 0;
      }
    else
        return -1;
}


/*  ---------------------------------------------------------------------[<]-
    Function: free_smtupm_message

    Synopsis: frees a structure allocated by get_smtupm_message().
    ---------------------------------------------------------------------[>]-*/

void
free_smtupm_message (
    struct_smtupm_message **params)
{
    mem_free ((*params)-> string);
    mem_free (*params);
    *params = NULL;
}

char *SMTUPM_PUT_BLOCK = "SMTUPM PUT BLOCK";

/*  ---------------------------------------------------------------------[<]-
    Function: lsend_smtupm_put_block

    Synopsis: Sends a put block -
              event to the smtupm agent
    ---------------------------------------------------------------------[>]-*/

int
lsend_smtupm_put_block (
    const QID  *_to,
    const QID  *_from,
          char *_accept,
          char *_reject,
          char *_expire,
          word _timeout,
    const dbyte ident,              /*                                   */
    const char *string)             /*                                   */
{
    byte *_body;
    int   _size,
          _rc;
    _size = put_smtupm_message
                (&_body,
                 ident,
                 string);
    if (_size)
      {
        _rc = event_send (_to, _from, SMTUPM_PUT_BLOCK,
                          _body, _size,
                          _accept, _reject, _expire, _timeout);
        mem_free (_body);
        return _rc;
      }
    else
        return -1;
}

/*---------------------------------------------------------------------------
 *  Message functions for smtpop - POP3 agent.
 *---------------------------------------------------------------------------*/


/*  ---------------------------------------------------------------------[<]-
    Function: put_smtpop_connection

    Synopsis: Formats a connection message, allocates a new buffer,
    and returns the formatted message in the buffer.  You should free the
    buffer using mem_free() when finished.  Returns the size of the buffer
    in bytes.
    ---------------------------------------------------------------------[>]-*/

int
put_smtpop_connection (
    byte **_buffer,
    const char *server,                 /*  pop3 server name                 */
    const char *user,                   /*  user name                        */
    const char *password)               /*  user password                    */
{
    struct_smtpop_connection
        *_struct_ptr;
    int
        _total_size = sizeof (struct_smtpop_connection);
    char
        *_ptr;

    _total_size += server ? strlen (server) + 1 : 0;
    _total_size += user ? strlen (user) + 1 : 0;
    _total_size += password ? strlen (password) + 1 : 0;
    _struct_ptr = mem_alloc (_total_size);
    *_buffer = (byte *) _struct_ptr;
    if (_struct_ptr)
      {
        _ptr = (char *) _struct_ptr + sizeof (struct_smtpop_connection);
        if (server)
          {
            _struct_ptr-> server            = (char *) _ptr;
            strcpy ((char *) _ptr, server);
            _ptr += strlen (server) + 1;
          }
        else
            _struct_ptr-> server            = NULL;
        if (user)
          {
            _struct_ptr-> user              = (char *) _ptr;
            strcpy ((char *) _ptr, user);
            _ptr += strlen (user) + 1;
          }
        else
            _struct_ptr-> user              = NULL;
        if (password)
          {
            _struct_ptr-> password          = (char *) _ptr;
            strcpy ((char *) _ptr, password);
            _ptr += strlen (password) + 1;
          }
        else
            _struct_ptr-> password          = NULL;

        return _total_size;
      }
    else
        return 0;
}


/*  ---------------------------------------------------------------------[<]-
    Function: get_smtpop_connection

    Synopsis: Accepts a buffer containing a connection message,
    and unpacks it into a new struct_smtpop_connection structure. Free the
    structure using free_smtpop_connection() when finished.
    ---------------------------------------------------------------------[>]-*/

int
get_smtpop_connection (
    byte *_buffer,
    struct_smtpop_connection **params)
{
    struct_smtpop_connection
        *_struct_ptr;
    char
        *_ptr;

    _struct_ptr = (struct_smtpop_connection *) _buffer;
    *params = mem_alloc (sizeof (struct_smtpop_connection));
    if (*params)
      {
        _ptr = (char *) _struct_ptr + sizeof (struct_smtpop_connection);
        if (_struct_ptr-> server)
          {
            (* params)-> server             = mem_strdup (_ptr);
            _ptr += strlen ((* params)-> server) + 1;
          }
        else
            (* params)-> server             = NULL;
        if (_struct_ptr-> user)
          {
            (* params)-> user               = mem_strdup (_ptr);
            _ptr += strlen ((* params)-> user) + 1;
          }
        else
            (* params)-> user               = NULL;
        if (_struct_ptr-> password)
          {
            (* params)-> password           = mem_strdup (_ptr);
            _ptr += strlen ((* params)-> password) + 1;
          }
        else
            (* params)-> password           = NULL;
        return 0;
      }
    else
        return -1;
}


/*  ---------------------------------------------------------------------[<]-
    Function: free_smtpop_connection

    Synopsis: frees a structure allocated by get_smtpop_connection().
    ---------------------------------------------------------------------[>]-*/

void
free_smtpop_connection (
    struct_smtpop_connection **params)
{
    mem_free ((*params)-> server);
    mem_free ((*params)-> user);
    mem_free ((*params)-> password);
    mem_free (*params);
    *params = NULL;
}

char *SMTPOP_CONNECT = "SMTPOP CONNECT";

/*  ---------------------------------------------------------------------[<]-
    Function: lsend_smtpop_connect

    Synopsis: Sends a connect -
              event to the smtpop agent
    ---------------------------------------------------------------------[>]-*/

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
    const char *password)           /*  user password                    */
{
    byte *_body;
    int   _size,
          _rc;
    _size = put_smtpop_connection
                (&_body,
                 server,
                 user,
                 password);
    if (_size)
      {
        _rc = event_send (_to, _from, SMTPOP_CONNECT,
                          _body, _size,
                          _accept, _reject, _expire, _timeout);
        mem_free (_body);
        return _rc;
      }
    else
        return -1;
}


/*  ---------------------------------------------------------------------[<]-
    Function: put_smtpop_connect_ok

    Synopsis: Formats a connect_ok message, allocates a new buffer,
    and returns the formatted message in the buffer.  You should free the
    buffer using mem_free() when finished.  Returns the size of the buffer
    in bytes.
    ---------------------------------------------------------------------[>]-*/

int
put_smtpop_connect_ok (
    byte **_buffer,
    const qbyte msg_cnt,                /*  count of new messages on server  */
    const qbyte msg_size)               /*  messages total size (bytes)      */
{
    struct_smtpop_connect_ok
        *_struct_ptr;

    _struct_ptr = mem_alloc (sizeof (struct_smtpop_connect_ok));
    *_buffer = (byte *) _struct_ptr;
    if (_struct_ptr)
      {
        _struct_ptr-> msg_cnt           = msg_cnt;
        _struct_ptr-> msg_size          = msg_size;

        return sizeof (*_struct_ptr);
      }
    else
        return 0;
}


/*  ---------------------------------------------------------------------[<]-
    Function: get_smtpop_connect_ok

    Synopsis: Accepts a buffer containing a connect_ok message,
    and unpacks it into a new struct_smtpop_connect_ok structure. Free the
    structure using free_smtpop_connect_ok() when finished.
    ---------------------------------------------------------------------[>]-*/

int
get_smtpop_connect_ok (
    byte *_buffer,
    struct_smtpop_connect_ok **params)
{
    struct_smtpop_connect_ok
        *_struct_ptr;

    _struct_ptr = (struct_smtpop_connect_ok *) _buffer;
    *params = mem_alloc (sizeof (struct_smtpop_connect_ok));
    if (*params)
      {
        (* params)-> msg_cnt            = _struct_ptr-> msg_cnt;
        (* params)-> msg_size           = _struct_ptr-> msg_size;
        return 0;
      }
    else
        return -1;
}


/*  ---------------------------------------------------------------------[<]-
    Function: free_smtpop_connect_ok

    Synopsis: frees a structure allocated by get_smtpop_connect_ok().
    ---------------------------------------------------------------------[>]-*/

void
free_smtpop_connect_ok (
    struct_smtpop_connect_ok **params)
{
    mem_free (*params);
    *params = NULL;
}

char *SMTPOP_CONNECT_OK = "SMTPOP CONNECT_OK";

/*  ---------------------------------------------------------------------[<]-
    Function: lsend_smtpop_connect_ok

    Synopsis: Sends a connect_ok -
              event to the smtpop agent
    ---------------------------------------------------------------------[>]-*/

int
lsend_smtpop_connect_ok (
    const QID  *_to,
    const QID  *_from,
          char *_accept,
          char *_reject,
          char *_expire,
          word _timeout,
    const qbyte msg_cnt,            /*  count of new messages on server  */
    const qbyte msg_size)           /*  messages total size (bytes)      */
{
    byte *_body;
    int   _size,
          _rc;
    _size = put_smtpop_connect_ok
                (&_body,
                 msg_cnt,
                 msg_size);
    if (_size)
      {
        _rc = event_send (_to, _from, SMTPOP_CONNECT_OK,
                          _body, _size,
                          _accept, _reject, _expire, _timeout);
        mem_free (_body);
        return _rc;
      }
    else
        return -1;
}


/*  ---------------------------------------------------------------------[<]-
    Function: put_smtpop_error

    Synopsis: Formats a error message, allocates a new buffer,
    and returns the formatted message in the buffer.  You should free the
    buffer using mem_free() when finished.  Returns the size of the buffer
    in bytes.
    ---------------------------------------------------------------------[>]-*/

int
put_smtpop_error (
    byte **_buffer,
    const char *reason,                 /*  why connection failed            */
    const dbyte code)                   /*                                   */
{
    struct_smtpop_error
        *_struct_ptr;
    int
        _total_size = sizeof (struct_smtpop_error);
    char
        *_ptr;

    _total_size += reason ? strlen (reason) + 1 : 0;
    _struct_ptr = mem_alloc (_total_size);
    *_buffer = (byte *) _struct_ptr;
    if (_struct_ptr)
      {
        _ptr = (char *) _struct_ptr + sizeof (struct_smtpop_error);
        if (reason)
          {
            _struct_ptr-> reason            = (char *) _ptr;
            strcpy ((char *) _ptr, reason);
            _ptr += strlen (reason) + 1;
          }
        else
            _struct_ptr-> reason            = NULL;
        _struct_ptr-> code              = code;

        return _total_size;
      }
    else
        return 0;
}


/*  ---------------------------------------------------------------------[<]-
    Function: get_smtpop_error

    Synopsis: Accepts a buffer containing a error message,
    and unpacks it into a new struct_smtpop_error structure. Free the
    structure using free_smtpop_error() when finished.
    ---------------------------------------------------------------------[>]-*/

int
get_smtpop_error (
    byte *_buffer,
    struct_smtpop_error **params)
{
    struct_smtpop_error
        *_struct_ptr;
    char
        *_ptr;

    _struct_ptr = (struct_smtpop_error *) _buffer;
    *params = mem_alloc (sizeof (struct_smtpop_error));
    if (*params)
      {
        _ptr = (char *) _struct_ptr + sizeof (struct_smtpop_error);
        if (_struct_ptr-> reason)
          {
            (* params)-> reason             = mem_strdup (_ptr);
            _ptr += strlen ((* params)-> reason) + 1;
          }
        else
            (* params)-> reason             = NULL;
        (* params)-> code               = _struct_ptr-> code;
        return 0;
      }
    else
        return -1;
}


/*  ---------------------------------------------------------------------[<]-
    Function: free_smtpop_error

    Synopsis: frees a structure allocated by get_smtpop_error().
    ---------------------------------------------------------------------[>]-*/

void
free_smtpop_error (
    struct_smtpop_error **params)
{
    mem_free ((*params)-> reason);
    mem_free (*params);
    *params = NULL;
}

char *SMTPOP_ERROR = "SMTPOP ERROR";

/*  ---------------------------------------------------------------------[<]-
    Function: lsend_smtpop_error

    Synopsis: Sends a error -
              event to the smtpop agent
    ---------------------------------------------------------------------[>]-*/

int
lsend_smtpop_error (
    const QID  *_to,
    const QID  *_from,
          char *_accept,
          char *_reject,
          char *_expire,
          word _timeout,
    const char *reason,             /*  why connection failed            */
    const dbyte code)               /*                                   */
{
    byte *_body;
    int   _size,
          _rc;
    _size = put_smtpop_error
                (&_body,
                 reason,
                 code);
    if (_size)
      {
        _rc = event_send (_to, _from, SMTPOP_ERROR,
                          _body, _size,
                          _accept, _reject, _expire, _timeout);
        mem_free (_body);
        return _rc;
      }
    else
        return -1;
}

char *SMTPOP_GET_SESSION_INFO = "SMTPOP GET_SESSION_INFO";

char *SMTPOP_QUIT = "SMTPOP QUIT";

char *SMTPOP_QUIT_OK = "SMTPOP QUIT_OK";


/*  ---------------------------------------------------------------------[<]-
    Function: put_smtpop_session_info

    Synopsis: Formats a session_info message, allocates a new buffer,
    and returns the formatted message in the buffer.  You should free the
    buffer using mem_free() when finished.  Returns the size of the buffer
    in bytes.
    ---------------------------------------------------------------------[>]-*/

int
put_smtpop_session_info (
    byte **_buffer,
    const qbyte count,                  /*  message count                    */
    const qbyte size)                   /*  messages total size in bytes     */
{
    struct_smtpop_session_info
        *_struct_ptr;

    _struct_ptr = mem_alloc (sizeof (struct_smtpop_session_info));
    *_buffer = (byte *) _struct_ptr;
    if (_struct_ptr)
      {
        _struct_ptr-> count             = count;
        _struct_ptr-> size              = size;

        return sizeof (*_struct_ptr);
      }
    else
        return 0;
}


/*  ---------------------------------------------------------------------[<]-
    Function: get_smtpop_session_info

    Synopsis: Accepts a buffer containing a session_info message,
    and unpacks it into a new struct_smtpop_session_info structure. Free the
    structure using free_smtpop_session_info() when finished.
    ---------------------------------------------------------------------[>]-*/

int
get_smtpop_session_info (
    byte *_buffer,
    struct_smtpop_session_info **params)
{
    struct_smtpop_session_info
        *_struct_ptr;

    _struct_ptr = (struct_smtpop_session_info *) _buffer;
    *params = mem_alloc (sizeof (struct_smtpop_session_info));
    if (*params)
      {
        (* params)-> count              = _struct_ptr-> count;
        (* params)-> size               = _struct_ptr-> size;
        return 0;
      }
    else
        return -1;
}


/*  ---------------------------------------------------------------------[<]-
    Function: free_smtpop_session_info

    Synopsis: frees a structure allocated by get_smtpop_session_info().
    ---------------------------------------------------------------------[>]-*/

void
free_smtpop_session_info (
    struct_smtpop_session_info **params)
{
    mem_free (*params);
    *params = NULL;
}

char *SMTPOP_SESSION_INFO = "SMTPOP SESSION_INFO";

/*  ---------------------------------------------------------------------[<]-
    Function: lsend_smtpop_session_info

    Synopsis: Sends a session_info -
              event to the smtpop agent
    ---------------------------------------------------------------------[>]-*/

int
lsend_smtpop_session_info (
    const QID  *_to,
    const QID  *_from,
          char *_accept,
          char *_reject,
          char *_expire,
          word _timeout,
    const qbyte count,              /*  message count                    */
    const qbyte size)               /*  messages total size in bytes     */
{
    byte *_body;
    int   _size,
          _rc;
    _size = put_smtpop_session_info
                (&_body,
                 count,
                 size);
    if (_size)
      {
        _rc = event_send (_to, _from, SMTPOP_SESSION_INFO,
                          _body, _size,
                          _accept, _reject, _expire, _timeout);
        mem_free (_body);
        return _rc;
      }
    else
        return -1;
}


/*  ---------------------------------------------------------------------[<]-
    Function: put_smtpop_msg_session_info

    Synopsis: Formats a msg_session_info message, allocates a new buffer,
    and returns the formatted message in the buffer.  You should free the
    buffer using mem_free() when finished.  Returns the size of the buffer
    in bytes.
    ---------------------------------------------------------------------[>]-*/

int
put_smtpop_msg_session_info (
    byte **_buffer,
    const qbyte msg_id,                 /*  requested message id             */
    const qbyte size)                   /*  messages total size in bytes     */
{
    struct_smtpop_msg_session_info
        *_struct_ptr;

    _struct_ptr = mem_alloc (sizeof (struct_smtpop_msg_session_info));
    *_buffer = (byte *) _struct_ptr;
    if (_struct_ptr)
      {
        _struct_ptr-> msg_id            = msg_id;
        _struct_ptr-> size              = size;

        return sizeof (*_struct_ptr);
      }
    else
        return 0;
}


/*  ---------------------------------------------------------------------[<]-
    Function: get_smtpop_msg_session_info

    Synopsis: Accepts a buffer containing a msg_session_info message,
    and unpacks it into a new struct_smtpop_msg_session_info structure. Free the
    structure using free_smtpop_msg_session_info() when finished.
    ---------------------------------------------------------------------[>]-*/

int
get_smtpop_msg_session_info (
    byte *_buffer,
    struct_smtpop_msg_session_info **params)
{
    struct_smtpop_msg_session_info
        *_struct_ptr;

    _struct_ptr = (struct_smtpop_msg_session_info *) _buffer;
    *params = mem_alloc (sizeof (struct_smtpop_msg_session_info));
    if (*params)
      {
        (* params)-> msg_id             = _struct_ptr-> msg_id;
        (* params)-> size               = _struct_ptr-> size;
        return 0;
      }
    else
        return -1;
}


/*  ---------------------------------------------------------------------[<]-
    Function: free_smtpop_msg_session_info

    Synopsis: frees a structure allocated by get_smtpop_msg_session_info().
    ---------------------------------------------------------------------[>]-*/

void
free_smtpop_msg_session_info (
    struct_smtpop_msg_session_info **params)
{
    mem_free (*params);
    *params = NULL;
}

char *SMTPOP_MSG_SESSION_INFO = "SMTPOP MSG_SESSION_INFO";

/*  ---------------------------------------------------------------------[<]-
    Function: lsend_smtpop_msg_session_info

    Synopsis: Sends a msg_session_info -
              event to the smtpop agent
    ---------------------------------------------------------------------[>]-*/

int
lsend_smtpop_msg_session_info (
    const QID  *_to,
    const QID  *_from,
          char *_accept,
          char *_reject,
          char *_expire,
          word _timeout,
    const qbyte msg_id,             /*  requested message id             */
    const qbyte size)               /*  messages total size in bytes     */
{
    byte *_body;
    int   _size,
          _rc;
    _size = put_smtpop_msg_session_info
                (&_body,
                 msg_id,
                 size);
    if (_size)
      {
        _rc = event_send (_to, _from, SMTPOP_MSG_SESSION_INFO,
                          _body, _size,
                          _accept, _reject, _expire, _timeout);
        mem_free (_body);
        return _rc;
      }
    else
        return -1;
}


/*  ---------------------------------------------------------------------[<]-
    Function: put_smtpop_msg_id

    Synopsis: Formats a msg_id message, allocates a new buffer,
    and returns the formatted message in the buffer.  You should free the
    buffer using mem_free() when finished.  Returns the size of the buffer
    in bytes.
    ---------------------------------------------------------------------[>]-*/

int
put_smtpop_msg_id (
    byte **_buffer,
    const qbyte msg_id,                 /*  message id, zero=all             */
    const char *attach_dir)             /*  directory where attchment will be stored  */
{
    struct_smtpop_msg_id
        *_struct_ptr;
    int
        _total_size = sizeof (struct_smtpop_msg_id);
    char
        *_ptr;

    _total_size += attach_dir ? strlen (attach_dir) + 1 : 0;
    _struct_ptr = mem_alloc (_total_size);
    *_buffer = (byte *) _struct_ptr;
    if (_struct_ptr)
      {
        _ptr = (char *) _struct_ptr + sizeof (struct_smtpop_msg_id);
        _struct_ptr-> msg_id            = msg_id;
        if (attach_dir)
          {
            _struct_ptr-> attach_dir        = (char *) _ptr;
            strcpy ((char *) _ptr, attach_dir);
            _ptr += strlen (attach_dir) + 1;
          }
        else
            _struct_ptr-> attach_dir        = NULL;

        return _total_size;
      }
    else
        return 0;
}


/*  ---------------------------------------------------------------------[<]-
    Function: get_smtpop_msg_id

    Synopsis: Accepts a buffer containing a msg_id message,
    and unpacks it into a new struct_smtpop_msg_id structure. Free the
    structure using free_smtpop_msg_id() when finished.
    ---------------------------------------------------------------------[>]-*/

int
get_smtpop_msg_id (
    byte *_buffer,
    struct_smtpop_msg_id **params)
{
    struct_smtpop_msg_id
        *_struct_ptr;
    char
        *_ptr;

    _struct_ptr = (struct_smtpop_msg_id *) _buffer;
    *params = mem_alloc (sizeof (struct_smtpop_msg_id));
    if (*params)
      {
        _ptr = (char *) _struct_ptr + sizeof (struct_smtpop_msg_id);
        (* params)-> msg_id             = _struct_ptr-> msg_id;
        if (_struct_ptr-> attach_dir)
          {
            (* params)-> attach_dir         = mem_strdup (_ptr);
            _ptr += strlen ((* params)-> attach_dir) + 1;
          }
        else
            (* params)-> attach_dir         = NULL;
        return 0;
      }
    else
        return -1;
}


/*  ---------------------------------------------------------------------[<]-
    Function: free_smtpop_msg_id

    Synopsis: frees a structure allocated by get_smtpop_msg_id().
    ---------------------------------------------------------------------[>]-*/

void
free_smtpop_msg_id (
    struct_smtpop_msg_id **params)
{
    mem_free ((*params)-> attach_dir);
    mem_free (*params);
    *params = NULL;
}

char *SMTPOP_GET_MSG_HEADER = "SMTPOP GET_MSG_HEADER";

/*  ---------------------------------------------------------------------[<]-
    Function: lsend_smtpop_get_msg_header

    Synopsis: Sends a get_msg_header - ask for message header
              event to the smtpop agent
    ---------------------------------------------------------------------[>]-*/

int
lsend_smtpop_get_msg_header (
    const QID  *_to,
    const QID  *_from,
          char *_accept,
          char *_reject,
          char *_expire,
          word _timeout,
    const qbyte msg_id,             /*  message id, zero=all             */
    const char *attach_dir)         /*  directory where attchment will be stored  */
{
    byte *_body;
    int   _size,
          _rc;
    _size = put_smtpop_msg_id
                (&_body,
                 msg_id,
                 attach_dir);
    if (_size)
      {
        _rc = event_send (_to, _from, SMTPOP_GET_MSG_HEADER,
                          _body, _size,
                          _accept, _reject, _expire, _timeout);
        mem_free (_body);
        return _rc;
      }
    else
        return -1;
}

char *SMTPOP_GET_MSG = "SMTPOP GET_MSG";

/*  ---------------------------------------------------------------------[<]-
    Function: lsend_smtpop_get_msg

    Synopsis: Sends a get_msg - ask for entire message
              event to the smtpop agent
    ---------------------------------------------------------------------[>]-*/

int
lsend_smtpop_get_msg (
    const QID  *_to,
    const QID  *_from,
          char *_accept,
          char *_reject,
          char *_expire,
          word _timeout,
    const qbyte msg_id,             /*  message id, zero=all             */
    const char *attach_dir)         /*  directory where attchment will be stored  */
{
    byte *_body;
    int   _size,
          _rc;
    _size = put_smtpop_msg_id
                (&_body,
                 msg_id,
                 attach_dir);
    if (_size)
      {
        _rc = event_send (_to, _from, SMTPOP_GET_MSG,
                          _body, _size,
                          _accept, _reject, _expire, _timeout);
        mem_free (_body);
        return _rc;
      }
    else
        return -1;
}

char *SMTPOP_DELETE_MSG = "SMTPOP DELETE_MSG";

/*  ---------------------------------------------------------------------[<]-
    Function: lsend_smtpop_delete_msg

    Synopsis: Sends a delete_msg - delete message from server
              event to the smtpop agent
    ---------------------------------------------------------------------[>]-*/

int
lsend_smtpop_delete_msg (
    const QID  *_to,
    const QID  *_from,
          char *_accept,
          char *_reject,
          char *_expire,
          word _timeout,
    const qbyte msg_id,             /*  message id, zero=all             */
    const char *attach_dir)         /*  directory where attchment will be stored  */
{
    byte *_body;
    int   _size,
          _rc;
    _size = put_smtpop_msg_id
                (&_body,
                 msg_id,
                 attach_dir);
    if (_size)
      {
        _rc = event_send (_to, _from, SMTPOP_DELETE_MSG,
                          _body, _size,
                          _accept, _reject, _expire, _timeout);
        mem_free (_body);
        return _rc;
      }
    else
        return -1;
}

char *SMTPOP_DELETE_OK = "SMTPOP DELETE_OK";

/*  ---------------------------------------------------------------------[<]-
    Function: lsend_smtpop_delete_ok

    Synopsis: Sends a delete_ok - delete ok response
              event to the smtpop agent
    ---------------------------------------------------------------------[>]-*/

int
lsend_smtpop_delete_ok (
    const QID  *_to,
    const QID  *_from,
          char *_accept,
          char *_reject,
          char *_expire,
          word _timeout,
    const qbyte msg_id,             /*  message id, zero=all             */
    const char *attach_dir)         /*  directory where attchment will be stored  */
{
    byte *_body;
    int   _size,
          _rc;
    _size = put_smtpop_msg_id
                (&_body,
                 msg_id,
                 attach_dir);
    if (_size)
      {
        _rc = event_send (_to, _from, SMTPOP_DELETE_OK,
                          _body, _size,
                          _accept, _reject, _expire, _timeout);
        mem_free (_body);
        return _rc;
      }
    else
        return -1;
}

char *SMTPOP_GET_MSG_INFO = "SMTPOP GET_MSG_INFO";

/*  ---------------------------------------------------------------------[<]-
    Function: lsend_smtpop_get_msg_info

    Synopsis: Sends a get_msg_info - ask for message info
              event to the smtpop agent
    ---------------------------------------------------------------------[>]-*/

int
lsend_smtpop_get_msg_info (
    const QID  *_to,
    const QID  *_from,
          char *_accept,
          char *_reject,
          char *_expire,
          word _timeout,
    const qbyte msg_id,             /*  message id, zero=all             */
    const char *attach_dir)         /*  directory where attchment will be stored  */
{
    byte *_body;
    int   _size,
          _rc;
    _size = put_smtpop_msg_id
                (&_body,
                 msg_id,
                 attach_dir);
    if (_size)
      {
        _rc = event_send (_to, _from, SMTPOP_GET_MSG_INFO,
                          _body, _size,
                          _accept, _reject, _expire, _timeout);
        mem_free (_body);
        return _rc;
      }
    else
        return -1;
}


/*  ---------------------------------------------------------------------[<]-
    Function: put_smtpop_msg_header

    Synopsis: Formats a msg_header message, allocates a new buffer,
    and returns the formatted message in the buffer.  You should free the
    buffer using mem_free() when finished.  Returns the size of the buffer
    in bytes.
    ---------------------------------------------------------------------[>]-*/

int
put_smtpop_msg_header (
    byte **_buffer,
    const qbyte msg_id,                 /*  message id requested             */
    const char *from,                   /*  message sender                   */
    const char *to,                     /*  people the message was addressed to  */
    const char *cc,                     /*  people the message is carbon copied to  */
    const char *date,                   /*  date the message was received    */
    const char *subject)                /*  guess what                       */
{
    struct_smtpop_msg_header
        *_struct_ptr;
    int
        _total_size = sizeof (struct_smtpop_msg_header);
    char
        *_ptr;

    _total_size += from ? strlen (from) + 1 : 0;
    _total_size += to ? strlen (to) + 1 : 0;
    _total_size += cc ? strlen (cc) + 1 : 0;
    _total_size += date ? strlen (date) + 1 : 0;
    _total_size += subject ? strlen (subject) + 1 : 0;
    _struct_ptr = mem_alloc (_total_size);
    *_buffer = (byte *) _struct_ptr;
    if (_struct_ptr)
      {
        _ptr = (char *) _struct_ptr + sizeof (struct_smtpop_msg_header);
        _struct_ptr-> msg_id            = msg_id;
        if (from)
          {
            _struct_ptr-> from              = (char *) _ptr;
            strcpy ((char *) _ptr, from);
            _ptr += strlen (from) + 1;
          }
        else
            _struct_ptr-> from              = NULL;
        if (to)
          {
            _struct_ptr-> to                = (char *) _ptr;
            strcpy ((char *) _ptr, to);
            _ptr += strlen (to) + 1;
          }
        else
            _struct_ptr-> to                = NULL;
        if (cc)
          {
            _struct_ptr-> cc                = (char *) _ptr;
            strcpy ((char *) _ptr, cc);
            _ptr += strlen (cc) + 1;
          }
        else
            _struct_ptr-> cc                = NULL;
        if (date)
          {
            _struct_ptr-> date              = (char *) _ptr;
            strcpy ((char *) _ptr, date);
            _ptr += strlen (date) + 1;
          }
        else
            _struct_ptr-> date              = NULL;
        if (subject)
          {
            _struct_ptr-> subject           = (char *) _ptr;
            strcpy ((char *) _ptr, subject);
            _ptr += strlen (subject) + 1;
          }
        else
            _struct_ptr-> subject           = NULL;

        return _total_size;
      }
    else
        return 0;
}


/*  ---------------------------------------------------------------------[<]-
    Function: get_smtpop_msg_header

    Synopsis: Accepts a buffer containing a msg_header message,
    and unpacks it into a new struct_smtpop_msg_header structure. Free the
    structure using free_smtpop_msg_header() when finished.
    ---------------------------------------------------------------------[>]-*/

int
get_smtpop_msg_header (
    byte *_buffer,
    struct_smtpop_msg_header **params)
{
    struct_smtpop_msg_header
        *_struct_ptr;
    char
        *_ptr;

    _struct_ptr = (struct_smtpop_msg_header *) _buffer;
    *params = mem_alloc (sizeof (struct_smtpop_msg_header));
    if (*params)
      {
        _ptr = (char *) _struct_ptr + sizeof (struct_smtpop_msg_header);
        (* params)-> msg_id             = _struct_ptr-> msg_id;
        if (_struct_ptr-> from)
          {
            (* params)-> from               = mem_strdup (_ptr);
            _ptr += strlen ((* params)-> from) + 1;
          }
        else
            (* params)-> from               = NULL;
        if (_struct_ptr-> to)
          {
            (* params)-> to                 = mem_strdup (_ptr);
            _ptr += strlen ((* params)-> to) + 1;
          }
        else
            (* params)-> to                 = NULL;
        if (_struct_ptr-> cc)
          {
            (* params)-> cc                 = mem_strdup (_ptr);
            _ptr += strlen ((* params)-> cc) + 1;
          }
        else
            (* params)-> cc                 = NULL;
        if (_struct_ptr-> date)
          {
            (* params)-> date               = mem_strdup (_ptr);
            _ptr += strlen ((* params)-> date) + 1;
          }
        else
            (* params)-> date               = NULL;
        if (_struct_ptr-> subject)
          {
            (* params)-> subject            = mem_strdup (_ptr);
            _ptr += strlen ((* params)-> subject) + 1;
          }
        else
            (* params)-> subject            = NULL;
        return 0;
      }
    else
        return -1;
}


/*  ---------------------------------------------------------------------[<]-
    Function: free_smtpop_msg_header

    Synopsis: frees a structure allocated by get_smtpop_msg_header().
    ---------------------------------------------------------------------[>]-*/

void
free_smtpop_msg_header (
    struct_smtpop_msg_header **params)
{
    mem_free ((*params)-> from);
    mem_free ((*params)-> to);
    mem_free ((*params)-> cc);
    mem_free ((*params)-> date);
    mem_free ((*params)-> subject);
    mem_free (*params);
    *params = NULL;
}

char *SMTPOP_MSG_HEADER = "SMTPOP MSG_HEADER";

/*  ---------------------------------------------------------------------[<]-
    Function: lsend_smtpop_msg_header

    Synopsis: Sends a msg_header - mail info
              event to the smtpop agent
    ---------------------------------------------------------------------[>]-*/

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
    const char *subject)            /*  guess what                       */
{
    byte *_body;
    int   _size,
          _rc;
    _size = put_smtpop_msg_header
                (&_body,
                 msg_id,
                 from,
                 to,
                 cc,
                 date,
                 subject);
    if (_size)
      {
        _rc = event_send (_to, _from, SMTPOP_MSG_HEADER,
                          _body, _size,
                          _accept, _reject, _expire, _timeout);
        mem_free (_body);
        return _rc;
      }
    else
        return -1;
}


/*  ---------------------------------------------------------------------[<]-
    Function: put_smtpop_msg

    Synopsis: Formats a msg message, allocates a new buffer,
    and returns the formatted message in the buffer.  You should free the
    buffer using mem_free() when finished.  Returns the size of the buffer
    in bytes.
    ---------------------------------------------------------------------[>]-*/

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
    const char *attach_dir)             /*  directory where attach have been stored  */
{
    struct_smtpop_msg
        *_struct_ptr;
    int
        _total_size = sizeof (struct_smtpop_msg);
    char
        *_ptr;

    _total_size += from ? strlen (from) + 1 : 0;
    _total_size += to ? strlen (to) + 1 : 0;
    _total_size += cc ? strlen (cc) + 1 : 0;
    _total_size += date ? strlen (date) + 1 : 0;
    _total_size += subject ? strlen (subject) + 1 : 0;
    _total_size += body ? strlen (body) + 1 : 0;
    _total_size += attachments ? strlen (attachments) + 1 : 0;
    _total_size += attach_dir ? strlen (attach_dir) + 1 : 0;
    _struct_ptr = mem_alloc (_total_size);
    *_buffer = (byte *) _struct_ptr;
    if (_struct_ptr)
      {
        _ptr = (char *) _struct_ptr + sizeof (struct_smtpop_msg);
        _struct_ptr-> msg_id            = msg_id;
        if (from)
          {
            _struct_ptr-> from              = (char *) _ptr;
            strcpy ((char *) _ptr, from);
            _ptr += strlen (from) + 1;
          }
        else
            _struct_ptr-> from              = NULL;
        if (to)
          {
            _struct_ptr-> to                = (char *) _ptr;
            strcpy ((char *) _ptr, to);
            _ptr += strlen (to) + 1;
          }
        else
            _struct_ptr-> to                = NULL;
        if (cc)
          {
            _struct_ptr-> cc                = (char *) _ptr;
            strcpy ((char *) _ptr, cc);
            _ptr += strlen (cc) + 1;
          }
        else
            _struct_ptr-> cc                = NULL;
        if (date)
          {
            _struct_ptr-> date              = (char *) _ptr;
            strcpy ((char *) _ptr, date);
            _ptr += strlen (date) + 1;
          }
        else
            _struct_ptr-> date              = NULL;
        if (subject)
          {
            _struct_ptr-> subject           = (char *) _ptr;
            strcpy ((char *) _ptr, subject);
            _ptr += strlen (subject) + 1;
          }
        else
            _struct_ptr-> subject           = NULL;
        if (body)
          {
            _struct_ptr-> body              = (char *) _ptr;
            strcpy ((char *) _ptr, body);
            _ptr += strlen (body) + 1;
          }
        else
            _struct_ptr-> body              = NULL;
        if (attachments)
          {
            _struct_ptr-> attachments       = (char *) _ptr;
            strcpy ((char *) _ptr, attachments);
            _ptr += strlen (attachments) + 1;
          }
        else
            _struct_ptr-> attachments       = NULL;
        if (attach_dir)
          {
            _struct_ptr-> attach_dir        = (char *) _ptr;
            strcpy ((char *) _ptr, attach_dir);
            _ptr += strlen (attach_dir) + 1;
          }
        else
            _struct_ptr-> attach_dir        = NULL;

        return _total_size;
      }
    else
        return 0;
}


/*  ---------------------------------------------------------------------[<]-
    Function: get_smtpop_msg

    Synopsis: Accepts a buffer containing a msg message,
    and unpacks it into a new struct_smtpop_msg structure. Free the
    structure using free_smtpop_msg() when finished.
    ---------------------------------------------------------------------[>]-*/

int
get_smtpop_msg (
    byte *_buffer,
    struct_smtpop_msg **params)
{
    struct_smtpop_msg
        *_struct_ptr;
    char
        *_ptr;

    _struct_ptr = (struct_smtpop_msg *) _buffer;
    *params = mem_alloc (sizeof (struct_smtpop_msg));
    if (*params)
      {
        _ptr = (char *) _struct_ptr + sizeof (struct_smtpop_msg);
        (* params)-> msg_id             = _struct_ptr-> msg_id;
        if (_struct_ptr-> from)
          {
            (* params)-> from               = mem_strdup (_ptr);
            _ptr += strlen ((* params)-> from) + 1;
          }
        else
            (* params)-> from               = NULL;
        if (_struct_ptr-> to)
          {
            (* params)-> to                 = mem_strdup (_ptr);
            _ptr += strlen ((* params)-> to) + 1;
          }
        else
            (* params)-> to                 = NULL;
        if (_struct_ptr-> cc)
          {
            (* params)-> cc                 = mem_strdup (_ptr);
            _ptr += strlen ((* params)-> cc) + 1;
          }
        else
            (* params)-> cc                 = NULL;
        if (_struct_ptr-> date)
          {
            (* params)-> date               = mem_strdup (_ptr);
            _ptr += strlen ((* params)-> date) + 1;
          }
        else
            (* params)-> date               = NULL;
        if (_struct_ptr-> subject)
          {
            (* params)-> subject            = mem_strdup (_ptr);
            _ptr += strlen ((* params)-> subject) + 1;
          }
        else
            (* params)-> subject            = NULL;
        if (_struct_ptr-> body)
          {
            (* params)-> body               = mem_strdup (_ptr);
            _ptr += strlen ((* params)-> body) + 1;
          }
        else
            (* params)-> body               = NULL;
        if (_struct_ptr-> attachments)
          {
            (* params)-> attachments        = mem_strdup (_ptr);
            _ptr += strlen ((* params)-> attachments) + 1;
          }
        else
            (* params)-> attachments        = NULL;
        if (_struct_ptr-> attach_dir)
          {
            (* params)-> attach_dir         = mem_strdup (_ptr);
            _ptr += strlen ((* params)-> attach_dir) + 1;
          }
        else
            (* params)-> attach_dir         = NULL;
        return 0;
      }
    else
        return -1;
}


/*  ---------------------------------------------------------------------[<]-
    Function: free_smtpop_msg

    Synopsis: frees a structure allocated by get_smtpop_msg().
    ---------------------------------------------------------------------[>]-*/

void
free_smtpop_msg (
    struct_smtpop_msg **params)
{
    mem_free ((*params)-> from);
    mem_free ((*params)-> to);
    mem_free ((*params)-> cc);
    mem_free ((*params)-> date);
    mem_free ((*params)-> subject);
    mem_free ((*params)-> body);
    mem_free ((*params)-> attachments);
    mem_free ((*params)-> attach_dir);
    mem_free (*params);
    *params = NULL;
}

char *SMTPOP_MSG = "SMTPOP MSG";

/*  ---------------------------------------------------------------------[<]-
    Function: lsend_smtpop_msg

    Synopsis: Sends a msg - complete message
              event to the smtpop agent
    ---------------------------------------------------------------------[>]-*/

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
    const char *attach_dir)         /*  directory where attach have been stored  */
{
    byte *_body;
    int   _size,
          _rc;
    _size = put_smtpop_msg
                (&_body,
                 msg_id,
                 from,
                 to,
                 cc,
                 date,
                 subject,
                 body,
                 attachments,
                 attach_dir);
    if (_size)
      {
        _rc = event_send (_to, _from, SMTPOP_MSG,
                          _body, _size,
                          _accept, _reject, _expire, _timeout);
        mem_free (_body);
        return _rc;
      }
    else
        return -1;
}

/*---------------------------------------------------------------------------
 *  Message functions for smtsmtp - SMTP agent.
 *---------------------------------------------------------------------------*/


/*  ---------------------------------------------------------------------[<]-
    Function: put_smtsmtp_message

    Synopsis: Formats a message message, allocates a new buffer,
    and returns the formatted message in the buffer.  You should free the
    buffer using mem_free() when finished.  Returns the size of the buffer
    in bytes.
    ---------------------------------------------------------------------[>]-*/

int
put_smtsmtp_message (
    byte **_buffer,
    const char *smtp_server,            /*                                   */
    const char *msg_body,               /*                                   */
    const char *sender_uid,             /*                                   */
    const char *dest_uids,              /*                                   */
    const char *subject)                /*                                   */
{
    struct_smtsmtp_message
        *_struct_ptr;
    int
        _total_size = sizeof (struct_smtsmtp_message);
    char
        *_ptr;

    _total_size += smtp_server ? strlen (smtp_server) + 1 : 0;
    _total_size += msg_body ? strlen (msg_body) + 1 : 0;
    _total_size += sender_uid ? strlen (sender_uid) + 1 : 0;
    _total_size += dest_uids ? strlen (dest_uids) + 1 : 0;
    _total_size += subject ? strlen (subject) + 1 : 0;
    _struct_ptr = mem_alloc (_total_size);
    *_buffer = (byte *) _struct_ptr;
    if (_struct_ptr)
      {
        _ptr = (char *) _struct_ptr + sizeof (struct_smtsmtp_message);
        if (smtp_server)
          {
            _struct_ptr-> smtp_server       = (char *) _ptr;
            strcpy ((char *) _ptr, smtp_server);
            _ptr += strlen (smtp_server) + 1;
          }
        else
            _struct_ptr-> smtp_server       = NULL;
        if (msg_body)
          {
            _struct_ptr-> msg_body          = (char *) _ptr;
            strcpy ((char *) _ptr, msg_body);
            _ptr += strlen (msg_body) + 1;
          }
        else
            _struct_ptr-> msg_body          = NULL;
        if (sender_uid)
          {
            _struct_ptr-> sender_uid        = (char *) _ptr;
            strcpy ((char *) _ptr, sender_uid);
            _ptr += strlen (sender_uid) + 1;
          }
        else
            _struct_ptr-> sender_uid        = NULL;
        if (dest_uids)
          {
            _struct_ptr-> dest_uids         = (char *) _ptr;
            strcpy ((char *) _ptr, dest_uids);
            _ptr += strlen (dest_uids) + 1;
          }
        else
            _struct_ptr-> dest_uids         = NULL;
        if (subject)
          {
            _struct_ptr-> subject           = (char *) _ptr;
            strcpy ((char *) _ptr, subject);
            _ptr += strlen (subject) + 1;
          }
        else
            _struct_ptr-> subject           = NULL;

        return _total_size;
      }
    else
        return 0;
}


/*  ---------------------------------------------------------------------[<]-
    Function: get_smtsmtp_message

    Synopsis: Accepts a buffer containing a message message,
    and unpacks it into a new struct_smtsmtp_message structure. Free the
    structure using free_smtsmtp_message() when finished.
    ---------------------------------------------------------------------[>]-*/

int
get_smtsmtp_message (
    byte *_buffer,
    struct_smtsmtp_message **params)
{
    struct_smtsmtp_message
        *_struct_ptr;
    char
        *_ptr;

    _struct_ptr = (struct_smtsmtp_message *) _buffer;
    *params = mem_alloc (sizeof (struct_smtsmtp_message));
    if (*params)
      {
        _ptr = (char *) _struct_ptr + sizeof (struct_smtsmtp_message);
        if (_struct_ptr-> smtp_server)
          {
            (* params)-> smtp_server        = mem_strdup (_ptr);
            _ptr += strlen ((* params)-> smtp_server) + 1;
          }
        else
            (* params)-> smtp_server        = NULL;
        if (_struct_ptr-> msg_body)
          {
            (* params)-> msg_body           = mem_strdup (_ptr);
            _ptr += strlen ((* params)-> msg_body) + 1;
          }
        else
            (* params)-> msg_body           = NULL;
        if (_struct_ptr-> sender_uid)
          {
            (* params)-> sender_uid         = mem_strdup (_ptr);
            _ptr += strlen ((* params)-> sender_uid) + 1;
          }
        else
            (* params)-> sender_uid         = NULL;
        if (_struct_ptr-> dest_uids)
          {
            (* params)-> dest_uids          = mem_strdup (_ptr);
            _ptr += strlen ((* params)-> dest_uids) + 1;
          }
        else
            (* params)-> dest_uids          = NULL;
        if (_struct_ptr-> subject)
          {
            (* params)-> subject            = mem_strdup (_ptr);
            _ptr += strlen ((* params)-> subject) + 1;
          }
        else
            (* params)-> subject            = NULL;
        return 0;
      }
    else
        return -1;
}


/*  ---------------------------------------------------------------------[<]-
    Function: free_smtsmtp_message

    Synopsis: frees a structure allocated by get_smtsmtp_message().
    ---------------------------------------------------------------------[>]-*/

void
free_smtsmtp_message (
    struct_smtsmtp_message **params)
{
    mem_free ((*params)-> smtp_server);
    mem_free ((*params)-> msg_body);
    mem_free ((*params)-> sender_uid);
    mem_free ((*params)-> dest_uids);
    mem_free ((*params)-> subject);
    mem_free (*params);
    *params = NULL;
}

char *SMTSMTP_SEND_MESSAGE = "SMTSMTP SEND_MESSAGE";

/*  ---------------------------------------------------------------------[<]-
    Function: lsend_smtsmtp_send_message

    Synopsis: Sends a send_message -
              event to the smtsmtp agent
    ---------------------------------------------------------------------[>]-*/

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
    const char *subject)            /*                                   */
{
    byte *_body;
    int   _size,
          _rc;
    _size = put_smtsmtp_message
                (&_body,
                 smtp_server,
                 msg_body,
                 sender_uid,
                 dest_uids,
                 subject);
    if (_size)
      {
        _rc = event_send (_to, _from, SMTSMTP_SEND_MESSAGE,
                          _body, _size,
                          _accept, _reject, _expire, _timeout);
        mem_free (_body);
        return _rc;
      }
    else
        return -1;
}

char *SMTSMTP_OPEN_MESSAGE = "SMTSMTP OPEN_MESSAGE";

/*  ---------------------------------------------------------------------[<]-
    Function: lsend_smtsmtp_open_message

    Synopsis: Sends a open_message -
              event to the smtsmtp agent
    ---------------------------------------------------------------------[>]-*/

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
    const char *subject)            /*                                   */
{
    byte *_body;
    int   _size,
          _rc;
    _size = put_smtsmtp_message
                (&_body,
                 smtp_server,
                 msg_body,
                 sender_uid,
                 dest_uids,
                 subject);
    if (_size)
      {
        _rc = event_send (_to, _from, SMTSMTP_OPEN_MESSAGE,
                          _body, _size,
                          _accept, _reject, _expire, _timeout);
        mem_free (_body);
        return _rc;
      }
    else
        return -1;
}


/*  ---------------------------------------------------------------------[<]-
    Function: put_smtsmtp_chunk

    Synopsis: Formats a chunk message, allocates a new buffer,
    and returns the formatted message in the buffer.  You should free the
    buffer using mem_free() when finished.  Returns the size of the buffer
    in bytes.
    ---------------------------------------------------------------------[>]-*/

int
put_smtsmtp_chunk (
    byte **_buffer,
    const char *chunk)                  /*                                   */
{
    struct_smtsmtp_chunk
        *_struct_ptr;
    int
        _total_size = sizeof (struct_smtsmtp_chunk);
    char
        *_ptr;

    _total_size += chunk ? strlen (chunk) + 1 : 0;
    _struct_ptr = mem_alloc (_total_size);
    *_buffer = (byte *) _struct_ptr;
    if (_struct_ptr)
      {
        _ptr = (char *) _struct_ptr + sizeof (struct_smtsmtp_chunk);
        if (chunk)
          {
            _struct_ptr-> chunk             = (char *) _ptr;
            strcpy ((char *) _ptr, chunk);
            _ptr += strlen (chunk) + 1;
          }
        else
            _struct_ptr-> chunk             = NULL;

        return _total_size;
      }
    else
        return 0;
}


/*  ---------------------------------------------------------------------[<]-
    Function: get_smtsmtp_chunk

    Synopsis: Accepts a buffer containing a chunk message,
    and unpacks it into a new struct_smtsmtp_chunk structure. Free the
    structure using free_smtsmtp_chunk() when finished.
    ---------------------------------------------------------------------[>]-*/

int
get_smtsmtp_chunk (
    byte *_buffer,
    struct_smtsmtp_chunk **params)
{
    struct_smtsmtp_chunk
        *_struct_ptr;
    char
        *_ptr;

    _struct_ptr = (struct_smtsmtp_chunk *) _buffer;
    *params = mem_alloc (sizeof (struct_smtsmtp_chunk));
    if (*params)
      {
        _ptr = (char *) _struct_ptr + sizeof (struct_smtsmtp_chunk);
        if (_struct_ptr-> chunk)
          {
            (* params)-> chunk              = mem_strdup (_ptr);
            _ptr += strlen ((* params)-> chunk) + 1;
          }
        else
            (* params)-> chunk              = NULL;
        return 0;
      }
    else
        return -1;
}


/*  ---------------------------------------------------------------------[<]-
    Function: free_smtsmtp_chunk

    Synopsis: frees a structure allocated by get_smtsmtp_chunk().
    ---------------------------------------------------------------------[>]-*/

void
free_smtsmtp_chunk (
    struct_smtsmtp_chunk **params)
{
    mem_free ((*params)-> chunk);
    mem_free (*params);
    *params = NULL;
}

char *SMTSMTP_MESSAGE_CHUNK = "SMTSMTP MESSAGE CHUNK";

/*  ---------------------------------------------------------------------[<]-
    Function: lsend_smtsmtp_message_chunk

    Synopsis: Sends a message chunk -
              event to the smtsmtp agent
    ---------------------------------------------------------------------[>]-*/

int
lsend_smtsmtp_message_chunk (
    const QID  *_to,
    const QID  *_from,
          char *_accept,
          char *_reject,
          char *_expire,
          word _timeout,
    const char *chunk)              /*                                   */
{
    byte *_body;
    int   _size,
          _rc;
    _size = put_smtsmtp_chunk
                (&_body,
                 chunk);
    if (_size)
      {
        _rc = event_send (_to, _from, SMTSMTP_MESSAGE_CHUNK,
                          _body, _size,
                          _accept, _reject, _expire, _timeout);
        mem_free (_body);
        return _rc;
      }
    else
        return -1;
}

char *SMTSMTP_CLOSE_MESSAGE = "SMTSMTP CLOSE_MESSAGE";


/*  ---------------------------------------------------------------------[<]-
    Function: put_smtsmtp_reply

    Synopsis: Formats a reply message, allocates a new buffer,
    and returns the formatted message in the buffer.  You should free the
    buffer using mem_free() when finished.  Returns the size of the buffer
    in bytes.
    ---------------------------------------------------------------------[>]-*/

int
put_smtsmtp_reply (
    byte **_buffer,
    const qbyte code,                   /*  error code                       */
    const char *msg)                    /*  error description                */
{
    struct_smtsmtp_reply
        *_struct_ptr;
    int
        _total_size = sizeof (struct_smtsmtp_reply);
    char
        *_ptr;

    _total_size += msg ? strlen (msg) + 1 : 0;
    _struct_ptr = mem_alloc (_total_size);
    *_buffer = (byte *) _struct_ptr;
    if (_struct_ptr)
      {
        _ptr = (char *) _struct_ptr + sizeof (struct_smtsmtp_reply);
        _struct_ptr-> code              = code;
        if (msg)
          {
            _struct_ptr-> msg               = (char *) _ptr;
            strcpy ((char *) _ptr, msg);
            _ptr += strlen (msg) + 1;
          }
        else
            _struct_ptr-> msg               = NULL;

        return _total_size;
      }
    else
        return 0;
}


/*  ---------------------------------------------------------------------[<]-
    Function: get_smtsmtp_reply

    Synopsis: Accepts a buffer containing a reply message,
    and unpacks it into a new struct_smtsmtp_reply structure. Free the
    structure using free_smtsmtp_reply() when finished.
    ---------------------------------------------------------------------[>]-*/

int
get_smtsmtp_reply (
    byte *_buffer,
    struct_smtsmtp_reply **params)
{
    struct_smtsmtp_reply
        *_struct_ptr;
    char
        *_ptr;

    _struct_ptr = (struct_smtsmtp_reply *) _buffer;
    *params = mem_alloc (sizeof (struct_smtsmtp_reply));
    if (*params)
      {
        _ptr = (char *) _struct_ptr + sizeof (struct_smtsmtp_reply);
        (* params)-> code               = _struct_ptr-> code;
        if (_struct_ptr-> msg)
          {
            (* params)-> msg                = mem_strdup (_ptr);
            _ptr += strlen ((* params)-> msg) + 1;
          }
        else
            (* params)-> msg                = NULL;
        return 0;
      }
    else
        return -1;
}


/*  ---------------------------------------------------------------------[<]-
    Function: free_smtsmtp_reply

    Synopsis: frees a structure allocated by get_smtsmtp_reply().
    ---------------------------------------------------------------------[>]-*/

void
free_smtsmtp_reply (
    struct_smtsmtp_reply **params)
{
    mem_free ((*params)-> msg);
    mem_free (*params);
    *params = NULL;
}

char *SMTSMTP_OK = "SMTSMTP OK";

/*  ---------------------------------------------------------------------[<]-
    Function: lsend_smtsmtp_ok

    Synopsis: Sends a ok -
              event to the smtsmtp agent
    ---------------------------------------------------------------------[>]-*/

int
lsend_smtsmtp_ok (
    const QID  *_to,
    const QID  *_from,
          char *_accept,
          char *_reject,
          char *_expire,
          word _timeout,
    const qbyte code,               /*  error code                       */
    const char *msg)                /*  error description                */
{
    byte *_body;
    int   _size,
          _rc;
    _size = put_smtsmtp_reply
                (&_body,
                 code,
                 msg);
    if (_size)
      {
        _rc = event_send (_to, _from, SMTSMTP_OK,
                          _body, _size,
                          _accept, _reject, _expire, _timeout);
        mem_free (_body);
        return _rc;
      }
    else
        return -1;
}

char *SMTSMTP_ERROR = "SMTSMTP ERROR";

/*  ---------------------------------------------------------------------[<]-
    Function: lsend_smtsmtp_error

    Synopsis: Sends a error -
              event to the smtsmtp agent
    ---------------------------------------------------------------------[>]-*/

int
lsend_smtsmtp_error (
    const QID  *_to,
    const QID  *_from,
          char *_accept,
          char *_reject,
          char *_expire,
          word _timeout,
    const qbyte code,               /*  error code                       */
    const char *msg)                /*  error description                */
{
    byte *_body;
    int   _size,
          _rc;
    _size = put_smtsmtp_reply
                (&_body,
                 code,
                 msg);
    if (_size)
      {
        _rc = event_send (_to, _from, SMTSMTP_ERROR,
                          _body, _size,
                          _accept, _reject, _expire, _timeout);
        mem_free (_body);
        return _rc;
      }
    else
        return -1;
}

/*---------------------------------------------------------------------------
 *  Message functions for smttmtp - TMTP agent.
 *---------------------------------------------------------------------------*/


/*  ---------------------------------------------------------------------[<]-
    Function: put_smttmtp_write

    Synopsis: Formats a write message, allocates a new buffer,
    and returns the formatted message in the buffer.  You should free the
    buffer using mem_free() when finished.  Returns the size of the buffer
    in bytes.
    ---------------------------------------------------------------------[>]-*/

int
put_smttmtp_write (
    byte **_buffer,
    const char *host,                   /*  Remote host                      */
    const char *port,                   /*                                   */
    const word  size,                   /*  Amount of data to write          */
    const void *data)                   /*  Block of data to write           */
{
    struct_smttmtp_write
        *_struct_ptr;
    int
        _total_size = sizeof (struct_smttmtp_write);
    char
        *_ptr;

    _total_size += host ? strlen (host) + 1 : 0;
    _total_size += port ? strlen (port) + 1 : 0;
    _total_size += size;
    _struct_ptr = mem_alloc (_total_size);
    *_buffer = (byte *) _struct_ptr;
    if (_struct_ptr)
      {
        _ptr = (char *) _struct_ptr + sizeof (struct_smttmtp_write);
        if (host)
          {
            _struct_ptr-> host              = (char *) _ptr;
            strcpy ((char *) _ptr, host);
            _ptr += strlen (host) + 1;
          }
        else
            _struct_ptr-> host              = NULL;
        if (port)
          {
            _struct_ptr-> port              = (char *) _ptr;
            strcpy ((char *) _ptr, port);
            _ptr += strlen (port) + 1;
          }
        else
            _struct_ptr-> port              = NULL;
        _struct_ptr-> size              = size;
        _struct_ptr-> data              = (byte *) _ptr;
        memcpy (_ptr, data, size);
        _ptr += size;

        return _total_size;
      }
    else
        return 0;
}


/*  ---------------------------------------------------------------------[<]-
    Function: get_smttmtp_write

    Synopsis: Accepts a buffer containing a write message,
    and unpacks it into a new struct_smttmtp_write structure. Free the
    structure using free_smttmtp_write() when finished.
    ---------------------------------------------------------------------[>]-*/

int
get_smttmtp_write (
    byte *_buffer,
    struct_smttmtp_write **params)
{
    struct_smttmtp_write
        *_struct_ptr;
    char
        *_ptr;

    _struct_ptr = (struct_smttmtp_write *) _buffer;
    *params = mem_alloc (sizeof (struct_smttmtp_write));
    if (*params)
      {
        _ptr = (char *) _struct_ptr + sizeof (struct_smttmtp_write);
        if (_struct_ptr-> host)
          {
            (* params)-> host               = mem_strdup (_ptr);
            _ptr += strlen ((* params)-> host) + 1;
          }
        else
            (* params)-> host               = NULL;
        if (_struct_ptr-> port)
          {
            (* params)-> port               = mem_strdup (_ptr);
            _ptr += strlen ((* params)-> port) + 1;
          }
        else
            (* params)-> port               = NULL;
        (* params)-> size               = _struct_ptr-> size;
        (* params)-> data               = mem_alloc (_struct_ptr-> size + 1);
        memcpy ((* params)-> data, _ptr, _struct_ptr-> size);
        *((byte *)(* params)-> data + _struct_ptr-> size) = 0;
        _ptr += _struct_ptr-> size;
        return 0;
      }
    else
        return -1;
}


/*  ---------------------------------------------------------------------[<]-
    Function: free_smttmtp_write

    Synopsis: frees a structure allocated by get_smttmtp_write().
    ---------------------------------------------------------------------[>]-*/

void
free_smttmtp_write (
    struct_smttmtp_write **params)
{
    mem_free ((*params)-> host);
    mem_free ((*params)-> port);
    mem_free ((*params)-> data);
    mem_free (*params);
    *params = NULL;
}

char *SMTTMTP_WRITE = "SMTTMTP WRITE";

/*  ---------------------------------------------------------------------[<]-
    Function: lsend_smttmtp_write

    Synopsis: Sends a write -
              event to the smttmtp agent
    ---------------------------------------------------------------------[>]-*/

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
    const void *data)               /*  Block of data to write           */
{
    byte *_body;
    int   _size,
          _rc;
    _size = put_smttmtp_write
                (&_body,
                 host,
                 port,
                 size,
                 data);
    if (_size)
      {
        _rc = event_send (_to, _from, SMTTMTP_WRITE,
                          _body, _size,
                          _accept, _reject, _expire, _timeout);
        mem_free (_body);
        return _rc;
      }
    else
        return -1;
}


/*  ---------------------------------------------------------------------[<]-
    Function: put_smttmtp_listen

    Synopsis: Formats a listen message, allocates a new buffer,
    and returns the formatted message in the buffer.  You should free the
    buffer using mem_free() when finished.  Returns the size of the buffer
    in bytes.
    ---------------------------------------------------------------------[>]-*/

int
put_smttmtp_listen (
    byte **_buffer,
    const char *port)                   /*                                   */
{
    struct_smttmtp_listen
        *_struct_ptr;
    int
        _total_size = sizeof (struct_smttmtp_listen);
    char
        *_ptr;

    _total_size += port ? strlen (port) + 1 : 0;
    _struct_ptr = mem_alloc (_total_size);
    *_buffer = (byte *) _struct_ptr;
    if (_struct_ptr)
      {
        _ptr = (char *) _struct_ptr + sizeof (struct_smttmtp_listen);
        if (port)
          {
            _struct_ptr-> port              = (char *) _ptr;
            strcpy ((char *) _ptr, port);
            _ptr += strlen (port) + 1;
          }
        else
            _struct_ptr-> port              = NULL;

        return _total_size;
      }
    else
        return 0;
}


/*  ---------------------------------------------------------------------[<]-
    Function: get_smttmtp_listen

    Synopsis: Accepts a buffer containing a listen message,
    and unpacks it into a new struct_smttmtp_listen structure. Free the
    structure using free_smttmtp_listen() when finished.
    ---------------------------------------------------------------------[>]-*/

int
get_smttmtp_listen (
    byte *_buffer,
    struct_smttmtp_listen **params)
{
    struct_smttmtp_listen
        *_struct_ptr;
    char
        *_ptr;

    _struct_ptr = (struct_smttmtp_listen *) _buffer;
    *params = mem_alloc (sizeof (struct_smttmtp_listen));
    if (*params)
      {
        _ptr = (char *) _struct_ptr + sizeof (struct_smttmtp_listen);
        if (_struct_ptr-> port)
          {
            (* params)-> port               = mem_strdup (_ptr);
            _ptr += strlen ((* params)-> port) + 1;
          }
        else
            (* params)-> port               = NULL;
        return 0;
      }
    else
        return -1;
}


/*  ---------------------------------------------------------------------[<]-
    Function: free_smttmtp_listen

    Synopsis: frees a structure allocated by get_smttmtp_listen().
    ---------------------------------------------------------------------[>]-*/

void
free_smttmtp_listen (
    struct_smttmtp_listen **params)
{
    mem_free ((*params)-> port);
    mem_free (*params);
    *params = NULL;
}

char *SMTTMTP_LISTEN = "SMTTMTP LISTEN";

/*  ---------------------------------------------------------------------[<]-
    Function: lsend_smttmtp_listen

    Synopsis: Sends a listen -
              event to the smttmtp agent
    ---------------------------------------------------------------------[>]-*/

int
lsend_smttmtp_listen (
    const QID  *_to,
    const QID  *_from,
          char *_accept,
          char *_reject,
          char *_expire,
          word _timeout,
    const char *port)               /*                                   */
{
    byte *_body;
    int   _size,
          _rc;
    _size = put_smttmtp_listen
                (&_body,
                 port);
    if (_size)
      {
        _rc = event_send (_to, _from, SMTTMTP_LISTEN,
                          _body, _size,
                          _accept, _reject, _expire, _timeout);
        mem_free (_body);
        return _rc;
      }
    else
        return -1;
}

char *SMTTMTP_WRITE_OK = "SMTTMTP WRITE OK";

char *SMTTMTP_LISTEN_OK = "SMTTMTP LISTEN OK";


/*  ---------------------------------------------------------------------[<]-
    Function: put_smttmtp_error

    Synopsis: Formats a error message, allocates a new buffer,
    and returns the formatted message in the buffer.  You should free the
    buffer using mem_free() when finished.  Returns the size of the buffer
    in bytes.
    ---------------------------------------------------------------------[>]-*/

int
put_smttmtp_error (
    byte **_buffer,
    const char *message)                /*                                   */
{
    struct_smttmtp_error
        *_struct_ptr;
    int
        _total_size = sizeof (struct_smttmtp_error);
    char
        *_ptr;

    _total_size += message ? strlen (message) + 1 : 0;
    _struct_ptr = mem_alloc (_total_size);
    *_buffer = (byte *) _struct_ptr;
    if (_struct_ptr)
      {
        _ptr = (char *) _struct_ptr + sizeof (struct_smttmtp_error);
        if (message)
          {
            _struct_ptr-> message           = (char *) _ptr;
            strcpy ((char *) _ptr, message);
            _ptr += strlen (message) + 1;
          }
        else
            _struct_ptr-> message           = NULL;

        return _total_size;
      }
    else
        return 0;
}


/*  ---------------------------------------------------------------------[<]-
    Function: get_smttmtp_error

    Synopsis: Accepts a buffer containing a error message,
    and unpacks it into a new struct_smttmtp_error structure. Free the
    structure using free_smttmtp_error() when finished.
    ---------------------------------------------------------------------[>]-*/

int
get_smttmtp_error (
    byte *_buffer,
    struct_smttmtp_error **params)
{
    struct_smttmtp_error
        *_struct_ptr;
    char
        *_ptr;

    _struct_ptr = (struct_smttmtp_error *) _buffer;
    *params = mem_alloc (sizeof (struct_smttmtp_error));
    if (*params)
      {
        _ptr = (char *) _struct_ptr + sizeof (struct_smttmtp_error);
        if (_struct_ptr-> message)
          {
            (* params)-> message            = mem_strdup (_ptr);
            _ptr += strlen ((* params)-> message) + 1;
          }
        else
            (* params)-> message            = NULL;
        return 0;
      }
    else
        return -1;
}


/*  ---------------------------------------------------------------------[<]-
    Function: free_smttmtp_error

    Synopsis: frees a structure allocated by get_smttmtp_error().
    ---------------------------------------------------------------------[>]-*/

void
free_smttmtp_error (
    struct_smttmtp_error **params)
{
    mem_free ((*params)-> message);
    mem_free (*params);
    *params = NULL;
}

char *SMTTMTP_WRITE_ERROR = "SMTTMTP WRITE ERROR";

/*  ---------------------------------------------------------------------[<]-
    Function: lsend_smttmtp_write_error

    Synopsis: Sends a write error -
              event to the smttmtp agent
    ---------------------------------------------------------------------[>]-*/

int
lsend_smttmtp_write_error (
    const QID  *_to,
    const QID  *_from,
          char *_accept,
          char *_reject,
          char *_expire,
          word _timeout,
    const char *message)            /*                                   */
{
    byte *_body;
    int   _size,
          _rc;
    _size = put_smttmtp_error
                (&_body,
                 message);
    if (_size)
      {
        _rc = event_send (_to, _from, SMTTMTP_WRITE_ERROR,
                          _body, _size,
                          _accept, _reject, _expire, _timeout);
        mem_free (_body);
        return _rc;
      }
    else
        return -1;
}

char *SMTTMTP_LISTEN_ERROR = "SMTTMTP LISTEN ERROR";

/*  ---------------------------------------------------------------------[<]-
    Function: lsend_smttmtp_listen_error

    Synopsis: Sends a listen error -
              event to the smttmtp agent
    ---------------------------------------------------------------------[>]-*/

int
lsend_smttmtp_listen_error (
    const QID  *_to,
    const QID  *_from,
          char *_accept,
          char *_reject,
          char *_expire,
          word _timeout,
    const char *message)            /*                                   */
{
    byte *_body;
    int   _size,
          _rc;
    _size = put_smttmtp_error
                (&_body,
                 message);
    if (_size)
      {
        _rc = event_send (_to, _from, SMTTMTP_LISTEN_ERROR,
                          _body, _size,
                          _accept, _reject, _expire, _timeout);
        mem_free (_body);
        return _rc;
      }
    else
        return -1;
}


/*  ---------------------------------------------------------------------[<]-
    Function: put_smttmtp_incoming_message

    Synopsis: Formats a incoming message message, allocates a new buffer,
    and returns the formatted message in the buffer.  You should free the
    buffer using mem_free() when finished.  Returns the size of the buffer
    in bytes.
    ---------------------------------------------------------------------[>]-*/

int
put_smttmtp_incoming_message (
    byte **_buffer,
    const word  size,                   /*  Amount of data to write          */
    const void *data)                   /*  Block of data read               */
{
    struct_smttmtp_incoming_message
        *_struct_ptr;
    int
        _total_size = sizeof (struct_smttmtp_incoming_message);
    char
        *_ptr;

    _total_size += size;
    _struct_ptr = mem_alloc (_total_size);
    *_buffer = (byte *) _struct_ptr;
    if (_struct_ptr)
      {
        _ptr = (char *) _struct_ptr + sizeof (struct_smttmtp_incoming_message);
        _struct_ptr-> size              = size;
        _struct_ptr-> data              = (byte *) _ptr;
        memcpy (_ptr, data, size);
        _ptr += size;

        return _total_size;
      }
    else
        return 0;
}


/*  ---------------------------------------------------------------------[<]-
    Function: get_smttmtp_incoming_message

    Synopsis: Accepts a buffer containing a incoming message message,
    and unpacks it into a new struct_smttmtp_incoming_message structure. Free the
    structure using free_smttmtp_incoming_message() when finished.
    ---------------------------------------------------------------------[>]-*/

int
get_smttmtp_incoming_message (
    byte *_buffer,
    struct_smttmtp_incoming_message **params)
{
    struct_smttmtp_incoming_message
        *_struct_ptr;
    char
        *_ptr;

    _struct_ptr = (struct_smttmtp_incoming_message *) _buffer;
    *params = mem_alloc (sizeof (struct_smttmtp_incoming_message));
    if (*params)
      {
        _ptr = (char *) _struct_ptr + sizeof (struct_smttmtp_incoming_message);
        (* params)-> size               = _struct_ptr-> size;
        (* params)-> data               = mem_alloc (_struct_ptr-> size + 1);
        memcpy ((* params)-> data, _ptr, _struct_ptr-> size);
        *((byte *)(* params)-> data + _struct_ptr-> size) = 0;
        _ptr += _struct_ptr-> size;
        return 0;
      }
    else
        return -1;
}


/*  ---------------------------------------------------------------------[<]-
    Function: free_smttmtp_incoming_message

    Synopsis: frees a structure allocated by get_smttmtp_incoming_message().
    ---------------------------------------------------------------------[>]-*/

void
free_smttmtp_incoming_message (
    struct_smttmtp_incoming_message **params)
{
    mem_free ((*params)-> data);
    mem_free (*params);
    *params = NULL;
}

char *SMTTMTP_INCOMING_DATA = "SMTTMTP INCOMING DATA";

/*  ---------------------------------------------------------------------[<]-
    Function: lsend_smttmtp_incoming_data

    Synopsis: Sends a incoming data -
              event to the smttmtp agent
    ---------------------------------------------------------------------[>]-*/

int
lsend_smttmtp_incoming_data (
    const QID  *_to,
    const QID  *_from,
          char *_accept,
          char *_reject,
          char *_expire,
          word _timeout,
    const word  size,               /*  Amount of data to write          */
    const void *data)               /*  Block of data read               */
{
    byte *_body;
    int   _size,
          _rc;
    _size = put_smttmtp_incoming_message
                (&_body,
                 size,
                 data);
    if (_size)
      {
        _rc = event_send (_to, _from, SMTTMTP_INCOMING_DATA,
                          _body, _size,
                          _accept, _reject, _expire, _timeout);
        mem_free (_body);
        return _rc;
      }
    else
        return -1;
}


