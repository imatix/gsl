/*===========================================================================*
 *                                                                           *
 *  sflslot.h - Time slot functions                                          *
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
    Synopsis:   The time-slot functions provide long-running programs with
                a means to 'switch-on' and 'switch-off' depending on the time
                of day, and day of year.  The intention is that the user can
                configure such programs to be active only between certain
                hours, on certain days, etc.  The time-slot functions work
                with 'range' bitmaps for a day (in seconds) and a year (in
                days), and provide functions to set, clear, and test these
                ranges.
 ------------------------------------------------------------------</Prolog>-*/

#ifndef SFLSLOT_INCLUDED               /*  Allow multiple inclusions        */
#define SFLSLOT_INCLUDED

#define MAX_DAY          366            /*  Max. days in a normal year       */
#define MAX_MIN         1440            /*  Max. minutes in a normal day     */
typedef byte year_range [46];           /*  366 bits (1 per day)             */
typedef byte day_range  [180];          /*  1440 bits (1 per minute)         */

#ifdef __cplusplus
extern "C" {
#endif

void  year_range_empty      (byte *range);
void  year_range_fill       (byte *range);
int   year_slot_set         (byte *range, int day_from, int day_to);
int   year_slot_clear       (byte *range, int day_from, int day_to);
Bool  year_slot_filled      (const byte *range, int day);

void  day_range_empty       (byte *range);
void  day_range_fill        (byte *range);
int   day_slot_set          (byte *range, int min_from, int min_to);
int   day_slot_clear        (byte *range, int min_from, int min_to);
Bool  day_slot_filled       (const byte *range, int minute);

int   date_to_day           (long date);
int   time_to_min           (long time);

#ifdef __cplusplus
}
#endif

#endif
