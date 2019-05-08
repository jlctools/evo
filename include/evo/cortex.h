// Evo C++ Library
/* Copyright 2019 Justin Crowell
Distributed under the BSD 2-Clause License -- see included file LICENSE.txt for details.
*/
///////////////////////////////////////////////////////////////////////////////
/** \file cortex.h Evo Cortex class. */
#pragma once
#ifndef INCL_evo_cortex_h
#define INCL_evo_cortex_h

#include "ptr.h"
#if defined(EVO_CORTEX_DEFAULT_HASHMAP)
    #include "maphash.h"
    #define EVO_CORTEX_IMPL_DEFAULT =MapHash<String,SharedPtr<PureBase> >
#elif !defined(EVO_CORTEX_NO_DEFAULT)
    #include "maplist.h"
    #define EVO_CORTEX_IMPL_DEFAULT =MapList<String,SharedPtr<PureBase> >
#else
    #define EVO_CORTEX_IMPL_DEFAULT
#endif

/** Helper for a Cortex type using MapList with given KEY and BASE types.
 - When using this: \#include <evo/maplist.h>
 - This evaluates to a Cortex type with the template arguments filled in
 - Use with typedef like this:
   \code
    typedef EVO_CORTEX_MAPLIST(String, PureBase) MyCortex;
   \endcode
 .
*/
#define EVO_CORTEX_MAPLIST(KEY, BASE) evo::Cortex< evo::MapList<KEY, evo::SharedPtr<BASE> > >

/** Helper for a Cortex type using MapHash with given KEY and BASE types.
 - When using this: \#include <evo/maphash.h>
 - This evaluates to a Cortex type with the template arguments filled in
 - Use with typedef like this:
   \code
    typedef EVO_CORTEX_MAPHASH(String, PureBase) MyCortex;
   \endcode
 .
*/
#define EVO_CORTEX_MAPHASH(KEY, BASE) evo::Cortex< evo::MapHash<KEY, evo::SharedPtr<BASE> > >

namespace evo {
/** \addtogroup EvoTools */
//@{

#if !defined(EVO_CORTEX_DYNAMIC_CAST)
    /** %Set whether Cortex uses `dynamic_cast`.
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

/** A Map with a collection of independent abstract context items.
 - This creates a kind of modular plug-in architecture, where items (modules or contexts) in the the Cortex may be accessed without depending on all the concrete types stored in it
 - Expand further by adding "hook" methods to the base interface, where you can apply design patterns like Observer, Visitor, Mediator, etc
   - PureBase is used as the base interface by default, an alternative base type must either inherit from PureBase or at least have a virtual destructor
   - See helper macros for making a custom Cortex type:
     - EVO_CORTEX_MAPLIST()
     - EVO_CORTEX_MAPHASH()
 - This basically wraps and inherits a Map container, where each value is a SharedPtr to an abstract base class such as PureBase
 - Though you can use the Map interface, the Cortex interface is preferred:
     - create()
     - get_create()
     - getptr()
     - getptr_const()
 - See also alternative approach with: CortexModuleBase
 .
 \tparam  T  Map type to use, values must be a `SharedPtr<PureBase>` (or SharedPtr to derived type, or at least have a virtual destructor) -- defaults to MapList using String keys
              - Default can be changed to MapHash with String keys by defining `EVO_CORTEX_DEFAULT_HASHMAP` (before including this)

\par Examples

This example shows a couple modules store in a Cortex, one of which has a dependency on the other.

\code
#include <evo/cortex.h>
#include <evo/io.h>
using namespace evo;

// Define module "foo"
static const char* MODULE_FOO = "foo";
struct ModuleFoo : PureBase {
    void foo() {
        // ...
    }
};

// Define module "bar"
static const char* MODULE_BAR = "bar";
struct ModuleBar : PureBase {
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
        //modules.create<ModuleFoo>(MODULE_FOO);
        //modules.create<ModuleBar>(MODULE_BAR);

        // Call ModuleBar method
        modules.get_create<ModuleBar>(MODULE_BAR).bar(modules);
    } EVO_CATCH(return 1);

    return 0;
}
\endcode

This example shows an interface with an event/hook: `event_thing()`

\code
#include <evo/cortex.h>
#include <evo/maplist.h>
#include <evo/io.h>
using namespace evo;

struct ModuleBase : PureBase {
    // Hook for when the "thing" happens
    virtual void event_thing() = 0;
};

// Define MyCortex type using ModuleBase interface
typedef EVO_CORTEX_MAPLIST(String, ModuleBase) MyCortex;

// Define module "foo"
static const char* MODULE_FOO = "foo";
struct ModuleFoo : ModuleBase {
    void event_thing() {
        con().out << "The thing happened!" << NL;
    }
};

int main() {
    MyCortex modules;

    // Create module "foo"
    modules.create<ModuleFoo>(MODULE_FOO);

    // Notify each module that the "thing" is happening
    for (MyCortex::IterM iter(modules); iter; ++iter)
        iter->value()->event_thing();

    // Notify again using C++11 loop (if supported)
    EVO_ONCPP11(
        for (auto& mod : modules)
            mod.value()->event_thing();
    )

    return 0;
}
\endcode
*/
template<class T EVO_CORTEX_IMPL_DEFAULT>
class Cortex : public T {
public:
    typedef Cortex<T> This;                     ///< This type
    typedef T MapType;                          ///< Map type (from `T`)
    typedef typename T::Value::Item ValueBase;  ///< Value base type, normally PureBase

#if defined(EVO_OLDCC)
    typedef typename T::Key Key;
    typedef typename T::Value Value;
#else
    using typename T::Key;
    using typename T::Value;
#endif

