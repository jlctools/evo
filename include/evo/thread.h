// Evo C++ Library
/* Copyright 2019 Justin Crowell
Distributed under the BSD 2-Clause License -- see included file LICENSE.txt for details.
*/
///////////////////////////////////////////////////////////////////////////////
/** \file thread.h Evo threads implementation. */
#pragma once
#ifndef INCL_evo_thread_h
#define INCL_evo_thread_h

#include "impl/systhread.h"
#include "thread_inert.h"
#include "atomic.h"
#include "lock.h"
#include "type.h"
#if defined(EVO_CPP11)
    #include <functional>
#endif

///////////////////////////////////////////////////////////////////////////////

// Disable certain MSVC warnings for this file, if !EVO_CPP11
#if defined(_MSC_VER) && !defined(EVO_CPP11)
    #pragma warning(push)
    #pragma warning(disable:4355)
#endif

namespace evo {
/** \addtogroup EvoThread
Evo threads and atomics
*/
//@{

///////////////////////////////////////////////////////////////////////////////

/** Get current thread ID from system.
 - \#include <evo/thread.h>
 .
 \return  Current thread ID
*/
inline ulong get_tid()
    { return SysThread::id(); }

///////////////////////////////////////////////////////////////////////////////

/** Spin-lock for thread synchronization.
 - This works like Mutex but does a busy wait (no sleep) while waiting for a lock
 - This consumes CPU and can be more efficient than Mutex under certain conditions, where wasting CPU is acceptable
   - This also supports a less busy wait with SpinLock::SleepLock
 - %Thread safe
 - See also: \link evo::SleepLock SleepLock\endlink
 - \b Caution: Only use if you're certain you understand the consequences, if in doubt use Mutex instead
*/
struct SpinLock : public AtomicFlag {
    typedef SmartLock<AtomicFlag>      Lock;        ///< Lock object type -- see SmartLock
    typedef SmartSleepLock<AtomicFlag> SleepLock;   ///< Sleep-Lock object type -- see SmartSleepLock

    /** Constructor. */
    SpinLock()
        { }

private:
    // Disable copying
    SpinLock(SpinLock&);
    SpinLock& operator=(SpinLock&);
};

///////////////////////////////////////////////////////////////////////////////

/** Variation of SpinLock for thread synchronization using sleep-wait.
 - This works the same as SpinLock but uses sleeplock() to lock
 - This is generally preferred over SpinLock since it wastes less CPU
 - %Thread safe
 - \b Caution: Only use if you're certain you understand the consequences, if in doubt use Mutex instead
*/
struct SleepLock : public SpinLock {
    typedef SmartSleepLock<AtomicFlag> Lock;    ///< Sleep-Lock object type -- see SmartSleepLock

    /** Constructor. */
    SleepLock()
        { }

private:
    // Disable copying
    SleepLock(SleepLock&);
    SleepLock& operator=(SleepLock&);

    // Disable sleepless spin-lock
    using SpinLock::lock;
};

///////////////////////////////////////////////////////////////////////////////

/** %Mutex for thread synchronization.
 - Used to synchronize access to shared data by multiple threads
 - This is a low-level interface with public members
 - %Thread safe
 - Linking:
   - Linux/Unix: `-pthread`
   - Cygwin: `-lpthread`
   - Windows: Usually multithreaded by default -- MSVC project settings: `C/C++ -> Code Generation -> Runtime Library`
 - See also: MutexInert
*/
struct Mutex : public SysMutex {
    typedef SmartLock<Mutex> Lock;      ///< Lock object type -- see SmartLock

    /** Constructor. */
    Mutex()
        { }

private:
    // Disable copying
    Mutex(const Mutex&);
    Mutex& operator=(const Mutex&);
};

///////////////////////////////////////////////////////////////////////////////

/** Read/Write %Mutex for thread synchronization.
 - Shared mutex used to synchronize read/write access to shared data by multiple threads
   - This allows multiple concurrent read (shared) locks while not locked for writing
   - Write access requires a full normal lock (same interface as Mutex), blocks all reader locks and other write locks -- see Lock
   - Read access requires a read (aka shared) lock, blocks write locks but doesn't block other reader locks -- see LockRead
   - Write locks take priority so this doesn't have a "writer starvation" problem with constant reads
   .
 - %Thread safe
 - Linking:
   - Linux/Unix: `-pthread`
   - Cygwin: `-lpthread`
   - Windows: Usually multithreaded by default -- MSVC project settings: `C/C++ -> Code Generation -> Runtime Library`
*/
struct MutexRW {
    typedef SmartLock<MutexRW>     Lock;          ///< Write Lock object type, general Mutex interface (Mutex::Lock will also work) -- see SmartLock
    typedef SmartLock<MutexRW>     LockWrite;     ///< Write Lock object type -- see SmartLock
    typedef SmartLockRead<MutexRW> LockRead;      ///< Read Lock object type -- see SmartLockRead

    /** Constructor. */
    MutexRW()
        { counter_ = 0; }

