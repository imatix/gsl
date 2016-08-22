/*===========================================================================*
 *                                                                           *
 *  sflbits.c - Large bitmap manipulation functions                          *
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

#include "prelude.h"                    /*  Universal header file            */
#include "sflcomp.h"                    /*  Compression functions            */
#include "sfllist.h"                    /*  Linked-list functions            */
#include "sflmem.h"                     /*  Memory allocation functions      */
#include "sflmime.h"                    /*  Mime functions                   */
#include "sflbits.h"                    /*  Prototypes for functions         */


/*  Local function prototypes                                                */

static int  locate_bit            (const BITS *bits, long bit,
                                   int *index_in_bits, int *section_in_index,
                                   dbyte *bit_in_section);
static int  get_section           (BITS *bits, int index, int section,
                                   byte *buffer, Bool update);
static int  put_section           (BITS *bits, int index, int section,
                                   byte *buffer);
static int  alloc_block           (BITS *bits, int block_number);
static long calculate_buffer_size (BITS *bits);

/*  Variables used in this source by various functions                       */

static byte
    section_data  [BIT_SECTSIZE + 10],
    section_data2 [BIT_SECTSIZE + 10],
    compressed    [(BIT_SECTSIZE * 11) / 10],
    comp_ones []= {129, 255, 0, 32, 255},/* What all ones looks like         */
    comp_zero []= {128, 0, 32};         /*  What all zeroes looks like       */
static int
    comp_ones_size = 5,                 /*  Size of all zeroes, compressed   */
    comp_zero_size = 3;                 /*  Size of all zeroes, compressed   */


/*  ---------------------------------------------------------------------[<]-
    Function: bits_create

    Synopsis: Creates a new bitstring and initialises all bits to zero.
    Returns a BITS handle which you should use in all further references
    to the bitstring.
    ---------------------------------------------------------------------[>]-*/

BITS *
bits_create (void)
{
    BITS
        *bits;                          /*  Newly-created bitstring          */
    BITBLOCK
        *index;                         /*  Newly-created index block        */

    bits = mem_alloc (sizeof (BITS));
    if (bits)
      {
        memset (bits, 0, sizeof (BITS));
        index = mem_alloc (sizeof (BITBLOCK));
        if (index)
          {
            /*  Set all index fields to 0: bitstring is all zeroes           */
            memset (index, 0, sizeof (BITBLOCK));
            index-> right      = 0;
            index-> size       = BIT_DATASIZE;
            bits-> block [0]   = index;
            bits-> block_count = 1;
            bits-> free_list   = 0;     /*  No blocks in free list           */
          }
        else
          {
            mem_free (bits);
            bits = NULL;
          }
      }
    return (bits);
}


/*  ---------------------------------------------------------------------[<]-
    Function: bits_destroy

    Synopsis: Releases all memory used by a bitstring and deletes the
    bitstring.  Do not refer to the bitstring after calling this function.
    ---------------------------------------------------------------------[>]-*/

void
bits_destroy (
    BITS *bits)
{
    int
        block_nbr;                      /*  Bitstring block number           */

    ASSERT (bits);

    /*  Free all blocks allocated to bitmap                                  */
    for (block_nbr = 0; block_nbr < bits-> block_count; block_nbr++)
        mem_free (bits-> block [block_nbr]);

    mem_free (bits);
}


/*  ---------------------------------------------------------------------[<]-
    Function: bits_set

    Synopsis: Sets the specified bit in the bitmap.  Returns -1 if fail.
    ---------------------------------------------------------------------[>]-*/

int
bits_set (
    BITS *bits,
    long bit)
{
    int
        index,                          /*  Number of index block            */
        section;                        /*  Number of section in index       */
    dbyte
        bit_nbr;                        /*  Number of bit in section         */
    int
        feedback = 0;

    ASSERT (bits);

    if (bit >= 0 && bit < BIT_MAXBITS)
      {
        locate_bit  (bits, bit, &index, &section, &bit_nbr);
        get_section (bits, index, section, section_data, TRUE);
        section_data [bit_nbr / 8] |= 1 << (bit_nbr % 8);
        put_section (bits, index, section, section_data);
      }
    else
        feedback = -1;
    return (feedback);
}


/*  ---------------------------------------------------------------------[<]-
    Function: bits_clear

    Synopsis: Clears the specified bit in the bitmap.  Returns -1 if fail.
    ---------------------------------------------------------------------[>]-*/

