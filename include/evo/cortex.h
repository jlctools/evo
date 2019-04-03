// Evo C++ Library
/* Copyright 2019 Justin Crowell
Distributed under the BSD 2-Clause License -- see included file LICENSE.txt for details.
*/
///////////////////////////////////////////////////////////////////////////////
/** \file cortex.h Evo Cortex class. */
#pragma once
#ifndef INCL_evo_cortex_h
#define INCL_evo_cortex_h

#if defined(EVO_CORTEX_DEFAULT_HASHMAP)
    #include "maphash.h"
    #define EVO_CORTEX_IMPL_DEFAULT =MapHash<String,CortexItemBase*>
#elif !defined(EVO_CORTEX_NO_DEFAULT)
    #include "maplist.h"
    #define EVO_CORTEX_IMPL_DEFAULT =MapList<String,CortexItemBase*>
#else
    #define EVO_CORTEX_IMPL_DEFAULT
#endif
#include "impl/iter.h"
#include "meta.h"

namespace evo {
/** \addtogroup EvoTools */
//@{

#if !defined(EVO_CORTEX_DYNAMIC_CAST)
    /** Set whether Cortex uses `dynamic_cast`.
     - This defaults to 1, to use `static_cast` instead `define EVO_CORTEX_DYNAMIC_CAST 0` (before including cortex.h)
     - \b Caution: `static_cast` does not do runtime checks and may be considered more dangerous, though `dynamic_cast` doesn't catch all invalid casts either
     .
    */
    #define EVO_CORTEX_DYNAMIC_CAST 1
#endif

/** \cond impl */
#if EVO_CORTEX_DYNAMIC_CAST
    #define EVO_CORTEX_IMPL_CAST dynamic_cast
#else
    #define EVO_CORTEX_IMPL_CAST static_cast
#endif
/** \endcond */

///////////////////////////////////////////////////////////////////////////////

/** Base class for use with Cortex.
 - Each item type stored in a Cortex must inherit this
 - See Cortex
 .
*/
struct CortexItemBase {
    /** Constructor. */
    CortexItemBase() : cortex_prev(NULL), cortex_next(NULL) {
    }

    /** Copy constructor.
    */
    CortexItemBase(const CortexItemBase& src) : cortex_prev(src.cortex_prev), cortex_next(src.cortex_next) {
    }

    /** Assignment operator.
    */
    CortexItemBase& operator=(const CortexItemBase& src) {
        cortex_prev = src.cortex_prev;
        cortex_next = src.cortex_next;
        return *this;
    }

    /** Destructor. */
    virtual ~CortexItemBase() {
    }

    CortexItemBase* cortex_prev;    ///< Pointer to previous item in list -- used by Cortex, do not modify
    CortexItemBase* cortex_next;    ///< Pointer to next item in list -- used by Cortex, do not modify
};

///////////////////////////////////////////////////////////////////////////////

/** Collection of different context items using a form of type erasure to reduce inter-dependencies.
 - This is used to help create a type of modular plug-in architecture, where items (modules) may access the Cortex without coupling to all the types stored in it
   - Large applications can use this to break up state into separate modules and minimize dependencies between them
   - An example is a server using a "request" context -- as more functionality is added, more state gets added to the "request", and storing state under a Cortex
     breaks up request state into separate modules while giving them an interface to find each other when needed
   - Ideally dependencies between items (modules) are minimized, and if possible only \#include other module headers from a module source (.cpp) file
 - Iteration order is preserved and matches the order of items added, regardless of the underlying map type used
 - This can be expanded on by creating a custom base class for items (derived from CortexItemBase), where you can apply design patterns like Observer, Visitor, Mediator, etc
 - This is implemented by wrapping a Map container, where map values are abstract (base) class pointers (owned and freed by this container)
 - See also: ModuleBase
 .
 \tparam  T  Underlying Map type to use, values must be `CortexItemBase*` (or pointer to derived type) -- defaults to MapList using String keys

\par Example

\code
#include <evo/cortex.h>
#include <evo/io.h>
using namespace evo;

static const char* MODULE_FOO = "foo";
struct ModuleFoo : CortexItemBase {
    void foo() {
        // ...
    }
};

static const char* MODULE_BAR = "bar";
struct ModuleBar : CortexItemBase {
    // Move implementation to a source (.cpp) file to decouple the dependency on ModuleFoo
    void bar(Cortex<>& modules) {
        modules.get_create<ModuleFoo>(MODULE_FOO).foo();
    }
};

int main() {
    Cortex<> modules;
    modules.reserve(2);
    try {
        // Uncomment to create modules in advance, otherwise created on demand
        //modules.get_create<ModuleFoo>(MODULE_FOO);
        //modules.get_create<ModuleBar>(MODULE_BAR);

        // Call ModuleBar method
        modules.get_create<ModuleBar>(MODULE_BAR).bar(modules);
    } EVO_CATCH(return 1);

    return 0;
}
\endcode
*/
template<class T EVO_CORTEX_IMPL_DEFAULT>
class Cortex {
public:
    typedef Cortex<T> This;                             ///< This type
    typedef T MapType;                                  ///< Map type (from `T`)
    typedef typename T::Key Key;                        ///< %Map key type (from `T::Key`)
    typedef typename T::Value Value;                    ///< %Map value type (from `T::Value`)
    typedef typename T::Size Size;                      ///< %Map size type (from `T::Size`)
    typedef typename RemoveExtents<Value>::Type Item;   ///< Base item type (from `T::Value` but without pointer)

