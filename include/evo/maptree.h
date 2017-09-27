// Evo C++ Library
/* Copyright (c) 2016 Justin Crowell
 This Source Code Form is subject to the terms of the Mozilla Public
 License, v. 2.0. If a copy of the MPL was not distributed with this
 file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
///////////////////////////////////////////////////////////////////////////////
/** \file maphash.h Evo MapHash container. */
#pragma once
#ifndef INCL_evo_maptree_h
#define INCL_evo_maptree_h

// Includes
#include "map.h"

// Namespace: evo
namespace evo {

/** \addtogroup EvoContainers */
//@{

///////////////////////////////////////////////////////////////////////////////

// TODO work in progress
#if 0

/** Map implemented as a binary search tree.

\tparam  TKey    %Map key type
\tparam  TValue  %Map value type
\tparam  TSize   %Size type to use for size values (must be unsigned integer) -- default: SizeT

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

 - resize()
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
   - set(const ThisType&)
   - setempty()
   - clear()
   - operator=(const MapBaseType&)
   - operator=(const ThisType&)
 - add(const Item&,bool)
   - add(const MapBaseType&)
 - remove(const Key&)
   - remove(typename MapBaseType::IterM&,IteratorDir)
 - move(MapBaseType::IterM&,IteratorDir)
   - move(ThisType::IterM&,IteratorDir)
 .

\par Example

This example works with any Map type.

\code
// Create map with numeric keys and values, set some key/values
MapHash<int,int> map;
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
MapHash<int,int>::Iter iter = map.iter(20);
if (iter)
    printf("Key:%i, Value:%i\n", iter->key, iter->value);
\endcode
*/
template<class TKey, class TValue, class TCompare=Compare<TKey>, class TSize=SizeT>
class MapTree : public Map<TKey, TValue, TSize>
{
protected:
    using Map<TKey,TValue,TSize>::size_;

public:
    EVO_CONTAINER_TYPE;
    typedef TCompare          Compare;                ///< Compare type for internal MapList (function pointer or functor)
    typedef TSize             Size;                    ///< Size integer type
    typedef TKey              Key;                    ///< Key type
    typedef TValue            Value;                ///< Value type
    typedef KeyVal<Key,Value> Item;                    ///< Item (key/value pair) type

    typedef MapTree<Key,Value,Compare,Size> ThisType;        ///< This type
    typedef Map<Key,Value,Size>             MapBaseType;    ///< Map base type (used when passing as parameter)

    // Iterator support types
    /** \cond impl */
    typedef typename MapBaseType::IterKey  IterKey;
    typedef typename MapBaseType::IterItem IterItem;
    /** \endcond */

    typedef typename IteratorBi<ThisType>::Const Iter;    ///< Iterator (const) - IteratorBi
    typedef IteratorBi<ThisType>                 IterM;    ///< Iterator (mutable) - IteratorBi

    /** Constructor. */
    MapTree()
        { }

    /** Constructor.
     \param  cmp  Comparison object to use
    */
    MapTree(const Compare& cmp) : data_(cmp)
        { }

    /** Copy constructor.
     \param  src  Source to copy
    */
    MapTree(const MapBaseType& src) {
        for (typename MapBaseType::Iter iter(src); iter; ++iter)
            getitem(iter->key).value = iter->value;
    }

    /** Copy constructor.
     \param  src  Source to copy
    */
    MapTree(const ThisType& src)
        { set(src); }

    ThisType& operator=(const MapBaseType& src)
        { set(src); return *this; }

    ThisType& operator=(const ThisType& src)
        { set(src); return *this; }

    // SET

    ThisType& clear()
        {size_ = 0; return *this; }

    ThisType& set()
        { size_ = 0; return *this; }

    ThisType& set(const MapBaseType& src) {
        clear();
        for (typename MapBaseType::Iter iter(src); iter; ++iter)
            getitem(iter->key).value = iter->value;
        return *this;
    }

    ThisType& set(const ThisType& src) {
        //buckets_.set(src.buckets_);
        data_ = src.data_;
        size_ = src.size_;
        return *this;
    }

    ThisType& setempty()
        { size_ = 0; return *this; }

    // INFO

    bool null() const
        { return (data_.root == NULL); }

    bool shared() const
        { return false; }

    Size capacity() const
        { return size_; }

    // COMPARE

    using MapBaseType::operator==;
    using MapBaseType::operator!=;

    // FIND

    const Value* find(const Key& key) const {
        if (size_ > 0) {
            // TODO
        }
        return NULL;
    }

    Value* findM(const Key& key) {
        if (size_ > 0) {
            // TODO
        }
        return NULL;
    }

    Iter iter(const Key& key) const {
        if (size_ > 0) {
            // TODO
        }
        return Iter(*this, iterEnd);
    }

    IterM iterM(const Key& key) {
        if (size_ > 0) {
            // TODO
        }
        return IterM(*this, iterEnd);
    }