    /** Destructor. */
    ~MutexRW() {
        assert( counter_ == 0 ); // shouldn't be any read locks
    }

    /** Try to Write-Lock mutex without blocking.
     - This allows polling for a write-lock without blocking
     - If current thread already has a lock, whether this succeeds is platform dependent -- some platforms (Windows) allow nested locks, others don't
     - \b Caution: Polling with this can starve (never lock) under high load (constant read/write locks)
     .
     \return  Whether successful, false if write-lock not available
    */
    bool trylock()
        { return write_mutex_.trylock(); }

    /** Try to Write-Lock mutex with a timeout.
     - This allows polling for a write-lock until timeout
     - If current thread already has a lock, whether this succeeds is platform dependent -- some platforms (Windows) allow nested locks, others don't
     - OSX: This does a spin wait (which consumes CPU) since OSX doesn't support timeout on pthread mutex lock
     - Windows: This does a spin wait (which consumes CPU) since Windows doesn't support timeout on Critical Section lock
     - \b Caution: This can starve (never lock) in Windows or OSX under high load (constant locks)
     .
     \param  timeout_ms  Timeout in milliseconds
     \return             Whether successful, false on timeout
    */
    bool trylock(ulong timeout_ms)
        { return write_mutex_.trylock(timeout_ms); }

    /** Write-Lock mutex.
     - Must call unlock() after each lock(), otherwise results are undefined
     - Results are undefined if already write-locked by current thread
    */
    void lock()
        { write_mutex_.lock(); }

    /** Write-Unlock mutex.
     - Results are undefined if called while mutex not write-locked
    */
    void unlock()
        { write_mutex_.unlock(); }

    /** Try to Read-Lock mutex without blocking.
     - This allows polling for a read-lock without blocking
     - If current thread already has a lock, whether this succeeds is platform dependent -- some platforms (Windows) allow nested locks, others don't
     - \b Caution: Polling with this can starve (never lock) under high load (constant read/write locks)
     .
     \return  Whether successful, false if read-lock not available
    */
    bool trylock_read() {
        bool result = false;
        if (read_mutex_.trylock()) {
            if (counter_ > 0 || write_mutex_.trylock()) {
                result = true;
                ++counter_;
                assert(counter_ > 0);
            }
            read_mutex_.unlock();
        }
        return result;
    }

    /** Read-Lock mutex.
     - Must call unlock_read() after each lock_read(), otherwise results are undefined
     - Results are undefined if already read-locked by current thread
    */
    void lock_read() {
        read_mutex_.lock();
        if (++counter_ == 1)
            write_mutex_.lock();
        assert(counter_ > 0);
        read_mutex_.unlock();
    }

    /** Read-Unlock mutex.
     - Results are undefined if called while mutex not read-locked
    */
    void unlock_read() {
        read_mutex_.lock();
        assert(counter_ > 0);
        if (--counter_ == 0)
            write_mutex_.unlock();
        read_mutex_.unlock();
    }

private:
    ulong counter_;
    Mutex read_mutex_;
    Mutex write_mutex_;
};

///////////////////////////////////////////////////////////////////////////////

/** %Condition object for thread synchronization.
 - Used to make one or more threads sleep until a notification is signalled
 - This works with an associated mutex, which is created if needed
 - This is a low-level interface with public members
 - %Thread safe
 - Linking:
   - Linux/Unix: `-pthread`
   - Cygwin: `-lpthread`
   - Windows: Usually multithreaded by default -- MSVC project settings: `C/C++ -> Code Generation -> Runtime Library`
*/
struct Condition {
    typedef SmartLock<Condition> Lock;      ///< Lock object type -- see SmartLock

#if defined(_WIN32)
    // Windows
    typedef CONDITION_VARIABLE Handle;

    static const ulong INF = INFINITE;

    Condition() {
        InitializeConditionVariable(&handle);
        owned = false;
        mutex = new Mutex;
        owned = true;
    }

    Condition(Mutex& mutex) : mutex(&mutex), owned(false)
        { InitializeConditionVariable(&handle); }

    bool wait(ulong timeout_ms=Condition::INF, bool locked=true) {
        if (!locked) {
            if (timeout_ms >= Condition::INF)
                mutex->lock();
            else
                mutex->trylock(timeout_ms);
        }
        return (SleepConditionVariableCS(&handle, &mutex->handle, timeout_ms) != 0);
    }

    void notify()
        { WakeConditionVariable(&handle); }

    void notify_all()
        { WakeAllConditionVariable(&handle); }

#else
    // Linux/Unix
    typedef pthread_cond_t Handle;

    static const ulong INF = ULONG_MAX;     ///< Infinite wait timeout value

    /** Default constructor.
     - This creates (and owns) a new default mutex to use
    */
    Condition() {
        init();
        owned = false;
        mutex = new Mutex;
        owned = true;
    }

    /** Constructor.
     - This associates with an existing mutex (doesn't assume ownership)
     - Associated mutex must remain valid while used here
     .
     \param  mutex  Mutex to associate with
    */
    Condition(Mutex& mutex) : mutex(&mutex), owned(false) {
        init();
    }

