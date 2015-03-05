/*===========================================================================*
 *                                                                           *
 *  sflfile.c - File manipulation functions                                  *
 *                                                                           *
 *  Copyright (c) 1991-2010 iMatix Corporation                               *
 *                                                                           *
 *  ------------------ GPL Licensed Source Code ------------------           *
 *  iMatix makes this software available under the GNU General               *
 *  Public License (GPL) license for open source projects.  For              *
 *  details of the GPL license please see www.gnu.org or read the            *
 *  file license.gpl provided in this package.                               *
 *                                                                           *
 *  This program is free software; you can redistribute it and/or            *
 *  modify it under the terms of the GNU General Public License as           *
 *  published by the Free Software Foundation; either version 2 of           *
 *  the License, or (at your option) any later version.                      *
 *                                                                           *
 *  This program is distributed in the hope that it will be useful,          *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of           *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            *
 *  GNU General Public License for more details.                             *
 *                                                                           *
 *  You should have received a copy of the GNU General Public                *
 *  License along with this program in the file 'license.gpl'; if            *
 *  not, write to the Free Software Foundation, Inc., 59 Temple              *
 *  Place - Suite 330, Boston, MA 02111-1307, USA.                           *
 *                                                                           *
 *  You can also license this software under iMatix's General Terms          *
 *  of Business (GTB) for commercial projects.  If you have not              *
 *  explicitly licensed this software under the iMatix GTB you may           *
 *  only use it under the terms of the GNU General Public License.           *
 *                                                                           *
 *  For more information, send an email to info@imatix.com.                  *
 *  --------------------------------------------------------------           *
 *===========================================================================*/

#include "prelude.h"                    /*  Universal header file            */
#include "sflstr.h"                     /*  String handling functions        */
#include "sfllist.h"                    /*  Linked-list functions            */
#include "sflmem.h"                     /*  Memory allocation functions      */
#include "sflnode.h"                    /*  Linked-list functions            */
#include "sfldir.h"                     /*  Directory access functions       */
#include "sfldate.h"                    /*  Date/time access functions       */
#include "sflsymb.h"                    /*  Symbol-table functions           */
#include "sfltok.h"                     /*  Token mashing functions          */
#include "sflcons.h"                    /*  Console output functions         */
#include "sflenv.h"                     /*  Environment access functions     */
#include "sflprint.h"                   /*  snprintf functions               */
#include "sfldescr.h"                   /*  Descriptor functions             */
#include "sflfile.h"                    /*  Prototypes for functions         */

#if defined(WIN32)
#include <windows.h>                    /*  For device name detection        */
#endif

/*  Ensure our buffers will be big enough for dir + name + delimiters        */
#if ((LINE_MAX - FILE_NAME_MAX) < (FILE_DIR_MAX + 10))
#   error "Cannot compile; FILE_NAME_MAX is too large."
#endif

static char
#if (PATHFOLD == TRUE || defined (GATES_FILESYSTEM))
    path_name [PATH_MAX + 1],           /*  Copy of path symbol              */
#endif
    work_name [LINE_MAX + 1],           /*  Name plus ext                    */
    full_name [LINE_MAX + 1],           /*  Dir plus name plus ext           */
    exec_name [LINE_MAX + 1];           /*  Executable file name             */

Bool file_crlf = FALSE;                 /*  Initial default                  */


/*  Function prototypes                                                      */

#if (defined (GATES_FILESYSTEM))
static Bool   system_devicename   (const char *filename);
#endif
static char  *build_next_path     (char *dest, const char *path,
                                               const char *name);
static char  *build_next_path_ext (char *dest, const char *path,
                                               const char *name,
                                               const char *ext);
static dbyte  file_mode           (const char *filename);
#if (defined (__MSDOS__))
static Bool   is_exe_file         (const char *filename);
#endif
static DESCR *file_load_data      (const char *filename, size_t limit);
static Bool   fully_specified     (const char *filename);


/*  ---------------------------------------------------------------------[<]-
    Function: file_open

    Synopsis: opens a text file for reading or writing.  Use in combination
    with the file_read() and file_write() functions.  These functions handle
    end-of-line sequences using a heuristic that works as follows.
    ... (at this point the author went for a pint of beer and has not been
    seen since.  We're hoping that the old version - following - is ok.)

    Synopsis: Opens the specified file for input or output.  If you use
    the file_read / file_write functions you must open the file using this
    function.  This set of functions lets you read files without concern
    for the line format (CRLF or LF).  Mode should be one of 'r' 'w' 'a'.

    Returns a FILE pointer if the file is opened correctly; else NULL.
    Sets the global variable file_crlf to FALSE on all systems except MS-DOS
    (and Windows by inheritence) where it is set to TRUE by default.

    When opening a file in append mode, automatically removes any Ctrl-Z
    character under MS-DOS or OS/2.
    ---------------------------------------------------------------------[>]-*/

FILE *
file_open (
    const char *filename,               /*  Name of file to open             */
    char mode)                          /*  'r', 'w', or 'a'                 */
{
#if (defined (GATES_FILESYSTEM))
    if (system_devicename (filename))
        return (NULL);                  /*  Not allowed on device names      */

    file_crlf = TRUE;
#   if (defined (WIN32))
    SetErrorMode (SEM_FAILCRITICALERRORS | SEM_NOOPENFILEERRORBOX);
#   endif
#   else

    ASSERT (filename);
    file_crlf = FALSE;
#endif

    if (mode == 'r')
        return (fopen (filename, FOPEN_READ_BINARY));
    else
    if (mode == 'w')
        return (fopen (filename, FOPEN_WRITE_BINARY));
    else
    if (mode == 'a'
    &&  safe_to_extend (filename))
        return (fopen (filename, FOPEN_APPEND_BINARY));
    else
        return (NULL);                  /*  Invalid mode                     */
}


#if (defined (GATES_FILESYSTEM))
/*  Under MS-DOS, and derived systems, use of filenames containing device
 *  names such as 'aux', 'con', 'prn', or 'nul' can cause problems, 
 *  especially for reading.  We reject attempts to open devices using our
 *  file routines to avoid these problems.  (Handling device files like
 *  this is unportable, and so should be done with the native API.)  Under
 *  Win32 we can ask directly if the we have a device name.  Under other
 *  operating systems we have to guess based on the filename.
 */

/*  List of dos device names that we know about, used as a fall back if we   */
/*  cannot query the OS more directly.                                       */
static const char * const KNOWN_DOS_DEVICES[] =
{
  "aux",
  "con",
  "nul",
  "prn",
  "clock$",
  "config$",
  "com1",
  "com2",
  "com3",
  "com4",
  "lpt1",
  "lpt2",
  "lpt3",
  0
};

