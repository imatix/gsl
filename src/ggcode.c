/*===========================================================================*
 *                                                                           *
 *  ggcode.c - Code generator functions                                      *
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


/*- Type definitions --------------------------------------------------------*/

typedef GGCODE_TCB TCB;

/*- Function prototypes -----------------------------------------------------*/

static THREAD   *create_gsl_thread              (QUEUE    *replyqueue,
                                                 JOBID    job);
static void      initialise_gsl_thread          (THREAD *thread);
static void      destroy_gsl_thread             (THREAD *thread);
static void      build_initial                  (THREAD *gsl_thread,
                                                 int     count,
                                                 va_list ap);
static void      initialise_predefined_scopes   (THREAD *thread);
static void      prepare_gsl_control_block      (THREAD *thread,
                                                 SYMTAB *switches);
static VALUE    *switch_value                   (SYMTAB *switches,
                                                 char *name);
static void      copy_gsl_control_block         (GSL_CONTROL *to,
                                                 GSL_CONTROL *from);
static void      report_error                   (event_t event,
                                                 char *format, ...);
static void      report_undefined_expression    (RESULT_NODE *node);
static void      must_be_defined_and_linear     (RESULT_NODE *node);
static void      open_output_file               (char *mode);
static void      create_scope_item_from_item    (SCOPE_BLOCK *scope_block);
static void      must_be_defined                (RESULT_NODE *node);
static int       parse_locator                  (CLASS_DESCRIPTOR **parent_class,
                                                 void             **parent_item,
                                                 CLASS_DESCRIPTOR **sibling_class,
                                                 void             **sibling_item,
                                                 THREAD *thread);
static void      lookup_innermost_item          (CLASS_DESCRIPTOR **class,
                                                 void             **item);
static Bool      sort_compare                   (LIST *t1, LIST *t2);
static int       substitute_params_into_symb    (void *void_item,
                                                 SCRIPT_NODE *args,
                                                 RESULT_NODE *params,
                                                 THREAD *thread);
static void      inherit_culprit                (RESULT_NODE *node);
static void      output_the_line                (char *line,
                                                 Bool cobol);
static void      send_text_to_output            (char *text);
static SCRIPT_NODE_TYPE
                 go_to_top_of_loop              (SCRIPT_LINE **script_line);
static SCRIPT_NODE_TYPE
                 block_end_type                 (SCRIPT_LINE *script_line);
static void      report_unaliased_unstacked_scope
                                                (void);
static void      pop_the_script                 (THREAD *thread);
static void      get_the_parameter              (void);
static void      report_non_numeric_error       (RESULT_NODE *node);
static void      pop_the_result_node            (void);
static void      lookup_the_item                (RESULT_NODE *item_node);
static void      must_be_pointer                (RESULT_NODE *item_node);

/*- Definitions -------------------------------------------------------------*/

#define AGENT_NAME      "GGCODE"        /*  Our public name                  */

/*- Global variables used in this source file only --------------------------*/

static GGCODE_TCB
    *tcb;                               /*  Address thread context block     */

static QID
    operq;                              /*  Operator console event queue     */

static MEMTRN
    *global_memtrn;                     /*  Global memory allocations.       */

static HANDLER_FCT
    *abort_fct = NULL;                  /*  Abort handler function           */

#include "ggcode.d"                     /*  Include dialog data              */
#include "ggfunc.inc"                   /*  Include inbuilt functions        */

event_t type_event [] = {
    comment_event,        /*  GG_COMMENT        */
    line_event,           /*  GG_LINE           */
    text_event,           /*  GG_TEXT           */
    substitute_event,     /*  GG_SUBSTITUTE     */
    operator_event,       /*  GG_OPERATOR       */
    literal_event,        /*  GG_LITERAL        */
    number_event,         /*  GG_NUMBER         */
    symbol_event,         /*  GG_SYMBOL         */
    member_event,         /*  GG_MEMBER         */
    attribute_event,      /*  GG_ATTRIBUTE      */
    call_event,           /*  GG_CALL           */
    output_event,         /*  GG_OUTPUT         */
    append_event,         /*  GG_APPEND         */
    close_event,          /*  GG_CLOSE          */
    if_event,             /*  GG_IF             */
    elsif_event,          /*  GG_ELSIF          */
    else_event,           /*  GG_ELSE           */
    end_if_event,         /*  GG_END_IF         */
    for_event,            /*  GG_FOR            */
    end_for_event,        /*  GG_END_FOR        */
    scope_event,          /*  GG_SCOPE          */
    end_scope_event,      /*  GG_END_SCOPE      */
    new_event,            /*  GG_NEW            */
    end_new_event,        /*  GG_END_NEW        */
    delete_event,         /*  GG_DELETE         */
    move_event,           /*  GG_MOVE           */
    copy_event,           /*  GG_COPY           */
    while_event,          /*  GG_WHILE          */
    end_while_event,      /*  GG_END_WHILE      */
    next_event,           /*  GG_NEXT           */
    last_event,           /*  GG_LAST           */
    macro_event,          /*  GG_MACRO          */
    end_macro_event,      /*  GG_END_MACRO      */
    function_event,       /*  GG_FUNCTION       */
    end_function_event,   /*  GG_END_FUNCTION   */
    return_event,         /*  GG_RETURN         */
    gsl_event,            /*  GG_GSL            */
    direct_event,         /*  GG_DIRECT         */
    xml_event,            /*  GG_XML            */
    template_event,       /*  GG_TEMPLATE       */
    end_template_event,   /*  GG_END_TEMPLATE   */
    echo_event,           /*  GG_ECHO           */
    abort_event,          /*  GG_ABORT          */
    define_event,         /*  GG_DEFINE         */
    save_event,           /*  GG_SAVE           */
    sort_event,           /*  GG_SORT           */
    _LR_NULL_EVENT };     /*  GG_UNDEFINED      */


/********************   INITIALISE AGENT - ENTRY POINT    ********************/

/*  ---------------------------------------------------------------------[<]-
    Function: gsl_init

    Synopsis: Initialises the GSLGen interpreter agent.  Returns 0 if
    initialised okay, -1 if there was an error.
    Supports these public methods:
    <Table>
    EXECUTE     Execute a script in its entirety (including initialise
                & terminate).
    INITIALISE  Prepare to execute a script
    NEXT        Execute one line of a script.
    GSL         Execute a GSL command
    FINISH      After executing a script.
    </Table>
    ---------------------------------------------------------------------[>]-*/

int
gsl_init (long size)
{
    AGENT
       *agent;                          /*  Handle for our agent             */
    THREAD
       *thread;                         /*  Handle to operator thread        */

#   include "ggcode.i"                  /*  Include dialog interpreter       */

    /*  Shutdown event comes from Kernel                                     */
    declare_smtlib_shutdown   (shutdown_event, SMT_PRIORITY_MAX);

    /*  Public methods supported by this agent                               */
    declare_ggcode_execute      (execute_event,      0);
    declare_ggcode_start        (start_event,        0);
    declare_ggcode_spawn        (spawn_event,        0);
    declare_ggcode_continue     (continue_event,     0);
    declare_ggcode_evaluate     (evaluate_event,     0);
    declare_ggcode_next         (next_event,         0);
    declare_ggcode_gsl          (gsl_event,          0);
    declare_ggcode_call         (call_event,         0);
    declare_ggcode_finish       (finish_event,       0);
    declare_ggcode_call_ok      (call_ok_event,      0);
    declare_ggcode_call_error   (call_error_event,   0);
    declare_ggcode_call_message (call_message_event, 0);

    /*  Reply messages from parser agent                                     */
    declare_ggpars_ok    (ok_event,    0);
    declare_ggpars_error (error_event, 0);

    /*  Ensure that operator console is running, else start it up            */
    smtoper_init ();
    if ((thread = thread_lookup (SMT_OPERATOR, "")) != NULL)
        operq = thread-> queue-> qid;
    else
        return (-1);

    /*  Prepare object manager and inbuilt functions  */
    initialise_objects ();
    if (register_inbuilt_functions ())
        return (-1);

    global_memtrn = mem_new_trans ();

    max_size = size;

    /*  Signal okay to caller that we initialised okay                       */
    return (0);
}


THREAD *
gsl_execute (QUEUE    *replyqueue,
             JOBID     job,
             SYMTAB   *switches,
             int       count,
             /* CLASS_ITEM *item */ ...)
{
    va_list
        ap;
    THREAD
       *thread;                         /*  Handle to GSL thread             */
    MEMTRN
        *save_memtrn;

    thread = create_gsl_thread (replyqueue,
                                job);

    /*  Memory allocations should be attached to gsl thread.  */
    save_memtrn = smt_memtrn_;
    smt_memtrn_ = thread-> memtrn;

    va_start (ap, count);
    build_initial (thread,
                   count,
                   ap);
    va_end (ap);

    initialise_classes (thread);
    initialise_predefined_scopes (thread);
    tcb-> script_name = mem_strdup (string_value (switch_value (switches, "script")));
    prepare_gsl_control_block (thread, switches);

    lsend_ggcode_execute (& thread-> queue-> qid,
                          replyqueue ? & replyqueue-> qid : NULL,
                          NULL, NULL, NULL, 0);

    /*  And restore old memory transaction.  */
    smt_memtrn_ = save_memtrn;

    return thread;
}


static THREAD *
create_gsl_thread (QUEUE    *replyqueue,
                   JOBID    job)
{
    THREAD
        *thread;                    /*  Handle to GSL thread             */
    MEMTRN
        *save_memtrn;

    thread = thread_create (AGENT_NAME, "");
    if (! thread)
        return NULL;

    initialise_gsl_thread (thread);

    /*  Memory allocations should be attached to gsl thread.  */
    save_memtrn = smt_memtrn_;
    smt_memtrn_ = thread-> memtrn;

    tcb = thread-> tcb;         /*  Point to thread's context        */

    tcb-> replyq         = replyqueue;
    tcb-> job            = job;
    tcb-> execute_full   = TRUE;
    tcb-> initial_cnt    = 0;
    tcb-> initial        = NULL;

    /*  And restore old memory transaction.  */
    smt_memtrn_ = save_memtrn;

    return thread;
}


static void
initialise_gsl_thread (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    thread-> atdestroy = destroy_gsl_thread;
    thread-> animate   = TRUE;

    /*  Initialise TCB stuff.                                                */
    tcb-> scratch_memtrn = mem_new_trans ();

    tcb-> thread          = thread;
    tcb-> execute_level   = 0;

    tcb-> script_name     = NULL;
    tcb-> script_text     = NULL;
    tcb-> script          = NULL;

    tcb-> output          = NULL;

    tcb-> script_root     = NULL;
    tcb-> script_node     = NULL;
    tcb-> evaluate_node   = NULL;
    tcb-> operand_node    = NULL;
    tcb-> fake_for_node   = NULL;

    tcb-> result_ptr      = NULL;
    tcb-> result_root     = NULL;
    tcb-> result_node     = NULL;
    tcb-> output_buffer   = NULL;

    tcb-> stdout_echo = TRUE;
    tcb-> stepping    = FALSE;

    list_reset (& tcb-> script_stack);
    list_reset (& tcb-> scope_stack);
    tcb-> node_stack = NULL;

    tcb-> output_fct      = NULL;

    tcb-> item = NULL;
}


static void
destroy_gsl_thread (THREAD *thread)
{
    int
        i;
    CLASS_ITEM
        class_item;

    tcb = thread-> tcb;                 /*  Point to thread's context        */

    mem_rollback (tcb-> scratch_memtrn);

    for (i = 0; i < tcb-> initial_cnt; i++)
      {
        class_item =  tcb-> initial [i];
        class_item. class-> destroy (class_item. item);
      }
    mem_free (tcb-> initial);

    destroy_result (tcb-> output_buffer);

    if (tcb-> output)
        file_close (tcb-> output);

    while (! list_empty (& tcb-> script_stack))
        list_pop (& tcb-> script_stack, tcb-> gsl-> line);

    if (tcb-> fake_for_node)
        mem_free (tcb-> fake_for_node);

    while (tcb-> node_stack)
      {
        tcb-> result_node = tcb-> node_stack;
        tcb-> node_stack  = tcb-> node_stack-> next;
        tcb-> result_node-> next = NULL;

        if (tcb-> result_node && (tcb-> result_node != tcb-> result_root))
            destroy_result (tcb-> result_node);
      }

    if (tcb-> result_root)
        destroy_result (tcb-> result_root);

    while (!list_empty (&tcb-> scope_stack))
        destroy_scope_block (tcb-> scope_stack. next);

    mem_strfree (& tcb-> script_name);
    mem_strfree (& tcb-> script_text);
}


static void
build_initial (THREAD *gsl_thread,
               int     count,
               va_list ap)
{
    int
        i;
    CLASS_ITEM
        *class_item;

    tcb = gsl_thread-> tcb;             /*  Point to thread's context        */

    if (count <= 0)
        return;

    tcb-> initial = memt_alloc (gsl_thread-> memtrn,
                                count * sizeof (CLASS_ITEM));
    tcb-> initial_cnt = count;
    for (i = 0; i < count; i++)
      {
        class_item =  va_arg (ap, CLASS_ITEM *);
        class_item-> class-> link (class_item-> item);
        tcb-> initial [i] =  *class_item;
      }
}


static void
initialise_predefined_scopes (THREAD *thread)
{
    SCOPE_BLOCK
        *scope_block;
    CLASS_DESCRIPTOR
        *class;
    void
        *item;
    int
        i;

    tcb = thread-> tcb;                 /*  Point to thread's context        */

    /*  Create global block  */
    scope_block = create_scope_block (& tcb-> scope_stack,
                                      GG_UNDEFINED,
                                      "global",
                                      NULL);
    symb_class. create ("global", NULL, NULL,
                         & scope_block-> class, & scope_block-> item);
    symb_class. link (scope_block-> item);

    /*  Create GSL block  */
    scope_block = create_scope_block (& tcb-> scope_stack,
                                      GG_UNDEFINED,
                                      "gsl",
                                      NULL);
    scope_block-> class = & gsl_class;
    scope_block-> item  = tcb-> gsl;
    gsl_class. link (scope_block-> item);

    /*  Create classes block  */
    scope_block = create_scope_block (& tcb-> scope_stack,
                                      GG_UNDEFINED,
                                      "class",
                                      NULL);
    scope_block-> class = & symb_class;
    scope_block-> item  = tcb-> classes;
    symb_class. link (tcb-> classes);

    /*  Create initial blocks  */
    for (i = 0; i < tcb-> initial_cnt; i++)
      {
        class = tcb-> initial [i]. class;
        item  = tcb-> initial [i]. item;
        scope_block = create_scope_block
            (& tcb-> scope_stack,
             GG_UNDEFINED,
             class-> item_name ? class-> item_name (item) : class-> name,
             class);
        create_scope_item (scope_block,
                           class,
                           item,
                           1);
        first_scope_item  (scope_block);
      }
}


static void
prepare_gsl_control_block (THREAD *thread, SYMTAB *switches)
{
    VALUE
        *arg_value;
    CLASS_DESCRIPTOR
        *dummyclass;
    int
        i;
    SYMBOL
        *symbol;

    tcb = thread-> tcb;                 /*  Point to thread's context        */

    tcb-> gsl-> ignorecase = (Bool) number_value (switch_value (switches,
                                                                "ignorecase"));
    tcb-> gsl-> cobol      = (Bool) number_value (switch_value (switches,
                                                                "cobol"));
    tcb-> gsl-> template   = (Bool) number_value (switch_value (switches,
                                                               "template"));
    tcb-> gsl-> line       = NULL;      /*  No script line yet  */
    tcb-> gsl-> shuffle    = (int) number_value (switch_value (switches,
                                                               "shuffle"));
    tcb-> gsl-> terminator = mem_strdup (string_value (switch_value (switches,
                                                                     "terminator")));
    tcb-> gsl-> input_file = mem_strdup (string_value (switch_value (switches,
                                                                     "filename")));

    /*  Count how many arguments we have  */
    i = 0;
    while ((arg_value = switch_value (switches, strprintf ("arg%u", i+1)))
       &&  (arg_value-> type != TYPE_UNDEFINED))
        i++;
    tcb-> gsl-> argc = i;
    if (tcb-> gsl-> argc)
      {
        tcb-> gsl-> argv = mem_alloc (i * sizeof (char *));

        /*  Assign the argument values  */
        for (i = 0; i < tcb-> gsl-> argc; i++)
          {
            arg_value = switch_value (switches, strprintf ("arg%u",
                                                           i + 1));
            tcb-> gsl-> argv [i] = mem_strdup (arg_value-> s);
          }
      }

    symb_class. create ("switches", NULL, NULL,
                        & dummyclass,
                        (void **) & tcb-> gsl-> switches);
    symb_class. link (tcb-> gsl-> switches);

    for (symbol = switches-> symbols; symbol; symbol = symbol-> next)
        sym_assume_symbol (tcb-> gsl-> switches-> symtab,
                           symbol-> name,
                           symbol-> value);
}


static VALUE *
switch_value (SYMTAB *switches, char *name)
{
    RESULT_NODE
        symbol_name = {NULL,
                       NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
                       NULL, NULL, NULL, NULL,
                       0, NULL, NULL,
                       0, 0, 0,
                       {TYPE_STRING, NULL, 0},
                       FALSE },
        symbol =      {NULL,
                       NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
                       NULL, NULL, NULL, NULL,
                       0, NULL, NULL,
                       0, 0, 0,
                       {TYPE_UNDEFINED, NULL, 0},
                       FALSE };
    static VALUE
        switches_value;
    VALUE
        *value;

    init_value (& switches_value);
    switches_value.s = sym_get_value (switches, name, NULL);
    if (switches_value.s)
      {
        switches_value.type = TYPE_UNKNOWN;
        value = & switches_value;
      }
    else
      {
        symbol. name = &symbol_name;
        symbol_name. value. s = name;
        value = symbol_value (& tcb-> scope_stack,
                              & symbol,
                              FALSE,
                              NULL);
      }

    return value;
}


