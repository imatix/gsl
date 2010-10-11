/*---------------------------------------------------------------------------
 *  ggscrp.c - GSL/script package
 *
 *  Generated from ggscrp by ggobjt.gsl using GSL/4.
 *  DO NOT MODIFY THIS FILE.
 *
 *  For documentation and updates see http://www.imatix.com.
 *---------------------------------------------------------------------------*/

#include "sfl.h"
#include "smt3.h"
#include "gsl.h"                        /*  Project header file              */
#include "ggscrp.h"                     /*  Include header file              */
#include "ggscrp.d"                     /*  Include dialog data              */

#define AGENT_NAME "GGSCRP"

/*- Macros ------------------------------------------------------------------*/

#define SCRIPT_LINE_NAME "script_line"  /*  GSL Script Line                  */

#define matches(s,t)    (s ? (ignorecase ? lexcmp (s, t) == 0 : streq (s, t))   : t == NULL)

/*- Function prototypes -----------------------------------------------------*/

static int script_line_link (void *item);
static int script_line_destroy (void *item);
static VALUE * script_line_get_attr (void *item, const char *name, Bool ignorecase);

/*- Global variables --------------------------------------------------------*/

CLASS_DESCRIPTOR
    script_line_class  = {
        "script_line",
        NULL,
        script_line_get_attr,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        script_line_destroy,
        script_line_link,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL, 0 };


typedef struct {
    SCRIPT_NODE_TYPE
        type;
    Bool
        template;
    SCRIPT_LINE
        *script_line;
} CONTROL;

typedef struct
{
    MEMTRN
        *memtrn;
    event_t
        thread_type;
    QID
        replyqid;
    RESULT_NODE
        *result;
    SEMAPH
       *semaph;
    SCRIPT_READ
       *script_read;
    SCRIPT_NODE
       *script_node;
    SCRIPT_HANDLE
       *script_handle;
    SCRIPT_LINE
       *script_line;
    FILE
       *script_file;
    char
        *path,
        *name,
        *text,
        *script_ptr,
        *line_text;
    Bool
        template,
        debug,
        end_of_file,
        error;
    word
        line_nbr,
        start_line;
    LIST
        control_stack;
    CONTROL
        control;
    SCRIPT_LINE
       *loop_start;
} TCB;

/*- Global variables --------------------------------------------------------*/

long
    max_size;

/*- Global variables used in this source file only --------------------------*/

static TCB
    *tcb;                               /*  Address thread context block     */

static long
    total_size;                         /*  Total space taken by parse trees */

static LIST
    script_list;

static event_t type_event [] = {
    other_event,          /*  GG_COMMENT        */
    other_event,          /*  GG_LINE           */
    anomaly_event,        /*  GG_TEXT           */
    anomaly_event,        /*  GG_SUBSTITUTE     */
    anomaly_event,        /*  GG_OPERATOR       */
    other_event,          /*  GG_LITERAL        */
    anomaly_event,        /*  GG_NUMBER         */
    anomaly_event,        /*  GG_SYMBOL         */
    anomaly_event,        /*  GG_MEMBER         */
    anomaly_event,        /*  GG_ATTRIBUTE      */
    other_event,          /*  GG_CALL           */
    other_event,          /*  GG_OUTPUT         */
    other_event,          /*  GG_APPEND         */
    other_event,          /*  GG_CLOSE          */
    if_event,             /*  GG_IF             */
    else_if_event,        /*  GG_ELSIF          */
    else_event,           /*  GG_ELSE           */
    end_if_event,         /*  GG_END_IF         */
    for_event,            /*  GG_FOR            */
    end_for_event,        /*  GG_END_FOR        */
    scope_event,          /*  GG_SCOPE          */
    end_scope_event,      /*  GG_END_SCOPE      */
    new_event,            /*  GG_NEW            */
    end_new_event,        /*  GG_END_NEW        */
    other_event,          /*  GG_DELETE         */
    other_event,          /*  GG_MOVE           */
    other_event,          /*  GG_COPY           */
    while_event,          /*  GG_WHILE          */
    end_while_event,      /*  GG_END_WHILE      */
    other_event,          /*  GG_NEXT           */
    other_event,          /*  GG_LAST           */
    macro_event,          /*  GG_MACRO          */
    end_macro_event,      /*  GG_END_MACRO      */
    function_event,       /*  GG_FUNCTION       */
    end_function_event,   /*  GG_END_FUNCTION   */
    other_event,          /*  GG_RETURN         */
    other_event,          /*  GG_GSL            */
    other_event,          /*  GG_DIRECT         */
    other_event,          /*  GG_XML            */
    template_event,       /*  GG_TEMPLATE       */
    end_template_event,   /*  GG_END_TEMPLATE   */
    other_event,          /*  GG_ECHO           */
    other_event,          /*  GG_ABORT          */
    other_event,          /*  GG_DEFINE         */
    other_event,          /*  GG_SAVE           */
    other_event,          /*  GG_SORT           */
    _LR_NULL_EVENT };     /*  GG_UNDEFINED      */


