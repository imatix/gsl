/*===========================================================================*
 *                                                                           *
 *  sflhttp.h - HTTP processing functions                                    *
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
/*  ----------------------------------------------------------------<Prolog>-
    Synopsis:   Provides various functions that support HTTP and CGI
                programming, including escaping/unescaping, and CGI data
                manipulation.
 ------------------------------------------------------------------</Prolog>-*/

#ifndef SFLHTTP_INCLUDED               /*  Allow multiple inclusions        */
#define SFLHTTP_INCLUDED

/*  Macro's and defines                                                      */

/* Macro to free up the input line that GetCgiInput created. */
#define cgi_free_input(strBuf) free((strBuf))

/* Defines for input methods. */
#define CGIGET   0
#define CGIPOST  1
#define CGIETHER 2

typedef struct _UPLOADFILE
{
    char file_name [255];               /* File name                         */
    char mime_type [255];               /* Mime type of file                 */
    long size;                          /* Size of file data                 */
    byte *data;                         /* File data                         */
} UPLOADFILE;

/*  Structure to represent the authority component of http and https urls    */
typedef struct {
    char
        *host;
    int
        port;
} URL_HTTP;

/*  Structure to represent generic URLs as per RFC 2396                      */
typedef struct {
    char
        *scheme,
        *authority,
        *path,
        *query,
        *fragment;
    URL_HTTP                            
        *http;                          /*  HTTP authority components        */
} URL;

/*  Function prototypes                                                      */

#ifdef __cplusplus
extern "C" {
#endif

char       *http_escape           (const char *string, char *result,
                                   size_t outmax);
char       *http_escape_hex       (const char *string, char *result,
                                   size_t outmax);
size_t      http_escape_size      (const char *string);
char       *http_escape_bin       (const byte *string, size_t string_size,
                                   char *result, size_t outmax);
size_t      http_escape_bin_size  (const byte *string, size_t string_size);
char       *http_unescape         (char *string, char *result);
char       *http_unescape_hex     (char *string, char *result);
char      **http_query2strt       (const char *query);
SYMTAB     *http_query2symb       (const char *query);
DESCR      *http_query2descr      (const char *query);
size_t      http_encode_meta      (char  *output, char **input,
                                   size_t outmax, Bool html);
size_t      encode_meta_char      (char  *output, char meta_char,
                                   size_t outmax, Bool html);
size_t      http_decode_meta      (char  *output, char **input, size_t outmax);
char        decode_meta_charn     (const char *meta_char, size_t length);
int         cgi_parse_query_vars  (SYMTAB *symtab, const char *query,
                                   const char *prefix);
int         cgi_parse_file_vars   (SYMTAB *symtab, FILE *file, const char *prefix,
                                   size_t size);
DESCR      *http_multipart_decode (const char *mime_file, const char *store_path,
                                   const char *local_format);
int         http_multipart_mem    (const byte *buffer, const long buffer_size,
                                   SYMTAB ** form_field, SYMTAB **files);
Bool        is_full_url           (const char *string);
char       *build_full_url        (const char *uri, const char *base_uri);
char       *http_time_str         (void);
UPLOADFILE *alloc_upload          (char *file_name, char *mime_type, long size,
                                   byte *data);
void        free_upload           (UPLOADFILE *file_uploaded);
URL_HTTP   *url_http_new     (void);
void        url_http_free         (URL_HTTP *http);
URL        *url_new               (void);
void        url_free              (URL *url);
URL        *url_from_string       (const char *string);
char       *url_to_string         (const URL *url);

#ifdef __cplusplus
}
#endif

#define decode_meta_char(string)  decode_meta_charn (string, sizeof (string))

#endif