THREAD *
gsl_start (QUEUE    *replyqueue,
           JOBID     job,
           SYMTAB   *switches,
           int       count,
             /* CLASS_ITEM *item */ ...)
{
    va_list
        ap;
    THREAD
       *new_thread;                     /*  Handle to GSL thread             */
    TCB
        *new_tcb;
    MEMTRN
        *save_memtrn;

    new_thread = create_gsl_thread (replyqueue,
                                    0);
    new_tcb = new_thread-> tcb;

    /*  Memory allocations should be attached to gsl thread.  */
    save_memtrn = smt_memtrn_;
    smt_memtrn_ = new_thread-> memtrn;

    va_start (ap, count);
    build_initial (new_thread,
                   count,
                   ap);
    va_end (ap);
    initialise_classes (new_thread);
    initialise_predefined_scopes (new_thread);
    tcb-> script_name = mem_strdup (string_value (switch_value (switches, "script")));
    prepare_gsl_control_block (new_thread, switches);
    new_tcb-> execute_full = FALSE;
    lsend_ggcode_start (& new_thread-> queue-> qid,
                        replyqueue ? & replyqueue-> qid : NULL,
                        NULL, NULL, NULL, 0);

    /*  And restore old memory transaction.  */
    smt_memtrn_ = save_memtrn;

    return new_thread;
}


THREAD *
gsl_copy (THREAD   *old_thread,
          QUEUE    *replyqueue,
          JOBID     job)
{
    THREAD
       *new_thread;                     /*  Handle to GSL thread             */
    TCB
        *old_tcb,
        *new_tcb;
    SCOPE_BLOCK
        *class_block;
    MEMTRN
        *save_memtrn;

    new_thread = create_gsl_thread (replyqueue,
                                    0);
    new_tcb = new_thread-> tcb;
    old_tcb = old_thread-> tcb;

    new_tcb-> job         = job;

    /*  Memory allocations should be attached to gsl thread.  */
    save_memtrn = smt_memtrn_;
    smt_memtrn_ = new_thread-> memtrn;

    /*  New thread inherits scopes from old thread  */
    copy_scope_stack (& new_tcb-> scope_stack,
                      & old_tcb-> scope_stack);

    /*  We need a new instance of the class definitions.  */
    class_block =
        first_scope_block (& new_tcb-> scope_stack)
            -> prev-> prev;  /*  Relies on being the 3rd scope block  */
    ASSERT (streq (class_block-> name, "class"));

    /*  Not really destroy, just break one link  */
    symb_class. destroy (class_block-> item);

    if (initialise_classes (new_thread))
      {
        thread_destroy (new_thread, TRUE);
        return NULL;
      }

    class_block-> class = & symb_class;
    class_block-> item  = tcb-> classes;
    gsl_class. link (class_block-> item);

    /*  We need a new instance of the GSL control block.  */
    class_block =
        first_scope_block (& new_tcb-> scope_stack)
            -> prev;         /*  Relies on being the 2nd scope block  */

    /*  Not really destroy, just break one link  */
    gsl_class. destroy (class_block-> item);

    /*  Attach our own GSL control block to this scope.  */
    class_block-> item = new_tcb-> gsl;
    gsl_class. link (class_block-> item);

    copy_gsl_control_block (new_tcb-> gsl, old_tcb-> gsl);

    lsend_ggcode_start (& new_thread-> queue-> qid,
                        replyqueue ? & replyqueue-> qid : NULL,
                        NULL, NULL, NULL, 0);

    /*  And restore old memory transaction.  */
    smt_memtrn_ = save_memtrn;

    return new_thread;
}


void
gsl_continue (THREAD   *gsl_thread,
              Bool      terminate,
              QUEUE    *replyqueue)
{
    ((TCB *) gsl_thread-> tcb)-> execute_full = terminate;
    lsend_ggcode_continue (& gsl_thread-> queue-> qid,
                           replyqueue ? & replyqueue-> qid : NULL,
                           NULL, NULL, NULL, 0);
}

void
gsl_next (THREAD   *gsl_thread,
          QUEUE    *replyqueue)
{
    lsend_ggcode_next (& gsl_thread-> queue-> qid,
                       replyqueue ? & replyqueue-> qid : NULL,
                       NULL, NULL, NULL, 0);
}

void
gsl_command (THREAD   *gsl_thread,
             char     *command,
             Bool      terminate,
             QUEUE    *replyqueue)
{
    ((TCB *) gsl_thread-> tcb)-> execute_full = terminate;
    lsend_ggcode_gsl (& gsl_thread-> queue-> qid,
                      replyqueue ? & replyqueue-> qid : NULL,
                      NULL, NULL, NULL, 0,
                      command);
}


int
gsl_function (      THREAD *gsl_thread,
                    QUEUE  *replyqueue,
              const char   *function,
                    int    parm_count,
                    VALUE  *parm_value[])
{
    MACRO_ITEM
        *macro = NULL;
    SCRIPT_NODE
        *arg;
    int
        argn,
        i;
    SCOPE_BLOCK
        *scope_block;
    MEMTRN
        *save_memtrn;
    void
        *symb_item;
    VALUE
        value;

    tcb = gsl_thread-> tcb;

    /*  This is copied from macro_value function  */
    FORLIST (scope_block, tcb-> scope_stack)
        if (scope_block-> macros)
          {
            macro = macro_lookup (scope_block-> macros,
                                  function);
            if (macro)
                break;
          }

    if (! macro)
        return -1;

    arg = macro-> line-> node-> op1;
    if (! arg)
        argn = 0;
    else
        argn = 1;

    while (arg
       && (arg-> type     == GG_OPERATOR)
       && (arg-> operator == OP_NEXT_ARG))
      {
        arg = arg-> op1;
        argn++;
      }

    /*  Must be at least as many args as params  */
    if (argn < parm_count)
        return -1;

    /*  Memory allocations should be attached to gsl thread.  */
    save_memtrn = smt_memtrn_;
    smt_memtrn_ = gsl_thread-> memtrn;

    scope_block = create_scope_block
                      (& tcb-> scope_stack,
                       GG_MACRO,
                       function,
                       NULL);
    scope_block-> stacked  = FALSE;     /*  Macro/f'n scopes aren't stacked  */
    symb_class. create (scope_block-> name, NULL, NULL,
                         & scope_block-> class, & scope_block-> item);
    symb_item = scope_block-> item;
    symb_class. link (symb_item);

    /*  Create unstacked alias 'my' for macro scope block.                   */
    scope_block = create_scope_block (& tcb-> scope_stack,
                                      GG_UNDEFINED, "my",
                                      & symb_class);
    scope_block-> stacked = FALSE;
    scope_block-> item    = symb_item;
    symb_class. link (symb_item);

    tcb-> gsl-> line   = macro-> line;
    tcb-> script = tcb-> gsl-> line-> parent;

    if (parm_count)
      {
        init_value (& value);
        copy_value (& value, parm_value [0]);
        symb_class. put_attr (symb_item,
                              (char *) arg-> op1,
                              & value,
                              tcb-> gsl-> ignorecase);
        destroy_value (& value);
      }
    for (i = 1; i < parm_count; i++)
      {
        init_value (& value);
        copy_value (& value, parm_value [i]);
        arg = arg-> parent;
        symb_class. put_attr (symb_item,
                              (char *) arg-> op2-> op1,
                              & value,
                              tcb-> gsl-> ignorecase);
      }

    lsend_ggcode_call (& gsl_thread-> queue-> qid,
                       replyqueue ? & replyqueue-> qid : NULL,
                       NULL, NULL, NULL, 0);

    /*  And restore old memory transaction.  */
    smt_memtrn_ = save_memtrn;

    return 0;
}


int     gsl_evaluate     (THREAD      *gsl_thread,
                          char        *expression,
                          Bool        terminate,
                          RESULT_NODE **result,
                          QUEUE       *replyqueue)
{
    ((TCB *) gsl_thread-> tcb)-> execute_full = terminate;
    tcb-> result_ptr  = result;
    lsend_ggcode_evaluate (& gsl_thread-> queue-> qid,
                           replyqueue ? & replyqueue-> qid : NULL,
                           NULL, NULL, NULL, 0,
                           expression);

    return 0;
}


void
gsl_finish (THREAD *gsl_thread)
{
    lsend_ggcode_finish (& gsl_thread-> queue-> qid,
                         NULL,
                         NULL, NULL, NULL, 0);
}


THREAD *
gsl_spawn (THREAD        *old_thread,
           QUEUE         *replyqueue,
           SCRIPT_HANDLE *script_handle)
{
    THREAD
       *new_thread;                     /*  Handle to GSL thread             */
    TCB
        *old_tcb,
        *new_tcb;
    SCOPE_BLOCK
        *class_block;
    MEMTRN
        *save_memtrn;

    new_thread = create_gsl_thread (replyqueue,
                                    0);
    new_tcb = new_thread-> tcb;
    old_tcb = old_thread-> tcb;

    /*  Memory allocations should be attached to gsl thread.  */
    save_memtrn = smt_memtrn_;
    smt_memtrn_ = new_thread-> memtrn;

    /*  New thread inherits scopes from old thread  */
    copy_scope_stack (& new_tcb-> scope_stack,
                      & old_tcb-> scope_stack);

    /*  We need a new instance of the class definitions.  */
    class_block =
        first_scope_block (& new_tcb-> scope_stack)
            -> prev-> prev;  /*  Relies on being the 3rd scope block  */
    ASSERT (streq (class_block-> name, "class"));

    /*  Not really destroy, just break one link  */
    symb_class. destroy (class_block-> item);

    if (initialise_classes (new_thread))
      {
        thread_destroy (new_thread, TRUE);
        return NULL;
      }

    class_block-> class = & symb_class;
    class_block-> item  = tcb-> classes;
    gsl_class. link (class_block-> item);

    copy_gsl_control_block (new_tcb-> gsl, old_tcb-> gsl);

    new_tcb-> script = script_handle;

    lsend_ggcode_spawn (& new_thread-> queue-> qid,
                        replyqueue ? & replyqueue-> qid : NULL,
                        NULL, NULL, NULL, 0);

    /*  And restore old memory transaction.  */
    smt_memtrn_ = save_memtrn;

    return new_thread;
}


static void
copy_gsl_control_block (GSL_CONTROL *to, GSL_CONTROL *from)
{
    int
        i;

    to-> ignorecase = from-> ignorecase;
    to-> cobol      = from-> cobol;
    to-> template   = from-> line ? from-> line-> template
                                  : from-> template;
    to-> shuffle    = from-> shuffle;
    to-> terminator = mem_strdup (from-> terminator);
    to-> argc       = from-> argc;
    to-> argv = mem_alloc (to-> argc * sizeof (char *));
    for (i = 0; i < to-> argc; i++)
        to-> argv [i] = mem_strdup (from-> argv [i]);
    to-> switches   = from-> switches;
    symb_class. link (to-> switches);
    to-> output_file = mem_strdup (from-> output_file);
    to-> input_file  = mem_strdup (from-> input_file);
}


char *
gsl_cur_script (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    if (tcb-> gsl-> line && tcb-> gsl-> line-> parent)
        return tcb-> gsl-> line-> parent-> name;
    else
        return NULL;
}


int
gsl_cur_line (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    if (tcb-> gsl-> line)
        return tcb-> gsl-> line-> line;
    else
        return 0;
}

char *
gsl_cur_text (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    if (tcb-> gsl-> line)
        return tcb-> gsl-> line-> text;
    else
        return 0;
}


int
gsl_term (void)
{
    memt_assert  (global_memtrn);
    mem_rollback (global_memtrn);

    destroy_objects ();
    destroy_caches ();

    return 0;
}


/*************************   INITIALISE THE THREAD   *************************/

MODULE initialise_the_thread (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */
}


static void
report_error (event_t event, char *format, ...)
{
    va_list
        argptr;
    char
        buffer [LINE_MAX];
    THREAD
       *thread = tcb-> thread;

    va_start (argptr, format);          /*  Start variable arguments list    */
    vsnprintf (buffer, LINE_MAX, format, argptr);
    va_end (argptr);                    /*  End variable arguments list      */

    if (tcb-> gsl-> line)
        send_ggcode_error (& tcb-> replyq-> qid,
                           tcb-> job,
                           gsl_cur_script (thread),
                           gsl_cur_line (thread),
                           buffer);
    else
        send_ggcode_error (& tcb-> replyq-> qid,
                           tcb-> job,
                           NULL,
                           0,
                           buffer);

    if (event != _LR_NULL_EVENT)
        call_exception (event);
}


/*************************   RESET ERROR OCCURRENCE   ************************/

MODULE reset_error_occurrence (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    tcb-> error_occurred = FALSE;
}


/************************   CALL LOAD INITIAL SCRIPT   ***********************/

MODULE call_load_initial_script (THREAD *thread)
{

    tcb = thread-> tcb;                 /*  Point to thread's context        */

    if (tcb-> script_name)
        call_exception  (load_script_file_event);
    else
        report_error (error_event, "No script specified.");
}


/*******************   RAISE EXCEPTION IF ERROR OCCURRED   *******************/

MODULE raise_exception_if_error_occurred (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    if (tcb-> error_occurred)
        raise_exception (exception_event);
}


/*************************   GET NEXT SCRIPT LINE   **************************/

MODULE get_next_script_line (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    if (! tcb-> gsl-> line)
      {
        if (! list_empty (& tcb-> script-> line_head))
            tcb-> gsl-> line = tcb-> script-> line_head. next;
      }
    else
      {
        if ((void *) tcb-> gsl-> line-> next !=  & tcb-> gsl-> line-> parent-> line_head)
            tcb-> gsl-> line  = tcb-> gsl-> line-> next;
        else
            tcb-> gsl-> line = NULL;
      }

    /*  Skip comment lines  */
    while (tcb-> gsl-> line && ! tcb-> gsl-> line-> node)
      {
        if ((void *) tcb-> gsl-> line-> next
        !=  & tcb-> gsl-> line-> parent-> line_head)
            tcb-> gsl-> line  = tcb-> gsl-> line-> next;
        else
            tcb-> gsl-> line = NULL;
      }

    if (tcb-> gsl-> line)
        tcb-> script_root = tcb-> gsl-> line-> node;
    else
      {
        tcb-> script_root = NULL;
        raise_exception (end_of_script_event);
      }

    tcb-> script_node = tcb-> script_root;
}


/************************   CALL ACCEPT GSL COMMAND   ************************/

MODULE call_accept_gsl_command (THREAD *thread)
{
    struct_ggcode_gsl
        *param;

    tcb = thread-> tcb;                 /*  Point to thread's context        */

    get_ggcode_gsl (thread-> event-> body, & param);

    tcb-> script_text = mem_strdup (param-> command);

    call_exception (load_script_text_event);
    free_ggcode_gsl (&param);
}


/************************   CALL EXECUTE SCRIPT LINE   ***********************/

MODULE call_execute_script_line (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    if (tcb-> script_node)
        call_exception  (execute_line_event);
}


/************************   REQUEST PARSE EXPRESSION   ***********************/

MODULE request_parse_expression (THREAD *thread)
{
    struct_ggcode_evaluate
        *param;

    tcb = thread-> tcb;                 /*  Point to thread's context        */

    get_ggcode_evaluate (thread-> event-> body, & param);

    gg_parse_expression (param-> expression,
                         tcb-> job,
                         thread-> queue);

    free_ggcode_evaluate (&param);
}


/**************************   RETURN PARSER ERROR   **************************/

MODULE return_parser_error (THREAD *thread)
{
    struct_ggpars_error
        *error;

    tcb = thread-> tcb;                 /*  Point to thread's context        */

    get_ggpars_error (thread-> event-> body, & error);

    if (error-> error_text)
        send_ggcode_fatal (& tcb-> replyq-> qid,
                           tcb-> job,
                           gsl_cur_script (thread),
                           gsl_cur_line (thread),
                           error-> error_text);
    else
        send_ggcode_fatal (& tcb-> replyq-> qid,
                           tcb-> job,
                           NULL, 0, NULL);

    free_ggpars_error (& error);
}


/*************************   RETURN ERROR FEEDBACK   *************************/

MODULE return_error_feedback (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    send_ggcode_fatal (& tcb-> replyq-> qid,
                       tcb-> job,
                       NULL, 0, NULL);
}


/**********************   CALL EVALUATE PARSER RESULT   **********************/

MODULE call_evaluate_parser_result (THREAD *thread)
{
    struct_ggpars_ok
        *parse_result;

    tcb = thread-> tcb;                 /*  Point to thread's context        */

    get_ggpars_ok (thread-> event-> body, & parse_result);

    tcb-> evaluate_node = (SCRIPT_NODE *) parse_result-> parse_root;
    mem_save (tcb-> scratch_memtrn, (MEMTRN *) parse_result-> parse_memtrn);
    call_exception (evaluate_event);

    free_ggpars_ok (& parse_result);
}


/***************************   FREE PARSE RESULT   ***************************/

MODULE free_parse_result (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    mem_rollback (tcb-> scratch_memtrn);
    tcb-> scratch_memtrn = mem_new_trans ();
}


/***************************   GENERATE OK EVENT   ***************************/

MODULE generate_ok_event (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    the_next_event = ok_event;
}


/**********************   CALL FUNCTION CALL EXCEPTION   *********************/

MODULE call_function_call_exception (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    call_exception (function_call_event);
}


/************************   GENERATE NODE TYPE EVENT   ***********************/

MODULE generate_node_type_event (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    if (tcb-> script_node)
      {
        ASSERT (tcb-> script_node-> type <  tblsize (type_event));
        the_next_event = type_event [tcb-> script_node-> type];
        if (the_next_event == _LR_NULL_EVENT)
            raise_exception (anomaly_event);
      }
    else
        the_next_event = end_of_script_event;
}


/***************************   CREATE RESULT ROOT   **************************/

MODULE create_result_root (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    tcb-> result_root = new_result_node ();;
    ASSERT (tcb-> result_root);
    init_result (tcb-> result_root);

    tcb-> result_root-> script_node = tcb-> script_root;
    tcb-> result_node = tcb-> result_root;
}


/***************************   CALL EVALUATE OP1   ***************************/

MODULE call_evaluate_op1 (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    if (tcb-> script_node-> op1)
      {
        tcb-> evaluate_node =   tcb-> script_node-> op1;
        tcb-> result_ptr    =   tcb-> result_node
                            ? & tcb-> result_node-> op1
                            : & tcb-> result_root;
        call_exception (evaluate_event);
      }
}


/**************************   COPY LINE TO OUTPUT   **************************/

