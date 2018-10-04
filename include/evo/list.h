// Evo C++ Library
/* Copyright 2018 Justin Crowell
Distributed under the BSD 2-Clause License -- see included file LICENSE.txt for details.
*/
///////////////////////////////////////////////////////////////////////////////
/** \file list.h Evo List container. */
#pragma once
#ifndef INCL_evo_list_h
#define INCL_evo_list_h

#include "impl/container.h"
#include "impl/iter.h"

// Disable certain MSVC warnings for this file
#if defined(_MSC_VER)
    #pragma warning(push)
    #pragma warning(disable:4458)
#endif

namespace evo {
/** \addtogroup EvoContainers */
//@{

///////////////////////////////////////////////////////////////////////////////

/** Sequential list container with random access.

\tparam  T      Item type to use
\tparam  TSize  Size type to use for size/index values (must be unsigned integer) -- default: SizeT

\par Features

 - Similar to STL vector
 - Items are stored sequentially in memory as a dynamic array -- random access uses constant time
 - Preallocates extra memory when buffer grows -- see capacity(), resize(), capacity(Size)
 - No memory allocated by new empty list
 .
 - Some methods have a read-only (const) version and modifier version with suffix "M" -- example: item() and itemM()
 - operator[]() is read-only -- use operator()() for mutable access
 - Supports efficient use as stack or queue -- see add(const Item&), pop(), popq()
 - Advanced methods have "adv" prefix -- these allow some intrusive control
 .
 - \ref Sharing "Sharing" and \ref Slicing "Slicing" make for simple and efficient copying/splitting
 - \b Caution: Copying from a raw pointer will use \ref UnsafePtrRef "Unsafe Pointer Referencing"
 .

\par Iterators

 - List<>::Iter -- Read-Only Iterator (IteratorRa)
 - List<>::IterM -- Mutable Iterator (IteratorRa)
 .

\par Constructors

 - List()
 - List(const ValEmpty&)
 - List(const ListType&)
 - List(const ListType&,Key,Key)
 - List(const ListBaseType&,Key,Key)
 - List(const Item*,Size)
 - List(const PtrBase<Item>&,Size)
 .

\par Read Access

 - size()
   - null(), empty()
   - capacity()
   - shared()
 - data()
   - item()
   - operator[]()
   - first(), last(), iend()
   - hash()
 - compare()
   - operator==()
   - operator!=()
   - starts(ItemVal) const
   - starts(const Item*,Size) const
   - ends(ItemVal) const
   - ends(const Item*,Size) const
 - find()
   - findr()
   - findany()
   - findanyr()
   - contains(ItemVal) const
   - contains(const Item*,Size) const
 - splitat(Key,T1&,T2&) const
   - splitat(Key,T1&) const
   - splitat(Key,ValNull,T2&) const
 .

\par Slicing

 - slice(Key)
   - slice(Key,Size), slice2()
   - truncate()
   - triml(), trimr()
 - splitat_setl(Key), splitat_setl(Key,T2&)
 - splitat_setr(Key), splitat_setr(Key,T1&)
 - pop(), popq()
 - unslice()
 .

\par Modifiers

 - dataM()
   - itemM()
   - operator()()
 - resize()
   - reserve(), compact()
   - capacity(Size)
   - capacitymin(), capacitymax()
   - unshare()
 - set()
   - set(const ListType&)
   - set(const ListType&,Key,Key)
   - set(const ListBaseType&,Key,Key)
   - set(const Item*,Size)
   - set(const PtrBase<Item>&,Size)
   - set2()
   - setempty()
   - clear()
   - operator=(const ListType& data)
   - operator=(const ValNull&)
   - operator=(const ValEmpty&)
   - copy(const ListBaseType&)
   - copy(const Item*,Size)
   - fill()
 - add(const Item&)
   - add(const ListBaseType&)
   - add(const Item*,Size)
   - addnew()
   - operator<<(const Item&)
   - operator<<(const ListBaseType&)
   - operator<<(const ValNull&)
   - operator<<(const ValEmpty&)
 - prepend(const Item&)
   - prepend(const ListBaseType&)
   - prepend(const Item*,Size)
   - prependnew()
 - insert(Key,const Item&)
   - insert(Key,const ListBaseType&)
   - insert(Key,const Item*,Size)
   - insertnew()
 - pop(T&)
   - pop(T&,Key)
   - popq(T&)
 - remove()
   - replace()
 - move(Key,Key)
   - move(Key,ListType&,Key,Size)
   - swap(Key,Key)
   - swap(ListType&)
 - reverse()
 .

\par Advanced

 - advItem(Key)
 - advResize()
 - advBuffer(Size)
   - advBuffer()
   - advSize()
 - advWrite()
   - advWriteDone()
 - advEdit()
   - advEditDone()
 - advAdd()
 - advPrepend()
 - advInsert()
 - advRemove()
 - advSwap()
 .

\par Example

\code
#include <evo/list.h>
#include <evo/io.h>
using namespace evo;
static Console& c = con();

int main() {
    // Create number list, add some numbers
    List<int> list;
    list.add(1);
    list << 2 << 3;

    // operator[] provides read-only (const) access
    int value = list[0];
    //list[0] = 0;     // Error: operator[] is read-only (const)

    // operator() provides write (mutable) access
    list(0) = 0;       // Allowed: operator() is mutable

    // Iterate and modify items (mutable)
    for (List<int>::IterM iter(list); iter; ++iter)
        (*iter) += 10;  // Parenthesis not required, only used to workaround doxygen formatting bug

    // Iterate and print items (read-only)
    for (List<int>::Iter iter(list); iter; ++iter)
        c.out << "Item: " << *iter << NL;

    return 0;
}
\endcode

Output:
\code{.unparsed}
Item: 10
Item: 12
Item: 13
\endcode
*/
template<class T,class TSize=SizeT>
class List : public ListBase<T,TSize> {
protected:
    using ListBase<T,TSize>::data_; // Slice data pointer, NULL if null, EVO_PEMPTY if empty (null/empty: buffer unused, no reference)
    using ListBase<T,TSize>::size_; // Slice size, 0 if empty

public:
    EVO_CONTAINER_TYPE;
    typedef TSize               Size;               ///< %List size integer type
    typedef Size                Key;                ///< %Key type (item index)
    typedef T                   Value;              ///< %Value type (same as Item)
    typedef T                   Item;               ///< %Item type (same as Value)
    typedef typename DataCopy<T>::PassType ItemVal; ///< %Item type as parameter (POD types passed by value, otherwise by const-ref)

    typedef List<T,Size>        ThisType;           ///< This list type
    typedef List<T,Size>        ListType;           ///< %List type for parameters
    typedef ListBase<T,Size>    ListBaseType;       ///< %List base type for any Evo list

    /** %Edit buffer for advEdit().
     - This holds buffer state during an edit operation
     - This cleans up if edit is incomplete so will not leak memory
     - See advEdit()
    */
    struct Edit {
        T*    ptr;      ///< Data pointer, use to write to buffer
        Size  size;     ///< Data size, update after write
        void* header;   ///< Internal buffer data, do not modify

        /** Constructor. */
        Edit() : ptr(NULL), size(0), header(NULL)
            { }

        /** Destructor, frees buffer if needed. */
        ~Edit() {
            if (header != NULL)
                ::free(header);
        }

        /** Clear data and free buffer. */
        void clear() {
            if (header != NULL) {
                ::free(header);
                header = NULL;
            }
            ptr  = NULL;
            size = 0;
        }

        /** Write (copy) data from source.
         - This appends to end of edit data (ptr+size) and updates size member
         - \b Caution: This assumes destination items are uninitialized (only matters for non-POD types)
         - \b Caution: Results are undefined if copying from buffer being edited in-place (advEdit() returned false)
         - \b Caution: Results are undefined if writing after end of buffer
         .
         \param  src    Source to copy from
         \param  start  Start position to copy from
         \param  count  Item count to copy, ALL for all
         \return        Actual item count written, 0 for none
        */
        Size write(const ListBaseType& src, Size start=0, Size count=ALL) {
            if (start < src.size_ && count > 0) {
                const Size maxcount = src.size_ - start;
                if (count > maxcount)
                    count = maxcount;
                DataInit<Item>::init(ptr + size, src.data_, count);
                size += count;
            } else
                count = 0;
            return count;
        }

        /** Write (copy) data from buffer.
         - This appends to end of edit data (ptr+size) and updates size member
         - \b Caution: This assumes destination items are uninitialized (only matters for non-POD types)
         - \b Caution: Results are undefined if copying from buffer being edited in-place (advEdit() returned false)
         - \b Caution: Results are undefined if writing after end of buffer
         .
         \param  data   Data pointer to copy from
         \param  count  Item count to copy
         \return        Actual item count written (same as count)
        */
        Size write(const Item* data, Size count) {
            if (count > 0) {
                assert( data != NULL );
                DataInit<Item>::init(ptr + size, data, count);
                size += count;
            }
            return count;
        }
    };

    /** Default constructor sets as null. */
    List() {
        data_ = NULL;
        size_ = 0;
        #if EVO_LIST_OPT_REFTERM
            terminated_ = false;
        #endif
    }

    /** Constructor sets as empty but not null.
     \param  val  vEMPTY
    */
    explicit List(const ValEmpty& val) {
        EVO_PARAM_UNUSED(val);
        data_ = EVO_PEMPTY;
        size_ = 0;
        #if EVO_LIST_OPT_REFTERM
            terminated_ = false;
        #endif
    }

    /** Copy constructor.
     - Makes shared copy if possible -- see \ref Sharing "Sharing"
     .
     \param  data  Data to copy
    */
    List(const ListType& data) {
        data_ = NULL;
        size_ = 0;
        #if EVO_LIST_OPT_REFTERM
            terminated_ = false;
        #endif
        ref(data);
    }

    /** Extended copy constructor.
     - Makes shared copy if possible -- see \ref Sharing "Sharing"
     .
     \param  data   Data to copy
     \param  index  Start index of data to copy, END to set as empty
     \param  size   Size as item count, ALL for all from index
    */
    List(const ListType& data, Key index, Key size=ALL) {
        data_ = NULL;
        size_ = 0;
        #if EVO_LIST_OPT_REFTERM
            terminated_ = false;
        #endif
        ref(data, index, size);
    }

    /** Constructor to copy sublist data.
     - For best performance (and less safety) reference sublist instead with set(const Item*,Size)
     .
     \param  data   Data to copy
     \param  index  Start index of data to reference, END to set as empty
     \param  size   Size as item count, ALL for all from index
    */
    List(const ListBaseType& data, Key index=0, Key size=ALL) {
        size_ = 0;
        #if EVO_LIST_OPT_REFTERM
            terminated_ = false;
        #endif
        if (data.data_ == NULL) {
            data_ = NULL;
        } else if (index < data.size_) {
            const Size max_size = data.size_ - index;
            if (size > max_size)
                size = max_size;
            if (size > 0)
                copy(data.data_+index, size);
            else
                data_ = EVO_PEMPTY;
        } else
            data_ = EVO_PEMPTY;
    }

    /** Constructor to copy sublist data.
     - For best performance (and less safety) reference sublist instead with set(const Item*,Size)
     .
     \param  data   Data pointer to copy from, NULL to set as null
     \param  index  Start index of data to reference, END to set as empty
     \param  size   Size as item count, ALL for all from index
    */
    List(const ListBaseType* data, Key index=0, Key size=ALL) {
        size_ = 0;
        #if EVO_LIST_OPT_REFTERM
            terminated_ = false;
        #endif
        if (data == NULL || data->data_ == NULL) {
            data_ = NULL;
        } else if (index < data->size_) {
            const Size max_size = data->size_ - index;
            if (size > max_size)
                size = max_size;
            if (size > 0)
                copy(data->data_+index, size);
            else
                data_ = EVO_PEMPTY;
        } else
            data_ = EVO_PEMPTY;
    }

    /** Constructor for data pointer.
     - \b Caution: Uses \ref UnsafePtrRef "Unsafe Pointer Referencing"
     .
     \param  data  Data pointer to use
     \param  size  Data size as item count
    */
    List(const Item* data, Size size) {
        data_ = NULL;
        size_ = 0;
        #if EVO_LIST_OPT_REFTERM
            terminated_ = false;
        #endif
        if (data != NULL)
            ref(data, size);
    }

    /** Constructor to copy from managed pointer.
     \param  data  Data pointer to use
     \param  size  Data size as item count
    */
    List(const PtrBase<Item>& data, Size size) {
        data_ = NULL;
        size_ = 0;
        #if EVO_LIST_OPT_REFTERM
            terminated_ = false;
        #endif
        if (data.ptr_ != NULL)
            copy(data.ptr_, size);
    }

    // SET

    /** Assignment operator.
     - Makes shared copy if possible -- see \ref Sharing "Sharing"
     .
     \param  data  Data to copy
     \return       This
    */
    ListType& operator=(const ListType& data)
        { return set(data); }

    /** Assignment operator to copy sublist.
     - For best performance (and less safety) reference sublist instead with set(const Item*,Size)
     .
     \param  data  Data to copy
     \return       This
    */
    ListType& operator=(const ListBaseType& data) {
        if (data.data_ == NULL)
            set();
        else if (data.size_ > 0)
            copy(data.data_, data.size_);
        else
            setempty();
        return *this;
    }

    /** Assignment operator to set as null and empty.
     - Same as set()
     .
    Example:
    \code
    list = vNULL;
    \endcode
     \return  This
    */
    ListType& operator=(const ValNull&)
        { return set(); }

    /** Assignment operator to set as empty but not null.
    Example:
    \code
    list = vEMPTY;
    \endcode
     \return  This
    */
    ListType& operator=(const ValEmpty&)
        { clear(); data_ = EVO_PEMPTY; return *this; }

