// Evo C++ Library
/* Copyright 2019 Justin Crowell
Distributed under the BSD 2-Clause License -- see included file LICENSE.txt for details.
*/
///////////////////////////////////////////////////////////////////////////////
/** \file iter.h Evo implementation detail: Container iterators. */
#pragma once
#ifndef INCL_evo_impl_iter_h
#define INCL_evo_impl_iter_h

#include "../type.h"

namespace evo {
/** \addtogroup EvoContainers */
//@{

///////////////////////////////////////////////////////////////////////////////

/** Iterator position value. */
enum IteratorPos {
    iterFIRST,          ///< First item iterator position
    iterLAST,           ///< Last item iterator position
    iterEND             ///< End iterator position
};

/** Iterator direction value. */
enum IteratorDir {
    iterNONE,           ///< No iterator direction
    iterFW,             ///< Forward iterator direction
    iterRV              ///< Reverse iterator direction
};

///////////////////////////////////////////////////////////////////////////////

/** Base iterator (used internally).
\par Iterator Types
 - IteratorFw  Forward iterator
 - IteratorBi  Bidirectional (forward and reverse) iterator
 - IteratorRa  Random access (forward, reverse, and random access) iterator
 .
*/
template<class T>
class IteratorBase : public SafeBool< IteratorBase<T> > {
public:
    typedef typename T::Size    Size;               ///< Size type to use
    typedef typename T::IterKey Key;                ///< Iterator key type
    typedef typename StaticIf< IsConst<T>::value,
        const typename T::IterItem,
        typename T::IterItem
    >::Type                     Item;               ///< Iterator item type

    typedef IteratorBase<T> IterBaseType;           ///< Iterator base type for parameter passing

    typedef IteratorBase<const T>                                    Const;         ///< Const iterator type
    typedef IteratorBase<typename RemoveConst<T>::Type>              Mutable;       ///< Mutable iterator type
    typedef typename StaticIf<IsConst<T>::value,Mutable,Const>::Type ToggleConst;   ///< Used for converting between Const/Mutable iterators

    typedef T Target;                               ///< Iterator target type

    /** Constructor. This sets iterator to end.
     \param  obj  Container object to iterate through
    */
    explicit IteratorBase(T& obj) : obj_(&obj), end_(true), data_(NULL)
        { init(); }

    /** Constructor. This initializes iterator with given position data.
     \param  obj   Container object to iterate through
     \param  key   Iterator key to set
     \param  data  Iterator data pointer to set
    */
    IteratorBase(T& obj, const Key& key, Item* data) : obj_(&obj), end_(false), key_(key), data_(data)
        { init(); }

    /** Copy constructor.
     \param  src  Source iterator to copy
    */
    IteratorBase(const IterBaseType& src)
        { memcpy(this, &src, sizeof(src)); init(); }

    /** Copy/Assignment operator to copy from source iterator.
     \param  src  Source iterator to copy
     \return      This
    */
    IterBaseType& operator=(const IterBaseType& src)
        { memcpy(this, &src, sizeof(src)); init(); return *this; }

    /** Assignment operator to copy from source iterator.
     - This allows assigning a mutable iterator to a const iterator
     - This triggers a compiler error on attempt to assign a const iterator to a mutable iterator
     .
     \param  src  Source iterator to copy.
     \return      This.
    */
    IterBaseType& operator=(const ToggleConst& src) {
        STATIC_ASSERT(IsConst<T>::value, ERROR_cannot_assign_const_iterator_to_mutable_iterator);
        operator=((IterBaseType&)src);
        return *this;
    }

    /** Assignment operator to reset with new object, at end.
     \param  obj  Container object to iterate through
     \return      This
    */
    IterBaseType& operator=(T& obj)
        { obj_ = &obj; end_ = true; data_ = NULL; return *this; }

    /** Assignment operator to set position.
     \param  pos  Position to set -- ignored, sets to end
     \return      This
    */
    IterBaseType& operator=(IteratorPos pos) {
        EVO_PARAM_UNUSED(pos);
        end_ = true; data_ = NULL; return *this;
    }

    /** Dereference iterator to get item data reference.
     - Iterator must be valid (not at end position)
     .
     \return  Reference to current item data
    */
    Item& operator*()
        { return *data_; }

    /** Dereference iterator to access data member.
     - Iterator must be valid (not at end position)
     .
     \return  Reference to current item data
    */
    Item* operator->()
        { return data_; }

    /** Check whether iterator is at end (not valid).
     \return  Whether at end
    */
    bool operator!() const
        { return end_; }

    /** Equality operator.
     \param  iter  Iterator to compare
     \return       Whether equal
    */
    bool operator==(const IterBaseType& iter) const
        { return (this == &iter || (end_ == iter.end_ && data_ == iter.data_)); }

    /** Inequality operator.
     \param  iter  Iterator to compare
     \return       Whether not equal
    */
    bool operator!=(const IterBaseType& iter) const
        { return (this != &iter && (end_ != iter.end_ || data_ != iter.data_)); }

    /** Check equality with another iterator.
     \param  oth  Other iterator to check against.
     \return      Whether equal.
    */
    template<typename T_>
    bool operator==(const IteratorBase<T_>& oth) const
        { return IterBaseType::operator==((const IterBaseType&)oth); }

