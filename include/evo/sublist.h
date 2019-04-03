// Evo C++ Library
/* Copyright 2019 Justin Crowell
Distributed under the BSD 2-Clause License -- see included file LICENSE.txt for details.
*/
///////////////////////////////////////////////////////////////////////////////
/** \file sublist.h Evo SubList container. */
#pragma once
#ifndef INCL_evo_sublist_h
#define INCL_evo_sublist_h

#include "impl/container.h"
#include "impl/iter.h"

namespace evo {
/** \addtogroup EvoContainers */
//@{

///////////////////////////////////////////////////////////////////////////////

/** Reference and access existing list data.

\tparam  T      Item type to use
\tparam  TSize  Size type to use for size/index values (must be unsigned integer) -- default: SizeT

\par Features

 - This provides read-only methods similar to List, but with reduced overhead
   - This is useful for referencing static/immutable data, or when performance or size is critical
   - List is generally safer, though SubList is smaller and slightly faster

 - This does not allocate or free any resources
 - \b Caution: SubList can be unsafe since it references a pointer, which must remain valid
 .

C++11:
 - Range-based for loop
   \code
    SubList<int> list;
    for (auto num : list) {
    }
   \endcode
 - Move semantics

\par Iterators

 - SubList<>::Iter -- Read-Only Iterator (IteratorRa)
 .

\par Constructors

 - SubList()
 - SubList(const ThisType&)
 - SubList(const ListBaseType&)
 - SubList(const ListBaseType*)
 - SubList(const Item*,Size)
 - SubList(const Item*,ValNull,ItemVal)
 - SubList(ThisType&&) [C++11]
 .

\par Read Access

 - size()
   - null(), empty()
 - data()
   - item()
   - operator[]()
   - first(), last(), iend()
   - hash()
 - compare(const ListBaseType&) const, ...
   - operator==(const ListBaseType&) const, ...
   - operator!=(const ListBaseType&) const, ...
   - starts(ItemVal) const
   - starts(const Item*,Size) const
   - ends(ItemVal) const
   - ends(const Item*,Size) const
 - cbegin(), cend()
   - begin(), end()
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

\par Modifiers

 - set()
   - set(const ListBaseType&)
   - set(const Item*,Size)
   - set2()
   - setempty()
   - clear()
   - operator=(const ListBaseType&)
   - operator=(const ThisType&)
   - operator=(ThisType&&) [C++11]
   - operator=(const ValNull&)
   - operator=(const ValEmpty&)
 - splitat_setl(Key), splitat_setl(Key,T2&)
 - splitat_setr(Key), splitat_setr(Key,T1&)
 - slice(Key)
   - slice(Key,Size), slice2()
   - truncate()
   - triml(), trimr()
 - unshare()
 - swap()
 .

\par Example

\code
#include <evo/sublist.h>
#include <evo/io.h>
using namespace evo;
static Console& c = con();

int main() {
    // Create number sublist from NULL-terminated constant
    const int LIST_ITEMS[] = { 1, 2, 3, (int)NULL };
    SubList<int> list(LIST_ITEMS, vNULL);

    // operator[]() provides read-only (const) access
    int value = list[0];
    //list[0] = 0;     // Error: operator[]() is read-only

    // Iterate and print items (read-only)
    for (SubList<int>::Iter iter(list); iter; ++iter)
        c.out << "Item: " << *iter << NL;

    return 0;
}
\endcode

Output:
\code{.unparsed}
Item: 1
Item: 2
Item: 3
\endcode
*/
template<class T,class TSize=SizeT>
struct SubList : public ListBase<T,TSize> {
    using ListBase<T,TSize>::data_;
    using ListBase<T,TSize>::size_;

    EVO_CONTAINER_TYPE;                             ///< Identify Evo container type
    typedef TSize               Size;               ///< List size integer type
    typedef Size                Key;                ///< Key type (item index)
    typedef T                   Value;              ///< Value type (same as Item)
    typedef T                   Item;               ///< Item type (same as Value)
    typedef typename DataCopy<T>::PassType ItemVal; ///< Item type as parameter (POD types passed by value, otherwise by const-ref)

    typedef SubList<T,Size>     ThisType;           ///< This list type
    typedef SubList<T,Size>     SubListType;        ///< %SubList base type
    typedef ListBase<T,Size>    ListBaseType;       ///< %List base type for any Evo list

