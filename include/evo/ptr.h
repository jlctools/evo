// Evo C++ Library
/* Copyright (c) 2016 Justin Crowell
 This Source Code Form is subject to the terms of the Mozilla Public
 License, v. 2.0. If a copy of the MPL was not distributed with this
 file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
///////////////////////////////////////////////////////////////////////////////
/** \file ptr.h Evo Smart Pointers. */
#pragma once
#ifndef INCL_evo_ptr_h
#define INCL_evo_ptr_h

// Includes
#include "type.h"

// Namespace: evo
namespace evo {

/** \addtogroup EvoCore */
//@{

///////////////////////////////////////////////////////////////////////////////

/** Smart pointer to single object.
 - Takes ownership of a given pointer and frees the pointer automatically by calling delete
 - Overrides pointer operators so smart pointer can be used like a raw pointer
 - Copying a smart pointer makes a copy of the pointed object (raw pointer not shared) -- ok for containers
   - This requires T implement a proper copy-constructor
 - Call detach() to release ownership of pointer without freeing it
 .

SmartPtr is specialized for arrays so the pointer is freed with delete[].
However an array SmartPtr cannot be copied (array size isn't known) so it isn't usable in containers, use List or Array instead.

 \tparam  T  Type to use pointer to (not the raw pointer type)

\par Example

\code
#include <evo/ptr.h>
#include <evo/string.h>
using namespace evo;

int main() {
    // Create smart pointer, set a value
    SmartPtr<String> ptr(new String);
    *ptr = "testing";    // use dereference operator* like normal pointer

    // Copying smart pointer makes a copy of the object
    {
        SmartPtr<String> ptr2(ptr);
        *ptr2 = "foo";   // dereference ptr2 and change value
    } // ptr2 automatically freed

    // Print ptr value ("testing") -- use operator-> like normal pointer
    printf("%s\n", ptr->cstr());
} // ptr automatically freed
\endcode

Output
\verbatim
testing
\endverbatim

\par Array Example

\code
#include <evo/ptr.h>
using namespace evo;

int main() {
    // Create array smart pointer, set a value (example only, better to use String)
    SmartPtr<char[]> ptr(new char[8]);
    strcpy(ptr.ptr(), "testing");

    *ptr = 'T';         // use dereference operator* like normal pointer
    ptr[0] = 'T';       // use operator[] like normal array pointer

    // Copying array smart pointer not supported (size unknown)
    //SmartPtr<char[]> ptr2(ptr);   // Error
    //ptr2 = ptr;                   // Error

    // Print ptr value
    printf("%s\n", ptr.ptr());
} // ptr automatically freed
\endcode

Output
\verbatim
Testing
\endverbatim

\par Transferring Ownership

SmartPtr takes ownership. Transfer ownership with detach().

Note that it's often better to share ownership with SharedPtr rather than manually transfer ownership around.

\code
#include <evo/ptr.h>
using namespace evo;

// Function that takes ownership of a pointer (SmartPtr passed by value)
void take(SmartPtr<int> ptr) {
    // ptr owns the pointer, it will be freed at end of scope
};

int main() {
    // Create local pointer, set a value
    SmartPtr<int> local_ptr(new int);
    *local_ptr = 1;

    // Call function, release ownership with detach()
    take(local_ptr.detach());
}
\endcode
*/
template<class T>
class SmartPtr : public PtrBase<T>
{
protected:
    using PtrBase<T>::ptr_;

public:
    typedef SmartPtr<T> ThisType;    ///< This pointer type

    /** Constructor.
     \param  ptr  Pointer to set, NULL for none
    */
    SmartPtr(T* ptr=NULL)
        { ptr_ = ptr; }

    /** Copy constructor.
     - This makes a copy of source object using T copy constructor
     - Disabled if array T[]
     .
     \param  src  Source pointer
    */
    SmartPtr(const ThisType& src)
        { ptr_ = new T(*src.ptr_); }

    /** Destructor. */
    ~SmartPtr()
        { free(); }

    /** Copy/Assignment operator.
     - This makes a copy of source object using T copy constructor
     - Disabled if array T[]
     .
     \param  src  Source pointer
     \return      This
    */
    ThisType& operator=(const ThisType& src)
        { free(); ptr_ = new T(*src.ptr_); return *this; }

    /** Assignment operator for new pointer.
     \param  ptr  Pointer to set
     \return      This
    */
    ThisType& operator=(T* ptr)
        { free(); ptr_ = ptr; return *this; }

    /** Clear (free) pointer and set as null.
     \return  This
    */
    ThisType& clear()
        { free(); ptr_ = NULL; return *this; }

