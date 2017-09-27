// Evo C++ Library
/* Copyright (c) 2016 Justin Crowell
 This Source Code Form is subject to the terms of the Mozilla Public
 License, v. 2.0. If a copy of the MPL was not distributed with this
 file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
///////////////////////////////////////////////////////////////////////////////
/** \file container.h Evo implementation detail: Container support. */
#pragma once
#ifndef INCL_evo_impl_container_h
#define INCL_evo_impl_container_h

// Includes - System
#include <memory.h>
#include <new>

// Includes
#include "../meta.h"
#include "hash.h"

// Namespace: evo
namespace evo {

/** \addtogroup EvoContainers */
//@{

///////////////////////////////////////////////////////////////////////////////

// TODO: Add allocator support
// Internal allocation/deallocation macros -- used by containers
/** \cond impl */
#define EVO_IMPL_CONTAINER_MEM_ALLOC1(TYPE) \
    (TYPE*)::malloc(sizeof(TYPE));
#define EVO_IMPL_CONTAINER_MEM_ALLOC_BYTES(TYPE, BYTES) \
    (TYPE*)::malloc(BYTES);
#define EVO_IMPL_CONTAINER_MEM_FREE(PTR) { \
    assert( PTR != NULL ); \
    ::free(PTR); \
}
#define EVO_IMPL_CONTAINER_SWAP(PTR1, PTR2, TYPE) { \
    char temp[sizeof(TYPE)]; \
    memcpy(temp, PTR1, sizeof(TYPE)); \
    memcpy(PTR1, PTR2, sizeof(TYPE)); \
    memcpy(PTR2, temp, sizeof(TYPE)); \
}
/** \endcond */

///////////////////////////////////////////////////////////////////////////////

// TODO - work in progress
// Allocators
//  - Allocators are not part of any container object type so containers using different allocators can be mixed
//  - Allocator is assigned explicitly to container constructor and cannot be changed for the life of the object
//  - Parent containers will pass their allocator to child containers when containers of containers are used, if allocator is a shared type
// Types
//  - Default
//  - Pool?

/** Allocator base class. */
struct Allocator {
    static const bool shared = true;    ///< This allocator can be shared among multiple containers

    /** Destructor */
    virtual ~Allocator() { }
    /** Allocate memory.
     \param  bytes  Number of bytes to allocate
     \return        Pointer to allocated memory
    */
    virtual char* alloc(ulong bytes) = 0;
    /** Reallocate memory.
     \param  ptr    Pointer to existing memory to reallocate (must have been allocated with same allocator)
     \param  bytes  Number of bytes to allocate
     \return        Pointer to reallocated memory
    */
    virtual char* realloc(void* ptr, ulong bytes) = 0;
    /** Free memory.
     \param  ptr  Pointer to memory to free (must have been allocated with same allocator)
    */
    virtual void free(void* ptr) = 0;
};

///////////////////////////////////////////////////////////////////////////////

/** Optimized resizing calculation. */
class SizeOp {
public:
    static const ulong INIT      = 64;            ///< Default initial size
    static const ulong THRESHOLD = 134217728;    ///< Growth threshold (max size increase) (128*1024*1024)

    /** Get initial data size.
     \param  size  Initial size to adjust, if needed
     \return       Adjusted initial size (given size or INIT, whichver is smaller)
    */
    static ulong init(ulong size)
        { return (size < INIT ? INIT : size); }

    /** Grow data size.
     - The data size is grown by a factor of 1.5x or THRESHOLD, whichever is smaller
     - Returns ULONG_MAX on overflow
     .
     \param  size  Size to grow - must be less than ULONG_MAX
     \return       Adjusted data size
    */
    static ulong grow(ulong size) {
        assert( size < ULONG_MAX );
        ulong inc = (size >> 1) + 1;
        if (inc > THRESHOLD)
            inc = THRESHOLD;
        if (ULONG_MAX - inc < size)
            size = ULONG_MAX;
        else
            size += inc;
        return size;
    }
};

///////////////////////////////////////////////////////////////////////////////