    /** Wait for notification or timeout.
     - Either the associated mutex must already be locked, or set locked to false to lock here
     - Associated mutex is atomically unlocked during wait, and will be locked again when this returns
     - This may wake up early for no apparent reason, so actual state should be tracked with another variable
     - Waiting forever is usually not ideal, threads should wake periodically to check if cancelled
     - When `locked=false`:
       - This will try to lock with `timeout_ms`
       - Windows & OSX: Trying to lock does a spin wait (which consumes CPU) -- to avoid this lock first then pass `locked=true`
     .
     \param  timeout_ms  Wait timeout in milliseconds, 0 for immediate (no wait), INF for indefinitely
     \param  locked      Whether to assume associated mutex is locked, false to lock it here
     \return             Whether condition was notified, false on error (timed out)
    */
    bool wait(ulong timeout_ms=Condition::INF, bool locked=true) {
        if (timeout_ms >= Condition::INF) {
            if (!locked)
                mutex->lock();
            return (pthread_cond_wait(&handle, &mutex->handle) == 0);
        } else {
            if (!locked)
                mutex->trylock(timeout_ms);
            // Clock choice should be consistent with init() below
            struct timespec ts;
            #if defined(_POSIX_TIMERS) && defined(CLOCK_REALTIME) && !defined(EVO_USE_GETTIMEOFDAY)
                #if defined(CLOCK_MONOTONIC) && !defined(__APPLE__)
                    ::clock_gettime(CLOCK_MONOTONIC, &ts);
                #else
                    ::clock_gettime(CLOCK_REALTIME, &ts);
                #endif
            #else
            {
                struct timeval tv;
                ::gettimeofday(&tv, NULL);
                SysLinux::set_timespec_tv(ts, tv);
            }
            #endif
            SysLinux::add_timespec_ms(ts, timeout_ms);
            return (pthread_cond_timedwait(&handle, &mutex->handle, &ts) == 0);
        }
    }

    /** Notify and wake a waiting thread.
     - Mutex lock is not required, but ideally mutex should be locked for most consistent/predictable behavior
     - See lock_notify()
    */
    void notify()
        { pthread_cond_signal(&handle); }

    /** Notify and wake all waiting threads.
     - Mutex lock is not required, but ideally mutex should be locked for most consistent/predictable behavior
     - See lock_notify_all()
    */
    void notify_all()
        { pthread_cond_broadcast(&handle); }

private:
    void init() {
        // Clock choice should be consistent with wait() above
    #if defined(_POSIX_TIMERS) && defined(CLOCK_REALTIME) && !defined(EVO_USE_GETTIMEOFDAY) && !defined(__APPLE__)
        clockid_t id;
        #if defined(CLOCK_MONOTONIC)
            id = CLOCK_MONOTONIC;
        #else
            id = CLOCK_REALTIME;
        #endif
        pthread_condattr_t attr;
        pthread_condattr_init(&attr);
        pthread_condattr_setclock(&attr, id);
        pthread_cond_init(&handle, &attr);
    #else
        pthread_cond_init(&handle, NULL);
    #endif
    }

public:
#endif

    /** Destructor. */
    ~Condition() {
        if (owned)
            delete mutex;
    }

    /** Wait for notification.
     - This calls wait() with `INF` timeout
     - See notes for wait()
     .
     \param  locked  Whether to assume associated mutex is locked, false to lock it here
     \return         Whether condition was notified, false on error (timed out)
    */
    bool wait_inf(bool locked=true) {
        bool result = wait(Condition::INF, locked);
        assert( result ); // should never timeout
        return result;
    }

    /** Try to lock associated mutex, fail if already locked (non-blocking).
     - On success, must call unlock() to unlock mutex
     - Results are undefined if already locked by current thread
     .
     \return  Whether successful, false if lock failed because mutex is already locked
    */
    bool trylock()
        { return mutex->trylock(); }

    /** Try to lock associated mutex with a timeout.
     - On success, must call unlock() to unlock mutex
     - Results are undefined if already locked by current thread
     - OSX: This does a spin wait (which consumes CPU) since OSX doesn't support timeout on pthread mutex lock
     - Windows: This does a spin wait (which consumes CPU) since Windows doesn't support timeout on Critical Section lock
     .
     \param  timeout_ms  Timeout in milliseconds
     \return             Whether successful, false on timeout
    */
    bool trylock(ulong timeout_ms)
        { return mutex->trylock(timeout_ms); }

    /** Lock associated mutex.
     - Must call unlock() after each lock(), otherwise results are undefined
     - Results are undefined if already locked by current thread
    */
    void lock()
        { mutex->lock(); }

    /** Unlock associated mutex.
     - Results are undefined if called while mutex not locked
    */
    void unlock()
        { mutex->unlock(); }

