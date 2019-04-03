// Evo C++ Library
/* Copyright 2019 Justin Crowell
Distributed under the BSD 2-Clause License -- see included file LICENSE.txt for details.
*/
///////////////////////////////////////////////////////////////////////////////
/** \file sethash.h Evo SetHash container. */
#pragma once
#ifndef INCL_evo_sethash_h
#define INCL_evo_sethash_h

#include "set.h"
#include "array.h"
#include "ptrlist.h"
#include "impl/hash.h"

namespace evo {
/** \addtogroup EvoContainers */
//@{

///////////////////////////////////////////////////////////////////////////////

/** Set implemented as a hash table.

\tparam  TKey   %Set key type
\tparam  THash  Hash and comparison type to use
\tparam  TSize  %Size type to use for size values (must be unsigned integer) -- default: SizeT

\par Features

 - Similar to STL set, with an implementation similar to STL unordered_set
 - Values are always unique (no duplicates)
 - This uses a hash table -- lookups and inserts use constant time `O(1)`, as long as a good hash function is used (low collisions)
 - This uses PtrList internally for buckets, and Array for collisions
   - No memory allocated by new empty set
   - \ref Sharing "Sharing" makes copying efficient
   - See PtrList features
 .

 C++11:
 - Range-based for loop -- see \ref StlCompatibility
   \code
    SetHash<int> hash;
    for (auto num : hash.asconst()) {
    }
   \endcode
 - Initialization lists
   \code
    SetHash<int> hash = {3, 1, 2};
    SetHash<String> strhash = {"foo", "bar"};
   \endcode
 - Move semantics

\par Hashing and Comparison

You can leave the default hashing and comparison type (CompareHash) or specify an alternative.

 - CompareHash
 .

See: \ref PrimitivesContainers "Primitives & Containers"

\par Iterators

 - SetHash::Iter -- Read-Only Iterator (IteratorBi)
 - SetHash::IterM -- Mutable Iterator (IteratorBi)
 .

\b Caution: Modifying or resizing a set will shift or invalidate existing iterators (and pointers) using it.

\par Constructors

 - SetHash()
 - SetHash(const SetBaseType&)
 - SetHash(const ThisType&)
 - SetHash(std::initializer_list<Value>) [C++11]
 - SetHash(ThisType&&) [C++11]

\par Read Access

 - asconst()
 - size()
   - null(), empty()
   - capacity()
   - shared()
   - ordered()
   - get_compare() const
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
   - get_compare()
 - begin(), end()
 - iterM()
 - set()
   - set(const ThisType&)
   - setempty()
   - clear()
   - operator=(const SetBaseType&)
   - operator=(const ThisType&)
   - operator=(ThisType&&) [C++11]
 - get()
 - add(const Value&,bool)
   - addfrom()
   - addsplit()
 - remove(const Value&)
   - remove(IterM&,IteratorDir)
 .

\par Examples

Example using a set of numbers

\code
#include <evo/string.h>
#include <evo/sethash.h>
#include <evo/io.h>
using namespace evo;
static Console& c = con();

int main() {
    // Create set with numeric values
    SetHash<int> set;
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
    for (SetHash<int>::Iter iter(set); iter; ++iter)
        c.out << *iter << NL;

    return 0;
}
\endcode

Output:
\code{.unparsed}
10,20,40,30
10
20
40
30
\endcode
*/
template<class TKey, class THash=CompareHash<TKey>, class TSize=SizeT>
class SetHash : public Set<TKey,TSize> {
protected:
    using Set<TKey,TSize>::size_;

public:
    EVO_CONTAINER_TYPE;
#if defined(_MSC_VER) || defined(EVO_OLDCC) // avoid errors with older compilers and MSVC
    typedef Set<TKey,TSize> SetBaseType;                        ///< %Set base type
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
    typedef SetHash<TKey,THash,TSize> ThisType;                 ///< This type
    typedef THash Hash;                                         ///< Hashing type -- default: CompareHash

