// Evo C++ Library
/* Copyright 2019 Justin Crowell
Distributed under the BSD 2-Clause License -- see included file LICENSE.txt for details.
*/
///////////////////////////////////////////////////////////////////////////////
/** \file container.h Evo container foundation types and macros. */
#pragma once
#ifndef INCL_evo_impl_container_h
#define INCL_evo_impl_container_h

#include <memory.h>
#include <new>

#include "../meta.h"
#include "hash.h"

#if defined(EVO_CPP11)
    #include <initializer_list> // std::initializer_list
    #include <utility>          // std::move
#endif

namespace evo {
/** \addtogroup EvoContainers
Evo containers
*/
//@{

///////////////////////////////////////////////////////////////////////////////

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

#if (defined(EVO_EXCEPTIONS_ENABLED) && !defined(EVO_EXCEPTIONS_NOGUARDS)) || defined(DOXYGEN)
    /** Start exception guard (try block). */
    #define EVO_EXCEPTION_GUARD_START try {

    /** End exception guard, catch and abort(). */
    #define EVO_EXCEPTION_GUARD_END } catch (...) { abort(); }
#else
    #define EVO_EXCEPTION_GUARD_START
    #define EVO_EXCEPTION_GUARD_END
#endif

///////////////////////////////////////////////////////////////////////////////

/** \cond impl */
/** %Allocator base class. */
struct Allocator {
    static const bool SHARED = true;    ///< This allocator can be shared among multiple containers

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
/** \endcond */

///////////////////////////////////////////////////////////////////////////////

/** Optimized container size and capacity calculation. */
class Capacity {
public:
    static const ulong INIT      = 64;          ///< Default initial size
    static const ulong THRESHOLD = 134217728;   ///< Growth threshold (max size increase) (128*1024*1024)

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

/** Base data type for optimizated data helpers.
 \tparam  T  Data type
*/
template<class T>
struct DataType {
    typedef typename RemoveConst<T>::Type Item;  ///< %Item type (const removed)
};

///////////////////////////////////////////////////////////////////////////////

/** Optimized data initialization and uninitialization helpers.
 - Operations are specialized and optimized for POD and ByteCopy types where possible
 - This is used by Evo containers
 .
 \tparam  T  Data type to use
 \tparam  U  Inferred from T
*/
template<class T, class U=typename TypeId::Get<T>::Id>
struct DataInit : public DataType<T> {
#if defined(EVO_OLDCC)
    typedef typename DataType<T>::Item Item;
#else
    using typename DataType<T>::Item;
#endif

    /** Initialize data using default constructor.
     - Same as init(T*,ulong) except will initialize new POD values to 0, so is safer but slower for POD types
     .
     \param  data  Data pointer - must be valid
     \param  size  Data size - must be positive
    */
    static void init_safe(Item* data, ulong size=1) {
        assert( data != NULL );
        assert( size > 0 );
        EVO_EXCEPTION_GUARD_START
        for (ulong i=0; i<size; ++i)
            new(&data[i]) Item();
        EVO_EXCEPTION_GUARD_END
    }

    /** Initialize data using copy constructor and default constructor.
     - Same as init(Item*,ulong,const Item*,ulong) except will initialize new POD values to 0, so is safer but slower for POD types
     .
     \param  data   Data pointer - must be valid
     \param  size   Data size - must be positive
     \param  src    Source data to copy - must be valid
     \param  count  Number of items to copy - must be positive
    */
    static void init_safe(Item* data, ulong size, const Item* src, ulong count) {
        assert( data != NULL );
        assert( size > 0 );
        assert( src != NULL );
        assert( count > 0 );
        if (count > size)
            count = size;
        ulong i = 0;
        EVO_EXCEPTION_GUARD_START
        for (; i<count; ++i)
            new(&data[i]) Item(src[i]);
        for (; i<size; ++i)
            new(&data[i]) Item();
        EVO_EXCEPTION_GUARD_END
    }

