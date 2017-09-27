// Evo C++ Library
/* Copyright (c) 2016 Justin Crowell
 This Source Code Form is subject to the terms of the Mozilla Public
 License, v. 2.0. If a copy of the MPL was not distributed with this
 file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
///////////////////////////////////////////////////////////////////////////////
/** \file array.h Evo Array container. */
#pragma once
#ifndef INCL_evo_array_h
#define INCL_evo_array_h

// Includes
#include "evo_config.h"
#include "type.h"
#include "impl/container.h"
#include "impl/iter.h"

// Namespace: evo
namespace evo {

/** \addtogroup EvoContainers */
//@{

///////////////////////////////////////////////////////////////////////////////

// Internal implementation macros -- only used in this file
/** \cond impl */
// Grows to add items, without initializing -- size_ must be updated after calling
#define EVO_IMPL_ARRAY_GROW_ADD(SIZE) { \
    if (size_ > 0) { \
        Size newsize = size_ + SIZE; \
        T* const old_data = data_; \
        data_ = (T*)::malloc((size_t)newsize*sizeof(T)); \
        memcpy(data_, old_data, (size_t)size_*sizeof(T)); \
        ::free(old_data); \
    } else \
        data_ = (T*)::malloc((size_t)SIZE*sizeof(T)); \
}
// Grows to insert items, without initializing -- updates size_
#define EVO_IMPL_ARRAY_GROW_INSERT(INDEX, SIZE) { \
    assert( size_ > 0 ); \
    Size newsize = size_ + SIZE; \
    T* const old_data = data_; \
    data_ = (T*)::malloc((size_t)newsize*sizeof(T)); \
    if (INDEX > 0) { \
        memcpy(data_, old_data, (size_t)INDEX*sizeof(T)); \
        Size tail = size_ - INDEX; \
        assert( tail > 0 ); \
        memcpy(data_+INDEX+SIZE, old_data+INDEX, (size_t)tail*sizeof(T)); \
    } else \
        memcpy(data_+SIZE, old_data, (size_t)size_*sizeof(T)); \
    ::free(old_data); \
    size_ = newsize; \
}
/** \endcond */

///////////////////////////////////////////////////////////////////////////////

/** Dynamic array container with similar interface to List.

\tparam  T      Item type to use
\tparam  TSize  Size type to use for size/index values (must be unsigned integer) -- default: SizeT

\par Features

 - Items are stored sequentially in contiguous memory -- random access uses constant time
 - Advanced methods have "adv" prefix -- these allow some intrusive control
 - No memory allocated by new empty array
 - No extra capacity allocation, sharing, or slicing like List
 .

\par Iterators

 - Array<>::Iter -- Read-Only Iterator (IteratorRa)
 - Array<>::IterM -- Mutable Iterator (IteratorRa)
 .

\par Constructors

 - Array()
 - Array(const ValEmpty&)
 - Array(const ThisType&)
 - Array(const T*,Size)
 .

\par Read Access

 - size()
   - null(), empty()
 - data() const
   - item(Key) const
   - operator[](Key) const
   - first() const, last() const, iend()
   - hash()
 - compare()
   - operator==()
   - operator!=()
   - starts(const T&) const
   - starts(const T*,Size) const
   - ends(const T&) const
   - ends(const T*,Size) const
 .

\par Modifiers

 - data()
   - item(Key)
   - operator[](Key)
 - resize()
 - set()
   - set(const ThisType&)
   - set(const T*,Size)
   - setempty()
   - clear()
   - operator=(const ThisType& data)
   - operator=(const ValNull&)
   - operator=(const ValEmpty&)
   - fill()
 .

\par Advanced

 - advResize()
 .

\par Example

\code
\endcode
*/
template<class T,class TSize=SizeT>
class Array {
public:
    EVO_CONTAINER_TYPE;                            ///< Identify Evo container type
    typedef TSize               Size;            ///< List size integer type
    typedef Size                Key;            ///< Key type (item index)
    typedef T                   Value;            ///< Value type (same as Item)
    typedef T                   Item;            ///< Item type (same as Value)

    typedef Array<T,Size>        ThisType;        ///< This array type

    /** Default constructor sets as null. */
    //[tags: self, set_null! ]
    Array() {
        data_ = NULL;
        size_ = 0;
    }

