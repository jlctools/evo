// Evo C++ Library
/* Copyright (c) 2016 Justin Crowell
 This Source Code Form is subject to the terms of the Mozilla Public
 License, v. 2.0. If a copy of the MPL was not distributed with this
 file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
///////////////////////////////////////////////////////////////////////////////
/** \file timer.h Evo Timer class. */
#pragma once
#ifndef INCL_evo_timer_h
#define INCL_evo_timer_h

// Includes - System
#if defined(__linux) || defined(__CYGWIN__)
    #include <sys/time.h>
    #include <time.h>
#elif defined(_WIN32)
    #define NOMINMAX // disable windows min/max macros
    #include <Windows.h>
#endif

// Includes
#include "impl/sys.h"

// Namespace: evo
namespace evo {

/** \addtogroup EvoSys */
//@{

///////////////////////////////////////////////////////////////////////////////

/** Timer that works like a stopwatch.
 - This measures actual time elapsed (as opposed to process CPU time)
 - Accuracy depends on the system
*/
class Timer
{
private:
    // General types and temp macros for current system
    #if defined(_WIN32)
        typedef FILETIME Stamp;
        #define EVO_TIMER_GET(STAMP) GetSystemTimeAsFileTime(&STAMP)
        #define EVO_TIMER_DIFF_USEC(START,END) \
            (( (((ulongl)END.dwHighDateTime << 32) | END.dwLowDateTime) - (((ulongl)START.dwHighDateTime << 32) | START.dwLowDateTime) ) / 10ULL)
    #else
        typedef timeval Stamp;
        #define EVO_TIMER_GET(STAMP) gettimeofday(&STAMP, NULL)
        #define EVO_TIMER_DIFF_USEC(START,END) \
            ( ((ulongl)(END.tv_sec - START.tv_sec) * 1000000ULL) + END.tv_usec - START.tv_usec )
    #endif

public:
    /** Constructor. */
    Timer()
        { memset(this, 0, sizeof(Timer)); }

    /** Copy constructor.
     \param  src  Source to copy
    */
    Timer(const Timer& src)
        { memcpy(this, &src, sizeof(Timer)); }

    /** Assignment/Copy operator.
     \param  src  Source to copy
     \return      This
    */
    Timer& operator=(const Timer& src)
        { memcpy(this, &src, sizeof(Timer)); return *this; }

    /** Stop and clear timer. */
    inline void clear()
        { memset(this, 0, sizeof(Timer)); }

    /** Get whether timer is active (started).
     \return  Whether active, false if not started
    */
    inline bool active() const
        { return active_; }

    /** Get current time elapsed in seconds with fraction.
     - If timer is active then this gives time elapsed so far, otherwise time elapsed from last start/stop
     .
     \return  Time elapsed in seconds
    */
    inline double sec() const {
        const ulongl cur_usec = elapsed();
        const ulongl cur_sec  = (cur_usec / 1000000ULL);
        return ( (double)(cur_usec - (cur_sec * 1000000ULL)) / 1000000.0 ) + cur_sec;
    }

    /** Get current time elapsed in milliseconds.
     - If timer is active then this gives time elapsed so far, otherwise time elapsed from last start/stop
     .
     \return  Time elapsed in milliseconds
    */
    inline ulongl msec() const
        { return (elapsed() / 1000ULL); }

    /** Get current time elapsed in microseconds.
     - If timer is active then this gives time elapsed so far, otherwise time elapsed from last start/stop
     .
     \return  Time elapsed in microseconds
    */
    inline ulongl usec() const
        { return elapsed(); }

    /** Start timer.
     - This clears stored elapsed time
     - Call stop() to stop timer
    */
    inline void start() {
        active_       = true;
        elapsed_usec_ = 0;
        EVO_TIMER_GET(start_);
    }

    /** Resume timer from last stop.
     - This starts timer again like a stopwatch
     - If timer never started then this is same as start()
     - Call stop() to stop timer
    */
    inline void resume() {
        active_ = true;
        EVO_TIMER_GET(start_);
    }

    /** Stop timer.
     - Time elapsed is stored internally
     - Timer may started again with start() or resume()
    */
    inline Timer& stop() {
        elapsed_usec_ = elapsed();
        active_ = false;
        return *this;
    }

    /** Sleep for number of milliseconds.
     \param  msec  Milliseconds to sleep, must be > 0
    */
    static inline void sleepms(ulong msec) {
        #if defined(_WIN32)
            Sleep(msec);
        #else
            timespec tm;
            tm.tv_sec  = (msec / 1000UL);
            tm.tv_nsec = (long)(msec - (1000UL * tm.tv_sec)) * 1000000;
            while (nanosleep(&tm, &tm) == -1 && errno == EINTR)
                { }
        #endif
    }

    /** Sleep for number of microseconds.
     - On UNIX/Linux systems this will ignore signals
     - On Windows this is equivalent to calling sleepms() with usec converted to at least 1 msec
     .
     \param  usec  Microseconds to sleep, must be > 0
    */
    static inline void sleepus(ulongl usec) {
        #if defined(_WIN32)
            Sleep( (ulong)(usec<1000ULL ? 1 : usec/1000ULL) );
        #else
            timespec tm;
            tm.tv_sec  = (usec / 1000000ULL);
            tm.tv_nsec = (long)(usec - (1000000ULL * tm.tv_sec)) * 1000;
            while (nanosleep(&tm, &tm) == -1 && errno == EINTR)
                { }
        #endif
    }

private:
    bool   active_;            ///< Whether timer is active (started)
    Stamp  start_;            ///< Start timestamp
    ulongl elapsed_usec_;    ///< Stored time elapsed in microseconds

    /** Get current elapsed time in microseconds.
     \return  Elapsed time
    */
    inline ulongl elapsed() const {
        ulongl usec = elapsed_usec_;
        if (active_) {
            Stamp end;
            EVO_TIMER_GET(end);
            usec += EVO_TIMER_DIFF_USEC(start_, end);
        }
        return usec;
    }

    // Delete temp macros
    #undef EVO_TIMER_GET
    #undef EVO_TIMER_DIFF_USEC
};

///////////////////////////////////////////////////////////////////////////////
} // Namespace: evo
//@}
#endif
