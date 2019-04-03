// Evo C++ Library
/* Copyright 2019 Justin Crowell
Distributed under the BSD 2-Clause License -- see included file LICENSE.txt for details.
*/
///////////////////////////////////////////////////////////////////////////////
/** \file set.h Evo Set interface. */
#pragma once
#ifndef INCL_evo_set_h
#define INCL_evo_set_h

#include "type.h"
#include "strtok.h"

namespace evo {
/** \addtogroup EvoContainers */
//@{

///////////////////////////////////////////////////////////////////////////////

/** Associative container with unique values for fast lookup.

\tparam  TKey   %Set value and value type
\tparam  TSize  %Size type to use for size values (must be unsigned integer) -- default: SizeT

\par Features

 - Interface similar to STL std::set
 - Values are always unique (no duplicates)
 - Implementations:
   - SetList (ordered)
   - SetHash (unordered)
   .
 - For best performance use concrete set types instead of the general interface (Set)
 .

\par Iterators

 - Set::Iter -- Read-Only Iterator (IteratorBi)
 - Set::IterM -- Mutable Iterator (IteratorBi)
 .

\b Caution: Modifying or resizing a set will shift or invalidate existing iterators using it.

\par Read Access

 - asconst()
 - size()
   - null(), empty()
   - capacity()
   - shared()
   - ordered()
 - contains()
 - cbegin(), cend()
   - begin() const, end() const
 - iter()
 - operator==()
   - operator!=()
 .

\par Modifiers

 - reserve()
   - capacity(Size)
   - capacitymin()
   - unshare()
   - compact()
 - begin(), end()
 - iterM()
 - set()
   - set(const SetBaseType&)
   - setempty()
   - clear()
   - operator=()
 - get()
 - add(const Value&,bool)
   - addfrom()
   - addsplit()
 - remove(const Value&)
   - remove(IterM&,IteratorDir)
 .

\par Comparison Object

%Sets that use a hash/comparison type will have these additional methods (not included in this interface), where T is the hash/compare type used:
 - `const T& get_compare() const`
 - `T& get_compare()`
 .

See SetList or SetHash for example.

\par Ordered Sets

Ordered sets will have these additional methods (not included in this interface):
 - `Iter iter_lower(const Value& value) const`
 - `Iter iter_upper(const Value& value) const`
 - `IterM iter_lowerM(const Value& value)`
 - `IterM iter_upperM(const Value& value)`
 - `Size remove(const Value& value, Size count)`
 - `Size remove_range(IterM& start, Size count)`
 - `Size remove_range(IterM& start, IterM& end)`
 .

See SetList for example.

\par List Sets

%List sets will have these additional methods (not included in this interface):
 - `Size item(Size index) const`
 - `Size removeat(Size index)`
 .

See SetList for example.

\par Example

See implementation types for examples: SetList, SetHash
*/
template<class TKey, class TSize=SizeT>
class Set {
public:
    EVO_CONTAINER_TYPE;
    typedef Set<TKey,TSize> ThisType;                   ///< This type
    typedef Set<TKey,TSize> SetBaseType;                ///< %Set base type
    typedef TSize Size;                                 ///< Size type for size values (must be unsigned integer) -- default: SizeT
    typedef TKey  Key;                                  ///< Key type (same as Value)
    typedef TKey  Value;                                ///< Value type
    typedef TKey  Item;                                 ///< Item type (same as Value)

    // Iterator support types
    /** \cond impl */
    struct IterKey {
        Size a, b;

        IterKey(Size a=0, Size b=0) : a(a), b(b)
            { }
        IterKey(const IterKey& src)
            { memcpy(this, &src, sizeof(src)); }
    };
    typedef Item IterItem;
    /** \endcond */

    typedef typename IteratorBi<ThisType>::Const Iter;  ///< Iterator (const) - IteratorBi
    typedef IteratorBi<ThisType>                 IterM; ///< Iterator (mutable) - IteratorBi

protected:
    Size size_;             ///< %Set size (number of items, automatically updated by concrete set members)

    /** Constructor. Used by derived types. */
    Set() : size_(0)
        { }

    /** Copy constructor (disabled). */
    Set(const ThisType&) EVO_ONCPP11(= delete);

