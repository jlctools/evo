// Evo C++ Library
/* Copyright 2019 Justin Crowell
Distributed under the BSD 2-Clause License -- see included file LICENSE.txt for details.
*/
///////////////////////////////////////////////////////////////////////////////
/** \file systime.h Evo system time implementation helpers. */
#pragma once
#ifndef INCL_evo_systime_h
#define INCL_evo_systime_h

#include "sys.h"

#if !defined(_WIN32) && !(defined(_POSIX_TIMERS) && defined(CLOCK_PROCESS_CPUTIME_ID))
    #include <sys/resource.h>
#endif

namespace evo {
/** \addtogroup EvoTime */
//@{

///////////////////////////////////////////////////////////////////////////////

/** Holds a system timestamp as native (platform specific) fields.
 - This is used to get and store the current current date/time efficiently, then later convert to a DateTime (which has some overhead)
 .
*/
struct SysNativeTimeStamp {
#if defined(_WIN32)
    // Windows
    typedef SYSTEMTIME Fields;
    SYSTEMTIME ts;

    void set_utc() {
        ::GetSystemTime(&ts);
    }

    int64 get_unix_timestamp() const {
        const ulongl NSEC100_PER_SEC = 10000000;
        const ulongl UNIX_OFFSET = 11644473600ULL; // seconds between 1601 (Windows epoch) and 1970 (Unix epoch)
        FILETIME ft;
        if (::SystemTimeToFileTime(&ts, &ft) == 0)
            return 0;
        const ulongl ft_nsec100 = ((ulongl)ft.dwHighDateTime << 32) | (ulongl)ft.dwLowDateTime;
        const int64 sec = (int64)(ft_nsec100 / NSEC100_PER_SEC);
        if (sec < UNIX_OFFSET)
            return 0;
        return sec - UNIX_OFFSET;
    }

    int get_msec() const {
        return ts.wMilliseconds;
    }

    long get_nsec() const {
        const long NSEC_PER_MSEC = 1000000L;
        return (long)ts.wMilliseconds / NSEC_PER_MSEC;
    }

    template<class DT>
    void convert_utc_dt(DT& dt) const {
        dt.date.year  = ts.wYear;
        dt.date.month = ts.wMonth;
        dt.date.day   = ts.wDay;
        dt.time.hour    = ts.wHour;
        dt.time.minute  = ts.wMinute;
        dt.time.second  = ts.wSecond;
        dt.time.msecond = ts.wMilliseconds;
        dt.tz.minutes = 0;
    }

    template<class DT>
    void convert_local_dt(DT& dt) const {
        TIME_ZONE_INFORMATION info;
        TIME_ZONE_INFORMATION* p_info;
        if (::GetTimeZoneInformation(&info) == TIME_ZONE_ID_INVALID) {
            p_info = NULL;
            dt.tz.minutes = 0;
        } else {
            p_info = &info;
            dt.tz.minutes = (int)-info.Bias;
        }

        SYSTEMTIME local;
        BOOL result = ::SystemTimeToTzSpecificLocalTime(p_info, &ts, &local);
        assert( result != 0 );
        if (result != 0) {
            dt.date.year  = local.wYear;
            dt.date.month = local.wMonth;
            dt.date.day   = local.wDay;
            dt.time.hour    = local.wHour;
            dt.time.minute  = local.wMinute;
            dt.time.second  = local.wSecond;
            dt.time.msecond = local.wMilliseconds;
        } else
            convert_utc_dt(dt);
    }

    template<class DT>
    void convert_local_dt_notz(DT& dt) const {
        SYSTEMTIME local;
        BOOL result = ::SystemTimeToTzSpecificLocalTime(NULL, &ts, &local);
        assert( result != 0 );
        if (result != 0) {
            dt.date.year  = local.wYear;
            dt.date.month = local.wMonth;
            dt.date.day   = local.wDay;
            dt.time.hour    = local.wHour;
            dt.time.minute  = local.wMinute;
            dt.time.second  = local.wSecond;
            dt.time.msecond = local.wMilliseconds;
            dt.tz.set();
        } else
            convert_utc_dt(dt);
    }

#else
    // Linux/Unix
#if defined(_POSIX_TIMERS) && defined(CLOCK_REALTIME) && !defined(EVO_USE_GETTIMEOFDAY)
    typedef struct timespec Fields;
    struct timespec ts;