// Implementation
/** \cond impl */
namespace impl
{
    // DataOp
    template<class T> struct DataOpT {
        typedef typename RemoveConst<T>::Type Item;
        #if EVO_UNIT_TEST_ITEMARG_REF
            // Always pass by-reference so unit tests can detect constructor calls
            typedef const Item& ParamT;
        #else
            typedef typename StaticIf<IsPodType<Item>::value,Item,const Item&>::Type ParamT;
        #endif

        // Init
        struct InitPod {
            static void init(Item& item)
                { memset(&item, 0, sizeof(Item)); }
            static void uninit(Item&)
                { }
        };
        struct InitObj {
            static void init(Item& item)
                { new(&item) T(); }
            static void uninit(Item& item)
                { item.~T(); }
        };
        typedef typename StaticIf<IsPodType<Item>::value,InitPod,InitObj>::Type Init;

        // Default
        struct DefaultPod {
            static void defval(Item& dest)
                { memset(&dest, 0, sizeof(Item)); }
            static void defvalpod(Item& dest)
                { memset(&dest, 0, sizeof(Item)); }
        };
        struct DefaultObj {
            static void defval(Item& dest)
                { static const Item DEFAULT; dest = DEFAULT; }
            static void defvalpod(Item&)
                { }
        };
        typedef typename StaticIf<IsPodType<Item>::value,DefaultPod,DefaultObj>::Type Default;

        // Fill
        struct FillAssign {
            static void fill(Item* dest, ulong size, const Item& val) {
                for (Item* end=dest+size; dest<end; ++dest)
                    *dest = val;
            }
            static void fillends(Item* dest, ulong size, const Item& val, ulong start, ulong end) {
                assert( sizeof(Item) > 1 );
                assert( dest != NULL );
                assert( start < end );
                while (start > 0)
                    dest[--start] = val;
                for (; end < size; ++end)
                    dest[end] = val;
            }
        };
        struct FillMemcopy {
            static void fill(Item* dest, ulong size, ParamT val) {
                assert( sizeof(Item) > 1 );
                for (Item* end=dest+size; dest<end; ++dest)
                    memcpy(dest, &val, sizeof(Item));
            }
            static void fillends(Item* dest, ulong size, ParamT val, ulong start, ulong end) {
                assert( sizeof(Item) > 1 );
                assert( dest != NULL );
                assert( start < end );
                while (start > 0)
                    memcpy(dest+(--start), &val, sizeof(Item));
                for (; end < size; ++end)
                    memcpy(dest+end, &val, sizeof(Item));
            }
        };
        struct FillMemset {
            static void fill(Item* dest, ulong size, ParamT val) {
                assert( sizeof(Item) == 1 );
                uchar cval;
                memcpy(&cval, &val, 1);
                memset(dest, (int)cval, size);
            }
            static void fillends(Item* dest, ulong size, ParamT val, ulong start, ulong end) {
                assert( sizeof(Item) == 1 );
                assert( dest != NULL );
                assert( start < end );
                uchar cval;
                memcpy(&cval, &val, 1);
                if (start > 0)
                    memset(dest, cval, start);
                if (end < size)
                    memset(dest+end, cval, size-end);
            }
        };
        typedef typename StaticIf< IsPodType<Item>::value || IsByteCopyType<Item>::value,
            typename StaticIf<sizeof(Item)==1,FillMemset,FillMemcopy>::Type,
            FillAssign
        >::Type Fill;

        // Compare
        struct CompareMem {
            static bool equal(const Item* data1, const Item* data2, ulong size) {
                assert( data1 != NULL || size == 0 );
                assert( data2 != NULL || size == 0 );
                return (size == 0 || data1 == data2 || memcmp(data1, data2, size*sizeof(Item)) == 0);
            }
            static int compare(const Item* data1, ulong size1, const Item* data2, ulong size2) {
                assert( data1 != NULL || size1 == 0 );
                assert( data2 != NULL || size2 == 0 );
                int result;
                if (data1 == data2) {
                    if (size1 == size2)
                        result = 0;
                    else
                        result = (size1 < size2 ? -1 : 1);
                } else if (size1 < size2) {
                    result = memcmp(data1, data2, size1*sizeof(Item));
                    if (result == 0)
                        result = -1;
                } else if (size2 < size1) {
                    result = memcmp(data1, data2, size2*sizeof(Item));
                    if (result == 0)
                        result = 1;
                } else
                    result = memcmp(data1, data2, size1*sizeof(Item));
                return result;
            }
            static int compare(const Item& item1, const Item& item2)
                { return memcmp(&item1, &item2, sizeof(Item)); }
        };
        struct CompareObj {
            static bool equal(const Item* data1, const Item* data2, ulong size) {
                assert( data1 != NULL || size == 0 );
                assert( data2 != NULL || size == 0 );
                bool result;
                if (size > 0 && data1 != data2) {
                    result = true;
                    ulong i = 0;
                    do {
                        if (!(data1[i] == data2[i]))
                            { result = false; break; }
                    } while (++i < size);
                } else
                    result = true;
                return result;
            }
            static int compare(const Item* data1, ulong size1, const Item* data2, ulong size2) {
                assert( data1 != NULL || size1 == 0 );
                assert( data2 != NULL || size2 == 0 );
                int result = 0;
                const int size = (size1 < size2 ? size1 : size2);
                if (size > 0) {
                    for (ulong i=0; i<size; ++i) {
                        result = data1[i].compare(data2[i]);
                        if (result != 0)
                            break;
                    }
                    if (result == 0) {
                        if (size1 < size2)
                            result = -1;
                        else if (size2 < size1)
                            result = 1;
                    }
                }
                return result;
            }
            static int compare(const Item& item1, const Item& item2)
                { return item1.compare(item2); }
        };
        typedef typename StaticIf<IsEvoItem<Item>::value,CompareObj,CompareMem>::Type Compare;

        // Hash
        template<class H> struct Hash {
            struct HashMem {
                static ulong hash(const Item* data, ulong size, ulong seed=0)
                    { return H::hash(data, size*sizeof(Item), seed); }
                static ulong hash(const Item& data, ulong seed=0)
                    { return H::hash(&data, sizeof(Item), seed); }
            };
            struct HashObj {
                static ulong hash(const Item* data, ulong size, ulong seed=0) {
                    for (ulong i=0; i<size; ++size)
                        seed = data[i].hash(seed);
                    return seed;
                }
                static ulong hash(const Item& data, ulong seed=0)
                    { return data.hash(seed); }
            };
            typedef typename StaticIf<IsEvoItem<Item>::value,HashObj,HashMem>::Type Type;
        };
    };
}
/** \endcond */

