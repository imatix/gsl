/*---------------------------------------------------------------------------
 *  ggfile.c - GSL/fileio package
 *
 *  Generated from ggfile by ggobjt.gsl using GSL/4.
 *  DO NOT MODIFY THIS FILE.
 *
 *  For documentation and updates see http://www.imatix.com.
 *---------------------------------------------------------------------------*/

#include "sfl.h"
#include "smt3.h"
#include "gsl.h"                        /*  Project header file              */
#include "ggfile.h"                     /*  Include header file              */

/*- Macros ------------------------------------------------------------------*/

#define DIRECTORY_NAME "directory"      /*  Directory                        */
#define DIRECTORY_ENTRY_NAME "directory entry"  /*  Directory                */
#define FILE_NAME "file"                /*  File                             */
#define FILE_ENTRY_NAME "file entry"    /*  File                             */

#define matches(s,t)    (s ? (ignorecase ? lexcmp (s, t) == 0 : streq (s, t))   : t == NULL)

/*- Function prototypes -----------------------------------------------------*/

static int
directory_open (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread);
static int
directory_setcwd (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread);
static int
directory_create (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread);
static int
directory_delete (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread);
static int
directory_resolve (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread);
static int
directory_entry_name (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread);
static int
directory_entry_first (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread);
static int
directory_entry_next (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread);
static int
directory_entry_parent_function (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread);
static int
directory_entry_new (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread);
static int
fileopen (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread);
static int
fileread (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread);
static int
filewrite (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread);
static int
fileclose (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread);
static int
file_tell (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread);
static int
file_seek (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread);
static int
fileslurp (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread);
static int
fileexists (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread);
static int
file_timestamp (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread);
static int
filerename (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread);
static int
filedelete (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread);
static int
filelocate (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread);
static int
filecopy (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread);
static int
file_basename (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread);
static int
file_entry_open (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread);
static int
file_entry_read (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread);
static int
file_entry_write (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread);
static int
file_entry_close (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread);
static int
file_entry_tell (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread);
static int
file_entry_seek (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread);
static int
file_entry_name (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread);
static int
file_entry_next (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread);
static int
file_entry_parent_function (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread);
static int directory_link (void *item);
static int directory_destroy (void *item);
static VALUE * directory_get_attr (void *item, const char *name, Bool ignorecase);
static int directory_entry_link (void *item);
static int directory_entry_destroy (void *item);
static const char * directory_entry_item_name (void *item);
static VALUE * directory_entry_get_attr (void *item, const char *name, Bool ignorecase);
static int directory_entry_put_attr (void *item, const char *name, VALUE *value, Bool ignorecase);
static int directory_entry_first_child (void *olditem, const char *name, Bool ignorecase, CLASS_DESCRIPTOR **class, void **item);
static int directory_entry_next_sibling (void *olditem, const char *name, Bool ignorecase, CLASS_DESCRIPTOR **class, void **item);
static int directory_entry_parent (void *olditem, CLASS_DESCRIPTOR **class, void **item, THREAD *gsl_thread);
static int directory_entry_create (const char *name, void *parent, void *sibling, CLASS_DESCRIPTOR **class, void **item);
static void * directory_entry_copy (void *item, CLASS_DESCRIPTOR *to_class, const char *name, void *parent, void *sibling);
static int file_link (void *item);
static int file_destroy (void *item);
static VALUE * file_get_attr (void *item, const char *name, Bool ignorecase);
static int file_entry_link (void *item);
static int file_entry_destroy (void *item);
static const char * file_entry_item_name (void *item);
static VALUE * file_entry_get_attr (void *item, const char *name, Bool ignorecase);
static int file_entry_next_sibling (void *olditem, const char *name, Bool ignorecase, CLASS_DESCRIPTOR **class, void **item);
static int file_entry_parent (void *olditem, CLASS_DESCRIPTOR **class, void **item, THREAD *gsl_thread);

/*- Global variables --------------------------------------------------------*/
static PARM_LIST parm_list_vr           = { PARM_VALUE,
                                            PARM_REFERENCE };
static PARM_LIST parm_list_vvr          = { PARM_VALUE,
                                            PARM_VALUE,
                                            PARM_REFERENCE };
static PARM_LIST parm_list_vvvr         = { PARM_VALUE,
                                            PARM_VALUE,
                                            PARM_VALUE,
                                            PARM_REFERENCE };
static PARM_LIST parm_list_r            = { PARM_REFERENCE };

static GSL_FUNCTION directory_functions [] =
{
    {"create",         1, 1, 1, (void *) &parm_list_vr, 1, directory_create},
    {"delete",         1, 2, 2, (void *) &parm_list_vr, 1, directory_delete},
    {"open",           0, 2, 2, (void *) &parm_list_vr, 1, directory_open},
    {"resolve",        1, 2, 1, (void *) &parm_list_vr, 1, directory_resolve},
    {"setcwd",         1, 2, 2, (void *) &parm_list_vr, 1, directory_setcwd}};

CLASS_DESCRIPTOR
    directory_class = {
        "directory",
        NULL,
        directory_get_attr,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        directory_destroy,
        directory_link,
        NULL,
        NULL,
        NULL,
        NULL,
        directory_functions, tblsize (directory_functions) };

static GSL_FUNCTION directory_entry_functions [] =
{
    {"first",          0, 1, 1, (void *) &parm_list_vr, 1, directory_entry_first},
    {"name",           0, 0, 0, NULL,            1, directory_entry_name},
    {"new",            0, 1, 1, (void *) &parm_list_vr, 1, directory_entry_new},
    {"next",           0, 1, 1, (void *) &parm_list_vr, 1, directory_entry_next},
    {"parent",         0, 0, 0, NULL,            1, directory_entry_parent_function}};

CLASS_DESCRIPTOR
    directory_entry_class = {
        "directory entry",
        directory_entry_item_name,
        directory_entry_get_attr,
        directory_entry_put_attr,
        directory_entry_first_child,
        directory_entry_next_sibling,
        directory_entry_parent,
        directory_entry_create,
        directory_entry_destroy,
        directory_entry_link,
        NULL,
        directory_entry_copy,
        NULL,
        NULL,
        directory_entry_functions, tblsize (directory_entry_functions) };

static GSL_FUNCTION file_functions [] =
{
    {"basename",       1, 1, 1, (void *) &parm_list_vvvr, 1, file_basename},
    {"close",          1, 2, 2, (void *) &parm_list_vr, 1, fileclose},
    {"copy",           2, 4, 4, (void *) &parm_list_vvvr, 1, filecopy},
    {"delete",         1, 2, 2, (void *) &parm_list_vr, 1, filedelete},
    {"exists",         1, 2, 2, (void *) &parm_list_vr, 1, fileexists},
    {"locate",         1, 3, 3, (void *) &parm_list_vvr, 1, filelocate},
    {"open",           1, 3, 3, (void *) &parm_list_vvr, 1, fileopen},
    {"read",           1, 2, 2, (void *) &parm_list_vr, 1, fileread},
    {"rename",         2, 3, 3, (void *) &parm_list_vvr, 1, filerename},
    {"seek",           1, 3, 3, (void *) &parm_list_vvr, 1, file_seek},
    {"slurp",          1, 2, 2, (void *) &parm_list_vr, 1, fileslurp},
    {"tell",           1, 2, 2, (void *) &parm_list_vr, 1, file_tell},
    {"timestamp",      1, 2, 2, (void *) &parm_list_vr, 1, file_timestamp},
    {"write",          2, 3, 3, (void *) &parm_list_vvr, 1, filewrite}};

