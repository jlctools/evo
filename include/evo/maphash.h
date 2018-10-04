// Evo C++ Library
/* Copyright 2018 Justin Crowell
Distributed under the BSD 2-Clause License -- see included file LICENSE.txt for details.
*/
///////////////////////////////////////////////////////////////////////////////
/** \file maphash.h Evo MapHash container. */
#pragma once
#ifndef INCL_evo_maphash_h
#define INCL_evo_maphash_h

#include "map.h"
#include "array.h"
#include "ptrlist.h"
#include "impl/hash.h"

// Disable certain MSVC warnings for this file
#if defined(_MSC_VER)
    #pragma warning(push)
    #pragma warning(disable:4457)
#endif

namespace evo {
/** \addtogroup EvoContainers */
//@{

///////////////////////////////////////////////////////////////////////////////

// Internal implementation macros -- only used in this file
/** \cond impl */
#define EVO_IMPL_MAPHASH_SIZEMASK(SIZE)  (SIZE - 1)
#define EVO_IMPL_MAPHASH_THRESHOLD(SIZE) ((SIZE / 10) * 7)
/** \endcond */

///////////////////////////////////////////////////////////////////////////////

/** Map implemented as a hash table.

\tparam  TKey    %Map key type
\tparam  TValue  %Map value type
\tparam  THash   %Hash type to use -- default: CompareHash
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
   - ordered()
 - contains(const Key&) const
   - contains(const Key&, const Value&) const
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
 - add(const Key&,const Value&,bool)
   - add(const Item&,bool)
   - add(const MapBaseType&,bool)
 - remove(const Key&)
   - remove(typename MapBaseType::IterM&,IteratorDir)
 .

\par Example

This example works with any Map type.

Example using a map of numbers

\code
#include <evo/string.h>
#include <evo/maphash.h>
#include <evo/io.h>
using namespace evo;
static Console& c = con();

int main() {
    typedef MapHash<int,int> MyMap;

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
2=20,3=30,1=10,4=40
2=20
3=30
1=100
4=40
\endcode

<i>Note: Item ordering will vary</i>
*/
template<class TKey, class TValue, class THash=CompareHash<TKey>, class TSize=SizeT>
class MapHash : public Map<TKey,TValue,TSize> {
protected:
    using Map<TKey,TValue,TSize>::size_;

public:
    EVO_CONTAINER_TYPE;
#if defined(_MSC_VER) || defined(EVO_OLDCC) // avoid errors with older compilers and MSVC
    typedef Map<TKey,TValue,TSize> MapBaseType;                 ///< %Map base type
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
    typedef MapHash<TKey,TValue,THash,TSize> ThisType;          ///< This type
    typedef THash Hash;                                         ///< Hashing type -- default: CompareHash

    typedef typename IteratorBi<ThisType>::Const Iter;          ///< Iterator (const) - IteratorBi
    typedef IteratorBi<ThisType>                 IterM;         ///< Iterator (mutable) - IteratorBi

    /** Constructor. */
    MapHash() : Map<TKey,TValue,TSize>(false)
        { }

    /** Copy constructor.
     \param  src  Source to copy
    */
    MapHash(const MapBaseType& src) : Map<TKey,TValue,TSize>(false) {
        for (typename MapBaseType::Iter iter(src); iter; ++iter)
            getitem(iter->first).second = iter->second;
    }

    /** Copy constructor.
     \param  src  Source to copy
    */
    MapHash(const ThisType& src) : Map<TKey,TValue,TSize>(false)
        { set(src); }

    // SET

    /** \copydoc Map::operator=() */
    ThisType& operator=(const MapBaseType& src)
        { set(src); return *this; }

    /** Assignment operator.
     - This copies the hash and comparison objects as well
     .
     \param  src  Source to copy
     \return      This
    */
    ThisType& operator=(const ThisType& src)
        { set(src); return *this; }

    ThisType& set()
        { buckets_.set(); size_ = 0; return *this; }

    ThisType& set(const MapBaseType& src) {
        clear();
        for (typename MapBaseType::Iter iter(src); iter; ++iter)
            getitem(iter->first).second = iter->second;
        return *this;
    }

    ThisType& set(const ThisType& src) {
        buckets_.set(src.buckets_);
        data_ = src.data_;
        size_ = src.size_;
        return *this;
    }

    ThisType& setempty()
        { buckets_.setempty(); size_ = 0; return *this; }

    ThisType& clear()
        { buckets_.clear(); size_ = 0; return *this; }

