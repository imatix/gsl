/*===========================================================================*
 *                                                                           *
 *  sflsearch.h - Full-text searching functions                              *
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
 *===========================================================================*/

#ifndef SFLSEARCH_INCLUDED              /*  Allow multiple inclusions        */
#define SFLSEARCH_INCLUDED


/*  Function types for accessing index sets.                                 */

typedef BITS * (*INDEX_GET) (char *field, char *value, char type);
typedef int    (*INDEX_PUT) (char *field, char *value, char type, BITS *bits, Bool create);


/*  This structure holds a single search criterion, taken from a data
    string (value, type are defined), or taken from a search query (all
    properties are defined).
 */
typedef struct {
    char *value;                        /*  Criterion value                  */
    char  type;                         /*  t=text, n=numeric, p=phonetic    */
    char  method;                       /*  '+'=or '*'=and '-'=not           */
    char  scope;                        /*  = eq > ge < le [ from ] to       */
} SCRIT;

#define SEARCH_METHOD_OR           '+'
#define SEARCH_METHOD_AND          '*'
#define SEARCH_METHOD_NOT          '-'
#define SEARCH_SCOPE_EQ            '='
#define SEARCH_SCOPE_LE            '<'
#define SEARCH_SCOPE_GE            '>'
#define SEARCH_SCOPE_FROM          '['
#define SEARCH_SCOPE_TO            ']'


/*  This structure holds a single index criterion, taken from a data
    string (value, type are defined), and referring to an object id and
    field name so that it can be sorted for building bitstrings.
 */
typedef struct {
    qbyte itemid;                       /*  Object identifier                */
    char *field;                        /*  Name of field being indexed      */
    char *value;                        /*  Criterion value                  */
    char  type;                         /*  t=text, n=numeric, p=phonetic    */
} ICRIT;


/*  This structure holds an active search                                    */

typedef struct {
    char      mode;                     /*  * = all, + = any fields          */
    INDEX_GET loadfct;                  /*  Function to read the index       */
    BITS     *result_bits;              /*  Results set so far               */
    LIST     *result_hits;              /*  Weighted results set             */
    long      item_bit;                 /*  For iterating through results    */
    LIST     *item_hit;                 /*  For iterating through results    */
    long      count;                    /*  Size of results set              */
    Bool      weighted;                 /*  If TRUE, sort results by weight  */
    Bool      sorted;                   /*  Do we still need to sort?        */
} SEARCH;


/*  This structure holds a search hit, in a list/set of hits                 */
typedef struct {
    long  itemid;                       /*  Object identifier                */
    int   weight;                       /*  Total hits for this itemid       */
} HIT;                                  /*  We assume the 'S'                */
    

/*  Function prototypes                                                      */

#ifdef __cplusplus
extern "C" {
#endif

LIST   *search_build_scrit      (char *string, char decimal);
void    search_free_scrit       (LIST **scrit_list);

LIST   *search_init_icrit       (void);
void    search_build_icrit      (LIST *icrit_list, qbyte id, char *field, char *value);
long    search_merge_icrit      (LIST *icrit_list, INDEX_GET loadfct, INDEX_PUT savefct);
long    search_unset_icrit      (LIST *icrit_list, INDEX_GET loadfct, INDEX_PUT savefct);
void    search_free_icrit       (LIST **icrit_list);

SEARCH *search_open             (Bool match_all, INDEX_GET loadfct);
void    search_ordered          (SEARCH *search, char *field, char *value, Bool mask);
void    search_weighted         (SEARCH *search, char *field, char *value, Bool mask);
long    search_count            (SEARCH *search);
long    search_first            (SEARCH *search);
long    search_next             (SEARCH *search);
void    search_close            (SEARCH *search);

#ifdef __cplusplus
}
#endif

#endif