    typedef typename IteratorBi<ThisType>::Const Iter;          ///< Iterator (const) - IteratorBi
    typedef IteratorBi<ThisType>                 IterM;         ///< Iterator (mutable) - IteratorBi

    /** Constructor. */
    SetHash()
        { }

    /** Copy constructor.
     \param  src  Source to copy
    */
    SetHash(const SetBaseType& src) {
        for (typename SetBaseType::Iter iter(src); iter; ++iter)
            add(*iter);
    }

    /** Copy constructor.
     \param  src  Source to copy
    */
    SetHash(const ThisType& src)
        { set(src); }

#if defined(EVO_CPP11)
    /** Sequence constructor (C++11).
     \param  init  Initializer list, passed as comma-separated values in braces `{ }`
    */
    SetHash(std::initializer_list<Value> init) : SetHash() {
        assert( init.size() < IntegerT<Size>::MAX );
        capacitymin((Size)init.size());
        for (const auto& val : init)
            add(val);
    }

    /** Move constructor (C++11).
     \param  src  Source to move
    */
    SetHash(ThisType&& src) : buckets_(std::move(src.buckets_)), data_(std::move(src.data_)) {
        SetBaseType::size_ = src.SetBaseType::size_;
        src.SetBaseType::size_ = 0;
    }

    /** Move assignment operator (C++11).
     \param  src  Source to move
     \return      This
    */
    ThisType& operator=(ThisType&& src) {
        buckets_ = std::move(src.buckets_);
        data_ = std::move(src.data_);
        SetBaseType::size_ = src.SetBaseType::size_;
        src.SetBaseType::size_ = 0;
        return *this;
    }
#endif

    /** \copydoc List::asconst() */
    const ThisType& asconst() const {
        return *this;
    }

    // SET

    /** \copydoc Set::operator=() */
    ThisType& operator=(const SetBaseType& src)
        { set(src); return *this; }

    /** \copydoc Set::operator=(const SetBaseType& src) */
    ThisType& operator=(const ThisType& src)
        { set(src); return *this; }

    ThisType& set()
        { buckets_.set(); size_ = 0; return *this; }

    /** \copydoc Set::set(const SetBaseType& src) */
    ThisType& set(const SetBaseType& src) {
        clear();
        for (typename SetBaseType::Iter iter(src); iter; ++iter)
            add(*iter);
        return *this;
    }

    /** \copydoc Set::set(const SetBaseType& src) */
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

    bool ordered() const
        { return false; }

    // COMPARE

    /** Get hash & comparison object being used for hashing and comparisons (const).
     \return  %Hash & comparison object reference
    */
    const Hash& get_compare() const
        { return data_; }

    /** Get hash & comparison object being used for hashing and comparisons.
     \return  %Hash & comparison object reference
    */
    Hash& get_compare()
        { return data_; }

    using SetBaseType::operator==;
    using SetBaseType::operator!=;

    /** \copydoc Set::operator==(const SetBaseType& other) const */
    bool operator==(const ThisType& set) const {
        if (this == &set)
            return true;
        if (buckets_.size() == set.buckets_.size())
            return (buckets_ == set.buckets_);
        return SetBaseType::operator==(set);
    }

    /** \copydoc Set::operator!=(const SetBaseType& other) const */
    bool operator!=(const ThisType& set) const
        { return !(*this == set); }

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

    bool contains(const Value& value) const
        { return (search(value) != NULL); }

    /** \copydoc Set::iter() */
    Iter iter(const Value& value) const {
        if (size_ > 0) {
            IterKey iterkey( data_.hash(value) & data_.sizemask );
            const Bucket* bucket = buckets_.item(iterkey.a);
            if (bucket != NULL) {
                if (data_(bucket->first, value) == 0) {
                    return Iter(*this, iterkey, (IterItem*)&bucket->first);
                } else {
                    const IterItem* item = (IterItem*)bucket->search(iterkey.b, value, data_);
                    if (item != NULL) {
                        ++iterkey.b;
                        return Iter(*this, iterkey, item);
                    }
                }
            }
        }
        return Iter(*this, iterEND);
    }

