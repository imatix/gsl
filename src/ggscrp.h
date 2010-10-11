/*---------------------------------------------------------------------------
 *  ggscrp.h - GSL/script package header
 *
 *  Generated from ggscrp by ggobjt.gsl using GSL/4.
 *  DO NOT MODIFY THIS FILE.
 *
 *  For documentation and updates see http://www.imatix.com.
 *---------------------------------------------------------------------------*/

#ifndef GGSCRP_INCLUDED
#define GGSCRP_INCLUDED
/*- Public definitions ------------------------------------------------------*/

typedef struct _SCRIPT_HANDLE {
    struct _SCRIPT_HANDLE
        *next,
        *prev;
    int
        links,
        waiting;                        /*  Use on incomplete file           */
    char
        *path,
        *name;
    Bool
        keep;
    long
        size;
    time_t
        last_closed,
        timestamp;
    LIST
        line_head;
} SCRIPT_HANDLE;

typedef struct _SCRIPT_LINE {
    struct _SCRIPT_LINE
        *next,
        *prev,
        *loop_start,                     /*  Start of loop block             */
        *block_end;                      /*  End of control block            */
    int
        links;
    SCRIPT_HANDLE
        *parent;
    long
        size;
    SCRIPT_NODE
        *node;                           /*  GSL parse tree for GSL lines    */
    char
        *text;                           /*  Text for non-GSL lines          */
    word
        line;
    Bool
        template:1;
} SCRIPT_LINE;

typedef struct {                        /*  Store reference for macros       */
    int
        links;
    SYMTAB
        *symtab;
} MACRO_TABLE_ITEM;

typedef struct {                        /*  Store reference for macros       */
    char
        *name;
    SCRIPT_LINE
        *line;
    SYMBOL
        *symbol;
    SYMTAB
        *symtab;
} MACRO_ITEM;

/**************************** Global variables *******************************/

extern long
    max_size;                           /*  Threshold before freeing space   */

/************************** Function prototypes ******************************/

void script_load_file   (char *filename, Bool template, Bool debug,
                         RESULT_NODE *result, QUEUE *replyq);

void script_load_string (char *name, char *string, Bool template, Bool debug,
                         RESULT_NODE *result, QUEUE *replyq);

void script_handle_link    (SCRIPT_HANDLE *script_handle);
void script_handle_destroy (SCRIPT_HANDLE *script_handle);

MACRO_TABLE_ITEM *macro_table_create  (void);
void              macro_table_link    (MACRO_TABLE_ITEM *macro_table);
void              macro_table_destroy (MACRO_TABLE_ITEM *macro_table);

MACRO_ITEM *macro_create (MACRO_TABLE_ITEM *macro_table, const char *name, SCRIPT_LINE *start);
MACRO_ITEM *macro_lookup (MACRO_TABLE_ITEM *macro_table, const char *name);
void macro_destroy       (MACRO_ITEM *macro);
MACRO_ITEM *macro_value  (LIST *scope_stack, RESULT_NODE *symbol);


/*- Global variables --------------------------------------------------------*/

extern CLASS_DESCRIPTOR
    script_line_class;

/*- Function prototypes -----------------------------------------------------*/

int register_script_line_classes (void);

int shutdown_script_line_classes (void);

#endif
