// Evo C++ Library
/* Copyright (c) 2016 Justin Crowell
 This Source Code Form is subject to the terms of the Mozilla Public
 License, v. 2.0. If a copy of the MPL was not distributed with this
 file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
///////////////////////////////////////////////////////////////////////////////
/** \file ptrlist.h Evo Pointer List. */
#pragma once
#ifndef INCL_evo_ptrlist_h
#define INCL_evo_ptrlist_h

// Includes
#include "evo_config.h"
#include "impl/container.h"
#include "impl/iter.h"

// Namespace: evo
namespace evo {

/** \addtogroup EvoContainers */
//@{

///////////////////////////////////////////////////////////////////////////////

// Internal implementation macros -- only used in this file
/** \cond impl */
// Alloc+Copy items
#define EVO_IMPL_PTRLIST_ITEMS_ALLOC_COPY(DEST, SRC, FIRST, LAST) { \
    Item item; \
    for (Size i=FIRST; i<=LAST; ++i) { \
        if (SRC[i] == NULL) { \
            DEST[i] = NULL; \
        } else { \
            DEST[i] = item = EVO_IMPL_CONTAINER_MEM_ALLOC1(Value); \
            new(item) T(*SRC[i]); \
        } \
    } \
}
// Zero new items after Alloc+Copy
#define EVO_IMPL_PTRLIST_ITEMS_ALLOC_COPY_ZERO_NEW(FIRST, LAST) { \
    if (FIRST > 0) \
        memset(data_, 0, sizeof(Item*)*FIRST); \
    Size end = LAST + 1; \
    if (end < size_) \
        memset(data_+end, 0, sizeof(Item*)*(size_-end)); \
}

// Alloc/Realloc/Free buffer
//  Assumes buffer is already freed/null
#define EVO_IMPL_PTRLIST_ALLOC(HDR, DATA, SIZE) { \
    assert( SIZE > 0 ); \
    HDR = (Header*)::malloc( sizeof(Header) + (SIZE*sizeof(T*)) ); \
    assert( HDR != NULL ); \
    HDR->size = SIZE; \
    HDR->refs = 1; \
    DATA = (Item*)(HDR + 1); \
}
//  Assumes buffer is already freed/null and size_ is already set
#define EVO_IMPL_PTRLIST_ALLOC_COPY(SRC_HDR, SRC_DATA) { \
    EVO_IMPL_PTRLIST_ALLOC(header_, data_, size_); \
    header_->used  = SRC_HDR->used; \
    header_->first = SRC_HDR->first; \
    header_->last  = SRC_HDR->last; \
    if (header_->used > 0) { \
        Size last = header_->last; \
        EVO_IMPL_PTRLIST_ITEMS_ALLOC_COPY(data_, SRC_DATA, header_->first, last); \
        EVO_IMPL_PTRLIST_ITEMS_ALLOC_COPY_ZERO_NEW(header_->first, last); \
    } else \
        memset(data_, 0, sizeof(Item*)*size_); \
}
// TODO: Support EVO_LIST_REALLOC
//  Assumes buffer is already allocated
#define EVO_IMPL_PTRLIST_REALLOC(HDR, DATA, SIZE) { \
    assert( (ulong)HDR > sizeof(Header) ); \
    assert( DATA != NULL ); \
    assert( SIZE > 0 ); \
    HDR = (Header*)::realloc( HDR, sizeof(Header) + (SIZE*sizeof(T*)) ); \
    assert( HDR != NULL ); \
    HDR->size = SIZE; \
    DATA = (Item*)(HDR+1); \
}
//  Assumes buffer is already allocated
#define EVO_IMPL_PTRLIST_CLEAR(HDR) { \
    if (HDR->used > 0) { \
        assert( HDR->first <= HDR->last ); \
        Item* data = (Item*)(HDR+1); \
        Item* end  = data + HDR->last; \
        for (data += HDR->first; data <= end; ++data) \
            if (*data != NULL) { \
                (**data).~T(); \
                EVO_IMPL_CONTAINER_MEM_FREE(*data); \
            } \
    } \
}
//  Assumes buffer is already allocated
#define EVO_IMPL_PTRLIST_FREEMEM(HDR) { \
    assert( (ulong)HDR > sizeof(Header) ); \
    ::free(HDR); \
}
//  Assumes buffer is already allocated
#define EVO_IMPL_PTRLIST_FREE(HDR) { \
    if (HDR != NULL && --HDR->refs == 0) { \
        EVO_IMPL_PTRLIST_CLEAR(HDR); \
        EVO_IMPL_PTRLIST_FREEMEM(HDR); \
    } \
}
/** \endcond */

