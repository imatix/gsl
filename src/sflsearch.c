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

/*
    Full-text searching functions

        These functions are based on a model designed by Leif Svalgaard
        and implemented here by Pieter Hintjens.  The goals are to achieve
        excellent response times while keeping the index database small.
        Additionally, we support a fairly rich search model including
        boolean searches and phonetic lookups.  The key to the full-text
        search model is the use of compressed bitmaps (sflbits), which
        allows us to represent sets of items with ease.  The average cost
        per set member ('bit') is about 4 bits, for sparse bitmaps, and
        less than one bit for dense bitmaps.  The alternatives (such as
        representing sets as database tables) take considerably more
        space to store, and time to manipulate.

        The search engine uses several structures to manage its process.
        An SCRIT holds a search criteria, built during indexation and
        during searching, and represent an index token built from the
        original raw data.  Raw text can be turned into multiple SCRITs,
        each representing some (arbitrary, defined by the search engine)
        searchable information.  SCRITs are used during the parsing
        process, mainly to communicate between the parser (sflscrit) and
        the search engine.

        An ICRIT holds a index criteria, built to represent a search
        criteria for a specific object instance.  The indexation process
        turns a series of data records (handled on a field-by-field basis)
        into a long list of ICRITs.  These are then sorted so that the
        construction of the bitmaps representing the set of objects which
        match each criteria is fast.

        The search engine imposes a requirement on the application database,
        namely that all objects be identified by a unique number, ideally
        in the range 1 to 16million.  (This limit of 16m comes from sflbits
        and may be increased in the future).

    Features:
        - reasonably fast indexing, very fast lookup
        - structured search on combinations of record fields
        - indexes numbers and text fields independently
        - uses a phonetic algorithm for text fields
        - indexes up to 16m application objects
        - logical operations on individual fields and overall searches
        - weighting of results by number of hits

    There are two steps, creating an index and searching the index.
    The index is stored in a database table managed by your application.
    sflsearch will call two functions (index_get and index_put) in your
    code to read and write index records.

    The present implementation handles creating and using indexes, but
    not updating them (i.e. when application data changes).

    To index one or more application objects, use this logic:

        list = search_init_icrit ();
        for each object you want to index:
            for each field you want to index:
                search_build_icrit (...);
            for each hidden criteria:
                search_build_icrit (...);
        search_merge_icrit (list)     -- updates the index
        search_free_icrit (&list);

    Note that it is often useful to index on hidden criteria that limit
    the search results in some way.  E.g. if the user will have access
    to a certain set of records, this can be easily controlled by adding
    a criteria.

    The pseudo-code for the index_get function is:

        fetch record indexed by field, value and type
        if found
            return (bits_load (bitdata))
        else
            return (NULL)

    The maximum size of a serialized bitstring varies, but your should
    allow at least 8k characters, and test the actual sizes generated so
    that you can increase internal limits as necessary.  The theoretical
    limit is very high (for sufficiently populated bitstrings).

    The pseudo-code for the index_put function is:

        serialize bits using bits_save
        if create
            create record indexed by field, value, and type
        else
            update record indexed by field, value, and type
        bits_destroy (bits)
        return (0)

    To search for some terms, use this logic:

        search_open (...)
        for each field the user provided
            search_ordered (..., FALSE);
        for each hidden criteria:
            search_ordered (..., TRUE);
        itemid = search_first (...)
        while itemid > 0
            ... process item
            itemid = search_next (...)
        search_close (...)

    You can use search_weighted in place of search_ordered to get a
    weighted search, which is often more useful.  A weighted search
    returns the items ordered by the number of hits (most significant
    items first).  Hidden criteria are not counted, if the 'mask'
    argument is set to true for search_weighted(), as shown above.
    Note that search_weighted() can consume lots of memory.

    Search logic: the general principle is that multiple values for a
    single field act to widen the search, whereas multiple field values
    restrict the search.  This can be controlled by the search mode
    specified in the search_open() call - if match_all is false,
    specifying multiple fields will also widen the search.

    Each criteria can be specified with modifiers:

        +value      criteria is mandatory
        -value      criteria may not occur

    There is also unfinished support for ranges:

        value>          from value and higher
        >value          up to and including value
        value1>value2   from value1 to value2

    All data is assumed to be in the ISO-8859-1 character set, and accented
    characters are folded into their non-accented 'equivalents'.

    Performance: for fastest indexation, index as many records as possible
    before doing a search_merge_icrit().  The cost of indexing individual
    records rises exponentially with the size of the database (number of
    records x number of different terms).  Indexing multiple records in a
    single go keeps this linear.  Note that memory may play a limiting
    factor for very large input samples.  In a typical test of indexing a
    500K file with 6000 records and about 7500 search terms, indexation
    record-by-record took about 30 minutes, while indexation in a single
    go took about 30 seconds.  Retrieval is always very fast, even for
    complex searches.
 */