///////////////////////////////////////////////////////////////////////////////

// TODO: Consolidate with ContainerOp?
/** Data operations optimized by type.
 - This is used by Evo containers
 - See also: ContainerOp
 .
 \tparam  T       Data type
 \tparam  Hasher  Hasher type to use on raw data
*/
template<class T,class Hasher=SpookyHash>
struct DataOp :
    public evo::impl::DataOpT<T>::Init,
    public evo::impl::DataOpT<T>::Default,
    public evo::impl::DataOpT<T>::Fill,
    public evo::impl::DataOpT<T>::Compare,
    public evo::impl::DataOpT<T>::template Hash<Hasher>::Type
{
    typedef evo::impl::DataOpT<T>    DataOpT;    ///< Implementation
    typedef typename DataOpT::Item   Item;        ///< Item type (const removed)
    typedef typename DataOpT::ParamT ParamT;    ///< Best parameter type to pass Item as (by-value or by-const-reference)

    #ifdef DOXYGEN
    /** Initialize value using default constructor.
     - For POD types this sets to 0
     - For all other types this calls item default-constructor via placement-new
     - Alternatively, placement-new can be called directly (but won't set POD types to 0) - example: new(&val) T();
     .
     \param  val  Value to initialize
    */
    static void init(Item& val);

    /** Uninitialize value using destructor.
     - For POD types this does nothing
     - For all other types this calls item destructor
     - Alternatively, destructor can be called directly - example: val.~T();
     .
     \param  val  Value to initialize
    */
    static void uninit(Item& val);

    /** Set value to default.
     - For POD types this sets to 0
     - For all other types this copies from a default-constructed object
     .
     \param  val  Value to initialize
    */
    static void defval(Item& val);

    /** Set new POD value to default value (0).
     - For POD types this sets to 0
     - For all other types this does nothing
     .
     \param  val  Value to initialize
    */
    static void defvalpod(Item& val);

    /** Fill with copies of given item.
     - This is optimized to use item operator=(), memcpy(), or memset() as applicable
     .
     \param  dest   Destination data pointer to fill - must be valid
     \param  size   Copies to fill with as item count - must be positive
     \param  value  Value to fill with
    */
    static void fill(T* dest, ulong size, const T& value);

    /** Fill each end of destination with copies of given item.
     - This is useful for initializing items outside given range (start to end), if any
     - The fills items before start (if start>0) and remaining items after (and including) end (if end<size)
     - This is optimized to use item operator=(), memcpy(), or memset() as applicable
     .
     \param  dest   Destination data pointer to fill - must be valid
     \param  size   Destination size as item count - must be positive
     \param  value  Value to fill with
     \param  start  Start index of items not to fill -- items before this are filled
     \param  end    End index of items not to fill -- items after (and including) this are filled
    */
    static void fillends(Item* dest, ulong size, const T& value, ulong start, ulong end);

    /** %Compare data for equality.
     - This uses Item::operator==() if available, otherwise uses memcmp()
     .
     \param  data1  Data to compare
     \param  data2  Data to compare to
     \param  size   Size to compare
    */
    static bool equal(const Item* data1, const Item* data2, ulong size);

    /** %Compare data.
     - This uses data.compare() if available, otherwise uses memcmp()
     .
     \param  data1  Data to compare
     \param  size1  Data size to compare as item count
     \param  data2  Data to compare to
     \param  size2  Data size to compare to as item count
     \return        Result (<0 if data1 is less, 0 if equal, >0 if data1 is greater)
    */
    static int compare(const Item* data1, ulong size1, const Item* data2, ulong size2);

    /** %Compare items.
     - This uses data.compare() if available, otherwise uses memcmp()
     .
     \param  item1  Item1 to compare
     \param  item2  Item2 to compare
     \return        Result (<0 if item1 is less, 0 if equal, >0 if item1 is greater)
    */
    static int compare(const Item& item1, const Item& item2);

    /** Compute hash value from data.
     - This uses data.hash() if available, otherwise uses Hasher::hash()
     .
     \param  data  Data to use
     \param  size  Data size as item count
     \param  seed  Seed value for including previous hash, 0 if none
     \return       Hash value
     */
    static ulong hash(const Item* data, ulong size, ulong seed=0);

    /** Compute hash value from data.
     - This uses data.hash() if available, otherwise uses Hasher::hash()
     .
     \param  data  Data to use
     \param  seed  Seed value for including previous hash, 0 if none
     \return       Hash value
     */
    static ulong hash(const Item& data, ulong seed=0);
    #endif
};

