/*===========================================================================*
 *                                                                           *
 *  sflmath.c - Mathematical functions                                       *
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
#include "sflmath.h"                    /*  Function prototypes              */
#include "sfllist.h"                    /*  List management functions        */
#include "sflmem.h"                     /*  Memory-management functions      */


/*  ---------------------------------------------------------------------[<]-
    Function: point_in_rect

    Synopsis: Checks if the requested FPOINT is within the specified
              rectangle.  Returns TRUE or FALSE appropriately.
    ---------------------------------------------------------------------[>]-*/

int
point_in_rect (const FPOINT *point, const FPOINT *coords)
{
    return ((point-> x >= coords [0].x && point-> x <= coords [1].x)
         && (point-> y >= coords [0].y && point-> y <= coords [1].y));
}


/*  ---------------------------------------------------------------------[<]-
    Function: point_in_circle

    Synopsis: Checks if the requested FPOINT is within the specified
              circle.  Returns TRUE or FALSE appropriately.
    ---------------------------------------------------------------------[>]-*/

int
point_in_circle (const FPOINT *point, const FPOINT *coords)
{
    double
        circle_radius,
        distance_from_centre;

    circle_radius        = ((coords [0].y - coords [1].y) *
                            (coords [0].y - coords [1].y)) +
                           ((coords [0].x - coords [1].x)  *
                            (coords [0].x - coords [1].x));

    distance_from_centre = ((coords [0].y - point-> y) *
                            (coords [0].y - point-> y)) +
                           ((coords [0].x - point-> x) *
                            (coords [0].x - point-> x));
    return (distance_from_centre <= circle_radius);
}


/*  ---------------------------------------------------------------------[<]-
    Function: point_in_poly

    Synopsis: Checks if the requested FPOINT is within the specified
              polygon.  Returns TRUE or FALSE.
    ---------------------------------------------------------------------[>]-*/

int
point_in_poly (const FPOINT *point, const FPOINT *pgon, int nbpoints)
{
    int
        inside_flag,
        xflag0,
        crossings;
    const double
        *stop;
    double
        *p,
        tx,
        ty,
        y;

    crossings = 0;

    tx = point-> x;
    ty = point-> y;
    y  = pgon [nbpoints - 1].y;

    p = (double *) pgon + 1;
    if ((y >= ty) != (*p >= ty))
      {
        if ((xflag0 = (pgon [nbpoints - 1].x >= tx)) ==
                (*(double *) pgon >= tx))
          {
            if (xflag0)
               crossings++;
          }
        else
            crossings += (pgon [nbpoints - 1].x - (y - ty) *
                         (*(double *) pgon - pgon [nbpoints - 1].x) /
                         (*p - y)) >= tx;
      }
     stop = &pgon [nbpoints].y;
     for (y = *p, p += 2; p <= stop; y = *p, p += 2)
       {
         if (y >= ty)
           {
             while ((p < stop) && (*p >= ty))
                 p += 2;
             if (p >= stop)
                break;
             if ((xflag0 = (*(p - 3) >= tx)) == (*(p - 1) >= tx))
               {
                 if (xflag0)
                    crossings++;
               }
             else
                 crossings += (*(p - 3) - (*(p - 2) - ty) *
                              (*(p - 1) - *(p - 3)) /
                              (*p - *(p - 2))) >= tx;
           }
         else
           {
             while ((p < stop) && (*p < ty))
                p += 2;
             if (p >= stop)
                break;
             if ((xflag0 = (*(p - 3) >= tx)) == (*(p - 1) >= tx))
               {
                 if (xflag0)
                    crossings++;
               }
             else
                 crossings += (*(p - 3) - (*(p - 2) - ty) *
                              (*(p - 1) -  *(p - 3)) /
                              (*p - *(p - 2))) >= tx;
           }
       }
     inside_flag = crossings & 0x01;
     return (inside_flag);
}


/*  These statistics functions still need to be polished - PH-2002-0505
 */

typedef struct _STAT_DATA {           
    struct _STAT_DATA
        *next,                          /*  Next item in list                */
        *prev;                          /*  Previous item in list            */
    int
        value;
} STAT_DATA;


struct _STAT_STRUCT {
    struct _STAT_DATA  *data_list;
    unsigned int        data_count;
    double              mean;
    double              sd;
    Bool                data_changed;
};


static void update_computed_values (STAT_STRUCT *stat_struct);
static STAT_DATA  *get_stat_data   (STAT_STRUCT *stat_struct, unsigned int index);


