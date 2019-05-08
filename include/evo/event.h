// Evo C++ Library
/* Copyright 2019 Justin Crowell
Distributed under the BSD 2-Clause License -- see included file LICENSE.txt for details.
*/
///////////////////////////////////////////////////////////////////////////////
/** \file event.h Evo async event handling. */
#pragma once
#ifndef INCL_evo_event_h
#define INCL_evo_event_h

#include "atomic.h"

#if defined(EVO_CPP11)
    #include <functional>
#endif

namespace evo {
/** \addtogroup EvoEvent
Evo event processesing for async I/O
*/
//@{

///////////////////////////////////////////////////////////////////////////////

/** %Event base type used with EventQueue.
 - An event is instantiated with C++ "new" operator, then is added to an EventQueue or EventThreadPool for processing (which takes ownership of the pointer)
   - See EventQueue::add() or EventThreadPool::add()
 - The event processor calls operator()() to invoke the event, which returns whether the event is finished, and if true the processor frees the event object
 .
*/
struct Event {
    /** Constructor. */
    Event()
        { }

    /** Destructor. */
    virtual ~Event()
        { }

    /** %Event function.
     - This is called by EventQueue to process the event
     - The return value identifies whether the caller should free this object
     - \b Caution: This must not throw any exception
     .
     \return  Whether event is finished and ready to be freed, false if ownership was transferred elsewhere
    */
    virtual bool operator()() = 0;
};

///////////////////////////////////////////////////////////////////////////////

#if defined(EVO_CPP11)

/** Implement Event using a lambda function (C++11).
 - This basically wraps an Event with a lambda
 - This can add overhead if the lambda is over a few lines or has complex state (exact conditions depend on compiler) -- for best performance create a type derived from Event directly
 .
*/
class EventLambda : public Event {
public:
    typedef std::function<bool()> Lambda;   ///< Lambda function type for Event

    /** Constructor.
     \param  lambda  Lambda to use
    */
    EventLambda(const Lambda& lambda) : lambda_(lambda) {
    }

    /** Copy constructor.
     \param  src  Source to copy
    */
    EventLambda(const EventLambda& src) : lambda_(src.lambda_) {
    }

    /** Assignment operator.
     \param  src  Source to copy
     \return      This
    */
    EventLambda& operator=(const EventLambda& src) {
        lambda_ = src.lambda_;
        return *this;
    }

    // Doc by parent
    bool operator()() {
        return lambda_();
    }

private:
    Lambda lambda_;
};

#endif

///////////////////////////////////////////////////////////////////////////////

/** Lock-free event processing queue.
 - Many producers add events while one or more consumers process (invoke) them
 - This supports a "single" or "multi" consumer model, each with different advantages:
   - Single: A single consumer thread calls process() repeatedly to process events, while other threads call add() to add events to the queue
     - This is useful for integrating with another event processing loop, which will have it's own way to wait for events
     - For other cases a "wait mechanism" is likely needed when process() returns false (queue is empty), to avoid a spin loop
   - Multi: Multiple consumers allow parallel event processing and must call process_multi() with a common mutex -- this doesn't affect the producers (mutex is only for consumers)
     - This is useful with async events to offload CPU-heavy or synchronous-I/O work to another thread, freeing up the async event-loop to handle other events
     - For CPU-heavy threads this generally shouldn't have more threads then the number of CPUs on the system
     - For synchronous-I/O more threads are likely needed since they spend time blocking (waiting) -- this should be tuned for a given system
   .
 - Each event:
   - is a callback object inheriting Event, which is popped from the queue and invoked (called) by the consumer thread
   - is assigned a unique sequence number -- this can handle 1 million events per second for over 500,000 years
     before maxing out `uint64` sequence numbers -- results are undefined if the sequence number overflows
   .
 - While the queue is full, producers will spin wait (with 1 nanosecond sleep) -- this should be avoided
 - \b Caution: When events are processed on the same thread, an event must not call add() on the same queue that invoked (called) the event, this will deadlock if the queue is full
 - \b Caution: Graceful shutdown with empty queue expected -- incomplete events can leak memory or have undefined behavior
 .
 \tparam  T  %Event type to use -- must be Event or inherit from it
              - Concrete types have slightly better performace (inlining, no vtable looups)

\par Example

\code
#include <evo/event.h>
using namespace evo;

struct MyEvent : Event {
    bool operator()() {
        // ...
        return true;
    }
};

int main() {
    EventQueue<> queue;

    // Add an event to queue
    queue.add(new MyEvent);

    // Process queue (single consumer)
    queue.process();
    return 0;
}
\endcode
*/
template<class T=Event>
class EventQueue {
public:
    typedef T EventT;       ///< %Event type used
    typedef uint Size;      ///< Queue size type

