/*===========================================================================*
 *                                                                           *
 *  ggpars.c - Script parser functions                                       *
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
#include "ggpars.d"                     /*  Include dialog data              */


/*- Type definitions --------------------------------------------------------*/


typedef struct {
    char
        token[16];
    event_t
        event;
} TOKEN_EVENT;


typedef struct
{
    /*  Pointers into parse tree */
    SCRIPT_NODE
      **the_node_ptr,                   /*  Ptr to ptr to current node       */
       *the_parent,                     /*  Ptr to parent of current node    */
       *the_root;                       /*  Ptr to root of parse tree        */

    /*  Variables concerning current token  */
    int
        script_ptr,                     /*  Offset to next char to parse     */
        prev_posn,                      /*  Offset before parsing last token */
        token_posn,                     /*  Offset of last token parsed      */
        token_width,                    /*  Width of last token parsed      */
        token_spaces,                   /*  Spaces before last token parsed  */
        brackets,                       /*  Number of brackets to open       */
        deferred_width,                 /*  Width of earlier tokens         */
        deferred_spaces;                /*  Spaces before earlier tokens     */
    char 
        the_token;                      /*  Current expression token         */
    long
        the_number;                     /*  Current number value             */
    OPERATOR
        the_operator;                   /*  Expression operator              */
    OPERATOR
        the_sign;                       /*  Operand sign                     */

    /*  Other variables  */
    SCRIPT_READ
       *script_read;                    /*  The script reading function      */
    JOBID
        job;                            /*  The job ID number                */
    char
       *expression,
       *script_line,
        buffer [LINE_MAX + 1];
    MEMTRN 
       *memtrn;                         /*  Transaction for parse tree.      */
    char
        error_message [LINE_MAX + 1];
    long
        size;                           /*  Total size allocated this job    */

    LIST
        state_stack,                    /*  Dialog state stack               */
        quote_stack;                    /*  Quote character                  */
} TCB;

        
/*- Prototypes --------------------------------------------------------------*/

static XML_ITEM *
            build_xml                             (XML_ITEM *xml_item, 
                                                   SCRIPT_NODE *node);
static void collect_spaces                        (void);
static void generate_text_event                   (char quote);
static void generate_extend_event                 (void);
static void generate_end_of_line_event            (void);
static void generate_other_event                  (void);
static void generate_simple_token_event           (const char *text,
                                                   event_t event);
static void generate_sign_event                   (void);
static void generate_number_event                 (void);
static void generate_literal_event                (void);
static void recognise_script_command_token        (void);
static event_t recognise_token                    (char *token,
                                                   TOKEN_EVENT token_event[],
                                                   int num);
static void generate_spaces_event                 (void);
static void generate_assign_event                 (void);
static void generate_unary_operator_event         (void);
static void generate_operator_event               (void);
static OPERATOR recognise_operator                (char ch);
static void generate_next_arg_event               (void);
static void recognise_script_token                (void);
static void generate_extra_literal_event          (void);
static void generate_direct_quoted_event          (char quote);
static void generate_modifier_text_event          (void);
static void generate_comment_text_event           (void);
static void insert_simple_node                    (SCRIPT_NODE_TYPE type);
static void insert_the_node                       (SCRIPT_NODE *app_node,
                                                   OPERATOR operator);
static void find_operator_insertion_point         (OPERATOR operator);
static int  return_one_level                      (void);

static void error_exception                       (char *format, ...);
static void confirm_node_doesnt_exist             (void);
static void insert_data_specifier_node            (THREAD *thread,
                                                   SCRIPT_NODE_TYPE type);

/*- Definitions -------------------------------------------------------------*/

#define AGENT_NAME      "GGPARS"        /*  Our public name                  */

/*- Global variables that control parsing -----------------------------------*/

char g_esc_symbol = '\\';
char g_sub_symbol = '$';

/*- Global variables used in this source file only --------------------------*/

static int priority[] = {
    8,                                  /*  UNDEFINED      */
    7, 7,                               /*  TIMES, DIVIDE  */
    6, 6,                               /*  PLUS, MINUS    */
    5, 5,                               /*  IIF, DEFAULT   */
    4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, /*  Comparisons    */
    3,                                  /*  NOT            */
    2, 2,                               /*  AND, OR        */
    1 };                                /*  NEXT_ARG       */

static TCB
    *tcb;                               /*  Address thread context block     */

/*  ---------------------------------------------------------------------[<]-
    Function: gg_free

    Synopsis: Frees all memory allocated to a parse tree.  Assumes that all
    pointer fields are either NULL or allocated with separate calls to
    sflmem memory allocation functions.
    ---------------------------------------------------------------------[>]-*/

void gg_free (SCRIPT_NODE *node)
{
    if (node)
      {
        mem_free (node-> result. s);
        mem_free (node-> result. b);
        gg_free (node-> scope);
        gg_free (node-> name);
        if (node-> type != GG_TEXT)
            gg_free (node-> op1);
        else
            mem_free (node-> op1);
        gg_free (node-> op2);
        gg_free (node-> as);
        gg_free (node-> to);
        gg_free (node-> before);
        gg_free (node-> after);
        mem_free (node);
      }
}


/*  ---------------------------------------------------------------------[<]-
    Function: gg_xml

    Synopsis: Builds an XML tree representing the parsed GSL line.
    ---------------------------------------------------------------------[>]-*/

XML_ITEM *
gg_xml (SCRIPT_NODE *node)
{
    if (!node)
        return NULL;
    else
        return build_xml (xml_new (NULL, "line", NULL), node);
}


static XML_ITEM *
build_xml (XML_ITEM *xml_item, SCRIPT_NODE *node)
{
    xml_put_attr (xml_item, "type",   node_type_string (node-> type));
    if (node-> width)
      xml_put_attr (xml_item, "width", strprintf ("%u", node-> width));
    if (node-> brackets)
        xml_put_attr (xml_item, "brackets", 
                      strprintf ("%u", node-> brackets));
    if (node-> spaces)
        xml_put_attr (xml_item, "spaces", 
                      strprintf ("%u", node-> spaces));
    if (node-> extend)
        xml_put_attr (xml_item, "extend", "1");
    if (node-> stacked )
        xml_put_attr (xml_item, "stacked", "1");
    if (node-> operator)
        xml_put_attr (xml_item, "operator", operator_text (node-> operator));
    if (node-> type == GG_TEXT && node-> op1)
        xml_new (xml_item, NULL, (char *) node-> op1);

    if (node-> scope)
        build_xml (xml_new (xml_item, "scope", NULL), node-> scope);
    if (node-> name)
        build_xml (xml_new (xml_item, "name", NULL), node-> name);
    if (node-> op1
    &&  node-> type != GG_TEXT)
        build_xml (xml_new (xml_item, "op1", NULL), node-> op1);
    if (node-> op2)
        build_xml (xml_new (xml_item, "op2", NULL), node-> op2);
    if (node-> as)
        build_xml (xml_new (xml_item, "as", NULL), node-> as);
    if (node-> to)
        build_xml (xml_new (xml_item, "to", NULL), node-> to);
    if (node-> before)
        build_xml (xml_new (xml_item, "before", NULL), node-> before);
    if (node-> after)
        build_xml (xml_new (xml_item, "after", NULL), node-> after);

    return xml_item;
}


char *
node_type_string (SCRIPT_NODE_TYPE type)
{
    switch (type)
      {
        case GG_COMMENT      :  return "COMMENT";
        case GG_LINE         :  return "LINE";
        case GG_TEXT         :  return "TEXT";
        case GG_SUBSTITUTE   :  return "SUBSTITUTE";
        case GG_LITERAL      :  return "LITERAL";
        case GG_NUMBER       :  return "NUMBER";
        case GG_SYMBOL       :  return "SYMBOL";
        case GG_MEMBER       :  return "MEMBER";
        case GG_ATTRIBUTE    :  return "ATTRIBUTE";
        case GG_CALL         :  return "CALL";
        case GG_OPERATOR     :  return "OPERATOR";
        case GG_CLOSE        :  return "CLOSE";
        case GG_ELSE         :  return "ELSE";
        case GG_END_IF       :  return "END IF";
        case GG_END_FOR      :  return "END FOR";
        case GG_END_SCOPE    :  return "END SCOPE";
        case GG_END_MACRO    :  return "END MACRO";
        case GG_END_FUNCTION :  return "END FUNCTION";
        case GG_END_NEW      :  return "END NEW";
        case GG_END_WHILE    :  return "END WHILE";
        case GG_OUTPUT       :  return "OUTPUT";
        case GG_APPEND       :  return "APPEND";
        case GG_GSL          :  return "GSL";
        case GG_DIRECT       :  return "DIRECT";
        case GG_XML          :  return "XML";
        case GG_TEMPLATE     :  return "TEMPLATE";
        case GG_END_TEMPLATE :  return "END TEMPLATE";
        case GG_ECHO         :  return "ECHO";
        case GG_DEFINE       :  return "DEFINE";
        case GG_MACRO        :  return "MACRO";
        case GG_FUNCTION     :  return "FUNCTION";
        case GG_RETURN       :  return "RETURN";
        case GG_SAVE         :  return "SAVE";
        case GG_NEW          :  return "NEW";
        case GG_DELETE       :  return "DELETE";
        case GG_MOVE         :  return "MOVE";
        case GG_COPY         :  return "COPY";
        case GG_IF           :  return "IF";
        case GG_ELSIF        :  return "ELSIF";
        case GG_FOR          :  return "FOR";
        case GG_SCOPE        :  return "SCOPE";
        case GG_WHILE        :  return "WHILE";
        case GG_NEXT         :  return "NEXT";
        case GG_LAST         :  return "LAST";
        case GG_ABORT        :  return "ABORT";
        case GG_SORT         :  return "SORT";
        case GG_UNDEFINED    :  return "UNDEFINED";
        default              :  return "UNKNOWN";
      }
}