    /** Initialize data using default constructor.
     - Same as init_safe() except will not initialize new POD values, so is faster for POD types
     .
     \param  data  Data pointer - must be valid
     \param  size  Data size - must be positive
    */
    static void init(Item* data, ulong size=1) {
        assert( data != NULL );
        assert( size > 0 );
        EVO_EXCEPTION_GUARD_START
        for (ulong i=0; i<size; ++i)
            new(&data[i]) Item();
        EVO_EXCEPTION_GUARD_END
    }

    /** Initialize data using copy constructor.
     \param  data   Data pointer - must be valid
     \param  src    Source data to copy - must be valid
     \param  count  Number of items to copy - must be positive
    */
    static void init(Item* data, const Item* src, ulong count) {
        assert( data != NULL );
        assert( src != NULL );
        assert( count > 0 );
        EVO_EXCEPTION_GUARD_START
        for (ulong i=0; i<count; ++i)
            new(&data[i]) Item(src[i]);
        EVO_EXCEPTION_GUARD_END
    }

    /** Initialize data using copy constructor and default constructor.
     \param  data   Data pointer - must be valid
     \param  size   Data size - must be positive
     \param  src    Source data to copy - must be valid
     \param  count  Number of items to copy - must be positive
    */
    static void init(Item* data, ulong size, const Item* src, ulong count) {
        assert( data != NULL );
        assert( size > 0 );
        assert( src != NULL );
        assert( count > 0 );
        if (count > size)
            count = size;
        ulong i = 0;
        EVO_EXCEPTION_GUARD_START
        for (; i<count; ++i)
            new(&data[i]) Item(src[i]);
        for (; i<size; ++i)
            new(&data[i]) Item();
        EVO_EXCEPTION_GUARD_END
    }

    /** Initialize new tail data (default constructor).
     - This is usefull for growing an array and initializes new data when newSize is greater than oldSize
     - Same as init_tail_fast() except will initialize new POD values to 0, so is safer but slower for POD types
     .
     \param  data     Data pointer - must be valid
     \param  oldSize  Old data size that is already initialized
     \param  newSize  New data size that will need data initialized if larger
    */
    static void init_tail_safe(Item* data, ulong oldSize, ulong newSize) {
        assert( data != NULL );
        assert( oldSize >= 0 );
        assert( newSize >= 0 );
        EVO_EXCEPTION_GUARD_START
        for (ulong i = oldSize; i < newSize; ++i)
            new(&data[i]) Item();
        EVO_EXCEPTION_GUARD_END
    }

    /** Initialize new tail data (default constructor).
     - This is usefull for growing an array and initializes new data when newSize is greater than oldSize
     - Same as init_tail_safe() except will not initialize new POD values, so is faster for POD types
     .
     \param  data     Data pointer - must be valid
     \param  oldSize  Old data size that is already initialized
     \param  newSize  New data size that will need data initialized if larger
    */
    static void init_tail_fast(Item* data, ulong oldSize, ulong newSize) {
        assert( data != NULL );
        assert( oldSize >= 0 );
        assert( newSize >= 0 );
        EVO_EXCEPTION_GUARD_START
        for (ulong i=oldSize; i<newSize; ++i)
            new(&data[i]) Item();
        EVO_EXCEPTION_GUARD_END
    }

    /** Initialize new item as copy of src (copy constructor).
     \param  item  Item to initialize
     \param  src   Source to copy
    */
    static void initcopy(Item* item, const Item* src) {
        assert( item != NULL );
        assert( src != NULL );
        EVO_EXCEPTION_GUARD_START
        new(*item) Item(*src);
        EVO_EXCEPTION_GUARD_END
    }

    /** Copy already initialized data (assignment operator).
     \param  dest  Destination data pointer - must be valid
     \param  src   Source data pointer - must be valid
     \param  size  Copy size - must be positive
    */
    static void copy(Item* dest, const Item* src, ulong size) {
        assert( dest != NULL );
        assert( src != NULL );
        assert( size > 0 );
        EVO_EXCEPTION_GUARD_START
        for (ulong i=0; i<size; ++i)
            dest[i] = src[i];
        EVO_EXCEPTION_GUARD_END
    }