STAT_STRUCT *stat_create (void)
{
    STAT_STRUCT *stat_struct = NULL;
    STAT_DATA   *data_list = NULL;

    stat_struct = mem_alloc (sizeof(STAT_STRUCT));
    ASSERT (stat_struct != NULL);

    list_create (data_list, sizeof (STAT_DATA));
    ASSERT (data_list != NULL);

    stat_struct-> data_count = 0;
    stat_struct-> data_list  = data_list;

    stat_struct-> data_changed = TRUE;

    return stat_struct;
}


void stat_free (STAT_STRUCT *stat_struct)
{
    STAT_DATA *data_list = NULL;

    if (stat_struct == NULL)
        return;

    data_list = stat_struct-> data_list;

    list_destroy (data_list);
    mem_free     (data_list);
    mem_free     (stat_struct);
}


void stat_add_datum (STAT_STRUCT *stat_struct, int datum)
{
    STAT_DATA *new_data  = NULL;

    ASSERT (stat_struct != NULL);

    list_create (new_data, sizeof (STAT_DATA));
    ASSERT (new_data != NULL);

    new_data-> value = datum;

    list_relink_before (new_data, stat_struct-> data_list);

    stat_struct-> data_changed = TRUE;
}


void stat_release_data   (STAT_STRUCT *stat_struct)
{
    ASSERT (stat_struct != NULL);

    list_destroy (stat_struct-> data_list);
    stat_struct-> data_count = 0;

    stat_struct-> data_changed = TRUE;
}



int stat_data_count (STAT_STRUCT *stat_struct)
{
    ASSERT (stat_struct != NULL);

    update_computed_values (stat_struct);

    ASSERT (!stat_struct-> data_changed);

    return stat_struct-> data_count;
}


double stat_mean (STAT_STRUCT *stat_struct)
{
    ASSERT (stat_struct != NULL);

    update_computed_values (stat_struct);

    ASSERT (!stat_struct-> data_changed);

    return stat_struct-> mean;
}


double stat_standard_deviation (STAT_STRUCT *stat_struct)
{
    ASSERT (stat_struct != NULL);

    update_computed_values (stat_struct);

    ASSERT (!stat_struct-> data_changed);

    return stat_struct-> sd;
}


double compute_data_sd_factor (STAT_STRUCT *stat_struct, int index)
{
    STAT_DATA *stat_data = NULL;
    double datum;
    double res;

    ASSERT (stat_struct != NULL);

    update_computed_values (stat_struct);

    stat_data = get_stat_data (stat_struct, index);
    ASSERT (stat_data != NULL);

    datum = (double)stat_data-> value;

    res = (datum - stat_struct-> mean) / stat_struct-> sd;

    return res;
}



static
void update_computed_values (STAT_STRUCT *stat_struct)
{
    STAT_DATA *data;
    STAT_DATA *data_list;
    unsigned int count = 0;
    double mean = -1.0;
    double sd   = -1.0;
    long total;

    ASSERT (stat_struct != NULL);
    if (!stat_struct-> data_changed)
        return;

    data_list = stat_struct-> data_list;
    ASSERT (data_list != NULL);

    /* we compute data count. must be done before computing mean */
    FORLIST (data, *data_list)
        count++;

    if (count > 0)
      {
        /* we compute the mean. Must be done BEFORE computing sd */
        /* Formula: 1/N * SUM (Xi) */
        total = 0;
        FORLIST (data, *data_list)
            total += data-> value;

        mean = (double)total / (double)count;

        /* we compute standard deviation. */
        /* Formula: [1/N * SUM (Xi * Xi)] - mean*mean  == variance == sd * sd */
        total = 0;
        FORLIST (data, *data_list)
            total += data-> value * data-> value;

        sd = (double)total / (double)count;
        sd -= mean * mean;
        sd = sqrt (sd);
      }

    stat_struct-> data_count = count;
    stat_struct-> mean = mean;
    stat_struct-> sd   = sd;

    stat_struct-> data_changed = FALSE;
}



static
STAT_DATA *get_stat_data (STAT_STRUCT *stat_struct, unsigned int index)
{
    STAT_DATA *res       = NULL;
    STAT_DATA *data_list = NULL;
    STAT_DATA *current   = NULL;

    unsigned int idx = 0;

    ASSERT (stat_struct != NULL);

    data_list = stat_struct-> data_list;
    ASSERT (data_list != NULL);

    /* we compute data count. must be done before computing mean */
    FORLIST (current, *data_list)
      {
        if (idx == index)
          {
            res = current;
            break;
          }
        idx++;
      }

    return res;
}