    /** Lock associated mutex, call wait(), then unlock.
     \param  timeout_ms  Wait timeout in milliseconds, 0 for immediate (no wait), INF for indefinitely
     \return             Whether condition was notified, false on error (timed out)
    */
    bool lock_wait(ulong timeout_ms=Condition::INF) {
        const bool result = wait(timeout_ms, false);
        mutex->unlock();
        return result;
    }

    /** Lock associated mutex, call notify(), then unlock.
     - This wakes up a waiting thread
    */
    void lock_notify() {
        mutex->lock();
        notify();
        mutex->unlock();
    }

    /** Lock associated mutex, call notify_all(), then unlock.
     - This wakes up all waiting threads
    */
    void lock_notify_all() {
        mutex->lock();
        notify_all();
        mutex->unlock();
    }

    Handle handle;      ///< Condition object handle -- do not modify
    Mutex* mutex;       ///< Pointer to associated mutex, never NULL -- do not modify
    bool   owned;       ///< Whether associated mutex is owned by this -- do not modify

private:
    // Disable copying
    Condition(const Condition&);
    Condition& operator=(const Condition&);
};

///////////////////////////////////////////////////////////////////////////////

// Implementation helpers for Thread
/** \cond impl */
namespace impl {
    // Basic smart pointer used internally without depending on ptr.h
    template<class T> struct BasicSmartPtr {
        T* ptr;

        BasicSmartPtr(T* ptr=NULL) : ptr(ptr)
            { }

        ~BasicSmartPtr() {
            if (ptr != NULL)
                delete ptr;
        }

    private:
        // Disable copying
        BasicSmartPtr(const BasicSmartPtr&);
        BasicSmartPtr& operator=(const BasicSmartPtr&);
    };
}
/** \endcond */

/** Manages a single thread of execution.
 - This implements a function-based thread handler, and serves as a common thread interface
 - Function-based thread:
   - Caller provides a thread function and an optional pointer argument
   - May be detached to run in the background
   .
 - See ThreadClass for a class-based thread, preferred with threads that share state
   - Note that this (Thread) is the real base class for *all* Evo threads, while ThreadClass is the base class for class-based threads
 - See also ThreadScope<Thread> for creating a simple scoped thread, and ThreadGroup for creating a group of threads
 - Linking:
   - Linux/Unix: `-pthread`
   - Cygwin: `-lpthread`
   - Windows: Usually multithreaded by default -- MSVC project settings: `C/C++ -> Code Generation -> Runtime Library`

\par Example

\code
#include <evo/thread.h>
using namespace evo;

static void thread_run(void* arg) {
    // Thread code
}

int main() {
    Thread thread(thread_run, NULL);
    thread.thread_start();
    thread.thread_join();
    return 0;
}
\endcode
*/
struct Thread {
#if defined(EVO_CPP11)
    typedef std::function<void (void*)> Func;   ///< %Thread function type -- with C++11 supports lambda/functor, otherwise just function pointer
#else
    typedef void (*Func)(void*);                ///< %Thread function pointer
#endif

    /** Initialization structure for starting a function-based thread. */
    struct Init {
        Func  func;     ///< %Thread function to use
        void* arg;      ///< Argument for thread function

        Init() : func(NULL), arg(NULL)
            { }

        Init(Func func, void* arg=NULL) : func(func), arg(arg)
            { }

        Init(const Init& src) : func(src.func), arg(src.arg)
            { }
    };

    Init thread_init;   ///< %Thread function pointer

    /** Constructor. */
    Thread() : thread_active_(false)
        { }

    /** Constructor for function-based thread.
     \param  func  Thread function pointer
     \param  arg   Argument to pass to thread function
    */
    Thread(Func func, void* arg=NULL) : thread_init(func, arg), thread_active_(false)
        { }

    /** Constructor for function-based thread.
     \param  init  Initialization data for thread
    */
    Thread(const Init& init) : thread_init(init), thread_active_(false)
        { }

    /** Destructor.
     - Calls thread_join() to wait for thread to stop, if active
     - \b WARNING: Class-based threads must be stopped already -- see ThreadClass
    */
    virtual ~Thread()
        { thread_join(); }

    /** Get platform-specific thread handle.
     \return  Thread handle
    */
    SysThread::Handle thread_handle() const
        { return thread_impl_.handle; }

    /** Get whether thread is active (running).
     - This gets whether the thread was started but hasn't been joined yet -- the thread may have terminated though
     .
     \return  Whether active, true if thread has started and hasn't been joined yet
    */
    bool thread_active() const
        { return thread_active_; }

    /** Start thread.
     - The thread will start immediately, possibly even before this returns
     .
     \return  Whether successful, false on error (out of memory/resources or thread_init.func=NULL)
    */
    bool thread_start() {
        if (!thread_active_ && thread_init.func != NULL) {
            impl::BasicSmartPtr<Init> init(new Init(thread_init));
            if (thread_impl_.start(Thread::thread_handler, init.ptr) == ENone) {
                // init.ptr freed by thread
                init.ptr       = NULL;
                thread_active_ = true;
                return true;
            }
        }
        return false;
    }