int
bits_clear (
    BITS *bits,
    long bit)
{
    int
        index,                          /*  Number of index block            */
        section;                        /*  Number of section in index       */
    dbyte
        bit_nbr;                        /*  Number of bit in section         */
    int
        feedback = 0;


    ASSERT (bits);

    if (bit >= 0 && bit < BIT_MAXBITS)
      {
        locate_bit  (bits, bit, &index, &section, &bit_nbr);
        get_section (bits, index, section, section_data, TRUE);
        section_data [bit_nbr / 8] &= 255 - (1 << (bit_nbr % 8));
        put_section (bits, index, section, section_data);
      }
    else
        feedback = -1;
    return (feedback);
}


/*  -------------------------------------------------------------------------
 *  locate_bit -- internal
 *
 *  For a particular bit in a bitstring, finds the index block that contains
 *  the bit, and returns the index block number, the section number within
 *  the index block, and the bit position within the section.  Returns TRUE
 *  if there was no error, FALSE if the specified bit lay outside the range
 *  currently defined by the bitstring.
 */

static int
locate_bit (
    const  BITS *bits,
    long   bit,
    int   *index_in_bits,
    int   *section_in_index,
    dbyte *bit_in_section)
{
    long
        index_base,
        relative_bit;

    ASSERT (bits);
    ASSERT (bit >= 0);
    ASSERT (bit < BIT_MAXBITS);

    *index_in_bits    = 0;              /*  Index block is always 0          */
    index_base        = *index_in_bits * (long) (BIT_SECTSIZE * BIT_INDEXSIZE);
    relative_bit      = bit - index_base;
    *section_in_index = (int)   (relative_bit / ((long) BIT_SECTSIZE * 8));
    *bit_in_section   = (dbyte) (relative_bit % ((long) BIT_SECTSIZE * 8));
    return (TRUE);
}


/*  -------------------------------------------------------------------------
 *  get_section -- internal
 *
 *  Expands the specified section into the working area specified.  If the
 *  update argument is TRUE, previously allocated blocks, if any, are put
 *  onto the free list.  Use this argument if you are changing the section
 *  and will recompress it using put_section().
 */

static int
get_section (
    BITS *bits,                         /*  Bitstring to work with           */
    int   index,                        /*  Index block number               */
    int   section,                      /*  Section within index             */
    byte *buffer,                       /*  Returned buffer                  */
    Bool  update)                       /*  If TRUE, frees section blocks    */
{
    BITBLOCK
        *index_block,                   /*  Points to index block            */
        *section_block;                 /*  Points to section block          */
    dbyte
        section_head,                   /*  Section block list head          */
        block_nbr,                      /*  Entry into block table           */
        block_next;                     /*  Next block in section list       */
    static byte
        comp [BIT_SECTSIZE + 1];        /*  Section blocks' data             */
    word
        comp_size,                      /*  Size of compressed data          */
        expand_size;                    /*  Size of expanded data            */

    ASSERT (bits);
    ASSERT (buffer);

    index_block  = bits-> block [index];
    section_head = index_block-> block.index [section];
    if (section_head == 0x0000)         /*  All 0's                          */
        memset (buffer, 0x00, BIT_SECTSIZE);
    else
    if (section_head == 0xFFFF)         /*  All 1's                          */
        memset (buffer, 0xFF, BIT_SECTSIZE);
    else
      {
        block_nbr = section_head;
        comp_size = 0;                  /*  Get compressed block             */
        while (block_nbr)               /*    from 1 or more sections        */
          {
            section_block = bits-> block [block_nbr];
            ASSERT (comp_size < BIT_SECTSIZE);
            memcpy (comp + comp_size, section_block-> block.data,
                                      section_block-> size);
            comp_size += section_block-> size;
            block_next = section_block-> right;
            if (update)
              {                         /*  Move block to free list          */
                section_block-> right = bits-> free_list;
                section_block-> size  = 0;
                bits-> free_list = block_nbr;
              }
            block_nbr = block_next;
          }
        if (update)                     /*  Wipe section block list          */
            index_block-> block.index [section] = 0;

        expand_size = expand_bits (comp, buffer, comp_size);
        ASSERT (expand_size == BIT_SECTSIZE);
      }
    return 0;
}