    /** Used by base class to get data to initialize iterator.
     \param  iterkey  Set to iterator data, if item found
     \param  value    Value to find
     \return          Value pointer, NULL if not found
    */
    virtual const Value* getiter(IterKey& iterkey, const Value& value) const = 0;

public:
    /** Destructor. */
    virtual ~Set()
        { }

    /** \copydoc List::asconst() */
    const SetBaseType& asconst() const {
        return *this;
    }

    // SET

    /** Assignment operator.
     \param  src  Source to copy
     \return      This
    */
    SetBaseType& operator=(const SetBaseType& src) {
        set(src);
        return *this;
    }

    /** %Set as null and empty.
     \return  This
    */
    virtual SetBaseType& set() = 0;

    /** %Set as copy of given set.
     - This clears all items in current set.
     .
     \param  src  Source items
     \return      This
    */
    virtual SetBaseType& set(const SetBaseType& src) = 0;

    /** %Set as empty but not null.
     \return  This
    */
    virtual SetBaseType& setempty() = 0;

    /** Clear by removing all items.
     - Does not set as null -- null status is unchanged
     .
     \return  This
    */
    virtual SetBaseType& clear() = 0;

    // INFO

    /** Get whether set is ordered.
     - Ordered sets keep items in order by value so iteration order is predictable
     .
    */
    virtual bool ordered() const = 0;

    /** Get whether set is null.
     \return  Whether null
    */
    virtual bool null() const = 0;

    /** Get whether set is empty (size is 0).
     \return  Whether empty
    */
    bool empty() const
        { return (size_ == 0); }

    /** Get set size (number of items).
     \return  %Set size
    */
    Size size() const
        { return size_; }

    /** Get whether shared.
     - Data is shared when referencing external data or buffer is allocated and shared (reference count > 1)
     .
     \return  Whether shared
    */
    virtual bool shared() const = 0;

    /** Get set capacity.
     \return  Current capacity
    */
    virtual Size capacity() const = 0;

    // COMPARE

    /** Equality operator.
     \param  set  %Set to compare to
     \return      Whether sets are equal (have same items)
    */
    bool operator==(const SetBaseType& set) const {
        bool result = false;
        if (this == &set)
            result = true;
        else if (this->null())
            result = set.null();
        else if (set.null())
            result = false;
        else if (size_ == set.size_) {
            result = true;
            for (Iter iter(*this); iter; ++iter) {
                if (!set.contains(*iter)) { // lookup rather than rely on ordering
                    result = false;
                    break;
                }
            }
        }
        return result;
    }

    /** Inequality operator.
     \param  set  %Set to compare to
     \return      Whether sets are not equal (don't have same items)
    */
    bool operator!=(const SetBaseType& set) const
        { return !this->operator==(set); }

    // CONTAINS

    /** Get whether the set contains the given value.
     \param  value  Value to look for
     \return        Whether value was found
    */
    virtual bool contains(const Value& value) const = 0;

    // FIND

    /** \copydoc List::cbegin() */
    Iter cbegin() const
        { return Iter(*this); }

    /** \copydoc List::cend() */
    Iter cend() const
        { return Iter(); }

    /** \copydoc List::begin() */
    IterM begin()
        { return IterM(*this); }

    /** \copydoc List::begin() const */
    Iter begin() const
        { return Iter(*this); }

    /** \copydoc List::end() */
    IterM end()
        { return IterM(); }

    /** \copydoc List::end() const */
    Iter end() const
        { return Iter(); }

    /** Find (lookup) iterator for given value (const).
     \param  value  Value to find
     \return        Iterator, at end position if value not found
    */
    Iter iter(const Value& value) const {
        IterKey iterkey;
        const Item* item = this->getiter(iterkey, value);
        return (item != NULL ? Iter(*this, iterkey, (IterItem*)item) : Iter(*this, iterEND));
    }

    /** Find (lookup) iterator for given value (mutable).
     - \b Caution: Results are undefined if value is modified in a way that changes it's ordered position in set
     .
     \param  value  Value to find
     \return        Iterator, at end position if value not found
    */
    IterM iterM(const Value& value) {
        IterKey iterkey;
        Item* item = (Item*)this->getiter(iterkey, value);
        return (item != NULL ? IterM(*this, iterkey, (IterItem*)item) : IterM(*this, iterEND));
    }