    /** Constructor sets as empty but not null.
     \param  val  vEmpty
    */
    //[tags: self, set_empty! ]
    explicit Array(const ValEmpty& val) {
        EVO_PARAM_UNUSED(val);
        data_ = EVO_PEMPTY;
        size_ = 0;
    }

    /** Copy constructor.
     \param  src  Data to copy
    */
    //[tags: self, set_list! ]
    Array(const ThisType& src) {
        if (src.size_ > 0) {
            data_ = (T*)::malloc((size_t)src.size_*sizeof(T));
            ContainerOp<T>::init(data_, src.data_, src.size_);
        } else
            data_ = src.data_;
        size_ = src.size_;
    }

    /** Copy constructor.
     \param  data  Data to copy
     \param  size  Data size as item count
    */
    //[tags: self, set_ptr!, add_ptr! ]
    Array(const T* data, Size size) {
        if (size > 0) {
            data_ = (T*)::malloc((size_t)size*sizeof(T));
            ContainerOp<T>::init(data_, data, size);
        } else if (data == NULL)
            data_ = NULL;
        else
            data_ = EVO_PEMPTY;
        size_ = size;
    }

    /** Destructor to free used memory. */
    ~Array() {
        if (size_ > 0) {
            ContainerOp<T>::unInit(data_, size_);
            ::free(data_);
        }
    }

    // SET

    /** Assignment operator.
     \param  src  Data to copy
     \return      This
    */
    //[tags: self, set_list, add_list! ]
    ThisType& operator=(const ThisType& src)
        { return set(src); }

    /** Assignment operator to set as null and empty.
     - Same as set()
     .
    Example:
    \code
array = vNull;
    \endcode
     \return  This
    */
    //[tags: self, set_null, add_item! ]
    ThisType& operator=(const ValNull&)
        { return set(); }

    /** Assignment operator to set as empty but not null.
    Example:
    \code
array = vEmpty;
    \endcode
     \return  This
    */
    //[tags: self, set_empty, add_item! ]
    ThisType& operator=(const ValEmpty&)
        { return setempty(); }

    /** Clear by removing all items.
     - Does not set as null -- null status is unchanged
     .
     \return  This
    */
    //[tags: self, set ]
    ThisType& clear() {
        if (size_ > 0) {
            ContainerOp<T>::unInit(data_, size_);
            ::free(data_);
            data_ = EVO_PEMPTY;
            size_ = 0;
        }
        return *this;
    }

    /** Set as null and empty.
     \return  This
    */
    //[tags: self, set_null, set ]
    ThisType& set() {
        if (size_ > 0) {
            ContainerOp<T>::unInit(data_, size_);
            ::free(data_);
            size_ = 0;
        }
        data_ = NULL;
        return *this;
    }

    /** Set as copy of another array.
     \param  src  Data to copy
     \return      This
    */
    //[tags: self, set_list, set ]
    ThisType& set(const ThisType& src) {
        if (this != &src) {
            if (src.size_ > 0) {
                if (size_ == src.size_) {
                    // Same positive size
                    ContainerOp<T>::unInit(data_, size_);
                    ContainerOp<T>::init(data_, src.data_, src.size_);
                } else {
                    // New positive size
                    if (size_ > 0) {
                        ContainerOp<T>::unInit(data_, size_);
                        ::free(data_);
                    }
                    size_ = src.size_;
                    data_ = (T*)::malloc((size_t)src.size_*sizeof(T));
                    ContainerOp<T>::init(data_, src.data_, src.size_);
                }
            } else {
                // Null/Empty
                if (size_ > 0) {
                    ContainerOp<T>::unInit(data_, size_);
                    ::free(data_);
                    size_ = 0;
                }
                data_ = (src.data_ == NULL ? NULL : EVO_PEMPTY);
            }
        }
        return *this;
    }

