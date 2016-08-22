/*===========================================================================*
 *                                                                           *
 *  sfltree.h - Binary trees                                                 *
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
    Synopsis:   Provides functions to maintain 'Red-Black' balanced binary
                trees.  You can use these functions to work with trees of any
                structure.  To make this work, all structures must start with
                the following: "void *left, *right, *parent; TREE_COLOUR
                colour;".  All trees need a pointer to the root of type TREE
                which should be initialised with tree_init - you can test
                whether a tree is empty by comparing its root with TREE_NULL.
                The order of nodes in the tree is determined by calling a
                node comparison function provided by the caller - this
                accepts two node pointers  and returns zero if the two nodes
                are equal, -1 if the first is smaller and 1 if the first is
                larger.
 ------------------------------------------------------------------</Prolog>-*/

#ifndef SFLTREE_INCLUDED               /*  Allow multiple inclusions        */
#define SFLTREE_INCLUDED


/* Red-Black tree description */

typedef enum {BLACK, RED} TREE_COLOUR;

/*  Node descriptor                                                          */

typedef struct _TREE {
    struct _TREE
        *left, *right, *parent;
    TREE_COLOUR
         colour;
} TREE;

/*  The tree algorithm needs to know how to sort the data.  It does this     */
/*  using a functions provided by the calling program.                       */

typedef int (TREE_COMPARE) (void *t1, void *t2);

/*  Define a function type for use with the tree traversal function          */

typedef void (TREE_PROCESS) (void *t);

/*  Global variables                                                         */

extern TREE
    TREE_EMPTY;

/*  Function prototypes                                                      */

#ifdef __cplusplus
extern "C" {
#endif

void  tree_init     (TREE **root);
int   tree_insert   (TREE **root, void *tree, TREE_COMPARE *comp,
                     Bool allow_duplicates);
void  tree_delete   (TREE **root, void *tree);
void *tree_find_eq  (TREE **root, void *tree, TREE_COMPARE *comp);
void *tree_find_lt  (TREE **root, void *tree, TREE_COMPARE *comp);
void *tree_find_le  (TREE **root, void *tree, TREE_COMPARE *comp);
void *tree_find_gt  (TREE **root, void *tree, TREE_COMPARE *comp);
void *tree_find_ge  (TREE **root, void *tree, TREE_COMPARE *comp);
void  tree_traverse (void *tree, TREE_PROCESS *process, int method);
void *tree_next     (void *tree);
void *tree_prev     (void *tree);
void *tree_first    (void *tree);
void *tree_last     (void *tree);

#ifdef __cplusplus
}
#endif

/*  Return codes                                                             */

#define TREE_DUPLICATE -1
#define TREE_OK         0

/*  Macros                                                                   */

#define TREE_NULL &TREE_EMPTY

#endif