#include "prelude.h"                    /*  Universal header file            */
#include "sfllist.h"                    /*  Linked-list functions            */
#include "sflmem.h"                     /*  Memory handling functions        */
#include "sflstr.h"                     /*  String-handling functions        */
#include "sflcons.h"                    /*  Console i/o functions            */
#include "sflsort.h"                    /*  List-sorting functions           */
#include "sflbits.h"                    /*  Large bitstring functions        */
#include "sflsearch.h"                  /*  Prototypes for functions         */


/*  Local function prototypes                                                */

static Bool  icrit_compare     (LIST *node1, LIST *node2);
static long  process_icrit     (LIST *icrit_list, INDEX_GET loadfct,
                                INDEX_PUT savefct, Bool insert);
static LIST *bits_to_hitlist   (BITS *bits);
static long  hitlist_union     (LIST *list1, LIST *list2, Bool addup);
static long  hitlist_intersect (LIST *list1, LIST *list2, Bool addup);
static void  hitlist_resort    (LIST *hitlist);
static Bool  hit_compare       (LIST *node1, LIST *node2);


/*  ---------------------------------------------------------------------[<]-
    Function: search_free_scrit

    Synopsis: Frees a SCRIT list built by search_build_scrit.  The SCRIT list
    may be empty but not null.  Frees the memory used by the SCRIT list and
    sets this pointer to null.
    ---------------------------------------------------------------------[>]-*/

void
search_free_scrit (LIST **scrit_list)
{
    SCRIT
        scrit;

    ASSERT (*scrit_list);
    while (!list_empty (*scrit_list)) {
        list_pop (*scrit_list, scrit);
        mem_free (scrit.value);
    }
    mem_free (*scrit_list);
    *scrit_list = NULL;
}


/*  ---------------------------------------------------------------------[<]-
    Function: search_init_icrit

    Synopsis: Creates an empty index-criteria (ICRIT) list and returns the
    address of this list.  The caller passes this to the search_collect_icrit
    function to collect index-criteria, and finally to search_free_icrit to
    delete it.
    ---------------------------------------------------------------------[>]-*/

LIST *
search_init_icrit (void)
{
    LIST
        *list;

    list = mem_alloc (sizeof (LIST));
    list_reset (list);
    return (list);
}


/*  ---------------------------------------------------------------------[<]-
    Function: search_build_icrit

    Synopsis: Builds a ICRIT items from the supplied field name and value,
    and attaches these to an ICRIT list.  The caller passes an object id,
    field name, and field value, and these are turned into a number of
    ICRIT blocks appened to the icrit list.  Each block specifies one index
    criteria that is appropriate for the field value.  For the algorithm that
    creates index criteria from a field value, see: search_build_scrit().
    ---------------------------------------------------------------------[>]-*/

