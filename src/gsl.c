/*===========================================================================*
 *                                                                           *
 *  gsl.c - Main program source                                              *
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
#include "ggfile.h"                     /*  File functions                   */
#include "ggstrn.h"                     /*  String functions                 */
#include "ggenvt.h"                     /*  Environment functions            */
#include "ggconv.h"                     /*  Conversion functions             */
#include "ggmath.h"                     /*  Mathematics functions            */
#include "ggsock.h"                     /*  Socket functions                 */
#include "ggthrd.h"                     /*  Thread functions                 */
#include "ggxml.h"                      /*  Thread functions                 */
#include "ggtime.h"                     /*  Time functions                   */
#include "ggpcre.h"                     /*  Regular expression functions     */
#include "ggdiag.h"                     /*  Diagnostic functions             */
#include "ggproc.h"                     /*  Process functions                */
#include "version.h"                    /*  Version definitions              */


/*- Macros ------------------------------------------------------------------*/

#define PATH         "PATH"

#define DEFAULT_SIZE 1000000

#define ME              "gsl/4"
#define AGENT_NAME      "gsl"           /*  Our public name                  */
#define SINGLE_THREADED TRUE

/*- Function prototypes -----------------------------------------------------*/

static void define_standard_values        (void);
static void display_welcome_if_not_quiet  (void);
static void display_command_line_syntax   (void);
static void process_the_switch            (void);
static void read_xml_or_gsl_file          (void);
static void prepare_gsl_file              (char *filename);
static void prepare_xml_file              (char *filename);

/*- Global variables used in this source file only --------------------------*/

static int
    feedback,
    next_arg,
    gsl_argc;

static char
    **gsl_argv,
    *gsl_arg,
    *filename;

static SYMTAB
    *switches;

static XML_ITEM
    *xml_root,
    *xml_source;

static CLASS_ITEM
    root,
    source;

static Bool
    switch_argument = FALSE,
    switch_quiet    = FALSE,
    switch_parallel = FALSE;

#include "gsl.d"                        /*  Include dialog data              */


/********************************   M A I N   ********************************/

