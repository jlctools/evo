// Evo C++ Library
/* Copyright 2018 Justin Crowell
Distributed under the BSD 2-Clause License -- see included file LICENSE.txt for details.
*/
///////////////////////////////////////////////////////////////////////////////
/** \file setlist.h Evo SetList container. */
#pragma once
#ifndef INCL_evo_setlist_h
#define INCL_evo_setlist_h

#include "set.h"
#include "list.h"

namespace evo {
/** \addtogroup EvoContainers */
//@{

///////////////////////////////////////////////////////////////////////////////

/** Set implemented as an ordered sequential array/list.

\tparam  TKey      %Set key type
\tparam  TCompare  Comparison type to use
\tparam  TSize     %Size type to use for size values (must be unsigned integer) -- default: SizeT

\par Features

 - Similar to STL set, with an implementation similar to STL vector
 - Values are stored sequentially in memory as a dynamic array -- random access with item() uses constant time
 - Values are always unique (no duplicates)
 - This uses binary search -- lookups are `O(log n)`, inserts are `O(n + log n)` since values must be shifted to make room
 - This uses List internally while keeping items ordered
   - No memory allocated by new empty set
   - Preallocates extra memory when buffer grows
   - \ref Sharing "Sharing" makes copying efficient
   - See List features
 .

\par Comparison

You can leave the default comparison type (Compare) or specify an alternative.

 - Compare, CompareR
 - CompareI, CompareIR
 .

See: \ref PrimitivesContainers "Primitives & Containers"

\par Iterators

 - Set::Iter -- Read-Only Iterator (IteratorRa)
 - Set::IterM -- Mutable Iterator (IteratorRa)
 .

\b Caution: Modifying or resizing a set will shift or invalidate existing iterators using it.

\par Read Access

 - size()
   - null(), empty()
   - capacity()
   - shared()
   - ordered()
   - get_compare() const
 - contains()
   - item()
 - iter()
   - iter_lower(), iter_upper()
   - cbegin(), cend()
 - operator==()
   - operator!=()
 .

\par Modifiers

 - reserve()
   - capacity(Size)
   - capacitymin()
   - unshare()
   - compact()
   - get_compare()
 - iterM()
   - iter_lowerM(), iter_upperM()
   - begin(), end()
 - set()
   - set(const ThisType&)
   - setempty()
   - clear()
   - operator=()
 - get()
 - add(const Value&,bool)
   - addfrom()
   - addsplit()
 - remove(const Value&)
   - remove(const Value&,Size)
   - remove(IterM&,IteratorDir)
   - removeat()
   - remove_range(IterM&,Size)
   - remove_range(IterM&,IterM&)
 .

\par Examples

Example using a set of numbers

\code
#include <evo/string.h>
#include <evo/setlist.h>
#include <evo/io.h>
using namespace evo;
static Console& c = con();

int main() {
    // Create set with numeric values
    SetList<int> set;
    set.add(20);
    set.add(10);

    // Add more values from delimited string
    set.addsplit("30,40");

    // Join set as delimited string and print it
    String str;
    str.join(set);
    c.out << str << NL;

    // Check if set contains value
    bool has10 = set.contains(10);      // true
    bool has50 = set.contains(50);      // false

    // Iterate and print values
    for (SetList<int>::Iter iter(set); iter; ++iter)
        c.out << *iter << NL;

    return 0;
}
\endcode

Output:
\code{.unparsed}
10,20,30,40
10
20
30
40
\endcode

Example using descending (reverse) order

\code
#include <evo/setlist.h>
#include <evo/io.h>
using namespace evo;
static Console& c = con();

int main() {
    typedef SetList<int,CompareR<int> > MySet;

    // Create set with numeric values
    MySet set;
    set.add(20);
    set.add(10);
    set.add(30);

    // Iterate and print values
    for (MySet::Iter iter(set); iter; ++iter)
        c.out << *iter << NL;

    return 0;
}
\endcode

Output:
\code{.unparsed}
30
20
10
\endcode

Example using string set:

\code
#include <evo/string.h>
#include <evo/setlist.h>
#include <evo/io.h>
using namespace evo;
static Console& c = con();

int main() {
    StrSetList strings;

    // Add values from delimited string
    strings.addsplit("a1,b1,b2,b3,c1,d1");

    // Get start iterator for values starting with PREFIX
    const String PREFIX("b");
    StrSetList::IterM iter_start(strings.iter_lowerM(PREFIX));

    // Get end iterator for values starting with PREFIX
    StrSetList::IterM iter_end(iter_start);
    while (iter_end && iter_end->starts(PREFIX))
        ++iter_end;

    // Remove values starting with PREFIX
    strings.remove_range(iter_start, iter_end);

    // Iterate and print values
    for (StrSetList::Iter iter(strings); iter; ++iter)
        c.out << *iter << NL;

    return 0;
}
\endcode

Output:
\code{.unparsed}
a1
c1
d1
\endcode
*/
template<class TKey, class TCompare=Compare<TKey>, class TSize=SizeT>
class SetList : public Set<TKey,TSize> {
public:
    EVO_CONTAINER_TYPE;
#if defined(_MSC_VER) || defined(EVO_OLDCC) // avoid errors with older compilers and MSVC
    typedef Set<TKey,TSize> SetBaseType;                ///< %Set base type
    typedef typename SetBaseType::Size Size;
    typedef typename SetBaseType::Key Key;
    typedef typename SetBaseType::Value Value;
    typedef typename SetBaseType::Item Item;
    typedef typename SetBaseType::IterKey IterKey;
    typedef typename SetBaseType::IterItem IterItem;
#else
    using typename Set<TKey,TSize>::SetBaseType;
    using typename Set<TKey,TSize>::Size;
    using typename Set<TKey,TSize>::Key;
    using typename Set<TKey,TSize>::Value;
    using typename Set<TKey,TSize>::Item;
    using typename Set<TKey,TSize>::IterKey;
    using typename Set<TKey,TSize>::IterItem;
#endif
    typedef SetList<TKey,TCompare,TSize> ThisType;      ///< This type
    typedef TCompare Compare;                           ///< %Compare type to use

