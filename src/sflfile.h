/*===========================================================================*
 *                                                                           *
 *  sflfile.h - File manipulation functions                                  *
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
 *  published by the Free Software Foundation; either version 3 of           *
 *  the License, or (at your option) any later version.                      *
 *                                                                           *
 *  This program is distributed in the hope that it will be useful,          *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of           *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            *
 *  GNU General Public License for more details.                             *
 *                                                                           *
 *  You should have received a copy of the GNU General Public                *
 *  License along with this program in the file 'license.gpl'; if            *
 *  not, see <http://www.gnu.org/licenses/>.                                 *
 *                                                                           *
 *  You can also license this software under iMatix's General Terms          *
 *  of Business (GTB) for commercial projects.  If you have not              *
 *  explicitly licensed this software under the iMatix GTB you may           *
 *  only use it under the terms of the GNU General Public License.           *
 *                                                                           *
 *  For more information, send an email to info@imatix.com.                  *
 *  --------------------------------------------------------------           *
 *===========================================================================*/
/*  ----------------------------------------------------------------<Prolog>-
    Synopsis:   Provides functions to read and write files with explicit
                new-line/carriage-return control; to find files on a path;
                to copy files, check files' protection, etc.
 ------------------------------------------------------------------</Prolog>-*/

#ifndef SFLFILE_INCLUDED               /*  Allow multiple inclusions        */
#define SFLFILE_INCLUDED


/*  System-specific definitions                                              */

#if (defined (__MSDOS__))
#   define FOPEN_READ_TEXT      "rt"    /*  Under DOS we can be explict      */
#   define FOPEN_READ_BINARY    "rb"    /*    and use 't' or 'b' in fopen    */
#   define FOPEN_WRITE_TEXT     "wt"
#   define FOPEN_WRITE_BINARY   "wb"
#   define FOPEN_APPEND_TEXT    "at"
#   define FOPEN_APPEND_BINARY  "ab"
#elif (defined (__VMS__))
#   define FOPEN_READ_TEXT      "r"     /*  Dec C does not like 't' or 'b'   */
#   define FOPEN_READ_BINARY    "r"
#   define FOPEN_WRITE_TEXT     "w"
#   define FOPEN_WRITE_BINARY   "w"
#   define FOPEN_APPEND_TEXT    "a"
#   define FOPEN_APPEND_BINARY  "a"
#elif (defined (__UNIX__))
#   define FOPEN_READ_TEXT      "rt"    /*  Under UNIX we can be explict     */
#   define FOPEN_READ_BINARY    "rb"    /*    and use 't' or 'b' in fopen    */
#   define FOPEN_WRITE_TEXT     "wt"
#   define FOPEN_WRITE_BINARY   "wb"
#   define FOPEN_APPEND_TEXT    "at"
#   define FOPEN_APPEND_BINARY  "ab"
#elif (defined (__OS2__))
#   define FOPEN_READ_TEXT      "rt"    /*  Under OS/2 we can be explict     */
#   define FOPEN_READ_BINARY    "rb"    /*    and use 't' or 'b' in fopen    */
#   define FOPEN_WRITE_TEXT     "wt"
#   define FOPEN_WRITE_BINARY   "wb"
#   define FOPEN_APPEND_TEXT    "at"
#   define FOPEN_APPEND_BINARY  "ab"
#else
#   error "No definitions for FOPEN constants"
#endif


/*  Constants                                                                */

enum {
    CYCLE_ALWAYS  = 0,                  /*  Cycle file unconditionally       */
    CYCLE_HOURLY  = 1,                  /*  Cycle file if hour has changed   */
    CYCLE_DAILY   = 2,                  /*  Cycle file if day has changed    */
    CYCLE_WEEKLY  = 3,                  /*  Cycle file if week has changed   */
    CYCLE_MONTHLY = 4,                  /*  Cycle file if month has changed  */
    CYCLE_NEVER   = 5                   /*  Don't cycle the file             */
};

/*  External variables                                                       */

#ifdef __cplusplus
extern "C" {
#endif

extern Bool  file_crlf;                 /*  TRUE or FALSE                    */

/*  Function prototypes                                                      */

FILE  *file_open           (const char *filename, char mode);
FILE  *file_locate         (const char *path, const char *name,
                            const char *ext);
int    file_close          (FILE *stream);
Bool   file_read           (FILE *stream, char *string);
Bool   file_readn          (FILE *stream, char *string, int line_max);
char  *file_write          (FILE *stream, const char *string);
int    file_copy           (const char *dest, const char *src, char mode);
int    file_copy_text      (const char *dest, const char *src);
int    file_concat         (const char *dest, const char *src);
int    file_rename         (const char *oldname, const char *newname);
int    file_delete         (const char *filename);
char  *file_where          (char mode, const char *path, const char *name,
                            const char *ext);
char  *file_where_ext      (char mode, const char *path, const char *name,
                            const char **ext);
Bool   file_exists         (const char *filename);
Bool   file_stable         (const char *filename);
Bool   file_cycle          (const char *filename, int how);
Bool   file_cycle_needed   (const char *filename, int how);
Bool   file_has_changed    (const char *filename, long old_date, long old_time);
Bool   safe_to_extend      (const char *filename);
char  *default_extension   (char *dest, const char *src, const char *ext);
char  *fixed_extension     (char *dest, const char *src, const char *ext);
char  *strip_extension     (char *filename);
char  *add_extension       (char *dest, const char *src, const char *ext);
char  *strip_file_path     (char *filename);
char  *strip_file_name     (char *filename);
char  *get_new_filename    (const char *filename);
Bool   file_is_readable    (const char *filename);
Bool   file_is_writeable   (const char *filename);
Bool   file_is_executable  (const char *filename);
Bool   file_is_directory   (const char *filename);
Bool   file_is_program     (const char *filename);
Bool   file_is_legal       (const char *filename);
int    file_is_text        (const char *filename);
char  *file_exec_name      (const char *filename);
long   get_file_size       (const char *filename);
time_t get_file_time       (const char *filename);
Bool   set_file_time       (const char *filename, long date, long time);
long   get_file_lines      (const char *filename);
DESCR *file_slurp          (const char *filename);
DESCR *file_slurpl         (const char *filename);
int    descr2file          (const DESCR *descr, const char *filename);

dbyte  file_set_eoln       (char *dest, const char *src, dbyte src_size,
                            Bool add_cr);
char  *get_tmp_file_name   (const char *path, qbyte *index, const char *ext);
int    file_fhredirect     (int source, int dest);
void   file_fhrestore      (int source, int dest);
FILE  *ftmp_open           (char **pathname);
void   ftmp_close          (FILE *tempstream);

#ifdef __cplusplus
}
#endif


/*  Symbols, macros                                                          */

#define FILE_NAME_MAX   160             /*  Max size of filename             */
#define FILE_DIR_MAX    64              /*  Max size of directory name       */
#define file_lines(f)   get_file_lines(f)   /*  Changed 98/07/23             */

#endif
