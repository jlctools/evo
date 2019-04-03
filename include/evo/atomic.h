// Evo C++ Library
/* Copyright 2019 Justin Crowell
Distributed under the BSD 2-Clause License -- see included file LICENSE.txt for details.
*/
///////////////////////////////////////////////////////////////////////////////
/** \file atomic.h Evo atomic types. */
#pragma once
#ifndef INCL_evo_atomic_h
#define INCL_evo_atomic_h

// Includes/Defines
#include "impl/sys.h"
#include "type.h"
#if defined(EVO_CPP11) || (defined(EVO_MSVC_YEAR) && EVO_MSVC_YEAR >= 2012) || defined(EVO_INTEL_VER)
    #include <atomic>
    #define EVO_INTRINSIC_ATOMICS 1		// 1 when compiler supports atomic operations
    #define EVO_STD_ATOMIC 1			// 1 when compiler supports std::atomic and std::atomic_flag
	#define EVO_ATOMIC_PTRMATH 1		// 1 when pointer math works correctly with AtomicPtr, i.e. increment uses sizeof()

    /** Relaxed memory ordering, used between start/end memory barriers. See \link evo::MemOrder MemOrder\endlink */
    #define EVO_ATOMIC_RELAXED std::memory_order_relaxed

    /** Start "consume" memory ordering barrier, usually followed by a matching "release" barrier. See \link evo::MemOrder MemOrder\endlink */
    #define EVO_ATOMIC_CONSUME std::memory_order_consume

    /** Start "acquire" memory ordering barrier, usually followed by a matching "release" barrier. See \link evo::MemOrder MemOrder\endlink */
    #define EVO_ATOMIC_ACQUIRE std::memory_order_acquire

    /** Release (end) memory ordering barrier started with "consume" or "acquire" barrier. See \link evo::MemOrder MemOrder\endlink */
    #define EVO_ATOMIC_RELEASE std::memory_order_release

    /** Combined "acquire" & "release" level memory barrier. See \link evo::MemOrder MemOrder\endlink */
    #define EVO_ATOMIC_ACQ_REL std::memory_order_acq_rel

    /** Full sync (sequentially consistent) memory barrier. See \link evo::MemOrder MemOrder\endlink */
    #define EVO_ATOMIC_SYNC    std::memory_order_seq_cst

    /** Sets a memory fence/barrier.
     - Memory ordering (fence) levels -- see: <a href="http://en.cppreference.com/w/cpp/atomic/memory_order">std::memory_order</a>
       and <a href="http://en.cppreference.com/w/cpp/language/memory_model">C++11 memory model</a>:
       - EVO_ATOMIC_RELAXED: Relaxed memory ordering, used between start/end memory barriers -- <a href="http://en.cppreference.com/w/cpp/atomic/memory_order#Relaxed_ordering">std::memory_order_relaxed</a>
       - EVO_ATOMIC_CONSUME: Start "consume" memory ordering barrier, usually followed by a matching "release" barrier -- <a href="http://en.cppreference.com/w/cpp/atomic/memory_order#Release-Consume_ordering">std::memory_order_consume</a>
       - EVO_ATOMIC_ACQUIRE: Start "acquire" memory ordering barrier, usually followed by a matching "release" barrier -- <a href="http://en.cppreference.com/w/cpp/atomic/memory_order#Release-Acquire_ordering">std::memory_order_acquire</a>
       - EVO_ATOMIC_RELEASE: Release (end) memory ordering barrier started with "consume" or "acquire" barrier -- std::memory_order_release
       - EVO_ATOMIC_ACQ_REL: Combined acquire/release fence -- std::memory_order_acq_rel
       - EVO_ATOMIC_SYNC: Full sync (sequentially consistent) memory barrier -- <a href="http://en.cppreference.com/w/cpp/atomic/memory_order#Sequentially-consistent_ordering">std::memory_order_seq_cst</a>
     - \b Caution: Exact behavior is CPU dependent, and compiler dependent before C++11 -- if in doubt it's safest to use a Mutex
     - See also: \link evo::MemOrder MemOrder\endlink
     .
     \param  MEM_ORDER  Memory ordering (fence) level
    */
    #define EVO_ATOMIC_FENCE(MEM_ORDER) std::atomic_thread_fence(MEM_ORDER)
#elif defined(EVO_GCC_VER) && EVO_GCC_VER >= 407
    // gcc 4.7+ has __atomic built-ins    
    #define EVO_INTRINSIC_ATOMICS 1
    #define EVO_GCC_ATOMICS 1

    #define EVO_ATOMIC_RELAXED __ATOMIC_RELAXED
    #define EVO_ATOMIC_CONSUME __ATOMIC_CONSUME
    #define EVO_ATOMIC_ACQUIRE __ATOMIC_ACQUIRE
    #define EVO_ATOMIC_RELEASE __ATOMIC_RELEASE
    #define EVO_ATOMIC_ACQ_REL __ATOMIC_ACQ_REL
    #define EVO_ATOMIC_SYNC    __ATOMIC_SEQ_CST

    #define EVO_ATOMIC_FENCE(MEM_ORDER) __atomic_thread_fence(MEM_ORDER)

    #define EVO_ATOMIC_FETCH_OP(OP, PTR, VAL, MEM_ORDER) __atomic_fetch_ ## OP (PTR, VAL, MEM_ORDER)
    #define EVO_ATOMIC_OP_FETCH(OP, PTR, VAL, MEM_ORDER) __atomic_ ## OP ## _fetch(PTR, VAL, MEM_ORDER)