    /** Detach and return pointer.
     - This returns current pointer and sets to null, releasing ownership
     .
     \return  Pointer
    */
    T* detach() {
        T* result = ptr_;
        ptr_ = NULL;
        return result;
    }

protected:
    void free() {
        if (ptr_ != NULL)
            delete ptr_;
    }
};

// Specialize for array
template<class T>
class SmartPtr<T[]> : public PtrBase<T>
{
protected:
    using PtrBase<T>::ptr_;

public:
    typedef SmartPtr<T[]> ThisType;

    SmartPtr(T* ptr=NULL)
        { ptr_ = ptr; }

    ~SmartPtr()
        { free(); }

    ThisType& operator=(T* ptr)
        { free(); ptr_ = ptr; return *this; }

    ThisType& clear()
        { free(); ptr_ = NULL; return *this; }

    T* detach() {
        T* result = ptr_;
        ptr_ = NULL;
        return result;
    }

protected:
    void free() {
        if (ptr_ != NULL)
            delete [] ptr_;
    }

private:
    // Disable copying
    explicit SmartPtr(const ThisType&);
    ThisType& operator=(const ThisType&);
};

///////////////////////////////////////////////////////////////////////////////

/** Shared smart pointer to single object.
 - Similar to SmartPtr except this uses reference-counted sharing between copies
 - A reference count is incremented for each copy, the last owner automatically frees the pointer
 - Overrides pointer operators so shared smart pointer can be used like a raw pointer
 - Copying a shared smart pointer makes a shared copy, incrementing the reference count -- ok for containers
   - Modifying a shared object affects all pointers referencing it
 - The following method requires T implement a proper copy-constructor: unshare()
 .

SharedPtr is specialized for arrays so the pointer is freed with delete[]. However an array SharedPtr doesn't support unshare() (array size isn't known).

 \tparam  T  Type to use pointer to (not the raw pointer type)

\par Example

\code
#include <evo/ptr.h>
#include <evo/string.h>
using namespace evo;

int main() {
    // Create shared pointer, set a value
    SharedPtr<String> ptr(new String);
    *ptr = "testing";    // use dereference operator* like normal pointer

    // Copying shared pointer makes a shared copy
    {
        SharedPtr<String> ptr2(ptr);
        *ptr2 = "foo";   // dereference ptr2 and change value (affects ptr as well)
    }

    // Print ptr value ("foo") -- use operator-> like normal pointer
    printf("%s\n", ptr->cstr());

    // May use Ptr to reference a shared pointer without sharing it (does not increment reference count)
    {
        Ptr<String> ptr2(ptr);
        *ptr2 = "bar";   // dereference ptr2 and change value again (affects ptr as well)
    }

    // Print ptr value ("bar") -- use operator-> like normal pointer
    printf("%s\n", ptr->cstr());
} // ptr automatically freed
\endcode

Output
\verbatim
foo
bar
\endverbatim

\par Array Example

\code
#include <evo/ptr.h>
#include <evo/string.h>
using namespace evo;

int main() {
    // Create shared pointer, set a value
    SharedPtr<String[]> ptr(new String[1]);
    ptr[0] = "testing";     // use dereference operator[] like normal pointer
    *ptr   = "testing";     // use dereference operator* like normal pointer

    // Copying shared pointer makes a shared copy
    {
        SharedPtr<String[]> ptr2(ptr);
        ptr2[0] = "foo";    // dereference ptr2 and change value (affects ptr as well)
    }

    // Print ptr value ("foo") -- use operator-> like normal pointer
    printf("%s\n", ptr->cstr());

    // May use Ptr to reference a shared pointer without sharing it (does not increment reference count)
    {
        Ptr<String> ptr2(ptr);
        ptr2[0] = "bar";   // dereference ptr2 and change value again (affects ptr as well)
    }

    // Print ptr value ("bar") -- use operator[] like normal pointer
    printf("%s\n", ptr[0].cstr());
} // ptr automatically freed
\endcode

Output
\verbatim
foo
bar
\endverbatim
*/
template<class T,class TSize=SizeT>
class SharedPtr : public PtrBase<T>
{
protected:
    using PtrBase<T>::ptr_;

public:
    typedef SharedPtr<T,TSize> ThisType;    ///< This pointer type

    /** Constructor to start with null pointer. */
    SharedPtr()
        { ptr_ = NULL; refs_ = NULL; }

    /** Constructor.
     - This takes ownership of the pointer and will free it later -- starts reference count at 1
     .
     \param  ptr  Pointer to set, NULL for none
    */
    SharedPtr(T* ptr) {
        if (ptr != NULL)
            { ptr_ = ptr; refs_ = new TSize; *refs_ = 1; }
        else
            { ptr_ = NULL; refs_ = NULL; }
    }

