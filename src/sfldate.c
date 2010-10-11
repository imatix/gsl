/*===========================================================================*
 *                                                                           *
 *  sfldate.c - Date and time manipulation functions                         *
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

#include "prelude.h"                    /*  Universal header file            */
#include "sflprint.h"                   /*  snprintf functions               */
#include "sfldate.h"                    /*  Prototypes for functions         */
#include "sflstr.h"                     /*  Prototypes for string handling   */
#include "sflsymb.h"                    /*  Prototypes for symbols           */
#include "sflenv.h"                     /*  Prototypes for environment       */
#include "sflfind.h"                    /*  Prototypes for string search     */
#include "sflmem.h"                     /*  Prototypes for memory allocation */

/*  ---------------------------------------------------------------------[<]-
    Function: date_now

    Synopsis: Returns the current date as a long value (CCYYMMDD).  Since
    most system clocks do not return a century, this function assumes that
    all years 80 and above are in the 20th century, and all years 00 to 79
    are in the 21st century.  For best results, consume before 1 Jan 2080.
    ---------------------------------------------------------------------[>]-*/

long
date_now (void)
{
    return (timer_to_date (time (NULL)));
}


/*  ---------------------------------------------------------------------[<]-
    Function: time_now

    Synopsis: Returns the current time as a long value (HHMMSSCC).  If the
    system clock does not return centiseconds, these are set to zero.
    ---------------------------------------------------------------------[>]-*/

long
time_now (void)
{
#if (!defined (TIMEZONE))               /*  No system support for TIMEZONE   */
    return (timer_to_time (time (NULL)));

#elif (defined (__TURBOC__))
    /*  The Turbo-C gettime() function returns just what we want             */
    /*  XXX: Can this be put in get_date_time_now()?  EDM, 2001/06/14        */
    struct time
        time_struct;

    gettime (&time_struct);
    return (MAKE_TIME (time_struct.ti_hour,
                       time_struct.ti_min,
                       time_struct.ti_sec,
                       time_struct.ti_hund));

#elif (defined (__UNIX__) || defined (__VMS_XOPEN) || defined (WIN32))
    long now = 0L;

    get_date_time_now (NULL, &now);
    return (now);

#else
    /*  Otherwise, just get the time without milliseconds                    */
    return (timer_to_time (time (NULL)));
#endif
}


/*  ---------------------------------------------------------------------[<]-
    Function: get_date_time_now

    Synopsis: Places the current date, as a long value (CCYYMMDD), and
    the current time, as a long value (HHMMSSCC) into the supplied 
    variables.  If the system clock does not return centiseconds, these 
    are set to zero.  If either pointer is NULL, the appropriate value is not
    calculated.  Where possible these values are calculated from the same
    time call to the operating system, to avoid races.
    ---------------------------------------------------------------------[>]-*/
void   
get_date_time_now  (long *current_date, long *current_time)
{
    if (current_date || current_time)
      {
#if (defined (__UNIX__) || defined (__VMS_XOPEN))
        /*  The BSD gettimeofday function returns seconds and microseconds   */
        struct timeval
            time_struct;

        gettimeofday (&time_struct, 0);

        if (current_date)
            *current_date = (timer_to_date (time_struct.tv_sec));

        if (current_time)
            *current_time = (timer_to_time (time_struct.tv_sec)
                                          + time_struct.tv_usec / 10000);
#elif (defined (WIN32))
        /*  The Win32 GetLocalTime function returns just what we want        */
        SYSTEMTIME
            time_struct;

        GetLocalTime (&time_struct);

        if (current_date)
            *current_date = (MAKE_DATE (time_struct.wYear / 100,
                                        time_struct.wYear % 100,
                                        time_struct.wMonth,
                                        time_struct.wDay));

        if (current_time)
            *current_time = (MAKE_TIME (time_struct.wHour,
                                        time_struct.wMinute,
                                        time_struct.wSecond,
                                        time_struct.wMilliseconds / 10));
#else
        /*  Cannot obtain the date and time at once, or code to do so        */
        /*  has not yet been written.  Just obtain them separately, and      */
        /*  repeat if time is before 0001.                                   */
        if (current_date)
            *current_date = date_now();

        if (current_time)
          {
            *current_time = time_now();
            if (current_date && *current_time < 00010000)
                *current_date = date_now();
          }
#endif
      }
}


/*  ---------------------------------------------------------------------[<]-
    Function: micro_time

    Synopsis: Returns the number of microseconds since the last whole
    second, to the accuracy of the current system.  If not known, returns
    zero.
    ---------------------------------------------------------------------[>]-*/

long
micro_time (void)
{
#if (defined (__UNIX__) || defined (__VMS_XOPEN))
    /*  The BSD gettimeofday function returns seconds and microseconds       */
    struct timeval
            time_struct;

    gettimeofday (&time_struct, 0);
    return (time_struct.tv_usec);
    
#elif (defined (WIN32))
    /*  The Win32 GetLocalTime function returns something less detailed      */
    SYSTEMTIME
        time_struct;

    GetLocalTime (&time_struct);
    return (time_struct.wMilliseconds * 1000);
#else
    return (0);
#endif
}