char *
operator_text (OPERATOR op)
{
    switch (op)
      {
        case OP_TIMES              : return "*";
        case OP_DIVIDE             : return "/";
        case OP_PLUS               : return "+";
        case OP_MINUS              : return "-";
        case OP_EQUALS             : return "=";
        case OP_NOT_EQUALS         : return "<>";
        case OP_GREATER_THAN       : return ">";
        case OP_LESS_THAN          : return "<";
        case OP_GREATER_EQUAL      : return ">=";
        case OP_LESS_EQUAL         : return "<=";
        case OP_SAFE_EQUALS        : return "?=";
        case OP_SAFE_NOT_EQUALS    : return "?<>";
        case OP_SAFE_GREATER_THAN  : return "?>";
        case OP_SAFE_LESS_THAN     : return "?<";
        case OP_SAFE_GREATER_EQUAL : return "?>=";
        case OP_SAFE_LESS_EQUAL    : return "?<=";
        case OP_OR                 : return "|";
        case OP_AND                : return "&";
        case OP_NOT                : return "!";
        case OP_DEFAULT            : return "?";
        case OP_IIF                : return "??";
        case OP_NEXT_ARG           : return ",";
        default                    : return "";
      }
}


void
init_script_node (SCRIPT_NODE *node)
{
    node-> type         = GG_UNDEFINED;
    node-> parent       = NULL;
    node-> brackets     = 0;
    node-> width        = 0;
    node-> spaces       = 0;
    node-> scope        = NULL;
    node-> name         = NULL;
    node-> op1          = NULL;
    node-> op2          = NULL;
    node-> as           = NULL;
    node-> to           = NULL;
    node-> before       = NULL;
    node-> after        = NULL;
    node-> operator     = OP_UNDEFINED;
    node-> extend       = FALSE;
    node-> constant     = FALSE;
    node-> stacked      = FALSE;
    node-> dynamic      = FALSE;
    init_value (& node-> result);
}


/********************   INITIALISE AGENT - ENTRY POINT    ********************/

/*  ---------------------------------------------------------------------[<]-
    Function: ggpars_init

    Synopsis: Initialises the GSLGen parser agent.  Returns 0 if
    initialised okay, -1 if there was an error.
    Supports these public methods:
    <Table>
    TEMPLATE    Parse a template line.
    GSL         Parse a GSL line.
    EXPRESSION  Parse an expression.
    </Table>
    ---------------------------------------------------------------------[>]-*/

int
ggpars_init (void)
{
    AGENT   *agent;                     /*  Handle for our agent             */
#   include "ggpars.i"                  /*  Include dialog interpreter       */

    /*                      Method name   Event value     Priority           */
    /*  Shutdown event comes from Kernel                                     */
    declare_smtlib_shutdown   (shutdown_event, SMT_PRIORITY_MAX);

    /*  Public methods supported by this agent                               */
    declare_ggpars_template   (template_event,   0);
    declare_ggpars_gsl        (gsl_event,        0);
    declare_ggpars_expression (expression_event, 0);

    /*  Signal okay to caller that we initialised okay                       */
    return (0);
}


/*  ---------------------------------------------------------------------[<]-
    Function: gg_parse_template

    Synopsis: Starts thread to parse the supplied template line into a
    script parse tree.
    ---------------------------------------------------------------------[>]-*/

int
gg_parse_template (SCRIPT_READ *read,
                   JOBID        job,
                   QUEUE       *replyqueue)
{
    THREAD  
       *thread;                         /*  Handle to GSL thread             */

    thread = thread_create (AGENT_NAME, "");

    if (! thread)
        return -1;

    ((TCB *) thread-> tcb)-> script_read = read;
    ((TCB *) thread-> tcb)-> job         = job;
    ((TCB *) thread-> tcb)-> expression  = NULL;
    ((TCB *) thread-> tcb)-> script_line = ((TCB *) thread-> tcb)-> buffer;

    lsend_ggpars_template (& thread-> queue-> qid,
                           & replyqueue-> qid,
                           NULL, NULL, NULL, 0,
                           0);          /*  No job number                    */

    return 0;
}


/*  ---------------------------------------------------------------------[<]-
    Function: gg_parse_gsl

    Synopsis: Starts thread to parse the supplied GSL line into a script
    parse tree.
    ---------------------------------------------------------------------[>]-*/

int
gg_parse_gsl (SCRIPT_READ *read,
              JOBID        job,
              QUEUE       *replyqueue)
{
    THREAD  *thread;                    /*  Handle to GSL thread             */

    thread = thread_create (AGENT_NAME, "");

    if (! thread)
        return -1;

    ((TCB *) thread-> tcb)-> script_read = read;
    ((TCB *) thread-> tcb)-> job         = job;  
    ((TCB *) thread-> tcb)-> expression  = NULL;
    ((TCB *) thread-> tcb)-> script_line = ((TCB *) thread-> tcb)-> buffer;

    lsend_ggpars_gsl (& thread-> queue-> qid,
                      & replyqueue-> qid,
                      NULL, NULL, NULL, 0,
                      0);               /*  No job number                    */

    return 0;
}


/*  ---------------------------------------------------------------------[<]-
    Function: gg_parse_expression

    Synopsis: Starts thread to parse the supplied exprssion textinto a
    script parse tree.
    ---------------------------------------------------------------------[>]-*/

int
gg_parse_expression (char   *expression,
                     JOBID   job,
                    QUEUE  *replyqueue)
{
    THREAD  *thread;                    /*  Handle to GSL thread             */

    thread = thread_create (AGENT_NAME, "");

    if (! thread)
        return -1;

    ((TCB *) thread-> tcb)-> script_read = NULL;
    ((TCB *) thread-> tcb)-> job         = job;  
    ((TCB *) thread-> tcb)-> expression  = mem_strdup (expression);
    ((TCB *) thread-> tcb)-> script_line = ((TCB *) thread-> tcb)-> expression;
    ((TCB *) thread-> tcb)-> script_ptr  = 0;     /*  Start of line          */

    lsend_ggpars_expression (& thread-> queue-> qid,
                             & replyqueue-> qid,
                             NULL, NULL, NULL, 0,
                             0);        /*  No job number                    */

    return 0;
}


/*  ---------------------------------------------------------------------[<]-
    Function: ggpars_term

    Synopsis: Terminates the GSLGen parser agent.  Returns 0 if
    terminated okay, -1 if there was an error.
    ---------------------------------------------------------------------[>]-*/

int
ggpars_term (void)
{
    return 0;
}


/*************************   INITIALISE THE THREAD   *************************/

MODULE initialise_the_thread (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    tcb-> brackets        = 0;
    tcb-> deferred_width  = 0;
    tcb-> deferred_spaces = 0;

    tcb-> memtrn = mem_new_trans ();

    tcb-> the_root     = NULL;
    tcb-> the_parent   = NULL;
    tcb-> the_node_ptr = & tcb-> the_root;

    tcb-> size = 0;

    list_reset (& tcb-> state_stack);
    list_reset (& tcb-> quote_stack);
}


/****************************   READ SCRIPT LINE   ***************************/

MODULE read_script_line (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    tcb-> script_ptr = 0;               /*  Start of line                    */
    if (tcb-> script_read)
        if ((tcb-> script_read) (tcb-> job, tcb-> script_line))
            return;

    tcb-> prev_posn       = 0;
    tcb-> token_posn      = 0;
    tcb-> token_width     = 0;
    tcb-> token_spaces    = 0;
    tcb-> script_line [0] = '\0';
    raise_exception (end_of_file_event);
}


/***********************   GENERATE SCRIPT EXCEPTION   ***********************/

MODULE generate_script_exception (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    tcb-> prev_posn = tcb-> script_ptr;
    tcb-> the_token = tcb-> script_line [tcb-> script_ptr];

    if (tcb-> the_token == '.')
      {
        raise_exception (script_event);
        tcb-> token_posn  = tcb-> script_ptr++;
        tcb-> token_width = 1;
      }
}


/***********************   GENERATE COMMENT EXCEPTION   **********************/

MODULE generate_comment_exception (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    tcb-> prev_posn = tcb-> script_ptr;
    tcb-> the_token = tcb-> script_line [tcb-> script_ptr];

    if (tcb-> the_token == '-')
      {
        raise_exception (comment_event);
        tcb-> token_posn  = tcb-> script_ptr++;
        tcb-> token_width = 1;
      }
}


/************************   GENERATE LINE EXCEPTION   ************************/

MODULE generate_line_exception (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    tcb-> prev_posn = tcb-> script_ptr;

    collect_spaces ();
    tcb-> the_token = tcb-> script_line [tcb-> script_ptr];

    if (tcb-> the_token == '>')
      {
        raise_exception (line_event);
        tcb-> token_posn  = tcb-> script_ptr++;
        tcb-> token_width = 1;
      }
    else
        tcb-> script_ptr = tcb-> prev_posn;
}


/***************************   GET TEMPLATE TOKEN   **************************/

MODULE get_template_token (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    tcb-> prev_posn = tcb-> script_ptr;
    collect_spaces ();

    tcb-> the_token      = tcb-> script_line [tcb-> script_ptr];
    the_next_event = _LR_NULL_EVENT;

    generate_text_event ('\0');
    generate_simple_token_event ("$(", substitute_event);
    generate_extend_event ();
    generate_end_of_line_event ();

    generate_other_event ();
}