    // INFO

    bool null() const
        { return buckets_.null(); }

    bool shared() const
        { return buckets_.shared(); }

    Size capacity() const
        { return buckets_.size(); }

    // COMPARE

    using MapBaseType::operator==;
    using MapBaseType::operator!=;

    // FIND

    bool contains(const Key& key) const
        { return (find(key) != NULL); }

    bool contains(const Key& key, const Value& value) const {
        const Value* item_value = find(key);
        return (item_value != NULL && *item_value == value);
    }

    const Value* find(const Key& key) const {
        if (size_ > 0) {
            const Bucket* bucket = buckets_.item( data_.hash(key) & data_.sizemask );
            if (bucket != NULL) {
                if (key == bucket->first.first) {
                    return &bucket->first.second;
                } else {
                    Size index;
                    const Item* item = bucket->search(index, key, data_);
                    if (item != NULL)
                        return &item->second;
                }
            }
        }
        return NULL;
    }

    Value* findM(const Key& key) {
        if (size_ > 0) {
            Bucket* bucket = buckets_.itemM( data_.hash(key) & data_.sizemask );
            if (bucket != NULL) {
                if (key == bucket->first.first) {
                    return &bucket->first.second;
                } else {
                    Size index;
                    Item* item = (Item*)bucket->search(index, key, data_);
                    if (item != NULL)
                        return &item->second;
                }
            }
        }
        return NULL;
    }

    /** \copydoc Map::iter() */
    Iter iter(const Key& key) const {
        if (size_ > 0) {
            IterKey iterkey( data_.hash(key) & data_.sizemask );
            const Bucket* bucket = buckets_.item(iterkey.a);
            if (bucket != NULL) {
                if (bucket->first.first == key) {
                    return Iter(*this, iterkey, (IterItem*)&bucket->first);
                } else {
                    const IterItem* item = (IterItem*)bucket->search(iterkey.b, key, data_);
                    if (item != NULL) {
                        ++iterkey.b;
                        return Iter(*this, iterkey, item);
                    }
                }
            }
        }
        return Iter(*this, iterEND);
    }

    /** \copydoc Map::iterM() */
    IterM iterM(const Key& key) {
        if (size_ > 0) {
            IterKey iterkey( data_.hash(key) & data_.sizemask );
            Bucket* bucket = buckets_.itemM(iterkey.a);
            if (bucket != NULL) {
                if (bucket->first.first == key) {
                    return IterM(*this, iterkey, (IterItem*)&bucket->first);
                } else {
                    IterItem* item = (IterItem*)bucket->search(iterkey.b, key, data_);
                    if (item != NULL) {
                        ++iterkey.b;
                        return IterM(*this, iterkey, item);
                    }
                }
            }
        }
        return IterM(*this, iterEND);
    }

    Item& getitem(const Key& key, bool* created=NULL) {
        if (data_.threshold == 0) {
            // Empty, resize buckets
            buckets_.resize(SIZE_INIT);
            data_.sizemask  = EVO_IMPL_MAPHASH_SIZEMASK(SIZE_INIT);
            data_.threshold = EVO_IMPL_MAPHASH_THRESHOLD(SIZE_INIT);
        } else if (size_ >= data_.threshold) {
            // Reached size threshold, time to grow
            Size newsize = buckets_.size() << 1;
            data_.sizemask  = EVO_IMPL_MAPHASH_SIZEMASK(newsize);
            data_.threshold = EVO_IMPL_MAPHASH_THRESHOLD(newsize);

            // Grow and rehash
            Buckets oldbuckets;
            buckets_.swap(oldbuckets);
            buckets_.resize(newsize);

            bool created;
            const Bucket* bucket;
            const Bucket** cur = (const Bucket**)oldbuckets.data();
            for (const Bucket** end=cur+oldbuckets.size(); cur < end; ++cur) {
                if ((bucket=*cur) != NULL) {
                    for (Size i=0, iend=bucket->others.size(); i <= iend; ++i) {
                        const Item* item = (i == 0 ? &bucket->first : &bucket->others[i-1]);
                        Bucket* newbucket = buckets_.getitem(data_.hash(item->first) & data_.sizemask, &created);
                        if (created)
                            newbucket->first = *item;
                        else {
                            Size index;
                            Item* newitem = (Item*)newbucket->search(index, item->first, data_);
                            if (newitem == NULL)
                                newbucket->others.insert(index, *item);
                            else
                                newitem->second = item->second;
                        }
                    }
                }
            }
        }

        // Get item, create if needed
        bool created_item;
        Item* item;
        Bucket* bucket = buckets_.getitem((data_.hash(key) & data_.sizemask), &created_item);
        if (created_item) {
            ++size_;
            item = &bucket->first;
            item->first = key;
        } else if (bucket->first.first == key) {
            item = &bucket->first;
        } else {
            Size index;
            item = (Item*)bucket->search(index, key, data_);
            if (item == NULL) {
                created_item = true;
                ++size_;
                item = (Item*)&(bucket->others[bucket->others.insertnew(index, 1)]);
                item->first = key;
            }
        }
        if (created != NULL)
            *created = created_item;
        return *item;
    }

