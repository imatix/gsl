/*===========================================================================*
 *                                                                           *
 *  sfldate.h - Date and time manipulation functions                         *
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
    Synopsis:   Includes functions to get the current date/time, calculate
                the day or week, week of year and leap year.  Dates and times
                are each stored in a 32-bit long value of 8 digits: dates are
                CCYYMMDD; times are HHMMSSCC.  You can compare dates and times
                directly - e.g. if (date_wanted >= date_now), but not do 
                arithmetic on them directly.
 ------------------------------------------------------------------</Prolog>-*/

#ifndef SFLDATE_INCLUDED               /*  Allow multiple inclusions        */
#define SFLDATE_INCLUDED

/*  Macros                                                                   */

#define GET_CENTURY(d)      (int) ( (d) / 1000000L)
#define GET_CCYEAR(d)       (int) ( (d) / 10000L)
#define GET_YEAR(d)         (int) (((d) % 1000000L) / 10000L)
#define GET_MONTH(d)        (int) (((d) % 10000L) / 100)
#define GET_DAY(d)          (int) ( (d) % 100)

#define GET_HOUR(t)         (int) ( (t) / 1000000L)
#define GET_MINUTE(t)       (int) (((t) % 1000000L) / 10000L)
#define GET_SECOND(t)       (int) (((t) % 10000L) / 100)
#define GET_CENTI(t)        (int) ( (t) % 100)

#define MAKE_DATE(c,y,m,d)  (long) (c) * 1000000L +                          \
                            (long) (y) * 10000L +                            \
                            (long) (m) * 100 + (d)
#define MAKE_TIME(h,m,s,c)  (long) (h) * 1000000L +                          \
                            (long) (m) * 10000L +                            \
                            (long) (s) * 100 + (c)

#define timeeq(d1,t1,d2,t2)  ((d1) == (d2) && (t1) == (t2))
#define timeneq(d1,t1,d2,t2) ((d1) != (d2) || (t1) != (t2))
#define timelt(d1,t1,d2,t2)  ((d1) < (d2) || ((d1) == (d2) && (t1) <  (t2)))
#define timele(d1,t1,d2,t2)  ((d1) < (d2) || ((d1) == (d2) && (t1) <= (t2)))
#define timegt(d1,t1,d2,t2)  ((d1) > (d2) || ((d1) == (d2) && (t1) >  (t2)))
#define timege(d1,t1,d2,t2)  ((d1) > (d2) || ((d1) == (d2) && (t1) >= (t2)))

/*  Days are numbered from 0=Sunday to 6=Saturday                            */

#define DAY_SUNDAY      0
#define DAY_MONDAY      1
#define DAY_TUESDAY     2
#define DAY_WEDNESDAY   3
#define DAY_THURSDAY    4
#define DAY_FRIDAY      5
#define DAY_SATURDAY    6


/*  Interval values, specified in centiseconds                               */

#define INTERVAL_CENTI      1
#define INTERVAL_SEC        100
#define INTERVAL_MIN        6000
#define INTERVAL_HOUR       360000L
#define INTERVAL_DAY        8640000L


/*  Function prototypes                                                      */

#ifdef __cplusplus
extern "C" {
#endif

long   date_now           (void);
long   time_now           (void);
void   get_date_time_now  (long *current_date, long *current_time);
long   micro_time         (void);
Bool   leap_year          (int year);
int    julian_date        (long date);
int    day_of_week        (long date);
int    week_of_year       (long date);
int    year_quarter       (long date);
long   next_weekday       (long date);
long   prev_weekday       (long date);
word   pack_date          (long date);
word   pack_time          (long time);
long   unpack_date        (word packdate);
long   unpack_time        (word packtime);
long   default_century    (long *date);
long   date_to_days       (long date);
long   days_to_date       (long days);
time_t date_to_timer      (long date, long time);
long   timer_to_date      (time_t time_secs);
long   timer_to_time      (time_t time_secs);
long   timer_to_gmdate    (time_t time_secs);
long   timer_to_gmtime    (time_t time_secs);
long   time_to_csecs      (long time);
long   csecs_to_time      (long csecs);
void   future_date        (long *date, long *time, long days, long csecs);
void   past_date          (long *date, long *time, long days, long csecs);
void   date_diff          (long date1, long time1, long date2, long time2,
                           long *days, long *csecs);
Bool   valid_date         (long date);
Bool   valid_time         (long time);
Bool   date_is_future     (long date, long time);
Bool   date_is_past       (long date, long time);
char  *timezone_string    (void);
void   local_to_gmt       (long date, long time, long *gmdate, long *gmtime);
void   gmt_to_local       (long gmdate, long gmtime, long *date, long *time);
struct tm *safe_localtime (const time_t *time_secs);
struct tm *safe_gmtime    (const time_t *time_secs);
double timestamp_now      (void);
double gmtimestamp_now    (void);
double make_timestamp     (long date, long time);
char  *gmtimestamp_to_iso (double timestamp);
double iso_to_gmtimestamp (const char *iso_time);
long   timestamp_date     (double timestamp);
long   timestamp_time     (double timestamp);
long   local_tz           (void);
int    local_dst          (void);

#ifdef __cplusplus
}
#endif

#endif