    void set_utc() {
    #if defined(CLOCK_REALTIME_COARSE)
        ::clock_gettime(CLOCK_REALTIME_COARSE, &ts);
    #else
        ::clock_gettime(CLOCK_REALTIME, &ts);
    #endif
        assert( ts.tv_sec > 0 );
    }

    int64 get_unix_timestamp() const {
        return (int64)ts.tv_sec;
    }

    int get_msec() const {
        const long NSEC_PER_MSEC = 1000000L;
        return (int)(ts.tv_nsec / NSEC_PER_MSEC);
    }

    long get_nsec() const {
        return ts.tv_nsec;
    }

#else
    typedef struct timeval Fields;
    struct timeval ts;

    /** Set to current date/time (UTC). */
    void set_utc() {
        ::gettimeofday(&ts, NULL);
        assert( ts.tv_sec > 0 );
    }

    /** Get current date/time as Unix timestamp.
     - Unix timestamps are seconds since Jan 1 1970 (not counting leap seconds)
     .
     \return  Unix timestamp, never negative, 0 for dates before 1970
    */
    int64 get_unix_timestamp() const {
        return (int64)ts.tv_sec;
    }

    /** Get current fractional second value in milliseconds.
     \return  Milliseconds
    */
    int get_msec() const {
        const long USEC_PER_MSEC = 1000L;
        return (int)((long)ts.tv_usec / USEC_PER_MSEC);
    }

    /** Get current fractional second value in nanoseconds.
     \return  Nanoseconds
    */
    long get_nsec() const {
        const long NSEC_PER_USEC = 1000L;
        return (long)ts.tv_usec * NSEC_PER_USEC;
    }

#endif
    static const int SEC_PER_MIN = 60;  ///< Number of seconds per minute

    /** Convert current date/time to DateTime holding UTC.
     \tparam  DateTime type, inferred from argument
     \param  dt  Stores converted date/time [out]
    */
    template<class DT>
    void convert_utc_dt(DT& dt) const {
        struct tm tm;
        ::gmtime_r(&ts.tv_sec, &tm);
        dt.date.year    = 1900 + tm.tm_year;
        dt.date.month   = tm.tm_mon + 1;
        dt.date.day     = tm.tm_mday;
        dt.time.hour    = tm.tm_hour;
        dt.time.minute  = tm.tm_min;
        dt.time.second  = tm.tm_sec;
        dt.time.msecond = get_msec();
    }

    /** Convert current date/time to DateTime holding Local Time, including timezone offset.
     \tparam  DateTime type, inferred from argument
     \param  dt  Stores converted date/time [out]
    */
    template<class DT>
    void convert_local_dt(DT& dt) const {
        struct tm tm;
        ::tzset();
        ::localtime_r(&ts.tv_sec, &tm);
    #if defined(__FreeBSD__)
        dt.tz.minutes = (tm.tm_gmtoff / SEC_PER_MIN);
    #else
        dt.tz.minutes = -(::timezone / SEC_PER_MIN);
    #endif
        dt.date.year    = 1900 + tm.tm_year;
        dt.date.month   = tm.tm_mon + 1;
        dt.date.day     = tm.tm_mday;
        dt.time.hour    = tm.tm_hour;
        dt.time.minute  = tm.tm_min;
        dt.time.second  = tm.tm_sec;
        dt.time.msecond = get_msec();
    }