    /** \copydoc Map::get() */
    Value& get(const Key& key, bool* created=NULL)
        { return getitem(key, created).second; }

    // INFO_SET

    /** \copydoc Map::operator[]() */
    Value& operator[](const Key& key)
        { return getitem(key, NULL).second; }

    ThisType& unshare()
        { buckets_.unshare(); return *this; }

    /** %Set hash map size (capacity).
     - Use to suggest a new capacity before adding a batch of items
     - This will set hash size to nearest power of 2 that can fit both current items and given size
     - \b Caution: This will invalidate iterators on this map
     .
     \param  size  New suggested capacity
     \return       This
    */
    ThisType& capacity(Size size) {
        const Size cursize = buckets_.size();
        if (size != cursize) {
            // Find new size (must be power of 2)
            Size newsize = SIZE_INIT;
            while (newsize < size)  newsize <<= 1;
            while (newsize < size_) newsize <<= 1;

            if (newsize != cursize) {
                // Reached size threshold, time to grow
                data_.sizemask  = EVO_IMPL_MAPHASH_SIZEMASK(newsize);
                data_.threshold = EVO_IMPL_MAPHASH_THRESHOLD(newsize);
                if (size_ == 0) {
                    // Empty, just resize buckets
                    buckets_.resize(newsize);
                } else {
                    // Resize and rehash
                    Buckets oldbuckets;
                    buckets_.swap(oldbuckets);
                    buckets_.resize(newsize);

                    bool created;
                    const Bucket* bucket;
                    const Bucket** cur = (const Bucket**)oldbuckets.data();
                    for (const Bucket** end=cur+oldbuckets.size(); cur < end; ++cur) {
                        if ((bucket=*cur) != NULL) {
                            for (Size i=0, iend=bucket->others.size(); i <= iend; ++i) {
                                const Item* item = (i == 0 ? &bucket->first : &bucket->others[i-1]);
                                Bucket* newbucket = buckets_.getitem(data_.hash(item->first) & data_.sizemask, &created);
                                if (created)
                                    newbucket->first = *item;
                                else {
                                    Size index;
                                    Item* newitem = (Item*)newbucket->search(index, item->first, data_);
                                    if (newitem == NULL)
                                        newbucket->others.insert(index, *item);
                                    else
                                        newitem->second = item->second;
                                }
                            }
                        }
                    }
                }
            }
        }
        return *this;
    }

    /** %Set map capacity to at least given minimum.
     \param  min  Minimum capacity
     \return      This
    */
    ThisType& capacitymin(Size min) {
        if (min > buckets_.size())
            capacity(min);
        return *this;
    }

    /** \copydoc Map::reserve() */
    ThisType& reserve(Size size)
        { return capacitymin(size_ + size); }

    // RESIZE

    /** %Set hash map size (capacity) directly.
     - This will force capacity to a given size (rounded up to next power of 2 if not a power of 2)
     - All existing items are preserved and rehashed as needed
     - If size is too small this will increase collisions and reduce lookup performance
     - Creating a new item or calling capacity(Size) will automatically resize the hash table if too small
     - \b Caution: This will invalidate iterators on this map
     .
     \param  size  New size/capacity, minimum size of 8 is used if lower
     \return       This
    */
    void resize(Size size) {
        Size newsize = MIN_SIZE;
        while (newsize < size) newsize <<= 1;
        if (newsize != buckets_.size()) {
            data_.sizemask  = EVO_IMPL_MAPHASH_SIZEMASK(newsize);
            data_.threshold = EVO_IMPL_MAPHASH_THRESHOLD(newsize);
            Buckets oldbuckets;
            buckets_.swap(oldbuckets);
            buckets_.resize(newsize);

            bool created;
            const Bucket* bucket;
            const Bucket** cur = (const Bucket**)oldbuckets.data();
            for (const Bucket** end=cur+oldbuckets.size(); cur < end; ++cur) {
                if ((bucket=*cur) != NULL) {
                    for (Size i=0, iend=bucket->others.size(); i <= iend; ++i) {
                        const Item* item = (i == 0 ? &bucket->first : &bucket->others[i-1]);
                        Bucket* newbucket = buckets_.getitem(data_.hash(item->first) & data_.sizemask, &created);
                        if (created)
                            newbucket->first = *item;
                        else {
                            Size index;
                            Item* newitem = (Item*)newbucket->search(index, item->first, data_);
                            if (newitem == NULL)
                                newbucket->others.insert(index, *item);
                            else
                                newitem->second = item->second;
                        }
                    }
                }
            }
        }
    }

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
        { return move((IterM&)src, iterNONE); }