#else
    // Legacy
    #define EVO_ATOMIC_RELAXED 0
    #define EVO_ATOMIC_CONSUME 0
    #define EVO_ATOMIC_ACQUIRE 0
    #define EVO_ATOMIC_RELEASE 0
    #define EVO_ATOMIC_ACQ_REL 0
    #define EVO_ATOMIC_SYNC    0

    #if defined(EVO_MSVC_YEAR) || (defined(EVO_GCC_VER) && EVO_GCC_VER < 401)
        // MSVC doesn't support atomics before 2012
        // gcc doesn't support atomics before 4.1
        #define EVO_INTRINSIC_ATOMICS 0
    #else    
        // Legacy __sync built-ins, supported by older gcc and clang, possibly others
        #define EVO_INTRINSIC_ATOMICS 1
        #define EVO_ATOMIC_FENCE(MEM_ORDER) __sync_synchronize()

        #define EVO_ATOMIC_FETCH_OP(OP, PTR, VAL, MEM_ORDER) __sync_fetch_and_ ## OP (PTR, VAL)
        #define EVO_ATOMIC_OP_FETCH(OP, PTR, VAL, MEM_ORDER) __sync_ ## OP ## _and_fetch(PTR, VAL)
    #endif
#endif
#if !EVO_INTRINSIC_ATOMICS
    // Emulate atomics with SysMutex
    #include "impl/systhread.h"
	#define EVO_ATOMIC_PTRMATH 1
#endif

// Check if static initialization is thread safe ("magic statics")
#if defined(EVO_ATOMIC_SAFE_STATICS)
    // Already defined, allow user to override
#else
    #if defined(EVO_MSVC_YEAR)
        #if EVO_MSVC_YEAR >= 2015            // No thread-safe static init until MSVC 2015
            #define EVO_ATOMIC_SAFE_STATICS 1
        #endif
    #elif defined(EVO_CPP11)                // C++11 requires thread-safe static init
        #define EVO_ATOMIC_SAFE_STATICS 1
    #elif defined(EVO_CLANG_VER)            // clang 2.9+
        #if EVO_CLANG_VER >= 209
            #define EVO_ATOMIC_SAFE_STATICS 1
        #endif
    #elif defined(EVO_GCC_VER)              // gcc 4.3+
        #if EVO_GCC_VER >= 403
            #define EVO_ATOMIC_SAFE_STATICS 1
        #endif
    #endif

    #if !defined(EVO_ATOMIC_SAFE_STATICS)
        /** Whether compiler supports thread-safe static initialization.
         - 1 means thread-safe static initialization is supported, 0 means not supported
         - If not defined already, Evo will attempt to detect and set this in Evo atomic.h or thread.h
        */
        #define EVO_ATOMIC_SAFE_STATICS 0
    #endif
#endif

// Disable certain MSVC warnings for this file, if !EVO_INTRINSIC_ATOMICS
#if defined(EVO_MSVC_YEAR) && !EVO_INTRINSIC_ATOMICS
    #pragma warning(push)
    #pragma warning(disable:4800 4100)
#endif

// Namespace: evo
namespace evo {

/** \addtogroup EvoThread */
//@{
///////////////////////////////////////////////////////////////////////////////

#if defined(EVO_STD_ATOMIC)
    /** %Atomic memory order (fence) type.
     - This is passed to an atomic function to specify memory ordering (fence) level
     - Values map to C++11 memory_order values -- see: <a href="http://en.cppreference.com/w/cpp/atomic/memory_order">std::memory_order</a>
       and <a href="http://en.cppreference.com/w/cpp/language/memory_model">C++11 memory model</a>:
       - EVO_ATOMIC_RELAXED: Relaxed memory ordering, used between start/end memory barriers -- <a href="http://en.cppreference.com/w/cpp/atomic/memory_order#Relaxed_ordering">std::memory_order_relaxed</a>
       - EVO_ATOMIC_CONSUME: Start "consume" memory ordering barrier, usually followed by a matching "release" barrier -- <a href="http://en.cppreference.com/w/cpp/atomic/memory_order#Release-Consume_ordering">std::memory_order_consume</a>
       - EVO_ATOMIC_ACQUIRE: Start "acquire" memory ordering barrier, usually followed by a matching "release" barrier -- <a href="http://en.cppreference.com/w/cpp/atomic/memory_order#Release-Acquire_ordering">std::memory_order_acquire</a>
       - EVO_ATOMIC_RELEASE: Release (end) memory ordering barrier started with "consume" or "acquire" barrier -- std::memory_order_release
       - EVO_ATOMIC_ACQ_REL: Combined acquire/release fence -- std::memory_order_acq_rel
       - EVO_ATOMIC_SYNC: Full sync (sequentially consistent) memory barrier -- <a href="http://en.cppreference.com/w/cpp/atomic/memory_order#Sequentially-consistent_ordering">std::memory_order_seq_cst</a>
     - \b Caution: Exact behavior is CPU dependent, and compiler dependent before C++11 -- if in doubt it's safest to use a Mutex
     - See also \link EVO_ATOMIC_FENCE()\endlink
    */
    typedef std::memory_order MemOrder;
#else
    typedef int MemOrder;
#endif

// Temp macro to emulate atomic op with mutex
#if !EVO_INTRINSIC_ATOMICS
    #define EVO_TMP_ATOMIC_OP(CODE) { T val; mutex_.lock(); CODE; mutex_.unlock(); return val; }
#endif

///////////////////////////////////////////////////////////////////////////////

/** %Atomic signalling flag.
 - All methods and operators are atomic (thread safe)
 - This can be used as a spin-lock, though better to use SpinLock for code clarity
 - This intentionally has a limited interface, use \link AtomicInt\endlink for a more standard atomic bool value
 .
*/
#if defined(EVO_STD_ATOMIC)
struct AtomicFlag : public std::atomic_flag {
    #if defined(EVO_CLANG_VER) || defined(EVO_MSVC_YEAR)
        AtomicFlag()  // some compilers don't support ATOMIC_FLAG_INIT init here 
            { clear(); }
    #else
        /** Constructor, initializes as clear (false). */
        AtomicFlag() : atomic_flag(ATOMIC_FLAG_INIT)
            { }
    #endif