/************************** Local function prototypes ************************/

static void really_destroy_script_handle (SCRIPT_HANDLE *script_handle);
static void destroy_script_handle_lines  (SCRIPT_HANDLE *script_handle);
static void destroy_one_script_line      (SCRIPT_LINE *script_line);
static void initialise_tcb               (TCB *tcb);
static void create_script_handle         (THREAD *thread);
static Bool file_script_read             (JOBID job, char *text);
static Bool text_script_read             (JOBID job, char *text);
static Bool free_excess_scripts          (long size);
static void push_control                 (SCRIPT_NODE_TYPE type,
                                          Bool template,
                                          SCRIPT_LINE *script_line);
static void pop_control                  (THREAD *thread,
                                          SCRIPT_NODE_TYPE type1,
                                          SCRIPT_NODE_TYPE type2);
static Bool destroy_macro_item           (SYMBOL *symbol);
static void log_error                    (TCB *tcb, char *format, ...);

/********************************  Entry points ******************************/

void
script_load_file (char *filename, Bool template, Bool debug,
                  RESULT_NODE *result, QUEUE *replyq)
{
    SCRIPT_HANDLE
       *script_handle;
    char
        *fname,
        *path,
        *name;
    THREAD
       *thread;
    SEMAPH
       *semaph = NULL;

    /*  If the script is already loaded, don't bother starting a thread,     */
    /*  just send the OK event.                                              */
    fname = memt_strdup (NULL,
                         file_where ('r', PATH, filename, "gsl"));
    if (! fname)
      {
        lsend_ggcode_call_error (& replyq-> qid, NULL,
                                 NULL, NULL, NULL, 0,
                                 NULL, 0,
                                 strprintf ("File: %s not found", filename));
        return;
      }

    path = memt_strdup (NULL, strip_file_name (fname));
    name = strip_file_path (memt_strdup (NULL, fname));

    FORLIST (script_handle, script_list)
        if (script_handle-> name
        &&  script_handle-> path
        &&  streq (script_handle-> name, name)
        &&  streq (script_handle-> path, path))
          {
            semaph = semaph_lookup (fname);
            if (! semaph)
              {
                /*  If file has disappeared or changed...  */
                if (script_handle-> timestamp != get_file_time (fname))
                  {
                    if (script_handle-> links   == 0
                    &&  script_handle-> waiting == 0)
                        really_destroy_script_handle (script_handle);
                    else
                        /*  Flag for destruction when other threads are done */
                        script_handle-> keep = FALSE;
                  }
                else                    /*  Re-use parsed script  */
                  {
                    script_handle_link (script_handle);
                    result-> value. i = script_handle;

                    lsend_ggcode_call_ok (
                        & replyq-> qid, NULL,
                        NULL, NULL, NULL, 0);

                    mem_free (fname);
                    mem_free (path);
                    mem_free (name);

                    return;
                  }
              }
            break;
          }

    /*  Create thread to parse file  */
    thread = thread_create (AGENT_NAME, "");

    tcb = thread-> tcb;
    initialise_tcb (tcb);

    tcb-> replyqid = replyq-> qid;
    tcb-> result   = result;
    tcb-> path     = path;
    tcb-> name     = name;

    /*  If we are waiting for another thread to parse this file, set up       */
    /*  thread to wait for other thread to finish.                            */
    if (semaph)
      {
        tcb-> thread_type   = wait_event;
        tcb-> script_handle = script_handle;
        script_handle-> waiting++;
      }
    else
      {
        semaph = semaph_create (fname, 1);

        tcb-> thread_type = file_event;

        create_script_handle (thread);
        tcb-> script_handle-> keep = TRUE;

        tcb-> script_read = file_script_read;
        tcb-> template    = template;
        tcb-> debug       = debug;
      }

    tcb-> semaph = semaph;

    mem_free (fname);

    return;
}

static void
really_destroy_script_handle (SCRIPT_HANDLE *script_handle)
{
    destroy_script_handle_lines (script_handle);

    /*  If script lines remain, don't delete handle yet, but unset keep flag */
    /*  and scrap file name and path.                                        */
    
    /*  Destroy the path now, indicating that this isn't a full script, but  */
    /*  keep the filename so that we still have meaningful error messages.   */
    mem_strfree (& script_handle-> path);
    if (! list_empty (& script_handle-> line_head))
        script_handle-> keep = FALSE;
    else
      {
        mem_strfree (& script_handle-> name);
        list_unlink (script_handle);
        mem_free    (script_handle);
      }
}

