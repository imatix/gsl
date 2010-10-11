/*===========================================================================*
 *                                                                           *
 *  sflcons.h - Console output functions                                     *
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
    Synopsis:   Provides redirectable console output: use the coprintf() and
                coputs() calls instead of printf() and puts() in a real-time
                application.  Then, you can call console_send() to send all
                console output to a specified function.  This is a useful
                way to get output into -- for example -- a GUI window.
 ------------------------------------------------------------------</Prolog>-*/

#ifndef SFLCONS_INCLUDED               /*  Allow multiple inclusions        */
#define SFLCONS_INCLUDED


/*  Type definition for operator redirection function                        */

typedef void (CONSOLE_FCT)  (const char *);

/*  Function prototypes                                                      */

#ifdef __cplusplus
extern "C" {
#endif

void  console_send        (CONSOLE_FCT  *console_fct, Bool echo);
void  console_enable      (void);
void  console_disable     (void);
void  console_set_mode    (int CONSOLE_MODE);
int   console_capture     (const char *filename, char mode);
int   coputs              (const char *string);
int   coprintf            (const char *format, ...);
int   coputc              (int character);

#ifdef __cplusplus
}
#endif


/*  Constant definitions                                                     */

enum {
    CONSOLE_PLAIN,                      /*  Print as requested               */
    CONSOLE_DATETIME,                   /*  Prefix with date and time        */
    CONSOLE_TIME,                       /*  Prefix with time only            */
    CONSOLE_DEBUG                       /*  Datetime, fully flushed to disk  */
};

#endif
