/*===========================================================================*
 *                                                                           *
 *  sflprint.c - Printing functions                                          *
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
 *  --------------------------------------------------------------           *
 *===========================================================================*/
/*  ----------------------------------------------------------------<Prolog>-
    Synopsis:   Provides printing functions which may be absent on some
                systems.   In particular ensures that the system has 
                snprintf()/vsnprintf() functions which can be called.  The
                functions supplied here are far from perfect (exiting if
                the buffer could have been overflowed "too much"), but 
                are better than having none at all, and do allow 
                snprintf()/vsnprintf() to be used in client code and gain
                the extra safety on platforms which provide the functions.
 ------------------------------------------------------------------</Prolog>-*/

#include "prelude.h"                    /*  Universal header file            */
#include "sflmem.h"                     /*  Memory allocation functions      */
#include "sflprint.h"                   /*  Prototypes for functions         */

#define SAFETY_FACTOR  3                /*  How much bigger to require temp  */
                                        /*  buffer to be than expected length*/
#define BUFFER_LEN     2048             /*  Length of temporary buffer       */

static char 
    shared_buffer[BUFFER_LEN];          /*  Static buffer for printing into  */

#if (! defined (DOES_SNPRINTF))
/*  ---------------------------------------------------------------------[<]-
    Function: snprintf

    Synopsis: Writes formatted output into supplied string, up to a maximum
    supplied length.  This function is provided for systems which do not have
    a snprintf() function in their C library.  It uses a temporary buffer
    to print into, and providing that temporary buffer wasn't overflowed it
    copies the data up to the supplied length into the supplied buffer.  If
    the temporary buffer was overflowed it exits immediately.  (This is a
    poor man's snprintf(), but allows other code to use snprintf() and get
    the advantages of the better library implementations where available.)
    snprintf() is implemented in terms of vsnprintf().  (This function offers
    the C99 style interface, not the GNU style interface.)

    Returns:  number of characters output (or that needed to be output)

    Safety:   thread safe, buffer safe.

    Examples:

    char buffer [50];
    int  len;

    len = snprintf (buffer, sizeof(buffer), "Hello %s", "World");
    ---------------------------------------------------------------------[>]-*/

int snprintf  (char *str, size_t n, const char *format, ...)
{
    va_list ap;
    int     rc = 0;

    va_start (ap, format);
    rc = vsnprintf (str, n, format, ap);
    va_end   (ap);

    return rc;
}
#endif

#if (! defined (DOES_SNPRINTF))
/*  ---------------------------------------------------------------------[<]-
    Function: vsnprintf

    Synopsis: Writes formatted output into supplied string, up to a maximum
    supplied length, given a va_list of arguments with variables in it.  
    This function is provided for systems which do not have a vsnprintf() 
    function in their C library.  It uses a temporary buffer to print into, 
    and providing that temporary buffer wasn't overflowed it copies the data 
    up to the supplied length into the supplied buffer.  If the temporary 
    buffer was overflowed it exits immediately.  (This is a poor man's 
    snprintf(), but allows other code to use snprintf() and get the 
    advantages of the better library implementations where available.)
    (This function offers the C99 style interface, not the GNU style interface.)

    Returns:  number of characters output (or that needed to be output)

    Safety:   thread safe, buffer safe.

    Examples:

    char buffer[50];
    int len;
    va_list ap;

    va_start (ap, lastarg);
    len = vsnprintf (buffer, sizeof(buffer), "Hello %s", ap);
    va_end (ap);
    ---------------------------------------------------------------------[>]-*/

int vsnprintf (char *str, size_t n, const char *format, va_list ap)
{
    char *
        buffer  = shared_buffer;              /*  Temporary buffer to use    */
    int
        buflen  = sizeof(shared_buffer),      /*  Size of temporary buffer   */
        outputlen = 0;                        /*  Length of output           */
    Bool
        freebuf = FALSE;                      /*  If true, free buffer       */

    /*  Make sure we have a big enough temporary buffer                      */
    if (buflen < (SAFETY_FACTOR * n)) 
      {
        buflen = SAFETY_FACTOR * n;
        buffer = (char *)mem_alloc (buflen);

        ASSERT (buffer);
        if (! buffer) 
          {
            str [0] = '\0';
            return -1;
          }
        else
            freebuf = TRUE;
      }

    /*  Do vsprintf() into temporary buffer                                  */
    outputlen = vsprintf (buffer, format, ap);

    /*  Check to see if we overflowed our temporary buffer: panic if we did  */
    if (outputlen > buflen) 
      {
        /*  Oh, no, buffer overflow!  It's in a static buffer, or in the     */
        /*  heap, so it is harder to exploit, but either way the system      */
        /*  is in an uncertain state, so we give up immediately.             */

        ASSERT (outputlen <= buflen);            /*  To aid debugging        */
        write (2, "vsnprintf buffer overflow\n", 26);    /*  To stderr       */

        exit (EXIT_FAILURE);
        ASSERT (FALSE);                          /*  Unreachable             */
      }

    /*  Okay, everything looks reasonable.  Copy into real buffer now.       */
    strncpy (str, buffer, n);
    str [n - 1] = '\0';                          /*  NUL terminate string    */

    if (freebuf)
        mem_free (buffer);

    return (outputlen);
}
#endif


/*  ---------------------------------------------------------------------[<]-
    Function: strprintf

    Synopsis: Writes formatted output into a static string, up to a maximum
    of BUFFER_LEN characters.

    Returns:  a pointer to the string.

    Safety:   NOT thread safe (shared buffer), buffer safe.

    Examples:

    strcpy (buffer, strprintf ("Hello %s", "World"))
    ---------------------------------------------------------------------[>]-*/

char *
strprintf  (const char *format, ...)
{
    va_list 
        ap;

    va_start (ap, format);
    vsnprintf (shared_buffer, sizeof (shared_buffer), format, ap);
    shared_buffer [sizeof (shared_buffer) - 1] = '\0';
    va_end   (ap);

    return shared_buffer;
}