///////////////////////////////////////////////////////////////////////////////

/** Container operations optimized by item type.
 - Operations are specialized and optimized for POD and ByteCopy types
 - This is used by Evo containers
 - See also: DataOp
 .
 \tparam  T    Data type
 \tparam  TId  Data type ID, leave default -- used internally
*/
template<class T,class TId=typename TypeId<T>::Type> struct ContainerOp
{
    /** Most efficient type for passing as parameter (const-reference or POD value). */
    typedef typename AddConst<T>::Type& Pass;

    /** Initialize data using default constructor.
     - Same as init(T*,ulong) except will initialize new POD values to 0, so is safer but slower for POD types
     .
     \param  data  Data pointer - must be valid
     \param  size  Data size - must be positive
    */
    static void initSafe(T* data, ulong size) {
        assert( data != NULL );
        assert( size > 0 );
        for (ulong i=0; i<size; ++i)
            new(&data[i]) T();
    }

    /** Initialize data using copy constructor and default constructor.
     - Same as init(T*,ulong,const T*,ulong) except will initialize new POD values to 0, so is safer but slower for POD types
     .
     \param  data   Data pointer - must be valid
     \param  size   Data size - must be positive
     \param  src    Source data to copy - must be valid
     \param  count  Number of items to copy - must be positive
    */
    static void initSafe(T* data, ulong size, const T* src, ulong count) {
        assert( data != NULL );
        assert( size > 0 );
        assert( src != NULL );
        assert( count > 0 );
        if (count > size)
            count = size;
        ulong i = 0;
        for (; i<count; ++i)
            new(&data[i]) T(src[i]);
        for (; i<size; ++i)
            new(&data[i]) T();
    }

    /** Initialize data using default constructor.
     - Same as initSafe() except will not initialize new POD values, so is faster for POD types
     .
     \param  data  Data pointer - must be valid
     \param  size  Data size - must be positive
    */
    static void init(T* data, ulong size) {
        assert( data != NULL );
        assert( size > 0 );
        for (ulong i=0; i<size; ++i)
            new(&data[i]) T();
    }

    /** Initialize data using copy constructor.
     \param  data   Data pointer - must be valid
     \param  src    Source data to copy - must be valid
     \param  count  Number of items to copy - must be positive
    */
    static void init(T* data, const T* src, ulong count) {
        assert( data != NULL );
        assert( src != NULL );
        assert( count > 0 );
        for (ulong i=0; i<count; ++i)
            new(&data[i]) T(src[i]);
    }

    /** Initialize data using copy constructor and default constructor.
     \param  data   Data pointer - must be valid
     \param  size   Data size - must be positive
     \param  src    Source data to copy - must be valid
     \param  count  Number of items to copy - must be positive
    */
    static void init(T* data, ulong size, const T* src, ulong count) {
        assert( data != NULL );
        assert( size > 0 );
        assert( src != NULL );
        assert( count > 0 );
        if (count > size)
            count = size;
        ulong i = 0;
        for (; i<count; ++i)
            new(&data[i]) T(src[i]);
        for (; i<size; ++i)
            new(&data[i]) T();
    }

    /** Initialize new tail data (default constructor).
     - This is usefull for growing an array and initializes new data when newSize is greater than oldSize
     - Same as initTailFast() except will initialize new POD values to 0, so is safer but slower for POD types
     .
     \param  data     Data pointer - must be valid
     \param  oldSize  Old data size that is already initialized
     \param  newSize  New data size that will need data initialized if larger
    */
    static void initTailSafe(T* data, ulong oldSize, ulong newSize) {
        assert( data != NULL );
        assert( oldSize >= 0 );
        assert( newSize >= 0 );
        for (ulong i=oldSize; i<newSize; ++i)
            new(&data[i]) T();
    }

    /** Initialize new tail data (default constructor).
     - This is usefull for growing an array and initializes new data when newSize is greater than oldSize
     - Same as initTailSafe() except will not initialize new POD values, so is faster for POD types
     .
     \param  data     Data pointer - must be valid
     \param  oldSize  Old data size that is already initialized
     \param  newSize  New data size that will need data initialized if larger
    */
    static void initTailFast(T* data, ulong oldSize, ulong newSize) {
        assert( data != NULL );
        assert( oldSize >= 0 );
        assert( newSize >= 0 );
        for (ulong i=oldSize; i<newSize; ++i)
            new(&data[i]) T();
    }

    /** Initialize new item as copy of src (copy constructor).
     \param  item  Item to initialize
     \param  src   Source to copy
    */
    static void initcopy(T* item, const T* src) {
        assert( item != NULL );
        assert( src != NULL );
        new(*item) T(*src);
    }

    // TODO: Move to DataOp
    /** Copy already initialized data (assignment operator).
     \param  dest  Destination data pointer - must be valid
     \param  src   Source data pointer - must be valid
     \param  size  Copy size - must be positive
    */
    static void copy(T* dest, const T* src, ulong size) {
        assert( dest != NULL );
        assert( src != NULL );
        assert( size > 0 );
        for (ulong i=0; i<size; ++i)
            dest[i] = src[i];
    }

    /** Uninitialize data (destructor).
     \param  data  Data pointer - must be valid
     \param  size  Size of data
    */
    static void unInit(T* data, ulong size) {
        assert( data != NULL );
        assert( size > 0 );
        while (size > 0)
            data[--size].~T();
    }

    /** Uninitialize and free array of pointers (destructor).
     - Skips null pointers
     .
     \param  data  Data pointer - must be valid
     \param  size  Size of data
    */
    static void unInitFreePtr(T** data, ulong size) {
        assert( data != NULL );
        assert( size > 0 );
        for (; size > 0; ++data, --size)
            if (*data != NULL) {
                (**data).~T();
                EVO_IMPL_CONTAINER_ITEM_FREE(*data);
            }
    }

    /** Uninitialize old tail data (destructor).
     - This is usefull for shrinking an array and uninitializing old data when newSize is less than oldSize
     .
     \param  data     Data pointer - must be valid
     \param  oldSize  Old data size that is already initialized
     \param  newSize  New data size that will need data uninitialized if smaller
    */
    static void unInitTail(T* data, ulong oldSize, ulong newSize) {
        assert( data != NULL );
        assert( oldSize >= 0 );
        assert( newSize >= 0 );
        while (oldSize > newSize)
            data[--oldSize].~T();
    }
};
/** \cond impl */
template<class T> struct ContainerOp<T,TIdByteCopy> {
    typedef typename AddConst<T>::Type& Pass;
    static void initSafe(T* data, ulong size) {
        assert( data != NULL );
        assert( size > 0 );
        for (ulong i=0; i<size; ++i)
            new(&data[i]) T();
    }
    static void initSafe(T* data, ulong size, const T* src, ulong count) {
        assert( data != NULL );
        assert( size > 0 );
        assert( src != NULL );
        assert( count > 0 );
        if (count > size)
            count = size;
        if (count > 0)
            memcpy(data, src, sizeof(T)*count);
        for (ulong i=count; i<size; ++i)
            new(&data[i]) T();
    }
    static void init(T* data, ulong size) {
        assert( data != NULL );
        assert( size > 0 );
        for (ulong i=0; i<size; ++i)
            new(&data[i]) T();
    }
    static void init(T* data, const T* src, ulong count) {
        assert( data != NULL );
        assert( src != NULL );
        assert( count > 0 );
        assert( (ulong)(data < src ? (src-data) : (data-src)) >= count );
        if (count > 0)
            memcpy(data, src, sizeof(T)*count);
    }
    static void init(T* data, ulong size, const T* src, ulong count) {
        assert( data != NULL );
        assert( size > 0 );
        assert( src != NULL );
        assert( count > 0 );
        if (count > size)
            count = size;
        if (count > 0)
            memcpy(data, src, sizeof(T)*count);
        for (ulong i=count; i<size; ++i)
            new(&data[i]) T();
    }
    static void initTailSafe(T* data, ulong oldSize, ulong newSize) {
        assert( data != NULL );
        assert( oldSize >= 0 );
        assert( newSize >= 0 );
        for (ulong i=oldSize; i<newSize; ++i)
            new(&data[i]) T();
    }
    static void initTailFast(T* data, ulong oldSize, ulong newSize) {
        assert( data != NULL );
        assert( oldSize >= 0 );
        assert( newSize >= 0 );
        for (ulong i=oldSize; i<newSize; ++i)
            new(&data[i]) T();
    }
    static void initcopy(T* item, const T* src) {
        assert( item != NULL );
        assert( src != NULL );
        new(*item) T(*src);
    }
    static void copy(T* dest, const T* src, ulong size) {
        assert( dest != NULL );
        assert( src != NULL );
        assert( size > 0 );
        if (size > 0)
            memcpy(dest, src, sizeof(T)*size);
    }
    static void unInit(T* data, ulong size) {
        assert( data != NULL );
        assert( size > 0 );
        while (size > 0)
            data[--size].~T();
    }
    static void unInitTail(T* data, ulong oldSize, ulong newSize) {
        assert( data != NULL );
        assert( oldSize >= 0 );
        assert( newSize >= 0 );
        while (oldSize > newSize)
            data[--oldSize].~T();
    }
};
template<class T> struct ContainerOp<T,TIdPod> {
    typedef T Pass;
    static void initSafe(T* data, ulong size)
        { memset(data, 0, size*sizeof(T)); }
    static void initSafe(T* data, ulong size, const T* src, ulong count) {
        assert( data != NULL );
        assert( size > 0 );
        assert( src != NULL );
        assert( count > 0 );
        if (count > size)
            count = size;
        if (count > 0)
            memcpy(data, src, sizeof(T)*count);
        size -= count;
        if (size > 0)
            memset(data+count, 0, sizeof(T)*size);
    }
    static void init(T*, ulong)
        { }
    static void init(T* data, const T* src, ulong count) {
        assert( data != NULL );
        assert( src != NULL );
        assert( count > 0 );
        if (count > 0)
            memcpy(data, src, sizeof(T)*count);
    }
    static void init(T* data, ulong size, const T* src, ulong count) {
        assert( data != NULL );
        assert( size > 0 );
        assert( src != NULL );
        assert( count > 0 );
        if (count > size)
            count = size;
        if (count > 0)
            memcpy(data, src, sizeof(T)*count);
    }
    static void initTailSafe(T* data, ulong oldSize, ulong newSize) {
        assert( data != NULL );
        assert( oldSize >= 0 );
        assert( newSize >= 0 );
        if (newSize > oldSize)
            memset(data+oldSize, 0, (newSize-oldSize)*sizeof(T));
    }
    static void initTailFast(T*, ulong, ulong)
        { }
    static void initcopy(T* item, const T* src) {
        assert( item != NULL );
        assert( src != NULL );
        memcpy(item, src, sizeof(T));
    }
    static void copy(T* dest, const T* src, ulong size) {
        assert( dest != NULL );
        assert( src != NULL );
        assert( size > 0 );
        if (size > 0)
            memcpy(dest, src, sizeof(T)*size);
    }
    static void unInit(T*, ulong)
        { }
    static void unInitTail(T*, ulong, ulong)
        { }
};
/** \endcond */