void collect_spaces (void)
{
    while (tcb-> script_line [tcb-> script_ptr] == ' ')
      {
        tcb-> script_ptr++;
        tcb-> token_spaces++;
      }
}


void generate_text_event (char quote)
{
    int
        script_ptr;
        
    if (the_next_event == _LR_NULL_EVENT)
      {
        tcb-> token_posn  = tcb-> script_ptr;
        tcb-> token_width = 0;

        while ((tcb-> the_token = tcb-> script_line [tcb-> script_ptr++]) != '\0')
            if (tcb-> the_token == g_sub_symbol)
              {
                script_ptr = tcb-> script_ptr;
                while (tcb-> script_line [script_ptr] == ' ')
                    script_ptr++;
                if ((tcb-> script_line [script_ptr] == '\0')
                ||  (tcb-> script_line [script_ptr] == '\n'))
                    break;
                else
                  {
                    tcb-> script_ptr++;
                    tcb-> token_width += 2;
                  }
              }
            else
                if (((tcb-> the_token == g_sub_symbol)  /*  Substitute?  */
                &&   (tcb-> script_line [tcb-> script_ptr] == '('))
                ||   (tcb-> the_token == quote)
                ||   (tcb-> the_token == ' '))
                    break;
                else
                    tcb-> token_width++;
        
        tcb-> script_ptr--;               /*  Back to point at terminator      */
        if (tcb-> token_width > 0)
            the_next_event = text_event;
      }
}


void generate_extend_event (void)
{
    int
        script_ptr;
        
    if ((the_next_event == _LR_NULL_EVENT)
    &&  (tcb-> the_token == g_sub_symbol))
      {
        script_ptr = tcb-> script_ptr;
        while (tcb-> script_line [++script_ptr] == ' ');
        if ((tcb-> script_line [script_ptr] == '\0')
        ||  (tcb-> script_line [script_ptr] == '\n'))
          {
            the_next_event = extend_event;
            tcb-> token_posn = tcb-> script_ptr;
            tcb-> script_ptr = script_ptr;
            tcb-> token_width = 1;
          }
      }
}


void generate_end_of_line_event (void)
{
    if ((the_next_event == _LR_NULL_EVENT)
    &&  (tcb-> the_token      == '\0'
    ||   tcb-> the_token      == '\n'))
        the_next_event = end_of_line_event;
}


void generate_other_event (void)
{
    if (the_next_event == _LR_NULL_EVENT)
      {
        the_next_event = other_event;
        tcb-> token_posn  = tcb-> script_ptr;
        tcb-> token_width = 0;
      }
}


/************************   GET SCRIPT COMMAND TOKEN   ***********************/

MODULE get_script_command_token (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    tcb-> prev_posn = tcb-> script_ptr;
    collect_spaces ();

    tcb-> the_token      = tcb-> script_line [tcb-> script_ptr];
    the_next_event = _LR_NULL_EVENT;

    generate_simple_token_event ("#", comment_event);
    generate_simple_token_event ("[", open_bracket_event);
    generate_simple_token_event (".", point_event);
    generate_simple_token_event ("->", member_event);
    generate_simple_token_event ("/*", open_comment_event);
    generate_sign_event ();
    generate_number_event ();
    generate_simple_token_event ("$(", substitute_event);
    generate_extend_event ();
    generate_end_of_line_event ();
    generate_literal_event ();
    recognise_script_command_token ();

    generate_other_event ();
}


void generate_simple_token_event (const char *text,
                                  event_t event)
{
    if ((the_next_event == _LR_NULL_EVENT)
    &&  (strncmp (tcb-> script_line + tcb-> script_ptr, text, strlen (text)) == 0))
      {
        the_next_event = event;
        tcb-> token_posn  = tcb-> script_ptr;
        tcb-> token_width = strlen (text);
        tcb-> script_ptr += tcb-> token_width;
      }
}


void generate_sign_event (void)
{
    if (the_next_event == _LR_NULL_EVENT)
      {
        if (tcb-> the_token == '+')
            tcb-> the_operator = OP_PLUS;
        else
        if (tcb-> the_token == '-')
            tcb-> the_operator = OP_MINUS;
        else
            return;

        tcb-> token_posn  = tcb-> script_ptr++;
        tcb-> token_width = 1;
        the_next_event = sign_event;
      }
}


void generate_number_event (void)
{
    char
        *ptr;

    if (the_next_event == _LR_NULL_EVENT)
      {
        tcb-> the_number   = strtol (& tcb-> script_line [tcb-> script_ptr], & ptr, 10);
        tcb-> token_width = ptr - & tcb-> script_line [tcb-> script_ptr];
        if (tcb-> token_width > 0)
          {
            tcb-> token_posn  = tcb-> script_ptr;
            tcb-> script_ptr += tcb-> token_width;
            the_next_event = number_event;
          }
      }
}


void generate_literal_event (void)
{
    int
        script_ptr;
        
    if (the_next_event == _LR_NULL_EVENT)
      {
        tcb-> token_posn  = tcb-> script_ptr;
        tcb-> token_width = 0;

        while ((tcb-> the_token = tcb-> script_line [tcb-> script_ptr++]) != '\0')
            if (tcb-> the_token == g_sub_symbol)      /*  Ignore next char unless EOL      */
              {
                script_ptr = tcb-> script_ptr;
                while (tcb-> script_line [script_ptr] == ' ')
                    script_ptr++;
                if ((tcb-> script_line [script_ptr] == '\0')
                ||  (tcb-> script_line [script_ptr] == '\n'))
                    break;
                else
                  {
                    tcb-> script_ptr++;
                    tcb-> token_width += 2;
                  }
              }
            else
                if (((tcb-> the_token >= 'a') && (tcb-> the_token <= 'z'))
                ||  ((tcb-> the_token >= 'A') && (tcb-> the_token <= 'Z'))
                ||  ((tcb-> the_token >= '0') && (tcb-> the_token <= '9'))
                ||   (tcb-> the_token == '_'))
                    tcb-> token_width++;
                else
                    break;
        
        tcb-> script_ptr--;                   /*  Back to point at terminator      */
        if (tcb-> token_width > 0)
            the_next_event = literal_event;
      }
}


void recognise_script_command_token (void)
{
    event_t
        the_event;

    static TOKEN_EVENT token_events [] = {
      { "abort",       abort_event },
      { "append",      append_event },
      { "close",       close_event },
      { "copy",        copy_event },
      { "define",      define_event },
      { "delete",      delete_event },
      { "echo",        echo_event },
      { "else",        else_event },
      { "elsif",       elsif_event },
      { "endfor",      end_for_event },
      { "endfunction", end_function_event },
      { "endif",       end_if_event },
      { "endmacro",    end_macro_event },
      { "endnew",      end_new_event },
      { "endscope",    end_scope_event },
      { "endtemplate", end_template_event },
      { "endwhile",    end_while_event },
      { "for",         for_event },
      { "function",    function_event },
      { "gsl",         gsl_event },
      { "if",          if_event },
      { "include",     include_event },
      { "invoke",      invoke_event },
      { "last",        last_event },
      { "literal",     direct_event },
      { "macro",       macro_event },
      { "move",        move_event },
      { "new",         new_event },
      { "next",        next_event },
      { "output",      output_event },
      { "return",      return_event },
      { "save",        save_event },
      { "scope",       scope_event },
      { "sort",        sort_event },
      { "template",    template_event },
      { "while",       while_event },
      { "xml",         xml_event }
    };

    char
        save;

    if (the_next_event == literal_event)
      {
        save = tcb-> script_line [tcb-> token_posn + tcb-> token_width];
        tcb-> script_line [tcb-> token_posn + tcb-> token_width] = 0;
        the_event = recognise_token (& tcb-> script_line [tcb-> token_posn],
                                     token_events,
                                     tblsize (token_events));
        tcb-> script_line [tcb-> token_posn + tcb-> token_width] = save;

        if (the_event != _LR_NULL_EVENT)
            the_next_event = the_event;
      }
}


event_t recognise_token (char *token, TOKEN_EVENT token_events [], int num)
{
    int
        min = 0,
        max = num,
        chop = 0,
        cmp = -1;

    chop = (max + min) / 2;
    cmp = strcmp (token, token_events [chop]. token);
    while (max > min)
      {
        if (cmp < 0)
            max = chop;
        else if (cmp > 0)
            min = chop + 1;
        else
            break;

        chop = (max + min) / 2;
        cmp = strcmp (token, token_events [chop]. token);
      }

    if (cmp == 0)
        return token_events [chop]. event;
    else
        return _LR_NULL_EVENT;
}


/**************************   GET EXPRESSION TOKEN   *************************/

MODULE get_expression_token (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    tcb-> prev_posn      = tcb-> script_ptr;
    tcb-> the_token      = tcb-> script_line [tcb-> script_ptr];
    the_next_event = _LR_NULL_EVENT;

    generate_spaces_event ();
    generate_simple_token_event ("#", comment_event);
    generate_simple_token_event ("(", open_event);
    generate_simple_token_event (")", close_event);
    generate_simple_token_event ("[", open_bracket_event);
    generate_simple_token_event ("]", close_bracket_event);
    generate_simple_token_event (".", point_event);
    generate_simple_token_event ("->", member_event);
    generate_simple_token_event ("/*", open_comment_event);
    generate_unary_operator_event ();
    generate_sign_event ();
    generate_operator_event ();
    generate_next_arg_event ();
    generate_number_event ();
    generate_simple_token_event ("'", quote_event);
    generate_simple_token_event ("\"", quote_event);
    generate_simple_token_event ("$(", substitute_event);
    generate_simple_token_event ("<<", incoming_event);
    generate_extend_event ();
    generate_end_of_line_event ();
    generate_literal_event ();
    recognise_script_token ();

    generate_other_event ();
}


