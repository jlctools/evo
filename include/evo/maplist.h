// Evo C++ Library
/* Copyright 2018 Justin Crowell
Distributed under the BSD 2-Clause License -- see included file LICENSE.txt for details.
*/
///////////////////////////////////////////////////////////////////////////////
/** \file maplist.h Evo MapList container. */
#pragma once
#ifndef INCL_evo_maplist_h
#define INCL_evo_maplist_h

#include "map.h"
#include "list.h"
#include "strtok.h"

namespace evo {
/** \addtogroup EvoContainers */
//@{

///////////////////////////////////////////////////////////////////////////////

/** Map implemented as an ordered list. This uses List internally.

\tparam  TKey      %Map key type
\tparam  TValue    %Map value type
\tparam  TCompare  Comparison type to use
\tparam  TSize     %Size type to use for size values (must be unsigned integer) -- default: SizeT

\par Comparison

You can leave the default comparison type (Compare) or specify an alternative.

 - Compare, CompareR
 - CompareI, CompareIR
 .

See: \ref PrimitivesContainers "Primitives & Containers"

\par Iterators

 - Map::Iter -- Read-Only Iterator (IteratorRa)
 - Map::IterM -- Mutable Iterator (IteratorRa)
 .

\b Caution: Modifying or resizing a map will shift or invalidate existing iterators using it.

\par Read Access

 - size()
   - null(), empty()
   - capacity()
   - shared()
   - ordered()
 - contains(const Key&) const
   - contains(const Key&, const Value&) const
 - find()
   - findindex()
   - iter()
   - item()
 - operator==()
   - operator!=()
 .

\par Modifiers

 - reserve()
   - compact()
   - capacity(Size)
   - capacitymin()
   - unshare()
 - findM()
   - iterM()
   - getitem()
   - get()
   - operator[]()
 - set()
   - set(const MapBaseType&)
   - set(const ThisType&)
   - setempty()
   - clear()
   - operator=(const MapBaseType&)
   - operator=(const ThisType&)
 - add(const Key&,const Value&,bool)
   - add(const Item&,bool)
   - add(const MapBaseType&,bool)
 - remove(const Key&)
   - remove(MapBaseType::IterM&,IteratorDir)
   - removeat()
 .

\par Example

Example using a map of numbers

\code
#include <evo/string.h>
#include <evo/maplist.h>
#include <evo/io.h>
using namespace evo;
static Console& c = con();

int main() {
    typedef MapList<int,int> MyMap;

    // Create map with numeric keys and values
    MyMap map;
    map.add(2, 20);
    map[1] = 10;

    // Add more values from delimited string
    map.addsplit("3=30,4=40");

    // Join map as delimited string and print it
    String str;
    str.joinmap(map);
    c.out << str << NL;

    // Check if map contains key
    bool has1 = map.contains(1);        // true
    bool has5 = map.contains(5);        // false

    // Check if map contains key and value
    bool has1_10 = map.contains(1, 10); // true
    bool has1_20 = map.contains(1, 20); // false
    bool has5_50 = map.contains(5, 50); // false

    // Find and modify value
    int* value = map.findM(1);
    if (value != NULL)
        *value = 100;

    // Iterate and print keys and values
    for (MyMap::Iter iter(map); iter; ++iter)
        c.out << iter->key() << "=" << iter->value() << NL;

    return 0;
}
\endcode

Output:
\code{.unparsed}
1=10,2=20,3=30,4=40
1=100
2=20
3=30
4=40
\endcode

Example using descending (reverse) order

\code
#include <evo/maplist.h>
#include <evo/io.h>
using namespace evo;
static Console& c = con();

int main() {
    typedef MapList<int,int,CompareR<int> > MyMap;

    // Create map with numeric keys and values
    MyMap map;
    map.add(2, 20);
    map[1] = 10;
    map.get(3) = 30;

    // Iterate and print keys and values
    for (MyMap::Iter iter(map); iter; ++iter)
        c.out << iter->key() << "=" << iter->value() << NL;

    return 0;
}
\endcode

Output:
\code{.unparsed}
3=30
2=20
1=10
\endcode
*/
template<class TKey, class TValue, class TCompare=Compare<TKey>, class TSize=SizeT>
class MapList : public Map<TKey,TValue,TSize> {
public:
    EVO_CONTAINER_TYPE;
#if defined(_MSC_VER) || defined(EVO_OLDCC) // avoid errors with older compilers and MSVC
    typedef Map<TKey,TValue,TSize> MapBaseType;             ///< %Map base type
    typedef typename MapBaseType::Size Size;
    typedef typename MapBaseType::Key Key;
    typedef typename MapBaseType::Value Value;
    typedef typename MapBaseType::Item Item;
    typedef typename MapBaseType::IterKey IterKey;
    typedef typename MapBaseType::IterItem IterItem;
#else
    using typename Map<TKey,TValue,TSize>::MapBaseType;
    using typename Map<TKey,TValue,TSize>::Size;
    using typename Map<TKey,TValue,TSize>::Key;
    using typename Map<TKey,TValue,TSize>::Value;
    using typename Map<TKey,TValue,TSize>::Item;
    using typename Map<TKey,TValue,TSize>::IterKey;
    using typename Map<TKey,TValue,TSize>::IterItem;
#endif
    typedef MapList<TKey,TValue,TCompare,TSize> ThisType;   ///< This type
    typedef TCompare Compare;                               ///< %Compare type to use