static qbyte crc_table [] = {
    0x00000000L, 0x77073096L, 0xEE0E612CL, 0x990951BAL,
    0x076DC419L, 0x706AF48FL, 0xE963A535L, 0x9E6495A3L,
    0x0EDB8832L, 0x79DCB8A4L, 0xE0D5E91EL, 0x97D2D988L,
    0x09B64C2BL, 0x7EB17CBDL, 0xE7B82D07L, 0x90BF1D91L,
    0x1DB71064L, 0x6AB020F2L, 0xF3B97148L, 0x84BE41DEL,
    0x1ADAD47DL, 0x6DDDE4EBL, 0xF4D4B551L, 0x83D385C7L,
    0x136C9856L, 0x646BA8C0L, 0xFD62F97AL, 0x8A65C9ECL,
    0x14015C4FL, 0x63066CD9L, 0xFA0F3D63L, 0x8D080DF5L,
    0x3B6E20C8L, 0x4C69105EL, 0xD56041E4L, 0xA2677172L,
    0x3C03E4D1L, 0x4B04D447L, 0xD20D85FDL, 0xA50AB56BL,
    0x35B5A8FAL, 0x42B2986CL, 0xDBBBC9D6L, 0xACBCF940L,
    0x32D86CE3L, 0x45DF5C75L, 0xDCD60DCFL, 0xABD13D59L,
    0x26D930ACL, 0x51DE003AL, 0xC8D75180L, 0xBFD06116L,
    0x21B4F4B5L, 0x56B3C423L, 0xCFBA9599L, 0xB8BDA50FL,
    0x2802B89EL, 0x5F058808L, 0xC60CD9B2L, 0xB10BE924L,
    0x2F6F7C87L, 0x58684C11L, 0xC1611DABL, 0xB6662D3DL,
    0x76DC4190L, 0x01DB7106L, 0x98D220BCL, 0xEFD5102AL,
    0x71B18589L, 0x06B6B51FL, 0x9FBFE4A5L, 0xE8B8D433L,
    0x7807C9A2L, 0x0F00F934L, 0x9609A88EL, 0xE10E9818L,
    0x7F6A0DBBL, 0x086D3D2DL, 0x91646C97L, 0xE6635C01L,
    0x6B6B51F4L, 0x1C6C6162L, 0x856530D8L, 0xF262004EL,
    0x6C0695EDL, 0x1B01A57BL, 0x8208F4C1L, 0xF50FC457L,
    0x65B0D9C6L, 0x12B7E950L, 0x8BBEB8EAL, 0xFCB9887CL,
    0x62DD1DDFL, 0x15DA2D49L, 0x8CD37CF3L, 0xFBD44C65L,
    0x4DB26158L, 0x3AB551CEL, 0xA3BC0074L, 0xD4BB30E2L,
    0x4ADFA541L, 0x3DD895D7L, 0xA4D1C46DL, 0xD3D6F4FBL,
    0x4369E96AL, 0x346ED9FCL, 0xAD678846L, 0xDA60B8D0L,
    0x44042D73L, 0x33031DE5L, 0xAA0A4C5FL, 0xDD0D7CC9L,
    0x5005713CL, 0x270241AAL, 0xBE0B1010L, 0xC90C2086L,
    0x5768B525L, 0x206F85B3L, 0xB966D409L, 0xCE61E49FL,
    0x5EDEF90EL, 0x29D9C998L, 0xB0D09822L, 0xC7D7A8B4L,
    0x59B33D17L, 0x2EB40D81L, 0xB7BD5C3BL, 0xC0BA6CADL,
    0xEDB88320L, 0x9ABFB3B6L, 0x03B6E20CL, 0x74B1D29AL,
    0xEAD54739L, 0x9DD277AFL, 0x04DB2615L, 0x73DC1683L,
    0xE3630B12L, 0x94643B84L, 0x0D6D6A3EL, 0x7A6A5AA8L,
    0xE40ECF0BL, 0x9309FF9DL, 0x0A00AE27L, 0x7D079EB1L,
    0xF00F9344L, 0x8708A3D2L, 0x1E01F268L, 0x6906C2FEL,
    0xF762575DL, 0x806567CBL, 0x196C3671L, 0x6E6B06E7L,
    0xFED41B76L, 0x89D32BE0L, 0x10DA7A5AL, 0x67DD4ACCL,
    0xF9B9DF6FL, 0x8EBEEFF9L, 0x17B7BE43L, 0x60B08ED5L,
    0xD6D6A3E8L, 0xA1D1937EL, 0x38D8C2C4L, 0x4FDFF252L,
    0xD1BB67F1L, 0xA6BC5767L, 0x3FB506DDL, 0x48B2364BL,
    0xD80D2BDAL, 0xAF0A1B4CL, 0x36034AF6L, 0x41047A60L,
    0xDF60EFC3L, 0xA867DF55L, 0x316E8EEFL, 0x4669BE79L,
    0xCB61B38CL, 0xBC66831AL, 0x256FD2A0L, 0x5268E236L,
    0xCC0C7795L, 0xBB0B4703L, 0x220216B9L, 0x5505262FL,
    0xC5BA3BBEL, 0xB2BD0B28L, 0x2BB45A92L, 0x5CB36A04L,
    0xC2D7FFA7L, 0xB5D0CF31L, 0x2CD99E8BL, 0x5BDEAE1DL,
    0x9B64C2B0L, 0xEC63F226L, 0x756AA39CL, 0x026D930AL,
    0x9C0906A9L, 0xEB0E363FL, 0x72076785L, 0x05005713L,
    0x95BF4A82L, 0xE2B87A14L, 0x7BB12BAEL, 0x0CB61B38L,
    0x92D28E9BL, 0xE5D5BE0DL, 0x7CDCEFB7L, 0x0BDBDF21L,
    0x86D3D2D4L, 0xF1D4E242L, 0x68DDB3F8L, 0x1FDA836EL,
    0x81BE16CDL, 0xF6B9265BL, 0x6FB077E1L, 0x18B74777L,
    0x88085AE6L, 0xFF0F6A70L, 0x66063BCAL, 0x11010B5CL,
    0x8F659EFFL, 0xF862AE69L, 0x616BFFD3L, 0x166CCF45L,
    0xA00AE278L, 0xD70DD2EEL, 0x4E048354L, 0x3903B3C2L,
    0xA7672661L, 0xD06016F7L, 0x4969474DL, 0x3E6E77DBL,
    0xAED16A4AL, 0xD9D65ADCL, 0x40DF0B66L, 0x37D83BF0L,
    0xA9BCAE53L, 0xDEBB9EC5L, 0x47B2CF7FL, 0x30B5FFE9L,
    0xBDBDF21CL, 0xCABAC28AL, 0x53B39330L, 0x24B4A3A6L,
    0xBAD03605L, 0xCDD70693L, 0x54DE5729L, 0x23D967BFL,
    0xB3667A2EL, 0xC4614AB8L, 0x5D681B02L, 0x2A6F2B94L,
    0xB40BBE37L, 0xC30C8EA1L, 0x5A05DF1BL, 0x2D02EF8DL
};