///////////////////////////////////////////////////////////////////////////////

// TODO: Support ListBase source for adding/copying items

/** Sequential list of managed pointers with random access.

 \tparam  T      Type to use -- an item will be T* (Item), a pointer to list will be T** (Item*)
 \tparam  TSize  Size type to use for size/index values (must be unsigned integer) -- default: SizeT

\par Features

 - This is similar to List but is specialized to hold an array of managed pointers, with PtrList:
   - \ref Sharing "Sharing" (without external references) is supported, but \ref Slicing "Slicing" is not supported
   - The size and capacity of the list are the same -- no extra capacity is allocated for adding new items
   - The size is the number of items allocated, any of which may be null
   - Item pointers are automatically freed (if not NULL)
   - Iterators skip null items and only return non-null items
   .
 - Some methods have a read-only (const) version and mutable version with suffix "M" -- example: item() and itemM()
 - operator[]() is read-only -- use operator()() or get() for mutable access
 - Create items with get()

 - Supports \ref Sharing "Sharing" -- however immutable data will be copied
 .

\par Iterators

 - PtrList<>::Iter -- Read-Only Iterator (IteratorRa)
 - PtrList<>::IterM -- Mutable Iterator (IteratorRa)
 .

\par Constructors

 - PtrList()
 - PtrList(const ThisType&)
 .

\par Read Access

 - size()
   - null(), empty()
   - shared()
 - data()
   - item()
   - operator[]()
   - first(), last(), iend()
   - hash()
 - compare()
   - operator==()
   - operator!=()
 - find()
   - findr()
 .

\par Modifiers

 - dataM()
   - itemM()
   - operator()()
 - resize()
   - resizemin()
   - unshare()
 - set()
   - set(const ThisType&)
   - setempty()
   - clear()
   - operator=(const ThisType&)
   - copy(const ThisType&)
 - get()
   - getitem()
 - remove()
 .

*/
template<class T,class TSize=SizeT>
class PtrList
{
public:
    EVO_CONTAINER_TYPE;
    typedef TSize               Size;            ///< %List size integer type
    typedef Size                Key;            ///< Key type (item index)
    typedef T                   Value;            ///< Value type (Item dereferenced, same as T)
    typedef T*                  Item;            ///< Item type (pointer to Value)

    typedef PtrList<T,Size>     ThisType;        ///< This list type

    /** Default constructor sets as null. */
    //[tags: self, set_null! ]
    PtrList() {
        header_ = NULL;
        data_   = NULL;
        size_   = 0;
    }

    /** Copy constructor.
     - Makes shared copy if possible -- see \ref Sharing "Sharing"
     .
     \param  data  Data to copy
    */
    //[tags: self, set_list!, unshare() ]
    PtrList(const ThisType& data) {
        if (data.size_ > 0) {
            header_ = data.header_;
            data_   = data.data_;
            ++header_->refs;
        } else {
            header_ = NULL;
            data_   = data.data_;
        }
        size_ = data.size_;
    }

    /** Destructor. */
    ~PtrList()
        { EVO_IMPL_PTRLIST_FREE(header_); }

    // SET

    /** Assignment operator.
     - Makes shared copy if possible -- see \ref Sharing "Sharing"
     .
     \param  data  Data to copy
     \return       This
    */
    //[tags: self, set_list, add_list!, unshare() ]
    ThisType& operator=(const ThisType& data)
        { return set(data); }

    /** Clear by removing all items.
     - Does not set list as null -- null status is unchanged
     - This does not change list size, only sets all items as null
     .
     \return  This
    */
    //[tags: self, set, unshare() ]
    ThisType& clear() {
        if (size_ > 0) {
            assert( header_ != NULL );
            if (header_->refs > 1) {
                // Detach from shared
                --header_->refs;
                header_ = NULL;
                data_   = EVO_PPEMPTY;
            } else if (header_->used > 0) {
                // Unique, clear items
                assert( data_ != NULL );
                Item* start = data_ + header_->first;
                Size  len   = (header_->last - header_->first) + 1;
                EVO_IMPL_PTRLIST_CLEAR(header_);
                memset(start, 0, len*sizeof(Item*));
                header_->used  = 0;
                header_->first = 0;
                header_->last  = 0;
            }
        }
        return *this;
    }

    /** Set as null and empty.
     \return  This
    */
    //[tags: self, set_null, set, unshare() ]
    ThisType& set() {
        if (size_ > 0) {
            assert( header_ != NULL );
            if (header_->refs > 1) {
                // Detach from shared
                --header_->refs;
            } else {
                // Unique, clear items
                assert( data_ != NULL );
                EVO_IMPL_PTRLIST_CLEAR(header_);
                EVO_IMPL_PTRLIST_FREEMEM(header_);
            }
            header_ = NULL;
            data_   = NULL;
            size_   = 0;
        } else
            data_ = NULL;
        return *this;
    }

