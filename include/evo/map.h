// Evo C++ Library
/* Copyright (c) 2016 Justin Crowell
 This Source Code Form is subject to the terms of the Mozilla Public
 License, v. 2.0. If a copy of the MPL was not distributed with this
 file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
///////////////////////////////////////////////////////////////////////////////
/** \file map.h Evo Map interface and MapList container. */
#pragma once
#ifndef INCL_evo_map_h
#define INCL_evo_map_h

// Includes
#include "list.h"
#include "strtok.h"

// Namespace: evo
namespace evo {

/** \addtogroup EvoContainers */
//@{

///////////////////////////////////////////////////////////////////////////////

// Use of "virtual" in Map interface is configurable
/** \cond impl */
#if EVO_MAP_VIRTUAL
    #define VIRTUAL virtual
    #define VIRTUAL_PURE = 0;
#else
    #define VIRTUAL
    #define VIRTUAL_PURE STATIC_ASSERT_FUNC_UNUSED
#endif
/** \endcond */

///////////////////////////////////////////////////////////////////////////////

/** Base interface for associative containers storing key/value pairs.

\tparam  TKey    %Map key type
\tparam  TValue  %Map value type
\tparam  TSize   %Size type to use for size values (must be unsigned integer) -- default: SizeT

\par %Map Types

 - MapList (ordered)
 - MapHash (unordered)
 .

\par Iterators

 - Map::Iter -- Read-Only Iterator (IteratorBi)
 - Map::IterM -- Mutable Iterator (IteratorBi)
 .

\b Caution: Modifying or resizing a map will shift or invalidate existing iterators using it.

\par Read Access

 - size()
   - null(), empty()
   - capacity()
   - shared()
 - type()
   - ordered()
 - find()
   - iter()
 - operator==()
   - operator!=()
 .

\par Modifiers

 - reserve()
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
   - setempty()
   - clear()
 - add(const Item&,bool)
   - add(const MapBaseType&)
 - remove(const Key&)
   - remove(IterM&,IteratorDir)
 - move(IterM&,IteratorDir)
 .

\par Example

This example works with any Map type.

\code
// Create map with numeric keys and values, set some key/values
MapList<int,int> map;
map[10] = 100;
map[20] = 200;

// Find (const), print value
const int* const_value = map.find(10);
if (const_value != NULL)
    printf("Value:%i\n", *const_value);

// Find (mutable), modify value
int* value = map.findM(10);
if (value != NULL)
    *value *= 10;

// Lookup iterator and print key and value
MapList<int,int>::Iter iter = map.iter(20);
if (iter)
    printf("Key:%i, Value:%i\n", iter->key, iter->value);
\endcode
*/
template<class TKey, class TValue, class TSize=SizeT>
class Map
{
public:
    EVO_CONTAINER_TYPE;
    typedef TSize             Size;                ///< Size integer type
    typedef TKey              Key;                ///< Key type
    typedef TValue            Value;            ///< Value type
    typedef KeyVal<Key,Value> Item;                ///< Item (key/value pair) type

    typedef Map<Key,Value,Size> ThisType;        ///< This type
    typedef Map<Key,Value,Size> MapBaseType;    ///< Map base type (used when passing as parameter)

    // Iterator support types
    /** \cond impl */
    struct IterKey {
        Size a, b;

        IterKey(Size a=0, Size b=0) : a(a), b(b)
            { }
        IterKey(const IterKey& src)
            { memcpy(this, &src, sizeof(src)); }
        IterKey& operator=(const IterKey& src)
            { memcpy(this, &src, sizeof(src)); return *this; }
    };
    typedef KeyVal<const Key,Value> IterItem;
    /** \endcond */

    typedef typename IteratorBi<ThisType>::Const Iter;    ///< Iterator (const) - IteratorBi
    typedef IteratorBi<ThisType>                 IterM;    ///< Iterator (mutable) - IteratorBi

protected:
    Size       size_;            ///< Map size (number of items, automatically updated by concrete map members)
    bool       ordered_;        ///< Whether map is ordered (keys are kept in order)

    /** Constructor. Used by derived types. */
    Map() : size_(0), ordered_(true)
        { }

