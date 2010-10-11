/*===========================================================================*
 *                                                                           *
 *  sflmem.h - Memory allocation/deallocation tracking                       *
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
    Synopsis:   Encapsulated memory allocation functions.  Based on an
                article by Jim Schimandle in DDJ August 1990.  Provides
                'safe' versions of malloc(), realloc(), free(), and strdup().
                These functions protect the programmer from errors in calling
                memory allocation/free routines.   When these calls are used,
                the allocation routines in this module add a data structure
                to the top of allocated memory blocks which tags them as legal
                memory blocks.  When the free routine is called, the memory
                block to be freed is checked for legality.  If the block
                is not legal, the memory list is dumped to stderr and the
                program is terminated.  Some of these functions are called
                through macros that add the filename and line number of the
                call, for tracing.  Do not call these functions directly.
 ------------------------------------------------------------------</Prolog>-*/

#ifndef SFLMEM_INCLUDED                /*  Allow multiple inclusions        */
#define SFLMEM_INCLUDED


/*- Type definitions --------------------------------------------------------*/

typedef Bool (*scavenger) (void *);     /*  Memory scavenger function        */

typedef struct _MEMHDR MEMHDR;          /*  Memory block header              */
typedef struct _MEMTRN MEMTRN;          /*  Transaction block identifier     */

/*- Function prototypes -----------------------------------------------------*/

#ifdef __cplusplus
extern "C" {
#endif

/*  General memory allocation functions                                      */

void  *mem_alloc_      (MEMTRN *trn, size_t size,
                        const char *source_file, size_t source_line);
void  *mem_realloc_    (void *block, size_t size,
                        const char *source_file, size_t source_line);
void   mem_free_       (void *block,
                        const char *source_file, size_t source_line);
void  *mem_copy_       (MEMTRN *trn, void *block,
                        const char *source_file, size_t source_line);
char  *mem_strdup_     (MEMTRN *trn, const char *string,
                        const char *source_file, size_t source_line);
char  *mem_strndup_    (MEMTRN *trn, const char *string, const size_t length,
                        const char *source_file, size_t source_line);
void   mem_strfree_    (char **string,
                        const char *source_file, size_t source_line);
DESCR *mem_descr_      (MEMTRN *trn, const void *block, size_t size,
                        const char *source_file, size_t source_line);
VDESCR *mem_vdescr_    (MEMTRN *trn, const void *block, size_t size,
                        const char *source_file, size_t source_line);

/*  Functions on transactions                                                */

MEMTRN *mem_new_trans_ (const char *source_file, size_t source_line);
void    mem_save_      (MEMTRN *to, MEMTRN *from,
                        const char *source_file, size_t source_line);
void    mem_rollback_  (MEMTRN *trn,
                        const char *source_file, size_t source_line);

/*  Hidden control functions                                                 */

void   mem_checkall_   (const char *source_file, size_t source_line);
void   mem_check_      (const void *block,
                        const char *source_file, size_t source_line);
void   mem_assert_     (MEMTRN *trn,                                          \
                        const char *source_file, size_t source_line);

/*  Visible control functions                                                */

size_t mem_size_       (const void *block,
                        const char *source_file, size_t source_line);
long   mem_used        (void);
long   mem_allocs      (void);
long   mem_frees       (void);
void   mem_freeall     (void);
void   mem_display     (FILE *save_to);
int    mem_scavenger   (scavenger scav_fct, void *scav_arg);

#ifdef __cplusplus
}
#endif

/*- Define macros to encapsulate calls to the hidden functions --------------*/

#if (defined (DEBUG))

/*  Transaction-based allocation macros                                      */

# define memt_alloc(t,n)    mem_alloc_     ((t),  (n),      __FILE__, __LINE__)
# define memt_copy(t,p)     mem_copy_      ((t),  (p),      __FILE__, __LINE__)
# define memt_strdup(t,s)   mem_strdup_    ((t),  (s),      __FILE__, __LINE__)
# define memt_strndup(t,s,n) mem_strndup_  ((t),  (s), (n), __FILE__, __LINE__)
# define memt_descr(t,p,n)  mem_descr_     ((t),  (p), (n), __FILE__, __LINE__)
# define memt_vdescr(t,p,n) mem_vdescr_    ((t),  (p), (n), __FILE__, __LINE__)
# define memt_assert(t)     mem_assert_    ((t),            __FILE__, __LINE__)