void
search_build_icrit (LIST *icrit_list, qbyte itemid, char *field, char *value)
{
    LIST
        *scrit_list,
        *scrit_node;
    SCRIT
        *scrit_item;
    ICRIT
       icrit;

    ASSERT (icrit_list);
    scrit_list = search_build_scrit (value, '.');
    scrit_node = scrit_list->next;
    while (scrit_node != scrit_list) {
        scrit_item = (SCRIT *) ((char *) scrit_node + sizeof (LIST));
        icrit.itemid = itemid;
        icrit.field  = field;
        icrit.value  = mem_strdup (scrit_item->value);
        icrit.type   = scrit_item->type;
        list_queue (icrit_list, icrit);
        scrit_node = scrit_node->next;
    }
    search_free_scrit (&scrit_list);
}


/*  ---------------------------------------------------------------------[<]-
    Function: search_merge_icrit

    Synopsis: processes an index criteria list and saves to some permanent
    storage.  The caller supplies two functions to read and write index
    information from the permanent storage.  One index record consists of an
    ICRIT structure that specifies the field name, value, and type for the
    criterion, and a BITS compressed bitstring that holds the set of data
    records which match that criterion.

    This function takes a list of ICRIT structures and for each one, sets
    the appropriate bit in the index.  Note that the search_unset_icrit()
    function does the opposite, deleting the specified bits in the index.
    ---------------------------------------------------------------------[>]-*/

long
search_merge_icrit (LIST *icrit_list, INDEX_GET loadfct, INDEX_PUT savefct)
{
    return (process_icrit (icrit_list, loadfct, savefct, TRUE));
}


/*  ---------------------------------------------------------------------[<]-
    Function: search_unset_icrit

    Synopsis: takes a list of ICRIT structures and for each one, clears
    the appropriate bit in the index.  This function is used to delete
    index information associated with a particular data set.
    ---------------------------------------------------------------------[>]-*/

long
search_unset_icrit (LIST *icrit_list, INDEX_GET loadfct, INDEX_PUT savefct)
{
    return (process_icrit (icrit_list, loadfct, savefct, FALSE));
}


/*  process_icrit -- local
    Does the hard work for search_merge/unset_icrit.
    Returns the number of ICRITs processed.
 */
static long
process_icrit (LIST *icrit_list, INDEX_GET loadfct, INDEX_PUT savefct, Bool insert)
{
    LIST
        *icrit_node;
    ICRIT
        *icrit_item;
    char
        *last_field = "",
        *last_value = "",
        last_type = ' ';
    BITS
        *bits = NULL;                   /*  Current bitset                   */
    long
        count = 0;
    Bool
        new_bits = FALSE;
        
    /*  Sort the list of index criteria                                      */
    list_sort (icrit_list, icrit_compare);

    /*  We now have the criteria sorted by field name, value, type and so
        we can collect the IDs for each of these and set/clear the matching
        bits.
     */
    icrit_node = icrit_list->next;
    while (icrit_node != icrit_list) {
        icrit_item = (ICRIT *) ((char *) icrit_node + sizeof (LIST));
        if (strneq (icrit_item->field, last_field)
        ||  strneq (icrit_item->value, last_value)
        ||          icrit_item->type != last_type) {
            if (*last_field)
                (*savefct) (last_field, last_value, last_type, bits, new_bits);
            last_field = icrit_item->field;
            last_value = icrit_item->value;
            last_type  = icrit_item->type;
            bits = (*loadfct) (last_field, last_value, last_type);
            if (bits == NULL) {
                bits = bits_create ();
                new_bits = TRUE;
            }
            else
                new_bits = FALSE;
        }
        if (insert)
            bits_set (bits, (long) icrit_item->itemid);
        else
            bits_clear (bits, (long) icrit_item->itemid);

        count++;
        icrit_node = icrit_node->next;
    }
    if (*last_field)
        (*savefct) (last_field, last_value, last_type, bits, new_bits);
    return (count);
}


/*  icrit_compare
    Comparison function for icrit list sorting
    Returns TRUE if the two nodes need swapping
 */
