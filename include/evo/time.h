// Evo C++ Library
/* Copyright 2019 Justin Crowell
Distributed under the BSD 2-Clause License -- see included file LICENSE.txt for details.
*/
///////////////////////////////////////////////////////////////////////////////
/** \file time.h Evo date and time classes. */
#pragma once
#ifndef INCL_evo_timestamp_h
#define INCL_evo_timestamp_h

#include "string.h"
#include "substring.h"
#include "impl/systime.h"

namespace evo {
/** \addtogroup EvoIO */
//@{

///////////////////////////////////////////////////////////////////////////////

/** \cond impl */
namespace impl_time {
    inline int parse_num(int& num, const char*& inp, const char* end) {
        const int OVERFLOW_THRESHOLD = Int::MAX / 10; // not exact, but good enough here
        const char* start = inp;
        num = 0;
        while (inp < end) {
            if (*inp < '0' || *inp > '9')
                break;
            if (num >= OVERFLOW_THRESHOLD) {
                num = 0;
                return Int::MAX; // overflow
            }
            num = (num * 10) + (*inp - '0');
            ++inp;
        }
        return (int)(inp - start);
    }

    inline double parse_frac(const char*& inp, const char* end) {
        double div = 1.0;
        double num = 0.0;
        while (inp < end) {
            if (*inp < '0' || *inp > '9')
                break;
            num = (num * 10) + (*inp - '0');
            div *= 10;
            ++inp;
        }
        num /= div;
        return num;
    }
}
/** \endcond */

///////////////////////////////////////////////////////////////////////////////

/** Structure holding a calendar date.
 - This holds year, month, and day fields
 - You can access the fields directly, or use methods as needed
 - Format with format(), parse with parse()
 - Validate with validate()
 - See also: TimeOfDay, DateTime
 .

\par Example

\code
#include <evo/time.h>
#include <evo/io.h>
using namespace evo;

int main() {
    Date date(1999, 12, 31);
    date.add_days(1);
    date.format(con().out) << NL;
    return 0;
}
\endcode

Output:
\code{.unparsed}
2000-01-01
\endcode
*/
struct Date {
    static const int YEAR_MIN = 1000;       ///< Minimum year for 4 digits
    static const int YEAR_MAX = 9999;       ///< Maximum year for 4 digits
    static const int MONTH_MIN = 1;         ///< First month per year
    static const int MONTH_MAX = 12;        ///< Last month per year
    static const int DAY_MIN = 1;           ///< First day per month
    static const int DAY_MAX = 31;          ///< Max day for any month -- see days_per_month()
    static const ulong JDN_MIN = 2086302;   ///< Minimum Julian Day Number for Jan 1, 1000
    static const ulong JDN_MOD = 2400000;   ///< Julian Day Number for Modified Julian Day, subtract from JDN to get MJD (smaller number), add back for normal JDN

    int year;   ///< 4 digit year (1000 - 9999)
    int month;  ///< Month of year (1 - 12)
    int day;    ///< Day of month (1 - 31)

    /** Constructor. */
    Date()
        { ::memset(this, 0, sizeof(Date)); }

    /** Constructor to initialize with date.
     - This _does not_ validate the fields, use valid_date() to check this
     .
     \param  year   4 digit year
     \param  month  Month of year
     \param  day    Day of month
    */
    Date(int year, int month, int day) : year(year), month(month), day(day) {
    }

    /** Copy constructor.
     \param  src  Source to copy
    */
    Date(const Date& src)
        { ::memcpy(this, &src, sizeof(Date)); }

    /** Assignment operator.
     \param  src  Source to copy
     \return      This
    */
    Date& operator=(const Date& src)
        { ::memcpy(this, &src, sizeof(Date)); return *this; }

    /** %Compare for equality with another date.
     \param  oth  Other date to compare to
     \return      Whether equal
    */
    bool operator==(const Date& oth) const
        { return (year == oth.year && month == oth.month && day == oth.day); }

    /** %Compare for inequality with another date.
     \param  oth  Other date to compare to
     \return      Whether inequal
    */
    bool operator!=(const Date& oth) const
        { return (year != oth.year || month != oth.month || day != oth.day); }

    /** %Compare whether less than another date.
     \param  oth  Other date to compare to
     \return      Whether less than
    */
    bool operator<(const Date& oth) const
        { return (year < oth.year || (year == oth.year && (month < oth.month || (month == oth.month && day < oth.day)))); }

    /** %Compare whether less than or equal to another date.
     \param  oth  Other date to compare to
     \return      Whether less than or equal
    */
    bool operator<=(const Date& oth) const
        { return (year < oth.year || (year == oth.year && (month < oth.month || (month == oth.month && day <= oth.day)))); }

    /** %Compare whether greater than another date.
     \param  oth  Other date to compare to
     \return      Whether greater than
    */
    bool operator>(const Date& oth) const
        { return (year > oth.year || (year == oth.year && (month > oth.month || (month == oth.month && day > oth.day)))); }

    /** %Compare whether greater than or equal to another date.
     \param  oth  Other date to compare to
     \return      Whether greater than or equal
    */
    bool operator>=(const Date& oth) const
        { return (year > oth.year || (year == oth.year && (month > oth.month || (month == oth.month && day >= oth.day)))); }

    /** %Compare to another date.
     \param  oth  Other date to compare to
     \return      Result (<0 if this is less, 0 if equal, >0 if this is greater)
    */
    int compare(const Date& oth) const {
        if (year == oth.year) {
            if (month == oth.month) {
                if (day == oth.day)
                    return 0;
                else if (day < oth.day)
                    return -1;
            } else if (month < oth.month)
                return -1;
        } else if (year < oth.year)
            return -1;
        return 1;
    }

    /** Get Julian Day Number for current date.
     - This is useful for turning a date into a single number in day units that is easy to add/subtract and convert back to a date
     - Call set_jdn() to convert a JDN back into date fields
     - For a Modified JDN: subtract JDN_MOD from a valid (non-zero) result, then add it back again
     - Notes:
       - Julian Days normally begin at noon, but this uses it purely as a date and pretends they begin at midnight -- time of day can be tracked separately
       - Not related to the Julian calendar
     .
     \return  Julian Day Number for current date, 0 if current date is not valid
    */
    ulong get_jdn() const {
        if (!validate())
            return 0;
        return calc_jdn(year, month, day);
    }

    /** Validate current date.
     \return  Whether date is valid, false if any field is invalid (out of range)
    */
    bool validate() const {
        return (year  >= YEAR_MIN  && year  <= YEAR_MAX &&
                month >= MONTH_MIN && month <= MONTH_MAX &&
                day   >= DAY_MIN   && day   <= days_per_month(month, year));
    }

    /** %Set all fields to 0, which is _not_ a valid date. */
    void set() {
        year = month = day = 0;
    }

    /** %Set new date fields.
     - This _does_ validate the fields before setting them
     .
     \param  new_year   New year value
     \param  new_month  New month value
     \param  new_day    New day value
     \return            Whether successful, false if one of the fields is invalid
    */
    bool set(int new_year, int new_month, int new_day) {
        if ( new_year  < YEAR_MIN  || new_year  > YEAR_MAX ||
             new_month < MONTH_MIN || new_month > MONTH_MAX ||
             new_day   < DAY_MIN   || new_day   > days_per_month(new_month, new_year) )
            return false;
        year  = new_year;
        month = new_month;
        day   = new_day;
        return true;
    }

    /** %Set date converted from Julian Day Number.
     - Call get_jdn() to convert date into a JDN first -- this is useful for turning a date into a single number in day units that is easy to add/subtract
     - For a Modified JDN: subtract JDN_MOD from a valid (non-zero) result, then add it back again before calling this
     - Notes:
       - Julian Days normally begin at noon, but this uses it purely as a date and pretends they begin at midnight -- time of day can be tracked separately
       - Not related to the Julian calendar
     .
     \param  jdn  Julian Day Number to set
     \return      Whether successful, false if JDN is invalid (0 or less than JDN_MIN)
    */
    bool set_jdn(ulong jdn) {
        if (jdn < JDN_MIN)
            return false;
        // https://en.wikipedia.org/wiki/Julian_day
        const long f = jdn + 1401 + (((4 * jdn + 274277) / 146097) * 3) / 4 + -38;
        const long e = 4 * f + 3;
        const long g = (e % 1461) / 4;
        const long h = 5 * g + 2;
        day   = (int)((h % 153) / 5 + 1);
        month = (int)(((h / 153 + 2) % 12) + 1);
        year  = (int)((e / 1461) - 4716 + (12 + 2 - month) / 12);
        return true;
    }