    /** Set as copy using data pointer.
     \param  data  Data to copy
     \param  size  Data size as item count
     \return       This
    */
    //[tags: self, set_ptr, set ]
    ThisType& set(const T* data, Size size) {
        if (size == size_) {
            if (size_ > 0)
                ContainerOp<T>::unInit(data_, size_);
            if (size > 0)
                ContainerOp<T>::init(data_, data, size);
            else if (data == NULL)
                data_ = NULL;
            else
                data_ = EVO_PEMPTY;
        } else {
            if (size_ > 0) {
                ContainerOp<T>::unInit(data_, size_);
                ::free(data_);
            }
            if (size > 0) {
                data_ = (T*)::malloc((size_t)size*sizeof(T));
                ContainerOp<T>::init(data_, data, size);
            } else if (data == NULL)
                data_ = NULL;
            else
                data_ = EVO_PEMPTY;
            size_ = size;
        }
        return *this;
    }

    /** Set as empty but not null.
     \return  This
    */
    //[tags: self, set_empty, set ]
    ThisType& setempty() {
        if (size_ > 0) {
            ContainerOp<T>::unInit(data_, size_);
            ::free(data_);
            size_ = 0;
        }
        data_ = EVO_PEMPTY;
        return *this;
    }

    // INFO

    /** Get whether null.
     - Always empty when null
     .
     \return  Whether null
    */
    //[tags: info_size, set_null! ]
    bool null() const
        { return (data_ == NULL); }

    /** Get whether empty.
     - Empty when size() is 0
     .
     \return  Whether empty
    */
    //[tags: info_size, null(), set_empty! ]
    bool empty() const
        { return (size_ == 0); }

    /** Get size.
     \return  Size as item count
    */
    //[tags: info_size, item(), data() ]
    Size size() const
        { return size_; }

    /** Get data pointer (const).
     - \b Caution: Modifying the size of the array will invalidate returned pointer
     .
     \return  Data pointer as read-only, NULL/invalid if empty
    */
    //[tags: info_item ]
    const T* data() const
        { return data_; }

    /** Get item at position (const).
     - \b Caution: Results are undefined if index is out of bounds
     .
     \param  index  Item index
     \return        Given item as read-only (const)
    */
    //[tags: info_item ]
    const T& operator[](Key index) const
        { assert( index < size_ ); return data_[index]; }

    /** Get item at position (const).
     - \b Caution: Results are undefined if index is out of bounds
     .
     \param  index  Item index
     \return        Given item as read-only (const)
    */
    //[tags: info_item ]
    const T& item(Key index) const
        { assert( index < size_ ); return data_[index]; }

    /** Get first item (const).
     \return  First item pointer, NULL if empty
    */
    //[tags: info_item ]
    const T* first() const
        { return (size_ > 0 ? data_ : NULL); }

    /** Get last item (const).
     \return  Last item pointer, NULL if empty
    */
    //[tags: info_item ]
    const T* last() const
        { return (size_ > 0 ? data_+size_-1 : NULL); }

    /** Get index from last item using offset.
     - This simplifies math when computing an index from last item
     - This uses the formula: iend = size - 1 - offset
     .
     \param  offset  Offset from end, 0 for last item, 1 for second-last, etc
     \return         Resulting index, END if offset out of bounds
    */
    //[tags: size(), info_item ]
    Key iend(Size offset=0) const
        { return (offset < size_ ? size_-1-offset : END); }

    /** Get data hash value for whole array.
     \param  seed  Seed value for hashing multiple values, 0 if none
     \return       Hash value
    */
    //[tags: info_items ]
    ulong hash(ulong seed=0) const
        { return DataOp<T>::hash(data_, size_, seed); }

    // COMPARE

    /** Comparison.
     \param  data  Data to compare to
     \return       Result (<0 if this is less, 0 if equal, >0 if this is greater)
    */
    //[tags: self, compare ]
    int compare(const ThisType& data) const {
        int result;
        if (this == &data)
            result = 0;
        else if (data_ == NULL)
            result = (data.data_ == NULL ? 0 : -1);
        else if (data.data_ == NULL)
            result = 1;
        else
            result = DataOp<T>::compare(data_, size_, data.data_, data.size_);
        return result;
    }

    /** Equality operator.
     \param  data  Data to compare to
     \return       Whether equal
    */
    //[tags: self, compare ]
    bool operator==(const ThisType& data) const {
        bool result;
        if (this == &data)
            result = true;
        else if (data_ == NULL)
            result = (data.data_ == NULL);
        else if (data.data_ == NULL || size_ != data.size_)
            result = false;
        else
            result = DataOp<T>::equal(data_, data.data_, data.size_);
        return result;
    }