/*  ---------------------------------------------------------------------[<]-
    Function: leap_year

    Synopsis: Returns TRUE if the year is a leap year.  You must supply a
    4-digit value for the year: 90 is taken to mean 90 ad.  Handles leap
    centuries correctly.
    ---------------------------------------------------------------------[>]-*/

Bool
leap_year (int year)
{
    ASSERT (year > 0);

    return ((year % 4 == 0 && year % 100 != 0) || year % 400 == 0);
}


/*  ---------------------------------------------------------------------[<]-
    Function: julian_date

    Synopsis: Returns the number of days since 31 December last year.  The
    Julian date of 1 January is 1.
    ---------------------------------------------------------------------[>]-*/

int
julian_date (long date)
{
    static int
        days [12] = {
        0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334
    };
    int
        julian;

    ASSERT (valid_date (date));

    julian = days [GET_MONTH (date) - 1] + GET_DAY (date);
    if (GET_MONTH (date) > 2 && leap_year (GET_YEAR (date)))
        julian++;

    return (julian);
}


/*  ---------------------------------------------------------------------[<]-
    Function: day_of_week

    Synopsis: Returns the day of the week where 0 is Sunday, 1 is Monday,
    ... 6 is Saturday.  Uses Zeller's Congruence algorithm.
    ---------------------------------------------------------------------[>]-*/

int
day_of_week (long date)
{
    int
        year  = GET_CCYEAR (date),
        month = GET_MONTH  (date),
        day   = GET_DAY    (date);

    ASSERT (valid_date (date));

    if (month > 2)
        month -= 2;
    else
      {
        month += 10;
        year--;
      }
    day = ((13 * month - 1) / 5) + day + (year % 100) +
          ((year % 100) / 4) + ((year / 100) / 4) - 2 *
           (year / 100) + 77;

    return (day - 7 * (day / 7));
}


/*  ---------------------------------------------------------------------[<]-
    Function: next_weekday

    Synopsis: Returns the date of the next weekday, skipping from Friday
    to Monday.
    ---------------------------------------------------------------------[>]-*/

long
next_weekday (long date)
{
    long
        days = date_to_days (date);

    if (day_of_week (date) == 5)        /*  Friday                           */
        days += 3;
    else
    if (day_of_week (date) == 6)        /*  Saturday                         */
        days += 2;
    else
        days += 1;                      /*  Sunday to Thursday               */

    return (days_to_date (days));
}


/*  ---------------------------------------------------------------------[<]-
    Function: prev_weekday

    Synopsis: Returns the date of the previous weekday, skipping from Monday
    to Friday.
    ---------------------------------------------------------------------[>]-*/

long
prev_weekday (long date)
{
    long
        days = date_to_days (date);

    if (day_of_week (date) == 1)        /*  Monday                           */
        days -= 3;
    else
    if (day_of_week (date) == 0)        /*  Sunday                           */
        days -= 2;
    else
        days -= 1;                      /*  Tuesday to Saturday              */

    return (days_to_date (days));
}


/*  ---------------------------------------------------------------------[<]-
    Function: week_of_year

    Synopsis: Returns the week of the year, where 1 is the first full week.
    Week 0 may or may not exist in any year.  Uses a Lillian date algorithm
    to calculate the week of the year.
    ---------------------------------------------------------------------[>]-*/

int
week_of_year (long date)
{
    long
        year = GET_CCYEAR (date) - 1501,
        day  = year * 365 + year / 4 - 29872L + 1
             - year / 100 + (year - 300) / 400;

    ASSERT (valid_date (date));

    return ((julian_date (date) + (int) ((day + 4) % 7)) / 7);
}


/*  ---------------------------------------------------------------------[<]-
    Function: year_quarter

    Synopsis: Returns the year quarter, 1 to 4, depending on the month
    specified.
    ---------------------------------------------------------------------[>]-*/

int
year_quarter (long date)
{
    ASSERT (valid_date (date));
    return ((GET_MONTH (date) - 1) / 3 + 1);
}


/*  ---------------------------------------------------------------------[<]-
    Function: default_century

    Synopsis: Supplies a default century for the year if necessary.  If
    the year is 51 to 99, the century is set to 19.  If the year is 0 to
    50, the century is set to 20.  Returns the adjusted date.
    ---------------------------------------------------------------------[>]-*/

long
default_century (long *date)
{
    ASSERT (date);
    if (GET_CENTURY (*date) == 0)
        *date += (GET_YEAR (*date) > 50? 19000000L: 20000000L);
    return (*date);
}


/*  ---------------------------------------------------------------------[<]-
    Function: pack_date

    Synopsis: Packs the date into a single unsigned short word.  Use this
    function to store dates when memory space is at a premium.  The packed
    date can be used correctly in comparisons.  Returns the packed date.
    The date must be later than 31 December 1979.
    ---------------------------------------------------------------------[>]-*/

