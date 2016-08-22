/*===========================================================================*
 *                                                                           *
 *  sfllist.h - Doubly-linked lists                                          *
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
                to another structure, declare the list head as a list.  You
                can then refer to this variable when you attach items to the
                list head.  The code sets the global list_unsafe to TRUE
                whenever it is changing a list.
 ------------------------------------------------------------------</Prolog>-*/

#ifndef SFLLIST_INCLUDED               /*  Allow multiple inclusions        */
#define SFLLIST_INCLUDED

/*- Types -------------------------------------------------------------------*/

/*  The list descriptor simply contains two pointers.  All blocks that are   */
/*  descriptors that are held in lists.  We can (a) allocate a dummy list    */
/*  instead of a complete block for a list head, and (b) use the same list   */
/*  handling functions for all descriptors.                                  */

typedef struct {                        /*  list descriptor                  */
    void *next, *prev;                  /*    for a doubly-linked list       */
} LIST;

/*  Function type for comparing nodes.  A function of this type is passed    */
/*  to the list sorting functions which use it to compare nodes.  A function */
/*  of this type should return TRUE iff the two nodes need swapping.         */

typedef Bool (*NODE_COMPARE) (LIST *t1, LIST *t2);

/*  Global variables                                                         */

#ifdef __cplusplus
extern "C" {
#endif

extern Bool
    list_unsafe;                        /*  TRUE if we're changing a list    */

/*- Function prototypes -----------------------------------------------------*/

void *list_relink (void *left, void *list, void *right);
void *list_unlink (            void *list             );
void *list_attach (LIST *list, void *data, size_t size);
void  list_remove (LIST *list, void *data, size_t size);
void  list_sort   (void *list, NODE_COMPARE comp);

#ifdef __cplusplus
}
#endif


/*  -------------------------------------------------------------------------
    Macro: list_relink_after

    Synopsis: Links a list into a doubly-linked list after a point in the
    list.  Generally a linked list is attached to a 'head': an empty list
    consists of just the head list.  To attach a list to the start of the
    list, link after the head.  To attach a list to the end of the list,
    link before the head using list_relink_before().  In this way you can
    build doubly-ended queues, fifo queue, lists, etc.  Returns the address
    of the list.
    -------------------------------------------------------------------------*/

#define list_relink_after(l,a) (list_relink (a, l, ((LIST *) a)-> next))


/*  -------------------------------------------------------------------------
    Macro: list_relink_before

    Synopsis: Links a list into a doubly-linked list before a point in the
    list.  To link a list to the end of a doubly-linked list, link it before
    the list header list.
    -------------------------------------------------------------------------*/

#define list_relink_before(l,b) (list_relink (((LIST *) b)-> prev, l, b))


/*  Other macros                                                             */

#define list_reset(list)        (list)-> prev = (list)-> next = (list)
#define list_empty(list)        ((list)-> prev == (list))
#define list_create(node,size)  if (((node) = mem_alloc (size)) != NULL) \
                                    list_reset (node)

/*  Macro to do all nodes on a linked list                                   */

#define FORLIST(node,root)      for ((node) = (root).next;       \
                                     (void *) (node) != &(root); \
                                     (node) = (node)-> next)

/*  Macros to use lists as stacks and queues                                 */

#define list_push(list,item)    list_attach (list,                            \
                                            &item,                            \
                                            sizeof (item))

#define list_queue(list,item)   list_attach (((LIST *)list)-> prev,           \
                                            &item,                            \
                                            sizeof (item))

#define list_pop(list,item)     list_remove (list,                            \
                                             &item,                           \
                                             sizeof (item))

#define list_destroy(list)      while (!list_empty(list))                     \
                                  {                                           \
                                    LIST *item = ((LIST *)list)-> next;       \
                                    list_unlink (item);                       \
                                    mem_free (item);                          \
                                  }
#endif