    typedef typename IteratorRa<ThisType>::Const Iter;  ///< Iterator (const) - IteratorRa
    typedef IteratorRa<ThisType>                 IterM; ///< Iterator (mutable) - IteratorRa

    /** Default constructor. */
    SetList()
        { }

    /** Copy constructor.
     - Uses default comparison object
     - Results are undefined if TCompare is a function pointer since this will not initialize it
     .
     \param  src  Source to copy
    */
    SetList(const SetBaseType& src)
        { set(src); }

    /** Copy constructor.
     - This copies the comparison object as well
     .
     \param  src  Source to copy
    */
    SetList(const ThisType& src) : data_(src.data_)
        { SetBaseType::size_ = src.size_; }

    /** Destructor. */
    ~SetList()
        { }

    // SET

    /** \copydoc Set::operator=(const SetBaseType& src) */
    ThisType& operator=(const SetBaseType& src)
        { set(src); return *this; }

    /** \copydoc Set::operator=(const SetBaseType& src) */
    ThisType& operator=(const ThisType& src)
        { set(src); return *this; }

    ThisType& set() {
        data_.items.set();
        SetBaseType::size_ = 0;
        return *this;
    }

    ThisType& set(const SetBaseType& src) {
        clear();
        for (typename SetBaseType::Iter iter(src); iter; ++iter)
            add(*iter);
        return *this;
    }

    /** \copydoc Set::set(const SetBaseType& src) */
    ThisType& set(const ThisType& src) {
        ((Compare&)data_) = (const Compare&)src.data_;
        data_.items.set(src.data_.items);
        SetBaseType::size_ = src.size_;
        return *this;
    }

    ThisType& setempty() {
        data_.items.setempty();
        SetBaseType::size_ = 0;
        return *this;
    }