static Bool
icrit_compare (LIST *node1, LIST *node2)
{
    ICRIT
        *icrit1,
        *icrit2;
    int
        rc;

    icrit1 = (ICRIT *) ((char *) node1 + sizeof (LIST));
    icrit2 = (ICRIT *) ((char *) node2 + sizeof (LIST));

    /*  Sort by field / criterion type / criterion value                     */
    rc = strcmp (icrit1->field, icrit2->field);
    if (rc < 0)
        return (FALSE);
    else
    if (rc > 0)
        return (TRUE);
    else {
        if (icrit1->type < icrit2->type)
            return (FALSE);
        else
        if (icrit1->type > icrit2->type)
            return (TRUE);
        else {
            rc = strcmp (icrit1->value, icrit2->value);
            if (rc <= 0)
                return (FALSE);
            else
                return (TRUE);
        }
    }
}


/*  ---------------------------------------------------------------------[<]-
    Function: search_free_icrit

    Synopsis: Frees a ICRIT list built by search_collect_icrit.  The icrit
    list may be empty but not null.  Frees the memory used by the icrit list
    and sets this pointer to null.
    ---------------------------------------------------------------------[>]-*/

void
search_free_icrit (LIST **icrit_list)
{
    ICRIT
        icrit;

    ASSERT (*icrit_list);
    while (!list_empty (*icrit_list)) {
        list_pop (*icrit_list, icrit);
        mem_free (icrit.value);
    }
    mem_free (*icrit_list);
    *icrit_list = NULL;
}


/*  ---------------------------------------------------------------------[<]-
    Function: search_open

    Synopsis: Prepares a new search, creates and initializes a SEARCH
    structure and returns the address of this structure.  If match_all is
    true, all search fields must match, otherwise any may match.  You can
    set the search fields after calling search_open() in order to change
    various aspects of the search engine.
    ---------------------------------------------------------------------[>]-*/

SEARCH *
search_open (Bool match_all, INDEX_GET loadfct)
{
    SEARCH
        *search;

    search = mem_alloc (sizeof (SEARCH));
    search->result_bits = NULL;        /*  No results yet                   */
    search->result_hits = NULL;
    search->item_bit    = 0;
    search->item_hit    = NULL;
    search->mode        = match_all? '*': '+';
    search->loadfct     = loadfct;
    search->weighted    = FALSE;
    search->sorted      = FALSE;
    return (search);
}


/*  ---------------------------------------------------------------------[<]-
    Function: search_ordered

    Synopsis: Executes the search specified by the field and the value.
    The search results are cummulated into the search->result_bits bitstring
    and returned in order of the original IDs.  If the mask argument is true
    the criteria is treated as a bitmask (AND) irrespective of the search
    mode.
    ---------------------------------------------------------------------[>]-*/

void
search_ordered (SEARCH *search, char *field, char *value, Bool mask)
{
    LIST
        *scrit_list,
        *scrit_node;
    SCRIT
        *scrit_item;
    BITS
        *result_bits,                   /*  Current results                  */
        *cur_bits,                      /*  Bitmap for this criterion        */
        *accumulator;                   /*  Intermediate results             */

    if (strnull (value))
        return;                         /*  Ignore an empty value            */

    /*  Parse value into criteria with search methods and scopes             */
    scrit_list = search_build_scrit (value, '.');

    /*  Our initial empty results set is an empty bitmap                     */
    result_bits = bits_create ();
        
    /*  Process the search criteria one by one                               */
    scrit_node = scrit_list->next;
    while (scrit_node != scrit_list) {
        scrit_item = (SCRIT *) ((char *) scrit_node + sizeof (LIST));
        /*  We don't do ranges yet...                                        */

        /*  Load bitset for this criteria from the index database            */
        cur_bits = (*search->loadfct) (field, scrit_item->value, scrit_item->type);

        /*  If not empty, fold into result bitset                            */
        if (cur_bits) {
            if (scrit_item->method == SEARCH_METHOD_OR) {
                accumulator = bits_or (result_bits, cur_bits);
                bits_destroy (result_bits);
                result_bits = accumulator;
            }
            else
            if (scrit_item->method == SEARCH_METHOD_AND) {
                accumulator = bits_and (result_bits, cur_bits);
                bits_destroy (result_bits);
                result_bits = accumulator;
            }
            else
            if (scrit_item->method == SEARCH_METHOD_NOT) {
                accumulator = bits_invert (cur_bits);
                bits_destroy (cur_bits);
                cur_bits = accumulator;
                accumulator = bits_and (result_bits, cur_bits);
                bits_destroy (result_bits);
                result_bits = accumulator;
            }
            bits_destroy (cur_bits);
        }
        scrit_node = scrit_node->next;
    }
    /*  Now fold results into search results, union or intersection          */
    if (search->result_bits) {
        if (search->mode == '*' || mask)
            accumulator = bits_and (search->result_bits, result_bits);
        else
            accumulator = bits_or  (search->result_bits, result_bits);
            
        bits_destroy (search->result_bits);
        bits_destroy (result_bits);
        search->result_bits = accumulator;
    }
    else
        search->result_bits = result_bits;

    search->count = 0;                 /*  Invalidate result set size       */
    search_free_scrit (&scrit_list);
}