    /** Convert current date/time to DateTime holding Local Time, without storing timezone offset.
     \tparam  DateTime type, inferred from argument
     \param  dt  Stores converted date/time [out]
    */
    template<class DT>
    void convert_local_dt_notz(DT& dt) const {
        struct tm tm;
        ::localtime_r(&ts.tv_sec, &tm);
        dt.date.year    = 1900 + tm.tm_year;
        dt.date.month   = tm.tm_mon + 1;
        dt.date.day     = tm.tm_mday;
        dt.time.hour    = tm.tm_hour;
        dt.time.minute  = tm.tm_min;
        dt.time.second  = tm.tm_sec;
        dt.time.msecond = get_msec();
    }
#endif

    /** Constructor.
     - This does not initialize current fields -- use set() or set_utc()
    */
    SysNativeTimeStamp() {
    }

    /** Copy constructor.
     \param  src  Source to copy
    */
    SysNativeTimeStamp(const SysNativeTimeStamp& src)
        { ::memcpy(this, &src, sizeof(SysNativeTimeStamp)); }

    /** Assignment operator.
     \param  src  Source to copy
     \return      This
    */
    SysNativeTimeStamp& operator=(const SysNativeTimeStamp& src)
        { ::memcpy(this, &src, sizeof(SysNativeTimeStamp)); return *this; }

    /** Set as null/invalid timestamp. */
    void set()
         { ::memset(&ts, 0, sizeof(Fields)); }
};

///////////////////////////////////////////////////////////////////////////////

/** Holds a system timestamp for storing date/time and measuring elapsed time.
 - See \link Timer\endlink, DateTime
*/
struct SysTimestamp {
    static const ulong NSEC_PER_SEC  = 1000000000UL;    ///< Nanoseconds per second
    static const ulong USEC_PER_SEC  = 1000000UL;       ///< Microseconds per second
    static const ulong MSEC_PER_SEC  = 1000;            ///< Milliseconds per second
    static const ulong NSEC_PER_MSEC = 1000000UL;       ///< Nanoseconds per millisecond
    static const ulong NSEC_PER_USEC = 1000;            ///< Nanoseconds per microsecond
    static const int SEC_PER_MIN     = 60;              ///< Seconds per minute

#if defined(_WIN32)
    static const int EPOCH_YEAR = 1601;
#else
    static const int EPOCH_YEAR = 1970;     ///< Epoch year used by system -- 1970 for Linux/Unix/POSIX, 1601 for Windows
#endif

    ulongl sec;     ///< Number of seconds since Jan 1, EPOCH_YEAR
    ulong nsec;     ///< Number of nanoseconds from seconds

    /** Constructor. */
    SysTimestamp() : sec(0), nsec(0) {
    }

    /** Copy constructor.
     \param  src  Source to copy
    */
    SysTimestamp(const SysTimestamp& src) : sec(src.sec), nsec(src.nsec) {
    }

    /** Assignment operator.
     \param  src  Source to copy
     \return      This
    */
    SysTimestamp& operator=(const SysTimestamp& src) {
        sec = src.sec;
        nsec = src.nsec;
        return *this;
    }

    /** Clear and reset as 0. */
    void clear() {
        sec = 0;
        nsec = 0;
    }

#if defined(_WIN32)
    void set_win32_ft(const FILETIME& ft) {
        const ulongl NSEC100_PER_SEC = 10000000;
        const ulong NSEC_PER_NSEC100 = 100;

        const ulongl ft_nsec100 = ((ulongl)ft.dwHighDateTime << 32) | (ulongl)ft.dwLowDateTime;
        sec = ft_nsec100 / NSEC100_PER_SEC;
        nsec = (ulong)(ft_nsec100 - (sec * NSEC100_PER_SEC)) * NSEC_PER_NSEC100;
    }

    void add_win32_ft(const FILETIME& ft) {
        const ulongl NSEC100_PER_SEC = 10000000;
        const ulong NSEC_PER_NSEC100 = 100;
        {
            const ulongl ft_nsec100 = ((ulongl)ft.dwHighDateTime << 32) | (ulongl)ft.dwLowDateTime;
            sec += ft_nsec100 / NSEC100_PER_SEC;
            nsec += (ulong)(ft_nsec100 - (sec * NSEC100_PER_SEC)) * NSEC_PER_NSEC100;
        }
        const ulong add_sec = (nsec / NSEC_PER_SEC);
        sec += add_sec;
        nsec -= (add_sec * NSEC_PER_SEC);
    }
#endif