    /** %Set to current date in UTC. */
    void set_utc() {
        int hour, minute, second, msecond;
        SysTimestamp::get_wall_datetime_fields_utc(year, month, day, hour, minute, second, msecond);
    }

    /** %Set to current date in local time zone. */
    void set_local() {
        int hour, minute, second, msecond;
        SysTimestamp::get_wall_datetime_fields_local(year, month, day, hour, minute, second, msecond);
    }

    /** Add years to current date, subtract if negative.
     - This adds the given years and adjusts day of month to last day of month if not valid for the new year and month
     .
     \param  years  Number of years to add, negative to subtract
     \return        Whether successful, false if current date is invalid before or after modification
    */
    bool add_years(int years) {
        if (!validate())
            return false;
        year += years;
        const int maxday = days_per_month(month, year);
        if (day > maxday)
            day = maxday;
        return (year >= YEAR_MIN && year <= YEAR_MAX);
    }

    /** Add months to current date, subtract if negative.
     - This adds the given months and adjusts day of month to last day of month if not valid for the new year and month
     .
     \param  months  Number of months to add, negative to subtract
     \return         Whether successful, false if current date is invalid before or after modification
    */
    bool add_months(int months) {
        if (!validate())
            return false;
        year += (months / MONTH_MAX);
        month += (months % MONTH_MAX);
        if (month > MONTH_MAX) {
            ++year;
            month -= MONTH_MAX;
        } else if (month < MONTH_MIN) {
            --year;
            month += MONTH_MAX;
        }
        const int maxday = days_per_month(month, year);
        if (day > maxday)
            day = maxday;
        return (year >= YEAR_MIN && year <= YEAR_MAX);
    }

    /** Add days to current date, subtract if negative.
     - This adds the given days and adjusts months and years accordingly
     .
     \param  days  Number of days to add, negative to subtract
     \return       Whether successful, false if current date is invalid before or after modification
    */
    bool add_days(int days) {
        const ulong jdn = get_jdn();
        if (jdn == 0)
            return false;
        set_jdn(jdn + days);
        return (year >= YEAR_MIN && year <= YEAR_MAX);
    }

    /** %Set date from parsing standard date string (used internally).
     - Supported date syntax is based on <a href="https://en.wikipedia.org/wiki/ISO_8601">ISO 8601</a>:
       - `YYYY-MM-DD` -- full date with delimiter
       - `YYYYMMDD` -- shorter version without delimiters
       - `YYYY-DDD` -- ordinal date with delimiter, with 4 digit year (1000 - 999) and day of year (1 - 366)
       - `YYYYDDD` -- ordinal date, shorter version without delimiters
       - Alternate delimiters supported: `SPACE`, `SLASH` (`/`), `PERIOD` (`.`)
     .
     \param  inp  Start parsing pointer (must be `<= end`), set to stop position  [in/out]
     \param  end  End of string pointer, must not be NULL
     \return      Whether successful, false on bad input or unrecognized format
    */
    bool parse_std_impl(const char*& inp, const char* end) {
        year = month = day = 0;
        for (char delim;;) {
            // Year
            int digits = impl_time::parse_num(year, inp, end);
            switch (digits) {
                case 4:
                    break;
                case 7: {
                    // Ordinal date (YYYYDDD)
                    const int year_day = (year % 1000);
                    year /= 1000;
                    const int days_in_year = (is_leap_year(year) ? 366 : 365);
                    if (year_day <= 0 || year_day > days_in_year)
                        return false;
                    month = 1;
                    day = 1;
                    return add_days(year_day - 1);
                }
                case 8:
                    // YYYYMMDD
                    day = (year % 100);
                    year /= 100;
                    if (day < DAY_MIN)
                        return false;

                    month = (year % 100);
                    year /= 100;
                    if (month < MONTH_MIN || month > MONTH_MAX || day > days_per_month(month, year))
                        return false;

                    inp += digits;
                    return true;
                default:
                    return false;
            }
            if (inp >= end)
                return false;

            // Delim
            if (*inp != '-' && *inp != '/' && *inp != '.' && *inp != ' ')
                return false;
            delim = *inp;
            ++inp;

            // Month
            digits = impl_time::parse_num(month, inp, end);
            switch (digits) {
                case 1:
                case 2:
                    if (month < MONTH_MIN || month > MONTH_MAX)
                        return false;
                    break;
                case 3: {
                    // Ordinal date (YYYY-DDD)
                    const int year_day = month;
                    const int days_in_year = (is_leap_year(year) ? 366 : 365);
                    if (year_day <= 0 || year_day > days_in_year)
                        return false;
                    month = 1;
                    day = 1;
                    return add_days(year_day - 1);
                }
                default:
                    return false;
            }

            // Delim
            if (*inp != delim)
                return false;
            ++inp;

            // Day
            digits = impl_time::parse_num(day, inp, end);
            if (digits > 2 || day < DAY_MIN || day > days_per_month(month, year))
                return false;
            break;
        }
        return true;
    }

    /** %Set date from parsing standard date string (used internally).
     - Supported date syntax is based on <a href="https://en.wikipedia.org/wiki/ISO_8601">ISO 8601</a>:
       - `YYYY-MM-DD` -- full date with delimiter
       - `YYYYMMDD` -- shorter version without delimiters
       - `YYYY-DDD` -- ordinal date with delimiter, with 4 digit year (1000 - 999) and day of year (1 - 366)
       - `YYYYDDD` -- ordinal date, shorter version without delimiters
       - Alternate delimiters supported: `SPACE`, `SLASH` (`/`), `PERIOD` (`.`)
     .
     \param  str  %String to parse
     \return      Whether successful, false on bad input or unrecognized format
    */
    bool parse(const SubString& str) {
        const char* p = str.data();
        return parse_std_impl(p, p + str.size());
    }

    /** Format date to String or Stream.
     - %Date syntax is based on <a href="https://en.wikipedia.org/wiki/ISO_8601">ISO 8601</a>:
       - `YYYY-MM-DD` -- with default delimiter, other delimiters may be use but aren't considered standard
       - `YYYYMMDD` -- with no delimiter
     - See also: format_yearday()
     .
     \tparam  T  String/Stream type, inferred from argument
     \param  out    Output String/Stream to format to
     \param  delim  Delimiter to use between fields, 0 for none
     \return        Reference to `out`
    */
    template<class T>
    T& format(T& out, char delim='-') const {
        out << FmtInt(year, fDEC, fPREFIX0, 4, '0');
        if (delim > 0)
            out << delim << FmtInt(month, fDEC, fPREFIX0, 2, '0') << delim;
        else
            out << FmtInt(month, fDEC, fPREFIX0, 2, '0');
        out << FmtInt(day, fDEC, fPREFIX0, 2, '0');
        return out;
    }

    /** Format ordinal date to String or Stream.
     - %Date syntax is based on <a href="https://en.wikipedia.org/wiki/ISO_8601">ISO 8601</a> ordinal date:
       - `YYYY-DDD` -- ordinal date with default delimiter, other delimiters may be use but aren't considered standard
       - `YYYYDDD` -- ordinal date with no delimiter
     - See also: format()
     .
     \tparam  Output String/Stream type, inferred from argument
     \param  out    Output String/Stream to format to
     \param  delim  Delimiter to use between fields, 0 for none
     \return        Reference to `out`
    */
    template<class T>
    T& format_yearday(T& out, char delim='-') {
        out << FmtInt(year, fDEC, fPREFIX0, 4, '0');
        if (delim > 0)
            out << delim;
        if (month > 0 && day > 0)
            out << FmtULong(calc_jdn(year, month, day) - calc_jdn(year, 1, 1) + 1, fDEC, fPREFIX0, 3, '0');
        else
            out << FmtInt(1, fDEC, fPREFIX0, 3, '0');
        return out;
    }

    /** Helper to check whether given year is a leap year.
     - Leap years are years divisible by 400, or years divisible by 4 but not divisible by 100
     .
     \param  year  Year to check
     \return       Whether a leap year
    */
    static bool is_leap_year(int year) {
        return (year % 400 == 0 || (year % 4 == 0 && year % 100 != 0));
    }