    typedef typename IteratorRa<ThisType>::Const Iter;      ///< Iterator (const) - IteratorRa
    typedef IteratorRa<ThisType>                 IterM;     ///< Iterator (mutable) - IteratorRa

    using MapBaseType::size_;

    /** Constructor. */
    MapList()
        { }

    /** Copy constructor.
     - Uses default comparison object
     - Results are undefined if TCompare is a function pointer since this will not initialize it
     .
     \param  src  Source to copy
    */
    MapList(const MapBaseType& src)
        { set(src); }

    /** Copy constructor.
     - This copies the comparison object as well
     .
     \param  src  Source to copy
    */
    MapList(const ThisType& src) : data_(src.data_)
        { size_ = src.size_; }

    /** Destructor. */
    ~MapList()
        { }

    // SET

    /** \copydoc Map::operator=() */
    ThisType& operator=(const MapBaseType& src)
        { set(src); return *this; }

    /** Assignment operator.
     - This copies the comparison object as well
     .
     \param  src  Source to copy
     \return      This
    */
    ThisType& operator=(const ThisType& src) {
        data_.items = src.data_.items;
        MapBaseType::size_ = src.size_;
        return *this;
    }

    ThisType& set() {
        data_.items.set();
        MapBaseType::size_ = 0;
        return *this;
    }

    ThisType& set(const MapBaseType& src) {
        clear();
        for (typename MapBaseType::Iter iter(src); iter; ++iter)
            this->get(iter->first) = iter->second;
        return *this;
    }

    /** %Set as copy of given MapList map.
     - This copies the comparison object as well
     .
     \param  src  Source map
     \return      This
    */
    ThisType& set(const ThisType& src) {
        ((Compare&)data_) = (const Compare&)src.data_;
        data_.items.set(src.data_.items);
        MapBaseType::size_ = src.size_;
        return *this;
    }

    ThisType& setempty() {
        data_.items.setempty();
        MapBaseType::size_ = 0;
        return *this;
    }

    ThisType& clear() {
        data_.items.clear();
        MapBaseType::size_ = 0;
        return *this;
    }

    // INFO

    bool null() const
        { return data_.items.null(); }

    bool shared() const
        { return data_.items.shared(); }

    Size capacity() const
        { return data_.items.capacity(); }

    // COMPARE

    using MapBaseType::operator==;
    using MapBaseType::operator!=;

    /** Equality operator.
     \param  map  Map to compre
     \return      Whether maps are equal (have same keys and values)
    */
    bool operator==(const ThisType& map) const
        { return (this == &map || data_.items == map.data_.items); }

    /** Inequality operator.
     \param  map  Map to compre
     \return      Whether maps are not equal (don't have same keys and values)
    */
    bool operator!=(const ThisType& map) const
        { return (this != &map && data_.items != map.data_.items); }

    // FIND

    bool contains(const Key& key) const {
        Size pos;
        return (search(pos, key) != NULL);
    }

    bool contains(const Key& key, const Value& value) const {
        Size pos;
        const Item* item = search(pos, key);
        return (item != NULL && item->second == value);
    }