MODULE copy_line_to_output (THREAD *thread)
{
    Bool
        extend;                         /*  Is line extended?  ie no EOL     */
    char
        *line;
    RESULT_NODE
        *new_node;
    char
        *error_text;

    tcb = thread-> tcb;                 /*  Point to thread's context        */

    extend = tcb-> script_node-> op1-> extend;

    if (tcb-> output_buffer)
      {
        new_node = new_result_node ();;
        ASSERT (new_node);
        init_result (new_node);

        new_node-> op1 = tcb-> output_buffer;
        /*  Migrate indent value to new node.  */
        new_node-> indent = tcb-> output_buffer-> indent;
        tcb-> output_buffer-> indent = 0;
        new_node-> op1-> parent = new_node;
        new_node-> op2 = tcb-> result_node-> op1;
        new_node-> op2-> parent = new_node;
        inherit_culprit (  new_node-> op1-> culprit
                         ? new_node-> op1
                         : new_node-> op2);
        tcb-> result_node-> op1 = new_node;
        tcb-> output_buffer = NULL;
      }
    /*  If this isn't end of line then save result and stop.  */
    if (extend)
      {
        tcb-> output_buffer = tcb-> result_node-> op1;
        tcb-> result_node-> op1 = NULL;
        tcb-> output_buffer-> parent = NULL;        /*  Make node an orphan  */
      }
    else
      {
        line = concatenate_results (tcb-> result_node-> op1,
                                    tcb-> gsl-> shuffle,
                                    TRUE,
                                    &error_text);
        if (! line)
          {
            report_error (error_event, "%s", error_text);
            return;
          }

        output_the_line (line, tcb-> gsl-> cobol);
/*        if (tcb-> gsl-> terminator && tcb-> gsl-> terminator [0])
            send_text_to_output (tcb-> gsl-> terminator);  JS 2004-11-15 */
      }
}


static void
inherit_culprit (RESULT_NODE *node)
{
    if (! node-> parent-> culprit)
      {
        node-> parent-> culprit = node-> culprit;
        node-> culprit = NULL;
      }
}


static void
output_the_line (char *line, Bool cobol)
{
    char
        *out,
        *ptr,
        linenum [7];

    out = line;
    while (out)
      {
        ptr = strchr (out, '\n');
        if (ptr)
            *ptr = 0;
        if (cobol)
          {
            snprintf (linenum, 7, "%04ld00", tcb-> gsl-> output_line);
            if (strlen (out) > 6)
              {
                memcpy (out, linenum, 6);
                send_text_to_output (out);
              }
            else
                send_text_to_output (linenum);
          }
        else
            send_text_to_output (out);

        if (ptr)
          {
            out = ptr + 1;
            tcb-> gsl-> output_line++;
            if (tcb-> gsl-> terminator && tcb-> gsl-> terminator [0])
                send_text_to_output (tcb-> gsl-> terminator);
          }
        else
            out = 0;
      }
    mem_free (line);
}


static void
send_text_to_output (char *text)
{
    if (tcb-> output)
        fprintf (tcb-> output, "%s", text);
    else
      {
        if (tcb-> output_fct)
            (tcb-> output_fct) (tcb-> job, text);
        if (tcb-> stdout_echo)
            fprintf (stdout, "%s", text);
      }
}


/**********************   COPY OUTPUT BUFFER TO OUTPUT   *********************/

MODULE copy_output_buffer_to_output (THREAD *thread)
{
    char
        *line;
    char
        *error_text;

    tcb = thread-> tcb;                 /*  Point to thread's context        */

    if (! tcb-> output_buffer)
        return;

    line = concatenate_results (tcb-> output_buffer,
                                tcb-> gsl-> shuffle,
                                TRUE,
                                &error_text);
    if (! line)
      {
        report_error (error_event, "%s", error_text);
        return;
      }
    output_the_line (line, tcb-> gsl-> cobol);
}


/****************************   FREE RESULT ROOT   ***************************/

MODULE free_result_root (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    destroy_result (tcb-> result_root);
    tcb-> result_root = NULL;
}


/**************************   CALL EVALUATE SCOPE   **************************/

MODULE call_evaluate_scope (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    if (tcb-> script_node-> scope)
      {
        tcb-> evaluate_node =   tcb-> script_node-> scope;
        tcb-> result_ptr    = & tcb-> result_node-> scope;
        call_exception (evaluate_event);
      }
}


/***************************   CALL EVALUATE NAME   **************************/

MODULE call_evaluate_name (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    if (tcb-> script_node-> name)
      {
        tcb-> evaluate_node =   tcb-> script_node-> name;
        tcb-> result_ptr    = & tcb-> result_node-> name;
        call_exception (evaluate_event);
      }
}


/**********************   GENERATE OPERATOR TYPE EVENT   *********************/

MODULE generate_operator_type_event (THREAD *thread)
{
    static event_t op_type_event [] = {
        undefined_event,                /*  OP_UNDEFINED           */
        arithmetic_event,               /*  OP_TIMES               */
        arithmetic_event,               /*  OP_DIVIDE              */
        arithmetic_event,               /*  OP_PLUS                */
        arithmetic_event,               /*  OP_MINUS               */
        iif_event,                      /*  OP_IIF                 */
        default_event,                  /*  OP_DEFAULT             */
        comparison_event,               /*  OP_EQUALS              */
        comparison_event,               /*  OP_NOT_EQUALS          */
        comparison_event,               /*  OP_GREATER_THAN        */
        comparison_event,               /*  OP_LESS_THAN           */
        comparison_event,               /*  OP_GREATER_EQUAL       */
        comparison_event,               /*  OP_LESS_EQUAL          */
        comparison_event,               /*  OP_SAFE_EQUALS         */
        comparison_event,               /*  OP_SAFE_NOT_EQUALS     */
        comparison_event,               /*  OP_SAFE_GREATER_THAN   */
        comparison_event,               /*  OP_SAFE_LESS_THAN      */
        comparison_event,               /*  OP_SAFE_GREATER_EQUAL  */
        comparison_event,               /*  OP_SAFE_LESS_EQUAL     */
        arithmetic_event,               /*  OP_NOT                 */
        or_event,                       /*  OP_OR                  */
        and_event,                      /*  OP_AND                 */
        next_arg_event };               /*  OP_NEXT_ARG            */

    tcb = thread-> tcb;                 /*  Point to thread's context        */

    ASSERT (tcb-> script_node-> operator < tblsize (op_type_event));
    the_next_event = op_type_event [tcb-> script_node-> operator];
}


/*******************************   SKIP BLOCK   ******************************/

MODULE skip_block (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    tcb-> gsl-> line = tcb-> gsl-> line-> block_end;
    tcb-> script_root = tcb-> gsl-> line-> node;
    tcb-> script_node = tcb-> script_root;
}


/***************************   CLOSE OUTPUT FILE   ***************************/

MODULE close_output_file (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    if (tcb-> output)
        file_close (tcb-> output);

    tcb-> output = NULL;
    mem_strfree (& tcb-> gsl-> output_file);
    tcb-> gsl-> output_line = 0;
}


/**************************   OPEN FILE FOR OUTPUT   *************************/

MODULE open_file_for_output (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    /*  .output <op1>  */

    must_be_defined_and_linear (tcb-> result_node-> op1);
    if (exception_raised)
        return;

    open_output_file (FOPEN_WRITE_TEXT);
    tcb-> gsl-> output_line = 1;
}


static void
must_be_defined_and_linear (RESULT_NODE *node)
{
    if (node)
      {
        if (node-> value. type == TYPE_UNDEFINED)
            report_undefined_expression (node);
        else
        if (node-> value. type == TYPE_POINTER)
          {
            string_value (& node-> value);
            if (! node-> value. s)
                report_undefined_expression (node);
          }
      }
}


static void
open_output_file (char *mode)
{
    char
        *filename = string_value (& tcb-> result_node-> op1-> value);

    if (filename && *filename)
        tcb-> output = fopen (filename, mode);

    if (tcb-> output)
      {
        mem_strfree (& tcb-> gsl-> output_file);
        tcb-> gsl-> output_file = mem_strdup (filename);
      }
    else
        report_error (error_event,
                      "Can't open output file: %s",
                      filename);
}


/**************************   OPEN FILE FOR APPEND   *************************/

MODULE open_file_for_append (THREAD *thread)
{
    FILE
        *current = NULL;
    char
        buffer [LINE_MAX + 1];

    tcb = thread-> tcb;                 /*  Point to thread's context        */

    /*  .append <op1>  */

    must_be_defined_and_linear (tcb-> result_node-> op1);
    if (exception_raised)
        return;

    /*  Count line number for extended file  */
    tcb-> gsl-> output_line = 0;
    current = fopen (string_value (& tcb-> result_node-> op1-> value),
                     FOPEN_READ_TEXT);
    if (current)
      {
        while (gsl_file_read (current, buffer))
            tcb-> gsl-> output_line++;
        file_close (current);
      }

    open_output_file (FOPEN_APPEND_TEXT);
}


/**************************   PUSH SCRIPT POSITION   *************************/

MODULE push_script_position (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    list_push (& tcb-> script_stack, tcb-> gsl-> line);
}


/*********************   CALL PREPARE TO INTERPRET GSL   *********************/

MODULE call_prepare_to_interpret_gsl (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    /*  GSL from current script is meaningless.  */
    if (tcb-> script_root-> name)
      {
        report_error (error_event, "Illegal GSL source.");
        return;
      }

    if (tcb-> script_root-> scope)
      {
        tcb-> script_name = mem_strdup (string_value (& tcb-> result_node-> scope-> value));
        call_exception (load_script_file_event);
        return;
      }

    if (tcb-> script_root-> op1)
      {
        tcb-> script_name = mem_strdup (strprintf ("(%s %u)",
                                                   gsl_cur_script (thread),
                                                   gsl_cur_line (thread)));
        tcb-> script_text = mem_strdup (string_value
                                (& tcb-> result_node-> op1-> value));
        call_exception (load_script_text_event);
      }
}


/*************************   COPY DIRECT TO OUTPUT   *************************/

MODULE copy_direct_to_output (THREAD *thread)
{
    char
        line [LINE_MAX + 2];
    FILE
        *file;

    tcb = thread-> tcb;                 /*  Point to thread's context        */

    if (tcb-> script_root-> scope)
      {
        file = file_locate (PATH,
                            string_value (& tcb-> result_node-> scope-> value),
                            NULL);
        if (! file)
          {
            report_error (error_event, "Can't open literal file %s",
                          string_value (& tcb-> result_node-> scope-> value));
            return;
          }
        while (gsl_file_read (file, line))
          {
            send_text_to_output (line);
          }
        file_close (file);
        return;
      }

    if (tcb-> script_root-> name)
      {
        send_text_to_output (gsl_cur_text (thread));
        return;
      }

    if (tcb-> script_root-> op1)
        send_text_to_output (string_value (& tcb-> result_node-> op1-> value));
}


/****************************   CALL EVALUATE AS   ***************************/

MODULE call_evaluate_as (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    if (tcb-> script_node-> as)
      {
        tcb-> evaluate_node =   tcb-> script_node-> as;
        tcb-> result_ptr    = & tcb-> result_node-> as;
        call_exception (evaluate_event);
      }
}


/****************************   CALL EVALUATE TO   ***************************/

MODULE call_evaluate_to (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    if (tcb-> script_node-> to)
      {
        tcb-> evaluate_node =   tcb-> script_node-> to;
        tcb-> result_ptr    = & tcb-> result_node-> to;
        call_exception (evaluate_event);
      }
}


/**************************   CALL EVALUATE BEFORE   *************************/

MODULE call_evaluate_before (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    if (tcb-> script_node-> before)
      {
        tcb-> evaluate_node =   tcb-> script_node-> before;
        tcb-> result_ptr    = & tcb-> result_node-> before;
        call_exception (evaluate_event);
      }
}


/**************************   CALL EVALUATE AFTER   **************************/

MODULE call_evaluate_after (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    if (tcb-> script_node-> after)
      {
        tcb-> evaluate_node =   tcb-> script_node-> after;
        tcb-> result_ptr    = & tcb-> result_node-> after;
        call_exception (evaluate_event);
      }
}


/****************************   LOAD SOURCE XML   ****************************/

MODULE load_source_xml (THREAD *thread)
{
    char
       *buffer;
    CLASS_DESCRIPTOR
        *parent_class,
        *sibling_class;
    void
        *parent_item,
        *sibling_item;
    XML_ITEM
        *xml_item,
        *xml_next,
        *file_root;
    int
        rc;

    tcb = thread-> tcb;                 /*  Point to thread's context        */

    /* .xml [as <as>] [to <to> | before <before> | after <after>] ... */
    /*     ... [from <scope> | << {script_line} | "<op1>"]            */

    must_be_defined_and_linear (tcb-> result_node-> as);
    must_be_defined            (tcb-> result_node-> to);
    must_be_defined            (tcb-> result_node-> before);
    must_be_defined            (tcb-> result_node-> after);
    if (exception_raised)
        return;

    if (parse_locator (& parent_class,  & parent_item,
                       & sibling_class, & sibling_item, thread))
        return;

    if (parent_class != & XML_item_class)
      {
        report_error (error_event,
                      "Cannot load XML as child of class: %s",
                      parent_class-> name);
        return;
      }

    if (tcb-> script_root-> scope)
      {
        file_root = xml_create (NULL, NULL);
        rc = xml_load_file (& file_root,
                            PATH,
                            string_value (& tcb-> result_node-> scope-> value),
                            TRUE);
      }
    else
      {
        if (tcb-> script_root-> name)
            buffer = gsl_cur_text (thread);
        else
            buffer = string_value (& tcb-> result_node-> op1-> value);

        file_root = xml_create (NULL, NULL);
        rc = xml_load_string (& file_root,
                              buffer,
                              TRUE);
      }

    if (rc)
        report_error (error_event, "%s", xml_error ());
    else
      {
        xml_item = xml_first_child (file_root);
        while (xml_item)
          {
            xml_next = xml_next_sibling (xml_item);
            xml_detach (xml_item);
            if (tcb-> result_node-> as)
                xml_rename (xml_item,
                            string_value (& tcb-> result_node-> as-> value));
            if (sibling_item)
                xml_attach_sibling (get_xml_item (sibling_item), xml_item);
            else
                xml_attach_child   (get_xml_item (parent_item),  xml_item);

            xml_item = xml_next;
          }
      }
    xml_free (file_root);

    free_pointer (sibling_class, sibling_item);
    free_pointer (parent_class,  parent_item);
}


static int
parse_locator (CLASS_DESCRIPTOR **parent_class,  void **parent_item,
               CLASS_DESCRIPTOR **sibling_class, void **sibling_item,
               THREAD *thread)
{
    RESULT_NODE
        *sibling_node;

    tcb = thread-> tcb;                 /*  Point to thread's context        */

    *parent_item  = NULL;
    *sibling_item = NULL;
    if (tcb-> result_root-> to)
      {
        *parent_class = tcb-> result_node-> to-> value. c;
        *parent_item  = tcb-> result_node-> to-> value. i;
      }

    if (! (tcb-> result_root-> to
        || tcb-> result_root-> before
        || tcb-> result_root-> after))
      {
        lookup_innermost_item (parent_class, parent_item);
        return 0;
      }

    if (tcb-> result_root-> before && tcb-> result_root-> after)
      {
        report_error (error_event,
                      "Only one of 'before' and 'after' may be specified.");
        return -1;
      }

    if (tcb-> result_root-> before || tcb-> result_root-> after)
      {
        sibling_node = tcb-> result_root-> before ? tcb-> result_node-> before
                                                  : tcb-> result_node-> after;
        *sibling_class = sibling_node-> value. c;
        *sibling_item  = sibling_node-> value. i;

        (*sibling_class)-> parent (*sibling_item,
                                   parent_class, parent_item,
                                   thread);
        if (tcb-> result_root-> to)
          {
            if (*parent_item != tcb-> result_node-> to-> value. i)
              {
                report_error (error_event, "Scope %s is not a child of scope %s",
                              string_value (& sibling_node-> value),
                              extended_scope_string (tcb-> result_node-> to));
                free_pointer (*parent_class, *parent_item);
                return -1;
              }
          }
        else
          {
            if (! *parent_item)
              {
                report_error (error_event, "Scope: %s has no parent.",
                              string_value (& sibling_node-> value));
                free_pointer (*parent_class, *parent_item);
                return -1;
              }
          }
      }

    if (tcb-> result_root-> after)
        (*sibling_class)-> next_sibling (*sibling_item,
                                         NULL, TRUE,
                                         sibling_class, sibling_item);

    return 0;
}


static void
lookup_innermost_item (CLASS_DESCRIPTOR **class,
                       void             **item)
{
    SCOPE_BLOCK
        *scope_block = NULL;

    FORLIST (scope_block, tcb-> scope_stack)
        if (scope_block-> stacked)
          {
            *class = scope_block-> class;
            *item  = scope_block-> item;
            return;
          }
}


/***********************   CALL BUILD ITERATION LIST   ***********************/

MODULE call_build_iteration_list (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    call_exception (build_list_event);
}


/********************   CALL BUILD MEMBER ITERATION LIST   *******************/

MODULE call_build_member_iteration_list (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    call_exception (build_member_list_event);
}

/* JS 2001/02/18 - for 'count' alias */
/********************   CALL BUILD COUNT ITERATION LIST   ********************/

MODULE call_build_count_iteration_list (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    call_exception (build_count_list_event);
}


/* JS 2001/02/18 - for 'count' alias */
/*************************   OPEN COUNT ALIAS BLOCK   ************************/

MODULE open_count_alias_block (THREAD *thread)
{
    SCOPE_BLOCK
        *last_scope,
        *scope;

    tcb = thread-> tcb;                 /*  Point to thread's context        */

    /*  If count loop isn't aliased, then make an unstacked alias 'count'.   */
    /*  This is for backwards compatibility.                                 */
    if (! tcb-> result_node-> as)
      {
        last_scope = last_scope_block (& tcb-> scope_stack);

        scope = create_scope_block (& tcb-> scope_stack,
                                    GG_UNDEFINED,
                                    "count",
                                    last_scope-> class);
        scope-> stacked = FALSE;
        scope-> item    = last_scope-> item;

        if (last_scope-> class-> link)
            last_scope-> class-> link (last_scope-> item);
      }
}


/* JS 2001/02/18 - for 'count' alias */
/************************   CLOSE COUNT ALIAS BLOCK   ************************/

MODULE close_count_alias_block (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    /*  Now close the unstacked alias 'count', if it exists.                 */
    /*  This is for backwards compatibility.                                 */
    if (! tcb-> result_node-> as)
        destroy_scope_block
            (last_scope_block (& tcb-> scope_stack));
}