/*  -------------------------------------------------------------------------
 *  put_section -- internal
 *
 *  Compresses the specified buffer.  This results in zero or more blocks,
 *  stored in the bitstring at the specified index and section.  If the
 *  buffer is all zeroes, the section list head has the value zero.  If the
 *  bitstring is all ones, the list head has the value 0xFFFF.  Takes blocks
 *  off the free list if possible, or from memory if necessary.  Returns 0
 *  if the operation was successful, -1 if there was an error (usually lack
 *  of memory for new blocks).
 */

static int
put_section (
    BITS *bits,                         /*  Bitstring to work with           */
    int   index,                        /*  Index block number               */
    int   section,                      /*  Section within index             */
    byte *buffer)                       /*  Buffer to compress               */
{
    BITBLOCK
        *index_block,                   /*  Points to index block            */
        *section_block,                 /*  Points to section block          */
        *section_prev;                  /*  Points to section block          */
    int
        block_nbr,                      /*  Entry into block table           */
        comp_size,                      /*  Size of compressed data          */
        copy_from;                      /*  Index into compressed data       */

    ASSERT (bits);
    ASSERT (buffer);

    /*  Compress the section and get the resulting size                      */
    index_block = bits-> block [index];
    comp_size   = compress_bits (buffer, compressed, BIT_SECTSIZE);
    ASSERT (comp_size <= BIT_SECTSIZE + 1);

    if (comp_size == comp_zero_size
    &&  memcmp (compressed, comp_zero, comp_zero_size) == 0)
        index_block-> block.index [section] = 0x0000;
    else
    if (comp_size == comp_ones_size
    &&  memcmp (compressed, comp_ones, comp_ones_size) == 0)
        index_block-> block.index [section] = 0xFFFF;
    else
      {
        section_prev = NULL;
        copy_from = 0;
        while (copy_from < comp_size)   /*  Slice comp data into blocks      */
          {
            if (bits-> free_list)       /*  Get block from free-list         */
              {                         /*    if available                   */
                block_nbr        = bits-> free_list;
                bits-> free_list = bits-> block [block_nbr]-> right;
              }
            else
              {                         /*  Allocate new block                */
                block_nbr = alloc_block (bits, -1);
                if (block_nbr < 0)      /*  If no memory left                */
                    return (-1);        /*    we give up here                */
              }
            section_block         = bits-> block [block_nbr];
            section_block-> right = 0;
            section_block-> size  = min (BIT_DATASIZE, (comp_size - copy_from));
            memcpy (section_block-> block.data, compressed + copy_from,
                    section_block-> size);

            /*  Attach block to chain                                        */
            if (section_prev)
                section_prev-> right = block_nbr;
            else
                index_block-> block.index [section] = block_nbr;

            copy_from += section_block-> size;
            section_prev = section_block;
          }
      }
    return 0;
}


/*  -------------------------------------------------------------------------
 *  alloc_block -- internal
 *
 *  Allocates a new section block (BITBLOCK bytes), and updates the bit
 *  string block table and block count accordingly.  Returns the index
 *  in the block table, or 0 if the block could not be allocated.
 *  If block_number is less than zero, use this value.
 */

static int
alloc_block (BITS *bits, int block_number)
{
    BITBLOCK
        *the_block;                     /*  Points to allocated block        */
    int
        block_nbr = -1;

    the_block = mem_alloc (sizeof (BITBLOCK));
    if (the_block)
      {
        memset (the_block, 0, sizeof (BITBLOCK));
        if (block_number < 0)
          {
            block_nbr = bits-> block_count++;
            bits-> block [block_nbr] = the_block;
          }
        else
          {
            if (bits-> block [block_number])
                mem_free (bits-> block [block_number]);
            bits-> block [block_number] = the_block;
            block_nbr = block_number;
          }
      }
    return (block_nbr);
}


/*  ---------------------------------------------------------------------[<]-
    Function: bits_test

    Synopsis: Tests the specified bit in the bitmap.  Returns 1 or 0.
    ---------------------------------------------------------------------[>]-*/