///////////////////////////////////////////////////////////////////////////////

// Implementation
/** \cond impl */
namespace impl
{
    template<class T>
    struct CompareT {
        typedef typename RemoveConst<T>::Type Item;

        struct Pod {
            int operator()(Item a, Item b) const
                { return (a < b ? -1 : (a == b ? 0 : 1)); }
        };
        struct Mem {
            int operator()(const Item& a, const Item& b) const
                { return memcmp(&a, &b, sizeof(Item)); }
        };
        struct Obj {
            int operator()(const Item& a, const Item& b) const
                { return a.compare(b); }
        };
        typedef typename StaticIf<IsPodType<Item>::value,Pod,Mem>::Type ComparePodMem;
        typedef typename StaticIf<IsEvoItem<Item>::value,Obj,ComparePodMem>::Type Compare;

        struct PodR {
            int operator()(Item a, Item b) const
                { return (b < a ? -1 : (b == a ? 0 : 1)); }
        };
        struct MemR {
            int operator()(const Item& a, const Item& b) const
                { return memcmp(&b, &a, sizeof(Item)); }
        };
        struct ObjR {
            int operator()(const Item& a, const Item& b) const
                { return b.compare(a); }
        };
        typedef typename StaticIf<IsPodType<Item>::value,PodR,MemR>::Type ComparePodMemR;
        typedef typename StaticIf<IsEvoItem<Item>::value,ObjR,ComparePodMemR>::Type CompareR;

