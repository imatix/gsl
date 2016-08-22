/*===========================================================================*
 *                                                                           *
 *  sflheap.c - Heap management functions                                    *
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

#include "prelude.h"
#include "sflheap.h"

#include "sfllist.h"
#include "sfldir.h"
#include "sflfile.h"
#include "sflmem.h"
#include "sflstr.h"

/*- Definitions -------------------------------------------------------------*/

#define heap_filename(key) file_where ('s', path, key, NULL)
#define BACKUP_SUFFIX ".bak"

/*- Global variables used in this source file only --------------------------*/

static char
    *path = NULL;

LIST
    *file_list = NULL;
FILEINFO
    *file_info = NULL;


/*- Function prototypes -----------------------------------------------------*/

static int   file_recover        (FILEINFO *file_info);
static int   load_filenames      (void);


/*  ---------------------------------------------------------------------[<]-
    Function: heap_init

    Synopsis: Initialize the directory where heap data will be stored. Either
    this function or heap_recover must be invoked PRIOR to other heap
    functions. Returns HEAP_OK on success.
    ---------------------------------------------------------------------[>]-*/
    
int
heap_init (
    const char *heappath)
{
    int
        rc = HEAP_OK;

    if (file_exists (heappath))
      {
        if (!file_is_directory (heappath))
            rc = HEAP_INVALID_PATH;
      }
    else
      {
        if (make_dir (heappath) != 0)
            rc = HEAP_CANNOT_CREATE_PATH;
      }

    if (! rc)
      {
        path = mem_strdup (heappath);
        if (!path)
            rc = HEAP_MEMORY_ERROR;
      }

    return rc;
}


/*  ---------------------------------------------------------------------[<]-
    Function: heap_recover

    Synopsis: Recovers the directory (following a crash) where heap data
    will be stored. Either this function or heap_initr must be invoked
    PRIOR to other heap functions. Returns HEAP_OK on success.
    ---------------------------------------------------------------------[>]-*/
    
int
heap_recover (
    const char *heappath)
{
    int
        rc = HEAP_OK;

    if (file_exists (heappath))
      {
        if (!file_is_directory (heappath))
            rc = HEAP_INVALID_PATH;
      }
    else
      {
        if (make_dir (heappath) != 0)
            rc = HEAP_CANNOT_CREATE_PATH;
      }

    if (! rc)
      {
        path = mem_strdup (heappath);
        if (!path)
            rc = HEAP_MEMORY_ERROR;
      }
    if (! rc)
      {
        file_list = load_dir_list (path, "t");
        if (!file_list)
            rc = HEAP_MEMORY_ERROR;
      }
    for (file_info  = (FILEINFO *) file_list-> next;
         file_info != (FILEINFO *) file_list;
         file_info  = file_info-> next)
      {
        rc = file_recover (file_info);
        if (rc)
            break;
      }
    free_dir_list (file_list);

    return rc;
}


static int
file_recover (FILEINFO *file_info)
{
    int
        rc = 0;
    char
        *origin_name,
        *origin_fullname,
        *backup_name,
        *backup_fullname;
    size_t
        backup_name_len;

    backup_name     = file_info-> dir. file_name;
    backup_name_len = strlen (backup_name);
    if (! streq (& backup_name [backup_name_len - strlen (BACKUP_SUFFIX)], 
                 BACKUP_SUFFIX))
        return HEAP_OK;

    origin_name = mem_alloc (backup_name_len - strlen (BACKUP_SUFFIX) + 1);
    strncpy (origin_name, backup_name, 
             backup_name_len - strlen (BACKUP_SUFFIX));
    origin_name [backup_name_len - strlen (BACKUP_SUFFIX)] = '\0';
    origin_fullname = NULL;

    backup_fullname = mem_strdup (heap_filename (backup_name));
    if (! file_exists (backup_fullname))
        rc = HEAP_MEMORY_ERROR;

    if (! rc)
      {
        origin_fullname = mem_strdup (heap_filename (origin_name));
        if (! origin_fullname)
            rc = HEAP_MEMORY_ERROR;
      }

    if (! rc)
        if (file_exists (origin_fullname))
            rc = file_delete (origin_fullname);
    
    if (! rc)
        rc = file_rename (backup_fullname, origin_fullname);

    if (rc)
        rc = HEAP_RECOVERY_ERROR;

    mem_free (origin_name);
    mem_free (origin_fullname);
    mem_free (backup_fullname);

    return rc;
}