/*  ---------------------------------------------------------------------[<]-
    Function: calculate_crc

    Synopsis: Calculates the 32-bit CCITT CRC for a memory block.  The CRC
    calculation is rapid, since the function uses a pre-calculated table.
    Returns the 32-bit CRC.
    ---------------------------------------------------------------------[>]-*/

qbyte
calculate_crc (byte *block, size_t length)
{
    size_t
        offset;
    word
        this_word;
    qbyte
        crc_value;                      /*  Running CRC value                */

    crc_value = 0xFFFFFFFFL;
    for (offset = 0; offset < length; offset++)
    {
        this_word = block [offset];
        this_word = this_word ^ (dbyte) (crc_value & 255);
        crc_value = (crc_value >> 8) ^ crc_table [this_word];
    }
    return (crc_value ^ 0xFFFFFFFFL);
}


/*  ---------------------------------------------------------------------[<]-
    Function: calculate_continuing_crc

    Synopsis: Calculates the 32-bit CCITT CRC for a memory block on a 
    continuing basis.  The first time it is called the CRC should be
    pre-initialised to zero.  The function may be called as many times
    as necessary, at the end of which the CRC will have been calculated
    for the concatenation of the successive memory blocks.  The CRC
    calculation is rapid, since the function uses a pre-calculated table.
    ---------------------------------------------------------------------[>]-*/

void
calculate_continuing_crc (byte *block, size_t length, qbyte *crc_value)
{
    size_t
        offset;
    word
        this_word;

    *crc_value ^= 0xFFFFFFFFL;
    for (offset = 0; offset < length; offset++)
    {
        this_word = block [offset];
        this_word = this_word ^ (dbyte) (*crc_value & 255);
        *crc_value = (*crc_value >> 8) ^ crc_table [this_word];
    }
    *crc_value ^= 0xFFFFFFFFL;
}
