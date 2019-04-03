// Evo C++ Library
/* Copyright 2019 Justin Crowell
Distributed under the BSD 2-Clause License -- see included file LICENSE.txt for details.
*/
///////////////////////////////////////////////////////////////////////////////
/** \file timer.h Evo Timer classes. */
#pragma once
#ifndef INCL_evo_timer_h
#define INCL_evo_timer_h

#include "impl/systime.h"

namespace evo {
/** \addtogroup EvoTime
Evo time management
*/
//@{

///////////////////////////////////////////////////////////////////////////////

/** Holds a real (wall clock) timer-stamp, used as template argument with TimerT.
 - See \link Timer\endlink, \link TimerCpu\endlink
*/
class TimerStampWall : public SysTimestamp {
public:
    void set() {
        set_wall_timer();
    }

private:
    void set_wall_datetime() EVO_ONCPP11(= delete);
    void set_cpu() EVO_ONCPP11(= delete);
};

/** Holds a CPU (process) timer-stamp, used as template argument with TimerT.
 - See \link TimerCpu\endlink, \link Timer\endlink
*/
class TimerStampCpu : public SysTimestamp {
public:
    void set() {
        set_cpu();
    }

private:
    void set_wall_datetime() EVO_ONCPP11(= delete);
    void set_wall_timer() EVO_ONCPP11(= delete);
};

///////////////////////////////////////////////////////////////////////////////

/** %Timer that works like a stopwatch.
 - Use \link Timer\endlink or \link TimerCpu\endlink
 - Accuracy depends on the system and hardware
*/
template<class T=TimerStampWall>
class TimerT {
public:
    typedef TimerT<T> This;     ///< This type

    /** Constructor. */
    TimerT() {
        memset(this, 0, sizeof(This));
    }

    /** Copy constructor.
     \param  src  Source to copy
    */
    TimerT(const This& src) {
        memcpy(this, &src, sizeof(This));
    }

    /** Assignment/Copy operator.
     \param  src  Source to copy
     \return      This
    */
    This& operator=(const This& src) {
        memcpy(this, &src, sizeof(This));
        return *this;
    }

    /** Stop and clear timer. */
    void clear() {
        memset(this, 0, sizeof(This));
    }

    /** Get whether timer is active (started).
     \return  Whether active, false if not started
    */
    bool active() const {
        return active_;
    }

    /** Get current time elapsed in floating-point seconds (with fraction).
     - If timer is active then this gives time elapsed so far, otherwise time elapsed from last start/stop
     .
     \return  Time elapsed in seconds
    */
    double sec() const {
        return (double)nsec() / SysTimestamp::NSEC_PER_SEC;
    }

    /** Get current time elapsed in milliseconds.
     - If timer is active then this gives time elapsed so far, otherwise time elapsed from last start/stop
     .
     \return  Time elapsed in milliseconds
    */
    ulongl msec() const {
        return elapsed() / SysTimestamp::NSEC_PER_MSEC;
    }

    /** Get current time elapsed in microseconds.
     - If timer is active then this gives time elapsed so far, otherwise time elapsed from last start/stop
     .
     \return  Time elapsed in microseconds
    */
    ulongl usec() const {
        return elapsed() / SysTimestamp::NSEC_PER_USEC;
    }

    /** Get current time elapsed in nanoseconds.
     - If timer is active then this gives time elapsed so far, otherwise time elapsed from last start/stop
     .
     \return  Time elapsed in nanoseconds
    */
    ulongl nsec() const {
        return elapsed();
    }

    /** Start timer.
     - This clears stored elapsed time
     - Call stop() to stop timer
    */
    void start() {
        elapsed_ = 0;
        active_ = true;
        start_.set();
    }

    /** Resume timer from last stop.
     - This starts timer again like a stopwatch
     - If timer never started then this is same as start()
     - Call stop() to stop timer
    */
    void resume() {
        active_ = true;
        start_.set();
    }

    /** Stop timer.
     - Time elapsed is stored internally
     - Timer may started again with start() or resume()
    */
    This& stop() {
        elapsed_ = elapsed();
        active_ = false;
        return *this;
    }

private:
    T start_;
    ulongl elapsed_;
    bool active_; 

    ulongl elapsed() const {
        ulongl nsec = elapsed_;
        if (active_) {
            T end;
            end.set();
            nsec += end.diff_nsec(start_);
        }
        return nsec;
    }
};

///////////////////////////////////////////////////////////////////////////////

/** %Timer that works like a stopwatch and measures real (wall clock) time.
 - See TimerT for full interface
 .

\par Example

\code
#include <evo/timer.h>
#include <evo/io.h>
using namespace evo;

int main() {
    Timer timer;

    timer.start();
    sleepus(1);
    timer.stop();

    con().out << "Slept for " << timer.nsec() << " nsec" << NL;
    return 0;
}
\endcode
*/
typedef TimerT<TimerStampWall> Timer;

/** %Timer that works like a stopwatch and measures CPU (process) time.
 - See TimerT for full interface
 .

\par Example

\code
#include <evo/timer.h>
#include <evo/io.h>
using namespace evo;

int main() {
    TimerCpu timer;

    timer.start();
    sleepus(1);
    timer.stop();

    con().out << "Slept for " << timer.nsec() << " nsec" << NL;
    return 0;
}
\endcode
*/
typedef TimerT<TimerStampCpu> TimerCpu;

///////////////////////////////////////////////////////////////////////////////
//@}
}
#endif