    #if defined(DOXYGEN)
        /** Clear flag (set to false). */
        void clear(MemOrder mem_order=EVO_ATOMIC_SYNC);

        /** Atomically set flag (to true) and return the previous value.
         \return  Previous value
        */
        bool test_and_set(MemOrder mem_order=EVO_ATOMIC_SYNC);
    #endif
#else
struct AtomicFlag {
    AtomicFlag()
        { clear(); }

    void clear(int mem_order=EVO_ATOMIC_SYNC) {
        #if !EVO_INTRINSIC_ATOMICS
            mutex_.lock();
            val_ = 0;
            mutex_.unlock();
        #elif defined(EVO_GCC_ATOMICS)
            __atomic_clear(&val_, mem_order);
        #else
            val_ = 0;
            EVO_ATOMIC_FENCE(mem_order);
        #endif
    }

    bool test_and_set(int mem_order=EVO_ATOMIC_SYNC) {
        #if !EVO_INTRINSIC_ATOMICS
			typedef int T;
            EVO_TMP_ATOMIC_OP(val = val_; val_ = 1)
        #elif defined(EVO_GCC_ATOMICS)
            return __atomic_test_and_set(&val_, mem_order);
        #else
            return !__sync_bool_compare_and_swap(&val_, 0, 1);
        #endif
    }

private:
    #if defined(EVO_GCC_ATOMICS)
        bool val_;
    #else
        int val_;
    #endif

public:
#endif
    /** Spin-lock flag.
     - This works like a mutex and does a busy wait (using CPU) until flag is clear (false) and atomically sets the flag
     - Call unlock() to unlock (clear flag)
     - Results are undefined (likely deadlock) if unlock() isn't called after each lock() or sleeplock(), or if already locked by the same thread
    */
    void lock() {
        while (test_and_set(EVO_ATOMIC_ACQUIRE))
            { }
    }

    /** Spin-lock flag with a sleep.
     - This works like a mutex and does a busy wait (using CPU) with sleepms() until flag is clear (false) and atomically sets the flag
     - This is generally less wasteful than lock()
     - Call unlock() to unlock (clear flag)
     - Results are undefined (likely deadlock) if unlock() isn't called after each lock() or sleeplock(), or if already locked by the same thread
     .
     \param  ms  Sleep time in milliseconds while waiting for lock, must be positive
    */
    void sleeplock(ulong ms=1) {
        assert(ms > 0);
        while (test_and_set(EVO_ATOMIC_ACQUIRE))
            sleepms(ms);
    }

    /** Spin-unlock flag.
     - Call after lock() to unlock (clear flag)
    */
    void unlock()
        { clear(EVO_ATOMIC_RELEASE); }

private:
#if !EVO_INTRINSIC_ATOMICS
    mutable SysMutex mutex_;
#endif

    // Disable copying
    AtomicFlag(const AtomicFlag&);
    AtomicFlag& operator=(const AtomicFlag&);
};

///////////////////////////////////////////////////////////////////////////////

/** %Atomic integer type.
 - All methods and operators listed here are are atomic (thread safe)
   - 
 - Best to use these types for integers:
   - \link AtomicInt\endlink, \link AtomicLong\endlink, \link AtomicLongL\endlink
   - \link AtomicUInt\endlink, \link AtomicULong\endlink, \link AtomicULongL\endlink
   - \link AtomicInt32\endlink, \link AtomicInt64\endlink
   - \link AtomicUInt32\endlink, \link AtomicUInt64\endlink
 - This interface is compatible as a subset of C++ 11 std::atomic -- or may actually inherit from std::atomic (under MSVC or if C++11 enabled)
 .
 \tparam  T  Integer type -- must be a pointer or one of the supported integer types for best portability, though certain OS/compiler may support more

\par Example

\code
#include <evo/atomic.h>
using namespace evo;

int main() {
    // Defaults to 0
    AtomicInt num;

    // Use like a normal number
    num = 1;
    if (++num == 2)
        num += 2;

    // Or use methods
    num.store(5);

    // Beware: Separate operations are still separate without any synchronization (Mutex)
    if (num > 0)    // num can change right after comparison here
        num = 0;    // and before this sets it to 0

    return 0;
}
\endcode 
*/
#if defined(EVO_STD_ATOMIC)
template<class T>
class Atomic : public std::atomic<T>, public SafeBool<Atomic<T> > {
public:
    typedef Atomic<T> This;                 ///< This type

    /** Store new value.
     \param  num  Value to store
     \return      This
    */
    This& operator=(T num) {
        this->store(num);
        return *this;
    }

    /** %Compare and set, storing new value if comparison matches.
     \param  cmpval             Old value to compare with
     \param  newval             New value to set if equal to oldval
     \param  mem_order_success  Memory order constraints to enforce when comparison succeeds
     \param  mem_order_failure  Memory order constraints to enforce when comparison fails -- must not be: EVO_ATOMIC_RELEASE or EVO_ATOMIC_ACQ_REL
      - also must not be stronger ordering than mem_order_success (until C++17)
     \return                    Whether successful, false if comparison failed
    */
    bool compare_set(T cmpval, T newval, MemOrder mem_order_success=EVO_ATOMIC_SYNC, MemOrder mem_order_failure=EVO_ATOMIC_ACQUIRE)
        { T oldval = cmpval; return this->compare_exchange_strong(oldval, newval, mem_order_success, mem_order_failure); }