static void
destroy_script_handle_lines (SCRIPT_HANDLE *script_handle)
{
    SCRIPT_LINE
        *script_line,
        *block_end,
        *next_line;

    script_line = (SCRIPT_LINE *) script_handle-> line_head. next;
    while (script_line &&
           script_line != (SCRIPT_LINE *) & script_handle-> line_head)
      {
        block_end = script_line-> block_end;
        if (block_end && block_end-> links)
          {
            /*  Nullify link to block end if it's about to be destroyed.     */
            if (block_end-> block_end
            &&  block_end-> block_end-> links == 0)
                block_end-> block_end = NULL;
                
            script_line = block_end-> next;
          }
        else
          {
            next_line = script_line-> next;
            destroy_one_script_line (script_line);
            /*  Zap last line of block manually because its block_end        */
            /*  field refers to next highest block.                          */
            if (next_line == block_end)
              {
                script_line = next_line;
                next_line = script_line-> next;
                destroy_one_script_line (script_line);
              }
            script_line = next_line;
          }
      }

}

static void
destroy_one_script_line (SCRIPT_LINE *script_line)
{
    if (script_line-> parent)
        script_line-> parent-> size -= script_line-> size;
    total_size -= script_line-> size;

    /*  Copied from sfllist with provision for NULL prev or next pointer  */
    if (script_line-> prev)
        script_line-> prev-> next = script_line-> next;
    if (script_line-> next)
        script_line-> next-> prev = script_line-> prev;

    gg_free  (script_line-> node);
    mem_free (script_line-> text);
    mem_free (script_line);
}

static void
initialise_tcb (TCB *tcb)
{
    tcb-> memtrn               = mem_new_trans ();
    tcb-> thread_type          = SMT_NULL_EVENT;
    tcb-> result               = NULL;
    tcb-> semaph               = NULL;
    tcb-> script_read          = NULL;
    tcb-> script_node          = NULL;
    tcb-> script_handle        = NULL;
    tcb-> script_line          = NULL;
    tcb-> script_file          = NULL;
    tcb-> path                 = NULL;
    tcb-> name                 = NULL;
    tcb-> text                 = NULL;
    tcb-> script_ptr           = NULL;
    tcb-> line_text            = NULL;
    tcb-> template             = FALSE;
    tcb-> debug                = FALSE;
    tcb-> end_of_file          = FALSE;
    tcb-> error                = FALSE;
    tcb-> line_nbr             = 0;
    tcb-> start_line           = 0;
    tcb-> control. type        = GG_UNDEFINED;
    tcb-> control. script_line = NULL;
    tcb-> loop_start           = NULL;

    list_reset (& tcb-> control_stack);
}

static void
create_script_handle (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    tcb-> script_handle = memt_alloc (tcb-> memtrn,
                                      sizeof (SCRIPT_HANDLE));
    ASSERT (tcb-> script_handle);
    list_reset (tcb-> script_handle);

    tcb-> script_handle-> path        = memt_strdup (tcb-> memtrn,
                                                     tcb-> path);
    tcb-> script_handle-> name        = memt_strdup (tcb-> memtrn,
                                                     tcb-> name);
    tcb-> script_handle-> keep        = FALSE;
    tcb-> script_handle-> size        = 0;
    tcb-> script_handle-> links       = 0;
    tcb-> script_handle-> waiting     = 1;    /*  Count ourselves already  */
    tcb-> script_handle-> last_closed = 0;
    tcb-> script_handle-> timestamp   = 0;

    list_reset (& tcb-> script_handle-> line_head);
    list_relink_after (tcb-> script_handle, & script_list);
}

static Bool
file_script_read (JOBID job, char *text)
{
    THREAD
        *thread;
    int
        length;

    thread = (THREAD *) job;
    tcb = thread-> tcb;

    ASSERT (job);
    ASSERT (text);

    if (! tcb-> script_file || ! gsl_file_read (tcb-> script_file, text))
        return FALSE;

    tcb-> line_nbr++;
    if (tcb-> debug)
      {
        if (tcb-> line_text)
          {
            length = strlen (tcb-> line_text);
            tcb-> line_text = mem_realloc (tcb-> line_text,
                                           length + strlen (text) + 1);
            strcpy (tcb-> line_text + length, text);
          }
        else
            tcb-> line_text = memt_strdup (tcb-> memtrn, text);
      }
    return TRUE;
}

void
script_load_string (char *name, char *string, Bool template, Bool debug,
                    RESULT_NODE *result, QUEUE *replyq)
{
    THREAD
       *thread;

    /*  Create thread to parse file  */
    thread = thread_create (AGENT_NAME, "");
    tcb = thread-> tcb;

    initialise_tcb (tcb);

    /*  Context is provided by GSL item, ie 1 per thread.  */
    tcb-> result   = result;
    tcb-> replyqid = replyq-> qid;

    tcb-> thread_type = string_event;
    tcb-> script_read = text_script_read;
    tcb-> template    = template;
    tcb-> debug       = debug;
    tcb-> path        = NULL;
    tcb-> name        = memt_strdup (thread-> memtrn, name);
    tcb-> text        = memt_strdup (thread-> memtrn, string);

    create_script_handle (thread);

    return;
}