    /** Uninitialize data (destructor).
     \param  data  Data pointer - must be valid
     \param  size  Size of data
    */
    static void uninit(Item* data, ulong size) {
        assert( data != NULL );
        assert( size > 0 );
        EVO_EXCEPTION_GUARD_START
        while (size > 0)
            data[--size].~T();
        EVO_EXCEPTION_GUARD_END
    }

    /** Uninitialize and free array of pointers (destructor).
     - Skips null pointers
     .
     \param  data  Data pointer - must be valid
     \param  size  Size of data
    */
    static void uninit_free_ptr(Item** data, ulong size) {
        assert( data != NULL );
        assert( size > 0 );
        EVO_EXCEPTION_GUARD_START
        for (; size > 0; ++data, --size)
            if (*data != NULL) {
                (**data).~T();
                EVO_IMPL_CONTAINER_ITEM_FREE(*data);
            }
        EVO_EXCEPTION_GUARD_END
    }

    /** Uninitialize old tail data (destructor).
     - This is usefull for shrinking an array and uninitializing old data when newSize is less than oldSize
     .
     \param  data     Data pointer - must be valid
     \param  oldSize  Old data size that is already initialized
     \param  newSize  New data size that will need data uninitialized if smaller
    */
    static void uninit_tail(Item* data, ulong oldSize, ulong newSize) {
        assert( data != NULL );
        assert( oldSize >= 0 );
        assert( newSize >= 0 );
        EVO_EXCEPTION_GUARD_START
        while (oldSize > newSize)
            data[--oldSize].~T();
        EVO_EXCEPTION_GUARD_END
    }
};

/** \cond impl */
template<class T> struct DataInit<T, TypeId::ByteCopy> : public DataType<T> {
#if defined(EVO_OLDCC)
    typedef typename DataType<T>::Item Item;
#else
    using typename DataType<T>::Item;
#endif
    static void init_safe(T* data, ulong size=1) {
        assert( data != NULL );
        assert( size > 0 );
        EVO_EXCEPTION_GUARD_START
        for (ulong i=0; i<size; ++i)
            new(&data[i]) T();
        EVO_EXCEPTION_GUARD_END
    }
    static void init_safe(T* data, ulong size, const T* src, ulong count) {
        assert( data != NULL );
        assert( size > 0 );
        assert( src != NULL );
        assert( count > 0 );
        if (count > size)
            count = size;
        if (count > 0)
            memcpy(data, src, sizeof(T)*count);
        EVO_EXCEPTION_GUARD_START
        for (ulong i=count; i<size; ++i)
            new(&data[i]) T();
        EVO_EXCEPTION_GUARD_END
    }
    static void init(T* data, ulong size=1) {
        assert( data != NULL );
        assert( size > 0 );
        EVO_EXCEPTION_GUARD_START
        for (ulong i=0; i<size; ++i)
            new(&data[i]) T();
        EVO_EXCEPTION_GUARD_END
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
        EVO_EXCEPTION_GUARD_START
        for (ulong i=count; i<size; ++i)
            new(&data[i]) T();
        EVO_EXCEPTION_GUARD_END
    }
    static void init_tail_safe(T* data, ulong oldSize, ulong newSize) {
        assert( data != NULL );
        assert( oldSize >= 0 );
        assert( newSize >= 0 );
        EVO_EXCEPTION_GUARD_START
        for (ulong i=oldSize; i<newSize; ++i)
            new(&data[i]) T();
        EVO_EXCEPTION_GUARD_END
    }
    static void init_tail_fast(T* data, ulong oldSize, ulong newSize) {
        assert( data != NULL );
        assert( oldSize >= 0 );
        assert( newSize >= 0 );
        EVO_EXCEPTION_GUARD_START
        for (ulong i=oldSize; i<newSize; ++i)
            new(&data[i]) T();
        EVO_EXCEPTION_GUARD_END
    }
    static void initcopy(T* item, const T* src) {
        assert( item != NULL );
        assert( src != NULL );
        EVO_EXCEPTION_GUARD_START
        new(*item) T(*src);
        EVO_EXCEPTION_GUARD_END
    }
    static void copy(T* dest, const T* src, ulong size) {
        assert( dest != NULL );
        assert( src != NULL );
        assert( size > 0 );
        if (size > 0)
            memcpy(dest, src, sizeof(T)*size);
    }
    static void uninit(T* data, ulong size) {
        assert( data != NULL );
        assert( size > 0 );
        EVO_EXCEPTION_GUARD_START
        while (size > 0)
            data[--size].~T();
        EVO_EXCEPTION_GUARD_END
    }
    static void uninit_tail(T* data, ulong oldSize, ulong newSize) {
        assert( data != NULL );
        assert( oldSize >= 0 );
        assert( newSize >= 0 );
        EVO_EXCEPTION_GUARD_START
        while (oldSize > newSize)
            data[--oldSize].~T();
        EVO_EXCEPTION_GUARD_END
    }
};
template<class T> struct DataInit<T, TypeId::Pod> : public DataType<T> {
#if defined(EVO_OLDCC)
    typedef typename DataType<T>::Item Item;
#else
    using typename DataType<T>::Item;
#endif
    static void init_safe(T* data, ulong size=1)
        { memset(data, 0, size*sizeof(T)); }
    static void init_safe(T* data, ulong size, const T* src, ulong count) {
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
    static void init_tail_safe(T* data, ulong oldSize, ulong newSize) {
        assert( data != NULL );
        assert( oldSize >= 0 );
        assert( newSize >= 0 );
        if (newSize > oldSize)
            memset(data+oldSize, 0, (newSize-oldSize)*sizeof(T));
    }
    static void init_tail_fast(T*, ulong, ulong)
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
    static void uninit(T*, ulong)
        { }
    static void uninit_tail(T*, ulong, ulong)
        { }
};
/** \endcond */

///////////////////////////////////////////////////////////////////////////////

/** Optimized data copy helpers.
 - Operations are specialized and optimized for POD and ByteCopy types where possible
 - This is used by Evo containers
 .
 \tparam  T  Data type to use
 \tparam  U  Inferred from T
*/
template<class T, class U=typename TypeId::Get<T>::Id>
struct DataCopy : public DataType<T> {
#if defined(EVO_OLDCC)
    typedef typename DataType<T>::Item Item;
#else
    using typename DataType<T>::Item;
#endif

    /** Most efficient type for passing as parameter (const-reference or POD value). */
    typedef typename AddConst<T>::Type& PassType;

    /** Set value to default.
     - For POD types this sets to 0
     - For all other types this copies from a default-constructed object
     .
     \param  val  Value to initialize
    */
    static void set_default(T& val)
        { static const T DEFAULT; val = DEFAULT; }

    /** Set new POD value to default value (0).
     - For POD types this sets to 0
     - For all other types this does nothing
     .
     \param  val  Value to initialize
    */
#if defined(DOXYGEN)
    static void set_default_pod(T& val)
#else
    static void set_default_pod(T&)
#endif
        { }
};

/** \cond impl */
template<class T> struct DataCopy<T, TypeId::Pod> : public DataType<T> {
#if defined(EVO_OLDCC)
    typedef typename DataType<T>::Item Item;
#else
    using typename DataType<T>::Item;
#endif

#if EVO_UNIT_TEST_ITEMARG_REF
    typedef typename AddConst<Item>::Type& PassType;    // Always pass by-reference so unit tests can detect constructor calls
#else
    typedef Item PassType;
#endif

    static void set_default(Item& val)
        { memset(&val, 0, sizeof(Item)); }
    static void set_default_pod(Item& val)
        { memset(&val, 0, sizeof(Item)); }
};
/** \endcond */

///////////////////////////////////////////////////////////////////////////////

/** Optimized data fill helpers.
 - Operations are specialized and optimized for POD and ByteCopy types where possible
 - This is used by Evo containers
 .
 \tparam  T  Data type to use
 \tparam  U  Inferred from T
*/
template<class T, class U=typename TypeId::GetFill<T>::Id>
struct DataFill : public DataType<T> {
#if defined(EVO_OLDCC)
    typedef typename DataType<T>::Item Item;
#else
    using typename DataType<T>::Item;
#endif

    /** Fill with copies of given item.
     - This is optimized to use item operator=(), memcpy(), or memset() as applicable
     .
     \param  dest   Destination data pointer to fill - must be valid
     \param  size   Copies to fill with as item count - must be positive
     \param  value  Value to fill with
    */
    static void fill(T* dest, ulong size, const T& value) {
        EVO_EXCEPTION_GUARD_START
        for (T* end = dest + size; dest < end; ++dest)
            *dest = value;
        EVO_EXCEPTION_GUARD_END
    }

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
    static void fillends(T* dest, ulong size, const T& value, ulong start, ulong end) {
        assert( sizeof(T) > 1 );
        assert( dest != NULL );
        assert( start < end );
        EVO_EXCEPTION_GUARD_START
        while (start > 0)
            dest[--start] = value;
        for (; end < size; ++end)
            dest[end] = value;
        EVO_EXCEPTION_GUARD_END
    }
};

/** \cond impl */
template<class T> struct DataFill<T, TypeId::ByteCopy> : public DataType<T> {
#if defined(EVO_OLDCC)
    typedef typename DataType<T>::Item Item;
#else
    using typename DataType<T>::Item;
#endif
    typedef typename DataCopy<T>::PassType PassType;
    static void fill(T* dest, ulong size, PassType value) {
        assert( sizeof(T) > 1 );
        for (T* end = dest + size; dest < end; ++dest)
            memcpy(dest, &value, sizeof(T));
    }
    static void fillends(T* dest, ulong size, PassType value, ulong start, ulong end) {
        assert( sizeof(T) > 1 );
        assert( dest != NULL );
        assert( start < end );
        while (start > 0)
            memcpy(dest + (--start), &value, sizeof(T));
        for (; end < size; ++end)
            memcpy(dest + end, &value, sizeof(T));
    }
};
template<class T> struct DataFill<T, TypeId::Pod> : public DataType<T> {
#if defined(EVO_OLDCC)
    typedef typename DataType<T>::Item Item;
#else
    using typename DataType<T>::Item;
#endif
    typedef typename DataCopy<T>::PassType PassType;
    static void fill(T* dest, ulong size, PassType value) {
        assert( sizeof(T) == 1 );
        uchar cval;
        memcpy(&cval, &value, 1);
        memset(dest, (int)cval, size);
    }
    static void fillends(T* dest, ulong size, PassType value, ulong start, ulong end) {
        assert( sizeof(T) == 1 );
        assert( dest != NULL );
        assert( start < end );
        uchar cval;
        memcpy(&cval, &value, 1);
        if (start > 0)
            memset(dest, cval, start);
        if (end < size)
            memset(dest+end, cval, size-end);
    }
};
/** \endcond */

///////////////////////////////////////////////////////////////////////////////

/** Data equality helper.
 - This compares POD types directly, or calls operator==() for non-POD types
 - This is used by Evo containers to compare items for equality
 .
 \tparam  T  Data type to compare -- must be a POD type or implement operator==()
 \tparam  B  Inferred from `T`
*/
template<class T, bool B=IsPodType<T>::value>
struct DataEqual {
    /** %Compare array data for equality.
     - This compares each item with operator==()
     .
     \param  data1  Data to compare
     \param  data2  Data to compare to
     \param  size   Size to compare as item count
     \return        Whether equals, true if `size=0`
    */
    static bool equal(const T* data1, const T* data2, ulong size) {
        assert( data1 != NULL || size == 0 );
        assert( data2 != NULL || size == 0 );
        if (data1 != data2)
            for (ulong i = 0; i < size; ++i)
                if (!(data1[i] == data2[i]))
                    return false;
        return true;
    }
};

/** \cond impl */
template<class T> struct DataEqual<T,true> {
    static bool equal(const T* data1, const T* data2, ulong size) {
        assert( data1 != NULL || size == 0 );
        assert( data2 != NULL || size == 0 );
        return (size == 0 || memcmp(data1, data2, size * sizeof(T)) == 0);
    }
};
/** \endcond */

///////////////////////////////////////////////////////////////////////////////

/** Data comparison helpers.
 - This compares POD types directly, or calls T compare() method for non-POD types
 - This is used by Evo containers to compare items
 .
 \tparam  T   Data type to compare -- must be either a POD type or implement `int compare(const T& item) const`
 \tparam  B1  Inferred from T
 \tparam  B2  Inferred from T
*/
template<class T, bool B1=IsPodType<T>::value, bool B2=sizeof(T) == 1>
struct DataCompare : public DataType<T> {
#if defined(EVO_OLDCC)
    typedef typename DataType<T>::Item Item;
#else
    using typename DataType<T>::Item;
#endif

    /** %Compare data.
     - This compares POD types directly, or calls T compare() method for non-POD types
     .
     \param  data1  Data to compare
     \param  size1  Data size to compare as item count
     \param  data2  Data to compare to
     \param  size2  Data size to compare to as item count
     \return        Result (<0 if data1 is less, 0 if equal, >0 if data1 is greater)
    */
    static int compare(const T* data1, ulong size1, const T* data2, ulong size2) {
        assert( data1 != NULL || size1 == 0 );
        assert( data2 != NULL || size2 == 0 );
        const ulong size = (size1 < size2 ? size1 : size2);
        int result;
        for (ulong i = 0; i < size; ++i) {
            result = data1[i].compare(data2[i]);
            if (result != 0)
                return result;
        }
        if (size1 < size2)
            result = -1;
        else if (size2 < size1)
            result = 1;
        else
            result = 0;
        return result;
    }

    /** %Compare items.
     - This compares POD types directly, or calls T compare() method for non-POD types
     .
     \param  item1  Item1 to compare
     \param  item2  Item2 to compare
     \return        Result (<0 if item1 is less, 0 if equal, >0 if item1 is greater)
    */
    static int compare(const T& item1, const T& item2)
        { return item1.compare(item2); }
};

/** \cond impl */
template<class T> struct DataCompare<T,true,false> : public DataType<T> {
#if defined(EVO_OLDCC)
    typedef typename DataType<T>::Item Item;
#else
    using typename DataType<T>::Item;
#endif

    static int compare(const T* data1, ulong size1, const T* data2, ulong size2) {
        assert( data1 != NULL || size1 == 0 );
        assert( data2 != NULL || size2 == 0 );
        int result;
        if (data1 == data2) {
            if (size1 == size2)
                result = 0;
            else
                result = (size1 < size2 ? -1 : 1);
        } else {
            const T* end = data1 + (size1 < size2 ? size1 : size2);
            for (result = 0; data1 < end; ++data1, ++data2) {
                if (*data1 < *data2) {
                    result = -1;
                    break;
                } else if (*data1 > *data2) {
                    result = 1;
                    break;
                }
            }
            if (result == 0 && (size1 != size2 || data1 != end))
                result = (size1 < size2 ? -1 : 1);
        }
        return result;
    }

    static int compare(T item1, T item2)
        { return (item1 < item2 ? -1 : (item1 == item2 ? 0 : 1)); }
};

template<class T> struct DataCompare<T,true,true> : public DataType<T> {
#if defined(EVO_OLDCC)
    typedef typename DataType<T>::Item Item;
#else
    using typename DataType<T>::Item;
#endif

    static int compare(const T* data1, ulong size1, const T* data2, ulong size2) {
        assert( data1 != NULL || size1 == 0 );
        assert( data2 != NULL || size2 == 0 );
        int result;
        if (data1 == data2) {
            if (size1 == size2)
                result = 0;
            else
                result = (size1 < size2 ? -1 : 1);
        } else if (size1 == size2) {
            result = memcmp(data1, data2, size1);
        } else if (size1 < size2) {
            result = memcmp(data1, data2, size1);
            if (result == 0)
                result = -1;
        } else {
            result = memcmp(data1, data2, size2);
            if (result == 0)
                result = 1;
        }
        return result;
    }

    static int compare(T item1, T item2)
        { return (item1 < item2 ? -1 : (item1 == item2 ? 0 : 1)); }
};
/** \endcond */

///////////////////////////////////////////////////////////////////////////////

/** Optimized data hash helpers.
 - Operations are specialized and optimized for POD types
 - This is used by Evo items and containers
 .
 \tparam  T  Data type to use
 \tparam  H  Hashing class to use for POD data, ignored with non-POD types
 \tparam  B  Inferred from T
*/
template<class T, class H=SpookyHash, bool B=IsPodType<T>::value>
struct DataHash : public DataType<T> {
#if defined(EVO_OLDCC)
    typedef typename DataType<T>::Item Item;
#else
    using typename DataType<T>::Item;
#endif
    typedef H HashType;                             ///< %Hash class type

    /** Compute hash value from data.
     - This uses T::hash() method to chain-hash each item, for POD types this uses `H::hash()` on the whole array
     .
     \param  data  Data to use
     \param  size  Data size as item count
     \param  seed  Seed value for chaining from a previous hash, 0 if none
     \return       Hash value
    */
    static ulong hash(const T* data, ulong size, ulong seed=0) {
        for (ulong i = 0; i < size; ++size)
            seed = data[i].hash(seed);
        return seed;
    }

    /** Compute hash value from data.
     - This uses T::hash() method, for POD types this uses `H::hash()`
     .
     \param  data  Data to use
     \param  seed  Seed value for chaining from a previous hash, 0 if none
     \return       Hash value
    */
    static ulong hash(const T& data, ulong seed=0)
        { return data.hash(seed); }
};

/** \cond impl */
template<class T, class H> struct DataHash<T,H,true> : public DataType<T> {
#if defined(EVO_OLDCC)
    typedef typename DataType<T>::Item Item;
#else
    using typename DataType<T>::Item;
#endif
    typedef H HashType;
    static ulong hash(const T* data, ulong size, ulong seed=0)
        { return H::hash(data, size*sizeof(T), seed); }
    static ulong hash(const T& data, ulong seed=0)
        { return H::hash(&data, sizeof(T), seed); }
};
/** \endcond */

///////////////////////////////////////////////////////////////////////////////

/** Base type for comparison types.
 - Implement to create a custom comparison type
*/
template<class T>
struct CompareBase : public DataType<T> {
#if defined(EVO_OLDCC)
    typedef typename DataType<T>::Item Item;
#else
    using typename DataType<T>::Item;
#endif
    typedef typename DataCopy<T>::PassType PassItem;    ///< Best type for passing Item, either `const Item&` (by reference) or `Item` (by value) for POD types

    /** Comparison method.
     \param  a  Item a to compare (left side)
     \param  b  Item b to compare (right side)
     \return    Comparison result: 0:equal, negative:(a < b), positive:(a > b)
    */
    virtual int operator()(PassItem a, PassItem b) const = 0;
};

/** Comparison object used with containers that order/sort items.
 - This produces an ascending sort order
 - POD and Byte-Copy types are compared directly
 - Normal class types are compared by calling method: T::compare()
   - Evo types like String implement this compare() method
   - See IsNormalType, TypeId
   .
 - See: CompareR, CompareI, CompareIR
 .
 \tparam  T  Type for comparison
*/
template<class T>
struct Compare : public CompareBase<T> {
#if defined(EVO_OLDCC)
    typedef typename DataType<T>::Item Item;
    typedef typename CompareBase<T>::PassItem PassItem;
#else
    using typename DataType<T>::Item;
    using typename CompareBase<T>::PassItem;
#endif

    int operator()(PassItem a, PassItem b) const
        { return DataCompare<T>::compare(a, b); }
};

/** Comparison object used with containers that order/sort items (reverse).
 - This produces a descending (reversed) sort order
 - POD and Byte-Copy types are compared directly
 - Normal class types are compared by calling method: T::compare()
   - Evo types like String implement this compare() method
   - See IsNormalType, TypeId
   .
 - See: Compare, CompareI, CompareIR
 .
 \tparam  T  Type for comparison
*/
template<class T>
struct CompareR : public CompareBase<T> {
#if defined(EVO_OLDCC)
    typedef typename DataType<T>::Item Item;
    typedef typename CompareBase<T>::PassItem PassItem;
#else
    using typename DataType<T>::Item;
    using typename CompareBase<T>::PassItem;
#endif

    int operator()(PassItem a, PassItem b) const
        { return DataCompare<T>::compare(b, a); }
};

/** Comparison object used with containers that order/sort items (case-insensitive).
 - This produces a case-insensitive ascending sort order
 - Only works on String and SubString types that implement `comparei()`, otherwise gives a compiler error
 - Objects are compared by calling method: T::comparei()
 - See: CompareIR, Compare, CompareR
 .
 \tparam  T  Type for comparison
*/
template<class T>
struct CompareI : public CompareBase<T> {
#if defined(EVO_OLDCC)
    typedef typename DataType<T>::Item Item;
    typedef typename CompareBase<T>::PassItem PassItem;
#else
    using typename DataType<T>::Item;
    using typename CompareBase<T>::PassItem;
#endif

    int operator()(PassItem a, PassItem b) const
        { return a.comparei(b); }
};

/** Comparison object used with containers that order/sort items (case-insensitive, reverse).
 - This produces a case-insensitive descending (reversed) sort order
 - Only works on String and SubString types that implement `comparei()`, otherwise gives a compiler error
 - Objects are compared by calling method: T::comparei()
 - See: CompareI, Compare, CompareR
 .
 \tparam  T  Type for comparison
*/
template<class T>
struct CompareIR : public CompareBase<T> {
#if defined(EVO_OLDCC)
    typedef typename DataType<T>::Item Item;
    typedef typename CompareBase<T>::PassItem PassItem;
#else
    using typename DataType<T>::Item;
    using typename CompareBase<T>::PassItem;
#endif

    int operator()(PassItem a, PassItem b) const
        { return b.comparei(a); }
};

///////////////////////////////////////////////////////////////////////////////

/** %Hash object used with containers that hash items.
 - This inherits an associated comparison type used with hash collisons
 - Hashing is done with the SpookyHash V2 library (included with Evo):
   - SpookyHash reference: http://burtleburtle.net/bob/hash/spooky.html
 - Non-POD types are compared by calling method the `T::hash()`, which should use DataHash to hash the actual data
   - Evo types like String implement this hash() method
   - See IsNormalType, TypeId
   .
 .
 \tparam  T  Type to hash
*/
template<class T>
struct CompareHash : public Compare<T> {
#if defined(EVO_OLDCC)
    typedef typename DataType<T>::Item Item;
    typedef typename CompareBase<T>::PassItem PassItem;
#else
    using typename DataType<T>::Item;
    using typename CompareBase<T>::PassItem;
#endif
    typedef Compare<T> CompareBase;             ///< Base compare type

    using Compare<T>::operator();

    /** Hash function method.
     \param  key   Key to hash
     \param  seed  Seed to start with, 0 for none, or can be from hashing another value
     \return       Hash function result
    */
    ulong hash(PassItem key, ulong seed=0) const
        { return DataHash<T>::hash(key, seed); }
};

///////////////////////////////////////////////////////////////////////////////

/** Get size of fixed-length array.
 - Template params and result are inferred from argument type
 - This is often used with SubStringMapList
 - See also: EVO_FIXED_ARRAY_SIZE()
 .
\par Example:
\code
#include <evo/type.h>
using namespace evo;

int main() {
    const char* STRLIST[] = {
        "one",
        "two",
        "three"
    };

    const uint size = fixed_array_size(STRLIST);  // set to 3
    return 0;
}
\endcode
*/
template<class T, uint sz>
static uint fixed_array_size(T(&)[sz]) {
    return sz;
}

/** \cond impl */
namespace impl {
    template <class T, uint N> char ( &FixedArraySizeHelper(T(&array)[N]) )[N];
}
/** \endcond */

/** Get size of fixed-length array at compile-time.
 - Same as fixed_array_size(), but resolves to a compile-time constant using sizeof()
*/
#define EVO_FIXED_ARRAY_SIZE(ARRAY) (sizeof(evo::impl::FixedArraySizeHelper(ARRAY)))

///////////////////////////////////////////////////////////////////////////////
//@}
}
#endif