    /** Clear by removing all items.
     - Does not set as null -- null status is unchanged
     - Append operators can be chained\n
       Example:
       \code
        // Clear character list and append two characters
        list.clear() << 'a' << 'b';
       \endcode
     .
     \return  This
    */
    ListType& clear() {
        if (data_ > EVO_PEMPTY) {
            if (buf_.ptr != NULL) {
                assert( buf_.header != NULL );
                if (buf_.header->refs > 1) {
                    // Detach from shared
                    --buf_.header->refs;
                    buf_.header = NULL;
                    buf_.ptr    = NULL;
                    data_       = EVO_PEMPTY;
                } else if (buf_.header->used > 0) {
                    // Clear buffer, leave buffer for later use
                    assert( buf_.header->refs == 1 );
                    DataInit<T>::uninit(buf_.ptr, buf_.header->used);
                    buf_.header->used = 0;
                    data_             = buf_.ptr;
                }
            } else {
                data_ = EVO_PEMPTY;
                #if !EVO_LIST_OPT_LAZYBUF
                    // Lazy buffer disabled
                    assert( buf_.header == NULL );
                #endif
            }
            size_ = 0;
            #if EVO_LIST_OPT_REFTERM
                terminated_ = false;
            #endif
        }
        #if !EVO_LIST_OPT_LAZYBUF
            // Lazy buffer disabled
            assert( (buf_.header != NULL) == (buf_.ptr != NULL) );
        #endif
        return *this;
    }

    /** %Set as null and empty.
     - Append operators can be chained\n
       Example:
       \code
        // Clear character list and append two characters
        list.set() << 'a' << 'b';
       \endcode
     .
     \return  This
    */
    ListType& set()
        { clear(); data_ = NULL; return *this; }

    /** %Set from data pointer.
     - \b Caution: Uses \ref UnsafePtrRef "Unsafe Pointer Referencing"
     .
     \param  data  Data pointer to use
     \param  size  Data size as item count
     \return       This
    */
    ListType& set(const Item* data, Size size)
        { ref(data, size); return *this; }

    /** %Set from managed data pointer.
     - \b Caution: Uses \ref UnsafePtrRef "Unsafe Pointer Referencing"
     .
     \param  data  Data pointer to copy
     \param  size  Data size as item count
    */
    ListType& set(const PtrBase<Item>& data, Size size) {
        if (data.ptr_ == NULL)
            set();
        else
            copy(data.ptr_, size);
        return *this;
    }

    /** %Set from another list.
     - Makes shared copy if possible -- see \ref Sharing "Sharing"
     .
     \param  data  Data to copy
     \return       This
    */
    ListType& set(const ListType& data)
        { ref(data); return *this; }

    /** %Set from subset of another list.
     - Makes shared copy if possible -- see \ref Sharing "Sharing"
     .
     \param  data   Data to copy
     \param  index  Start index of data, END to set as empty
     \param  size   Data size as item count, ALL for all from index
     \return        This
    */
    ListType& set(const ListType& data, Key index, Key size=ALL)
        { ref(data, index, size); return *this; }

    /** %Set as copy of sublist.
     - For best performance (and less safety) reference sublist instead with set(const Item*,Size)
     .
     \param  data   Data to copy
     \param  index  Start index of sublist data, END to set as empty
     \param  size   Data size as item count, ALL for all from index
     \return        This
    */
    ListType& set(const ListBaseType& data, Key index=0, Key size=ALL) {
        if (data.data_ == NULL)
            set();
        else if (index < data.size_) {
            const Size max_size = data.size_ - index;
            if (size > max_size)
                size = max_size;
            if (size > 0)
                copy(data.data_+index, size);
            else
                setempty();
        } else
            setempty();
        return *this;
    }

    /** %Set from subset of another list using start/end positions.
     - Makes shared copy if possible -- see \ref Sharing "Sharing"
     - If index2 < index1 then index2 will be set to index1 (empty sublist)
     .
     \param  data    Data to copy
     \param  index1  Start index of new slice, END to slice all items from beginning
     \param  index2  End index of new slice (this item not included), END for all after index1
     \return         This
    */
    ListType& set2(const ListType& data, Key index1, Key index2)
        { ref(data, index1, (index1<index2?index2-index1:0)); return *this; }

    /** %Set as copy of sublist using start/end positions.
     - For best performance (and less safety) reference sublist instead with set(const Item*,Size)
     .
     \param  data    Data to copy
     \param  index1  Start index of sublist data, END to set as empty
     \param  index2  End index of sublist data (this item not included), END for all after index1
     \return         This
    */
    ListType& set2(const ListBaseType& data, Key index1, Key index2) {
        if (data.data_ == NULL) {
            set();
        } else {
            if (index2 > data.size_)
                index2 = data.size_;
            if (index1 < data.size_ && index2 > index1)
                copy(data.data_+index1, (index2-index1));
            else
                setempty();
        }
        return *this;
    }

    /** %Set as empty but not null.
     - Append operators can be chained\n
       Example:
       \code
        // Set as empty character list then append two characters
        list.setempty() << 'a' << 'b';
       \endcode
     .
     \return  This
    */
    ListType& setempty()
        { clear(); data_ = EVO_PEMPTY; return *this; }

    // INFO

    /** Get whether null.
     - Always empty when null
     .
     \return  Whether null
    */
    bool null() const
        { return (data_ == NULL); }

    /** Get whether empty.
     - Empty when size() is 0
     .
     \return  Whether empty
    */
    bool empty() const
        { return (size_ == 0); }

    /** Get size.
     \return  Size as item count
    */
    Size size() const
        { return size_; }

    /** Get whether shared.
     - Data is shared when referencing external data or buffer is allocated and shared (reference count > 1)
     .
     \return  Whether shared
    */
    bool shared() const
        { return (buf_.ptr == NULL ? (size_ > 0) : (buf_.header->refs > 1)); }

    /** Get capacity.
     - This gets the total size of currently allocated buffer
     - Data still may be referenced without a buffer so this could return less than size()
     - Buffer may be allocated (by previous use) even though not currently used
     .
     \return  Capacity as item count, 0 if no buffer allocated
    */
    Size capacity() const
        { return (buf_.header == NULL ? 0 : buf_.header->size); }

    /** Get data pointer (const).
     - \b Caution: May return an invalid non-NULL pointer if empty
     - \b Caution: Calling any modifier/mutable method like unshare() after this may (will) invalidate the returned pointer
     .
     \return  Data pointer as read-only, NULL if null, may be invalid if empty (const)
    */
    const Item* data() const
        { return data_; }

    /** Get item at position (const).
     - \b Caution: Results are undefined if index is out of bounds
     .
     \param  index  Item index
     \return        Given item as read-only (const)
    */
    const Item& operator[](Key index) const
        { assert( index < size_ ); return data_[index]; }

    /** Get item at position (const).
     - \b Caution: Results are undefined if index is out of bounds
     .
     \param  index  Item index
     \return        Given item as read-only (const)
    */
    const Item& item(Key index) const
        { assert( index < size_ ); return data_[index]; }

    /** Get first item (const).
     \return  First item pointer, NULL if empty
    */
    const Item* first() const
        { return (size_ > 0 ? data_ : NULL); }

    /** Get last item (const).
     \return  Last item pointer, NULL if empty
    */
    const Item* last() const
        { return (size_ > 0 ? data_+size_-1 : NULL); }

    /** Get index from last item using offset.
     - This simplifies math when computing an index from last item
     - This uses the formula: iend = size - 1 - offset
     .
     \param  offset  Offset from end, 0 for last item, 1 for second-last, etc
     \return         Resulting index, END if offset out of bounds
    */
    Key iend(Size offset=0) const
        { return (offset < size_ ? size_-1-offset : END); }

    /** Get data hash value.
     \param  seed  Seed value for hashing multiple values, 0 if none
     \return       Hash value
    */
    ulong hash(ulong seed=0) const
        { return DataHash<T>::hash(data_, size_, seed); }

    // COMPARE

    /** Comparison.
     \param  data  Data to compare to
     \return       Result (<0 if this is less, 0 if equal, >0 if this is greater)
    */
    int compare(const ListBaseType& data) const {
        int result;
        if (data_ == NULL)
            result = (data.data_ == NULL ? 0 : -1);
        else if (data.data_ == NULL)
            result = 1;
        else
            result = DataCompare<T>::compare(data_, size_, data.data_, data.size_);
        return result;
    }

    /** Equality operator.
     \param  data  Data to compare to
     \return       Whether equal
    */
    bool operator==(const ListBaseType& data) const {
        bool result;
        if (data_ == NULL)
            result = (data.data_ == NULL);
        else if (data.data_ == NULL || size_ != data.size_)
            result = false;
        else if (size_ == 0 || data_ == data.data_) {
            result = true;
        } else
            result = DataEqual<T>::equal(data_, data.data_, data.size_);
        return result;
    }
    
    /** Inequality operator.
     \param  data  Data to compare to
     \return       Whether inequal
    */
    bool operator!=(const ListBaseType& data) const {
        bool result;
        if (data_ == NULL)
            result = (data.data_ != NULL);
        else if (data.data_ == NULL || size_ != data.size_)
            result = true;
        else if (size_ == 0 || data_ == data.data_) {
            result = false;
        } else
            result = !DataEqual<T>::equal(data_, data.data_, data.size_);
        return result;
    }

    /** Check if this starts with given item.
     - Uses item %operator==() for comparisons
     .
     \param  item  Item to check
     \return       Whether starts with item
    */
    bool starts(ItemVal item) const
        { return (size_ > 0 && data_[0] == item); }

    /** Check if starts with given items.
     - Uses item %operator==() for comparisons
     .
     \param  items  Items to check
     \param  size   Size as item count to check
     \return        Whether starts with items
    */
    bool starts(const Item* items, Size size) const
        { return (size > 0 && size_ >= size && DataEqual<T>::equal(data_, items, size)); }

    /** Check if this starts with given items.
     - Uses item %operator==() for comparisons
     .
     \param  items  Items to check
     \return        Whether starts with items
    */
    bool starts(const ListBaseType& items) const
        { return (items.size_ > 0 && size_ >= items.size_ && DataEqual<T>::equal(data_, items.data_, items.size_)); }

    /** Check if this ends with given item.
     - Uses item %operator==() for comparisons
     .
     \param  item  Item to check
     \return       Whether ends with item
    */
    bool ends(ItemVal item) const
        { return (size_ > 0 && data_[size_-1] == item); }

    /** Check if this ends with given items.
     - Uses item %operator==() for comparisons
     .
     \param  items  Items to check
     \param  size   Item count to check
     \return        Whether ends with items
    */
    bool ends(const Item* items, Size size) const
        { return (size > 0 && size_ >= size && DataEqual<T>::equal(data_+(size_-size), items, size)); }

    /** Check if this ends with given items.
     - Uses item %operator==() for comparisons
     .
     \param  items  Items to check
     \return        Whether ends with items
    */
    bool ends(const ListBaseType& items) const
        { return (items.size_ > 0 && size_ >= items.size_ && DataEqual<T>::equal(data_+(size_-items.size_), items.data_, items.size_)); }

    // FIND

    /** Find first occurrence of item with forward search.
     - This searches for given item, using item %operator==() for comparisons
     - Search stops before reaching end index or end of list
     - Item at end index is not checked
     .
     \param  item   Item to find
     \param  start  Starting index for search
     \param  end    End index for search, END for end of list
     \return        Found item index or NONE if not found
    */
    Key find(ItemVal item, Key start=0, Key end=END) const {
        if (end > size_)
            end = size_;
        for (; start < end; ++start)
            if (data_[start] == item)
                return start;
        return NONE;
    }

    /** Find last occurrence of item with reverse search.
     - This searches for given item, using item %operator==() for comparisons
     - This does a reverse search starting right before end index
     - Item at end index is not checked
     .
     \param  item   Item to find
     \param  start  Starting index for search range -- last item checked in reverse search
     \param  end    End index for search range (reverse search starting point), END for end of list
     \return        Found item index or NONE if not found
    */
    Key findr(ItemVal item, Key start=0, Key end=END) const {
        if (end > size_)
            end = size_;
        while (end > start)
            if (data_[--end] == item)
                return end;
        return NONE;
    }

    /** Find first occurrence of any given items with forward search.
     - This searches for any of given items, using item %operator==() for comparisons
     - Search stops before reaching end index or end of list
     .
     \param  items  Items to search for
     \param  count  Count of items to search for
     \param  start  Starting index for search
     \param  end    End index for search, END for end of list
     \return        Found item index or NONE if not found
    */
    Key findany(const Item* items, Size count, Key start=0, Key end=END) const {
        Size j;
        if (end > size_)
            end = size_;
        for (; start < end; ++start)
            for (j=0; j<count; ++j)
                if (data_[start] == items[j])
                    return start;
        return NONE;
    }

    /** Find last occurrence of any given items with reverse search.
     - This searches for any of given items, using item %operator==() for comparisons
     - Same as findany() but does reverse search starting right before end index or at last item if end of list
     - As with findany(), item at end index is not checked
     .
     \param  items  Items to search for
     \param  count  Count of items to search for
     \param  start  Starting index for search range -- last item checked in reverse search
     \param  end    End index for search range (reverse search starting point), END for end of list
     \return        Found item index or NONE if not found
    */
    Key findanyr(const Item* items, Size count, Key start=0, Key end=END) const {
        Size j;
        if (end > size_)
            end = size_;
        while (end>start) {
            --end;
            for (j=0; j < count; ++j)
                if (data_[end] == items[j])
                    return end;
        }
        return NONE;
    }

    /** Check whether contains given item.
     - This does a simple linear search for given item, using item %operator==() for comparisons
     .
     \param  item  Item to check for
     \return       Whether item was found
    */
    bool contains(ItemVal item) const {
        bool result = false;
        for (Key i=0; i < size_; ++i)
            if (data_[i] == item)
                { result = true; break; }
        return result;
    }

    /** Check if contains given data.
     - This does a simple linear search for given array of items, using item %operator==() for comparisons
     .
     \param  data  Data to check for
     \param  size  Size as item count to check for
     \return       Whether data was found
    */
    bool contains(const Item* data, Size size) const {
        bool result = false;
        if (size > 0 && size_ >= size) {
            const Size end = size_ - size;
            for (Key i=0; i <= end; ++i)
                if (DataEqual<T>::equal(data_+i, data, size))
                    { result = true; break; }
        }
        return result;
    }

    /** Check if contains given data.
     - This does a simple linear search for given array of items, using item %operator==() for comparisons
     .
     \param  data  Data to check for
     \return       Whether data was found
    */
    bool contains(const ListBaseType& data) const {
        bool result = false;
        if (data.size_ > 0 && size_ >= data.size_) {
            const Size end = size_ - data.size_;
            for (Key i=0; i <= end; ++i)
                if (DataEqual<T>::equal(data_+i, data.data_, data.size_))
                    { result = true; break; }
        }
        return result;
    }

    // SPLIT

