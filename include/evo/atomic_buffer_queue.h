// Evo C++ Library
/* Copyright 2019 Justin Crowell
Distributed under the BSD 2-Clause License -- see included file LICENSE.txt for details.
*/
///////////////////////////////////////////////////////////////////////////////
/** \file atomic_buffer_queue.h Evo AtomicBufferQueue. */
#pragma once
#ifndef INCL_evo_atomic_buffer_queue_h
#define INCL_evo_atomic_buffer_queue_h

#include "atomic.h"

namespace evo {
/** \addtogroup EvoContainers */
//@{

///////////////////////////////////////////////////////////////////////////////

/** Fast buffer-based queue, implemented with a ring-buffer.
 \tparam  T      Item type to use, copied with assignment operator
 \tparam  TSize  Size type to use for queue size (must be unsigned integer) -- default: SizeT

\par Features

 - This is lock free and thread safe, though pop() may only be called by 1 consumer thread
 - Efficient buffer based queue, especially for simple (POD or Byte-Copy) types
 - This allocates a fixed size buffer and is not resizable, size is always a power of 2
   - For a dynamic size queue (or stack) see: List
 - Adding items does not allocate memory (though this calls the item assignment operator, which could)
 - Popped (removed or cleared) items are left as-is in buffer, to be overwritten as new items are added
 - All operations take constant time, except copy constructor and assignment operator which often take linear time (to copy items)
 .
 
Note that this is not a full EvoContainer and doesn't have iterators.

\par Example

\code
#include <evo/atomic_buffer_queue.h>
using namespace evo;

int main() {
    AtomicBufferQueue<int> queue(20); // rounds to 32 (power of 2)

    queue.add(1);
    queue.add(2);
    queue.add(3);

    int a = 0, b = 0, c = 0;
    queue.pop(a);   // set to 1
    queue.pop(b);   // set to 2
    queue.pop(c);   // set to 3

    return 0;
}
\endcode
*/
template<class T, class TSize=SizeT>
class AtomicBufferQueue {
public:
    typedef AtomicBufferQueue<T,TSize> This;    ///< %This type
    typedef TSize Size;                         ///< Queue size integer type (always unsigned)
    typedef T     Item;                         ///< Item type

    static const Size DEFAULT_SIZE = 128;       ///< Default size to use

    /** Constructor, sets buffer size.
     \param  size  Buffer size to use as item count, rouned to next power of 2 if needed
    */
    AtomicBufferQueue(Size size=DEFAULT_SIZE) {
        size = size_pow2(size);
        buf_       = new Item[size];
        size_      = size;
        size_mask_ = size - 1;
        next_pos_.store(1);
        read_pos_.store(1);
    }

    /** Destructor. */
    ~AtomicBufferQueue() {
        delete [] buf_;
    }

    /** Get buffer size.
     - Thread safe
     .
     \return  Buffer size as item count, always a power of 2
    */
    Size size() const {
        return size_;
    }

    /** Get used item count.
     - Thread safe
     .
     \return  Item count used, 0 if queue is empty
    */
    Size used() const {
        const uint64 cursor = cursor_pos_.load(EVO_ATOMIC_ACQUIRE);
        const uint64 read   = read_pos_.load(EVO_ATOMIC_ACQUIRE);
        return (Size)(cursor < read ? 0 : cursor + 1 - read);
    }

    /** Get whether queue is empty.
     - Thread safe
     .
     \return  Whether empty, same as used() == 0
    */
    bool empty() const {
        return (cursor_pos_.load(EVO_ATOMIC_ACQUIRE) < read_pos_.load(EVO_ATOMIC_ACQUIRE));
    }

    /** Get whether queue is full.
     - Thread safe
     .
     \return  Whether full, same as used() == size()
    */
    bool full() const {
        return (used() >= size_);
    }

    /** Clear all items from queue, making it empty.
     - \b Caution: Not thread safe while an `add*()` method is being called by another thread, otherwise thread safe
    */
    void clear() {
        next_pos_.store(1, EVO_ATOMIC_RELEASE);
        cursor_pos_.store(0, EVO_ATOMIC_RELEASE);
        read_pos_.store(1, EVO_ATOMIC_RELEASE);
    }