    /** \copydoc Set::iterM() */
    IterM iterM(const Value& value) {
        if (size_ > 0) {
            IterKey iterkey( data_.hash(value) & data_.sizemask );
            Bucket* bucket = buckets_.itemM(iterkey.a);
            if (bucket != NULL) {
                if (data_(bucket->first, value) == 0) {
                    return IterM(*this, iterkey, (IterItem*)&bucket->first);
                } else {
                    IterItem* item = (IterItem*)bucket->search(iterkey.b, value, data_);
                    if (item != NULL) {
                        ++iterkey.b;
                        return IterM(*this, iterkey, item);
                    }
                }
            }
        }
        return IterM(*this, iterEND);
    }

    Value& get(const Value& value, bool* created=NULL) {
        if (size_ == 0 || data_.threshold == 0)
            // Empty, use init size
            rehash(SIZE_INIT);
        else if (size_ >= data_.threshold)
            // Reached size threshold, time to grow
            rehash(buckets_.size() << 1);

        // Get item, create if needed
        bool created_item;
        Value* item;
        Bucket* bucket = buckets_.getitem((data_.hash(value) & data_.sizemask), &created_item);
        if (created_item) {
            ++size_;
            item = &bucket->first;
            *item = value;
        } else if (data_(bucket->first, value) == 0) {
            item = &bucket->first;
        } else {
            Size index;
            item = (Value*)bucket->search(index, value, data_);
            if (item == NULL) {
                created_item = true;
                ++size_;
                item = (Value*)&(bucket->others[bucket->others.insert(index, value)]);
            }
        }
        if (created != NULL)
            *created = created_item;
        return *item;
    }

    // INFO_SET

    ThisType& unshare()
        { buckets_.unshare(); return *this; }

    /** %Set hash set capacity (capacity).
     - Use to suggest a new capacity before adding a batch of items
     - This will set hash size to nearest power of 2 that can fit both current items and given size
     - \b Caution: This will invalidate iterators and pointers to values on this set
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

            if (newsize != cursize)
                // Rehash with new size
                rehash(newsize);
        }
        return *this;
    }

    /** %Set set capacity to at least given minimum.
     \param  min  Minimum capacity
     \return      This
    */
    ThisType& capacitymin(Size min) {
        if (min > buckets_.size())
            capacity(min);
        return *this;
    }

    /** \copydoc Set::reserve() */
    ThisType& reserve(Size size)
        { return capacitymin(size_ + size); }

    // ADD

    Value& add(const Value& value, bool update=false) {
        bool created_val;
        Value& upditem = get(value, &created_val);
        if (!created_val && update)
            upditem = value;
        return upditem;
    }

    // REMOVE