    /** Detach thread to run in background.
     - The thread will continue running and will no longer be joinable or accessible
     - Start a new thread with thread_start()
    */
    virtual bool thread_detach() {
        if (thread_active_) {
            thread_impl_.detach();
            thread_active_ = false;
        }
        return true;
    }

    /** Join thread by waiting for thread to stop.
     - Class-based thread: May need to call thread_cancel() and wake the thread first, depending on thread implementation
     .
     \return  Whether successful, false on error (out of memory/resources) or if thread not started
    */
    bool thread_join() {
        if (thread_active_ && thread_impl_.join()) {
            thread_active_ = false;
            return true;
        }
        return false;
    }

    /** Yield control to another thread or process. */
    static void yield()
        { SysThread::yield(); }

    /** Get current thread ID from system.
     \return  Current thread ID
    */
    static ulong id()
        { return SysThread::id(); }

protected:
    SysThread thread_impl_;
    bool      thread_active_;

private:
    // Platform-specific handler
    static EVO_THREAD_RUN_DEFINE(thread_handler, ptr) {
        assert(ptr != NULL);
        Thread::Init* init_ptr = (Thread::Init*)ptr;
        if (init_ptr != NULL) {
            Thread::Init init(*init_ptr);
            delete init_ptr;
            if (init.func != NULL)
                init.func(init.arg);
        }
        EVO_THREAD_RUN_RETURN;
    }
};

///////////////////////////////////////////////////////////////////////////////

/** Base class for managing a single class-based thread of execution.
 - A class-based thread is implemented by inheriting this and implementing thread_run()
   - The thread is finished when thread_run() returns
   - For long running threads the thread_run() method should call cancel_check() regularly, and return ASAP when cancelled -- otherwise the parent thread can hang waiting on it
   - Synchronization objects and shared state should be stored as member variables
   - \b WARNING: %Thread must be stopped before ThreadClass destructor is called -- otherwise it will crash (abort or segfault)
     - The derived class destructor should call thread_cancel_join() or thread_join() to make sure the thread stops
     - Alternatively use ThreadScope to create threads, which will stop the thread in the destructor
   .
 - See Thread for function-based thread
 - See also ThreadScope for creating a simple scoped thread, and ThreadGroup for creating a group of threads
 - Linking:
   - Linux/Unix: `-pthread`
   - Cygwin: `-lpthread`
   - Windows: Usually multithreaded by default -- MSVC project settings: `C/C++ -> Code Generation -> Runtime Library`

\par Example

\code
#include <evo/thread.h>
using namespace evo;

class MyThread : public ThreadClass {
public:
    ~MyThread() {
        thread_cancel_join();
    }

    void thread_run() {
        // Thread code
    }
};

int main() {
    MyThread thread;
    thread.thread_start();
    thread.thread_cancel_join();
    return 0;
}
\endcode
*/
struct ThreadClass : public Thread {
    Condition condmutex;        ///< Condition and mutex object for thread, used to wake up thread

    /** Constructor. */
    ThreadClass() : Thread(ThreadClass::thread_base_handler, this)
        { }

    /** Destructor.
     - Derived class destructor should call thread_cancel_join() to stop the thread
     - \b WARNING: This crashes (calls abort()) if thread is still running -- though the running thread may segfault first since at this point the thread object is already partially destroyed
    */
    virtual ~ThreadClass() {
        if (thread_active_)
            abort();    // Thread must be stopped/joined first!
    }

    /** %Set cancel flag to signal thread to stop, and wake thread via condition object.
     - Cancellation is not enforced, the thread has to call cancel_check() regularly to check if it's been cancelled
     - After setting cancel flag, this notifies on condmutex to wake thread
     - This always unlocks condmutex when done
     .
     \param  locked  Whether condmutex object is already locked
     \return         This
    */
    ThreadClass& thread_cancel(bool locked=false) {
        if (thread_active_)
            thread_cancel_flag_.store(1);
        if (!locked)
            condmutex.lock();
        condmutex.notify_all();
        condmutex.unlock();
        return *this;
    }

    /** Cancels and joins thread.
     - This calls thread_cancel() then thread_join()
     - Destructors in derived class should call this to stop the thread
     .
     \param  locked  Passed to thread_cancel(): Whether condmutex object is already locked
     \return         Whether successful, false on error (out of memory/resources) or if thread not started
    */
    bool thread_cancel_join(bool locked=false)
        { return thread_cancel(locked).thread_join(); }

    /** %Thread run method.
     - This is the thread implementation
     - This should NOT use anything in base class (ThreadClass) beginning with "thread_", otherwise results are undefined
     - Cancellation is not enforced, this has to call cancel_check() regularly to check if it's been cancelled, and if cancelled return ASAP from this method
     - This should NOT block indefinitely without at least checking for cancellation -- otherwise this can hang the parent thread
    */
    virtual void thread_run() = 0;

    /** Check if thread has been cancelled.
     - Cancellation is not enforced, the thread has to call cancel_check() regularly to check if it's been cancelled, and stop when cancelled
     - If the thread does not properly stop itself when cancelled, this can hang the parent thread waiting for it
     .
     \return  Whether thread has been cancelled
    */
    bool cancel_check() {
        SysThread::yield();
        return (thread_cancel_flag_.load() != 0);
    }

private:
    AtomicInt thread_cancel_flag_;