    // Iterator support types
    /** \cond impl */
    typedef Key IterKey;
    typedef T   IterItem;
    /** \endcond */

    typedef typename IteratorRa<ThisType>::Const Iter;  ///< Iterator (const) - IteratorRa

    /** Default constructor sets as null. */
    SubList() {
        data_ = NULL;
        size_ = 0;
    }

    /** Copy constructor to reference source data.
     - This will reference the same pointer as given data
     - \b Caution: Source data pointer must remain valid
     .
     \param  data  Data to reference
    */
    SubList(const ThisType& data) {
        data_ = data.data_;
        size_ = data.size_;
    }

    /** Copy constructor to reference source data.
     - This will reference the same pointer as given data
     - \b Caution: Source data pointer must remain valid
     .
     \param  data  Data to reference
    */
    SubList(const ListBaseType& data) {
        data_ = data.data_;
        size_ = data.size_;
    }

    /** Copy constructor to reference source data.
     - This will reference the same pointer as given data
     - \b Caution: Source data pointer must remain valid
     .
     \param  data   Data to reference
     \param  index  Start index of data to reference, END to set as empty
     \param  size   Size as item count, ALL for all from index
    */
    SubList(const ListBaseType& data, Key index, Size size=ALL) {
        if (data.data_ == NULL) {
            data_ = NULL;
            size_ = 0;
        } else {
            if (index > data.size_)
                index = data.size_;
            const Size max_size = data.size_ - index;
            data_ = (Item*)data.data_ + index;
            size_ = (size > max_size ? max_size : size);
        }
    }

    /** Copy constructor to reference source data from pointer.
     - This will reference the same pointer as given data pointer (data->data_), if not NULL
     - \b Caution: Source data pointer (data->data_) must remain valid
     .
     \param  data  Data pointer to reference, calls set() if NULL
    */
    SubList(const ListBaseType* data) {
        if (data == NULL) {
            data_ = NULL;
            size_ = 0;
        } else {
            data_ = data->data_;
            size_ = data->size_;
        }
    }

    /** Constructor to reference data pointer.
     \param  data  Data to reference
     \param  size  Data size as item count
    */
    SubList(const Item* data, Size size) {
        data_ = (Item*)data;
        size_ = size;
    }

    /** Constructor to reference terminated data pointer.
     - Useful for referencing immutable lists\n
       Example:
       \code
        // Size is automatically determined by finding the terminator (NULL)
        static const int LIST_ITEMS[] = {1, 2, 3, 4, (int)NULL};
        static const SubList<int> LIST(LIST_ITEMS, vNULL);
       \endcode
     - \b Caution: A terminator must be present
     .
     \param  data  Data to reference
     \param  v     vNULL to indicate data is terminated
     \param  term  Terminator to use (defaults to NULL)
    */
    SubList(const Item* data, ValNull v, ItemVal term=0) {
        EVO_PARAM_UNUSED(v);
        data_ = (Item*)data;
        size_ = 0;
        if (data != NULL)
            while (!(data[size_] == term))
                ++size_;
    }

#if defined(EVO_CPP11)
    /** Move constructor (C++11).
     \param  src  Source to move
    */
    SubList(ThisType&& src) {
        ::memcpy(this, &src, sizeof(ThisType));
    }

    /** Move assignment operator (C++11).
     \param  src  Source to move
     \return      This
    */
    ThisType& operator=(ThisType&& src) {
        ::memcpy(this, &src, sizeof(ThisType));
        return *this;
    }
#endif

    /** Explicitly use a const reference to this.
     - This is useful to force using this as const without casting
     .
     \return  This
    */
    const ThisType& asconst() const {
        return *this;
    }

    // SET

    /** Assignment operator.
     \param  src  Source to copy
     \return      This
    */
    ThisType& operator=(const ThisType& src) {
        ::memcpy(this, &src, sizeof(ThisType));
        return *this;
    }

    /** Assignment operator sets as null.
     \param  val  vNULL
     \return      This
    */
    ThisType& operator=(const ValNull& val) {
        EVO_PARAM_UNUSED(val);
        data_ = NULL;
        size_ = 0;
        return *this;
    }

    /** Assignment operator sets as null.
     \param  val  vEMPTY
     \return      This
    */
    ThisType& operator=(const ValEmpty& val) {
        EVO_PARAM_UNUSED(val);
        data_ = NULL;
        size_ = 0;
        return *this;
    }