/*****************************   SORT THE ITEMS   ****************************/

MODULE sort_the_items (THREAD *thread)
{
    SCOPE_BLOCK
        *scope_block;
    long
        index,
        count;
    void
        *parent,
        *next_item = NULL,
        **next_item_arr,
        *attach_item;
    CLASS_DESCRIPTOR
        *parent_class,
        *next_class,
        **next_class_arr;

    tcb = thread-> tcb;                 /*  Point to thread's context        */

    scope_block = last_scope_block (& tcb-> scope_stack);

    if (tcb-> script_root-> after && scope_block-> total > 1)
      {
        /*  We build an array to hold the first item not in the scope        */
        /*  for each item in the scope.  This allows us to sort the          */
        /*  scope items in situ.                                             */
        next_item_arr  = mem_alloc (scope_block-> total * sizeof (void *));
        next_class_arr = mem_alloc (scope_block-> total * sizeof (CLASS_DESCRIPTOR *));
        ASSERT (next_item_arr);
        ASSERT (next_class_arr);

        index = 0;
        first_scope_item (scope_block);
        if (scope_block-> class-> parent)
          {
            scope_block-> class-> parent (scope_block-> item,
                                          &parent_class, &parent,
                                          thread);
            if (parent)
                parent_class-> link (parent);
          }
        else
            parent = NULL;

        while (scope_block-> item)
          {
            /*  Count how many consecutive XML items are in the scope.       */
            count = 0;
            do
              {
                /*  Get sort key data type right now, so that we don't have  */
                /*  to do it inside the tighter sorting loop.  If sort_type  */
                /*  is TYPE_UNKNOWN then all the sort keys were converted to */
                /*  numbers during the first pass, but if sort_type is       */
                /*  TYPE_STRING then it's possible that some weren't         */
                /*  converted to strings, so we must do it now.              */
                if (tcb-> sort_type == TYPE_STRING)
                    string_value (& scope_block-> scope_item-> sort_key);

                if (next_item)
                    next_class-> destroy (next_item);

                scope_block-> class-> next_sibling (scope_block-> item,
                                                    NULL, TRUE,
                                                    &next_class,
                                                    &next_item);

                /*  Link and destroy each item in turn so don't get freed.   */
                if (next_item)
                    next_class-> link (next_item);

                next_scope_item (scope_block);
                count++;
              } while (next_item
                   && (next_item == scope_block-> item));

            while (count-- > 0)
              {
                next_item_arr  [index] = next_item;
                if (next_item)
                    next_class_arr [index] = next_class;
                else
                    next_class_arr [index] = next_class;

                index++;
                if (next_item)
                    next_class-> link (next_item);
              }
            if (next_item)
                next_class-> destroy (next_item);
            next_item = NULL;
          }

        /*  Now do the sort.                                                 */
        list_sort (& scope_block-> item_list, sort_compare);

        /*  And reattach the sorted items in the same location they were */
        index = 0;
        first_scope_item (scope_block);
        while (scope_block-> item)
          {
            next_item  = next_item_arr  [index];
            next_class = next_class_arr [index];
            index++;
            if (next_class)
              {
                attach_item = next_class-> move (scope_block-> item,
                                                 scope_block-> class,
                                                 NULL,
                                                 parent,
                                                 next_item);
                next_class-> destroy (next_item);
              }
            else
                attach_item = parent_class-> move (scope_block-> item,
                                                   scope_block-> class,
                                                   NULL,
                                                   parent,
                                                   next_item);
            if (! attach_item)
              {
                report_error (error_event,
                              "Error sorting item of class: %s.",
                              next_class-> name);
                return;
              }

            next_scope_item (scope_block);
          }
        parent_class-> destroy (parent);
        mem_free (next_item_arr);
        mem_free (next_class_arr);
      }
}


static Bool
sort_compare (LIST *t1, LIST *t2)
{
    if (tcb-> sort_type == TYPE_STRING)
        return (strcmp (((SCOPE_ITEM *) t1)-> sort_key. s,
                        ((SCOPE_ITEM *) t2)-> sort_key. s) > 0);
    else
        return (((SCOPE_ITEM *) t1)-> sort_key. n
             >  ((SCOPE_ITEM *) t2)-> sort_key. n);
}


/***************************   DESTROY FOR BLOCK   ***************************/

MODULE destroy_for_block (THREAD *thread)
{
    SCRIPT_NODE_TYPE
        scope_type;

    tcb = thread-> tcb;                 /*  Point to thread's context        */

    do
      {
        scope_type = last_scope_block
            (& tcb-> scope_stack)-> scope_type;
        destroy_scope_block
            (last_scope_block (& tcb-> scope_stack));
      }
    while (scope_type != GG_FOR);
}


/**************************   INITIALISE FOR BLOCK   *************************/

MODULE initialise_for_block (THREAD *thread)
{
    SCOPE_BLOCK
        *scope_block;

    tcb = thread-> tcb;                 /*  Point to thread's context        */

    scope_block = last_scope_block (& tcb-> scope_stack);

    /*  Sort if necessary  */
    if (tcb-> script_node-> after)
      {
        /*  Get sort key data type right now, so that we don't have to       */
        /*  it inside the tighter sorting loop.  If sort_type is             */
        /*  TYPE_UNKNOWN then all the sort keys were converted to            */
        /*  numbers during the first pass, but if sort_type is               */
        /*  TYPE_STRING then it's possible that some weren't converted       */
        /*  to strings, so we must do it now.                                */
        first_scope_item (scope_block);
        while (scope_block-> item)
          {
            if (tcb-> sort_type == TYPE_STRING)
                string_value (& scope_block-> scope_item-> sort_key);

            next_scope_item (scope_block);
          }

        list_sort (& scope_block-> item_list, sort_compare);
      }

    if (! first_scope_item (scope_block))
      {
        skip_block (thread);
        destroy_scope_block
            (last_scope_block (& tcb-> scope_stack));
      }
}


/***************************   GO TO TOP OF BLOCK   **************************/

MODULE go_to_top_of_block (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    tcb-> gsl-> line = tcb-> gsl-> line-> loop_start;
    if (tcb-> gsl-> line)
        tcb-> script_root = tcb-> gsl-> line-> node;
    else
      {
        tcb-> script_root = NULL;
        report_error (error_event, "No loop.");
      }

    tcb-> script_node = tcb-> script_root;
}


/************************   GO TO TOP OF NAMED BLOCK   ***********************/

MODULE go_to_top_of_named_block (THREAD *thread)
{
    SCOPE_BLOCK
        *scope_block = NULL;
    SCRIPT_LINE
        *script_line;
    SCRIPT_NODE_TYPE
        type;

    tcb = thread-> tcb;                 /*  Point to thread's context        */

    /*  .(next|last) [<op1>]  */

    script_line = tcb-> gsl-> line;
    type = go_to_top_of_loop (& script_line);

    /*  If name was specified then must be a .for loop -> go find it  */
    if (tcb-> result_node-> op1)
      {
        while (script_line
           &&  type != GG_END_FOR)
            type = go_to_top_of_loop (& script_line);

        scope_block = last_scope_block (& tcb-> scope_stack);

        while (script_line
           &&  scope_block
           &&  scope_block-> name
           && (! streq (scope_block-> name,
                        string_value (& tcb-> result_node-> op1-> value))))
          {
            destroy_scope_block
                (last_scope_block (& tcb-> scope_stack));

            type = go_to_top_of_loop (& script_line);
            while (script_line
               &&  type != GG_END_FOR)
                type = go_to_top_of_loop (& script_line);

            scope_block = last_scope_block (& tcb-> scope_stack);
          }
      }

    tcb-> gsl-> line = script_line;

    if (script_line)
        tcb-> script_root = script_line-> node;
    else
      {
        tcb-> script_root = NULL;
        if (tcb-> result_node-> op1)
            report_error (error_event,
                          strprintf ("No loop named '%s'.",
                                     tcb-> result_node-> op1-> value. s));
        else
            report_error (error_event,
                          "No loop.");
      }

    tcb-> script_node = tcb-> script_root;
}

static SCRIPT_NODE_TYPE
go_to_top_of_loop (SCRIPT_LINE **script_line)
{
    Bool
        loop = FALSE;
    SCRIPT_NODE_TYPE
        type = GG_UNDEFINED;

    /*  This loop skips over .else blocks of .for/.else/.endfor loops  */
    while (*script_line
       && (! loop))
      {
        if (block_end_type (*script_line) == GG_ELSE)
            loop = TRUE;

        *script_line = (*script_line)-> loop_start;
        if (*script_line)
          {
            type = block_end_type (*script_line);

            if (type == GG_END_FOR
            ||  type == GG_END_WHILE)
                loop = TRUE;
          }
        else
            type = GG_UNDEFINED;
      }
    return type;
}


static SCRIPT_NODE_TYPE
block_end_type (SCRIPT_LINE *script_line)
{
    return script_line-> block_end ? script_line-> block_end-> node-> type
                                   : GG_UNDEFINED;
}


/*******************   GENERATE BLOCK END NODE TYPE EVENT   ******************/

MODULE generate_block_end_node_type_event (THREAD *thread)
{
    SCRIPT_NODE
        *node;

    tcb = thread-> tcb;                 /*  Point to thread's context        */

    node = tcb-> gsl-> line-> loop_start-> block_end-> node;
    ASSERT (node);
    ASSERT (node-> type <  tblsize (type_event));

    the_next_event = type_event [node-> type];
    if (the_next_event == _LR_NULL_EVENT)
        raise_exception (anomaly_event);
}


/**********************   CONFIRM OP1 NAME IS CORRECT   **********************/

MODULE confirm_op1_name_is_correct (THREAD *thread)
{
    SCOPE_BLOCK
        *scope_block;

    tcb = thread-> tcb;                 /*  Point to thread's context        */

    /*  .end??? [<op1>]  */
    if (! tcb-> result_node-> op1)
        return;

    must_be_defined_and_linear (tcb-> result_node-> op1);
    if (exception_raised)
        return;

    scope_block = last_scope_block (& tcb-> scope_stack);
    if (! streq (scope_block-> name,
                 string_value (& tcb-> result_node-> op1-> value)))
      {
        report_error (error_event,
                      strprintf ("Mismatched scope name '%s'; should be '%s'",
                                 tcb-> result_node-> op1-> value. s,
                                 scope_block-> name));
        return;
      }
}


/****************************   ITERATE FOR LOOP   ***************************/

MODULE iterate_for_loop (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    while (last_scope_block (& tcb-> scope_stack)-> scope_type
        != GG_FOR)
        destroy_scope_block
            (last_scope_block (& tcb-> scope_stack));

    if (! next_scope_item (last_scope_block (& tcb-> scope_stack)))
      {
        do
            skip_block (thread);
        while (tcb-> script_node-> type != GG_END_FOR);
        destroy_scope_block
            (last_scope_block (& tcb-> scope_stack));
      }
}


/*********************   SKIP BLOCK IF CONDITION FALSE   *********************/

MODULE skip_block_if_condition_false (THREAD *thread)
{
    Bool
        where_condition;

    tcb = thread-> tcb;                 /*  Point to thread's context        */

    must_be_defined_and_linear (tcb-> result_node-> op1);
    if (exception_raised)
        return;

    where_condition = (Bool) number_value (& tcb-> result_node-> op1-> value);
    if (! where_condition)
        skip_block (thread);
}


/************************   GENERATE CONDITION EVENT   ***********************/

MODULE generate_condition_event (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    must_be_defined_and_linear (tcb-> result_node-> op1);
    if (exception_raised)
        return;

    if (number_value (& tcb-> result_node-> op1-> value))
        the_next_event = true_event;
    else
        the_next_event = false_event;
}


/**************************   ECHO TEXT TO CONSOLE   *************************/

MODULE echo_text_to_console (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    must_be_defined_and_linear (tcb-> result_node-> op1);
    if (exception_raised)
        return;

    send_smtoper_info (
        &operq,
        strprintf ("%s M: %s",
                   me,
                   tcb-> script_node-> op1
                       ? string_value (& tcb-> result_node-> op1-> value)
                       : ""));
}


/*****************************   COPY THE ITEM   *****************************/

MODULE copy_the_item (THREAD *thread)
{
    CLASS_DESCRIPTOR
        *from_class = NULL,
        *parent_class = NULL,
        *sibling_class = NULL;
    void
        *from_item = NULL,
        *parent_item = NULL,
        *sibling_item = NULL,
        *new_item = NULL;

    tcb = thread-> tcb;                 /*  Point to thread's context        */

    /*  copy <scope> as <as> [to <to> | before <before> | after <after>] */
    must_be_defined_and_linear (tcb-> result_node-> as);
    if (exception_raised)
        return;

    if (tcb-> result_node-> scope)
      {
        from_class = tcb-> result_node-> scope-> value. c;
        from_item  = tcb-> result_node-> scope-> value. i;
      }
    else
        lookup_innermost_item (& from_class, & from_item);

    if (parse_locator (& parent_class,  & parent_item,
                       & sibling_class, & sibling_item, thread))
        return;

    if (from_class-> copy)
        new_item = from_class-> copy (from_item,
                                      sibling_item ? sibling_class : parent_class,
                                      string_result (tcb-> result_root-> as),
                                      parent_item,
                                      sibling_item);

    if (! new_item)
        report_error (error_event,
                      "Unable to copy item: %s of class: %s to class: %s.",
                      extended_scope_string (tcb-> result_node-> scope),
                      from_class-> name,
                      parent_class-> name);

    free_pointer (from_class,    new_item);  /* Not quite right but OK 4 now */
    free_pointer (sibling_class, sibling_item);
    free_pointer (parent_class,  parent_item);
}


/****************************   DELETE THE ITEM   ****************************/

MODULE delete_the_item (THREAD *thread)
{
    CLASS_DESCRIPTOR
        *class = NULL;
    void
        *delete = NULL;

    tcb = thread-> tcb;                 /*  Point to thread's context        */

    /*  .delete <scope>  */

    if (tcb-> result_node-> scope)
      {
        class  = tcb-> result_node-> scope-> value. c;
        delete = tcb-> result_node-> scope-> value. i;
      }
    else
        lookup_innermost_item (& class, & delete);

    if ((!class-> delete)
    ||  (class-> delete (delete)))
        report_error (error_event,
                      "Unable to delete scope: %s of class: %s.",
                      tcb-> result_node-> scope-> value. s,
                      class-> name);
}


/*****************************   MOVE THE ITEM   *****************************/

MODULE move_the_item (THREAD *thread)
{
    CLASS_DESCRIPTOR
        *from_class = NULL,
        *parent_class  = NULL,
        *sibling_class = NULL,
        *scan_class,
        *temp_class;
    void
        *parent_item  = NULL,
        *sibling_item = NULL,
        *from_item = NULL,
        *scan_item,
        *temp_item;

    tcb = thread-> tcb;                 /*  Point to thread's context        */

    /*  .move <scope> as <as> [to <to> | before <before> | after <after>] */

    must_be_defined_and_linear (tcb-> result_node-> as);
    if (exception_raised)
        return;

    if (tcb-> result_node-> scope)
      {
        from_class = tcb-> result_node-> scope-> value. c;
        from_item  = tcb-> result_node-> scope-> value. i;
      }
    else
        lookup_innermost_item (& from_class, & from_item);

    if (tcb-> result_node-> to
    ||  tcb-> result_node-> before
    ||  tcb-> result_node-> after)
      {
        if (parse_locator (& parent_class,  & parent_item,
                           & sibling_class, & sibling_item, thread))
            return;

        scan_item  = parent_item;
        scan_class = parent_class;
        while (scan_item && scan_class-> parent)
          {
            if (scan_item == from_item)
              {
                report_error (error_event,
                              "Attempt to make object its own child.");
                return;
              }
            temp_item  = scan_item;
            temp_class = scan_class;
            temp_class-> parent (temp_item, & scan_class, & scan_item, thread);
            free_pointer (temp_class, temp_item);
          }
        temp_item = NULL;
        if (sibling_item == from_item)       /*  Moving item before itself  */
            from_class-> next_sibling (from_item, NULL, TRUE,
                                       &sibling_class, &sibling_item);
      }

    if ((! from_class-> move)
    ||  (! from_class-> move (from_item,
                              sibling_item ? sibling_class : parent_class,
                              string_result (tcb-> result_node-> as),
                              parent_item,
                              sibling_item)))
        report_error (error_event,
                      "Unable to move item: %s of class: %s to class: %s.",
                      extended_scope_string (tcb-> result_node-> scope),
                      from_class-> name,
                      parent_class-> name);

    free_pointer (sibling_class, sibling_item);
    free_pointer (parent_class,  parent_item);
}


/*****************************   SAVE THE ITEM   *****************************/

MODULE save_the_item (THREAD *thread)
{
    char
        *filename;
    CLASS_DESCRIPTOR
        *class = NULL;
    void
        *item = NULL,
        *new_item,
        *root = NULL;
    XML_ITEM
        *xml_root = NULL,
        *xml_item;
    RESULT_NODE
        *id;

    tcb = thread-> tcb;                 /*  Point to thread's context        */

    /*  .save <scope> [as <as>]  */

    must_be_defined_and_linear (tcb-> result_node-> as);
    if (exception_raised)
        return;

    if (tcb-> result_node-> scope)
      {
        class = tcb-> result_node-> scope-> value. c;
        item  = tcb-> result_node-> scope-> value. i;
      }
    else
        lookup_innermost_item (& class, & item);

    if (class == & XML_item_class)
        xml_item = get_xml_item (item);
    else
    if (! class-> copy)
      {
        report_error (error_event,
                      "Cannot save item of class: %s.", class-> name);
        return;
      }
    else
      {
        /*  Copy item to temporary XML structure.  */
        xml_root = xml_create (NULL, NULL);
        root = get_gsl_xml_item (xml_root);
        XML_item_class. link (root);
        new_item = class-> copy (item,
                                 & XML_item_class,
                                 NULL,
                                 root,
                                 NULL);
        XML_item_class. link    (new_item);
        XML_item_class. destroy (new_item);
        xml_item = xml_first_child (xml_root);
      }

    if (tcb-> script_root-> as)
        filename = string_value (& tcb-> result_node-> as-> value);
    else
      {
        if (tcb-> script_node-> scope-> type == GG_ATTRIBUTE
        ||  tcb-> script_node-> scope-> type == GG_MEMBER)
            id = tcb-> result_node-> scope-> name;
        else
            id = tcb-> result_node-> scope;

        filename = mem_alloc
                       (strlen
                           (string_value
                               (& id-> value)) + 5);
        strcpy (filename, id-> value. s);
        strcat (filename, ".xml");
      }
    xml_save_file (xml_item, filename);

    if (! tcb-> script_root-> as)
        mem_free (filename);

    if (xml_root)
      {
        XML_item_class. destroy (root);
        xml_free (xml_root);
      }
}