    /** Set from another list.
     - Makes shared copy if possible -- see \ref Sharing "Sharing"
     .
     \param  data  Data to set/copy
     \return       This
    */
    //[tags: self, set_list, set, unshare() ]
    ThisType& set(const ThisType& data) {
        if (data_ != data.data_) {
            if (size_ > 0)
                EVO_IMPL_PTRLIST_FREE(header_);
            if (data.size_ > 0) {
                header_ = data.header_;
                data_   = data.data_;
                ++header_->refs;
            } else {
                header_ = NULL;
                data_   = data.data_;
            }
            size_ = data.size_;
        }
        return *this;
    }

    /** Set as empty but not null.
     \return  This
    */
    //[tags: self, set_empty, set, unshare() ]
    ThisType& setempty() {
        if (data_ == NULL)
            data_ = EVO_PPEMPTY;
        else
            clear();
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
     - Empty when size() is 0 or all items are null
     .
     \return  Whether empty
    */
    //[tags: info_size, null(), set_empty! ]
    bool empty() const
        { return (size_ == 0 || header_ == NULL || header_->used == 0); }

    /** Get list size.
     \return  Size as item count
    */
    //[tags: info_size, capacity!, item(), data() ]
    Size size() const
        { return size_; }

    /** Get list used size, number of non-null items.
     \return  Used size as item count
    */
    //[tags: info_size, capacity!, item(), data() ]
    Size used() const
        { return (header_ == NULL ? 0 : header_->used); }

    /** Get whether shared.
     - List is shared when a buffer is allocated and shared (reference count > 1)
     .
     \return  Whether shared
    */
    //[tags: shared, resize() ]
    bool shared() const
        { return (header_ != NULL && header_->refs > 1); }

    /** Get data pointer for direct access (const).
     - \b Caution: Calling any modifier/mutable method like unshare() after this may (will) invalidate the returned pointer
     - Result is only valid if size() > 0, otherwise may be NULL or an invalid internal value
     .
     \return  Data pointer as read-only, NULL/invalid if size() is 0 (const)
    */
    //[tags: info_item ]
    const Item* data() const
        { return data_; }

    /** Get item at position (const).
     - This is a pointer list so the given item can be null
     - \b Caution: Results are undefined if index is out of bounds
     .
     \param  index  Item index
     \return        Given item as read-only pointer, NULL if item is null (const)
    */
    //[tags: info_item ]
    const Item operator[](Key index) const {
        assert( header_ != NULL );
        assert( data_ != NULL );
        assert( index < size_ );
        return data_[index];
    }

    /** Get item at position (const).
     - This is a pointer list so the given item can be null
     - \b Caution: Results are undefined if index is out of bounds
     .
     \param  index  Item index
     \return        Given item as read-only pointer, NULL if item is null (const)
    */
    //[tags: info_item ]
    const Item item(Key index) const {
        assert( header_ != NULL );
        assert( data_ != NULL );
        assert( index < size_ );
        return data_[index];
    }

    /** Get first non-null item (const).
     - For best performance store result rather than calling repeatedly
     - \b Caution: Any operation that changes
     .
     \return  First item pointer, NULL if empty or all items null
    */
    //[tags: info_item ]
    const Item first() const
        { return (header_ != NULL && header_->used > 0 ? data_[header_->first] : NULL); }

    /** Get last non-null item (const).
     - The last non-null item does not necessarilly mean the very last item in list
     - For best performance store result rather than calling repeatedly
     .
     \return  Last item pointer, NULL if empty or all items null
    */
    //[tags: info_item ]
    const Item last() const
        { return (header_ != NULL && header_->used > 0 ? data_[header_->last] : NULL); }

    /** Get index for last item position using offset.
     - This simplifies math when computing an index from last item position
     - This uses the formula: iend = size - 1 - offset
     .
     \param  offset  Offset from end, 0 for last item, 1 for second-last, etc
     \return         Resulting index, END if offset out of bounds
    */
    //[tags: size(), info_item ]
    Key iend(Size offset=0) const
        { return (offset < size_ ? size_-1-offset : END); }

    // TODO: hash()

    // COMPARE

    /** Comparison.
     \param  data  Data to compare to
     \return       Result (<0 if this is less, 0 if equal, >0 if this is greater)
    */
    //[tags: self, compare ]
    int compare(const ThisType& data) const {
        int result;
        if (data_ == NULL)
            result = (data.data_ == NULL ? 0 : -1);
        else if (data.data_ == NULL)
            result = 1;
        else if (header_ == NULL || header_->used == 0)
            result = (data.header_ != NULL && data.header_->used > 0 ? -1 : 0);
        else if (data.header_ == NULL || data.header_->used == 0)
            result = 1;
        else {
            if (header_->first == data.header_->first) {
                result = 0;
                Item item1, item2;
                Size last1 = header_->last;
                Size last2 = data.header_->last;
                for (Size i=header_->first;; ++i) {
                    if (i > last1)
                        { result = (last1 == last2 ? 0 : -1); break; }
                    if (i > last2)
                        { result = 1; break; }
                    item1 = data_[i];
                    item2 = data.data_[i];
                    if (item1 == NULL) {
                        if (item2 != NULL)
                            { result = -1; break; }
                    } else if (item2 == NULL)
                        { result = 1; break; }
                    else if ((result=DataOp<Value>::compare(*item1, *item2)) != 0)
                        break;
                }
            } else
                result = (header_->first < data.header_->first ? 1 : -1);
        }
        return result;
    }

    /** Equality operator.
     - Items are compared with %operator==()
     .
     \param  data  Data to compare to
     \return       Whether equal
    */
    //[tags: self, compare ]
    bool operator==(const ThisType& data) const {
        bool result;
        if (data_ == NULL)
            result = (data.data_ == NULL);
        else if (data.data_ == NULL)
            result = false;
        else if (data_ == data.data_)
            result = true;
        else if (header_ == NULL || header_->used == 0)
            result = (data.header_ == NULL || data.header_->used == 0);
        else if (data.header_ == NULL || data.header_->used == 0)
            result = false;
        else {
            if ( header_->used  == data.header_->used  &&
                 header_->first == data.header_->first &&
                 header_->last  == data.header_->last ) {
                result = true;
                Item item1, item2;
                for (Size i=header_->first, last=header_->last; i<=last; ++i) {
                    item1 = data_[i];
                    item2 = data.data_[i];
                    if (item1 == NULL) {
                        if (item2 != NULL)
                            { result = false; break; }
                    } else if (item2 == NULL || !(*item1 == *item2))
                        { result = false; break; }
                }
            } else
                result = false;
        }
        return result;
    }

    /** Inequality operator.
     - Items are compared with %operator==()
     .
     \param  data  Data to compare to
     \return       Whether inequal
    */
    //[tags: self, compare ]
    bool operator!=(const ThisType& data) const {
        bool result;
        if (data_ == NULL)
            result = (data.data_ != NULL);
        else if (data.data_ == NULL)
            result = true;
        else if (data_ == data.data_)
            result = false;
        else if (header_ == NULL || header_->used == 0)
            result = (data.header_ != NULL && data.header_->used > 0);
        else if (data.header_ == NULL || data.header_->used == 0)
            result = true;
        else {
            if ( header_->used  == data.header_->used  &&
                 header_->first == data.header_->first &&
                 header_->last  == data.header_->last ) {
                result = false;
                Item item1, item2;
                for (Size i=header_->first, last=header_->last; i<=last; ++i) {
                    item1 = data_[i];
                    item2 = data.data_[i];
                    if (item1 == NULL) {
                        if (item2 != NULL)
                            { result = true; break; }
                    } else if (item2 == NULL || !(*item1 == *item2))
                        { result = true; break; }
                }
            } else
                result = true;
        }
        return result;
    }

    // FIND

    /** Find first occurrence of item with forward search.
     - This searches non-null items for given value, using item %operator==() for comparisons
     - Search stops before reaching end index or end of list
     .
     \param  value  Value to find
     \param  start  Starting index for search
     \param  end    End index for search, END for end of list
     \return        Found item index or NONE if not found
    */
    //[tags: self, find_item ]
    Key find(const Value& value, Key start=0, Key end=END) const {
        Key result = (Key)NONE;
        if (header_ != NULL && header_->used > 0) {
            if (start < header_->first)
                start = header_->first;
            if (end > header_->last + 1)
                end = header_->last + 1;
            Item item;
            for (; start<end; ++start) {
                item = data_[start];
                if (item != NULL && *item == value)
                    { result = start; break; }
            }
        }
        return result;
    }

    /** Find last occurrence of item with reverse search.
     - This searches non-null items for given value, using item %operator==() for comparisons
     - Same as find() but does reverse search starting right before end index, or at last item if end of list
     - As with find(), item at end index is not checked
     .
     \param  value  Item to find
     \param  start  Starting index for search range -- last item checked in reverse search
     \param  end    End index for search range (reverse search starting point), END for end of list
     \return        Found item index or NONE if not found
    */
    //[tags: self, find_item, find_item ]
    Key findr(const T& value, Key start=0, Key end=END) const {
        Key result = (Key)NONE;
        if (header_ != NULL && header_->used > 0) {
            if (start < header_->first)
                start = header_->first;
            if (end > header_->last + 1)
                end = header_->last + 1;
            Item item;
            while (end>start) {
                item = data_[--end];
                if (item != NULL && *item == value)
                    { result = end; break; }
            }
        }
        return result;
    }

    // TODO: contains()

    // INFO_SET

    /** Get data pointer (mutable).
     - Calls unshare()
     - \b Caution: Calling any modifier method like resize() after this may (will) invalidate the returned pointer
     - For best performance, reuse returned pointer for repeated access, or use data() instead for read-only access
     .
     \return  Data pointer (mutable).
    */
    //[tags: info_item ]
    Item* dataM()
        { unshare(); return data_; }

    /** Get item for key, creating if needed (mutable).
     - Calls unshare()
     - Item is created with default value if not found
     - This grows list as needed using resize() if item key is too small to fit
     .
     \param  key      Key (index) to use
     \param  created  Stores whether new item was created, if not NULL
     \return          Item pointer for key
    */
    Item getitem(const Key& key, bool* created=NULL) {
        Item result;
        if (key >= size_) {
            resize(key+1);
            goto newitem;
        } else {
            unshare();
            result = data_[key];
        }
        if (result == NULL) {
            // Create new item
            newitem: // used to skip NULL check
            data_[key] = result = EVO_IMPL_CONTAINER_MEM_ALLOC1(Value);
            DataOp<T>::init(*result);
            if (++header_->used == 1)
                header_->first = header_->last = key;
            else if (key < header_->first)
                header_->first = key;
            else if (key > header_->last)
                header_->last = key;
            assert( header_->first <= header_->last );
            if (created != NULL)
                *created = true;
        } else if (created != NULL)
            *created = false;
        return result;
    }

    /** Get item value for key, creating if needed (mutable).
     - Calls unshare()
     - Item is created with default value if not found
     - This grows list as needed using resize() if item key is too small to fit
     .
     \param  key      Key (index) to use
     \param  created  Stores whether new item was created, if not NULL
     \return          Value reference for key
    */
    Value& get(const Key& key, bool* created=NULL)
        { return *getitem(key, created); }

    /** Get item at position (mutable).
     - Calls unshare()
     - This is a pointer list so the given item can be null
     - This doesn't allow modifying the stored pointer, for that use get()
     - For best performance, use dataM() once for repeated item access, or item() for read-only access
     - \b Caution: Results are undefined if index is out of bounds -- though index is checked with assert()
     .
     \param  index  Item index
     \return        Given item pointer, NULL if item is null (mutable)
    */
    //[tags: info_item ]
    Item operator()(Key index)
        { assert( index < size_ ); unshare(); return data_[index]; }

    /** Get item at position (mutable).
     - Calls unshare()
     - This is a pointer list so the given item can be null
     - This doesn't allow modifying the stored pointer, for that use get()
     - For best performance, use dataM() once for repeated item access, or item() for read-only access
     - \b Caution: Results are undefined if index is out of bounds -- though index is checked with assert()
     .
     \param  index  Item index
     \return        Given item pointer, NULL if item is null (mutable)
    */
    //[tags: info_item ]
    Item itemM(Key index)
        { assert( index < size_ ); unshare(); return data_[index]; }

    /** Make sure data is not shared by allocating new buffer if needed (modifier).
     - Use to make buffer unique (not shared) and writable (when not empty)
     - This is called automatically by mutable/modifier methods
     - This does nothing if empty or not shared
     .
     \return  This
    */
    //[tags: shared, resize() ]
    ThisType& unshare() {
        if (size_ > 0) {
            assert( header_ != NULL );
            if (header_->refs > 1) {
                --header_->refs;
                Header* oldheader = header_;
                Item*   olddata   = data_;
                EVO_IMPL_PTRLIST_ALLOC_COPY(oldheader, olddata);
            }
        }
        return *this;
    }

    // RESIZE

    /** Resize while preserving existing data (modifier).
     - This adds/removes items as needed until given size is reached
     - Effectively calls unshare()
     - \b Advanced: See advBuffer() for getting writable pointer to buffer
     - \b Advanced: See advResize() for best performance in certain POD cases
     .
     \param  newsize  New size as item count
     \return          This
    */
    //[tags: capacity, set_ptr!, size(), dataM(), unshare() ]
    ThisType& resize(Size newsize) {
        if (newsize == size_) {
            unshare();
        } else {
            if (newsize == 0) {
                EVO_IMPL_PTRLIST_FREE(header_);
                header_ = NULL;
                data_   = EVO_PPEMPTY;
                size_   = 0;
                return *this;
            } else if (size_ > 0) {
                assert( header_ != NULL );
                if (header_->refs > 1) {
                    // Shared: Unshare
                    --header_->refs;
                    Size used  = header_->used;
                    Size first = header_->first;
                    if (used > 0 && first < newsize) {
                        // There are items to copy from original list (otherwise create new empty list below)
                        Header* oldheader = header_;
                        Item*   olddata   = data_;
                        if (newsize > size_) {
                            // New null items
                            size_ = newsize;
                            EVO_IMPL_PTRLIST_ALLOC_COPY(oldheader, olddata);
                        } else {
                            // Truncate
                            Size last = header_->last;
                            if (newsize <= last) {
                                // Find new used count (after truncation)
                                {
                                    Item* data = data_ + newsize;
                                    if ((newsize + 1 - first) < (size_- newsize)) {
                                        // Count items not truncated (less pointers to check)
                                        used = 1; // safe to assume item at first is not null
                                        for (Item* start=data_+first; --data > start; )
                                            if (*data != NULL)
                                                ++used;
                                    } else {
                                        // Count items truncated (less pointers to check)
                                        --used; // safe to assume item at last is not null
                                        for (Item* end=data_+last; data < end; ++data)
                                            if (*data != NULL)
                                                --used;
                                    }
                                }
                                assert( used > 0 ); // always true since first < newsize

                                // Find new last item (after truncation)
                                last = newsize;
                                while (--last > first && data_[last] == NULL)
                                    { }
                            }

                            // Create new list, copying up to newsize
                            size_ = newsize;
                            EVO_IMPL_PTRLIST_ALLOC(header_, data_, size_);
                            header_->used  = used;
                            header_->first = first;
                            header_->last  = last;
                            EVO_IMPL_PTRLIST_ITEMS_ALLOC_COPY(data_, olddata, first, last);
                            EVO_IMPL_PTRLIST_ITEMS_ALLOC_COPY_ZERO_NEW(first, last);
                        }
                        return *this;
                    } // else create new list (below)
                } else {
                    // Unique
                    if (newsize > size_) {
                        // New null items
                        EVO_IMPL_PTRLIST_REALLOC(header_, data_, newsize);
                        memset(data_+size_, 0, sizeof(Item)*(newsize-size_));
                        size_ = newsize;
                    } else {
                        // Truncate
                        Size used = header_->used;
                        Size last = header_->last;
                        if (used > 0 && newsize <= last) {
                            // Truncate items
                            {
                                Item* data = data_ + newsize;
                                for (Item* end=data_+last; data <= end; ++data)
                                    if (*data != NULL) {
                                        (**data).~T();
                                        EVO_IMPL_CONTAINER_MEM_FREE(*data);
                                        --used;
                                    }
                            }

                            // Find new last item, update header
                            if (used > 0) {
                                Size first = header_->first;
                                last = newsize;
                                while (--last > first && data_[last] == NULL)
                                    { }
                                header_->used  = used;
                                header_->last  = last;
                            } else {
                                header_->used  = 0;
                                header_->first = 0;
                                header_->last  = 0;
                            }

                            // Realloc
                            EVO_IMPL_PTRLIST_REALLOC(header_, data_, newsize);
                            size_ = newsize;
                        } else {
                            // Truncate null items
                            EVO_IMPL_PTRLIST_REALLOC(header_, data_, newsize);
                            size_ = newsize;
                        }
                    }
                    return *this;
                }
            } // else create new list (below)

            // New list
            EVO_IMPL_PTRLIST_ALLOC(header_, data_, newsize);
            memset(data_, 0, sizeof(Item)*newsize);
            header_->used  = 0;
            header_->first = 0;
            header_->last  = 0;
            size_ = newsize;
        }
        return *this;
    }

    /** Resize to minimum size while preserving existing data (modifier).
     - This calls resize() if current size is smaller than minimum
     .
     \param  minsize  Minimum size as item count
     \return          This
    */
    //[tags: capacity, set_ptr!, size(), dataM(), unshare() ]
    ThisType& resizemin(Size minsize) {
        if (minsize > size_)
            resize(minsize);
        return *this;
    }

    // COPY

    /** Set as full (unshared) copy using data pointer (modifier).
     - Effectively calls unshare()
     .
     \param  data  Data to copy
     \return       This
    */
    //[tags: self, set_ptr, set, unshare() ]
    ThisType& copy(const ThisType& data) {
        if (data_ == data.data_) {
            unshare();
        } else {
            if (size_ > 0)
                EVO_IMPL_PTRLIST_FREE(header_);
            if (data.size_ > 0) {
                size_ = data.size_;
                EVO_IMPL_PTRLIST_ALLOC_COPY(data.header_, data.data_);
            } else {
                header_ = NULL;
                data_   = (data.data_ == NULL ? NULL : EVO_PPEMPTY);
                size_   = 0;
            }
        }
        return *this;
    }

    // REMOVE

    // TODO: performance issue when removing first or last item repeatedly
    /** Remove item and set as null (modifier).
     - Effectively calls unshare()
     - The removed item is set as null -- no change to list size
     .
     \param  key  Item key (index) to remove
     \return      This
    */
    //[tags: set! ]
    ThisType& remove(Key key) {
        if (key < size_) {
            assert( header_ != NULL );
            if (key < header_->first || key > header_->last) {
                // No item, unshare
                unshare();
            } else if (header_->refs > 1) {
                // Shared: Unshare
                if (data_[key] != NULL)    {
                    --header_->refs;
                    if (header_->used == 1) {
                        // New list
                        EVO_IMPL_PTRLIST_ALLOC(header_, data_, size_);
                        memset(data_, 0, sizeof(Item)*size_);
                        header_->used  = 0;
                        header_->first = 0;
                        header_->last  = 0;
                    } else {
                        // New list, minus removed item
                        Header* oldheader = header_;
                        Item*   olddata   = data_;
                        Size    first     = header_->first;
                        Size    last      = header_->last;

                        // Variation of: EVO_IMPL_PTRLIST_ITEMS_ALLOC_COPY(data_, olddata, header_->first, last);
                        EVO_IMPL_PTRLIST_ALLOC(header_, data_, size_);
                        header_->used  = oldheader->used - 1;
                        header_->first = first;
                        header_->last  = last;
                        Item item;
                        for (Size i=first, last_copy=i; i<=last; ++i) {
                            if (i == key) {
                                data_[i] = NULL;
                                if (i == first) {
                                    // Find new first item then continue copying
                                    while (++i <= last)
                                        if (olddata[i] != NULL) {
                                            header_->first = i;
                                            data_[i] = item = EVO_IMPL_CONTAINER_MEM_ALLOC1(Value);
                                            new(item) T(*olddata[i]);
                                            break; // no need to set last_copy since not removing last item
                                        } else
                                            data_[i] = NULL;
                                    assert( i <= last ); // used > 1, so always another first item
                                } else if (i == last) {
                                    // New last item
                                    assert( last_copy > 0 && last_copy < last ); // used > 1 so last_copy already set
                                    header_->last = last_copy;
                                }
                            } else if (olddata[i] == NULL) {
                                data_[i] = NULL;
                            } else {
                                data_[i] = item = EVO_IMPL_CONTAINER_MEM_ALLOC1(Value);
                                new(item) T(*olddata[i]);
                                last_copy = i;
                            }
                        }
                        EVO_IMPL_PTRLIST_ITEMS_ALLOC_COPY_ZERO_NEW(first, last);
                    }
                }
            } else {
                // Unique
                Item& item = data_[key];
                if (item != NULL) {
                    (*item).~T();
                    EVO_IMPL_CONTAINER_MEM_FREE(item);
                    item = NULL;
                    if (--header_->used == 0)
                        // Empty
                        header_->first = header_->last = 0;
                    else if (key == header_->first) {
                        // Find new first item
                        const Size last = header_->last;
                        while (++key < last && data_[key] == NULL)
                            { }
                        header_->first = key;
                    } else if (key == header_->last) {
                        // Find new last item
                        const Size first = header_->first;
                        while (--key > first && data_[key] == NULL)
                            { }
                        header_->last = key;
                    }
                }
            }
        } else
            // No item, unshare
            unshare();
        return *this;
    }

    // SWAP

    /** Swap with another list.
     - This swaps internal state directly for best performance
     .
     \param  list  %List to swap with
    */
    //[tags: self, move ]
    void swap(ThisType& list)
        { EVO_IMPL_CONTAINER_SWAP(this, &list, ThisType); }

    // ITERATORS

    // Iterator support types
    /** \cond impl */
    typedef Key   IterKey;
    typedef Value IterItem;
    /** \endcond */

    typedef typename IteratorRa<ThisType>::Const Iter;    ///< Iterator (const) - IteratorRa
    typedef IteratorRa<ThisType>                 IterM;    ///< Iterator (mutable) - IteratorRa

    // INTERNAL

    // Iterator support methods
    /** \cond impl */
    void iterInitMutable()
        { unshare(); }
    const IterItem* iterFirst(IterKey& key) const {
        const IterItem* result;
        if (header_ != NULL && header_->used > 0) {
            key    = header_->first;
            result = data_[key];
        } else {
            key    = END;
            result = NULL;
        }
        return result;
    }
    const IterItem* iterNext(IterKey& key) const {
        const IterItem* result;
        if (key != END && header_ != NULL && header_->used > 0) {
            const Size last = header_->last;
            if (key < last) {
                for (;;)
                    if (++key == last || data_[key] != NULL)
                        { result = data_[key]; break; }
            } else
                { key = END; result = NULL; }
        } else
            result = NULL;
        return result;
    }
    const IterItem* iterNext(Size count, IterKey& key) const {
        const IterItem* result;
        if (key != END && header_ != NULL && header_->used > 0 && count > 0) {
            const Size last = header_->last;
            if (key < last) {
                for (;;)
                    if (++key == last) {
                        if (--count > 0)
                            { key = END; result = NULL; break; }
                        result = data_[key];
                        break;
                    } else if (data_[key] != NULL && --count == 0)
                        { result = data_[key]; break; }
            } else
                { key = END; result = NULL; }
        } else
            result = NULL;
        return result;
    }
    const IterItem* iterLast(IterKey& key) const {
        const IterItem* result;
        if (header_ != NULL && header_->used > 0) {
            key    = header_->last;
            result = data_[key];
        } else {
            key    = END;
            result = NULL;
        }
        return result;
    }
    const IterItem* iterPrev(IterKey& key) const {
        const IterItem* result;
        if (key != END && header_ != NULL && header_->used > 0) {
            const Size first = header_->first;
            if (key > first) {
                for (;;)
                    if (--key == first || data_[key] != NULL)
                        { result = data_[key]; break; }
            } else
                { key = END; result = NULL; }
        } else
            result = NULL;
        return result;
    }
    const IterItem* iterPrev(Size count, IterKey& key) const {
        const IterItem* result;
        if (key != END && header_ != NULL && header_->used > 0 && count > 0) {
            const Size first = header_->first;
            if (key > first) {
                for (;;)
                    if (--key == first) {
                        if (--count > 0)
                            { key = END; result = NULL; break; }
                        result = data_[key];
                        break;
                    } else if (data_[key] != NULL && --count == 0)
                        { result = data_[key]; break; }
            } else
                { key = END; result = NULL; }
        } else
            result = NULL;
        return result;
    }
    Size iterCount() const
        { return (header_ != NULL ? header_->used : 0); }
    const IterItem* iterSet(IterKey& key) const {
        const IterItem* result;
        if (header_ != NULL && header_->used > 0) {
            if (key >= header_->first) {
                const Size last = header_->last;
                for (;; ++key)
                    if (key > last)
                        { key = END; result = NULL; break; }
                    else if (data_[key] != NULL)
                        { result = data_[key]; break; }
            } else
                { key = header_->first; result = data_[key]; }
        } else
            result = NULL;
        return result;
    }
    /** \endcond */

protected:
    /** List data header */
    struct Header {
        Size size;                ///< Buffer size allocated as item count (capacity)
        Size used;                ///< Buffer size used/initialized as item count
        Size first;                ///< Index of first used item, 0 if used=0
        Size last;                ///< Index of last used item, 0 if used=0
        Size refs;                ///< Buffer reference count
    };

    // States:
    //  null:    data=NULL, size=0
    //  empty:   data=1 or data=buffer, size=0
    //  buffer:  data=buffer, size>0, header.refs>=1
    Header* header_;            ///< Data header pointer, NULL if no buffer allocated
    Item*   data_;                ///< Data pointer, NULL if null, can be 1 if empty (size_=0)
    Size    size_;                ///< Data size (same as header.size), 0 if empty
};

///////////////////////////////////////////////////////////////////////////////

// Remove implementation macros
#undef EVO_IMPL_PTRLIST_ITEMS_ALLOC_COPY
#undef EVO_IMPL_PTRLIST_ITEMS_ALLOC_COPY_ZERO_NEW
#undef EVO_IMPL_PTRLIST_ALLOC
#undef EVO_IMPL_PTRLIST_ALLOC_COPY
#undef EVO_IMPL_PTRLIST_REALLOC
#undef EVO_IMPL_PTRLIST_CLEAR
#undef EVO_IMPL_PTRLIST_FREEMEM
#undef EVO_IMPL_PTRLIST_FREE

///////////////////////////////////////////////////////////////////////////////
//@}
} // Namespace: evo
#endif