    /** Check inequality with another iterator.
     \param  oth  Other iterator to check against.
     \return      Whether inequal.
    */
    template<typename T_>
    bool operator!=(const IteratorBase<T_>& oth) const
        { return IterBaseType::operator!=((const IterBaseType&)oth); }

    /** \cond impl */
    /** Get parent object (used internally). */
    T& getParent() const
        { return *obj_; }

    /** Get end flag (used internally). */
    bool getEnd() const
        { return end_; }

    /** Get key (used internally). */
    const Key& getKey() const
        { return key_; }

    /** Get key (used internally). */
    Key& getKey()
        { return key_; }

    /** Set new key/pointer (used internally) */
    void set(const Key& key, const Item* data) {
        key_  = key;
        data_ = (Item*)data;
        end_  = (data_ == NULL);
    }

    /** Set item pointer (used internally). */
    void setData(const Item* item)
        { data_ = (Item*)item; }
    /** \endcond */

    /** Get iterator at end position. */
    static const IterBaseType& end()
        { static const IterBaseType End; return End; }

protected:
    T*    obj_;             ///< Container object pointer
    bool  end_;             ///< End flag
    Key   key_;             ///< Iterator key
    Item* data_;            ///< Item pointer

    /** Constructor (used internally). */
    explicit IteratorBase() : obj_(NULL), end_(true), data_(NULL)
        { }

    /** Constructor (used internally).
    Only initializes object pointer.
     \param  obj  Pointer to container object (must not be NULL).
    */
    explicit IteratorBase(T* obj) : obj_(obj)
        { init(); }

private:
    void init() {
        if (obj_ != NULL && !IsConst<T>::value)
            // Only called with mutable iterator
            ((typename RemoveConst<T>::Type*)obj_)->iterInitMutable();
    }
};

///////////////////////////////////////////////////////////////////////////////

/** Forward iterator.
Used to iterate through container items.
 - Forward iterators only support forward iteration.
 .
 \tparam  T  Container type (must not be const)

\par Usage

Iterators work like normal pointers.
 - Containers with iterators will have built-in Iter (const) and IterM (mutable) iterator types for iteration
 - Create iterator for given container, normally set to first position by default
 - Explicitly set to start/end position using operator=(IteratorPos):
   \code
    iter = iterFIRST;
    iter = iterEND;
   \endcode
 - Increment to move to next item:
   \code
    ++iter;
    iter++;
   \endcode
 - Dereference to access item data:
   \code
    *iter
   \endcode
 - Evaluate iterator as a bool to check if active (at valid position) or has reached the end:
   \code
    if (iter) {
        // Iter is active/valid position
    }
    if (!iter) {
        // Iter is at end position
    }
   \endcode
   Note: This is done through safe bool conversion to avoid implicit conversion side-effects (see SafeBool)
 - Caution:
   - Adding or removing container items while an iterator is active (not at end) may move or invalidate the iterator
   - Results are undefined if an invalidated iterator is dereferenced
   .
 .

\par Example

\code
#include <evo/list.h>
#include <evo/io.h>
using namespace evo;
static Console& c = con();

int main() {
    // Using simple List container
    List<int> list;
    list.add(1);
    list.add(2);
    list.add(3);

    // Const iterator (prints items)
    for (List<int>::Iter iter(list, iterFIRST); iter; ++iter)
        c.out << *iter << NL;

    // Mutable iterator (changes all item values to 1)
    for (List<int>::IterM iter(list, iterFIRST); iter; ++iter)
        *iter = 1;

    return 0;
}
\endcode

\par Container Definition

Containers normally define iterator types via member typedefs with const and mutable (non-const) variations, normally named Iter (const) and IterM (mutable).
The iterator key type is stored by the iterator as a handle and is used to implement the iterator.
\n\n Example:
\code
// Example container class
template<class T> class Container {
public:
    // Iterator key -- used internally by container (using uint here as example)
    typedef uint IterKey;

    // Container iterator types
    typedef typename IteratorFw< Container<T>,IterKey,T >::Const Iter;
    typedef IteratorFw< Container<T>,IterKey,T > IterM;

    // ...
};
\endcode

\par Implementation Detail - Container Interface

Iterators interact with containers using an expected interface implemented by the container -- this is used internally:
 - Iterators store a Key value that is passed to the container for internal tracking
 - Forward iterators require the following container interface -- const-correctness is expected as shown:
   \code
    // Iterator key and state, keeps state for iterator, holds iterator key/position
    struct IterKey { ... };
    typedef ... IterKey;

    // Iterator item pointer, dereferences to iterator value
    typedef ... IterItem;

    // Called by mutable iterator constructor to notify container object that items may change
    void iterInitMutable();

    // Return pointer to first item or NULL if none
    const IterItem* iterFirst(IterKey& key) const;

    // Return pointer to next item data or NULL if no more
    const IterItem* iterNext(IterKey& key) const;
   \endcode
*/
template<class T>
class IteratorFw : public IteratorBase<T> {
protected:
    using IteratorBase<T>::obj_;
    using IteratorBase<T>::end_;
    using IteratorBase<T>::key_;
    using IteratorBase<T>::data_;

public:
    typedef typename IteratorBase<T>::Size Size;                                    ///< Size type to use
    typedef typename IteratorBase<T>::Key  Key;                                     ///< Iterator key type
    typedef typename IteratorBase<T>::Item Item;                                    ///< Iterator item type