    /** Assignment operator sets as reference to source data.
     - This will reference the same pointer as given data
     - \b Caution: Source data pointer must remain valid
     .
     \param  data  Data to reference
     \return       This
    */
    ThisType& operator=(const ListBaseType& data) {
        data_ = data.data_;
        size_ = data.size_;
        return *this;
    }

    /** Assignment operator sets as reference to source data from pointer.
     - This will reference the same pointer as given data pointer, if not NULL
     - \b Caution: Source data pointer (data->data_) must remain valid
     .
     \param  data  Data pointer to reference, calls set() if NULL
     \return       This
    */
    ThisType& operator=(const ListBaseType* data) {
        if (data == NULL) {
            data_ = NULL;
            size_ = 0;
        } else {
            data_ = data->data_;
            size_ = data->size_;
        }
        return *this;
    }

    /** Clear by removing all items.
     - Does not set as null -- null status is unchanged
     .
     \return  This
    */
    ThisType& clear() {
        if (size_ > 0) {
            data_ = EVO_PEMPTY;
            size_ = 0;
        }
        return *this;
    }

    /** %Set as null.
     \return  This
    */
    ThisType& set() {
        data_ = NULL;
        size_ = 0;
        return *this;
    }

    /** %Set as reference to source data.
     - This will reference the same pointer as given data
     - \b Caution: Source data pointer must remain valid
     .
     \param  data  Data to reference
     \return       This
    */
    ThisType& set(const ListBaseType& data) {
        data_ = data.data_;
        size_ = data.size_;
        return *this;
    }

    /** %Set as reference to source data.
     - This will reference the same pointer as given data
     - \b Caution: Source data pointer must remain valid
     .
     \param  data   Data to reference
     \param  index  Start index of data to reference, END to set as empty
     \param  size   Size as item count, ALL for all from index
     \return        This
    */
    ThisType& set(const ListBaseType& data, Key index, Key size=ALL) {
        if (data.data_ == NULL) {
            data_ = NULL;
            size_ = 0;
        } else {
            if (index > data.size_)
                index = data.size_;
            const Size max_size = data.size_ - index;
            data_ = (Item*)data.data_ + index;
            size_ = (size > max_size ? max_size : size);
        }
        return *this;
    }

    /** %Set as reference to source data by pointer.
     - This will reference the same pointer as given data
     - \b Caution: Source data pointer must remain valid
     .
     \param  data  Pointer to data to reference, null if NULL
     \return       This
    */
    ThisType& set(const ListBaseType* data) {
        if (data == NULL)
            set();
        else
            set(*data);
        return *this;
    }

    /** %Set as reference to data pointer.
     \param  data  Data to reference
     \param  size  Data size as item count
    */
    ThisType& set(const Item* data, Size size) {
        data_ = (Item*)data;
        size_ = size;
        return *this;
    }

    /** %Set as reference to subset of source data using start/end positions.
     - This will reference the same pointer as given data
     - If data is null then this will be set to null
     - If index2 <= index1 then this will be set to an empty sublist
     - \b Caution: Source data pointer must remain valid
     .
     \param  data    Data to reference
     \param  index1  Start index of data to reference, END to set as empty
     \param  index2  End index of data (this item not included), END for all after index1
     \return         This
    */
    ThisType& set2(const ListBaseType& data, Key index1, Key index2) {
        if (data.data_ == NULL) {
            data_ = NULL;
            size_ = 0;
        } else {
            if (index1 > data.size_)
                index1 = data.size_;
            data_ = (Item*)data.data_ + index1;
            if (index2 > data.size_)
                size_ = data.size_ - index1;
            else if (index2 <= index1)
                size_ = 0;
            else
                size_ = index2 - index1;
        }
        return *this;
    }

    /** %Set as empty but not null.
     \return  This
    */
    ThisType& setempty()
        { data_ = EVO_PEMPTY; size_ = 0; return *this; }

    // INFO

    /** \copydoc evo::List::null() const */
    bool null() const
        { return (data_ == NULL); }

    /** \copydoc evo::List::empty() const */
    bool empty() const
        { return (size_ == 0); }

    /** \copydoc evo::List::size() const */
    Size size() const
        { return size_; }

    /** Get whether shared (false).
     - This doesn't support sharing or unique states, so this always returns false
     - Referenced data is considered constant and immutable while it's referenced
     .
     \return  Whether shared (always false)
    */
    bool shared() const
        { return false; }