    // Disable copying
    ThreadClass(const ThreadClass&);
    ThreadClass& operator=(const ThreadClass&);

    // Disable access to: thread_init, thread_detach
    using Thread::thread_init;
    bool thread_detach()
        { return false; }

    // Thread handler
    static void thread_base_handler(void* ptr)
        { ((ThreadClass*)ptr)->thread_run(); }
};

///////////////////////////////////////////////////////////////////////////////

/** Creates a single thread of execution in given scope.
 - This starts the thread in the constructor, and cancels + joins it in destructor
 - This wraps and inherits an existing thread type (T)
   - Depending on thread type (T), this will be a function-based thread (see ThreadScope<Thread>) or a class-based thread (this)
 - Class-based thread (T):
   - Derives from ThreadClass
   - Supports a cancellation mechanism -- though the thread may ignore the cancel flag
   .
 - Linking:
   - Linux/Unix: `-pthread`
   - Cygwin: `-lpthread`
   - Windows: Usually multithreaded by default -- MSVC project settings: `C/C++ -> Code Generation -> Runtime Library`
 .
 \tparam  T  %Thread type: Thread for function-based thread (default), or type derived from ThreadClass for class-based thread

\par Example -- Class-Based Thread

\code
#include <evo/thread.h>
using namespace evo;

class MyThread : public ThreadClass {
public:
    void thread_run() {
        // Thread code
    }
};

int main() {
    {
        // This runs the thread and joins in destructor
        ThreadScope<MyThread> thread;
    }
    return 0;
}
\endcode
*/
template<class T=Thread>
struct ThreadScope : public T {
    typedef ThreadScope<T> This;    ///< Current ThreadScope

    /** Constructor starts thread. */
    ThreadScope()
        { this->thread_start(); }

    /** Destructor cancels and joins thread. */
    ~ThreadScope()
        { cancel_join(); }

    /** Get platform-specific thread handle.
     \return  Thread handle
    */
    SysThread::Handle handle() const
        { return this->thread_handle(); }

    /** Get whether thread is active (running).
     \return  Whether active
    */
    bool active() const
        { return this->thread_active(); }

    /** Start thread.
     - The thread will start immediately, possibly even before this returns
     .
     \return  Whether successful, false on error (out of memory/resources or thread_init.func=NULL)
    */
    bool start()
        { return this->thread_start(); }

    /** %Set cancel flag to signal thread to stop, and wake thread via condition object.
     - Cancellation is not enforced, the thread has to call cancel_check() regularly to check if it's been cancelled
     - After setting cancel flag, this notifies on condmutex to wake thread
     - This always unlocks condmutex when done
     .
     \param  locked  Whether condmutex object is already locked
     \return         This
    */
    This& cancel(bool locked=false)
        { this->thread_cancel(locked); return *this; }

    /** Cancels and joins thread.
     - This calls cancel() then join()
     .
     \param  locked  Passed to thread_cancel(): Whether condmutex object is already locked
     \return         Whether successful, false on error (out of memory/resources) or if thread not started
    */
    bool cancel_join(bool locked=false)
        { return this->thread_cancel(locked).thread_join(); }

    /** Join thread by waiting for thread to stop.
     - Class-based thread: May need to call thread_cancel() and wake the thread first, depending on thread implementation
     .
     \return  Whether successful, false on error (out of memory/resources) or if thread not started
    */
    bool join()
        { return this->thread_join(); }

private:
    ThreadScope(const ThreadScope&);
    ThreadScope& operator=(const ThreadScope&);
};

/** Creates a single thread of execution in given scope.
 - This starts the thread in the constructor, and joins it in destructor
 - This wraps and inherits an existing thread type (T)
   - Depending on thread type (T), this will be a function-based thread (this specialization) or a class-based thread (ThreadScope)
 - Function-based thread (T):
   - Caller provides a thread function and an optional pointer argument
   - May be detached to run in the background
   .
 - Linking:
   - Linux/Unix: `-pthread`
   - Cygwin: `-lpthread`
   - Windows: Usually multithreaded by default -- MSVC project settings: `C/C++ -> Code Generation -> Runtime Library`
 .

\par Example -- Function-Based Thread

\code
#include <evo/thread.h>
using namespace evo;

static void thread_run(void* arg) {
    // Thread code
}

int main() {
    {
        // This runs the thread and joins in destructor, arg=NULL
        ThreadScope<> thread(thread_run, NULL);
    }
    return 0;
}
\endcode
*/
template<>
struct ThreadScope<Thread> : public Thread {
    typedef ThreadScope<Thread> This;

    /** Constructor starts thread.
     \param  func  Thread function pointer
     \param  arg   Argument to pass to thread function
    */
    ThreadScope(Thread::Func func, void* arg=NULL) : Thread(func, arg)
        { this->thread_start(); }