    /** %Set from system native timestamp.
     - This converts from SysNativeTimeStamp, which has platform-specific fields
     - Timezone is always UTC here
     .
     \param  src  Source to convert from
    */
    void set(const SysNativeTimeStamp& src) {
    #if defined(_WIN32)
        FILETIME ft;
        ::SystemTimeToFileTime(&src.ts, &ft);
        set_win32_ft(ft);
    #else
        sec = (ulongl)src.ts.tv_sec;
        nsec = (ulong)src.get_nsec();
    #endif
    }

    /** %Set as current real (wall clock) time for calendar date/time use.
     - This is intended for getting current calendar date/time, but is not accurate as a timer as it's subject to time adjustments (daylight savings, NTP, user modification)
     - This uses a faster but more coarse (millisecond) system clock, if possible
     - Timezone is always UTC here
    */
    void set_wall_datetime() {
    #if defined(_WIN32)
        FILETIME ft;
        SYSTEMTIME stm;
        ::GetSystemTime(&stm);
        ::SystemTimeToFileTime(&stm, &ft);
        set_win32_ft(ft);
    #elif defined(_POSIX_TIMERS) && defined(CLOCK_REALTIME) && !defined(EVO_USE_GETTIMEOFDAY)
        struct timespec ts;
        #if defined(CLOCK_REALTIME_COARSE)
            ::clock_gettime(CLOCK_REALTIME_COARSE, &ts);
        #else
            ::clock_gettime(CLOCK_REALTIME, &ts);
        #endif
        assert( ts.tv_sec > 0 );
        assert( ts.tv_nsec >= 0 );

        sec = (ulongl)ts.tv_sec;
        nsec = (ulong)ts.tv_nsec;
    #else
        struct timeval tv;
        ::gettimeofday(&tv, NULL);
        assert( tv.tv_sec > 0 );
        assert( tv.tv_usec >= 0 );

        sec = (ulongl)tv.tv_sec;
        nsec = (ulong)tv.tv_usec * NSEC_PER_USEC;
    #endif
    }

    /** Set as current real (wall clock) time for use by timers.
     - This is intended for calculating time elapsed on the system and is accurate for that purpose, but is not accurate for getting current date/time
     - This uses a monotonic high-resolution (nanosecond) system clock, if possible
    */
    void set_wall_timer() {
    #if defined(_WIN32)
        #if defined(EVO_WIN32_NO_QPC)
            // More compatible with old hardware, subject to time adjustments (daylight savings, NTP)
            FILETIME ft;
            ::GetSystemTimeAsFileTime(&ft);
            set_win32_ft(ft);
        #else
            // Monotonic, best for relatively modern systems
            static const ulongl FREQ = (ulongl)qpc_get_freq();
            LARGE_INTEGER qpc;
            ::QueryPerformanceCounter(&qpc);
            assert( qpc.QuadPart > 0 );

            sec = qpc.QuadPart / FREQ;
            nsec = (ulong)( ((ulongl)qpc.QuadPart - (sec * FREQ)) * NSEC_PER_SEC / FREQ );
        #endif
    #elif defined(_POSIX_TIMERS) && defined(CLOCK_REALTIME) && !defined(EVO_USE_GETTIMEOFDAY)
        struct timespec ts;
        #if defined(CLOCK_MONOTONIC_RAW)
            ::clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
        #elif defined(CLOCK_MONOTONIC)
            ::clock_gettime(CLOCK_MONOTONIC, &ts);
        #else
            ::clock_gettime(CLOCK_REALTIME, &ts);
        #endif
        assert( ts.tv_sec > 0 );
        assert( ts.tv_nsec >= 0 );

        sec = (ulongl)ts.tv_sec;
        nsec = (ulong)ts.tv_nsec;
    #else
        struct timeval tv;
        ::gettimeofday(&tv, NULL);
        assert( tv.tv_sec > 0 );
        assert( tv.tv_usec >= 0 );

        sec = (ulongl)tv.tv_sec;
        nsec = (ulong)tv.tv_usec * NSEC_PER_USEC;
    #endif
    }