static Bool
system_devicename (const char *supplied_filename)
{
    Bool
        is_devicefile;

#if defined(WIN32)
    /*  Under Win32 we can open the file, and ask what type it is; if it is  */
    /*  not a disk file we reject it.  If we can't open it, we indicate that */
    /*  it is safe, so that the open will be retried and the correct error   */
    /*  codes set for the caller.                                            */
    HANDLE fh;

    is_devicefile = TRUE;

    fh = CreateFileA(supplied_filename, GENERIC_READ, FILE_SHARE_READ, NULL,
                    OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if (fh != INVALID_HANDLE_VALUE)
      {
        if (GetFileType(fh) == FILE_TYPE_DISK)
            is_devicefile = FALSE;

        CloseHandle(fh);
      }
    else
        is_devicefile = FALSE;          /* Doesn't exist                     */

    return (is_devicefile);
#else
    char
        *filename,    
        *char_ptr,
        *token,
        **tokens;
    int
        token_nbr;
    const char 
        * const * dev_ptr;

    filename = mem_strdup (supplied_filename);
    strconvch (filename, ' ', '_');     /*  Don't break on real spaces       */
    strconvch (filename, '/', ' ');
    strconvch (filename, '\\', ' ');
    strlwc    (filename);               /*  All comparisons in lowercase     */

    /*  Skip disk specifier if present (along with something else)           */
    if (strlen (filename) > 2 && filename [1] == ':')
        filename [0] = filename [1] = ' ';

    /*  Wipe out extensions on file/directory names                          */
    for (char_ptr = filename; *char_ptr; char_ptr++)
      {
        if (*char_ptr == '.')           /*  Wipe up to dir marker/end        */
            while (*char_ptr && *char_ptr != ' ')
                *char_ptr++ = ' ';

        if (*char_ptr == '\0')
            break;
      }

    tokens = tok_split (filename);
    is_devicefile = FALSE;

    for (token_nbr = 0; (! is_devicefile) && tokens [token_nbr]; token_nbr++)
      {
        token = tokens [token_nbr];

        /*  Perform manual check against our list of "known" device names.   */
        for (dev_ptr = KNOWN_DOS_DEVICES; *dev_ptr; ++dev_ptr)
            if (streq (token, *dev_ptr))
              {
                is_devicefile = TRUE;
                break;
              }
      }
    tok_free (tokens);
    mem_free (filename);
    return (is_devicefile);
#endif
}
#endif


/*  ---------------------------------------------------------------------[<]-
    Function: file_locate

    Synopsis: Combines the functions of file_where() and file_open when you
    want to read a file.  Searches for a file on a specified path, opens the
    file if found, and returns a FILE * for the open file.  Returns NULL if
    the file was not found or could not be opened for reading.
    ---------------------------------------------------------------------[>]-*/

FILE *
file_locate (
    const char *path,
    const char *name,
    const char *ext)
{
    char
        *filename;

    ASSERT (name);
    filename = file_where ('r', path, name, ext);
    if (filename)
        return (file_open (filename, 'r'));
    else
        return (NULL);
}


/*  ---------------------------------------------------------------------[<]-
    Function: file_close

    Synopsis: Closes an open file stream.  Returns 0 if okay, -1 if there
    was an error.  For now, equivalent to fclose, and supplied because it
    looks nice when you use file_open() and file_close() together.
    ---------------------------------------------------------------------[>]-*/

int
file_close (
    FILE *stream)
{
    if (stream)
        return (fclose (stream));
    else
        return (0);
}


/*  ---------------------------------------------------------------------[<]-
    Function: file_read

    Synopsis: Reads a line of text delimited by newline from the stream.
    The string must be LINE_MAX + 1 long.  Places a null byte in place of
    the newline character.  Expands tab characters to every 8th column.
    Returns TRUE when there is more input waiting; FALSE when the last line
    of the file has been read.

    Sets the global variable file_crlf to TRUE if CR was found in the file.
    This variable is by default FALSE.  It is also used by file_write.
    ---------------------------------------------------------------------[>]-*/

Bool
file_read (
    FILE *stream,
    char *string)
{
    int
        ch,                             /*  Character read from file         */
        cnbr;                           /*  Index into returned string       */

    ASSERT (stream);
    ASSERT (string);

    cnbr = 0;                           /*  Start at the beginning...        */
    memset (string, ' ', LINE_MAX);     /*    and prepare entire line        */
    for (;;)
      {
        ch = fgetc (stream);            /*  Get next character from file     */
        if (ch == '\t')                 /*  Jump if tab                      */
            cnbr = ((cnbr >> 3) << 3) + 8;
        else
        if (ch == '\r')                 /*  Found carriage-return            */
            file_crlf = TRUE;           /*    Set flag and ignore CR         */
        else
        if ((ch == '\n')                /*  Have end of line                 */
        ||  (ch == EOF)                 /*    or end of file                 */
        ||  (ch == 26))                 /*    or MS-DOS Ctrl-Z               */
          {
            string [cnbr] = '\0';       /*  Terminate string                 */
            return (ch == '\n' || cnbr);    /*  and return TRUE/FALSE        */
          }
        else
        if (cnbr < LINE_MAX)
            string [cnbr++] = (char) ch;    /*  Else add char to string      */

        if (cnbr >= LINE_MAX)           /*  Return in any case if line is    */
          {                             /*    too long - the line will be    */
            string [LINE_MAX] = '\0';   /*    cut into pieces                */
            return (TRUE);
          }
      }
}


/*  ---------------------------------------------------------------------[<]-
    Function: file_readn

    Synopsis: Works as file_read() but with a maximum line-length specified
    by the caller.  The supplied buffer must be at least as large as the
    specified line_max + 1.
    ---------------------------------------------------------------------[>]-*/

Bool
file_readn (
    FILE *stream,
    char *string,
    int   line_max)
{
    int
        ch,                             /*  Character read from file         */
        cnbr;                           /*  Index into returned string       */

    ASSERT (stream);
    ASSERT (string);

    cnbr = 0;                           /*  Start at the beginning...        */
    memset (string, ' ', line_max);     /*    and prepare entire line        */
    for (;;)
      {
        ch = fgetc (stream);            /*  Get next character from file     */
        if (ch == '\t')                 /*  Jump if tab                      */
            cnbr = ((cnbr >> 3) << 3) + 8;
        else
        if (ch == '\r')                 /*  Found carriage-return            */
            file_crlf = TRUE;           /*    Set flag and ignore CR         */
        else
        if ((ch == '\n')                /*  Have end of line                 */
        ||  (ch == EOF)                 /*    or end of file                 */
        ||  (ch == 26))                 /*    or MS-DOS Ctrl-Z               */
          {
            string [cnbr] = '\0';       /*  Terminate string                 */
            return (ch == '\n' || cnbr);    /*  and return TRUE/FALSE        */
          }
        else
        if (cnbr < line_max)
            string [cnbr++] = (char) ch;    /*  Else add char to string      */

        if (cnbr >= line_max)           /*  Return in any case if line is    */
          {                             /*    too long - the line will be    */
            string [line_max] = '\0';   /*    cut into pieces                */
            return (TRUE);
          }
      }
}


/*  ---------------------------------------------------------------------[<]-
    Function: file_write

    Synopsis: Writes a line of text to the specified output stream.  If the
    variable file_crlf is TRUE, adds a carriage-return to the line being
    written to the output stream.  This variable is supplied so that you can
    either ignore crlf issues (do nothing), or handle them explicitly (play
    with file_crlf).  Returns the string written, or NULL if no data could
    be written to the file.
    ---------------------------------------------------------------------[>]-*/

char *
file_write (
    FILE *stream,
    const char *string)
{
    ASSERT (stream);
    ASSERT (string);

    fputs (string, stream);
    if (file_crlf)
        fputc ('\r', stream);

    if (fputc ('\n', stream) == EOF)
        return (NULL);
    else
        return ((char *) string);
}


/*  ---------------------------------------------------------------------[<]-
    Function: file_copy

    Synopsis: Copies a file called src to one called dest.  The dest file
    may not already exist.  If mode is 'b', copies a binary file; if mode is
    't', copies a text file.  This distinction only applies to MS-DOS file
    systems; on other platforms the two modes are equivalent.  Returns 0
    if no problems occurred, -1 if an error occurred, 1 if the destination
    file already exists.
    ---------------------------------------------------------------------[>]-*/

int
file_copy (
    const char *dest,
    const char *src,
    char mode)
{
    FILE *inf, *outf;
    char *buffer,
         openmode [3] = "??";
    size_t chars_read;                  /*  Amount read from stream          */
    int  feedback = 0;

    ASSERT (dest);
    ASSERT (src);
    if (file_exists (dest))
        return (1);                     /*  Cancel: dest already exists      */

#   if (defined (GATES_FILESYSTEM))
    if (system_devicename (dest) || system_devicename (src))
        return (-1);                    /*  Not allowed on device names      */
#   endif
#   if (defined (GATES_FILESYSTEM))
    openmode [1] = mode;
#   else
    openmode [1] = 0;
#   endif
    openmode [0] = 'r';
    if ((inf = fopen (src, openmode)) == NULL)
        return (-1);                    /*  Input file not found             */

    if ((buffer = mem_alloc (SHRT_MAX)) == NULL)
        feedback = -1;                  /*  Insufficient memory for buffer   */
    else
      {
        openmode [0] = 'w';
        if ((outf = fopen (dest, openmode)) == NULL)
            feedback = -1;             /*  Could not create output file     */
        else        
          {
            while ((chars_read = fread (buffer, 1, SHRT_MAX, inf)) != 0)
                if (fwrite (buffer, 1, chars_read, outf) != chars_read)
                  {
                    feedback = -1;
                    break;
                  }
            fclose (outf);
          }
        mem_free (buffer);
      }
    fclose (inf);
    return (feedback);
}


/*  ---------------------------------------------------------------------[<]-
    Function: file_concat

    Synopsis: Copies the contents of src onto dest.  If dest does not exist,
    it is created.  Returns 0 if the concatenation operation succeeded, or
    -1 if some error occurred.
    ---------------------------------------------------------------------[>]-*/

int
file_concat (
    const char *src,
    const char *dest)
{
    FILE *inf, *outf;
    char *buffer;
    size_t chars_read;                  /*  Amount read from stream          */
    int  feedback = 0;

    ASSERT (src);
    ASSERT (dest);
    if (! src || ! dest) 
        return (-1);

#   if (defined (GATES_FILESYSTEM))
    if (system_devicename (src) || system_devicename (dest))
        return (-1);                    /*  Not allowed on device names      */
#   endif
    if ((inf = fopen (src, FOPEN_READ_BINARY)) == NULL)
        return (-1);                    /*  Input file not found             */

    if ((buffer = mem_alloc (SHRT_MAX)) == NULL)
        feedback = -1;                  /*  Insufficient memory for buffer   */
    else
      {
        if ((outf = fopen (dest, FOPEN_APPEND_BINARY)) == NULL)
          {
            mem_free (buffer);
            feedback = -1;              /*  Could not open output file       */
          }
        else 
          {
             while ((chars_read = fread (buffer, 1, SHRT_MAX, inf)) != 0)
                 if (fwrite (buffer, 1, chars_read, outf) != chars_read)
                   {
                     feedback = -1;
                     break;
                   }
             fclose (outf);
          }
        mem_free (buffer);
      }
    fclose (inf);
    return (feedback);
}


/*  ---------------------------------------------------------------------[<]-
    Function: file_rename

    Synopsis: Renames a file from oldname to newname.  Returns 0 if okay,
    or -1 if there was an error.  Does not overwrite existing files.
    ---------------------------------------------------------------------[>]-*/

int
file_rename (
    const char *oldname,
    const char *newname)
{
#   if (defined (GATES_FILESYSTEM))
    char *dos_newname;
    int   feedback;

    ASSERT (oldname);
    ASSERT (newname);

    if (system_devicename (oldname) || system_devicename (newname))
        return (-1);                    /*  Not allowed on device names      */

    dos_newname = mem_strdup (newname);
    strconvch (dos_newname, '/', '\\');
    feedback = rename (oldname, dos_newname);
    if (feedback && errno == EACCES)
      {
        /*  Under WinNT and Win95, a delete of a freshly-created file can
         *  sometimes fail with a permission error which passes after a
         *  short delay.  Ugly but it seems to work.
         */
        Sleep (200);
        feedback = rename (oldname, dos_newname);
      }
    mem_free (dos_newname);
    return (feedback);

#   else
    ASSERT (oldname);
    ASSERT (newname);

    return (rename (oldname, newname));
#   endif
}


/*  ---------------------------------------------------------------------[<]-
    Function: file_delete

    Synopsis: Deletes the specified file.  Returns 0 if okay, -1 in case of
    an error.
    ---------------------------------------------------------------------[>]-*/

int
file_delete (
    const char *filename)
{
#if (defined (__VMS__))
    ASSERT (filename);
    return (remove (filename));

#elif (defined (WIN32))
    int
        rc;

    ASSERT (filename);

#if (defined GATES_FILESYSTEM)
    if (system_devicename (filename))
        return (-1);                    /*  Not allowed on device names      */
#endif

    rc = !DeleteFileA (filename);
    if (rc && errno == EACCES)
      {
        /*  Under WinNT and Win95, a delete of a freshly-created file can
         *  sometimes fail with a permission error which passes after a
         *  short delay.  Ugly but it seems to work.
         */
        Sleep (200);
        rc = !DeleteFileA (filename);
      }
    return (rc);
#else

    ASSERT (filename);
    return (unlink (filename));
#endif
}


/*  ---------------------------------------------------------------------[<]-
    Function: file_exists

    Synopsis: Returns TRUE if the file exists, or FALSE if it does not.
    ---------------------------------------------------------------------[>]-*/

Bool
file_exists (
    const char *filename)
{
    ASSERT (filename);
    return (file_mode (filename) > 0);
}


/*  -------------------------------------------------------------------------
 *  file_mode -- internal
 *
 *  Returns the file mode for the specified file or directory name; returns
 *  0 if the specified file does not exist.
 */

static dbyte
file_mode (const char *filename)
{
#if (defined (WIN32))
    DWORD   dwfa;
    dbyte   mode;

    ASSERT (filename);

#if (defined GATES_FILESYSTEM)
    if (system_devicename (filename))
        return (0);                     /*  Not allowed on device names      */
#endif
        
    dwfa = GetFileAttributesA (filename);
    if (dwfa == 0xffffffff)
        return (0);

    mode = 0;
    if (dwfa & FILE_ATTRIBUTE_DIRECTORY)
        mode |= S_IFDIR;
    else
        mode |= S_IFREG;

    if (!(dwfa & FILE_ATTRIBUTE_HIDDEN))
        mode |= S_IREAD;

    if (!(dwfa & FILE_ATTRIBUTE_READONLY))
        mode |= S_IWRITE;

    if (is_exe_file (filename))
        mode |= S_IEXEC;

    return (mode);

#else
    static struct stat
        stat_buf;

    ASSERT (filename);

#   if (defined (GATES_FILESYSTEM))
    /*  Handle simple disk specifiers ourselves, since some compilers cannot
     *  do a 'stat' on these.
     */
    if ( filename [1] == ':'
    && ((filename [2] == '\\' && filename [3] == '\0')
    ||  (filename [2] == '/'  && filename [3] == '\0')
    ||  (filename [2] == '\0')))
        return (S_IFDIR | S_IREAD | S_IWRITE);
#   endif

    if (strnull (filename))
        return (0);
    else
    if (stat ((char *) filename, &stat_buf) == 0)
        return ((dbyte) stat_buf.st_mode);
    else
        return (0);
#endif
}


/*  ---------------------------------------------------------------------[<]-
    Function: file_stable

    Synopsis: Returns FALSE if the file appears to be open for writing by a
    process, TRUE if the file appears to have been fully closed.
    ---------------------------------------------------------------------[>]-*/
Bool
file_stable (
    const char *filename)
{
    Bool
        feedback = FALSE;
    struct stat
        stat_buf;
    time_t
        cur_time;

    ASSERT (filename);

    /*  Check if the file is at least 3 seconds old                          */
    cur_time = time (NULL);
    if (stat (filename, &stat_buf) == 0) {
        if (stat_buf.st_size > 0) {
            if (stat_buf.st_mtime <= (cur_time - 3)
            &&  stat_buf.st_atime <= (cur_time - 3))
                feedback = TRUE;
        }
    }
#if defined (__WINDOWS__)
    /*  On windows, some copy methods preserve ctime and atime !!            */
    if (feedback == FALSE
    || (stat_buf.st_mtime - cur_time) > 30
    || (stat_buf.st_atime - cur_time) > 30) {
        int
            handle;

        feedback = FALSE;
        handle = sopen (filename, _O_WRONLY, _SH_DENYRW, 0);
        if (handle != -1) {
            close (handle);
            feedback = TRUE;
        }
    }
#endif
    return (feedback);
}


/*  ---------------------------------------------------------------------[<]-
    Function: file_where

    Synopsis: Scans a user-specified path symbol for a specific file, and
    returns the fully-specified filename.  Also adds an extension if this
    is required.

    The mode argument can be one of: r, w, a, or s for read, write, append,
    or static.  The function tries to locate existing files somewhere on the
    path.  New files are always created in the current directory.  Static
    files are created in the first directory on the path.

    The path argument is only used when more is r, a, or s.  If the path is
    NULL or empty, it is ignored.  Otherwise, the path is translated as an
    environment variable, and cut into a list of directory names.  The path
    is cut up as follows:
    <TABLE>
        MS-DOS    directory names separated by ';'. ;; means current.
        OS/2      directory names separated by ';'. ;; means current.
        Unix      directory names separated by ':'. :: means current.
        VMS       directory names separated by ','. " ", means current.
        Other     single directory name.
    </TABLE>

    When the mode is 'r' or 'a', searches the current directory before
    considering the path value.  When the path cannot be translated, and is
    not null or empty, it is used as a literal value.

    The name argument is the filename with or without extension.  It will
    be prefixed by the path and suffixed by the extension, if required.

    The ext argument is a default or mandatory extension.  If ext starts
    with a dot, it is mandatory and always used.  Otherwise it is used only
    if the name does not already have an extension.  If ext is NULL or empty,
    it is ignored.

    The total length of a name including path, name, extension, and any
    delimiters is FILE_NAME_MAX.  Names are truncated if too long.  The
    maximum size of one directory component is FILE_DIR_MAX chars.

    All parameters are case-sensitive; the precise effect of this depends on
    the system.  On MS-DOS, filenames are always folded to uppercase, but the
    path must be supplied in uppercase correctly.  On UNIX, all parameters are
    case sensitive.  On VMS, path and filenames are folded into uppercase.

    Returns a pointer to a static character array containing the filename; if
    mode is 'r' and the file does not exist, returns NULL.  If the mode is
    'w', 'a', or 's', always returns a valid filename.

    Under VMS, all filenames are handled in POSIX mode, i.e. /disk/path/file
    instead of $disk:[path]file.
    ---------------------------------------------------------------------[>]-*/

char *
file_where (
    char mode,
    const char *path,
    const char *name,
    const char *ext)
{
    const char
        *pathptr;                       /*  End of directory in path         */
    char
        *curdir;

    ASSERT (name);
    if (ext != NULL && *ext)            /*  Append extension if not null     */
      {                                 /*    to get name + ext into         */
        if (ext [0] == '.')             /*    work_name.                     */
            fixed_extension (work_name, name, ext);
        else
            default_extension (work_name, name, ext);
      }
    else
        strcpy (work_name, name);
#if (NAMEFOLD == TRUE)
    strupc (work_name);                 /*  Fold to uppercase if needed      */
#endif

    if (path != NULL && *path)          /*  Get value of path, or NULL       */
      {
        pathptr = getenv (path);        /*  Translate path symbol            */
        if (pathptr == NULL)
            pathptr = path;             /*  If not found, use literally      */
#if (PATHFOLD == TRUE)                  /*  Fold to uppercase if necessary   */
        if (pathptr)
          {
            ASSERT (strlen (pathptr) < PATH_MAX);
            strcpy (path_name, pathptr);
            strupc (path_name);
            pathptr = path_name;        /*  Redirect to uppercase version    */
          }
#endif
      }
    else
        pathptr = NULL;

#if (defined (GATES_FILESYSTEM))
    /*  Normalise the path value by changing any slashes to backslashes      */
    if (pathptr)
      {
        if (pathptr != path_name)
          {
            strcpy (path_name, pathptr);
            pathptr = path_name;
          }
        strconvch (path_name, '/', '\\');
      }
#endif

    /*  Take care of 'w' and 's' options first                               */
    if (mode == 'w')                    /*  Create output file locally       */
        return (work_name);

    if (mode == 's')                    /*  Get specific directory name      */
      {
        if (fully_specified (work_name))
            strncpy (full_name, work_name, sizeof (full_name));
        else
        if (pathptr && file_is_directory (pathptr))
            build_next_path (full_name, pathptr, work_name);
        else
#if (defined (GATES_FILESYSTEM))
            build_next_path (full_name, ".\\", work_name);
#else
            build_next_path (full_name, "./", work_name);
#endif
        return (full_name);
      }

    /*  If file exists as defined, prefix with current directory if not an   */
    /*  absolute filename, then return the resulting filename                */
    if (file_exists (work_name))
      {
        if (fully_specified (work_name))
            strncpy (full_name, work_name, sizeof (full_name));
        else
          {
            curdir = get_curdir ();
            snprintf (full_name, sizeof (full_name), "%s%s", curdir, work_name);
            mem_free (curdir);
          }
#if (defined (GATES_FILESYSTEM))
        strconvch (full_name, '/', '\\');
#endif
        return (full_name);             /*  Then return path + name + ext    */
      }
    if (!pathptr)                       /*  Now we need a path               */
        return (NULL);                  /*   - if none defined, give up      */

    for (;;)                            /*  Try each path component          */
      {
        pathptr = build_next_path (full_name, pathptr, work_name);
        if (file_exists (full_name))
            return (full_name);         /*  Until we find one,               */

        if (*pathptr == '\0')           /*    or we come to the end of       */
          {                             /*    the path                       */
            if (mode == 'r')
                return (NULL);          /*  Input file was not found...      */
            else
                return (full_name);
          }
      }
}


/*  ---------------------------------------------------------------------[<]-
    Function: file_where_ext

    Synopsis: Scans a user-specified path symbol for a specific file, and
    returns the fully-specified filename.  Can also scan a series of file
    extensions while looking for the file in the path.  The extensions are
    scanned in each directory in the path prior to moving on to the next
    directory.

    The mode argument can be one of: r, w, a, or s for read, write, append,
    or static.  The function tries to locate existing files somewhere on the
    path.  New files are always created in the current directory.  Static
    files are created in the first directory on the path.

    The path argument is only used when mode is r, a, or s.  If the path is
    NULL or empty, it is ignored.  Otherwise, the path is translated as an
    environment variable.  If the path cannot be translated, it is used as
    a literal value.  The path is then cut into a list of directory names,
    as follows:
    <TABLE>
        MS-DOS    directory names separated by ';'. ;; means current.
        OS/2      directory names separated by ';'. ;; means current.
        Unix      directory names separated by ':'. :: means current.
        VMS       directory names separated by ','. " ", means current.
        Other     single directory name.
    </TABLE>

    When the mode is 'r' or 'a', it searches the current directory before
    considering the path value.

    The name argument is the filename with or without extension.  It will
    be prefixed by the path and suffixed by the extension, if required.

    The ext argument is an array of default or mandatory extensions.
    If the extension starts with a dot, it is mandatory and will override
    any existing extension.  Otherwise it is used only if the name does
    not already have an extension.  The filename will be tried as supplied
    if ext is NULL, or if it has an extension, and one or more of the
    entries is a default extension.  The first extension, if any, is
    always used in 'w' mode and 's' mode.  The last path component and
    extension will be used in 'a' mode, if either is supplied.

    The total length of a name including path, name, extension, and any
    delimiters is FILE_NAME_MAX.  Names are truncated if too long.  The
    maximum size of one directory component is FILE_DIR_MAX chars.

    All parameters are case-sensitive; the precise effect of this depends on
    the system.  On MS-DOS, filenames are always folded to uppercase, but the
    path must be supplied in uppercase correctly.  On UNIX, all parameters are
    case sensitive.  On VMS, path and filenames are folded into uppercase.

    Returns a pointer to a static character array containing the filename; if
    mode is 'r' and the file does not exist, returns NULL.  If the mode is
    'w', 'a', or 's', always returns a valid filename.

    Under VMS, all filenames are handled in POSIX mode, i.e. /disk/path/file
    instead of $disk:[path]file.
    ---------------------------------------------------------------------[>]-*/

char *
file_where_ext (
    char mode,
    const char *path,
    const char *name,
    const char **ext)
{
    const char
        *pathptr;                       /*  End of directory in path         */
    const char
        **extptr;                       /*  Pointer to next extension        */
    char
        *curdir;
    Bool
        search_curdir = TRUE;           /*  Look in current directory?       */

    ASSERT (name);
    if (!name)
        return NULL;

    if (path != NULL && *path)          /*  Get value of path, or NULL       */
      {
        pathptr = getenv (path);        /*  Translate path symbol            */
        if (pathptr == NULL)
          {
            pathptr = path;             /*  If not found, use literally      */
            search_curdir = FALSE;      /*  Path now takes priority          */
          }
#if (PATHFOLD == TRUE)                  /*  Fold to uppercase if necessary   */
        if (pathptr)
          {
            ASSERT (strlen (pathptr) < PATH_MAX);
            strcpy (path_name, pathptr);
            strupc (path_name);
            pathptr = path_name;        /*  Redirect to uppercase version    */
          }
#endif
      }
    else
        pathptr = NULL;

#if (defined (GATES_FILESYSTEM))
    /*  Normalise the path value by changing any slashes to backslashes      */
    if (pathptr)
      {
        if (pathptr != path_name)
          {
            strcpy (path_name, pathptr);
            pathptr = path_name;
          }
        strconvch (path_name, '/', '\\');
      }
#endif

    /*  Take care of 'w' and 's' options first                               */
    if (mode == 'w')                    /*  Create output file locally       */
      {
        if (ext != NULL && ext [0] != NULL)
            add_extension (work_name, name, ext [0]);
        else
            strcpy (work_name, name);
#if (NAMEFOLD == TRUE)
        strupc (work_name);             /*  Fold to uppercase if needed      */
#endif

        return (work_name);
      }

    if (mode == 's')                    /*  Get specific directory name      */
      {
        if (ext != NULL && ext [0] != NULL)
            add_extension (work_name, name, ext [0]);
        else
            strcpy (work_name, name);
#if (NAMEFOLD == TRUE)
        strupc (work_name);             /*  Fold to uppercase if needed      */
#endif

        if (fully_specified (work_name))
            strcpy (full_name, work_name);
        else
        if (pathptr && file_is_directory (pathptr))
            build_next_path (full_name, pathptr, work_name);
        else
#if (defined (GATES_FILESYSTEM))
            build_next_path (full_name, ".\\", work_name);
#else
            build_next_path (full_name, "./", work_name);
#endif
        return (full_name);
      }

    /*  If file exists as defined (with one of the extensions), prefix with  */
    /*  current directory if not an absolute filename, then return the       */
    /*  resulting filename                                                   */
    if (search_curdir)
      {
        extptr = ext;
        do
          {
            if (extptr != NULL && *extptr != NULL)
                add_extension (work_name, name, *extptr);
            else
                strcpy (work_name, name);
#if (NAMEFOLD == TRUE)
            strupc (work_name);         /*  Fold to uppercase if needed      */
#endif

            if (file_exists (work_name))
              {
                if (fully_specified (work_name))
                    strcpy (full_name, work_name);
                else
                  {
                    curdir = get_curdir ();
                    snprintf (full_name, sizeof (full_name), 
                                         "%s%s", curdir, work_name);
                    mem_free (curdir);
                  }
#if (defined (GATES_FILESYSTEM))
                strconvch (full_name, '/', '\\');
#endif
                return (full_name);     /*  Then return path + name + ext    */
              }

            if (extptr)
                extptr++;
          }
         while (extptr != NULL && *extptr != NULL);
      }

    if (!pathptr)                       /*  We need a path to look further   */
        return (NULL);                  /*   - if none defined, give up      */

    for (;;)                            /*  Try each path component          */
      {                                 /*   - and extension within that     */
        const char *savedptr = pathptr;
        extptr = ext;

        do
          {
            const char *extension = NULL;

            if (extptr != NULL && *extptr != NULL)
                extension = *extptr;

            pathptr = build_next_path_ext (full_name, savedptr,
                                           name,      extension);

            if (file_exists (full_name))
                return (full_name);     /*  Until we find one that matches   */

            if (extptr)
                extptr++;
          }
        while (extptr != NULL && *extptr != NULL);

        if (*pathptr == '\0')           /*    or we come to the end of       */
          {                             /*    the path                       */
            if (mode == 'r')
                return (NULL);          /*  Input file was not found...      */
            else
                return (full_name);
          }
      }
}

/*  -------------------------------------------------------------------------
 *  fully_specified -- internal
 *
 */

static Bool
fully_specified (const char *filename)
{
#if (defined (GATES_FILESYSTEM))  \
    /*  Under MSDOS we have a full path if we have any of:
     *     /directory/directory/filename
     *     D:/directory/directory/filename
     *     the variations of those with backslashes.
     */
    if (filename [0] == '\\'   || filename [0] == '/' ||
       (isalpha (filename [0]) && filename [1] == ':' &&
       (filename [2] == '\\'   || filename [2] == '/')))
#else
    /*  Under UNIX, VMS, or OS/2, we have a full path if the path starts
     *  with the directory marker
     */
    if (filename [0] == PATHEND)
#endif
        return (TRUE);
    else
        return (FALSE);
}


/*  -------------------------------------------------------------------------
 *  build_next_path -- internal
 *
 */

static char *
build_next_path (char *dest, const char *path, const char *name)
{
    int
        length;                         /*  length of directory name         */
    char
        *pathptr = (char *) path;

    if (path)
      {
        length = strcspn (path, PATHSEP);
        strncpy (dest, path, length);
        pathptr += length;              /*  Bump past path delimiter         */
        if (*pathptr)                   /*    unless we are at the end       */
            pathptr++;                  /*    of the path                    */

        if ((length)
        && (dest [length - 1] != PATHEND))
            dest [length++] = PATHEND;  /*  Add path-to-filename delimiter   */

        dest [length] = '\0';
        strcat (dest, name);
      }
    else
        strcpy (dest, name);

    return (pathptr);
}

/*  -------------------------------------------------------------------------
 *  build_next_path_ext -- internal
 *
 */

static char *
build_next_path_ext (char *dest, const char *path,
                     const char *name, const char *ext)
{
    int
        length;                         /*  length of directory name         */
    char
        *pathptr = (char *) path;

    if (path)
      {
        length = strcspn (path, PATHSEP);
        strncpy (dest, path, length);
        pathptr += length;              /*  Bump past path delimiter         */
        if (*pathptr)                   /*    unless we are at the end       */
            pathptr++;                  /*    of the path                    */

        if ((length)
        && (dest [length - 1] != PATHEND))
            dest [length++] = PATHEND;  /*  Add path-to-filename delimiter   */

        dest [length] = '\0';
        strcat (dest, name);
        add_extension (dest, dest, ext);
      }
    else
      {
        if (ext)
            add_extension (dest, name, ext);
        else
            strcpy (dest, name);
      }

    return (pathptr);
}


/*  ---------------------------------------------------------------------[<]-
    Function: file_cycle

    Synopsis: Cycles the file: if the file already exists, renames the
    existing file.  This function tries to rename the file using the date
    of creation of the file; if this fails because an existing file had the
    same name, generates a guaranteed unique file name.  Returns TRUE if
    the cycle operation succeeded, or FALSE if it failed (e.g. due to a
    protection problem).  The how argument must be one of:
    <TABLE>
    CYCLE_ALWAYS        Cycle file unconditionally
    CYCLE_HOURLY        Cycle file if hour has changed
    CYCLE_DAILY         Cycle file if day has changed
    CYCLE_WEEKLY        Cycle file if week has changed
    CYCLE_MONTHLY       Cycle file if month has changed
    CYCLE_NEVER         Don't cycle the file
    </TABLE>
    ---------------------------------------------------------------------[>]-*/

Bool
file_cycle (
    const char *filename,
    int how)
{
    long
        file_date;                      /*  Datestamp of file                */
    char
        *point,
        *insert_at;                     /*  Where we start messing name      */
    int
        unique_nbr;                     /*  To generate a unique name        */

    ASSERT (filename);

    /*  If no cycling needed, do nothing                                     */
    if (!file_cycle_needed (filename, how))
        return (TRUE);                  /*  No errors, nothing in fact       */

    file_date = timer_to_date (get_file_time (filename));
    strcpy (full_name, filename);
    point = strrchr (full_name, '.');
    if (point)
      {
        strcpy (work_name, point);      /*  Save extension, if any           */
        *point = '\0';                  /*    and truncate original name     */
      }
    else
        strclr (work_name);

    /*  We leave up to 2 original letters of the filename, then stick-in     */
    /*  the 6-digit timestamp.                                               */
    insert_at = strrchr (full_name, '/');
#if (defined (GATES_FILESYSTEM))
    if (!insert_at)
        insert_at = strrchr (full_name, '\\');
#endif
    if (insert_at)
        insert_at++;                    /*  Bump past slash, if found        */
    else
        insert_at = full_name;

    if (*insert_at)                     /*  Bump insert_at twice, to leave   */
        insert_at++;                    /*    up to 2 letters before we      */
    if (*insert_at)                     /*    stick-in the date stamp        */
        insert_at++;

    /*  Format new name for file and make sure it does not already exist     */
    sprintf (insert_at, "%06d", (int) (file_date % 1000000L));
    strcat  (insert_at, work_name);
    if (file_exists (full_name))
      {
        point = strrchr (full_name, '.') + 1;
        for (unique_nbr = 0; unique_nbr < 1000; unique_nbr++)
          {
            sprintf (point, "%03d", unique_nbr);
            if (!file_exists (full_name))
                break;
          }
      }
    if (file_exists (full_name))
        return (FALSE);                 /*  We give up!                      */

    if (file_rename (filename, full_name))
        return (FALSE);                 /*  No permission                    */
    else
        return (TRUE);                  /*  Okay, it worked                  */
}


/*  ---------------------------------------------------------------------[<]-
    Function: file_cycle_needed

    Synopsis: Checks whether the file should be cycled or not.  Returns
    TRUE if the file needs to be cycled, FALSE if not.  The how argument
    must be one of:
    <TABLE>
    CYCLE_ALWAYS        Cycle file unconditionally
    CYCLE_HOURLY        Cycle file if hour has changed
    CYCLE_DAILY         Cycle file if day has changed
    CYCLE_WEEKLY        Cycle file if week has changed
    CYCLE_MONTHLY       Cycle file if month has changed
    CYCLE_NEVER         Don't cycle the file
    </TABLE>
    If the specified file does not exist or is not accessible, returns
    FALSE.
    ---------------------------------------------------------------------[>]-*/

Bool
file_cycle_needed (
    const char *filename,
    int how)
{
    long
        curr_time,                      /*  Current time                     */
        curr_date,                      /*  Current date                     */
        file_date,                      /*  Timestamp of file                */
        file_time;                      /*  Datestamp of file                */
    Bool
        cycle;                          /*  Do we want to cycle the file?    */

    ASSERT (filename);
    if (!file_exists (filename))        /*  Not found - nothing more to do   */
        return (FALSE);

    file_time = timer_to_time (get_file_time (filename));
    file_date = timer_to_date (get_file_time (filename));
    get_date_time_now (&curr_date, &curr_time);

    switch (how)
      {
        case CYCLE_ALWAYS:
            cycle = TRUE;
            break;
        case CYCLE_HOURLY:
            cycle = GET_HOUR (file_time) != GET_HOUR (curr_time);
            break;
        case CYCLE_DAILY:
            cycle = GET_DAY (file_date) != GET_DAY (curr_date);
            break;
        case CYCLE_WEEKLY:
            cycle = week_of_year (file_date) != week_of_year (curr_date);
            break;
        case CYCLE_MONTHLY:
            cycle = GET_MONTH (file_date) != GET_MONTH (curr_date);
            break;
        case CYCLE_NEVER:
            cycle = FALSE;
            break;
        default:
            cycle = FALSE;
      }
    return (cycle);
}


/*  ---------------------------------------------------------------------[<]-
    Function: file_has_changed

    Synopsis: Returns TRUE if the file has changed since it was last read.
    The calling program must supply the date and time of the file as it
    was read.  If the file is not present or accessible, returns FALSE.
    ---------------------------------------------------------------------[>]-*/

Bool
file_has_changed (
    const char *filename,
    long old_date,
    long old_time)
{
    long
        file_date,                      /*  Timestamp of file                */
        file_time;                      /*  Datestamp of file                */

    ASSERT (filename);
    if (!file_exists (filename))        /*  Not found - nothing more to do   */
        return (FALSE);

    file_time = timer_to_time (get_file_time (filename));
    file_date = timer_to_date (get_file_time (filename));
    if (file_date  > old_date
    || (file_date == old_date && file_time > old_time))
        return (TRUE);
    else
        return (FALSE);
}


/*  ---------------------------------------------------------------------[<]-
    Function: safe_to_extend

    Synopsis: Handles system-specific case of extending a file that may not
    be in a valid state for such an operation. Returns TRUE if the extend
    can go ahead; returns FALSE if the extend cannot be permitted.

    Under MS-DOS and Windows, if the last byte in the file is Ctrl-Z (26)
    the file is truncated by 1 position to remove this character.
    ---------------------------------------------------------------------[>]-*/

Bool
safe_to_extend (
    const char *filename)
{
#if (defined (GATES_FILESYSTEM))
    int  handle;                        /*  Opened file handle               */
    char endoffile;                     /*  Last character in file           */

    ASSERT (filename);

    if (system_devicename (filename))
        return (FALSE);                 /*  Not allowed on device names      */

    handle = open (filename, O_RDWR + O_BINARY, S_IREAD | S_IWRITE);
    if (handle)                         /*  If not found, ignore             */
      {
        lseek (handle, -1, SEEK_END);
        read  (handle, &endoffile, 1);
        if (endoffile == 26)
            chsize (handle, filelength (handle) - 1);

        close (handle);
      }
#endif
    return (TRUE);
}


/*  ---------------------------------------------------------------------[<]-
    Function: default_extension

    Synopsis: Copies src to dest and adds ext if necessary.  Returns dest.
    Dest must be large enough for a fully-formatted filename; define it as
    char [FILE_NAME_MAX + 1].  The ext argument can start with or without
    a dot. If ext is null or empty, does nothing.
    ---------------------------------------------------------------------[>]-*/

char *
default_extension (
    char *dest,
    const char *src,
    const char *ext)
{
    int len, i;
    char *ptr;

    ASSERT (dest);
    ASSERT (src);

    if (dest != src)                    /*  Copy src to dest if not same     */
        strcpy (dest, src);

    if (ext != NULL && *ext != 0)
      {
        len = strlen (dest);
        for (i = len - 1, ptr = dest + i; i >= 0; i--, ptr--)
            if (*ptr == '\\' || *ptr == '/' || *ptr == '.')
                break;

        if (i < 0 || *ptr != '.')
          {
            if (*ext != '.')
              {
                dest [len++] = '.';
                dest [len] = '\0';
              }
            strcat (dest + len, ext);
          }
      }
    return (dest);
}


/*  ---------------------------------------------------------------------[<]-
    Function: fixed_extension

    Synopsis: Copies src to dest and enforces ext extension.  Returns dest.
    Dest must be large enough for a fully-formatted filename; define it as
    char [FILE_NAME_MAX + 1].  The ext argument can start with or without
    a dot.  If ext is null or empty, does nothing.
    ---------------------------------------------------------------------[>]-*/

char *
fixed_extension (
    char *dest,
    const char *src,
    const char *ext)
{
    ASSERT (dest);
    ASSERT (src);

    if (dest != src)                    /*  Copy src to dest if not same     */
        strcpy (dest, src);

    strip_extension (dest);
    return (default_extension (dest, dest, ext));
}


/*  ---------------------------------------------------------------------[<]-
    Function: strip_extension

    Synopsis: Removes dot and extension from the name, if any was present.
    If the name contained multiple extensions, removes the last one only.
    Returns name.
    ---------------------------------------------------------------------[>]-*/

char *
strip_extension (
    char *name)
{
    char *dot, *slash;

    ASSERT (name);

    dot   = strrchr (name, '.');        /*  Find dot in name, if any         */
    slash = strrchr (name, '\\');       /*  Find last slash (DOS or Unix)    */
    if (slash == NULL)
        slash = strrchr (name, '/');
    if (dot > slash)
        *dot = 0;                       /*  If we had a dot, truncate name   */

    return (name);
}


/*  ---------------------------------------------------------------------[<]-
    Function: add_extension

    Synopsis: Copies src to dest and adds ext if necessary.  If extension
    starts with "." then it will be added, in place of any existing extension.
    If extension does not start with "." it will be added only if there is
    no existing extension.  If ext is null or empty, just copies src into
    dest if required.
    Dest must be large enough for a fully-formatted filename; define it as
    char [FILE_NAME_MAX + 1].
    ---------------------------------------------------------------------[>]-*/

char *
add_extension (
    char *dest,
    const char *src,
    const char *ext)
{
    char
        *result;

    ASSERT (dest);
    ASSERT (src);
    if (!src || !dest)
        return (NULL);

    if (!ext || *ext == '\0')
      {
        if (dest != src)                /*  Copy src to dest if not same     */
            strcpy (dest, src);

        result = dest;
      }
    else
        if (*ext == '.')
            result = fixed_extension (dest, src, ext);
        else
            result = default_extension (dest, src, ext);

    return (result);
}


/*  ---------------------------------------------------------------------[<]-
    Function: strip_file_path

    Synopsis: Removes the leading path from the filename, if any path was
    present.  Returns name.  The path can be specified using the local
    operating system syntax; under MS-DOS, / and \ are interchangeable.
    ---------------------------------------------------------------------[>]-*/

char
*strip_file_path (
    char *name)
{
    char *path_end;

    ASSERT (name);

    path_end = strrchr (name, PATHEND); /*  Find end of path, if any         */
#if (defined (GATES_FILESYSTEM))
    if (path_end == NULL)
        path_end = strrchr (name, '/');
#endif
    if (path_end != NULL)
        memmove (name, path_end + 1, strlen (path_end));
    return (name);
}


/*  ---------------------------------------------------------------------[<]-
    Function: strip_file_name

    Synopsis: Returns the path for a fully-qualified filename. The returned 
    string is held in a static area that should be copied directly after 
    calling this function.  The returned path does not end in '/' unless 
    that is the entire path.  If the supplied name contains no path, the 
    returned path is ".".
    ---------------------------------------------------------------------[>]-*/

char
*strip_file_name (
    char *name)
{
    char *path_end;

    ASSERT (name);
    ASSERT (strlen (name) <= LINE_MAX);

    strcpy (work_name, name);
    path_end = strrchr (work_name, PATHEND); /*  Find end of path, if any    */
#if (defined (GATES_FILESYSTEM))
    if (path_end == NULL)
        path_end = strrchr (work_name, '/');
#endif
    if (path_end == NULL)
        return (".");

    path_end [1] = '\0';
    return work_name;
}


/*  ---------------------------------------------------------------------[<]-
    Function: get_new_filename

    Synopsis: Appends a numeric suffix (_001, _002,...) to the filename until
    it is unique.  Returns a freshly-allocated string containing the new
    filename.
    ---------------------------------------------------------------------[>]-*/

char *
get_new_filename (
    const char *filename)
{
    char
        suffix [8],
        *new_name;
    int
        counter;

    for (counter = 0; ; counter++)
      {
        sprintf (suffix, "_%03d", counter);
        new_name = xstrcpy (NULL, filename, suffix, NULL);
        if (!file_exists (new_name))
            return (new_name);
        else
            mem_free (new_name);
      }
}


/*  ---------------------------------------------------------------------[<]-
    Function: file_is_readable

    Synopsis: Returns TRUE if the current process can read the specified
    file or directory.  The filename may end in a slash (/ or \) only if
    it is a directory.
    ---------------------------------------------------------------------[>]-*/

Bool
file_is_readable (
    const char *filename)
{
    ASSERT (filename);
    if (file_is_directory (filename))
        return ((file_mode (clean_path (filename)) & S_IREAD) != 0);
    else
    if (strlast (filename) == '/')
        return (FALSE);
    else
        return ((file_mode (filename) & S_IREAD) != 0);
}


/*  ---------------------------------------------------------------------[<]-
    Function: file_is_writeable

    Synopsis: Returns TRUE if the current process can write the specified
    file or directory.  The filename may end in a slash (/ or \) only if
    it is a directory.
    ---------------------------------------------------------------------[>]-*/

Bool
file_is_writeable (
    const char *filename)
{
    ASSERT (filename);

    if (file_is_directory (filename))
        return ((file_mode (clean_path (filename)) & S_IWRITE) != 0);
    else
    if (strlast (filename) == '/')
        return (FALSE);
    else
        return ((file_mode (filename) & S_IWRITE) != 0);
}


/*  ---------------------------------------------------------------------[<]-
    Function: file_is_executable

    Synopsis: Returns TRUE if the current process can execute the specified
    file.  Directories are _not_ considered to be executable.

    Under DOS, Windows, appends ".com", ".exe", and ".bat" to the filename,
    in that order, to build a possible executable filename.  If this fails,
    opens the file (if it exists) and examines the first few bytes of the
    file: if these are "#!", or '/'*! or "MZ" then the file is assumed to
    be executable.  #! is a standard mechanism under Unix for indicating
    executable files.  Note that process_create() uses a compatible
    mechanism to launch the correct interpreter for such 'executable'
    scripts.  NOTE: '/'*! is provided for REXX.    [XXX]

    Under OS/2 appends ".exe" and ".cmd" to the filename, in that order,
    to be a possible executable filename.  If this fails, it opens the
    file (if it exists) and examines the first few bytes of the file: if
    these are "#!" then the file is assumed to be executable.  NOTE:
    REXX scripts MUST be in files named script.cmd in order to be found.
    BAT files are not considered, nor are COM files, since at present
    process_create does not support launching DOS processes.

    Under VMS, appends .exe and .com, in that order to build a possible
    executable filename.

    Does not search the PATH symbol; the filename must be specified with a
    path if necessary.
    ---------------------------------------------------------------------[>]-*/

Bool
file_is_executable (
    const char *filename)
{
#if (defined (__UNIX__))
    ASSERT (filename);

    return ((file_mode (filename) & S_IEXEC) != 0
         && (file_mode (filename) & S_IFDIR) == 0);

#elif (defined (GATES_FILESYSTEM))
    Bool
        executable;                     /*  Return code                      */
    FILE
        *stream;                        /*  Opened file stream               */
    char
        input_char = 0,                 /*  First and second bytes of file   */
        *extension;                     /*  File extension, if any           */

    ASSERT (filename);

    /*  Find file extension; if not found, set extension to empty string     */
    extension = strrchr (filename, '.');
    if (extension == NULL
    ||  strchr (extension, '/')         /*  If last '.' is part of the path  */
    ||  strchr (extension, '\\'))       /*  then the filename has no ext.    */
        extension = "";

    /*  Windows: If extension is .exe/.com/.bat, the file is an executable   */
    /*  OS/2:    If the extension is .exe/.cmd, the file is an executable    */
#if (defined ( __OS2__))
    if (lexcmp (extension, ".exe") == 0
    ||  lexcmp (extension, ".cmd") == 0)
#else /* DOS, WINDOWS */
    if (lexcmp (extension, ".com") == 0
    ||  lexcmp (extension, ".exe") == 0
    ||  lexcmp (extension, ".bat") == 0)
#endif
        executable = file_exists (filename);
    else
    /*  Windows: If the extension is empty, try .com, .exe, .bat             */
    /*  OS/2:    If the extension is empty, try .exe, .cmd                   */
    if (strnull (extension)
#if (defined( __OS2__))
    && (file_exists (default_extension (work_name, filename, "exe"))
    ||  file_exists (default_extension (work_name, filename, "cmd"))))
#else /* DOS, WINDOWS */
    && (file_exists (default_extension (work_name, filename, "com"))
    ||  file_exists (default_extension (work_name, filename, "exe"))
    ||  file_exists (default_extension (work_name, filename, "bat"))))
#endif
        executable = TRUE;              /*  Executable file found            */
    else
      {
        /*  Look for magic header at start of file                           */
        stream = file_open (filename, 'r');
        if (stream)
          {
            input_char = fgetc (stream);
            executable = ((input_char == '#' && fgetc (stream) == '!')
#   if (defined (__MSDOS__))
                       || (input_char == '/' && fgetc (stream) == '*'
                                             && fgetc (stream) == '!')
                       || (input_char == 'M' && fgetc (stream) == 'Z')
#   endif
                       );
            file_close (stream);
          }
        else
            executable = FALSE;
      }
    return (executable);

#elif (defined (__VMS__))
    Bool
        executable;                     /*  Return code                      */
    char
        *extension;                     /*  File extension, if any           */

    ASSERT (filename);

    /*  Find file extension, if any                                          */
    extension = strrchr (filename, '.');
    if ((file_mode (filename) & S_IEXEC) != 0)
        executable = TRUE;
    else
    /*  If the extension is empty, try .exe and .com                         */
    if (!extension)
      {
        default_extension (work_name, filename, "exe");
        if ((file_mode (work_name) & S_IEXEC) != 0)
            executable = TRUE;
        else
          {
            default_extension (work_name, filename, "com");
            if ((file_mode (work_name) & S_IEXEC) != 0)
                executable = TRUE;
            else
                executable = FALSE;
          }
      }
    else
        executable = FALSE;

    return (executable);

#else
    return (FALSE);                     /*  Not supported on this system     */
#endif
}


