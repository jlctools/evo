// Evo C++ Library
/* Copyright 2019 Justin Crowell
Distributed under the BSD 2-Clause License -- see included file LICENSE.txt for details.
*/
///////////////////////////////////////////////////////////////////////////////
/** \file ptr.h Evo Smart Pointers. */
#pragma once
#ifndef INCL_evo_ptr_h
#define INCL_evo_ptr_h

#include "type.h"

namespace evo {
/** \addtogroup EvoCore */
//@{

///////////////////////////////////////////////////////////////////////////////

/** Smart pointer to single object.
 - Takes ownership of a given pointer and frees the pointer automatically by calling delete
 - Overrides pointer operators so smart pointer can be used like a raw pointer
 - Copying a smart pointer makes a copy of the pointed object (raw pointer not shared) -- ok for containers
   - This requires T implement a proper copy-constructor
 - Call detach() to release (transfer) ownership of pointer without freeing it
   - Note that it's often better to share ownership with SharedPtr rather than manually transfer ownership around
 - \b Caution: Not thread safe
 .

SmartPtr<T[]> is specialized for arrays so the pointer is freed with delete[].
However an array SmartPtr<T[]> cannot be copied (array size isn't known) so that specialization isn't usable in containers, use List or Array instead.

See \ref ManagedPtr "Managed Pointers"

C++11:
 - Move semantics

\tparam  T  Type to use pointer to (not the raw pointer type)

\par Example

\code
#include <evo/ptr.h>
#include <evo/string.h>
#include <evo/io.h>
using namespace evo;
static Console& c = con();

int main() {
    // Create smart pointer, set a value
    SmartPtr<String> ptr(new String);
    *ptr = "testing";    // use dereference operator* like normal pointer

    // Copying smart pointer makes a copy of the object
    {
        SmartPtr<String> ptr2(ptr);
        *ptr2 = "foo";   // dereference ptr2 and change value
    } // ptr2 automatically freed

    // Print ptr value as terminated string (just for example) -- use operator-> like normal pointer
    c.out << ptr->cstr() << NL;

    return 0;
} // ptr automatically freed
\endcode

Output:
\code{.unparsed}
testing
\endcode

\par Example transferring ownership

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
class SmartPtr : public PtrBase<T> {
protected:
    using PtrBase<T>::ptr_;

public:
    typedef SmartPtr<T> This;       ///< This pointer type

    /** Constructor.
     \param  ptr  Pointer to set, NULL for none
    */
    SmartPtr(T* ptr=NULL) {
        ptr_ = ptr;
    }

    /** Copy constructor.
     - This makes a copy of source object using T copy constructor
     - Disabled if array T[]
     .
     \param  src  Source pointer
    */
    SmartPtr(const This& src) {
        ptr_ = new T(*src.ptr_);
    }

    /** Destructor. */
    ~SmartPtr() {
        free();
    }

    /** Copy/Assignment operator.
     - This makes a copy of source object using T copy constructor
     - Disabled if array T[]
     .
     \param  src  Source pointer
     \return      This
    */
    This& operator=(const This& src) {
        free();
        ptr_ = new T(*src.ptr_);
        return *this;
    }

    /** Assignment operator for new pointer.
     \param  ptr  Pointer to set
     \return      This
    */
    This& operator=(T* ptr) {
        free();
        ptr_ = ptr;
        return *this;
    }

#if defined(EVO_CPP11)
    /** Move constructor (C++11).
     \param  src  Source to move
    */
    SmartPtr(This&& src) {
        ptr_ = src.ptr_;
        src.ptr_ = NULL;
    }

    /** Move assignment operator (C++11).
     \param  src  Source to move
     \return      This
    */
    This& operator=(This&& src) {
        free();
        ptr_ = src.ptr_;
        src.ptr_ = NULL;
        return *this;
    }
#endif

    /** Clear (free) pointer and set as null.
     \return  This
    */
    This& clear() {
        free();
        ptr_ = NULL;
        return *this;
    }
    