    /** Constructor. Used by derived types.
     \param  ordered  Whether map is ordered
    */
    Map(bool ordered) : size_(0), ordered_(ordered)
        { }

    /** Copy constructor (disabled). */
    Map(const Map&);

    /** Used by base class to get data to initialize iterator.
     \param  iterkey  Set to iterator data, if item found
     \param  key      Key to find
     \return          Item pointer, NULL if not found
    */
    VIRTUAL const Item* getiter(IterKey& iterkey, const Key& key) const VIRTUAL_PURE;

public:
    /** Assignment operator.
     \param  src  Source to copy
     \return      This
    */
    MapBaseType& operator=(const MapBaseType& src) {
        set(src);
        return *this;
    }

    /** Destructor. */
    VIRTUAL ~Map()
        { }

    // SET

    /** Clear by removing all items.
     - Does not set as null -- null status is unchanged
     .
     \return  This
    */
    VIRTUAL MapBaseType& clear() VIRTUAL_PURE

    /** Set as null and empty.
     \return  This
    */
    VIRTUAL MapBaseType& set() VIRTUAL_PURE

    /** Set as copy of given map.
     - This clears all items in current map.
     .
     \param  src  Source map
     \return      This
    */
    VIRTUAL MapBaseType& set(const MapBaseType& src) VIRTUAL_PURE

    /** Set as empty but not null.
     \return  This
    */
    VIRTUAL MapBaseType& setempty() VIRTUAL_PURE

    // INFO

    /** Get whether map is ordered.
     - Ordered maps keep items in order by key so iteration is predictable
     .
    */
    bool ordered() const
        { return ordered_; }

    /** Get whether map is null.
     \return  Whether null
    */
    VIRTUAL bool null() const VIRTUAL_PURE

    /** Get whether map is empty (size is 0).
     \return  Whether empty
    */
    bool empty() const
        { return (size_ == 0); }

    /** Get map size (number of items).
     \return  Map size
    */
    Size size() const
        { return size_; }

    /** Get whether shared.
     - Data is shared when referencing external data or buffer is allocated and shared (reference count > 1)
     .
     \return  Whether shared
    */
    //[tags: shared, reserve() ]
    VIRTUAL bool shared() const VIRTUAL_PURE

    /** Get map capacity.
     \return  Current capacity
    */
    VIRTUAL Size capacity() const VIRTUAL_PURE

    // COMPARE

    /** Equality operator.
     \param  map  Map to compre
     \return      Whether maps are equal (have same keys and values)
    */
    bool operator==(const MapBaseType& map) const {
        bool result = false;
        if (this == &map)
            result = true;
        else if (this->null())
            result = map.null();
        else if (map.null())
            result = false;
        else if (size_ == map.size_) {
            result = true;
            if (size_ > 0) {
                const Value* val;
                for (Iter iter(*this); iter; ++iter) {
                    val = map.find(iter->key);
                    if (val == NULL || !(*val == iter->value))
                        { result = false; break; }
                }
            }
        }
        return result;
    }

    /** Inequality operator.
     \param  map  Map to compre
     \return      Whether maps are not equal (don't have same keys and values)
    */
    bool operator!=(const MapBaseType& map) const
        { return !this->operator==(map); }

    // FIND

    /** Find (lookup) value for given key (const).
     \param  key  Key to find
     \return      Found value const pointer, NULL if not found
    */
    VIRTUAL const Value* find(const Key& key) const VIRTUAL_PURE

    /** Find (lookup) value for given key (mutable).
     \param  key  Key to find
     \return      Found value pointer, NULL if not found
    */
    VIRTUAL Value* findM(const Key& key) VIRTUAL_PURE

    /** Find (lookup) iterator for given key (const).
     \param  key  Key to find
     \return      Iterator, at end position if not key not found
    */
    Iter iter(const Key& key) const {
        IterKey iterkey;
        const Item* item = this->getiter(iterkey, key);
        return (item != NULL ? Iter(*this, iterkey, (IterItem*)item) : Iter(*this, iterEnd));
    }