/*  ---------------------------------------------------------------------[<]-
    Function: file_is_program

    Synopsis: Returns TRUE if the specified filename is an executable
    program on the PATH.
    Under DOS, and Windows, will try appending ".exe", ".com" to the file,
    in that order, to build an executable filename, if the file does not 
    have an extension, then searches the PATH definition for the executable 
    filename.  Under OS/2, appends ".exe" to the file to build an executable 
    filename, then searches the PATH definition for the executable filename.  
    If the filename already has a path specifier, will not use the PATH 
    definition.  Under VMS, appends "exe" and "com" to the file, in that 
    order, to build an executable filename.  Searches the PATH if necessary.
    ---------------------------------------------------------------------[>]-*/

Bool
file_is_program (
    const char *filename)
{
    Bool
        executable = FALSE;             /*  Return code                      */

#if (defined (__UNIX__))
    char
        *found_file;

    ASSERT (filename);

    found_file = file_where ('r', "PATH", filename, "");
    if (found_file && (file_mode (found_file) & S_IEXEC))
        executable = TRUE;              /*  Executable file found            */

#elif (defined (__VMS__))
    char
        *found_file;

    ASSERT (filename);

    found_file = file_where ('r', "PATH", filename, "");
    if (!found_file)
        found_file = file_where ('r', "PATH", filename, ".exe");
    if (!found_file)
        found_file = file_where ('r', "PATH", filename, ".com");

    if (found_file && (file_mode (found_file) & S_IEXEC))
        executable = TRUE;              /*  Executable file found            */

#elif (defined (GATES_FILESYSTEM))
    char
        *path;                          /*  What path do we search?          */
    char 
        *extension = NULL;              /*  Existing extension, if any       */

    ASSERT (filename);
    /*  If the filename already contains a path, don't look at PATH          */
    if (strchr (filename, '/') || strchr (filename, '\\'))
        path = NULL;
    else
        path = "PATH";

    /*  Find the current file extension, if any, to use.                     */
    extension = strrchr (filename, '.');
    if (extension &&  (strchr (extension, '/') || strchr (extension, '\\')))
        extension = NULL;                /* No extension after last path sep */

#   if (defined (__MSDOS__))
    if (extension) 
      {
        if ((lexcmp (extension, ".com") == 0
        ||   lexcmp (extension, ".exe") == 0
        ||   lexcmp (extension, ".bat") == 0)
        && file_where('r', path, filename, ""))
             executable = TRUE;             /*  It is an executable file     */
      }
    else
      {
        if (file_where ('r', path, filename, ".exe")
        ||  file_where ('r', path, filename, ".com")
        ||  file_where ('r', path, filename, ".bat"))
            executable = TRUE;              /*  Executable file found        */
      }
#   else /* OS/2 */
    if (extension)
      {
        if (lexcmp (extension, ".exe") == 0
        &&  file_where ('r', path, filename, ""))
            executable = TRUE;              /*  It is an executable file     */

      }
    else
      {
        if (file_where ('r', path, filename, ".exe"))
             executable = TRUE;
      }
#   endif
#endif

    return (executable);
}


