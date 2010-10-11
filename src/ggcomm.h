/*===========================================================================*
 *                                                                           *
 *  ggcomm.h - Common functions                                              *
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

#ifndef GGCOMM_INCLUDED                 /*  Allow multiple inclusions        */
#define GGCOMM_INCLUDED

/*- Macros ------------------------------------------------------------------*/

#define LONG_WIDTH 10
#define PATH       "PATH"

#define BLOCK_START_MARKER 1
#define BLOCK_END_MARKER   2
#define BLOCK_DONE_MARKER  3

#define copy_value(d,s)       copy_value_ (NULL, d, s)

#define string_result(r) ((r) ? string_value (& (r)-> value) : NULL)
#define number_result(r) ((r) ? number_value (& (r)-> value) : NULL)

/*- Type definitions --------------------------------------------------------*/

typedef struct _SCRIPT_NODE      SCRIPT_NODE;
typedef struct _CLASS_DESCRIPTOR CLASS_DESCRIPTOR;
typedef struct _RESULT_NODE      RESULT_NODE;


typedef enum { GG_COMMENT,
               GG_LINE,
               GG_TEXT,
               GG_SUBSTITUTE,
               GG_OPERATOR,
               GG_LITERAL,
               GG_NUMBER,
               GG_SYMBOL,
               GG_MEMBER,
               GG_ATTRIBUTE,
               GG_CALL,
               GG_OUTPUT,
               GG_APPEND,
               GG_CLOSE,
               GG_IF,
               GG_ELSIF,
               GG_ELSE,
               GG_END_IF,
               GG_FOR,
               GG_END_FOR,
               GG_SCOPE,
               GG_END_SCOPE,
               GG_NEW,
               GG_END_NEW,
               GG_DELETE,
               GG_MOVE,
               GG_COPY,
               GG_WHILE,
               GG_END_WHILE,
               GG_NEXT,
               GG_LAST,
               GG_MACRO,
               GG_END_MACRO,
               GG_FUNCTION,
               GG_END_FUNCTION,
               GG_RETURN,
               GG_GSL,
               GG_DIRECT,
               GG_XML,
               GG_TEMPLATE,
               GG_END_TEMPLATE,
               GG_ECHO,
               GG_ABORT,
               GG_DEFINE,
               GG_SAVE,
               GG_SORT,
               GG_UNDEFINED }
    SCRIPT_NODE_TYPE;


typedef enum { OP_UNDEFINED,
               OP_TIMES,
               OP_DIVIDE,
               OP_PLUS,
               OP_MINUS,
               OP_IIF,
               OP_DEFAULT,
               OP_EQUALS,
               OP_NOT_EQUALS,
               OP_GREATER_THAN,
               OP_LESS_THAN,
               OP_GREATER_EQUAL,
               OP_LESS_EQUAL,
               OP_SAFE_EQUALS,
               OP_SAFE_NOT_EQUALS,
               OP_SAFE_GREATER_THAN,
               OP_SAFE_LESS_THAN,
               OP_SAFE_GREATER_EQUAL,
               OP_SAFE_LESS_EQUAL,
               OP_NOT,
               OP_OR,
               OP_AND,
               OP_NEXT_ARG }
    OPERATOR;


typedef enum { TYPE_UNDEFINED,
               TYPE_STRING,
               TYPE_NUMBER,
               TYPE_BLOCK,
               TYPE_POINTER,
               TYPE_UNKNOWN }
    DATA_TYPE;

typedef struct {
    DATA_TYPE
        type;
    char
       *s;                              /*  String value                     */
    double
        n;                              /*  Numeric value                    */
    char
      **b;                              /*  Block value                      */
    CLASS_DESCRIPTOR
        *c;                             /*  Pointer value - class            */
    void
        *i;                             /*  Pointer value - item             */
} VALUE;

struct _SCRIPT_NODE {
    SCRIPT_NODE
        *parent;
    SCRIPT_NODE
        *scope,
        *name,
        *op1,
        *op2,
        *as,
        *to,
        *before,                        /*  Also 'where'                     */
        *after;                         /*  Also 'by'                        */
    VALUE          
        result;                         /*  Only used if result is constant  */
    int  
        width:16,                       /*  Width of script construct        */
        spaces:16,                      /*  Leading spaces in script         */
        brackets:16;
    OPERATOR
        operator:8;
    SCRIPT_NODE_TYPE
        type:8;
    int
        extend:1,
        constant:1,                     /*  Is the result constant?          */
        stacked:1,                      /*  Is scope to be 'stacked'?        */
        dynamic:1;                      /*  TRUE if job == 0.                */
};

typedef struct {
    CLASS_DESCRIPTOR
        *class;
    void
        *item;
} CLASS_ITEM;

typedef enum {
    PARM_VALUE,
    PARM_REFERENCE,
    PARM_SIMPLE_SCOPE,
    PARM_EXPRESSION
} PARM_TYPE;

typedef
    PARM_TYPE PARM_LIST [];

typedef int  PLUGIN_INITIALISE (CLASS_DESCRIPTOR **class,
                                void             **item,
                                THREAD            *gsl_thread);