    const Value* find(const Key& key) const {
        Size pos;
        const Item* item = search(pos, key);
        return (item != NULL ? &item->second : NULL);
    }

    Value* findM(const Key& key) {
        Size pos;
        const Item* item = search(pos, key);
        return (item != NULL ? (Value*)&item->second : NULL);
    }

    /** Find (lookup) index for given key (const).
     \param  key  Key to find
     \return      Found index for item key, END if not found
    */
    Size findindex(const Key& key) const
        { Size pos; return (search(pos, key) == NULL ? END : pos); }

    /** \copydoc Map::iter() */
    Iter iter(const Key& key) const {
        IterKey iterkey;
        const Item* item = search(iterkey.a, key);
        return (item != NULL ? Iter(*this, iterkey, (IterItem*)item) : Iter(*this, iterEND));
    }

    /** \copydoc Map::iterM() */
    IterM iterM(const Key& key) {
        IterKey iterkey;
        const Item* item = search(iterkey.a, key);
        return (item != NULL ? IterM(*this, iterkey, (IterItem*)item) : IterM(*this, iterEND));
    }

    Item& getitem(const Key& key, bool* created=NULL) {
        Item* item;
        Size pos;
        if ( (item=(Item*)search(pos, key)) == NULL) {
            item = &data_.items.itemM(data_.items.insertnew(pos));
            item->first = key;
            if (created != NULL)
                *created = true;
            ++MapBaseType::size_;
        } else if (created != NULL)
            *created = false;
        return *item;
    }

    /** \copydoc Map::get() */
    Value& get(const Key& key, bool* created=NULL)
        { return getitem(key, created).second; }

    /** Get item at position (const).
     - Results are undefined if index is out of bounds
     .
     \param  index  Item index
     \return        Item as read-only (const)
    */
    const Item& item(Size index) const
        { return data_.items.item(index); }

    /** Get item at position (mutable).
     - Results are undefined if index is out of bounds
     - Calls unshare()
     .
     \param  index  Item index
     \return        Item (mutable)
    */
    Item& itemM(Size index)
        { return data_.items.itemM(index); }

    // INFO_SET

    /** \copydoc Map::operator[]() */
    Value& operator[](const Key& key)
        { return getitem(key, NULL).second; }

    ThisType& unshare()
        { data_.items.unshare(); return *this; }

    ThisType& capacity(Size size)
        { data_.items.capacity(size); return *this; }

    ThisType& capacitymin(Size min)
        { data_.items.capacitymin(min); return *this; }

    ThisType& compact()
        { data_.items.compact(); return *this; }

    /** \copydoc Map::reserve() */
    ThisType& reserve(Size size)
        { capacitymin(this->size_ + size); return *this; }

    // ADD

    Item& add(const Key& key, const Value& value, bool update=true) {
        bool created_val;
        Item& upditem = getitem(key, &created_val);
        if (created_val || update)
            upditem.second = value;
        return upditem;
    }

    Item& add(const Item& item, bool update=true)
        { return add(item.first, item.second, update); }

    ThisType& add(const MapBaseType& map, bool update=true) {
        if (this != &map) {
            reserve(map.size());
            for (typename MapBaseType::Iter iter(map); iter; ++iter)
                add(iter->first, iter->second, update);
        }
        return *this;
    }

    // MOVE

    using MapBaseType::move;

    /** Move given item from another map.
     - This has the same effect as doing an add() then remove() on src map
     .
     \param  src  Source iterator in other map to move from
     \param  dir  Direction to move src iterator to next item, iterNONE for end position
     \return      This
    */
    ThisType& move(IterM& src, IteratorDir dir=iterNONE) {
        if (src) {
            ThisType& srcObj = src.getParent();
            if (&srcObj != this) {
                // Move item directly
                IterKey& iterkey = src.getKey();
                Item* item;
                Size pos;
                if ( (item=(Item*)search(pos, src->first)) == NULL) {
                    // Move item
                    bool nextitem = false;
                    data_.items.move(pos, srcObj.data_.items, iterkey.a, 1);
                    ++MapBaseType::size_;

                    // Adjust iterator
                    if (--srcObj.MapBaseType::size_ > 0 && dir != iterNONE) {
                        if (dir == iterRV) {
                            if (iterkey.a > 0)
                                { --iterkey.a; nextitem = true; }
                        } else if (iterkey.a < srcObj.data_.items.size())
                            nextitem = true;
                    }
                    if (nextitem)
                        src.setData( (IterItem*)&srcObj.data_.items.item(iterkey.a) );
                    else
                        src = iterEND;
                } else {
                    // Key already exists, overwrite item value
                    item->second = src->second;
                    srcObj.remove(src, dir);
                }
            }
        }
        return *this;
    }