/*  ---------------------------------------------------------------------[<]-
    Function: file_is_directory

    Synopsis: Returns TRUE if the specified file is a directory.  The
    filename may end in a slash (/ or \).  Under MS-DOS/OS2/Windows, a
    directory name may consist solely of a disk-drive specifier.  Under
    VMS the directory may optionally take the extension '.dir'.
    ---------------------------------------------------------------------[>]-*/

Bool
file_is_directory (
    const char *filename)
{
    char
        *dir_name;
    Bool
        rc;

    ASSERT (filename);

    dir_name = mem_strdup (clean_path (filename));
#if (defined (__VMS__))
    if (!file_exists (dir_name))
        default_extension (dir_name, dir_name, "dir");
#endif
    rc = (file_mode (dir_name) & S_IFDIR) != 0;
    mem_free (dir_name);
    return (rc);
}


/*  ---------------------------------------------------------------------[<]-
    Function: file_is_legal

    Synopsis: Checks whether the specified file is 'legal', which is a
    system-dependent definition.  Under 32-bit Windows, a legal file is one
    who's name is not a shortened 8.3 version of a long name.  This can be
    used to bypass filename-based security schemes.  On other systems, the
    notion of 'illegal' is not defined.  Returns TRUE if the file exists
    and is legal.  Returns FALSE otherwise.
    ---------------------------------------------------------------------[>]-*/