    /** Set as current CPU (process) time for use by timers. */
    void set_cpu() {
    #if defined(_WIN32)
        FILETIME create, exit, kernel, user;
        ::GetProcessTimes(::GetCurrentProcess(), &create, &exit, &kernel, &user);
        set_win32_ft(kernel);
        add_win32_ft(user);
    #elif defined(_POSIX_TIMERS) && defined(CLOCK_PROCESS_CPUTIME_ID)
        struct timespec ts;
        ::clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &ts);
        assert( ts.tv_nsec >= 0 );

        sec = (ulongl)ts.tv_sec;
        nsec = (ulong)ts.tv_nsec;
    #else
        struct rusage ru;
        ::getrusage(RUSAGE_SELF, &ru);

        sec = (ulongl)ru.ru_stime.tv_sec + (ulongl)ru.ru_utime.tv_sec;
        nsec = ((ulong)ru.ru_stime.tv_usec + (ulong)ru.ru_stime.tv_usec) * NSEC_PER_USEC;

        if (nsec >= NSEC_PER_SEC) {
            ++sec;
            nsec -= NSEC_PER_SEC;
        }
    #endif
    }

    /** Add milliseconds to current time.
     - This normalizes the current time so the `nsec` field has less than 1 second
     .
     \param  new_msec  Milliseconds to add
    */
    void add_msec(ulong new_msec) {
        // normalize new_nsec first to avoid overflow
        ulong new_sec = new_msec / MSEC_PER_SEC;
        sec += new_sec;
        nsec += (ulong)(new_msec - (new_sec * MSEC_PER_SEC)) * NSEC_PER_MSEC;

        new_sec = nsec / NSEC_PER_SEC;
        sec += new_sec;
        nsec -= (ulong)(new_sec * NSEC_PER_SEC);
    }

    /** Add nanoseconds to current time.
     - This normalizes the current time so the `nsec` field has less than 1 second
     .
     \param  new_nsec  Nanoseconds to add
    */
    void add_nsec(ulongl new_nsec) {
        // normalize new_nsec first to avoid overflow
        ulongl new_sec = new_nsec / NSEC_PER_SEC;
        sec += new_sec;
        nsec += (ulong)(new_nsec - (new_sec * NSEC_PER_SEC));

        new_sec = nsec / NSEC_PER_SEC;
        sec += new_sec;
        nsec -= (ulong)(new_sec * NSEC_PER_SEC);
    }

    /** %Compare to another timestamp.
     \param  oth  Other timestamp to compare to
     \return      Result (<0 if this is less, 0 if equal, >0 if this is greater)
    */
    int compare(const SysTimestamp& oth) const {
        if (sec < oth.sec)
            return -1;
        else if (sec > oth.sec)
            return 1;
        else if (nsec < oth.nsec)
            return -1;
        else if (nsec > oth.nsec)
            return 1;
        return 0;
    }

    /** Use this as an end-time and get the difference from start time in milliseconds.
     \param  start  Start time to use, must not be greater than this
     \return        Difference in milliseconds
    */
    ulongl diff_msec(const SysTimestamp& start) const {
        assert( start.sec <= sec );
        assert( start.sec < sec || start.nsec <= nsec );
        return ((sec - start.sec) * MSEC_PER_SEC) + (nsec / NSEC_PER_MSEC) - (start.nsec / NSEC_PER_MSEC);
    }

    /** Use this as an end-time and get the difference from start time in microseconds.
     \param  start  Start time to use, must not be greater than this
     \return        Difference in microseconds
    */
    ulongl diff_usec(const SysTimestamp& start) const {
        assert( start.sec <= sec );
        assert( start.sec < sec || start.nsec <= nsec );
        return ((sec - start.sec) * USEC_PER_SEC) + (nsec / NSEC_PER_USEC) - (start.nsec / NSEC_PER_USEC);
    }