        struct CompareI {
            STATIC_ASSERT(IsEvoItem<Item>::value, CompareI_requires_EvoItem_type);
            int operator()(const Item& a, const Item& b) const
                { return a.comparei(b); }
        };
        struct CompareIR {
            STATIC_ASSERT(IsEvoItem<Item>::value, CompareIR_requires_EvoItem_type);
            int operator()(const Item& a, const Item& b) const
                { return b.comparei(a); }
        };
    };
}
/** \endcond */

/** Comparison object used with containers that order/sort items.
 - This produces an ascending sort order
 - POD and Byte-Copy types are compared directly
 - Normal class types are compared by calling T method: a.compare(b)
   - Evo types like String implement this compare() method
   - See IsNormalType, TypeId
   .
 - See: CompareR, CompareI, CompareIR, MapList
 .
 \tparam  T  Type for comparison
 */
template<class T> struct Compare : public evo::impl::CompareT<T>::Compare {
    /** Item type for comparison. */
    typedef typename RemoveConst<T>::Type Item;

    /** Comparison function pointer type. */
    typedef int (*Func)(const Item&,const Item&);

    #ifdef DOXYGEN
    /** Comparison method.
     \param  a  Item a to compare (left side)
     \param  b  Item b to compare (right side)
     \return    Comparison result: 0:equal, negative:(a < b), positive:(a > b)
     */
    int operator()(const Item& a, const Item& b) const;
    #endif
};