Bool
file_is_legal (
    const char *arg_filename)
{
#if (defined (WIN32))
    static WIN32_FIND_DATAA
        found;
    HANDLE
        handle;
    char
        *filename,                      /*  Our copy of arg_filename         */
        *slash,                         /*  Position of '\' in filename      */
        *component;                     /*  Component to compare             */
    Bool
        feedback;                       /*  Function feedback                */

    /*  For each path component of the filename, check that the long form
     *  of the name is the same as the short form.  We scan backwards
     *  from the end of the filename, get the full pathname, and compare
     *  the last component each time:
     *
     *      aaa\bbb\ccc\name.ext    name.ext
     *      aaa\bbb\ccc             ccc
     *      aaa\bbb                 bbb
     *      aaa                     aaa
     */

#if (defined GATES_FILESYSTEM)
    if (system_devicename (arg_filename))
        return (FALSE);                 /*  Not allowed on device names      */
#endif

    filename = mem_strdup (arg_filename);
    feedback = TRUE;                    /*  Assume we match everything       */
    strconvch (filename, '/', '\\');
    if (strlast (filename) == '\\')
        strlast (filename) = '\0';      /*  Drop any trailing slash          */

    do
      {
        slash     = strrchr (filename, '\\');
        component = slash? slash + 1: filename;
        handle    = FindFirstFileA (filename, &found);

        if (handle != INVALID_HANDLE_VALUE
        &&  strneq (component, ".")
        &&  lexcmp (component, found.cFileName))
          {
            feedback = FALSE;
            break;
          }
        FindClose (handle);
        if (slash)
          {
            *slash = '\0';              /*  Cut filename at slash            */
            if (filename [1] == ':'
            &&  filename [2] == '\0')   /*  We're at a disk specifier        */
                break;                  /*    which is okay by now           */
          }
      }
    while (slash && *filename);
    mem_free (filename);
    return (feedback);

#else
    return (TRUE);               /*  On other OSes, all filenames are legal  */
#endif
}