/*************************   INITIALISE SCOPE BLOCK   ************************/

MODULE initialise_scope_block (THREAD *thread)
{
    SCOPE_BLOCK
        *scope_block;
    CLASS_DESCRIPTOR
        *class = NULL;
    void
        *item = NULL;
    RESULT_NODE
        *last_name;
    char
        *alias;

    tcb = thread-> tcb;                 /*  Point to thread's context        */

    /*  .scope <scope> as <as>  */

    must_be_defined_and_linear  (tcb-> result_node-> as);
    if (exception_raised)
        return;

    /*  Find the item referred to.  */
    if (tcb-> result_node-> scope)
      {
        class = tcb-> result_node-> scope-> value. c;
        item  = tcb-> result_node-> scope-> value. i;
      }
    else
        lookup_innermost_item (& class, & item);

    if (tcb-> result_node-> as)
      {
        string_value (& tcb-> result_node-> as-> value);
        if (tcb-> result_node-> as-> value. s [0] != 0)
            alias = tcb-> result_node-> as-> value. s;
        else
            alias = NULL;
      }
    else
      {
        /*  Find the last name in the extended scope.  */
        last_name = tcb-> result_node-> scope;
        if (last_name-> script_node-> type == GG_MEMBER
        ||  last_name-> script_node-> type == GG_ATTRIBUTE)
            last_name = last_name-> name;

        alias = string_value (& last_name-> value);
      }

    if ((! alias)
    && (! tcb-> script_node-> stacked))
      {
        report_unaliased_unstacked_scope ();
        return;
      }

    scope_block = create_scope_block (& tcb-> scope_stack,
                                      GG_SCOPE,
                                      alias,
                                      class);
    scope_block-> stacked = tcb-> script_node-> stacked;
    scope_block-> item    = item;

    if (class-> link)
        class-> link (item);
}


static void
report_unaliased_unstacked_scope (void)
{
    report_error (error_event,
                  "Attempt to created an unstacked scope with no alias.");
}


/****************************   CLOSE THE SCOPE   ****************************/

MODULE close_the_scope (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    destroy_scope_block
        (last_scope_block (& tcb-> scope_stack));
}


/**************************   INITIALISE NEW BLOCK   *************************/

MODULE initialise_new_block (THREAD *thread)
{
    SCOPE_BLOCK
        *scope_block;
    CLASS_DESCRIPTOR
        *parent_class,
        *sibling_class,
        *class;
    void
        *parent_item,
        *sibling_item,
        *item;
    char
        *name,
        *alias;

    tcb = thread-> tcb;                 /*  Point to thread's context        */

    /*  .new [[<scope>] .] <name> as <as>
                               [to <to> | before <before> | after <after>]   */

    must_be_defined_and_linear (tcb-> result_node-> name);
    must_be_defined_and_linear (tcb-> result_node-> as);
    if (exception_raised)
        return;

    /*  This is where we fudge the to field by setting it from the scope     */
    /*  field, allowing the alternative forms of the .new statement.         */
    if (tcb-> result_node-> scope && tcb-> result_node-> to)
      {
        report_error (error_event,
                      "Only one of 'scope' and 'to' may be specified.");
        return;
      }
    if (! tcb-> result_node-> to)
      {
        tcb-> result_node-> to    = tcb-> result_node-> scope;
        tcb-> result_node-> scope = NULL;
      }

    if (parse_locator (& parent_class,  & parent_item,
                       & sibling_class, & sibling_item, thread))
        return;

    if (tcb-> script_root-> name)
        name = string_value (& tcb-> result_node-> name-> value);
    else
        name = NULL;

    if (tcb-> result_node-> as)
      {
        string_value (& tcb-> result_node-> as-> value);
        if (tcb-> result_node-> as-> value. s [0] != 0)
            alias = tcb-> result_node-> as-> value. s;
        else
            alias = NULL;
      }
    else
        alias = name;

    if ((! alias)
    && (! tcb-> script_node-> stacked))
      {
        report_unaliased_unstacked_scope ();
        return;
      }

    class = sibling_item ? sibling_class : parent_class;

    if (class-> create
    && (! class-> create (name,
                          parent_item,
                          sibling_item,
                          & class,
                          & item)))
      {
        scope_block = create_scope_block (& tcb-> scope_stack,
                                          GG_NEW,
                                          alias,
                                          class);
        scope_block-> stacked = tcb-> script_node-> stacked;
        scope_block-> item    = item;

        if (class-> link)
            class-> link (item);
      }
    else
      {
        if (parent_item)
            report_error (
                error_event,
                "Unable to create child: %s to item: %s of class: %s.",
                name,
                parent_class-> item_name ? parent_class-> item_name (parent_item)
                                         : "",
                class-> name);
        else
            report_error (
                error_event,
                "Unable to create sibling: %s to item: %s of class: %s.",
                name,
                sibling_class-> item_name ? sibling_class-> item_name (sibling_item)
                                          : "",
                class-> name);
      }
}


/***********************   REGISTER MACRO OR FUNCTION   **********************/

MODULE register_macro_or_function (THREAD *thread)
{
    SCOPE_BLOCK
        *scope_block;
    char
        *error_text;
    CLASS_DESCRIPTOR
        *dummyclass;
    void
        *dummyitem;
    MACRO_ITEM
        *macro;

    tcb = thread-> tcb;                 /*  Point to thread's context        */

    must_be_defined_and_linear (tcb-> result_node-> scope);
    must_be_defined_and_linear (tcb-> result_node-> name);
    if (exception_raised)
        return;

    if (tcb-> script_node-> scope)
      {
        scope_block = lookup_simple_scope (& tcb-> scope_stack,
                                           & tcb-> result_node-> scope-> value,
                                           tcb-> gsl-> ignorecase,
                                           & dummyclass,
                                           & dummyitem,
                                           & error_text);
        if (! scope_block)
          {
            report_error (error_event,
                          "%s", error_text);
            return;
          }
      }
    else
        /*  Use outermost stacked scope block.  */
        scope_block = first_scope_block (& tcb-> scope_stack);

    if (! scope_block-> macros)
      {
        scope_block-> macros = macro_table_create ();
        macro_table_link (scope_block-> macros);
      }

    macro = macro_create (scope_block-> macros,
                          string_result (tcb-> result_node-> name),
                          tcb-> gsl-> line);
}


/***************************   CALL EVALUATE CALL   **************************/

MODULE call_evaluate_call (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    init_result (& tcb-> call_result);
    call_exception (evaluate_call_event);
}


/***************************   PICKUP CALL RESULT   **************************/

MODULE pickup_call_result (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    copy_result (tcb-> result_node, & tcb-> call_result);

    if (tcb-> call_result. value. type == TYPE_UNDEFINED
    && (! tcb-> result_node-> culprit))
      {
        tcb-> result_node-> culprit =
            mem_strdup (strprintf ("%s%s%s (...)",
                        extended_scope_string (tcb-> result_node-> scope),
                        (tcb-> result_node-> scope) ? "." : "",
                        string_value (& tcb-> result_node-> name-> value)));
      }
    /*  Free allocated blocks  */
    mem_free (tcb-> call_result. value. s);
    mem_free (tcb-> call_result. value. b);

    /*  Remove link if value was a pointer  */
    if (tcb-> call_result. value. type == TYPE_POINTER
    &&  tcb-> call_result. value. c-> destroy)
        tcb-> call_result. value. c-> destroy (tcb-> call_result. value. i);

    /*  And reset call result to null  */
    init_result (& tcb-> call_result);
}


/***************************   SAVE RETURN VALUE   ***************************/

MODULE save_return_value (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    if (tcb-> result_node-> op1)
        copy_result (& tcb-> call_result,
                     tcb-> result_node-> op1);
}


/***************************   CLOSE MACRO BLOCK   ***************************/

MODULE close_macro_block (THREAD *thread)
{
    SCOPE_BLOCK
        *scope_block;

    tcb = thread-> tcb;                 /*  Point to thread's context        */

    /*  Clean up any intermediate scopes (after a .return)  */
    scope_block = last_scope_block (& tcb-> scope_stack);
    while (scope_block && scope_block-> scope_type != GG_MACRO)
      {
        destroy_scope_block
            (last_scope_block (& tcb-> scope_stack));
        scope_block = last_scope_block (& tcb-> scope_stack);
      }

    /*  No scope block can only mean return with call.  ggscrp picks up     */
    /*  mismatched .endmacro/.endfunction                                   */
    if (! scope_block)
      {
        report_error (error_event, "Return without call.");
        return;
      }

    /*  And finally destroy the macro block.  */
    destroy_scope_block
        (last_scope_block (& tcb-> scope_stack));
}


/**************************   POP SCRIPT POSITION   **************************/

MODULE pop_script_position (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    pop_the_script (thread);
}


static void
pop_the_script (THREAD *thread)
{
    if (! list_empty (& tcb-> script_stack))
      {
        list_pop (& tcb-> script_stack, tcb-> gsl-> line);
        tcb-> script = tcb-> gsl-> line-> parent;
      }
    else
        raise_exception (script_stack_empty_event);
}


/***************************   PUSH CURRENT NODE   ***************************/

MODULE push_current_node (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

/*    if (tcb-> result_node)  JS 2004/11/14 to make gsl_evaluate work */
    tcb-> result_node-> next = tcb-> node_stack;
    tcb-> node_stack = tcb-> result_node;
}


/**************************   GET FIRST PARAMETER   **************************/

MODULE get_first_parameter (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    while ((tcb-> script_node-> op1)
       &&  (tcb-> script_node-> op1-> type     == GG_OPERATOR)
       &&  (tcb-> script_node-> op1-> operator == OP_NEXT_ARG))
        tcb-> script_node = tcb-> script_node-> op1;

    tcb-> evaluate_node = tcb-> script_node-> op1;

    tcb-> result_node-> item_nbr = 0;

    get_the_parameter ();
}


static void
get_the_parameter ()
{
    GSL_FUNCTION
        *gsl_function;
    int
        parmn;
    static PARM_TYPE parm_type_event [] = {
        value_event,
        reference_event,
        simple_scope_event,
        expression_event };
    PARM_TYPE
        parm_type;

    tcb-> result_node-> script_node = tcb-> script_node;
    gsl_function = tcb-> result_node-> gsl_function;
    if (tcb-> result_node-> item_nbr >= tcb-> result_node-> argc)
        the_next_event = none_event;
    else
      {
        parmn = tcb-> result_node-> item_nbr;
        if (parmn >= gsl_function-> cnt_parmt)
            parmn = gsl_function-> cnt_parmt - 1;
        if (parmn >= 0)
            parm_type = (*gsl_function-> parmt) [parmn];
        else
            parm_type = PARM_VALUE;      /*  No params at all -> assume VALUE */

        if (parm_type >= PARM_VALUE
        &&  parm_type <= PARM_EXPRESSION)
            the_next_event = parm_type_event [parm_type];
        else
            raise_exception (anomaly_event);
      }
}


/*************************   INITIALISE MACRO BLOCK   ************************/

MODULE initialise_macro_block (THREAD *thread)
{
    SCOPE_BLOCK
        *scope_block;
    void
        *symb_item;

    tcb = thread-> tcb;                 /*  Point to thread's context        */

    /* [<scope> .]  <name> [(<op1>)] */

    scope_block = create_scope_block
                      (& tcb-> scope_stack,
                       GG_MACRO,
                       string_value (& tcb-> result_node-> name-> value),
                       NULL);
    scope_block-> stacked = FALSE;      /*  Macro/f'n scopes aren't stacked  */
    symb_class. create (scope_block-> name, NULL, NULL,
                         & scope_block-> class, & scope_block-> item);
    symb_item = scope_block-> item;
    symb_class. link (symb_item);

    /*  Create unstacked alias 'my' for macro scope block.                   */
    scope_block = create_scope_block (& tcb-> scope_stack,
                                      GG_UNDEFINED, "my",
                                      & symb_class);
    scope_block-> stacked = FALSE;
    scope_block-> item    = symb_item;
    symb_class. link (symb_item);

    tcb-> gsl-> line   = ((MACRO_ITEM *) tcb-> result_node-> macro)-> line;
    tcb-> script = tcb-> gsl-> line-> parent;

    if (substitute_params_into_symb (symb_item,
                                     tcb-> gsl-> line-> node-> op1,
                                     tcb-> result_node-> op1,
                                     thread))
        return;
}


static int
substitute_params_into_symb (void *symb_item,
                             SCRIPT_NODE *args,
                             RESULT_NODE *params,
                             THREAD *thread)
{
    SCRIPT_NODE
        *arg = args;
    RESULT_NODE
        *param = params,
        *param_parent = NULL;           /*  Keep in case child is NULL       */
    int
        argn,
        paramn;

    if (! params)
        return 0;

    paramn = 1;
    while (param
       && (param-> script_node-> type     == GG_OPERATOR)
       && (param-> script_node-> operator == OP_NEXT_ARG))
      {
        param_parent = param;
        param = param-> op1;
        paramn++;
      }

    if (! arg)
        argn = 0;
    else
        argn = 1;

    while (arg
       && (arg-> type     == GG_OPERATOR)
       && (arg-> operator == OP_NEXT_ARG))
      {
        arg = arg-> op1;
        argn++;
      }

    /*  Must be at least as many args as params  */
    if (argn < paramn)
      {
        pop_the_script (thread);
        pop_the_result_node ();
        report_error (error_event, "Mismatched parameters.");
        return -1;
      }

    symb_class. put_attr (symb_item,
                          arg
                              ? (char *) arg-> op1         /*  GG_TEXT node  */
                              : NULL,
                          param
                              ? & param-> value
                              : NULL,
                          tcb-> gsl-> ignorecase);
    while (param != params)
      {
        param = param_parent;
        param_parent = param-> parent;
        arg = arg-> parent;

        symb_class. put_attr (symb_item,
                              arg-> op2
                                  ? (char *) arg-> op2-> op1 /* GG_TEXT node */
                                  : NULL,
                              param-> op2
                                  ? & param-> op2-> value
                                  : NULL,
                              tcb-> gsl-> ignorecase);
      }
    return 0;
}


/**************************   BUILD FAKE FOR NODE   *************************/

MODULE build_fake_for_node (THREAD *thread)
{
    int
        argc;
    SCRIPT_NODE
        *item_node,
        *where_node = NULL;
    RESULT_NODE
        *as_node    = NULL;

    tcb = thread-> tcb;                 /*  Point to thread's context        */

    /*  Manually free the function name value - otherwise it gets lost       */
    destroy_result (tcb-> result_node-> name);

    argc = tcb-> result_node-> argc;

    if (argc == 0)
      {
        report_error (error_event, "Missing parameter for function 'count'.");
        return;
      }

    item_node  =            tcb-> result_node-> argv [0]-> script_node;
    where_node = argc > 1 ? tcb-> result_node-> argv [1]-> script_node : NULL;
    if (argc > 2)
      {
        /*  If there was a third, empty argument, create a null string       */
        /*  result.  This will force an unaliased scope.                     */
        as_node = tcb-> result_node-> argv [2];
        if (! as_node)
          {
            as_node = new_result_node ();;
            ASSERT (as_node);
            init_result (as_node);
            as_node-> value. type = TYPE_STRING;
            as_node-> value. s    = mem_strdup ("");
          }
      }
    else
        as_node = NULL;

    if (item_node-> type != GG_SYMBOL)
      {
        report_error (error_event, "Illegal count item.");
        return;
      }

    tcb-> fake_for_node = mem_alloc (sizeof (SCRIPT_NODE));
    ASSERT (tcb-> fake_for_node);
    init_script_node (tcb-> fake_for_node);

    tcb-> fake_for_node-> scope = item_node-> scope;
    tcb-> fake_for_node-> name  = item_node-> name;

    tcb-> fake_for_node-> before  = where_node;
    tcb-> fake_for_node-> stacked = TRUE;

    tcb-> result_node-> as = as_node;

    tcb-> result_node-> script_node = tcb-> fake_for_node;
    tcb-> script_node = tcb-> fake_for_node;
}


/**************************   SAVE TOTAL AS RESULT   *************************/

MODULE save_total_as_result (THREAD *thread)
{
    SCOPE_BLOCK
        *scope_block;

    tcb = thread-> tcb;                 /*  Point to thread's context        */

    scope_block = last_scope_block (& tcb-> scope_stack);

    tcb-> call_result. value. type = TYPE_NUMBER;
    tcb-> call_result. value. n    = scope_block-> total;
}


/*************************   DESTROY FAKE FOR NODE   *************************/

MODULE destroy_fake_for_node (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    mem_free (tcb-> fake_for_node);
    tcb-> fake_for_node = NULL;
}


/************************   EVALUATE SUBSTITUTE NODE   ***********************/