int
bits_test (
    const BITS *bits,
    long bit)
{
    int
        index,                          /*  Number of index block            */
        section;                        /*  Number of section in index       */
    dbyte
        bit_nbr;                        /*  Number of bit in section         */

    ASSERT (bits);

    locate_bit  (bits, bit, &index, &section, &bit_nbr);
    get_section ((BITS *) bits, index, section, section_data, FALSE);
    if ((section_data [bit_nbr / 8]) & (1 << (bit_nbr % 8)))
        return (1);
    else
        return (0);
}


/*  ---------------------------------------------------------------------[<]-
    Function: bits_fput

    Synopsis: Writes the bitstring to the specified file stream.  To read
    the bitstring, use the bits_fget() function.  The structure of the
    bitstring is:
    ---------------------------------------------------------------------[>]-*/

int
bits_fput (FILE *file,
    const BITS *bits)
{
    int
        block_nbr;                      /*  Bitstring block number           */
    word
        comp_size;                      /*  Size of compressed block         */
    BITBLOCK
        *block_ptr;                     /*  Points to bitstring block        */

    ASSERT (bits);
    ASSERT (file);

    /*  Write bitstring header to file                                       */
    ASSERT (fwrite (&bits-> block_count, sizeof (bits-> block_count), 1, file) == 1);
    ASSERT (fwrite (&bits-> free_list,   sizeof (bits-> free_list),   1, file) == 1);

    /*  Write bitstring blocks to file                                       */
    for (block_nbr = 0; block_nbr < bits-> block_count; block_nbr++)
      {
        block_ptr = bits-> block [block_nbr];
        ASSERT (fwrite (&block_ptr-> right, sizeof (block_ptr-> right), 1, file) == 1);
        ASSERT (fwrite (&block_ptr-> size,  sizeof (block_ptr-> size),  1, file) == 1);

        comp_size = compress_block (block_ptr-> block.data,
                                    compressed, (word)BIT_DATASIZE);

        ASSERT (fwrite (&comp_size, sizeof (comp_size), 1, file) == 1);
        ASSERT (fwrite (compressed,         comp_size,  1, file) == 1);
      }
    return 0;
}


/*  ---------------------------------------------------------------------[<]-
    Function: bits_fget

    Synopsis: Reads a bitstring from the specified file stream.  You must
    have previously written the bitstring using bits_fput ().  Returns a
    newly-created bitmap, or NULL if there was insufficient memory.
    ---------------------------------------------------------------------[>]-*/

BITS *
bits_fget (FILE *file)
{
    int
        block_nbr;                      /*  Bitstring block number           */
    word
        comp_size = 0;                  /*  Size of compressed block         */
    BITBLOCK
        *block_ptr;                     /*  Points to bitstring block        */
    BITS
        *bits;

    ASSERT (file);

    bits = bits_create ();              /*  Create a new, empty bitmap       */

    /*  Read bitstring header from file                                      */
    ASSERT (fread (&bits-> block_count, sizeof (bits-> block_count), 1, file) == 1);
    ASSERT (fread (&bits-> free_list,   sizeof (bits-> free_list),   1, file) == 1);

    /*  Read bitstring blocks from file                                      */
    for (block_nbr = 0; block_nbr < bits-> block_count; block_nbr++)
      {
        block_nbr = alloc_block (bits, block_nbr);
        if (block_nbr < 0)
          {
            bits_destroy (bits);
            return (NULL);
          }
        block_ptr        = bits-> block [block_nbr];
        ASSERT (fread (&block_ptr-> right, sizeof (block_ptr-> right), 1, file) == 1);
        ASSERT (fread (&block_ptr-> size,  sizeof (block_ptr-> size),  1, file) == 1);
        ASSERT (fread (&comp_size,         sizeof (comp_size),         1, file) == 1);
        ASSERT (fread (compressed,         comp_size,                  1, file) == 1);
        expand_block (compressed, block_ptr-> block.data, comp_size);
      }
    return (bits);
}

/*  -------------------------------------------------------------------------
    Function: calculate_buffer_size - internal

    Synopsis: Calculate the size of buffer to serialize bitmap into a byte
              buffer
    -------------------------------------------------------------------------*/

