/*===========================================================================*
 *                                                                           *
 *  ggobjt.c - Object functions                                              *
 *                                                                           *
 *  Copyright (c) 1996-2010 iMatix Corporation                               *
 *                                                                           *
 *  This program is free software; you can redistribute it and/or modify     *
 *  it under the terms of the GNU General Public License as published by     *
 *  the Free Software Foundation; either version 2 of the License, or (at    *
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

#include "ggpriv.h"                     /*  Project header file              */


/*- Types -------------------------------------------------------------------*/

typedef struct _GSL_OBJECT GSL_OBJECT;

struct _GSL_OBJECT {
    GSL_OBJECT        *next,
                      *prev;
    PLUGIN_INITIALISE *init;            /*  Called when thread starts        */
    function          shutdown;         /*  Called when GSL terminates       */
};

/*- Global variables --------------------------------------------------------*/

char
    object_error [LINE_MAX + 1];

/*- Global variables used in this source file only --------------------------*/

static LIST
    object_list = {&object_list, &object_list};

/*- Function prototypes -----------------------------------------------------*/

/*- Functions ---------------------------------------------------------------*/

void
initialise_objects (void)
{
    list_reset (& object_list);
}

void
destroy_objects (void)
{
    GSL_OBJECT
        *object = object_list. next;

    while ((void *) object != & object_list)
      {
        if (object-> shutdown)
            (void) (object-> shutdown) ();
        list_unlink (object);
        mem_free (object);
        object = object_list. next;
      }
}

int
object_register (PLUGIN_INITIALISE *init,
                 function          shutdown)
{
    GSL_OBJECT
        *object;

    list_create (object, sizeof (GSL_OBJECT));
    ASSERT (object);
    list_relink_before (object, & object_list);

    object-> init     = init;
    object-> shutdown = shutdown;

    return 0;
}


int
initialise_classes (THREAD *thread)
{
    GGCODE_TCB
        *tcb;
    GSL_OBJECT
        *object;
    CLASS_DESCRIPTOR
        *class;
    void
        *item;
    int
        rc;
    VALUE
        value;

    tcb = thread-> tcb;

    symb_class. create ("class", NULL, NULL,
                         & class, & item);
    tcb-> classes = item;

    FORLIST (object, object_list)
      {
        class = NULL;
        item  = NULL;
        rc = object-> init (& class, & item, thread);
        if (rc)
            return rc;
        if (class)
          {
            init_value (& value);
            value. type = TYPE_POINTER;
            value. c    = class;
            value. i    = item;
            symb_class. put_attr (tcb-> classes,
                                  class-> name ? class-> name : "$",
                                  & value,
                                  FALSE);
          }
      }
    return 0;
}


GSL_FUNCTION *
locate_method (CLASS_DESCRIPTOR *class,
               const char *name)
{
    int
        min = 0,
        max,
        chop = 0,
        cmp = -1;

    max = class-> method_cnt;

    /*  Check for case where object has one function to handle all methods  */
    if ((max == 1)
    &&  (! class-> methods [chop]. name))
      {
        chop = 0;
        cmp  = 0;
      }
    else
      {
        while (max > min)
          {
            chop = (max + min) / 2;
            cmp = strcmp (class-> methods [chop]. name, name);

            if (cmp < 0)
                min = chop + 1;
            else if (cmp > 0)
                max = chop;
            else
                break;
          }
      }

    if ((cmp != 0) && (min < class-> method_cnt))
      {
        chop = (max + min) / 2;
        cmp = strcmp (class-> methods [chop]. name, name);
      }
    if (cmp == 0)
        return & class-> methods [chop];
    else
        return NULL;
}


int
build_method_arguments (SCRIPT_NODE *fn_node,
                        RESULT_NODE ***arg)
{
    int
        count,
        n;
    SCRIPT_NODE
        *node;

    count = 0;
    node = fn_node;

    if (node)
        count++;

    while (node != NULL)
      {
        if ((node-> type     == GG_OPERATOR)
        &&  (node-> operator == OP_NEXT_ARG))
          {
            count++;
            node = node-> op1;
          }
        else
            break;
      }

    if (count)
      {
        *arg = mem_alloc (sizeof (RESULT_NODE *) * count);
        for (n = 0; n < count; n++)
            (*arg) [n] = NULL;
      }
    return count;
}


Bool
arguments_are_defined (int argc, RESULT_NODE **argv, RESULT_NODE *result)
{
    int
        i;

    for (i = 0; i < argc; i++)
        if (argv [i]
        &&  argv [i]-> value. type == TYPE_UNDEFINED)
          {
            result-> culprit = argv [i]-> culprit;
            argv [i]-> culprit = NULL;
            return FALSE;
          }
    return TRUE;
}


void *
get_class_item (THREAD *gsl_thread, const char *class)
{
    VALUE
        *value;

    value = symb_class. get_attr
                (((GGCODE_TCB *) gsl_thread-> tcb)-> classes,
                 class, FALSE);

    return value ? value-> i : NULL;
}