MODULE evaluate_substitute_node (THREAD *thread)
{
    VALUE
        *value;
    char
        format [32],
        conversion,
        *example = NULL;
    int
        length,
        width,
        space;
    SCRIPT_NODE
        *symbol_node,
        *parent;
    RESULT_NODE
        *symbol_result_node;
    SCRIPT_LINE
        *next_line;
    char
        *error_text;

    tcb = thread-> tcb;                 /*  Point to thread's context        */

    /*  'op2' is pretty-print specifier, 'as' is format string  */
    must_be_defined_and_linear (tcb-> result_node-> op2);
    must_be_defined_and_linear (tcb-> result_node-> as);
    if (exception_raised)
        return;

    tcb-> result_node-> constant = tcb-> result_node-> op1   -> constant
        && (tcb-> script_node-> op2 ? tcb-> result_node-> op2-> constant : TRUE)
        && (tcb-> script_node-> as ? tcb-> result_node-> as-> constant : TRUE);

    /*  Convert pointer to linear value if possible  */
    if (tcb-> result_node-> op1-> value. type == TYPE_POINTER)
      {
        if (tcb-> result_node-> op1-> value. c-> get_attr)
          {
            value = tcb-> result_node-> op1-> value. c-> get_attr
                        (tcb-> result_node-> op1-> value. i,
                         NULL,
                         tcb-> gsl-> ignorecase);
            if (value)
                copy_value (& tcb-> result_node-> value, value);
          }
        inherit_culprit (tcb-> result_node-> op1);
      }
    else
        copy_result (tcb-> result_node, tcb-> result_node-> op1);

    if (tcb-> result_node-> value. type == TYPE_UNDEFINED)
      {
        /*  Replaced undefined expression with an empty string in two        */
        /*  specific cases:                                                  */
        /*      1.  $(xxx?)                                                  */
        /*      2.  $(cond??xxx) where cond is FALSE                         */
        if  ((tcb-> script_node-> op1-> type     == GG_OPERATOR)
        && (((tcb-> script_node-> op1-> operator == OP_DEFAULT)
        &&   (tcb-> script_node-> op1-> op2      == NULL))
        ||  ((tcb-> script_node-> op1-> operator == OP_IIF)
        &&  (! number_value (&tcb-> result_node-> op1-> op1-> value)))))
          {
            tcb-> result_node-> value. type = TYPE_UNKNOWN;
            tcb-> result_node-> value. s    = mem_strdup ("");
          }
        else
          {
            inherit_culprit (tcb-> result_node-> op1);
            return;
          }
      }

    /*  Only match the case if ignorecase is TRUE and the expression         */
    /*  consists of a single identifier.                                     */
    if (tcb-> gsl-> ignorecase)
      {
        symbol_node        = tcb-> script_node-> op1;
        symbol_result_node = tcb-> result_node-> op1;
        if ((symbol_node-> type == GG_SYMBOL)
        &&  (symbol_node-> name))
            example = string_value (& symbol_result_node-> name-> value);
      }

    width = parse_format (format, 32, & conversion,
                          tcb-> result_node, &error_text);
    if (width < 0)
      {
        report_error (error_event, "%s", error_text);
        return;
      }
    space = width;

    if (space == 0)
      {
        /*  Calculate the space for this node by adding the length of    */
        /*  the GSL construct to the number of spaces in the next        */
        /*   concatenated node minus the shuffle parameter.              */
        space = tcb-> script_node-> width;

        parent = tcb-> script_node;
        while (parent-> parent
          &&  parent-> parent-> type     == GG_OPERATOR
          &&  parent-> parent-> operator == OP_UNDEFINED
          &&  parent == parent-> parent-> op2)
            parent = parent-> parent;

        if (parent
        &&  parent-> type     == GG_OPERATOR
        &&  parent-> operator == OP_UNDEFINED
        &&  parent-> op2
        &&  parent-> op2-> spaces >= tcb-> gsl-> shuffle)
            space += parent-> op2-> spaces
                  - tcb-> gsl-> shuffle;
        else
        /*  This is a tricky but important case that looks something   */
        /*  like:                                                      */
        /*      $(attr:block)\                                         */
        /*                .                                            */
        if (tcb-> script_node-> extend)
          {
            next_line = tcb-> gsl-> line-> next;
            if (next_line)
              {
                if (next_line-> node-> type == GG_LINE)
                  {
                    space = next_line-> node-> op1-> spaces
                          - tcb-> script_node-> spaces
                          - tcb-> gsl-> shuffle;

                    /*  Check for preceding stuff on the same line          */
                    if (tcb-> script_node-> parent
                    &&  tcb-> script_node-> parent-> type     == GG_OPERATOR
                    &&  tcb-> script_node-> parent-> operator == OP_UNDEFINED
                    &&  tcb-> script_node == tcb-> script_node-> parent-> op2)
                        space -= strlen (string_value
                                    (& tcb-> result_node-> parent-> op1-> value));
                  }
              }
          }
      }

    if (pretty_print (& tcb-> result_node-> value,
                      tcb-> result_node-> op2,
                      example,
                      space,
                      &error_text))
      {
        report_error (error_event, "%s", error_text);
        return;
      }
    length = format_output (format,
                            conversion,
                            width,
                            tcb-> result_node);
    if (length < 0)
      {
        report_error (error_event, "Formatting error");
        return;
      }
}


/*************************   EVALUATE LITERAL NODE   *************************/

MODULE evaluate_literal_node (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    tcb-> result_node-> constant = tcb-> result_node-> op1-> constant;
    if (tcb-> result_node-> op1-> value. type != TYPE_UNDEFINED
    &&  tcb-> result_node-> op1-> value. type != TYPE_POINTER)
      {
        string_value (& tcb-> result_node-> op1-> value);
        tcb-> result_node-> value. type = TYPE_STRING;
        tcb-> result_node-> value. s    = mem_alloc
             (tcb-> result_node-> op1-> indent +
                  strlen (tcb-> result_node-> op1-> value. s) + 1);
        memset (tcb-> result_node-> value. s, ' ',
                tcb-> result_node-> op1-> indent);
        strcpy (tcb-> result_node-> value. s
              + tcb-> result_node-> op1-> indent,
                tcb-> result_node-> op1-> value. s);
      }
    else
        inherit_culprit (tcb-> result_node-> op1);
}


/***************************   CALL EVALUATE OP2   ***************************/

MODULE call_evaluate_op2 (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    if (tcb-> script_node-> op2)
      {
        tcb-> evaluate_node =   tcb-> script_node-> op2;
        tcb-> result_ptr    = & tcb-> result_node-> op2;
        call_exception (evaluate_event);
      }
}


/**************************   EVALUATE NUMBER NODE   *************************/

MODULE evaluate_number_node (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    tcb-> result_node-> constant =
        (tcb-> result_node-> op1 ? tcb-> result_node-> op1-> constant : TRUE)
     && (tcb-> result_node-> op2 ? tcb-> result_node-> op2-> constant : TRUE);

    tcb-> result_node-> value. s = mem_alloc (
        (tcb-> result_node-> op1 ? strlen (tcb-> result_node-> op1-> value. s) : 1)
      + (tcb-> script_node-> operator == OP_MINUS ? 1 : 0)
      + (tcb-> result_node-> op2 ? strlen (tcb-> result_node-> op2-> value. s) + 1 : 0)
      + 1);

    ASSERT (tcb-> result_node-> value. s);
    tcb-> result_node-> value. s [0] = '\0';
    if (tcb-> script_node-> operator == OP_MINUS)
        strcat (tcb-> result_node-> value. s, "-");
    strcat (tcb-> result_node-> value. s,
            tcb-> result_node-> op1 ? tcb-> result_node-> op1-> value. s : "0");

    if (tcb-> script_node-> op2)
      {
        strcat (tcb-> result_node-> value. s, ".");
        strcat (tcb-> result_node-> value. s,
                tcb-> result_node-> op2-> value. s);
      }
    tcb-> result_node-> value. type = TYPE_UNKNOWN;

    number_value (& tcb-> result_node-> value);
}


/**************************   EVALUATE SYMBOL NODE   *************************/

MODULE evaluate_symbol_node (THREAD *thread)
{
    char
        *error_text;
    VALUE
        *value;

    tcb = thread-> tcb;                 /*  Point to thread's context        */

    if (tcb-> result_node-> scope
    &&  tcb-> result_node-> scope-> value. type == TYPE_UNDEFINED)
      {
        inherit_culprit (tcb-> result_node-> scope);
        return;
      }

    value = symbol_value (& tcb-> scope_stack,
                          tcb-> result_node,
                          tcb-> gsl-> ignorecase,
                          &error_text);

    /*  Use copy_value so that link will get made if value is a pointer  */
    if (value)
        copy_value (& tcb-> result_node-> value,
                    value);

    if (error_text)
        report_error (error_event, "%s", error_text);

    if ((! value)
    ||  value-> type == TYPE_UNDEFINED)
        tcb-> result_node-> culprit =
            mem_strdup (name_the_symbol (tcb-> result_node));
}


/**************************   EVALUATE MEMBER NODE   *************************/

MODULE evaluate_member_node (THREAD *thread)
{
    SCOPE_BLOCK
        *scope_block;

    tcb = thread-> tcb;                 /*  Point to thread's context        */

    scope_block = last_scope_block (& tcb-> scope_stack);
    first_scope_item (scope_block);
    if (scope_block-> item)
      {
        tcb-> result_node-> value. type = TYPE_POINTER;
        tcb-> result_node-> value. c    = scope_block-> class;
        tcb-> result_node-> value. i    = scope_block-> item;
        if (tcb-> result_node-> value. c-> link)
            tcb-> result_node-> value. c-> link (tcb-> result_node-> value. i);
      }
    else
        tcb-> result_node-> culprit =
            mem_strdup (extended_scope_string (tcb-> result_node));
}


/***************************   EVALUATE TEXT NODE   **************************/

MODULE evaluate_text_node (THREAD *thread)
{
    int
        length,
        chunklen;
    char
        *in,
        *ptr,
        *out,
        *sign;

    tcb = thread-> tcb;                 /*  Point to thread's context        */

    sign = operator_text (tcb-> script_node-> operator);

    /*  Count total string length  */
    length = strlen (sign);
    in = (char *) tcb-> script_node-> op1;
    while (in)
      {
        ptr = strchr (in, '\\');
        if (ptr)
          {
            length += ptr - in + 1;
            ptr += 2;
          }
        else
            length += strlen (in);

        in = ptr;
      }

    tcb-> result_node-> value. type = TYPE_UNKNOWN;
    tcb-> result_node-> value. s = mem_alloc (length + 1);

    strcpy (tcb-> result_node-> value. s, sign);
    out = tcb-> result_node-> value. s + strlen (sign);

    in = (char *) tcb-> script_node-> op1;
    while (in)
      {
        ptr = strchr (in, '\\');
        if (ptr)
          {
            memcpy (out, in, ptr - in);
            out += ptr++ - in;
            if (*ptr == 'n')
                *out++ = '\n';
            else
            if (*ptr == 'r')
                *out++ = '\r';
            else
            if (*ptr == 't')
                *out++ = '\t';
            else
                *out++ = *ptr;

            ptr++;
          }
        else
          {
            chunklen = strlen (in);
            memcpy (out, in, chunklen);
            out += chunklen;
          }
        in = ptr;
      }

    *out = 0;

    tcb-> result_node-> constant = TRUE;
}


/********************   DEFINE SYMBOL AS NODE OP2 RESULT   *******************/

MODULE define_symbol_as_node_op2_result (THREAD *thread)
{
    char
        *error_text;

    tcb = thread-> tcb;                 /*  Point to thread's context        */

    /*  An undefined result gives an error except in three specific cases:   */
    /*      1. '.define x ='                                                 */
    /*      2. '.define x = y ?'                                             */
    /*      2. '.define x ?= y ?'                                            */
    if ((tcb-> script_root-> op2)                           /*  Case 1      */
    &&  (tcb-> result_node-> op2-> value. type == TYPE_UNDEFINED)
    &&  ! (((tcb-> script_root-> operator == OP_UNDEFINED)  /*  Case 2      */
        ||  (tcb-> script_root-> operator == OP_DEFAULT))   /*  Case 3      */
        && ((tcb-> script_root-> op2-> type     == GG_OPERATOR)
        &&  (tcb-> script_root-> op2-> operator == OP_DEFAULT)
        &&  (tcb-> script_root-> op2-> op2      == NULL)) ))
      {
        report_undefined_expression (tcb-> result_node-> op2);
        return;
      }

    if (! store_symbol_definition (& tcb-> scope_stack,
                                   tcb-> gsl-> ignorecase,
                                   tcb-> result_node,
                                   tcb-> result_node-> op2
                                     ? & tcb-> result_node-> op2-> value
                                       : NULL,
                                   & error_text))
        report_error (error_event, error_text);
}


static void
report_undefined_expression (RESULT_NODE *node)
{
    char
        *error_text;

    result_node_error ("Undefined expression", node, & error_text);
    report_error (error_event, error_text);
}


/***********************   PREPARE DEFINE EXPRESSION   ***********************/

MODULE prepare_define_expression (THREAD *thread)
{
    RESULT_NODE
        *new_node;
    char
        *error_text;
    VALUE
        *value;

    tcb = thread-> tcb;                 /*  Point to thread's context        */

    /*  <scope>.<name> [operator]= <op2>  */

    value = symbol_value (& tcb-> scope_stack,
                          tcb-> result_node,
                          tcb-> gsl-> ignorecase,
                          &error_text);

    if (error_text)
      {
        report_error (error_event, "%s", error_text);
        return;
      }

    new_node = new_result_node ();;
    ASSERT (new_node);
    init_result (new_node);

    /*  Tie new node into structure so it'll get freed properly  */
    new_node-> parent = tcb-> result_node;
    tcb-> result_node-> op1 = new_node;

    init_value (& new_node-> value);
    if (value)
        copy_value (& new_node-> value,
                    value);

    if ((! value)
    ||  value-> type == TYPE_UNDEFINED)
        new_node-> culprit = mem_strdup (name_the_symbol (tcb-> result_node));

    new_node-> constant = TRUE;   /* We already have the result */
}


/**********************   EVALUATE ARITHMETIC OPERATOR   *********************/

MODULE evaluate_arithmetic_operator (THREAD *thread)
{
    int
        length;
    long
        i;

    tcb = thread-> tcb;                 /*  Point to thread's context        */

    tcb-> result_node-> constant =
        (tcb-> result_node-> op1 ? tcb-> result_node-> op1-> constant : TRUE)
     && (tcb-> result_node-> op2 ? tcb-> result_node-> op2-> constant : TRUE);

    if (tcb-> result_node-> op1)
      {
        if (tcb-> result_node-> op1-> value. type == TYPE_UNDEFINED)
          {
            inherit_culprit (tcb-> result_node-> op1);
            return;
          }
      }

    if (tcb-> result_node-> op2-> value. type == TYPE_UNDEFINED)
      {
        inherit_culprit (tcb-> result_node-> op2);
        return;
      }

    if (tcb-> result_node-> op1)
      {
        /*  Binary operators  */

        /*  Handle PLUS to catch "x" + "y" = "xy"  */
        if (tcb-> script_node-> operator == OP_PLUS)
          {
            /*  If one operand is non-string and the other is unknown,       */
            /*  try to make both numeric.                                    */
            if ((tcb-> result_node-> op1-> value. type != TYPE_STRING)
            &&  (tcb-> result_node-> op2-> value. type == TYPE_UNKNOWN))
                number_value (& tcb-> result_node-> op2-> value);

            if ((tcb-> result_node-> op2-> value. type != TYPE_STRING)
            &&  (tcb-> result_node-> op1-> value. type == TYPE_UNKNOWN))
                number_value (& tcb-> result_node-> op1-> value);

            if ((tcb-> result_node-> op1-> value. type == TYPE_NUMBER)
            &&  (tcb-> result_node-> op2-> value. type == TYPE_NUMBER))
              {
                tcb-> result_node-> value. n
                    = tcb-> result_node-> op1-> value. n
                    + tcb-> result_node-> op2-> value. n;
                tcb-> result_node-> value. type = TYPE_NUMBER;
              }
            else
              {
                /*  Convert both to strings then concatenate.  */
                string_value (& tcb-> result_node-> op1-> value);
                string_value (& tcb-> result_node-> op2-> value);

                length = strlen (tcb-> result_node-> op1-> value. s)
                       + strlen (tcb-> result_node-> op2-> value. s);

                tcb-> result_node-> value. s = mem_alloc (length + 1);
                ASSERT (tcb-> result_node-> value. s);
                strcpy (tcb-> result_node->       value. s,
                        tcb-> result_node-> op1-> value. s);
                strcat (tcb-> result_node->       value. s,
                        tcb-> result_node-> op2-> value. s);

                tcb-> result_node-> value. type = TYPE_STRING;
              }
          }
        else
        /*  Handle TIMES to catch "x" * 3 = "xxx"  */
        if (tcb-> script_node-> operator == OP_TIMES)
          {
            /*  2nd operand must be numeric.  */
            number_value (& tcb-> result_node-> op2-> value);
            if (tcb-> result_node-> op2-> value. type != TYPE_NUMBER)
              {
                report_non_numeric_error (tcb-> result_node-> op2);
                return;
              }

            /*  Try to make 1st operand numeric, unless explicitly string.   */
            if (tcb-> result_node-> op1-> value. type != TYPE_STRING)
                number_value (& tcb-> result_node-> op1-> value);

            if (tcb-> result_node-> op1-> value. type == TYPE_STRING)
              {
                length = strlen (string_value (& tcb-> result_node-> op1->
value));                i = (long) tcb-> result_node-> op2-> value. n;    /*
Integer */                if (i < 0)
                  {
                    report_error (error_event,
                                  "String repeat count is negative.");
                    return;
                  }

                tcb-> result_node-> value. s = mem_alloc (length * i + 1);
                tcb-> result_node-> value. s [0] = '\0';
                ASSERT (tcb-> result_node-> value. s);
                while (i-- > 0)
                    strcat (tcb-> result_node-> value. s, tcb-> result_node->
op1-> value. s);                tcb-> result_node-> value. type = TYPE_STRING;
              }
            else
              {
                tcb-> result_node-> value. n
                    = tcb-> result_node-> op1-> value. n
                    * tcb-> result_node-> op2-> value. n;
                tcb-> result_node-> value. type = TYPE_NUMBER;
              }
          }
        else
          {
            /*  Other operators besides '*', '+'  */

            /*  Operators must be numeric.  */
            number_value (& tcb-> result_node-> op1-> value);
            if (tcb-> result_node-> op1-> value. type != TYPE_NUMBER)
              {
                report_non_numeric_error (tcb-> result_node-> op1);
                return;
              }

            number_value (& tcb-> result_node-> op2-> value);
            if (tcb-> result_node-> op2-> value. type != TYPE_NUMBER)
              {
                report_non_numeric_error (tcb-> result_node-> op2);
                return;
              }

            switch (tcb-> script_node-> operator)
              {
                case OP_MINUS:
                    tcb-> result_node-> value. n
                        = tcb-> result_node-> op1-> value. n
                        - tcb-> result_node-> op2-> value. n;
                    break;

                case OP_DIVIDE:
                    if (tcb-> result_node-> op2-> value. n != 0)
                        tcb-> result_node-> value. n = tcb-> result_node-> op1->
value. n                                               / tcb-> result_node->
op2-> value. n;                    else
                      {
                        report_error (error_event, "Division by zero.");
                        return;
                      }
                    break;

                case OP_OR:
                    tcb-> result_node-> value. n
                        = (double) ((Bool) tcb-> result_node-> op1-> value. n
                        ||          (Bool) tcb-> result_node-> op2-> value. n);
                    break;

                case OP_AND:
                    tcb-> result_node-> value. n
                        = (double) ((Bool) tcb-> result_node-> op1-> value. n
                        &&          (Bool) tcb-> result_node-> op2-> value. n);
                    break;

                default:
                    raise_exception (anomaly_event);
                    return;
              }
            tcb-> result_node-> value. type = TYPE_NUMBER;
          }
      }
    else
      {
        /*  Unary operators  */
        number_value (& tcb-> result_node-> op2-> value);
        if (tcb-> result_node-> op2-> value. type != TYPE_NUMBER)
          {
            report_non_numeric_error (tcb-> result_node-> op2);
            return;
          }
        switch (tcb-> script_node-> operator)
          {
            case OP_NOT:
                tcb-> result_node-> value. n
                    = (double) ! (Bool) tcb-> result_node-> op2-> value. n;
                break;

            case OP_PLUS:
                tcb-> result_node-> value. n
                    = tcb-> result_node-> op2-> value. n;
                break;

            case OP_MINUS:
                tcb-> result_node-> value. n
                    = - tcb-> result_node-> op2-> value. n;
                break;

            default:
                raise_exception (anomaly_event);
                return;
          }
        tcb-> result_node-> value. type = TYPE_NUMBER;
      }
}


