// Evo C++ Library
/* Copyright 2019 Justin Crowell
Distributed under the BSD 2-Clause License -- see included file LICENSE.txt for details.
*/
///////////////////////////////////////////////////////////////////////////////
/** \file event_thread.h Evo EventThreadPool class. */
#pragma once
#ifndef INCL_evo_event_thread_h
#define INCL_evo_event_thread_h

#include "event.h"
#include "thread.h"

namespace evo {
/** \addtogroup EvoEvent */
//@{

///////////////////////////////////////////////////////////////////////////////

/** Internal state used by EventThreadPool. */
struct EventThreadState {
    EventQueue<> queue;     ///< EventQueue used by pool
    Condition condmutex;    ///< Condition for multithreading
    AtomicInt shutdown;     ///< Thread shutdown flag
    ulong waitms;           ///< Thread wait timeout in milliseconds
};

/** %Event processing thread pool.
 - This runs a group of threads as EventQueue consumers that process events added to the queue
 - %Events are popped from the queue and invoked by a thread in the pool, then are destroyed (if event returns true)
 - Use start() to start threads, and shutdown() and join() to shutdown
 - Use add() to add events to queue
 .

\par Example

\code
#include <evo/event_thread.h>
using namespace evo;

struct MyEvent : Event {
    bool operator()() {
        // ...
        return true;
    }
};

int main() {
    // Setup
    EventThreadPool threadpool;
    threadpool.start(2);

    // Add an event to invoke in thread pool
    threadpool.add(new MyEvent);

    // Shutdown
    threadpool.shutdown().join();
    return 0;
}
\endcode
*/
class EventThreadPool : public ThreadGroup<Thread,EventThreadState> {
public:
    typedef EventThreadPool This;   ///< This type

    /** Constructor.
     \param  wait_timeout_ms  Maximum thread wait while idle -- see `waitms` in EventQueue::process_multiwait()
    */
    EventThreadPool(ulong wait_timeout_ms=1) : ThreadGroup<Thread,EventThreadState>(thread_run) {
        shared_state.waitms = wait_timeout_ms;
    }

    /** Add an event to queue to be processed.
     - This takes ownership of the event pointer, and will free it once the event is completed (via C++ delete operator)
       - The event is only freed if it returns true, otherwise it's assumed that ownership was transferred elsewhere
     - This blocks while queue is full (spin-wait with 1 nanosecond sleep) -- a full queue should be avoided
     - \b Caution: Do not call from the same queue that invoked (called) the event, this will deadlock if the queue is full
     .
     \param  event        %Event pointer to add and take ownership of
     \param  spinwait_ns  Spin-wait sleep time in nanoseconds (usually default is preferred) -- used to sleep each loop while spin waiting
     \return              This
    */
    This& add(Event* event, ulongl spinwait_ns=1) {
        shared_state.queue.add(event, spinwait_ns);
        shared_state.queue.notify_multiwait(shared_state.condmutex);
        return *this;
    }

    /** Stop processing events and shutdown threads.
     - This starts the shutdown process, call join() to wait for all threads to fully stop
     .
     \return  This
    */
    This& shutdown() {
        shared_state.shutdown.store(1, EVO_ATOMIC_RELEASE);
        shared_state.queue.notify_multiwait(shared_state.condmutex);
        return *this;
    }

private:
    typedef ThreadGroup<Thread,EventThreadState> Base;

    Base& cancel() EVO_ONCPP11(= delete);
    bool cancelled() const EVO_ONCPP11(= delete);

    static void thread_run(void* arg) {
        EventThreadState& state = *(EventThreadState*)arg;
        const ulong waitms = state.waitms;
        while (state.shutdown.load(EVO_ATOMIC_ACQUIRE) == 0)
            state.queue.process_multiwait(state.condmutex, state.shutdown, waitms);
    }
};

///////////////////////////////////////////////////////////////////////////////
//@}
}
#endif