    /** Find (lookup) iterator for given key (mutable).
     \param  key  Key to find
     \return      Iterator, at end position if not key not found
    */
    IterM iterM(const Key& key) {
        IterKey iterkey;
        Item* item = (Item*)this->getiter(iterkey, key);
        return (item != NULL ? IterM(*this, iterkey, (IterItem*)item) : IterM(*this, iterEnd));
    }

    /** Get map item for key (mutable).
     - Item is created with default value if not found.
     .
     \param  key      Key to use
     \param  created  Stores whether new item was created, if not NULL
     \return          Value reference for key
    */
    VIRTUAL Item& getitem(const Key& key, bool* created=NULL) VIRTUAL_PURE

    /** Get item value for key (mutable).
     - Item is created with default value if not found.
     .
     \param  key      Key to use
     \param  created  Stores whether new item was created, if not NULL
     \return          Value reference for key
    */
    Value& get(const Key& key, bool* created=NULL)
        { return this->getitem(key, created).value; }

    // TODO: contains(key), contains(key, value)

    // INFO_SET

    /** Get item value for key (mutable).
     - Item is created with default value if not found.
     - Same as get(const Key&)
     .
     \param  key  Key to use
     \return      Value reference for key
    */
    Value& operator[](const Key& key)
        { return this->getitem(key, NULL).value; }

    /** Make data unique by allocating new buffer, if needed (modifier).
     - Use reserve() instead to reserve additional space while unsharing
     - Use to make buffer unique (not shared) and writable (when not empty)
     - This is called automatically by mutable/modifier methods
     - This does nothing if empty or not shared
     .
     \return  This
    */
    //[tags: shared, reserve() ]
    VIRTUAL MapBaseType& unshare() VIRTUAL_PURE

    /** Set map capacity.
     - This is just a suggestion -- some maps may ignore it
     .
     \param  size  New capacity
     \return       This
    */
    VIRTUAL MapBaseType& capacity(Size size) VIRTUAL_PURE

    /** Set map capacity to at least given minimum.
     - This is just a suggestion -- some maps may ignore it
     .
     \param  min  Minimum capacity
     \return      This
    */
    VIRTUAL MapBaseType& capacitymin(Size min) VIRTUAL_PURE

    /** Reserve space for new items.
     - This calls capacitymin() with current size + size to reserve
     .
     \param  size  Size to reserve
     \return       This
    */
    MapBaseType& reserve(Size size)
        { this->capacitymin(size_ + size); return *this; }

    // ADD

    /** Add or update using given item.
     \param  item    Item to use for add/update (copied)
     \param  update  Whether to update existing item
     \return         Item added/updated
    */
    VIRTUAL Item& add(const Item& item, bool update=true) VIRTUAL_PURE

    /** Add items from given map, replacing existing items.
     \param  map  %Map to add items from
     \return      This
    */
    VIRTUAL MapBaseType& add(const MapBaseType& map) VIRTUAL_PURE

    // TODO
    /** Split delimited string into map key/value items.
     - This parses/tokenizes str and adds each item to map, using convert() for conversion to map key and value types
     - Map key and value types must be convertible from String via convert()
     - See joinmap() to join map back into string
     .
     \tparam  T       String type to parse -- inferred from str parameter
     \param  str      String to parse
     \param  delim    Item delimiter to use
     \param  kvdelim  Key/Value delimiter to use
     \return          Number of items added to map
    */
    template<class T>
    Size addsplit(const T& str, char delim=',', char kvdelim='=') {
        char delims_buf[2];
        delims_buf[0] = delim;
        delims_buf[1] = kvdelim;
        const SubString delims(delims_buf, 2);
        const SubString EMPTY;

        Size count = 0;
        bool created;
        StrTok tok(str);
        for (; tok.nextany(delims); ++count) {
            Value& val = get(tok.value().convert<Key>(), &created);
            if (tok.delim().null() || *tok.delim() == delim) {
                if (!created)
                    DataOp<Value>::defval(val);
            } else {
                tok.next(delim);
                val = tok.value().convert<Value>();
            }
        }
        return count;
    }

    // MOVE