    typedef IteratorFw<const T>                       Const;                        ///< Forward const iterator type
    typedef IteratorFw<typename RemoveConst<T>::Type> Mutable;                      ///< Forward mutable iterator type
    typedef IteratorFw<T>                             IterType;                     ///< Iterator type for parameter passing
    typedef IteratorBase<T>                           IterBaseType;                 ///< Iterator base type for parameter passing

    typedef typename StaticIf<IsConst<T>::value,Mutable,Const>::Type ToggleConst;   ///< Used for converting between Const/Mutable iterators

    /** Constructor. This sets empty iterator. */
    IteratorFw() : IteratorBase<T>()
        { }

    /** Constructor. This sets iterator to first item (or end if empty).
     \param  obj  Container object for iterator
    */
    explicit IteratorFw(T& obj) : IteratorBase<T>(obj)
        { first(); }

    /** Constructor. This sets iterator to first or end position.
     \param  obj  Container object for iterator
     \param  pos  Position to set, either iterFIRST or iterEND
    */
    explicit IteratorFw(T& obj, IteratorPos pos) : IteratorBase<T>(&obj) {
        switch (pos) {
            case iterFIRST: first(); break;
            default: end_ = true; data_ = NULL; break;
        };
    }

    /** Constructor. This initializes iterator with given position data (used internally).
     \param  obj   Container object for iterator
     \param  key   Iterator key to set
     \param  data  Iterator data pointer to set
    */
    IteratorFw(T& obj, const Key& key, Item* data) : IteratorBase<T>(obj, key, data)
        { }

    /** Copy constructor.
     \param  src  Source iterator to copy
    */
    IteratorFw(const IterType& src) : IteratorBase<T>(src)
        { }

    /** \copydoc IteratorFw(const IterType&) */
    IteratorFw(const IterBaseType& src) : IteratorBase<T>(src)
        { }

    /** \copydoc IteratorBase::operator=(const IterBaseType&) */
    IterType& operator=(const IterType& src)
        { IterBaseType::operator=(src); return *this; }

    /** \copydoc IteratorBase::operator=(const IterBaseType&) */
    IterType& operator=(const IterBaseType& src)
        { IterBaseType::operator=(src); return *this; }

    /** \copydoc IteratorBase::operator=(const ToggleConst&) */
    IterType& operator=(const ToggleConst& src) {
        STATIC_ASSERT(IsConst<T>::value, ERROR_cannot_assign_const_iterator_to_mutable_iterator);
        IterBaseType::operator=((IterType&)src);
        return *this;
    }

    /** \copydoc IteratorBase::operator=(const ToggleConst&) */
    IterType& operator=(const typename IterBaseType::ToggleConst& src) {
        STATIC_ASSERT(IsConst<T>::value, ERROR_cannot_assign_const_iterator_to_mutable_iterator);
        IterBaseType::operator=((IterType&)src);
        return *this;
    }

    /** Assignment operator. This sets iterator to first item (or end if empty).
     \param  obj  Container object to iterate through
     \return      This
    */
    IterType& operator=(T& obj)
        { IterBaseType::operator=(obj); first(); return *this; }

    /** Assignment operator to set position.
     \param  pos  Position to set, either iterFIRST or iterEND
     \return      This
    */
    IterType& operator=(IteratorPos pos) {
        switch (pos) {
            case iterFIRST: first(); break;
            default: end_ = true; data_ = NULL; break;
        };
        return *this;
    }

    /** Pre increment operator. Same as next().
     \return  This
    */
    IterType& operator++()
        { next(); return *this; }

    /** Post increment operator. Same as next(), but returns iterator copy before increment.
     \return  Iterator copy before increment
    */
    IterType operator++(int)
        { IterType tmp(*this); next(); return tmp; }

    // Inherited overloads
    using IteratorBase<T>::operator==;
    using IteratorBase<T>::operator!=;

protected:
    /** Constructor (used internally). Only initializes object pointer.
     \param  obj   Container object pointer
    */
    explicit IteratorFw(T* obj) : IteratorBase<T>(obj)
        { }

    /** Go to first item (used internally).
     \return  Whether first item was found, false if empty
    */
    bool first() {
        data_ = (Item*)obj_->iterFirst(key_);
        return end_ = (data_ == NULL);
    }