CLASS_DESCRIPTOR
    file_class = {
        "file",
        NULL,
        file_get_attr,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        file_destroy,
        file_link,
        NULL,
        NULL,
        NULL,
        NULL,
        file_functions, tblsize (file_functions) };

static GSL_FUNCTION file_entry_functions [] =
{
    {"close",          0, 1, 1, (void *) &parm_list_r, 1, file_entry_close},
    {"name",           0, 0, 0, NULL,            1, file_entry_name},
    {"next",           0, 1, 1, (void *) &parm_list_vvvr, 1, file_entry_next},
    {"open",           0, 2, 2, (void *) &parm_list_vr, 1, file_entry_open},
    {"parent",         0, 0, 0, NULL,            1, file_entry_parent_function},
    {"read",           0, 1, 1, (void *) &parm_list_r, 1, file_entry_read},
    {"seek",           0, 2, 2, (void *) &parm_list_vr, 1, file_entry_seek},
    {"tell",           0, 1, 1, (void *) &parm_list_r, 1, file_entry_tell},
    {"write",          1, 2, 2, (void *) &parm_list_vr, 1, file_entry_write}};

CLASS_DESCRIPTOR
    file_entry_class = {
        "file entry",
        file_entry_item_name,
        file_entry_get_attr,
        NULL,
        NULL,
        file_entry_next_sibling,
        file_entry_parent,
        NULL,
        file_entry_destroy,
        file_entry_link,
        NULL,
        NULL,
        NULL,
        NULL,
        file_entry_functions, tblsize (file_entry_functions) };


typedef struct {
    int
        links;
    char
        *error_msg;
} FILE_CONTEXT;

typedef struct _DIRECTORY_ENTRY_ITEM {
    int
        links;
    DIRST
        *dirst;
    struct _DIRECTORY_ENTRY_ITEM
        *parent;
    Bool
        exists;
    char
        *path,
        *name;
} DIRECTORY_ENTRY_ITEM;

typedef struct {
    int
        links;
    DIRST
        *dirst;
    struct _DIRECTORY_ENTRY_ITEM
        *parent;
    char
        *path,
        *name;
    off_t
        size;
    time_t
        timestamp;
    FILE
        *handle;
    char
        *error_msg;
} FILE_ENTRY_ITEM;

/*  last_context is used so that the directory and file classes can share a  */
/*  FILE_CONTEXT block to hold the last error message.  It's not elegant but */
/*  We use assertions to be sure that it works as it should.                 */

static FILE_CONTEXT
    *last_context = NULL;

static char
    line_buffer [LINE_MAX + 1];

#define FILE_NOT_OPEN_MESSAGE "File not open"

static void
get_directory_entry (DIRECTORY_ENTRY_ITEM *parent,
                     DIRST *dirst,
                     const char *name,
                     Bool ignorecase,
                     CLASS_DESCRIPTOR **class,
                     void **item)
{
    Bool
        getdir,
        getfile;
    DIRECTORY_ENTRY_ITEM
        *directory;
    FILE_ENTRY_ITEM
        *file;
    int
        rc = TRUE;

    if (streq (name, ""))
      {
        getdir  = TRUE;
        getfile = TRUE;
      }
    else
      {
        getfile = matches (name, "file");
        getdir  = matches (name, "directory");
      }

    if (! (getfile || getdir))
        rc = FALSE;

    while (rc)
      {
        if ((dirst-> file_attrs & ATTR_SUBDIR) != 0 ? getdir : getfile)
            break;
        rc = read_dir (dirst);
      }

    if (rc)
      {
        if ((dirst-> file_attrs & ATTR_SUBDIR) != 0)
          {
            directory = memt_alloc (NULL, sizeof (DIRECTORY_ENTRY_ITEM));
            directory-> path      = memt_alloc (NULL, strlen (dirst-> dir_name) + 2);
            directory-> name      = memt_strdup (NULL, dirst-> file_name);
            directory-> dirst     = dirst;
            directory-> links     = 0;
            directory-> parent    = parent;
            directory-> exists    = TRUE;

            xstrcpy (directory-> path,
                     dirst-> dir_name, "/", NULL);

            *class = & directory_entry_class;
            *item  =   directory;
          }
        else
          {
            file = memt_alloc (NULL, sizeof (FILE_ENTRY_ITEM));
            file-> path      = memt_alloc (NULL, strlen (dirst-> dir_name) + 2);
            file-> name      = memt_strdup (NULL, dirst-> file_name);
            file-> size      = dirst-> file_size;
            file-> timestamp = dirst-> file_time;
            file-> dirst     = dirst;
            file-> links     = 0;
            file-> parent    = parent;
            file-> handle    = NULL;
            file-> error_msg = NULL;

            xstrcpy (file-> path,
                     dirst-> dir_name, "/", NULL);

            *class = & file_entry_class;
            *item  =   file;
          }
      }
    else
      {
        close_dir (dirst);
        mem_free (dirst);
      }
}

static int
store_module_error (THREAD       *gsl_thread,
                    FILE_CONTEXT *context,
                    RESULT_NODE  *error,
                    const char   *error_msg)
{
    GGCODE_TCB
        *gsl_tcb = gsl_thread-> tcb;
    VALUE
        value;
    char
        *error_text;

    if (error_msg)
      {
        if (! context)
            context = get_class_item (gsl_thread, FILE_NAME);
        mem_free (context-> error_msg);
        context-> error_msg = memt_strdup (NULL, error_msg);

        if (error)
          {
            init_value (& value);
            assign_string (& value, context-> error_msg);
            if (! store_symbol_definition (& gsl_tcb-> scope_stack,
                                           gsl_tcb-> gsl-> ignorecase,
                                           error,
                                           &value,
                                           &error_text))
              {
                strncpy (object_error, error_text, LINE_MAX);
                return -1;
              }
          }
        }
    return 0;
}

static int
store_file_error (FILE_ENTRY_ITEM *file,
                  THREAD          *gsl_thread,
                  RESULT_NODE     *error,
                  const char      *error_msg)
{
    if (error_msg)
      {
        mem_free (file-> error_msg);
        file-> error_msg = memt_strdup (NULL, error_msg);
      }
    return store_module_error (gsl_thread, NULL, error, error_msg);
}

