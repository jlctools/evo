// Evo C++ Library
/* Copyright 2018 Justin Crowell
Distributed under the BSD 2-Clause License -- see included file LICENSE.txt for details.
*/
///////////////////////////////////////////////////////////////////////////////
/** \file map.h Evo Map interface. */
#pragma once
#ifndef INCL_evo_map_h
#define INCL_evo_map_h

#include "type.h"
#include "pair.h"
#include "strtok.h"

namespace evo {
/** \addtogroup EvoContainers */
//@{

///////////////////////////////////////////////////////////////////////////////

/** Associative container holding key/value pairs for fast lookup.

\tparam  TKey    %Map key type
\tparam  TValue  %Map value type
\tparam  TSize   Size type to use for size values (must be unsigned integer) -- default: SizeT

\par Features

 - Interface similar to STL map
 - Keys are always unique (no duplicates)
 - Implementations:
   - MapList (ordered)
   - MapHash (unordered)
   .
 - For best performance use concrete map types instead of the general interface (Map)
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
   - ordered()
 - contains(const Key&) const
   - contains(const Key&, const Value&) const
 - find()
 - iter()
   - cbegin(), cend()
 - operator==()
   - operator!=()
 .

\par Modifiers

 - reserve()
   - capacity(Size)
   - capacitymin()
   - unshare()
 - findM()
   - getitem()
   - get()
   - operator[]()
 - iterM()
   - begin(), end()
 - set()
   - set(const MapBaseType&)
   - setempty()
   - clear()
 - add(const Key&,const Value&,bool)
   - add(const Item&,bool)
   - add(const MapBaseType&,bool)
 - remove(const Key&)
   - remove(IterM&,IteratorDir)
 .

\par Example

See implementation types for examples: MapList, MapHash
*/
template<class TKey, class TValue, class TSize=SizeT>
class Map {
public:
    EVO_CONTAINER_TYPE;
    typedef Map<TKey,TValue,TSize> ThisType;                ///< This type
    typedef Map<TKey,TValue,TSize> MapBaseType;             ///< %Map base type
    typedef TSize           Size;                           ///< Size type for size values (must be unsigned integer) -- default: SizeT
    typedef TKey            Key;                            ///< Key type
    typedef TValue          Value;                          ///< Value type
    typedef Pair<Key,Value> Item;                           ///< Item type (key/value pair)

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
    typedef Pair<const Key,Value> IterItem;
    /** \endcond */

    typedef typename IteratorBi<ThisType>::Const Iter;      ///< Iterator (const) - IteratorBi
    typedef IteratorBi<ThisType>                 IterM;     ///< Iterator (mutable) - IteratorBi

protected:
    Size       size_;           ///< %Map size (number of items, automatically updated by concrete set members)
    bool       ordered_;        ///< Whether map is ordered (items are kept in order by key)

    /** Constructor. Used by derived types. */
    Map() : size_(0), ordered_(true)
        { }

    /** Constructor. Used by derived types.
     \param  ordered  Whether map is ordered
    */
    Map(bool ordered) : size_(0), ordered_(ordered)
        { }

    /** Copy constructor (disabled). */
    Map(const Map&) EVO_ONCPP11(= delete);

    /** Used by base class to get data to initialize iterator.
     \param  iterkey  %Set to iterator data, if item found
     \param  key      Key to find
     \return          Item pointer, NULL if not found
    */
    virtual const Item* getiter(IterKey& iterkey, const Key& key) const = 0;

public:
    /** Destructor. */
    virtual ~Map()
        { }

    // SET

    /** Assignment operator.
     \param  src  Source to copy
     \return      This
    */
    MapBaseType& operator=(const MapBaseType& src) {
        set(src);
        return *this;
    }

    /** %Set as null and empty.
     \return  This
    */
    virtual MapBaseType& set() = 0;

    /** %Set as copy of given map.
     - This clears all items in current map.
     .
     \param  src  Source map
     \return      This
    */
    virtual MapBaseType& set(const MapBaseType& src) = 0;

    /** %Set as empty but not null.
     \return  This
    */
    virtual MapBaseType& setempty() = 0;

    /** Clear by removing all items.
     - Does not set as null -- null status is unchanged
     .
     \return  This
    */
    virtual MapBaseType& clear() = 0;

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
    virtual bool null() const = 0;

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
    virtual bool shared() const = 0;

    /** Get map capacity.
     \return  Current capacity
    */
    virtual Size capacity() const = 0;

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
                    val = map.find(iter->first);
                    if (val == NULL || !(*val == iter->second)) {
                        result = false;
                        break;
                    }
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

    /** Get whether map contains the given key.
     \param  key  Key to look for
     \return      Whether key was found
    */
    virtual bool contains(const Key& key) const = 0;

    /** Get whether map contains the given key with given value.
     \param  key    Key to look for
     \param  value  Value to check for
     \return        Whether key was found and matches value
    */
    virtual bool contains(const Key& key, const Value& value) const = 0;

    /** Find (lookup) value for given key (const).
     \param  key  Key to find
     \return      Found value const pointer, NULL if not found
    */
    virtual const Value* find(const Key& key) const = 0;

    /** Find (lookup) value for given key (mutable).
     \param  key  Key to find
     \return      Found value pointer, NULL if not found
    */
    virtual Value* findM(const Key& key) = 0;

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
     .
     \return  Iterator at end position
     \see Iter, cbegin(), begin(), end()
    */
    Iter cend() const
        { return Iter(); }