static Bool
text_script_read (JOBID job, char *text)
{
    THREAD
        *thread;
    char
       *endptr;
    int
        len;
    int
        length;

    thread = (THREAD *) job;
    tcb = thread-> tcb;

    /* Handle blank line separately - strtok doesn't like empty tokens */
    if (! *tcb-> script_ptr)
        return FALSE;

    endptr = strchr (tcb-> script_ptr, '\n');
    if (endptr)
        len = endptr - tcb-> script_ptr + 1;
    else
        len = strlen (tcb-> script_ptr);

    if (len > LINE_MAX)
        len = LINE_MAX;

    strncpy (text, tcb-> script_ptr, len);
    text [len] = '\0';

    if (endptr)
        tcb-> script_ptr = endptr + 1;
    else
        tcb-> script_ptr += len;

    tcb-> line_nbr++;
    if (tcb-> debug)
      {
        if (tcb-> line_text)
          {
            length = strlen (tcb-> line_text);
            tcb-> line_text = mem_realloc (tcb-> line_text,
                                           length + strlen (text) + 1);
            strcpy (tcb-> line_text + length, text);
          }
        else
            tcb-> line_text = memt_strdup (tcb-> memtrn, text);
      }
    return TRUE;
}


void
script_handle_link (SCRIPT_HANDLE *script_handle)
{
    script_handle-> links++;
}

void
script_handle_destroy (SCRIPT_HANDLE *script_handle)
{
    script_handle-> links--;
    ASSERT (script_handle-> links >= 0);
    if (  script_handle-> links == 0
    && (! script_handle-> keep))
        really_destroy_script_handle (script_handle);

    free_excess_scripts (max_size);
}

static Bool
free_excess_scripts (long size)
{
    SCRIPT_HANDLE
        *script_handle,
        *oldest_source;

    while (total_size > size)
      {
        oldest_source = NULL;
        FORLIST (script_handle, script_list)
            if (script_handle-> links   == 0
            &&  script_handle-> waiting == 0
            && ((SCRIPT_LINE *) script_handle-> line_head. prev)-> links == 0
            && ((! oldest_source)
            ||  script_handle-> last_closed < oldest_source-> last_closed))
                oldest_source = script_handle;

        if (oldest_source)
            really_destroy_script_handle (oldest_source);
        else
            return FALSE;               /*  Unsuccessful                     */
      }
    return TRUE;                        /*  Successful                       */
}

/*- Macro Functions ---------------------------------------------------------*/

MACRO_TABLE_ITEM *
macro_table_create (void)
{
    MACRO_TABLE_ITEM
        *macro_table;

    macro_table = memt_alloc (NULL, sizeof (MACRO_TABLE_ITEM));
    macro_table-> links  = 0;
    macro_table-> symtab = sym_create_table ();

    return macro_table;
}

void
macro_table_link (MACRO_TABLE_ITEM *macro_table)
{
    macro_table-> links++;
}

void
macro_table_destroy (MACRO_TABLE_ITEM *macro_table)
{
    macro_table-> links--;
    ASSERT (macro_table-> links >= 0);
    if (macro_table-> links == 0)
      {
        sym_exec_all (macro_table-> symtab, destroy_macro_item);
        sym_delete_table (macro_table-> symtab);
        mem_free (macro_table);
      }
}

static Bool
destroy_macro_item (SYMBOL *symbol)
{
    macro_destroy (symbol-> data);
    return TRUE;
}

MACRO_ITEM *
macro_create (MACRO_TABLE_ITEM *macro_table, const char *name, SCRIPT_LINE *start)
{
    MACRO_ITEM
        *macro;

    macro = memt_alloc (NULL, sizeof (MACRO_ITEM));
    macro-> name     = memt_strdup (NULL, name);
    macro-> line     = start;
    script_line_link (macro-> line);
    macro-> symtab   = macro_table-> symtab;
    macro-> symbol   = sym_create_symbol (macro_table-> symtab,
                                          macro-> name,
                                          NULL);
    macro-> symbol-> data = macro;

    return macro;
}

MACRO_ITEM *
macro_lookup (MACRO_TABLE_ITEM *macro_table, const char *name)
{
    SYMBOL
        *symbol;

    symbol = sym_lookup_symbol (macro_table-> symtab,
                                name);

    return symbol ? symbol-> data : NULL;
}

void
macro_destroy (MACRO_ITEM *macro)
{
    mem_free (macro-> name);
    script_line_destroy (macro-> line);
    sym_delete_symbol (macro-> symtab,
                       macro-> symbol);
    mem_free (macro);
}

MACRO_ITEM *
macro_value (LIST *scope_stack,
             RESULT_NODE *symbol)
{
    SCOPE_BLOCK
        *scope_block;
    CLASS_DESCRIPTOR
        *dummyclass;
    void
        *dummyitem;
    MACRO_ITEM
        *macro;

    if (symbol-> scope)
      {
        scope_block = lookup_simple_scope (scope_stack,
                                           & symbol-> scope-> value,
                                           FALSE,
                                           &dummyclass,
                                           &dummyitem,
                                           NULL);
        if (scope_block && scope_block-> macros)
            return macro_lookup (scope_block-> macros,
                                 string_value (& symbol-> name-> value));
      }
    else
      {
        FORLIST (scope_block, *scope_stack)
            if (scope_block-> macros)
              {
                macro = macro_lookup (scope_block-> macros,
                                      string_value (& symbol-> name-> value));
                if (macro)
                    return macro;
              }
      }
    return NULL;

}


