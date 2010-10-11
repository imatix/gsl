/*===========================================================================*
 *                                                                           *
 *  sflcons.c - Console output functions                                     *
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

#include "prelude.h"                    /*  Universal header file            */
#include "sflfile.h"                    /*  File-handling functions          */
#include "sflstr.h"                     /*  String-handling functions        */
#include "sfllist.h"                    /*  Linked-list functions            */
#include "sflmem.h"                     /*  Memory-management functions      */
#include "sfldate.h"                    /*  Date/time functions              */
#include "sflprint.h"                   /*  snprintf functions               */
#include "sflcons.h"                    /*  Prototypes for functions         */


#define CONSOLE_LINE_MAX      16000     /*  Maximum size for coprintf        */
#define CONSOLE_ECHO_STREAM   stderr    /*  Stream to echo console output to */

static Bool
    console_active = TRUE,              /*  Allow console output?            */
    console_echo   = TRUE;              /*  Copy to console echo stream      */
static CONSOLE_FCT
    *console_fct = NULL;                /*  Redirector function              */
static int
    console_mode = CONSOLE_PLAIN;       /*  Output display mode              */
static FILE
    *console_file = NULL;               /*  Capture file, if any             */
static const char
    *console_filename = NULL;           /*  Name of capture file             */
    
static char  *date_str  (void);
static char  *time_str  (void);


/*  ---------------------------------------------------------------------[<]-
    Function: console_send

    Synopsis: Redirects console output to a specified CONSOLE_FCT function.
    If the specified address is NULL, the default handling will be reinstated.
    This is independent of any console capturing in progress.  If the echo
    argument is TRUE, console output is also sent to the echo stream (stderr).

    Safety:  NOT thread safe (shared variables), buffer safe.
    ---------------------------------------------------------------------[>]-*/

void
console_send (CONSOLE_FCT *new_console_fct, Bool echo)
{
    console_fct  = new_console_fct;
    console_echo = echo;                /*  Copy to console echo stream      */
}


/*  ---------------------------------------------------------------------[<]-
    Function: console_enable

    Synopsis: Enables console output.  Use together with console_disable()
    to stop and start console output.

    Safety:  NOT thread safe (shared variables), buffer safe.
    ---------------------------------------------------------------------[>]-*/

void
console_enable (void)
{
    console_active = TRUE;
}


/*  ---------------------------------------------------------------------[<]-
    Function: console_disable

    Synopsis: Disables console output. Use together with console_enable()
    to stop and start console output.

    Safety:  NOT thread safe (shared variables), buffer safe.
    ---------------------------------------------------------------------[>]-*/

void
console_disable (void)
{
    console_active = FALSE;
}


/*  ---------------------------------------------------------------------[<]-
    Function: console_set_mode

    Synopsis: Sets console display mode; the argument can be one of:
    <TABLE>
    CONSOLE_PLAIN       Output text exactly as specified
    CONSOLE_DATETIME    Prefix text by "yy/mm/dd hh:mm:ss "
    CONSOLE_TIME        Prefix text by "hh:mm:ss "
    CONSOLE_DEBUG       As CONSOLE_DATETIME but output is fully flushed
    </TABLE>
    The default is plain output.

    Safety:  NOT thread safe (shared variables), buffer safe.
    ---------------------------------------------------------------------[>]-*/

void
console_set_mode (int mode)
{
    ASSERT (mode == CONSOLE_PLAIN
         || mode == CONSOLE_DATETIME
         || mode == CONSOLE_TIME
         || mode == CONSOLE_DEBUG);

    console_mode = mode;
}


/*  ---------------------------------------------------------------------[<]-
    Function: console_capture

    Synopsis: Starts capturing console output to the specified file.  If the
    mode is 'w', creates an empty capture file.  If the mode is 'a', appends
    to any existing data.  Returns 0 if okay, -1 if there was an error - in
    this case you can test the value of errno.  If the filename is NULL or
    an empty string, closes any current capture file.

    Safety: NOT thread safe (shared variables), buffer safe.
    ---------------------------------------------------------------------[>]-*/

int
console_capture (const char *filename, char mode)
{
    if (console_file)
      {
        file_close (console_file);
        console_file = NULL;
      }
    if (filename && *filename)
      {
        ASSERT (mode == 'w' || mode == 'a');
        console_file = file_open (filename, mode);
        if (console_file)
            console_filename = filename;
        else
            return (-1);
      }
    return (0);
}


/*  ---------------------------------------------------------------------[<]-
    Function: coprintf

    Synopsis: As printf() but sends output to the current console.  This is
    by default the stderr device, unless you used console_send() to direct
    console output to some function.  A newline is added automatically.

    Safety:  NOT thread safe (shared variables), buffer safe.
    ---------------------------------------------------------------------[>]-*/

