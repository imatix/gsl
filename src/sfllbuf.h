/*===========================================================================*
 *                                                                           *
 *  sfllbuf.h - Line buffering routines                                      *
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
    Synopsis:   Provides circular line buffering functions.  A line buffer
                is a data structure that holds a fixed amount of data in a
                serial fashion; the oldest data gets discarded as new data
                is added.
 ------------------------------------------------------------------</Prolog>-*/

#ifndef SFLLBUF_INCLUDED               /*  Allow multiple inclusions        */
#define SFLLBUF_INCLUDED


/*- Type definitions --------------------------------------------------------*/

typedef struct {
    char  *data;                        /*  Buffer contents                  */
    char  *head;                        /*  Where we add new data            */
    char  *tail;                        /*  Oldest data is here              */
    size_t size;                        /*  Actual size of buffer            */
    char  *top;                         /*  Address of top of buffer         */
} LINEBUF;                              /*  Empty when tail == head          */


/*- Function Prototypes -----------------------------------------------------*/

#ifdef __cplusplus
extern "C" {
#endif

LINEBUF *linebuf_create  (size_t maxsize);
void     linebuf_destroy (LINEBUF *buffer);
void     linebuf_reset   (LINEBUF *buffer);
void     linebuf_append  (LINEBUF *buffer, const char *line);
char    *linebuf_first   (LINEBUF *buffer, DESCR *line);
char    *linebuf_next    (LINEBUF *buffer, DESCR *line, const char *cur);
char    *linebuf_last    (LINEBUF *buffer, DESCR *line);
char    *linebuf_prev    (LINEBUF *buffer, DESCR *line, const char *cur);

#ifdef __cplusplus
}
#endif

#endif