static long 
calculate_buffer_size (BITS *bits)
{
    int
        block_nbr;                      /*  Bitstring block number           */
    word
        comp_size;                      /*  Size of compressed block         */
    BITBLOCK
        *block_ptr;                     /*  Points to bitstring block        */
    long
        feedback = 0;                   /*  Calulated value to return        */

    ASSERT (bits);

   
    feedback += sizeof (bits-> block_count);
    feedback += sizeof (bits-> free_list);

    /*  Write bitstring blocks to file                                       */
    for (block_nbr = 0; block_nbr < bits-> block_count; block_nbr++)
      {
        block_ptr = bits-> block [block_nbr];
        comp_size = compress_block (block_ptr-> block.data,
                                    compressed, (word)BIT_DATASIZE);
        feedback += sizeof (block_ptr-> right);
        feedback += sizeof (block_ptr-> size);
        feedback += sizeof (comp_size) + comp_size;
      }
    return (feedback);
}


/*  ---------------------------------------------------------------------[<]-
    Function: bits_save

    Synopsis: Returns a string holding the bitstring in a printable format
    (base64 encoded).  The caller must free the returned string when
    finished.  On errors, returns null.
    ---------------------------------------------------------------------[>]-*/

char *
bits_save (BITS *bits)
{
    int
        block_nbr;                      /*  Bitstring block number           */
    word
        comp_size;                      /*  Size of compressed block         */
    BITBLOCK
        *block_ptr;                     /*  Points to bitstring block        */
    byte
        *buffer = NULL,                 /*  Stream buffer                    */
        *position;                      /*  Current position in the stream   */
    long
        size;
    char
        *encoded = NULL;

    ASSERT (bits);

    size = calculate_buffer_size (bits);
    buffer = mem_alloc (size);
    if (buffer)
      {
        position = buffer;
        /*  Write bitstring header to stream                                 */
        *((int *)position) = bits-> block_count;
        position += sizeof (bits-> block_count);

        *((dbyte *)position) = bits-> free_list;
        position += sizeof (bits-> free_list);

        /*  Write bitstring blocks to stream                                 */
        for (block_nbr = 0; block_nbr < bits-> block_count; block_nbr++)
          {
            block_ptr = bits-> block [block_nbr];
            comp_size = compress_block (block_ptr-> block.data,
                                    compressed, (word)BIT_DATASIZE);

            *((dbyte *)position) = block_ptr-> right;
            position += sizeof (block_ptr-> right);

            *((int *)position) = block_ptr-> size;
            position += sizeof (block_ptr-> size);

            *((word *)position) = comp_size;
            position += sizeof (word);
            memcpy (position, compressed, comp_size);
            position += comp_size;
          }
        /* Encode binary buffer in base64                                    */
        encoded = mem_alloc ((long)((double) size * 1.5) + 2);
        if (encoded)
            encode_base64 (buffer, encoded, size);

        mem_free (buffer);
      }
    return (encoded);
}


/*  ---------------------------------------------------------------------[<]-
    Function: bits_load

    Synopsis: Creates a bitstring (BITS structure) from an encoded bitstring
    built using bits_save().  
    ---------------------------------------------------------------------[>]-*/

BITS *
bits_load (const char *buffer)
{
    int
        block_nbr;                      /*  Bitstring block number           */
    word
        comp_size;                      /*  Size of compressed block         */
    BITBLOCK
        *block_ptr;                     /*  Points to bitstring block        */
    BITS
        *bits;
    byte
        *buf,
        *position;                      /*  Current position in the stream   */

    buf = (byte *) mem_strdup (buffer);
    if (buf == NULL)
        return (NULL);

    bits = bits_create ();              /*  Create a new, empty bitmap       */
    decode_base64 (buffer, buf, strlen ((char *) buf) + 1);

    position = (byte *) buf;

    /*  Read bitstring header from buffer                                    */
    bits-> block_count = *((int *) position);
    position += sizeof (bits-> block_count);

    bits-> free_list = *((dbyte *) position);
    position += sizeof (bits-> free_list);

    /*  Read bitstring blocks from file                                      */
    for (block_nbr = 0; block_nbr < bits-> block_count; block_nbr++)
      {
        block_nbr = alloc_block (bits, block_nbr);
        if (block_nbr < 0)
          {
            bits_destroy (bits);
            return (NULL);
          }
        block_ptr = bits-> block [block_nbr];

        block_ptr-> right = *((dbyte *)position);
        position += sizeof (block_ptr-> right);

        block_ptr-> size = *((int *)position);
        position += sizeof (block_ptr-> size);

        comp_size = *((word *)position);
        position += sizeof (word);
        memcpy (compressed, position, comp_size);
        position += comp_size;

        expand_block (compressed, block_ptr-> block.data, comp_size);
      }
    mem_free (buf);
    return (bits);
}