    #if defined(DOXYGEN)
        /** Load and return current value.
         \param  mem_order  Memory order constraints to enforce, must be EVO_ATOMIC_RELAXED, EVO_ATOMIC_CONSUME, EVO_ATOMIC_ACQUIRE, or EVO_ATOMIC_SYNC
         \return            Current value
        */
        T load(MemOrder mem_order=EVO_ATOMIC_SYNC) const;

        /** Exchange current value with new value.
         \param  num        New value to store
         \param  mem_order  Memory order constraints to enforce, except EVO_ATOMIC_CONSUME not supported here
         \return            Replaced value
        */
        T exchange(T num, MemOrder mem_order=EVO_ATOMIC_SYNC);

        /** Store new value.
         \param  num        Value to store
         \param  mem_order  Memory order constraints to enforce, must be EVO_ATOMIC_RELAXED, EVO_ATOMIC_RELEASE, or EVO_ATOMIC_SYNC
        */
        void store(T num, MemOrder mem_order=EVO_ATOMIC_SYNC);
    
        /** Add number to value and return the previous value.
         \param  num        Number to add
         \param  mem_order  Memory order constraints to enforce
         \return            Previous value before this
        */
        T fetch_add(T num, MemOrder mem_order=EVO_ATOMIC_SYNC);

        /** Subtract number to value and return the previous value.
         \param  num        Number to subtract
         \param  mem_order  Memory order constraints to enforce
         \return            Previous value before this
        */
        T fetch_sub(T num, MemOrder mem_order=EVO_ATOMIC_SYNC);

        /** Bitwise AND with value and return the previous value.
         \param  num        Number to AND with
         \param  mem_order  Memory order constraints to enforce
         \return            Previous value before this
        */
        T fetch_and(T num, MemOrder mem_order=EVO_ATOMIC_SYNC);

        /** Bitwise OR with value and return the previous value.
         \param  num        Number to OR with
         \param  mem_order  Memory order constraints to enforce
         \return            Previous value before this
        */
        T fetch_or(T num, MemOrder mem_order=EVO_ATOMIC_SYNC);

        /** Bitwise XOR with value and return the previous value.
         \param  num        Number toXOR with
         \param  mem_order  Memory order constraints to enforce
         \return            Previous value before this
        */
        T fetch_xor(T num, MemOrder mem_order=EVO_ATOMIC_SYNC);

        /** Increment value and return new value.
         \return  Copy of new value after increment
        */
        T operator++();

        /** Increment value and return previous value.
         \return  Copy of previous value before increment
        */
        T operator++(int);

        /** Decrement value and return new value.
         \return  Copy of new value after decrement
        */
        T operator--();

        /** Decrement value and return previous value.
         \return  Copy of previous value before decrement
        */
        T operator--(int);

        /** Increment value and return new value.
         \return  Copy of new value after increment
        */
        T operator+=(T num);

        /** Decrement value and return new value.
         \return  Copy of new value after decrement
        */
        T operator-=(T num);

        /** Bitwise AND value and return new value.
         \return  Copy of new value after AND
        */
        T operator&=(T num);

        /** Bitwise OR value and return new value.
         \return  Copy of new value after OR
        */
        T operator|=(T num);

        /** Bitwise XOR value and return new value.
         \return  Copy of new value after XOR
        */
        T operator^=(T num);
    #endif
#else
template<class T>
struct Atomic : public SafeBool<Atomic<T> > {
    typedef Atomic<T> This;     ///< This type

    T load(int mem_order=EVO_ATOMIC_SYNC) const {
        #if !EVO_INTRINSIC_ATOMICS
            EVO_TMP_ATOMIC_OP(val = val_)
        #elif defined(EVO_GCC_ATOMICS)
            return __atomic_load_n(&val_, mem_order);
        #else
            T val;
            __sync_synchronize();
            val = val_;
            __sync_synchronize();
            return val;
        #endif
    }

    T exchange(T num, int mem_order=EVO_ATOMIC_SYNC) {
        #if !EVO_INTRINSIC_ATOMICS
            EVO_TMP_ATOMIC_OP(val = val_; val_ = num)
        #elif defined(EVO_GCC_ATOMICS)
            return __atomic_exchange_n(&val_, num, mem_order);
        #else
            T prev_val;
            do {
                __sync_synchronize();
                prev_val = val_;
            } while (!__sync_bool_compare_and_swap(&val_, prev_val, num));
            return prev_val;
        #endif
    }

    void store(T num, int mem_order=EVO_ATOMIC_SYNC) {
        #if !EVO_INTRINSIC_ATOMICS
            mutex_.lock();
            val_ = num;
            mutex_.unlock();
        #elif defined(EVO_GCC_ATOMICS)
            __atomic_store_n(&val_, num, mem_order);
        #else
            val_ = num;
            __sync_synchronize();
        #endif
    }

    This& operator=(T num) {
        store(num);
        return *this;
    }

    bool compare_set(T cmpval, T newval, int mem_order_success=EVO_ATOMIC_SYNC, int mem_order_failure=EVO_ATOMIC_SYNC) {
        #if !EVO_INTRINSIC_ATOMICS
            bool replaced = false;
            mutex_.lock();
            if (val_ == cmpval) {
                val_ = newval;
                replaced = true;
            }
            mutex_.unlock();
            return replaced;
        #elif defined(EVO_GCC_ATOMICS)
            return __atomic_compare_exchange_n(&val_, &cmpval, newval, false, mem_order_success, mem_order_failure);
        #else
            return __sync_bool_compare_and_swap(&val_, cmpval, newval);
        #endif
    }

#if EVO_INTRINSIC_ATOMICS
    T fetch_add(T num, int mem_order=EVO_ATOMIC_SYNC)
        { return EVO_ATOMIC_FETCH_OP(add, &val_, num, mem_order); }

    T fetch_sub(T num, int mem_order=EVO_ATOMIC_SYNC)
        { return EVO_ATOMIC_FETCH_OP(sub, &val_, num, mem_order); }