/****************************   GET SCRIPT TOKEN   ***************************/

MODULE get_script_token (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    tcb-> prev_posn      = tcb-> script_ptr;
    tcb-> the_token      = tcb-> script_line [tcb-> script_ptr];
    the_next_event = _LR_NULL_EVENT;

    generate_spaces_event ();
    generate_simple_token_event ("#", comment_event);
    generate_simple_token_event ("(", open_event);
    generate_simple_token_event (")", close_event);
    generate_simple_token_event ("[", open_bracket_event);
    generate_simple_token_event ("]", close_bracket_event);
    generate_simple_token_event (".", point_event);
    generate_simple_token_event ("->", member_event);
    generate_simple_token_event ("/*", open_comment_event);
    generate_assign_event ();
    generate_sign_event ();
    generate_number_event ();
    generate_simple_token_event ("'", quote_event);
    generate_simple_token_event ("\"", quote_event);
    generate_simple_token_event ("$(", substitute_event);
    generate_simple_token_event ("<<", incoming_event);
    generate_extend_event ();
    generate_end_of_line_event ();
    generate_literal_event ();
    recognise_script_token ();

    generate_other_event ();
}


void generate_spaces_event (void)
{
    if ((the_next_event == _LR_NULL_EVENT)
    &&  (tcb-> the_token == ' '))
      {
        collect_spaces ();
        tcb-> token_posn  = tcb-> script_ptr;
        tcb-> token_width = 0;
        the_next_event = spaces_event;
      }
}


void generate_assign_event (void)
{
    OPERATOR
        op;

    if (the_next_event == _LR_NULL_EVENT)
      {
        tcb-> token_posn  = tcb-> script_ptr++;
        tcb-> token_width = 1;
        op = OP_UNDEFINED;

        if (tcb-> the_token == '=')
            the_next_event = simple_assign_event;
        else
          {
            op = recognise_operator (tcb-> the_token);

            if ((op != OP_UNDEFINED)
            &&  (op != OP_LESS_THAN)
            &&  (op != OP_GREATER_THAN)
            &&  (op != OP_NOT)
            &&  (tcb-> script_line [tcb-> script_ptr] == '='))
              {
                the_next_event = arithmetic_assign_event;
                tcb-> script_ptr++;
                tcb-> token_width++;
              }
            else
              {
                op = OP_UNDEFINED;
                tcb-> script_ptr   = tcb-> token_posn;
              }
          }
        tcb-> the_operator = op;
      }           
}


void generate_unary_operator_event (void)
{
    if (the_next_event == _LR_NULL_EVENT)
      {
        if (tcb-> the_token == '!')
            tcb-> the_operator = OP_NOT;
        else
            return;

        tcb-> token_posn  = tcb-> script_ptr++;
        tcb-> token_width = 1;
        the_next_event = unary_operator_event;
      }
}


void generate_operator_event (void)
{
    Bool
        safe = FALSE;
    char
        the_char;
    OPERATOR
        op;

    if (the_next_event == _LR_NULL_EVENT)
      {
        tcb-> token_posn  = tcb-> script_ptr++;
        tcb-> token_width = 1;
        op = OP_UNDEFINED;

        if (tcb-> the_token == '?')
          {
            the_char = tcb-> script_line [tcb-> script_ptr];
            if ((the_char == '<') || (the_char == '>') || (the_char == '='))
              {
                tcb-> token_width++;
                tcb-> script_ptr++;
                tcb-> the_token = the_char;
                safe = TRUE;
              }
            else if (the_char == '?')
              {
                tcb-> token_width++;
                tcb-> script_ptr++;
                op = OP_IIF;
              }
          }

        if (op == OP_UNDEFINED)
            op = recognise_operator (tcb-> the_token);

        if ((op == OP_DIVIDE)
        &&  (tcb-> script_line [tcb-> script_ptr] == '*'))  /*  Open comment  */
            op = OP_UNDEFINED;

        if (op == OP_UNDEFINED)
            tcb-> script_ptr = tcb-> token_posn;

        if (op == OP_LESS_THAN)
          {
            if (tcb-> script_line [tcb-> script_ptr] == '>')
              {
                op = OP_NOT_EQUALS;
                tcb-> script_ptr++;
                tcb-> token_width++;
              }
            else if (tcb-> script_line [tcb-> script_ptr] == '=')
              {
                op = OP_LESS_EQUAL;
                tcb-> script_ptr++;
                tcb-> token_width++;
              }
          }
        if ((op == OP_GREATER_THAN)
        &&  (tcb-> script_line [tcb-> script_ptr] == '='))
          {
            op = OP_GREATER_EQUAL;
            tcb-> script_ptr++;
            tcb-> token_width++;
          }
        if (safe)
          {
            switch (op)
              {
                case OP_EQUALS        : op = OP_SAFE_EQUALS;
                                        break;
                case OP_NOT_EQUALS    : op = OP_SAFE_NOT_EQUALS;
                                        break;
                case OP_GREATER_THAN  : op = OP_SAFE_GREATER_THAN;
                                        break;
                case OP_LESS_THAN     : op = OP_SAFE_LESS_THAN;
                                        break;
                case OP_GREATER_EQUAL : op = OP_SAFE_GREATER_EQUAL;
                                        break;
                case OP_LESS_EQUAL    : op = OP_SAFE_LESS_EQUAL;
                                        break;
                default               : op = OP_UNDEFINED;
                                        tcb-> script_ptr = tcb-> token_posn;
              }
          }
        if (op == OP_DEFAULT)
            the_next_event = default_operator_event;
        else
        if (op != OP_UNDEFINED)
            the_next_event = operator_event;

        if (op != OP_UNDEFINED)
            tcb-> the_operator = op;
      }
}


OPERATOR recognise_operator (char ch)
{
    switch (tcb-> the_token)
      {
        case '*' : return OP_TIMES;
        case '/' : return OP_DIVIDE;
        case '+' : return OP_PLUS;
        case '-' : return OP_MINUS;
        case '=' : return OP_EQUALS;
        case '<' : return OP_LESS_THAN;
        case '>' : return OP_GREATER_THAN;
        case '|' : return OP_OR;
        case '&' : return OP_AND;
        case '!' : return OP_NOT;
        case '?' : return OP_DEFAULT;
        default  : return OP_UNDEFINED;
      }
}


void generate_next_arg_event (void)
{
    if ((the_next_event == _LR_NULL_EVENT)
    &&  (tcb-> the_token == ','))
      {
        the_next_event = next_arg_event;
        tcb-> token_posn   = tcb-> script_ptr++;
        tcb-> token_width  = 1;
        tcb-> the_operator = OP_NEXT_ARG;
      }
}
    

void recognise_script_token (void)
{
    event_t
        the_event;

    static TOKEN_EVENT token_events [] = {
      { "after",       after_event },
      { "as",          as_event },
      { "before",      before_event },
      { "by",          by_event },
      { "from",        from_event },
      { "noalias",     noalias_event },
      { "nostack",     nostack_event },
      { "to",          to_event },
      { "where",       where_event }
    };

    char
        save;

    if (the_next_event == literal_event)
      {
        save = tcb-> script_line [tcb-> token_posn + tcb-> token_width];
        tcb-> script_line [tcb-> token_posn + tcb-> token_width] = 0;
        the_event = recognise_token (& tcb-> script_line [tcb-> token_posn],
                                     token_events,
                                     tblsize (token_events));
        tcb-> script_line [tcb-> token_posn + tcb-> token_width] = save;

        if (the_event != _LR_NULL_EVENT)
            the_next_event = the_event;
      }
}


/*********************   GET BRACKETED IDENTIFIER TOKEN   ********************/

MODULE get_bracketed_identifier_token (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    tcb-> prev_posn      = tcb-> script_ptr;
    tcb-> the_token      = tcb-> script_line [tcb-> script_ptr];
    the_next_event = _LR_NULL_EVENT;

    generate_extra_literal_event ();
    generate_simple_token_event ("]", close_bracket_event);

    generate_other_event ();
}


void generate_extra_literal_event (void)
{
    int
        script_ptr;
        
    if (the_next_event == _LR_NULL_EVENT)
      {
        tcb-> token_posn  = tcb-> script_ptr;
        tcb-> token_width = 0;

        while ((tcb-> the_token = tcb-> script_line [tcb-> script_ptr++]) != '\0')
            if (tcb-> the_token == g_sub_symbol)    /*  Ignore next char unless EOL  */
              {
                script_ptr = tcb-> script_ptr;
                while (tcb-> script_line [script_ptr] == ' ')
                    script_ptr++;
                if ((tcb-> script_line [script_ptr] == '\0')
                ||  (tcb-> script_line [script_ptr] == '\n'))
                    break;
                else
                  {
                    tcb-> script_ptr++;
                    tcb-> token_width += 2;
                  }
              }
            else
                if (tcb-> the_token != ']')
                    tcb-> token_width++;
                else
                    break;
        
        tcb-> script_ptr--;             /*  Back to point at terminator      */
        if (tcb-> token_width > 0)
            the_next_event = literal_event;
      }
}


/****************************   GET QUOTED TOKEN   ***************************/

MODULE get_quoted_token (THREAD *thread)
{
    char
        the_quote[2] = " ";

    tcb = thread-> tcb;                 /*  Point to thread's context        */

    tcb-> prev_posn = tcb-> script_ptr;
    collect_spaces ();

    tcb-> the_token      = tcb-> script_line [tcb-> script_ptr];
    the_next_event = _LR_NULL_EVENT;

    generate_simple_token_event ("$(", substitute_event);
    generate_extend_event ();
    if (the_next_event == _LR_NULL_EVENT)
      {
        list_pop (&tcb-> quote_stack, the_quote[0]);
        generate_simple_token_event (the_quote, quote_event);
        generate_text_event (the_quote[0]);
        if (the_next_event != quote_event)
            list_push (&tcb-> quote_stack, the_quote[0]);
      }
    generate_end_of_line_event ();
    
    generate_other_event ();
}