    // REMOVE

    bool remove(const Key& key) {
        bool result;
        Size bucket_index = data_.hash(key) & data_.sizemask;
        Bucket* bucket = buckets_.itemM(bucket_index);
        if (bucket != NULL) {
            Size index;
            if (bucket->first.first == key) {
                if (bucket->others.size() > 0) {
                    // Remove "first" and replace with last item in "others"
                    Item* item1 = &bucket->first;
                    Item* item2 = bucket->others.data();
                    EVO_IMPL_CONTAINER_SWAP(item1, item2, Item);
                    bucket->others.remove(0);
                } else
                    // Remove bucket since "first" is only item
                    buckets_.remove(bucket_index);
                result = true;
                --size_;
            } else if (bucket->search(index, key, data_) != NULL) {
                // Remove from "others"
                bucket->others.remove(index);
                result = true;
                --size_;
            } else
                result = false;
        } else
            result = false;
        return result;
    }

    bool remove(typename MapBaseType::IterM& iter, IteratorDir dir=iterNONE)
        { return remove((IterM&)iter, dir); }

    bool remove(IterM& iter, IteratorDir dir=iterNONE) {
        if (iter && this == &iter.getParent()) {
            IterKey& iterkey = iter.getKey();
            assert( iterkey.a < buckets_.size() );
            Bucket* bucket = buckets_.itemM(iterkey.a);
            assert( bucket != NULL );
            if (iterkey.b > 0) {
                // Remove from "others"
                bucket->others.remove(iterkey.b-1);
            } else {
                if (bucket->others.size() > 0) {
                    // Remove "first" and replace with last item in "others"
                    Item* item1 = &bucket->first;
                    Item* item2 = bucket->others.data();
                    EVO_IMPL_CONTAINER_SWAP(item1, item2, Item);
                    bucket->others.remove(0);
                } else {
                    // Remove bucket since "first" is only item
                    buckets_.remove(iterkey.a);
                    bucket = NULL;
                }
            }

            // Update size and iterator
            if (--size_ > 0 && dir != iterNONE) {
                if (dir == iterRV) {
                    // Previous
                    if (iterkey.b > 0 && bucket != NULL) {
                        iter.setData( (IterItem*)(--iterkey.b == 0 ? &(bucket->first) : &(bucket->others[iterkey.b-1])) );
                    } else {
                        bucket = (Bucket*)buckets_.iterPrev(iterkey.a);
                        if (bucket != NULL) {
                            iterkey.b = bucket->others.size();
                            iter.setData( (IterItem*)(iterkey.b == 0 ? &bucket->first : &(bucket->others[iterkey.b-1])) );
                        } else
                            iter = iterEND;
                    }
                } else {
                    // Next
                    if (bucket != NULL && iterkey.b <= bucket->others.size()) {
                        iter.setData( (IterItem*)(iterkey.b == 0 ? &bucket->first : &(bucket->others[iterkey.b-1])) );
                    } else {
                        iterkey.b = 0;
                        if ((bucket=(Bucket*)buckets_.iterNext(iterkey.a)) != NULL)
                            iter.setData( (IterItem*)&bucket->first );
                        else
                            iter = iterEND;
                    }
                }
            } else
                iter = iterEND;
            return true;
        }
        return false;
    }

    // INTERNAL

    // Testing methods
    /** \cond impl */
#if EVO_UNIT_TEST_MODE
    Size utCollisions() const {
        Size count = 0;
        for (typename Buckets::Iter iter(buckets_); iter; ++iter)
            count += iter->others.size();
        return count;
    }
#endif
    /** \endcond */