    T fetch_and(T num, int mem_order=EVO_ATOMIC_SYNC)
        { return EVO_ATOMIC_FETCH_OP(and, &val_, num, mem_order); }

    T fetch_or(T num, int mem_order=EVO_ATOMIC_SYNC)
        { return EVO_ATOMIC_FETCH_OP(or, &val_, num, mem_order); }

    T fetch_xor(T num, int mem_order=EVO_ATOMIC_SYNC)
        { return EVO_ATOMIC_FETCH_OP(xor, &val_, num, mem_order); }

    T operator++()
        { return EVO_ATOMIC_OP_FETCH(add, &val_, (T)1, EVO_ATOMIC_SYNC); }

    T operator++(int)
        { return EVO_ATOMIC_FETCH_OP(add, &val_, (T)1, EVO_ATOMIC_SYNC); }

    T operator--()
        { return EVO_ATOMIC_OP_FETCH(sub, &val_, (T)1, EVO_ATOMIC_SYNC); }

    T operator--(int)
        { return EVO_ATOMIC_FETCH_OP(sub, &val_, (T)1, EVO_ATOMIC_SYNC); }

    T operator+=(T num)
        { return EVO_ATOMIC_OP_FETCH(add, &val_, num, EVO_ATOMIC_SYNC); }

    T operator-=(T num)
        { return EVO_ATOMIC_OP_FETCH(sub, &val_, num, EVO_ATOMIC_SYNC); }

    T operator&=(T num)
        { return EVO_ATOMIC_OP_FETCH(and, &val_, num, EVO_ATOMIC_SYNC); }

    T operator|=(T num)
        { return EVO_ATOMIC_OP_FETCH(or, &val_, num, EVO_ATOMIC_SYNC); }

    T operator^=(T num)
        { return EVO_ATOMIC_OP_FETCH(xor, &val_, num, EVO_ATOMIC_SYNC); }

#else
	// Emulate atomic type with mutex
	typedef typename StaticIf<IsPointer<T>::value,ulong,T>::Type NumType;

    T fetch_add(T num, int mem_order=EVO_ATOMIC_SYNC)
        { EVO_TMP_ATOMIC_OP(val = val_; val_ += (NumType)num) }

    T fetch_sub(T num, int mem_order=EVO_ATOMIC_SYNC)
        { EVO_TMP_ATOMIC_OP(val = val_; val_ -= (NumType)num) }

    T fetch_and(T num, int mem_order=EVO_ATOMIC_SYNC)
        { EVO_TMP_ATOMIC_OP(val = val_; val_ &= (NumType)num) }

    T fetch_or(T num, int mem_order=EVO_ATOMIC_SYNC)
        { EVO_TMP_ATOMIC_OP(val = val_; val_ |= (NumType)num) }

    T fetch_xor(T num, int mem_order=EVO_ATOMIC_SYNC)
        { EVO_TMP_ATOMIC_OP(val = val_; val_ ^= (NumType)num) }

    T operator++()
        { EVO_TMP_ATOMIC_OP(val = ++val_) }

    T operator++(int)
        { EVO_TMP_ATOMIC_OP(val = val_; ++val_) }

    T operator--()
        { EVO_TMP_ATOMIC_OP(val = --val_) }

    T operator--(int)
        { EVO_TMP_ATOMIC_OP(val = val_; --val_) }

    T operator+=(T num)
        { EVO_TMP_ATOMIC_OP(val = val_ += (NumType)num) }

    T operator-=(T num)
        { EVO_TMP_ATOMIC_OP(val = val_ -= (NumType)num) }

    T operator&=(T num)
        { EVO_TMP_ATOMIC_OP(val = val_ &= (NumType)num) }

    T operator|=(T num)
        { EVO_TMP_ATOMIC_OP(val = val_ |= (NumType)num) }

    T operator^=(T num)
        { EVO_TMP_ATOMIC_OP(val = val_ ^= (NumType)num) }
#endif

protected:
    T val_;     ///< Stored value
#endif
    
public:
    /** Constructor, initializes with 0. */
    Atomic()
        { this->store(0); }

    /** Constructor.
     \param  num  Number value to initialize with
    */
    Atomic(T num)
        { this->store(num); }

    /** Implicit conversion to integer type.
     - Works the same as load()
     \return  Current value
    */
    operator T() const
        { return this->load(); }

    /** Member access operator (const).
     - Only use where T is apointer
     - Results are undefined if pointer is NULL
     .
     \return  Pointer
    */
    const T operator->() const
        { return this->load(); }

    /** Member access operator.
     - Only use where T is apointer
     - Results are undefined if pointer is NULL
     .
     \return  Pointer
    */
    T operator->()
        { return this->load(); }

    /** Negation operator checks if NULL/false (0).
     \return  Whether NULL or false (0)
    */
    bool operator!() const
        { return !this->load(); }

    /** %Compare whether current value is equal to number.
     \param  num  Number to compare with
     \return      Whether equal
    */
    bool operator==(const This& num) const
        { return (this == &num || this->load() == num.load()); }

    /** %Compare whether current value is equal to number.
     \param  num  Number to compare with
     \return      Whether equal
    */
    bool operator==(T num) const
        { return this->load() == num; }

    /** %Compare whether current value is not equal to number.
     \param  num  Number to compare with
     \return      Whether not equal
    */
    bool operator!=(const This& num) const
        { return (this != &num && this->load() != num.load()); }

    /** %Compare whether current value is not equal to number.
     \param  num  Number to compare with
     \return      Whether not equal
    */
    bool operator!=(T num) const
        { return this->load() != num; }

    /** %Compare whether current value is less than number.
     \param  num  Number to compare with
     \return      Whether less
    */
    bool operator<(T num) const
        { return this->load() < num; }