/************************   GET DIRECT QUOTED TOKEN   ************************/

MODULE get_direct_quoted_token (THREAD *thread)
{
    char
        the_quote[2] = " ";

    tcb = thread-> tcb;                 /*  Point to thread's context        */

    tcb-> prev_posn = tcb-> script_ptr;
    collect_spaces ();

    tcb-> the_token = tcb-> script_line [tcb-> script_ptr];
    the_next_event  = _LR_NULL_EVENT;

    generate_end_of_line_event ();
    if (the_next_event == _LR_NULL_EVENT)
      {
        list_pop (&tcb-> quote_stack, the_quote[0]);
        generate_simple_token_event (the_quote, quote_event);
        generate_direct_quoted_event (the_quote[0]);
        if (the_next_event != quote_event)
            list_push (&tcb-> quote_stack, the_quote[0]);
      }
    
    generate_other_event ();
}


void generate_direct_quoted_event (char quote)
{
    if (the_next_event == _LR_NULL_EVENT)
      {
        tcb-> token_posn  = tcb-> script_ptr;
        tcb-> token_width = 0;

        while ((tcb-> the_token = tcb-> script_line [tcb-> script_ptr++]) != '\0')
            if (tcb-> the_token == quote)
                break;
            else
                tcb-> token_width++;
        
        tcb-> script_ptr--;               /*  Back to point at terminator    */
        if (tcb-> token_width > 0)
            the_next_event = text_event;
      }
}


/***************************   GET MODIFIER TOKEN   **************************/

MODULE get_modifier_token (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    tcb-> prev_posn = tcb-> script_ptr;
    collect_spaces ();

    tcb-> the_token      = tcb-> script_line [tcb-> script_ptr];
    the_next_event = _LR_NULL_EVENT;

    generate_simple_token_event (":", pretty_event);
    generate_simple_token_event ("%", format_event);
    generate_simple_token_event (")", close_event);
    generate_simple_token_event ("$(", substitute_event);
    generate_extend_event ();
    generate_modifier_text_event ();

    generate_other_event ();
}


void generate_modifier_text_event (void)
{
    int
        script_ptr;
        
    if (the_next_event == _LR_NULL_EVENT)
      {
        tcb-> token_posn  = tcb-> script_ptr;
        tcb-> token_width = 0;

        while ((tcb-> the_token = tcb-> script_line [tcb-> script_ptr++]) != '\0')
            if ((tcb-> the_token == ':')              /*  Modifer?  */
            ||  (tcb-> the_token == '%')
            ||  (tcb-> the_token == ')'))             /*  Close?  */
                break;
            else
            if (tcb-> the_token == g_sub_symbol)
              {
                script_ptr = tcb-> script_ptr;
                while (tcb-> script_line [script_ptr] == ' ')
                    script_ptr++;
                if ((tcb-> script_line [script_ptr] == '\0')
                ||  (tcb-> script_line [script_ptr] == '\n'))
                    break;
                else
                  {
                    tcb-> script_ptr++;
                    tcb-> token_width += 2;
                  }
              }
            else
                if (((tcb-> the_token == g_sub_symbol)  /*  Substitute?  */
                &&   (tcb-> script_line [tcb-> script_ptr] == '('))
                ||   (tcb-> the_token == ' '))
                    break;
                else
                    tcb-> token_width++;
        
        tcb-> script_ptr--;               /*  Back to point at terminator      */
        if (tcb-> token_width > 0)
            the_next_event = text_event;
      }
}


/***************************   GET COMMENT TOKEN   ***************************/

MODULE get_comment_token (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    tcb-> prev_posn = tcb-> script_ptr;
    collect_spaces ();

    tcb-> the_token      = tcb-> script_line [tcb-> script_ptr];
    the_next_event = _LR_NULL_EVENT;

    generate_simple_token_event ("*/", close_comment_event);
    generate_end_of_line_event ();
    generate_comment_text_event ();
}


void generate_comment_text_event (void)
{
    if (the_next_event == _LR_NULL_EVENT)
      {
        tcb-> token_posn  = tcb-> script_ptr;
        tcb-> token_width = 0;

        while ((tcb-> the_token = tcb-> script_line [tcb-> script_ptr++]) != '\0')
            if ((tcb-> the_token == '*')  /*  Close?  */
            &&  (tcb-> script_line [tcb-> script_ptr] == '/'))
                break;
            else
                tcb-> token_width++;
        
        tcb-> script_ptr--;               /*  Back to point at terminator      */
        if (tcb-> token_width > 0)
            the_next_event = text_event;
      }
}


/**************************   PUSH QUOTE CHARACTER   *************************/

MODULE push_quote_character (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    list_push (&tcb-> quote_stack, tcb-> the_token);
}


/*************************   INSERT EMPTY TEXT NODE   ************************/

MODULE insert_empty_text_node (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    insert_simple_node (GG_TEXT);
}


/****************************   INSERT TEXT NODE   ***************************/

MODULE insert_text_node (THREAD *thread)
{
    char
        save;

    tcb = thread-> tcb;                 /*  Point to thread's context        */

    insert_simple_node (GG_TEXT);
    /*  After insert_simple_node, tcb-> the_parent points to the new node.   */

    /*  Convoluted way of extracting string straight from source line  */
    save = tcb-> script_line [tcb-> token_posn + tcb-> token_width];
    tcb-> script_line [tcb-> token_posn + tcb-> token_width] = 0;
    if (*tcb-> the_node_ptr)
        (*tcb-> the_node_ptr)-> op1 = 
            (SCRIPT_NODE *) memt_strdup (
                tcb-> memtrn,
                & tcb-> script_line [tcb-> token_posn]);
    else
        tcb-> the_parent-> op1 =
            (SCRIPT_NODE *) memt_strdup (
                tcb-> memtrn,
                & tcb-> script_line [tcb-> token_posn]);

    tcb-> size += strlen (& tcb-> script_line [tcb-> token_posn]) + 1;
    tcb-> script_line [tcb-> token_posn + tcb-> token_width] = save;
}


void insert_simple_node (SCRIPT_NODE_TYPE type)
{
    SCRIPT_NODE
        *new_leaf;

    new_leaf = memt_alloc (tcb-> memtrn, sizeof (SCRIPT_NODE));
    tcb-> size += sizeof (SCRIPT_NODE);
    ASSERT (new_leaf);
    init_script_node (new_leaf);

    new_leaf-> type     = type;
    new_leaf-> brackets = tcb-> brackets;

    tcb-> brackets = 0;

    insert_the_node (new_leaf, OP_UNDEFINED);
    (*tcb-> the_node_ptr)-> brackets += tcb-> brackets;
    tcb-> brackets = 0;

    if (! tcb-> job)
        new_leaf-> dynamic = TRUE;
}


/*  insert_the_node - does the tree-building work.  tcb-> the_node_ptr is a  */
/*  pointer to a pointer (usually in another node, but may be the global     */
/*  variable tcb-> the_root.  The value of *tcb-> the_node_ptr may either be */
/*  NULL, in which case the node *app_node is inserted, or non-NULL, in      */
/*  which case an operator node is inserted between **tcb-> the_node_ptr     */
/*  and *app_node, which may by NULL.  If tcb-> the_node_ptr is NULL then    */
/*  there is no current insertion point indicating an internal error.        */

void insert_the_node (SCRIPT_NODE *app_node, OPERATOR operator)
{
    SCRIPT_NODE
        *new_node;

    ASSERT (tcb-> the_node_ptr);
    if ((operator      != OP_UNDEFINED)
    ||  (*tcb-> the_node_ptr != NULL))
      {
        find_operator_insertion_point (operator);
        
        new_node = memt_alloc (tcb-> memtrn, sizeof (SCRIPT_NODE));
        tcb-> size += sizeof (SCRIPT_NODE);
        ASSERT (new_node);
        init_script_node (new_node);
        
        new_node-> type       = GG_OPERATOR;
        new_node-> operator   = operator;
        new_node-> op1        = *tcb-> the_node_ptr;
        new_node-> op2        = app_node;
        new_node-> parent     = tcb-> the_parent;
        new_node-> width      = tcb-> deferred_width;
        new_node-> spaces     = tcb-> deferred_spaces;
        tcb-> deferred_width  = 0;
        tcb-> deferred_spaces = 0;
        
        /*  Migrate tcb-> brackets, spaces and width to highest level  */
        if (*tcb-> the_node_ptr)
          {
            new_node-> spaces  += (*tcb-> the_node_ptr)-> spaces;
            new_node-> width   += (*tcb-> the_node_ptr)-> width;
            new_node-> brackets = (*tcb-> the_node_ptr)-> brackets;
            (*tcb-> the_node_ptr)-> spaces   = 0;
            (*tcb-> the_node_ptr)-> brackets = 0;
            (*tcb-> the_node_ptr)-> parent   = new_node;
          }

        *tcb-> the_node_ptr = new_node;

        if (app_node)
          {
            app_node -> parent = new_node;

            tcb-> the_node_ptr = &new_node-> op2;
            tcb-> the_parent   = new_node;
          }
      }
    else
      {
        ASSERT (app_node);
        app_node-> parent  = tcb-> the_parent;
        app_node-> width  += tcb-> deferred_width;
        app_node-> spaces += tcb-> deferred_spaces;
        tcb-> deferred_width  = 0;
        tcb-> deferred_spaces = 0;

        *tcb-> the_node_ptr     = app_node;
      }
}