/*  ---------------------------------------------------------------------[<]-
    Function: file_exec_name

    Synopsis: If the specified filename is an executable program, formats
    a filename including any required extension and returns a static
    string with that value.  If the specified filename is not an executable
    program, returns NULL.  Under DOS, and Windows, appends ".com", ".exe",
    and ".bat" to the filename, in that order, to build a possible executable
    filename.  Under OS/2, appends ".exe", and ".cmd" to the filename, in
    that order, to build a possible executable filename.  If this fails,
    returns NULL.  Does not search the PATH symbol; the filename must be
    specified with a path if necessary.  The returned filename (if not NULL)
    points to a static string.
    ---------------------------------------------------------------------[>]-*/

char *
file_exec_name (
    const char *filename)
{
#if (defined (__UNIX__) || defined (__VMS__))
    ASSERT (filename);

    strcpy (exec_name, filename);

    if (file_mode (exec_name) & S_IEXEC)
        return (exec_name);
    else
        return (NULL);

#elif (defined (GATES_FILESYSTEM))
    char
        *extension;                     /*  File extension, if any           */

    ASSERT (filename);

    /*  Find file extension; if not found, set extension to empty string     */
    extension = strrchr (filename, '.');
    if (extension == NULL
    ||  strchr (extension, '/')         /*  If last '.' is part of the path  */
    ||  strchr (extension, '\\'))       /*  then the filename has no ext.    */
        extension = "";

    /*  Windows: If extension is .exe/.com/.bat, the file is an executable   */
    /*  OS/2:    If extension is .exe/.cmd, the file is executable           */
#   if (defined (__OS2__))
    if (lexcmp (extension, ".exe") == 0
    ||  lexcmp (extension, ".cmd") == 0
#   else /* DOS, WINDOWS */
    if (lexcmp (extension, ".com") == 0
    ||  lexcmp (extension, ".exe") == 0
    ||  lexcmp (extension, ".bat") == 0
#     if (defined (__MSDOS__))
    ||  is_exe_file (filename)
#     endif
#   endif
    )
      {
        strcpy (exec_name, filename);
        return (exec_name);
      }
    else
    /*  Windows: If the extension is empty, try .com, .exe, .bat             */
    /*  OS/2:    If the extension is empty, try .exe, .cmd                   */
    if (strnull (extension)
#   if (defined (__OS2__))
    && (file_exists (default_extension (exec_name, filename, "exe"))
    ||  file_exists (default_extension (exec_name, filename, "cmd"))))
#   else /* DOS, WINDOWS */
    && (file_exists (default_extension (exec_name, filename, "com"))
    ||  file_exists (default_extension (exec_name, filename, "exe"))
    ||  file_exists (default_extension (exec_name, filename, "bat"))))
#   endif
        return (exec_name);             /*  Executable file found            */
    else
        return (NULL);
#else
    return (NULL);                      /*  Not supported on this system     */
#endif
}