    /** Use this as an end-time and get the difference from start time in nanoseconds.
     \param  start  Start time to use, must not be greater than this
     \return        Difference in nanoseconds
    */
    ulongl diff_nsec(const SysTimestamp& start) const {
        assert( start.sec <= sec );
        assert( start.sec < sec || start.nsec <= nsec );
        return ((sec - start.sec) * NSEC_PER_SEC) + nsec - start.nsec;
    }

    /** Get fields for current real (wall clock) time for calendar date/time use (UTC).
     - This is equivalent of using get_wall_datetime() then converting the timestamp to date/time fields
     - Timezone is always UTC here
     .
     \param  year     Set to 4 digit year for date  [out]
     \param  month    Set to month of year for date (1 - 12)  [out]
     \param  day      Set to day of moneth for date (1 - 31)  [out]
     \param  hour     Set to hour for time of day (0 - 23)  [out]
     \param  minute   Minutes for time of day (0 - 59)  [out]
     \param  second   Seconds for time of day (0 - 60)  [out]
     \param  msecond  Milliseconds for time of day (0 - 999)  [out]
    */
    static void get_wall_datetime_fields_utc(int& year, int& month, int& day, int& hour, int& minute, int& second, int& msecond) {
    #if defined(_WIN32)
        SYSTEMTIME stm;
        ::GetSystemTime(&stm);
        year  = stm.wYear;
        month = stm.wMonth;
        day   = stm.wDay;
        hour    = stm.wHour;
        minute  = stm.wMinute;
        second  = stm.wSecond;
        msecond = stm.wMilliseconds;
    #else
        struct tm tm;
        {
            SysTimestamp ts;
            ts.set_wall_datetime();
            const time_t sec = (time_t)ts.sec;
            ::gmtime_r(&sec, &tm);
            msecond = (int)(ts.nsec / NSEC_PER_MSEC);
        }
        year    = 1900 + tm.tm_year;
        month   = tm.tm_mon + 1;
        day     = tm.tm_mday;
        hour    = tm.tm_hour;
        minute  = tm.tm_min;
        second  = tm.tm_sec;
    #endif
    }

    /** Get fields for current real (wall clock) time for calendar date/time use (Local Time).
     - This is equivalent of using get_wall_datetime() then converting the timestamp to date/time fields in local time
     - Time is in the current local timezone, but this does _not_ get the local timezone offset
     .
     \param  year     Set to 4 digit year for date  [out]
     \param  month    Set to month of year for date (1 - 12)  [out]
     \param  day      Set to day of moneth for date (1 - 31)  [out]
     \param  hour     Set to hour for time of day (0 - 23)  [out]
     \param  minute   Minutes for time of day (0 - 59)  [out]
     \param  second   Seconds for time of day (0 - 60)  [out]
     \param  msecond  Milliseconds for time of day (0 - 999)  [out]
    */
    static void get_wall_datetime_fields_local(int& year, int& month, int& day, int& hour, int& minute, int& second, int& msecond) {
    #if defined(_WIN32)
        SYSTEMTIME stm;
        ::GetLocalTime(&stm);
        year  = stm.wYear;
        month = stm.wMonth;
        day   = stm.wDay;
        hour    = stm.wHour;
        minute  = stm.wMinute;
        second  = stm.wSecond;
        msecond = stm.wMilliseconds;
    #else
        struct tm tm;
        {
            SysTimestamp ts;
            ts.set_wall_datetime();
            const time_t sec = (time_t)ts.sec;
            ::localtime_r(&sec, &tm);
            msecond = (int)(ts.nsec / NSEC_PER_MSEC);
        }
        year    = 1900 + tm.tm_year;
        month   = tm.tm_mon + 1;
        day     = tm.tm_mday;
        hour    = tm.tm_hour;
        minute  = tm.tm_min;
        second  = tm.tm_sec;
    #endif
    }