/*  This function finds the tree node to insert a new operator by climbing   */
/*  the tree as long as there is a parent, the parent is an operator node,   */
/*  there are no brackets, it isn't a concatenation on the same line,        */
/*  the priority of the parent's operator is at least as great as the        */
/*  operator being inserted and the parent's operator isn't unary.           */
void find_operator_insertion_point (OPERATOR operator)
{
/*     while (tcb-> the_parent */
/*        &&  tcb-> the_parent-> type == GG_OPERATOR */
/*        && (*tcb-> the_node_ptr ? (*tcb-> the_node_ptr)-> brackets == 0 : TRUE) */
/*        && ( (! tcb-> the_parent-> op1) */
/*        || ( (operator == OP_UNDEFINED && tcb-> the_parent-> operator == OP_UNDEFINED) */
/*           ? ! tcb-> the_parent-> op1-> extend */
/*           : priority [tcb-> the_parent-> operator] >= priority [operator]) ) ) */
    while (tcb-> the_parent
       &&  tcb-> the_parent-> type == GG_OPERATOR
       && (*tcb-> the_node_ptr ? (*tcb-> the_node_ptr)-> brackets == 0 : TRUE)
       && ( (! tcb-> the_parent-> op1)
       || ( priority [tcb-> the_parent-> operator] >= priority [operator]) ) )
        if (return_one_level ())
            return;
}


int return_one_level (void)
{
    ASSERT (tcb-> the_parent);

    if (tcb-> brackets > 0)
      {
        error_exception ("Unexpected token.");
        return -1;
      }
    if (*tcb-> the_node_ptr ? (*tcb-> the_node_ptr)-> brackets > 0 : FALSE)
      {
        error_exception ("Mismatched brackets.");
        return -1;
      }

    /*  Width and spaces of child is added to width of parent  */
    if (*tcb-> the_node_ptr)
      {
        tcb-> the_parent-> width += (*tcb-> the_node_ptr)-> spaces
                                  + (*tcb-> the_node_ptr)-> width;

        if (tcb-> the_node_ptr == & tcb-> the_parent-> op2)
          {
            /*  Migrate line extend flag */
            if ((*tcb-> the_node_ptr)-> extend)
                tcb-> the_parent-> extend = TRUE;
          }
      }

    /*  And deferred width & spaces  */
    tcb-> the_parent-> width += tcb-> deferred_spaces
                              + tcb-> deferred_width;
    tcb-> deferred_spaces = 0;
    tcb-> deferred_width  = 0;

    /*  Find out which child the current parent is */
    if (tcb-> the_parent-> parent == NULL)
        tcb-> the_node_ptr = & tcb-> the_root;
    else
    if (tcb-> the_parent == tcb-> the_parent-> parent-> scope)
        tcb-> the_node_ptr = & tcb-> the_parent-> parent-> scope;
    else
    if (tcb-> the_parent == tcb-> the_parent-> parent-> name)
        tcb-> the_node_ptr = & tcb-> the_parent-> parent-> name;
    else
    if (tcb-> the_parent == tcb-> the_parent-> parent-> op1)
        tcb-> the_node_ptr = & tcb-> the_parent-> parent-> op1;
    else
    if (tcb-> the_parent == tcb-> the_parent-> parent-> op2)
        tcb-> the_node_ptr = & tcb-> the_parent-> parent-> op2;
    else
    if (tcb-> the_parent == tcb-> the_parent-> parent-> as)
        tcb-> the_node_ptr = & tcb-> the_parent-> parent-> as;
    else
    if (tcb-> the_parent == tcb-> the_parent-> parent-> to)
        tcb-> the_node_ptr = & tcb-> the_parent-> parent-> to;
    else
    if (tcb-> the_parent == tcb-> the_parent-> parent-> before)
        tcb-> the_node_ptr = & tcb-> the_parent-> parent-> before;
    else
    if (tcb-> the_parent == tcb-> the_parent-> parent-> after)
        tcb-> the_node_ptr = & tcb-> the_parent-> parent-> after;
    else
      {
        raise_exception (anomaly_event);
        return -1;
      }
    tcb-> the_parent = tcb-> the_parent-> parent;

    return 0;
}


/*************************   INSERT SUBSTITUTE NODE   ************************/

MODULE insert_substitute_node (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    insert_simple_node (GG_SUBSTITUTE);
}


/*************************   READY FOR SCOPE FIELD   *************************/

MODULE ready_for_scope_field (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    ASSERT (tcb-> the_node_ptr);
    ASSERT (*tcb-> the_node_ptr);

    tcb-> the_parent   =    *tcb-> the_node_ptr;
    tcb-> the_node_ptr = & (*tcb-> the_node_ptr)-> scope;

    confirm_node_doesnt_exist ();
}


static void
confirm_node_doesnt_exist (void)
{
    if (*tcb-> the_node_ptr)
        raise_exception (other_event);
}


/**************************   READY FOR NAME FIELD   *************************/

MODULE ready_for_name_field (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    ASSERT (tcb-> the_node_ptr);
    ASSERT (*tcb-> the_node_ptr);

    tcb-> the_parent   =    *tcb-> the_node_ptr;
    tcb-> the_node_ptr = & (*tcb-> the_node_ptr)-> name;

    confirm_node_doesnt_exist ();
}


/**************************   READY FOR OP1 FIELD   **************************/

MODULE ready_for_op1_field (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    ASSERT (tcb-> the_node_ptr);
    ASSERT (*tcb-> the_node_ptr);

    tcb-> the_parent   =    *tcb-> the_node_ptr;
    tcb-> the_node_ptr = & (*tcb-> the_node_ptr)-> op1;

    confirm_node_doesnt_exist ();
}


/**************************   READY FOR OP2 FIELD   **************************/

MODULE ready_for_op2_field (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    ASSERT (tcb-> the_node_ptr);
    ASSERT (*tcb-> the_node_ptr);

    tcb-> the_parent   =    *tcb-> the_node_ptr;
    tcb-> the_node_ptr = & (*tcb-> the_node_ptr)-> op2;

    confirm_node_doesnt_exist ();
}


/***************************   READY FOR AS FIELD   **************************/

MODULE ready_for_as_field (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    ASSERT (tcb-> the_node_ptr);
    ASSERT (*tcb-> the_node_ptr);

    tcb-> the_parent   =    *tcb-> the_node_ptr;
    tcb-> the_node_ptr = & (*tcb-> the_node_ptr)-> as;

    confirm_node_doesnt_exist ();
}


/***************************   READY FOR TO FIELD   **************************/

MODULE ready_for_to_field (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    ASSERT (tcb-> the_node_ptr);
    ASSERT (*tcb-> the_node_ptr);

    tcb-> the_parent   =    *tcb-> the_node_ptr;
    tcb-> the_node_ptr = & (*tcb-> the_node_ptr)-> to;

    confirm_node_doesnt_exist ();
}


/*************************   READY FOR BEFORE FIELD   ************************/

MODULE ready_for_before_field (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    ASSERT (tcb-> the_node_ptr);
    ASSERT (*tcb-> the_node_ptr);

    tcb-> the_parent   =    *tcb-> the_node_ptr;
    tcb-> the_node_ptr = & (*tcb-> the_node_ptr)-> before;

    confirm_node_doesnt_exist ();
}


/*************************   READY FOR AFTER FIELD   *************************/

MODULE ready_for_after_field (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    ASSERT (tcb-> the_node_ptr);
    ASSERT (*tcb-> the_node_ptr);

    tcb-> the_parent   =    *tcb-> the_node_ptr;
    tcb-> the_node_ptr = & (*tcb-> the_node_ptr)-> after;

    confirm_node_doesnt_exist ();
}


/*****************************   SAVE THE SIGN   *****************************/

MODULE save_the_sign (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    tcb-> the_sign = tcb-> the_operator;
}


/****************************   RESTORE THE SIGN   ***************************/

MODULE restore_the_sign (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    tcb-> the_operator = tcb-> the_sign;
    tcb-> the_sign = OP_UNDEFINED;
}


/***************************   COPY THE OPERATOR   ***************************/

MODULE copy_the_operator (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    ASSERT (tcb-> the_node_ptr);
    ASSERT (*tcb-> the_node_ptr);

    (*tcb-> the_node_ptr)-> operator = tcb-> the_operator;
    tcb-> the_operator = OP_UNDEFINED;
}


/****************************   SET EXTEND FLAG   ****************************/

MODULE set_extend_flag (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    ASSERT (tcb-> the_node_ptr);

    if (*tcb-> the_node_ptr)
        (*tcb-> the_node_ptr)-> extend = TRUE;
    else
        tcb-> the_parent-> extend = TRUE;
}


/***************************   UNSET STACKED FLAG   **************************/

MODULE unset_stacked_flag (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    ASSERT (tcb-> the_node_ptr);
    ASSERT (*tcb-> the_node_ptr);

    if ((*tcb-> the_node_ptr)-> stacked)
        (*tcb-> the_node_ptr)-> stacked = FALSE;
    else                                /*  Already unset  */
        raise_exception (other_event);
}


/************************   INSERT SPACES ONLY NODE   ************************/

MODULE insert_spaces_only_node (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    if ((tcb-> token_spaces > 0)
    ||  (*tcb-> the_node_ptr == NULL))
      {
        insert_simple_node (GG_TEXT);
        (*tcb-> the_node_ptr)-> spaces += tcb-> token_spaces;
        tcb-> token_spaces = 0;
      }
}


/**************************   RETURN TO ROOT NODE   **************************/