#if (defined (__MSDOS__))
/*  is_exe_file -- internal
 *
 *  Returns TRUE if the file corresponds to the criteria for an executable
 *  file under Windows.
 */

static Bool
is_exe_file (const char *filename)
{
    Bool
        executable;                     /*  Return code                      */
    FILE
        *stream;                        /*  Opened file stream               */

    stream = file_open (filename, 'r');
    if (stream)
      {
        executable = (fgetc (stream) == 'M' && fgetc (stream) == 'Z');
        file_close (stream);
      }
    else
        executable = FALSE;             /*  File not found                  */

    return (executable);
}

#endif


/*  ---------------------------------------------------------------------[<]-
    Function: get_file_size

    Synopsis: Returns the size, in bytes, of the specified file or directory.
    The size of a directory is not a portable concept.  If there is an error,
    returns -1.
    ---------------------------------------------------------------------[>]-*/

long
get_file_size (
    const char *filename)
{
    struct stat
        stat_buf;

    ASSERT (filename);

#   if (defined (GATES_FILESYSTEM))
    if (system_devicename (filename))
        return (-1);                    /*  Not allowed on device names      */
#   endif
    if (stat ((char *) filename, &stat_buf) == 0)
        return ((long) stat_buf.st_size);
    else
        return (-1);
}


/*  ---------------------------------------------------------------------[<]-
    Function: get_file_time

    Synopsis: Returns the modification time of the specified file or
    directory.  The returned time is suitable for feeding to localtime().
    ---------------------------------------------------------------------[>]-*/

time_t
get_file_time (
    const char *filename)
{
#if (defined (WIN32_NOT_IMPLEMENTED))
    /*  This code has been disactivated because it returns incorrect
        values depending on the seasonal clock change.
     */
    unsigned long thi,tlo;
    double  dthi,dtlo;
    double  secs_since_1601, secs_time_t;
    double  delta = 11644473600.;
    double  two_to_32 = 4294967296.;
    HANDLE  handle;
    FILETIME creation, last_access, last_write;

    handle = CreateFile (filename, GENERIC_READ, FILE_SHARE_WRITE,
                         NULL, OPEN_EXISTING, 0, 0);
    if (handle == INVALID_HANDLE_VALUE)
        return (0);
    GetFileTime (handle, &creation, &last_access, &last_write);
    CloseHandle (handle);
    thi = last_write.dwHighDateTime;
    tlo = last_write.dwLowDateTime;
    dthi = (double) thi;
    dtlo = (double) tlo;
    secs_since_1601 = (dthi * two_to_32 + dtlo) / 1.0e7;
    secs_time_t     = secs_since_1601 - delta;
    return ((time_t) secs_time_t);

#else
    struct stat
        stat_buf;

    ASSERT (filename);

#   if (defined (GATES_FILESYSTEM))
    if (system_devicename (filename))
        return (0);                     /*  Not allowed on device names      */
#   endif
    if (stat ((char *) filename, &stat_buf) == 0)
        return (stat_buf.st_mtime > 0? stat_buf.st_mtime: 0);
    else
        return (0);
#endif
}


/*  ---------------------------------------------------------------------[<]-
    Function: set_file_time

    Synopsis: Set the file time if you have write access on file.
              date is on format yyyymmdd and time is on format hhmmsscc.
              Return TRUE is file is updated.
    ---------------------------------------------------------------------[>]-*/

Bool
set_file_time (const char *filename, long date, long time)
{
    struct utimbuf
        file_time;
    time_t
        timer;
    Bool
        feedback = FALSE;

    timer = date_to_timer (date, time);

    if (timer > 0)
      {
        file_time.actime  = timer;
        file_time.modtime = timer;

        if (utime (filename, &file_time) != -1)
            feedback = TRUE;
      }
    return (feedback);
}


/*  ---------------------------------------------------------------------[<]-
    Function: get_file_lines

    Synopsis: Reads an entire file, and returns the number of lines in the
    file.  The file should be normal text.  Returns 0 if the file cannot be
    opened for reading.  May be a bit slow on large files.
    ---------------------------------------------------------------------[>]-*/

long
get_file_lines (
    const char *filename)
{
    long
        file_size;
    FILE
        *file_stream;
    int
        ch;

    ASSERT (filename);

    file_stream = file_open (filename, 'r');
    if (file_stream == NULL)
        return (0);

    file_size = 0;
    while ((ch = fgetc (file_stream)) != EOF)
        if (ch == '\n')
            file_size++;

    fclose (file_stream);
    return (file_size);
}


/*  ---------------------------------------------------------------------[<]-
    Function: file_slurp

    Synopsis: Reads an entire file, and returns a DESCR containing the file
    data.  The file is read as binary data.  The returned DESCR should be
    freed using the mem_free() call.   if the file is > 64K long, only the
    first 64K bytes are read into memory.  This is to stop really silly
    things from happening.  Returns NULL if the file cannot be found.
    Appends a null byte to the data in any case.
    ---------------------------------------------------------------------[>]-*/

DESCR *
file_slurp (
    const char *filename)
{
    return (file_load_data (filename, 65535UL));
}


static DESCR *
file_load_data (const char *filename, size_t limit)
{
    DESCR
        *buffer;
    long
        file_size;
    int
        rc;
    FILE
        *file_stream;

    ASSERT (filename);

    file_size = get_file_size (filename);
    if (file_size == -1)
        return (NULL);
    else
    if (limit && file_size > (long) limit)
        file_size = limit;

    buffer = mem_descr (NULL, (size_t) file_size + 1);
    if (buffer == NULL)
        return (NULL);

    buffer-> size = file_size;          /* caller should not be aware of the 
                                           extra byte */

    file_stream = fopen (filename, FOPEN_READ_BINARY);
    if (file_stream == NULL)
      {
        mem_free (buffer);
        return (NULL);
      }
    rc = fread (buffer-> data, (size_t) file_size, 1, file_stream);
    fclose (file_stream);
    if (rc != 1)
      {
        mem_free (buffer);
        return (NULL);
      }
    buffer-> data [(size_t) file_size] = '\0';
    return (buffer);
}


/*  ---------------------------------------------------------------------[<]-
    Function: file_slurpl

    Synopsis: Reads an entire file, and returns a DESCR containing the file
    data.  The file is read as binary data.  The returned DESCR should be
    freed using the mem_free() call.   Does not impose any limit on the size
    of the file (unlike file_slurp() which stops at 64K bytes).  Returns NULL
    if the file cannot be found.  Appends a null byte to the data in any case.
    ---------------------------------------------------------------------[>]-*/

DESCR *
file_slurpl (
    const char *filename)
{
    return (file_load_data (filename, 0));
}


/*  ---------------------------------------------------------------------[<]-
    Function: descr2file

    Synopsis: saves data in a DESCR to a file. The file is saved as binary
    data. Returns 0 on success, or negative error code on IO error.
    ---------------------------------------------------------------------[>]-*/
