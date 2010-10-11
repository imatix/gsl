/*===========================================================================*
 *                                                                           *
 *  sflsymb.h - Symbol table functions                                       *
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
    Synopsis:   Symbol lookup is by name.  Symbols contain a string value
                and a pointer to an caller-defined memory block.

                The symbol-table functions let you create and manage symbol
                tables.  The functions are designed to be as general as
                possible (to support a wide variety of applications), but at
                the same time fast.  The symbol table data structure is based
                on a combined linked list & hash table representation.  The
                file sflsymb.h contains definitions for the various structures
                and external functions used in the sflsymb.c.  Both the
                linked-list and hash-table representations have a guaranteed
                order.  In the linked-list, new symbols are pushed on to the
                head of the list.  In the hash table each bucket just contains
                a pointer to a linked-list of symbols.  When a new symbol is
                created, it is pushed onto the front of this list.  The reason
                that both data structures are used is to make the algorithm
                faster.  Each representation has its stengths and weaknesses.
                For instance, if you wanted to lookup a symbol table entry
                for a given name using the hash table you could find it
                immediately, whereas with the linked-list, you would need to
                traverse most of the table to find the symbol.  Some of these
                functions are called through macros that add the filename and
                line number of the call, for tracing.  Do NOT call these
                functions directly.
 ------------------------------------------------------------------</Prolog>-*/

#ifndef SFLSYMB_INCLUDED                /*  Allow multiple inclusions        */
#define SFLSYMB_INCLUDED


/*  Structure of a symbol                                                    */

typedef struct _SYMBOL {
    struct _SYMBOL
         *next,                         /*  Next symbol in table             */
         *prev,                         /*  Previous symbol in table         */
         *h_next,                       /*  Next symbol in bucket            */
         *h_prev;                       /*  Previous symbol in bucket        */
    char *name;                         /*  Copy of name                     */
    char *value;                        /*  String value, or null            */
    void *data;                         /*  Caller data, or null             */
    byte  hash;                         /*  Hash bucket #                    */
} SYMBOL;

#define SYM_HASH_SIZE   256             /*  Assumed by sym_hash ()           */


/*  Structure of a symbol table                                              */

typedef struct {
    SYMBOL *symbols;                    /*  Pointer to list of symbols       */
    SYMBOL *hash [SYM_HASH_SIZE];       /*  Table of hash buckets            */
    int     size;                       /*  Number of symbols defined        */
} SYMTAB;


/*  Function that handles a symbol                                           */

typedef Bool (*symfunc) (SYMBOL *);


/*  Function to compare two symbols, for sorting                             */

typedef int (*symsort) (const void *symb1, const void *symb2);


/*  Function prototypes                                                      */

#ifdef __cplusplus
extern "C" {
#endif

SYMTAB *sym_create_table_  (const char *source_file, size_t source_line);
void    sym_delete_table   (SYMTAB *symtab);
void    sym_empty_table    (SYMTAB *symtab);
int     sym_merge_tables   (SYMTAB *symtab, const SYMTAB *import);
SYMBOL *sym_lookup_symbol  (const SYMTAB *symtab, const char *name);
SYMBOL *sym_create_symbol_ (SYMTAB *symtab, const char *name, const char *val,
                            const char *source_file, size_t source_line);
SYMBOL *sym_assume_symbol_ (SYMTAB *symtab, const char *name, const char *val,
                            const char *source_file, size_t source_line);
SYMBOL *sym_delete_symbol  (SYMTAB *symtab, SYMBOL *symbol);
const char *sym_get_name   (const SYMBOL *sym);
char   *sym_get_value      (const SYMTAB *symtab, const char *name,
                            const char *default_value);
long    sym_get_number     (const SYMTAB *symtab, const char *key,
                            const long default_value);
Bool    sym_get_boolean    (const SYMTAB *symtab, const char *key,
                            const Bool default_value);
void    sym_set_value      (SYMBOL *symbol, const char *value);
int     sym_exec_all       (const SYMTAB *symtab, symfunc handler);
int     sym_hash           (const char *name);
void    sym_sort_table     (SYMTAB *symtab, symsort sort_function);
char  **symb2strt_         (const SYMTAB *symtab,
                            const char *source_file, size_t source_line);
SYMTAB *strt2symb_         (char **strings,
                            const char *source_file, size_t source_line);
DESCR  *symb2descr_        (const SYMTAB *symtab,
                            const char *source_file, size_t source_line);
SYMTAB *descr2symb_        (const DESCR *descr,
                            const char *source_file, size_t source_line);


#ifdef __cplusplus
}
#endif


/*- Define macros to encapsulate calls to the hidden functions --------------*/

#if (defined (DEBUG))
# define sym_create_table()       sym_create_table_  (__FILE__, __LINE__)
# define sym_create_symbol(t,n,v) sym_create_symbol_ ((t), (n), (v), \
                                                      __FILE__, __LINE__)
# define sym_assume_symbol(t,n,v) sym_assume_symbol_ ((t), (n), (v), \
                                                      __FILE__, __LINE__)
# define symb2strt(t)  symb2strt_  ((t), __FILE__, __LINE__)
# define strt2symb(s)  strt2symb_  ((s), __FILE__, __LINE__)
# define symb2descr(t) symb2descr_ ((t), __FILE__, __LINE__)
# define descr2symb(d) descr2symb_ ((d), __FILE__, __LINE__)

#else
# define sym_create_table()       sym_create_table_  (NULL, 0)
# define sym_create_symbol(t,n,v) sym_create_symbol_ ((t), (n), (v), NULL, 0)
# define sym_assume_symbol(t,n,v) sym_assume_symbol_ ((t), (n), (v), NULL, 0)
# define symb2strt(t)  symb2strt_  ((t), NULL, 0)
# define strt2symb(s)  strt2symb_  ((s), NULL, 0)
# define symb2descr(t) symb2descr_ ((t), NULL, 0)
# define descr2symb(d) descr2symb_ ((d), NULL, 0)
#endif

#endif                                  /*  Include SFLSYMB.H                */