typedef const char * ITEM_NAME_FUNCTION (void *item);
typedef VALUE *      GET_ATTR_FUNCTION (      void *item,
                                        const char *name,
                                              Bool ignorecase);
typedef int  PUT_ATTR_FUNCTION (      void  *item,
                                const char  *name,
                                      VALUE *value,
                                      Bool  ignorecase);
typedef int  NAVIGATE_FUNCTION (void              *olditem,
                                const       char  *name,
                                            Bool   ignorecase,
                                CLASS_DESCRIPTOR **class,
                                            void **item);
typedef int  PARENT_FUNCTION   (void              *olditem,
                                CLASS_DESCRIPTOR **class,
                                void             **item,
                                THREAD            *gsl_thread);
typedef int  CREATE_FUNCTION   (const       char  *name,
                                            void  *parent,
                                            void  *sibling,
                                CLASS_DESCRIPTOR **class,
                                            void **item);
typedef int  ITEM_FUNCTION     (void  *item);
typedef void * ATTACH_FUNCTION (            void *item,
                                CLASS_DESCRIPTOR *to_class,
                                const       char *name,
                                            void *parent,
                                            void *sibling);
typedef void  REMOVE_FUNCTION  (void *item, void *remove);
                           
typedef int EVAL_FUNCTION (int argc,
                           RESULT_NODE **argv,
                           void        *item,
                           RESULT_NODE *result,
                           THREAD *gsl_thread);

typedef int THREAD_FUNCTION (THREAD *gsl_thread);

typedef struct {
    char
       *name;
    int
        min_parmc,
        max_parmc,
        cnt_parmt;
    PARM_LIST
       *parmt;
    Bool
        immediate;
    EVAL_FUNCTION
       *evaluate;
} GSL_FUNCTION;

struct _CLASS_DESCRIPTOR {
    const char
        *name;
    ITEM_NAME_FUNCTION
        *item_name;
    GET_ATTR_FUNCTION
        *get_attr;
    PUT_ATTR_FUNCTION
        *put_attr;
    NAVIGATE_FUNCTION
        *first_child,
        *next_sibling;
    PARENT_FUNCTION
        *parent;
    CREATE_FUNCTION
        *create;
    ITEM_FUNCTION
        *destroy;
    ITEM_FUNCTION
        *link,
        *delete;
    ATTACH_FUNCTION
        *copy,
        *move;
    REMOVE_FUNCTION
        *remove;
    GSL_FUNCTION
        *methods;
    int
        method_cnt;
};

struct _RESULT_NODE {
    RESULT_NODE
        *next,                          /*  In stack or cache                */
        *parent,
        *scope,
        *name,
        *op1,
        *op2,
        *as,
        *to,
        *before,
        *after,
        *operand;                       /*  Result of evaluating operand     */
    SCRIPT_NODE
        *script_node;                   /*  Corresponding parse node         */
    RESULT_NODE
        *result_node;
    void
        *macro;
    GSL_FUNCTION
        *gsl_function;
    int
        argc;
    RESULT_NODE
      **argv;
    char
        *culprit;
    int
        indent,                         /*  Leading spaces after shuffling   */
        width,                          /*  Width of last line of result     */
        item_nbr;
    VALUE
        value;                          /*  Result                           */
    Bool
        constant;                       /*  Is result constant?              */
};

/*- Functions ---------------------------------------------------------------*/

Bool         gsl_file_read              (FILE *stream,
                                         char *string);
char *       name_the_symbol            (RESULT_NODE *node);
void         result_node_error          (const char *message,
                                         RESULT_NODE *node,
                                         char **error_text);
char        *extended_scope_string      (RESULT_NODE *scope);
void         init_value                 (VALUE  *value);
void         destroy_value              (VALUE  *value);
void         copy_value_                (MEMTRN *memtrn,
                                         VALUE  *dest,
                                         VALUE  *source);
void         assign_number              (VALUE  *dest,
                                         double n);
void         assign_string              (VALUE *dest,
                                         char  *s);
void         assign_pointer             (VALUE            *dest,
                                         CLASS_DESCRIPTOR *c,
                                         void             *i);
void         free_pointer               (CLASS_DESCRIPTOR *c,
                                         void *i);
RESULT_NODE *new_result_node            (void);
void         init_result                (RESULT_NODE *node);
void         destroy_result             (RESULT_NODE *node);
void         copy_result                (RESULT_NODE *dest,
                                         RESULT_NODE *source);

char        *string_value               (VALUE *value);
double       number_value               (VALUE *value);
int          parse_format               (char *format, size_t format_max,
                                         char *conversion,
                                         RESULT_NODE *node,
                                         char **error_text);
int          format_output              (char *format, 
                                         char conversion, 
                                         int width,
                                         RESULT_NODE *node);
int          pretty_print               (VALUE *result,
                                         RESULT_NODE *pretty,
                                         char *example,
                                         int  space,
                                         char **error_text);
size_t       strllen                    (const char *s);
char        *concatenate_results        (RESULT_NODE *r,
                                         int shuffle,
                                         Bool convert_indent,
                                         char **error_text);
void         destroy_caches             (void);

/*---------------------------------------------------------------------------*/

#endif