static void
create_file_entry (const char *filename,
                   FILE_CONTEXT *context,
                   RESULT_NODE *result,
                   RESULT_NODE *error,
                   THREAD *gsl_thread)
{
    char
        *curpath,
        *fullname,
        *lastchar;
    FILE_ENTRY_ITEM
        *file;

    curpath = get_curdir ();
    fullname = locate_path (curpath, filename);
    mem_free (curpath);
    lastchar = fullname + strlen (fullname) - 1;
    if (*lastchar == '/')
        *lastchar = 0;
    curpath = strip_file_name (fullname);

    file = memt_alloc (NULL, sizeof (FILE_ENTRY_ITEM));
    file-> links     = 0;
    file-> dirst     = NULL;
    file-> parent    = NULL;
    file-> path      = memt_alloc (NULL, strlen (curpath) + 2);
    xstrcpy (file-> path, curpath, "/", NULL);
    file-> name      = memt_strdup (NULL, strip_file_path (fullname));
    file-> size      = 0;
    file-> timestamp = 0;
    file-> handle    = NULL;
    file-> error_msg = NULL;

    if (file_exists (fullname))
      {
        file-> size      = get_file_size (fullname);
        file-> timestamp = get_file_time (fullname);
      }

    assign_pointer (& result-> value, & file_entry_class, file);

    mem_free (fullname);
}

static int
open_the_file (FILE_ENTRY_ITEM *file, char mode,
               RESULT_NODE *error,
               THREAD *gsl_thread)
{
    char
        *filename;

    filename = memt_alloc (NULL,
                           strlen (file-> path) + strlen (file-> name) + 2);
    xstrcpy (filename,
             file-> path, "/", file-> name, NULL);

    errno = 0;
    file-> handle = file_open (filename, mode);
    mem_free (filename);

    return store_file_error (file, gsl_thread, error,
                             errno ? strerror (errno) : NULL);
}

static int
read_the_file (FILE_ENTRY_ITEM *file,
               RESULT_NODE *result,
               RESULT_NODE *error,
               THREAD *gsl_thread)
{
    int
        rc;

    if (! file-> handle)
        return store_file_error (file, gsl_thread, error,
                                 FILE_NOT_OPEN_MESSAGE);

    errno = 0;
    rc = gsl_file_read (file-> handle, line_buffer);
    if (rc)
        assign_string (& result-> value, memt_strdup (NULL, line_buffer));

    return store_file_error (file, gsl_thread, error,
                             rc ? NULL : "End of file");
}

static int
write_the_file (FILE_ENTRY_ITEM *file,
                const char  *buffer,
                RESULT_NODE *result,
                RESULT_NODE *error,
                THREAD *gsl_thread)
{
    char
        *rc;

    if (! file-> handle)
        return store_file_error (file, gsl_thread, error,
                                 FILE_NOT_OPEN_MESSAGE);

    errno = 0;
    rc = file_write (file-> handle, buffer);
    if (rc)
        assign_number (& result-> value, 0);
    else
        assign_number (& result-> value, -1);

    return store_file_error (file, gsl_thread, error,
                             errno ? strerror (errno) : NULL);
}

static int
close_the_file (FILE_ENTRY_ITEM *file,
                RESULT_NODE *result,
                RESULT_NODE *error,
                THREAD *gsl_thread)
{
    errno = 0;

    if (! file-> handle)
        return store_file_error (file, gsl_thread, error,
                                 FILE_NOT_OPEN_MESSAGE);

    if (file-> handle)
      {
        assign_number (& result-> value, file_close (file-> handle));
        file-> handle = NULL;
      }
    return store_file_error (file, gsl_thread, error,
                             errno ? strerror (errno) : NULL);
}

static int
tell_the_file (FILE_ENTRY_ITEM *file,
               RESULT_NODE *result,
               RESULT_NODE *error,
               THREAD *gsl_thread)
{
    errno = 0;

    if (! file-> handle)
        return store_file_error (file, gsl_thread, error,
                                 FILE_NOT_OPEN_MESSAGE);

    if (file-> handle)
        assign_number (& result-> value, ftell (file-> handle));

    return store_file_error (file, gsl_thread, error,
                             errno ? strerror (errno) : NULL);
}

static int
seek_the_file (FILE_ENTRY_ITEM *file,
               qbyte offset,
               RESULT_NODE *result,
               RESULT_NODE *error,
               THREAD *gsl_thread)
{
    errno = 0;

    if (! file-> handle)
        return store_file_error (file, gsl_thread, error,
                                 FILE_NOT_OPEN_MESSAGE);

    if (file-> handle)
        assign_number (& result-> value,
                       (offset == -1)
                            ? fseek (file-> handle, 0,      SEEK_END)
                            : fseek (file-> handle, offset, SEEK_SET));

    return store_file_error (file, gsl_thread, error,
                             errno ? strerror (errno) : NULL);
}

static int directory_link (void *item)
{
    
    ((FILE_CONTEXT *) item)-> links++;
    return 0;
    
}

static int directory_destroy (void *item)
{
    
  {
    FILE_CONTEXT
        *context = item;

    if (--context-> links == 0)
      {
        mem_free (context-> error_msg);
        mem_free (context);
      }
    return 0;
  }
    
}

static VALUE * directory_get_attr (void *item, const char *name, Bool ignorecase)
{

    static VALUE
        value;
    char
        *ptr;

    init_value (& value);
        
    if (matches (name, "cwd"))
      {

        ptr = get_curdir ();
        strncpy (line_buffer, ptr, LINE_MAX);
        line_buffer [LINE_MAX] = 0;
        mem_free (ptr);
        assign_string (& value, line_buffer);
        
      }

    return & value;
        
}

static int directory_entry_link (void *item)
{
    
    if (item)
        ((DIRECTORY_ENTRY_ITEM *) item)-> links++;
    return 0;
    
}

static int directory_entry_destroy (void *item)
{
    
    DIRECTORY_ENTRY_ITEM
        *directory = item;

    if (directory
    &&  --directory-> links <= 0)
      {
        mem_free (directory-> path);
        mem_free (directory-> name);
        if (directory-> dirst)
          {
            close_dir (directory-> dirst);
            mem_free  (directory-> dirst);
          }
        mem_free (directory);
      }
    return 0;
    
}

static const char * directory_entry_item_name (void *item)
{
    
    return item ? "directory" : NULL;
    
}

static VALUE * directory_entry_get_attr (void *item, const char *name, Bool ignorecase)
{

    DIRECTORY_ENTRY_ITEM
        *directory = item;
    static VALUE
        value;

    ASSERT (directory);

    init_value (& value);
        
    if (matches (name, "path"))
      {

        assign_string (& value, directory-> path);
        
      }
    else
    if (matches (name, "name"))
      {

        assign_string (& value, directory-> name);
        
      }

    return & value;
        
}

