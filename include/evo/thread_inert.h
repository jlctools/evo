// Evo C++ Library
/* Copyright 2019 Justin Crowell
Distributed under the BSD 2-Clause License -- see included file LICENSE.txt for details.
*/
///////////////////////////////////////////////////////////////////////////////
/** \file thread_inert.h Evo inert synchronization implementation. */
#pragma once
#ifndef INCL_evo_thread_inert_h
#define INCL_evo_thread_inert_h

#include "impl/sys.h"

namespace evo {
/** \addtogroup EvoThread */
//@{

///////////////////////////////////////////////////////////////////////////////

/** Inert lock that doesn't do anything.
 - This is used with MutexInert and implements all the lock interfaces (SmartLock, SmartLockRead, SmartSleepLock) with no-ops
 - All methods are no-ops
 - See MutexInert
 .
 \tparam  T  Locking synchronization type to use -- ex: MutexInert
*/
template<class T>
struct SmartLockInert {
    /** Constructor -- arg is ignored. */
    SmartLockInert(T&)
        { }

    /** Constructor -- args are ignored. */
    SmartLockInert(T&, bool)
        { }

    /** Constructor -- args are ignored. */
    SmartLockInert(T&, ulong)
        { }

    /** Lock object (no-op).
     \return  This
    */
    SmartLockInert& lock()
        { return *this; }

    /** Lock object (no-op).
     - Arg is ignored
     \return  This
    */
    SmartLockInert& lock(ulong)
        { return *this; }

    /** Unlock object (no-op).
     \return  This
    */
    SmartLockInert& unlock()
        { return *this; }
};

///////////////////////////////////////////////////////////////////////////////

/** Inert mutex used to disable thread synchronization.
 - Replace a mutex type with this to disable thread synchronization
 - This is useful with mutex type template parameters (policy based design)
 - Not thread safe -- All methods are no-ops
*/
struct MutexInert {
    typedef SmartLockInert<MutexInert> Lock;            ///< Lock object type (inert) -- see SmartLockInert
    typedef SmartLockInert<MutexInert> LockWrite;       ///< Write %Lock object type (inert) -- see SmartLockInert
    typedef SmartLockInert<MutexInert> LockRead;        ///< Read %Lock object type (inert) -- see SmartLockInert
    typedef SmartLockInert<MutexInert> SleepLock;       ///< Sleep-Lock object type (inert) -- see SmartLockInert

    /** Try to lock (no-op).
     \return  Always succeeds (returns true)
    */
    bool trylock()
        { return true; }

    /** %Lock object (no-op). */
    void lock()
        { }
    
    /** %Lock object (no-op).
     - Arg is ignored
    */
    void sleeplock(ulong)
        { }

    /** Unlock object (no-op). */
    void unlock()
        { }

    /** Try to read-lock (no-op).
     \return  Always succeeds (returns true)
    */
    bool trylock_read()
        { return true; }

    /** Read-lock object (no-op). */
    void lock_read()
        { }

    /** Un-read-lock object (no-op). */
    void unlock_read()
        { }
};

///////////////////////////////////////////////////////////////////////////////
}
//@}
#endif