static void
report_non_numeric_error (RESULT_NODE *node)
{
    report_error (error_event, "Illegal numeric operand: %s",
                  string_value (& node-> value));
}


/**********************   DEFINE SYMBOL AS NODE RESULT   *********************/

MODULE define_symbol_as_node_result (THREAD *thread)
{
    char
        *error_text;

    tcb = thread-> tcb;                 /*  Point to thread's context        */

    must_be_defined_and_linear (tcb-> result_node-> name);
    must_be_defined_and_linear (tcb-> result_node);
    if (exception_raised)
        return;

    if (! store_symbol_definition (& tcb-> scope_stack,
                                   tcb-> gsl-> ignorecase,
                                   tcb-> result_node,
                                   & tcb-> result_node-> value,
                                   & error_text))
        report_error (error_event, error_text);
}


/******************   RAISE EXCEPTION IF SYMBOL IS DEFINED   *****************/

MODULE raise_exception_if_symbol_is_defined (THREAD *thread)
{
    CLASS_DESCRIPTOR
        *class;
    void
        *parent;
    char
        *error_text;
    VALUE
        *value;

    tcb = thread-> tcb;                 /*  Point to thread's context        */

    must_be_defined_and_linear (tcb-> result_node-> name);
    if (exception_raised)
        return;

    symbol_item (& tcb-> scope_stack,
                 tcb-> result_node,
                 tcb-> gsl-> ignorecase,
                 & class,
                 & parent,
                 &error_text);
    if (class
    &&  class-> get_attr)
      {
        value = class-> get_attr
                    (parent,
                     string_value (& tcb-> result_node-> name-> value),
                     tcb-> gsl-> ignorecase);
        if (value && value-> type != TYPE_UNDEFINED)
            raise_exception (exception_event);
      }
}


/**********************   REJECT IF WHERE CLAUSE FALSE   *********************/

MODULE reject_if_where_clause_false (THREAD *thread)
{
    Bool
        where_condition = TRUE;
    SCOPE_BLOCK
        *scope_block;

    tcb = thread-> tcb;                 /*  Point to thread's context        */

    scope_block = last_scope_block (& tcb-> scope_stack);

    must_be_defined_and_linear (tcb-> result_node-> before);
    if (exception_raised)
        return;

    if (tcb-> script_node-> before)
      {
        where_condition = (Bool) number_value (& tcb-> result_node-> before->
value);
        destroy_result (tcb-> result_node-> before);
        tcb-> result_node-> before = NULL;

        if (! where_condition)
          {
            raise_exception (reject_event);
            return;
          }
      }
    scope_block-> total++;
}


/**************************   SAVE SORT KEY VALUE   **************************/

MODULE save_sort_key_value (THREAD *thread)
{
    SCOPE_BLOCK
        *scope_block;

    tcb = thread-> tcb;                 /*  Point to thread's context        */

    scope_block = last_scope_block (& tcb-> scope_stack);

    must_be_defined_and_linear (tcb-> result_node-> after);
    if (exception_raised)
        return;

    if (tcb-> script_node-> after)
      {
        /*  If type is unknown, see if it can be made a number.              */
        if ((tcb-> result_node-> after-> value. type == TYPE_UNKNOWN)
        &&  (tcb-> sort_type                         == TYPE_UNKNOWN))
            number_value (& tcb-> result_node-> after-> value);
        else
            string_value (& tcb-> result_node-> after-> value);

        /*  If one sort key is a string, all must be treated as strings.     */
        if (tcb-> result_node-> after-> value. type != TYPE_NUMBER)
            tcb-> sort_type = TYPE_STRING;

        copy_value (& scope_block-> scope_item-> sort_key,
                    & tcb-> result_node-> after-> value);

        destroy_result (tcb-> result_node-> after);
        tcb-> result_node-> after = NULL;
      }
}


/*****************************   GET NEXT CHILD   ****************************/

MODULE get_next_child (THREAD *thread)
{
    SCOPE_BLOCK
        *scope_block;
    const char
        *name;
    CLASS_DESCRIPTOR
        *class;
    void
        *item;

    tcb = thread-> tcb;                 /*  Point to thread's context        */

    scope_block = last_scope_block (& tcb-> scope_stack);
    name = string_result (tcb-> result_node-> name);

    if (scope_block-> scope_item-> class-> next_sibling)
      {
        class = NULL;
        item  = NULL;
        scope_block-> scope_item-> class-> next_sibling
            (  scope_block-> item,
               name, tcb-> gsl-> ignorecase,
             & class,
             & item);
        scope_block-> class = class;
        scope_block-> item  = item;
      }
    else
        scope_block-> item = NULL;

    create_scope_item_from_item (scope_block);
}


/************************   KILL PREVIOUS SCOPE ITEM   ***********************/

MODULE kill_previous_scope_item (THREAD *thread)
{
    SCOPE_BLOCK
        *scope_block;
    SCOPE_ITEM
        *scope_item;

    tcb = thread-> tcb;                 /*  Point to thread's context        */

    scope_block = last_scope_block (& tcb-> scope_stack);
    if (scope_block-> scope_item)
        scope_item = scope_block-> scope_item-> prev;
    else
        scope_item = (SCOPE_ITEM *) scope_block-> item_list. prev;

    destroy_scope_item (scope_item);
}


/**********************   CALL EVALUATE THE PARAMETER   **********************/

MODULE call_evaluate_the_parameter (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    tcb-> result_ptr = & tcb-> result_node-> argv [tcb-> result_node-> item_nbr];
    if (tcb-> evaluate_node)
        call_exception (evaluate_event);
}


/*****************   CALL EVALUATE THE REFERENCE PARAMETER   *****************/

MODULE call_evaluate_the_reference_parameter (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    tcb-> result_ptr = & tcb-> result_node-> argv [tcb-> result_node-> item_nbr];
    if (tcb-> evaluate_node)
        call_exception (evaluate_reference_event);
}


/************************   INSERT NULL RESULT NODE   ************************/

MODULE insert_null_result_node (THREAD *thread)
{
    RESULT_NODE
        *new_node;

    tcb = thread-> tcb;                 /*  Point to thread's context        */

    new_node = new_result_node ();;
    ASSERT (new_node);
    init_result (new_node);

    new_node-> parent      = tcb-> result_node;
    new_node-> script_node = tcb-> evaluate_node;

    tcb-> result_node-> argv [tcb-> result_node-> item_nbr] = new_node;
}


/****************   CALL EVALUATE THE SIMPLE SCOPE PARAMETER   ***************/

MODULE call_evaluate_the_simple_scope_parameter (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    tcb-> result_ptr = & tcb-> result_node-> argv [tcb-> result_node-> item_nbr];
    if (tcb-> evaluate_node)
        call_exception (evaluate_simple_scope_event);
}


/***************************   GET NEXT PARAMETER   **************************/

MODULE get_next_parameter (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    tcb-> result_node-> item_nbr++;
    if (tcb-> result_node-> item_nbr < tcb-> result_node-> argc)
      {
        tcb-> evaluate_node = tcb-> script_node-> op2;
        tcb-> script_node   = tcb-> script_node-> parent;
      }

    get_the_parameter ();
}


/***********************   EVALUATE CALL METHOD NODE   ***********************/

MODULE evaluate_call_method_node (THREAD *thread)
{
    int
        rc;

    tcb = thread-> tcb;                 /*  Point to thread's context        */

    /*  Clean up error message just in case */
    object_error [0] = '\0';
    rc = tcb-> result_node-> gsl_function-> evaluate
                                               (  tcb-> result_node-> argc,
                                                  tcb-> result_node-> argv,
                                                  tcb-> item,
                                                & tcb-> call_result,
                                                thread);

    /*  Set tcb again as gsl function may have changed it.  Ugly */
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    if ((! rc) && tcb-> result_node-> gsl_function-> immediate)
        the_next_event = call_ok_event;

    if (rc)
      {
        if (object_error [0] == '\0')
            sprintf (object_error, "Error calling function: %s",
                                   tcb-> result_node-> gsl_function-> name);
        report_error (error_event, object_error);
      }
}


/***************************   LOG OBJECT MESSAGE   **************************/

MODULE log_object_message (THREAD *thread)
{
    struct_ggcode_call_error
        *error;

    tcb = thread-> tcb;                 /*  Point to thread's context        */

    get_ggcode_call_error (thread-> event-> body, & error);

    if (error-> error_text)
        send_ggcode_message (& tcb-> replyq-> qid,
                             tcb-> job,
                             error-> error_name ? error-> error_name
                                                : gsl_cur_script (thread),
                             error-> error_name ? error-> error_line
                                                : gsl_cur_line (thread),
                             error-> error_text);

    free_ggcode_call_error (& error);
}


/****************************   LOG OBJECT ERROR   ***************************/

MODULE log_object_error (THREAD *thread)
{
    struct_ggcode_call_error
        *error;

    tcb = thread-> tcb;                 /*  Point to thread's context        */

    get_ggcode_call_error (thread-> event-> body, & error);

    if (error-> error_text)
        send_ggcode_error (& tcb-> replyq-> qid,
                           tcb-> job,
                           error-> error_name ? error-> error_name
                                              : gsl_cur_script (thread),
                           error-> error_name ? error-> error_line
                                              : gsl_cur_line (thread),
                           error-> error_text);
    else
        send_ggcode_error (& tcb-> replyq-> qid,
                           tcb-> job,
                           NULL, 0, NULL);

    free_ggcode_call_error (& error);
}


/************************   REPORT ILLEGAL PARAMETER   ***********************/

MODULE report_illegal_parameter (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    /*  We pop the node to get the correct location for the error message    */
    /*  but then re-push in order to keep things in place for the dialog.    */
    pop_current_node (thread);
    report_error (error_event, "Illegal parameter %i to function %s%s%s",
                  tcb-> result_node-> item_nbr + 1,
                  tcb-> script_node-> scope
                      ? tcb-> result_node-> scope-> value.s : "",
                  tcb-> script_node-> scope
                      ? "." : "",
                  tcb-> result_node-> name-> value.s);
    push_current_node (thread);
}


/*************************   COPY RESULT FROM NAME   *************************/

MODULE copy_result_from_name (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    copy_result (tcb-> result_node,
                 tcb-> result_node-> name);
}


/************************   NODE SCOPE MUST BE NULL   ************************/

MODULE node_scope_must_be_null (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    if (tcb-> script_node-> scope)
        report_illegal_parameter (thread);
}


/*************************   NODE OP2 MUST BE NULL   *************************/

MODULE node_op2_must_be_null (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    if (tcb-> script_node-> op2)
        report_illegal_parameter (thread);
}


/**************************   CONCATENATE OPERANDS   *************************/

MODULE concatenate_operands (THREAD *thread)
{
    char
        *error_text;

    tcb = thread-> tcb;                 /*  Point to thread's context        */

    inherit_culprit (  tcb-> result_node-> op1-> culprit
                     ? tcb-> result_node-> op1
                     : tcb-> result_node-> op2);
    /*  Concatenation is delayed until all operands to be concatenated have */
    /*  been evaluated.  This is needed for blocks to be concatenated.      */
    /*  We don't evaluate if this is a template line either, done in        */
    /*  copy_line_to_output instead.                                        */
    if ( (! (tcb-> script_node-> parent-> type     == GG_OPERATOR
    &&       tcb-> script_node-> parent-> operator == OP_UNDEFINED) )
    &&      (tcb-> script_node-> parent-> type != GG_LINE) )
      {
        tcb-> result_node-> value. s = concatenate_results (tcb-> result_node,
                                                            tcb-> gsl-> shuffle,
                                                            FALSE,
                                                            &error_text);
        if (tcb-> result_node-> value. s)
          {
            tcb-> result_node-> value. type = TYPE_STRING;
            /*  Result is constant if both operands are constant, plus 2nd   */
            /*  operand has no spaces.  Otherwise shuffle may hit us.        */
            tcb-> result_node-> constant
                = (tcb-> result_node-> op1-> constant
                && (tcb-> result_node-> op2-> script_node-> spaces == 0)
                &&  tcb-> result_node-> op2-> constant);
          }
      }
}


/*************************   RETURN OP1 IF DEFINED   *************************/

MODULE return_op1_if_defined (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    if (tcb-> result_node-> op1-> value. type != TYPE_UNDEFINED)
      {
        tcb-> result_node-> constant = tcb-> result_node-> op1-> constant;

        copy_result (tcb-> result_node, tcb-> result_node-> op1);
        raise_exception (dialog_return_event);
      }
}


/*******************************   RETURN OP2   ******************************/

MODULE return_op2 (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    tcb-> result_node-> constant =
        (tcb-> result_node-> op1 ? tcb-> result_node-> op1-> constant : TRUE)
     && (tcb-> result_node-> op2 ? tcb-> result_node-> op2-> constant : TRUE);
    copy_result (tcb-> result_node, tcb-> result_node-> op2);
    raise_exception (dialog_return_event);
}


/**********************   EVALUATE COMPARISON OPERATOR   *********************/

MODULE evaluate_comparison_operator (THREAD *thread)
{
    int
        cmp;

    tcb = thread-> tcb;                 /*  Point to thread's context        */

    /*  Deal with special case of undefined operands.  */
    if (tcb-> result_node-> op1-> value. type == TYPE_UNDEFINED
    ||  tcb-> result_node-> op2-> value. type == TYPE_UNDEFINED)
        /*  Both undefined is a special case.  */
        if (tcb-> result_node-> op1-> value. type == TYPE_UNDEFINED
        &&  tcb-> result_node-> op2-> value. type == TYPE_UNDEFINED)
            switch (tcb-> script_node-> operator)
              {
                case OP_SAFE_EQUALS:
                    tcb-> result_node-> value. n = 1;
                    break;
                case OP_SAFE_NOT_EQUALS:
                case OP_SAFE_GREATER_THAN:
                case OP_SAFE_LESS_THAN:
                case OP_SAFE_GREATER_EQUAL:
                case OP_SAFE_LESS_EQUAL:
                    tcb-> result_node-> value. n = 0;
                    break;
                default:
                    inherit_culprit (tcb-> result_node-> op1);
                    return;
              }
        else
            switch (tcb-> script_node-> operator)
              {
                case OP_SAFE_EQUALS:
                case OP_SAFE_NOT_EQUALS:
                case OP_SAFE_GREATER_THAN:
                case OP_SAFE_LESS_THAN:
                case OP_SAFE_GREATER_EQUAL:
                case OP_SAFE_LESS_EQUAL:
                    tcb-> result_node-> value. n = 0;
                    break;
                default:
                    inherit_culprit (
                        tcb-> result_node-> op1-> value. type == TYPE_UNDEFINED
                            ? tcb-> result_node-> op1
                            : tcb-> result_node-> op2);
                    return;
              }
    else
    /*  If both operands are pointers, equals and not-equals return result.  */
    if (tcb-> result_node-> op1-> value. type == TYPE_POINTER
    &&  tcb-> result_node-> op2-> value. type == TYPE_POINTER)
        if (tcb-> script_node-> operator == OP_EQUALS
        ||  tcb-> script_node-> operator == OP_SAFE_EQUALS)
            tcb-> result_node-> value. n
                = (tcb-> result_node-> op1-> value. i
                == tcb-> result_node-> op2-> value. i);
        else
        if (tcb-> script_node-> operator == OP_NOT_EQUALS
        ||  tcb-> script_node-> operator == OP_SAFE_NOT_EQUALS)
            tcb-> result_node-> value. n
                = (tcb-> result_node-> op1-> value. i
                != tcb-> result_node-> op2-> value. i);
        else
            return;
    else
    /*  If both operands are non-pointers, evaluate according to type.       */
    if (tcb-> result_node-> op1-> value. type != TYPE_POINTER
    &&  tcb-> result_node-> op2-> value. type != TYPE_POINTER)
      {
        /*  If one operand is non-string and the other is unknown,           */
        /*  try to make both numeric.                                        */
        if ((tcb-> result_node-> op1-> value. type != TYPE_STRING)
        &&  (tcb-> result_node-> op2-> value. type == TYPE_UNKNOWN))
            number_value (& tcb-> result_node-> op2-> value);

        if ((tcb-> result_node-> op2-> value. type != TYPE_STRING)
        &&  (tcb-> result_node-> op1-> value. type == TYPE_UNKNOWN))
            number_value (& tcb-> result_node-> op1-> value);

        /*  Handle numeric operators  */
        if ((tcb-> result_node-> op1-> value. type == TYPE_NUMBER)
        &&  (tcb-> result_node-> op2-> value. type == TYPE_NUMBER))
          {
            switch (tcb-> script_node-> operator)
              {
                case OP_EQUALS:
                case OP_SAFE_EQUALS:
                    tcb-> result_node-> value. n
                        = (tcb-> result_node-> op1-> value. n
                        == tcb-> result_node-> op2-> value. n);
                    break;

                case OP_NOT_EQUALS:
                case OP_SAFE_NOT_EQUALS:
                    tcb-> result_node-> value. n
                        = (tcb-> result_node-> op1-> value. n
                        != tcb-> result_node-> op2-> value. n);
                    break;

                case OP_GREATER_THAN:
                case OP_SAFE_GREATER_THAN:
                    tcb-> result_node-> value. n
                        = (tcb-> result_node-> op1-> value. n
                        >  tcb-> result_node-> op2-> value. n);
                    break;

                case OP_LESS_THAN:
                case OP_SAFE_LESS_THAN:
                    tcb-> result_node-> value. n
                        = (tcb-> result_node-> op1-> value. n
                        <  tcb-> result_node-> op2-> value. n);
                    break;

                case OP_GREATER_EQUAL:
                case OP_SAFE_GREATER_EQUAL:
                    tcb-> result_node-> value. n
                        = (tcb-> result_node-> op1-> value. n
                        >= tcb-> result_node-> op2-> value. n);
                    break;

                case OP_LESS_EQUAL:
                case OP_SAFE_LESS_EQUAL:
                    tcb-> result_node-> value. n
                        = (tcb-> result_node-> op1-> value. n
                        <= tcb-> result_node-> op2-> value. n);
                    break;

                default:
                    raise_exception (anomaly_event);
                    return;
              }
          }
        else
          /*  Handle non-numeric operators  */
          {
            cmp = strcmp (string_value (& tcb-> result_node-> op1-> value),
                          string_value (& tcb-> result_node-> op2-> value));
            switch (tcb-> script_node-> operator)
              {
                case OP_EQUALS:
                case OP_SAFE_EQUALS:
                    tcb-> result_node-> value. n = (cmp == 0);
                    break;

                case OP_NOT_EQUALS:
                case OP_SAFE_NOT_EQUALS:
                    tcb-> result_node-> value. n = (cmp != 0);
                    break;

                case OP_GREATER_THAN:
                case OP_SAFE_GREATER_THAN:
                    tcb-> result_node-> value. n = (cmp >  0);
                    break;

                case OP_LESS_THAN:
                case OP_SAFE_LESS_THAN:
                    tcb-> result_node-> value. n = (cmp <  0);
                    break;

                case OP_GREATER_EQUAL:
                case OP_SAFE_GREATER_EQUAL:
                    tcb-> result_node-> value. n = (cmp >= 0);
                    break;

                case OP_LESS_EQUAL:
                case OP_SAFE_LESS_EQUAL:
                    tcb-> result_node-> value. n = (cmp <= 0);
                    break;

                default:
                    raise_exception (anomaly_event);
                    return;
              }
          }
      }
    /*  If only one operand is a pointer, produce an error message.          */
    else
      {
        report_error (error_event,
                      "Illegal comparison between pointer and scalar values.");
        return;
      }
    tcb-> result_node-> value. type = TYPE_NUMBER;
}