static int directory_entry_put_attr (void *item, const char *name, VALUE *value, Bool ignorecase)
{

    DIRECTORY_ENTRY_ITEM
        *directory = item;
    char
        *oldfullname,
        *newfullname;
    int
        rc = 0;

    ASSERT (directory);

    if ((! name)
    ||  (! value)
    || value-> type == TYPE_POINTER)
        return -1;

    string_value (value);
        
    if (matches (name, "name"))
      {

        if ((! directory-> exists)
        ||  (! streq (directory-> name, value-> s)))
          {
            newfullname = memt_alloc (NULL,
                                      strlen (directory-> path)
                                    + strlen (value-> s) + 1);
            xstrcpy (newfullname, directory-> path, value-> s, NULL);
            if (directory-> exists)
              {
                oldfullname = memt_alloc (NULL,
                                          strlen (directory-> path)
                                        + strlen (directory-> name) + 1);
                xstrcpy (oldfullname, directory-> path,
                                      directory-> name, NULL);
                rc = file_rename (oldfullname, newfullname);
                mem_free (oldfullname);
              }
            else
              {
                if (file_exists (newfullname))
                    rc = -1;
                else
                    rc = make_dir (newfullname);
              }
            mem_free (newfullname);

            if (! rc)
              {
                directory-> exists = TRUE;
                mem_free (directory-> name);
                directory-> name = memt_strdup (NULL, value-> s);
              }
          }
        
      }

    else
        rc = -1;

    return rc;
        
}

static int directory_entry_first_child (void *olditem, const char *name, Bool ignorecase, CLASS_DESCRIPTOR **class, void **item)
{
    
    DIRECTORY_ENTRY_ITEM
        *directory = olditem;
    DIRST
        *dirst;

    ASSERT (directory);

    dirst = directory-> dirst;
    directory-> dirst = NULL;
    get_directory_entry (directory, dirst, name, ignorecase, class, item);
    return 0;
    
}

static int directory_entry_next_sibling (void *olditem, const char *name, Bool ignorecase, CLASS_DESCRIPTOR **class, void **item)
{
    
    DIRECTORY_ENTRY_ITEM
        *directory = olditem;
    DIRST
        *dirst;
    int
        rc;

    ASSERT (directory);

    dirst = directory-> dirst;
    directory-> dirst = NULL;
    rc = read_dir (dirst);

    if (rc)
      {
        get_directory_entry (directory-> parent, dirst, name, ignorecase, class, item);
        return 0;
      }
    else
      {
        close_dir (dirst);
        mem_free (dirst);
        return -1;
      }
    
}

static int directory_entry_parent (void *olditem, CLASS_DESCRIPTOR **class, void **item, THREAD *gsl_thread)
{
    
    DIRECTORY_ENTRY_ITEM
        *directory = olditem;

    ASSERT (directory);

    *item = directory-> parent;
    if (*item)
        *class = & directory_entry_class;

    return 0;
    
}

static int directory_entry_create (const char *name, void *parent, void *sibling, CLASS_DESCRIPTOR **class, void **item)
{
    
    DIRECTORY_ENTRY_ITEM
        *directory;

    if (! streq (name, "directory"))
        return -1;

    if (sibling)                            /*  Can't specify sibling.  */
        return -1;

    directory = memt_alloc (NULL, sizeof (DIRECTORY_ENTRY_ITEM));
    directory-> parent    = parent;
    directory-> path = memt_alloc (NULL,
                                   strlen (directory-> parent-> path)
                                 + strlen (directory-> parent-> name) + 2);
    xstrcpy (directory-> path, directory-> parent-> path,
                               directory-> parent-> name, "/", NULL);
    directory-> name      = NULL;
    directory-> dirst     = NULL;
    directory-> links     = 0;
    directory-> exists    = FALSE;

    *class = & directory_entry_class;
    *item  =   directory;

    return 0;
    
}

static void * directory_entry_copy (void *item, CLASS_DESCRIPTOR *to_class, const char *name, void *parent, void *sibling)
{
    
    int
        rc = -1;
    CLASS_DESCRIPTOR
        *new_class;
    void
        *new_item = NULL;
    VALUE
        value;

    init_value (& value);
    value. type = TYPE_STRING;

    if (to_class-> create)
        rc = to_class-> create (name ? name : "directory",
                                parent, sibling,
                                &new_class, &new_item);

    if ((! rc)
    &&  new_item
    &&  new_class-> put_attr)
      {
        value. s = ((DIRECTORY_ENTRY_ITEM *) item)-> name;
        rc = new_class-> put_attr (new_item,
                                   "name", & value,
                                   FALSE);
        if (rc)
          {
            if (new_class-> destroy)
                new_class-> destroy (new_item);
            new_item = NULL;
          }
      }
    return new_item;
    
}

static int file_link (void *item)
{
    
    ((FILE_CONTEXT *) item)-> links++;
    return 0;
    
}

static int file_destroy (void *item)
{
    
  {
    FILE_CONTEXT
        *context = item;

    if (--context-> links == 0)
      {
        mem_free (context-> error_msg);
        mem_free (context);
      }
    return 0;
  }
    
}

static VALUE * file_get_attr (void *item, const char *name, Bool ignorecase)
{

    FILE_CONTEXT
        *context = item;
    static VALUE
        value;

    init_value (& value);
        
    if (matches (name, "error"))
      {

        if (context-> error_msg)
            assign_string (& value, context-> error_msg);
        
      }

    return & value;
        
}

static int file_entry_link (void *item)
{
    
    if (item)
        ((FILE_ENTRY_ITEM *) item)-> links++;
    return 0;
    
}

static int file_entry_destroy (void *item)
{
    
    FILE_ENTRY_ITEM
        *file = item;

    if (file
    &&  --file-> links <= 0)
      {
        mem_free (file-> path);
        mem_free (file-> name);
        if (file-> dirst)
          {
            close_dir (file-> dirst);
            mem_free  (file-> dirst);
          }
        if (file-> handle)
            file_close (file-> handle);
        mem_free (file-> error_msg);
        mem_free (file);
      }
    return 0;
    
}

static const char * file_entry_item_name (void *item)
{
    
    return item ? "file" : NULL;
    
}

static VALUE * file_entry_get_attr (void *item, const char *name, Bool ignorecase)
{

    FILE_ENTRY_ITEM
        *file = item;
    static VALUE
        value;

    ASSERT (file);

    if (! name)
        return NULL;

    init_value (& value);
        
    if (matches (name, "path"))
      {

        assign_string (& value, file-> path);
        
      }
    else
    if (matches (name, "name"))
      {

        assign_string (& value, file-> name);
        
      }
    else
    if (matches (name, "size"))
      {

        assign_string (& value, strprintf ("%lu", file-> size));
        
      }
    else
    if (matches (name, "time"))
      {

        assign_string (& value,
                       strprintf ("%lu", timer_to_time (file-> timestamp)));
        
      }
    else
    if (matches (name, "date"))
      {

        assign_string (& value,
                       strprintf ("%lu", timer_to_date (file-> timestamp)));
        
      }
    else
    if (name == NULL || name [0] == 0)
      {

        assign_string (& value, file-> name);
        
      }

    return & value;
        
}