    /** Go to next item (used internally).
     \return  Whether next item was found, false if no more
    */
    bool next() {
        if (end_)
            return false;
        data_ = (Item*)obj_->iterNext(key_);
        if (data_ == NULL)
            end_ = true;
        return !end_;
    }
};

///////////////////////////////////////////////////////////////////////////////

/** Bidirectional iterator.
Used to iterate through container items.
 - Bidirectional iterators support forward and reverse iteration.
 .
 \tparam  T  Container type (must not be const)

\par Usage

Iterators work like normal pointers.
 - Containers with iterators will have built-in Iter (const) and IterM (mutable) iterator types for iteration
 - Create iterator for given container, normally set to first position by default
 - Explicitly set to start/last/end position using operator=(IteratorPos):
   \code
    iter = iterFIRST;
    iter = iterLAST;
    iter = iterEND;
   \endcode
 - Increment to move to next item:
   \code
    ++iter;
    iter++;
   \endcode
 - Decrement to move to previous item:
   \code
    --iter;
    iter--;
   \endcode
 - Dereference to access item data:
   \code
    *iter
   \endcode
 - Evaluate iterator as a bool to check if active (at valid position) or has reached the end:
   \code
    if (iter) {
        // Iter is active/valid position
    }
    if (!iter) {
        // Iter is at end position
    }
   \endcode
   Note: This is done through safe bool conversion to avoid implicit conversion side-effects (see SafeBool)
 - Caution:
   - Adding or removing container items while an iterator is active (not at end) may move or invalidate the iterator
   - Results are undefined if an invalidated iterator is dereferenced
   .
 .

\par Example

\code
#include <evo/list.h>
#include <evo/io.h>
using namespace evo;
static Console& c = con();

int main() {
    // Using simple List container
    List<int> list;
    list.add(1);
    list.add(2);
    list.add(3);

    // Const iterator (prints items in order and then in reverse)
    for (List<int>::Iter iter(list); iter; ++iter)
        c.out << *iter << NL;
    for (List<int>::Iter iter(list, iterLAST); iter; --iter)
        c.out << *iter << NL;

    // Mutable iterator (changes all item values to 1)
    for (List<int>::IterM iter(list); iter; ++iter)
        *iter = 1;

    return 0;
}
\endcode

\par Container Definition

Containers normally define iterator types via member typedefs with const and mutable (non-const) variations, normally named Iter (const) and IterM (mutable).
The iterator key type is stored by the iterator as a handle and is used to implement the iterator.
\n\n Example:
\code
// Example container class
template<class T> class Container {
public:
    // Iterator key -- used internally by container (using uint here as example)
    typedef uint IterKey;

    // Container iterator types
    typedef typename IteratorBi< Container<T>,IterKey,T >::Const Iter;
    typedef IteratorBi< Container<T>,IterKey,T > IterM;

    // ...
};
\endcode

\par Implementation Detail - Container Interface

Iterators interact with containers using an expected interface implemented by the container -- this is used internally:
 - Iterators store a Key value that is passed to the container for internal tracking
 - Bidirectional iterators require the following container interface -- const-correctness is expected as shown:
   \code
    // Iterator key and state, keeps state for iterator, holds iterator key/position
    struct IterKey { ... };
    typedef ... IterKey;

    // Iterator item pointer, dereferences to iterator value
    typedef ... IterItem;

    // Called by mutable iterator constructor to notify container object that items may change
    void iterInitMutable();

    // Return pointer to first item or NULL if none
    const IterItem* iterFirst(IterKey& key) const;

    // Return pointer to next item data or NULL if no more
    const IterItem* iterNext(IterKey& key) const;

    // Return pointer to last item or NULL if none
    const IterItem* iterLast(IterKey& key) const;

    // Return pointer to previous item data or NULL if no more
    const IterItem* iterPrev(IterKey& key) const;
   \endcode
*/
template<class T>
class IteratorBi : public IteratorFw<T> {
protected:
    using IteratorBase<T>::obj_;
    using IteratorBase<T>::end_;
    using IteratorBase<T>::key_;
    using IteratorBase<T>::data_;

    using IteratorFw<T>::first;

public:
    typedef typename IteratorBase<T>::Size Size;                                    ///< Size type to use
    typedef typename IteratorBase<T>::Key  Key;                                     ///< Iterator key type
    typedef typename IteratorBase<T>::Item Item;                                    ///< Iterator item type

    typedef IteratorBi<const T>                       Const;                        ///< Bidirectional const iterator type
    typedef IteratorBi<typename RemoveConst<T>::Type> Mutable;                      ///< Bidirectional mutable iterator type
    typedef IteratorBi<T>                             IterType;                     ///< Iterator type
    typedef IteratorBase<T>                           IterBaseType;                 ///< Iterator base type for parameter passing

    typedef typename StaticIf<IsConst<T>::value,Mutable,Const>::Type ToggleConst;   ///< Used for converting between Const/Mutable iterators

    /** Constructor. This sets empty iterator at end. */
    IteratorBi() : IteratorFw<T>()
        { }

    /** Constructor. This sets iterator to first item (or end if empty).
     \param  obj  Container object for iterator
    */
    explicit IteratorBi(T& obj) : IteratorFw<T>(obj)
        { }

    /** Constructor setting position. This sets iterator to given position (or end if empty).
     \param  obj  Container object for iterator
     \param  pos  Position to set, either iterFIRST, iterLAST, or iterEND
    */
    explicit IteratorBi(T& obj, IteratorPos pos) : IteratorFw<T>(&obj) {
        switch (pos) {
            case iterFIRST: first(); break;
            case iterLAST:  last();  break;
            default: end_ = true; data_ = NULL; break;
        };
    }

    /** Constructor. This initializes iterator with given position data (used internally).
     \param  obj   Container object to iterate through
     \param  key   Iterator key to set
     \param  data  Iterator data pointer to set
    */
    IteratorBi(T& obj, const Key& key, Item* data) : IteratorFw<T>(obj, key, data)
        { }

    /** Copy constructor.
     \param  src  Source iterator to copy.
    */
    IteratorBi(const IterType& src) : IteratorFw<T>(src)
        { }