    /** Move given item from another map.
     - This has the same effect as doing an add() then remove() on src map
     - For best performance use concrete iterator instead of base Map::Iter
     .
     \tparam  T  Map type, must have same key/value types -- inferred from src parameter
     \param  src  Source iterator in other map to move from
     \param  dir  Direction to move src iterator to next item, iterNone for end position
     \return      This
    */
    template<class T>
    MapBaseType& move(IteratorBi<T>& src, IteratorDir dir=iterNone) {
        STATIC_ASSERT(!IsConst<T>::value, ERROR_move_iterator_must_be_mutable_not_const);
        STATIC_ASSERT((IsSame<typename T::Item,Item>::value), ERROR_move_from_incompatible_source_iterator);
        if (src) {
            MapBaseType& srcObj = src.getParent();
            if (&srcObj != this) {
                this->add((Item&)*src);
                srcObj.remove((IterM&)src, dir);
            }
        }
        return *this;
    }

    /** Move given item from another map.
     - This has the same effect as doing an add() then remove() on src map
     - For best performance use concrete iterator instead of base Map::Iter
     .
     \tparam  T  Map type, must have same key/value types -- inferred from src parameter
     \param  src  Source iterator in other map to move from
     \return      This
    */
    template<class T>
    MapBaseType& move(const IteratorBi<T>& src)
        { return move<T>((IteratorBi<T>&)src, iterNone); }

    // REMOVE

    /** Find and remove item with given key.
     - \b Caution: Removing an item may shift other iterators on same map, in an unpredictable way -- see remove(IterM&,IteratorDir)
     .
     \param  key  Key to use
     \return      Whether key was removed, false if not found
    */
    VIRTUAL bool remove(const Key& key) VIRTUAL_PURE

    /** Remove item using given iterator.
     - This will move the iterator to the next item according to dir, or end position if no more
     - \b Caution: Removing an item may shift other iterators on same map, in an unpredictable way
     .
     \param  iter  Iterator position
     \param  dir   Direction to move iterator to next item, iterNone for end position
     \return       Whether item was removed, false if iterator at end position
    */
    VIRTUAL bool remove(IterM& iter, IteratorDir dir=iterNone) VIRTUAL_PURE

    // ADVANCED

    // INTERNAL

    // Iterator support methods (used internally)
    /** \cond impl */
    VIRTUAL void iterInitMutable() VIRTUAL_PURE
    VIRTUAL const IterItem* iterFirst(IterKey& key) const VIRTUAL_PURE
    VIRTUAL const IterItem* iterNext(IterKey& key) const VIRTUAL_PURE
    VIRTUAL const IterItem* iterLast(IterKey& key) const VIRTUAL_PURE
    VIRTUAL const IterItem* iterPrev(IterKey& key) const VIRTUAL_PURE
    /** \endcond */
};

///////////////////////////////////////////////////////////////////////////////

/** \cond impl */
#undef VIRTUAL
#undef VIRTUAL_PURE
/** \endcond */

///////////////////////////////////////////////////////////////////////////////

/** Map implemented as an ordered list. This uses List internally.

\tparam  TKey      %Map key type
\tparam  TValue    %Map value type
\tparam  TCompare  Comparison type to use (function pointer or functor) -- must be initialized via constructor if function pointer!
\tparam  TSize     %Size type to use for size values (must be unsigned integer) -- default: SizeT

\par Comparison

You can use the default comparison object (Compare) or set another one.
The compare function/functor compares a pair of TKey arguments (usually passed by reference) and returns int result (0:equal, <0:first is less, >0:first is greater).

 - Compare, CompareR
 - CompareI, CompareIR

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
 - type()
   - ordered()
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
 - add(const Item&,bool)
   - add(const MapBaseType&)
 - remove(const Key&)
   - remove(MapBaseType::IterM&,IteratorDir)
   - removeat()
 - move(MapBaseType::IterM&,IteratorDir)
   - move(ThisType::IterM&,IteratorDir)
 .

\par Example

This example works with any Map type.

\code
// Create map with numeric keys and values, set some key/values
MapList<int,int> map;
map[10] = 100;
map[20] = 200;

// Find (const), print value
{
    const int* value = map.find(10);
    if (value != NULL)
        printf("Value:%i\n", *value);
}

// Find (mutable), modify value
{
    int* value = map.findM(10);
    if (value != NULL)
        *value *= 10;
}

// Lookup iterator and print key and value
{
    MapList<int,int>::Iter iter = map.iter(20);
    if (iter)
        printf("Key:%i, Value:%i\n", iter->key, iter->value);
}

// Create new map storing items in descending (reverse) order
MapList<int,int,CompareR> map2;
map2[10] = 100;
map2[20] = 200;

// Use iterator to print all keys and values
for (MapList<int,int,CompareR>::Iter iter(map2); iter; ++iter)
    printf("Key:%i, Value:%i\n", iter->key, iter->value);
\endcode
*/
template<class TKey, class TValue, class TCompare=Compare<TKey>, class TSize=SizeT>
class MapList : public Map<TKey,TValue,TSize>
{
public:
    EVO_CONTAINER_TYPE;
    typedef TCompare          Compare;            ///< Compare type (function pointer or functor)
    typedef TSize             Size;                ///< Size integer type
    typedef TKey              Key;                ///< Key type
    typedef TValue            Value;            ///< Value type
    typedef KeyVal<Key,Value> Item;                ///< Item (key/value pair) type