/** Comparison object used with containers that order/sort items (reverse).
 - This produces a descending (reversed) sort order
 - POD and Byte-Copy types are compared directly
 - Normal class types are compared by calling T method: a.compare(b)
   - Evo types like String implement this compare() method
   - See IsNormalType, TypeId
   .
 - See: Compare, CompareI, CompareIR, MapList
 .
 \tparam  T  Type for comparison
 */
template<class T> struct CompareR : public evo::impl::CompareT<T>::CompareR {
    /** Item type for comparison. */
    typedef typename RemoveConst<T>::Type Item;

    /** Comparison function pointer type. */
    typedef int (*Func)(const Item&,const Item&);

    #ifdef DOXYGEN
    /** Comparison method.
     \param  a  Item a to compare (left side)
     \param  b  Item b to compare (right side)
     \return    Comparison result: 0:equal, negative:(a > b), positive:(a < b)
     */
    int operator()(const Item& a, const Item& b) const;
    #endif
};

/** Comparison object used with containers that order/sort items (case-insensitive).
 - This produces a case-insensitive ascending sort order
 - Not to be used with POD and Byte-Copy types -- will get compiler error
 - Normal class types are compared by calling T method: a.comparei(b)
   - Evo types like String implement this comparei() method
   - See IsNormalType, TypeId
   .
 - See: MapList
 .
 \tparam  T  Type for comparison
 */