/************************   RETURN TRUE IF OP1 TRUE   ************************/

MODULE return_true_if_op1_true (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    inherit_culprit (tcb-> result_node-> op1);
    if ((tcb-> result_node-> op1-> value. type != TYPE_UNDEFINED)
    &&  number_value (& tcb-> result_node-> op1-> value))
      {
        tcb-> result_node-> value. type = TYPE_NUMBER;
        tcb-> result_node-> value. n    = 1;
        raise_exception (dialog_return_event);
      }
}


/***********************   RETURN FALSE IF OP1 FALSE   ***********************/

MODULE return_false_if_op1_false (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    inherit_culprit (tcb-> result_node-> op1);
    if ((tcb-> result_node-> op1-> value. type != TYPE_UNDEFINED)
    &&  (! number_value (& tcb-> result_node-> op1-> value)))
      {
        tcb-> result_node-> value. type = TYPE_NUMBER;
        tcb-> result_node-> value. n    = 0;
        raise_exception (dialog_return_event);
      }
}


/**************************   RETURN IF OP1 FALSE   **************************/

MODULE return_if_op1_false (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    inherit_culprit (tcb-> result_node-> op1);
    if ((tcb-> result_node-> op1-> value. type == TYPE_UNDEFINED)
    || (! number_value (& tcb-> result_node-> op1-> value)))
        raise_exception (dialog_return_event);
}


/*************************   BEGIN EVALUATING NODE   *************************/

MODULE begin_evaluating_node (THREAD *thread)
{
    RESULT_NODE
        *new_node;

    tcb = thread-> tcb;                 /*  Point to thread's context        */

    tcb-> script_node = tcb-> evaluate_node;
    new_node = new_result_node ();;
    ASSERT (new_node);
    init_result (new_node);

    new_node-> parent = tcb-> result_node;

    *tcb-> result_ptr = new_node;
    new_node-> script_node = tcb-> script_node;

    new_node-> indent      = tcb-> script_node-> spaces;

    tcb-> result_node = new_node;

    if (tcb-> script_node-> constant)
      {
        copy_value (& new_node-> value, & tcb-> script_node-> result);
        raise_exception (dialog_return_event);
      }
}


/************************   GENERATE CALL TYPE EVENT   ***********************/

MODULE generate_call_type_event (THREAD *thread)
{
    char
        *scope,
        *name;
    CLASS_DESCRIPTOR
        *class = NULL;
    void
        *item = NULL;
    GSL_FUNCTION
        *gsl_function = NULL;
    int
        argc;
    VALUE
        *value;

    tcb = thread-> tcb;                 /*  Point to thread's context        */

    must_be_defined            (tcb-> result_node-> scope);
    must_be_defined_and_linear (tcb-> result_node-> name);
    if (exception_raised)
        return;

    /*  See if this is a GSL macro/function.  */
    tcb-> result_node-> macro = macro_value (& tcb-> scope_stack,
                                               tcb-> result_node);
    if (tcb-> result_node-> macro)
      {
        the_next_event = macro_event;
        return;
      }

    scope = tcb-> result_node-> scope
          ? extended_scope_string (tcb-> result_node-> scope)
          : NULL;
    name  = string_value (& tcb-> result_node-> name-> value);

    /*  Otherwise look for a method.  */
    if (tcb-> result_node-> scope)
      {
        lookup_the_item (tcb-> result_node-> scope);
        if (tcb-> result_node-> scope-> value. type == TYPE_POINTER)
          {
            class = tcb-> result_node-> scope-> value. c;
            item  = tcb-> result_node-> scope-> value. i;
          }
      }
    else  /*  This is a fudge to find the built-in functions.  */
      {
        value = symb_class. get_attr (tcb-> classes, "$", FALSE);
        class = value-> c;
        item  = value-> i;
      }

    if (class)
        gsl_function = locate_method (class,
                                      name);
    if (! gsl_function)
      {
        report_error (error_event, "Unknown function: %s%s%s",
                      scope ? scope : "",
                      scope ?"." : "",
                      name);
        return;
      }
    argc = build_method_arguments (  tcb-> script_node-> op1,
                                   & tcb-> result_node-> argv);
    tcb-> result_node-> argc = argc;

    /*  Check that we have a valid number of parameters  */
    if (argc >= gsl_function-> min_parmc
    && ((gsl_function-> max_parmc == 0
    &&   gsl_function-> cnt_parmt  > 0)
    ||   argc <= gsl_function-> max_parmc))
      {
        tcb-> result_node-> gsl_function = gsl_function;
        tcb-> item = item;
        the_next_event = method_event;
      }
    else
        report_error (error_event, "Parameter mismatch for function: %s%s%s",
                      scope ? scope : "",
                      scope ?"." : "",
                      name);
}


static void
must_be_defined (RESULT_NODE *node)
{
    if (node
    &&  node-> value. type == TYPE_UNDEFINED)
        report_undefined_expression (node);
}


static void
lookup_the_item (RESULT_NODE *item_node)
{
    CLASS_DESCRIPTOR
        *class = NULL;
    void
        *item;
    char
        *error_text;

    if (item_node
    &&  item_node-> value. type != TYPE_UNDEFINED
    &&  item_node-> value. type != TYPE_POINTER)
      {
        /*  Catch empty string = innermost scope  */
        if (item_node-> value. s
        && (item_node-> value. s [0]))
            lookup_simple_scope (& tcb-> scope_stack,
                                 & item_node-> value,
                                 tcb-> gsl-> ignorecase,
                                 & class, & item,
                                 &error_text);
        else
            lookup_innermost_item (& class, & item);

        if (class)
            assign_pointer (& item_node-> value, class, item);
        else
        if (error_text)
            report_error (error_event, "%s", error_text);
      }
}


/****************************   GET FIRST CHILD   ****************************/

MODULE get_first_child (THREAD *thread)
{
    SCOPE_BLOCK
        *scope_block;
    CLASS_DESCRIPTOR
        *class = NULL;
    void
        *parent = NULL;
    const char
        *name,
        *alias;

    tcb = thread-> tcb;                 /*  Point to thread's context        */

    /*  .(for | sort) <scope>.<name> as <as> where <before> by <after> */
    /*  <scope>-><name> (<before>)                                     */
    /*  count (<scope>.<name>, <before>, <as>)                         */

    must_be_defined_and_linear (tcb-> result_node-> name);
    must_be_defined_and_linear (tcb-> result_node-> as);
    if (exception_raised)
        return;

    if (tcb-> result_node-> scope)
      {
        class  = tcb-> result_node-> scope-> value. c;
        parent = tcb-> result_node-> scope-> value. i;
      }
    else
        lookup_innermost_item (& class, & parent);

    name = string_result (tcb-> result_node-> name);

    if (tcb-> result_node-> as)
      {
        string_value (& tcb-> result_node-> as-> value);
        if (tcb-> result_node-> as-> value. s [0] != 0)
            alias = tcb-> result_node-> as-> value. s;
        else
            alias = NULL;
      }
    else
        alias = name;

    if ((! alias)
    && (! tcb-> script_node-> stacked))
      {
        report_unaliased_unstacked_scope ();
        return;
      }

    if (! class-> first_child)
      {
        report_error (error_event,
                      "Unable to iterate children of class: %s",
                      class-> name);
        return;
      }

    scope_block = create_scope_block (& tcb-> scope_stack,
                                      GG_FOR,
                                      alias,
                                      class);
    scope_block-> stacked = tcb-> script_node-> stacked;
    class-> first_child (parent,
                         name, tcb-> gsl-> ignorecase,
                         & scope_block-> class, & scope_block-> item);

    tcb-> result_node-> item_nbr = 0;
    tcb-> sort_type = TYPE_UNKNOWN;

    create_scope_item_from_item (scope_block);
}


static void
create_scope_item_from_item (SCOPE_BLOCK *scope_block)
{
    if (scope_block-> item)
      {
        scope_block-> scope_item
            = create_scope_item (scope_block,
                                 scope_block-> class,
                                 scope_block-> item,
                                 ++tcb-> result_node-> item_nbr);

        the_next_event = ok_event;
      }
    else
      {
        scope_block-> scope_item = NULL;

        the_next_event = none_event;
      }
}


/***************************   CLOSE SCRIPT FILE   ***************************/

MODULE close_script_file (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    script_handle_destroy (tcb-> script);
    tcb-> script = NULL;
    tcb-> gsl-> line   = NULL;
}


/*************************   FINISH EVALUATING NODE   ************************/

MODULE finish_evaluating_node (THREAD *thread)
{
    MEMTRN
        *memtrn;

    tcb = thread-> tcb;                 /*  Point to thread's context        */

    if (tcb-> script_node
    &&  tcb-> result_node-> constant && ! tcb-> script_node-> constant)
      {
        tcb-> script_node-> constant = TRUE;
        if (tcb-> script_node-> dynamic)
            memtrn = tcb-> scratch_memtrn;
        else
            memtrn = NULL;

        copy_value_ (memtrn,
                     & tcb-> script_node-> result,
                     & tcb-> result_node-> value);
      }
}


/****************************   POP CURRENT NODE   ***************************/

MODULE pop_current_node (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    pop_the_result_node ();
}


static void
pop_the_result_node (void)
{
    if (tcb-> node_stack)
      {
        tcb-> result_node = tcb-> node_stack;
        tcb-> node_stack  = tcb-> node_stack-> next;
        tcb-> result_node-> next = NULL;

        tcb-> result_root = tcb-> result_node;
        while (tcb-> result_root-> parent)
            tcb-> result_root = tcb-> result_root-> parent;

        tcb-> script_node = tcb-> result_node-> script_node;
        tcb-> script_root = tcb-> result_root-> script_node;
      }
    else
        raise_exception (anomaly_event);
}


/************************   REQUEST LOAD SCRIPT FILE   ***********************/

MODULE request_load_script_file (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    script_load_file (tcb-> script_name,
                      (Bool) (tcb-> gsl-> line ? tcb-> gsl-> line-> template
                                               : tcb-> gsl-> template),
                      FALSE,
                      & tcb-> call_result,
                      thread-> queue);
}


/**************************   DESTROY SCRIPT TEXT   **************************/

MODULE destroy_script_text (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    mem_strfree (& tcb-> script_name);
    mem_strfree (& tcb-> script_text);
}


/****************************   START NEW SCRIPT   ***************************/

MODULE start_new_script (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    tcb-> script = tcb-> call_result. value. i;
    tcb-> gsl-> line   = NULL;
}


/*************************   FLAG ERROR OCCURRENCE   *************************/

MODULE flag_error_occurrence (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    tcb-> error_occurred = TRUE;
}


/************************   REQUEST LOAD SCRIPT TEXT   ***********************/

MODULE request_load_script_text (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    script_load_string (tcb-> script_name,
                        tcb-> script_text,
                        (Bool) (tcb-> gsl-> line ? tcb-> gsl-> line-> template
                                                 : tcb-> gsl-> template),
                        ((Bool) (tcb-> execute_full == 0)),
                        & tcb-> call_result,
                        thread-> queue);
}


/*****************************   LOOKUP TO ITEM   ****************************/

MODULE lookup_to_item (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    lookup_the_item (tcb-> result_node-> to);
    must_be_pointer (tcb-> result_node-> to);
}


static void
must_be_pointer (RESULT_NODE *item_node)
{
    if ((! exception_raised)
    &&  item_node
    &&  item_node-> value. type != TYPE_POINTER)
        report_error (error_event, "Undefined expression: %s",
                      extended_scope_string (item_node));
}


/***************************   LOOKUP BEFORE ITEM   **************************/

MODULE lookup_before_item (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    lookup_the_item (tcb-> result_node-> before);
    must_be_pointer (tcb-> result_node-> before);
}


/***************************   LOOKUP AFTER ITEM   ***************************/

MODULE lookup_after_item (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    lookup_the_item (tcb-> result_node-> after);
    must_be_pointer (tcb-> result_node-> after);
}


/***************************   LOOKUP SCOPE ITEM   ***************************/

MODULE lookup_scope_item (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    lookup_the_item (tcb-> result_node-> scope);
    must_be_pointer (tcb-> result_node-> scope);
}


/***********************   LOOKUP OPTIONAL SCOPE ITEM   **********************/

MODULE lookup_optional_scope_item (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    if (tcb-> result_node-> scope
    &&  tcb-> result_node-> scope-> value. s
    &&  tcb-> result_node-> scope-> value. s [0])
      {
        lookup_the_item (tcb-> result_node-> scope);
        if (tcb-> result_node-> scope-> value. type != TYPE_POINTER)
            tcb-> result_node-> culprit =
                mem_strdup (extended_scope_string (tcb-> result_node-> scope));
      }
}


/*******************   RAISE EXCEPTION IF NO SCOPE CLASS   *******************/

MODULE raise_exception_if_no_scope_class (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    if (tcb-> result_node-> scope)
      {
        /*  Catch empty scope which is OK  */
        if (   tcb-> result_node-> scope-> value. s
        && (! *tcb-> result_node-> scope-> value. s))
            return;
        else
        if (! tcb-> result_node-> scope-> value. c)
          {
            inherit_culprit (tcb-> result_node-> scope);
            raise_exception (exception_event);
          }
      }
}


/***************************   RETURN OK FEEDBACK   **************************/

MODULE return_ok_feedback (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    send_ggcode_ok (& tcb-> replyq-> qid,
                    tcb-> job);
}


/********************   FINISH EXCEPTION IF EXECUTE FULL   *******************/

MODULE finish_exception_if_execute_full (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    if (tcb-> execute_full)
        raise_exception (finish_event);
}


/***********************   CHECK FOR SHUTDOWN REQUEST   **********************/

MODULE check_for_shutdown_request (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    if (thread-> queue-> shutdown)
        raise_exception (shutdown_event);
}


/*************************   BEFORE EXECUTING LINE   *************************/

MODULE before_executing_line (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    ASSERT (tcb-> execute_level >= 0);
    tcb-> execute_level++;
}


/**************************   AFTER EXECUTING LINE   *************************/

MODULE after_executing_line (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    if (--tcb-> execute_level == 0)
      {
        mem_rollback (tcb-> scratch_memtrn);
        tcb-> scratch_memtrn = mem_new_trans ();
      }
}


/**************************   TERMINATE THE THREAD   *************************/

MODULE terminate_the_thread (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    the_next_event = terminate_event;
}


/*************************   SIGNAL INTERNAL ERROR   *************************/

MODULE signal_internal_error (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    report_error (_LR_NULL_EVENT,
                  "%s", "Internal error");
}


/**************************   INVOKE ABORT HANDLER   *************************/

MODULE invoke_abort_handler (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */
    if (abort_fct)
        (abort_fct) ();                 /*  Call abort handler if defined    */
}


/*  ---------------------------------------------------------------------[<]-
    Function: gg_send_output

    Synopsis: Redirects stdout output to a specified OUTPUT_FCT function.
    If the specified address is NULL, redirects back to the stdout stream.
    If the echo argument is TRUE, stdout output is also sent to stdout as
    normal.
    ---------------------------------------------------------------------[>]-*/

void
gg_send_output (THREAD *gsl_thread, OUTPUT_FCT *new_output_fct, Bool echo)
{
    tcb = gsl_thread-> tcb;             /*  Point to thread's context        */

    tcb-> output_fct  = new_output_fct;
    tcb-> stdout_echo = echo;           /*  Copy to stdout                   */
}


/*  ---------------------------------------------------------------------[<]-
    Function: gg_set_handler

    Synopsis: Sets-up an event handler for an internal GSL event.  Currently
    only supports one event, EVENT_ABORT.
    ---------------------------------------------------------------------[>]-*/

void
gg_set_handler (HANDLER_FCT *handler_fct, int event)
{
    switch (event) {
        case EVENT_ABORT:
            abort_fct = handler_fct;
            break;
    }
}