    /** Get data pointer.
     - \b Caution: May return an invalid non-NULL pointer if empty
     .
     \return  Data pointer as read-only, NULL if null, may be invalid if empty (const)
    */
    const Item* data() const
        { return data_; }

    /** Get item at position.
     - \b Caution: Results are undefined if index is out of bounds -- though index is checked with assert()
     .
     \param  index  Item index
     \return        Given item as read-only (const)
    */
    const Item& operator[](Key index) const {
        assert( index < size_ );
        return data_[index];
    }

    /** Get item at position.
     - \b Caution: Results are undefined if index is out of bounds -- though index is checked with assert()
     .
     \param  index  Item index
     \return        Given item as read-only (const)
    */
    const Item& item(Key index) const {
        assert( index < size_ );
        return data_[index];
    }

    /** \copydoc evo::List::first() */
    const Item* first() const
        { return (size_ > 0 ? data_ : NULL); }

    /** \copydoc evo::List::last() */
    const Item* last() const
        { return (size_ > 0 ? data_+size_-1 : NULL); }

    /** \copydoc evo::List::iend() */
    Key iend(Size offset=0) const
        { return (offset >= size_ ? END : size_ - 1 - offset); }

    /** \copydoc evo::List::hash() */
    ulong hash(ulong seed=0) const
        { return DataHash<Item>::hash(data_, size_, seed); }

    // COMPARE

    /** \copydoc evo::List::compare() */
    int compare(const ListBaseType& data) const {
        int result;
        if (data_ == NULL)
            result = (data.data_ != NULL ? -1 : 0);
        else if (data.data_ == NULL)
            result = 1;
        else
            result = DataCompare<Item>::compare(data_, size_, data.data_, data.size_);
        return result;
    }

    /** \copydoc evo::List::operator==(const ListBaseType&) const */
    bool operator==(const ListBaseType& data) const {
        bool result;
        if (data_ == NULL)
            result = (data.data_ == NULL);
        else if (data.data_ == NULL)
            result = false;
        else
            result = (DataCompare<Item>::compare(data_, size_, data.data_, data.size_) == 0);
        return result;
    }

    /** \copydoc evo::List::operator!=(const ListBaseType&) const */
    bool operator!=(const ListBaseType& data) const {
        bool result;
        if (data_ == NULL)
            result = (data.data_ != NULL);
        else if (data.data_ == NULL)
            result = true;
        else
            result = (DataCompare<Item>::compare(data_, size_, data.data_, data.size_) != 0);
        return result;
    }

    /** \copydoc evo::List::starts(ItemVal) const */
    bool starts(ItemVal item) const
        { return (size_ > 0 && *data_ == item); }

    /** \copydoc evo::List::starts(const Item*,Size) const */
    bool starts(const Item* items, Size size) const
        { return (size > 0 && size_ >= size && DataEqual<Item>::equal(data_, items, size)); }

    /** \copydoc evo::List::starts(const ListBaseType&) const */
    bool starts(const ListBaseType& items) const
        { return (items.size_ > 0 && size_ >= items.size_ && DataEqual<Item>::equal(data_, items.data_, items.size_)); }

    /** \copydoc evo::List::ends(ItemVal) const */
    bool ends(ItemVal item) const
        { return (size_ > 0 && data_[size_-1] == item); }

    /** \copydoc evo::List::ends(const Item*,Size) const */
    bool ends(const Item* items, Size size) const
        { return (size > 0 && size_ >= size && DataEqual<Item>::equal(data_+size_-size, items, size)); }

    /** \copydoc evo::List::ends(const ListBaseType&) const */
    bool ends(const ListBaseType& items) const
        { return (items.size_ > 0 && size_ >= items.size_ && DataEqual<Item>::equal(data_+size_-items.size_, items.data_, items.size_)); }

    // FIND

    /** Get iterator at first item (const).
     - This allows compatibility with range-based for loops and other libraries, otherwise use container Iter directly
     .
     \return  Iterator at first item, or at end position if empty
     \see Iter, cend(), begin(), end()
    */
    Iter cbegin() const
        { return Iter(*this); }

    /** Get iterator at end (const).
     - This allows compatibility with range-based for loops and other libraries, otherwise use container Iter directly
     - This really just creates an empty iterator
     .
     \return  Iterator at end position
     \see Iter, cbegin(), begin(), end()
    */
    Iter cend() const
        { return Iter(); }