    static const Size DEFAULT_SIZE = 256;   ///< Default queue size

    /** Constructor.
     \param  size  Queue size, rounded up to nearest power of 2
    */
    EventQueue(Size size=DEFAULT_SIZE) {
        ringbuf_size_ = adjust_size(size);
        ringbuf_ = new T*[ringbuf_size_];
        ringbuf_size_mask_ = ringbuf_size_ - 1; // mask: set all bits up to size
        memset(ringbuf_, 0, sizeof(T*) * ringbuf_size_);
        next_pos_.store(1);
        read_pos_.store(1);
    }

    /** Destructor.
     - Queue should be empty, otherwise incomplete events can leak memory or have undefined behavior
    */
    ~EventQueue() {
        if (read_pos_.load() <= cursor_pos_.load()) {
            assert( false ); // Queue should be empty
        }
        delete [] ringbuf_;
    }

    /** Add an event to queue.
     - This takes ownership of the event pointer, and will free it once the event is completed (via C++ delete operator)
       - The event is only freed if it returns true, otherwise it's assumed that ownership was transferred elsewhere
     - This blocks while queue is full (spin-wait with 1 nanosecond sleep) -- a full queue should be avoided
     - If this is a multi-consumer queue using process_multiwait(), call notify_multiwait() after this
     - \b Caution: When events are processed on the same thread, do not call from the same queue that invoked (called) the event, this will deadlock if the queue is full
     .
     \param  event        %Event pointer to add and take ownership of
     \param  spinwait_ns  Spin-wait sleep time in nanoseconds (usually default is preferred) -- used to sleep each loop while spin waiting
    */
    void add(T* event, ulongl spinwait_ns=1) {
        // Claim a slot and wait for available capacity
        const uint64 seq = next_pos_.fetch_add(1, EVO_ATOMIC_ACQ_REL);
        while (seq - read_pos_.load(EVO_ATOMIC_ACQUIRE) >= ringbuf_size_)
            sleepns(spinwait_ns);

        // Store event in queue
        EVO_ATOMIC_FENCE(EVO_ATOMIC_ACQUIRE);
        ringbuf_[seq & ringbuf_size_mask_] = event;
        EVO_ATOMIC_FENCE(EVO_ATOMIC_RELEASE);

        // Wait for cursor to reach previous slot, then increment cursor to commit the write
        const uint64 prev_seq = seq - 1;
        while (!cursor_pos_.compare_set(prev_seq, seq, EVO_ATOMIC_ACQ_REL, EVO_ATOMIC_ACQUIRE))
            sleepns(spinwait_ns);
    }

    /** Notify an item has been added with multiple consumer threads.
     - This is non-blocking so doesn't notify if `condmutex` is locked, but the event will be picked up when a consumer wakes up -- see `waitms` process_multiwait()
     - Only use this where threads are using process_multiwait()
     - \b Caution: Do not mix with process() or threads using process_multi() on the same instance
     .
     \tparam  U  Conditio/Mutex type, inferred from argument
     \param  condmutex  Condition/mutex to use
    */
    template<class U>
    void notify_multiwait(U& condmutex) {
        if (condmutex.trylock()) { // non-blocking
            condmutex.notify();
            condmutex.unlock();
        }
    }

    /** Process queued events and return.
     - This pops and invokes (calls) all queued events
     - Popped events that return true are freed, otherwise they are detached (owned elsewhere)
     - \b Caution: Only 1 thread may call this at a time, otherwise results are undefined
     .
     \return  Whether any events were processed
    */
    bool process() {
        T* event;
        uint64 start, seq;
        start = seq = read_pos_.load(EVO_ATOMIC_ACQUIRE);
        while (seq <= cursor_pos_.load(EVO_ATOMIC_ACQUIRE)) {
            event = ringbuf_[seq & ringbuf_size_mask_];
            seq = read_pos_.fetch_add(1, EVO_ATOMIC_ACQ_REL) + 1;
            if ((*event)())
                delete event;
        }
        return (seq > start);
    }