/*  Basic allocation macros                                                  */

# define mem_alloc(n)       mem_alloc_     (NULL, (n),      __FILE__, __LINE__)
# define mem_copy(p)        mem_copy_      (NULL, (p),      __FILE__, __LINE__)
# define mem_strdup(s)      mem_strdup_    (NULL, (s),      __FILE__, __LINE__)
# define mem_strndup(s,n)   mem_strndup_   (NULL, (s), (n), __FILE__, __LINE__)
# define mem_descr(p,n)     mem_descr_     (NULL, (p), (n), __FILE__, __LINE__)
# define mem_vdescr(p,n)    mem_vdescr_     (NULL, (p), (n), __FILE__, __LINE__)

/*  Commit means save to default transaction                                 */
# define mem_commit_(t,f,l) mem_save_      (NULL, (t),      (f),      (l))
# define mem_commit(t)      mem_save_      (NULL, (t),      __FILE__, __LINE__)
# define mem_save(t,f)      mem_save_      ((t),  (f),      __FILE__, __LINE__)

/*  Other functions requiring __FILE__ & __LINE__ substitution               */

# define mem_new_trans()    mem_new_trans_ (                __FILE__, __LINE__)
# define mem_rollback(t)    mem_rollback_  ((t),            __FILE__, __LINE__)
# define mem_realloc(p,n)   mem_realloc_   ((p), (n),       __FILE__, __LINE__)
# define mem_free(p)        mem_free_      ((p),            __FILE__, __LINE__)
# define mem_strfree(ps)    mem_strfree_   ((ps),           __FILE__, __LINE__)
# define mem_assert()       mem_assert_    (NULL,           __FILE__, __LINE__)
# define mem_checkall()     mem_checkall_  (                __FILE__, __LINE__)
# define mem_check(p)       mem_check_     ((p),            __FILE__, __LINE__)
# define mem_size(p)        mem_size_      ((p),            __FILE__, __LINE__)

#else

/*  Transaction-based allocation macros                                      */

# define memt_alloc(t,n)    mem_alloc_     ((t),  (n),      NULL, 0)
# define memt_copy(t,p)     mem_copy_      ((t),  (p),      NULL, 0)
# define memt_strdup(t,s)   mem_strdup_    ((t),  (s),      NULL, 0)
# define memt_strndup(t,s,n) mem_strndup_  ((t),  (s), (n), NULL, 0)
# define memt_descr(t,p,n)  mem_descr_     ((t),  (p), (n), NULL, 0)
# define memt_vdescr(t,p,n) mem_vdescr_    ((t),  (p), (n), NULL, 0)
# define memt_assert(t)     mem_assert_    ((t),            NULL, 0)

/*  Basic allocation macros                                                  */

# define mem_alloc(n)       mem_alloc_     (NULL, (n),      NULL, 0)
# define mem_copy(p)        mem_copy_      (NULL, (p),      NULL, 0)
# define mem_strdup(s)      mem_strdup_    (NULL, (s),      NULL, 0)
# define mem_strndup(s,n)   mem_strndup_   (NULL, (s), (n), NULL, 0)
# define mem_descr(p,n)     mem_descr_     (NULL, (p), (n), NULL, 0)
# define mem_vdescr(p,n)    mem_vdescr_    (NULL, (p), (n), NULL, 0)

/*  Commit means save to default transaction                                 */
# define mem_commit_(t,f,l) mem_save_      (NULL, (t),      (f),      (l))
# define mem_commit(t)      mem_save_      (NULL, (t),      NULL, 0)
# define mem_save(t,f)      mem_save_      ((t),  (f),      NULL, 0)

/*  Other functions requiring __FILE__ & __LINE__ substitution               */

# define mem_new_trans()    mem_new_trans_ (                NULL, 0)
# define mem_rollback(t)    mem_rollback_  ((t),            NULL, 0)
# define mem_realloc(p,n)   mem_realloc_   ((p), (n),       NULL, 0)
# define mem_free(p)        mem_free_      ((p),            NULL, 0)
# define mem_strfree(ps)    mem_strfree_   ((ps),           NULL, 0)
# define mem_assert()       mem_assert_    (NULL,           NULL, 0)
# define mem_checkall()     mem_checkall_  (                NULL, 0)
# define mem_check(p)       mem_check_     ((p),            NULL, 0)
# define mem_size(p)        mem_size_      ((p),            NULL, 0)

#endif

#endif