    /** Get iterator at first item (const).
     - This allows compatibility with range-based for loops and other libraries, otherwise use container Iter directly
     .
     \return  Iterator at first item, or at end position if empty
     \see end(), cbegin()
    */
    Iter begin() const
        { return Iter(*this); }

    /** Get iterator at end (const).
     - This allows compatibility with range-based for loops and other libraries, otherwise use container Iter directly
     - This really just creates an empty iterator
     .
     \return  Iterator at end position
     \see begin(), cend()
    */
    Iter end() const
        { return Iter(); }

    /** \copydoc evo::List::find(ItemVal,Key,Key) const */
    Key find(ItemVal item, Key start=0, Key end=END) const {
        if (end > size_)
            end = size_;
        for (; start<end; ++start)
            if (data_[start] == item)
                return start;
        return (Key)NONE;
    }

    /** \copydoc evo::List::findr(ItemVal,Key,Key) const */
    Key findr(ItemVal item, Key start=0, Key end=END) const {
        if (end > size_)
            end = size_;
        while (end>start)
            if (data_[--end] == item)
                return end;
        return (Key)NONE;
    }

    /** \copydoc evo::List::findany(const Item*,Size,Key,Key) const */
    Key findany(const Item* items, Size count, Key start=0, Key end=END) const {
        Size j;
        if (end > size_)
            end = size_;
        for (; start<end; ++start)
            for (j=0; j<count; ++j)
                if (data_[start] == items[j])
                    return start;
        return (Key)NONE;
    }

    /** \copydoc evo::List::findanyr(const Item*,Size,Key,Key) const */
    Key findanyr(const Item* items, Size count, Key start=0, Key end=END) const {
        Size j;
        if (end > size_)
            end = size_;
        while (end>start) {
            --end;
            for (j=0; j<count; ++j)
                if (data_[end] == items[j])
                    return end;
        }
        return (Key)NONE;
    }

    /** \copydoc evo::List::contains(ItemVal) const */
    bool contains(ItemVal item) const {
        bool result = false;
        for (Key i=0; i<size_; ++i)
            if (data_[i] == item)
                { result = true; break; }
        return result;
    }

    /** \copydoc evo::List::contains(const Item*,Size) const */
    bool contains(const Item* data, Size size) const {
        bool result = false;
        if (size > 0 && size_ >= size) {
            const Size end = size_ - size;
            for (Key i=0; i<=end; ++i)
                if (DataEqual<Item>::equal(data_+i, data, size))
                    { result = true; break; }
        }
        return result;
    }

    /** \copydoc evo::List::contains(const ListBaseType&) const */
    bool contains(const ListBaseType& data) const {
        bool result = false;
        if (data.size_ > 0 && size_ >= data.size_) {
            const Size end = size_ - data.size_;
            for (Key i=0; i<=end; ++i)
                if (DataEqual<Item>::equal(data_+i, data.data_, data.size_))
                    { result = true; break; }
        }
        return result;
    }

    // SPLIT

    /** \copydoc evo::List::splitat(Key,T1&,T2&) const */
    template<class T1,class T2>
    bool splitat(Key index, T1& left, T2& right) const {
        bool result = false;
        if (index >= size_) {
            left.set(*this);
            right.set();
        } else {
            left.set(*this, 0, index);
            right.set(*this, index+1, size_-index-1);
            result = true;
        }
        return result;
    }

    /** \copydoc evo::List::splitat(Key,T1&) const */
    template<class T1>
    bool splitat(Key index, T1& left) const {
        bool result = false;
        if (index >= size_)
            left.set(*this);
        else {
            left.set(*this, 0, index);
            result = true;
        }
        return result;
    }

    /** \copydoc evo::List::splitat(Key,ValNull,T2&) const */
    template<class T2>
    bool splitat(Key index, ValNull left, T2& right) const {
        EVO_PARAM_UNUSED(left);
        bool result = false;
        if (index >= size_)
            right.set();
        else {
            right.set(*this, index+1, size_-index-1);
            result = true;
        }
        return result;
    }

    /** \copydoc evo::List::splitat_setl(Key) */
    bool splitat_setl(Key index) {
        bool result = false;
        if (index < size_) {
            set(data_, index);
            result = true;
        }
        return result;
    }