    /** %Compare whether current value is less than or equal to number.
     \param  num  Number to compare with
     \return      Whether less/equal
    */
    bool operator<=(T num) const
        { return this->load() <= num; }

    /** %Compare whether current value is greater than number.
     \param  num  Number to compare with
     \return      Whether greater
    */
    bool operator>(T num) const
        { return this->load() > num; }

    /** %Compare whether current value is greater than or equal to number.
     \param  num  Number to compare with
     \return      Whether greater/equal
    */
    bool operator>=(T num) const
        { return this->load() >= num; }

private:
#if !EVO_INTRINSIC_ATOMICS
    mutable SysMutex mutex_;
#endif

    // Disable copying
    Atomic(const This&);
    This& operator=(const This&);
};

///////////////////////////////////////////////////////////////////////////////

typedef Atomic<int>    AtomicInt;       ///< Atomic signed int
typedef Atomic<uint>   AtomicUInt;      ///< Atomic unsigned int
typedef Atomic<long>   AtomicLong;      ///< Atomic signed long
typedef Atomic<ulong>  AtomicULong;     ///< Atomic unsigned long
typedef Atomic<longl>  AtomicLongL;     ///< Atomic signed long long
typedef Atomic<ulongl> AtomicULongL;    ///< Atomic unsigned long long

typedef Atomic<int32>  AtomicInt32;     ///< Atomic 32-bit signed int
typedef Atomic<uint32> AtomicUInt32;    ///< Atomic 32-bit unsigned int
typedef Atomic<int64>  AtomicInt64;     ///< Atomic 64-bit signed int
typedef Atomic<uint64> AtomicUInt64;    ///< Atomic 64-bit unsigned int

///////////////////////////////////////////////////////////////////////////////

// Undef temp macro
#if defined(EVO_TMP_ATOMIC_OP)
    #undef EVO_TMP_ATOMIC_OP
#endif

#if defined(_MSC_VER) && !EVO_INTRINSIC_ATOMICS
    #pragma warning(pop)
#endif

///////////////////////////////////////////////////////////////////////////////

/** %Atomic dumb pointer to single object.
 - This is an atomic version of Ptr with increment/decrement operator support
 - All operations are atomic
   - \b Caution: This does not make the pointed type thread safe!
   - \b Caution: T should be atomic or thread safe, unless you're sure only one thread will use an item at a time
 - Specialized for array pointer: AtomicPtr<T[]>
 .
 \tparam  T  Type to use pointer to (not the raw pointer type)
*/
template<class T>
class AtomicPtr : public PtrBase<T, Atomic<T*> > {
protected:
    using PtrBase<T, Atomic<T*> >::ptr_;

public:
    typedef AtomicPtr<T>            This;       ///< This type
    typedef PtrBase<T, Atomic<T*> > Base;       ///< Base type
    typedef PtrBase<T>              PtrBaseT;   ///< Managed pointer base type

    /** Default constructor sets as NULL. */
    AtomicPtr()
        { }

    /** Constructor.
     \param  ptr  Pointer to set, NULL for none
    */
    AtomicPtr(T* ptr)
        { ptr_.store(ptr); }

    /** Constructor.
     \param  ptr  Pointer to set, NULL for none
    */
    AtomicPtr(const T* ptr)
        { ptr_.store((T*)ptr); }

    /** Copy constructor.
     \param  src  Source pointer
    */
    AtomicPtr(const This& src)
        { ptr_.store(src.ptr_.load()); }

    /** Copy constructor to reference pointer.
     \param  src  Source pointer
    */
    AtomicPtr(const Base& src)
        { ptr_.store(src.ptr_); }

    /** Copy constructor to reference pointer.
     \param  src  Source pointer
    */
    AtomicPtr(const PtrBaseT& src)
        { ptr_.store(src.ptr_); }

    /** Copy/Assignment operator.
     \param  src  Source pointer
     \return      This
    */
    This& operator=(const This& src)
        { ptr_.store(src.ptr_.load()); return *this; }

    /** Assignment operator to reference pointer.
     \param  src  Source pointer
     \return      This
    */
    This& operator=(const Base& src)
        { ptr_.store(src.ptr_); return *this; }

    /** Assignment operator to reference pointer.
     \param  src  Source pointer
     \return      This
    */
    This& operator=(const PtrBaseT& src)
        { ptr_.store(src.ptr_); return *this; }

    /** Assignment operator for raw pointer.
     \param  ptr  Pointer to set
     \return      This
    */
    This& operator=(T* ptr)
        { ptr_.store(ptr); return *this; }

    /** Assignment operator for raw pointer.
     \param  ptr  Pointer to set
     \return      This
    */
    This& operator=(const T* ptr)
        { ptr_.store((T*)ptr); return *this; }

    /** Clear pointer, setting as null.
     \return  This
    */
    This& clear()
        { ptr_.store(NULL); return *this; }

    /** Detach and return pointer.
     - This returns current pointer and sets to null
     .
     \return  Pointer
    */
    T* detach()
        { return ptr_.exchange(NULL); }

    /** Prefix increment operator.
     \return  Pointer after increment
    */
    T* operator++() {
    #if defined(EVO_ATOMIC_PTRMATH)
        return ++ptr_;
    #else
        return ptr_ += (T*)sizeof(T);
    #endif
    }

    /** Prefix increment operator.
     \return  Pointer before increment
    */
    T* operator++(int) {
    #if defined(EVO_ATOMIC_PTRMATH)
        return ptr_++;
    #else
        return ptr_.fetch_add((T*)sizeof(T));
    #endif
    }

    /** Increment by count operator.
     \param  count  Count to increment
     \return        This
    */
    This& operator+=(int count) {
    #if defined(EVO_STD_ATOMIC)
        ptr_ += count;
	#elif defined(EVO_ATOMIC_PTRMATH)
		ptr_ += (T*)count;
    #else
        ptr_ += (T*)(count * sizeof(T));
    #endif
        return *this;
    }