/*  ---------------------------------------------------------------------[<]-
    Function: heap_dispose

    Synopsis: releases resources used by sflheap.  Returns HEAP_OK.
    ---------------------------------------------------------------------[>]-*/

int 
heap_dispose (void)
{
    mem_strfree (&path);
    if (file_list)
      {
        free_dir_list (file_list);
        file_list = NULL;
        file_info = NULL;
      }

    return HEAP_OK;
}


/*  ---------------------------------------------------------------------[<]-
    Function: heap_add

    Synopsis: adds data to some heap. A key must be provided by caller, to be 
    able to get the data back from the heap_name and the key.
    If the key already exists, an error code is returned and the data is not
    added
    ---------------------------------------------------------------------[>]-*/

int 
heap_add (
    const char   *key, 
    const DESCR  *data)
{
    int
        rc = HEAP_OK;
    char 
        *filename = NULL;

    ASSERT (key);
    ASSERT (data);

    rc = (filename = mem_strdup (heap_filename (key)))
        ? HEAP_OK
        : HEAP_MEMORY_ERROR;

    if (!rc)
        rc = !file_exists (filename) ? HEAP_OK : HEAP_DATA_ALREADY_EXISTS;
    
    if (!rc)
        rc = descr2file (data, filename) ? HEAP_IO_FAILED : HEAP_OK;

    mem_free (filename);

    return rc;
}


/*  ---------------------------------------------------------------------[<]-
    Function: heap_update

    Synopsis: Updates data to some heap.  This is done 'safely' by renaming
    the existing heap file to a backup file, creating the new file, then
    deleting the backup file.  If there is a crash during the operation,
    heap_recover should return either the old or the new data.
    ---------------------------------------------------------------------[>]-*/

int 
heap_update (
    const char   *key, 
    const DESCR  *data)
{
    int
        rc = HEAP_OK;
    char 
        *filename   = NULL,
        *backupname = NULL;

    ASSERT (key);
    ASSERT (data);

    rc = (filename = mem_strdup (heap_filename (key)))
        ? HEAP_OK
        : HEAP_MEMORY_ERROR;

    if (!rc)
      {
        if (file_exists (filename))
          {
            backupname = mem_alloc (strlen (filename)
                                  + strlen (BACKUP_SUFFIX) + 1);
            if (!backupname)
                rc = HEAP_MEMORY_ERROR;
            else
                file_rename (filename, backupname);
          }
        else
            rc = HEAP_DATA_NOT_FOUND;
      }
    
    if (!rc)
        rc = descr2file (data, filename) ? HEAP_IO_FAILED : HEAP_OK;

    if ((!rc) && backupname)
        rc = file_delete (backupname);

    mem_free (filename);
    mem_free (backupname);

    return rc;
}


/*  ---------------------------------------------------------------------[<]-
    Function: heap_remove

    Synopsis: deletes a data previously added to the heap.  
    ---------------------------------------------------------------------[>]-*/

int 
heap_remove (
    const char *key)
{
    int 
        rc = HEAP_OK;
    char
        *filename = NULL;
    
    ASSERT (key != NULL);

    rc = (filename = mem_strdup (heap_filename (key)))
        ? HEAP_OK
        : HEAP_MEMORY_ERROR;

    if (!rc)
        rc = file_exists (filename) ? HEAP_OK : HEAP_DATA_NOT_FOUND;

    if (!rc)
        rc = file_delete (filename) ? HEAP_CANNOT_DELETE_DATA : HEAP_OK;

    mem_free (filename);

    return rc;
}


/*  ---------------------------------------------------------------------[<]-
    Function: heap_exists

    Synopsis: returns TRUE if the data associated to 'key' is present within
    a heap.
    ---------------------------------------------------------------------[>]-*/

Bool 
heap_exists (
    const char *key)
{
    Bool 
        res;
    char 
        *filename = NULL;

    filename = mem_strdup (heap_filename (key));
    if (filename == NULL)
        return FALSE;

    res = file_exists (filename);
    mem_free (filename);
    return res;
}


/*  ---------------------------------------------------------------------[<]-
    Function: heap_get

    Synopsis: retrieves a data previously added to the heap. The data is 
    returned through argument 'data'. The DESCR is allocated and should be
    freed using the mem_free() call.
    Returns:
    - HEAP_OK on success
    - error code otherwise.
    ---------------------------------------------------------------------[>]-*/

