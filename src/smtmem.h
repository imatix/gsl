/*===========================================================================*
 *                                                                           *
 *  smtmem.h - Thread-aware memory wrapper                                   *
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

#ifndef SMTMEM_INCLUDED                 /*  Allow multiple inclusions        */
#define SMTMEM_INCLUDED

/*  Undefine previous definitions.                                           */

# undef  mem_alloc
# undef  mem_copy
# undef  mem_strdup
# undef  mem_descr
# undef  mem_commit_
# undef  mem_commit
# undef  mem_assert

/*- Define macros to encapsulate calls to the hidden functions --------------*/

#if (defined (DEBUG))

/*  Basic allocation macros                                                  */

# define mem_alloc(n)       mem_alloc_     (smt_memtrn_,                      \
                                                  (n),      __FILE__, __LINE__)
# define mem_copy(p)        mem_copy_      (smt_memtrn_,                      \
                                                  (p),      __FILE__, __LINE__)
# define mem_strdup(s)      mem_strdup_    (smt_memtrn_,                      \
                                                  (s),      __FILE__, __LINE__)
# define mem_descr(p,n)     mem_descr_     (smt_memtrn_,                      \
                                                  (p), (n), __FILE__, __LINE__)

/*  Commit means save to default transaction                                 */
# define mem_commit_(t,f,l) mem_save_      (smt_memtrn_,                      \
                                                  (t),      (f),      (l))
# define mem_commit(t)      mem_save_      (smt_memtrn_,                      \
                                                  (t),      __FILE__, __LINE__)

/*  Other functions requiring __FILE__ & __LINE__ substitution               */

# define mem_assert()       mem_assert_    (smt_memtrn_,    __FILE__, __LINE__)

#else

/*  Basic allocation macros                                                  */

# define mem_alloc(n)       mem_alloc_     (smt_memtrn_,                      \
                                                  (n),      NULL, 0)
# define mem_copy(p)        mem_copy_      (smt_memtrn_,                      \
                                                  (p),      NULL, 0)
# define mem_strdup(s)      mem_strdup_    (smt_memtrn_,                      \
                                                  (s),      NULL, 0)
# define mem_descr(p,n)     mem_descr_     (smt_memtrn_,                      \
                                                  (p), (n), NULL, 0)

/*  Commit means save to default transaction                                 */
# define mem_commit_(t,f,l) mem_save_      (smt_memtrn_,                      \
                                                  (t),      (f),      (l))
# define mem_commit(t)      mem_save_      (smt_memtrn_,                      \
                                                  (t),      NULL, 0)

/*  Other functions requiring __FILE__ & __LINE__ substitution               */

# define mem_assert()       mem_assert_    (smt_memtrn_,    NULL, 0)

#endif

#endif