    Item& getitem(const Key& key, bool* created=NULL) {
        // TEMP
        printf("\n");
        Node* cur;
        if (data_.root == NULL || data_.root == Node::get_null())
            cur = data_.root = new Node(1, key);
        else {
            Node* result;
            Node* stack[64];
            int top = 0, dir, cmp;

            cur = data_.root;
            for (;;) {
                stack[top++] = cur;
                cmp = data_(key, cur->data.key);
                // TEMP
                printf("XXX cmp:%i key:%lu get:%lu\n", cmp, cur->data.key, key);

                if (cmp < 0)
                    dir = 0;
                else if (cmp > 0)
                    dir = 1;
                else {
                    // TEMP
                    printf("XXX found %lu\n", key);
                    if (created != NULL)
                        *created = false;
                    return cur->data;
                }
                if (cur->link[dir] == Node::get_null())
                    break;
                cur = cur->link[dir];
            }

            result = new Node(1, key);
            cur->link[dir] = result;

            //cur = skew(cur);
            //cur = split(cur);
        }

        // TEMP
        printf("XXX new:%lu (%lu)\n", cur->data.key, key);
        dump(data_.root);

        ++size_;
        if (created != NULL)
            *created = true;
        return cur->data;
    }

    Value& get(const Key& key, bool* created=NULL)
        { return getitem(key, created).value; }

    // INFO_SET

    Value& operator[](const Key& key)
        { return getitem(key, NULL).value; }

    ThisType& unshare()
        { return *this; }

    /** Set hash map size (capacity).
     - Use to suggest a new capacity before adding a batch of items
     - This will set hash size to nearest power of 2 that can fit both current items and given size
     - \b Caution: This will invalidate iterators on this map
     .
     \param  size  New suggested capacity
     \return       This
    */
    //[tags: self, capacity, size() ]
    ThisType& capacity(Size size) {
        // TODO
        return *this;
    }

    /** Set map capacity to at least given minimum.
     \param  min  Minimum capacity
     \return      This
    */
    ThisType& capacitymin(Size min) {
        // TODO
        //if (min > buckets_.size())
        //    capacity(min);
        return *this;
    }

    ThisType& reserve(Size size)
        { return capacitymin(size_ + size); }

    // ADD

    Item& add(const Item& item, bool update=true) {
        bool created_val;
        Item& upditem = getitem(item.key, &created_val);
        if (created_val || update)
            upditem.value = item.value;
        return upditem;
    }

    ThisType& add(const MapBaseType& map) {
        assert( this != &map );
        reserve(map.size());
        for (typename MapBaseType::Iter iter(map); iter; ++iter)
            getitem(iter->key).value = iter->value;
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
            ThisType& srcparent = src.getParent();
            if (&srcparent != this) {
                add((Item&)*src);
                srcparent.remove(src, dir);
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

    // REMOVE

    bool remove(const Key& key) {
        bool result;
        // TODO
        result = false;
        return result;
    }

    bool remove(typename MapBaseType::IterM& iter, IteratorDir dir=iterNone)
        { return remove((IterM&)iter, dir); }

    bool remove(IterM& iter, IteratorDir dir=iterNone) {
        // TODO
        return false;
    }

    // INTERNAL

    // Iterator support methods
    /** \cond impl */
    void iterInitMutable()
        { }
    const IterItem* iterFirst(IterKey& key) const {
        // TODO
        return NULL;
    }
    const IterItem* iterNext(IterKey& key) const {
        // TODO
        return NULL;
    }
    const IterItem* iterLast(IterKey& key) const {
        // TODO
        return NULL;

    }
    const IterItem* iterPrev(IterKey& key) const {
        // TODO
        return NULL;
    }
    /** \endcond */

protected:
    const Item* getiter(IterKey& iterkey, const Key& key) const {
        // TODO
        return NULL;
    }

private:
    struct Node {
        Node* link[2];
        uint  level;
        Item  data;

        Node(uint level, const Key& key) : level(level), data(key)
            { link[0] = link[1] = Node::get_null(); }
        Node(uint level, const Key& key, const Value& value) : level(level), data(key, value)
            { link[0] = link[1] = Node::get_null(); }

        static Node* get_null()
            { static Node node; return &node; }

    private:
        Node() : level(0)
            { link[0] = link[1] = this; }
    };

    // Use inheritance to reduce size bloat with empty Compare
    struct Data : public Compare {
        Node* root;

        Data()
            { root = NULL; }
        Data(const Compare& cmp) : Compare(cmp)
            { root = NULL; }
        Data(const Data& data) : Compare(data)
            { root = NULL; }
        Data& operator=(const Data& data) {
            Compare::operator=((const Compare&)data);
            return *this;
        }
    };
    Data data_;

    static Node* skew(Node* root) {
        if (root->link[0]->level == root->level && root->level != 0) {
            Node* temp = root->link[0];
            root->link[0] = temp->link[1];
            temp->link[1] = root;
            root = temp;
        }
        return root;
    }

    static Node* split(Node* root) {
        if (root->link[1]->link[1]->level == root->level && root->level != 0) {
            Node* temp = root->link[1];
            root->link[1] = temp->link[0];
            temp->link[0] = root;
            root = temp;
            ++root->level;
        }
        return root;
    }

    // TEMP
    static void dump(Node* root) {
        if (root != Node::get_null()) {
            printf(" %p %u: %lu - %lu %lu\n", root, root->level, root->data.key, root->link[0]->data.key, root->link[1]->data.key);
            dump(root->link[0]);
            dump(root->link[1]);
        }
    }
};

///////////////////////////////////////////////////////////////////////////////

// TODO: use #ifdef to detect string.h?
/** MapTree using String keys and values.
 - This is an alias to MapTree
 */
typedef MapTree<String,String> StrTree;

#endif

///////////////////////////////////////////////////////////////////////////////
//@}
} // Namespace: evo
#endif