int
main (int _argc, char *_argv [])
{
    THREAD
        *thread;
    AGENT
        *agent;
    int
        script_arg = 0;

    smt_init ();                        /*  Initialise SMT kernel            */

#   include "gsl.i"                     /*  Include dialog interpreter       */

    gsl_argc = _argc;
    gsl_argv = _argv;

    feedback = 0;                       /*  No errors so far                 */

    if (gsl_argc <= 1)
      {
        display_welcome_if_not_quiet ();
        display_command_line_syntax ();
        return 0;
      }

    xml_root = xml_create ("root", NULL);
    root. item  = get_gsl_xml_item (xml_root);
    root. class = & XML_item_class;
    XML_item_class. link (root. item);

    xml_source = NULL;

    switches = sym_create_table ();

    /*  Start operator console  */
    smtoper_init ();
    thread_lookup (SMT_OPERATOR, "");

    if (gsl_init (DEFAULT_SIZE))        /*  Initialise GSL agent             */
      {
        coprintf ("%s E: Can't start GSL agent...", me);
        exit (1);
      }

    me      = ME;
    version = VERSION;

    /*  Shutdown event comes from Kernel                                     */
    declare_smtlib_shutdown   (shutdown_event, SMT_PRIORITY_MAX);

    /*  Declare GSL interpreter reply events  */
    declare_ggcode_ok      (ok_event,    0);
    declare_ggcode_message (message_event, 0);
    declare_ggcode_error   (error_event, 0);
    declare_ggcode_fatal   (fatal_event, 0);

    /*  Create controlling thread  */
    thread = thread_create (AGENT_NAME, "");

    if (
        register_script_line_classes ()
    ||  register_gsl_classes ()
    ||  register_file_classes ()
    ||  register_string_classes ()
    ||  register_env_classes ()
    ||  register_conv_classes ()
    ||  register_math_classes ()
    ||  register_sock_classes ()
    ||  register_thread_classes ()
    ||  register_XML_classes ()
    ||  register_time_classes ()
    ||  register_regexp_classes ()
    ||  register_proc_classes ()
    ||  register_diag_classes ()
     )
      {
        coprintf ("%s E: Error registering functions.", me);
        exit (1);
      }

    define_standard_values ();

    next_arg = 1;
    while (next_arg < gsl_argc)
      {
        gsl_arg = gsl_argv [next_arg++];
        if (gsl_arg [0] == '-')
            process_the_switch ();
        else
          {
            display_welcome_if_not_quiet ();
            filename = gsl_arg;

            if (switch_argument)
                while (next_arg < gsl_argc)
                    sym_assume_symbol (switches,
                                       strprintf ("arg%u", ++script_arg),
                                       gsl_argv [next_arg++]);

            read_xml_or_gsl_file ();
            if (xml_source)
              {
                gsl_execute (thread-> queue, 0, switches,
                             2, & root, & source);
                XML_item_class. destroy (source. item);
                xml_source = NULL;
              }
            else
                gsl_execute (thread-> queue, 0, switches,
                             1, & root);

                                         
            if (!switch_parallel)
              {
                smt_exec_full ();
                if (xml_source)
                  {
                    XML_item_class. destroy (source. item);
                    xml_source = NULL;
                  }
                XML_item_class. destroy (root.   item);
                xml_free (xml_root);

                xml_root = xml_create ("root", NULL);
                root. item  = get_gsl_xml_item (xml_root);
                root. class = & XML_item_class;
                XML_item_class. link (root. item);
              }
          }
      }
    if (switch_parallel)
        smt_exec_full ();

    shutdown_XML_classes ();

    gsl_term ();
    smt_term ();                        /*  Shut-down SMT kernel             */

    shutdown_script_line_classes ();
    sym_delete_table (switches);
    if (xml_source)
      {
        XML_item_class. destroy (source. item);
        xml_source = NULL;
      }
    XML_item_class. destroy (root. item);
    xml_free (xml_root);

    mem_assert ();
    return (feedback);
}


static void display_welcome_if_not_quiet (void)
{
    if (!switch_quiet)
        coprintf ("%s %s", PRODUCT, COPYRIGHT);
}


static void display_command_line_syntax (void)
{
    printf ("syntax: gslgen -<option> ... -<attr>[:<value>] ... <filename> ...\n");
    printf ("    or: gslgen -a -<option> ... -<attr>[:<value>] <filename> <arg> ...\n");
    printf ("    Options:\n");
    printf ("        -a   argument: Pass arguments following filename to GSL script\n");
    printf ("        -q   quiet:    suppress routine messages\n");
    printf ("        -p   parallel: process files in parallel\n");
    printf ("        -s:n size:n    set script cache size - default is %lu\n", (long) DEFAULT_SIZE);
    printf ("        -h   help:     show command-line summary\n");
    printf ("        -v   version:  show full version information\n");
}


static void define_standard_values (void)
{
    sym_assume_symbol (switches, "shuffle",     "2");
    sym_assume_symbol (switches, "ignorecase",  "1");
    sym_assume_symbol (switches, "terminator",  "\n");
}


static void process_the_switch (void)
{
    char
        *name,
        *value;
    VALUE
        val;

    name  = strtok (gsl_arg, ":") + 1;
    value = strtok (NULL, "");

    if ((lexcmp (name, "a") == 0 || lexcmp (name, "argument") == 0) && !value)
        switch_argument = TRUE;
    else
    if ((lexcmp (name, "q") == 0 || lexcmp (name, "quiet") == 0) && !value)
        switch_quiet = TRUE;
    else
    if ((lexcmp (name, "p") == 0 || lexcmp (name, "parallel") == 0) && !value)
        switch_parallel = TRUE;
    else
    if ((lexcmp (name, "v") == 0 || lexcmp (name, "version") == 0) && !value)
      {
        printf ("%s\n", PRODUCT);
        printf ("%s\n", BUILDMODEL);
        printf ("%s\n", COPYRIGHT);
#if defined (CCOPTS)
        printf ("Compiler: " CCOPTS "\n");
#endif
        exit (0);
      }
    else
    if ((lexcmp (name, "h") == 0 || lexcmp (name, "help") == 0) && !value)
      {
        display_command_line_syntax ();
        exit (0);
      }
    else
    if ((lexcmp (name, "s") == 0 || lexcmp (name, "size") == 0) && value)
      {
        init_value (& val);
        max_size =  atol (value);
      }
    else
      {
        if (value)
            sym_assume_symbol (switches, name, value);
        else
            sym_assume_symbol (switches, name, "");
      }
}