/*  ---------------------------------------------------------------------[<]-
    Function: search_weighted

    Synopsis: Executes the search specified by the field and the value.
    The search results are cummulated into the search->result_hits hitlist
    and returned in order of the weight of each result, with the most
    important matches first.  If the mask argument is TRUE, the criteria
    is treated as a bitmask on the results, irrespective of the search mode
    and without impact on the weighting.
    ---------------------------------------------------------------------[>]-*/

void
search_weighted (SEARCH *search, char *field, char *value, Bool mask)
{
    LIST
        *scrit_list,
        *scrit_node,
        *result_hits,                   /*  For weighted searches            */
        *cur_hits = NULL;
    SCRIT
        *scrit_item;
    BITS
        *cur_bits,                      /*  Bitmap for this criterion        */
        *accumulator;                   /*  Intermediate results             */
    long
        hits = 0;                       /*  Last hit count                   */

    if (strnull (value))
        return;                         /*  Ignore an empty value            */

    /*  Parse value into criteria with search methods and scopes             */
    scrit_list = search_build_scrit (value, '.');

    /*  Our initial empty results set is an empty hitlist                    */
    result_hits = mem_alloc (sizeof (LIST));
    list_reset (result_hits);
        
    /*  Process the search criteria one by one                               */
    scrit_node = scrit_list->next;
    while (scrit_node != scrit_list) {
        scrit_item = (SCRIT *) ((char *) scrit_node + sizeof (LIST));
        /*  We don't do ranges yet...                                        */

        /*  Load bitset for this criteria from the index database            */
        cur_bits = (*search->loadfct) (field, scrit_item->value, scrit_item->type);

        /*  If not empty, fold into result bitset                            */
        if (cur_bits) {
            if (scrit_item->method == SEARCH_METHOD_OR) {
                cur_hits = bits_to_hitlist (cur_bits);
                hits = hitlist_union (result_hits, cur_hits, TRUE);
            }
            else
            if (scrit_item->method == SEARCH_METHOD_AND) {
                cur_hits = bits_to_hitlist (cur_bits);
                hits = hitlist_intersect (result_hits, cur_hits, TRUE);
            }
            else
            if (scrit_item->method == SEARCH_METHOD_NOT) {
                accumulator = bits_invert (cur_bits);
                bits_destroy (cur_bits);
                cur_bits = accumulator;
                cur_hits = bits_to_hitlist (cur_bits);
                hits = hitlist_intersect (result_hits, cur_hits, TRUE);
            }
            bits_destroy (cur_bits);
            list_destroy (cur_hits);
            mem_free (cur_hits);
        }
        scrit_node = scrit_node->next;
    }
    /*  Now fold results into search results, union or intersection          */
    if (search->result_hits) {
        if (mask)
            hits = hitlist_intersect (search->result_hits, result_hits, FALSE);
        else
        if (search->mode == '*')
            hits = hitlist_intersect (search->result_hits, result_hits, TRUE);
        else
            hits = hitlist_union     (search->result_hits, result_hits, TRUE);
            
        list_destroy (result_hits);
        mem_free (result_hits);
    }
    else
        search->result_hits = result_hits;

    search->weighted = TRUE;
    search->sorted   = FALSE;
    search->count    = hits;
    search_free_scrit (&scrit_list);
}