    typedef MapList<Key,Value,Compare,Size> ThisType;    ///< This type
    typedef Map<Key,Value,Size> MapBaseType;            ///< Map base type (used when passing as parameter)

    // Iterator support types
    /** \cond impl */
    typedef typename MapBaseType::IterKey  IterKey;
    typedef typename MapBaseType::IterItem IterItem;
    /** \endcond */

    typedef typename IteratorRa<ThisType>::Const Iter;    ///< Iterator (const) - IteratorRa
    typedef IteratorRa<ThisType>                 IterM;    ///< Iterator (mutable) - IteratorRa

    /** Constructor. */
    MapList()
        { }

    /** Constructor.
     - Uses default comparison object
     - Results are undefined if TCompare is a function pointer since this will not initialize it
     .
     \param  cmp  Comparison object to use
     */
    MapList(const Compare& cmp) : data_(cmp)
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
     \param  src  Source to copy
     \param  cmp  Comparison object to use
    */
    MapList(const MapBaseType& src, const Compare& cmp) : data_(cmp) {
        for (typename MapBaseType::Iter iter(src); iter; ++iter)
            this->get(iter->key) = iter->value;
    }

    /** Copy constructor.
     - This copies the comparison object as well
     .
     \param  src  Source to copy
    */
    MapList(const ThisType& src) : data_(src.data_)
        { MapBaseType::size_ = src.size_; }

    /** Destructor. */
    ~MapList()
        { }

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

    // SET

    ThisType& clear() {
        data_.items.clear();
        MapBaseType::size_ = 0;
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
            this->get(iter->key) = iter->value;
        return *this;
    }

    /** Set as copy of given MapList map.
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

    const Value* find(const Key& key) const {
        Size pos;
        const Item* item = search(pos, key);
        return (item != NULL ? &item->value : NULL);
    }

    Value* findM(const Key& key) {
        Size pos;
        const Item* item = search(pos, key);
        return (item != NULL ? (Value*)&item->value : NULL);
    }

    /** Find (lookup) index for given key (const).
     \param  key  Key to find
     \return      Found index for item key, END if not found
    */
    Size findindex(const Key& key) const
        { Size pos; return (search(pos, key) == NULL ? END : pos); }

    Iter iter(const Key& key) const {
        IterKey iterkey;
        const Item* item = search(iterkey.a, key);
        return (item != NULL ? Iter(*this, iterkey, (IterItem*)item) : Iter(*this, iterEnd));
    }

    IterM iterM(const Key& key) {
        IterKey iterkey;
        const Item* item = search(iterkey.a, key);
        return (item != NULL ? IterM(*this, iterkey, (IterItem*)item) : IterM(*this, iterEnd));
    }

    Item& getitem(const Key& key, bool* created=NULL) {
        Item* item;
        Size pos;
        if ( (item=(Item*)search(pos, key)) == NULL) {
            // TODO: item = &( data_.items.advBuffer()[data_.items.insertnew(pos)] );
            item = &data_.items.itemM(data_.items.insertnew(pos));
            item->key = key;
            if (created != NULL)
                *created = true;
            ++MapBaseType::size_;
        } else if (created != NULL)
            *created = false;
        return *item;
    }