    /** Split into left/right sublists at index.
     - Can use methods like find() or findany() for index
     - Template types are automatically deduced from arguments
     - For more advanced parsing see StrTok
     .
     \tparam  T1    %List type to store left sublist
     \tparam  T2    %List type to store right sublist

     \param  index  Index to split at, bad index (NONE or out-of-bounds) splits at end
     \param  left   %Set to sublist before index, set to this if bad index [out]
     \param  right  %Set to sublist after index, null if bad index [out]
     \return        Whether successful, false if bad index
    */
    template<class T1,class T2>
    bool splitat(Key index, T1& left, T2& right) const {
        bool result;
        if (index >= size_) {
            left.set(*this);
            right.set();
            result = false;
        } else {
            left.set(*this, 0, index);
            right.set(*this, index+1, ALL);
            result = true;
        }
        return result;
    }

    /** Split into left sublist at index.
     - Can use methods like find() or findany() for index
     - Template types are automatically deduced from arguments
     - For more advanced parsing see StrTok
     .
     \tparam  T1    %List type to store left sublist

     \param  index  Index to split at, bad index (NONE or out-of-bounds) splits at end
     \param  left   %Set to sublist before index, set to this if bad index [out]
     \return        Whether successful, false if bad index
    */
    template<class T1>
    bool splitat(Key index, T1& left) const {
        bool result;
        if (index >= size_) {
            left.set(*this);
            result = false;
        } else {
            left.set(*this, 0, index);
            result = true;
        }
        return result;
    }

    /** Split into right sublist at index.
     - Can use methods like find() or findany() for index
     - Template types are automatically deduced from arguments
     - For more advanced parsing see StrTok
     .
     \tparam  T2    %List type to store right sublist

     \param  index  Index to split at, bad index (NONE or out-of-bounds) splits at end
     \param  left   vNULL (ignored)
     \param  right  %Set to sublist after index, null if bad index [out]
     \return        Whether successful, false if bad index
    */
    template<class T2>
    bool splitat(Key index, ValNull left, T2& right) const {
        EVO_PARAM_UNUSED(left);
        bool result;
        if (index >= size_) {
            right.set();
            result = false;
        } else {
            right.set(*this, index+1, ALL);
            result = true;
        }
        return result;
    }

    // SPLIT_SET

    /** Split at index and set as left sublist.
     - Can use methods like find() or findany() for index
     - Sets this to sublist before index, unchanged if bad index [out]
     .
     \param  index  Index to split at, bad index (NONE or out-of-bounds) splits at end (i.e. unchanged)
     \return        Whether successful, false if bad index
    */
    bool splitat_setl(Key index) {
        bool result;
        if (index >= size_) {
            result = false;
        } else {
            slice(0, index);
            result = true;
        }
        return result;
    }

    /** Split at index, set as left sublist, and save right sublist.
     - Can use methods like find() or findany() for index
     - Sets this to sublist before index, unchanged if bad index [out]
     .
     \param  index  Index to split at, bad index (NONE or out-of-bounds) splits at end (i.e. unchanged)
     \param  right  %Set to sublist after index, null if bad index [out]
     \return        Whether successful, false if bad index
    */
    template<class T2>
    bool splitat_setl(Key index, T2& right) {
        bool result;
        if (index >= size_) {
            right.set();
            result = false;
        } else {
            right.set(*this, index+1, ALL);
            slice(0, index);
            result = true;
        }
        return result;
    }

    /** Split at index and set as right sublist.
     - Can use methods like find() or findany() for index
     - Sets this to sublist after index, null if bad index [out]
     .
     \param  index  Index to split at, bad index (NONE or out-of-bounds) splits at end
     \return        Whether successful, false if bad index
    */
    bool splitat_setr(Key index) {
        bool result;
        if (index >= size_) {
            set();
            result = false;
        } else {
            slice(index+1, ALL);
            result = true;
        }
        return result;
    }

    /** Split at index, set as right sublist, and save left sublist.
     - Can use methods like find() or findany() for index
     - Sets this to sublist after index, null if bad index [out]
     .
     \param  index  Index to split at, bad index (NONE or out-of-bounds) splits at end
     \param  left   %Set to sublist before index, set to this if bad index [out]
     \return        Whether successful, false if bad index
    */
    template<class T1>
    bool splitat_setr(Key index, T1& left) {
        bool result;
        if (index >= size_) {
            left.set(*this);
            set();
            result = false;
        } else {
            left.set(*this, 0, index);
            slice(index+1, ALL);
            result = true;
        }
        return result;
    }

    // TRIM

    /** Trim left (beginning) items.
     - This non-destructively trims beginning items
     .
     \param  size  Trim size as item count to remove
     \return       This
    */
    ListType& triml(Size size) {
        if (size > size_)
            size = size_;
        if (size > 0) {
            size_ -= size;
            data_ += size;
        }
        return *this;
    }

    /** Trim right (ending) items.
     - This non-destructively trims ending items
     .
     \param  size  Trim size as item count to remove
     \return       This
    */
    ListType& trimr(Size size) {
        if (size > 0) {
            if (size < size_)
                size_ -= size;
            else
                size_ = 0;
            #if EVO_LIST_OPT_REFTERM
                terminated_ = false;
            #endif
        }
        return *this;
    }

    /** Truncate to given size.
     - This non-destructively trims ending items
     .
     \param  size  Size to truncate to as item count
     \return       This
    */
    ListType& truncate(Size size=0) {
        if (size < size_) {
            size_       = size;
            #if EVO_LIST_OPT_REFTERM
                terminated_ = false;
            #endif
        }
        return *this;
    }

    // SLICE

    /** Slice beginning items.
     - This non-destructively trims beginning items
     .
     \param  index  Start index of new slice, END to slice (remove) all items from beginning
     \return        This
    */
    ListType& slice(Key index) {
        if (index > 0) {
            if (index >= size_) {
                // Empty end slice
                data_ = data_ + size_;
                size_ = 0;
            } else {
                // Normal slice
                data_  = data_ + index;
                size_ -= index;
            }
        }
        return *this;
    }

    /** Slice to given sublist.
     - This non-destructively trims beginning and/or ending items
     .
     \param  index  Start index of new slice, END to slice (remove) all items from beginning
     \param  size   Slice size as item count, ALL for all from index
     \return        This
    */
    ListType& slice(Key index, Size size) {
        if (index > 0) {
            if (index >= size_) {
                // Empty end slice
                size_ = 0;
                #if EVO_LIST_OPT_REFTERM
                    terminated_ = false;
                #endif
            } else {
                // Normal slice
                data_ = data_ + index;
                size_ -= index;
                if (size < size_) {
                    size_       = size;
                    #if EVO_LIST_OPT_REFTERM
                        terminated_ = false;
                    #endif
                }
            }
        } else if (size < size_) {
            size_       = size;
            #if EVO_LIST_OPT_REFTERM
                terminated_ = false;
            #endif
        }
        return *this;
    }

    /** Slice to given sublist using start/end positions.
     - This non-destructively trims beginning and/or ending items
     - If index2 < index1 then index2 will be set to index1 (empty sublist)
     .
     \param  index1  Start index of new slice, END to slice all items from beginning
     \param  index2  End index of new slice (this item not included), END for all after index1
     \return         This
    */
    ListType& slice2(Key index1, Key index2)
        { return slice(index1, (index1 < index2 ? index2-index1 : 0)); }

    /** Clean and remove hidden items previously removed via slicing (modifier).
     - Modify operations do this automatically as needed
     - This is mainly useful for cleanup before starting critical performance code
     .
     \return  This
    */
    ListType& unslice() {
        if (buf_.ptr != NULL && buf_.header->used > size_) {
            if (buf_.header->refs > 1) {
                // New buffer, was shared
                --buf_.header->refs;
                if (size_ > 0) {
                    assert( data_ != NULL );
                    buf_.ptr = (Item*)buf_.memalloc(Capacity::init(size_+1), size_, buf_.header);
                    DataInit<Item>::init(buf_.ptr, data_, size_);
                       data_ = buf_.ptr;
                } else {
                    buf_.header = NULL;
                    buf_.ptr    = NULL;
                    data_       = EVO_PEMPTY;
                }
            } else {
                // Existing buffer, unslice
                assert( buf_.header->refs == 1 );
                assert( buf_.header->used > 0 );
                unsliceBuffer(size_);
            }
            #if EVO_LIST_OPT_REFTERM
                terminated_ = false;
            #endif
        }
        return *this;
    }

    // INFO_SET

    /** Get data pointer (mutable).
     - Calls unshare()
     - \b Caution: Calling any modifier method like add() after this may (will) invalidate the returned pointer
     - For best performance, reuse returned pointer for repeated access, or use data() instead for read-only access
     .
     \return  Data pointer (mutable).
    */
    T* dataM()
        { unshare(); return data_; }

    /** Get item at position (mutable).
     - Calls unshare()
     - For best performance, use operator[]() or item() instead for read-only access, or dataM() to get mutable pointer for repeated access
     - \b Caution: Results are undefined if index is out of bounds -- though index is checked with assert()
     .
     \param  index  Item index
     \return        Given item (mutable)
    */
    T& operator()(Key index)
        { assert( index < size_ ); unshare(); return data_[index]; }

    /** Get item at position (mutable).
     - Calls unshare()
     - For best performance, use operator[]() or item() instead for read-only access,
       or dataM() to get mutable pointer for repeated access
     - \b Caution: Results are undefined if index is out of bounds -- though index is checked with assert()
     .
     \param  index  Item index
     \return        Given item (mutable)
    */
    T& itemM(Key index)
        { assert( index < size_ ); unshare(); return data_[index]; }

    /** %Set new capacity (modifier).
     - Consider using reserve() instead to allocate additional capacity in advance
     - Items that don't fit new capacity are removed, even if buffer isn't used
     - Effectively calls unshare() if buffer is shared
     - May still reference external data even though buffer is allocated
     - Use unshare() to force data to buffer
     .
     \param  size  New capacity
     \return       This
    */
    ListType& capacity(Size size) {
        #if !EVO_LIST_OPT_LAZYBUF
            // Lazy buffer disabled
            assert( (buf_.header != NULL) == (buf_.ptr != NULL) );
        #endif
        if (buf_.header != NULL) {
            #if EVO_LIST_OPT_LAZYBUF
                if (buf_.ptr == NULL) {
                    // Existing buffer, unused
                    assert( buf_.header->used == 0 );
                    assert( buf_.header->refs == 1 );
                    if (buf_.header->size != size) {
                        if (size_ > size) {
                            size_ = size;
                            #if EVO_LIST_OPT_REFTERM
                                terminated_ = false;
                            #endif
                        }
                        buf_.memfree();
                        if (size > 0)
                            buf_.memalloc(size, 0, buf_.header);
                        else {
                            buf_.header = NULL;
                            if (data_ != NULL)
                                data_ = EVO_PEMPTY;
                        }
                    }
                } else
            #else
                // Lazy buffer disabled
                assert( buf_.ptr != NULL );
            #endif
            if (buf_.header->refs == 1) {
                // Existing buffer
                if (buf_.header->size != size) {
                    if (size > 0) {
                        if (buf_.header->used > 0) {
                            unsliceBuffer(size_);
                            if (size < size_) {
                                // Shrinking capacity, removing items
                                DataInit<T>::uninit(buf_.ptr+size, buf_.header->used-size);
                                buf_.ptr          = buf_.memrealloc(size);
                                buf_.header->used = size;
                                size_             = size;
                                #if EVO_LIST_OPT_REFTERM
                                    terminated_ = false;
                                #endif
                            } else
                                // Resizing without affecting items
                                buf_.ptr = buf_.memrealloc(size);
                            data_ = buf_.ptr;
                        } else {
                            // Empty, resizing buffer
                            buf_.ptr = buf_.memrealloc(size);
                            if (data_ != NULL)
                                data_ = buf_.ptr;
                        }
                    } else {
                        buf_.free();
                        buf_.clear();
                        if (data_ != NULL)
                            data_ = EVO_PEMPTY;
                        size_ = 0;
                        #if EVO_LIST_OPT_REFTERM
                            terminated_ = false;
                        #endif
                    }
                }
            } else {
                // Shared buffer
                assert( buf_.header->refs > 1);
                --buf_.header->refs;
                if (size > 0) {
                    if (size_ > size)
                        size_ = size;
                    buf_.ptr = buf_.memalloc(size, size_, buf_.header);
                    if (size_ > 0)
                        DataInit<T>::init(buf_.ptr, data_, size_);
                    data_ = buf_.ptr;
                } else {
                    assert( data_ != NULL );
                    buf_.clear();
                    data_ = EVO_PEMPTY;
                    size_ = 0;
                }
                #if EVO_LIST_OPT_REFTERM
                    terminated_ = false;
                #endif
            }
        } else {
            // New buffer, leave unused
            assert( buf_.ptr == NULL );
            if (size_ > size) {
                size_       = size;
                #if EVO_LIST_OPT_REFTERM
                    terminated_ = false;
                #endif
            }
            if (size > 0) {
                #if EVO_LIST_OPT_LAZYBUF
                    buf_.memalloc(size, 0, buf_.header);
                #else
                    // Lazy buffer disabled so use buffer
                    buf_.ptr = buf_.memalloc(size, size_, buf_.header);
                    if (size_ > 0) {
                        DataInit<T>::init(buf_.ptr, data_, size_);
                        data_ = buf_.ptr;
                    } else if (data_ != NULL)
                        data_ = buf_.ptr;
                    #if EVO_LIST_OPT_REFTERM
                        terminated_ = false;
                    #endif
                #endif
            } else if (data_ != NULL)
                data_ = EVO_PEMPTY;
        }
        assert( size_ <= size );
        assert( size > 0 || data_ == NULL || data_ == EVO_PEMPTY );
        #if !EVO_LIST_OPT_LAZYBUF
            // Lazy buffer disabled
            assert( (buf_.header != NULL) == (buf_.ptr != NULL) );
        #endif
        return *this;
    }

    /** %Set minimum capacity (modifier).
     - This increases capacity to given minimum if it's less
     - Effectively calls unshare() if capacity changes and buffer is shared
     - May still reference external data even though buffer is allocated
     - Use unshare() to force data to buffer
     .
     \param  min  Minimum capacity
     \return      This
    */
    ListType& capacitymin(Size min) {
        if (buf_.header == NULL)
            capacity(size_ > min ? size_ : min);
        else if (min > buf_.header->size)
            capacity(min);
        return *this;
    }