    bool remove(const Value& value) {
        bool result;
        Size bucket_index = data_.hash(value) & data_.sizemask;
        Bucket* bucket = buckets_.itemM(bucket_index);
        if (bucket != NULL) {
            Size index;
            if (data_(bucket->first, value) == 0) {
                if (bucket->others.size() > 0) {
                    // Remove "first" and replace with last item in "others"
                    Value* item1 = &bucket->first;
                    Value* item2 = bucket->others.data();
                    EVO_IMPL_CONTAINER_SWAP(item1, item2, Item);
                    bucket->others.remove(0);
                } else
                    // Remove bucket since "first" is only item
                    buckets_.remove(bucket_index);
                result = true;
                --size_;
            } else if (bucket->search(index, value, data_) != NULL) {
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

    bool remove(typename SetBaseType::IterM& iter, IteratorDir dir=iterNONE)
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
                    Value* item1 = &bucket->first;
                    Value* item2 = bucket->others.data();
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
    const Value* getiter(IterKey& iterkey, const Value& value) const {
        if (size_ > 0) {
            iterkey.a = (data_.hash(value) & data_.sizemask);
            const Bucket* bucket = buckets_.item(iterkey.a);
            if (bucket != NULL) {
                if (data_(bucket->first, value) == 0) {
                    iterkey.b = 0;
                    return &bucket->first;
                } else {
                    const Value* item = bucket->search(iterkey.b, value, data_);
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
    static const Size SIZE_INIT = 64;   // initial hash size -- must be power of 2
    static const Size MIN_SIZE  = 8;    // minimum hash size -- must be power of 2

    struct Bucket {
        Value first;
        Array<Value> others;

        Bucket()
            { others.setempty(); }
        Bucket(const Bucket& src) : first(src.first), others(src.others)
            { }
        bool operator==(const Bucket& data)
            { return (first == data.first && others == data.others); }

        // Search "others" for key, using compare, set index (insertion point if not found)
        const Value* search(Size& index, const Value& key, const THash& compare) const {
            int cmp;
            Size left = 0, right = others.size(), mid = 0;
            const Value* items = others.data();
            while (left < right) {
                mid = left + ((right-left) / 2);
                const Value* item = items + mid;
                cmp = compare(key, *item);
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

    private:
        // Copying is done via copy constructor
        Bucket& operator=(const Bucket&) EVO_ONCPP11(= delete);
    };

    typedef PtrList<Bucket,Size> Buckets;

    Buckets buckets_;

    // Use inheritance to reduce size bloat with empty Hash
    struct Data : public THash {
        Size sizemask;
        Size threshold;

        Data() : sizemask(0), threshold(0) {
        }
        Data& operator=(const Data& src) {
            THash::operator=(src);
            sizemask  = src.sizemask;
            threshold = src.threshold;
            return *this;
        }

    #if defined(EVO_CPP11)
        Data(Data&& src) {
            THash::operator=(std::move(src));
            sizemask  = src.sizemask;
            threshold = src.threshold;
            src.sizemask  = 0;
            src.threshold = 0;
        }

        Data& operator=(Data&& src) {
            THash::operator=(std::move(src));
            sizemask  = src.sizemask;
            threshold = src.threshold;
            src.sizemask  = 0;
            src.threshold = 0;
            return *this;
        }
    #endif
    private:
        // Copying is done via assignment operator
        Data(const Data&);
    };

    Data data_;

    void rehash(Size newsize) {
        const float THRESHOLD = 0.7f;
        data_.sizemask  = newsize - 1;
        data_.threshold = (Size)(newsize * THRESHOLD);
        if (size_ == 0) {
            // Empty, just resize buckets if needed
            buckets_.resize(newsize);
        } else {
            // Grow and rehash
            Buckets oldbuckets;
            evo::swap(buckets_, oldbuckets);
            buckets_.resize(newsize);

            bool created;
            const Bucket* bucket;
            const Bucket** cur = (const Bucket**)oldbuckets.data();
            for (const Bucket** end=cur+oldbuckets.size(); cur < end; ++cur) {
                if ((bucket=*cur) != NULL) {
                    for (Size i=0, iend=bucket->others.size(); i <= iend; ++i) {
                        const Value* item = (i == 0 ? &bucket->first : &bucket->others[i-1]);
                        Bucket* newbucket = buckets_.getitem(data_.hash(*item) & data_.sizemask, &created);
                        if (created) {
                            newbucket->first = *item;
                        } else {
                            Size index;
                            Value* newitem = (Value*)newbucket->search(index, *item, data_);
                            if (newitem == NULL)
                                newbucket->others.insert(index, *item);
                        }
                    }
                }
            }
        }
    }

    const Value* search(const Value& value) const {
        if (size_ > 0) {
            const Bucket* bucket = buckets_.item( data_.hash(value) & data_.sizemask );
            if (bucket != NULL) {
                if (data_(value, bucket->first) == 0) {
                    return &bucket->first;
                } else {
                    Size index;
                    const Value* item = bucket->search(index, value, data_);
                    if (item != NULL)
                        return item;
                }
            }
        }
        return NULL;
    }
};

///////////////////////////////////////////////////////////////////////////////

#if defined(INCL_evo_string_h) || defined(DOXYGEN)
/** SetHash using String values.
 - This is an alias to SetHash
 - This is only defined if string.h is included before sethash.h
*/
typedef SetHash<String> StrSetHash;
#endif

///////////////////////////////////////////////////////////////////////////////
//@}
}
#endif
