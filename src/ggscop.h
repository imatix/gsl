/*===========================================================================*
 *                                                                           *
 *  ggscop.h - Scope functions                                               *
 *                                                                           *
 *  Copyright (c) 1996-2010 iMatix Corporation                               *
 *                                                                           *
 *  This program is free software; you can redistribute it and/or modify     *
 *  it under the terms of the GNU General Public License as published by     *
 *  the Free Software Foundation; either version 3 of the License, or (at    *
 *  your option) any later version.                                          *
 *                                                                           *
 *  This program is distributed in the hope that it will be useful, but      *
 *  WITHOUT ANY WARRANTY; without even the implied warranty of               *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU        *
 *  General Public License for more details.                                 *
 *                                                                           *
 *  For information on alternative licensing for OEMs, please contact        *
 *  iMatix Corporation.                                                      *
 *                                                                           *
 *===========================================================================*/

#ifndef GGSCOP_INCLUDED                 /*  Allow multiple inclusions        */
#define GGSCOP_INCLUDED

/*- Type definitions --------------------------------------------------------*/

typedef struct _SCOPE_ITEM {
    struct _SCOPE_ITEM
        *next,
        *prev;
    CLASS_DESCRIPTOR
        *class;
    void
        *item;                          /*  Data for this item               */
    long
        item_num;                       /*  Item number                      */
    VALUE
        sort_key;
} SCOPE_ITEM;

typedef struct _SCOPE_BLOCK {
    struct _SCOPE_BLOCK
        *next,
        *prev;
    SCRIPT_NODE_TYPE
        scope_type;
    char
        *name;                          /*  Scope name                       */
    LIST
        item_list;                      /*  List of items to iterate through */
    SCOPE_ITEM
        *scope_item;                    /*  Current item in for list         */
    CLASS_DESCRIPTOR
        *class;
    void
        *item;                          /*  Data for this item               */
    long
        total,                          /*  Total number of items in loop    */
        index;                          /*  Loop index                       */
    Bool
        stacked;                        /*  Is scope 'stacked'?              */
    MACRO_TABLE_ITEM
        *macros;                        /*  Spaces for macros in this block  */
} SCOPE_BLOCK;

/*- Functions ---------------------------------------------------------------*/

SCOPE_BLOCK *lookup_simple_scope        (LIST  *scope_stack,
                                         VALUE *scope_value,
                                         Bool  ignorecase,
                                         CLASS_DESCRIPTOR **class,
                                         void **item,
                                         char  **error_text);
int          symbol_item                (LIST *scope_stack,
                                         RESULT_NODE *symbol,
                                         Bool ignorecase,
                                         CLASS_DESCRIPTOR **class,
                                         void **item,
                                         char **error_text);
VALUE       *symbol_value               (LIST *scope_stack,
                                         RESULT_NODE *symbol,
                                         Bool ignorecase,
                                         char **error_text);
Bool         store_symbol_definition    (LIST *scope_stack,
                                         Bool  ignorecase,
                                         RESULT_NODE *symbol,
                                         VALUE *value,
                                         char **error_text);

SCOPE_BLOCK *create_scope_block         (LIST *scope_stack,
                                         SCRIPT_NODE_TYPE scope_type,
                                         const char *alias,
                                         CLASS_DESCRIPTOR *class);
void         destroy_scope_block        (SCOPE_BLOCK *scope_block);
SCOPE_BLOCK *first_scope_block          (LIST *scope_stack);
SCOPE_BLOCK *last_scope_block           (LIST *scope_stack);

SCOPE_ITEM  *create_scope_item          (SCOPE_BLOCK *scope_block,
                                         CLASS_DESCRIPTOR *class,
                                         void *item,
                                         long item_num);
void         destroy_scope_item         (SCOPE_ITEM *scope_item);

Bool         first_scope_item           (SCOPE_BLOCK *scope_block);
Bool         next_scope_item            (SCOPE_BLOCK *scope_block);

void         copy_scope_stack           (LIST *to,
                                         LIST *from);

#endif
