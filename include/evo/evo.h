// Evo C++ Library
/* Copyright 2018 Justin Crowell
Distributed under the BSD 2-Clause License -- see included file LICENSE.txt for details.
*/
///////////////////////////////////////////////////////////////////////////////
/** \file evo.h Evo Core Library Documentation -- Includes all the core Evo headers. */
#pragma once
#ifndef INCL_evo_h
#define INCL_evo_h

///////////////////////////////////////////////////////////////////////////////

/** \addtogroup EvoCore
Evo core types and primitives
*/
//@{

/** Evo C++ Library namespace. */
namespace evo {

/** \mainpage Evo C++ Library
\par Evo

Evo is a modern high performance C++ library designed to make writing efficient and portable code easy.
It covers:
 - core types, strings, and containers
 - string tokenization and parsing
 - file/pipe/socket streams and formatting
 - threads and atomics
 - C++03 and C++11 (and newer)
 .

Inspired by modern languages like [Python](https://www.python.org) and [D](https://dlang.org), Evo is the result of many years of lessons learned from writing (and rewriting) high-performance,
object oriented C++ server code. Evo aims to make C++ easier and more powerful without sacrificing performance.

 - \ref WhatsNew "What's New?"
 - \ref DesignGoals "Design Goals"
 - \ref LicenseCredits "License and Credits"
 - \ref CppCompilers "C++ Compilers"
 .

<table border="0"><tr><td valign="top">

\par Features & Concepts

 - \ref ManagedPtr "Managed Pointers"
 - \ref Sharing "Sharing"
 - \ref Slicing "Slicing"
 - \ref UnsafePtrRef "Unsafe Pointer Referencing"
 - \ref PlainOldData "Plain Old Data & Copying"
 - \ref PrimitivesContainers "Primitives & Containers"
 - \ref EnumConversion "Enum Conversion"
 - \ref Streams "I/O Streams & Sockets"
 - \ref Threads "Threads"
 - \ref Metaprogramming "Metaprogramming"
 - \ref Unicode "Unicode"
 - \ref StlCompatibility "STL Compatibility"
 .
</td><td valign="top">

\par Mini Tutorials, Best Practices

 - \ref StringPassing "String Passing"
 - \ref StringConversion "String Conversion"
 - \ref StringParsing "String Parsing"
 - \ref StringFormatting "String Formatting"
 - \ref StreamFormatting "Stream Formatting"
 - \ref StringCustomConversion "Advanced: Custom String Conversion/Formatting"
 - \ref StringStreamCommon "Advanced: Common Stream/String Interface"
 .
</td><td valign="top">
</td></tr><tr><td valign="top">

\par Primitives

 - Bool
 - CharT
    - Char
 - IntegerT
    - \link Short\endlink, \link Int\endlink, \link Long\endlink, \link LongL\endlink / \link UShort\endlink, \link UInt\endlink, \link ULong\endlink, \link ULongL\endlink
    - \link Int8\endlink, \link Int16\endlink, \link Int32\endlink, \link Int64\endlink / \link UInt8\endlink, \link UInt16\endlink, \link UInt32\endlink, \link UInt64\endlink
 - FloatT
    - \link Float\endlink, \link FloatD\endlink, \link FloatL\endlink
 - SmartPtr, SharedPtr, Ptr
 .

 - Pair
 - SafeBool
 - StringInt, StringFlt
 .

</td><td valign="top">

\par Containers

 - Array, List, SubList
   - \link BitArray\endlink, \link BitArraySubset\endlink
   - PtrList
 - String, SubString, \link evo::StringBase StringBase\endlink
   - UnicodeString
   - SubStringMapList
 - Set
   - SetList, \link StrSetList\endlink
   - SetHash, \link StrSetHash\endlink
 - Map
   - MapList, \link StrMapList\endlink
   - MapHash, \link StrHash\endlink
   - lookupsub()
   .
 .

 - Compare, CompareR, CompareI, CompareIR
 - CompareHash
 - fixed_array_size(), EVO_FIXED_ARRAY_SIZE()
 .

</td><td valign="top">

\par Enum Helpers

 - EVO_ENUM_MAP(), EVO_ENUM_CLASS_MAP()
 - EVO_ENUM_MAP_PREFIXED()
 .

\par String Tokenizers

 - StrTok, StrTokR
   - \link StrTokQ\endlink, \link StrTokQR\endlink
   - \link StrTokWord\endlink, \link StrTokWordR\endlink
 - StrTokS, StrTokRS
   - \link StrTokWordS\endlink, \link StrTokWordRS\endlink
 - StrTokLine
 .
 - EVO_TOK_NEXT_OR_BREAK()
 - EVO_TOK_OR_BREAK()
 .
</td></tr><tr><td valign="top">

\par Stream I/O

 - Console, ConsoleMT
 - File
   - \link FileMT\endlink
 - Pipe, PipeIn, PipeOut
   - \link PipeInMT\endlink, \link PipeOutMT\endlink
 - Socket, SocketCast
 - EVO_IO_MT()
 .
</td><td valign="top">

\par Stream/String Formatting

 - Stream::Format, String::Format
 - \ref FmtBase, \ref FmtBasePrefix, \ref FmtPrecision, \ref FmtAlign
 - FmtSetInt, FmtSetFloat, FmtSetNull, FmtSetField
 .
 - FmtChar
 - \ref FmtShort, \ref FmtInt, \ref FmtLong, \ref FmtLongL
 - \ref FmtUShort, \ref FmtUInt, \ref FmtULong, \ref FmtULongL
 - \ref FmtFloat, \ref FmtFloatD, \ref FmtFloatL
 .
</td><td valign="top">

\par Asynchronous I/O

Coming soon...

</td></tr><tr><td valign="top">

\par Threads

 - Thread, ThreadClass
   - ThreadScope, ThreadScope<Thread>
   - ThreadGroup
 - Mutex, MutexRW
   - Condition
 - SmartLock
   - Mutex::Lock
   - Condition::Lock
 - Atomic
   - AtomicFlag
   - AtomicPtr
 .
 </td><td valign="top">

\par Time

 - Timer
 - sleepms(), sleepus(), sleepns()
 .
</td><td valign="top">

\par Constants

 - \link NL\endlink, \link fFLUSH\endlink
 - \link ALL\endlink, \link END\endlink, \link NONE\endlink
 - \link vNULL\endlink, \link vEMPTY\endlink, \link vREVERSE\endlink
 - \link EVO_CNULL\endlink, \link EVO_VNULL\endlink
 .

\par Error Handling

 - \link Error\endlink
 - Exception, ExceptionStream
 - EVO_THROW()
   - EVO_THROW_ERR(), EVO_THROW_ERR_CHECK()
 - EVO_CATCH(), EVO_CATCH_MT()
 .

</td></tr></table>

----

BSD 2-Clause License

Copyright 2018 Justin Crowell

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the
following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following
disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following
disclaimer in the documentation and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

///////////////////////////////////////////////////////////////////////////////

/** \page WhatsNew What's New?

Evo change history.

\par Version 0.4 - Oct 2018
 - Now using <a href="https://opensource.org/licenses/BSD-2-Clause">BSD 2-Clause License</a>
 - Add initial \ref Unicode support to String, add UnicodeString
 - Add updated \ref StringFormatting "String Formatting" and \ref StreamFormatting "Stream Formatting"
   - Replaced PREC_AUTO constant with \ref fPREC_AUTO
   - Stream interface tweaks, using void* instead of char*
 - Add String and SubString `find*()` methods for string searching
 - Add ConsoleMT and support for thread-safe (syncrhonized) I/O
 - Add \ref Threads and atomics support
 - Add Socket streams
 - Add Pair, now used with Map types
 - Add SetList and SetHash types
 - Add StrTokLine
 - Add SubStringMapList and fixed_array_size()
 - Add SubString::token*() methods
 - Add additional stripl() and stripr() methods to String and SubString
 - Add \ref BitArray and \ref BitArraySubset and related Bits helpers
 - Nullable:
   - Add Nullable::denull()
   - Add comparison operators and compare() method
   - Made Nullable::value() return const reference
   - No longer uses implicit conversion, which causes tricky problems with comparisons
 - Made List/String reserve() unslice by default, add "prefer_realloc" param to prefer realloc vs unslice
 - Add sleepns() and add return value to sleepms() and sleepus()
   - sleepus() now uses better than msec precision under Windows
 - Fixed enum constants to use consistent convention, including:
   - Iterator enums (\link iterLAST\endlink, \link iterEND\endlink, \link vNULL\endlink)
   - Char, Integer, and Float traits (\link Int::SIGN\endlink, \link Int::MAX\endlink, etc)
 - Cleanup and some refactoring in container and meta internals, including DataType, Compare, CompareHash, and Map types
 - Refined compiler detection -- see: \link EVO_COMPILER\endlink, \link EVO_COMPILER_VER\endlink
 - Bug fix: SubString::splitat_setl() reset on null index
 - Bug fix: StrTok and StrTokR on space or tab delim
 - Bug fix: String::setn() on floats rounding up unexpectedly in some cases
 - Bug fix: String::numull() returned ulongl when it should be ULongL
 - Many documentation updates

\par Version 0.3 - Dec 2016
 - I/O stream refinements
 - Support C++11 and clang compilers
 - Add STL compatibility: std::string
 - Add StrTok::nextanyq()
 - Improved quoting with \ref StrTokQ and Convert
 - Rename setnull() to set() for consistency
 - Bug fix: SharedPtr::clear() crash
 - Many documentation updates, added Mini Tutorials
*/

///////////////////////////////////////////////////////////////////////////////

/** \page DesignGoals Design Goals

General design goals:
 - Trivial operations are short and simple, like parsing a string
 - Advanced operations are supported, like writing directly to a string buffer
 - High performance, especially for critical systems like servers and game engines
 - Simple interfaces that are easy to understand and use, differentiate basic and advanced features
 - Headers-only library that's easy to integrate with projects
 - Good documentation and extensive unit test coverage
 - Structured naming to make related things naturally sort together -- ex: MapList, MapHash, \link evo::String::split split()\endlink, \link evo::String::splitr splitr()\endlink, \link evo::String::splitat splitat()\endlink
 - Support multiple platforms: Linux, Unix, MacOS, Windows, Cygwin
 .

*/

///////////////////////////////////////////////////////////////////////////////

// NOTE: Compilers copied to README.txt (bottom)
/** \page CppCompilers C++ Compilers

Evo at least requires a C++03 (ideally C++11) compliant compiler. Without C++11, Evo uses some compiler specific features, most notably with threads and atomic operations.

Tested on the following systems and compilers:

\par Ubuntu
 - 18.04: gcc 7.3.0, clang 6.0.0, valgrind 3.13.0 -- VM
 - 16.04: gcc 5.4.0, clang 3.8.0, valgrind 3.11.0 -- Intel Core i7
 - 14.04: gcc 4.8.4, clang 3.4.0, valgrind 3.10.1 -- VM
   - Also: gcc 4.6.4, gcc 4.7.3

\par CentOS -- VM
 - 7.3: gcc 4.8.5, clang 3.4.2, valgrind 3.13.0
 - 6.10: gcc 4.4.7, clang 3.4.2

\par FreeBSD -- VM
 - 11.1: clang 6.0.0 valgrind 3.10.1

\par MacOS -- Intel Core i5
 - 10.13.6: Apple LLVM 10.0.0 (clang 1000-11.45.2)

\par Windows 10 -- Intel Core i7
 - <a href="https://www.visualstudio.com/vs/cplusplus/">MSVC</a> 2017 Community 15.8.5
 - MSVC 2015 Community, Update 3

\par Windows 7 SP1 32-bit -- VM
 - MSVC 2013 Community, Update 5
 - <a href="https://www.cygwin.com/">cygwin 2.11.0</a>: gcc 7.3.0

\par Notes
 - All tested systems are 64-bit, unless otherwise noted
 - Microsoft Visual C++:
   - Before 2015: <a href="https://msdn.microsoft.com/en-us/library/hh567368.aspx">Static var initialization is not thread-safe ("magic statics")</a>
   - 2013: Atomic or AtomicPtr with const template param causes release build compiler error
   - Before 2012: <a href="https://msdn.microsoft.com/en-us/library/hh874894(v=vs.110).aspx">Atomics not supported</a>, but Evo will emulate atomics with a mutex
 - Cygwin:
   - For C++11 (or newer), enable with the `gnu++` option variant to avoid compile errors, ex: `g++ -std=gnu++11`
 - With old GCC versions (such as 4.4), use option `-fno-strict-aliasing` to prevent lots of warnings and stability issues with release builds
*/

///////////////////////////////////////////////////////////////////////////////

/** \page ManagedPtr Managed Pointers

A managed pointer is a class that emulates a raw (built-in) pointer type.
Using managed pointers makes the intent and ownership of the pointer clear and reduces risk of pointer misuse and memory bugs.

Managed pointers:
 - SmartPtr is a smart pointer to a single object, meaning it takes ownership of a pointer and is responsible for freeing it.
   - Copying a SmartPtr copies the pointed object too.
 - SharedPtr is similar to SmartPtr except it can be shared among multiple owners and uses Reference Counting.
   - Copying a SharedPtr makes a shared copy (increments reference count) -- all copies share the same instance.
 - Ptr is a dumb pointer to a single object or array, meaning it only references the pointer and does not take ownership.
   - This is useful to explicitly indicate the pointer is owned elsewhere or to distinghuish from a static immutable data pointer
     - <em>Never assign (or copy construct from) a raw pointer to an Evo container unless it refers to static immutable data</em> (ex: string literal) -- see \ref Sharing "Sharing" for an example
   - Dumb pointers can also be moved with addition or subtraction
   - %Atomic version: AtomicPtr

All managed pointers are specialized for array types (<tt>T[]</tt>) to use <tt>delete[]</tt> to free memory (when applicable). Managed array pointers can't make a copy of the pointed objects (array size not known).
 - SmartPtr<T[]> (array version) cannot be copied
 - \link evo::SharedPtr<T[],TSize> SharedPtr<T[]>\endlink (array version) doesn't support unshare()

Managed pointers should be used instead of raw pointers as much as possible.
Dereference operators (operator*, operator->, operator[]) are overloaded so they work like raw pointers.

\par Quick Example

\code
#include <evo/ptr.h>
using namespace evo;

int maint() {
    // Smart pointer to int
    SmartPtr<int> num(new int);
    *num = 1;

    // Smart pointer to int array
    SmartPtr<int[]> nums(new int[2]);
    nums[0] = 1;
    nums[1] = 2;

    return 0;
}
\endcode

\par Detailed Example

\code
#include <evo/ptr.h>
#include <evo/io.h>
using namespace evo;
static Console& c = con();

// Example class/object
struct Data {
    int val;
};

// Uses dumb Ptr so it's clear this function does not take pointer ownership
void print(Ptr<Data> ptr) {
    if (ptr)    // evaluate managed pointers as bool for easy not-null check (uses SafeBool)
        c.out << "Val: " << ptr->val << NL;
}

int main() {
    // Smart pointer
    SmartPtr<Data> ptr(new Data);

    // Set a value, dereference like normal pointer (3 different examples here)
    ptr->val   = 1;
    ptr[0].val = 1;
    (*ptr).val = 1;

    // Pass pointer to a function
    print(ptr);

    // New scope, use dumb pointer, change value
    {
        // Dumb pointer referencing ptr
        Ptr<Data> ptr2;
        ptr2 = ptr;

        // Change value, pass to function
        ptr2->val = 2;
        print(ptr2);
    }

    // Transfer ownership for fun
    SmartPtr<Data> ptr2(ptr.detach());

    // Get a raw pointer for fun, pass to function
    Data* ptr3 = ptr2.ptr();
    print(ptr3);

    return 0;
} // ptr is null, ptr2 automatically freed
\endcode

Output:
\code{.unparsed}
Val: 1
Val: 2
Val: 2
\endcode

\par Low-Level Auto-Pointers

AutoPtr is a minimal stripped down version of SmartPtr, mainly used internally to make code exception/leak safe without a dependency on SmartPtr

 - Minimal low-level struct interface
 - Not copyable
 - Specialized for array pointers with AutoPtr<T[]>

FreePtr is similar to AutoPtr, but frees with C library free() -- for pointers returned by malloc().

*/

///////////////////////////////////////////////////////////////////////////////

/** \page Sharing Sharing

Sharing is a form of Copy-On-Write (COW) optimization with Reference Counting.

Containers like List and String use sharing:

 - This works in conjunction with \ref Slicing "Slicing" to improve performance by minimizing memory reallocation and copying
 - When a container is copied, a "shared" copy may be used, meaning the underlying data is shared between two (or more) objects
   - Some containers (such as String) <em>may also share a raw pointer</em> (read-only data) such as a string literal -- see example below
     - <em>Never assign (or copy construct from) a raw pointer to an Evo container unless it refers to static immutable data</em> (ex: string literal), otherwise use copy() method or a dumb pointer (Ptr)
   - Copying from a managed pointer (SmartPtr, Ptr, etc) always makes a full (unshared) copy
   - A shared copy will become a full copy when a write operation (modifier) is called
   - An empty container is considered unshared (no data is shared)
   .
 - Containers that support sharing will have these methods:
   - Constructors: Same as `set(src)`
   - `set(src)` -- %Set as copy (shared copy if possible)
   - `copy(src)` -- %Set as full (unshared) copy
   - `operator=(src)` -- Same as `set(src)`
   - `shared()` -- Get whether shared
   - `unshare()` -- If shared then make unshared, allocating a new buffer if needed -- calls unslice() (if present)
     - _Some containers only reference (i.e. don't own) data_ (ex: SubString), for these `unshare()` is a no-op and `shared()` always returns false
   - Modifier methods end with an uppercase 'M' to distinguish them, call `unshare()` to make the internal buffer unshared and writable
   .
 - Sharing is automatic, though being mindful of how "modifier" methods are used will improve performance
 - \b Caution: Sharing can impact thread safety, always make full (unshared) copies across separate threads
 .

Containers that support Sharing: List, String, PtrList, MapList, MapHash

\par Example

\code
#include <evo/string.h>
#include <evo/io.h>
using namespace evo;
static Console& c = con();

int main() {
    // Reference a string literal -- no memory allocated here
    String str("test");

    // Append a space and number
    //  - This is a write operation (modifier) so will allocate a buffer to hold "test 123"
    str << " 123";

    // Split into two tokens separated by space -- data is shared, no memory allocated here
    String left, right;
    str.split(' ', left, right);

    // Append "ing" to first token
    //  - This is a write operation (modifier) so will allocate a buffer to hold "testing"
    left << "ing";

    // Prints: testing,123
    c.out << left << ',' << right << NL;

    return 0;
}
\endcode

\par Example with raw pointer

\code
#include <evo/string.h>
#include <evo/ptr.h>
using namespace evo;

int main() {
    // Fixed buffers like this are common in C
    char buf[10];
    strcpy(buf, "testing");

    String str;
    str = "foo";            // Ok, string literal (static and immutable)
    str = buf;              // DANGER: assigning raw pointer to Evo container, which may reference that pointer instead of copy from it
    str = Ptr<char>(buf);   // Ok, assign as dumb pointer so the assignment will make a copy

    return 0;
}
\endcode

\par Example with sharing and slicing

\code
#include <evo/string.h>
#include <evo/io.h>
using namespace evo;
static Console& c = con();

int main() {
    // Reference a string literal -- no memory allocated here
    String str("_test_");

    // Remove (slice out) underscore characters, same literal is still referenced
    str.slice(1, 4);

    // Append a dash, space, and number
    //  - This is a write operation (modifier) so will allocate a buffer to hold "test- 123" -- sliced out data is discarded
    str << "- 123";

    // Split into two tokens separated by dash -- data is shared, no memory allocated here
    String left, right;
    str.split('-', left, right);

    // Strip (slice out) leading space from right substring
    right.stripl(' ');

    // Append "ing" to first token
    //  - This is a write operation (modifier) so will allocate a buffer to hold "testing"
    left << "ing";

    // Prints: testing,123
    c.out << left << ',' << right << NL;
}
\endcode
*/

///////////////////////////////////////////////////////////////////////////////

/** \page Slicing Slicing

Slicing is a way for a container to reference a subset (a "slice") of sequential data.

 - This allows a kind of non-destructive editing where beginning and end items can be "trimmed" without actually modifying the data
 - This works in conjunction with \ref Sharing "Sharing" to improve performance by minimizing memory reallocation and copying
 - The "sliced out" (removed) items remain in memory and are automatically cleaned up later when needed
 - Containers that support slicing will have these methods:
   - `slice()` -- Slices object, trimming all items outside given subset
   - `unslice()` -- Cleans up sliced out data -- called automatically by modifier methods
   - `triml()` -- Trim (remove) items from left side (beginning)
   - `trimr()` -- Trim (remove) items from right side (end)
   - `truncate()` -- Truncate to given size by trimming (removing) items from end as needed
   .
 - Inspired by D %Array Slices: http://dlang.org/d-array-article.html
 .

Containers that support Slicing: List, String, MapList

Example:
\code
#include <evo/string.h>
#include <evo/io.h>
using namespace evo;
static Console& c = con();

int main() {
    // Reference a string literal -- no memory allocated here
    String str("_test_");

    // Remove (slice out) underscore characters, same literal is still referenced
    str.slice(1, 4);

    // Append a space and number
    //  - This is a write operation (modifier) so will allocate a buffer to hold "test 123" -- sliced out data is discarded
    str << " 123";

    // Prints: test 123
    c.out << str << NL;

    return 0;
}
\endcode
*/

///////////////////////////////////////////////////////////////////////////////

/** \page PlainOldData Plain Old Data & Copying

Evo uses these type categories for optimization.

\par Plain Old Data (POD)

These types are plain data primitives that only use default constructors, assignment operators, and destructor.
 - This generally means built-in types such as char or int
 - Containers use memcpy() for copying these types -- constructors, assignment operators, and destructors are skipped
 - You may declare a custom POD type with EVO_PODTYPE() (rare)

\par Byte-Copy Types

These types can be directly byte-copied without any copy constructor or assignment operator.
 - These types still use a default constructor and destructor
 - Containers use memcpy() for copying these types instead of copy constructor or assignment operator
 - You may declare a custom Byte-Copy type with EVO_BCTYPE()

\par Normal Types

These are normal types that use constructors, assignment operators and destructors.
 - This means non-POD and non-Byte-Copy types -- the default for custom types not recognized by Evo
 - Containers call constructors and destructor on these types and use copy-constructor or operator=() to copy
*/

///////////////////////////////////////////////////////////////////////////////

/** \page PrimitivesContainers Primitives & Containers

Evo primitive and basic container types.

\par Primitives

Evo has class variations of basic primitives with some added features:
 - %Nullable: Can have an explicit null value
 - Defaulting: Value defaults to 0 by constructor
 - Type traits like min/max value
 - Various helper methods
 - Smart pointers make sure memory is always freed
 .

 All Evo primitives implement these methods:
 - `bool null() const`
 - `bool valid() const`
 - `This& set()`
 .

Primitives:
 - CharT
    - Char
 - IntegerT
    - \link Short\endlink, \link Int\endlink, \link Long\endlink, \link LongL\endlink / \link UShort\endlink, \link UInt\endlink, \link ULong\endlink, \link ULongL\endlink
    - \link Int8\endlink, \link Int16\endlink, \link Int32\endlink, \link Int64\endlink / \link UInt8\endlink, \link UInt16\endlink, \link UInt32\endlink, \link UInt64\endlink
 - FloatT
    - \link Float\endlink, \link FloatD\endlink, \link FloatL\endlink
 - SmartPtr, SharedPtr, Ptr
 - Pair
 .

Related:
 - SafeBool
 - StringInt, StringFlt
 .

\par Containers

Evo containers:
 - %Nullable: Can have an explicit null value
 - %Hashable: Have a `hash()` method for hashing items as a group
 - Containers have iterator subtypes:
   - Iter for read-only (const) access
   - IterM for read and write (mutable) access (optional)
   - See below for more on iterators
   .
 - Optimizations for handling different types efficiently -- see \ref PlainOldData "Plain Old Data & Copying"
 - May support \ref Sharing "Sharing" and/or \ref Slicing "Slicing"
 - New empty containers do not allocate memory until necessary
 - Advanced methods with "adv" prefix for more intrusive use
 .

All Evo containers implement these methods:
 - `bool null() const`
 - `bool empty() const`
 - `Size size() const`
 - `bool shared() const`
 - `This& set()`
 - `This& clear()`
 - `This& unshare()`
 - `ulong hash(ulong seed=0) const`
 .

Types:
 - Array, List, SubList
   - \link BitArray\endlink
   - PtrList
 - String, SubString
   - UnicodeString
 - Set
   - SetList, \link StrSetList\endlink
   - SetHash, \link StrSetHash\endlink
 - Map
   - MapList, \link StrMapList\endlink
   - MapHash, \link StrHash\endlink
   .
 .

Related:
 - \link evo::StringBase StringBase\endlink
 - Pair
 - SubStringMapList
 - \link BitArraySubset\endlink
 .

<i>Notes for item classes used with containers:</i> 
 - <b>%Exception Safety</b>
   - Item *destructor* must *never throw an exception*
   - Item *default constructor* and *copy constructor* must *never throw an exception*
   - Item *assignment operator* for copying item must *never throw an exception*
 - <b>Relocatable Objects</b>
   - Evo containers may relocate (move) items to a different location in memory (as an internal optimization)
   - Items must be relocatable, i.e. must not have any member variables pointing to or referencing a member variable in the _same instance_ (a hacky thing to do anyway)
 - \b Comparisons
   - Item must implement operator==() for equality checking
   - Item should implement compare() for ordered comparisons
 - All POD types are supported by Evo containers
 - \b Caution: If item type violates these requirements then <i>results are undefined (i.e. memory leak/crash under edge cases/weird bugs)</i> if used with Evo containers
 .

\par Iterators

Evo iterators work similar to pointers.
 - Increment or decrement iterator to move to next or previous item
 - Dereference iterator to get value
 .

The general iterator types aren't normally used directly. A container will have subtypes defining concrete iterator types to use -- see List::Iter and List::IterM for example.

Iterator types:
 - IteratorFw: Forward iterator
 - IteratorBi: Bidirectional (forward and reverse) iterator
 - IteratorRa: Random access (forward, reverse, and random access) iterator
 .

\par Comparison

Containers with ordering or sorting do comparisons with a comparison type that implements CompareBase. Comparison types:

 - are specified via template param, and the compiler can optimize out (inline) the comparison calls
 - implement `operator()()` to compare 2 items (`a` & `b`) and return an int: 0 if `a == b`, positive if `a > b`, negative if `a < b`
 - define a `PassItem` type (inheriting CompareBase) used for passing items to compare
 - can hold state for advanced use, such as tracking stats
 - interface:
   - `int operator()(PassItem a, PassItem b) const`
 . 

Standard comparison types:
 
 - Compare, CompareR
 - CompareI, CompareIR
 .

\par Hashing

Containers using hash tables use a single combined type for hashing and comparisons because values that compare as equal must also hash to the same bucket,
otherwise collisions won't be handled correctly. Hashing types:

 - are specified via template param, and the compiler can optimize out (inline) the hash/comparison calls
 - implement `operator()()` to compare 2 items (`a` & `b`) in case of collisions and return an int: 0 if `a == b`, positive if `a > b`, negative if `a < b`
 - implement `hash()` method to hash a key/value, with a seed for chaining a previous hash method
 - define a `PassItem` type (inheriting CompareBase) used for passing items to hash/compare
 - can hold state for advanced use, such as tracking stats
 - interface:
   - `int operator()(PassItem a, PassItem b) const`
   - `ulong hash(PassItem key, ulong seed=0) const`
 .

Standard hashing types:

 - CompareHash
 .
*/

///////////////////////////////////////////////////////////////////////////////

/** \page EnumConversion Enum Conversion

Evo has helpers for efficiently converting between strings and enums.
 - Enum string list is built at compile-time -- doesn't allocate memory
 - Lookups are fast -- binary search is used to find the enum string in an ordered list
 - This is done using EVO_ENUM_MAP_PREFIXED() or EVO_ENUM_CLASS_MAP() or SubStringMapList
 - \b Caution: This requires string values to be _pre-sorted_, and _no gaps_ in enum values
 .

This example defines an enum, then uses the EVO_ENUM_MAP_PREFIXED() helper to create string mappings.
 - This requires the enum to have the expected first/last guard values (`UNKNOWN`, `ENUM_END` -- with a prefix where applicable) to deduce the mappings
 - These enum values have a lowercase prefix to avoid collisions with other enums -- referred to as a "prefixed" enum
 .

\code
#include <evo/substring.h>
using namespace evo;

enum Color {
    cUNKNOWN = 0,   // Must be first
    // Must match string order below
    cBLUE,
    cGREEN,
    cRED,
    cENUM_END       // Must be last
};

EVO_ENUM_MAP_PREFIXED(Color, c,
    // Must be sorted, and must match enum values
    "blue",
    "green",
    "red"
);

int main() {
    Color     color_val = ColorEnum::get_enum("green");     // set to cGREEN
    SubString color_str = ColorEnum::get_string(cGREEN);    // set to "green"
    return 0;
}
\endcode

\par Enum Class (C++11)

This is simplied further with C++11 enum class, as long as the enum has the expected first/last guard values (`UNKNOWN`, `ENUM_END`).

\code
#include <evo/substring.h>
using namespace evo;

enum class Color {
    UNKNOWN = 0,    // Must be first
    // Must match string order below
    BLUE,
    GREEN,
    RED,
    ENUM_END        // Must be last
};

EVO_ENUM_CLASS_MAP(Color,
    // Must be sorted, and must match enum values
    "blue",
    "green",
    "red"
);

int main() {
    Color     color_val = ColorEnum::get_enum("green");         // set to Color::GREEN
    SubString color_str = ColorEnum::get_string(Color::GREEN);  // set to "green"
    return 0;
}
\endcode
*/

///////////////////////////////////////////////////////////////////////////////

/** \page Streams I/O Streams

Evo supports various types of I/O streams:
 - Console, ConsoleMT
 - File
   - \link FileMT\endlink
 - Pipe, PipeIn, PipeOut
   - \link PipeInMT\endlink, \link PipeOutMT\endlink
 - Socket, SocketCast

_Note: Streams use synchronous I/O_

Base class for all stream exceptions: ExceptionStream

Features include:
 - Files, pipes, and sockets
 - Output formatting (operator<<) -- see \ref StreamFormatting "Stream Formatting"
 - Text newline conversion (LF, CRLF, CR)
   - Text reading supports all common newline types
   - Text writing converts to system newline type by default -- explicit \link Newline\endlink values are not converted
 - High performance buffering
   - Read/Write buffers minimize the overhead of low-level I/O calls
   - Output formatting writes directly to stream buffer
 - Thread-safe streams with wrappers (ConsoleMT, StreamMT, StreamExtMT) and EVO_IO_MT() macro
 - Low-level I/O abstraction layer (OS files, pipes, sockets, etc) for supporting multiple platforms

Streams inherit from Stream (read/write), StreamIn (read-only), or StreamOut (write-only).
Note that a Stream (read/write) may be opened in read-only or write-only mode.

\dot
digraph "Streams" {
  edge [fontname="Helvetica",fontsize="10",labelfontname="Helvetica",labelfontsize="10"];
  node [fontname="Helvetica",fontsize="10",shape=record];

  Console [URL="\ref Console"];
  Pipe    [URL="\ref Pipe"];
  PipeIn  [URL="\ref PipeIn" style=filled fillcolor=lightgrey];
  PipeOut [URL="\ref PipeOut" style=filled fillcolor=lightgrey];
  File    [URL="\ref File"];
  Socket  [URL="\ref Socket"];
  Console -> PipeIn  [style="dashed" label="in"];
  Console -> PipeOut [style="dashed" label="out,err"];
  Pipe    -> PipeIn  [style="dashed" label="in"];
  Pipe    -> PipeOut [style="dashed" label="out"];

  Stream    [URL="\ref Stream" style=filled fillcolor=lightgrey];
  StreamIn  [URL="\ref StreamIn" style=filled fillcolor=lightgrey];
  StreamOut [URL="\ref StreamOut" style=filled fillcolor=lightgrey];
  PipeIn  -> StreamIn;
  PipeOut -> StreamOut;
  File    -> Stream;
  Socket  -> Stream;

  SocketCast [URL="\ref SocketCast"];
  StreamBase [URL="\ref StreamBase" style=filled fillcolor=lightgrey];
  Stream    -> StreamBase;
  StreamIn  -> StreamBase;
  StreamOut -> StreamBase;

  IoBase [URL="\ref IoBase" style=filled fillcolor=lightgrey];
  SocketCast -> IoBase;
  StreamBase -> IoBase;

  label="Streams";
  labelloc=top;
}
\enddot

<center><i>Note: Implementation detail classes above are grayed and are not normally instantiated directly</i></center>

\par Implementation Detail

A Stream (or StreamIn or StreamOut) type is coupled (via template param) with a type implementing IoDevice (IoFile, IoSocket), which handles the low-level device I/O.
 - The stream implementation will use an IoReader to read from the device, and/or an IoWriter to write to the device
 - IoReader and IoWriter implement the buffering layer

\dot
digraph "StreamsLowLevel" {
  edge [fontname="Helvetica",fontsize="10",labelfontname="Helvetica",labelfontsize="10"];
  node [fontname="Helvetica",fontsize="10",shape=record];

  IoSocket [URL="\ref IoSocket"];
  IoFile   [URL="\ref IoFile"];
  IoDevice [URL="\ref IoDevice"];
  IoFile   -> IoDevice;
  IoSocket -> IoDevice;

  //IoFilterBase  [URL="\ref IoFilterBase"];
  //IoFilterChain [URL="\ref IoFilterChain"];
  //IoFilterChain -> IoFilterBase;

  {rank=min
    IoReader [URL="\ref IoReader"];
    IoWriter [URL="\ref IoWriter"];
    //IoReader -> IoFilterChain [style="dashed" label="filters_"];
    //IoWriter -> IoFilterChain [style="dashed" label="filters_"];
  }

  Stream    [label="Stream\<IoDevice\>" URL="\ref Stream"];
  StreamIn  [label="StreamIn\<IoDevice\>" URL="\ref StreamIn"];
  StreamOut [label="StreamOut\<IoDevice\>" URL="\ref StreamOut"];
  Stream    -> IoReader [style="dashed" label="bufrd_"];
  Stream    -> IoWriter [style="dashed" label="bufwr_"];
  StreamIn  -> IoReader [style="dashed" label="bufrd_"];
  StreamOut -> IoWriter [style="dashed" label="bufwr_"];

  StreamBase [URL="\ref StreamBase"];
  IoBase     [URL="\ref IoBase"];
  Stream     -> StreamBase;
  StreamIn   -> StreamBase;
  StreamOut  -> StreamBase;
  StreamBase -> IoBase;

  label="Streams Lower-Level";
  labelloc=top;
}
\enddot

IoReader, IoWriter, and IoFilterChain use RawBuffer for low-level memory buffers.
*/

///////////////////////////////////////////////////////////////////////////////

/** \page Threads Threads

Evo provides multithreading and synchronization classes that wrap the OS thread APIs (pthreads, win32) and compiler atomics.
These classes provide a common interface on all supported platforms, before and after C++11.

%Thread classes:
 - Thread, ThreadClass
   - ThreadScope, ThreadScope<Thread>
   - ThreadGroup

%Thread safe I/O:
 - ConsoleMT, \link FileMT\endlink
 - EVO_IO_MT()

Synchronization classes:
 - Mutex, MutexRW, Condition
   - MutexInert, SpinLock, SleepLock
 - Mutex::Lock
   - MutexRW::LockWrite, MutexRW::LockRead
   - Condition::Lock

Thread-Local Storage:
 - \link EVO_THREAD_LOCAL EVO_THREAD_LOCAL\endlink

%Atomic types and Memory fences:
 - Atomic, AtomicFlag
   - \link AtomicInt\endlink, \link AtomicLong\endlink, \link AtomicLongL\endlink
   - \link AtomicUInt\endlink, \link AtomicULong\endlink, \link AtomicULongL\endlink
   - \link AtomicInt32\endlink, \link AtomicInt64\endlink
   - \link AtomicUInt32\endlink, \link AtomicUInt64\endlink
 - AtomicPtr
 - EVO_ATOMIC_FENCE()
   - \link EVO_ATOMIC_RELAXED\endlink, \link EVO_ATOMIC_CONSUME\endlink
   - \link EVO_ATOMIC_ACQUIRE\endlink, \link EVO_ATOMIC_RELEASE\endlink, \link EVO_ATOMIC_ACQ_REL\endlink
   - \link EVO_ATOMIC_SYNC\endlink (default)

Note: Before C++ 11, thread safety depends on the compiler. Be especially careful with static and global variables on pre-C++11 compilers. See notes in: \ref CppCompilers "C++ Compilers".
*/

///////////////////////////////////////////////////////////////////////////////

/** \page Metaprogramming Metaprogramming

Evo uses some metaprogramming internally -- these features are available for advanced use.

\par Type Traits

 - EVO_BCTYPE()
 - TypeId
 - IsPodType
 - IsByteCopyType
 - IsNormalType
 - IsArray
 - IsPointer
 - IsReference
 - IsConst
 - IsSame
 .

\par Type Conversion

 - RemoveConst
 - RemoveConstVol
 - RemoveExtents
 - RemoveExtentsConstVol
 - RemoveVolatile
 .

\par Static Expressions

 - STATIC_ASSERT()
 - \link STATIC_ASSERT_FUNC_UNUSED\endlink
 - STATIC_ASSERT_FUNC_UNUSED_RET()
 - EVO_STATIC_JOIN()
 - StaticIf
 .
*/

///////////////////////////////////////////////////////////////////////////////

/** \page Unicode Unicode

Evo strings and I/O streams are byte-based and are naturally compatible with ASCII and UTF-8.

 - Linux/Unix APIs prefer UTF-8 for Unicode
 - Windows APIs prefer UTF-16 for Unicode
   - Note that the Windows console doesn't normally display UTF-8 correctly
 - Evo defines it's own consistent wide character types (all unsigned):
   - `wchar16` -- UTF-16 character
   - `wchar32` -- UTF-32 character, full code point
 - Evo doesn't use the C/C++ `wchar_t` type since it's platform dependent

Recommended: Use String (UTF-8) and convert with UnicodeString (UTF-16) on demand as needed for APIs that require UTF-16 (like Windows).

For background and rationale see: http://utf8everywhere.org

\par Conversion

 - %Convert to UTF-8 with String::set_unicode()
 - %Convert to UTF-16 with UnicodeString

\par Full Unicode with ICU

ICU4C (i.e. libicu) is a mature C/C++ library for Unicode support and software globalization.

 - Evo integration with libicu coming soon...
 - For more on libicu: http://site.icu-project.org

\par Low-Level Functions

These are implemented with Evo and don't require libicu.

 - utf8_scan()
 - utf8_min()
 - utf8_count()
 - utf8_to16()
 - utf16_scan()
 - utf16_scan_term()
 - utf16_compare()
 - utf16_strlen()
 - utf16_min()
 - utf16_count()
 - utf16_to8()
*/

///////////////////////////////////////////////////////////////////////////////

/** \page StlCompatibility STL Compatibility

Evo STL compatibility info.

 - Evo List is similar to `std::vector` (dynamic contiguous array)
 - Evo String is similar to `std::string` (dynamic string)
   - Evo \link evo::StringBase StringBase\endlink is used as a common base type for referencing all string types (pass by const reference)
 - Evo SubString is similar to C++17 `std::string_view` (substring reference)
 - Evo SetHash is similar to C++11 `std::unordered_set` (hash table as set)
 - Evo MapHash is similar to C++11 `std::unordered_map` (hash table as map)
 - Evo Nullable is similar to `std::optional` (but only for primitive types)
 - Some Evo containers have begin/end methods for compatibility with C++11 range-based for-loops

\par STL String

Evo supports implicitly converting `const std::string&` and `const std::string*` to \link evo::StringBase StringBase\endlink so STL strings can be passed to most functions that
accept Evo string types. Simply include STL strings first and Evo will try to auto-detect and enable std::string support.

Note that in some cases you may run into problems with "too many implicit conversions." Here's an example:
\code
#include <string>
#include <evo/string.h>

int main() {
    std::string stdstring = "test";              // uses default constructor, assigns via temporary, ok here (though not ideal)
    evo::String string    = stdstring;           // uses default constructor, assigns via temporary, possible compiler error: too many implicit conversions

    evo::String string(stdstring);               // ok, explicit constructor (no temporary) -- preferred
    evo::String string = StringBase(stdstring);  // ok, explicit temporary

    return 0;
}
\endcode

You may also enable STL string compatibility directly with a preprocessor define, before including any Evo headers:
\code
#define EVO_STD_STRING 1
#include <evo/string.h>
\endcode

For more info on enabling STL string compatibility see: \link EVO_STD_STRING EVO_STD_STRING\endlink

\par STL Map

Evo provides some helpers for std::map container lookups.

 - stdlookupsub()
 - stdlookup()
 - stdlookupM()
*/

///////////////////////////////////////////////////////////////////////////////

/** \page StringPassing %String Passing

Evo strings and containers use \ref Sharing "Sharing" so copying is generally efficient (though see warning about thread safety).

This shows the preferred ways of passing a string with Evo.

 - Passing a string as an argument (by const reference):
   \code
    #include <evo/string.h>
    using namespace evo;

    void foo(const String& str) {
        ...
    }

    int main() {
        // Call function, pass a string
        foo("test");

        return 0;
    }
   \endcode
 - Passing a string by value is ok but not ideal (less efficient):
   \code
    void foo(evo::String str);  // Pass by value: not ideal
   \endcode
 - Best to "return" a string using an "out" parameter (by reference):
   \code
    void foo(evo::String& out);
   \endcode
 - You can also return a string by value, but this is not ideal (less efficient):
   \code
    evo::String foo();          // Return by value: not ideal
   \endcode
   - Note that circumstances may allow compiler Return Value Optimization (RVO), but you have to assume the compiler is able to do this and there
     are cases where it can't (where the compiler will silently return by value) -- sticking with an out parameter for consistency and simplicity is recommended
 .

These also apply in general to Evo containers like List, and other complex objects.

For passing read-only or non-persistent strings it may be useful to use SubString.
This explicitly shows the string is read-only and prevents further \ref Sharing when copied to another String,
and is often useful in multithreaded situations (with sychronization as needed).
 - Pass substring as an argument (by const reference):
   \code
    #include <evo/substring.h>
    using namespace evo;

    struct MyData {
        String data;

        // Takes a string reference, without sharing
        void store(const SubString& str) {
            data = str;         // Store a unique copy
        }
    };

    int main() {
        String str("test");
        MyData data;

        // Pass str
        data.store(str);

        // Works with other string types too
        data.store("testing");

        return 0;
    }
   \endcode
 - As with String, passing SubString by value is ok but not preferred (less efficient)
 - Returning a SubString in any way is usually dangerous, unless returning one of the arguments
   \code
    const SubString& foo(const SubString& str) {
        ...
        return str;
    }
   \endcode
 .

See also: \ref UnsafePtrRef
*/

////

/** \page StringConversion %String Conversion

This goes over converting strings to simpler types like numbers. The reverse is covered by \ref StringFormatting "String Formatting".

 - If conversion to a C++ primitive like bool or int fails, the result will be false or 0, respectively
 - Evo primitives like Bool and \link Int\endlink are useful for conversion: If the conversion fails the result will be null
   - Evo primitives can be null so they help distinguish between conversion failure and conversion to false or 0
 - Conversion failure doesn't throw an error or exception
   - Many conversion functions have a lower-level variant that can return an error code like \link String::getnum(Error&,int) const getnum()\endlink

This also goes over more advanced conversion using Generic Conversion and split/join features.

\par Bool Conversion

%Convert strings to bool:
\code
#include <evo/string.h>
using namespace evo;

int main() {
    Bool val1 = String("true").boolval();
    bool val2 = String("true").getbool<bool>();

    return 0;
}
\endcode

\par Number Conversion

%Convert strings to numbers:
\code
#include <evo/string.h>
using namespace evo;

int main() {
    Int    val1 = String("123").num();
    FloatD val2 = String("1.2").numfd();
    int    val3 = String("123").getnum<int>();
    double val4 = String("1.2").getnumf<double>();

    return 0;
}
\endcode

\par Generic Conversion

Generic conversion is supported via templates:
\code
#include <evo/string.h>
using namespace evo;

int main() {
    Bool      val1 = String("1").convert<Bool>();
    Int       val2 = String("123").convert<Int>();
    FloatD    val3 = String("1.2").convert<FloatD>();
    int       val4 = String("123").convert<int>();
    double    val5 = String("1.2").convert<double>();
    SubString val6 = String("1.2").convert<SubString>();

    return 0;
}
\endcode

Reverse generic conversion is also supported. See \ref StringFormatting "String Formatting".

Note: Generic conversion to an incompatible type will result in a compiler error.

\par Join Conversion

List, Set, and Map collections can be joined into a delimited string using generic conversion.

See:
 - String::join(), String::joinq()
 - String::joinmap(), String::joinmapq()

\par Split Conversion

A delimited string can be split into List, Set, and Map collections using generic conversion.

See:
 - String::split()
 - String::splitmap()
*/

////

/** \page StringParsing %String Parsing

This shows different approaches for string parsing.

\par Check Null or Empty

\code
#include <evo/string.h>
using namespace evo;

int main() {
    String str;

    // Null string
    if (str.null())
        { }

    // Empty string is either null or ""
    if (str.empty())
        { }

    // Non-empty string
    if (!str.empty())
        { }

    return 0;
}
\endcode

\par Splitting

Splitting to substrings:
\code
#include <evo/string.h>
using namespace evo;

int main() {
    String str("foo=bar"), left, right;

    // Split to left/right on first occurrence of '='
    str.split('=', left, right);

    // Split to just left
    str.split('=', left);

    // Split to just right
    str.split('=', vNULL, right);

    return 0;
}
\endcode

Splitting to a list with generic conversion:
\code
#include <evo/string.h>
using namespace evo;

int main() {
    String str("1,2,3");

    // Split into list
    List<Int> nums;
    str.split(nums);

    // Join back into list
    str.join(nums.set());

    return 0;
}
\endcode

Splitting to a list with generic conversion and alternate tokenizer:
\code
#include <evo/string.h>
using namespace evo;

int main() {
    String str("1,'2,2',3");

    // Split into list, with tokenizer supporting quoting
    List<String> nums;
    str.split<StrTokQ>(nums);

    // Join back into list, with quoting
    str.joinq(nums.set());

    return 0;
}
\endcode

\par Tokenizing

SubString has methods for extracting tokens:
\code
#include <evo/substring.h>
#include <evo/io.h>
using namespace evo;
static Console& c = con();

int main() {
    SubString str("one,two,three"), value;
    while (str.token(value, ','))
        c.out << value << NL;

    return 0;
}
\endcode

StrTok classes can parse into substring tokens:
\code
#include <evo/substring.h>
#include <evo/io.h>
using namespace evo;
static Console& c = con();

int main() {
    // Tokenize in order (left to right)
    StrTok tok1("one,two,three");
    while (tok1.next(','))
        c.out << tok1.value() << NL;

    // Tokenize in reverse (right to left)
    StrTokR tok2("one,two,three");
    while (tok2.next(','))
        c.out << tok2.value() << NL;

    // Tokenize with quoting -- using StrTokQ
    StrTokQ tok3("one,'two,two',three");
    while (tok3.next(','))
        c.out << tok3.value() << NL;

    // Tokenize with quoting -- using nextq()
    StrTok tok4("one,'two,two',three");
    while (tok4.nextq(','))
        c.out << tok4.value() << NL;

    return 0;
}
\endcode

\par Quoting

For delimited text Evo uses quoting unstead if escaping, which avoids having to mutate tokens by adding or filtering escape characters (like backslash).
Where quoted tokenizing is supported, multiple quoting types are supported to handle any text.

No quoting:
 - If a token doesn't begin _and_ end with quotes then it's considered unquoted
 - The parser doesn't get confused when quotes appear in unquoted tokens
 - Example comma-delimited strings that aren't considered quoted at all -- these don't confuse the parser:
   - can't,won't,'bout
     - can't
     - won't
     - 'bout
   - 'not' quoted,also not 'quoted'
     - 'not' quoted
     - also not 'quoted'
 - Example that will confuse the parser using words that begin _and_ end with an apostrophe:
   - can't,'bout,runnin',jumpin'
     - can't
     - bout,runnin -- _considered quoted, fix with actual quoting_
     - jumpin' -- _dangerous but ok here_

Single-char quoting:
 - Single-quotes: 'foo bar'
 - Double-quotes: "foo bar"
 - Backtick-quotes: \`foo bar\`

Triple quoting:
 - Triple single-quotes: '''foo bar'''
 - Triple double-quotes: """foo bar"""
 - Triple backtick-quotes: \`\`\`foo bar\`\`\`

Backtick-DEL quoting:
 - This is single-backtick quoting with a DEL char (code 7F, normally not printable but shown here as ␡) added after each backtick
   - Example: \`␡foo bar\`␡
 - The DEL char is not normally used at all with text, and can still be used as a delimiter (and won't be confused with backtick-DEL)
   - Example with DEL delimiter and backtick-DEL quoting:
     - \`␡foo bar\`␡␡\`␡stuff things\`␡
       - foo bar
       - stuff things
 - Only needed when no other quoting is possible, which is very rare

*/

////

/** \page StringFormatting %String Formatting

This shows different approaches for string formatting.

\par Direct Formatting

Methods for direct formatting modify the string directly:
\code
#include <evo/string.h>
using namespace evo;

int main() {
    String str;

    // Append a separator, if applicable
    str.addsep(',');    // ignored, string is empty

    // Set as formatted integer
    str.setn(123);
    str.setn(0xFF, 16); // hex

    // Set as formatted floating point
    str.setn(1.23);     // default precision
    str.setn(1.23, 2);  // 2 digit precision

    // Append a separator, if applicable
    str.addsep(',');    // add separator
    str.addsep(',');    // ignored, already ends with separator

    // Append formatted number with similar methods
    str.addn(123);
    str.addn(1.23);

    // Prepend formatted number with similar methods
    str.prependn(123);
    str.prependn(1.23);

    // Insert formatted number with similar methods, at position 1
    str.insertn(1, 123);
    str.insertn(1, 1.23);

    return 0;
}
\endcode

\par Stream Style Formatting

Strings can be appended to using the stream operator (<<), similar to output \ref StreamFormatting "Stream Formatting":
\code
#include <evo/string.h>
using namespace evo;

int main() {
    String str;

    // Append string and number
    str << "Testing " << 123;

    // Clear string and append new string and number
    str.clear() << "Foobar " << 1.23;

    return 0;
}
\endcode

Strings can also use explicit stream formatting to customize the output:
\code
#include <evo/string.h>
using namespace evo;

int main() {
    String str;

    // Format str as: Testing aaaa...
    str.clear() << "Testing " << FmtChar('a', 4) << "...";

    // Format str as: Testing 0123...
    str.clear() << "Testing " << FmtInt(123, fDEC, 4) << "...";

    // Format str as: Testing 0x07B...
    str.clear() << "Testing " << FmtUInt(0x7B, fHEX, fPREFIX2, 3) << "...";

    // Format str as: Testing 1.230...
    str.clear() << "Testing " << FmtFloat(1.23f, 3) << "...";

    // Format str as: Testing 01.230...
    str.clear() << "Testing " << FmtFloatD(1.23, 3, 6) << "...";

    return 0;
}
\endcode

Formatting attributes must be explicitly passed with each value -- String doesn't keep any formatting state.

Formatting types used with `operator<<()`:
 - \link Newline\endlink: \ref NL, \ref NL_SYS, \ref nLF, \ref nCRLF, etc
 - \ref FmtInt, \ref FmtInt32, \ref FmtLong, \ref FmtUInt, \ref FmtUInt32, \ref FmtULong -- for more int variants see FmtIntT
 - \ref FmtFloat, \ref FmtFloatD, \ref FmtFloatL
 - FmtChar
 - FmtDump

\par Stream Style Formatting With State

Create a String::Format object to keep formatting state and make formatting "sticky" through that object, similar to Stream::Format:
\code
#include <evo/string.h>
using namespace evo;

int main() {
    String str;
    String::Format out(str);

    // Set int formatting to use hex with prefix, padded to 2 digits
    out << FmtSetInt(fHEX, fPREFIX2, 2);

    // Format some numbers as: 0x01,0x02,0x03
    out << 1 << ',' << 2 << ',' << 3;

    // Set int formatting to use octal with no prefix
    out << fOCT << fPREFIX0;

    // Format some numbers in octal as: 7,10,11
    out.str.clear();
    out << 7 << ',' << 8 << ',' << 9;

    // Set int formatting to use decimal and pad to 3 digits, set field width to 5 and align-right
    out << FmtSetInt(fDEC, 3) << FmtSetField(fRIGHT, 5, '.');

    // Format some numbers in decimal as: ..012,..034
    out.str.clear();
    out << 12 << ',' << 34;

    return 0;
}
\endcode

This can also be done as a one-liner using a temporary formatter:
\code
#include <evo/string.h>
using namespace evo;

int main() {
    String str;

    // Format some numbers as hex with prefix: 0x1,0x2,0x3
    String::Format(str) << fHEX << fPREFIX2 << 1 << ',' << 2 << ',' << 3;

    return 0;
}
\endcode

Sticky formatting is useful for applying the same formatting to many fields.
You may still use explicit stream formatting (see previous section) to override sticky formatting.

Formatting types for setting attributes with `operator<<()`
 - \link FmtBase\endlink: \ref fDEC, \ref fHEX, etc -- can cast an int to this type for base
 - \link FmtBasePrefix\endlink: \ref fPREFIX0, \ref fPREFIX1, \ref fPREFIX2
 - \link FmtPrecision\endlink: \ref fPREC_AUTO, \ref fPREC0, \ref fPREC2, \ref fPREC6, etc -- can cast an int to this type for precision
 - \link FmtAlign\endlink: \ref fLEFT, \ref fCENTER, \ref fRIGHT
 - \link FmtWidth\endlink: \ref fWIDTH0 -- cast an int to this type for field width
 - FmtSetNull
 - FmtSetField
 - FmtSetInt
 - FmtSetFloat

\par Generic Conversion Formatting

Generic conversion and formatting is supported via templates:
\code
#include <evo/string.h>
using namespace evo;

int main() {
    String str;

    // set
    str.convert_set<Bool>(true);
    str.convert_set<Int>(123);
    str.convert_set<FloatD>(1.2);
    str.convert_set<int>(123);
    str.convert_set<double>(1.2);

    // append
    str.convert_add<Bool>(true);
    str.convert_add<Int>(123);
    str.convert_add<FloatD>(1.2);
    str.convert_add<int>(123);
    str.convert_add<double>(1.2);

    return 0;
}
\endcode

See also: \ref StringConversion
*/

////

/** \page StreamFormatting %Stream Formatting

Evo supports extensive stream formatting.

\par Stream Formatting

Text streams can be written with formatting using the stream operator (<<).

The following examples write format to stdout, but also apply to any Stream type (File, Socket, PipeOut, etc).

\code
#include <evo/io.h>
using namespace evo;

int main() {
    // Get console streams, use stdout
    Console& c = con();

    // Write a line with a string and number
    c.out << "Testing " << 123 << NL;

    // Write another with another string and number
    c.out << "Foobar " << 1.23 << NL;

    return 0;
}
\endcode

Streams also support explicit formatting to customize the output:
\code
#include <evo/io.h>
using namespace evo;

int main() {
    // Get console streams, use stdout
    Console& c = con();

    // Format line as: Testing aaaa...
    c.out << "Testing " << FmtChar('a', 4) << "..." << NL;

    // Format line as: Testing 0123...
    c.out << "Testing " << FmtInt(123, fDEC).width(4) << "..." << NL;

    // Format line as: Testing 0x07B...
    c.out << "Testing " << FmtUInt(0x7B, fHEX, fPREFIX2, 3) << "..." << NL;

    // Format line as: Testing 1.230...
    c.out << "Testing " << FmtFloat(1.23f, 3) << "..." << NL;

    // Format line as: Testing 01.230...
    c.out << "Testing " << FmtFloatD(1.23, 3, 6) << "..." << NL;

    return 0;
}
\endcode

Formatting attributes must be explicitly passed with each value -- Stream itself doesn't keep any formatting state.

Formatting types used with `operator<<()`:
 - \link Newline\endlink: \ref NL, \ref NL_SYS, \ref nLF, \ref nCRLF, etc
 - \ref FmtInt, \ref FmtInt32, \ref FmtLong, \ref FmtUInt, \ref FmtUInt32, \ref FmtULong -- for more int variants see FmtIntT
 - \ref FmtFloat, \ref FmtFloatD, \ref FmtFloatL
 - FmtChar
 - FmtDump

\par Stream Newlines

Streams have a default newline value (see constructor) formatted with \link NL\endlink -- default is determined by the stream itself.

\code
#include <evo/io.h>
using namespace evo;

int main() {
    {
        // Write file with Windows newlines (nCRLF) by default (regardless of system)
        File file("windows.txt", oWRITE_NEW, nCRLF);
        file << "Windows" << NL;

        // Add string with newline (converted to default newline)
        file << "foo\n";

        // You can also explicitly add other newline chars (not converted)
        file << nLF;

        // Or write system default newline (depends on system)
        file << NL_SYS;
    }
    {
        // Write file with Linux/Unix newlines (nLF) by default (regardless of system)
        File file("linux.txt", oWRITE_NEW, nLF);
        file << "Linux/Unix" << NL;
    }

    return 0;
}
\endcode

\par Stream Formatting With State

Create a Stream::Format object (where Stream is the stream type used) to keep formatting state and make formatting "sticky" through that object, similar to String::Format:
\code
#include <evo/io.h>
using namespace evo;

int main() {
    // Get formatting object, use stdout
    Console::Format out(con().out);

    // Set int formatting to use hex with prefix, padded to 2 digits
    out << FmtSetInt(fHEX, fPREFIX2, 2);

    // Format line with some numbers as: 0x01,0x02,0x03
    out << 1 << ',' << 2 << ',' << 3 << NL;

    // Set int formatting to use octal with no prefix
    out << fOCT << fPREFIX0;

    // Format line with some numbers in octal as: 7,10,11
    out << 7 << ',' << 8 << ',' << 9 << NL;

    // Set int formatting to use decimal and pad to 3 digits, set field width to 5 and align-right
    out << FmtSetInt(fDEC, 3) << FmtSetField(fRIGHT, 5, '.');

    // Format line with some numbers in decimal as: ..012,..034
    out << 12 << ',' << 34 << NL;
}
\endcode

This can also be done as a one-liner using a temporary formatter:
\code
#include <evo/io.h>
using namespace evo;

int main() {
    // Open new file for writing
    File file("file.txt", oWRITE_NEW);

    // Format line with some numbers as hex with prefix: 0x1,0x2,0x3
    File::Format(file) << fHEX << fPREFIX2 << 1 << ',' << 2 << ',' << 3 << NL;

    return 0;
}
\endcode

Sticky formatting is useful for applying the same formatting to many fields.
You may still use explicit stream formatting (see previous section) to override sticky formatting.

Formatting types for setting attributes with `operator<<()`
 - \link FmtBase\endlink: \ref fDEC, \ref fHEX, etc -- can cast an int to this type for base
 - \link FmtBasePrefix\endlink: \ref fPREFIX0, \ref fPREFIX1, \ref fPREFIX2
 - \link FmtPrecision\endlink: \ref fPREC_AUTO, \ref fPREC0, \ref fPREC2, \ref fPREC6, etc -- can cast an int to this type for precision
 - \link FmtAlign\endlink: \ref fLEFT, \ref fCENTER, \ref fRIGHT
 - \link FmtWidth\endlink: \ref fWIDTH0 -- cast an int to this type for field width
 - FmtSetNull
 - FmtSetField
 - FmtSetInt
 - FmtSetFloat
*/

////

/** \page StringCustomConversion Custom %String Conversion and Formatting

This shows ways to convert between types using conversion templates.

_Note: Automated conversion here is "best effort" and never throws an exception_

\par Conversion to Custom Types

Conversion is extendable for custom types by specializing the Convert template class (in `evo` namespace). This is used to:
 - %Convert string to a value and return it -- used by methods like
   \link String::convert() convert()\endlink and \link String::split() split()\endlink.
 - Format string with value (reversed conversion) -- used by methods like
   \link String::convert_set() convert_set()\endlink.
 - Format value and append to string (reversed conversion) -- used by methods like
   \link String::convert_add() convert_add()\endlink and \link String::join() join()\endlink.
 .

Note: Include evo/type.h for conversion features

Example for converting String to bool -- this is just an example and is already implemented:
\code
#include <evo/type.h>

namespace evo {

template<> struct Convert<String,bool> {
    static void set(String& dest, bool value) {
        if (value)
            dest.set("true", 4);
        else
            dest.set("false", 5);
    }

    static void add(String& dest, bool value) {
        if (value)
            dest.add("true", 4);
        else
            dest.add("false", 5);
    }

    static bool value(const String& src) {
        return src.getbool<bool>();
    }
};

}
\endcode

Some conversions make sense one way but not the other. Here's an example for converting `char*` to String -- again just an example and is already implemented:
\code
#include <evo/type.h>

namespace evo {

template<> struct Convert<String,char*> {
    static void set(String& dest, const char* value) {
        dest.set(value);
    }

    static void add(String& dest, const char* value) {
        dest.add(value);
    }

    // Unsafe: Converting String to char* -- not defined so trying this conversion will give a compiler error
    //template<class U> static char* value(U&)
};

}
\endcode

See also: \ref StringStreamCommon
*/

////

/** \page StringStreamCommon Common %Stream/%String Interface

String, Stream, and StreamOut have a common formatting interface, but not a common base class. Still,
common code can use these different types via template parameters.

\par Common Interface

String, Stream, and StreamOut support this common interface:
 - `Format` nested class for formatting with state
   - Format objects don't support the Advanced members below, use `write_out()` to call them
   - See String::Format or Stream::Format
 - `operator<<()` methods for formatting various types
 - `This& write_out()` -- Get reference to parent stream/string (that isn't a `Format` type)
 - Advanced -- not supported by `Format` type so use write_out() to get stream/string:
   - typedef `Size` -- Number type for data size parameters (always unsigned)
   - `Size writetext(const char* buf, Size size)` -- Write (append) text to stream/string
   - `ulong writefmtstr(const char* buf, ulong size, const FmtSetField& field)` -- Write (append) formatted string with field alignment to stream/string
   - `bool writefmtdump(const FmtDump& fmt, Newline nl=NL)` -- Write (append) formatted data dump to stream/string
   - Numbers with `template<class TNum>`:
     - `bool writenum(TNum num, int base=fDEC)` -- Append formatted signed number to stream/string
     - `bool writenumu(TNum num, int base=fDEC)` -- Append formatted unsigned number to stream/string
     - `bool writenumf(TNum num, int precision=fPREC_AUTO)` -- Append formatted floating point number to stream/string
     - `bool writefmtnum(TNum num, const FmtSetInt &fmt, const FmtSetField *field=NULL)` -- Append formatted signed number with field alignment to stream/string
     - `bool writefmtnumu(TNum num, const FmtSetInt &fmt, const FmtSetField *field=NULL)` -- Append formatted unsigned number with field alignment to stream/string
     - `bool writefmtnumf(TNum num, const FmtSetFloat &fmt, const FmtSetField *field=NULL)` -- Append formatted floating point number with field alignment to stream/string
   - `char* write_direct(Size size)` -- Setup to append directly to internal buffer of stream/string
   - `char* write_direct_multi(Size& available, Size reserve_size)` -- Setup to append directly to internal buffer of stream/string and allow multiple passes for larger data
   - `char* write_direct_flush(Size& available, Size written_size, Size reserve_size)` -- Flush data written directly to internal buffer of stream/string and setup to append more with another pass
   - `bool write_direct_finish(Size size)` -- Finish appending directly to internal buffer of stream/string
 .

Additional custom `operator<<()` support may be defined using global inline functions (see example below).

\par Formatting via Template Functions

It can be useful to write a single function that can write formatted output to any Stream or String type.
Strings are not a stream type but this is possible with a template function.

\code
#include <evo/string.h>
#include <evo/io.h>
using namespace evo;

// This works with all output stream and string types, including: Stream, StreamOut, Stream::Format, String, String::Format
template<class T>
inline void write_nums(T& out) {
    // Write line with comma-separated numbers
    out << 1 << ',' 2 << ',' << 20 << NL;
}

int main() {
    // Write line to stdout: 1,2,20
    Console& c = con();
    write_nums(c.out);

    // Write line to stdout via formatting object, change int formatting to 2-digit hex: 01,02,14
    write_nums(Console::Format(c.out) << FmtSetInt(fHEX, 2));

    // Write line to a string: 1,2,20
    String str;
    write_nums(str);

    // Write line to string, change int formatting to 2-digit hex: 01,02,14
    write_nums(String::Format(c.out) << FmtSetInt(fHEX, 2));

    return 0;
}
\endcode

\par Direct Buffer Formatting

This shows a template function that writes directly to a String or Stream buffer.
 - This is more efficient than pre-formatting in a temporary buffer and copying from there
 - Note that direct writes aren't considered text so newlines from it aren't converted
 - Stream types must have buffering enabled (which they do by default), otherwise this fails as there's no buffer to write to
 - This assumes the write size is small -- see next section for writing larger sizes that may not fit in the write buffer
 .

Example:
\code
#include <evo/string.h>
#include <evo/io.h>
using namespace evo;
static Console& c = con();

// This works with all output stream and string types, including: Stream, StreamOut, Stream::Format, String, String::Format
template<class T>
inline bool write_hello(T& out) {
    const typename T::Size SIZE = 5;

    char* buf = out.write_out().write_direct(SIZE);
    if (buf == NULL)
        return false;   // Error making room

    // Write directly to buffer
    memcpy(buf, "Hello", SIZE);
    out.write_out().write_direct_finish(SIZE);

    return true;
}

int main() {
    // Write to String, print: Hello
    String str;
    write_hello(str);
    c.out << str << NL;

    // Reset String and write to String via a Format object, print: Hello
    str.set();
    {
        String::Format fmt(str);
        write_hello(fmt);
        c.out << str << NL;
    }

    // Write to StreamOut, print: Hello
    write_hello(c.out);
    c.out << NL;

    return 0;
}
\endcode

\par Direct Buffer Formatting with Larger Data

Writing larger data sizes may require multiple passes with Stream types since they use a fixed size buffer:
 - This is more efficient than pre-formatting in a temporary buffer and copying from there
 - Note that direct writes aren't considered text so newlines from it aren't converted
 - Stream types must have buffering enabled (which they do by default), otherwise this fails as there's no buffer to write to
 - This works with any data size
 .

\code
#include <evo/string.h>
#include <evo/io.h>
using namespace evo;
static Console& c = con();

// This works with all output stream and string types, including: Stream, StreamOut, Stream::Format, String, String::Format
template<class T>
inline bool write_repeated_char(T& out, char ch, uint count) {
    typename T::Out& outstream = out.write_out();
    typename T::Out::Size available = 0;

    // Make room to write directly to buffer
    char* buf = outstream.write_direct_multi(available, count);
    if (buf == NULL)
        return false;   // Stream error

    // Write directly to buffer with multiple passes if needed
    while (count > 0) {
        memset(buf, ch, available);
        buf = outstream.write_direct_flush(available, available, count);
        if (buf == NULL)
            return false;   // Stream error
        count -= available;
    }

    return true;
}

int main() {
    // Write to String, print 10,000 occurrences of 'a'
    String str;
    write_repeated_char(str, 'a', 10000);
    c.out << str << NL;

    // Reset String and write to String via a Format object, print 10,000 occurrences of 'b'
    str.set();
    {
        String::Format fmt(str);
        write_repeated_char(fmt, 'b', 10000);
        c.out << str << NL;
    }

    // Write to StreamOut, print 10,000 occurrences of 'c'
    write_repeated_char(c.out, 'c', 10000);
    c.out << NL;

    return 0;
}
\endcode

\par Custom Output Operator Overloads

This shows a custom class with a templated `format()` method that works with all string/stream types and
defines a global string/stream output operator that uses the `format()` method.

\code
#include <evo/string.h>
#include <evo/io.h>
using namespace evo;

// Custom class with a format method that works with all string/stream types
struct CustomClass {
    template<class T>
    void format(T& out) const {
        out << "Hello";
    }
};

// String/Stream formatting operator with CustomClass
template<class T>
inline T& operator<<(T& out, const CustomClass& obj) {
    obj.format(out);
    return out;
}

int main() {
    // Create object and format it, print: Hello
    CustomClass obj;
    con().out << obj << NL;
    return 0;
}
\endcode
*/

///////////////////////////////////////////////////////////////////////////////

/** \page UnsafePtrRef Unsafe Pointer Referencing

Some Evo classes use Pointer Referencing to copy without allocating memory, improving performance.
This can be _unsafe_ (dangerous) in certain situations so documentation for classes/methods that do this will have a warning and a link to this page.

 - These features may be avoided for maximum safety -- Evo will still optimize where it can and will still be fast
 - Some of these features may still be used with minimal risk

\par Definition

Unsafe Pointer Referencing means an object is using (referencing) a pointer it does not own that may become invalid.
If the actual owner frees this pointer then the object referencing it will be invalidated, and dereferencing the pointer will crash.
You're safe as long as the object referencing the pointer is cleared/destroyed before the pointer is invalidated.
If the pointer is never freed (ex: string literal) then this is always safe.

\par Methods

The following classes have certain methods that use this technique:
 - List, String

This technique is used by method overloads that take a raw pointer:
 - Constructor
 - `operator=(src)`
 - `set(src)`

Safety:
 - Referencing static data that is never freed is safe<br>
   Example:\code
    evo::String str;
    str = "test";                   // Safe!
   \endcode
 - Best: Avoid overloads that take a raw pointer, pass Ptr instead (forces an unshared copy)<br>
   Example:\code
    evo::String str;
    {
        char buffer[5];
        strcpy(buffer, "test");
        str = Ptr<char>(buffer);    // Safe (unshared copy)
    }
   \endcode
 - Call `unshare()` after Pointer Reference copy to force an unshared copy<br>
   Example:\code
    evo::String str;
    {
        char buffer[5];
        strcpy(buffer, "test");
        str = buffer;               // Unsafe without next line
        str.unshare();              // Safe (unshared copy)
    }
   \endcode
 - Use `copy()` to force an unshared copy<br>
   Example:\code
    evo::String str;
    {
        char buffer[5];
        strcpy(buffer, "test");
        str.copy(buffer);           // Safe (unshared copy)
    }
   \endcode

\par Classes

The following classes rely on Pointer Referencing:
 - SubString, SubList, Ptr

Safety:
 - Referencing static data that is never freed is safe<br>
   Example:\code
    evo::SubString str;
    str = "test";           // Safe!
   \endcode
 - Beware referencing a local buffer with shorter life/scope<br>
   Example:\code
    evo::SubString substr;
    {
        char buffer[5];
        strcpy(buffer, "test");
        substr = buffer;    // BAD!
    }
    // substr now references an invalid pointer!
   \endcode
 - Beware referencing an object with a shorter life/scope<br>
   Example:\code
    evo::SubString substr;
    {
        String str;
        str.copy("testing");
        substr = str;       // BAD!
    }   // str is destroyed
    // substr now references an invalid pointer!
   \endcode
 - Beware of modifying (reallocating) an object while it's being referenced<br>
   Example:\code
    evo::String str;
    str.copy("test");

    evo::SubString substr;
    substr = str;
    str.copy("test123");    // BAD! substr now references an invalid pointer since str was reallocated!
   \endcode
 - Classes that support \ref Sharing with each other (List, String) are safe to copy and modify in any order<br>
   Example:\code
    evo::String str1;
    str1.copy("test");

    String str2(str1);      // str1 and str2 safely Share same data
    str1.copy("test123");   // Safe since Sharing covers modifications on either side
   \endcode

\par Read-Only Arguments

The above classes are useful for passing arguments while making the intent of passing read-only data more clear.
This is especially useful in multithreaded code that may pass data between threads (with approproiate synchronization).

See also: \ref StringPassing

Example:\code
#include <evo/substring.h>
#include <evo/io.h>
using namespace evo;
static Console& c = con();

// Passing as SubString makes it clear str is read-only
void print(const SubString& str) {
    c.out << str << NL;
}

int main() {
    String str("test 123");
    print(str);
    print("test 456");
    return 0;
}
\endcode

Output:
\code{.unparsed}
test 123
test 456
\endcode

This is safe as long as the function assumes referenced data will not persist.

A _bad_ example:\code
#include <evo/substring.h>
using namespace evo;

struct MyData {
    SubString mydata;   // DANGER: Ownership isn't clear, use String instead

    void store(const SubString& str) {
        mydata = str;   // DANGER!
    }
};

int main() {
    MyData data;
    data.store("test 123");
    return 0;
}
\endcode
*/

///////////////////////////////////////////////////////////////////////////////
}
//@}
#include "core.h"
#endif
