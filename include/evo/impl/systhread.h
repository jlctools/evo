// Evo C++ Library
/* Copyright 2018 Justin Crowell
Distributed under the BSD 2-Clause License -- see included file LICENSE.txt for details.
*/
///////////////////////////////////////////////////////////////////////////////
/** \file systhread.h Evo system threads implementation. */
#pragma once
#ifndef INCL_evo_systhread_h
#define INCL_evo_systhread_h

#include "sys.h"

#if !defined(_WIN32)
    // Linux/Unix
    #include <pthread.h>
    #if defined(__linux) && !defined(__CYGWIN__)
        #include <sys/syscall.h>
        #include <linux/version.h>
		#if defined(LINUX_VERSION_CODE) && LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)
			#define EVO_LINUX_NPTL 1 // Linux 2.6+ uses native pthreads (NPTL)
		#endif
    #endif
#endif

///////////////////////////////////////////////////////////////////////////////

// Thread Helpers
#if defined(_WIN32)
    #define EVO_THREAD_RUN_DEFINE(NAME,PARAM_NAME) DWORD WINAPI NAME(LPVOID PARAM_NAME)
    #define EVO_THREAD_RUN_RETURN return 0
#else
    /** Used to define a thread run function used with SysThread (used internally).
     - The actual signature depends on the OS, though param is always void*
     \param  NAME        Function name
     \param  PARAM_NAME  Paremter name (always void*)
    */
    #define EVO_THREAD_RUN_DEFINE(NAME,PARAM_NAME) void* NAME(void* PARAM_NAME)

    /** Used to return from a thread run function used with SysThread (used internally).
     - The actual return type depends on the OS, use this for best portability
    */
    #define EVO_THREAD_RUN_RETURN return NULL
#endif

namespace evo {
/** \addtogroup EvoThread */
//@{

///////////////////////////////////////////////////////////////////////////////

struct SysThread {
#if defined(_WIN32)
    // Windows
    typedef DWORD (WINAPI *RunFunc)(LPVOID);
    typedef HANDLE Handle;

    SysThread()
        { handle = NULL; }

    Error start(RunFunc run_func, void* run_ptr) {
        handle = CreateThread(NULL, 0, run_func, run_ptr, 0, NULL);
        if (handle != NULL)
            return ENone;
        return EUnknown;
    }

    void detach() {
        if (handle != NULL) {
            CloseHandle(handle);
            handle = NULL;
        }
    }

    bool join() {
        if (handle != NULL) {
            const bool done = (WaitForSingleObject(handle, INFINITE) == WAIT_OBJECT_0);
            CloseHandle(handle);
            handle = NULL;
            return done;
        }
        return false;
    }

    static void yield()
        { YieldProcessor(); }

    static ulong id()
        { return (ulong)GetCurrentThreadId(); }

#else
    // Linux/Unix
    typedef void* (*RunFunc)(void*);
    typedef pthread_t Handle;

    SysThread() {
        attached = false;
        memset(&handle, 0, sizeof(Handle));
    }

    Error start(RunFunc run_func, void* run_ptr) {
        detach();
        if (pthread_create(&handle, NULL, run_func, run_ptr) == 0) {
            attached = true;
            return ENone;
        }
        return EUnknown;
    }

    void detach() {
        if (attached) {
            pthread_detach(handle);
            memset(&handle, 0, sizeof(Handle));
            attached = false;
        }
    }

    bool join() {
        if (attached && pthread_join(handle, NULL) == 0) {
            attached = false;
            return true;
        }
        return false;
    }

    static void yield() {
        #if defined(__APPLE__)
            pthread_yield_np();
		#elif defined(__CYGWIN__)
			__gthread_yield();
		#else
            pthread_yield();
        #endif
    }

    static ulong id() {
        #if defined(SYS_gettid) && defined(EVO_LINUX_NPTL)
            // Linux 2.6+ uses native pthreads (NPTL) so each pthread maps 1:1 to a kernel thread, and kernel TIDs are smaller numbers
            return (ulong)syscall(SYS_gettid);
        #else
            // Converting pthread_t to ulong works on tested systems, though technically isn't portable
            return (ulong)pthread_self();
        #endif
    }

    bool attached;

#endif
    ~SysThread()
        { detach(); }

    Handle handle;

private:
    // Disable copying
    SysThread(const SysThread&);
    SysThread& operator=(const SysThread&);
};

///////////////////////////////////////////////////////////////////////////////

struct SysMutex {
#if defined(_WIN32)
    // Windows
    typedef CRITICAL_SECTION Handle;

    SysMutex()
        { InitializeCriticalSection(&handle); }

    ~SysMutex()
        { DeleteCriticalSection(&handle); }

    bool trylock()
        { return (TryEnterCriticalSection(&handle) != 0); }

    void lock()
        { EnterCriticalSection(&handle); }

    void unlock()
        { LeaveCriticalSection(&handle); }

#else
    // Linux/Unix
    typedef pthread_mutex_t Handle;     ///< Mutex handle type

    /** Constructor. */
    SysMutex()
        { pthread_mutex_init(&handle, NULL); }

    /** Destructor.
     - Must be unlocked, otherwise results are undefined
    */
    ~SysMutex()
        { pthread_mutex_destroy(&handle); }

    /** Try to lock mutex without blocking.
     - This allows polling for a lock without blocking
     - If current thread already has a lock, whether this succeeds is platform dependent -- some platforms (Windows) allow nested locks, others don't
     - \b Caution: Polling with this can starve (never lock) under load (constant locks)
     .
     \return  Whether successful, false if not available
    */
    bool trylock() {
        const int result = pthread_mutex_trylock(&handle);
        if (result == 0)
            return true;
        assert(result == EBUSY);
        return false;
    }

    /** Lock mutex.
     - Must call unlock() after each lock(), otherwise results are undefined
     - Results are undefined if already locked by current thread
    */
    void lock()
        { pthread_mutex_lock(&handle); }

    /** Unlock mutex.
     - Results are undefined if called while mutex not locked
    */
    void unlock()
        { pthread_mutex_unlock(&handle); }

#endif

    Handle handle;      ///< Mutex handle -- do not modify

private:
    // Disable copying
    SysMutex(const SysMutex&);
    SysMutex& operator=(const SysMutex&);
};

///////////////////////////////////////////////////////////////////////////////
}
//@}
#endif
