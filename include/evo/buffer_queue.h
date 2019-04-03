// Evo C++ Library
/* Copyright 2019 Justin Crowell
Distributed under the BSD 2-Clause License -- see included file LICENSE.txt for details.
*/
///////////////////////////////////////////////////////////////////////////////
/** \file buffer_queue.h Evo BufferQueue. */
#pragma once
#ifndef INCL_evo_buffer_queue_h
#define INCL_evo_buffer_queue_h

#include "type.h"

namespace evo {
/** \addtogroup EvoContainers */
//@{

///////////////////////////////////////////////////////////////////////////////

/** Fast buffer-based queue, implemented with a ring-buffer.
 \tparam  T      Item type to use, copied with assignment operator
 \tparam  TSize  Size type to use for queue size (must be unsigned integer) -- default: SizeT

\par Features

 - Efficient buffer based queue, especially for simple (POD or Byte-Copy) types
 - This allocates a fixed size buffer and is not resizable, size is always a power of 2
   - For a dynamic size queue (or stack) see: List
 - Adding items does not allocate memory (though the item assignment operator may)
 - Popped (removed or cleared) items are left as-is in buffer, to be overwritten as new items are added
 - All operations take constant time, except copy constructor and assignment operator which take linear time (to copy items)
 .
 
Note that this is not a full EvoContainer and doesn't have iterators.

 - See also: AtomicBufferQueue

\par Example

\code
#include <evo/buffer_queue.h>
using namespace evo;

int main() {
    BufferQueue<int> queue(20); // rounds to 32 (power of 2)

    queue.add(1);
    queue.add(2);
    queue.add(3);

    int a = queue.pop();    // set to 1
    int b = queue.pop();    // set to 2
    int c = queue.pop();    // set to 3

    return 0;
}
\endcode
*/
template<class T, class TSize=SizeT>
class BufferQueue {
public:
    typedef BufferQueue<T,TSize> This;      ///< %This type
    typedef TSize Size;                     ///< Queue size integer type (always unsigned)
    typedef T     Item;                     ///< Item type

    static const Size MIN_SIZE     = 2;     ///< Minimum size to use
    static const Size DEFAULT_SIZE = 128;   ///< Default size to use

    /** Constructor, sets buffer size.
     \param  size  Buffer size to use as item count, rouned to next power of 2 if needed
    */
    BufferQueue(Size size=DEFAULT_SIZE) {
        size = adjust_size(size);
        buf_       = new T[size];
        size_      = size;
        size_mask_ = size - 1;
        used_      = 0;
        start_     = 0;
    }

    /** Copy constructor. 
     - Not exception safe: Item assignment operator must not throw
     .
     \param  src  Queue to copy from
    */
    BufferQueue(const This& src) {
        buf_       = new T[src.size_];
        size_      = src.size_;
        size_mask_ = src.size_mask_;
        used_      = src.used_;
        start_     = src.start_;
        copydata(src);
    }

    /** Destructor. */
    ~BufferQueue() {
        delete [] buf_;
    }

    /** Get buffer size.
     \return  Buffer size as item count, always a power of 2
    */
    Size size() const {
        return size_;
    }

    /** Get used item count.
     \return  Item count used, 0 if queue is empty
    */
    Size used() const {
        return used_;
    }

    /** Get whether queue is empty.
     \return  Whether empty, same as used() == 0
    */
    bool empty() const {
        return (used_ == 0);
    }

    /** Get whether queue is full.
     \return  Whether full, same as used() == size()
    */
    bool full() const {
        return (used_ >= size_);
    }

    /** Clear all items from queue, making it empty. */
    void clear() {
        used_  = 0;
        start_ = 0;
    }

    /** Add item to queue.
     - Exception safe: Queue is unchanged if item copy throws
     .
     \param  item   Item to add, copied with assignment operator
     \param  force  Whether to overwrite oldest item when full, false to return false if full
     \return        Whether new item added, false if full or if oldest item was overwritten with force=true
    */
    bool add(const T& item, bool force=false) {
        if (used_ < size_) {
            buf_[((uint64)start_ + used_) & size_mask_] = item;
            ++used_;
            return true;
        } else if (force) {
            buf_[start_] = item;
            start_ = (start_ + 1) & size_mask_;
        }
        return false;
    }