    /** Constructor. */
    Cortex() : first_(NULL), last_(NULL) {
        #if defined(EVO_CPP11)
            static_assert(IsPointer<Value>::value, "Cortex Map Value must be a pointer to a type derived from evo::CortexItemBase");
        #else
            STATIC_ASSERT( IsPointer<Value>::value, ERROR__Cortex_Map_Value_must_be_a_pointer );
        #endif
    }

    /** Destructor. */
    ~Cortex() {
        clear();
    }

    /** Get cortex size as item count.
     \return  Item count
    */
    Size size() const {
        return map_.size();
    }

    /** Reserve space for new items.
     \param  size  Number of new items to reserve space
     \return       Reference to this
    */
    This& reserve(Size size) {
        map_.reserve(size);
        return *this;
    }

    /** Remove all items.
     \return  Reference to this
    */
    This& clear() {
        Item* item = first_;
        for (Item* temp; item != NULL; ) {
            temp = item;
            item = item->cortex_next;
            delete temp;
        }
        first_ = last_ = NULL;
        map_.clear();
        return *this;
    }

    /** Remove item by key.
     \param  key  Key to find and remove
     \return      Whether key was removed, false if not found
    */
    bool remove(const Key& key) {
        typename MapType::IterM iter(map_.iterM(key));
        if (iter) {
            Item* item = iter->second;
            assert( item != NULL );
            if (item == first_) {
                if (item == last_) {
                    first_ = last_ = NULL;
                } else {
                    first_ = item->cortex_next;
                    assert( first_ != NULL );
                    first_->cortex_prev = NULL;
                }
            } else if (item == last_) {
                last_ = item->cortex_prev;
                assert( last_ != NULL );
                last_->cortex_next = NULL;
            } else {
                item->cortex_prev->cortex_next = item->cortex_next;
                item->cortex_next->cortex_prev = item->cortex_prev;
            }
            delete item;            
            return map_.remove(iter);
        }
        return false;
    }

    /** Get stored item using key, create if needed.
     - If the item doesn't exist this creates it first
     - This uses `dynamic_cast` to cast the base type to the requested type
       - This has some runtime overhead, and throws `std::bad_cast` if the cast fails
       - To use `static_cast` instead `define EVO_CORTEX_DYNAMIC_CAST 0` (before including this)
     - \b Caution: Results are undefined if the concrete type (`ItemT`) doesn't match the current object for key
     .
     \tparam  ItemT  Item type for underlying key
     \param  key  Key to find item
     \return      Reference to item
    */
    template<class ItemT>
    ItemT& get_create(const Key& key) {
        bool created = false;
        Item*& item = map_.get(key, &created);
        if (created) {
            item = new ItemT;
            item->cortex_prev = last_;
            item->cortex_next = NULL;
            if (last_ == NULL) {
                first_ = last_ = item;
            } else {
                if (last_ != NULL)
                    last_->cortex_next = item;
                last_ = item;
            }
        }
        return *(EVO_CORTEX_IMPL_CAST <ItemT*>(item));
    }

    /** Get stored item pointer using key (mutable).
     - This uses `dynamic_cast` to cast the base type to the requested type
       - This has some runtime overhead, and throws `std::bad_cast` if the cast fails
       - To use `static_cast` instead `define EVO_CORTEX_DYNAMIC_CAST 0` (before including this)
     - \b Caution: Results are undefined if the concrete type (`ItemT`) doesn't match the current object for key
     .
     \tparam  ItemT  Item type for underlying key
     \param  key  Key to find item
     \return      Pointer to item found, NULL if not found
    */
    template<class ItemT>
    ItemT* get(const Key& key) {
        Value* item = map_.findM(key);
        if (item == NULL)
            return NULL;
        return EVO_CORTEX_IMPL_CAST <ItemT*>(*item);
    }

    /** Get stored item pointer using key (const).
     - This uses `dynamic_cast` to cast the base type to the requested type
       - This has some runtime overhead, and throws `std::bad_cast` if the cast fails
       - To use `static_cast` instead `define EVO_CORTEX_DYNAMIC_CAST 0` (before including this)
     - \b Caution: Results are undefined if the concrete type (`ItemT`) doesn't match the current object for key
     .
     \tparam  ItemT  Item type for underlying key
     \param  key  Key to find item
     \return      Pointer to item found, NULL if not found
    */
    template<class ItemT>
    const ItemT* get_const(const Key& key) const {
        const Value* item = map_.find(key);
        if (item == NULL)
            return NULL;
        return EVO_CORTEX_IMPL_CAST <const ItemT*>(*item);
    }

private:
    Cortex(const Cortex&) EVO_ONCPP11(= delete);
    Cortex& operator=(const Cortex&) EVO_ONCPP11(= delete);

    Item* first_;
    Item* last_;
    MapType map_;
};

///////////////////////////////////////////////////////////////////////////////

#undef EVO_CORTEX_IMPL_CAST

///////////////////////////////////////////////////////////////////////////////
//@}
}
#endif