    /** %Set as null -- same as clear().
     \return  This
    */
    This& set() {
        free();
        ptr_ = NULL;
        return *this;
    }

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

/** Smart pointer to array.
 - Takes ownership of a given pointer and frees the pointer automatically by calling delete[]
 - Overrides pointer operators so smart pointer can be used like a raw pointer
 - An array smart pointer cannot be copied (array size isn't known) so it isn't usable in containers, use List or Array instead
 - For single object smart pointer see: SmartPtr
 - Call detach() to release (transfer) ownership of pointer without freeing it
   - Note that it's often better to share ownership with \link SharedPtr<T[],TSize> SharedPtr<T[]>\endlink rather than manually transfer ownership around
 - \b Caution: Not thread safe
 .

See \ref ManagedPtr "Managed Pointers"

C++11:
 - Move semantics

\tparam  T  Type to use pointer to with array suffix, ex: int[]

\par Example

\code
#include <evo/ptr.h>
using namespace evo;

int main() {
    // Create array smart pointer, set a value (example only, better to use String)
    SmartPtr<int[]> ptr(new int[3]);

    *ptr = 1;         // use dereference operator* like normal pointer
    ptr[1] = 2;       // use operator[] like normal array pointer
    ptr[2] = 3;

    // Copying array smart pointer not supported
    //SmartPtr<int[]> ptr2(ptr);    // Error
    //ptr2 = ptr;                   // Error
} // ptr automatically freed
\endcode
*/
template<class T>
class SmartPtr<T[]> : public PtrBase<T> {
protected:
    using PtrBase<T>::ptr_;

public:
    typedef SmartPtr<T[]> This;

    /** Constructor.
     \param  ptr  Pointer to set, NULL for none
    */
    SmartPtr(T* ptr=NULL) {
        ptr_ = ptr;
    }

    /** Destructor. */
    ~SmartPtr() {
        free();
    }

    /** Assignment operator for new pointer.
     \param  ptr  Pointer to set
     \return      This
    */
    This& operator=(T* ptr) {
        free();
        ptr_ = ptr;
        return *this;
    }

#if defined(EVO_CPP11)
    /** Move constructor (C++11).
     \param  src  Source to move
    */
    SmartPtr(This&& src) {
        ptr_ = src.ptr_;
        src.ptr_ = NULL;
    }

    /** Move assignment operator (C++11).
     \param  src  Source to move
     \return      This
    */
    This& operator=(This&& src) {
        free();
        ptr_ = src.ptr_;
        src.ptr_ = NULL;
        return *this;
    }
#endif

    /** Clear (free) pointer and set as null.
     \return  This
    */
    This& clear() {
        free();
        ptr_ = NULL;
        return *this;
    }

    /** %Set as null -- same as clear().
     \return  This
    */
    This& set() {
        free();
        ptr_ = NULL;
        return *this;
    }

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
            delete [] ptr_;
    }

private:
    // Disable copying
    explicit SmartPtr(const This&) EVO_ONCPP11(= delete);
    This& operator=(const This&) EVO_ONCPP11(= delete);
};

///////////////////////////////////////////////////////////////////////////////

