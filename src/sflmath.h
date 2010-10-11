/*===========================================================================*
 *                                                                           *
 *  sflmath.h - Mathematical functions                                       *
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
    Synopsis:   Provides miscellaneous mathematical functions, including
                calculation of points within areas, and CRC calculation.
 ------------------------------------------------------------------</Prolog>-*/

#ifndef SFLMATH_INCLUDED               /*  Allow multiple inclusions        */
#define SFLMATH_INCLUDED

/*  Structure declarations                                                  */

typedef struct _STAT_STRUCT STAT_STRUCT;

typedef struct
  {
    double x;
    double y;
  } FPOINT;

/*  Function prototypes                                                      */

#ifdef __cplusplus
extern "C" {
#endif

STAT_STRUCT *
        stat_create             (void);
void    stat_free               (STAT_STRUCT *stat_struct);
void    stat_add_datum          (STAT_STRUCT *stat_struct, int datum);
void    stat_release_data       (STAT_STRUCT *stat_struct);
void    stat_release_datum      (STAT_STRUCT *stat_struct, int index);
int     stat_get_struct_value   (STAT_STRUCT *stat_struct, int index);
int     stat_data_count         (STAT_STRUCT *stat_struct);
double  stat_mean               (STAT_STRUCT *stat_struct);
double  stat_standard_deviation (STAT_STRUCT *stat_struct);
double  compute_data_sd_factor  (STAT_STRUCT *stat_struct, int index);

int     point_in_rect   (const FPOINT *point, const FPOINT *coords);
int     point_in_circle (const FPOINT *point, const FPOINT *coords);
int     point_in_poly   (const FPOINT *point, const FPOINT *coords, int nb_point);

qbyte   calculate_crc            (byte *block, size_t length);
void    calculate_continuing_crc (byte *block, size_t length, 
                                  qbyte *crc_value);

#ifdef __cplusplus
}
#endif

#endif