    /** Get value, create if needed (mutable).
     - Existing value is returned as-is
     - New value is created via copy constructor
     .
     \param  value    Value to use, copied if new value is created
     \param  created  Stores whether new item was created, if not NULL
     \return          Reference to new value
    */
    virtual Value& get(const Value& value, bool* created=NULL) = 0;

    // INFO_SET

    /** Make data unique by allocating new buffer, if needed (modifier).
     - Use reserve() instead to reserve additional space while unsharing
     - Use to make buffer unique (not shared) and writable (when not empty)
     - This is called automatically by mutable/modifier methods
     - This does nothing if empty or not shared
     .
     \return  This
    */
    virtual SetBaseType& unshare() = 0;

    /** %Set capacity for set (modifier).
     - This is just a suggestion -- some sets may ignore it
     .
     \param  size  New capacity
     \return       This
    */
    virtual SetBaseType& capacity(Size size) = 0;

    /** %Set capacity to at least given minimum for set (modifier).
     - This is just a suggestion -- some sets may ignore it
     .
     \param  min  Minimum capacity
     \return      This
    */
    virtual SetBaseType& capacitymin(Size min) = 0;

    /** Reduce capacity to fit current size (modifier).
     - Call to save memory when done adding items
     - This is just a suggestion -- some implementations may ignore it
     .
     \return  This
    */
    virtual SetBaseType& compact()
        { return *this; }

    /** Reserve space for new items.
     - For best performance, call this before adding multiple values
     - This calls capacitymin() with current size + size to reserve space
     - This is just a suggestion -- some implementations may ignore it
     .
     \param  size  Size to reserve
     \return       This
    */
    SetBaseType& reserve(Size size) {
        this->capacitymin(size_ + size);
        return *this;
    }

    // ADD

    /** Add or update using given item.
     \param  value   Value to add (copied)
     \param  update  Whether to update existing item, true to overwrite existing item -- use when items that compare as equal can have diff metadata
     \return         Reference to added item
    */
    virtual Value& add(const Value& value, bool update=false) = 0;

    /** Add items from given list or set.
     \tparam  T      Container type to add from -- can be Set, List, Array, or similar
     \param  items   %List to add items from
     \param  update  Whether to update existing item, true to overwrite existing items -- use when items that compare as equal can have diff metadata
     \return         Number of new items added
    */
    template<class T>
    Size addfrom(const T& items, bool update=false) {
        if (this != &items) {
            const Size start_size = size_;
            reserve(items.size());
            for (typename T::Iter iter(items); iter; ++iter)
                this->add(*iter, update);
            return (size_ - start_size);
        }
        return 0;
    }

    /** Split delimited string into set items.
     - This parses/tokenizes str and adds each item to set, using convert() for conversion to set item type
     - %Set value types must be convertible from String via convert()
     - See joinset() to join set back into string
     .
     \tparam  T     String type to parse -- inferred from str parameter
     \param  str    String to parse
     \param  delim  Value delimiter to use
     \return        Number of items added to set
    */
    template<class T>
    Size addsplit(const T& str, char delim=',') {
        Size count = 0;
        StrTok tok(str);
        for (; tok.next(delim); ++count)
            this->add(tok.value().convert<Value>());
        return count;
    }

    // REMOVE

    /** Find and remove given value.
     - \b Caution: Removing an item may shift other iterators on same set, in an unpredictable way -- see remove(IterM&,IteratorDir)
     .
     \param  value  Value to use
     \return        Whether value was removed, false if not found
    */
    virtual bool remove(const Value& value) = 0;

    /** Remove item using given iterator.
     - This will move the iterator to the next item according to dir, or end position if no more
     - \b Caution: Removing an item may shift other iterators on same set, in an unpredictable way
     .
     \param  iter  Iterator position
     \param  dir   Direction to move iterator to next item, iterNONE for end position
     \return       Whether item was removed, false if iterator at end position
    */
    virtual bool remove(IterM& iter, IteratorDir dir=iterNONE) = 0;

    // INTERNAL

    // Iterator support methods (used internally)
    /** \cond impl */
    virtual void iterInitMutable() = 0;
    virtual const IterItem* iterFirst(IterKey& key) const = 0;
    virtual const IterItem* iterNext(IterKey& key) const = 0;
    virtual const IterItem* iterLast(IterKey& key) const = 0;
    virtual const IterItem* iterPrev(IterKey& key) const = 0;
    /** \endcond */
};

///////////////////////////////////////////////////////////////////////////////
//@}
}
#endif
