/*===========================================================================*
 *                                                                           *
 *  ggcode.h - Code generator functions                                      *
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

#ifndef GGCODE_INCLUDED                /*  Allow multiple inclusions         */
#define GGCODE_INCLUDED

/*---------------------------------------------------------------------------*/

typedef void (OUTPUT_FCT) (JOBID, const char *);
typedef void (HANDLER_FCT) (void);

typedef struct                          /*  Thread context block:            */
{
    THREAD
        *thread;
    MEMTRN
        *scratch_memtrn;
    JOBID
        job;
    GSL_CONTROL
        *gsl;                           /*  GSL internal data                */
    SYMTAB_ITEM
        *classes;                       /*  Table of predefined classes      */
    CLASS_ITEM
        *initial;                       /*  Array of initial objects         */
    int
        initial_cnt;                    /*  Number of initial objects        */
    QUEUE
        *replyq;
    int
        execute_level;
    char
        *script_name,
        *script_text;
    SCRIPT_HANDLE
        *script;                        /*  Careful - can be dangling        */
    FILE
        *output;
    SCRIPT_NODE
        *script_root,
        *script_node,
        *evaluate_node,
        *operand_node,
        *fake_for_node;
    RESULT_NODE
       **result_ptr,
        *result_root,
        *result_node,
       *output_buffer;
    RESULT_NODE
        call_result;
    Bool
        stdout_echo,                   /*  Copy to stdout                   */
        execute_full,
        stepping,
        error_occurred;
    LIST
        script_stack,
        scope_stack;
    RESULT_NODE
        *node_stack;
    OUTPUT_FCT
        *output_fct;                    /*  Redirector function              */
    DATA_TYPE
        sort_type;
    void
        *item;
} GGCODE_TCB;


/*- Function prototypes -----------------------------------------------------*/

int     gsl_init         (long size);

THREAD *gsl_execute      (QUEUE    *replyqueue,
                          JOBID     job,
                          SYMTAB   *switches,
                          int       count,
                          /* CLASS_ITEM *item */ ...);
THREAD *gsl_start        (QUEUE    *replyqueue,
                          JOBID     job,
                          SYMTAB   *switches,
                          int       count,
                          /* CLASS_ITEM *item */ ...);
THREAD *gsl_copy         (THREAD   *old_thread,
                          QUEUE    *replyqueue,
                          JOBID     job);
void    gsl_continue     (THREAD   *gsl_thread,
                          Bool      terminate,
                          QUEUE    *replyqueue);
void    gsl_next         (THREAD   *gsl_thread,
                          QUEUE    *replyqueue);
void    gsl_command      (THREAD   *gsl_thread,
                          char     *command,
                          Bool      terminate,
                          QUEUE    *replyqueue);
int     gsl_function     (THREAD   *gsl_thread,
                          QUEUE    *replyqueue,
                          const char *function,
                          int       parm_count,
                          VALUE    *parm_value[]);
int     gsl_evaluate     (THREAD      *gsl_thread,
                          char        *expression,
                          Bool        terminate,
                          RESULT_NODE **result,
                          QUEUE       *replyqueue);
THREAD *gsl_spawn        (THREAD   *gsl_thread,
                          QUEUE    *replyqueue,
                          SCRIPT_HANDLE *script_handle);
void    gsl_finish       (THREAD   *gsl_thread);

char   *gsl_cur_script   (THREAD *gsl_thread);
int     gsl_cur_line     (THREAD *gsl_thread);
char   *gsl_cur_text     (THREAD *gsl_thread);

int  gsl_term         (void);

void gg_send_output   (THREAD *gsl_thread, OUTPUT_FCT *output_fct, Bool echo);
void gg_set_handler   (HANDLER_FCT *handler_fct, int event);

/*  These are the events that we can provide handlers for                    */

typedef enum {
    EVENT_ABORT
} GG_EVENT;


/*---------------------------------------------------------------------------*/

#endif