    /** Copy constructor.
     - This makes a shared copy of source object, incrementing reference count
     .
     \param  src  Source pointer
    */
    SharedPtr(const ThisType& src) {
        if (src.ptr_ != NULL)
            { ptr_ = src.ptr_; refs_ = src.refs_; ++(*refs_); }
        else
            { ptr_ = NULL; refs_ = NULL; }
    }

    /** Destructor. */
    ~SharedPtr()
        { free(); }

    /** Copy/Assignment operator.
     - This makes a copy of source object using T copy constructor
     - The previous pointer is released (freed, if applicable)
     .
     \param  src  Source pointer
     \return      This
    */
    ThisType& operator=(const ThisType& src) {
        free();
        if (src.ptr_ != NULL)
            { ptr_ = src.ptr_; refs_ = src.refs_; ++(*refs_); }
        else
            { ptr_ = NULL; refs_ = NULL; }
        return *this;
    }

    /** Assignment operator for new pointer.
     - This takes ownership of the pointer and will free it later -- starts reference count at 1
     - The previous pointer is released (freed, if applicable)
     .
     \param  ptr  Pointer to set
     \return      This
    */
    ThisType& operator=(T* ptr) {
        clear();
        if (ptr != NULL) {
            ptr_ = ptr;
            if (refs_ == NULL)
                refs_ = new TSize;
            *refs_ = 1;
        }
        return *this;
    }

    /** Release pointer and set as null.
     - This will decrement the reference count, and free the pointer if not shared
     .
     \return  This
    */
    ThisType& clear() {
        if (ptr_ != NULL) {
            if (--(*refs_) == 0)
                delete ptr_; // leave refs_ allocated for later
            else
                refs_ = NULL; // detach from shared
            ptr_  = NULL;
        }
        return *this;
    }

    /** Unshare pointer by setting as a new copy, if shared.
     - Disabled if array T[]
     .
     \return  This
    */
    ThisType& unshare() {
        if (ptr_ != NULL && *refs_ > 1) {
            ptr_   = new T(*ptr_);
            --(*refs_);
            refs_  = new TSize;
            *refs_ = 1;
        }
        return *this;
    }

    /** Get whether pointer is shared (reference count > 1).
     \return  Whether shared
    */
    bool shared() const
        { return (ptr_ != NULL && *refs_ > 1); }

protected:
    TSize* refs_;

    void free() {
        if (ptr_ != NULL) {
            if (--(*refs_) == 0)
                { delete ptr_; delete refs_; }
        } else if (refs_ != NULL)
            delete refs_;
    }
};

// Specialize for array
template<class T,class TSize>
class SharedPtr<T[],TSize> : public PtrBase<T>
{
protected:
    using PtrBase<T>::ptr_;

public:
    typedef SharedPtr<T[],TSize> ThisType;

    SharedPtr()
        { ptr_ = NULL; refs_ = NULL; }

    SharedPtr(T* ptr) {
        if (ptr != NULL)
            { ptr_ = ptr; refs_ = new TSize; *refs_ = 1; }
        else
            { ptr_ = NULL; refs_ = NULL; }
    }

    SharedPtr(const ThisType& src) {
        if (src.ptr_ != NULL)
            { ptr_ = src.ptr_; refs_ = src.refs_; ++(*refs_); }
        else
            { ptr_ = NULL; refs_ = NULL; }
    }

    ~SharedPtr()
        { free(); }

    ThisType& operator=(const ThisType& src) {
        free();
        if (src.ptr_ != NULL)
            { ptr_ = src.ptr_; refs_ = src.refs_; ++(*refs_); }
        else
            { ptr_ = NULL; refs_ = NULL; }
        return *this;
    }

    ThisType& operator=(T* ptr) {
        clear();
        if (ptr != NULL) {
            ptr_ = ptr;
            if (refs_ == NULL)
                refs_ = new TSize;
            *refs_ = 1;
        }
        return *this;
    }

    ThisType& clear() {
        if (ptr_ != NULL) {
            if (--(*refs_) == 0) {
                delete [] ptr_; // leave refs_ allocated for later
                ptr_ = NULL;
            } else
                refs_ = NULL; // detach from shared
        }
        return *this;
    }

    bool shared() const
        { return (ptr_ != NULL && *refs_ > 1); }

protected:
    TSize* refs_;