/*************************   INITIALISE THE THREAD   *************************/

MODULE initialise_the_thread (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    the_next_event = tcb-> thread_type;
}


/**************************   TERMINATE THE THREAD   *************************/

MODULE terminate_the_thread (THREAD *thread)
{
    char
        *fname;
    SEMAPH
       *semaph;

    tcb = thread-> tcb;                 /*  Point to thread's context        */

    if (tcb-> script_file)
        file_close (tcb-> script_file);

    /*  If errors occurred then tcb-> script_handle still has a value  */
    if (tcb-> script_handle)
      {
        if (tcb-> script_handle-> waiting == 0)
            really_destroy_script_handle (tcb-> script_handle);
        else
          {
            /*  Keep script handle for other thread  */
            destroy_script_handle_lines  (tcb-> script_handle);
            mem_commit (tcb-> memtrn);
            tcb-> memtrn = NULL;
          }
      }

    if (tcb-> thread_type == wait_event
    ||  tcb-> thread_type == file_event)
      {
        fname = mem_strdup (file_where ('r', tcb-> path, tcb-> name, NULL));
        semaph = semaph_lookup (fname);
        if (semaph)
            semaph_destroy (semaph);
        mem_free (fname);
      }

    mem_free (tcb-> path);
    mem_free (tcb-> name);
    mem_free (tcb-> text);
    mem_free (tcb-> line_text);

    if (tcb-> memtrn)
      {
        memt_assert  (tcb-> memtrn);
        mem_rollback (tcb-> memtrn);
      }

    the_next_event = terminate_event;
}


/***************************   WAIT ON SEMAPHORE   ***************************/

MODULE wait_on_semaphore (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    if (tcb-> semaph)
        semaph_wait (tcb-> semaph);
}


/*****************************   OPEN THE FILE   *****************************/

MODULE open_the_file (THREAD *thread)
{
    char
        *fullname;

    tcb = thread-> tcb;                 /*  Point to thread's context        */

    fullname = xstrcpy (NULL, tcb-> path, "/", tcb-> name, NULL);

    tcb-> script_handle-> timestamp = get_file_time (fullname);
    tcb-> script_file = file_open (fullname, 'r');

    mem_free (fullname);

    if (! tcb-> script_file)
      {
        tcb-> error = TRUE;
        raise_exception (error_event);
      }
}


/*****************************   OPEN THE TEXT   *****************************/

MODULE open_the_text (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    tcb-> script_ptr = tcb-> text;
}


/*******************************   PARSE LINE   ******************************/

MODULE parse_line (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    /*  Save line number in case command is multi-line */
    tcb-> start_line = tcb-> line_nbr + 1;

    /*  Reset line text  */
    tcb-> line_text = NULL;

    if (tcb-> template)
        gg_parse_template (tcb-> script_read,
                           thread,
                           thread-> queue);
    else
        gg_parse_gsl (tcb-> script_read,
                      thread,
                      thread-> queue);
}


/************************   SEND FILE ERROR MESSAGE   ************************/

MODULE send_file_error_message (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    log_error (thread-> tcb, "Error opening file: %s\n%s",
                             tcb-> name,
                             strerror (errno));
}

static void
log_error (TCB *tcb, char *format, ...)
{
    va_list
        argptr;
    char
        buffer [LINE_MAX];
    int
        length;

    va_start (argptr, format);          /*  Start variable arguments list    */
    length = vsnprintf (buffer, LINE_MAX, format, argptr);
    ASSERT (length != -1);
    va_end (argptr);                    /*  End variable arguments list      */

    tcb-> error = TRUE;

    lsend_ggcode_call_message (& tcb-> replyqid, NULL,
                               NULL, NULL, NULL, 0,
                               tcb-> name,
                               tcb-> start_line,
                               buffer);
}


/******************************   RETURN DONE   ******************************/

MODULE return_done (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    ASSERT (tcb-> script_handle-> waiting > 0);
    tcb-> script_handle-> waiting--;

    if (tcb-> error)
        lsend_ggcode_call_error (& tcb-> replyqid, NULL,
                                 NULL, NULL, NULL, 0,
                                 NULL, 0, NULL);
    else
    if (tcb-> thread_type == wait_event
    &&  list_empty (& tcb-> script_handle-> line_head))
        lsend_ggcode_call_error (& tcb-> replyqid, NULL,
                                 NULL, NULL, NULL, 0,
                                 NULL, 0, NULL);
    else
      {
        script_handle_link (tcb-> script_handle);
        tcb-> result-> value. i = tcb-> script_handle;

        tcb-> script_handle = NULL;     /*  Avoid being cleaned up  */

        mem_commit (tcb-> memtrn);
        tcb-> memtrn = NULL;
        free_excess_scripts (max_size);

        lsend_ggcode_call_ok (& tcb-> replyqid, NULL,
                              NULL, NULL, NULL, 0);
      }
}