/*  bits_to_hitlist -- internal
    Create a hitlist from a bits structure, i.e. a list of HITs with a
    weight of 1 for each matching bit.  May use large amounts of memory
    if the bitstring is highly populated.
 */
static LIST *
bits_to_hitlist (BITS *bits)
{
    LIST
       *hitlist;
    HIT
        hit;
    long
        itemid;
    
    hitlist = mem_alloc (sizeof (LIST));
    list_reset (hitlist);

    itemid = bits_search_set (bits, 0, FALSE);
    while (itemid >= 0) {
        hit.itemid = itemid;
        hit.weight = 1;
        list_queue (hitlist, hit);
        itemid = bits_search_set (bits, itemid, FALSE);
    }
    return (hitlist);
}

/*  hitlist_union -- internal
    Merges one list into a second, keeping all items that are present
    in either list, and adding the weights for these items if the
    addup argument is true.  Both lists must be ordered by itemid.
    This function modifies list1 and leaves list2 unchanged.  Returns
    the number of nodes in list1.
 */
static long
hitlist_union (LIST *list1, LIST *list2, Bool addup)
{
    LIST
        *node1,
        *node2;
    HIT
        *hit1,
        *hit2;
    long
        count = 0;                      /*  Size of resulting list1          */

    /*  We process each item in list2, adding it to list1 as possible        */
    node1 = list1->next;
    node2 = list2->next;
    while (node2 != list2) {
        hit1 = (HIT *) ((char *) node1 + sizeof (LIST));
        hit2 = (HIT *) ((char *) node2 + sizeof (LIST));
        if (node1 != list1 && hit1->itemid == hit2->itemid) {
            /*  We have a hit in both lists, so add weights                  */
            if (addup)
                hit1->weight += hit2->weight;
            node1 = node1->next;
            node2 = node2->next;
            count++;
        }
        else
        if (node1 != list1 && hit1->itemid < hit2->itemid) {
            /*  We can skip a node in list1                                  */
            node1 = node1->next;
        }
        else {
            /*  We have to copy a node from list2                            */
            list_attach (node1->prev, hit2, sizeof (HIT));
            node2 = node2->next;
            count++;
        }
    }
    return (count);
}

/*  hitlist_intersect -- internal
    Merges one list into a second, keeping only items that are present
    in both lists, and adding the weights for these items if the addup
    argument is true.  Both lists must be ordered by itemid.  This
    function modifies list1 and leaves list2 unchanged.  It returns the
    number of nodes in list1.
 */
static long
hitlist_intersect (LIST *list1, LIST *list2, Bool addup)
{
    LIST
        *node1,
        *node2,
        *junk;                          /*  This poor bugger gets it         */
    HIT
        *hit1,
        *hit2;
    long
        count = 0;                      /*  Size of resulting list1          */

    /*  We process each item in list1, keeping it if it is in list2          */
    node1 = list1->next;
    node2 = list2->next;
    while (node1 != list1) {
        hit1 = (HIT *) ((char *) node1 + sizeof (LIST));
        hit2 = (HIT *) ((char *) node2 + sizeof (LIST));
        if (node2 == list2
        ||  hit1->itemid < hit2->itemid) {
            /*  Hit1 does not appear on list2, so junk it                    */
            junk  = node1;
            node1 = node1->next;
            list_unlink (junk);
            mem_free    (junk);
        }
        else
        if (hit1->itemid == hit2->itemid) {
            /*  Hit1 also appears in list2, so keep it                       */
            if (addup)
                hit1->weight += hit2->weight;
            node1 = node1->next;
            node2 = node2->next;
            count++;
        }
        else
            /*  Hit1 may still appear on list2 - let's bump & try again      */
            node2 = node2->next;
    }
    return (count);
}