MODULE return_to_root_node (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    while (tcb-> the_parent)
        if (return_one_level ())
            return;

/*     if (*tcb-> the_node_ptr) */
/*       gg_print (stdout, *tcb-> the_node_ptr); */
/*     else */
/*       printf ("<NULL>\n"); */
}


/****************************   RETURN TO STRING   ***************************/

MODULE return_to_string (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    while (tcb-> the_parent)
        if ((tcb-> the_parent-> type     == GG_OPERATOR)
        &&  (tcb-> the_parent-> operator == OP_UNDEFINED))
          {
            if (return_one_level ())
                return;
          }
        else
            break;

/*     if (*tcb-> the_node_ptr) */
/*       gg_print (stdout, *tcb-> the_node_ptr); */
/*     else */
/*       printf ("<NULL>\n"); */
}


/****************************   RETURN TO PARENT   ***************************/

MODULE return_to_parent (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    if (tcb-> the_parent)
        return_one_level ();
}


/**************************   RETURN TO ARGUMENTS   **************************/

MODULE return_to_arguments (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    while (tcb-> the_parent)
        if ((tcb-> the_parent-> type     == GG_OPERATOR)
        &&  (tcb-> the_parent-> operator == OP_NEXT_ARG))
          {
            if (return_one_level ())
                return;
          }
        else
            break;

/*     if (*tcb-> the_node_ptr) */
/*       gg_print (stdout, *tcb-> the_node_ptr); */
/*     else */
/*       printf ("<NULL>\n"); */
}


/**************************   RETURN TO EXPRESSION   *************************/

MODULE return_to_expression (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    while (tcb-> the_parent)
        if ((tcb-> the_parent-> type     == GG_OPERATOR)
        &&  (tcb-> the_parent-> operator != OP_NEXT_ARG))
          {
            if (return_one_level ())
                return;
          }
        else
            break;

/*     if (*tcb-> the_node_ptr) */
/*       gg_print (stdout, *tcb-> the_node_ptr); */
/*     else */
/*       printf ("<NULL>\n"); */
}


/************************   RETURN TO DATA SPECIFIER   ***********************/

MODULE return_to_data_specifier (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    while (tcb-> the_parent)
        if ((tcb-> the_parent-> type == GG_MEMBER)
        ||  (tcb-> the_parent-> type == GG_ATTRIBUTE))
          {
            if (return_one_level ())
                return;
          }
        else
            break;
}


/**************************   INSERT LITERAL NODE   **************************/

MODULE insert_literal_node (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    insert_simple_node (GG_LITERAL);
}


/***************************   INSERT NUMBER NODE   **************************/

MODULE insert_number_node (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    insert_simple_node (GG_NUMBER);
}


/***************************   COUNT TOKEN WIDTH   ***************************/

MODULE count_token_width (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    ASSERT (tcb-> the_node_ptr);

    if (*tcb-> the_node_ptr)
      {
        (*tcb-> the_node_ptr)-> width  += tcb-> token_width;
        (*tcb-> the_node_ptr)-> spaces += tcb-> token_spaces;
      }
    else
      {
        tcb-> deferred_width  += tcb-> token_width;
        tcb-> deferred_spaces += tcb-> token_spaces;
      }
    tcb-> token_spaces = 0;
}


/***************************   INSERT SYMBOL NODE   **************************/

MODULE insert_symbol_node (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    insert_simple_node (GG_SYMBOL);
}


/***************************   INSERT MEMBER NODE   **************************/

MODULE insert_member_node (THREAD *thread)
{
    insert_data_specifier_node (thread, GG_MEMBER);
    (*tcb-> the_node_ptr)-> stacked = TRUE;
}

static void
insert_data_specifier_node (THREAD *thread, SCRIPT_NODE_TYPE type)
{
    SCRIPT_NODE
        *new_node;

    tcb = thread-> tcb;                 /*  Point to thread's context        */

    new_node = memt_alloc (tcb-> memtrn, sizeof (SCRIPT_NODE));
    tcb-> size += sizeof (SCRIPT_NODE);
    ASSERT (new_node);
    init_script_node (new_node);

    new_node-> type    = type;
    new_node-> scope   = *tcb-> the_node_ptr;
    new_node-> parent  = tcb-> the_parent;

    if (*tcb-> the_node_ptr)
      {
        new_node-> spaces   = (*tcb-> the_node_ptr)-> spaces;
        new_node-> width    = (*tcb-> the_node_ptr)-> width;
        new_node-> brackets = (*tcb-> the_node_ptr)-> brackets;
        (*tcb-> the_node_ptr)-> brackets = 0;
        (*tcb-> the_node_ptr)-> parent = new_node;
      }

    *tcb-> the_node_ptr = new_node;
}


/*************************   INSERT ATTRIBUTE NODE   *************************/

MODULE insert_attribute_node (THREAD *thread)
{
    insert_data_specifier_node (thread, GG_ATTRIBUTE);
}


/****************************   ADD ONE BRACKET   ****************************/

MODULE add_one_bracket (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    tcb-> brackets++;
}


/******************************   REWIND TOKEN   *****************************/

MODULE rewind_token (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    tcb-> script_ptr = tcb-> prev_posn;
    tcb-> token_spaces = 0;
}


/**********************   CHANGE NUMBER TO SYMBOL NODE   *********************/

MODULE change_number_to_symbol_node (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    ASSERT (tcb-> the_node_ptr);
    ASSERT (*tcb-> the_node_ptr);

    (*tcb-> the_node_ptr)-> type = GG_SYMBOL;

    /*  If there is a sign, it belongs to the scope  */
    (*tcb-> the_node_ptr)-> op1-> operator = (*tcb-> the_node_ptr)-> operator;
    (*tcb-> the_node_ptr)-> operator = OP_UNDEFINED;
}


/**************************   CHANGE TO CALL NODE   **************************/

MODULE change_to_call_node (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    ASSERT (tcb-> the_node_ptr);
    ASSERT (*tcb-> the_node_ptr);

    (*tcb-> the_node_ptr)-> type = GG_CALL;
}


/***********************   INSERT UNARY OPERATOR NODE   **********************/

MODULE insert_unary_operator_node (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    insert_simple_node (GG_OPERATOR);
    
    (*tcb-> the_node_ptr)-> operator = tcb-> the_operator;
}


/**************************   INSERT OPERATOR NODE   *************************/

MODULE insert_operator_node (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    insert_the_node (NULL, tcb-> the_operator);

    tcb-> the_operator = OP_UNDEFINED;
    (*tcb-> the_node_ptr)-> brackets += tcb-> brackets;
    tcb-> brackets = 0;
}


/***************************   CLOSE ONE BRACKET   ***************************/

MODULE close_one_bracket (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    ASSERT (tcb-> the_node_ptr);

    if (*tcb-> the_node_ptr == NULL)
        if (tcb-> brackets > 0)
            error_exception ("Unexpected token.");
        else
            raise_exception (no_bracket_event);
    else
      {
        while ( (tcb-> the_parent)
                 ? (((*tcb-> the_node_ptr)-> brackets == 0)
                   && (tcb-> the_parent-> type     == GG_OPERATOR)
                   && (tcb-> the_parent-> operator != OP_NEXT_ARG))
                 : FALSE )
            if (return_one_level ())
                return;

        if ((*tcb-> the_node_ptr)-> brackets > 0)
            (*tcb-> the_node_ptr)-> brackets--;
        else
            raise_exception (no_bracket_event);
      }
}


/************************   SEPARATE NAME FROM SCOPE   ***********************/

MODULE separate_name_from_scope (THREAD *thread)
{
    SCRIPT_NODE
        *node;

    tcb = thread-> tcb;                 /*  Point to thread's context        */

    ASSERT (tcb-> the_node_ptr);
    ASSERT (*tcb-> the_node_ptr);

    node = (*tcb-> the_node_ptr)-> op1;
    (*tcb-> the_node_ptr)-> op1 = NULL;
    if (node)
      {
        /*  If it looks like an attribute, separate scope from attribute  */
        if (node-> type == GG_ATTRIBUTE)
          {
            (*tcb-> the_node_ptr)-> scope = node-> scope;
            (*tcb-> the_node_ptr)-> scope-> parent = *tcb-> the_node_ptr;
            (*tcb-> the_node_ptr)-> name  = node-> name;
            (*tcb-> the_node_ptr)-> name-> parent = *tcb-> the_node_ptr;
            node-> scope = NULL;
            node-> name  = NULL;
            gg_free (node);
          }
        else
        /*  Otherwise if it isn't a member, the whole thing is the name  */
        if (node-> type != GG_MEMBER)
          {
            (*tcb-> the_node_ptr)-> name  = node;
            (*tcb-> the_node_ptr)-> scope = NULL;
          }
        else
        /*  Otherwise the whole thing is the scope  */
          {
            (*tcb-> the_node_ptr)-> name  = NULL;
            (*tcb-> the_node_ptr)-> scope = node;
          }
      }
}


/**********************   INSERT EMPTY NODE IF NEEDED   **********************/

MODULE insert_empty_node_if_needed (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    if (tcb-> the_node_ptr)
        if (!*tcb-> the_node_ptr)
          {
            insert_simple_node (GG_TEXT);
            (*tcb-> the_node_ptr)-> op1 =
                (SCRIPT_NODE *) memt_strdup (tcb-> memtrn, "");
            tcb-> size += 1;
          }
}


/**************************   INSERT COMMENT NODE   **************************/

MODULE insert_comment_node (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    insert_simple_node (GG_COMMENT);
}


/****************************   INSERT LINE NODE   ***************************/

MODULE insert_line_node (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    insert_simple_node (GG_LINE);
    tcb-> token_spaces = 0;             /*  Get rid of spaces preceding >    */
}


/***************************   INSERT OUTPUT NODE   **************************/