word
pack_date (long date)
{
    ASSERT (valid_date (date));
    return (word) (((GET_CCYEAR (date) - 1980) << 9) +
                    (GET_MONTH  (date) << 5) +
                     GET_DAY    (date));
}


/*  ---------------------------------------------------------------------[<]-
    Function: pack_time

    Synopsis: Packs the time into a single unsigned short word.  Use this
    function to store times when memory space is at a premium.  The packed
    time can be used correctly in comparisons.  Returns the packed time.
    Seconds are stored with 2-second accuracy and centiseconds are lost.
    ---------------------------------------------------------------------[>]-*/

word
pack_time (long time)
{
    ASSERT (valid_time (time));
    return (word) ((GET_HOUR   (time) << 11) +
                   (GET_MINUTE (time) << 5)  +
                   (GET_SECOND (time) >> 1));
}


/*  ---------------------------------------------------------------------[<]-
    Function: unpack_date

    Synopsis: Converts a packed date back into a long value.
    ---------------------------------------------------------------------[>]-*/

long
unpack_date (word packdate)
{
    int year;

    year = ((word) (packdate & 0xfe00) >> 9) + 80;
    return (MAKE_DATE (year > 80? 19: 20,
                       year,
                       (word) (packdate & 0x01e0) >> 5,
                       (word) (packdate & 0x001f)));
}


/*  ---------------------------------------------------------------------[<]-
    Function: unpack_time

    Synopsis: Converts a packed time back into a long value.
    ---------------------------------------------------------------------[>]-*/

long
unpack_time (word packtime)
{
    return (MAKE_TIME ((word) (packtime & 0xf800) >> 11,
                       (word) (packtime & 0x07e0) >> 5,
                       (word) (packtime & 0x001f) << 1, 0));
}


/*  ---------------------------------------------------------------------[<]-
    Function: date_to_days

    Synopsis: Converts the date into a number of days since a distant but
    unspecified epoch.  You can use this function to calculate differences
    between dates, and forward dates.  Use days_to_date() to calculate the
    reverse function.  Author: Robert G. Tantzen, translated from the Algol
    original in Collected Algorithms of the CACM (algorithm 199).  Original
    translation into C by Nat Howard, posted to Usenet 5 Jul 1985.
    ---------------------------------------------------------------------[>]-*/

long
date_to_days (long date)
{
    long
        year    = GET_YEAR    (date),
        century = GET_CENTURY (date),
        month   = GET_MONTH   (date),
        day     = GET_DAY     (date);

    ASSERT (valid_date (date));

    if (month > 2)
        month -= 3;
    else
      {
        month += 9;
        if (year)
            year--;
        else
          {
            year = 99;
            century--;
          }
      }
    return ((146097L * century)    / 4L +
            (1461L   * year)       / 4L +
            (153L    * month + 2L) / 5L +
                       day   + 1721119L);
}


/*  ---------------------------------------------------------------------[<]-
    Function: days_to_date

    Synopsis: Converts a number of days since some distant but unspecified
    epoch into a date.  You can use this function to calculate differences
    between dates, and forward dates.  Use date_to_days() to calculate the
    reverse function.  Author: Robert G. Tantzen, translated from the Algol
    original in Collected Algorithms of the CACM (algorithm 199).  Original
    translation into C by Nat Howard, posted to Usenet 5 Jul 1985.
    ---------------------------------------------------------------------[>]-*/

long
days_to_date (long days)
{
    long
        century,
        year,
        month,
        day;

    days   -= 1721119L;
    century = (4L * days - 1L) / 146097L;
    days    =  4L * days - 1L  - 146097L * century;
    day     =  days / 4L;

    year    = (4L * day + 3L) / 1461L;
    day     =  4L * day + 3L  - 1461L * year;
    day     = (day + 4L) / 4L;

    month   = (5L * day - 3L) / 153L;
    day     =  5L * day - 3   - 153L * month;
    day     = (day + 5L) / 5L;

    if (month < 10)
        month += 3;
    else
      {
        month -= 9;
        if (year++ == 99)
          {
            year = 0;
            century++;
          }
      }
    return (MAKE_DATE (century, year, month, day));
}


/*  ---------------------------------------------------------------------[<]-
    Function: date_to_timer

    Synopsis: Converts the supplied date and time into a time_t timer value.
    This is the number of non-leap seconds since 00:00:00 GMT Jan 1, 1970.
    Function was rewritten by Bruce Walter <walter@fortean.com>.  If the
    input date and time are invalid, returns 0.
    ---------------------------------------------------------------------[>]-*/

time_t
date_to_timer (long date, long time)
{
    struct tm
        time_struct;
    time_t
        timer;

    ASSERT (valid_date (date));
    ASSERT (valid_time (time));

    time_struct.tm_sec   = GET_SECOND (time);
    time_struct.tm_min   = GET_MINUTE (time);
    time_struct.tm_hour  = GET_HOUR   (time);
    time_struct.tm_mday  = GET_DAY    (date);
    time_struct.tm_mon   = GET_MONTH  (date) - 1;
    time_struct.tm_year  = GET_CCYEAR (date) - 1900;
    time_struct.tm_isdst = -1;
    timer = mktime (&time_struct);
    if (timer == (time_t) -1)
        return (0);
    else
        return (timer);
}