    /** Move given item from another map.
     - This has the same effect as doing an add() then remove() on src map
     - Note: Though src iterator as passed as const reference, it's still set to end position to maintain data integrity
     .
     \param  src  Source iterator in other map to move from
     \return      This
    */
    ThisType& move(const IterM& src)
        { return move((IterM&)src, iterNONE); }

    // REMOVE

    bool remove(const Key& key) {
        bool result = false;
        Size index;
        if (search(index, key) != NULL) {
            data_.items.remove(index);
            --MapBaseType::size_;
            result = true;
        }
        return result;
    }

    bool remove(typename MapBaseType::IterM& iter, IteratorDir dir=iterNONE)
        { return remove((IterM&)iter, dir); }

    bool remove(IterM& iter, IteratorDir dir=iterNONE) {
        if (iter && this == &iter.getParent()) {
            IterKey& iterkey = iter.getKey();
            data_.items.remove(iterkey.a);
            bool nextitem = false;
            if (--MapBaseType::size_ > 0 && dir != iterNONE) {
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

    /** Remove item at given position (mutable).
     - Results are undefined if index is out of bounds
     - Calls unshare()
     .
     \param  index  Item index
    */
    void removeat(Size index) {
        assert( index < data_.items.size() );
        data_.items.remove(index);
        --MapBaseType::size_;
    }

    // ADVANCED

    /** Advanced: Get internal list (const).
     - \b Caution: The list must remain sorted, otherwise results are undefined
     .
     \return  Internal list
    */
    const List<Item>& advList() const
        { return data_.items; }

    /** Advanced: Get internal list.
     - \b Caution: The list must remain sorted, otherwise results are undefined
     .
     \return  Internal list
    */
    List<Item>& advList()
        { return data_.items; }

    // INTERNAL

    // Iterator support methods
    /** \cond impl */
    void iterInitMutable()
        { data_.items.iterInitMutable(); }
    const IterItem* iterFirst(IterKey& key) const
        { return (IterItem*)data_.items.iterFirst(key.a); }
    const IterItem* iterNext(IterKey& key) const
        { return (IterItem*)data_.items.iterNext(key.a); }
    const IterItem* iterLast(IterKey& key) const
        { return (IterItem*)data_.items.iterLast(key.a); }
    const IterItem* iterPrev(IterKey& key) const
        { return (IterItem*)data_.items.iterPrev(key.a); }
    /** \endcond */

protected:
    const Item* getiter(IterKey& iterkey, const Key& key) const
        { return search(iterkey.a, key); }

private:
    struct CompareSwapNone {
        static void swap(Compare&, Compare&)
            { }
    };
    struct CompareSwapMem {
        static void swap(Compare& a, Compare& b) {
            Compare t;
            t = b;
            b = a;
            a = t;
        }
    };
    typedef List<Item> Items;

    // Use inheritance to reduce size bloat with empty Compare
    struct Data : public Compare {
        Items items;

        Data() { }
        Data(const Compare& compare) : Compare(compare) { }
        Data(const Items& items) : items(items) { }
        Data(const Data& data) : Compare(data), items(data.items) { }
    };
    Data data_;

    const Item* search(Size& index, const Key& key) const {
        int cmp;
        Size left = 0, right = data_.items.size(), mid = 0;
        while (left < right) {
            mid = left + ((right-left) / 2);
            const Item& item = data_.items.item(mid);
            cmp = data_.Compare::operator()(key, item.first);
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
/** MapList using String keys and values.
 - This is an alias to MapList
 - This is only defined if string.h is included before maplist.h
*/
typedef MapList<String,String> StrMapList;
#endif

///////////////////////////////////////////////////////////////////////////////
//@}
}
#endif