template<class T> struct CompareI : public evo::impl::CompareT<T>::CompareI {
    /** Item type for comparison. */
    typedef typename RemoveConst<T>::Type Item;

    /** Comparison function pointer type. */
    typedef int (*Func)(const Item&,const Item&);

    #ifdef DOXYGEN
    /** Comparison method.
     \param  a  Item a to compare (left side)
     \param  b  Item b to compare (right side)
     \return    Comparison result: 0:equal, negative:(a < b), positive:(a > b)
     */
    int operator()(const Item& a, const Item& b) const;
    #endif
};

/** Comparison object used with containers that order/sort items (case-insensitive, reverse).
 - This produces a case-insensitive descending (reversed) sort order
 - Not to be used with POD and Byte-Copy types -- will get compiler error
 - Normal class types are compared by calling T method: a.comparei(b)
   - Evo types like String implement this comparei() method
   - See IsNormalType, TypeId
   .
 - See: MapList
 .
 \tparam  T  Type for comparison
 */
template<class T> struct CompareIR : public evo::impl::CompareT<T>::CompareIR {
    /** Item type for comparison. */
    typedef typename RemoveConst<T>::Type Item;

    /** Comparison function pointer type. */
    typedef int (*Func)(const Item&,const Item&);

    #ifdef DOXYGEN
    /** Comparison method.
     \param  a  Item a to compare (left side)
     \param  b  Item b to compare (right side)
     \return    Comparison result: 0:equal, negative:(a > b), positive:(a < b)
     */
    int operator()(const Item& a, const Item& b) const;
    #endif
};

///////////////////////////////////////////////////////////////////////////////

// Implementation
/** \cond impl */
namespace impl
{
    template<class T, class H>
    struct HashT {
        typedef typename RemoveConst<T>::Type Item;
        struct Pod {
            ulong operator()(Item key, ulong seed=0) const
                { return H::hash_pod(key, seed); }
        };
        struct Mem {
            ulong operator()(const Item& key, ulong seed=0) const
                { return H::hash(&key, sizeof(key), seed); }
        };
        struct Obj {
            ulong operator()(const Item& key, ulong seed=0) const
                { return key.hash(seed); }
        };
        typedef EVO_TYPE_SELECT(Item,Pod,Mem,Obj) Hash;
    };
}
/** \endcond */

/** %Hash object used with containers that hash items.
 - POD and Byte-Copy types are hashed directly using internal SpookyHash V2 library
   - SpookyHash reference: http://burtleburtle.net/bob/hash/spooky.html
 - Normal class types are hashed by calling T method: key.hash(seed)
   - Evo types like String implement this hash() method
   - See IsNormalType, TypeId
   .
 - See: MapHash
 .
 \tparam  T  Type to hash
 */
template<class T> struct Hash : public evo::impl::HashT<T,SpookyHash>::Hash {
    /** Item type to hash. */
    typedef typename RemoveConst<T>::Type Item;

    /** Hash function pointer type. */
    typedef ulong (*Func)(const Item& key, ulong seed);

    #ifdef DOXYGEN
    /** Hash function method.
     \param  key   Key to hash
     \param  seed  Seed to start with, 0 for none, or can be from hashing another value
     \return       Hash function result
     */
    ulong operator()(const Item& key, ulong seed=0) const;
    #endif
};

///////////////////////////////////////////////////////////////////////////////
//@}
} // Namespace: evo
#endif