    /** Inequality operator.
     \param  data  Data to compare to
     \return       Whether inequal
    */
    //[tags: self, compare ]
    bool operator!=(const ThisType& data) const {
        bool result;
        if (this == &data)
            result = false;
        else if (data_ == NULL)
            result = (data.data_ != NULL);
        else if (data.data_ == NULL || size_ != data.size_)
            result = true;
        else
            result = !DataOp<T>::equal(data_, data.data_, data.size_);
        return result;
    }

    /** Check if starts with given item.
     - Uses item %operator==() for comparisons
     .
     \param  item  Item to check
     \return       Whether starts with item
    */
    //[tags: self, compare_sub, find(), compare() ]
    bool starts(const T& item) const
        { return (size_ > 0 && data_[0] == item); }

    /** Check if starts with given items.
     - Uses item %operator==() for comparisons
     .
     \param  items  Items to check
     \param  size   Size as item count to check
     \return        Whether starts with items
    */
    //[tags: self, compare_sub, find(), compare() ]
    bool starts(const T* items, Size size) const
        { return (size_ >= size && DataOp<T>::equal(data_, items, size)); }

    /** Check if ends with given item.
     - Uses item %operator==() for comparisons
     .
     \param  item  Item to check
     \return       Whether ends with item
    */
    //[tags: self, compare_sub, find(), compare() ]
    bool ends(const T& item) const
        { return (size_ > 0 && data_[size_-1] == item); }

    /** Check if ends with given items.
     - Uses item %operator==() for comparisons
     .
     \param  items  Items to check
     \param  size   Item count to check
     \return        Whether ends with items
    */
    //[tags: self, compare_sub, find(), compare() ]
    bool ends(const T* items, Size size) const
        { return (size_ >= size && DataOp<T>::equal(data_+(size_-size), items, size)); }

    // FIND

    // TODO: find(), findr(), findany(), findanyr(), contains()

    // INFO_SET

    /** Get data pointer (mutable).
     - \b Caution: Modifying the size of the array will invalidate returned pointer
     .
     \return  Data pointer (mutable), NULL/invalid if empty
    */
    //[tags: info_item ]
    T* data()
        { return data_; }

    /** Get item at position (mutable).
     - \b Caution: Results are undefined if index is out of bounds
     .
     \param  index  Item index
     \return        Given item (mutable)
    */
    //[tags: info_item ]
    T& operator[](Key index)
        { assert( index < size_ ); return data_[index]; }

    /** Get item at position (mutable).
     - \b Caution: Results are undefined if index is out of bounds
     .
     \param  index  Item index
     \return        Given item (mutable)
    */
    //[tags: info_item ]
    T& item(Key index)
        { assert( index < size_ ); return data_[index]; }

    /** Get first item (mutable).
     \return  First item pointer, NULL if empty
    */
    //[tags: info_item ]
    T* first()
        { return (size_ > 0 ? data_ : NULL); }

    /** Get last item (mutable).
     \return  Last item pointer, NULL if empty
    */
    //[tags: info_item ]
    T* last()
        { return (size_ > 0 ? data_+size_-1 : NULL); }

    /** Resize while preserving existing data (modifier).
     - This adds/removes items as needed until given size is reached
     - This will
     - \b Advanced: See advBuffer() for getting writable pointer to buffer
     - \b Advanced: See advResize() for best performance in certain POD cases
     .
     \param  size  New size as item count
     \return       This
    */
    //[tags: set_ptr!, size(), data() ]
    ThisType& resize(Size size) {
        if (size == 0) {
            // Null/Empty
            if (size_ > 0) {
                ContainerOp<T>::unInit(data_, size_);
                ::free(data_);
                data_ = EVO_PEMPTY;
                size_ = 0;
            }
        } else if (size_ != size) {
            // New positive size
            assert( size > 0 );
            if (size_ > 0) {
                // Preserve existing items
                T* const   old_data  = data_;
                const Size save_size = (size_ < size ? size_ : size);
                data_ = (T*)::malloc((size_t)size*sizeof(T));
                memcpy(data_, old_data, (size_t)save_size*sizeof(T));
                ContainerOp<T>::initTailSafe(data_, save_size, size);

                size_ -= save_size;
                if (size_ > 0)
                    ContainerOp<T>::unInit(old_data+save_size, size_);
                ::free(old_data);
            } else {
                // New array
                data_ = (T*)::malloc((size_t)size*sizeof(T));
                ContainerOp<T>::initSafe(data_, size);
            }
            size_ = size;
        }
        return *this;
    }