/*  ---------------------------------------------------------------------[<]-
    Function: search_count

    Synopsis: Return size of result set.
    ---------------------------------------------------------------------[>]-*/

long
search_count (SEARCH *search)
{
    /*  We may need to sort the results hitlist                              */
    if (search->weighted && !search->sorted) {
        hitlist_resort (search->result_hits);
        search->sorted = TRUE;        
    }
    else
    if (search->result_bits) {
        if (search->count == 0)
            search->count = bits_set_count (search->result_bits);
    }
    else
        search->count = 0;
        
    return (search->count);
}


/*  ---------------------------------------------------------------------[<]-
    Function: search_first

    Synopsis: Return first item in search results, 0 if none.
    ---------------------------------------------------------------------[>]-*/

long
search_first (SEARCH *search)
{
    HIT
        *hit;
        
    if (search->weighted) {
        /*  Be prepared to resort when starting                              */
        if (!search->sorted) {
            hitlist_resort (search->result_hits);
            search->sorted = TRUE;        
        }
        search->item_hit = search->result_hits->next;
        if (search->item_hit != search->result_hits) {
            hit = (HIT *) ((char *) search->item_hit + sizeof (LIST));
            return (hit->itemid);
        }
        else
            return (0);
    }
    else {
        if (search->result_bits) {
            search->item_bit = bits_search_set (search->result_bits, 0, FALSE);
            return (search->item_bit);
        }
        else
            return (0);
    }
}


/*  ---------------------------------------------------------------------[<]-
    Function: search_next

    Synopsis: Return next item in search results, 0 if none.
    ---------------------------------------------------------------------[>]-*/

long
search_next (SEARCH *search)
{
    HIT
        *hit;
        
    if (search->weighted) {
        if (search->item_hit != search->result_hits)
            search->item_hit = search->item_hit->next;
        if (search->item_hit != search->result_hits) {
            hit = (HIT *) ((char *) search->item_hit + sizeof (LIST));
            return (hit->itemid);
        }
        else
            return (0);
    }
    else {
        if (search->result_bits) {
            search->item_bit = bits_search_set (search->result_bits, search->item_bit, FALSE);
            if (search->item_bit == -1)
                search->item_bit = 0;
            return (search->item_bit);
        }
        else
            return (0);
    }
}


/*  ---------------------------------------------------------------------[<]-
    Function: search_close

    Synopsis: Releases all memory used by the search.  Do not refer to the
    search after calling this function.  Does nothing if the search argument
    is null.
    ---------------------------------------------------------------------[>]-*/

void
search_close (SEARCH *search)
{
    if (search) {
        if (search->result_bits)
            bits_destroy (search->result_bits);
        if (search->result_hits) {
            list_destroy (search->result_hits);
            mem_free (search->result_hits);
        }
        mem_free (search);
    }
}


/*  hitlist_resort -- internal
    Sorts the specified hit list by weight from highest to lowest.
    Returns the number of nodes in list1.
 */
static void
hitlist_resort (LIST *hitlist)
{
    if (hitlist)
        list_sort (hitlist, hit_compare);
}


/*  hit_compare
    Comparison function for hitlist sorting
    Returns TRUE if the two nodes need swapping
 */
static Bool
hit_compare (LIST *node1, LIST *node2)
{
    HIT
        *hit1,
        *hit2;

    hit1 = (HIT *) ((char *) node1 + sizeof (LIST));
    hit2 = (HIT *) ((char *) node2 + sizeof (LIST));
    if (hit1->weight < hit2->weight)
        return (TRUE);
    else
        return (FALSE);
}