/*  ---------------------------------------------------------------------[<]-
    Function: timer_to_date

    Synopsis: Converts the supplied timer value into a long date value.
    Dates are stored as long values: CCYYMMDD.  If the supplied value is
    zero, returns zero.  The timer value is assumed to be UTC (GMT).
    ---------------------------------------------------------------------[>]-*/

long
timer_to_date (time_t time_secs)
{
    struct tm
        *time_struct;

    if (time_secs == 0)
        return (0);
    else
      {
        /*  Convert into a long value CCYYMMDD                               */
        time_struct = safe_localtime (&time_secs);
        time_struct->tm_year += 1900;
        return (MAKE_DATE (time_struct->tm_year / 100,
                           time_struct->tm_year % 100,
                           time_struct->tm_mon + 1,
                           time_struct->tm_mday));
      }
}


/*  ---------------------------------------------------------------------[<]-
    Function: timer_to_time

    Synopsis: Converts the supplied timer value into a long time value.
    Times are stored as long values: HHMMSS00.  Since the timer value does
    not hold centiseconds, these are set to zero.  If the supplied value
    was zero, returns zero.  The timer value is assumed to be UTC (GMT).
    ---------------------------------------------------------------------[>]-*/

long
timer_to_time (time_t time_secs)
{
    struct tm
        *time_struct;

    if (time_secs == 0)
        return (0);
    else
      {
        /*  Convert into a long value HHMMSS00                               */
        time_struct = safe_localtime (&time_secs);
        return (MAKE_TIME (time_struct->tm_hour,
                           time_struct->tm_min,
                           time_struct->tm_sec,
                           0));
      }
}


/*  ---------------------------------------------------------------------[<]-
    Function: timer_to_gmdate

    Synopsis: Converts the supplied timer value into a long date value in
    Greenwich Mean Time (GMT).  Dates are stored as long values: CCYYMMDD.
    If the supplied value is zero, returns zero.
    ---------------------------------------------------------------------[>]-*/

long
timer_to_gmdate (time_t time_secs)
{
    struct tm
        *time_struct;

    if (time_secs == 0)
        return (0);
    else
      {
        /*  Convert into a long value CCYYMMDD                               */
        time_struct = safe_gmtime (&time_secs);
        time_struct->tm_year += 1900;
        return (MAKE_DATE (time_struct->tm_year / 100,
                           time_struct->tm_year % 100,
                           time_struct->tm_mon + 1,
                           time_struct->tm_mday));
      }
}


/*  ---------------------------------------------------------------------[<]-
    Function: timer_to_gmtime

    Synopsis: Converts the supplied timer value into a long time value in
    Greenwich Mean Time (GMT).  Times are stored as long values: HHMMSS00.
    On most systems the clock does not return centiseconds, so these are
    set to zero.  If the supplied value is zero, returns zero.
    ---------------------------------------------------------------------[>]-*/

long
timer_to_gmtime (time_t time_secs)
{
    struct tm
        *time_struct;

    if (time_secs == 0)
        return (0);
    else
      {
        /*  Convert into a long value HHMMSS00                               */
        time_struct = safe_gmtime (&time_secs);
        return (MAKE_TIME (time_struct->tm_hour,
                           time_struct->tm_min,
                           time_struct->tm_sec,
                           0));
      }
}


/*  ---------------------------------------------------------------------[<]-
    Function: time_to_csecs

    Synopsis: Converts a time (HHMMSSCC) into a number of centiseconds.
    ---------------------------------------------------------------------[>]-*/

long
time_to_csecs (long time)
{
    ASSERT (valid_time (time));

    return ((long) (GET_HOUR   (time) * (long) INTERVAL_HOUR)
          + (long) (GET_MINUTE (time) * (long) INTERVAL_MIN)
          + (long) (GET_SECOND (time) * (long) INTERVAL_SEC)
          + (long) (GET_CENTI  (time)));
}


/*  ---------------------------------------------------------------------[<]-
    Function: csecs_to_time

    Synopsis: Converts a number of centiseconds (< INTERVAL_DAY) into a
    time value (HHMMSSCC).
    ---------------------------------------------------------------------[>]-*/

long
csecs_to_time (long csecs)
{
    long
        hour,
        min,
        sec;

    ASSERT (csecs >= 0 && csecs < INTERVAL_DAY);

    hour  = csecs / INTERVAL_HOUR;
    csecs = csecs % INTERVAL_HOUR;
    min   = csecs / INTERVAL_MIN;
    csecs = csecs % INTERVAL_MIN;
    sec   = csecs / INTERVAL_SEC;
    csecs = csecs % INTERVAL_SEC;
    return (MAKE_TIME (hour, min, sec, csecs));
}