/*  ---------------------------------------------------------------------[<]-
    Function: bits_search_set

    Synopsis: Search the next bit set to 1 after the bit parameter.
              If reverse is set to TRUE, search from bit to the beginning
              Return -1 if search end with no result.
    ---------------------------------------------------------------------[>]-*/

long  
bits_search_set (const BITS *bits, long bit, Bool reverse)
{
    long
         current_bit,
         feedback = -1;
    int
        index,                          /*  Number of index block            */
        section,                        /*  Number of section in index       */
        data_index, 
        bit_index;
    dbyte
        bit_nbr;                        /*  Number of bit in section         */

    BITBLOCK
        *index_block;                   /*  Points to index block            */
    dbyte
        section_head;                   /*  Section block list head          */

    ASSERT (bits);

    if (reverse == FALSE)
      {
        current_bit = bit + 1;
        locate_bit (bits, current_bit, &index, &section, &bit_nbr);
        index_block = bits-> block [index];
        FOREVER
          {
            section_head = index_block-> block.index [section];
            if (section_head == 0x0000)         /*  All 0's                          */
              {
                while (   index_block-> block.index [section] == 0x0000
                       && section < BIT_INDEXSIZE)
                    section++;
                if (section == BIT_INDEXSIZE)
                    break;
                else
                    section_head = index_block-> block.index [section];
              }

            if (section_head == 0xFFFF)     /*  All 1's                          */
              {
                feedback = current_bit;
                break;
              }
            else
              {
                get_section ((BITS *)bits, index, section, section_data, FALSE);
                data_index = bit_nbr / 8;
                for (bit_index = bit_nbr % 8; bit_index < 8; bit_index++, bit_nbr++)
                  {
                    if ((section_data [data_index]) & (1 << bit_index))
                      {
                         feedback = (((long) BIT_SECTSIZE * 8) * section) + bit_nbr;
                         break;
                      }
                  }

                if (feedback != -1)
                    break;

                data_index++;
                while (   section_data [data_index] == 0x00
                       && data_index < BIT_SECTSIZE)
                    data_index++;
                if (data_index == BIT_SECTSIZE)
                  {
                    section++;
                    bit_nbr = 0;
                  }
                else
                  {
                    for (bit_index = 0; bit_index < 8; bit_index++, bit_nbr++)
                      {
                        if ((section_data [data_index]) & (1 << bit_index))
                          {
                             feedback = (((long) BIT_SECTSIZE * 8) * section) + 
                                        ( 8 * data_index) + bit_index;
                             break;
                          }
                      }
                  }
                if (feedback != -1)
                    break;
                current_bit = (((long) BIT_SECTSIZE * 8) * section) + bit_nbr;
             }
          }
       }
     else
       {
        current_bit = bit - 1;
        locate_bit (bits, current_bit, &index, &section, &bit_nbr);
        index_block = bits-> block [index];
        FOREVER
          {
            section_head = index_block-> block.index [section];
            if (section_head == 0x0000)         /*  All 0's                          */
              {
                while (   index_block-> block.index [section] == 0x0000
                       && section >= 0)
                    section--;
                if (section < 0)
                    break;
                else
                    section_head = index_block-> block.index [section];
              }

            if (section_head == 0xFFFF)     /*  All 1's                          */
              {
                feedback = current_bit;
                break;
              }
            else
              {
                get_section ((BITS *)bits, index, section, section_data, FALSE);
                data_index = bit_nbr / 8;
                for (bit_index = (bit_nbr % 8); bit_index >= 0 && bit_index < 8; bit_index--, bit_nbr--)
                  {
                    if ((section_data [data_index]) & (1 << bit_index))
                      {
                         feedback = (((long) BIT_SECTSIZE * 8) * section) + bit_nbr;
                         break;
                      }
                  }

                if (feedback != -1)
                    break;

                data_index--;
                while (   data_index >= 0 
                       && data_index < BIT_SECTSIZE
                       && section_data [data_index] == 0x00)
                    data_index--;
                if (data_index < 0 || data_index > BIT_SECTSIZE)
                  {
                    section--;
                    bit_nbr = (BIT_SECTSIZE * 8) - 1;
                    if (section < 0)
                        break;
                  }
                else
                  {
                    for (bit_index = 7; bit_index >= 0; bit_index--, bit_nbr--)
                      {
                        if ((section_data [data_index]) & (1 << bit_index))
                          {
                             feedback = (((long) BIT_SECTSIZE * 8) * section) + 
                                        ( 8 * data_index) + bit_index;
                             break;
                          }
                      }
                  }
                if (feedback != -1)
                    break;
                current_bit = (((long) BIT_SECTSIZE * 8) * section) + bit_nbr;
             }
          }
       }
    return (feedback);
}


