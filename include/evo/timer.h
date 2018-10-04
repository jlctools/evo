// Evo C++ Library
/* Copyright 2018 Justin Crowell
Distributed under the BSD 2-Clause License -- see included file LICENSE.txt for details.
*/
///////////////////////////////////////////////////////////////////////////////
/** \file timer.h Evo Timer class. */
#pragma once
#ifndef INCL_evo_timer_h
#define INCL_evo_timer_h

#include "impl/sys.h"

namespace evo {
/** \addtogroup EvoTime
Evo time management
*/
//@{

///////////////////////////////////////////////////////////////////////////////

/** %Timer that works like a stopwatch.
 - This measures actual time elapsed (as opposed to process CPU time)
 - Accuracy depends on the system
*/
class Timer {
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
    void clear()
        { memset(this, 0, sizeof(Timer)); }

    /** Get whether timer is active (started).
     \return  Whether active, false if not started
    */
    bool active() const
        { return active_; }

    /** Get current time elapsed in seconds with fraction.
     - If timer is active then this gives time elapsed so far, otherwise time elapsed from last start/stop
     .
     \return  Time elapsed in seconds
    */
    double sec() const {
        const ulongl cur_usec = elapsed();
        const ulongl cur_sec  = (cur_usec / 1000000ULL);
        return ( (double)(cur_usec - (cur_sec * 1000000ULL)) / 1000000.0 ) + cur_sec;
    }

    /** Get current time elapsed in milliseconds.
     - If timer is active then this gives time elapsed so far, otherwise time elapsed from last start/stop
     .
     \return  Time elapsed in milliseconds
    */
    ulongl msec() const
        { return (elapsed() / 1000ULL); }

    /** Get current time elapsed in microseconds.
     - If timer is active then this gives time elapsed so far, otherwise time elapsed from last start/stop
     .
     \return  Time elapsed in microseconds
    */
    ulongl usec() const
        { return elapsed(); }

    /** Start timer.
     - This clears stored elapsed time
     - Call stop() to stop timer
    */
    void start() {
        active_       = true;
        elapsed_usec_ = 0;
        EVO_TIMER_GET(start_);
    }

    /** Resume timer from last stop.
     - This starts timer again like a stopwatch
     - If timer never started then this is same as start()
     - Call stop() to stop timer
    */
    void resume() {
        active_ = true;
        EVO_TIMER_GET(start_);
    }

    /** Stop timer.
     - Time elapsed is stored internally
     - Timer may started again with start() or resume()
    */
    Timer& stop() {
        elapsed_usec_ = elapsed();
        active_ = false;
        return *this;
    }

private:
    bool   active_;            ///< Whether timer is active (started)
    Stamp  start_;            ///< Start timestamp
    ulongl elapsed_usec_;    ///< Stored time elapsed in microseconds

    /** Get current elapsed time in microseconds.
     \return  Elapsed time
    */
    ulongl elapsed() const {
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
//@}
}
#endif