static int file_entry_next_sibling (void *olditem, const char *name, Bool ignorecase, CLASS_DESCRIPTOR **class, void **item)
{
    
    FILE_ENTRY_ITEM
        *file = olditem;
    DIRST
        *dirst;
    int
        rc;

    ASSERT (file);

    dirst = file-> dirst;
    file-> dirst = NULL;
    rc = read_dir (dirst);

    if (rc)
      {
        get_directory_entry (file-> parent, dirst, name, ignorecase, class, item);
        return 0;
      }
    else
      {
        close_dir (dirst);
        mem_free (dirst);
        return -1;
      }
    
}

static int file_entry_parent (void *olditem, CLASS_DESCRIPTOR **class, void **item, THREAD *gsl_thread)
{
    
    FILE_ENTRY_ITEM
        *file = olditem;

    ASSERT (file);

    *item = file-> parent;
    if (*item)
        *class = & directory_entry_class;

    return 0;
    
}


static int
directory_open (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread)
{
    RESULT_NODE *path    = argc > 0 ? argv [0] : NULL;
    RESULT_NODE *error   = argc > 1 ? argv [1] : NULL;


  {
    FILE_CONTEXT
        *context = item;
    char
        *curpath,
        *pathname,
        *lastchar;
    DIRST
        *dirst;
    int
        rc;
    DIRECTORY_ENTRY_ITEM
        *directory;

    ASSERT (context);

    curpath = get_curdir ();
    pathname = locate_path (curpath,
                            path ? string_value (&path-> value) : ".");
    mem_free (curpath);
    lastchar = pathname + strlen (pathname) - 1;
    if (*lastchar == '/')
        *lastchar = 0;
    curpath = strip_file_name (pathname);

    dirst = memt_alloc (NULL, sizeof (DIRST));

    errno = 0;
    rc = open_dir (dirst, pathname);

    if (rc)
      {
        directory = memt_alloc (NULL, sizeof (DIRECTORY_ENTRY_ITEM));
        curpath =  strip_file_name (pathname);
        directory-> path      = memt_alloc (NULL, strlen (curpath) + 2);
        xstrcpy (directory-> path, curpath, "/", NULL);
        directory-> name      = memt_strdup (NULL, strip_file_path (pathname));
        directory-> dirst     = dirst;
        directory-> links     = 0;
        directory-> parent    = NULL;
        directory-> exists    = TRUE;

        assign_pointer (& result-> value, & directory_entry_class, directory);
      }
    else
      {
        close_dir (dirst);
        mem_free (dirst);
      }
    mem_free (pathname);

    return store_module_error (gsl_thread, context, error,
                               errno ? strerror (errno) : NULL);
  }
        
    return 0;  /*  Just in case  */
}


static int
directory_setcwd (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread)
{
    RESULT_NODE *path    = argc > 0 ? argv [0] : NULL;
    RESULT_NODE *error   = argc > 1 ? argv [1] : NULL;

    if (! path)
      {
        strcpy (object_error, "Missing argument: path");
        return -1;
      }
    if (path-> value. type == TYPE_UNDEFINED)
      {
        result-> culprit = path-> culprit;
        path-> culprit = NULL;
        return 0;
      }

  {
    FILE_CONTEXT
        *context = item;

    errno = 0;

    assign_number (& result-> value,
                 set_curdir (string_value (&path-> value)));
    return store_module_error (gsl_thread, context, error,
                               errno ? strerror (errno) : NULL);
  }
    
    return 0;  /*  Just in case  */
}


static int
directory_create (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread)
{
    RESULT_NODE *path    = argc > 0 ? argv [0] : NULL;

    if (! path)
      {
        strcpy (object_error, "Missing argument: path");
        return -1;
      }
    if (path-> value. type == TYPE_UNDEFINED)
      {
        result-> culprit = path-> culprit;
        path-> culprit = NULL;
        return 0;
      }

  {
    result-> value.n    = make_dir (string_value (&path-> value));
    result-> value.type = TYPE_NUMBER;
  }
        
    return 0;  /*  Just in case  */
}


static int
directory_delete (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread)
{
    RESULT_NODE *path    = argc > 0 ? argv [0] : NULL;
    RESULT_NODE *error   = argc > 1 ? argv [1] : NULL;

    if (! path)
      {
        strcpy (object_error, "Missing argument: path");
        return -1;
      }
    if (path-> value. type == TYPE_UNDEFINED)
      {
        result-> culprit = path-> culprit;
        path-> culprit = NULL;
        return 0;
      }

  {
    FILE_CONTEXT
        *context = item;

    errno = 0;

    assign_number (& result-> value,
                   remove_dir (string_value (& path-> value)));
    return store_module_error (gsl_thread, context, error,
                               errno ? strerror (errno) : NULL);
  }
        
    return 0;  /*  Just in case  */
}


static int
directory_resolve (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread)
{
    RESULT_NODE *path    = argc > 0 ? argv [0] : NULL;
    RESULT_NODE *separator = argc > 1 ? argv [1] : NULL;

    if (! path)
      {
        strcpy (object_error, "Missing argument: path");
        return -1;
      }
    if (path-> value. type == TYPE_UNDEFINED)
      {
        result-> culprit = path-> culprit;
        path-> culprit = NULL;
        return 0;
      }

  {
    char
        *curpath,
        *clean;

    if (separator
    &&  separator-> value. type != TYPE_UNDEFINED)
      {
        if (strlen (string_value (& separator-> value)) != 1)
          {
            strcpy (object_error,
                    "Argument 2 (separator) to directory.resolve must be a single character.");
            return -1;
          }
      }
    curpath = get_curdir ();
    clean = locate_path (curpath, string_value (& path-> value));
    mem_free (curpath);
    if (separator
    &&  separator-> value. type != TYPE_UNDEFINED)
      {
        strconvch (clean, '/',  separator-> value. s [0]);
        strconvch (clean, '\\', separator-> value. s [0]);
      }
    assign_string (& result-> value, clean);
  }
        
    return 0;  /*  Just in case  */
}


static int
directory_entry_name (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread)
{


  {
    char
        *item_name;

    item_name = mem_strdup ((char *) directory_entry_item_name
                                         (item));

    if (item_name)
        assign_string (& result-> value, item_name);

    return 0;
  }
        
    return 0;  /*  Just in case  */
}


static int
directory_entry_first (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread)
{
    RESULT_NODE *name    = argc > 0 ? argv [0] : NULL;


  {
    int
        rc;
    CLASS_DESCRIPTOR
        *returnclass;
    void
        *returnitem;

    rc = directory_entry_first_child
             (item,
              name ? string_value (& name-> value) : NULL,
              ((GGCODE_TCB *) gsl_thread-> tcb)-> gsl-> ignorecase,
              & returnclass,
              & returnitem);

    if ((! rc)
    &&  returnitem)
        assign_pointer (& result-> value, returnclass, returnitem);

    return rc;
  }
        
    return 0;  /*  Just in case  */
}