    /** Process queued events and return, allowing multiple consumer threads.
     - This locks the mutex while extracting the next event, and unlocks it while invoking (calling) the event (giving the queue to the next consumer)
     - \b Caution: Do not mix with process() on the same instance
     .
     \tparam  U  Mutex or Condition type, inferred from argument
     \param  mutex  Mutex or Condition object to lock while extracting next event from queue
     \return        Whether any events were processed
    */
    template<class U>
    bool process_multi(U& mutex) {
        T* event;
        uint64 seq, count = 0;
        typename U::Lock lock(mutex);
        seq = read_pos_.load(EVO_ATOMIC_RELAXED);
        for (; seq <= cursor_pos_.load(EVO_ATOMIC_ACQUIRE); ++count) {
            event = ringbuf_[seq & ringbuf_size_mask_];
            read_pos_.fetch_add(1, EVO_ATOMIC_RELEASE);
            lock.unlock();
            if ((*event)())
                delete event;
            lock.lock();
            seq = read_pos_.load(EVO_ATOMIC_RELAXED);
        }
        lock.unlock();
        return (count > 0);
    }

    /** Process queued events until stopflag is set, allowing multiple consumer threads, and waiting with condmutex while idle.
     - This locks the mutex while extracting the next event, and unlocks it while invoking (calling) the event (giving the queue to the next consumer)
     - While queue is empty this waits on `condmutex` using a timeout of `waitms`
     - Call notify_multiwait() after adding an event to try to wake up a consumer
     - \b Caution: Do not mix with process() or process_multi() on the same instance
     .
     \tparam  U  Condition/Mutex type, inferred from argument
     \param  condmutex  Condition/mutex object to lock while extracting next event from queue
     \param  stopflag   Flag to stop processing when set to non-zero
     \param  waitms     Max wait time in milliseconds, 0 for none (spin-wait, not recommended)
    */
    template<class U>
    void process_multiwait(U& condmutex, AtomicInt& stopflag, ulong waitms=1) {
        T* event;
        uint64 seq;
        typename U::Lock lock(condmutex);
        for (;;) {
            seq = read_pos_.load(EVO_ATOMIC_RELAXED);
            for (; seq <= cursor_pos_.load(EVO_ATOMIC_ACQUIRE);) {
                event = ringbuf_[seq & ringbuf_size_mask_];
                read_pos_.fetch_add(1, EVO_ATOMIC_RELEASE);
                lock.unlock();
                if ((*event)())
                    delete event;
                lock.lock();
                seq = read_pos_.load(EVO_ATOMIC_RELAXED);
            }
            if (stopflag.load(EVO_ATOMIC_RELAXED))
                break;
            condmutex.wait(waitms, true);
        }
        lock.unlock();
    }

private:
    // Disable copying
    EventQueue(const EventQueue&);
    EventQueue& operator=(const EventQueue&);

    // Ring buffer
    T**  ringbuf_;
    Size ringbuf_size_;         // Must be a power of 2 for mask to work
    Size ringbuf_size_mask_;    // Mask for faster modulus

    // Positions increase to infinity (index = pos % ringbuf_size_), would take hundreds of years to max out 64 bits
    AtomicUInt64 cursor_pos_;   // Position of latest item committed to queue
    AtomicUInt64 next_pos_;     // Next write position in queue (cursor + 1 when no add() in progress)
    AtomicUInt64 read_pos_;     // Position of next item to read from queue (cursor + 1 when queue is empty)

    // Make sure size is within min/max and is a power of 2
    static Size adjust_size(Size size) {
        const Size MIN_SIZE = 16;
        const Size MAX_SIZE = (std::numeric_limits<Size>::max() >> 1) + 1;
        if (size <= MIN_SIZE)
            size = MIN_SIZE;
        else if (size >= MAX_SIZE)
            size = MAX_SIZE;
        else
            size = next_pow2(size);
        return size;
    }
};

///////////////////////////////////////////////////////////////////////////////
//@}
}
#endif