    /** Prefix decrement operator.
     \return  Pointer after decrement
    */
    T* operator--() {
    #if defined(EVO_ATOMIC_PTRMATH)
        return --ptr_;
    #else
        return ptr_ -= (T*)sizeof(T);
    #endif
    }

    /** Postfix decrement operator.
     \return  Pointer before decrement
    */
    T* operator--(int) {
    #if defined(EVO_ATOMIC_PTRMATH)
        return ptr_--;
    #else
        return ptr_.fetch_sub((T*)sizeof(T));
    #endif
    }

    /** Decrement by count operator.
     \param  count  Count to decrement
     \return        This
    */
    This& operator-=(int count) {
    #if defined(EVO_STD_ATOMIC)
        ptr_ -= count;
	#elif defined(EVO_ATOMIC_PTRMATH)
		ptr_ -= (T*)count;
    #else
        ptr_ -= (T*)(count * sizeof(T));
    #endif
        return *this;
    }

    using Base::operator==;
    using Base::operator!=;
    using Base::operator<;
    using Base::operator<=;
    using Base::operator>;
    using Base::operator>=;

    /** Equality operator.
     \param  ptr  Pointer to compare to
     \return      Whether equal
    */
    bool operator==(const PtrBaseT& ptr) const
        { return ptr_.load() == ptr.ptr_; }

    /** Inequality operator.
     \param  ptr  Pointer to compare to
     \return      Whether not equal
    */
    bool operator!=(const PtrBaseT& ptr) const
        { return ptr_.load() != ptr.ptr_; }

    /** Less-than operator.
     \param  ptr  Pointer to compare to
     \return      Whether less than ptr
    */
    bool operator<(const PtrBaseT& ptr) const
        { return ptr_.load() < ptr.ptr_; }

    /** Less-than-or-equals operator.
     \param  ptr  Pointer to compare to
     \return      Whether less than or equal to ptr
    */
    bool operator<=(const PtrBaseT& ptr) const
        { return ptr_.load() <= ptr.ptr_; }

    /** Greater-than operator.
     \param  ptr  Pointer to compare to
     \return      Whether greater than ptr
    */
    bool operator>(const PtrBaseT& ptr) const
        { return ptr_.load() > ptr.ptr_; }

    /** Greater-than-or-equals operator.
     \param  ptr  Pointer to compare to
     \return      Whether greater than or equal to ptr
    */
    bool operator>=(const PtrBaseT& ptr) const
        { return ptr_.load() >= ptr.ptr_; }
};

/** %Atomic dumb pointer to array.
 - This is an atomic version of Ptr<T[]> with increment/decrement operator support
 - All operations are atomic
   - \b Caution: This does not make the pointed type thread safe!
   - \b Caution: T should be atomic or thread safe, unless you're sure only one thread will use an item at a time
 - For single object pointer: AtomicPtr
 .
 \tparam  T  Type to use pointer to (not the raw pointer type)
*/
template<class T>
class AtomicPtr<T[]> : public PtrBase<T, Atomic<T*> > {
protected:
    using PtrBase<T, Atomic<T*> >::ptr_;

public:
    typedef AtomicPtr<T[]>          This;       ///< This type
    typedef PtrBase<T, Atomic<T*> > Base;       ///< Base type
    typedef PtrBase<T>              PtrBaseT;   ///< Managed pointer base type

    /** Default constructor sets as NULL. */
    AtomicPtr()
        { }

    /** Constructor.
     \param  ptr  Pointer to set, NULL for none
    */
    AtomicPtr(T* ptr)
        { ptr_.store(ptr); }

    /** Constructor.
     \param  ptr  Pointer to set, NULL for none
    */
    AtomicPtr(const T* ptr)
        { ptr_.store((T*)ptr); }

    /** Copy constructor.
     \param  src  Source pointer
    */
    AtomicPtr(const This& src)
        { ptr_.store(src.ptr_.load()); }

    /** Copy constructor to reference pointer.
     \param  src  Source pointer
    */
    AtomicPtr(const Base& src)
        { ptr_.store(src.ptr_); }

    /** Copy constructor to reference pointer.
     \param  src  Source pointer
    */
    AtomicPtr(const PtrBaseT& src)
        { ptr_.store(src.ptr_); }

    /** Copy/Assignment operator.
     \param  src  Source pointer
     \return      This
    */
    This& operator=(const This& src)
        { ptr_.store(src.ptr_.load()); return *this; }

    /** Assignment operator to reference pointer.
     \param  src  Source pointer
     \return      This
    */
    This& operator=(const Base& src)
        { ptr_.store(src.ptr_); return *this; }

    /** Assignment operator to reference pointer.
     \param  src  Source pointer
     \return      This
    */
    This& operator=(const PtrBaseT& src)
        { ptr_.store(src.ptr_); return *this; }

    /** Assignment operator for raw pointer.
     \param  ptr  Pointer to set
     \return      This
    */
    This& operator=(T* ptr)
        { ptr_.store(ptr); return *this; }

    /** Assignment operator for raw pointer.
     \param  ptr  Pointer to set
     \return      This
    */
    This& operator=(const T* ptr)
        { ptr_.store((T*)ptr); return *this; }

    /** Clear pointer, setting as null.
     \return  This
    */
    This& clear()
        { ptr_.store(NULL); return *this; }

    /** Detach and return pointer.
     - This returns current pointer and sets to null
     .
     \return  Pointer
    */
    T* detach()
        { return ptr_.exchange(NULL); }