/*****************************   SAVE THE NODE   *****************************/

MODULE save_the_node (THREAD *thread)
{
    struct_ggpars_ok
        *parse_result;

    tcb = thread-> tcb;                 /*  Point to thread's context        */

    /*  Get parse tree from parser agent message                             */
    get_ggpars_ok (thread-> event-> body, & parse_result);
    tcb-> script_node = (SCRIPT_NODE *) parse_result-> parse_root;
    mem_save (tcb-> memtrn,
              (MEMTRN *) parse_result-> parse_memtrn);

    tcb-> script_line = memt_alloc (tcb-> memtrn,
                                    sizeof (SCRIPT_LINE));
    ASSERT (tcb-> script_line);
    list_reset (tcb-> script_line);

    list_relink_before (  tcb-> script_line,
                        & tcb-> script_handle-> line_head);

    tcb-> script_line-> parent      = tcb-> script_handle;
    tcb-> script_line-> loop_start  = tcb-> loop_start;
    tcb-> script_line-> block_end   = NULL;
    tcb-> script_line-> links       = 0;
    tcb-> script_line-> size        = parse_result-> size;
    tcb-> script_line-> line        = tcb-> start_line;
    tcb-> script_line-> template    = tcb-> template;
    tcb-> script_line-> node        = tcb-> script_node;

    tcb-> script_line-> text = tcb-> line_text;
    tcb-> line_text = NULL;

    tcb-> script_handle-> size += tcb-> script_line-> size;
    total_size                 += tcb-> script_line-> size;

    free_ggpars_ok (& parse_result);
}


/************************   GENERATE NODE TYPE EVENT   ***********************/

MODULE generate_node_type_event (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    /*  If the node has an 'incoming' node, then we need to collect the      */
    /*  incoming script lines.  This should never clash with another event   */
    /*  ie the event would have been 'other'                                 */
    if (((tcb-> script_node-> type == GG_GSL)
    ||  (tcb-> script_node-> type == GG_DIRECT)
    ||  (tcb-> script_node-> type == GG_XML))
    &&  (tcb-> script_node-> name))
        the_next_event = incoming_event;
    else
      {
        ASSERT (tcb-> script_node-> type <  tblsize (type_event));
        the_next_event = type_event [tcb-> script_node-> type];
      }
}


/**********************   GENERATE CONTROL TYPE EVENT   **********************/

MODULE generate_control_type_event (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    ASSERT (tcb-> control. type <  tblsize (type_event));
    the_next_event = type_event [tcb-> control. type];
}


/*********************   CONFIRM CONTROL STACK IS EMPTY   ********************/

MODULE confirm_control_stack_is_empty (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    if (! list_empty (& tcb-> control_stack))
      {
        list_pop (& tcb-> control_stack, tcb-> control);
        raise_exception (unmatched_control_event);
      }
}


/**************************   CLEAN CONTROL STACK   **************************/

MODULE clean_control_stack (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    list_destroy (& tcb-> control_stack);
}


/***************************   SEND ERROR MESSAGE   **************************/

MODULE send_error_message (THREAD *thread)
{
    struct_ggpars_error
        *parse_error;

    tcb = thread-> tcb;                 /*  Point to thread's context        */

    /*  Get parse tree from parser agent message                             */
    get_ggpars_error (thread-> event-> body, & parse_error);
    log_error (thread-> tcb, "%s", parse_error-> error_text);
    free_ggpars_error (& parse_error);

    /*  Free line text  */
    mem_free (tcb-> line_text);
}


/****************************   SAVE LOOP START   ****************************/

MODULE save_loop_start (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    tcb-> loop_start = tcb-> script_line;
}


/****************************   PUSH THE CONTROL   ***************************/

MODULE push_the_control (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    push_control (tcb-> script_node-> type, tcb-> template, tcb-> script_line);
}

static void
push_control (SCRIPT_NODE_TYPE type, Bool template, SCRIPT_LINE *script_line)
{
    CONTROL
        control;

    control.type        = type;
    control.template    = template;
    control.script_line = script_line;
    list_push (& tcb-> control_stack, control);
}

/***************************   SET TEMPLATE MODE   ***************************/

MODULE set_template_mode (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    tcb-> template = TRUE;
}


/******************************   SET GSL MODE   *****************************/

MODULE set_gsl_mode (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    tcb-> template = FALSE;
}


/************************   SET TEMPLATE OR GSL MODE   ***********************/

MODULE set_template_or_gsl_mode (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    tcb-> template = tcb-> script_node-> op1
                     ? (Bool) atoi ((char *) tcb-> script_node-> op1-> op1) 
                     : TRUE;
}


/*************************   POP IF OR ELSE CONTROL   ************************/

MODULE pop_if_or_else_control (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    pop_control (thread, GG_IF, GG_ELSE);
}