static void read_xml_or_gsl_file (void)
{
    char
        *ch,
        *fname;
    int
        rc;

    if (!switch_quiet)
        coprintf ("%s I: Processing %s...", me, filename);

    ch = strrchr (filename, '.');
    if (ch)
      {
        if (streq (ch + 1, "gsl"))
          {
            prepare_gsl_file (filename);
            return;
          }
        else
        if (streq (ch + 1, "xml"))
          {
            prepare_xml_file (filename);
            return;
          }
      }
    rc = xml_seems_to_be (PATH, filename);
    if (rc == XML_NOERROR)
      {
        prepare_xml_file (filename);
        return;
      }
    if (rc == XML_LOADERROR)
      {
        prepare_gsl_file (filename);
        return;
      }

    fname = file_where ('r', PATH, filename, "gsl");
    if (fname)
      {
        prepare_gsl_file (filename);
        return;
      }
    else
      {
        coprintf ("%s E: Error processing %s...", me, filename);
        coprintf ("File not found");
        exit (1);
      }
}


void
prepare_gsl_file (char *filename)
{
    xml_source = NULL;
    sym_assume_symbol (switches, "script",   filename);
    if (! sym_lookup_symbol (switches, "template"))
        sym_assume_symbol (switches, "template", "0");
}


void
prepare_xml_file (char *filename)
{
    int
        rc;
    XML_ITEM
        *xml_temp = xml_new (NULL, "temp", NULL);
    VALUE
        value;

    ASSERT (xml_temp);

    init_value (& value);
    rc = xml_load_file (& xml_temp, PATH, filename, FALSE);
    if (rc == XML_NOERROR)
      {
        xml_source = xml_first_child (xml_temp);

        if (xml_source)
          {
            xml_attach_child (xml_root, xml_source);
            source. item  = get_gsl_xml_item (xml_source);
            source. class = & XML_item_class;
            XML_item_class. link (source. item);

            sym_assume_symbol (switches, "filename", filename);
            if (! sym_lookup_symbol (switches, "template"))
                sym_assume_symbol (switches, "template", "1");
          }

        xml_free (xml_temp);
      }
    else
      {
        coprintf ("%s E: Error processing %s...", me, filename);
        coprintf ("%s", xml_error ());
        exit (1);
      }
}


/*************************   INITIALISE THE THREAD   *************************/

MODULE initialise_the_thread (THREAD *thread)
{
    the_next_event = ok_event;
}


/*************************   DISPLAY ERROR MESSAGE   *************************/

MODULE display_error_message (THREAD *thread)
{
    struct_ggcode_error_reply
        *error_reply;
    char
        *ptr;

    feedback = 1;

    get_ggcode_error_reply (thread-> event-> body, & error_reply);

    if (error_reply-> error_text)
      {
        ptr = strtok (error_reply-> error_text, "\n");
        while (ptr)
          {
            if (        error_reply-> error_name
            &&  strlen (error_reply-> error_name))
                coprintf ("(%s %u) %s", strip_file_path (error_reply-> error_name),
                                        error_reply-> error_line,
                                        ptr);
            else
                coprintf ("%s", ptr);
    
            ptr = strtok (NULL, "\n");
          }
      }

    free_ggcode_error_reply (& error_reply);
}


/**************************   TERMINATE THE THREAD   *************************/

MODULE terminate_the_thread (THREAD *thread)
{
    the_next_event = terminate_event;
}