    // ADD

    /** Append new items.
     \param  size  Size as item count to append
     \return       This
    */
    //[tags: add_item, add, addrem_n, remove() ]
    ThisType& addnew(Size size=1) {
        if (size > 0) {
            EVO_IMPL_ARRAY_GROW_ADD(size);
            ContainerOp<T>::initSafe(data_+size_, size);
            size_ += size;
        }
        return *this;
    }

    /** Append new item.
     \param  item  Item to append
     \return       This
    */
    //[tags: self, add, add_item, addrem_item, set_item, remove() ]
    ThisType& add(const Item& item) {
        EVO_IMPL_ARRAY_GROW_ADD(1);
        ContainerOp<T>::init(data_+size_, &item, 1);
        ++size_;
        return *this;
    }

    // INSERT

    /** Insert new items.
     \param  index  Insert index, END to append
     \param  size   Size as item count to insert
     \return        Inserted index
    */
    //[tags: insert_item, insert, addrem_n, remove() ]
    Size insertnew(Key index, Size size=1) {
        if (size > 0) {
            if (index < size_) {
                EVO_IMPL_ARRAY_GROW_INSERT(index, size);
            } else {
                index = size_;
                EVO_IMPL_ARRAY_GROW_ADD(size);
                size_ += size;
            }
            ContainerOp<T>::initSafe(data_+index, size);
        }
        return index;
    }

    /** Insert new item.
     \param  index  Insert index, END to append
     \param  item   Item to insert
     \return        Inserted index
    */
    //[tags: self, insert, insert_item, addrem_item, set_item, remove() ]
    Size insert(Key index, const Item& item) {
        if (index < size_) {
            EVO_IMPL_ARRAY_GROW_INSERT(index, 1);
        } else {
            index = size_;
            EVO_IMPL_ARRAY_GROW_ADD(1);
            ++size_;
        }
        ContainerOp<T>::init(data_+index, &item, 1);
        return index;
    }

    // REMOVE

    /** Remove items.
     \param  index  Remove index
     \param  size   Remove size, ALL for all items from index
     \return        This
    */
    //[tags: set! ]
    ThisType& remove(Key index, Size size=1) {
        if (index < size_ && size > 0) {
            Size tempsize = size_ - index;
            if (size > tempsize)
                size = tempsize;
            if (size < size_) {
                // Preserve existing items
                tempsize = size_ - size;
                T* const old_data  = data_;
                data_ = (T*)::malloc((size_t)tempsize*sizeof(T));
                if (index > 0)
                    memcpy(data_, old_data, (size_t)index*sizeof(T));
                Size tail = size_ - index - size;
                if (tail > 0)
                    memcpy(data_+index, old_data+index+size, (size_t)tail*sizeof(T));

                ContainerOp<T>::unInit(old_data+index, size);
                ::free(old_data);
                size_ = tempsize;
            } else {
                // Remove all
                ContainerOp<T>::unInit(data_, size_);
                ::free(data_);
                data_ = EVO_PEMPTY;
                size_ = 0;
            }
        }
        return *this;
    }

    // FILL

    /** Fill using item (modifier).
     - Resizes to fill new items if needed
     .
     \param  item   Item to fill with
     \param  index  Start index, END to start at end and append
     \param  size   Size to fill as item count from index, ALL for all items from index, 0 to do nothing
    */
    //[tags: replace, resize() ]
    ThisType& fill(const T& item, Key index=0, Size size=ALL) {
        if (index == END)
            index = size_;
        if (size == ALL)
            size = (index < size_ ? size_ - index : 0);
        if (size > 0) {
            const Size newsize = index + size;
            if (newsize > size_)
                advResize(newsize);
            DataOp<T>::fill(data_+index, size, item);
        }
        return *this;
    }

    // MOVE / SWAP