    /** %Set maximum capacity (modifier).
     - This reduces capacity to given maximum if it's greater
     - Items that don't fit new capacity are removed, even if buffer isn't used
     - Effectively calls unshare() if capacity changes and buffer is shared
     - May still reference external data even though buffer is allocated
     - Use unshare() to force data to buffer
     .
     \param  max  Maximum capacity
     \return      This
    */
    ListType& capacitymax(Size max) {
        if (buf_.header != NULL && buf_.header->size > max)
            capacity(max);
        else if (size_ > max) {
            size_ = max;
            #if EVO_LIST_OPT_REFTERM
                terminated_ = false;
            #endif
        }
        return *this;
    }

    /** Reduce capacity to fit current size (modifier).
     - Call to save memory when done expanding size
     - Ignored if shared, may want to call unshare() first
     .
     \return  This
    */
    ListType& compact() {
        if (buf_.header != NULL && buf_.header->refs == 1) {
            const Size min = size_ + CONSERVE;
            if (buf_.header->size > min)
                capacity(min);
        }
        return *this;
    }

    /** Reserve capacity for additional items (modifier).
     - Use to make buffer unique (not shared) and writable while reserving extra space
     - This effectively calls unshare(), though may still be sliced if prefer_realloc is true
     .
     \param  size            Size as item count to reserve
     \param  prefer_realloc  Advanced: True to prefer realloc for certain conditition where would otherwise unslice via alloc and move to new buffer
     \return                 This
    */
    ListType& reserve(Size size, bool prefer_realloc=false) {
        const Size minsize = size_ + size;
        if (buf_.header != NULL) {
            #if EVO_LIST_OPT_LAZYBUF
                if (buf_.ptr == NULL) {
                    if (buf_.header->size >= minsize) {
                        // Use previous buffer
                        assert( buf_.header->refs == 1 );
                        assert( buf_.header->used == 0 );
                        buf_.ptr = (Item*)(buf_.header + 1);
                        if (size_ > 0)
                            DataInit<Item>::init(buf_.ptr, data_, size_);
                        buf_.header->used = size_;
                        data_ = buf_.ptr;
                        #if EVO_LIST_OPT_REFTERM
                            terminated_ = false;
                        #endif
                        return *this;
                    } else {
                        // Previous buffer not big enough, free it
                        buf_.memfree();
                        buf_.header = NULL;
                    }
                } else
            #else
                // Lazy buffer disabled
                assert( buf_.ptr != NULL );
            #endif
            if (buf_.header->refs > 1) {
                // New buffer, was shared
                assert( buf_.ptr != NULL );
                --buf_.header->refs;
            } else {
                // Already unique, ensure buffer is ready, unslice if sliced
                assert( buf_.header->refs == 1 );
                assert( buf_.ptr != NULL );
                if (minsize > buf_.header->size) {
                    if (data_ < buf_.ptr) {
                        // Realloc previous buffer
                        data_ = buf_.ptr = buf_.memrealloc(minsize);
                    } else if (prefer_realloc || data_ == buf_.ptr) {
                        // Either not sliced or prefer realloc
                        assert( data_ >= buf_.ptr && data_ <= buf_.ptr+buf_.header->used );
                        const Size offset = (Size)(data_ - buf_.ptr);
                        buf_.ptr = buf_.memrealloc(minsize);
                        data_ = buf_.ptr + offset;
                    } else
                        goto newbuf;    // unslice and move to new buffer (below)
                } else if (data_ <= EVO_PEMPTY)
                    data_ = buf_.ptr;
                return *this;
            }
        }

        // New buffer
        if (minsize > 0) {
        newbuf:
            if (size_ > 0) {
                buf_.ptr = buf_.memalloc(Capacity::init(minsize+1), size_, buf_.header);
                DataInit<T>::init(buf_.ptr, data_, size_);
                data_ = buf_.ptr;
            } else
                data_ = buf_.ptr = buf_.memalloc(Capacity::init(minsize+1), 0, buf_.header);
            #if EVO_LIST_OPT_REFTERM
                terminated_ = false;
            #endif
        } else {
            assert( buf_.header == NULL && buf_.ptr == NULL );
        }
        return *this;
    }