    /** Add item to queue.
     - Thread safe
     - This uses Item::operator=() to copy the item to queue memory
       - Copying should be as fast as possible (use `memcpy()` and/or swap pointers) -- adding further items will block (semi-busy wait) while this is copying
       - For best perforamce, copying should not allocate any memory or involve any expensive processing
       - Not exception safe -- item assignment operator must not throw
       - For more control over copying the item see add_start() and add_commit()
     .
     \param  item  Item to add, copied with assignment operator
    */
    void add(typename DataCopy<Item>::PassType item) {
        // Claim a slot and wait for available capacity
        const uint64 seq = next_pos_.fetch_add(1, EVO_ATOMIC_ACQ_REL);
        while (seq - read_pos_.load(EVO_ATOMIC_ACQUIRE) >= size_)
            sleepus(1);

        // Store event in queue
        EVO_EXCEPTION_GUARD_START
        EVO_ATOMIC_FENCE(EVO_ATOMIC_ACQUIRE);
        buf_[seq & size_mask_] = item;
        EVO_ATOMIC_FENCE(EVO_ATOMIC_RELEASE);
        EVO_EXCEPTION_GUARD_END

        // Wait for cursor to reach previous slot, then increment cursor to commit the write
        const uint64 prev_seq = seq - 1;
        while (!cursor_pos_.compare_set(prev_seq, seq, EVO_ATOMIC_ACQ_REL, EVO_ATOMIC_ACQUIRE))
            sleepus(1);
    }

    /** Start adding item to queue directly.
     - Thread safe
     - Call this to claim an item, then overwrite the item, then call add_commit() to finish adding the item
       - This returns a reference to the claimed item, which should be overwritten, and also returns (via out param) a sequence number to pass to add_commit()
       - Copying should be as fast as possible (use `memcpy()` and/or swap pointers) -- adding further items will block (semi-busy wait) until add_commit() is called with the same sequence number
       - For best perforamce, copying should not allocate any memory or involve any expensive processing
       - \b Caution: Every add_start() call must have a subsequent matching add_commit() with the same sequence number, otherwise results are undefined (likely hang)
       - \b Caution: Make sure an exception while setting the item doesn't prevent the call to add_commit()
     .
     \param  seq  Stores sequence number for added item, pass this to add_commit()  [out]
     \return      Reference to item in queue, which should be reset -- items are reused so the state of the item is undefined
    */
    Item& add_start(uint64& seq) {
        // Claim a slot and wait for available capacity
        seq = next_pos_.fetch_add(1, EVO_ATOMIC_ACQ_REL);
        while (seq - read_pos_.load(EVO_ATOMIC_ACQUIRE) >= size_)
            sleepus(1);

        // Return event, user will set it
        EVO_ATOMIC_FENCE(EVO_ATOMIC_ACQUIRE);
        return buf_[seq & size_mask_];
    }

    /** Commit adding an item.
     - Thread safe
     - See add_start(), which must be called first to get the item to overwrite and sequence number to pass here
     - Call this immediately after overwriting the claimed item
     - \b Caution: Every add_start() call must have a subsequent matching add_commit() with the same sequence number, otherwise results are undefined (likely hang)
     .
     \param  seq  Item sequence number from add_start()
    */
    void add_commit(uint64 seq) {
        // Wait for cursor to reach previous slot, then increment cursor to commit the write
        EVO_ATOMIC_FENCE(EVO_ATOMIC_RELEASE);
        const uint64 prev_seq = seq - 1;
        while (!cursor_pos_.compare_set(prev_seq, seq, EVO_ATOMIC_ACQ_REL, EVO_ATOMIC_ACQUIRE))
            sleepus(1);
    }

    /** Pop oldest item from queue.
     - This doesn't really remove the item, but copies it and leaves it as-is in buffer to be overwritten later
     - This uses Item::operator=() to copy the item from queue memory
       - Copying should be as fast as possible (use `memcpy()` and/or swap pointers) -- adding further items will block (semi-busy wait) while the queue is full
       - Exception safe: Queue is unchanged if item copy throws (though ideally it should not throw)
     - Thread safety: Only call from 1 consumer thread at a time, otherwise results are undefined
     .
     \param  item  Stores popped item, copied with assignment operator  [out]
     \return       Whether item popped, false if queue is empty
    */
    bool pop(Item& item) {
        uint64 seq;
        seq = read_pos_.load(EVO_ATOMIC_ACQUIRE);
        if (seq <= cursor_pos_.load(EVO_ATOMIC_ACQUIRE)) {
            item = buf_[seq & size_mask_];
            read_pos_.fetch_add(1, EVO_ATOMIC_RELEASE);
            return true;
        }
        return false;
    }

private:
    // Disable copying
    AtomicBufferQueue(const This&);
    This& operator=(const This&);

    T*   buf_;
    Size size_;                 // Must be a power of 2 for mask to work
    Size size_mask_;            // Mask for faster modulus

    // Positions increase to infinity (index = pos % ringbuf_size_), would take hundreds of years to max out 64 bits
    AtomicUInt64 cursor_pos_;   // Position of latest item committed to queue
    AtomicUInt64 next_pos_;     // Next write position in queue (cursor + 1 when no add() in progress)
    AtomicUInt64 read_pos_;     // Position of next item to read from queue (cursor + 1 when queue is empty)
};

///////////////////////////////////////////////////////////////////////////////
//@}
}
#endif