    /** Prefix increment operator.
     \return  Pointer after increment
    */
    T* operator++() {
    #if defined(EVO_ATOMIC_PTRMATH)
        return ++ptr_;
    #else
        return ptr_ += (T*)sizeof(T);
    #endif
    }

    /** Prefix increment operator.
     \return  Pointer before increment
    */
    T* operator++(int) {
    #if defined(EVO_ATOMIC_PTRMATH)
        return ptr_++;
    #else
        return ptr_.fetch_add((T*)sizeof(T));
    #endif
    }

    /** Increment by count operator.
     \param  count  Count to increment
     \return        This
    */
    This& operator+=(int count) {
    #if defined(EVO_STD_ATOMIC)
        ptr_ += count;
	#elif defined(EVO_ATOMIC_PTRMATH)
		ptr_ += (T*)count;
    #else
        ptr_ += (T*)(count * sizeof(T));
    #endif
        return *this;
    }

    /** Prefix decrement operator.
     \return  Pointer after decrement
    */
    T* operator--() {
    #if defined(EVO_ATOMIC_PTRMATH)
        return --ptr_;
    #else
        return ptr_ -= (T*)sizeof(T);
    #endif
    }

    /** Postfix decrement operator.
     \return  Pointer before decrement
    */
    T* operator--(int) {
    #if defined(EVO_ATOMIC_PTRMATH)
        return ptr_--;
    #else
        return ptr_.fetch_sub((T*)sizeof(T));
    #endif
    }

    /** Decrement by count operator.
     \param  count  Count to decrement
     \return        This
    */
    This& operator-=(int count) {
    #if defined(EVO_STD_ATOMIC)
        ptr_ -= count;
	#elif defined(EVO_ATOMIC_PTRMATH)
		ptr_ -= (T*)count;
    #else
        ptr_ -= (T*)(count * sizeof(T));
    #endif
        return *this;
    }

    using Base::operator==;
    using Base::operator!=;
    using Base::operator<;
    using Base::operator<=;
    using Base::operator>;
    using Base::operator>=;

    /** Equality operator.
     \param  ptr  Pointer to compare to
     \return      Whether equal
    */
    bool operator==(const PtrBaseT& ptr) const
        { return ptr_.load() == ptr.ptr_; }

    /** Inequality operator.
     \param  ptr  Pointer to compare to
     \return      Whether not equal
    */
    bool operator!=(const PtrBaseT& ptr) const
        { return ptr_.load() != ptr.ptr_; }

    /** Less-than operator.
     \param  ptr  Pointer to compare to
     \return      Whether less than ptr
    */
    bool operator<(const PtrBaseT& ptr) const
        { return ptr_.load() < ptr.ptr_; }

    /** Less-than-or-equals operator.
     \param  ptr  Pointer to compare to
     \return      Whether less than or equal to ptr
    */
    bool operator<=(const PtrBaseT& ptr) const
        { return ptr_.load() <= ptr.ptr_; }

    /** Greater-than operator.
     \param  ptr  Pointer to compare to
     \return      Whether greater than ptr
    */
    bool operator>(const PtrBaseT& ptr) const
        { return ptr_.load() > ptr.ptr_; }

    /** Greater-than-or-equals operator.
     \param  ptr  Pointer to compare to
     \return      Whether greater than or equal to ptr
    */
    bool operator>=(const PtrBaseT& ptr) const
        { return ptr_.load() >= ptr.ptr_; }
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/** Equality operator for managed pointer base and atomic pointer base.
 \param  ptr1  Pointer to compare
 \param  ptr2  Pointer to compare to
 \return       Whether equal
*/
template<class T>
inline bool operator==(const PtrBase<T>& ptr1, const PtrBase<T, Atomic<T*> >& ptr2)
    { return ptr1.ptr_ == ptr2.ptr_.load(); }

/** Inequality operator for managed pointer base and atomic pointer base.
 \param  ptr1  Pointer to compare
 \param  ptr2  Pointer to compare to
 \return       Whether not equal
*/
template<class T>
inline bool operator!=(const PtrBase<T>& ptr1, const PtrBase<T, Atomic<T*> >& ptr2)
    { return ptr1.ptr_ != ptr2.ptr_.load(); }

/** Less-than operator for managed pointer base and atomic pointer base.
 \param  ptr1  Pointer to compare
 \param  ptr2  Pointer to compare to
 \return       Whether less than ptr
*/
template<class T>
inline bool operator<(const PtrBase<T>& ptr1, const PtrBase<T, Atomic<T*> >& ptr2)
    { return ptr1.ptr_ < ptr2.ptr_.load(); }

/** Less-than-or-equals operator for managed pointer base and atomic pointer base.
 \param  ptr1  Pointer to compare
 \param  ptr2  Pointer to compare to
 \return       Whether less than or equal to ptr
*/
template<class T>
inline bool operator<=(const PtrBase<T>& ptr1, const PtrBase<T, Atomic<T*> >& ptr2)
    { return ptr1.ptr_ <= ptr2.ptr_.load(); }

/** Greater-than operator for managed pointer base and atomic pointer base.
 \param  ptr1  Pointer to compare
 \param  ptr2  Pointer to compare to
 \return       Whether greater than ptr
*/
template<class T>
inline bool operator>(const PtrBase<T>& ptr1, const PtrBase<T, Atomic<T*> >& ptr2)
    { return ptr1.ptr_ > ptr2.ptr_.load(); }

/** Greater-than-or-equals operator for managed pointer base and atomic pointer base.
 \param  ptr1  Pointer to compare
 \param  ptr2  Pointer to compare to
 \return       Whether greater than or equal to ptr
*/
template<class T>
inline bool operator>=(const PtrBase<T>& ptr1, const PtrBase<T, Atomic<T*> >& ptr2)
    { return ptr1.ptr_ >= ptr2.ptr_.load(); }

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
} // Namespace: evo
//@}
#endif