    /** Advanced: Add new item to queue and get pointer to it.
     - This doesn't actually reset or overwrite the item, but just returns a pointer to it
     - Use this to setup the new item directly, instead of making a copy with add()
     .
     \param  force  Whether to overwrite oldest item when full, false to fail if full
     \return        Pointer to added item, NULL if full (and force is false)
    */
    T* advAdd(bool force=false) {
        if (used_ < size_) {
            return buf_ + (((uint64)start_ + used_++) & size_mask_);
        } else if (force) {
            T* item = buf_ + start_;
            start_ = (start_ + 1) & size_mask_;
            return item;
        }
        return NULL;
    }

    /** Peek at oldest item in queue.
     - Use pop() to remove this item
     .
     \return  Reference to oldest item in queue, or oldest/first item if queue is empty
    */
    const T& peek() const {
        return buf_[start_];
    }

    /** Peek at oldest items in queue.
     - This is useful for bulk processing items in queue, though may take up to 2 passes when the ring-buffer wraps around
     - This returns a pointer to the oldest items and the size of contiguous items from there
     - Use pop_size() with size to remove these items, then call this again to see any remaining items
     .
     \param  size  Set to number of contiguous items from pointer, 0 if empty
     \return       Oldest item pointer, NULL if empty
    */
    const T* peek(Size& size) const {
        if (used_ > 0) {
            size = size_ - start_;
            if (size > used_)
                size = used_;
            return buf_ + start_;
        }
        size = 0;
        return NULL;
    }

    /** Pop oldest item from queue.
     - This doesn't really remove the item, but leaves it as-is in buffer to be overwritten later
     - Exception safe: Queue is unchanged if item copy throws
     .
     \param  item  Stores popped item, copied with assignment operator
     \return       Whether item popped, false if queue is empty
    */
    bool pop(T& item) {
        if (used_ > 0) {
            item = buf_[start_];
            start_ = (start_ + 1) & size_mask_;
            --used_;
            return true;
        }
        return false;
    }

    /** Pop oldest item from queue without returning it.
     - Use peek() const to get the item first
     - This doesn't really remove the item, but leaves it as-is in buffer to be overwritten later
     .
     \return  Whether item popped, false if queue is empty
    */
    bool pop() {
        if (used_ > 0) {
            start_ = (start_ + 1) & size_mask_;
            --used_;
            return true;
        }
        return false;
    }

    /** Pop oldest items from queue in bulk.
     - Use peek(Size&) const to get the items first
     - This doesn't really remove items, but leaves them as-is in buffer to be overwritten later
     .
     \param  size  Number of items to pop, ALL for all
     \return       Number of items popped, may be less than requested if queue is smaller, 0 if queue is empty
    */
    Size pop_size(Size size) {
        if (used_ > 0) {
            if (size > used_)
                size = used_;
            start_ = (Size)(((uint64)start_ + size) & size_mask_);
            used_ -= size;
            return size;
        }
        return 0;
    }

    /** Assignment operator.
     - Copies all items from another queue to this
     - This will resize the current queue to match src, all current items will be lost
     - Not exception safe: Item assignment operator must not throw
     .
    */
    This& operator=(const This& src) {
        if (size_ != src.size_) {
            delete [] buf_;
            buf_       = new T[src.size_];
            size_mask_ = src.size_mask_;
            size_      = src.size_;
        }
        used_  = src.used_;
        start_ = src.start_;
        copydata(src);
        return *this;
    }

private:
    T*   buf_;
    Size size_;
    Size size_mask_;
    Size used_;
    Size start_;

    // This assumes buf_ is allocated and everything but buf_ data has been copied
    void copydata(const This& src) {
        if (used_ > 0) {
            Size copysize = size_ - start_;
            DataInit<T>::copy(buf_ + start_, src.buf_ + start_, copysize);
            if (copysize < used_) {
                copysize = used_ - copysize;
                DataInit<T>::copy(buf_, src.buf_, copysize);
            }
        }
    }

    // Make sure size is a power of 2
    static Size adjust_size(Size size) {
        assert( !IntegerT<Size>::SIGN );
        const Size MAX_SIZE = (IntegerT<Size>::MAX >> 1) + 1;
        if (size <= MIN_SIZE) {
            size = MIN_SIZE;
        } else if (size >= MAX_SIZE) {
            size = MAX_SIZE;
        } else {
            const Size targetsize = size;
            size = MIN_SIZE;
            while (size < targetsize)
                size <<= 1;
        }
        return size;
    }
};

///////////////////////////////////////////////////////////////////////////////
//@}
}
#endif
