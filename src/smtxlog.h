/*===========================================================================*
 *                                                                           *
 *  smtxlog.h - SMT extended logging agent                                   *
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

#ifndef _SMTXLOG_INCLUDED                /*  Allow multiple inclusions        */
#define _SMTXLOG_INCLUDED


/*- Constants ---------------------------------------------------------------*/

#define SMTXLOG_AGENT     "smtxlog"     /*  Xixlog agent name                */


/*  When to cycle the log file                                               */

#define XLOG_CYCLE_STARTUP  "startup"   /*  Cycle when starting the server   */
#define XLOG_CYCLE_HOURLY   "hourly"    /*  Cycle every hour at X minutes    */
#define XLOG_CYCLE_DAILY    "daily"     /*  Cycle every day at hh:mm         */
#define XLOG_CYCLE_WEEKLY   "weekly"    /*  Cycle every week at dd:hh:mm     */
#define XLOG_CYCLE_MONTHLY  "monthly"   /*  Cycle every month at dd:hh:mm    */
#define XLOG_CYCLE_SIZE     "size"      /*  Cycle when exceeds X Kb          */
#define XLOG_CYCLE_LINES    "lines"     /*  Cycle when exceeds X lines       */
#define XLOG_CYCLE_MANUAL   "manual"    /*  Do not cycle the log file        */

/*  How to cycle the log file                                                */

#define XLOG_CYCLE_RENAME   "rename"    /*  Rename old file                  */
#define XLOG_CYCLE_DELETE   "delete"    /*  Delete old file                  */
#define XLOG_CYCLE_MOVE     "move"      /*  Move old file to another path    */
#define XLOG_CYCLE_CONCAT   "concat"    /*  Concat old data to file          */
#define XLOG_CYCLE_PROCESS  "process"   /*  Execute some command on file     */

/*  Log file statistics, updated in each thread's TCB                        */

typedef struct {
    qbyte file_size;                    /*    Log file size, in bytes        */
    qbyte file_lines;                   /*    Log file size, in lines        */
} XLOG_STATS;


/*- Function prototypes -----------------------------------------------------*/

#ifdef __cplusplus
extern "C" {
#endif

int  smtxlog_init (void);
void smtxlog_log  (QID *to, char *log_file, SYMTAB *table);

#ifdef __cplusplus
}

#endif

#endif