    /** \copydoc IteratorBi(const IterType&) */
    IteratorBi(const IterBaseType& src) : IteratorFw<T>(src)
        { }

    /** \copydoc IteratorBase::operator=(const IterBaseType&) */
    IterType& operator=(const IterType& src)
        { IterBaseType::operator=(src); return *this; }

    /** \copydoc IteratorBase::operator=(const IterBaseType&) */
    IterType& operator=(const IterBaseType& src)
        { IterBaseType::operator=(src); return *this; }

    /** \copydoc IteratorBase::operator=(const ToggleConst&) */
    IterType& operator=(const ToggleConst& src) {
        STATIC_ASSERT(IsConst<T>::value, ERROR_cannot_assign_const_iterator_to_mutable_iterator);
        IterBaseType::operator=((IterType&)src);
        return *this;
    }

    /** \copydoc IteratorBase::operator=(const ToggleConst&) */
    IterType& operator=(const typename IterBaseType::ToggleConst& src) {
        STATIC_ASSERT(IsConst<T>::value, ERROR_cannot_assign_const_iterator_to_mutable_iterator);
        IterBaseType::operator=((IterType&)src);
        return *this;
    }

    /** \copydoc IteratorFw::operator=(T&) */
    IterType& operator=(T& obj)
        { IterBaseType::operator=(obj); first(); return *this; }

    /** Assignment operator to set position.
     \param  pos  Position to set, either iterFIRST, iterLAST, or iterEND
     \return      This
    */
    IterType& operator=(IteratorPos pos) {
        switch (pos) {
            case iterFIRST: first(); break;
            case iterLAST:  last();  break;
            default: end_ = true; data_ = NULL; break;
        };
        return *this;
    }

    /** Pre decrement operator. Same as prev().
     \return  This.
    */
    IterType& operator--()
        { prev(); return *this; }

    /** Post decrement operator. Same as prev(), but returns iterator copy before decrement.
     \return  Iterator copy before decrement.
    */
    IterType operator--(int)
        { IterType tmp(*this); prev(); return tmp; }

    // Inherited overloads
    using IteratorFw<T>::operator==;
    using IteratorFw<T>::operator!=;

protected:
    /** Constructor (used internally).
    Only initializes object pointer.
     \param  obj  Pointer to container objct.
    */
    explicit IteratorBi(T* obj) : IteratorFw<T>(obj)
        { }

    /** Go to last item.
     \return  Whether last item was found (false if empty).
    */
    bool last() {
        data_ = (Item*)obj_->iterLast(key_);
        return end_ = (data_ == NULL);
    }

    /** Go to previous item.
     \return  Whether previous item was found (false if no more).
    */
    bool prev() {
        if (end_)
            return false;
        data_ = (Item*)obj_->iterPrev(key_);
        if (data_ == NULL)
            end_ = true;
        return !end_;
    }
};

///////////////////////////////////////////////////////////////////////////////

/** Random access iterator.
Used to iterate through container items.
 - Random access iterators support forward and reverse iteration as well as random access by numeric index
 - Random access iterators always have a numeric key and support numeric comparison
 .
 \tparam  T  Container type (must not be const)

\par Usage

Iterators work like normal pointers.
 - Containers with iterators will have built-in Iter (const) and IterM (mutable) iterator types for iteration
 - Create iterator for given container, normally set to first position by default
 - Explicitly set to start/last/end position using operator=(IteratorPos):
   \code
    iter = iterFIRST;
    iter = iterLAST;
    iter = iterEND;
   \endcode
 - Explicitly set index position using operator=(Key):
   \code
    iter = 5;
   \endcode
 - Increment to move to next item:
   \code
    ++iter;
    iter++;
    iter += 2;
   \endcode
 - Decrement to move to previous item:
   \code
    --iter;
    iter--;
    iter -= 2;
   \endcode
 - Dereference to access item data:
   \code
    *iter
   \endcode
 - Evaluate iterator as a bool to check if active (at valid position) or has reached the end:
   \code
    if (iter) {
        // Iter is active/valid position
    }
    if (!iter) {
        // Iter is at end position
    }
   \endcode
   Note: This is done through safe bool conversion to avoid implicit conversion side-effects (see SafeBool)
 - Caution:
   - Adding or removing container items while an iterator is active (not at end) may move or invalidate the iterator
   - Results are undefined if an invalidated iterator is dereferenced
   .
 .

\par Example

\code
#include <evo/list.h>
#include <evo/io.h>
using namespace evo;
static Console& c = con();

int main() {
    // Using simple List container
    List<int> list;
    list.add(1);
    list.add(2);
    list.add(3);

    // Const iterator (prints items in order and then in reverse)
    for (List<int>::Iter iter(list, iterFIRST); iter; ++iter)
        c.out << *iter << NL;
    for (List<int>::Iter iter(list, iterLAST); iter; --iter)
        c.out << *iter << NL;

    // Mutable iterator (changes all item values to 1)
    for (List<int>::IterM iter(list, iterFIRST); iter; ++iter)
        *iter = 1;

    // Mutable iterator (change first item to 2)
    {
        List<int>::IterM iter(list);
        iter = 0;
        *iter = 2;
    }

    return 0;
}
\endcode

\par Container Definition

Containers normally define iterator types via member typedefs with const and mutable (non-const) variations, normally named Iter (const) and IterM (mutable).
The iterator key type is stored by the iterator as a handle and is used to implement the iterator.
\n\n Example:
\code
// Example container class
template<class T> class Container {
public:
    // Iterator key -- used internally by container (using uint here as example)
    typedef uint IterKey;

    // Container iterator types
    typedef typename IteratorBi< Container<T>,IterKey,T >::Const Iter;
    typedef IteratorBi< Container<T>,IterKey,T > IterM;

    // ...
};
\endcode

\par Implementation Detail - Container Interface

Iterators interact with containers using an expected interface implemented by the container -- this is used internally:
 - Iterators store a Key value that is passed to the container for internal tracking
 - Random access iterators require the following container interface -- const-correctness is expected as shown:
   \code
    // Iterator key and state, keeps state for iterator, holds iterator key/position
    struct IterKey { ... };
    typedef ... IterKey;

    // Iterator item pointer, dereferences to iterator value
    typedef ... IterItem;

    // Called by mutable iterator constructor to notify container object that items may change
    void iterInitMutable();

    // Return pointer to first item or NULL if none
    const IterItem* iterFirst(IterKey& key) const;

    // Return pointer to next item data or NULL if no more
    const IterItem* iterNext(IterKey& key) const;

    // Return pointer to last item or NULL if none
    const IterItem* iterLast(IterKey& key) const;

    // Return pointer to previous item data or NULL if no more
    const IterItem* iterPrev(IterKey& key) const;

    // Return pointer to item data at given numeric key index, or NULL if not valid
    const IterItem* iterSet(IterKey key) const;

    // Return item count
    Size iterCount() const;
   \endcode
*/
template<class T>
class IteratorRa : public IteratorBi<T> {
protected:
    using IteratorBase<T>::obj_;
    using IteratorBase<T>::end_;
    using IteratorBase<T>::key_;
    using IteratorBase<T>::data_;