static void
pop_control (THREAD *thread, SCRIPT_NODE_TYPE type1, SCRIPT_NODE_TYPE type2)
{
    SCRIPT_LINE
        *script_line;

    tcb = thread-> tcb;                 /*  Point to thread's context        */

    tcb-> control. type        = GG_UNDEFINED;
    tcb-> control. script_line = NULL;

    if (list_empty (& tcb-> control_stack))
      {
        raise_exception (unmatched_control_event);
        return;
      }

    list_pop (& tcb-> control_stack, tcb-> control);
    if (tcb-> control. type != type1
    &&  tcb-> control. type != type2)
        raise_exception (unmatched_control_event);
    else
      {
        tcb-> template = tcb-> control.template;

        /*  Fill in block_end field for intervening lines.  */
        script_line = tcb-> control. script_line;
        while (script_line != tcb-> script_line)
          {
            if (script_line-> block_end)
                script_line = script_line-> block_end;
            else
              {
                script_line-> block_end = tcb-> script_line;
                script_line = script_line-> next;
              }
          }
      }
}

/****************************   CONFIRM IF BLOCK   ***************************/

MODULE confirm_if_block (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    /*  What we need to be sure of is that this isn't .for/.else/.endif      */
    if (tcb-> loop_start
    &&  tcb-> loop_start-> block_end == tcb-> control. script_line)
        raise_exception (unmatched_control_event);
}


/*************************   POP IF OR FOR CONTROL   *************************/

MODULE pop_if_or_for_control (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    pop_control (thread, GG_IF, GG_FOR);
}


/************************   POP FOR OR ELSE CONTROL   ************************/

MODULE pop_for_or_else_control (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    pop_control (thread, GG_FOR, GG_ELSE);
}


/****************************   CONFIRM FOR LOOP   ***************************/

MODULE confirm_for_loop (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    if ((! tcb-> loop_start)
    ||  tcb-> loop_start-> block_end != tcb-> control. script_line)
        raise_exception (unmatched_control_event);
}


/*****************************   POP IF CONTROL   ****************************/

MODULE pop_if_control (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    pop_control (thread, GG_IF, GG_UNDEFINED);
}


/****************************   PUSH IF CONTROL   ****************************/

MODULE push_if_control (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    push_control (GG_IF, tcb-> template, tcb-> script_line);
}


/***************************   POP SCOPE CONTROL   ***************************/

MODULE pop_scope_control (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    pop_control (thread, GG_SCOPE, GG_UNDEFINED);
}


/************************   GET PREVIOUS LOOP START   ************************/

MODULE get_previous_loop_start (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    tcb-> loop_start = tcb-> loop_start-> loop_start;
}


/***************************   POP WHILE CONTROL   ***************************/

MODULE pop_while_control (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    pop_control (thread, GG_WHILE, GG_UNDEFINED);
}


/***************************   POP MACRO CONTROL   ***************************/

MODULE pop_macro_control (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    pop_control (thread, GG_MACRO, GG_UNDEFINED);
}


/**************************   POP FUNCTION CONTROL   *************************/

MODULE pop_function_control (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    pop_control (thread, GG_FUNCTION, GG_UNDEFINED);
}


/****************************   POP NEW CONTROL   ****************************/

MODULE pop_new_control (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    pop_control (thread, GG_NEW, GG_UNDEFINED);
}


/**********************   SEND UNMATCHED CONTROL ERROR   *********************/

MODULE send_unmatched_control_error (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    if (tcb-> control. script_line)
        log_error (thread-> tcb,
                   "Unmatched control at (%s %u).",
                   tcb-> control. script_line-> parent-> name,
                   tcb-> control. script_line-> line);
    else
        log_error (thread-> tcb,
                   "Unmatched control.");
}


/**************************   POP TEMPLATE CONTROL   *************************/

MODULE pop_template_control (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    pop_control (thread, GG_TEMPLATE, GG_UNDEFINED);
}


/****************************   READ SCRIPT LINE   ***************************/

MODULE read_script_line (THREAD *thread)
{
    static char
        line [LINE_MAX + 1];

    tcb = thread-> tcb;                 /*  Point to thread's context        */

    if (! tcb-> script_read ((JOBID) thread, line))
        the_next_event = end_of_file_event;
    else
    if (strncmp (line,
                 (char *) tcb-> script_node-> name-> op1,
                 strlen ((char *) tcb-> script_node-> name-> op1)) == 0)
      {
        tcb-> script_line-> text = memt_strdup (tcb-> memtrn, "");

        the_next_event = terminator_event;
      }
    else
      {
        tcb-> script_line-> text = memt_strdup (tcb-> memtrn, line);

        the_next_event = ok_event;
      }
}


/***************************   EXTEND SCRIPT LINE   **************************/