/*  ---------------------------------------------------------------------[<]-
    Function: bits_and

    Synopsis: Apply a logical AND between two bitstrings and return the result.
    ---------------------------------------------------------------------[>]-*/

BITS *
bits_and (const BITS *bits1, const BITS *bits2)
{
    BITS
        *bits = NULL;
    int
        section;
    BITBLOCK
        *index_block1,                   /*  Points to index block            */
        *index_block2,                   /*  Points to index block            */
        *index_block3;                   /*  Points to index block            */
    register long
        *data1,
        *data1_end,
        *data2;

    ASSERT (bits1);
    ASSERT (bits2);

    bits = bits_create ();              /*  Create a new, empty bitmap       */

    index_block1  = bits1-> block [0];
    index_block2  = bits2-> block [0];
    index_block3  = bits-> block [0];

    for (section = 0; section < BIT_INDEXSIZE; section++)
      {
        if (index_block1-> block.index [section] == 0xFFFF
        &&  index_block2-> block.index [section] == 0xFFFF)
            index_block3-> block.index [section] = 0xFFFF;
        else
        if (index_block1-> block.index [section] == 0x0000
        ||  index_block2-> block.index [section] == 0x0000)
            index_block3-> block.index [section] = 0x0000;
        else
          {
            get_section ((BITS *)bits1, 0, section, section_data,  FALSE);
            get_section ((BITS *)bits2, 0, section, section_data2, FALSE);
            data1_end = (long *)section_data + (BIT_SECTSIZE / sizeof (long) + 1);
            data1 = (long *)section_data;
            data2 = (long *)section_data2;
            while (data1 < data1_end)
                *data1++ &= *data2++;
            put_section (bits, 0, section, section_data);
          }
      }
    return (bits);
}


/*  ---------------------------------------------------------------------[<]-
    Function: bits_or

    Synopsis: Apply a logical OR between two bitstrings and return the result.
    ---------------------------------------------------------------------[>]-*/

BITS *
bits_or (const BITS *bits1, const BITS *bits2)
{
    BITS
        *bits = NULL;
    int
        section;
    BITBLOCK
        *index_block1,                   /*  Points to index block            */
        *index_block2,                   /*  Points to index block            */
        *index_block3;                   /*  Points to index block            */
    register long
        *data1,
        *data1_end,
        *data2;

    ASSERT (bits1);
    ASSERT (bits2);

    bits = bits_create ();              /*  Create a new, empty bitmap       */

    index_block1  = bits1-> block [0];
    index_block2  = bits2-> block [0];
    index_block3  = bits-> block [0];

    for (section = 0; section < BIT_INDEXSIZE; section++)
      {
        if (index_block1-> block.index [section] == 0xFFFF
        ||  index_block2-> block.index [section] == 0xFFFF)
            index_block3-> block.index [section] = 0xFFFF;
        else
        if (index_block1-> block.index [section] == 0x0000
        &&  index_block2-> block.index [section] == 0x0000)
            index_block3-> block.index [section] = 0x0000;
        else
        if (index_block1-> block.index [section] == 0x0000
        ||  index_block2-> block.index [section] == 0x0000)
          {
            if (index_block1-> block.index [section] == 0x0000)
                get_section ((BITS *)bits2, 0, section, section_data,  FALSE);
            else
                get_section ((BITS *)bits1, 0, section, section_data,  FALSE);
            put_section (bits, 0, section, section_data);
          }
        else
          {
            get_section ((BITS *)bits1, 0, section, section_data,  FALSE);
            get_section ((BITS *)bits2, 0, section, section_data2, FALSE);
            data1_end = (long *)section_data + (BIT_SECTSIZE / sizeof (long) + 1);
            data1 = (long *)section_data;
            data2 = (long *)section_data2;
            while (data1 < data1_end)
                *data1++ |= *data2++;
            put_section (bits, 0, section, section_data);
          }
      }
    return (bits);
}


