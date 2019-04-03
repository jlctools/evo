// Evo C++ Library
/* Copyright 2019 Justin Crowell
Distributed under the BSD 2-Clause License -- see included file LICENSE.txt for details.
*/
///////////////////////////////////////////////////////////////////////////////
/** \file lock.h Evo smart locks for synchronization. */
#pragma once
#ifndef INCL_evo_lock_h
#define INCL_evo_lock_h

namespace evo {
/** \addtogroup EvoThread */
//@{

///////////////////////////////////////////////////////////////////////////////

/** Smart locking for synchronization.
 - This is associated with a locking object, and used to lock it
 - By default you lock with the constructor, and the destructor automatically unlocks (if locked)
 - This keeps track of whether it has locked the object, which protects against double lock or unlock here
 - \b Caution: Since this references a synchronization object, that object must be valid as long as this references it
 - \b Caution: Do not share an instance across multiple threads, this is not safe
 - See also: SmartLockInert
 .
 \tparam  T  Synchronization type to use -- ex: Mutex, Condition
*/
template<class T>
struct SmartLock {
    /** Constructor.
     \param  object  Synchronization object to use
     \param  lock    Whether to lock here
    */
    SmartLock(T& object, bool lock=true) : object_(object) {
        locked_ = false;
        if (lock) {
            object_.lock();
            locked_ = true;
        }
    }

    /** Destructor, unlocks if locked here. */
    ~SmartLock() {
        if (locked_)
            object_.unlock();
    }

    /** Lock object, if not already locked by this.
     \return  This
    */
    SmartLock& lock() {
        if (!locked_) {
            object_.lock();
            locked_ = true;
        }
        return *this;
    }

    /** Unlock object, if locked by this.
     \return  This
    */
    SmartLock& unlock() {
        if (locked_) {
            object_.unlock();
            locked_ = false;
        }
        return *this;
    }

    T& object_;     ///< Synchronization object to lock

protected:
    bool locked_;   ///< Whether object is locked by this
};

///////////////////////////////////////////////////////////////////////////////

/** Smart read-locking for synchronization.
 - This is associated with a read/write locking object, and used to lock it for reading
 - By default you lock with the constructor, and the destructor automatically unlocks (if locked)
 - This keeps track of whether it has locked the object, which protects against double lock or unlock here
 - \b Caution: Since this references a synchronization object, that object must be valid as long as this references it
 - \b Caution: Do not share an instance across multiple threads, this is not safe
 .
 \tparam  T  Read/Write synchronization type to use -- ex: MutexRW
*/
template<class T>
struct SmartLockRead {
    /** Constructor.
     \param  object  Synchronization object to use
     \param  lock    Whether to read-lock here
    */
    SmartLockRead(T& object, bool lock=true) : object_(object) {
        locked_ = false;
        if (lock) {
            object_.lock_read();
            locked_ = true;
        }
    }

    /** Destructor, unlocks if locked here. */
    ~SmartLockRead() {
        if (locked_)
            object_.unlock_read();
    }

    /** Read-Lock object, if not already locked by this.
     \return  This
    */
    SmartLockRead& lock() {
        if (!locked_) {
            object_.lock_read();
            locked_ = true;
        }
        return *this;
    }

    /** Read-Unlock object, if locked by this.
     \return  This
    */
    SmartLockRead& unlock() {
        if (locked_) {
            object_.unlock_read();
            locked_ = false;
        }
        return *this;
    }

    T& object_;     ///< Synchronization object to read-lock

protected:
    bool locked_;   ///< Whether object is read-locked by this
};

///////////////////////////////////////////////////////////////////////////////

/** Smart sleep-locking for synchronization.
 - This is associated with a read/write locking object suporting sleeplock() (usually SpinLock), and is used to lock it with a sleep while waiting
 - By default you lock with the constructor, and the destructor automatically unlocks (if locked)
 - This keeps track of whether it has locked the object, which protects against double lock or unlock here
 - \b Caution: Since this references a synchronization object, that object must be valid as long as this references it
 - \b Caution: Do not share an instance across multiple threads, this is not safe
 .
 \tparam  T  Sleep-locking synchronization type to use -- ex: SpinLock
*/
template<class T>
struct SmartSleepLock {
    /** Constructor.
     \param  object    Synchronization object to use
     \param  sleep_ms  Sleep time in milliseconds while waiting for lock, 0 to leave unlocked
    */
    SmartSleepLock(T& object, ulong sleep_ms=1) : object_(object) {
        locked_ = false;
        if (sleep_ms > 0) {
            object_.sleeplock(sleep_ms);
            locked_ = true;
        }
    }

    /** Destructor, unlocks if locked here. */
    ~SmartSleepLock() {
        if (locked_)
            object_.unlock();
    }

    /** Lock object, if not already locked by this.
     \param   sleep_ms  Sleep time in milliseconds while waiting for lock, 0 to leave unlocked
     \return  This
    */
    SmartSleepLock& lock(ulong sleep_ms=1) {
        if (!locked_ && sleep_ms > 0) {
            object_.sleeplock(sleep_ms);
            locked_ = true;
        }
        return *this;
    }

    /** Unlock object, if locked by this.
     \return  This
    */
    SmartSleepLock& unlock() {
        if (locked_) {
            object_.unlock();
            locked_ = false;
        }
        return *this;
    }

    T& object_;     ///< Synchronization object to read-lock

protected:
    bool locked_;   ///< Whether object is read-locked by this
};

///////////////////////////////////////////////////////////////////////////////
}
//@}
#endif