    using IteratorFw<T>::first;

public:
    typedef typename IteratorBase<T>::Size Size;                                    ///< Size type to use
    typedef typename IteratorBase<T>::Key  Key;                                     ///< Iterator key type
    typedef typename IteratorBase<T>::Item Item;                                    ///< Iterator item type

    typedef IteratorRa<const T>                       Const;                        ///< Random access const iterator type
    typedef IteratorRa<typename RemoveConst<T>::Type> Mutable;                      ///< Random access mutable iterator type
    typedef IteratorRa<T>                             IterType;                     ///< Iterator type
    typedef IteratorBase<T>                           IterBaseType;                 ///< Iterator base type for parameter passing

    typedef typename StaticIf<IsConst<T>::value,Mutable,Const>::Type ToggleConst;   ///< Used for converting between Const/Mutable iterators

    /** Constructor. This sets empty iterator at end. */
    IteratorRa() : IteratorBi<T>()
        { }

    /** Constructor. This sets iterator to first item (or end if empty).
     \param  obj  Container object for iterator
    */
    explicit IteratorRa(T& obj) : IteratorBi<T>(obj)
        { }

    /** Constructor setting position. This sets iterator to given position (or end if empty).
     \param  obj  Container object for iterator
     \param  pos  Position to set, either iterFIRST, iterLAST, or iterEND
    */
    explicit IteratorRa(T& obj, IteratorPos pos) : IteratorBi<T>(obj, pos)
        { }

    /** Constructor setting position. This sets iterator to given position (or end if empty).
     \param  obj  Container object for iterator
     \param  num  Position index to set
    */
    IteratorRa(T& obj, Key num) : IteratorBi<T>(&obj) {
        data_ = (Item*)obj_->iterSet(num);
        end_ = (data_ == NULL);
        key_ = num;
    }

    /** Constructor. This initializes iterator with given position data (used internally).
     \param  obj   Container object for iterator
     \param  key   Iterator key to set
     \param  data  Iterator data pointer to set
    */
    IteratorRa(T& obj, const Key& key, Item* data) : IteratorBi<T>(obj, key, data)
        { }

    /** Copy constructor.
     \param  src  Source iterator to copy
    */
    IteratorRa(const IterType& src) : IteratorBi<T>(src)
        { }

    /** \copydoc IteratorRa(const IterType&) */
    IteratorRa(const IterBaseType& src) : IteratorBi<T>(src)
        { }

    /** \copydoc IteratorBase::operator=(const IterBaseType&) */
    IterType& operator=(const IterType& src)
        { IterBaseType::operator=(src); return *this; }

    /** \copydoc IteratorBase::operator=(const IterBaseType&) */
    IterType& operator=(const IterBaseType& src)
        { IterBaseType::operator=(src); return *this; }

    /** \copydoc IteratorBase::operator=(const ToggleConst&) */
    IterType& operator=(const ToggleConst& src) {
        STATIC_ASSERT(IsConst<T>::value, ERROR_cannot_assign_const_iterator_to_mutable_iterator);
        IterBaseType::operator=((IterType&)src);
        return *this;
    }

    /** \copydoc IteratorBase::operator=(const ToggleConst&) */
    IterType& operator=(const typename IterBaseType::ToggleConst& src) {
        STATIC_ASSERT(IsConst<T>::value, ERROR_cannot_assign_const_iterator_to_mutable_iterator);
        IterBaseType::operator=((IterType&)src);
        return *this;
    }

