/*===========================================================================*
 *                                                                           *
 *  sfluid.h - User and group ID functions                                   *
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
    Synopsis:   Provides functions to access user and group id names and
                manage the current real/effective uid's and gid's for a
                process.  These functions are only meaningful on UNIX
                systems, and partially on VMS systems, but may be used by
                portable programs that must operate under UNIX as well as
                other environments.  Some uid functions are non-portable
                between UNIX systems; this package provides a single API.
                Changes for OS/2 were done by Ewen McNeill <ewen@naos.co.nz>.
 ------------------------------------------------------------------</Prolog>-*/

#ifndef SFLUID_INCLUDED                /*  Allow multiple inclusions        */
#define SFLUID_INCLUDED

/*  Function prototypes                                                      */

#ifdef __cplusplus
extern "C" {
#endif

char *get_uid_name  (uid_t uid);
char *get_gid_name  (gid_t gid);
int   set_uid_user  (void);
int   set_uid_root  (void);
int   set_gid_user  (void);
int   set_gid_root  (void);
int   set_uid_gid   (char *new_uid, char *new_gid);
char *get_login     (void);


#ifdef __cplusplus
}
#endif

#endif