    /** Make data unique by allocating new buffer, if needed (modifier).
     - Use reserve() instead to reserve additional space while unsharing
     - Use to make buffer unique (not shared) and writable (when not empty)
     - This is called automatically by mutable/modifier methods
     - This does nothing if empty or not shared
     .
     \return  This
    */
    ListType& unshare() {
        if (buf_.header != NULL) {
            #if EVO_LIST_OPT_LAZYBUF
                if (buf_.ptr == NULL) {
                    if (buf_.header->size >= size_) {
                        // Use previous buffer
                        assert( buf_.header->refs == 1 );
                        assert( buf_.header->used == 0 );
                        buf_.ptr = (T*)(buf_.header + 1);
                        if (size_ > 0)
                            DataInit<T>::init(buf_.ptr, data_, size_);
                        buf_.header->used = size_;
                        data_ = buf_.ptr;
                        #if EVO_LIST_OPT_REFTERM
                            terminated_ = false;
                        #endif
                        return *this;
                    } else {
                        // Previous buffer not big enough, free it
                        buf_.memfree();
                        buf_.header = NULL;
                    }
                } else
            #else
                // Lazy buffer disabled
                assert( buf_.ptr != NULL );
            #endif
            if (buf_.header->refs > 1) {
                // New buffer, was shared
                assert( buf_.ptr != NULL );
                --buf_.header->refs;
            } else {
                // Already unique
                assert( buf_.header->refs == 1 );
                assert( buf_.ptr != NULL );
                if (data_ <= EVO_PEMPTY)
                    data_ = buf_.ptr;
                return *this;
            }
        }

        // New buffer
        if (size_ > 0) {
            assert( data_ != NULL );
            buf_.ptr = (T*)buf_.memalloc(Capacity::init(size_+1), size_, buf_.header);
            DataInit<T>::init(buf_.ptr, data_, size_);
               data_ = buf_.ptr;
            #if EVO_LIST_OPT_REFTERM
                terminated_ = false;
            #endif
        } else {
            assert( buf_.header == NULL && buf_.ptr == NULL );
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
     \param  size  New size as item count
     \return       This
    */
    ListType& resize(Size size) {
        if (size == 0) {
            clear();
            capacity(0);
        } else {
            if (buf_.header != NULL) {
                #if EVO_LIST_OPT_LAZYBUF
                    if (buf_.ptr == NULL) {
                        // Use previous buffer
                        assert( buf_.header->used == 0 );
                        assert( buf_.header->refs == 1 );
                        buf_.ptr = (T*)(buf_.header+1);
                        if (size > buf_.header->size)
                            // Grow previous buffer
                            buf_.memrealloc(size);
                        if (size <= size_) {
                            DataInit<T>::init(buf_.ptr, data_, size);
                        } else {
                            if (size_ > 0)
                                DataInit<T>::init(buf_.ptr, data_, size_);
                            DataInit<T>::init_tail_safe(buf_.ptr, size_, size);
                        }
                        data_ = buf_.ptr;
                        buf_.header->used = size_ = size;
                        return *this;
                    } else
                #else
                    // Lazy buffer disabled
                    assert( buf_.ptr != NULL );
                #endif
                if (buf_.header->refs == 1) {
                    // Existing buffer
                    if (buf_.header->used > 0)
                        unsliceBuffer(size < size_ ? size : size_);
                    if (size > buf_.header->size)
                        // Grow buffer
                        buf_.ptr = buf_.memrealloc(size);
                    if (buf_.header->used < size) {
                        // Add items
                        DataInit<T>::init_tail_safe(buf_.ptr, buf_.header->used, size);
                        buf_.header->used = size;
                    }
                    data_ = buf_.ptr;
                    size_ = size;
                    return *this;
                } else {
                    // New buffer, was shared
                    assert( buf_.header->refs > 1 );
                    --buf_.header->refs;
                }
            }

            // New buffer
            if (size_ > 0) {
                assert( data_ != NULL );
                buf_.ptr = (T*)buf_.memalloc(Capacity::init(size+1), size, buf_.header);
                DataInit<T>::init_safe(buf_.ptr, size, data_, size_);
                data_ = buf_.ptr;
            } else {
                data_ = buf_.ptr = (T*)buf_.memalloc(Capacity::init(size+1), size, buf_.header);
                DataInit<T>::init_safe(data_, size);
            }
            size_ = size;
            #if EVO_LIST_OPT_REFTERM
                terminated_ = false;
            #endif
        }
        return *this;
    }

    // COPY

    /** %Set as full (unshared) copy using data pointer (modifier).
     - Effectively calls unshare()
     .
     \param  data  Data to copy
     \param  size  Data size as item count
     \return       This
    */
    ListType& copy(const Item* data, Size size) {
        if (buf_.header != NULL) {
            if (buf_.header->refs > 1) {
                // New buffer, was shared
                --buf_.header->refs;
            } else {
                // Existing buffer
                assert( buf_.header->refs == 1 );
                buf_.ptr = (T*)(buf_.header + 1);
                if (buf_.header->used > 0) {
                    DataInit<T>::uninit(buf_.ptr, buf_.header->used);
                    buf_.header->used = 0;
                }
                if (size > buf_.header->size)
                    // Grow buffer
                    buf_.memrealloc(size);
                if (size > 0) {
                    DataInit<T>::init(buf_.ptr, data, size);
                    buf_.header->used = size;
                    data_ = buf_.ptr;
                } else
                    data_ = EVO_PEMPTY;
                size_ = size;
                return *this;
            }
        }

        // New buffer
        if (size > 0) {
            assert( data != NULL );
            data_ = buf_.ptr = (T*)buf_.memalloc(Capacity::init(size+1), size, buf_.header);
            DataInit<T>::init(buf_.ptr, data, size);
        } else {
            buf_.header = NULL;
            buf_.ptr    = NULL;
            data_       = EVO_PEMPTY;
        }
        #if EVO_LIST_OPT_REFTERM
            terminated_ = false;
        #endif
        size_ = size;
        return *this;
    }

    /** %Set as full (unshared) copy of another list (modifier).
     - Effectively calls unshare()
     .
     \param  data  Data to copy
     \return       This
    */
    ListType& copy(const ListBaseType& data) {
        if (data.data_ == NULL)
            set();
        else
            copy(data.data_, data.size_);
        return *this;
    }

    // ADD

    /** Append new items (modifier).
     - Effectively calls unshare()
     - Removes items sliced from end -- see unslice()
     .
     \param  size  Size as item count to append
     \return       This
    */
    ListType& addnew(Size size=1)
        { modAppend(EVO_PDEFAULT, size); return *this; }

    /** Append new items copied from data pointer (modifier).
     - Effectively calls unshare()
     - Removes items sliced from end -- see unslice()
     .
     \param  data   Data to append
     \param  size   Size as item count to append
     \return        This
    */
    ListType& add(const Item* data, Size size)
        { modAppend(data, size); return *this; }

    /** Append new items copied from another list (modifier).
     - Effectively calls unshare()
     - Removes items sliced from end -- see unslice()
     .
     \param  data  Data to append
     \return       This
    */
    ListType& add(const ListBaseType& data)
        { modAppend(data.data_, data.size_); return *this; }

    /** Append new item (modifier).
     - Effectively calls unshare()
     - Removes items sliced from end -- see unslice()
     .
     \param  data  Data to append
     \return       This
    */
    ListType& add(const Item& data)
        { modAppend(&data, 1); return *this; }

    /** Append operator.
     - Same as add(const Item&)
     - Append operators can be chained\n
       Example:
       \code
        // Clear character list and append two characters
        list << vEMPTY << 'a' << 'b';
       \endcode
     .
     \param  data  Data to append
     \return       This
    */
    ListType& operator<<(const Item& data)
        { return add(data); }

    /** Append operator.
     - Same as add(const ListBaseType&)
     - Append operators can be chained\n
       Example:
       \code
        // Clear list and append two other lists
        list << vEMPTY << list2 << list3;
       \endcode
     .
     \param  data  Data to append
     \return       This
    */
    ListType& operator<<(const ListBaseType& data)
        { return add(data); }

    /** Append operator to set as null and empty.
     - Same as set()
     - Append operators can be chained\n
       Example:
       \code
        // Clear character list and append two characters
        list << vNULL << 'a' << 'b';
       \endcode
     .
     \param  val  vNULL
     \return      This
    */
    ListType& operator<<(const ValNull& val) {
        EVO_PARAM_UNUSED(val);
        clear(); data_ = NULL; return *this;
    }

    /** Append operator to set as empty but not null.
     - Append operators can be chained\n
       Example:
       \code
        // Clear character list and append two characters
        list << vEMPTY << 'a' << 'b';
       \endcode
     .
     \param  val  vEMPTY
     \return      This
    */
    ListType& operator<<(const ValEmpty& val) {
        EVO_PARAM_UNUSED(val);
        clear(); data_ = EVO_PDEFAULT; return *this;
    }

    // PREPEND

    /** Prepend new items (modifier).
     - Effectively calls unshare()
     - Removes items sliced from beginning -- see unslice()
     .
     \param  size  Size as item count to prepend
     \return       This
    */
    ListType& prependnew(Size size=1)
        { modPrepend(EVO_PDEFAULT, size); return *this; }

    /** Prepend new items copied from data pointer (modifier).
     - Effectively calls unshare()
     - Removes items sliced from beginning -- see unslice()
     .
     \param  data  Data to prepend
     \param  size  Size as item count to prepend
     \return       This
    */
    ListType& prepend(const Item* data, Size size)
        { modPrepend(data, size); return *this; }

    /** Prepend new items copied from another list (modifier).
     - Effectively calls unshare()
     - Removes items sliced from beginning -- see unslice()
     .
     \param  data  Data to prepend
     \return       This
    */
    ListType& prepend(const ListBaseType& data)
        { modPrepend(data.data_, data.size_); return *this; }

    /** Prepend new item (modifier).
     - Removes items sliced from beginning -- see unslice()
     .
     \param  data  Data to prepend
     \return       This
    */
    ListType& prepend(const Item& data)
        { modPrepend(&data, 1); return *this; }

    // INSERT

    /** Insert new items (modifier).
     \param  index  Insert index, END to append
     \param  size   Size as item count to insert
     \return        Inserted index
    */
    Size insertnew(Key index, Size size=1)
        { return modInsert(index, EVO_PDEFAULT, size); }

    /** Insert new items copied from data pointer (modifier).
     \param  index  Insert index, END to append
     \param  data   Data to insert
     \param  size   Size as item count to insert
     \return        Inserted index
    */
    Size insert(Key index, const Item* data, Size size)
        { return modInsert(index, data, size); }

    /** Insert new items copied from another list (modifier).
     \param  index  Insert index, END to append
     \param  data   Data to insert
     \return        Inserted index
    */
    Size insert(Key index, const ListBaseType& data)
        { return modInsert(index, data.data_, data.size_); }

    /** Insert new item (modifier).
     \param  index  Insert index, END to append
     \param  data   Data to insert
     \return        Inserted index
    */
    Size insert(Key index, const Item& data)
        { return modInsert(index, &data, 1); }

    // REMOVE

    /** Remove items (modifier).
     \param  index  Remove index
     \param  size   Remove size, ALL for all items from index
     \return        Number of items removed
    */
    Size remove(Key index, Size size=1)
        { return modRemove(index, size); }

    // POP

    /** Pop a copy of given item (modifier).
     - This copies given item then removes the original
     .
     \param  item   Stores popped item [out]
     \param  index  Index to pop
     \return        Whether successful, false if empty or bad index
    */
    bool pop(T& item, Key index) {
        bool result;
        if (index < size_) {
            DataInit<T>::copy(&item, data_+index, 1);
            modRemove(index, 1);
            result = true;
        } else 
            result = false;
        return result;
    }

    /** Pop a copy of last item (stack) (modifier).
     - This copies last item then removes the original
     - For best performance use pop() to avoid the copy operation
     .
     \param  item  Stores popped item [out]
     \return       Whether successful, false if empty
    */
    bool pop(T& item) {
        bool result;
        if (size_ > 0) {
            const Size index = size_ - 1;
            DataInit<T>::copy(&item, data_+index, 1);
            modRemove(index, 1);
            result = true;
        } else 
            result = false;
        return result;
    }

    /** Pop last item (stack).
     - This slices out and pops the last item
     - \b Caution: Removing sliced item will invalidate the returned pointer -- see unslice() and unshare()
     .
     \return  Popped item pointer (const), NULL if empty
    */
    const Item* pop() {
        const Item* result;
        if (size_ > 0)
            result = data_ + --size_;
        else
            result = NULL;
        return result;
    }

    /** Pop a copy of first item (queue) (modifier).
     - This copies first item then removes the original
     - For best performance use popq() to avoid the copy operation
     .
     \param  item  Stores popped item [out]
     \return       Whether successful, false if empty
    */
    bool popq(T& item) {
        bool result;
        if (size_ > 0) {
            DataInit<T>::copy(&item, data_, 1);
            modRemove(0, 1);
            result = true;
        } else
            result = false;
        return result;
    }

    /** Pop first item (queue).
     - This slices out and pops the first item
     - \b Caution: Removing sliced item will invalidate the returned pointer -- see unslice() and unshare()
     .
     \return  Popped item pointer (const), NULL if empty
    */
    const Item* popq() {
        const Item* result;
        if (size_ > 0) {
            result = data_;
            data_  = data_ + 1;
            --size_;
        } else 
            result = NULL;
        return result;
    }

    // FILL

    /** Fill using item (modifier).
     - Calls unshare() if any filling done
     - Resizes to fill new items if needed
     .
     \param  item   Item to fill with
     \param  index  Start index, END to start at end and append
     \param  size   Size to fill as item count from index, ALL for all items from index, 0 to do nothing
    */
    ListType& fill(const Item& item, Key index=0, Size size=ALL) {
        if (index == END)
            index = size_;
        if (size == ALL)
            size = (index < size_ ? size_ - index : 0);
        if (size > 0) {
            const Size newsize = index + size;
            if (newsize > size_)
                advResize(newsize);
            else
                unshare();
            DataFill<T>::fill(data_+index, size, item);
        }
        return *this;
    }

    // REPLACE

    /** Replace items with new data (modifier).
     \param  index  Start index to replace
     \param  rsize  Size as item count from index to replace, ALL for all items from index
     \param  data   Replacement data to copy
     \param  size   Replacement data size as item count
     \return        This
    */
    ListType& replace(Key index, Size rsize, const Item* data, Size size) {
        if (rsize == 0)
            modInsert(index, data, size);
        else if (size == 0)
            modRemove(index, rsize);
        else if (index >= size_)
            modAppend(data, size);
        else
            modReplace(index, rsize, data, size);
        return *this;
    }

    // MOVE / SWAP

    /** Move item to position (modifier).
     - This will remove item at index and insert it at dest, shifting items in between as needed
     - If item is moved left it will be inserted before item at dest
     - If item is moved right it will be inserted after item at dest, since items are shifted before insert
     .
     \param  dest   Destination index to move item to, END to move to end
     \param  index  Item index to move from
    */
    void move(Key dest, Key index) {
        if (index < size_) {
            if (dest >= size_)
                dest = size_ - 1;
            if (index != dest) {
                unshare();
                char buf[sizeof(T)];
                memcpy(buf, data_+index, sizeof(T));
                if (index > dest)
                    // Moving left, shift items right
                    memmove(data_+dest+1, data_+dest, (index-dest)*sizeof(T));
                else
                    // Moving right, shift items left
                    memmove(data_+index, data_+index+1, (dest-index)*sizeof(T));
                memcpy(data_+dest, buf, sizeof(T));
            }
        }
    }

    /** Move items from another list.
     - Effectively calls unshare() on both this and src, if any items moved
     .
     \param  dest      Destination index to insert moved items, END for append
     \param  src       Source to move from
     \param  srcindex  Source index to move items from
     \param  size      Size as item count to move from source
    */
    Size move(Key dest, ListType& src, Key srcindex=0, Size size=ALL) {
        const Size maxsize = (srcindex < src.size_ ? src.size_-srcindex : 0);
        if (size > maxsize)
            size = maxsize;
        if (size > 0) {
            if (dest > size_)
                dest = size_;
            const Size newused = size_ + size;
            if (buf_.header != NULL) {
                #if EVO_LIST_OPT_LAZYBUF
                    if (buf_.ptr == NULL) {
                        if (buf_.header->size >= newused) {
                            // Use previous buffer, there's room
                            assert( size_ > 0 );
                            buf_.ptr = (T*)(buf_.header + 1);
                            if (dest > 0)
                                DataInit<T>::init(buf_.ptr, data_, dest);
                            const Size nextindex = dest + size;
                            if (nextindex < newused)
                                DataInit<T>::init(buf_.ptr+nextindex, data_+dest, newused-nextindex);
                            buf_.header->used = newused;
                            data_             = buf_.ptr;
                            size_             = newused;
                            #if EVO_LIST_OPT_REFTERM
                                terminated_ = false;
                            #endif
                            goto movedata;
                        }
                    } else
                #else
                    // Lazy buffer disabled
                    assert( buf_.ptr != NULL );
                #endif
                if (buf_.header->refs == 1) {
                    // Existing buffer -- mostly copied from modInsertMid()
                    Size offset;
                    if (buf_.header->used > 0) {
                        assert( data_ >= buf_.ptr && data_ <= buf_.ptr+buf_.header->used );
                        offset = (Size)(data_ - buf_.ptr);

                        const Size tailsize = buf_.header->used - size_ - offset;
                        if (tailsize > 0) {
                            DataInit<T>::uninit(data_+size_, tailsize);
                            buf_.header->used -= tailsize;
                        }
                    } else {
                        offset = 0;
                        data_  = buf_.ptr;
                    }
                    if (newused > buf_.header->size) {
                        // Move to new bigger buffer
                        Size newbufsize = Capacity::grow(buf_.header->size);
                        if (newbufsize <= newused)
                            newbufsize = newused + 1; // Leave extra space
                        Header* newheader;
                        T* newbuf = buf_.memalloc(newbufsize, newused, newheader);
                        if (dest > 0)
                            memcpy(newbuf, data_, sizeof(T)*dest);
                        const Size tailsize = size_ - dest;
                        if (tailsize > 0)
                            memcpy(newbuf+dest+size, data_+dest, sizeof(T)*tailsize);
                        if (offset > 0)
                            DataInit<T>::uninit(buf_.ptr, offset);
                        buf_.memfree();
                        buf_.header = newheader;
                        buf_.ptr    = newbuf;
                        data_       = newbuf;
                    } else  {
                        if (size > offset) {
                            // Shift beginning and end to make room
                            if (offset > 0) {
                                DataInit<T>::uninit(buf_.ptr, offset);
                                if (dest > 0)
                                    memmove(buf_.ptr, data_, sizeof(T)*dest);
                                data_ = buf_.ptr;
                            }
                            const Size tailsize = size_ - dest;
                            if (tailsize > 0)
                                memmove(data_+dest+size, data_+dest+offset, sizeof(T)*tailsize);
                            buf_.header->used = newused;
                        } else {
                            // Shift beginning to make room
                            const Size newoffset = offset - size;
                            data_ = buf_.ptr + newoffset;
                            DataInit<T>::uninit(buf_.ptr, offset-newoffset);
                            if (dest > 0)
                                memmove(data_, buf_.ptr+offset, sizeof(T)*dest);
                        }
                    }
                    size_ = newused;
                    goto movedata;
                }
            }

            // New buffer
            Header* newheader;
            T* newbuf = buf_.memalloc(newused, newheader);
            if (size_ > 0) {
                if (dest > 0)
                    DataInit<T>::init(newbuf, data_, dest);
                const Size nextindex = dest + size;
                if (nextindex < newheader->used)
                    DataInit<T>::init(newbuf+nextindex, data_+dest, newheader->used-nextindex);
            }
            data_ = buf_.replace(newbuf, newheader);
            size_ = buf_.header->used;
            #if EVO_LIST_OPT_REFTERM
                terminated_ = false;
            #endif
            goto movedata;
        }
        return 0;

    movedata:
        if (src.buf_.ptr != NULL && src.buf_.header->refs == 1) {
            // Move from src buffer, is unique
            memcpy(data_+dest, src.data_+srcindex, sizeof(T)*size);
            src.modRemove(srcindex, size, false);
        } else {
            // Copy and remove from src
            meminit(data_+dest, src.data_+srcindex, size);
            src.modRemove(srcindex, size, true);
        }
        return size;
    }

    /** Swap items.
     - Calls unshare() and does swap, if index1 and index2 are valid and not the same
     .
     \param  index1  Index of first item to swap
     \param  index2  Index of second item to swap
    */
    void swap(Key index1, Key index2) {
        if (index1 != index2 && index1 < size_ && index2 < size_) {
            unshare();
            advSwap(index1, index2);
        }
    }

    /** Swap with another list.
     - This swaps internal state directly so is faster than moving items
     .
     \param  list  %List to swap with
    */
    void swap(ListType& list)
        { EVO_IMPL_CONTAINER_SWAP(this, &list, ThisType); }

    // ALGS

    /** Reverse item order (modifier).
     - Calls unshare()
     .
     \return  This
    */
    ListType& reverse() {
        if (size_ > 0) {
            unshare();
            char temp[sizeof(T)];
            T* left = data_;
            T* right = data_ + (size_ - 1);
            while (left < right) {
                memcpy(temp, left, sizeof(T));
                memcpy(left, right, sizeof(T));
                memcpy(right, temp, sizeof(T));
                ++left;
                --right;
            }
        }
        return *this;
    }

    // ADVANCED

    /** Advanced: Resize while preserving existing data, POD items not initialized (modifier).
     - This is a slightly modified version of resize():
       - No difference on non POD item types
       - For POD item type: New items created here will not be initialized/zeroed
       - This gives a slight performance increase in some cases but will leave uninitialized garbage data for POD items
       - In most cases resize() is preferred since it's safer and the performance difference is usually negligible
       .
     - This adds/removes items as needed until given size is reached
     - Effectively calls unshare()
     - \b Advanced: See advBuffer() for getting writable pointer to buffer
     .
     \param  size  New size as item count
     \return       This
    */
    ListType& advResize(Size size) {
        // Copied from resize(), only difference is init() and init_tail_fast()
        if (size == 0) {
            clear();
            capacity(0);
        } else {
            if (buf_.header != NULL) {
                #if EVO_LIST_OPT_LAZYBUF
                    if (buf_.ptr == NULL) {
                        // Use previous buffer
                        assert( buf_.header->used == 0 );
                        assert( buf_.header->refs == 1 );
                        buf_.ptr = (T*)(buf_.header+1);
                        if (size > buf_.header->size)
                            // Grow previous buffer
                            buf_.memrealloc(size);
                        if (size <= size_) {
                            DataInit<T>::init(buf_.ptr, data_, size);
                        } else {
                            if (size_ > 0)
                                DataInit<T>::init(buf_.ptr, data_, size_);
                            DataInit<T>::init_tail_fast(buf_.ptr, size_, size);
                        }
                        data_ = buf_.ptr;
                        buf_.header->used = size_ = size;
                        return *this;
                    } else
                #else
                    // Lazy buffer disabled
                    assert( buf_.ptr != NULL );
                #endif
                if (buf_.header->refs == 1) {
                    // Existing buffer
                    if (buf_.header->used > 0)
                        unsliceBuffer(size < size_ ? size : size_);
                    if (size > buf_.header->size)
                        // Grow buffer
                        buf_.ptr = buf_.memrealloc(size);
                    if (buf_.header->used < size) {
                        // Add items
                        DataInit<T>::init_tail_fast(buf_.ptr, buf_.header->used, size);
                        buf_.header->used = size;
                    }
                    data_ = buf_.ptr;
                    size_ = size;
                    return *this;
                } else {
                    // New buffer, was shared
                    assert( buf_.header->refs > 1 );
                    --buf_.header->refs;
                }
            }

            // New buffer
            if (size_ > 0) {
                assert( data_ != NULL );
                buf_.ptr = (T*)buf_.memalloc(Capacity::init(size+1), size, buf_.header);
                DataInit<T>::init(buf_.ptr, size, data_, size_);
                data_ = buf_.ptr;
            } else {
                data_ = buf_.ptr = (T*)buf_.memalloc(Capacity::init(size+1), size, buf_.header);
                DataInit<T>::init(data_, size);
            }
            size_ = size;
            #if EVO_LIST_OPT_REFTERM
                terminated_ = false;
            #endif
        }
        return *this;
    }

    /** Advanced: Start optimized in-place/buffer edit.
     - This approach optimizes by selecting the best case for editing (in-place or new buffer), and the caller must handle each case correctly
     - For easier approach use advBuffer(Size) and always edit in-place -- but is less efficient in certain cases, like when buffer grows
     - If the current buffer is unique and big enough (and beginning isn't sliced, and inplace flag is set):
       - This sets 'edit.ptr' and 'edit.size' to current buffer and size, and returns true to edit in-place
       - Modify buffer as needed via 'edit.ptr', update 'edit.size'
       - Call advEditDone() when done to adjust to new size
       - Note: If you don't call advEditDone() then in-place edits will still remain and the previous size will be used
       .
     - Otherwise a new empty buffer is created:
       - This sets 'edit.ptr' to a new buffer and 'edit.size' to 0, and returns false
       - Write to buffer in 'edit.ptr' as needed, copy from data() or use edit.write() methods if needed, update 'edit.size'
       - Call advEditDone() when done to use new buffer and new size
       - Note: If you don't call advEditDone() then Edit destructor will still free the new buffer (no memory leak)
       .
     - This is mainly intended for POD types
     - \b Caution: Results are undefined if another modifier method is used during an edit
     - \b Caution: Results are undefined if edit buffer is accessed after minsize
     - \b Caution: For non-POD type, results are undefined if you don't initialize new items or don't uninitialize removed items correctly
     .
     \param  edit     Edit buffer data, use 'edit.ptr'
     \param  minsize  Minimum buffer size needed, must be positive
     \param  inplace  Whether to try in-place edit, false to always use new buffer
     \return          Whether editing in-place, false if a new empty buffer was created
    */
    bool advEdit(Edit& edit, Size minsize, bool inplace=true) {
        assert( minsize > 0 );
        edit.clear();
        if (buf_.header != NULL) {
            #if EVO_LIST_OPT_LAZYBUF
                if (buf_.ptr == NULL) {
                    if (buf_.header->size < minsize) {
                        // Previous buffer not big enough, free it
                        buf_.memfree();
                        buf_.header = NULL;
                        goto newbuf;
                    } else {
                        // Previous buffer is big enough, detach and use as new buffer
                        assert( buf_.header->used == 0 );
                        assert( buf_.header->refs == 1 );
                        edit.ptr    = (T*)(buf_.header+1);
                        edit.header = (void*)buf_.header;
                        edit.size   = 0;
                        buf_.ptr    = NULL;
                        buf_.header = NULL;
                        return false;
                    }
                }
            #else
                // Lazy buffer disabled
                assert( buf_.ptr != NULL );
            #endif

            // If unique buffer is big enough and beginning isn't sliced, stop here to edit in-place
            if (inplace && buf_.header->refs == 1 && buf_.header->size >= minsize && data_ == buf_.ptr) {
                edit.ptr  = buf_.ptr;
                edit.size = size_;
                return true;
            }
        }

    newbuf:
        // Create new edit buffer
        edit.ptr  = (T*)buf_.memalloc(Capacity::init(minsize+1), minsize, (Header*&)edit.header);
        edit.size = 0;
        return false;
    }

    /** Advanced: Finish edit started with advEdit() (modifier).
     - Must call advEdit() first to start edit
     - This applies the pending edit and makes it permanent
     .
     \param  edit  Edit data set from advEdit()
    */
    void advEditDone(Edit& edit) {
        assert( edit.ptr != NULL );
        if (edit.header == NULL) {
            // Update size after in-place editing
            assert( buf_.header != NULL );
            size_ = buf_.header->used = edit.size;
        } else {
            // Switch to new buffer
            if (buf_.header != NULL && --buf_.header->refs == 0)
                buf_.memfree();
            buf_.header = (Header*)edit.header;
            buf_.ptr    = edit.ptr;
            data_       = buf_.ptr;
            size_       = buf_.header->used = edit.size;
            assert( buf_.header->refs == 1 );
            edit.header = NULL;
        }
        edit.ptr  = NULL;
        edit.size = 0;
    }

    /** Advanced: Resize and get buffer pointer (modifier).
     - Used for writing directly to buffer -- this calls advResize() first to make room
     - For best performance call clear() first
     - Call advSize() after writing to adjust size, if needed
     - \b Caution: Results are undefined if buffer is accessed after size
     - \b Caution: For non-POD type results are undefined if don't initialize new items or uninitialize removed items correctly
     .
     \param  size  Size to resize to
     \return       Buffer pointer
    */
    T* advBuffer(Size size) {
        advResize(size);
        return buf_.ptr;
    }

    /** Advanced: Get buffer pointer (modifier).
     - Used for writing directly to buffer
     - Buffer must be resized first to make room -- see advBuffer(Size), advResize(), resize()
     - Call advSize() after writing to adjust size, if needed
     - \b Caution: Data must be unique (not shared) when writing or results are undefined
     - \b Caution: Results are undefined if buffer is accessed out of bounds
     - \b Caution: For non-POD type results are undefined if don't initialize new items or uninitialize removed items correctly
     .
     \return  Buffer pointer, NULL for none
    */
    T* advBuffer()
        { return buf_.ptr; }

    /** Advanced: %Set new size after writing directly to buffer.
     - Used after calling advBuffer() and writing to buffer
     - \b Caution: Using this on non POD list can create/leave uninitialized items and undefined results
     - \b Caution: Data must be unique (not shared) when writing or results are undefined
     - \b Caution: No bounds checking is used on new size
     .
     \param  size  New size
    */
    void advSize(Size size) {
        assert( buf_.header != NULL );
        size_ = buf_.header->used = size;
    }

    /** Advanced: Get buffer pointer to write/append (modifier).
     - Used for appending by writing directly to buffer -- this calls unslice() and resrve() first to make room
     - Call advWriteDone() after writing to adjust actual size used, skip if nothing written
     - \b Caution: Results are undefined if buffer is accessed after addsize
     - \b Caution: For non-POD type results are undefined if don't initialize new items items correctly
     .
     \param  addsize  Max size to write/append, must be at least 1
     \return          Pointer to buffer to write to (after existing data)
    */
    T* advWrite(Size addsize) {
        assert( addsize > 0 );
        unslice();
        reserve(addsize);
        return buf_.ptr + buf_.header->used;
    }

    /** Advanced: Update size added after writing directly to buffer.
     - Used after calling advWrite() and writing to buffer
       - No need to call this if nothing written
     - \b Caution: Using this on non POD list can create uninitialized items and undefined results
     - \b Caution: Data must be unique (not shared) when writing or results are undefined -- advWrite() covers this
     - \b Caution: No bounds checking is used on addsize
     .
     \param  addsize  Actual size written
    */
    void advWriteDone(Size addsize) {
        assert( buf_.header != NULL );
        size_ = buf_.header->used += addsize;
    }

    /** Advanced: Get item (mutable).
     - \b Caution: Data must be unique (not shared) or results are undefined
     - \b Caution: Results are undefined if index is out of bounds -- though index is checked with assert()
     .
     \param  index  Item index
     \return        Given item
    */
    T& advItem(Key index) {
        assert( index < size_ );
        return data_[index];
    }

    /** Advanced: Append new items without initializing (constructing) them.
     - \b Caution: Data must be unique (not shared) or results are undefined
     - \b Caution: The new items must be initialized with DataInit::init_safe() or removed with advRemove() before items are accessed, modified, or list is destroyed
     - \b Caution: Results are undefined if new items are left uninitialized
     .
     \param  size  Size to append
    */
    void advAdd(Size size)
        { modAppend(NULL, size); }

    /** Advanced: Prepend new items without initializing (constructing) them.
     - \b Caution: Data must be unique (not shared) or results are undefined
     - \b Caution: The new items must be initialized with DataInit::init_safe() or removed with advRemove() before items are accessed, modified, or list is destroyed
     - \b Caution: Results are undefined if new items are left uninitialized
     .
     \param  size  Size to prepend
    */
    void advPrepend(Size size)
        { modPrepend(NULL, size); }

    /** Advanced: Insert new items without initializing (constructing) them.
     - \b Caution: Data must be unique (not shared) or results are undefined
     - \b Caution: The new items must be initialized with DataInit::init_safe() or removed with advRemove() before items are accessed, modified, or list is destroyed
     - \b Caution: Results are undefined if new items are left uninitialized
     .
     \param  index  Insert index (END for end of list)
     \param  size   Insert data size
     \return        Actual insert index
    */
    Key advInsert(Key index, Size size)
        { return modInsert(index, NULL, size); }

    /** Advanced: Remove given items without uninitializing (destructing) them.
     - \b Caution: Use only to remove uninitialized items -- use DataInit::uninit() to uninitialize items that have been initialized
     - \b Caution: Data must be unique (not shared) or results are undefined
     - \b Caution: Uninitialized items must be initialized with DataInit::init_safe() or removed with advRemove() before items are accessed, modified, or list is destroyed
     .
     \param  index  Remove index
     \param  size   Remove size as item count, ALL for all from index
    */
    void advRemove(Key index, Size size)
        { modRemove(index, size, false); }

    /** Advanced: Swap given items.
     - A faster version of swap() without bounds checking
     - \b Caution: Data must be unique (not shared) and indexes must be valid or results are undefined
     .
     \param  index1  Index of first item to swap
     \param  index2  Index of second item to swap
    */
    void advSwap(Key index1, Key index2) {
        assert( index1 < size_ && index2 < size_ );
        char buf[sizeof(T)];
        memcpy(buf, data_+index1, sizeof(T));
        memcpy(data_+index1, data_+index2, sizeof(T));
        memcpy(data_+index2, buf, sizeof(T));
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

    // Internal methods
    /** \cond impl */
    Size used() const
        { return (buf_.ptr == NULL ? size_ : buf_.header->used); }
    /** \endcond */

    // Testing methods
    /** \cond impl */
    #if EVO_UNIT_TEST_MODE
    bool utTerminated() const {
        #if EVO_LIST_OPT_REFTERM
            return terminated_;
        #else
            return false;
        #endif
    }
    Size utRefs() const
        { return (buf_.ptr == NULL ? 0 : buf_.header->refs); }
    const Item* utBuffer() const
        { return buf_.ptr; }
    void utSetEmptyBuffer(bool setempty, size_t size=1) {
        assert( size < IntegerT<Size>::MAX );
        resize((Size)size);
        DataInit<T>::uninit(buf_.ptr, (Size)size);
        buf_.header->used = 0;
        size_ = 0;
        if (setempty)
            data_ = EVO_PEMPTY;
    }
    void utSetUnusedBuffer(bool setempty, size_t size=1) {
        assert( size < IntegerT<Size>::MAX );
        resize((Size)size);
        DataInit<T>::uninit(buf_.ptr, (Size)size);
        buf_.header->used = 0;
        buf_.ptr = NULL;
        size_    = 0;
        if (setempty)
            data_ = EVO_PEMPTY;
    }
    void utSetBufPtr() {
        if (buf_.header != NULL)
            buf_.ptr = (T*)(buf_.header + 1);
    }
    #endif
    /** \endcond */

    // Iterator support methods
    /** \cond impl */
    void iterInitMutable()
        { unshare(); }
    const IterItem* iterFirst(IterKey& key) const {
        const IterItem* result;
        if (size_ > 0) {
            key    = 0;
            result = data_;
        } else {
            key    = END;
            result = NULL;
        }
        return result;
    }
    const IterItem* iterNext(IterKey& key) const {
        const IterItem* result = NULL;
        if (key != END) {
            if (++key < size_)
                result = data_ + key;
            else
                key = END;
        }
        return result;
    }
    const IterItem* iterNext(Size count, IterKey& key) const {
        const Item* result = NULL;
        if (key != END) {
            if ( (key+=count) < size_ )
                result = data_ + key;
            else
                key = END;
        }
        return result;
    }
    const IterItem* iterLast(IterKey& key) const {
        const IterItem* result;
        if (size_ > 0) {
            key    = size_ - 1;
            result = data_ + key;
        } else {
            key    = END;
            result = NULL;
        }
        return result;
    }
    const IterItem* iterPrev(IterKey& key) const {
        const IterItem* result = NULL;
        if (key != END) {
            if (key > 0)
                result = data_ + --key;
            else
                key = END;
        }
        return result;
    }
    const IterItem* iterPrev(Size count, IterKey& key) const {
        const IterItem* result = NULL;
        if (key != END) {
            if (key > 0 && count <= key)
                result = data_ + (key-=count);
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
            result = data_ + key;
        return result;
    }
    /** \endcond */

protected:
    /** List data header */
    struct Header {
        Size used;                  ///< Buffer size used/initialized as item count
        Size size;                  ///< Buffer size allocated as item count
        Size refs;                  ///< Buffer reference count
    };

    /** List buffer data helper. */
    struct Buf {
        Header* header;             ///< Data header pointer, NULL if no buffer allocated
        T*      ptr;                ///< Data pointer, NULL if buffer not used
        #if EVO_ALLOCATORS
            Allocator*  allocator;  ///< Allocator pointer (NULL for default or if not owner)
        #endif

        /** Constructor. */
        Buf() {
            header = NULL;
            ptr    = NULL;
        }

        /** Destructor. */
        ~Buf()
            { free(); }

        /** Clear buffer data.
         - Assumes buffer is not allocated
        */
        void clear() {
            header = NULL;
            ptr    = NULL;
        }

        /** Allocate new memory.
         - This doesn't modify the current buffer, use to setup replacement buffer
         .
         \param  size    New buffer size
         \param  used    Buffer used value to set
         \param  header  Stores pointer to header data [out]
         \return         Pointer to new buffer
        */
        T* memalloc(Size size, Size used, Header*& header) {
            assert( size > 0 );
            const Size bytes = sizeof(Header) + (size*sizeof(T));
            #if EVO_ALLOCATORS
                if (allocator == NULL)
                    header = (Header*)::malloc(bytes);
                else
                    header = (Header*)allocator->alloc(bytes);
            #else
                header = (Header*)::malloc(bytes);
            #endif
            assert( header != NULL );
            header->refs = 1;
            header->used = used;
            header->size = size;
            return (T*)(header + 1);
        }

        /** Allocate new memory.
         - Calls memalloc(Size,Size,Header*&)
         - This doesn't modify the current buffer, use to setup replacement buffer
         - This adjusts memory allocated using Capacity::init()
         .
         \param  size    New buffer size
         \param  header  Stores pointer to header data [out]
         \return         Pointer to new buffer
        */
        T* memalloc(Size size, Header*& header)
            { return memalloc(Capacity::init(size), size, header); }

        /** Reallocate buffer memory.
         - Assumes buffer is already allocated
         .
         \param  size  New buffer size - must be positive
         \return       Pointer to reallocated list buffer
        */
        T* memrealloc(Size size) {
            assert( (size_t)this->header > sizeof(Header) );
            assert( this->ptr != NULL );
            assert( size > 0 );
            const Size bytes = sizeof(Header) + (size*sizeof(T));
            #if EVO_ALLOCATORS
                if (allocator == NULL)
                    this->header = (Header*)::realloc(this->header, bytes);
                else
                    this->header = (Header*)allocator->realloc(this->header, bytes);
            #else
                this->header = (Header*)::realloc(this->header, bytes);
            #endif
            assert( this->header != NULL );
            this->ptr = (T*)(this->header+1);
            this->header->size = size;
            return this->ptr;
        }

        /** Free buffer memory.
         - Assumes buffer is allocated
        */
        void memfree() {
            assert( (size_t)header > sizeof(Header) );
            #if EVO_ALLOCATORS
                if (this->allocator == NULL)
                    ::free(header);
                else
                    this->allocator->free(header);
            #else
                ::free(header);
            #endif
        }

        /** Free and uninitialize allocated buffer. */
        void free() {
            if (header != NULL && --header->refs == 0) {
                if (header->used > 0)
                    DataInit<T>::uninit((T*)(header+1), header->used);
                memfree();
            }
        }

        /** Free and uninitialize current buffer and replace with new buffer.
         \param  newptr     New buffer pointer
         \param  newheader  New buffer header
         \return            New buffer pointer
        */
        T* replace(T* newptr, Header* newheader) {
            assert( newptr != NULL );
            assert( newheader != NULL );
            assert( newptr != ptr );
            free();
            header = newheader;
            ptr    = newptr;
            return ptr;
        }
    };

    Buf   buf_;                 ///< List buffer
    #if EVO_LIST_OPT_REFTERM
    bool  terminated_;          ///< Whether list data has a terminator
    #endif

    /** %Set as reference to another list.
     \param  data  Data to reference
    */
    void ref(const ListType& data) {
        if (data.data_ == NULL) {
            set();
        } else if (data.size_ == 0) {
            setempty();
        #if EVO_ALLOCATORS
        } else if (buf_.allocator != NULL && data.buf_.ptr != NULL) {
            copy(data);
        #endif
        } else if (data.buf_.ptr == NULL) {
            // External reference
            if (buf_.ptr != NULL) {
                assert( buf_.header != NULL );
                if (buf_.header->refs > 1) {
                    // Detach from shared buffer
                    --buf_.header->refs;
                    buf_.header = NULL;
                    buf_.ptr    = NULL;
                } else {
                    // Leave buffer for later use
                    assert( buf_.header->refs == 1 );
                    if (buf_.header->used > 0)
                        DataInit<T>::uninit(buf_.ptr, buf_.header->used);
                    #if EVO_LIST_OPT_LAZYBUF
                        buf_.header->used = 0;
                    #else
                        // Lazy buffer disabled -- can't leave for later use
                        buf_.memfree();
                        buf_.header = NULL;
                    #endif
                    buf_.ptr = NULL;
                }
            }
            data_ = data.data_;
            size_ = data.size_;
            #if EVO_LIST_OPT_REFTERM
                terminated_ = data.terminated_;
            #endif
        } else {
            // Shared
            assert( data.buf_.header != NULL );
            assert( data.buf_.ptr != NULL );
            buf_.free();
            buf_.header = data.buf_.header;
            buf_.ptr    = data.buf_.ptr;
            ++buf_.header->refs;
            data_ = data.data_;
            size_ = data.size_;
            #if EVO_LIST_OPT_REFTERM
                terminated_ = data.terminated_;
            #endif
        }
    }

    /** %Set as sliced reference to another list.
     \param  data   Data to reference
     \param  index  Start index of data, END to set as empty
     \param  size   Size as item count, ALL for all from index
    */
    void ref(const ListType& data, Size index, Size size) {
        if (data.data_ == NULL)
            set();
        else if (index >= data.size_)
            setempty();
        else {
            const Size max_size = (data.size_ - index);
            if (size > max_size)
                size = max_size;
            if (size == 0) {
                setempty();
            #if EVO_ALLOCATORS
            } else if (buf_.allocator != NULL && data.buf_.header != NULL) {
                copy(data.data_+index, size);
            #endif
            } else if (data.buf_.ptr == NULL) {
                // External reference
                if (buf_.ptr != NULL) {
                    assert( buf_.header != NULL );
                    if (buf_.header->refs > 1) {
                        // Detach from shared buffer
                        --buf_.header->refs;
                        buf_.header = NULL;
                        buf_.ptr    = NULL;
                    } else {
                        // Leave buffer for later use
                        assert( buf_.header->refs == 1 );
                        if (buf_.header->used > 0)
                            DataInit<T>::uninit(buf_.ptr, buf_.header->used);
                        #if EVO_LIST_OPT_LAZYBUF
                            buf_.header->used = 0;
                        #else
                            // Lazy buffer disabled -- can't leave for later use
                            buf_.memfree();
                            buf_.header = NULL;
                        #endif
                        buf_.ptr = NULL;
                    }
                }
                data_ = data.data_ + index;
                size_ = size;
                #if EVO_LIST_OPT_REFTERM
                    terminated_ = (data.terminated_ && size == max_size);
                #endif
            } else {
                // Shared
                assert( data.buf_.header != NULL );
                buf_.free();
                buf_.header = data.buf_.header;
                buf_.ptr    = data.buf_.ptr;
                data_       = data.data_ + index;
                size_       = size;
                #if EVO_LIST_OPT_REFTERM
                    terminated_ = data.terminated_;
                #endif
                ++buf_.header->refs;
            }
        }
    }

    /** %Set as reference to given data.
     \param  data  Data to reference
     \param  size  Data size
     \param  term  Whether referenced data is terminated
    */
    void ref(const Item* data, Size size, bool term=false) {
        if (data == NULL)
            set();
        else if (size == 0)
            setempty();
        else {
            // External reference
            if (buf_.ptr != NULL) {
                assert( buf_.header != NULL );
                if (buf_.header->refs > 1) {
                    // Detach from shared buffer
                    --buf_.header->refs;
                    buf_.header = NULL;
                    buf_.ptr    = NULL;
                } else {
                    // Leave buffer for later use
                    assert( buf_.header->refs == 1 );
                    if (buf_.header->used > 0)
                        DataInit<T>::uninit(buf_.ptr, buf_.header->used);
                    #if EVO_LIST_OPT_LAZYBUF
                        buf_.header->used = 0;
                    #else
                        // Lazy buffer disabled -- can't leave for later use
                        buf_.memfree();
                        buf_.header = NULL;
                    #endif
                    buf_.ptr = NULL;
                }
            }
            data_ = (T*)data;
            size_ = size;
            #if EVO_LIST_OPT_REFTERM
                terminated_ = term;
            #else
                EVO_PARAM_UNUSED(term);
            #endif
        }
    }

private:
    /** Used to conserve extra space for terminator when T is a char type (1 byte) */
    static const Size CONSERVE = (sizeof(T) == 1 ? 1 : 0);

    /** Unslice buffer.
     - This assumes a unique (not shared) buffer is allocated
     .
     \param  size  Current slice size to preserve, size_ or less
    */
    void unsliceBuffer(Size size) {
        assert( buf_.header != NULL && buf_.ptr >= (T*)(buf_.header+1) );
        assert( data_ >= buf_.ptr && data_ <= buf_.ptr+buf_.header->used );
        const Size offset = (Size)(data_ - buf_.ptr);

        const Size tailsize = buf_.header->used - size - offset;
        if (tailsize > 0) {
            DataInit<T>::uninit(data_+size, tailsize);
            buf_.header->used -= tailsize;
        }

        if (offset > 0) {
            DataInit<T>::uninit(buf_.ptr, offset);
            if (size > 0)
                memmove(buf_.ptr, buf_.ptr+offset, size*sizeof(T));
            data_ = buf_.ptr;
            buf_.header->used -= offset;
        }
    }

    /** Append new data. Calls unshare().
     \param  data  Append data, NULL to leave new memory uninitialized, PDEFAULT for default init
     \param  size  Append size
    */
    void modAppend(const Item* data, Size size) {
        if (size > 0) {
            const Size newused = size_ + size;
            if (buf_.header != NULL) {
                #if EVO_LIST_OPT_LAZYBUF
                    if (buf_.ptr == NULL) {
                        assert( buf_.header->refs == 1 );
                        if (buf_.header->size >= newused) {
                            // Use previous buffer
                            buf_.ptr = (T*)(buf_.header + 1);
                            if (size_ > 0)
                                DataInit<T>::init(buf_.ptr, data_, size_);
                            meminit(buf_.ptr+size_, data, size);
                            buf_.header->used = newused;
                            data_       = buf_.ptr;
                            size_       = newused;
                            #if EVO_LIST_OPT_REFTERM
                                terminated_ = false;
                            #endif
                            return;
                        }
                    } else
                #else
                    // Lazy buffer disabled
                    assert( buf_.ptr != NULL );
                #endif
                if (buf_.header->refs == 1) {
                    // Existing buffer
                    Size offset;
                    if (buf_.header->used > 0) {
                        assert( data_ >= buf_.ptr && data_ <= buf_.ptr+buf_.header->used );
                        offset = (Size)(data_ - buf_.ptr);

                        const Size tailsize = buf_.header->used - size_ - offset;
                        if (tailsize > 0) {
                            DataInit<T>::uninit(data_+size_, tailsize);
                            buf_.header->used -= tailsize;
                        }
                    } else
                        offset = 0;
                    if (newused > buf_.header->size) {
                        // Move to new bigger buffer
                        Size newbufsize = Capacity::grow(buf_.header->size);
                        if (newbufsize <= newused)
                            newbufsize = newused + 1; // Leave extra space
                        Header* newheader;
                        T* newbuf = buf_.memalloc(newbufsize, newused, newheader);
                        if (size_ > 0)
                            memcpy(newbuf, data_, sizeof(T)*size_);
                        if (offset > 0)
                            DataInit<T>::uninit(buf_.ptr, offset);
                        buf_.memfree();
                        buf_.header = newheader;
                        buf_.ptr    = newbuf;
                        data_       = newbuf;
                    } else if (offset > 0 && size > buf_.header->size-buf_.header->used) {
                        // Shift to make room at end
                        DataInit<T>::uninit(buf_.ptr, offset);
                        memmove(buf_.ptr, data_, sizeof(T)*size_);
                        buf_.header->used = newused;
                        data_             = buf_.ptr;
                    } else {
                        // Enough room at end
                        if (data_ < buf_.ptr) {
                            assert( offset == 0 );
                            data_ = buf_.ptr;
                        }
                        buf_.header->used += size;
                    }

                    // Write new items
                    meminit(data_+size_, data, size);
                    size_ = newused;
                    return;
                }
            }

            // New buffer
            Header* newheader;
            T* newbuf = buf_.memalloc(Capacity::init(newused+1), newused, newheader);
            if (size_ > 0)
                DataInit<T>::init(newbuf, data_, size_);
            meminit(newbuf+size_, data, size);
            newheader->used = newused;
            data_       = buf_.replace(newbuf, newheader);
            size_       = newused;
            #if EVO_LIST_OPT_REFTERM
                terminated_ = false;
            #endif
        } else if (data_ == NULL)
            data_ = EVO_PEMPTY;
    }

    /** Prepend new data. Calls unshare().
     \param  data  Prepend data, NULL to leave new memory uninitialized, PDEFAULT for default init
     \param  size  Prepend size
    */
    void modPrepend(const Item* data, Size size) {
        if (size > 0) {
            const Size newused = size_ + size;
            if (buf_.header != NULL) {
                #if EVO_LIST_OPT_LAZYBUF
                    if (buf_.ptr == NULL) {
                        if (buf_.header->size >= newused) {
                            // Use previous buffer
                            buf_.ptr = (T*)(buf_.header + 1);
                            if (size_ > 0)
                                DataInit<T>::init(buf_.ptr+size, data_, size_);
                            meminit(buf_.ptr, data, size);
                            buf_.header->used = newused;
                            data_ = buf_.ptr;
                            size_ = newused;
                            #if EVO_LIST_OPT_REFTERM
                                terminated_ = false;
                            #endif
                            return;
                        }
                    } else
                #else
                    // Lazy buffer disabled
                    assert( buf_.ptr != NULL );
                #endif
                if (buf_.header->refs == 1) {
                    // Existing buffer
                    Size offset;
                    if (buf_.header->used > 0) {
                        assert( data_ >= buf_.ptr && data_ <= buf_.ptr+buf_.header->used );
                        offset = (ulong)(data_ - buf_.ptr);

                        const Size tailsize = buf_.header->used - size_ - offset;
                        if (tailsize > 0) {
                            DataInit<T>::uninit(data_+size_, tailsize);
                            buf_.header->used -= tailsize;
                        }
                    } else
                        offset = 0;
                    if (size > offset) {
                        // Not enough room at beginning, cleanup and make room
                        if (newused > buf_.header->size-size_) {
                            // Move to new bigger buffer
                            Size newbufsize = Capacity::grow(buf_.header->size);
                            if (newbufsize <= newused)
                                newbufsize = newused + 1; // Leave extra space
                            Header* newheader;
                            T* newbuf = buf_.memalloc(newbufsize, newused, newheader);
                            if (size_ > 0)
                                memcpy(newbuf+size, data_, sizeof(T)*size_);
                            if (offset > 0)
                                DataInit<T>::uninit(buf_.ptr, offset);
                            buf_.memfree();
                            buf_.header = newheader;
                            buf_.ptr    = newbuf;
                            data_       = newbuf;
                        } else {
                            // Shift to make room at beginning
                            if (offset > 0)
                                DataInit<T>::uninit(buf_.ptr, offset);
                            memmove(buf_.ptr+size, data_, sizeof(T)*size_);
                            buf_.header->used = newused;
                            data_             = buf_.ptr;
                        }
                    } else {
                        // Enough room at beginning, cleanup
                        data_ = buf_.ptr + (offset - size);
                        DataInit<T>::uninit(data_, size);
                    }
                    size_ = newused;

                    // Write new items
                    meminit(data_, data, size);
                    return;
                }
            }

            // New buffer
            Header* newheader;
            T* newbuf = buf_.memalloc(newused, newheader);
            if (size_ > 0) {
                assert( data_ != NULL );
                DataInit<T>::init(newbuf+size, data_, size_);
            }
            meminit(newbuf, data, size);
            data_ = buf_.replace(newbuf, newheader);
            size_ = buf_.header->used;
            #if EVO_LIST_OPT_REFTERM
                terminated_ = false;
            #endif
        } else if (data_ == NULL)
            data_ = EVO_PEMPTY;
    }

    /** Insert new data. Calls unshare().
     \param  index  Insert index, END for end of list
     \param  data   Insert data, NULL to leave new memory uninitialized, PDEFAULT for default init
     \param  size   Insert size
     \return        Actual insert index, NONE if size is 0
    */
    Size modInsert(Size index, const Item* data, Size size) {
        if (index >= size_) {
            index = size_;
            modAppend(data, size);
        } else if (index == 0)
            modPrepend(data, size);
        else
            index = modInsertMid(index, data, size);
        return index;
    }

    /** Insert new data. Calls unshare(). Used by modInsert().
     \param  index  Insert index, END for end of list
     \param  data   Insert data, NULL to leave new memory uninitialized, PDEFAULT for default init
     \param  size   Insert size
     \return        Actual insert index, NONE if size is 0
    */
    Size modInsertMid(Size index, const Item* data, Size size) {
        assert( index > 0 && index < size_ );
        assert( size_ > 0 );
        if (size > 0) {
            if (buf_.header != NULL) {
                const Size newused = size_ + size;
                #if EVO_LIST_OPT_LAZYBUF
                    if (buf_.ptr == NULL) {
                        if (buf_.header->size >= newused) {
                            // Use previous buffer
                            buf_.ptr = (T*)(buf_.header + 1);
                            DataInit<T>::init(buf_.ptr, data_, index);
                            {
                                const Size nextindex = index + size;
                                assert( nextindex < newused );
                                DataInit<T>::init(buf_.ptr+nextindex, data_+index, newused-nextindex);
                            }
                            meminit(buf_.ptr+index, data, size);
                            buf_.header->used = newused;
                            data_ = buf_.ptr;
                            size_ = newused;
                            #if EVO_LIST_OPT_REFTERM
                                terminated_ = false;
                            #endif
                            return index;
                        }
                    } else
                #else
                    // Lazy buffer disabled
                    assert( buf_.ptr != NULL );
                #endif
                if (buf_.header->refs == 1) {
                    // Existing buffer -- not empty
                    assert( data_ >= buf_.ptr && data_ <= buf_.ptr+buf_.header->used );
                    Size offset = (Size)(data_ - buf_.ptr);
                    {
                        const Size tailsize = buf_.header->used - size_ - offset;
                        if (tailsize > 0) {
                            DataInit<T>::uninit(data_+size_, tailsize);
                            buf_.header->used -= tailsize;
                        }
                    }
                    if (newused > buf_.header->size) {
                        // Move to new bigger buffer
                        Size newbufsize = Capacity::grow(buf_.header->size);
                        if (newbufsize <= newused)
                            newbufsize = newused + 1; // Leave extra space
                        Header* newheader;
                        T* newbuf = buf_.memalloc(newbufsize, newused, newheader);
                        memcpy(newbuf, data_, sizeof(T)*index);
                        memcpy(newbuf+index+size, data_+index, sizeof(T)*(size_-index));
                        if (offset > 0)
                            DataInit<T>::uninit(buf_.ptr, offset);
                        buf_.memfree();
                        buf_.header = newheader;
                        buf_.ptr    = newbuf;
                        data_       = newbuf;
                    } else  {
                        if (size > offset) {
                            // Shift beginning and end to make room
                            if (offset > 0) {
                                DataInit<T>::uninit(buf_.ptr, offset);
                                memmove(buf_.ptr, data_, sizeof(T)*index);
                                data_ = buf_.ptr;
                            }
                            memmove(data_+index+size, data_+index+offset, sizeof(T)*(size_-index));
                            buf_.header->used = newused;
                        } else {
                            // Shift beginning to make room
                            const Size newoffset = offset - size;
                            data_ = buf_.ptr + newoffset;
                            DataInit<T>::uninit(buf_.ptr, offset-newoffset);
                            memmove(data_, buf_.ptr+offset, sizeof(T)*index);
                        }
                    }
                    size_ = newused;

                    // Write new items
                    meminit(data_+index, data, size);
                    return index;
                }
            }

            // New buffer
            Header* newheader;
            T* newbuf = buf_.memalloc(size_+size, newheader);
            {
                DataInit<T>::init(newbuf, data_, index);
                const Size nextindex = index + size;
                assert( nextindex < newheader->used );
                DataInit<T>::init(newbuf+nextindex, data_+index, newheader->used-nextindex);
            }
            meminit(newbuf+index, data, size);
            data_  = buf_.replace(newbuf, newheader);
            size_  = buf_.header->used;
            #if EVO_LIST_OPT_REFTERM
                terminated_ = false;
            #endif
            return index;
        }
        return NONE;
    }

    /** Remove data. Calls unshare().
     \param  index   Remove start index
     \param  size    Size to remove, ALL for all remaining
     \param  uninit  Whether to uninitialize (call destructor on) items before removal
     \return         Number of items removed
    */
    Size modRemove(Size index, Size size, bool uninit=true) {
        if (index < size_) {
            {
                const Size maxsize = size_ - index;
                if (size > maxsize)
                    size = maxsize;
            }
            if (size >= size_) {
                // Remove all -- mostly copied from clear()
                assert( size == size_ );
                assert( data_ > EVO_PEMPTY );
                if (buf_.ptr != NULL) {
                    assert( buf_.header != NULL );
                    if (buf_.header->refs > 1) {
                        // Detach from shared
                        --buf_.header->refs;
                        buf_.header = NULL;
                        buf_.ptr    = NULL;
                        data_       = EVO_PEMPTY;
                    } else if (buf_.header->used > 0) {
                        // Clear buffer, leave buffer for later use
                        assert( buf_.header->refs == 1 );
                        if (uninit) {
                            DataInit<T>::uninit(buf_.ptr, buf_.header->used);
                        } else {
                            // Uninitialize only previously removed items
                            assert( data_ >= buf_.ptr && data_ <= buf_.ptr+buf_.header->used );
                            const Size offset = (Size)(data_ - buf_.ptr);
                            if (offset > 0)
                                DataInit<T>::uninit(buf_.ptr, offset);

                            const Size tailsize = buf_.header->used - size_ - offset;
                            if (tailsize > 0) {
                                DataInit<T>::uninit(data_+size_, tailsize);
                                buf_.header->used -= tailsize;
                            }
                        }
                        buf_.header->used = 0;
                        data_             = buf_.ptr;
                    }
                } else {
                    data_ = EVO_PEMPTY;
                    #if !EVO_LIST_OPT_LAZYBUF
                        // Lazy buffer disabled
                        assert( buf_.header == NULL );
                    #endif
                }
                size_ = 0;
                #if EVO_LIST_OPT_REFTERM
                    terminated_ = false;
                #endif
            } else if (size > 0) {
                // Remove some
                if (buf_.header != NULL) {
                    const Size newsize = size_ - size;
                    #if EVO_LIST_OPT_LAZYBUF
                        if (buf_.ptr == NULL) {
                            if (buf_.header->size >= newsize) {
                                // Use previous buffer
                                assert( buf_.header->used == 0 );
                                buf_.ptr = (T*)(buf_.header + 1);
                                if (index > 0)
                                    DataInit<T>::init(buf_.ptr, data_, index);
                                const Size nextindex = index + size;
                                if (nextindex < size_)
                                    DataInit<T>::init(buf_.ptr+index, data_+nextindex, size_-nextindex);
                                buf_.header->used = newsize;
                                data_  = buf_.ptr;
                                size_  = newsize;
                                #if EVO_LIST_OPT_REFTERM
                                    terminated_ = false;
                                #endif
                                return size;
                            }
                        } else
                    #else
                        // Lazy buffer disabled
                        assert( buf_.ptr != NULL );
                    #endif
                    if (buf_.header->refs == 1) {
                        // Existing buffer
                        assert( data_ >= buf_.ptr && data_ <= buf_.ptr+buf_.header->used );
                        const Size offset = (Size)(data_ - buf_.ptr);
                        {
                            const Size tailsize = buf_.header->used - size_ - offset;
                            if (tailsize > 0) {
                                DataInit<T>::uninit(data_+size_, tailsize);
                                buf_.header->used -= tailsize;
                            }
                        }

                        // Remove items
                        if (uninit)
                            DataInit<T>::uninit(data_+index, size);
                        const Size nextindex = index + size;
                        if (nextindex < size_)
                            memmove(data_+index, data_+nextindex, sizeof(T)*(size_-nextindex));
                        buf_.header->used -= size;
                        size_              = newsize;
                        return size;
                    }
                }

                // New buffer
                assert( data_ != NULL );
                Header* newheader;
                const Size newused = size_ - size;
                T* newbuf = buf_.memalloc(newused+1, newused, newheader); // Leave extra space
                {
                    if (index > 0)
                        DataInit<T>::init(newbuf, data_, index);
                    const Size nextindex = index + size;
                    if (nextindex < size_)
                        DataInit<T>::init(newbuf+index, data_+nextindex, size_-nextindex);
                }
                data_ = buf_.replace(newbuf, newheader);
                size_ = buf_.header->used;
                #if EVO_LIST_OPT_REFTERM
                    terminated_ = false;
                #endif
            }
        } else
            size = 0;
        #if !EVO_LIST_OPT_LAZYBUF
            // Lazy buffer disabled
            assert( (buf_.header != NULL) == (buf_.ptr != NULL) );
        #endif
        return size;
    }

    /** Replace data.
     - This effectively calls unshare()
     .
     \param  index    Start index to replace from, END to append instead of replace
     \param  size     Size to replace, ALL for all remaining, 0 to insert instead of replace
     \param  data     Replacement data (copied), ignored if newsize is 0
     \param  newsize  Replacement size, 0 to replace with nothing (i.e. remove)
    */
    void modReplace(Size index, Size size, const Item* data, Size newsize) {
        assert( size > 0 && newsize > 0 );
        assert( index < size_ );
        assert( data != NULL );
        const Size maxsize = size_ - index;
        if (size > maxsize)
            size = maxsize;
        if (buf_.header != NULL) {
            const Size newdatasize = size_ - size + newsize;
            #if EVO_LIST_OPT_LAZYBUF
                if (buf_.ptr == NULL) {
                    if (buf_.header->size >= newdatasize) {
                        // Use previous buffer
                        assert( buf_.header->used == 0 );
                        buf_.ptr = (T*)(buf_.header + 1);
                        if (index > 0)
                            DataInit<T>::init(buf_.ptr, data_, index);
                        DataInit<T>::init(buf_.ptr+index, data, newsize);

                        const Size nextindex = index + size;
                        if (nextindex < size_)
                            DataInit<T>::init(buf_.ptr+index+newsize, data_+nextindex, size_-nextindex);
                        buf_.header->used = newdatasize;
                        data_ = buf_.ptr;
                        size_ = newdatasize;
                        #if EVO_LIST_OPT_REFTERM
                            terminated_ = false;
                        #endif
                        return;
                    }
                } else
            #else
                // Lazy buffer disabled
                assert( buf_.ptr != NULL );
            #endif
            if (buf_.header->refs == 1) {
                // Existing buffer
                assert( data_ >= buf_.ptr && data_ <= buf_.ptr+buf_.header->used );
                {
                    // Replace existing items
                    const Size copysize = (size < newsize ? size : newsize);
                    DataInit<T>::copy(data_+index, data, copysize);
                    index     += copysize;
                    data      += copysize;
                    size      -= copysize;
                    newsize   -= copysize;
                }
                if (size > 0) {
                    // Remove extra items
                    T* dataptr = data_ + index;
                    DataInit<T>::uninit(dataptr, size);
                    const Size nextindex = index + size;
                    if (nextindex < size_)
                        memmove(dataptr, data_+nextindex, sizeof(T)*(size_-nextindex));
                    buf_.header->used -= size;
                    size_             -= size;
                } else if (newsize > 0) {
                    // Insert new items
                    const Size newused = buf_.header->used + newsize;
                    if (newused > buf_.header->size) {
                        const Size offset = (Size)(data_ - buf_.ptr);
                        buf_.ptr = buf_.memrealloc(Capacity::grow(newused));
                        data_    = buf_.ptr + offset;
                    }
                    T* dataptr = data_ + index;
                    if (index < size_)
                        memmove(dataptr+newsize, dataptr, sizeof(T)*(size_-index));
                    DataInit<T>::init(dataptr, data, newsize);
                    buf_.header->used = newused;
                    size_            += newsize;
                }
                return;
            }
        }
        
        // New buffer
        Header* newheader;
        T* newbuf = buf_.memalloc(size_-size+newsize, newheader);
        {
            if (index > 0)
                DataInit<T>::init(newbuf, data_, index);
            DataInit<T>::init(newbuf+index, data, newsize);
            const Size fromindex = index + size;
            if (fromindex < size_)
                DataInit<T>::init(newbuf+index+newsize, data_+fromindex, size_-fromindex);
        }
        data_ = buf_.replace(newbuf, newheader);
        size_ = buf_.header->used;
        #if EVO_LIST_OPT_REFTERM
            terminated_ = false;
        #endif
    }

    /** Initialize given memory.
     \param  ptr   Memory to initialize
     \param  data  Data to initialize, NULL to leave uninitialized, PDEFAULT to initialize with default constructor
     \param  size  Data size -- ignored if data is NULL or PDEFAULT
    */
    static void meminit(T* ptr, const Item* data, Size size) {
        if (data == EVO_PDEFAULT)
            DataInit<T>::init(ptr, size);
        else if (data != NULL)
            DataInit<T>::init(ptr, data, size);
    }
};

///////////////////////////////////////////////////////////////////////////////
//@}
}
#if defined(_MSC_VER)
    #pragma warning(pop)
#endif
#endif