    /** \copydoc evo::List::splitat_setl(Key,T2&) */
    template<class T2>
    bool splitat_setl(Key index, T2& right) {
        bool result = false;
        if (index >= size_) {
            right.set();
        } else {
            right.set(*this, index+1, size_-index-1);
            set(data_, index);
            result = true;
        }
        return result;
    }

    /** \copydoc evo::List::splitat_setr(Key) */
    bool splitat_setr(Key index) {
        bool result = false;
        if (index >= size_)
            set();
        else {
            set(data_+index+1, size_-index-1);
            result = true;
        }
        return result;
    }
    
    /** \copydoc evo::List::splitat_setr(Key,T1&) */
    template<class T1>
    bool splitat_setr(Key index, T1& left) {
        bool result = false;
        if (index >= size_) {
            left.set(*this);
            set();
        } else {
            left.set(*this, 0, index);
            set(data_+index+1, size_-index-1);
            result = true;
        }
        return result;
    }

    // TRIM

    /** \copydoc evo::List::triml(Size) */
    ThisType& triml(Size size) {
        if (size < size_) {
            size_ -= size;
            data_ += size;
        } else if (size_ > 0) {
            data_ = EVO_PEMPTY;
            size_ = 0;
        }
        return *this;
    }

    /** \copydoc evo::List::trimr(Size) */
    ThisType& trimr(Size size) {
        if (size < size_) {
            size_ -= size;
        } else if (size_ > 0) {
            data_ = EVO_PEMPTY;
            size_ = 0;
        }
        return *this;
    }

    /** \copydoc evo::List::truncate(Size) */
    ThisType& truncate(Size size) {
        if (size < size_) {
            if (size == 0)
                data_ = EVO_PEMPTY;
            size_ = size;
        }
        return *this;
    }

    // SLICE

    /** Slice beginning items.
     - This adjusts pointer, trimming beginning items
     - If null then will be left null
     .
     \param  index  Start index of new slice, END to set as empty (or leave null if null)
     \return        This
    */
    ThisType& slice(Key index) {
        if (index < size_) {
            data_ += index;
            size_ -= index;
        } else if (size_ > 0) {
            data_ = EVO_PEMPTY;
            size_ = 0;
        }
        return *this;
    }

    /** Slice to given subset.
     - This adjusts pointer to given subset
     - If null then will be left null
     .
     \param  index  Start index of new slice, END to set as empty (or leave null if null)
     \param  size   Slice size as item count, ALL for all from index
     \return        This
    */
    ThisType& slice(Key index, Size size) {
        if (index < size_) {
            data_ += index;
            const Size maxsize = size_ - index;
            if (size > maxsize)
                size_ = maxsize;
            else
                size_ = size;
        } else if (size_ > 0) {
            data_ = EVO_PEMPTY;
            size_ = 0;
        }
        return *this;
    }

    /** Slice to given subset using start/end positions.
     - This adjusts pointer to given subset
     - If index2 < index1 then result will be empty
     - If null then will be left null
     .
     \param  index1  Start index of new slice, END to set as empty (or leave null if null)
     \param  index2  End index of new slice (this item not included), END for all after index1
     \return         This
    */
    ThisType& slice2(Key index1, Key index2)
        { return slice(index1, (index1 < index2 ? index2-index1 : 0)); }

    // UNSHARE

    /** Make data unique -- no-op.
     - This doesn't support sharing or unique states, so this is always a no-op
     - Referenced data is considered constant and immutable while it's referenced
     .
     \return  This
    */
    ThisType& unshare()
        { return *this; }

    // SWAP

    /** Swap with another sublist.
     \param  list  %List to swap with
    */
    void swap(ThisType& list)
        { EVO_IMPL_CONTAINER_SWAP(this, &list, ThisType); }

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
                result = data_ + key;
            else
                key = END;
        }
        return result;
    }
    const IterItem* iterNext(Size count, IterKey& key) const {
        const IterItem* result = NULL;
        if (key != END) {
            if ( (key+=count) < size_ )
                result = data_ + key;
            else
                key = END;
        }
        return result;
    }
    const IterItem* iterLast(IterKey& key) const {
        const IterItem* result = NULL;
        if (size_ > 0) {
            key    = size_ - 1;
            result = data_ + key;
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
};

///////////////////////////////////////////////////////////////////////////////
//@}
}
#endif