    /** Destructor joins thread. */
    ~ThreadScope()
        { this->thread_join(); }

    /** Get platform-specific thread handle.
     \return  Thread handle
    */
    SysThread::Handle handle() const
        { return this->thread_handle(); }

    /** Get whether thread is active (running).
     \return  Whether active
    */
    bool active() const
        { return this->thread_active(); }

    /** Start thread.
     - The thread will start immediately, possibly even before this returns
     .
     \return  Whether successful, false on error (out of memory/resources or thread_init.func=NULL)
    */
    bool start()
        { return this->thread_start(); }

    /** Join thread by waiting for thread to stop.
     \return  Whether successful, false on error (out of memory/resources) or if thread not started
    */
    bool join()
        { return this->thread_join(); }

    /** Detach thread to run in background.
     - The thread will continue running and will no longer be joinable or accessible
     - Start a new thread with thread_start()
    */
    bool detach()
        { return this->thread_detach(); }

private:
    ThreadScope();
    ThreadScope(const ThreadScope&);
    ThreadScope& operator=(const ThreadScope&);
};

///////////////////////////////////////////////////////////////////////////////

// Implementation helpers for ThreadGroup
/** \cond impl */
namespace impl {
    // Class-based thread node (derives from ThreadClass)
    template<class S, class T>
    struct ThreadGroupNode {
        typedef ThreadGroupNode<S,T> Node;
        static const bool THREAD_CLASS = true;

        T thread;
        Node* next;
        Node* prev;
        bool  used;

        ThreadGroupNode(Thread::Init&, S& state, Node* prev_node=NULL) : thread(state) {
            next = NULL;
            prev = prev_node;
            used = false;
        }

        void cancel()
            { thread.thread_cancel(); }
    };

    // Function-based thread node (specialize on Thread)
    template<class S>
    struct ThreadGroupNode<S,Thread> {
        typedef ThreadGroupNode<S,Thread> Node;
        static const bool THREAD_CLASS = false;

        Thread thread;
        Node* next;
        Node* prev;
        bool  used;

        ThreadGroupNode(Thread::Init& init, S& state, Node* prev_node=NULL) : thread(init) {
            next = NULL;
            prev = prev_node;
            used = false;
            thread.thread_init.arg = &state;
        }

        void cancel()
            { }
    };
}
/** \endcond */

/** Manages a group of threads with shared state.
 - This manages a collection of threads that are started/joined together
   - Each thread has access to the same state shared by all threads -- the thread group owns this shared state
 - This works with function-based threads and class-based threads, though there are some differences:
  - Class-based threads use T derived from ThreadClass
    - %Thread constructor must take a SharedState reference parameter: SharedState&
    .
  - Function-based threads use T=Thread
    - Caller must pass a thread function pointer to constructor (or set thread_init.func) before starting any threads
    - Thread function is passed a SharedState reference parameter, cast to: SharedSatate*
    .
 - %Thread safety depends on mutex type (M) -- default is MutexInert (not thread safe) as this isn't usually shared between threads
 - Linking:
   - Linux/Unix: `-pthread`
   - Cygwin: `-lpthread`
   - Windows: Usually multithreaded by default -- MSVC project settings: `C/C++ -> Code Generation -> Runtime Library`
 .
 \tparam  T  %Thread type to use, Thread (function-based thread) or class derived from ThreadClass (class-based thread)
 \tparam  S  Shared state type to use, usually contains a Mutex or Condition for synchronization, defaults to T::SharedState
 \tparam  M  %Mutex type for synchronization, defaults to MutexInert (no synchronization)

\par Example with Class-Based ThreadClass

\code
#include <evo/thread.h>
using namespace evo;

struct MyThread : public ThreadClass {
    struct SharedState {
        // Shared thread state
    };

    SharedState& state;

    MyThread(SharedState& state) : state(state)
        { }

    ~MyThread()
        { thread_cancel_join(); }

    void thread_run() {
        // Thread code
    }
};

int main() {
    ThreadGroup<MyThread> threads;
    threads.start(5);
    threads.join();
    return 0;
}
\endcode

\par Example with Function-Based Thread

\code
#include <evo/thread.h>
using namespace evo;

struct ThreadState {
    // Shared thread state
};

static void thread_run(void* arg) {
    ThreadState* state = (ThreadState*)arg;
    // Thread code
}

int main() {
    ThreadGroup<Thread,ThreadState> threads(thread_run);
    threads.start(5);
    threads.join();
    return 0;
}
\endcode
*/
template<class T, class S=typename T::SharedState, class M=MutexInert>
struct ThreadGroup {
    typedef ThreadGroup<T,S,M>    This;         ///< This type
    typedef typename Thread::Init ThreadInit;   ///< %Thread init type
    typedef S                     SharedState;  ///< %Thread shared state
    typedef M                     MutexT;       ///< %Mutex type

    /** Default constructor. */
    ThreadGroup() {
        size_  = 0;
        first_ = last_ = NULL;
        active_      = false;
        cancel_flag_ = false;
    }