int
heap_get (
    const char   *key,
          DESCR **data)
{
    int
        rc = 0;
    char
        *filename = NULL;
    
    filename = mem_strdup (heap_filename (key));
    if (filename == NULL)
        rc = HEAP_IO_FAILED;

    if (!rc)
      {
        if (!file_exists (filename))
            rc = HEAP_DATA_NOT_FOUND;
      }

    if (!rc)
      {
        *data = file_slurpl (filename);
        if (*data == NULL)
            rc = HEAP_IO_FAILED;
      }
        
    mem_free (filename);

    return rc;
}


/*  ---------------------------------------------------------------------[<]-
    Function: heap_first

    Synopsis: gets the first data of some heap. The key and data are returned
    through the function arguments, and are consistent only if HEAP_OK is 
    returned. 'key' and 'data' should be freed using the mem_free() call.
    The data are sorted on time (FIFO).
    Returns: see heap_next (below)
    ---------------------------------------------------------------------[>]-*/

int heap_first (char   **key, 
                DESCR  **data)
{
    int 
        rc = HEAP_OK;
    
    ASSERT (key != NULL);

    rc = load_filenames ();
    if (!rc)
      {
        file_info = file_list-> next;
        rc = heap_next (key, data);
      }
    return rc;
}



static
int load_filenames (void)
{
    int 
        rc = HEAP_OK;

    file_list = load_dir_list (path, "t");
    if (! file_list)
        rc = HEAP_MEMORY_ERROR;

    return rc;
}


/*  ---------------------------------------------------------------------[<]-
    Function: heap_next

    Synopsis: gets the next data of some heap. The key and data are returned
    through the function arguments, and are consistent only if HEAP_OK is 
    returned. 'key' and 'data' should be freed using the mem_free() call.
    The data are sorted on time (FIFO).
    
    In addition to heap_first (see above), we have everything to iterate on all
    previously added data.
    A standard usage would be:
    
    rc = heap_first (HEAPNAME, &key, &data);
    while (rc == HEAP_OK)
      {
        handle (data, key);
        mem_free (data);
        mem_free (key);
        rc = heap_next (HEAPNAME, &key, &data);
      }

    safety: NOT thread safe (shared variables)
    ---------------------------------------------------------------------[>]-*/

int heap_next (char  **key, 
               DESCR **data)
{
    int 
        rc = HEAP_OK;
    char
        *file_name = NULL,
        *temp_key;

    ASSERT (key);

    if ((! file_list)
    ||  (! file_info))
        return HEAP_DATA_NOT_FOUND;

    if (file_info == (FILEINFO *) file_list)
      {
        free_dir_list (file_list);
        file_list = NULL;
        file_info = NULL;

        return HEAP_DATA_NOT_FOUND;
      }

    file_name = file_info-> dir. file_name;
    file_info = file_info-> next;

    temp_key = mem_strdup (file_name);
    
    rc = heap_get (temp_key, data);

    if (rc)
        mem_free (temp_key);
    else
        *key = temp_key;
    
    return rc;
}


/*  ---------------------------------------------------------------------[<]-
    Function: heap_rename

    Synopsis: changes heapname and/or key of some previously added data.
    This function is strictly equivalent to :

    DESCR
        *data = NULL;

    heap_get (src_name, src_key, &data);
    heap_add (dst_name, dst_key, data);
    heap_remove (src_name, src_key);

    mem_free (desc);

    but saves time and memory because files are directly renamed, instead of
    loading data to memory.
    ---------------------------------------------------------------------[>]-*/

int 
heap_rename (
    const char *src_name, const char *src_key,
    const char *dst_name, const char *dst_key
  )
{
    char
        *src_file = NULL,
        *dst_file = NULL;
    int
        err = HEAP_OK;
    
    ASSERT (src_name && src_key && dst_name && dst_key);

    if (err == HEAP_OK)
      {
        src_file = file_where ('s', src_name, src_key, NULL);
        err = src_file ? HEAP_OK : HEAP_MEMORY_ERROR;
      }

    if (err == HEAP_OK)
        err = file_exists (src_file) ? HEAP_OK : HEAP_DATA_NOT_FOUND;

    if (err == HEAP_OK)
      {
        dst_file = file_where ('s', dst_name, dst_key, NULL);
        err = dst_file ? HEAP_OK : HEAP_MEMORY_ERROR;
      }     

    if (err == HEAP_OK)
        err = !file_exists (dst_file) ? HEAP_OK : HEAP_DATA_ALREADY_EXISTS;

    if (err == HEAP_OK)
        err = file_rename (src_file, dst_file) == 0 ? HEAP_OK : HEAP_IO_FAILED;

    mem_free (src_file);
    mem_free (dst_file);

    return err;
}