    // Iterator support methods
    /** \cond impl */
    void iterInitMutable()
        { buckets_.unshare(); }
    const IterItem* iterFirst(IterKey& key) const {
        key.b = 0;
        if (size_ > 0)
            return (IterItem*)&(buckets_.iterFirst(key.a)->first);
        key.a = END;
        return NULL;
    }
    const IterItem* iterNext(IterKey& key) const {
        if (key.a != END) {
            const Bucket* bucket = buckets_.item(key.a);
            if (++key.b <= bucket->others.size())
                return (IterItem*)&(bucket->others[key.b-1]);
            key.b = 0;
            if ((bucket=buckets_.iterNext(key.a)) != NULL)
                return (IterItem*)&bucket->first;
        }
        return NULL;
    }
    const IterItem* iterLast(IterKey& key) const {
        if (size_ > 0) {
            const Bucket* bucket = buckets_.iterLast(key.a);
            assert( bucket != NULL );
            key.b = bucket->others.size();
            return (IterItem*)(key.b == 0 ? &bucket->first : &(bucket->others[key.b-1]));
        }
        key.a = END;
        key.b = 0;
        return NULL;

    }
    const IterItem* iterPrev(IterKey& key) const {
        if (key.a != END) {
            if (key.b > 0) {
                return (IterItem*)(--key.b == 0 ? &(buckets_.item(key.a)->first) : &(buckets_.item(key.a)->others[key.b-1]));
            } else {
                const Bucket* bucket = buckets_.iterPrev(key.a);
                if (bucket != NULL) {
                    key.b = bucket->others.size();
                    return (IterItem*)(key.b == 0 ? &bucket->first : &(bucket->others[key.b-1]));
                }
            }
            key.a = END;
        }
        return NULL;
    }
    /** \endcond */

protected:
    const Item* getiter(IterKey& iterkey, const Key& key) const {
        if (size_ > 0) {
            iterkey.a = (data_.hash(key) & data_.sizemask);
            const Bucket* bucket = buckets_.item(iterkey.a);
            if (bucket != NULL) {
                if (bucket->first.first == key) {
                    iterkey.b = 0;
                    return &bucket->first;
                } else {
                    const Item* item = bucket->search(iterkey.b, key, data_);
                    if (item != NULL) {
                        ++iterkey.b;
                        return item;
                    }
                }
            }
        }
        iterkey.a = END;
        iterkey.b = 0;
        return NULL;
    }

private:
    static const Size SIZE_INIT = 64; // initial hash size -- must be power of 2
    static const Size MIN_SIZE  = 8;  // minimum hash size -- must be power of 2

    struct Bucket {
        Item        first;
        Array<Item> others;

        Bucket()
            { }
        Bucket(const Bucket& src) : first(src.first), others(src.others)
            { }

        Bucket& operator=(const Bucket& src) {
            first  = src.first;
            others = src.others;
            return *this;
        }

        // Search "others" for key, using compare, set index (insertion point if not found)
        const Item* search(Size& index, const Key& key, const typename Hash::CompareBase& compare) const {
            int cmp;
            Size left = 0, right = others.size(), mid = 0;
            const Item* items = others.data();
            while (left < right) {
                mid = left + ((right-left) / 2);
                const Item* item = items + mid;
                cmp = compare(key, item->first);
                if (cmp < 0) {
                    right = mid;
                } else if (cmp == 0) {
                    index = mid;
                    return item;
                } else
                    left = mid + 1;
            }
            index = left;
            return NULL;
        }
    };

    typedef PtrList<Bucket,Size> Buckets;

    Buckets buckets_;

    // Use inheritance to reduce size bloat with empty Hash
    struct Data : public THash {
        Size sizemask;
        Size threshold;

        Data() : sizemask(0), threshold(0)
            { }
        Data(const THash& hash) : THash(hash), sizemask(0), threshold(0)
            { }
        Data(const Data& data) : Hash(data), sizemask(data.sizemask), threshold(data.threshold)
            { }
    };

    Data data_;
};

#undef EVO_IMPL_MAPHASH_SIZEMASK
#undef EVO_IMPL_MAPHASH_THRESHOLD

///////////////////////////////////////////////////////////////////////////////

#if defined(INCL_evo_string_h) || defined(DOXYGEN)
/** MapHash using String keys and values.
 - This is an alias to MapHash
 - This is only defined if string.h is included before maphash.h
*/
typedef MapHash<String,String> StrHash;
#endif

///////////////////////////////////////////////////////////////////////////////
//@}
}
#if defined(_MSC_VER)
    #pragma warning(pop)
#endif
#endif
