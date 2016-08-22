/*===========================================================================*
 *  sysman.h - sysman constants                                              *
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
 *  published by the Free Software Foundation; either version 3 of           *
 *  the License, or (at your option) any later version.                      *
 *                                                                           *
 *  This program is distributed in the hope that it will be useful,          *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of           *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            *
 *  GNU General Public License for more details.                             *
 *                                                                           *
 *  You should have received a copy of the GNU General Public                *
 *  License along with this program in the file 'license.gpl'; if            *
 *  not, see <http://www.gnu.org/licenses/>.                                 *
 *                                                                           *
 *  You can also license this software under iMatix's General Terms          *
 *  of Business (GTB) for commercial projects.  If you have not              *
 *  explicitly licensed this software under the iMatix GTB you may           *
 *  only use it under the terms of the GNU General Public License.           *
 *                                                                           *
 *  For more information, send an email to info@imatix.com.                  *
 *===========================================================================*/

#ifndef _SYSMAN_INCLUDED                /*  Allow multiple inclusions        */
#define _SYSMAN_INCLUDED


/*---------------------------------------------------------------------------
 *  Function prototypes
 */

#ifdef __cplusplus
extern "C" {
#endif

int  sysmana_init (char *port);
int  sysclia_init (char *command, char *port);

#ifdef __cplusplus
}
#endif

/*---------------------------------------------------------------------------
 *  Constants
 */

#define SYSMAN_VERSION   "2.3"          /*  Daemon version                   */
#define SYSMAN_DEFAULT_PORT  "5050"     /*  Default port to listen on        */
#define SYSCLI_VERSION   "1.3"          /*  Client version                   */

/*  Messages (EXDR formatted events)                                         */

#define SYSMAN_MESSAGE  "ds"
/*  d=message identifier                                                     */
/*  s=message string                                                         */

#define SYSMAN_LIST         100         /*  List processes                   */
#define SYSMAN_HALT         101         /*  Halt SYSMAN and processes        */
#define SYSMAN_START        102         /*  Start-up a process               */
#define SYSMAN_PAUSE        103         /*  Pause a process                  */
#define SYSMAN_STOP         104         /*  Shut-down a process              */
#define SYSMAN_STATUS       105         /*  Enquire process status           */
#define SYSMAN_REFRESH      106         /*  Reloads SYSMAN.ini               */
#define SYSMAN_READY        200         /*  Reply ready for command          */
#define SYSMAN_ERROR        201         /*  Reply fatal error                */
#define SYSMAN_HALTING      202         /*  Reply halting SYSMAN             */
#define SYSMAN_TASK_ID      203         /*  Reply task name                  */
#define SYSMAN_TASK_NF      204         /*  Reply unknown task               */
#define SYSMAN_TASK_RUNNING 205         /*  Reply task is running            */
#define SYSMAN_TASK_PAUSED  206         /*  Reply task is running            */
#define SYSMAN_TASK_STOPPED 207         /*  Reply task is not running        */
#define SYSMAN_START_OK     208         /*  Reply task started ok            */
#define SYSMAN_START_ERROR  209         /*  Reply task could not be started  */
#define SYSMAN_PAUSE_OK     210         /*  Reply task paused ok             */
#define SYSMAN_PAUSE_ERROR  211         /*  Reply task could not be paused   */
#define SYSMAN_STOP_OK      212         /*  Reply task stopped ok            */
#define SYSMAN_STOP_ERROR   213         /*  Reply task could not be stopped  */

#endif                                  /*  Include sysman.h                 */