    /** Helper to get number of days per given month.
     \param  month      Month to get number of days of (1 - 12)
     \param  leap_year  Whether this is for a leap year, which affects the result for `month=2`
     \return            Number of days in `month`
    */
    static int days_per_month(int month, bool leap_year) {
        static const int DAYS[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
        assert( month >= MONTH_MIN && month <= MONTH_MAX );
        if (month == 2 && leap_year)
            return 29;
        return DAYS[month - 1];
    }

    /** Helper to get number of days per given month in given year.
     \param  month  Month to get number of days of (1 - 12)
     \param  year   Year for month, used to determine whether a leap year applies for `month=2`
     \return        Number of days in `month`
    */
    static int days_per_month(int month, int year) {
        return days_per_month(month, is_leap_year(year));
    }

    /** Get name of given month.
     - String is always static/immutable and terminated
     .
     \param  month  Month to get name of (1 - 12)
     \return        Name of month, null if invalid `month`
    */
    static SubString month_name(int month) {
        static const char* MONTHS[] = { "January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December" };
        if (month < MONTH_MIN || month > MONTH_MAX)
            return SubString();
        return MONTHS[month - 1];
    }

    /** Get abbreviated name of given month.
     - String is always static/immutable and terminated
     .
     \param  month  Month to get name of (1 - 12)
     \return        Name of month, null if invalid `month`
    */
    static SubString month_name3(int month) {
        static const char* MONTHS[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
        if (month < MONTH_MIN || month > MONTH_MAX)
            return SubString();
        return MONTHS[month - 1];
    }

    /** Calculate Julian Day Number from date fields.
     - This does not validate the input
     - Notes:
       - Julian Days normally begin at noon, but this uses it purely as a date and pretends they begin at midnight -- time of day can be tracked separately
       - Not related to the Julian calendar
     .
     \param  year   Year to use (1000 - 9999)
     \param  month  Month of year to use (1 - 12)
     \param  day    Day of month to use (1 - 31)
     \return        Julian Day Number for date
    */
    static ulong calc_jdn(int year, int month, int day) {
        // https://en.wikipedia.org/wiki/Julian_day
        return (1461 * (year + 4800 + (month - 14) / 12)) / 4 +
            (367 * (month - 2 - 12 * ((month - 14) / 12))) / 12 -
            (3 * ((year + 4900 + (month - 14) / 12) / 100)) / 4 + day - 32075;
    }
};

///////////////////////////////////////////////////////////////////////////////

/** Structure holding a time of day.
 - This holds hour, minute, second, and millisecond fields.
 - You can access the fields directly, or use methods as needed
 - Format with format(), parse with parse()
 - Validate with validate()
 - See also: Date, DateTime
 .

\par Example

\code
#include <evo/time.h>
#include <evo/io.h>
using namespace evo;

int main() {
    TimeOfDay time(12, 30, 59);
    time.add_minutes(15);
    time.format(con().out) << NL;
    return 0;
}
\endcode

Output:
\code{.unparsed}
12:45:59
\endcode
*/
struct TimeOfDay {
    static const int HOUR_MIN = 0;      ///< Minimum hour value
    static const int HOUR_MAX = 23;     ///< Maximum hour value
    static const int MINUTE_MIN = 0;    ///< Minimum minute value
    static const int MINUTE_MAX = 59;   ///< Maximum minute value
    static const int SECOND_MIN = 0;    ///< Minimum second value
    static const int SECOND_MAX = 60;   ///< Maximum second value
    static const int MSECOND_MIN = 0;   ///< Minimum millisecond value
    static const int MSECOND_MAX = 999; ///< Maximum millisecond value

    static const int HOURS_PER_DAY = 24;    ///< Number of hours per day
    static const int MIN_PER_HOUR = 60;     ///< Number of minutes per hour
    static const int MIN_PER_DAY  = 1440;   ///< Number of minutes per day
    static const int SEC_PER_MIN  = 60;     ///< Number of seconds per minute
    static const int SEC_PER_HOUR = 3600;   ///< Number of seconds per hour
    static const int SEC_PER_DAY  = 86400;  ///< Number of seconds per day
    static const int MSEC_PER_SEC = 1000;   ///< Number of milliseconds per second
    static const int MSEC_PER_MIN = 60000;  ///< Number of milliseconds per minute
    static const long MSEC_PER_HOUR = 3600000;  ///< Number of milliseconds per hour
    static const long MSEC_PER_DAY  = 86400000; ///< Number of milliseconds per day

    int hour;       ///< Hour of day (0 - 23)
    int minute;     ///< Minute of hour (0 - 59)
    int second;     ///< Second of minute (0 - 60), `60` is a special case for a leap second (rare)
    int msecond;    ///< Millisecond of second (0 - 999)

    /** Constructor. */
    TimeOfDay()
        { ::memset(this, 0, sizeof(TimeOfDay)); }

    /** Constructor to initialize with time of day.
     \param  hour  Hour of day
     \param  min   Minute of hour
     \param  sec   Second of minute
     \param  msec  Millisecond of second
    */
    TimeOfDay(int hour, int min=0, int sec=0, int msec=0) : hour(hour), minute(min), second(sec), msecond(msec) {
    }

    /** Copy constructor.
     \param  src  Source to copy
    */
    TimeOfDay(const TimeOfDay& src)
        { ::memcpy(this, &src, sizeof(TimeOfDay)); }

    /** Assignment operator.
     \param  src  Source to copy
     \return      This
    */
    TimeOfDay& operator=(const TimeOfDay& src)
        { ::memcpy(this, &src, sizeof(TimeOfDay)); return *this; }

    /** %Compare for equality with another time of day.
     \param  oth  Other time of day to compare to
     \return      Whether equal
    */
    bool operator==(const TimeOfDay& oth) const
        { return (hour == oth.hour && minute == oth.minute && second == oth.second && msecond == oth.msecond); }

    /** %Compare for inequality with another time of day.
     \param  oth  Other time of day to compare to
     \return      Whether inequal
    */
    bool operator!=(const TimeOfDay& oth) const
        { return (hour != oth.hour || minute != oth.minute || second != oth.second || msecond != oth.msecond); }

    /** %Compare whether less than another time of day.
     \param  oth  Other time of day to compare to
     \return      Whether less than
    */
    bool operator<(const TimeOfDay& oth) const
        { return (hour < oth.hour || (hour == oth.hour && (minute < oth.minute || (minute == oth.minute && (second < oth.second || (second == oth.second && msecond < oth.msecond)))))); }

    /** %Compare whether less than or equal to another time of day.
     \param  oth  Other time of day to compare to
     \return      Whether less than or equal
    */
    bool operator<=(const TimeOfDay& oth) const
        { return (hour < oth.hour || (hour == oth.hour && (minute < oth.minute || (minute == oth.minute && (second < oth.second || (second == oth.second && msecond <= oth.msecond)))))); }

    /** %Compare whether greater than another time of day.
     \param  oth  Other time of day to compare to
     \return      Whether greater than
    */
    bool operator>(const TimeOfDay& oth) const
        { return (hour > oth.hour || (hour == oth.hour && (minute > oth.minute || (minute == oth.minute && (second > oth.second || (second == oth.second && msecond > oth.msecond)))))); }

    /** %Compare whether greater than or equal to another time of day.
     \param  oth  Other time of day to compare to
     \return      Whether greater than or equal
    */
    bool operator>=(const TimeOfDay& oth) const
        { return (hour > oth.hour || (hour == oth.hour && (minute > oth.minute || (minute == oth.minute && (second > oth.second || (second == oth.second && msecond >= oth.msecond)))))); }

    /** %Compare to another time of day.
     \param  oth  Other time of day to compare to
     \return      Result (<0 if this is less, 0 if equal, >0 if this is greater)
    */
    int compare(const TimeOfDay& oth) const {
        if (hour == oth.hour) {
            if (minute == oth.minute) {
                if (second == oth.second) {
                    if (msecond == oth.msecond)
                        return 0;
                    else if (msecond < oth.msecond)
                        return -1;
                } else if (second < oth.second)
                    return -1;
            } else if (minute < oth.minute)
                return -1;
        } else if (hour < oth.hour)
            return -1;
        return 1;
    }

    /** Get current time as a day fraction.
     - This turns the separate time fields into a single floating-point number, where the fraction is the time of day
     - Examples:
       - `0.0` for midnight
       - `0.25` for 6 AM
       - `0.5` for noon
       - `0.75` for 6 PM
     - \b Caution: Milliseconds aren't stored in the fraction (not enough precision) and are ignored here
     - See set_fraction()
     .
     \return  Time as a day fraction, `0.0` for midnight, `0.5` for noon, etc
    */
    double get_fraction() const {
        return ((double)hour / HOURS_PER_DAY) +
               ((double)minute / MIN_PER_DAY) +
               ((double)second / SEC_PER_DAY);
    }

    /** Get number of days from current hour value.
     - This simply dvides the current hour by `24`
     .
     \return  Number of days, negative if current hour is under -23
    */
    int get_days() const {
        return (hour / HOURS_PER_DAY);
    }

    /** Get number of days and hours from current hour value.
     - This also gets a `result_hour` value and results are normalized so hour isn't negative (when current input hour is negative)
       - Examples:
         - `hour=47` returns `1 day, 23 hours`
         - `hour=-47` returns `-2 days, 1 hour`
     - This is useful when a TimeOfDay is combined with a Date -- see DateTime
     .
     \param  result_hour  Stores result hour
     \return  Number of days, negative if current hour is under -23
    */
    int get_days(int& result_hour) const {
        int days = (hour / HOURS_PER_DAY);
        result_hour = hour - (days * HOURS_PER_DAY);
        if (result_hour < 0) {
            --days;
            result_hour += HOURS_PER_DAY;
        }
        return days;
    }

    /** Validate current time of day.
     \param  allow_hour_overflow  Allow `hour` higher than `23` -- use get_days() to get number of days
     \return                      Whether time of day is valid, false if any field is invalid (negative or out of range)
    */
    bool validate(bool allow_hour_overflow=false) const {
        return (hour >= HOUR_MIN && (allow_hour_overflow || hour <= HOUR_MAX) &&
                minute >= MINUTE_MIN && minute <= MINUTE_MAX &&
                second >= SECOND_MIN && second <= SECOND_MAX &&
                msecond >= MSECOND_MIN && msecond <= MSECOND_MAX);
    }

    /** %Set all fields to 0, which is a valid time (midnight). */
    void set()
         { hour = minute = second = msecond = 0; }

    /** %Set new time of day fields.
     - This _does_ validate the fields before setting them
     .
     \param  new_hour     New hour value
     \param  new_minute   New minute value
     \param  new_second   New second value
     \param  new_msecond  New millisecond value
     \return              Whether successful, false if one of the fields is invalid
    */
    bool set(int new_hour, int new_minute=0, int new_second=0, int new_msecond=0) {
        if ( new_hour < HOUR_MIN || new_hour > HOUR_MAX ||
             new_minute < MINUTE_MIN || new_minute > MINUTE_MAX ||
             new_second < SECOND_MIN || new_second > SECOND_MAX ||
             new_msecond < MSECOND_MIN || new_msecond > MSECOND_MAX )
            return false;
        hour = new_hour;
        minute = new_minute;
        second = new_second;
        msecond = new_msecond;
        return true;
    }

    /** %Set time fields from day fraction.
     - \b Caution: Milliseconds aren't stored in the fraction (not enough precision) so `msecond` is set from `msec`
     - See get_fraction()
     .
     \param  tm    Time as a day fraction, `0.0` for midnight, `0.5` for noon, etc
     \param  msec  Milliseconds to set (0 - 999), defaults to `0`
    */
    void set_fraction(double tm, int msec=0) {
        hour = (int)(tm * HOURS_PER_DAY);
        tm -= (double)hour / HOURS_PER_DAY;
        minute = (int)(tm * MIN_PER_DAY);
        tm -= (double)minute / MIN_PER_DAY;
        second = (int)(tm * SEC_PER_DAY);
        msecond = msec;
    }

    /** %Set to current UTC time of day. */
    void set_utc() {
        int year, month, day;
        SysTimestamp::get_wall_datetime_fields_utc(year, month, day, hour, minute, second, msecond);
    }

    /** %Set to current local time of day. */
    void set_local() {
        int year, month, day;
        SysTimestamp::get_wall_datetime_fields_local(year, month, day, hour, minute, second, msecond);
    }

    /** Add minutes to current time, subtract if negative.
     - This adds the given minutes and adjusts the hour accordingly
     - Current minute is kept in range, but this does not check if the resulting hour is within a valid range -- see validate() to check that
     .
     \param  minutes  Number of minutes to add, negative to subtract
    */
    void add_minutes(int minutes) {
        minute += minutes;
        int newhours = (minute / MIN_PER_HOUR);
        hour += newhours;
        minute -= (newhours * MIN_PER_HOUR);
        if (minute < 0) {
            --hour;
            minute += MIN_PER_HOUR;
        }
    }

    /** Add seconds to current time, subtract if negative.
     - This adds the given seconds and adjusts the minute and hour accordingly
     - Current second and minute are kept in range, but this does not check if the resulting hour is within a valid range -- see validate() to check that
     .
     \param  seconds  Number of seconds to add, negative to subtract
    */
    void add_seconds(int seconds) {
        if (second == 60)
            second = 59; // ignore leap second so it doesn't throw off the math

        long total_seconds = (minute * SEC_PER_MIN) + second + seconds;
        int newhours = (total_seconds / SEC_PER_HOUR);
        total_seconds -= (newhours * SEC_PER_HOUR);

        hour += newhours;
        minute = (total_seconds / SEC_PER_MIN);
        total_seconds -= (minute * SEC_PER_MIN);

        second = (int)total_seconds;
        if (second < 0) {
            --minute;
            second += SEC_PER_MIN;
        }
        if (minute < 0) {
            --hour;
            minute += MIN_PER_HOUR;
        }
    }

    /** Add milliseconds to current time, subtract if negative.
     - This adds the given milliseconds and adjusts the second, minute, and hour accordingly
     - Current millisecond, second, and minute are all kept in range, but this does not check if the resulting hour is within a valid range -- see validate() to check that
     .
     \param  milliseconds  Number of milliseconds to add, negative to subtract
    */
    void add_milliseconds(int milliseconds) {
        msecond += milliseconds;
        int newseconds = (msecond / MSEC_PER_SEC);
        msecond -= (newseconds * MSEC_PER_SEC);
        if (msecond < 0) {
            --newseconds;
            msecond += MSEC_PER_SEC;
        }
        add_seconds(newseconds);
    }

    /** %Set time of day from parsing standard time string (used internally).
     - Supported time syntax is based on <a href="https://en.wikipedia.org/wiki/ISO_8601">ISO 8601</a>:
       - `HH:MM:SS` -- full time of day with delimiters, but without milliseconds
       - `HH:MM` -- time of day with delimiter and only hour and minute
       - `HH` -- time of day with only hour value
       - `HHMM` -- time of day with hour and minute, without delimimter
       - `HHMMSS` -- full time of day without delimiters
       - `HH:MM:SS.fff` or `HHMMSS.fff` -- full time of day with fractional second (milliseconds)
       - `HH:MM.fff` or `HHMM.fff` -- full time of day with fractional minute (to give second and millisecond values)
       - `HH.fff` or `HH.fff` -- full time of day with fractional hour (to give minute, second, and millisecond values)
       - Alternate delimiter supported for fractional or millisecond values: `COMMA` (`,`)
     .
     \param  inp                  Start parsing pointer (must be `<= end`), set to stop position  [in/out]
     \param  end                  End of string pointer, must not be NULL
     \param  allow_hour_overflow  Allow `hour` higher than `23`, but still must be 2 digits
     \return                      Whether successful, false on bad input or unrecognized format
    */
    bool parse_std_impl(const char*& inp, const char* end, bool allow_hour_overflow=false) {
        const double ROUND_MSEC = 0.001;
        hour = minute = second = msecond = 0;
        for (;;) {
            // Hour
            int digits = impl_time::parse_num(hour, inp, end);
            switch (digits) {
                case 1:
                case 2:
                    break;
                case 4:
                    minute = hour % 100;
                    hour /= 100;
                    if ( hour < HOUR_MIN || (hour > HOUR_MAX && !allow_hour_overflow) ||
                         minute < MINUTE_MIN || minute > MINUTE_MAX )
                        return false;
                    if (inp < end && (*inp == '.' || *inp == ','))
                        goto frac_min;
                    return true;
                case 6:
                    second = hour % 100;
                    hour /= 100;
                    minute = hour % 100;
                    hour /= 100;
                    if ( hour < HOUR_MIN || (hour > HOUR_MAX && !allow_hour_overflow) ||
                         minute < MINUTE_MIN || minute > MINUTE_MAX ||
                         second < SECOND_MIN || second > SECOND_MAX )
                        return false;
                    goto endfrac;
                default:
                    return false; // invalid
            }

            if (hour < HOUR_MIN || (hour > HOUR_MAX && !allow_hour_overflow))
                return false;
            if (inp >= end)
                break;

            if (*inp == '.' || *inp == ',') {
                // Fractional hour
                double frac = impl_time::parse_frac(++inp, end);
                frac *= MIN_PER_HOUR;
                minute = (int)frac;
                frac -= minute;

                frac *= SEC_PER_MIN;
                second = (int)frac;
                frac -= second;

                msecond = (int)((frac * MSEC_PER_SEC) + ROUND_MSEC);
                return true;
            }
            if (*inp != ':')
                break;
            ++inp;

            // Minute
            digits = impl_time::parse_num(minute, inp, end);
            switch (digits) {
                case 1:
                case 2:
                    break;
                default:
                    return false; // invalid
            }
            if (minute < MINUTE_MIN || minute > MINUTE_MAX)
                return false;

            if (*inp == '.' || *inp == ',') {
                // Fractional minute
            frac_min:
                double frac = impl_time::parse_frac(++inp, end);
                frac *= SEC_PER_MIN;
                second = (int)frac;
                frac -= second;

                msecond = (int)((frac * MSEC_PER_SEC) + ROUND_MSEC);
                break;
            }
            if (*inp != ':')
                break;
            ++inp;

            // Second
            digits = impl_time::parse_num(second, inp, end);
            switch (digits) {
                case 1:
                case 2:
                    break;
                default:
                    return false; // invalid
            }
            if (second < SECOND_MIN || second > SECOND_MAX)
                return false;

        endfrac:
            if (*inp == '.' || *inp == ',') {
                // Fractional second, i.e. millisecond
                msecond = (int)((impl_time::parse_frac(++inp, end) * MSEC_PER_SEC) + ROUND_MSEC);
                break;
            }

            break;
        }
        return true;
    }

    /** %Set time of day from parsing standard time string.
     - Supported time syntax is based on <a href="https://en.wikipedia.org/wiki/ISO_8601">ISO 8601</a>:
       - `HH:MM:SS` -- full time of day with delimiters, but without milliseconds
       - `HH:MM` -- time of day with delimiter and only hour and minute
       - `HH` -- time of day with only hour value
       - `HHMM` -- time of day with hour and minute, without delimimter
       - `HHMMSS` -- full time of day without delimiters
       - `HH:MM:SS.fff` or `HHMMSS.fff` -- full time of day with fractional second (milliseconds)
       - `HH:MM.fff` or `HHMM.fff` -- full time of day with fractional minute (to give second and millisecond values)
       - `HH.fff` or `HH.fff` -- full time of day with fractional hour (to give minute, second, and millisecond values)
       - Alternate delimiter supported for fractional or millisecond values: `COMMA` (`,`)
     .
     \param  str                  %String to parse
     \param  allow_hour_overflow  Allow `hour` higher than `23`, but still must be 2 digits
     \return                      Whether successful, false on bad input or unrecognized format
    */
    bool parse(const SubString& str, bool allow_hour_overflow=false) {
        const char* p = str.data();
        return parse_std_impl(p, p + str.size(), allow_hour_overflow);
    }

    /** Format time of day to String or Stream.
     \tparam  T  String/Stream type, inferred from argument
     \param  out      Output String/Stream to format to
     \param  delim    Time delimiter, 0 for none, ':' for standard -- other delimiters are not supported by parse()
     \param  msdelim  Delimiter for milliseconds, 0 to not format milliseconds
     \return          Reference to `out`
    */
    template<class T>
    T& format(T& out, char delim=':', char msdelim=0) const {
        out << FmtInt(hour, fDEC, fPREFIX0, 2, '0');
        if (delim > 0)
            out << delim << FmtInt(minute, fDEC, fPREFIX0, 2, '0') << delim;
        else
            out << FmtInt(minute, fDEC, fPREFIX0, 2, '0');
        out << FmtInt(second, fDEC, fPREFIX0, 2, '0');
        if (msecond > 0 && msdelim > 0)
            out << msdelim << FmtInt(msecond, fDEC, fPREFIX0, 3, '0');
        return out;
    }
};

///////////////////////////////////////////////////////////////////////////////

/** Structure holding a time zone offset from UTC.
 - This holds the time zone offset from UTC in minutes
   - Examples:
     - UTC offset: 0
     - Pacific Standard Time (UTC -8) offset: -480
     - Pacific Daylight Time (UTC -7) offset: -420
     - New Zealand Standard Time (UTC +12) offset: 720
     - New Zealand Daylight Time (UTC +13) offset: 780
 - This can be null, meaning no time zone is assigned
 - Format with format(), parse with parse()
 - Validate with validate()
 - See also: DateTime
 .
*/
struct TimeZoneOffset {
    static const int OFFSET_MIN = -1439;        ///< Minimum time zone offset in minutes (normally -720 but enforced at -1439)
    static const int OFFSET_MAX = 1439;         ///< Maximum time zone offset in minutes (normally 840 but enforced at 1439)
    static const int OFFSET_NULL = Int::MIN;    ///< Special value for null time zone (null is less than all other values), i.e. no assigned time zone

    int minutes;    ///< Time zone offset from UTC in minutes (-720 - 840), OFFSET_NULL for null, negative for the Western Hemisphere (America), positive for the remaining time zones

    /** Constructor to set as null (no time zone). */
    TimeZoneOffset()
        { minutes = OFFSET_NULL; }

    /** Constructor to initialize with a time zone offset in minutes.
     \param  minutes  Time zone offset from UTC in minutes, 0 for UTC, negative for the Western Hemisphere (America), positive for the remaining time zones, OFFSET_NULL for null (no time zone)
    */
    TimeZoneOffset(int minutes) : minutes(minutes) {
    }

    /** Constructor to initialize with a time zone offset from hours and minutes.
     - This does _not_ validate the arguments -- use validate() to validate
     .
     \param  hours         Offset hours (-23 - 23)
     \param  hour_minutes  Offset minutes (0 - 59)
    */
    TimeZoneOffset(int hours, int hour_minutes) {
        if (hours < 0)
            minutes = (hours * TimeOfDay::MIN_PER_HOUR) - hour_minutes;
        else
            minutes = (hours * TimeOfDay::MIN_PER_HOUR) + hour_minutes; 
    }

    /** Copy constructor.
     \param  src  Source to copy
    */
    TimeZoneOffset(const TimeZoneOffset& src)
        { minutes = src.minutes; }

    /** Assignment operator.
     \param  src  Source to copy
     \return      This
    */
    TimeZoneOffset& operator=(const TimeZoneOffset& src)
        { minutes = src.minutes; return *this; }

    /** %Compare for equality with another time of day.
     \param  oth  Other time of day to compare to
     \return      Whether equal
    */
    bool operator==(const TimeZoneOffset& oth) const
        { return (minutes == oth.minutes); }

    /** %Compare for inequality with another time of day.
     \param  oth  Other time of day to compare to
     \return      Whether inequal
    */
    bool operator!=(const TimeZoneOffset& oth) const
        { return (minutes != oth.minutes); }

    /** %Compare whether less than another time of day.
     \param  oth  Other time of day to compare to
     \return      Whether less than
    */
    bool operator<(const TimeZoneOffset& oth) const
        { return (minutes < oth.minutes); }

    /** %Compare whether less than or equal to another time of day.
     \param  oth  Other time of day to compare to
     \return      Whether less than or equal
    */
    bool operator<=(const TimeZoneOffset& oth) const
        { return (minutes <= oth.minutes); }

    /** %Compare whether greater than another time of day.
     \param  oth  Other time of day to compare to
     \return      Whether greater than
    */
    bool operator>(const TimeZoneOffset& oth) const
        { return (minutes > oth.minutes); }

    /** %Compare whether greater than or equal to another time of day.
     \param  oth  Other time of day to compare to
     \return      Whether greater than or equal
    */
    bool operator>=(const TimeZoneOffset& oth) const
        { return (minutes >= oth.minutes); }

    /** %Compare to another time zone offset.
     \param  oth  Other time zone to compare to
     \return      Result (<0 if this is less, 0 if equal, >0 if this is greater)
    */
    int compare(const TimeZoneOffset& oth) const
        { return (minutes == oth.minutes ? 0 : (minutes < oth.minutes ? -1 : 1)); }

    /** Get whether time zone offset is null (not set).
     \return  Whether null
    */
    bool null() const 
        { return minutes == OFFSET_NULL; }

    /** Validate current timezone offset.
     \return  Whether timzone offset is valid, false if value is invalid (not null and out of range)
    */
    bool validate() const
        { return (minutes == OFFSET_NULL || (minutes >= OFFSET_MIN && minutes <= OFFSET_MAX)); }

    /** %Set as null (no time zone). */
    void set()
        { minutes = OFFSET_NULL; }

    /** %Set to time zone offset in minutes.
     \param  new_minutes  Time zone offset from UTC in minutes, 0 for UTC, negative for the Western Hemisphere (America), positive for the remaining time zones, OFFSET_NULL for null (no time zone)
     \return              Whether successful, false if field is invalid
    */
    bool set(int new_minutes) {
        if (new_minutes == OFFSET_NULL || (new_minutes >= OFFSET_MIN && new_minutes <= OFFSET_MAX)) {
            minutes = new_minutes;
            return true;
        }
        return false;
    }

    /** %Set new time zone offset from hours and minutes.
     \param  hours         Offset hours (-23 - 23)
     \param  hour_minutes  Offset minutes (0 - 59)
     \return               Whether successful, false if one of the fields is invalid
    */
    bool set(int hours, int hour_minutes) {
        if ( hours <= -TimeOfDay::HOURS_PER_DAY || hours >= TimeOfDay::HOURS_PER_DAY ||
             hour_minutes < TimeOfDay::MINUTE_MIN || hour_minutes > TimeOfDay::MINUTE_MAX )
            return false;
        if (hours < 0)
            minutes = (hours * TimeOfDay::MIN_PER_HOUR) - hour_minutes;
        else
            minutes = (hours * TimeOfDay::MIN_PER_HOUR) + hour_minutes; 
        return true;
    }

    /** %Set to UTC. */
    TimeZoneOffset& set_utc() {
        minutes = 0;
        return *this;
    }

    /** %Set to current time zone offset.
     - If the system returns an error (time zone unknown or unavailable), this assumes UTC
    */
    TimeZoneOffset& set_local() {
        minutes = SysTimestamp::tz_get_offset();
        return *this;
    }

    /** %Set timezone offset from parsing standard timezone offset string (used internally).
     - Supported timezone offset syntax is based on <a href="https://en.wikipedia.org/wiki/ISO_8601">ISO 8601</a>:
       - `Z` or `z` if no offset, same if empty or a non-digit character is found that doesn't match the syntax
       - `+HH:MM` or `-HH:MM` -- offset with standard delimiter, other delimiters not supported
       - `+HHMM` or `-HHMM` -- offset without a delimiter
       - `+HH` or `-HH` -- offset with just hours
       - Sign is required with numeric offset
     - An empty time zone is treated as null if `required=false`, or an error if `required=true`
     .
     \param  inp       Start parsing pointer (must be `<= end`), set to stop position  [in/out]
     \param  end       End of string pointer, must not be NULL
     \param  required  Whether a value is required, false if null is ok, true if null is an error
     \return           Whether successful, false on bad input or unrecognized format or if no value while `required=true`
    */
    bool parse_std_impl(const char*& inp, const char* end, bool required=false) {
        minutes = OFFSET_NULL;
        if (inp >= end)
            return !required; // no offset

        int neg_mult;
        switch (*inp) {
            case 'Z':
            case 'z':
                minutes = 0;
                return true; // UTC
            case '+':
                neg_mult = 1;
                break;
            case '-':
                neg_mult = -1;
                break;
            default:
                if (*inp >= '0' && *inp <= '9')
                    return false; // invalid
                return !required; // no offset
        }

        int hours = 0;
        int digits = impl_time::parse_num(hours, ++inp, end);
        switch (digits) {
            case 1:
            case 2:
                break;
            case 4:
                minutes = (hours / 100 * TimeOfDay::MIN_PER_HOUR * neg_mult) + (hours % 100 * neg_mult);
                return (minutes >= OFFSET_MIN && minutes <= OFFSET_MAX);
            default:
                return false;
        }
        if (hours >= TimeOfDay::HOURS_PER_DAY)
            return false;

        if (inp >= end || *inp != ':') {
            minutes = (hours * TimeOfDay::MIN_PER_HOUR * neg_mult);
            return true;
        }
        if (++inp >= end)
            return false;

        int hour_minutes = 0;
        digits = impl_time::parse_num(hour_minutes, inp, end);
        if (digits > 2 || hour_minutes > TimeOfDay::MINUTE_MAX)
            return false;
        
        minutes = (hours * TimeOfDay::MIN_PER_HOUR * neg_mult) + (hour_minutes * neg_mult);
        return true;
    }

    /** %Set timezone offset from parsing standard timezone offset string.
     - Supported timezone offset syntax is based on <a href="https://en.wikipedia.org/wiki/ISO_8601">ISO 8601</a>:
       - `Z` or `z` if no offset
       - `+HH:MM` or `-HH:MM` -- offset with standard delimiter, other delimiters not supported
       - `+HHMM` or `-HHMM` -- offset without a delimiter
       - `+HH` or `-HH` -- offset with just hours
       - Sign is required with numeric offset
     - An empty time zone is treated as null if `required=false`, or an error if `required=true`
     .
     \param  str       %String to parse
     \param  required  Whether a value is required, false if null is ok, true if null is an error
     \return           Whether successful, false on bad input or unrecognized format
    */
    bool parse(const SubString& str, bool required=false) {
        const char* p = str.data();
        return parse_std_impl(p, p + str.size(), required);
    }

    /** Format timezone offset to String or Stream.
     - Syntax is based on <a href="https://en.wikipedia.org/wiki/ISO_8601">ISO 8601</a>:
       - `Z` if no offset and `allow_z=true`
       - `+HH:MM` -- positive offset with default delimiter, other delimiters may be used but aren't considered standard
       - `-HH:MM` -- negative offset with default delimiter, other delimiters may be used but aren't considered standard
       - `+HHMM` -- positive offset without a delimiter
       - `-HHMM` -- negative offset without a delimiter
     .
     \param  out      Output String/Stream to format to
     \param  delim    Delimiter to use between fields, usually `:` or 0 for no delimiter -- other values are non-standard
     \param  allow_z  Whether to allow formatting as `Z` for UTC, false to use `+00:00` instead (where `:` is `delim`)
     \return          Reference to `out`
    */
    template<class T>
    T& format(T& out, char delim=':', bool allow_z=true) const {
        if (minutes == OFFSET_NULL)
            return out;
        if (allow_z && minutes == 0) {
            out << 'Z';
        } else {
            int hours = minutes / 60;
            int hour_minutes = minutes % 60;
            if (hours < 0) {
                out << '-';
                hours *= -1;
                hour_minutes *= -1;
            } else
                out << '+';
            out << FmtInt(hours, fDEC, fPREFIX0, 2, '0');
            if (delim > 0)
                out << delim;
            out << FmtInt(hour_minutes, fDEC, fPREFIX0, 2, '0');
        }
        return out;
    }
};

///////////////////////////////////////////////////////////////////////////////

/** Full calendar date and time of day with timezone offset.
 - This combines Date, TimeOfDay, and TimeZoneOffset as members to form a full calendar date and time
 - You can access the fields directly, or use methods as needed
 - Format with format(), parse with parse()
 - Validate with validate()
 - %Set to current time with: set_utc(), set_local(), set_local_notz()
 .

\par Example

\code
#include <evo/time.h>
#include <evo/io.h>
using namespace evo;

int main() {
    DateTime dt;
    dt.parse("1999-12-31-23:59:59");
    dt.add_seconds(1);
    dt.format(con().out, ' ') << NL;
    return 0;
}
\endcode

Output:
\code{.unparsed}
2000-01-01 00:00:00
\endcode

*/
struct DateTime {
    static const int   OFFSET_NULL = TimeZoneOffset::OFFSET_NULL;     ///< Null time zone offset
    static const ulong JDN_MIN     = Date::JDN_MIN;                   ///< Minimum Julian Day Number for Jan 1, 1000

    Date      date;     ///< Date fields
    TimeOfDay time;     ///< TimeOfDay fields
    TimeZoneOffset tz;  ///< TimeZoneOffset fields

    /** Constructor. */
    DateTime() {
    }

    /** Constructor.
     \param  year       4 digit year
     \param  month      Month of year
     \param  day        Day of month
     \param  hour       Hour of day
     \param  minute     Minute of hour
     \param  second     Second of minute
     \param  msecond    Millisecond of second
     \param  tz_offset  Time zone offset from UTC in minutes, 0 for UTC, negative for the Western Hemisphere (America), positive for the remaining time zones, OFFSET_NULL for null (no time zone)
    */
    DateTime(int year, int month, int day, int hour=0, int minute=0, int second=0, int msecond=0, int tz_offset=OFFSET_NULL) :
        date(year, month, day), time(hour, minute, second, msecond), tz(tz_offset) {
    }

    /** Copy constructor.
     \param  src  Source to copy
    */
    DateTime(const DateTime& src) : date(src.date), time(src.time), tz(src.tz) {
    }

    /** Date constructor.
     \param  date  %Date to copy from
    */
    DateTime(const Date& date) : date(date) {
    }

    /** Date and TimeOfDay constructor.
     \param  date  %Date to copy from
     \param  time  %Time to copy from
    */
    DateTime(const Date& date, const TimeOfDay& time) : date(date), time(time) {
    }

    /** Date and TimeOfDay constructor.
     \param  date  %Date to copy from
     \param  time  %Time to copy from
     \param  tz    Timezone offset to copy from
    */
    DateTime(const Date& date, const TimeOfDay& time, const TimeZoneOffset& tz) : date(date), time(time), tz(tz) {
    }

    /** Timezone offset constructor.
     \param  tz  Timezone offset to copy from
    */
    DateTime(const TimeZoneOffset& tz) : tz(tz) {
    }

    /** Assignment operator.
     \param  src  Source to copy
     \return      This
    */
    DateTime& operator=(const DateTime& src) {
        date = src.date;
        time = src.time;
        tz   = src.tz;
        return *this;
    }

    /** %Compare for equality with another date and time.
     \param  oth  Other date and time to compare to
     \return      Whether equal
    */
    bool operator==(const DateTime& oth) const
        { return (compare_op_helper(oth) == 0); }

    /** %Compare for inequality with another date and time.
     \param  oth  Other date and time to compare to
     \return      Whether inequal
    */
    bool operator!=(const DateTime& oth) const
        { return (compare_op_helper(oth) != 0); }

    /** %Compare whether less than another date and time.
     - If this and `oth` have different time zone offsets, the differnce is taken into account, as if both are converted to UTC for comparison
     - If either time zone offset is null, then time zones are ignored here
     .
     \param  oth  Other date and time to compare to
     \return      Whether less than
    */
    bool operator<(const DateTime& oth) const
        { return (compare_op_helper(oth) < 0); }

    /** %Compare whether less than or equal to another date and time.
     - If this and `oth` have different time zone offsets, the differnce is taken into account, as if both are converted to UTC for comparison
     - If either time zone offset is null, then time zones are ignored here
     .
     \param  oth  Other date and time to compare to
     \return      Whether less than or equal
    */
    bool operator<=(const DateTime& oth) const
        { return (compare_op_helper(oth) <= 0); }

    /** %Compare whether greater than another date and time.
     - If this and `oth` have different time zone offsets, the differnce is taken into account, as if both are converted to UTC for comparison
     - If either time zone offset is null, then time zones are ignored here
     .
     \param  oth  Other date and time to compare to
     \return      Whether greater than
    */
    bool operator>(const DateTime& oth) const
        { return (compare_op_helper(oth) > 0); }

    /** %Compare whether greater than or equal to another date and time.
     - If this and `oth` have different time zone offsets, the differnce is taken into account, as if both are converted to UTC for comparison
     - If either time zone offset is null, then time zones are ignored here
     .
     \param  oth  Other date and time to compare to
     \return      Whether greater than or equal
    */
    bool operator>=(const DateTime& oth) const
        { return (compare_op_helper(oth) >= 0); }

    /** %Compare to another date and time.
     - If this and `oth` have different time zone offsets, the differnce is taken into account, as if both are converted to UTC for comparison
     - If either time zone offset is null, then time zones are ignored here
     .
     \param  oth  Other date and time to compare to
     \return      Result (<0 if this is less, 0 if equal, >0 if this is greater)
    */
    int compare(const DateTime& oth) const
        { return compare_op_helper(oth); }

    /** Validate current date, time, and timezone offset.
     \return  Whether valid, false if any date/time/tz field is invalid (out of range)
    */
    bool validate() const
        { return (date.validate() && time.validate() && tz.validate()); }

    /** Get Julian Day Number with time fraction.
     - This is useful for turning a date and time into a single floating-point number in day units that is easy to add/subtract and convert back
     - Call set_jdn_dt() to convert a JDN back into date and time fields
     - For a Modified JDN: subtract JDN_MOD from a valid (non-zero) result, then add it back again
     - Notes:
       - Julian Days normally begin at noon, but this pretends they begin at midnight (12 hours earlier) since this is easier to manage
       - Not related to the Julian calendar
     - \b Caution: Milliseconds aren't stored in the fraction (not enough precision) and are ignored here
     - See: set_jdn_dt()
     .
     \return  Julian Day Number with whole number as date and fraction for time since midight -- never negative
    */
    double get_jdn_dt() const
        { return date.get_jdn() + time.get_fraction(); }

    /** %Set all fields to 0, which is _not_ a validate but _is_ a valid time (midnight) and timezone offset (UTC). */
    DateTime& set() {
        date.set();
        time.set();
        tz.set();
        return *this;
    }

    /** %Set new new fields.
     - \b Caution: This does _not_ modify the time zone offset (`tz`)
     .
     \param  year     4 digit year
     \param  month    Month of year
     \param  day      Day of month
     \param  hour     Hour of day
     \param  minute   Minute of hour
     \param  second   Second of minute
     \param  msecond  Millisecond of second
     \return          Whether successful, false if one of the fields is invalid
    */
    bool set(int year, int month, int day, int hour=0, int minute=0, int second=0, int msecond=0)
        { return (date.set(year, month, day) && time.set(hour, minute, second, msecond)); }

    /** %Set new fields from Julian Day Number.
     - \b Caution: This does _not_ modify the time zone offset (`tz`)
     - \b Caution: Milliseconds aren't stored in the fraction (not enough precision) so `time.msecond` is set from `msec`
     .
     \param  tm    Julian Day Number with whole number as date and fraction for time since midight -- must not be negative
     \param  msec  Milliseconds to set (0 - 999), defaults to `0`
     \return       Whether successful, false if `tm` is invalid (0 or less than JDN_MIN)
    */
    bool set_jdn_dt(double tm, int msec=0) {
        if (tm < JDN_MIN || !date.set_jdn((ulong)tm))
            return false;
        time.set_fraction(tm - (long)tm, msec);
        return true;
    }

    /** %Set to current UTC date and time.
     \return  This
    */
    DateTime& set_utc() {
        SysTimestamp::get_wall_datetime_fields_utc(date.year, date.month, date.day, time.hour, time.minute, time.second, time.msecond);
        tz.set_utc();
        return *this;
    }

    /** %Set to current local date and time with time zone offset.
     - Getting the time zone has some overhead so this can be slightly slower than set_local_notz()
     .
     \return  This
    */
    DateTime& set_local() {
        SysTimestamp::get_wall_datetime_fields_local(date.year, date.month, date.day, time.hour, time.minute, time.second, time.msecond, tz.minutes);
        return *this;
    }

    /** %Set to current local date and time without time zone offset.
     - Getting the time zone has some overhead so this can be slightly faster than set_local()
     .
     \return  This
    */
    DateTime& set_local_notz() {
        SysTimestamp::get_wall_datetime_fields_local(date.year, date.month, date.day, time.hour, time.minute, time.second, time.msecond);
        tz.set();
        return *this;
    }

    /** Add years to current date, subtract if negative.
     - This adds the given years and adjusts day of month to last day of month if not valid for the new year and month
     .
     \param  years  Number of years to add, negative to subtract
     \return        Whether successful, false if current date is invalid before or after modification
    */
    bool add_years(int years)
        { return date.add_years(years); }

    /** Add months to current date, subtract if negative.
     - This adds the given months and adjusts day of month to last day of month if not valid for the new year and month
     .
     \param  months  Number of months to add, negative to subtract
     \return         Whether successful, false if current date is invalid before or after modification
    */
    bool add_months(int months)
        { return date.add_months(months); }

    /** Add days to current date, subtract if negative.
     - This adds the given days and adjusts months and years accordingly
     .
     \param  days  Number of days to add, negative to subtract
     \return       Whether successful, false if current date is invalid before or after modification
    */
    bool add_days(int days)
        { return date.add_days(days); }

    /** Add hours to current date and time, subtract if negative.
     - This adds the given hours and adjusts the date accordingly
     .
     \param  hours  Number of hours to add, negative to subtract
     \return        Whether successful, false if current date is invalid before or after modification
    */
    bool add_hours(int hours)
        { time.hour += hours; return date.add_days(time.get_days(time.hour)); }

    /** Add minutes to current date and time, subtract if negative.
     - This adds the given minutes and adjusts the hour and date accordingly
     .
     \param  minutes  Number of minutes to add, negative to subtract
     \return          Whether successful, false if current date is invalid before or after modification
    */
    bool add_minutes(int minutes)
        { time.add_minutes(minutes); return date.add_days(time.get_days(time.hour)); }

    /** Add seconds to current date and time, subtract if negative.
     - This adds the given seconds and adjusts the minute, hour, and date accordingly
     .
     \param  seconds  Number of seconds to add, negative to subtract
     \return          Whether successful, false if current date is invalid before or after modification
    */
    bool add_seconds(int seconds)
        { time.add_seconds(seconds); return date.add_days(time.get_days(time.hour)); }

    /** Add milliseconds to current date and time, subtract if negative.
     - This adds the given milliseconds and adjusts the second, minute, hour, and date accordingly
     .
     \param  msec  Number of milliseconds to add, negative to subtract
     \return       Whether successful, false if current date is invalid before or after modification
    */
    bool add_milliseconds(int msec)
        { time.add_milliseconds(msec); return date.add_days(time.get_days(time.hour)); }

    /** %Set date/time from parsing standard ISO 8601 based string.
     - Supported date/time syntax is based on <a href="https://en.wikipedia.org/wiki/ISO_8601">ISO 8601</a>:
       - High level syntax: `DATE "T" TIME TZ` (without quotes or spaces)
       - `DATE`: `YYYY-MM-DD` -- see Date::parse() for more variations and detail
       - `TIME`: `HH:MM:SS` or `HH:MM:SS.fff` -- see TimeOfDay::parse() for more variations and detail
       - `TZ`: `Z` or `+HH:MM` or `-HH:MM` -- see TimeZoneOffset::parse() for more variations and detail
     .
     \param  inp  Start parsing pointer (must be `<= end`), set to stop position  [in/out]
     \param  end  End of string pointer, must not be NULL
     \return      Whether successful, false on bad input or unrecognized format
    */
    bool parse_std_impl(const char*& inp, const char* end) {
        if (date.parse_std_impl(inp, end)) {
            if (inp < end) {
                switch (*inp) {
                    case 'T':
                    case 't':
                    case '-':
                    case ':':
                    case '_':
                    case '/':
                    case ',':
                    case '.':
                    case '@':
                        if (!time.parse_std_impl(++inp, end))
                            return false;
                        date.add_days(time.get_days(time.hour));
                        break;
                    default:
                        time.set();
                        break;
                }
                return tz.parse_std_impl(inp, end);
            } else {
                time.set();
                tz.set();
            }
            return true;
        }
        return false;
    }

    /** %Set date/time from parsing standard ISO 8601 based string.
     - Supported date/time syntax is based on <a href="https://en.wikipedia.org/wiki/ISO_8601">ISO 8601</a>:
       - High level syntax: `DATE "T" TIME TZ` (without quotes or spaces)
       - `DATE`: `YYYY-MM-DD` -- see Date::parse() for more variations and detail
       - `TIME`: `HH:MM:SS` or `HH:MM:SS.fff` -- see TimeOfDay::parse() for more variations and detail
       - `TZ`: `Z` or `+HH:MM` or `-HH:MM` -- see TimeZoneOffset::parse() for more variations and detail
     .
     \param  str  %String to parse
     \return      Whether successful, false on bad input or unrecognized format
    */
    bool parse(const SubString& str) {
        const char* p = str.data();
        return parse_std_impl(p, p + str.size());
    }

    /** Format date and time to String or Stream using given delimiters.
     - See also: format_std(), format_nodelim()
     - Example outputs using default delimiters:
       - `1999-12-31T23:59:59Z`
       - `1999-12-31T23:59:59+07:30`
       - `1999-12-31T23:59:59-08:00`
     .
     \tparam  T  String/Stream type, inferred from argument
     \param  out         Output String/Stream to format to
     \param  dt_delim    Delimiter between date and time, ISO 8601 uses `T`, Evo also supports space and underscore
     \param  d_delim     Date field delimiter, usually `-`, 0 for none -- see Date::format()
     \param  t_delim     TimeOfDay field delimiter, usually `:`, 0 for none -- see TimeOfDay::format()
     \param  msec_delim  Milliseconds delimiter, can be `.` or `,`, 0 for none -- see TimeOfDay::format()
     \param  tz_delim    TimeZoneOffset field delimiter, usually `:`, 0 for none -- see TimeZoneOffset::format()
     \return             Reference to `out`
    */
    template<class T>
    T& format(T& out, char dt_delim='T', char d_delim='-', char t_delim=':', char msec_delim=0, char tz_delim=':') const {
        date.format(out, d_delim);
        if (dt_delim > 0)
            out << dt_delim;
        time.format(out, t_delim, msec_delim);
        tz.format(out, tz_delim);
        return out;
    }

    /** Format date and time to String or Stream using standard delimiters.
     - See also: format_nodelim(), format()
     - Example outputs using `_` and `.` delimiters:
       - `1999-12-31_23:59:59.999Z`
       - `1999-12-31_23:59:59.999+07:30`
       - `1999-12-31_23:59:59.999-08:00`
     .
     \tparam  T  String/Stream type, inferred from argument
     \param  out         Output String/Stream to format to
     \param  dt_delim    Delimiter between date and time, ISO 8601 uses `T`, Evo also supports space and underscore
     \param  msec_delim  Milliseconds delimiter, can be `.` or `,`, 0 for none -- see TimeOfDay::format()
     \return             Reference to `out`
    */
    template<class T>
    T& format_std(T& out, char dt_delim='T', char msec_delim=0) const
        { return format(out, dt_delim, '-', ':', msec_delim, ':'); }

    /** Format date and time to String or Stream using standard delimiters.
     - See also: format_nodelim(), format()
     - Example outputs using default delimiters:
       - `19991231T235959Z`
       - `19991231T235959+07:30`
       - `19991231T235959-08:00`
     .
     \tparam  T  String/Stream type, inferred from argument
     \param  out         Output String/Stream to format to
     \param  dt_delim    Delimiter between date and time, ISO 8601 uses `T`, Evo also supports space and underscore
     \param  msec_delim  Milliseconds delimiter, can be `.` or `,`, 0 for none -- see TimeOfDay::format()
     \return             Reference to `out`
    */
    template<class T>
    T& format_nodelim(T& out, char dt_delim='T', char msec_delim=0) const
        { return format(out, dt_delim, 0, 0, msec_delim, 0); }

private:
    int compare_op_helper(const DateTime& oth) const {
        int cmp;
        if (tz.minutes == oth.tz.minutes || tz.null() || oth.tz.null()) {
            cmp = date.compare(oth.date);
            if (cmp == 0)
                cmp = time.compare(oth.time);
        } else {
            DateTime tmp(*this);
            tmp.add_minutes(oth.tz.minutes - tz.minutes);
            cmp = tmp.date.compare(oth.date);
            if (cmp == 0)
                cmp = tmp.time.compare(oth.time);
        }
        return cmp;
    }
};

///////////////////////////////////////////////////////////////////////////////
//@}
}
#endif
