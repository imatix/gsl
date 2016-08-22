/*===========================================================================*
 *                                                                           *
 *  sflnode.h - Linked list functions, deprecated                            *
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
    Synopsis:   Provides functions to maintain doubly-linked lists.  You can
                use these functions to work with lists of any structure.  To
                make this work, all structures must start with two pointers,
                "void *next, *prev;".  When you want to attach a linked-list
                to another structure, declare the list head as a NODE.  You
                can then refer to this variable when you attach items to the
                list head.  The code sets the global node_unsafe to TRUE
                whenever it is changing a list.  NOTE: DEPRECATED IN FAVOUR
                OF SFLLIST.C.
 ------------------------------------------------------------------</Prolog>-*/

#ifndef SFLNODE_INCLUDED               /*  Allow multiple inclusions        */
#define SFLNODE_INCLUDED


/*  The node descriptor simply contains two pointers.  All blocks that are   */
/*  descriptors that are held in lists.  We can (a) allocate a dummy node    */
/*  instead of a complete block for a list head, and (b) use the same list   */
/*  handling functions for all descriptors.                                  */

typedef struct {                        /*  Node descriptor                  */
    void *next, *prev;                  /*    for a doubly-linked list       */
} NODE;

/*  Global variables                                                         */

extern Bool
    node_unsafe;                        /*  TRUE if we're changing a list    */

/*  Function prototypes                                                      */

#ifdef __cplusplus
extern "C" {
#endif

void *node_create        (void *after, size_t size);
void  node_destroy       (void *node);
void *node_unlink        (void *node);
void *node_relink        (void *left, void *node, void *right);
void *node_relink_after  (void *node, void *after);
void *node_relink_before (void *node, void *before);

#ifdef __cplusplus
}
#endif

/*  Macros                                                                   */

#define node_reset(node)          (node)-> prev = (node)-> next = (node)

#endif