static int
directory_entry_next (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread)
{
    RESULT_NODE *name    = argc > 0 ? argv [0] : NULL;


  {
    int
        rc;
    CLASS_DESCRIPTOR
        *returnclass;
    void
        *returnitem;

    rc = directory_entry_next_sibling
             (item,
              name ? string_value (& name-> value) : NULL,
              ((GGCODE_TCB *) gsl_thread-> tcb)-> gsl-> ignorecase,
              & returnclass,
              & returnitem);

    if ((! rc)
    &&  item)
        assign_pointer (& result-> value, returnclass, returnitem);

    return rc;
}
        
    return 0;  /*  Just in case  */
}


static int
directory_entry_parent_function (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread)
{


  {
    int
        rc;
    CLASS_DESCRIPTOR
        *returnclass;
    void
        *returnitem;

    rc = directory_entry_parent
             (item,
              & returnclass,
              & returnitem,
              gsl_thread);

    if ((! rc)
    &&  item)
        assign_pointer (& result-> value, returnclass, returnitem);

    return rc;
  }
        
    return 0;  /*  Just in case  */
}


static int
directory_entry_new (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread)
{
    RESULT_NODE *name    = argc > 0 ? argv [0] : NULL;


  {
    int
        rc;
    CLASS_DESCRIPTOR
        *returnclass;
    void
        *returnitem;

    rc = directory_entry_create
             (name ? string_value (& name-> value) : NULL,
              item,
              NULL,
              & returnclass,
              & returnitem);

    if ((! rc)
    &&  item)
        assign_pointer (& result-> value, returnclass, returnitem);

    return rc;
  }
        
    return 0;  /*  Just in case  */
}


static int
fileopen (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread)
{
    RESULT_NODE *filename = argc > 0 ? argv [0] : NULL;
    RESULT_NODE *mode    = argc > 1 ? argv [1] : NULL;
    RESULT_NODE *error   = argc > 2 ? argv [2] : NULL;

    if (! filename)
      {
        strcpy (object_error, "Missing argument: filename");
        return -1;
      }
    if (filename-> value. type == TYPE_UNDEFINED)
      {
        result-> culprit = filename-> culprit;
        filename-> culprit = NULL;
        return 0;
      }

  {
    FILE_CONTEXT
        *context = item;
    FILE_ENTRY_ITEM
        *file;
    int
        rc;

    ASSERT (context);

    create_file_entry (string_value (& filename-> value),
                       context,
                       result,
                       error,
                       gsl_thread);

    file = result-> value. i;
    rc = open_the_file (file,
                        (char) (mode ? *string_value (& mode-> value) : 'r'),
                        error,
                        gsl_thread);
    if (rc
    || (! file-> handle))
      {
        file_entry_destroy (file);
        init_value (& result-> value);
      }
    return rc;
  }
        
    return 0;  /*  Just in case  */
}


static int
fileread (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread)
{
    RESULT_NODE *handle  = argc > 0 ? argv [0] : NULL;
    RESULT_NODE *error   = argc > 1 ? argv [1] : NULL;

    if (! handle)
      {
        strcpy (object_error, "Missing argument: handle");
        return -1;
      }
    if (handle-> value. type == TYPE_UNDEFINED)
      {
        result-> culprit = handle-> culprit;
        handle-> culprit = NULL;
        return 0;
      }

  {
    FILE_CONTEXT
        *context = item;

    ASSERT (context);

    if (handle-> value. type != TYPE_POINTER
    ||  handle-> value. c    != & file_entry_class)
      {
        errno = EBADF;
        return store_module_error (gsl_thread, context, error,
                                   errno ? strerror (errno) : NULL);
      }

    return read_the_file (handle-> value. i,
                          result,
                          error,
                          gsl_thread);
  }
        
    return 0;  /*  Just in case  */
}


static int
filewrite (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread)
{
    RESULT_NODE *handle  = argc > 0 ? argv [0] : NULL;
    RESULT_NODE *string  = argc > 1 ? argv [1] : NULL;
    RESULT_NODE *error   = argc > 2 ? argv [2] : NULL;

    if (! handle)
      {
        strcpy (object_error, "Missing argument: handle");
        return -1;
      }
    if (handle-> value. type == TYPE_UNDEFINED)
      {
        result-> culprit = handle-> culprit;
        handle-> culprit = NULL;
        return 0;
      }
    if (! string)
      {
        strcpy (object_error, "Missing argument: string");
        return -1;
      }
    if (string-> value. type == TYPE_UNDEFINED)
      {
        result-> culprit = string-> culprit;
        string-> culprit = NULL;
        return 0;
      }

  {
    if (handle-> value. type != TYPE_POINTER
    ||  handle-> value. c    != & file_entry_class)
      {
        errno = EBADF;
        return store_file_error (handle-> value. i, gsl_thread, error,
                                 errno ? strerror (errno) : NULL);
      }

    return write_the_file (handle-> value. i,
                           string_value (&string-> value),
                           result,
                           error,
                           gsl_thread);
  }
        
    return 0;  /*  Just in case  */
}


static int
fileclose (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread)
{
    RESULT_NODE *handle  = argc > 0 ? argv [0] : NULL;
    RESULT_NODE *error   = argc > 1 ? argv [1] : NULL;

    if (! handle)
      {
        strcpy (object_error, "Missing argument: handle");
        return -1;
      }
    if (handle-> value. type == TYPE_UNDEFINED)
      {
        result-> culprit = handle-> culprit;
        handle-> culprit = NULL;
        return 0;
      }

  {
    if (handle-> value. type != TYPE_POINTER
    ||  handle-> value. c    != & file_entry_class)
      {
        errno = EBADF;
        return store_file_error (handle-> value. i, gsl_thread, error,
                                 errno ? strerror (errno) : NULL);
      }

    return close_the_file (handle-> value. i,
                           result,
                           error,
                           gsl_thread);
  }
        
    return 0;  /*  Just in case  */
}


static int
file_tell (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread)
{
    RESULT_NODE *handle  = argc > 0 ? argv [0] : NULL;
    RESULT_NODE *error   = argc > 1 ? argv [1] : NULL;

    if (! handle)
      {
        strcpy (object_error, "Missing argument: handle");
        return -1;
      }
    if (handle-> value. type == TYPE_UNDEFINED)
      {
        result-> culprit = handle-> culprit;
        handle-> culprit = NULL;
        return 0;
      }

  {
    if (handle-> value. type != TYPE_POINTER
    ||  handle-> value. c    != & file_entry_class)
      {
        errno = EBADF;
        return store_file_error (handle-> value. i, gsl_thread, error,
                                 errno ? strerror (errno) : NULL);
      }

    return tell_the_file (handle-> value. i,
                          result,
                          error,
                          gsl_thread);
  }
        
    return 0;  /*  Just in case  */
}