    /** \copydoc IteratorFw::operator=(T&) */
    IterType& operator=(T& obj)
        { IterBaseType::operator=(obj); first(); return *this; }

    /** \copydoc IteratorBi::operator=(IteratorPos) */
    IterType& operator=(IteratorPos pos)
        { IteratorBi<T>::operator=(pos); return *this; }

    /** Assignment operator for random access position index.
     \param  num  Iterator index to set
     \return      This
    */
    IterType& operator=(Key num) {
        data_ = (Item*)obj_->iterSet(num);
        end_ = (data_ == NULL);
        key_ = num;
        return *this;
    }

    /** In-place addition (multi increment) operator. Same as next(Size).
     \param  count  Count to add
     \return        This
    */
    IterType& operator+=(Size count)
        { next(count); return *this; }

    /** In-place subtraction (multi decrement) operator. Same as prev(Size).
     \param  count  Count to subtract
     \return        This
    */
    IterType& operator-=(Size count)
        { prev(count); return *this; }

    /** Addition (multi increment) operator. Same as next(Size) on the returned temporary iterator.
     - In-place operator+=() is preferred to avoid creating temporary object
     .
     \param  count  Count to add
     \return        Temporary incremented iterator copy
    */
    IterType operator+(Size count) const
        { IterType tmp(*this); tmp.next(count); return tmp; }

    /** Subtraction (multi decrement) operator. Same as prev(Size) on the returned temporary iterator.
     - In-place operator-=() is preferred to avoid creating temporary object
     .
     \param  count  Count to subtract
     \return        Temporary decremented iterator copy
    */
    IterType operator-(Size count) const
        { IterType tmp(*this); tmp.prev(count); return tmp; }

    /** %Compare to another iterator.
     \param  iter  Iterator to compare
     \return       Comparison result, -1: this is less, 0: equal, 1: this is greater
    */
    int compare(const IterBaseType& iter) const {
        if (iter.getEnd())
            return (end_ ? 0 : -1);
        else if (end_)
            return 1;
        else if (key_ == iter.getKey())
            return 0;
        else
            return (key_ < iter.getKey() ? -1 : 1);
    }

    /** %Compare to a position index.
     \param  num  Position index to compare, END for end position
     \return      Comparison result, -1: this is less, 0: equal, 1: this is greater
    */
    int compare(Key num) const {
        if (num == END)
            return (end_ ? 0 : -1);
        else if (end_)
            return 1;
        else if (key_ == num)
            return 0;
        else
            return (key_ < num ? -1 : 1);
    }

    /** Less-than operator.
     \param  iter  Iterator to compare
     \return       Whether less than iter
    */
    bool operator<(const IterBaseType& iter) const
        { return (compare(iter) < 0); }

    /** Less-than operator.
     \param  num  Position index to compare
     \return      Whether less than index
    */
    bool operator<(Key num) const
        { return (compare(num) < 0); }

    /** Less-than-or-equal operator.
     \param  iter  Iterator to compare
     \return       Whether less than or equal to iter
    */
    bool operator<=(const IterBaseType& iter) const
        { return (compare(iter) <= 0); }

    /** Less-than-or-equal operator.
     \param  num  Position index to compare
     \return      Whether less than or equal to index
    */
    bool operator<=(Key num) const
        { return (compare(num) <= 0); }

    /** Greater-than operator.
     \param  iter  Iterator to compare
     \return       Whether greater than iter
    */
    bool operator>(const IterBaseType& iter) const
        { return (compare(iter) > 0); }

    /** Greater-than operator.
     \param  num  Position index to compare
     \return      Whether greater than index
    */
    bool operator>(Key num) const
        { return (compare(num) > 0); }

    /** Greater-than-or-equal operator.
     \param  iter  Iterator to compare
     \return       Whether greater than or equal to iter
    */
    bool operator>=(const IterBaseType& iter) const
        { return (compare(iter) >= 0); }

    /** Greater-than-or-equal operator.
     \param  num  Position index to compare
     \return      Whether greater than or equal to index
    */
    bool operator>=(Key num) const
        { return (compare(num) >= 0); }

    // Inherited overloads
    using IteratorBi<T>::operator==;
    using IteratorBi<T>::operator!=;

    /** Equality operator.
     \param  num  Position index to compare
     \return      Whether equal to index
    */
    bool operator==(Key num) const
        { return (compare(num) == 0); }

    /** Inequality operator.
     \param  num  Position index to compare
     \return      Whether not equal to index
    */
    bool operator!=(Key num) const
        { return (compare(num) != 0); }

    /** Get container item count.
     \return  Item count
    */
    Size count() const
        { return obj_->iterCount(); }

    /** Get iterator position index.
     \return  Position index, END if end
    */
    Key index() const
        { return (end_ ? Key(END) : key_); }

protected:
    /** Go to next item, skipping given count (used internally).
     \param  count  Number of items to advance
     \return        Whether given item was found, false if no more
    */
    bool next(Size count) {
        if (end_)
            return false;
        data_ = (Item*)obj_->iterNext(count, key_);
        if (data_ == NULL)
            end_ = true;
        return !end_;
    }