    Value& get(const Key& key, bool* created=NULL)
        { return getitem(key, created).value; }

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

    Value& operator[](const Key& key)
        { return getitem(key, NULL).value; }

    ThisType& unshare()
        { data_.items.unshare(); return *this; }

    /** Set map capacity.
     \param  size  New capacity
     \return       This
    */
    ThisType& capacity(Size size)
        { data_.items.capacity(size); return *this; }

    /** Set map capacity to at least given minimum.
     \param  min  Minimum capacity
     \return      This
    */
    ThisType& capacitymin(Size min)
        { data_.items.capacitymin(min); return *this; }

    /** Reduce capacity to fit current size (modifier).
     - Call to save memory when done expanding size
     \return  This
    */
    ThisType& compact()
        { data_.items.compact(); return *this; }

    ThisType& reserve(Size size)
        { capacitymin(this->size_ + size); return *this; }

    // ADD

    Item& add(const Item& item, bool update=true) {
        bool created_val;
        Item& upditem = getitem(item.key, &created_val);
        if (created_val || update)
            upditem.value = item.value;
        return upditem;
    }

    ThisType& add(const MapBaseType& map) {
        if (this != &map) {
            reserve(map.size());
            for (typename MapBaseType::Iter iter(map); iter; ++iter)
                getitem(iter->key).value = iter->value;
        }
        return *this;
    }

    template<class T>
    Size addsplit(const T& str, char delim=',', char kvdelim='=')
        { return MapBaseType::addsplit(str, delim, kvdelim); }

    // MOVE

    using MapBaseType::move;