int
descr2file (
    const DESCR *descr,
    const char *filename)
{
    FILE
        *file_stream = NULL;
    int
        rc;

    if (!filename || !descr)
        return -1;

    file_stream = fopen (filename, FOPEN_WRITE_BINARY);
    if (file_stream == NULL)
        return -1;

    rc = fwrite (descr-> data, 1, descr-> size, file_stream);
    fclose (file_stream);
    if (rc != 1)
        return -1;

    return 0;
}


/*  ---------------------------------------------------------------------[<]-
    Function: file_set_eoln

    Synopsis: Formats any end-of-line sequences in the buffer according to
    the value of the add_cr argument.  If this is TRUE, all end-of-lines
    (LF or CRLF or LFCR) are represented by a CRLF sequence.  If FALSE, all
    end-of-lines are represented by LF by itself.  The target buffer must
    be large enough to accomodate the resulting line (twice the size of the
    source data).  Returns the size of the resulting data in the target
    buffer not counting the final trailing null byte.  The input data does
    not need to be null-terminated, but the output data is terminated with
    an extra null in any case.
    ---------------------------------------------------------------------[>]-*/

dbyte
file_set_eoln (char *dst, const char *src, dbyte src_size, Bool add_cr)
{
    char
        *srcptr,                        /*  Current character in src         */
        *dstptr,                        /*  Current character in dst         */
        *last;                          /*  Last character in src            */

    ASSERT (src);
    ASSERT (dst);

    srcptr = (char *) src;
    dstptr = dst;
    last   = (char *) src + src_size;

    while (*srcptr && srcptr < last)
      {
        if (*srcptr == '\n')
          {
            if (add_cr)
                *dstptr++ = '\r';
            *dstptr++ = '\n';
          }
        else
        if (*srcptr != '\r' && *srcptr != 26)
            *dstptr++ = *srcptr;
        srcptr++;
      }
    *dstptr = '\0';
    return ((dbyte) (dstptr - dst));
}


/*  ---------------------------------------------------------------------[<]-
    Function: get_tmp_file_name

    Synopsis: Get a temporary file name.  The filename is allocated as a
    fresh string, which the calling program must free when finished.  The
    calling program may provide a pointer to a qbyte sequence number that
    is used for building unique file names and updated with the last used
    value, or it may leave this argument NULL, in which case a random number
    is used internally.  Provide this argument only if you care about the
    order in which temporary files are created.
    ---------------------------------------------------------------------[>]-*/

char *
get_tmp_file_name (const char *path, qbyte *index, const char *ext)
{
    static qbyte
        file_number;                    /*  We generate unique file names    */
    char
        index_str [9],                  /*  Formatted 8-hex digit value      */
        *filename = NULL;

    if (index == NULL) {                /*  Use a random index if needed     */
        srand (micro_time ());
        file_number = rand () * 0xFFFFFFFF;
        index = &file_number;
     }
    do {
        mem_strfree (&filename);
        sprintf (index_str, "%08lX", (long) *index);
        if (path)
            filename = xstrcpy (NULL, path, "/", index_str, ".", ext, NULL);
        else
            filename = xstrcpy (NULL, index_str, ".", ext, NULL);
        (*index)++;
    }
    while (file_exists (filename));
    return (filename);
}


/*  ---------------------------------------------------------------------[<]-
    Function: file_fhredirect

    Synopsis: Duplicates the dest file handle to a safe location (saves a
    backup copy of it).  Then duplicates the source file handle into the dest.
    Returns the backup of the original dest, which can be used to undo the
    redirection later.  Returns -1 if there were errors.
    ---------------------------------------------------------------------[>]-*/

int
file_fhredirect (int source, int dest)
{
    int
        dupe_file_handle = 0;

    dupe_file_handle = dup (dest);
    if (dupe_file_handle < 0)
        return (-1);                    /*  Cannot acomplish redirection     */

    /*  Let dup2() close dest (if open) if duplication suceeds               */
    if (dup2 (source, dest) < 0)
      {
        close (dupe_file_handle);       /*  Close unneeded duplicate         */
        return (-1);                    /*  Cannot accomplish redirection    */
      }
    return (dupe_file_handle);          /*  Return copy of file handle       */
}


/*  ---------------------------------------------------------------------[<]-
    Function: file_fhrestore

    Synopsis: Restores a file handle redirection done by file_fhredirect().
    Supply the saved file handle (returned by file_fhdirect()) and the same
    destination file handle.  Ignores a source less than zero (invalid or
    not set).
    ---------------------------------------------------------------------[>]-*/

void
file_fhrestore (int source, int dest)
{
    if (source >= 0)
      {
        dup2 (source, dest);
        if (source != dest)
           close (source);
      }
}

/*  -------------------------------------------------------------------------
    This list holds all temporary file references, used by ftmp_close() to
    know what file to delete when a temporary file is closed.
    ------------------------------------------------------------------------ */

typedef struct {   
    void    *next;                      /*  Next item in list                */
    void    *prev;                      /*  Previous item in list            */
    FILE    *stream;                    /*  File pointer                     */
    char    *filename;                  /*  File name                        */
} FTMPITEM;

static FTMPITEM
    ftmplist = { &ftmplist, &ftmplist, NULL, NULL };
    

/*  ---------------------------------------------------------------------[<]-
    Function: ftmp_open

    Synopsis: Creates a temporary file, like the tmpfile() function, but
    without the problem under some systems where tmpfile() will try to
    create temporary files on read-only drives.  If the pathname argument is
    not null, allocates a copy of the full filename used for the temporary
    file and passes this back in the pathname argument.
    ---------------------------------------------------------------------[>]-*/

FILE *
ftmp_open (char **pathname)
{
    FILE
        *tempstream;
    char
        *tempdir,
        *tempfile;
    FTMPITEM
        *tempitem;

#if (defined (GATES_FILESYSTEM))
    tempdir = env_get_string  ("TEMP", NULL);
    if (!tempdir)
        tempdir = env_get_string  ("TMP", NULL);
    if (!tempdir)
        tempdir = ".";   

#elif (defined (__UNIX__))
    tempdir = env_get_string  ("TMPDIR", NULL);
    if (!tempdir)
        tempdir = "/tmp";

#else
    tempdir = env_get_string  ("TMPDIR", NULL);
    if (!tempdir)
        tempdir = ".";
#endif
    tempfile = get_tmp_file_name (tempdir, NULL, "tmp");
    if (pathname)
        *pathname = mem_strdup (tempfile);

    tempstream = fopen (tempfile, "wb");
    if (tempstream)
      {
        list_create (tempitem, sizeof (FTMPITEM));
        list_relink_before (&ftmplist, tempitem);
        tempitem-> stream   = tempstream;
        tempitem-> filename = tempfile;
      }
    else
        mem_free (tempfile);
        
    return (tempstream);
}


/*  ---------------------------------------------------------------------[<]-
    Function: ftmp_close

    Synopsis: Closes a temporary file created by ftmp_open().  The file
    is closed and deleted.
    ---------------------------------------------------------------------[>]-*/

void
ftmp_close (FILE *tempstream)
{
    FTMPITEM
        *tempitem;

    ASSERT (tempstream);
    fclose (tempstream);

    /*  Find the matching tempitem node and release all resources            */
    FORLIST (tempitem, ftmplist)
      {
        if (tempitem-> stream == tempstream)
          {
            file_delete (tempitem-> filename);
            mem_free (tempitem-> filename);
            list_unlink (tempitem);
            mem_free (tempitem);
            break;
          }
      }
}


/*  ---------------------------------------------------------------------[<]-
    Function: file_is_text

    Synopsis: Detects whether a file contains binary or textual data, which
    means printable data.  The algorithm works by scanning the first 1Kb of
    data in the file.  If this block contains control characters then the
    file is defined as being binary.  Returns 0 for a binary file, 1 for a
    textual file, and -1 in case of errors, including an unreadable or
    missing file.
    ---------------------------------------------------------------------[>]-*/

int
file_is_text (const char *file_name)
{
    int
        feedback = -1;
    static byte
        buffer [1024 + 1];
    int
        index,
        size;
    FILE
        *file;
        
    if (file_name && *file_name && !file_is_directory (file_name)) {
        file = fopen (file_name, "rb");
        if (file) {
            feedback = 1;
            size = fread (buffer, 1, 1024, file);
            for (index = 0; index < size; index++) {
                if (is_text (buffer [index]) == 0) {
                    feedback = 0;
                    break;
                }
            }
            fclose (file);
        }
    }
    return (feedback);
}


/*  ---------------------------------------------------------------------[<]-
    Function: file_copy_text

    Synopsis: Copies a file called src to one called dest.  The dest file
    may not already exist.  
    Returns 0 if no problems occurred, -1 if an error occurred,
    1 if the destination file already exists.
    ---------------------------------------------------------------------[>]-*/

int
file_copy_text (
    const char *dest,
    const char *src)
{
    FILE
        *inf,
        *outf;
    size_t
        chars_read;                  /*  Amount read from stream          */
    int
        feedback = 0;
    char
        *src_buffer,
        *dest_buffer;
    Bool
        add_cr;

    ASSERT (dest);
    ASSERT (src);

    if (file_exists (dest))
        return (1);                     /*  Cancel: dest already exists      */

#   if (defined (GATES_FILESYSTEM))
    if (system_devicename (dest) || system_devicename (src))
        return (-1);                    /*  Not allowed on device names      */
#   endif

    if ((inf = fopen (src, "rb")) == NULL)
        return (-1);                    /*  Input file not found             */

    if ((src_buffer = mem_alloc (SHRT_MAX + 1)) == NULL)
        feedback = -1;                  /*  Insufficient memory for buffer   */
    else
      {
        if ((dest_buffer = mem_alloc ((SHRT_MAX * 2) + 1)) == NULL)
            feedback = -1;              /*  Insufficient memory for buffer   */
        else
          {            
            if ((outf = fopen (dest, "wb")) == NULL)
                feedback = -1;         /*  Could not create output file     */
            else        
              {
#   if (defined (GATES_FILESYSTEM))
                add_cr = TRUE;
#   else
                add_cr = FALSE;
#   endif
                while ((chars_read = fread (src_buffer, 1, SHRT_MAX, inf)) != 0)
                  {
                    src_buffer [chars_read] = '\0';
                    chars_read  = file_set_eoln (dest_buffer,
                                                 src_buffer,
                                                 (dbyte)chars_read,
                                                 add_cr);
                    if (fwrite (dest_buffer, 1, chars_read, outf) != chars_read)
                      {
                        feedback = -1;
                        break;
                      }
                  }
                fclose (outf);
              }
            mem_free (dest_buffer);
          }
        mem_free (src_buffer);
      }
    fclose (inf);
    return (feedback);
}