MODULE insert_output_node (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    insert_simple_node (GG_OUTPUT);
}


/***************************   INSERT APPEND NODE   **************************/

MODULE insert_append_node (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    insert_simple_node (GG_APPEND);
}


/***************************   INSERT CLOSE NODE   ***************************/

MODULE insert_close_node (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    insert_simple_node (GG_CLOSE);
}


/*****************************   INSERT IF NODE   ****************************/

MODULE insert_if_node (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    insert_simple_node (GG_IF);
}


/***************************   INSERT ELSIF NODE   ***************************/

MODULE insert_elsif_node (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    insert_simple_node (GG_ELSIF);
}


/****************************   INSERT ELSE NODE   ***************************/

MODULE insert_else_node (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    insert_simple_node (GG_ELSE);
}


/***************************   INSERT END IF NODE   **************************/

MODULE insert_end_if_node (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    insert_simple_node (GG_END_IF);
}



/****************************   INSERT FOR NODE   ****************************/

MODULE insert_for_node (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    insert_simple_node (GG_FOR);
    (*tcb-> the_node_ptr)-> stacked = TRUE;
}


/***************************   INSERT SCOPE NODE   ***************************/

MODULE insert_scope_node (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    insert_simple_node (GG_SCOPE);
    (*tcb-> the_node_ptr)-> stacked = TRUE;
}


/**************************   INSERT END FOR NODE   **************************/

MODULE insert_end_for_node (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    insert_simple_node (GG_END_FOR);
}


/*************************   INSERT END SCOPE NODE   *************************/

MODULE insert_end_scope_node (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    insert_simple_node (GG_END_SCOPE);
}


/****************************   INSERT NEW NODE   ****************************/

MODULE insert_new_node (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    insert_simple_node (GG_NEW);
    (*tcb-> the_node_ptr)-> stacked = TRUE;
}


/**************************   INSERT END NEW NODE   **************************/

MODULE insert_end_new_node (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    insert_simple_node (GG_END_NEW);
}


/***************************   INSERT WHILE NODE   ***************************/

MODULE insert_while_node (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    insert_simple_node (GG_WHILE);
}


/*************************   INSERT END WHILE NODE   *************************/

MODULE insert_end_while_node (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    insert_simple_node (GG_END_WHILE);
}


/****************************   INSERT NEXT NODE   ***************************/

MODULE insert_next_node (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    insert_simple_node (GG_NEXT);
}


/****************************   INSERT LAST NODE   ***************************/

MODULE insert_last_node (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    insert_simple_node (GG_LAST);
}


/***************************   INSERT MACRO NODE   ***************************/

MODULE insert_macro_node (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    insert_simple_node (GG_MACRO);
}


/*************************   INSERT END MACRO NODE   *************************/

MODULE insert_end_macro_node (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    insert_simple_node (GG_END_MACRO);
}


/**************************   INSERT FUNCTION NODE   *************************/

MODULE insert_function_node (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    insert_simple_node (GG_FUNCTION);
}


/************************   INSERT END FUNCTION NODE   ***********************/

MODULE insert_end_function_node (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    insert_simple_node (GG_END_FUNCTION);
}


/***************************   INSERT RETURN NODE   **************************/

MODULE insert_return_node (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    insert_simple_node (GG_RETURN);
}


/****************************   INSERT CALL NODE   ***************************/

MODULE insert_call_node (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    insert_simple_node (GG_CALL);
}


/****************************   INSERT GSL NODE   ****************************/

MODULE insert_gsl_node (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    insert_simple_node (GG_GSL);
}


/***************************   INSERT DIRECT NODE   **************************/

MODULE insert_direct_node (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    insert_simple_node (GG_DIRECT);
}


/****************************   INSERT XML NODE   ****************************/

MODULE insert_xml_node (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    insert_simple_node (GG_XML);
}


/**************************   INSERT TEMPLATE NODE   *************************/

MODULE insert_template_node (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    insert_simple_node (GG_TEMPLATE);
}


/************************   INSERT END TEMPLATE NODE   ***********************/

MODULE insert_end_template_node (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    insert_simple_node (GG_END_TEMPLATE);
}


/****************************   INSERT ECHO NODE   ***************************/

MODULE insert_echo_node (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    insert_simple_node (GG_ECHO);
}


/***************************   INSERT ABORT NODE   ***************************/

MODULE insert_abort_node (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    insert_simple_node (GG_ABORT);
}


/***************************   INSERT DEFINE NODE   **************************/

MODULE insert_define_node (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    insert_simple_node (GG_DEFINE);
}


/****************************   INSERT MOVE NODE   ***************************/

MODULE insert_move_node (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    insert_simple_node (GG_MOVE);
}


/****************************   INSERT COPY NODE   ***************************/

MODULE insert_copy_node (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    insert_simple_node (GG_COPY);
}


/***************************   INSERT DELETE NODE   **************************/

MODULE insert_delete_node (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    insert_simple_node (GG_DELETE);
}


/****************************   INSERT SAVE NODE   ***************************/

MODULE insert_save_node (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    insert_simple_node (GG_SAVE);
}


/****************************   INSERT SORT NODE   ***************************/

MODULE insert_sort_node (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    insert_simple_node (GG_SORT);
    (*tcb-> the_node_ptr)-> stacked = TRUE;
}


/*************************   CALL CONTINUE COMMENT   *************************/

MODULE call_continue_comment (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    call_exception (continue_comment_event);
}


/*************************   CALL EXPECT IDENTIFIER   ************************/

MODULE call_expect_identifier (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    call_exception (expect_identifier_event);
}


/***************************   CALL EXPECT NUMBER   **************************/

MODULE call_expect_number (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    call_exception (expect_number_event);
}


/***************************   CALL EXPECT STRING   **************************/

MODULE call_expect_string (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    call_exception (expect_string_event);
}


/***********************   CALL EXPECT DATA SPECIFIER   **********************/

MODULE call_expect_data_specifier (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    call_exception (expect_data_specifier_event);
}


/**************************   CALL ALLOW ARGUMENTS   *************************/

MODULE call_allow_arguments (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    call_exception (allow_arguments_event);
}


/**********************   CALL CONTINUE DATA SPECIFIER   *********************/

MODULE call_continue_data_specifier (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    call_exception (continue_data_specifier_event);
}


/***************************   CALL ALLOW OPERAND   **************************/

MODULE call_allow_operand (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    call_exception (allow_operand_event);
}


/*************************   CALL ALLOW IDENTIFIER   *************************/

MODULE call_allow_identifier (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    call_exception (allow_identifier_event);
}


/*************************   CALL ALLOW MACRO ARGS   *************************/

MODULE call_allow_macro_args (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    call_exception (allow_macro_args_event);
}


/**************************   CALL EXPECT OPERAND   **************************/

MODULE call_expect_operand (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    call_exception (expect_operand_event);
}


/***************************   CALL ALLOW LOCATOR   **************************/

MODULE call_allow_locator (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    call_exception (allow_locator_event);
}


/***************************   CALL ALLOW NUMBER   ***************************/

MODULE call_allow_number (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    call_exception (allow_number_event);
}


/*************************   CALL EXPECT SUBSTITUTE   ************************/

MODULE call_expect_substitute (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    call_exception (expect_substitute_event);
}


/*************************   SIGNAL INTERNAL ERROR   *************************/

MODULE signal_internal_error (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    snprintf (tcb-> error_message, LINE_MAX,
              "Internal parser error.\n");
}


/**********************   ROLLBACK MEMORY ALLOCATIONS   **********************/

MODULE rollback_memory_allocations (THREAD *thread)
{
    char
        ch;

    tcb = thread-> tcb;                 /*  Point to thread's context        */

    mem_rollback (tcb-> memtrn);

    while (!list_empty (& tcb-> quote_stack))
        list_pop (&tcb-> quote_stack, ch);
}


/*************************   SIGNAL INVALID TOKEN   **************************/

MODULE signal_invalid_token (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    error_exception ("Unexpected token.");
}


/*********************   SIGNAL UNEXPECTED END OF FILE   *********************/

MODULE signal_unexpected_end_of_file (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    error_exception ("Unexpected end of file.");
}


/**************************   TERMINATE THE THREAD   *************************/

MODULE terminate_the_thread (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    mem_free (tcb-> expression);
    
    the_next_event = terminate_event;
}

/*****************************************************************************/

void
error_exception (char *format, ...)
{
    int
        offset = 0;
    va_list
        argptr;

    if (*tcb-> script_line)
      {
        offset =  snprintf (tcb-> error_message, LINE_MAX,
                            "%s\n",  tcb-> script_line);
        offset += snprintf (tcb-> error_message + offset, LINE_MAX - offset,
                             "%*s\n", tcb-> token_posn + 1, "^");
      }
    va_start (argptr, format);          /*  Start variable arguments list    */
    offset += vsnprintf (tcb-> error_message + offset, LINE_MAX - offset,
                         format, argptr);
    va_end   (argptr);                  /*  End variable arguments list      */

    raise_exception (error_event);
}

/***************************   RETURN OK FEEDBACK   **************************/

MODULE return_ok_feedback (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    send_ggpars_ok (& thread-> event-> sender,
                    tcb-> job,
                    tcb-> the_root,
                    tcb-> memtrn,
                    tcb-> size);
}


/**************************   RETURN EOF FEEDBACK   **************************/

MODULE return_eof_feedback (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    send_ggpars_eof (& thread-> event-> sender,
                     tcb-> job);
}


/*************************   RETURN ERROR FEEDBACK   *************************/

MODULE return_error_feedback (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    send_ggpars_error (& thread-> event-> sender,
                       tcb-> job,
                       tcb-> error_message);
}