    /** Get iterator at first item (mutable).
     - This allows compatibility with range-based for loops and other libraries, otherwise use container Iter directly
     - cbegin() is more efficient, since this effectively calls unshare() to make items mutable
     - \b Caution: Results are undefined if value is modified in a way that changes it's ordered position in set
     .
     \return  Iterator at first item, or at end position if empty
     \see IterM, end(), cbegin(), cend()
    */
    IterM begin()
        { return IterM(*this); }

    /** Get iterator at end (const).
     - This allows compatibility with range-based for loops and other libraries, otherwise use container Iter directly
     .
     \return  Iterator at end position
     \see IterM, begin(), cbegin(), cend()
    */
    IterM end()
        { return IterM(); }

    /** Find (lookup) iterator for given key (const).
     \param  key  Key to find
     \return      Iterator, at end position if not key not found
    */
    Iter iter(const Key& key) const {
        IterKey iterkey;
        const Item* item = this->getiter(iterkey, key);
        return (item != NULL ? Iter(*this, iterkey, (IterItem*)item) : Iter(*this, iterEND));
    }

    /** Find (lookup) iterator for given key (mutable).
     \param  key  Key to find
     \return      Iterator, at end position if not key not found
    */
    IterM iterM(const Key& key) {
        IterKey iterkey;
        Item* item = (Item*)this->getiter(iterkey, key);
        return (item != NULL ? IterM(*this, iterkey, (IterItem*)item) : IterM(*this, iterEND));
    }

    /** Get map item for key (mutable).
     - Item is created with default value if not found.
     .
     \param  key      Key to use
     \param  created  Stores whether new item was created, if not NULL
     \return          Value reference for key
    */
    virtual Item& getitem(const Key& key, bool* created=NULL) = 0;

    /** Get item value for key (mutable).
     - Item is created with default value if not found
     .
     \param  key      Key to use
     \param  created  Stores whether new item was created, if not NULL
     \return          Value reference for key
    */
    Value& get(const Key& key, bool* created=NULL)
        { return this->getitem(key, created).second; }

    // INFO_SET

    /** Get item value for key (mutable).
     - Item is created with default value if not found
     - Same as get(const Key&)
     .
     \param  key  Key to use
     \return      Value reference for key
    */
    Value& operator[](const Key& key)
        { return this->getitem(key, NULL).second; }

    /** Make data unique by allocating new buffer, if needed (modifier).
     - Use reserve() instead to reserve additional space while unsharing
     - Use to make buffer unique (not shared) and writable (when not empty)
     - This is called automatically by mutable/modifier methods
     - This does nothing if empty or not shared
     .
     \return  This
    */
    virtual MapBaseType& unshare() = 0;

    /** %Set map capacity.
     - This is just a suggestion -- some implementations may ignore it
     .
     \param  size  New capacity
     \return       This
    */
    virtual MapBaseType& capacity(Size size) = 0;

    /** %Set map capacity to at least given minimum.
     - This is just a suggestion -- some implementations may ignore it
     .
     \param  min  Minimum capacity
     \return      This
    */
    virtual MapBaseType& capacitymin(Size min) = 0;

    /** Reduce capacity to fit current size (modifier).
     - Call to save memory when done adding items
     - This is just a suggestion -- some implementations may ignore it
     .
     \return  This
    */
    virtual MapBaseType& compact()
        { return *this; }

    /** Reserve space for new items.
     - For best performance, call this before adding multiple items
     - This calls capacitymin() with current size + size to reserve space
     - This is just a suggestion -- some implementations may ignore it
     .
     \param  size  Size to reserve
     \return       This
    */
    MapBaseType& reserve(Size size)
        { this->capacitymin(size_ + size); return *this; }

    // ADD

    /** Add or update using given key and value.
     \param  key     Key to find/add (copied)
     \param  value   Value to add/update (copied)
     \param  update  Whether to update value for existing item
     \return         Item added/updated
    */
    virtual Item& add(const Key& key, const Value& value, bool update=true) = 0;

    /** Add or update using given item.
     \param  item    Item to use for add/update (copied)
     \param  update  Whether to update value for existing item
     \return         Item added/updated
    */
    virtual Item& add(const Item& item, bool update=true) = 0;

    /** Add items from given map.
     \param  map     %Map to add items from
     \param  update  Whether to update value for existing items
     \return         This
    */
    virtual MapBaseType& add(const MapBaseType& map, bool update=true) = 0;

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
                    DataCopy<Value>::set_default(val);
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
     \param  dir  Direction to move src iterator to next item, iterNONE for end position
     \return      This
    */
    template<class T>
    MapBaseType& move(IteratorBi<T>& src, IteratorDir dir=iterNONE) {
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
        { return move<T>((IteratorBi<T>&)src, iterNONE); }

    // REMOVE

    /** Find and remove item with given key.
     - \b Caution: Removing an item may shift other iterators on same map, in an unpredictable way -- see remove(IterM&,IteratorDir)
     .
     \param  key  Key to use
     \return      Whether key was removed, false if not found
    */
    virtual bool remove(const Key& key) = 0;

    /** Remove item using given iterator.
     - This will move the iterator to the next item according to dir, or end position if no more
     - \b Caution: Removing an item may shift other iterators on same map, in an unpredictable way
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
}
#endif