/** Shared smart pointer to single object.
 - Similar to SmartPtr except copying uses reference counting to make shared copies
 - Takes ownership of a given pointer and frees the pointer automatically by calling delete when no longer used
   - A reference count is incremented for each copy, and decremented when each copy is destroyed
   - The last owner automatically frees the pointer
 - Overrides pointer operators so shared smart pointer can be used like a raw pointer
 - Copying a shared smart pointer makes a shared copy, incrementing the reference count -- ok for containers
   - Modifying a shared object affects all pointers referencing it
 - The following method requires T implement a proper copy-constructor: unshare()
 - \b Caution: Not thread safe
 .

\link SharedPtr<T[],TSize> SharedPtr<T[]>\endlink is specialized for arrays so the pointer is freed with delete[]. However an array \link SharedPtr<T[],TSize> SharedPtr<T[]>\endlink doesn't support unshare() (array size isn't known).

See \ref ManagedPtr "Managed Pointers"

C++11:
 - Move semantics

\tparam  T  Type to use pointer to (not the raw pointer type)

\par Example

\code
#include <evo/ptr.h>
#include <evo/string.h>
#include <evo/io.h>
using namespace evo;
static Console& c = con();

int main() {
    // Create shared pointer, set a value
    SharedPtr<String> ptr(new String);
    *ptr = "testing";    // use dereference operator* like normal pointer

    // Copying shared pointer makes a shared copy
    {
        SharedPtr<String> ptr2(ptr);
        *ptr2 = "foo";   // dereference ptr2 and change value (affects ptr as well)
    }

    // Print ptr value as terminated string (just for example) -- use operator-> like normal pointer
    c.out << ptr->cstr() << NL;

    // May use Ptr to reference a shared pointer without sharing it (does not increment reference count)
    {
        Ptr<String> ptr2(ptr);  // this can be dangerous if Ptr outlives SharedPtr
        *ptr2 = "bar";          // dereference ptr2 and change value again (affects ptr as well)
    }

    // Print ptr value as terminated string (just for example) -- use operator-> like normal pointer
    c.out << ptr->cstr() << NL;

    return 0;
} // ptr automatically freed
\endcode

Output:
\code{.unparsed}
foo
bar
\endcode
*/
template<class T,class TSize=SizeT>
class SharedPtr : public PtrBase<T> {
protected:
    using PtrBase<T>::ptr_;

public:
    typedef SharedPtr<T,TSize> This;        ///< This pointer type

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
    SharedPtr(const This& src) {
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
    This& operator=(const This& src) {
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
    This& operator=(T* ptr) {
        clear();
        if (ptr != NULL) {
            ptr_ = ptr;
            if (refs_ == NULL)
                refs_ = new TSize;
            *refs_ = 1;
        }
        return *this;
    }

#if defined(EVO_CPP11)
    /** Move constructor (C++11).
     \param  src  Source to move
    */
    SharedPtr(This&& src) {
        ptr_ = src.ptr_;
        refs_ = src.refs_;
        src.ptr_ = NULL;
        src.refs_ = NULL;
    }

    /** Move assignment operator (C++11).
     \param  src  Source to move
     \return      This
    */
    This& operator=(This&& src) {
        free();
        ptr_ = src.ptr_;
        refs_ = src.refs_;
        src.ptr_ = NULL;
        src.refs_ = NULL;
        return *this;
    }
#endif

    /** Release pointer and set as null.
     - This will decrement the reference count, and free the pointer if not shared
     .
     \return  This
    */
    This& clear() {
        if (ptr_ != NULL) {
            if (--(*refs_) == 0)
                delete ptr_; // leave refs_ allocated for later
            else
                refs_ = NULL; // detach from shared
            ptr_  = NULL;
        }
        return *this;
    }

    /** %Set as null -- same as clear().
     \return  This
    */
    This& set()
        { return clear(); }

    /** Unshare pointer by setting as a new copy, if shared.
     - Disabled if array T[]
     .
     \return  This
    */
    This& unshare() {
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

/** Shared smart pointer to array.
 - Similar to SmartPtr<T[]> except copying uses reference counting to make shared copies
 - Takes ownership of a given pointer and frees the pointer automatically by calling delete when no longer used
   - A reference count is incremented for each copy, and decremented when each copy is destroyed
   - The last owner automatically frees the pointer
 - Overrides pointer operators so shared smart pointer can be used like a raw pointer
 - Copying a shared smart pointer makes a shared copy, incrementing the reference count -- ok for containers
   - Modifying a shared object affects all pointers referencing it
 - For single object shared smart pointer see: SharedPtr
 - \b Caution: Not thread safe
 .

See \ref ManagedPtr "Managed Pointers"

C++11:
 - Move semantics

\tparam  T  Type to use pointer to with array suffix, ex: int[]

\par Example

\code
#include <evo/ptr.h>
#include <evo/string.h>
#include <evo/io.h>
using namespace evo;
static Console& c = con();

int main() {
    // Create shared pointer, set a value
    SharedPtr<String[]> ptr(new String[2]);
    *ptr   = "testing";     // use dereference operator* like normal pointer
    ptr[1] = "bar";         // use dereference operator[] like normal pointer

    // Copying shared pointer makes a shared copy
    {
        SharedPtr<String[]> ptr2(ptr);
        ptr2[0] = "foo";    // dereference ptr2 and change value (affects ptr as well)
    }

    // Print ptr first value as terminated string (just for example) -- use operator-> like normal pointer
    c.out << ptr->cstr() << NL;

    // Print ptr second value as terminated string (just for example) -- use operator[]() like normal pointer
    c.out << ptr[1].cstr() << NL;

    return 0;
} // ptr automatically freed
\endcode

Output:
\code{.unparsed}
foo
bar
\endcode
*/
template<class T,class TSize>
class SharedPtr<T[],TSize> : public PtrBase<T> {
protected:
    using PtrBase<T>::ptr_;

public:
    typedef SharedPtr<T[],TSize> This;

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
    SharedPtr(const This& src) {
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
    This& operator=(const This& src) {
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
    This& operator=(T* ptr) {
        clear();
        if (ptr != NULL) {
            ptr_ = ptr;
            if (refs_ == NULL)
                refs_ = new TSize;
            *refs_ = 1;
        }
        return *this;
    }

#if defined(EVO_CPP11)
    /** Move constructor (C++11).
     \param  src  Source to move
    */
    SharedPtr(This&& src) {
        ptr_ = src.ptr_;
        refs_ = src.refs_;
        src.ptr_ = NULL;
        src.refs_ = NULL;
    }

    /** Move assignment operator (C++11).
     \param  src  Source to move
     \return      This
    */
    This& operator=(This&& src) {
        free();
        ptr_ = src.ptr_;
        refs_ = src.refs_;
        src.ptr_ = NULL;
        src.refs_ = NULL;
        return *this;
    }
#endif

    /** Release pointer and set as null.
     - This will decrement the reference count, and free the pointer if not shared
     .
     \return  This
    */
    This& clear() {
        if (ptr_ != NULL) {
            if (--(*refs_) == 0) {
                delete [] ptr_; // leave refs_ allocated for later
                ptr_ = NULL;
            } else
                refs_ = NULL; // detach from shared
        }
        return *this;
    }

    /** %Set as null -- same as clear().
     \return  This
    */
    This& set()
        { return clear(); }

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
                { delete [] ptr_; delete refs_; }
        } else if (refs_ != NULL)
            delete refs_;
    }

private:
    // Disabled if array
    This& unshare();
};

///////////////////////////////////////////////////////////////////////////////

/** Dumb pointer to single object.
 - Useful for making it clear the pointer is not owned here, and is freed elsewhere
   - Does not take ownership of any pointer, does not automatically free anything
 - Also useful for distinguishing between a static immutable data pointer (ex: string literal) and a dumb managed pointer
   - Evo containers always copy from a dumb pointer, but may reference a raw pointer (assuming static immutable data)
 - Overrides pointer operators so dumb pointer can be used like a raw pointer/array
 - Copying a dumb pointer makes a copy of the pointer itself, referencing the same object(s) -- only safe with containers if pointer outlives the container
 - Ptr<T[]> is specialized for arrays for consistency with other managed pointers
 - \b Caution: Not thread safe
 .

See \ref ManagedPtr "Managed Pointers"

C++11:
 - Move semantics

\tparam  T  Type to use pointer to (not the raw pointer type)

\par Example

\code
#include <evo/ptr.h>
using namespace evo;

// It's clear here that this function does not take ownership of ptr
void func(Ptr<int> ptr) {
    // Use dumb ptr
}

int main() {
    // Create and set a smart pointer
    SmartPtr<int> ptr(new int);
    *ptr = 1;

    // Call function that uses the pointer
    func(ptr);

    return 0;
}
\endcode
*/
template<class T>
class Ptr : public PtrBase<T> {
protected:
    using PtrBase<T>::ptr_;

public:
    typedef Ptr<T>     This;        ///< This pointer type
    typedef PtrBase<T> Base;        ///< Base type

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
    Ptr(const This& src)
        { ptr_ = src.ptr_; }

    /** Copy constructor to reference pointer.
     \param  src  Source pointer
    */
    Ptr(const Base& src)
        { ptr_ = src.ptr_; }

    /** Copy/Assignment operator.
     \param  src  Source pointer
     \return      This
    */
    This& operator=(const This& src) {
        ptr_ = src.ptr_;
        return *this;
    }

    /** Assignment operator to reference pointer.
     \param  src  Source pointer
     \return      This
    */
    This& operator=(const Base& src) {
        ptr_ = src.ptr_;
        return *this;
    }

    /** Assignment operator for raw pointer.
     \param  ptr  Pointer to set
     \return      This
    */
    This& operator=(T* ptr) {
        ptr_ = ptr;
        return *this;
    }

    /** Assignment operator for raw pointer.
     \param  ptr  Pointer to set
     \return      This
    */
    This& operator=(const T* ptr) {
        ptr_ = (T*)ptr;
        return *this;
    }

#if defined(EVO_CPP11)
    /** Move constructor (C++11).
     \param  src  Source to move
    */
    Ptr(This&& src) {
        ptr_ = src.ptr_;
        src.ptr_ = NULL;
    }

    /** Move assignment operator (C++11).
     \param  src  Source to move
     \return      This
    */
    This& operator=(This&& src) {
        ptr_ = src.ptr_;
        src.ptr_ = NULL;
        return *this;
    }
#endif

    /** Clear pointer, setting as null.
     \return  This
    */
    This& clear() {
        ptr_ = NULL;
        return *this;
    }

    /** %Set as null -- same as clear().
     \return  This
    */
    This& set() {
        ptr_ = NULL;
        return *this;
    }

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

/** Dumb pointer to array.
 - Useful for making it clear the pointer is not owned here, and is freed elsewhere
   - Does not take ownership of any pointer, does not automatically free anything
 - Also useful for distinguishing between a static immutable data pointer (ex: string literal) and a dumb managed pointer
   - Evo containers always copy from a dumb pointer, but may reference a raw pointer (assuming static immutable data)
 - Overrides pointer operators so dumb pointer can be used like a raw pointer/array
 - Copying a dumb pointer makes a copy of the pointer itself, referencing the same object(s) -- only safe with containers if pointer outlives the container
 - For single object dumb pointer see: Ptr
 - \b Caution: Not thread safe
 .

See \ref ManagedPtr "Managed Pointers"

C++11:
- Move semantics

\tparam  T  Type to use pointer to with array suffix, ex: int[]

\par Example

\code
#include <evo/ptr.h>
using namespace evo;

// It's clear here that this function does not take ownership of ptr
void func(Ptr<int[]> ptr) {
    // Use dumb array ptr
}

int main() {
    // Create a smart pointer, set to value
    SmartPtr<int[]> ptr(new int[3]);
    ptr[0] = 1;
    ptr[1] = 2;
    ptr[2] = 3;

    // Call function that uses the pointer
    func(ptr);

    return 0;
}
\endcode
*/
template<class T>
class Ptr<T[]> : public PtrBase<T> {
protected:
    using PtrBase<T>::ptr_;

public:
    typedef Ptr<T[]>   This;        ///< This pointer type
    typedef PtrBase<T> Base;        ///< Base type

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
    Ptr(const This& src)
        { ptr_ = src.ptr_; }

    /** Copy constructor to reference pointer.
     \param  src  Source pointer
    */
    Ptr(const Base& src)
        { ptr_ = src.ptr_; }

    /** Copy/Assignment operator.
     \param  src  Source pointer
     \return      This
    */
    This& operator=(const This& src)
        { ptr_ = src.ptr_; return *this; }

    /** Assignment operator to reference pointer.
     \param  src  Source pointer
     \return      This
    */
    This& operator=(const Base& src)
        { ptr_ = src.ptr_; return *this; }

    /** Assignment operator for raw pointer.
     \param  ptr  Pointer to set
     \return      This
    */
    This& operator=(T* ptr)
        { ptr_ = ptr; return *this; }

    /** Assignment operator for raw pointer.
     \param  ptr  Pointer to set
     \return      This
    */
    This& operator=(const T* ptr)
        { ptr_ = (T*)ptr; return *this; }

#if defined(EVO_CPP11)
    /** Move constructor (C++11).
     \param  src  Source to move
    */
    Ptr(This&& src) {
        ptr_ = src.ptr_;
        src.ptr_ = NULL;
    }

    /** Move assignment operator (C++11).
     \param  src  Source to move
     \return      This
    */
    This& operator=(This&& src) {
        ptr_ = src.ptr_;
        src.ptr_ = NULL;
        return *this;
    }
#endif

    /** Clear pointer, setting as null.
     \return  This
    */
    This& clear()
        { ptr_ = NULL; return *this; }

    /** %Set as null -- same as clear().
     \return  This
    */
    This& set()
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

///////////////////////////////////////////////////////////////////////////////
//@}
}
#endif