    /** Get fields for current real (wall clock) time for calendar date/time use (Local Time).
     - This is equivalent of using get_wall_datetime() then converting the timestamp to date/time fields in local time
     - Time is in the current local timezone
     .
     \param  year       Set to 4 digit year for date  [out]
     \param  month      Set to month of year for date (1 - 12)  [out]
     \param  day        Set to day of moneth for date (1 - 31)  [out]
     \param  hour       Set to hour for time of day (0 - 23)  [out]
     \param  minute     Minutes for time of day (0 - 59)  [out]
     \param  second     Seconds for time of day (0 - 60)  [out]
     \param  msecond    Milliseconds for time of day (0 - 999)  [out]
     \param  tz_offset  Time zone offset in minutes, see tz_get_offset()  [out]
    */
    static void get_wall_datetime_fields_local(int& year, int& month, int& day, int& hour, int& minute, int& second, int& msecond, int& tz_offset) {
    #if defined(_WIN32)
        SYSTEMTIME stm;
        ::GetLocalTime(&stm);
        year  = stm.wYear;
        month = stm.wMonth;
        day   = stm.wDay;
        hour    = stm.wHour;
        minute  = stm.wMinute;
        second  = stm.wSecond;
        msecond = stm.wMilliseconds;
        tz_offset = tz_get_offset();
    #else
        struct tm tm;
        {
            SysTimestamp ts;
            ts.set_wall_datetime();
            const time_t sec = (time_t)ts.sec;
            ::tzset();
            ::localtime_r(&sec, &tm);
            msecond = (int)(ts.nsec / NSEC_PER_MSEC);
        #if defined(__FreeBSD__)
            tz_offset = (tm.tm_gmtoff / SEC_PER_MIN);
        #else
            tz_offset = -(::timezone / SEC_PER_MIN);
        #endif
        }
        year    = 1900 + tm.tm_year;
        month   = tm.tm_mon + 1;
        day     = tm.tm_mday;
        hour    = tm.tm_hour;
        minute  = tm.tm_min;
        second  = tm.tm_sec;
    #endif
    }

    /** Initializater that optimizes timezone (local time) conversion in some cases.
     - On some systems (Linux) this sets an env var (`TZ`) if not already set, which reduces system calls for local time conversion
     - \b Caution: Not thread safe -- this should be called once at startup, before other threads are started
     .
    */
    static void tz_init() {
    #if defined(__linux__)
        // Setting TZ env var reduces stat() system calls with localtime_r()
        const char* FILE_STR = ":/etc/localtime"; // file path with ':' prefix
        if (::getenv("TZ") == NULL && ::access(FILE_STR + 1, R_OK) == 0)
            ::setenv("TZ", FILE_STR, 1);
    #endif
    }

    /** Get current time zone (local time) offset from UTC in minutes.
     - See: https://en.wikipedia.org/wiki/List_of_UTC_time_offsets
     - \b Caution: Time zone offset can change during each year with daylight savings, and the rules for this vary by region
     .
     \return  Time zone offset from UTC in minutes, 0 for UTC, negative for the Western Hemisphere (America), positive for the remaining time zones
    */
    static int tz_get_offset() {
    #if defined(_WIN32)
        TIME_ZONE_INFORMATION info;
        if (::GetTimeZoneInformation(&info) == TIME_ZONE_ID_INVALID)
            return 0;
        return (int)-info.Bias;
    #else
        ::tzset();
        #if defined(__FreeBSD__)
            time_t dummy = 0;
            struct tm tm;
            if (localtime_r(&dummy, &tm) == NULL)
                return 0;
            return (int)(tm.tm_gmtoff / SEC_PER_MIN);
        #else
            return (int)-(::timezone / SEC_PER_MIN);
        #endif
    #endif
    }

private:
#if defined(_WIN32) && !defined(EVO_WIN32_NO_QPC)
    static longl qpc_get_freq() {
        LARGE_INTEGER freq;
        ::QueryPerformanceFrequency(&freq);
        assert( freq.QuadPart > 0 );
        return freq.QuadPart;
    }
#endif
};

///////////////////////////////////////////////////////////////////////////////
//@}
}
#endif