    /** Go to previous item, skipping given count (used internally).
     \param  count  Number of items to advance
     \return        Whether given item was found, false if no more
    */
    bool prev(Size count) {
        if (end_)
            return false;
        data_ = (Item*)obj_->iterPrev(count, key_);
        if (data_ == NULL)
            end_ = true;
        return !end_;
    }
};

///////////////////////////////////////////////////////////////////////////////

/** Iterator template for sequential enum values.
 - Enum numeric values must be sequential, and must not have any gaps between first and last values
 - This is used by enum types created by helpers like: EVO_ENUM_MAP_PREFIXED() and EVO_ENUM_CLASS_MAP()
 - See \ref EnumConversion
 .
 \tparam  T  Enum type to iterator values on
 \tparam  F  First enum value to start iterator at
 \tparam  L  Last enum value to end iterator at

\par Example

This simple example prints out enum number values
 - For an example printing enum string values see EnumMapIterator
 .

\code
#include <evo/type.h>
#include <evo/io.h>
using namespace evo;

enum MyEnum {
    ENUM_ONE,
    ENUM_TWO,
    ENUM_THREE
};

typedef EnumIterator<MyEnum,ENUM_ONE,ENUM_THREE> MyEnumIter;

int main() {
    Console& c = con();
    for (MyEnumIter iter; iter; ++iter)
        c.out << iter.value_num() << NL;

    return 0;
}
\endcode
*/
template<class T, int F, int L>
class EnumIterator : public SafeBool< EnumIterator<T,F,L> > {
public:
    typedef EnumIterator<T,F,L> This;
    typedef T EnumType;
    static const T FIRST = (T)F;
    static const T LAST = (T)L;

    /** Constructor, sets to first enum value. */
    EnumIterator() : value_(FIRST), end_(false) {
    }

    /** Copy constructor.
     \param  src  Source to copy
    */
    EnumIterator(const This& src) : value_(src.value_), end_(src.end_) {
    }

    /** Constructor to start at explicit enum value.
     \param  value  Enum value to start at
    */
    EnumIterator(EnumType value) : value_(value), end_(false) {
    }

    /** Constructor to start at given position.
     \param  pos  Position to start at, either: iterFIRST, iterLAST, or iterEND
    */
    EnumIterator(IteratorPos pos) {
        setpos(pos);
    }

    /** Assignment operator.
     \param  src  Source to copy
     \return      This
    */
    This& operator=(const This& src) {
        value_ = src.value_;
        end_ = src.end_;
        return *this;
    }

    /** Assignment operator to set at explicit enum value.
     \param  value  Enum value to set at
     \return        This
    */
    This& operator=(EnumType value) {
        value_ = value;
        end_ = false;
        return *this;
    }

    /** Assignment operator to set at given position.
     \param  pos  Position to start at, either: iterFIRST, iterLAST, or iterEND
     \return        This
    */
    This& operator=(IteratorPos pos) {
        setpos(pos);
        return *this;
    }

    /** Check whether iterator is at end.
     \return  Whether at end
    */
    bool operator!() const
        { return end_; }

    /** Dereference iterator to get current enum value.
     \return  Current enum value
    */
    EnumType operator*() const
        { return value_; }

    /** Equality operator to compare with another iterator to same enum.
     \param  oth  Other iterator to compare to
     \return      Whether equal
    */
    bool operator==(const This& oth) const
        { return (end_ == oth.end_ && value_ == oth.value_); }

    /** Inequality operator to compare with another iterator to same enum.
     \param  oth  Other iterator to compare to
     \return      Whether inequal
    */
    bool operator!=(const This& oth) const
        { return (end_ != oth.end_ || value_ != oth.value_); }

    /** Pre increment operator, moves to next enum value or end.
     \return  This
    */
    This& operator++() {
        next();
        return *this;
    }

    /** Post increment operator, moves to next enum value or end.
     \return  Iterator copy before increment
    */
    This operator++(int) {
        This result(*this);
        next();
        return result;
    }

    /** Pre decrement operator, moves to previous enum value or end.
     \return  This
    */
    This& operator--() {
        prev();
        return *this;
    }

    /** Post decrement operator, moves to previous enum value or end.
     \return  Iterator copy before decrement
    */
    This operator--(int) {
        This result(*this);
        prev();
        return result;
    }

    /** Get current enum value.
     \return  Current enum value
    */
    EnumType value() const
        { return value_; }

    /** Get current enum number value.
     \return  Current enum number value
    */
    int value_num(int endvalue=0) const {
        if (end_)
            return endvalue;
        return (int)value_;
    }

protected:
    EnumType value_;
    bool end_;

    void setpos(IteratorPos pos) {
        switch (pos) {
            case iterFIRST:
                value_ = FIRST;
                end_ = false;
                break;
            case iterLAST:
                value_ = LAST;
                end_ = false;
                break;
            default:
                value_ = LAST;
                end_ = true;
                break;
        }
    }

    void prev() {
        if (!end_) {
            if (value_ <= FIRST)
                end_ = true;
            else
                value_ = (EnumType)((int)value_ - 1);
        }
    }

    void next() {
        if (!end_) {
            if (value_ >= LAST)
                end_ = true;
            else
                value_ = (EnumType)((int)value_ + 1);
        }
    }
};

///////////////////////////////////////////////////////////////////////////////
//@}
}
#endif
