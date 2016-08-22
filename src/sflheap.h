/*===========================================================================*
 *                                                                           *
 *  sflheap.h - Heap management functions                                    *
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
 *  --------------------------------------------------------------           *
 *===========================================================================*/
/*  ----------------------------------------------------------------<Prolog>-
    Synopsis:   Provides functions to handle a disk-based heap. Data can be 
                added to a heap with a unique key, and can be retrieved using 
                that key.
 ------------------------------------------------------------------</Prolog>-*/

#ifndef SFLHEAP_INCLUDED                /*  Allow multiple inclusions       */
#define SFLHEAP_INCLUDED


/*  Constants                                                               */

enum {
    HEAP_OK                  = 0, 
    HEAP_DATA_NOT_FOUND         ,
    HEAP_CANNOT_OPEN_FILE       ,
    HEAP_IO_FAILED              ,
    HEAP_MEMORY_ERROR           ,
    HEAP_INVALID_PATH           ,
    HEAP_CANNOT_CREATE_PATH     ,
    HEAP_DATA_ALREADY_EXISTS    ,
    HEAP_CANNOT_DELETE_DATA     ,
    HEAP_EMPTY_DATA             ,
    HEAP_RECOVERY_ERROR
};

/*  Function prototypes                                                     */

#ifdef __cplusplus
extern "C" {
#endif

int   heap_init    (const char *heappath);
int   heap_recover (const char *heappath);
int   heap_dispose (void);

int   heap_add    (const char  *key, const DESCR  *data);
int   heap_update (const char  *key, const DESCR  *data);
int   heap_remove (const char  *key);

Bool  heap_exists (const char  *key);
int   heap_get    (const char  *key,       DESCR **data);
int   heap_first  (      char **key,       DESCR **data);
int   heap_next   (      char **key,       DESCR **data);

int   heap_rename (const char *src_name, const char *src_key,
                   const char *dst_name, const char *dst_key);

#ifdef __cplusplus
}
#endif

#endif /* SFLHEAP_INCLUDED */