    void free() {
        if (ptr_ != NULL) {
            if (--(*refs_) == 0)
                { delete [] ptr_; delete refs_; }
        } else if (refs_ != NULL)
            delete refs_;
    }

private:
    // Disabled if array
    ThisType& unshare();
};

///////////////////////////////////////////////////////////////////////////////

/** Dumb pointer to single object or array.
 - Useful for making it clear the pointer is not owned here, and is freed elsewhere
 - Does not take ownership of any pointer, does not automatically free anything
 - Overrides pointer operators so dumb pointer can be used like a raw pointer/array
 - Copying a dumb pointer makes a copy of the pointer itself, referencing the same object(s) -- only safe with containers if pointer outlives the container
 .

Ptr is specialized for arrays for consistency with other managed pointers -- functionality is identical.

 \tparam  T  Type to use pointer to (not the raw pointer type)

\par Example

\code
#include <evo/ptr.h>
using namespace evo;

// It's clear here that this function does not take ownership of ptr
void func(Ptr<char> ptr) {
    if (ptr)
        printf("%s\n", ptr.ptr());
}

int main() {
    // Array pointer with data (example only, better to use String)
    SmartPtr<char[]> data(new char[5]);
    strcpy(data.ptr(), "foo");

    // Call function that uses the pointer
    func(data);

    return 0;
}
\endcode
*/
template<class T>
class Ptr : public PtrBase<T>
{
protected:
    using PtrBase<T>::ptr_;

public:
    typedef Ptr<T> ThisType;        ///< This pointer type
    typedef PtrBase<T> BaseType;    ///< Base type

    /** Default constructor sets as NULL. */
    Ptr()
        { ptr_ = NULL; }

    /** Constructor.
     \param  ptr  Pointer to set, NULL for none
    */
    Ptr(T* ptr)
        { ptr_ = ptr; }

    /** Constructor.
     \param  ptr  Pointer to set, NULL for none
    */
    Ptr(const T* ptr)
        { ptr_ = (T*)ptr; }

    /** Copy constructor.
     \param  src  Source pointer
    */
    Ptr(const ThisType& src)
        { ptr_ = src.ptr_; }

    /** Copy constructor to reference pointer.
     \param  src  Source pointer
    */
    Ptr(const BaseType& src)
        { ptr_ = src.ptr_; }

    /** Copy/Assignment operator.
     \param  src  Source pointer
     \return      This
    */
    ThisType& operator=(const ThisType& src)
        { ptr_ = src.ptr_; return *this; }

    /** Assignment operator to reference pointer.
     \param  src  Source pointer
     \return      This
    */
    ThisType& operator=(const BaseType& src)
        { ptr_ = src.ptr_; return *this; }

    /** Assignment operator for raw pointer.
     \param  ptr  Pointer to set
     \return      This
    */
    ThisType& operator=(T* ptr)
        { ptr_ = ptr; return *this; }

    /** Assignment operator for raw pointer.
     \param  ptr  Pointer to set
     \return      This
    */
    ThisType& operator=(const T* ptr)
        { ptr_ = (T*)ptr; return *this; }

    /** Clear pointer, setting as null.
     \return  This
    */
    ThisType& clear()
        { ptr_ = NULL; return *this; }

    /** Detach and return pointer.
     - This returns current pointer and sets to null
     .
     \return  Pointer
    */
    T* detach() {
        T* result = ptr_;
        ptr_ = NULL;
        return result;
    }
};

// Specialize for array (consistency with SmartPtr, SharedPtr) -- exact same functionality
template<class T>
class Ptr<T[]> : public PtrBase<T>
{
protected:
    using PtrBase<T>::ptr_;

public:
    typedef Ptr<T[]>   ThisType;
    typedef PtrBase<T> BaseType;

    Ptr()
        { ptr_ = NULL; }

    Ptr(T* ptr)
        { ptr_ = ptr; }

    Ptr(const T* ptr)
        { ptr_ = (T*)ptr; }

    Ptr(const ThisType& src)
        { ptr_ = src.ptr_; }

    Ptr(const BaseType& src)
        { ptr_ = src.ptr_; }

    ThisType& operator=(const ThisType& src)
        { ptr_ = src.ptr_; return *this; }

    ThisType& operator=(const BaseType& src)
        { ptr_ = src.ptr_; return *this; }

    ThisType& operator=(T* ptr)
        { ptr_ = ptr; return *this; }

    ThisType& operator=(const T* ptr)
        { ptr_ = (T*)ptr; return *this; }

    ThisType& clear()
        { ptr_ = NULL; return *this; }

    T* detach() {
        T* result = ptr_;
        ptr_ = NULL;
        return result;
    }
};

///////////////////////////////////////////////////////////////////////////////
//@}
} // Namespace: evo
#endif