    /** Constructor for function-based thread.
     \param  func  Thread function pointer
    */
    ThreadGroup(Thread::Func func) : thread_init(func) {
        size_  = 0;
        first_ = last_ = NULL;
        active_      = false;
        cancel_flag_ = false;
    }

    /** Destructor.
     - Calls cancel() and join() to stop all current threads, if active
    */
    virtual ~ThreadGroup() {
        cancel();
        join();
    }

    /** Get whether thread group is active (threads running).
     - The group is active when at least 1 thread is started, and inactive once join() returns
     - The group does not become inactive until join() is called, even if all threads finished
     .
     \return  Whether active
    */
    bool active() const
        { typename MutexT::Lock lock(mutex_); return active_; }

    /** Get cancel flag.
     - This flag is set when active and cancel() is called to stop threads
     - This flag is reset by join()
     .
     \return  Whether threads are cancelled
    */
    bool cancelled() const
        { typename MutexT::Lock lock(mutex_); return cancel_flag_; }

    /** Get thread group size.
     \return  Number of threads running
    */
    ulong size() const
        { typename MutexT::Lock lock(mutex_); return size_; }

    /** Create new threads, add to group and start them.
     - This may be called multiple times to grow the number of threads
     - This fails once cancel() is called, until join() is called to reset
     - Fails for function-based threads if thread_init.func isn't set (by constructor or directly)
     .
     \param  count  Number of threads to add and start
     \return        Whether successful, false on error
    */
    bool start(uint count=1) {
        typename MutexT::Lock lock(mutex_);
        if (count > 0 && !cancel_flag_ && (Node::THREAD_CLASS || thread_init.func != NULL)) {
            active_ = true;
            if (first_ == NULL) {
                first_ = last_ = new Node(thread_init, shared_state);
                last_->thread.thread_start();
                ++size_;
                --count;
            }

            while (count > 0) {
                last_->next = new Node(thread_init, shared_state, last_);
                last_ = last_->next;
                last_->thread.thread_start();
                ++size_;
                --count;
            }
            return true;
        }
        return false;
    }

    /** %Set cancel flags to signal all threads to stop.
     - Ignored if using function-based threads
     .
     \return  This
    */
    This& cancel() {
        typename MutexT::Lock lock(mutex_);
        if (Node::THREAD_CLASS && active_ && !cancel_flag_) {
            cancel_flag_ = true;
            Node* node = first_;
            while (node != NULL) {
                node->cancel();
                node = node->next;
            }
        }
        return *this;
    }

    /** Join all threads by waiting for them to stop.
     \return  Always true
    */
    bool join() {
        typename MutexT::Lock lock(mutex_);
        if (active_) {
            Node* node = first_;
            while (node != NULL) {
                Node* next = node->next;
                node->cancel();
                node->thread.thread_join();
                delete node;
                node = next;
            }
            size_ = 0;
            first_ = last_ = NULL;
            cancel_flag_ = false;
            active_      = false;
        }
        return true;
    }

    SharedState shared_state;       ///< Shared state used by threads
    ThreadInit  thread_init;        ///< %Thread init values for function-based threads, not used for class-based threads

protected:
    typedef impl::ThreadGroupNode<S,T> Node;

    ulong size_;
    Node* first_;
    Node* last_;
    bool  active_;
    bool  cancel_flag_;
    mutable MutexT mutex_;

private:
    // Disable copying
    ThreadGroup(const This&);
    This& operator=(const This&);
};

///////////////////////////////////////////////////////////////////////////////

#if EVO_ATOMIC_SAFE_STATICS == 1 || defined(DOXYGEN)
/** Manages a singleton instance for given type.
 - This wraps a type and applies the singleton pattern with an instance of this type
 - This is thread safe and uses the Double-Checked Locking Pattern for speed, but this doesn't mean the wrapped class is thread safe
 - Note: This will only be available if the compiler supports thread-safe static initialization ("magic statics")
 .
 \tparam  T  Type to wrap
*/
template<class T>
struct Singleton {
    /** Get singleton instance.
     \return  Pointer to singleton instance, never NULL
    */
    static T* instance() {
        static Mutex mutex;
        static AtomicSmartPtr static_ptr;
        T* p = static_ptr.load(EVO_ATOMIC_CONSUME);
        if (p == NULL) {
            Mutex::Lock lock(mutex);
            p = static_ptr.load(EVO_ATOMIC_RELAXED);
            if (p == NULL) {
                p = new T;
                static_ptr.store(p, EVO_ATOMIC_RELEASE);
            }
        }
        return p;
    }

private:
    struct AtomicSmartPtr : public Atomic<T*> {
        AtomicSmartPtr()
            { }

        ~AtomicSmartPtr() {
            T* ptr = this->load();
            if (ptr != NULL)
                delete ptr;
        }
    };
};
#endif

///////////////////////////////////////////////////////////////////////////////
//@}
}
#if defined(_MSC_VER) && !defined(EVO_CPP11)
    #pragma warning(pop)
#endif
#endif