/*  ---------------------------------------------------------------------[<]-
    Function: bits_xor

    Synopsis: Apply a logical XOR between two bitstrings and return the result.
    ---------------------------------------------------------------------[>]-*/

BITS *
bits_xor (const BITS *bits1, const BITS *bits2)
{
    BITS
        *bits = NULL;
    int
        section;
    BITBLOCK
        *index_block1,                   /*  Points to index block            */
        *index_block2,                   /*  Points to index block            */
        *index_block3;                   /*  Points to index block            */
    register long
        *data1,
        *data1_end,
        *data2;

    ASSERT (bits1);
    ASSERT (bits2);

    bits = bits_create ();              /*  Create a new, empty bitmap       */

    index_block1  = bits1-> block [0];
    index_block2  = bits2-> block [0];
    index_block3  = bits-> block [0];

    for (section = 0; section < BIT_INDEXSIZE; section++)
      {
        if (index_block1-> block.index [section] == 0xFFFF
        &&  index_block2-> block.index [section] == 0xFFFF)
            index_block3-> block.index [section] = 0x0000;
        else
        if (index_block1-> block.index [section] == 0x0000
        &&  index_block2-> block.index [section] == 0x0000)
            index_block3-> block.index [section] = 0x0000;
        else
          {
            get_section ((BITS *)bits1, 0, section, section_data,  FALSE);
            get_section ((BITS *)bits2, 0, section, section_data2, FALSE);
            data1_end = (long *)section_data + (BIT_SECTSIZE / sizeof (long) + 1);
            data1 = (long *)section_data;
            data2 = (long *)section_data2;
            while (data1 < data1_end)
                *data1++ ^= *data2++;
            put_section (bits, 0, section, section_data);
          }
      }
    return (bits);
}


/*  ---------------------------------------------------------------------[<]-
    Function: bits_invert

    Synopsis: Logical Invert bitstring
    ---------------------------------------------------------------------[>]-*/

BITS *
bits_invert (const BITS *bits)
{
    BITS
        *feedback = NULL;
    int
        section;
    BITBLOCK
        *target_block,                  /*  Points to index block            */
        *index_block;                   /*  Points to index block            */
    register long
        *data,
        *data_end;

    ASSERT (bits);

    feedback = bits_create ();              /*  Create a new, empty bitmap       */

    index_block   = bits-> block [0];
    target_block  = feedback-> block [0];

    for (section = 0; section < BIT_INDEXSIZE; section++)
      {
        if (index_block-> block.index [section] == 0xFFFF)
            target_block-> block.index [section] = 0x0000;
        else
        if (index_block-> block.index [section] == 0x0000)
            target_block-> block.index [section] = 0xFFFF;
        else
          {
            get_section ((BITS *)bits, 0, section, section_data,  FALSE);
            data_end = (long *)section_data + (BIT_SECTSIZE / sizeof (long) + 1);
            data = (long *)section_data;
            while (data < data_end)
              {
                *data = ~*data;
                ++data;
              }
            put_section (feedback, 0, section, section_data);
          }
      }
    return (feedback);
}


/*  ---------------------------------------------------------------------[<]-
    Function: bits_set_count

    Synopsis: count the numbers of bits set to 1.
    ---------------------------------------------------------------------[>]-*/

long
bits_set_count (const BITS *bits)
{
    long
        count = 0;
    register int
        data_index,
        bit_index,
        section;
    BITBLOCK
        *index_block;                   /*  Points to index block            */

    ASSERT (bits);

    index_block   = bits-> block [0];

    for (section = 0; section < BIT_INDEXSIZE; section++)
      {
        if (index_block-> block.index [section] == 0xFFFF)
            count += BIT_SECTSIZE * 8;
        else
        if (index_block-> block.index [section] != 0x0000)
          {
            get_section ((BITS *)bits, 0, section, section_data, FALSE);
            for (data_index = 0; data_index < BIT_SECTSIZE; data_index++)
              {
                if (section_data [data_index])
                  {
                    for (bit_index = 0; bit_index < 8; bit_index++)
                      {
                        if ((section_data [data_index]) & (1 << bit_index))
                            count++;
                      }
                  }
              }
          }
      }
    return (count);
}