    ThisType& clear() {
        data_.items.clear();
        SetBaseType::size_ = 0;
        return *this;
    }

    // INFO

    bool null() const
        { return data_.items.null(); }

    bool shared() const
        { return data_.items.shared(); }

    Size capacity() const
        { return data_.items.capacity(); }

    bool ordered() const
        { return true; }

    // COMPARE

    /** Get comparison object being used for comparisons (const).
     \return  Comparison object reference
    */
    const Compare& get_compare() const
        { return data_; }

    /** Get comparison object being used for comparisons.
     \return  Comparison object reference
    */
    Compare& get_compare()
        { return data_; }

    using SetBaseType::operator==;
    using SetBaseType::operator!=;

    /** \copydoc Set::operator==(const SetBaseType& other) const */
    bool operator==(const ThisType& set) const
        { return (this == &set || data_.items == set.data_.items); }

    /** \copydoc Set::operator!=(const SetBaseType& other) const */
    bool operator!=(const ThisType& set) const
        { return (this != &set && data_.items != set.data_.items); }

    // FIND

    bool contains(const Value& value) const
        { Size i = 0; return (search(i, value) != NULL); }

    /** \copydoc Set::iter() */
    Iter iter(const Value& value) const {
        IterKey iterkey;
        const Item* item = search(iterkey.a, value);
        return (item != NULL ? Iter(*this, iterkey, (IterItem*)item) : Iter(*this, iterEND));
    }

    /** Find first value greater or equal to given value (lower bound) (const).
     - This gives the lower-bound iterator for the given value according to the ordering of values
     - With a set of strings, this is useful for finding string values beginning with a prefix:
       - Use this to get a starting iterator, then loop while iterator value begins with the desired prefix (or reaches end)
     - Use with iter_upper() to get iterators representing a range of values in set
     .
     \param  value  Value to look for
     \return        Iterator, at end if value is greater than the last item, or if set is empty
     \see iter_lowerM(), iter_upper()
    */
    Iter iter_lower(const Value& value) const {
        IterKey iterkey;
        const Item* item = search(iterkey.a, value);
        if (item != NULL)
            return Iter(*this, iterkey, (IterItem*)item);
        if (iterkey.a < data_.items.size())
            return Iter(*this, iterkey, (IterItem*)&data_.items.item(iterkey.a));
        return Iter(*this, iterEND);
    }

    /** Find first value greater or equal to given value (lower bound).
     - This gives the lower-bound iterator for the given value according to the ordering of values
     - With a set of strings, this is useful for finding string values beginning with a prefix:
       - Use this to get a starting iterator, then loop while iterator value begins with the desired prefix (or reaches end)
     - Use with iter_upper() to get iterators representing a range of values in set
     .
     \param  value  Value to look for
     \return        Iterator, at end if value is greater than the last item, or if set is empty
     \see iter_lower(), iter_upperM()
    */
    IterM iter_lowerM(const Value& value) {
        IterKey iterkey;
        const Item* item = search(iterkey.a, value);
        if (item != NULL)
            return IterM(*this, iterkey, (IterItem*)item);
        if (iterkey.a < data_.items.size())
            return IterM(*this, iterkey, (IterItem*)&data_.items.item(iterkey.a));
        return IterM(*this, iterEND);
    }

    /** Find first value greater than given value (upper bound) (const).
     - This gives the upper-bound iterator for the given value according to the ordering of values
     - Use with iter_lower() to get iterators representing a range of values in set
     .
     \param  value  Value to look for
     \return        Iterator, at end if value is less than the first item, or if set is empty
     \see iter_upperM(), iter_lower()
    */
    Iter iter_upper(const Value& value) const {
        IterKey iterkey;
        if (search(iterkey.a, value) != NULL)
            ++iterkey.a;
        if (iterkey.a < data_.items.size())
            return Iter(*this, iterkey, (IterItem*)&data_.items.item(iterkey.a));
        return Iter(*this, iterEND);
    }