    /** Swap with another array.
     - This swaps internal state directly so is faster than moving items
     .
     \param  array  %Array to swap with
    */
    //[tags: self, move ]
    void swap(ThisType& array)
        { EVO_IMPL_CONTAINER_SWAP(this, &array, ThisType); }

    // ALGS

    // TODO: search() binary search

    // ADVANCED

    /** Advanced: Resize while preserving existing data, POD items not initialized (modifier).
     - This is a slightly modified version of resize():
       - No difference on non POD item types
       - For POD item type: New items created here will not be initialized/zeroed
       - This gives a slight performance increase in some cases but will leave uninitialized garbage data for POD items
       - In most cases resize() is preferred since it's safer and the performance difference is usually negligible
       .
     - This adds/removes items as needed until given size is reached
     - \b Advanced: See advBuffer() for getting writable pointer to buffer
     .
     \param  size  New size as item count
     \return       This
    */
    //[tags: resize(), set_ptr!, size(), data() ]
    ThisType& advResize(Size size) {
        if (size == 0) {
            // Null/Empty
            if (size_ > 0) {
                ContainerOp<T>::unInit(data_, size_);
                ::free(data_);
                data_ = EVO_PEMPTY;
                size_ = 0;
            }
        } else if (size_ != size) {
            // New positive size
            assert( size > 0 );
            if (size_ > 0) {
                // Preserve existing items
                T* const   old_data  = data_;
                const Size save_size = (size_ < size ? size_ : size);
                data_ = (T*)::malloc((size_t)size*sizeof(T));
                memcpy(data_, old_data, (size_t)save_size*sizeof(T));
                ContainerOp<T>::initTailFast(data_, save_size, size);

                size_ -= save_size;
                if (size_ > 0)
                    ContainerOp<T>::unInit(old_data+save_size, size_);
                ::free(old_data);
            } else {
                // New array
                data_ = (T*)::malloc((size_t)size*sizeof(T));
                ContainerOp<T>::init(data_, size);
            }
            size_ = size;
        }
        return *this;
    }

    // ITERATORS

    // Iterator support types
    /** \cond impl */
    typedef Key IterKey;
    typedef T   IterItem;
    /** \endcond */

    typedef typename IteratorRa<ThisType>::Const Iter;    ///< Iterator (const) - IteratorRa
    typedef IteratorRa<ThisType>                 IterM;    ///< Iterator (mutable) - IteratorRa

    // INTERNAL

    // Iterator support methods
    /** \cond impl */
    void iterInitMutable()
        { }
    const IterItem* iterFirst(IterKey& key) const
        { key = 0; return data_; }
    const IterItem* iterNext(IterKey& key) const {
        const IterItem* result = NULL;
        if (key != END) {
            if (++key < size_)
                result = &(data_[key]);
            else
                key = END;
        }
        return result;
    }
    const IterItem* iterNext(Size count, IterKey& key) const {
        const T* result = NULL;
        if (key != END) {
            if ( (key+=count) < size_ )
                result = &(data_[key]);
            else
                key = END;
        }
        return result;
    }
    const IterItem* iterLast(IterKey& key) const {
        const IterItem* result = NULL;
        if (size_ > 0) {
            key    = size_ - 1;
            result = &(data_[key]);
        } else
            key = END;
        return result;
    }
    const IterItem* iterPrev(IterKey& key) const {
        const IterItem* result = NULL;
        if (key != END) {
            if (key > 0)
                result = &(data_[--key]);
            else
                key = END;
        }
        return result;
    }
    const IterItem* iterPrev(Size count, IterKey& key) const {
        const IterItem* result = NULL;
        if (key != END) {
            if (key > 0 && count <= key)
                result = &(data_[(key-=count)]);
            else
                key = END;
        }
        return result;
    }
    Size iterCount() const
        { return size_; }
    const IterItem* iterSet(IterKey key) const {
        const IterItem* result = NULL;
        if (key < size_)
            result = &(data_[key]);
        return result;
    }
    /** \endcond */

protected:
    T*   data_;
    Size size_;
};

///////////////////////////////////////////////////////////////////////////////

// Remove implementation macros
#undef EVO_IMPL_ARRAY_GROW_ADD
#undef EVO_IMPL_ARRAY_GROW_INSERT

///////////////////////////////////////////////////////////////////////////////
//@}
} // Namespace: evo
#endif