static int
file_seek (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread)
{
    RESULT_NODE *handle  = argc > 0 ? argv [0] : NULL;
    RESULT_NODE *offset  = argc > 1 ? argv [1] : NULL;
    RESULT_NODE *error   = argc > 2 ? argv [2] : NULL;

    if (! handle)
      {
        strcpy (object_error, "Missing argument: handle");
        return -1;
      }
    if (handle-> value. type == TYPE_UNDEFINED)
      {
        result-> culprit = handle-> culprit;
        handle-> culprit = NULL;
        return 0;
      }

  {
    if (handle-> value. type != TYPE_POINTER
    ||  handle-> value. c    != & file_entry_class)
      {
        errno = EBADF;
        return store_file_error (handle-> value. i, gsl_thread, error,
                                 errno ? strerror (errno) : NULL);
      }

    return seek_the_file (handle-> value. i,
                          offset ? (qbyte) number_value (&offset-> value) : 0,
                          result,
                          error,
                          gsl_thread);
  }
        
    return 0;  /*  Just in case  */
}


static int
fileslurp (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread)
{
    RESULT_NODE *filename = argc > 0 ? argv [0] : NULL;
    RESULT_NODE *error   = argc > 1 ? argv [1] : NULL;

    if (! filename)
      {
        strcpy (object_error, "Missing argument: filename");
        return -1;
      }
    if (filename-> value. type == TYPE_UNDEFINED)
      {
        result-> culprit = filename-> culprit;
        filename-> culprit = NULL;
        return 0;
      }

  {
    FILE_CONTEXT
        *context = item;
    DESCR
        *descr;
    byte
        *end_ptr;
    size_t
        length;
    char
        *string;

    errno = 0;
    descr = file_slurpl (string_value (&filename-> value));
    if (descr)
      {
        /*  Look for NULL byte  */
        end_ptr = memchr (descr-> data, 0, descr-> size);
        if (! end_ptr)
            length = descr-> size;
        else
            length = end_ptr - descr-> data;

        string = memt_alloc (NULL, length + 1);
        memcpy (string, descr-> data, length);
        string [length] = 0;

        mem_free (descr);

        assign_string (& result-> value, string);
      }
    return store_module_error (gsl_thread, context, error,
                               errno ? strerror (errno) : NULL);
  }
        
    return 0;  /*  Just in case  */
}


static int
fileexists (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread)
{
    RESULT_NODE *filename = argc > 0 ? argv [0] : NULL;
    RESULT_NODE *error   = argc > 1 ? argv [1] : NULL;

    if (! filename)
      {
        strcpy (object_error, "Missing argument: filename");
        return -1;
      }
    if (filename-> value. type == TYPE_UNDEFINED)
      {
        result-> culprit = filename-> culprit;
        filename-> culprit = NULL;
        return 0;
      }

  {
    FILE_CONTEXT
        *context = item;

    ASSERT (context);

    errno = 0;

    assign_number (& result-> value,
                   file_exists (string_value (&filename-> value)));

    return store_module_error (gsl_thread, context, error,
                               errno ? strerror (errno) : NULL);
  }
        
    return 0;  /*  Just in case  */
}


static int
file_timestamp (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread)
{
    RESULT_NODE *filename = argc > 0 ? argv [0] : NULL;
    RESULT_NODE *error   = argc > 1 ? argv [1] : NULL;

    if (! filename)
      {
        strcpy (object_error, "Missing argument: filename");
        return -1;
      }
    if (filename-> value. type == TYPE_UNDEFINED)
      {
        result-> culprit = filename-> culprit;
        filename-> culprit = NULL;
        return 0;
      }

  {
    FILE_CONTEXT
        *context = item;
    time_t
        timer;

    errno = 0;
    timer = get_file_time (string_value (&filename-> value));

    ASSERT (context);

    if (timer)
        assign_number (& result-> value, (double) timer_to_date (timer) * 1000000
                                     + (double) timer_to_time (timer));

    return store_module_error (gsl_thread, context, error,
                               errno ? strerror (errno) : NULL);
  }
        
    return 0;  /*  Just in case  */
}


static int
filerename (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread)
{
    RESULT_NODE *oldname = argc > 0 ? argv [0] : NULL;
    RESULT_NODE *newname = argc > 1 ? argv [1] : NULL;
    RESULT_NODE *error   = argc > 2 ? argv [2] : NULL;

    if (! oldname)
      {
        strcpy (object_error, "Missing argument: oldname");
        return -1;
      }
    if (oldname-> value. type == TYPE_UNDEFINED)
      {
        result-> culprit = oldname-> culprit;
        oldname-> culprit = NULL;
        return 0;
      }
    if (! newname)
      {
        strcpy (object_error, "Missing argument: newname");
        return -1;
      }
    if (newname-> value. type == TYPE_UNDEFINED)
      {
        result-> culprit = newname-> culprit;
        newname-> culprit = NULL;
        return 0;
      }

  {
    FILE_CONTEXT
        *context = item;

    ASSERT (context);

    errno = 0;

    assign_number (& result-> value,
                   file_rename (string_value (&oldname-> value),
                                string_value (&newname-> value)));

    return store_module_error (gsl_thread, context, error,
                               errno ? strerror (errno) : NULL);
  }
        
    return 0;  /*  Just in case  */
}


static int
filedelete (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread)
{
    RESULT_NODE *filename = argc > 0 ? argv [0] : NULL;
    RESULT_NODE *error   = argc > 1 ? argv [1] : NULL;

    if (! filename)
      {
        strcpy (object_error, "Missing argument: filename");
        return -1;
      }
    if (filename-> value. type == TYPE_UNDEFINED)
      {
        result-> culprit = filename-> culprit;
        filename-> culprit = NULL;
        return 0;
      }

  {
    FILE_CONTEXT
        *context = item;

    errno = 0;

    assign_number (& result-> value,
                   file_delete (string_value (&filename-> value)));
    return store_module_error (gsl_thread, context, error,
                               errno ? strerror (errno) : NULL);
  }
        
    return 0;  /*  Just in case  */
}


static int
filelocate (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread)
{
    RESULT_NODE *filename = argc > 0 ? argv [0] : NULL;
    RESULT_NODE *path    = argc > 1 ? argv [1] : NULL;
    RESULT_NODE *error   = argc > 2 ? argv [2] : NULL;

    if (! filename)
      {
        strcpy (object_error, "Missing argument: filename");
        return -1;
      }
    if (filename-> value. type == TYPE_UNDEFINED)
      {
        result-> culprit = filename-> culprit;
        filename-> culprit = NULL;
        return 0;
      }

  {
    FILE_CONTEXT
        *context = item;
    char
        *found;

    ASSERT (context);

    errno = 0;

    found = file_where ('r',
            path? string_value (&path-> value): "PATH",
            string_value (&filename-> value), NULL);

    if (found)
        assign_string (& result-> value, memt_strdup (NULL, found));

    return store_module_error (gsl_thread, context, error,
                               errno ? strerror (errno) : NULL);
  }
        
    return 0;  /*  Just in case  */
}