    /** Find first value greater than given value (upper bound).
     - This gives the upper-bound iterator for the given value according to the ordering of values
     - Use with iter_lower() to get iterators representing a range of values in set
     .
     \param  value  Value to look for
     \return        Iterator, at end if value is less than the first item, or if set is empty
     \see iter_upper(), iter_lowerM()
    */
    IterM iter_upperM(const Value& value) {
        IterKey iterkey;
        if (search(iterkey.a, value) != NULL)
            ++iterkey.a;
        if (iterkey.a < data_.items.size())
            return IterM(*this, iterkey, (IterItem*)&data_.items.item(iterkey.a));
        return IterM(*this, iterEND);
    }

    /** \copydoc Set::iterM() */
    IterM iterM(const Value& value) {
        IterKey iterkey;
        const Item* item = search(iterkey.a, value);
        return (item != NULL ? IterM(*this, iterkey, (IterItem*)item) : IterM(*this, iterEND));
    }

    Value& get(const Value& value, bool* created=NULL) {
        Item* item;
        Size pos;
        if ( (item=(Item*)search(pos, value)) == NULL) {
            item = &(data_.items.itemM(data_.items.insert(pos, &value, 1)));
            if (created != NULL)
                *created = true;
            ++SetBaseType::size_;
        } else if (created != NULL)
            *created = false;
        return *item;
    }

    /** Get item value at position (const).
     - Results are undefined if index is out of bounds
     .
     \param  index  Item index
     \return        Reference to value as read-only (const)
    */
    const Value& item(Size index) const
        { return data_.items.item(index); }

    // INFO_SET

    ThisType& unshare()
        { data_.items.unshare(); return *this; }

    ThisType& capacity(Size size)
        { data_.items.capacity(size); return *this; }

    ThisType& capacitymin(Size min)
        { data_.items.capacitymin(min); return *this; }

    ThisType& compact()
        { data_.items.compact(); return *this; }

    // ADD

    Value& add(const Value& item, bool update=false) {
        bool created_val;
        Value& upditem = get(item, &created_val);
        if (!created_val && update)
            upditem = item;
        return upditem;
    }

    // REMOVE

    bool remove(const Value& value) {
        Size index;
        if (search(index, value) != NULL) {
            data_.items.remove(index);
            --SetBaseType::size_;
            return true;
        }
        return false;
    }

    /** Find and remove value and following values.
     - \b Caution: Removing items may shift other iterators on same set, in an unpredictable way -- see remove(IterM&,IteratorDir)
     .
     \param  value  Value to to find and remove
     \param  count  Number of values to remove, ALL for all remaining values
     \return        Number of value removed
    */
    Size remove(const Value& value, Size count) {
        Size index;
        if (count > 0 && search(index, value) != NULL) {
            count = data_.items.remove(index, count);
            SetBaseType::size_ -= count;
            return count;
        }
        return 0;
    }

    bool remove(typename SetBaseType::IterM& iter, IteratorDir dir=iterNONE)
        { return remove((IterM&)iter, dir); }

    /** \copydoc Set::remove(IterM&,IteratorDir) */
    bool remove(IterM& iter, IteratorDir dir=iterNONE) {
        if (iter && this == &iter.getParent()) {
            IterKey& iterkey = iter.getKey();
            data_.items.remove(iterkey.a);
            bool nextitem = false;
            if (--SetBaseType::size_ > 0 && dir != iterNONE) {
                if (dir == iterRV) {
                    if (iterkey.a > 0)
                        { --iterkey.a; nextitem = true; }
                } else if (iterkey.a < data_.items.size())
                    nextitem = true;
            }
            if (nextitem)
                iter.setData( (IterItem*)&data_.items.item(iterkey.a) );
            else 
                iter = iterEND;
            return true;
        }
        return false;
    }

    /** Remove value(s) at given position/index (mutable).
     - Results are undefined if index is out of bounds
     - Calls unshare()
     .
     \param  index  Value index to start removing
     \param  count  Number of value to remove, ALL for all values after index
     \return        Number of values removed
    */
    Size removeat(Size index, Size count=1) {
        assert( index < data_.items.size() );
        count = data_.items.remove(index, count);
        SetBaseType::size_ -= count;
        return count;
    }