    /** Move given item from another map.
     - This has the same effect as doing an add() then remove() on src map
     .
     \param  src  Source iterator in other map to move from
     \param  dir  Direction to move src iterator to next item, iterNone for end position
     \return      This
    */
    ThisType& move(IterM& src, IteratorDir dir=iterNone) {
        if (src) {
            ThisType& srcObj = src.getParent();
            if (&srcObj != this) {
                // Move item directly
                IterKey& iterkey = src.getKey();
                Item* item;
                Size pos;
                if ( (item=(Item*)search(pos, src->key)) == NULL) {
                    // Move item
                    bool nextitem = false;
                    data_.items.move(pos, srcObj.data_.items, iterkey.a, 1);
                    ++MapBaseType::size_;

                    // Adjust iterator
                    if (--srcObj.MapBaseType::size_ > 0 && dir != iterNone) {
                        if (dir == iterRv) {
                            if (iterkey.a > 0)
                                { --iterkey.a; nextitem = true; }
                        } else if (iterkey.a < srcObj.data_.items.size())
                            nextitem = true;
                    }
                    if (nextitem)
                        src.setData( (IterItem*)&srcObj.data_.items.item(iterkey.a) );
                    else
                        src = iterEnd;
                } else {
                    // Key already exists, overwrite item value
                    item->value = src->value;
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
        { return move((IterM&)src, iterNone); }

    /** Swap with another list.
     - This swaps internal state directly so is faster than moving items
     .
     \param  list  %List to swap with
    */
    void swap(ThisType& list) {
        data_.items.swap(list.data_.items);
        StaticIf<sizeof(Data) == sizeof(Items),CompareSwapNone,CompareSwapMem>::Type::swap((Compare&)data_, (Compare&)list.data_);
    }

    // TODO: swap(ListType&) -- swap and sort items

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

    bool remove(typename MapBaseType::IterM& iter, IteratorDir dir=iterNone)
        { return remove((IterM&)iter, dir); }

    bool remove(IterM& iter, IteratorDir dir=iterNone) {
        if (iter && this == &iter.getParent()) {
            IterKey& iterkey = iter.getKey();
            data_.items.remove(iterkey.a);
            bool nextitem = false;
            if (--MapBaseType::size_ > 0 && dir != iterNone) {
                if (dir == iterRv) {
                    if (iterkey.a > 0)
                        { --iterkey.a; nextitem = true; }
                } else if (iterkey.a < data_.items.size())
                    nextitem = true;
            }
            if (nextitem)
                iter.setData( (IterItem*)&data_.items.item(iterkey.a) );
            else 
                iter = iterEnd;
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

    /** Advanced: Get map item for key without initializing (constructing) it.
     - This gets existing item, or gets new uninitialized item if key not found
     - \b Caution: Data must be unique (not shared) or results are undefined
     - \b Caution: The new item must be immediately initialized, and must contain the same given key
     - \b Caution: Results are undefined if new items are not initialized correctly
     .
     \param  item  Stores pointer to item, existing or created [out]
     \param  key   Key to use
     \return       Whether new uninitialized item created, false if existing item found
    */
    bool advGetitem(Item*& item, const Key& key) {
        Size pos;
        bool created;
        if ( (item=(Item*)search(pos, key)) == NULL) {
            // TODO: item = &( data_.items.advBuffer()[data_.items.advInsert(pos, 1)] );
            item = &data_.items.itemM(data_.items.advInsert(pos, 1));
            created = true;
            ++MapBaseType::size_;
        } else
            created = false;
        return created;
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
            cmp = data_.Compare::operator()(key, item.key);
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

/** MapList using String keys and values.
 - This is an alias to MapList
 */
typedef MapList<String,String> StrMapList;

///////////////////////////////////////////////////////////////////////////////

/** Lookup (find) map value as SubString for given key.
 - Useful helper for looking up any string value and converting via SubString\n
   Example:\code
StrMapList map;
map["key"] = "123";
Int val = lookupsub(map, "key").num(); // val is null if key not found
\endcode
 .
 \tparam  TMap  Evo Map container
 \param  map  Map to use
 \param  key  Key to find
 \return      Found value as SubString, set to null if key not found
 \see  stdlookupsub()
*/
template<class TMap>
inline SubString lookupsub(const TMap& map, const typename TMap::Key& key) {
    const typename TMap::Value* val = map.find(key);
    return (val != NULL ? SubString(*val) : SubString());
}

// STD/STL container helpers

/** Lookup (find) STL map value as SubString for given key.
 - Useful helper for looking up any string value and converting via SubString\n
   Example:\code
std::map<std::string,std::string> map;
map["key"] = "123";
Int val = stdlookupsub(map, "key").num(); // val is null if key not found
\endcode
 - Useful for doing STL lookups without messing with STL iterators
 .
 \tparam  TMap  STL map container
 \param  map  %Map to use
 \param  key  Key to find
 \return      Found value as SubString, set to null if key not found
 \see  lookupsub(), stdlookup(), stdlookupM()
*/
template<class TMap>
inline SubString stdlookupsub(const TMap& map, const typename TMap::key_type& key) {
    typename TMap::const_iterator i = map.find(key);
    return (i != map.end() ? SubString(i->second) : SubString());
}

/** Lookup (find) STL map value for given key (const).
 - Useful for doing STL lookups without messing with STL iterators
 .
 \tparam  TMap  STL map container
 \param  map  %Map to use
 \param  key  Key to find
 \return      Pointer to found value, NULL if not found
 \see  stdlookupM(), stdlookupsub()
*/
template<class TMap>
inline const typename TMap::mapped_type* stdlookup(const TMap& map, const typename TMap::key_type& key) {
    typename TMap::const_iterator i = map.find(key);
    return (i != map.end() ? &i->second : NULL);
}

/** Lookup (find) STL map value for given key (mutable).
 - Useful for doing STL lookups without messing with STL iterators
 .
 \tparam  TMap  STL map container
 \param  map  %Map to use
 \param  key  Key to find
 \return      Pointer to found value, NULL if not found
 \see  stdlookup(), stdlookupsub()
*/
template<class TMap>
inline typename TMap::mapped_type* stdlookupM(TMap& map, const typename TMap::key_type& key) {
    typename TMap::iterator i = map.find(key);
    return (i != map.end() ? &i->second : NULL);
}

///////////////////////////////////////////////////////////////////////////////
//@}
} // Namespace: evo
#endif