int
coprintf (const char *format, ...)
{
    va_list
        argptr;                         /*  Argument list pointer            */
    int
        fmtsize = 0;                    /*  Size of formatted line           */
    char
        *formatted = NULL,              /*  Formatted line                   */
        *prefixed = NULL;               /*  Prefixed formatted line          */

    if (console_active)
      {
        formatted = mem_alloc (CONSOLE_LINE_MAX + 1);
        if (!formatted)
            return (0);
        va_start (argptr, format);      /*  Start variable args processing   */
        vsnprintf (formatted, CONSOLE_LINE_MAX, format, argptr);
        va_end (argptr);                /*  End variable args processing     */
        switch (console_mode)
          {
            case CONSOLE_DATETIME:
            case CONSOLE_DEBUG:
                xstrcpy_debug ();
                prefixed = xstrcpy (NULL, date_str (), " ", time_str (), ": ",
                                    formatted, NULL);
                break;
            case CONSOLE_TIME:
                xstrcpy_debug ();
                prefixed = xstrcpy (NULL, time_str (), ": ", formatted, NULL);
                break;
          }
        if (console_file)
          {
            file_write (console_file, prefixed? prefixed: formatted);
            if (console_mode == CONSOLE_DEBUG)
                console_capture (console_filename, 'a');
            else
                fflush (console_file);
          }
        if (console_fct)
            (console_fct) (prefixed? prefixed: formatted);

        if (console_echo)
          {
            fprintf (CONSOLE_ECHO_STREAM, "%s", prefixed? prefixed: formatted);
            fprintf (CONSOLE_ECHO_STREAM, "\n");
            fflush  (CONSOLE_ECHO_STREAM);
          }
        if (prefixed)
          {
            fmtsize = strlen (prefixed);
            mem_free (prefixed);
          }
        else
            fmtsize = strlen (formatted);

        mem_free (formatted);
      }
    return (fmtsize);
}


/*  -------------------------------------------------------------------------
 *  date_str
 *
 *  Returns the current date formatted as: "yyyy/mm/dd".
 *
 *  Safety:  NOT thread safe (shared variables), buffer safe.
 */

static char *
date_str (void)
{
    static char
        formatted_date [11];
    time_t
        time_secs;
    struct tm
        *time_struct;

    time_secs   = time (NULL);
    time_struct = safe_localtime (&time_secs);

    snprintf (formatted_date, sizeof (formatted_date),
                              "%4d/%02d/%02d",
                              time_struct-> tm_year + 1900,
                              time_struct-> tm_mon  + 1,
                              time_struct-> tm_mday);

    return (formatted_date);
}


/*  -------------------------------------------------------------------------
 *  time_str
 *
 *  Returns the current time formatted as: "hh:mm:ss".
 *
 *  Safety:  NOT thread safe (shared variables), buffer safe.
 */

static char *
time_str (void)
{
    static char
        formatted_time [9];
    time_t
        time_secs;
    struct tm
        *time_struct;

    time_secs   = time (NULL);
    time_struct = safe_localtime (&time_secs);

    snprintf (formatted_time, sizeof (formatted_time),
                              "%02d:%02d:%02d",
                              time_struct-> tm_hour,
                              time_struct-> tm_min,
                              time_struct-> tm_sec);
    return (formatted_time);
}


/*  ---------------------------------------------------------------------[<]-
    Function: coputs

    Synopsis: As puts() but sends output to the current console.  This is
    by default the stderr device, unless you used console_send() to direct
    console output to some function, and/or have turned on console capturing.

    Safety:  NOT thread safe (shared variables), buffer safe.
    ---------------------------------------------------------------------[>]-*/

int
coputs (const char *string)
{
    coprintf ("%s", string);
    return (1);
}


/*  ---------------------------------------------------------------------[<]-
    Function: coputc

    Synopsis: As putc() but sends output to the current console.  This is
    by default the stderr device, unless you used console_send() to direct
    console output to some function.

    Safety:  NOT thread safe (shared variables), buffer safe.
    ---------------------------------------------------------------------[>]-*/

int
coputc (int character)
{
    char
        buffer [2];

    if (console_active)
      {
        if (console_file)
          {
            putc (character, console_file);
            fflush (console_file);
          }
        if (console_fct)
          {
            buffer [0] = (char) character;
            buffer [1] = '\0';
            (console_fct) (buffer);
          }
        if (console_echo)
          {
            putc (character, CONSOLE_ECHO_STREAM);
            fflush  (CONSOLE_ECHO_STREAM);
          }
      }
    return (character);
}