    /** Constructor. */
    Cortex() {
        #if defined(EVO_CPP11)
            static_assert(IsSmartPtr<Value>::value, "Cortex Map Value must be a SharedPtr to a type derived from evo::PureBase");
        #else
            STATIC_ASSERT( IsSmartPtr<Value>::value, ERROR__Cortex_Map_Value_must_be_a_SharedPtr_to_type_derived_from_PureBase );
        #endif
    }

    /** Create item for key, if needed.
     - If the item doesn't exist this creates it, otherwise this is a no-op
     .
     \tparam  ItemT  Item type for underlying key
     \param  key  Key to find item
     \return      This
    */
    template<class ItemT>
    This& create(const Key& key) {
        bool created = false;
        Value& item = MapType::get(key, &created);
        if (created)
            item = new ItemT;
        return *this;
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
        Value& item = MapType::get(key, &created);
        if (created)
            item = new ItemT;
        return *(EVO_CORTEX_IMPL_CAST <ItemT*>(item.ptr()));
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
    ItemT* getptr(const Key& key) {
        Value* item = MapType::findM(key);
        if (item == NULL)
            return NULL;
        return EVO_CORTEX_IMPL_CAST <ItemT*>(item->ptr());
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
    const ItemT* getptr_const(const Key& key) const {
        const Value* item = MapType::find(key);
        if (item == NULL)
            return NULL;
        return EVO_CORTEX_IMPL_CAST <const ItemT*>(item->ptr());
    }

private:
    Cortex(const Cortex&) EVO_ONCPP11(= delete);
    Cortex& operator=(const Cortex&) EVO_ONCPP11(= delete);
};

///////////////////////////////////////////////////////////////////////////////

/** A simple base class for defining a module using a similar pattern to Cortex, but without the dynamic Map.
 - Use \ref CortexModulePtr to store a pointer (or array of pointers) to a module inheriting this class
 - Consider using CortexModuleAsBase to create helpers for getting a module from a \ref CortexModulePtr
 - See also: Cortex
 .

\par Example

Here are some examples using alternatives to Cortex.

This example uses a struct to hold modules.

\code
#include <evo/cortex.h>
using namespace evo;

struct Modules {
    CortexModulePtr foo, bar;
};

struct ModuleFoo : CortexModuleBase {
    void foo() {
        // ...
    }
};
typedef CortexModuleAsBase<ModuleFoo> ModuleAsFoo;

struct ModuleBar : CortexModuleBase {
    // Move implementation to a source (.cpp) file to decouple the dependency on ModuleFoo
    void bar(Modules& modules) {
        ModuleAsFoo::get(modules.foo).foo();
    }
};
typedef CortexModuleAsBase<ModuleBar> ModuleAsBar;

int main() {
    Modules modules;

    // Uncomment to create modules in advance, otherwise created on demand
    //modules.foo = new ModuleFoo;
    //modules.bar = new ModuleBar;

    // Call ModuleBar method
    ModuleAsBar::get(modules.bar).bar(modules);

    return 0;
}
\endcode

This example uses a raw array -- List or Array can also be used.

\code
#include <evo/cortex.h>
using namespace evo;

// ID for each module, used as array index
enum ModuleId {
    MODULE_FOO,     // ID for module Foo
    MODULE_BAR,     // ID for module Bar
    MODULE_COUNT    // Guard used as number of modules
};

struct ModuleFoo : CortexModuleBase {
    void foo() {
        // ...
    }
};
typedef CortexModuleAsBase<ModuleFoo> ModuleAsFoo;

struct ModuleBar : CortexModuleBase {
    // Move implementation to a source (.cpp) file to decouple the dependency on ModuleFoo
    void bar(CortexModulePtr modules[]) {
        ModuleAsFoo::get(modules[MODULE_FOO]).foo();
    }
};
typedef CortexModuleAsBase<ModuleBar> ModuleAsBar;

int main() {
    CortexModulePtr modules[MODULE_COUNT];

    // Uncomment to create modules in advance, otherwise created on demand
    //modules[MODULE_FOO] = new ModuleFoo;
    //modules[MODULE_BAR] = new ModuleBar;

    // Call ModuleBar method
    ModuleAsBar::get(modules[MODULE_BAR]).bar(modules);

    return 0;
}
\endcode
*/
struct CortexModuleBase {
    /** Destructor. */
    virtual ~CortexModuleBase() {
    }

    /** Get this module as a concrete type.
     - This uses `dynamic_cast` to cast the base type to the requested type
       - This has some runtime overhead, and throws `std::bad_cast` if the cast fails
       - To use `static_cast` instead `define EVO_CORTEX_DYNAMIC_CAST 0` (before including this)
     - \b Caution: Results are undefined if the concrete type (`ItemT`) doesn't match the current object for key
     .
     \tparam  T  Concrete module type to cast this to
     \return     Concrete reference to this object (`T&`)
    */
    template<class T>
    T& as() {
        return * EVO_CORTEX_IMPL_CAST <T*>(this);
    }
};

///////////////////////////////////////////////////////////////////////////////

/** Smart/Shared pointer to a class inheriting CortexModuleBase.
 - This defines a common interface for a collection of modules using a similar pattern as Cortex
 - If you build on CortexModuleBase with another base class, consider defining a SharedPtr alias like this for that type
 - See CortexModuleBase
*/
typedef SharedPtr<CortexModuleBase> CortexModulePtr;

///////////////////////////////////////////////////////////////////////////////

/** Helper for getting a concrete module from a \ref CortexModulePtr (or similar pointer).
 - This is a helper for calling CortexModuleBase::as() in a cleaner way (without a template argument)
 - Use this to create a helper type for each module inheriting CortexModuleBase like this, where `ModuleFoo` is the concrete module type:
   \code
    typedef CortexModuleAsBase<ModuleFoo> ModuleAsFoo;
   \endcode
 - Call get() to get an instance (create if needed), or getptr() to get instance pointer
 .
 \tparam  T  Concrete type to use, must inherit CortexModuleBase
*/
template<class T>
struct CortexModuleAsBase {
    /** Get module reference from stored pointer, create if needed.
     - This uses `dynamic_cast` to cast the base type to the requested type
       - This has some runtime overhead, and throws `std::bad_cast` if the cast fails
       - To use `static_cast` instead `define EVO_CORTEX_DYNAMIC_CAST 0` (before including this)
     - \b Caution: Results are undefined if the concrete type (`ItemT`) doesn't match the current object for key
     .
     \tparam  U  Smart pointer type -- inferred from argument
     \param  ptr  Smart pointer type holding the object, if null then a new object is created and stored here
     \return      Reference to value from `ptr`
    */
    template<class U>
    static T& get(U& ptr) {
        if (!ptr)
            ptr = new T;
        return *EVO_CORTEX_IMPL_CAST <T*>(ptr.ptr());
    }

    /** Get module pointer from stored pointer.
     - This uses `dynamic_cast` to cast the base type to the requested type
       - This has some runtime overhead, and throws `std::bad_cast` if the cast fails
       - To use `static_cast` instead `define EVO_CORTEX_DYNAMIC_CAST 0` (before including this)
     - \b Caution: Results are undefined if the concrete type (`ItemT`) doesn't match the current object for key
     .
     \tparam  U  Smart pointer type -- inferred from argument
     \param  ptr  Pointer type holding the object
     \return      Pointer to value from `ptr`, NULL if none
    */
    template<class U>
    static T* getptr(U& ptr) {
        if (ptr)
            return EVO_CORTEX_IMPL_CAST <T*>(ptr.ptr());
        return NULL;
    }
};

///////////////////////////////////////////////////////////////////////////////

#undef EVO_CORTEX_IMPL_CAST

///////////////////////////////////////////////////////////////////////////////
//@}
}
#endif
