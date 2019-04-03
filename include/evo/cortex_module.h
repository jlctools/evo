// Evo C++ Library
/* Copyright 2019 Justin Crowell
Distributed under the BSD 2-Clause License -- see included file LICENSE.txt for details.
*/
///////////////////////////////////////////////////////////////////////////////
/** \file cortex_module.h Evo ModuleBase class, Cortex alternative. */
#pragma once
#ifndef INCL_evo_cortex_module_h
#define INCL_evo_cortex_module_h

#include "ptr.h"

namespace evo {
/** \addtogroup EvoTools */
//@{

#if !defined(EVO_MODULEBASE_DYNAMIC_CAST)
    /** Set whether ModuleBase and ModuleAsBase use `dynamic_cast`.
     - This defaults to 1 -- to use `static_cast` instead `define EVO_MODULEBASE_DYNAMIC_CAST 0` (before including cortex_module.h)
     - \b Caution: `static_cast` does not do runtime checks and may be considered more dangerous, though `dynamic_cast` doesn't catch all invalid casts either
     .
    */
    #define EVO_MODULEBASE_DYNAMIC_CAST 1
#endif

/** \cond impl */
#if EVO_MODULEBASE_DYNAMIC_CAST
    #define EVO_MODULEBASE_IMPL_CAST dynamic_cast
#else
    #define EVO_MODULEBASE_IMPL_CAST static_cast
#endif
/** \endcond */

///////////////////////////////////////////////////////////////////////////////

/** A simple base class for defining a module using a similar pattern as Cortex.
 - Use \ref ModulePtr to store a pointer (or array of pointers) to a module inheriting this class
 - Consider using ModuleAsBase to create helpers for getting a module from a ModulePtr
 - See Cortex
 .

\par Example

Here are some examples using alternatives to Cortex.

This example uses a struct to hold modules.

\code
#include <evo/cortex_module.h>
using namespace evo;

struct Modules {
    ModulePtr foo, bar;
};

struct ModuleFoo : ModuleBase {
    void foo() {
        // ...
    }
};
typedef ModuleAsBase<ModuleFoo> ModuleAsFoo;

struct ModuleBar : ModuleBase {
    // Move implementation to a source (.cpp) file to decouple the dependency on ModuleFoo
    void bar(Modules& modules) {
        ModuleAsFoo::get(modules.foo).foo();
    }
};
typedef ModuleAsBase<ModuleBar> ModuleAsBar;

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
#include <evo/cortex_module.h>
using namespace evo;

// ID for each module, used as array index
enum ModuleId {
    MODULE_FOO,     // ID for module Foo
    MODULE_BAR,     // ID for module Bar
    MODULE_COUNT    // Guard used as number of modules
};

struct ModuleFoo : ModuleBase {
    void foo() {
        // ...
    }
};
typedef ModuleAsBase<ModuleFoo> ModuleAsFoo;

struct ModuleBar : ModuleBase {
    // Move implementation to a source (.cpp) file to decouple the dependency on ModuleFoo
    void bar(ModulePtr modules[]) {
        ModuleAsFoo::get(modules[MODULE_FOO]).foo();
    }
};
typedef ModuleAsBase<ModuleBar> ModuleAsBar;

int main() {
    ModulePtr modules[MODULE_COUNT];

    // Uncomment to create modules in advance, otherwise created on demand
    //modules[MODULE_FOO] = new ModuleFoo;
    //modules[MODULE_BAR] = new ModuleBar;

    // Call ModuleBar method
    ModuleAsBar::get(modules[MODULE_BAR]).bar(modules);

    return 0;
}
\endcode
*/
struct ModuleBase {
    /** Destructor. */
    virtual ~ModuleBase() {
    }

    /** Get this module as a concrete type.
     - This uses `dynamic_cast` to cast the base type to the requested type
       - This has some runtime overhead, and throws `std::bad_cast` if the cast fails
       - To use `static_cast` instead `define EVO_MODULEBASE_DYNAMIC_CAST 0` (before including this)
     - \b Caution: Results are undefined if the concrete type (`ItemT`) doesn't match the current object for key
     .
     \tparam  T  Concrete module type to cast this to
     \return     Concrete reference to this object (`T&`)
    */
    template<class T>
    T& as() {
        return * EVO_MODULEBASE_IMPL_CAST <T*>(this);
    }
};

///////////////////////////////////////////////////////////////////////////////

/** Smart pointer to a class inheriting ModuleBase.
 - This defines a common interface for a collection of modules using a similar pattern as Cortex
 - If you build on ModuleBase with another base class, consider defining a SmartPtr alias like this for that type
 - See ModuleBase
*/
typedef SmartPtr<ModuleBase> ModulePtr;

///////////////////////////////////////////////////////////////////////////////

/** Helper for getting a concrete module from a \ref ModulePtr (or similar pointer).
 - This is a helper for calling ModuleBase::as() in a cleaner way (without a template argument)
 - Use this to create a helper type for each module inheriting ModuleBase like this, where `ModuleFoo` is the concrete module type:
   \code
    typedef ModuleAsBase<ModuleFoo> ModuleAsFoo;
   \endcode
 - Call get() to get an instance (create if needed), or getptr() to get instance pointer
 .
 \tparam  T  Concrete type to use, must inherit ModuleBase
*/
template<class T>
struct ModuleAsBase {
    /** Get module reference from stored pointer, create if needed.
     - This uses `dynamic_cast` to cast the base type to the requested type
       - This has some runtime overhead, and throws `std::bad_cast` if the cast fails
       - To use `static_cast` instead `define EVO_MODULEBASE_DYNAMIC_CAST 0` (before including this)
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
        return *EVO_MODULEBASE_IMPL_CAST <T*>(ptr.ptr());
    }

    /** Get module pointer from stored pointer.
     - This uses `dynamic_cast` to cast the base type to the requested type
       - This has some runtime overhead, and throws `std::bad_cast` if the cast fails
       - To use `static_cast` instead `define EVO_MODULEBASE_DYNAMIC_CAST 0` (before including this)
     - \b Caution: Results are undefined if the concrete type (`ItemT`) doesn't match the current object for key
     .
     \tparam  U  Smart pointer type -- inferred from argument
     \param  ptr  Pointer type holding the object
     \return      Pointer to value from `ptr`, NULL if none
    */
    template<class U>
    static T* getptr(U& ptr) {
        if (ptr)
            return EVO_MODULEBASE_IMPL_CAST <T*>(ptr.ptr());
        return NULL;
    }
};

///////////////////////////////////////////////////////////////////////////////

#undef EVO_MODULEBASE_IMPL_CAST

///////////////////////////////////////////////////////////////////////////////
//@}
}
#endif