    /** Remove range of values using iterator (mutable).
     - This removes `count` values starting from (and including) `start`
     - This is more efficient than removing items 1 by 1
     - No values are removed if start is at end, or count is 0
     .
     \param  start  Iterator to first value to remove, updated to next remaining value (or end if no more)
     \param  count  Count of values to remove, ALL for all
     \return        Number of values removed, which may be less than count
    */
    Size remove_range(IterM& start, Size count) {
        if (count > 0 && start && this == &start.getParent()) {
            count = data_.items.remove(start.index().a, count);
            SetBaseType::size_ -= count;
            if (start.index().a >= SetBaseType::size_)
                start = iterEND;
            return count;
        }
        return 0;
    }

    /** Remove range of values using iterators (mutable).
     - This removes values starting from (and including) `start`, ending at (but not including) `end`
     - This is more efficient than removing items 1 by 1
     - No values are removed if start is at end, or if start >= end
     .
     \param  start  Iterator to first value to remove, updated to next remaining value (or end if no more)
     \param  end    Iterator to remove to, updated to end
     \return        Number of values removed, which may be less than count
    */
    Size remove_range(IterM& start, IterM& end) {
        if (start && this == &start.getParent()) {
            const Size index = start.index().a;
            if (end) {
                if (this == &end.getParent()) {
                    const Size end_index = end.index().a;
                    if (end_index > index) {
                        const Size count = data_.items.remove(index, end_index - index);
                        SetBaseType::size_ -= count;
                        end = iterEND;
                        return count;
                    }
                }
            } else {
                const Size count = data_.items.remove(index, ALL);
                SetBaseType::size_ -= count;
                start = iterEND;
                return count;
            }
        }
        return 0;
    }

    // INTERNAL

    // Iterator support methods
    /** \cond impl */
    void iterInitMutable()
        { data_.items.iterInitMutable(); }
    const IterItem* iterFirst(IterKey& key) const
        { return (IterItem*)data_.items.iterFirst(key.a); }
    const IterItem* iterNext(IterKey& key) const
        { return (IterItem*)data_.items.iterNext(key.a); }
    const IterItem* iterNext(Size count, IterKey& key) const
        { return (IterItem*)data_.items.iterNext(count, key.a); }
    const IterItem* iterLast(IterKey& key) const
        { return (IterItem*)data_.items.iterLast(key.a); }
    const IterItem* iterPrev(IterKey& key) const
        { return (IterItem*)data_.items.iterPrev(key.a); }
    /** \endcond */

protected:
    const Value* getiter(IterKey& iterkey, const Value& value) const
        { return search(iterkey.a, value); }

private:
    // Use inheritance to reduce size bloat with empty Compare
    struct Data : public TCompare {
        using TCompare::operator();

        List<Item> items;

        Data()
            { }
        Data(const TCompare& compare) : TCompare(compare)
            { }
        Data(const Data& data) : TCompare(data), items(data.items)
            { }
    };

    Data data_;

    const Value* search(Size& index, const Value& value) const {
        int cmp;
        Size left = 0, right = data_.items.size(), mid = 0;
        while (left < right) {
            mid = left + ((right - left) / 2);
            const Value& item = data_.items.item(mid);
            cmp = data_(value, item);
            if (cmp < 0) {
                right = mid;
            } else if (cmp == 0) {
                index = mid;
                return &item;
            } else
                left = mid + 1;
        }
        index = left;
        return NULL;
    }
};

///////////////////////////////////////////////////////////////////////////////

#if defined(INCL_evo_string_h) || defined(DOXYGEN)
/** SetList using String values.
 - This is an alias to SetList
 - This is only defined if string.h is included before setlist.h
*/
typedef SetList<String> StrSetList;
#endif

///////////////////////////////////////////////////////////////////////////////
//@}
}
#endif