MODULE extend_script_line (THREAD *thread)
{
    static char
        line [LINE_MAX + 1];
    int
        len;

    tcb = thread-> tcb;                 /*  Point to thread's context        */

    if (! tcb-> script_read ((JOBID) thread, line))
        the_next_event = end_of_file_event;
    else
    if (strncmp (line,
                 (char *) tcb-> script_node-> name-> op1,
                 strlen ((char *) tcb-> script_node-> name-> op1)) == 0)
        the_next_event = terminator_event;
    else
      {
        len = strlen (tcb-> script_line-> text);
        tcb-> script_line-> text = mem_realloc (tcb-> script_line-> text,
                                                len + strlen (line) + 1);
        strcpy (tcb-> script_line-> text + len, line);

        the_next_event = ok_event;
      }
}


/**************************   UNEXPECTED EOF ERROR   *************************/

MODULE unexpected_eof_error (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    log_error (thread-> tcb,
               "Unexpected end of script.");
}


/*************************   SIGNAL INTERNAL ERROR   *************************/

MODULE signal_internal_error (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    coprintf ("Internal error.");
}


/**************************   SHUTDOWN EVERYTHING   **************************/

MODULE shutdown_everything (THREAD *thread)
{
    tcb = thread-> tcb;                 /*  Point to thread's context        */

    smt_shutdown ();
}

static int script_line_link (void *item)
{
    
    SCRIPT_LINE
        *script_line = item,
        *prev_line;

    if (script_line-> block_end)
      {
        /*  We want to go to the end of the block, but not the next block    */
        /*  if we were already at the end of the current block.              */
        prev_line = script_line-> prev;
        if (prev_line
        &&  script_line-> parent
        &&  prev_line == (SCRIPT_LINE *) & script_line-> parent-> line_head)
            prev_line = NULL;
        if ((! prev_line)
        ||  (script_line != prev_line-> block_end))
            script_line = script_line-> block_end;
      }
    script_line-> links++;

    return 0;
    
}

static int script_line_destroy (void *item)
{
    
    SCRIPT_LINE
        *script_line = item,
        *block_end,
        *prev_line,
        *next_line;

    if (script_line-> block_end)
      {
        /*  We want to go to the end of the block, but not the next block    */
        /*  if we were already at the end of the current block.              */
        prev_line = script_line-> prev;
        if (prev_line
        &&  script_line-> parent
        &&  prev_line == (SCRIPT_LINE *) & script_line-> parent-> line_head)
            prev_line = NULL;

        if (prev_line
        &&  prev_line-> block_end
        &&  prev_line-> block_end == script_line)
            block_end = script_line;
        else
            block_end = script_line-> block_end;
      }
    else
        block_end = script_line;

    block_end-> links--;
    ASSERT (block_end-> links >= 0);
    if (  block_end-> links == 0
    && (! block_end-> parent))
      {
        next_line = script_line-> next;
        destroy_one_script_line (script_line);
        script_line = next_line;
        while (script_line != block_end)
          {
            next_line = script_line-> next;
            if (script_line-> block_end
            &&  script_line-> block_end != block_end
            &&  script_line-> block_end-> links)
              {
                /*  Nullify link to block end if it's about to be destroyed. */
                if (script_line-> block_end-> block_end == block_end)
                    script_line-> block_end-> block_end = NULL;
                    
                next_line = script_line-> block_end-> next;
              }
            else
              {
                next_line = script_line-> next;
                destroy_one_script_line (script_line);
              }
            script_line = next_line;
          }
        destroy_one_script_line (script_line);
      }
    return 0;
    
}

static VALUE * script_line_get_attr (void *item, const char *name, Bool ignorecase)
{

    SCRIPT_LINE
        *script_line = item;
    static VALUE
        value;

    init_value (& value);
        
    if (matches (name, "path"))
      {

        assign_string (& value, script_line-> parent-> path);
        
      }
    else
    if (matches (name, "name"))
      {

        assign_string (& value, script_line-> parent-> name);
        
      }
    else
    if (matches (name, "line"))
      {

        assign_number (& value, script_line-> line);
        
      }
    else
    if (name == NULL || name [0] == 0)
      {

        assign_string (& value, script_line-> parent-> name);
        
      }

    return & value;
        
}

int shutdown_script_line_classes (void)
{

    SCRIPT_HANDLE
        *script_handle;

    script_handle = (SCRIPT_HANDLE *) script_list. next;
    while (script_handle != (SCRIPT_HANDLE *) & script_list)
      {
        script_handle = script_handle-> next;
        really_destroy_script_handle (script_handle-> prev);
      }
    return ggpars_term ();


    return 0;
}

int register_script_line_classes (void)
{
    int
        rc = 0;
    AGENT   *agent;                     /*  Handle for our agent             */
#include "ggscrp.i"                     /*  Include dialog interpreter       */


    /*  Shutdown event comes from Kernel                                     */
    declare_smtlib_shutdown   (shutdown_event, SMT_PRIORITY_MAX);

    /*  Reply messages from parser agent                                     */
    declare_ggpars_ok    (ok_event,          0);
    declare_ggpars_eof   (end_of_file_event, 0);
    declare_ggpars_error (error_event,       0);

    /*  Initialise list of parsed scripts                                    */
    list_reset (& script_list);
    total_size = 0;

    /*  Start parser                                                         */
    if (ggpars_init ())
        return (-1);

    return rc;
}