static int
filecopy (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread)
{
    RESULT_NODE *src     = argc > 0 ? argv [0] : NULL;
    RESULT_NODE *dest    = argc > 1 ? argv [1] : NULL;
    RESULT_NODE *mode    = argc > 2 ? argv [2] : NULL;
    RESULT_NODE *error   = argc > 3 ? argv [3] : NULL;

    if (! src)
      {
        strcpy (object_error, "Missing argument: src");
        return -1;
      }
    if (src-> value. type == TYPE_UNDEFINED)
      {
        result-> culprit = src-> culprit;
        src-> culprit = NULL;
        return 0;
      }
    if (! dest)
      {
        strcpy (object_error, "Missing argument: dest");
        return -1;
      }
    if (dest-> value. type == TYPE_UNDEFINED)
      {
        result-> culprit = dest-> culprit;
        dest-> culprit = NULL;
        return 0;
      }

  {
    FILE_CONTEXT
        *context = item;

    ASSERT (context);

    errno = 0;

    assign_number (& result-> value,
                  (double) file_copy (
                      string_value (&dest-> value),
                      string_value (&src -> value),
                      (char) (mode ? *string_value (&mode-> value): 'b')));

    return store_module_error (gsl_thread, context, error,
                               errno ? strerror (errno) : NULL);
  }
        
    return 0;  /*  Just in case  */
}


static int
file_basename (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread)
{
    RESULT_NODE *filename = argc > 0 ? argv [0] : NULL;

    if (! filename)
      {
        strcpy (object_error, "Missing argument: filename");
        return -1;
      }
    if (filename-> value. type == TYPE_UNDEFINED)
      {
        result-> culprit = filename-> culprit;
        filename-> culprit = NULL;
        return 0;
      }

  {
    char
        *strptr = strip_extension (string_value (&filename-> value));

    assign_string (& result-> value, memt_strdup (NULL, strptr));
  }
        
    return 0;  /*  Just in case  */
}


static int
file_entry_open (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread)
{
    RESULT_NODE *mode    = argc > 0 ? argv [0] : NULL;
    RESULT_NODE *error   = argc > 1 ? argv [1] : NULL;


  {
    FILE_ENTRY_ITEM
        *file = item;
    int
        rc;

    ASSERT (file);

    rc = open_the_file (file,
                        (char) (mode ? *string_value (& mode-> value) : 'r'),
                        error,
                        gsl_thread);
    if (! rc)
        assign_number (& result-> value, file-> handle ? 0 : -1);

    return rc;
  }
        
    return 0;  /*  Just in case  */
}


static int
file_entry_read (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread)
{
    RESULT_NODE *error   = argc > 0 ? argv [0] : NULL;


  {
    FILE_ENTRY_ITEM
        *file = item;

    ASSERT (file);

    return read_the_file (file,
                          result,
                          error,
                          gsl_thread);
  }
        
    return 0;  /*  Just in case  */
}


static int
file_entry_write (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread)
{
    RESULT_NODE *string  = argc > 0 ? argv [0] : NULL;
    RESULT_NODE *error   = argc > 1 ? argv [1] : NULL;

    if (! string)
      {
        strcpy (object_error, "Missing argument: string");
        return -1;
      }
    if (string-> value. type == TYPE_UNDEFINED)
      {
        result-> culprit = string-> culprit;
        string-> culprit = NULL;
        return 0;
      }

  {
    FILE_ENTRY_ITEM
        *file = item;

    ASSERT (file);

    return write_the_file (file,
                           string_value (&string-> value),
                           result,
                           error,
                           gsl_thread);
  }
        
    return 0;  /*  Just in case  */
}


static int
file_entry_close (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread)
{
    RESULT_NODE *error   = argc > 0 ? argv [0] : NULL;


  {
    FILE_ENTRY_ITEM
        *file = item;

    ASSERT (file);

    return close_the_file (file,
                           result,
                           error,
                           gsl_thread);
  }
        
    return 0;  /*  Just in case  */
}


static int
file_entry_tell (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread)
{
    RESULT_NODE *error   = argc > 0 ? argv [0] : NULL;


  {
    FILE_ENTRY_ITEM
        *file = item;

    ASSERT (file);

    return tell_the_file (file,
                          result,
                          error,
                          gsl_thread);
  }
        
    return 0;  /*  Just in case  */
}


static int
file_entry_seek (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread)
{
    RESULT_NODE *offset  = argc > 0 ? argv [0] : NULL;
    RESULT_NODE *error   = argc > 1 ? argv [1] : NULL;


  {
    FILE_ENTRY_ITEM
        *file = item;

    ASSERT (file);

    return seek_the_file (file,
                          offset ? (qbyte) number_value (&offset-> value) : 0,
                          result,
                          error,
                          gsl_thread);
  }
        
    return 0;  /*  Just in case  */
}


static int
file_entry_name (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread)
{


  {
    char
        *item_name;

    item_name = mem_strdup ((char *) file_entry_item_name
                                         (item));

    if (item_name)
        assign_string (& result-> value, item_name);

    return 0;
  }
        
    return 0;  /*  Just in case  */
}


static int
file_entry_next (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread)
{
    RESULT_NODE *name    = argc > 0 ? argv [0] : NULL;


  {
    int
        rc;
    CLASS_DESCRIPTOR
        *returnclass;
    void
        *returnitem;

    rc = file_entry_next_sibling
             (item,
              name ? string_value (& name-> value) : NULL,
              ((GGCODE_TCB *) gsl_thread-> tcb)-> gsl-> ignorecase,
              & returnclass,
              & returnitem);

    if ((! rc)
    &&  item)
        assign_pointer (& result-> value, returnclass, returnitem);

    return rc;
}
        
    return 0;  /*  Just in case  */
}


static int
file_entry_parent_function (int argc, RESULT_NODE **argv, void *item, RESULT_NODE *result, THREAD *gsl_thread)
{


  {
    int
        rc;
    CLASS_DESCRIPTOR
        *returnclass;
    void
        *returnitem;

    rc = file_entry_parent
             (item,
              & returnclass,
              & returnitem,
              gsl_thread);

    if ((! rc)
    &&  item)
        assign_pointer (& result-> value, returnclass, returnitem);

    return rc;
  }
        
    return 0;  /*  Just in case  */
}

static int directory_class_init (CLASS_DESCRIPTOR **class, void **item, THREAD *gsl_thread)
{
     *class = & directory_class;


  {
    ASSERT (last_context == NULL);

    last_context = memt_alloc (NULL, sizeof (FILE_CONTEXT));
    last_context-> links     = 0;
    last_context-> error_msg = NULL;

    *item = last_context;
  }
    
    return 0;
}

static int file_class_init (CLASS_DESCRIPTOR **class, void **item, THREAD *gsl_thread)
{
     *class = & file_class;


    ASSERT (last_context);
    *item = last_context;
    last_context = NULL;
    
    return 0;
}

int register_file_classes (void)
{
    int
        rc = 0;
    rc |= object_register (directory_class_init,
                           NULL);
    rc |= object_register (file_class_init,
                           NULL);
    return rc;
}