/*  ---------------------------------------------------------------------[<]-
    Function: future_date

    Synopsis: Calculates a future date and time from the date and time
    specified, plus an interval specified in days and 1/100th seconds.
    The date can be any date since some distant epoch (around 1600).
    If the date and time arguments are both zero, the current date and
    time are used.  Either date and time arguments may be null.
    ---------------------------------------------------------------------[>]-*/

void
future_date (long *date, long *time, long days, long csecs)
{
    long
        dummy_date = 0,
        dummy_time = 0;

    if (date == NULL)
        date = &dummy_date;
    if (time == NULL)
        time = &dummy_time;

    /*  Set date and time to NOW if necessary                                */
    if (*date == 0 && *time == 0)
        get_date_time_now (date, time);

    /*  Get future date in days and centiseconds                             */
    days  = date_to_days  (*date) + days;
    csecs = time_to_csecs (*time) + csecs;

    /*  Normalise overflow in centiseconds                                   */
    while (csecs >= INTERVAL_DAY)
      {
        days++;
        csecs -= INTERVAL_DAY;
      }

    /*  Convert date and time back into organised values                     */
    *date = days_to_date  (days);
    *time = csecs_to_time (csecs);
}


/*  ---------------------------------------------------------------------[<]-
    Function: past_date

    Synopsis: Calculates a past date and time from the date and time
    specified, minus an interval specified in days and 1/100th seconds.
    The date can be any date since some distant epoch (around 1600).
    If the date and time arguments are both zero, the current date and
    time are used.  Either date and time arguments may be null.
    ---------------------------------------------------------------------[>]-*/

void
past_date (long *date, long *time, long days, long csecs)
{
    long
        dummy_date = 0,
        dummy_time = 0;

    if (date == NULL)
        date = &dummy_date;
    if (time == NULL)
        time = &dummy_time;

    /*  Set date and time to NOW if necessary                                */
    if (*date == 0 && *time == 0)
        get_date_time_now (date, time);

    /*  Get past date in days and centiseconds                               */
    days  = date_to_days  (*date) - days;
    csecs = time_to_csecs (*time) - csecs;

    /*  Normalise underflow in centiseconds                                  */
    while (csecs < 0)
      {
        days--;
        csecs += INTERVAL_DAY;
      }

    /*  Convert date and time back into organised values                     */
    *date = days_to_date  (days);
    *time = csecs_to_time (csecs);
}


/*  ---------------------------------------------------------------------[<]-
    Function: date_diff

    Synopsis: Calculates the difference between two date/time values, and
    returns the difference as a number of days and a number of centiseconds.
    The date can be any date since some distant epoch (around 1600).  The
    calculation is date1:time1 - date2:time2.  If date1:time1 is before 
    date2:time2, both values will be zero or negative.  If date1:time1 is 
    after date2:time2, both values will be zero or positive.  (Otherwise 
    both will be 0, as they're the same time!)
    ---------------------------------------------------------------------[>]-*/

void
date_diff (
    long date1, long time1,             /*  Date and time                    */
    long date2, long time2,             /*    minus this date and time       */
    long *days, long *csecs             /*  Gives these values               */
)
{
    ASSERT (days && csecs);

    if (days)
        *days  = date_to_days  (date1) - date_to_days  (date2);
    if (csecs)
        *csecs = time_to_csecs (time1) - time_to_csecs (time2);

    /*  Now perform any fixup that is required on the date/csecs so that     */
    /*  both are positive or negative.                                       */
    ASSERT (*csecs >= -INTERVAL_DAY && *csecs <= INTERVAL_DAY);

    if (days && csecs)
      {
        if (*days > 0 && *csecs < 0)
          {
            /*  Force both to be zero or positive                            */
            (*days)--;
            (*csecs) += INTERVAL_DAY;
          }
        else if (*days < 0 && *csecs > 0)
          {
            /*  Force both to be zero or negative                            */
            (*days)++;
            (*csecs) -= INTERVAL_DAY;
          }
      }
}


/*  ---------------------------------------------------------------------[<]-
    Function: valid_date

    Synopsis: Returns TRUE if the date is valid or zero; returns FALSE if
    the date is not valid.
    ---------------------------------------------------------------------[>]-*/

Bool
valid_date (long date)
{
    int
        month,
        day;
    Bool
        feedback;
    static byte
        month_days [] = { 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

    month = GET_MONTH (date);
    day   = GET_DAY   (date);

    if (date == 0)
        feedback = TRUE;                /*  Zero date is okay                */
    else
    if (month < 1 || month > 12)
        feedback = FALSE;               /*  Month out of range               */
    else
    if ((day < 1 || day > month_days [month - 1])
    ||  (month == 2 && day == 29 && !leap_year (GET_YEAR (date))))
        feedback = FALSE;               /*  Day out of range                 */
    else
        feedback = TRUE;                /*  Zero date is okay                */

    return (feedback);
}


/*  ---------------------------------------------------------------------[<]-
    Function: valid_time

    Synopsis: Returns TRUE if the time is valid or zero; returns FALSE if
    the time is not valid.
    ---------------------------------------------------------------------[>]-*/

Bool
valid_time (long time)
{
    return (time >= 0 
        &&  GET_SECOND (time) < 60
        &&  GET_MINUTE (time) < 60
        &&  GET_HOUR   (time) < 24);
}


/*  ---------------------------------------------------------------------[<]-
    Function: date_is_future

    Synopsis: Returns TRUE if the specified date and time are in the future.
    Returns FALSE if the date and time are in the past, or the present (which
    will be the past by the time you've read this).  Date is specified as a
    YYYYMMDD value; time as HHMMSSCC.
    ---------------------------------------------------------------------[>]-*/

Bool
date_is_future (long date, long time)
{
    long
        current_date,
        current_time;

    ASSERT (valid_date (date));
    ASSERT (valid_time (time));

    get_date_time_now (&current_date, &current_time);

    return (date  > current_date
        || (date == current_date && time > current_time));
}


/*  ---------------------------------------------------------------------[<]-
    Function: date_is_past

    Synopsis: Returns TRUE if the specified date and time are in the past.
    Returns FALSE if the date and time are in the future or present (which
    despite any assertion to the contrary, is not the past.  Although that
    may change soon).  Date is specified as YYYYMMDD; time as HHMMSSCC.
    ---------------------------------------------------------------------[>]-*/

Bool
date_is_past (long date, long time)
{
    long
        current_date,
        current_time;

    ASSERT (valid_date (date));
    ASSERT (valid_time (time));

    get_date_time_now (&current_date, &current_time);

    return (date  < current_date
        || (date == current_date && time < current_time));
}


/*  ---------------------------------------------------------------------[<]-
    Function: timezone_string

    Synopsis: Returns a static string containing the time zone as a 4-digit
    number, with a leading '+' or '-' character.   GMT is represented as
    "+0000"; Central European Time is "+1000". If TIMEZONE is undefined,
    looks for the environment string ZONE which should be set to the local
    timezone as above in autoexec.bat, or exported if in some broken version
    of *nix. If the system does not support the timezone, returns "+0000".
    ---------------------------------------------------------------------[>]-*/

char *
timezone_string (void)
{
#if (!defined (TIMEZONE))               /*  No system support for TIMEZONE   */
    static char *
        timezone;
    timezone = deletestring ((void *) strdupl (env_get_string ("ZONE", "+0000")), "DST", 0);
    return (timezone);
#elif (defined (TIMEZONE))
    static char
        formatted_string [10];          /*  -nnnn plus null                  */
    int
        minutes;                        /*  TIMEZONE is in seconds           */

    minutes = 0 - (int) (TIMEZONE / 60);
    snprintf (formatted_string, sizeof (formatted_string),
              "%03d%02d", minutes / 60, abs (minutes % 60));
    formatted_string [sizeof (formatted_string) - 1] = '\0';
    if (*formatted_string == '0')
        *formatted_string = '+';
    return  (formatted_string);
#endif
}


/*  ---------------------------------------------------------------------[<]-
    Function: local_to_gmt

    Synopsis: Converts the specified date and time to GMT.  Returns the GMT
    date and time in two arguments. Date and time returned will be in error
    by one hour if they cross into or out of a Daylight Saving period, so
    use with care if converting anything except current local date and time.
    ---------------------------------------------------------------------[>]-*/

void
local_to_gmt (long date, long time, long *gmt_date, long *gmt_time)
{
    time_t
        time_value;
    struct tm
        *time_struct;

    ASSERT (gmt_date && gmt_time);

    /*  Convert from GMT                                                     */
#if (!defined (TIMEZONE))              /*  No system support for TIMEZONE    */
    time_value  = date_to_timer (date, time) + local_tz();
#else
    time_value  = date_to_timer (date, time) + TIMEZONE;
#endif
    time_struct = safe_localtime (&time_value);
    time_struct->tm_year += 1900;
    *gmt_date = MAKE_DATE (time_struct->tm_year / 100,
                           time_struct->tm_year % 100,
                           time_struct->tm_mon + 1,
                           time_struct->tm_mday);
    *gmt_time = MAKE_TIME (time_struct->tm_hour,
                           time_struct->tm_min,
                           time_struct->tm_sec,
                           0);
}


/*  ---------------------------------------------------------------------[<]-
    Function: gmt_to_local

    Synopsis: Converts the specified GMT date and time to the local time.
    Returns the local date and time in two arguments.  If the supplied value
    is out of range, returns 00:00 on 1 January, 1970 (19700101). If TIMEZONE
    is undefined, uses faked value via local_tz() function. Take care
    when crossing into or out of a DST period, as with local_to_gmt.
    ---------------------------------------------------------------------[>]-*/

void
gmt_to_local (long gmt_date, long gmt_time, long *date, long *time)
{
    time_t
        time_value;
    struct tm
        *time_struct;

    ASSERT (date && time);

    /*  Convert from GMT                                                     */
#if (!defined (TIMEZONE))              /*  No system support for TIMEZONE    */
    time_value  = date_to_timer (gmt_date, gmt_time) - local_tz();
#else
    time_value  = date_to_timer (gmt_date, gmt_time) - TIMEZONE;
#endif
    time_struct = safe_localtime (&time_value);
    time_struct->tm_year += 1900;
    *date = MAKE_DATE (time_struct->tm_year / 100,
                       time_struct->tm_year % 100,
                       time_struct->tm_mon + 1,
                       time_struct->tm_mday);
    *time = MAKE_TIME (time_struct->tm_hour,
                       time_struct->tm_min,
                       time_struct->tm_sec,
                       0);
}


/*  ---------------------------------------------------------------------[<]-
    Function: safe_localtime

    Synopsis: Handles time_t values that exceed 2038.  The standard C
    localtime() function fails on most systems when the date passes 2038.
    The 20-year offset used is an arbitrary value of approximately the 
    right size; it may be possible to find an offset where the day of the 
    week also stays correct.
    ---------------------------------------------------------------------[>]-*/

struct tm
*safe_localtime (const time_t *time_secs)
{
    time_t
        adjusted_time;
    struct tm
        *time_struct;
    int
        adjust_years = 0;

    ASSERT (time_secs);

    adjusted_time = (qbyte) *time_secs; 
    while (adjusted_time > LONG_MAX)
      {
        adjust_years  += 20;
        adjusted_time -= 631152000;     /*  Number of seconds in 20 years    */
      }
    time_struct = localtime (&adjusted_time);
    time_struct->tm_year += adjust_years;

    return (time_struct);
}


/*  ---------------------------------------------------------------------[<]-
    Function: safe_gmtime

    Synopsis: Handles time_t values that exceed 2038.  The standard C
    gmtime() function fails on most systems when the date passes 2038.
    The 20-year offset used is an arbitrary value of approximately the
    right size; it may be possible to find an offset where the day of the
    week also stays correct.
    ---------------------------------------------------------------------[>]-*/

struct tm
*safe_gmtime (const time_t *time_secs)
{
    time_t
        adjusted_time;
    struct tm
        *time_struct;
    int
        adjust_years = 0;

    ASSERT (time_secs);

    adjusted_time = (qbyte) *time_secs;
    while (adjusted_time > LONG_MAX)
      {
        adjust_years  += 20;
        adjusted_time -= 631152000;     /*  Nbr seconds in 20 years          */
      }
    time_struct = gmtime (&adjusted_time);
    if (time_struct)                    /*  gmtime may be unimplemented      */
        time_struct->tm_year += adjust_years;

    return (time_struct);
}

/*  ---------------------------------------------------------------------[<]-
    Function: local_tz

    Synopsis: Returns faked local TIMEZONE value as a signed long in seconds,
    corrected for Daylight Saving Time if in force. If the environment value
    ZONE isn't a sensible number or if none is found, the function returns 0.
    Accuracy of the conversion algorithm is +/- 0 seconds, which is near enough.
    ---------------------------------------------------------------------[>]-*/

long
local_tz (void)
{
    static Bool
        done_init = FALSE;
    static long
        tz_secs = (time_t) 0L;

    if (!done_init)
      {
        char *
            timezone;
        int
            tz;

        timezone = env_get_string ("ZONE", "+0000");
        tz = (atoi (timezone));
        tz_secs = -((tz / 100) + (long)((float)(tz % 100) / 60)) * 3600 -
                   local_dst();
        if (tz_secs < -90001 || tz_secs > 90001)  /* 25 hours plus one second */
            tz_secs = 0;                          /* Out of range, assume 0.  */

        done_init = TRUE;
      }
    return tz_secs;
}

/*  ---------------------------------------------------------------------[<]-
    Function: local_dst

    Synopsis: Checks for the existence of local Daylight Savings Time tag in
    ZONE environment variable, returns number of seconds in an hour if found.
    ZONE needs to be manually edited at the beginning and end of DST.  The 
    environment string is case-insensitive.
    Example of enabling  local DST:  set ZONE=+1000DST
    Example of disabling local DST:  set ZONE=+1000
    Author: Rob Judd <judd@alphalink.com.au>
    Extensively rewritten by iMatix SFL Team <sfl@imatix.com> to improve
    performance.
    ---------------------------------------------------------------------[>]-*/

int
local_dst (void)
{
    static Bool 
        done_init  = FALSE,
        is_dst     = FALSE;

    if (! done_init)
      {
#if defined (__WINDOWS__) && defined (_MSC_VER)
        is_dst = _daylight;
#else
        char * 
            timezone;

        timezone = env_get_string ("ZONE", "+0000");

        if (txtfind (timezone, "DST") != NULL)
            is_dst = TRUE;
        else
            is_dst = FALSE;
#endif
        done_init = TRUE;
      }

    if (is_dst)
        return (3600);
    else
        return (0);
}

/*  ---------------------------------------------------------------------[<]-
    Function: timestamp_now

    Synopsis: Get current date and time and encode into a real value (8 bytes).
    ---------------------------------------------------------------------[>]-*/

double 
timestamp_now (void)
{
    long
        current_date,
        current_time;

    get_date_time_now (&current_date, &current_time);
    return (make_timestamp (current_date, current_time));
}

/*  ---------------------------------------------------------------------[<]-
    Function: gmtimestamp_now

    Synopsis: Get current GMT date and time and encode into a real value 
    (8 bytes).
    ---------------------------------------------------------------------[>]-*/

double
gmtimestamp_now (void)
{
    long
        current_date,
        current_time,
        gm_date,
        gm_time;

    get_date_time_now (&current_date, &current_time);
    local_to_gmt (current_date, current_time, &gm_date, &gm_time);
    return (make_timestamp (gm_date, gm_time));
}

/*  ---------------------------------------------------------------------[<]-
    Function: make_timestamp

    Synopsis: Encode a date and a time into timestamp format
    ---------------------------------------------------------------------[>]-*/

double
make_timestamp (long date, long time)
{
    char
        buffer[20];

    snprintf (buffer, sizeof (buffer), "%ld.%07ld", date_to_days  (date),
                                                    time_to_csecs (time));
    buffer [sizeof (buffer) - 1] = '\0';
    return ((double) atof (buffer));
}


/*  ---------------------------------------------------------------------[<]-
    Function: timestamp_date

    Synopsis: Get date part of timestamp
    ---------------------------------------------------------------------[>]-*/

long
timestamp_date (double timestamp)
{
    long
        date;
    char
        buffer [20];

    snprintf (buffer, sizeof (buffer), "%08.07f", timestamp);
    buffer [7] = '\0';
    date = atol (buffer);
    date = days_to_date (date);

    return (date);
}

/*  ---------------------------------------------------------------------[<]-
    Function: timestamp_time

    Synopsis: Get time part of timestamp
    ---------------------------------------------------------------------[>]-*/

long
timestamp_time (double timestamp)
{
   long
        time;
    char
        buffer [20];

    snprintf (buffer, sizeof (buffer), "%08.07f", timestamp);
    buffer [7] = '\0';
    time = atol (&buffer [8]);
    time = csecs_to_time ((long)time);

    return (time);
}


/*  ---------------------------------------------------------------------[<]-
    Function: gmtimestamp_to_iso

    Synopsis: Returns the time passed in timestamp as a string in ISO
    8601 format.  The returned string will be formatted as follows:

    YYYY-MM-DDThh:mm:ss.sTZD (eg 1997-07-16T19:20:30.45Z)

    where:

     YYYY = four-digit year
     MM   = two-digit month (01=January, etc.)
     DD   = two-digit day of month (01 through 31)
     hh   = two digits of hour (00 through 23)
     mm   = two digits of minute (00 through 59)
     ss   = two digits of second (00 through 59)
     s    = one or more digits representing a decimal fraction of a second
     TZD  = time zone designator (Z or +hh:mm or -hh:mm)

    The caller is responsible for freeing the returned string.
    ---------------------------------------------------------------------[>]-*/

char *gmtimestamp_to_iso (double timestamp)
{
    char
        *iso_time,
        *buffer;
    long
        date,
        time;

    date = timestamp_date (timestamp);
    time = timestamp_time (timestamp);

    buffer = strprintf ("%04d-%02d-%02dT%02d:%02d:%02d.%ldZ",
                        GET_CCYEAR (date), GET_MONTH (date), GET_DAY (date),
                        GET_HOUR (time), GET_MINUTE (time), GET_SECOND (time),
                        GET_CENTI (time));
    iso_time = mem_alloc (strlen (buffer) + 1);
    if (iso_time == NULL)
        return NULL;
    strcpy (iso_time, buffer);

    return (iso_time);
}


/*  ---------------------------------------------------------------------[<]-
    Function: iso_to_gmtimestamp

    Synopsis: Returns the ISO 8601 formatted time passed in iso_time as a 
    timestamp in GMT. The passed string should be formatted as follows:

    YYYY-MM-DDThh:mm:ss.sTZD (eg 1997-07-16T19:20:30.45Z)

    where:

     YYYY = four-digit year
     MM   = two-digit month (01=January, etc.)
     DD   = two-digit day of month (01 through 31)
     hh   = two digits of hour (00 through 23)
     mm   = two digits of minute (00 through 59)
     ss   = two digits of second (00 through 59)
     s    = one or more digits representing a decimal fraction of a second
     TZD  = time zone designator (Z or +hh:mm or -hh:mm)
    ---------------------------------------------------------------------[>]-*/

double iso_to_gmtimestamp (const char *iso_time)
{
    int
        year,
        month,
        day,
        hour,
        minute,
        second,
        feedback;
    long
        centi,
        date,
        time;
    double
        timestamp;

    feedback = sscanf (iso_time, "%04d-%02d-%02dT%02d:%02d:%02d.%ldZ",
                       &year, &month, &day, &hour, &minute, &second, &centi);
    if (feedback != 7)
        return 0;

    date = MAKE_DATE (year / 100, year % 100, month, day);
    time = MAKE_TIME (hour, minute, second, (int) centi);
    timestamp = make_timestamp (date, time);

    return timestamp;
}
