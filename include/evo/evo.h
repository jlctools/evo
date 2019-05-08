// Evo C++ Library
/* Copyright 2019 Justin Crowell
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

<a href="https://github.com/jlctools/evo">Home on Github</a>

Evo is a modern high performance C++ library designed to make writing efficient and portable code easy.
It covers:
 - core types, strings, and containers
 - string tokenization and parsing
 - file/pipe/socket streams and formatting
 - async I/O and server tools
 - logging and command-line tools
 - threads and atomics
 - C++03, C++11, and newer
 .

Inspired by modern languages like [Python](https://www.python.org) and [D](https://dlang.org), Evo is the result of many years of lessons learned from writing (and rewriting) high-performance,
object oriented C++ server code. Evo aims to make C++ easier and more powerful without sacrificing performance.

 - \ref WhatsNew "What's New?"
 - \ref DesignGoals "Design Goals"
 - \ref LicenseCredits "License and Credits"
 - \ref CppCompilers "C++ Compilers & Optimization"
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
 - \ref Async "Asynchronous I/O" (_Alpha_)
 - \ref Threads "Threads"
 - \ref Metaprogramming "Metaprogramming & Macros"
 - \ref SmartQuoting "Smart Quoting"
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
 - \ref AdditionalFormatting "Additional Formatting"
 - \ref StringCustomConversion "Advanced: Custom String Conversion/Formatting"
 - \ref StringStreamCommon "Advanced: Common Stream/String Interface"
 .
 - \ref FullServer "Full Server Example"
 .
</td><td valign="top">

\par Tools

 - \link CommandLineT CommandLine\endlink
 - Logger, LoggerConsole, LoggerPtr
   - EVO_LOG_ALERT(), EVO_LOG_ERROR(), EVO_LOG_WARN()
   - EVO_LOG_INFO(), EVO_LOG_DEBUG(), EVO_LOG_DEBUG_LOW()
 - FilePath
 - Directory
 - Signal
 - Benchmark
 .
 - get_pid(), get_tid()
 - get_cwd(), get_abspath(), set_cwd()
 - daemonize()
 .
 - Cortex, CortexModuleBase
 .

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
 - \link ValNull\endlink
 .

</td><td valign="top">

\par Containers

 - Array, List, SubList
   - \link BitArrayT BitArray\endlink, \link BitArraySubsetT BitArraySubset\endlink
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
   - lookupsub(), map_contains()
   - EVO_MAP_FIELDS(), EVO_MAP_FIELDS_KEY()
   .
 - Var
 - BufferQueue
 .

 - Compare, CompareR, CompareI, CompareIR
 - CompareHash
 - fixed_array_size(), EVO_FIXED_ARRAY_SIZE()
 - is_null()
 .

</td><td valign="top">

\par Enum Helpers

 - EVO_ENUM_MAP(), EVO_ENUM_MAP_PREFIXED()
   - EVO_ENUM_REMAP(), EVO_ENUM_REMAP_PREFIXED()
 - EVO_ENUM_CLASS_MAP()
   - EVO_ENUM_CLASS_REMAP()
 - EnumIterator, EnumMapIterator
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
 - FmtTable, fmt_table(), fmt_table_nocache()
 .
</td><td valign="top">

\par Asynchronous I/O

_Alpha: Work In Progress_

 - \link async::MemcachedClient MemcachedClient\endlink, \link async::MemcachedServerHandlerBase MemcachedServerHandlerBase\endlink
 - AsyncClient, AsyncServer
 .
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
   - AtomicBufferQueue
 .
 </td><td valign="top">

\par Events

 - EventQueue, Event, EventLambda
 - EventThreadPool
 .

\par Date/Time

 - DateTime
 - Date, TimeOfDay
 .
 - \link Timer\endlink, \link TimerCpu\endlink
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
 - \ref EVO_NOEXCEPT
 .

</td></tr></table>

----

BSD 2-Clause License

Copyright 2019 Justin Crowell

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

\par Version 0.5.1 - May 2019
 - Add EVO_ENUM_TRAITS() and EVO_ENUM_CLASS_TRAITS()
 - Add \ref AdditionalFormatting with FmtTable, fmt_table(), fmt_table_nocache()
 - Add PureBase for a common empty base class
 - Add STL `string_view` compatibility to \ref StringBase
 - Updated and simplified Cortex, removed cortex_module.h and moved contents to cortex.h
   - Renamed `ModuleBase` to `CortexModuleBase`, and renamed related types too
 - Updated Benchmark: Add warmup_count to constructor, simplify helper macros, improved report output
 - Updated StrTok (and variants) to use `const StringBase&` arguments to support for STL string compatibility
 - Updated List and String methods: add/prepend/insert/replace now support copying from SubString or pointer in the same list/string (previously undefined behavior)
   - Note that String::replace() where source and destination overlap still results in undefined behavior
 - Bug fix: EventThreadPool not working correctly in MSVC 2017, MSVC 2013, FreeBSD
 - Bug fix: Cortex items not "relocatable", insert order no longer tracked
 - Bug fix: GCC 4.6 and 4.4 compiler errors
 - Bug fix: GCC warnings in var.h
 - Bug fix: MSVC security warning on _open()
 - Bug fix: Cygwin GCC x86 signed/unsigned warnings
 - Bug fix: Fixed missing async dir in previous release
 - Various documentation updates
 - Add FAQ and benchmarks in <a href="https://github.com/jlctools/evo">Github Home</a>

\par Version 0.5 - Apr 2019
 - Async I/O:
   - Add \link async::MemcachedClient MemcachedClient\endlink, \link async::MemcachedServerHandlerBase MemcachedServerHandlerBase\endlink
   - Add AsyncClient, AsyncServer
   - Add EventThreadPool, EventQueue, Event
 - Tools:
   - Add \link CommandLineT CommandLine\endlink processing
   - Add Cortex and ModuleBase
   - Add Logger, LoggerConsole, LoggerPtr
   - Add FilePath and Directory classes
     - Add get_cwd(), get_abspath(), set_cwd()
     - Move File class to `file.h`
   - Add Signal, daemonize()
 - Date/Time:
   - Add DateTime, Date, TimeOfDay, TimeZoneOffset classes
   - Add \link TimerCpu\endlink, Benchmark
     - \link Timer\endlink internal refactoring to support nanosecond precision and CPU time
     - Update \link Timer\endlink to use monotonic clock when available
 - Strings:
   - Add FmtString and FmtStringWrap string/stream formatting
   - Add `token*()`, `findany*()`, `find*word*()` methods to String and SubString
   - Add String `findreplace()` methods
   - Add `strip2*()` and `strip*_newlines()` methods to String and SubString
   - Add `addnew()` and `add()` methods to StringInt
 - Misc containers:
   - Add Var class for general variable types and structures
   - Add `asconst()`, `begin()`, `end()`, `cbegin()`, `cend()` to Evo containers
   - Add IsNullable and is_null()
   - Add List methods: `addmin()`, `firstM()`, `lastM()`, `advFirst()`, `advLast()`
   - Add mutable `value()` and `operator*()` methods in Nullable
 - Synchronization:
   - Add Mutex::trylock() with timeout
   - Add Condition::trylock()
   - Condition::wait() now uses trylock() with timeout to lock
 - Enum Conversion:
   - Add enum helpers for unsorted enum values: EVO_ENUM_REMAP(), EVO_ENUM_REMAP_PREFIXED(), EVO_ENUM_CLASS_REMAP()
   - Add EnumIterator and `Iter` type to types created by helpers like EVO_ENUM_MAP()
   - Add enum int conversion methods
   - Moved enum macros from substring.h to enum.h
   - See \ref EnumConversion
 - C++11:
   - Add C++11 features to Evo containers: range-based for loop, initializer lists, move semantics
   - Add C++11 UTF-16 string iteral (`u` prefix) support to UnicodeString
 .
 - Add SSE optimizations, see \ref CppCompilers
   - StrTok and related tokenizer refinements and optimizations, especially with \ref SmartQuoting
 - Add ConsoleNotMT
 .
 - Add advanced macros, see \ref Metaprogramming
   - Add Compiler Helpers in \ref Metaprogramming
 - Add \ref EVO_NOEXCEPT and \ref EVO_VERSION_STRING
 - Add NewlineValue for recognizing and storing default vs specific newline types
 .
 - Updated MSVC compiler detection for MSVC 2017 15.9
 - Bug fix: BitArrayT::Iter doesn't find first bit with small array size
 - Bug fix: BitArrayT edge case in: `store()`, `setbits()`, `countbits()`, `checkall()`, `checkany()`, `toggle_multi()`
 - Fix maphash.h compiler errors in GCC 4.8
 - Many documentation updates

\par Version 0.4 - Oct 2018
 - Now using <a href="https://opensource.org/licenses/BSD-2-Clause">BSD 2-Clause License</a>
 - Add initial \ref Unicode support to String, add UnicodeString
 - Add updated \ref StringFormatting "String Formatting" and \ref StreamFormatting "Stream Formatting"
   - Replaced PREC_AUTO constant with \ref fPREC_AUTO
   - Stream interface tweaks, using void* instead of char*
 - Add String and SubString `find*()` methods for string searching
 - Add ConsoleMT and support for thread-safe (synchronized) I/O
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
 - Straightforward interfaces that are easy to understand and use, differentiate basic and advanced features
 - Header-only library that's easy to integrate with projects
 - Good and thorough documentation with examples
 - Structured naming to make related things naturally sort together -- ex: MapList, MapHash, \link evo::String::split split()\endlink, \link evo::String::splitr splitr()\endlink, \link evo::String::splitat splitat()\endlink
 - Portability before and after C++11
 - Cross platform: Linux, Unix, MacOS, Windows, Cygwin
 - Tested with at least 99% code coverage
 .
*/

///////////////////////////////////////////////////////////////////////////////

// NOTE: Compilers copied to README.txt (bottom)
/** \page CppCompilers C++ Compilers & Optimization

Evo at least requires a C++03 (ideally C++11) compliant compiler. Without C++11, Evo uses some compiler specific features, most notably with threads and atomic operations.

\par Optimization

Evo uses optimizations that take advantage of SSE 4.2 and SSE 2 CPU instructions (via compiler intrinsics). These are mainly used for string scanning/parsing and bit manipulation.

 - Most compilers enable at least SSE 2 instructions by default, which is supported by all x64 CPUs
 - Evo follows the compiler options by default (using macros set by compiler) -- defaults are generally preferred, but you may fine-tune for more specific hardware
   - Note that Evo uses SSE 4.2 by default when AVX or AVX2 is enabled with the compiler
 - When fine tuning for best performance, testing and benchmarking is recommended as results may vary

Notes for best performance:
 - GCC & Clang
   - Compile with `-msse4.2` to enable optimizations for SSE 4.2 instructions (if requiring a CPU with SSE 4.2 support is acceptable)
   - You can also optimize further for current or specific hardware with `-march=` and `-tune=`
   - See: https://gcc.gnu.org/onlinedocs/gcc/x86-Options.html
   - Tools like valgrind may have issues with SSE 4.2 instructions, with errors like: Unknown instruction/opcode
 - MSVC:
   - `Code Generation` is configured under `C/C++` configuration for project, or with `/arch` option
     - MSVC doesn't have an option to enable SSE 4.2 -- if needed enable this by defining `EVO_USE_SSE42` (before including Evo headers) (if requiring a CPU with SSE 4.2 support is acceptable)
     - You can also enable Evo SSE 4.2 use by enabling AVX code generation (if requiring a CPU with AVX support is acceptable)
   - See: https://docs.microsoft.com/en-us/cpp/build/reference/arch-minimum-cpu-architecture?view=vs-2017
 .

\par Testing

Tested on the following systems and compilers:

Ubuntu
 - 18.04: gcc 7.4.0, clang 6.0.0, valgrind 3.13.0 -- Intel Core i7
 - 16.04: gcc 5.4.0, clang 3.8.0, valgrind 3.11.0 -- Intel Core i7
 - 14.04: gcc 4.8.4, clang 3.4.0, valgrind 3.10.1 -- VM
   - Also: gcc 4.6.4, gcc 4.7.3

CentOS -- VM
 - 7.6: gcc 4.8.5, clang 3.4.2, valgrind 3.13.0
 - 6.10: gcc 4.4.7, clang 3.4.2

FreeBSD -- VM
 - 12.0: clang 6.0.1, valgrind 3.10.1

MacOS -- Intel Core i5
 - 10.13.6: Apple LLVM 10.0.0 (clang 1000-10.44.4)

Windows 10 -- Intel Core i7
 - <a href="https://www.visualstudio.com/vs/cplusplus/">MSVC</a> 2017 Community 15.9.11, libevent 2.1.8, libevent 2.0.22
 - MSVC 2015 Community Update 3

Windows 7 SP1 32-bit -- VM
 - MSVC 2013 Community Update 5, libevent 2.1.8, libevent 2.0.22
 - <a href="https://www.cygwin.com/">cygwin 3.0.6</a>: gcc 7.4.0

\par Notes

 - All tested systems are 64-bit, unless otherwise noted
 - Microsoft Visual C++:
   - Before 2015: <a href="https://msdn.microsoft.com/en-us/library/hh567368.aspx">Static var initialization is not thread-safe ("magic statics")</a>
   - 2013: Atomic or AtomicPtr with const template param causes release build compiler error
   - Before 2012: <a href="https://msdn.microsoft.com/en-us/library/hh874894(v=vs.110).aspx">Atomics not supported</a>, but Evo will try to emulate atomics with a mutex (this may not work well)
 - Cygwin:
   - For C++11 (or newer), enable with the `gnu++` option variant to avoid compile errors, ex: `g++ -std=gnu++11`
 - Older compilers (like gcc 4.4 or clang 3.4) should use `-fno-strict-aliasing` to avoid warnings and stability issues
   - RedHat GCC 4.4 on CentOS 6 has a vtable optimization bug, <a href="https://stackoverflow.com/questions/41810222/pure-virtual-function-called-on-gcc-4-4-but-not-on-newer-version-or-clang-3-4">see here</a>
 - See also: \ref BuildDependencies*/

///////////////////////////////////////////////////////////////////////////////

/** \page BuildDependencies Build Dependencies

Evo is a self-contained library but \ref Async currently requires libevent 2.0+. This gives some tips on installing this dependency.

\par Windows / MSVC

Building newer libevent (with cmake):
 - Download and install cmake: https://cmake.org/download/
   - %Set install option to add to system path
 - Download libevent source and extract it: https://libevent.org/
 - Open MSVC Command Prompt, cd to extracted dir
 - Build:
   \code
     > cmake -G "NMake Makefiles" -DEVENT__DISABLE_OPENSSL=ON -DEVENT__DISABLE_TESTS=ON -DEVENT__DISABLE_BENCHMARK=ON ..
     > nmake
   \endcode
 - Assemble results (where x.y.z is the version):
   \code
     > md libevent-x.y.z
     > xcopy ..\LICENSE libevent-x.y.z\
     > xcopy ..\ChangeLog libevent-x.y.z\
     > xcopy ..\whatsnew*.txt libevent-x.y.z\
     > xcopy /s ..\include libevent-x.y.z\include\
     > xcopy ..\WIN32-Code\nmake\event2\*.h libevent-x.y.z\include\event2\
     > del libevent-x.y.z\include\include.am
     > xcopy /s lib libevent-x.y.z\lib\
   \endcode
 .

Building older libevent (before was cmake supported):
 - Download libevent source and extract it: https://libevent.org/
 - Open MSVC Command Prompt, cd to extracted dir
 - Build:
   \code
     > nmake -f makefile.nmake
   \endcode
 - Assemble results (where x.y.z is the version):
   \code
     > md libevent-x.y.z
     > xcopy LICENSE libevent-x.y.z\
     > xcopy ChangeLog libevent-x.y.z\
     > xcopy whatsnew*.txt libevent-x.y.z\
     > xcopy /s ..\include libevent-x.y.z\include\
     > del libevent-x.y.z\include\Makefile.*
     > xcopy WIN32-Code\event2\*.h libevent-x.y.z\include\event2\
     > xcopy *.lib libevent-x.y.z\lib\
   \endcode
 .

\par Linux/Unix/MacOS

See packages:
 - Debian/Ubuntu: libevent, libevent-dev
 - RedHat/CentOS: libevent, libevent-devel
 - FreeBSD: libevent
 - MacOS (brew): libevent

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
   - Modifier methods end with an uppercase 'M' to distinguish them, these effectively call `unshare()` to make the internal buffer unshared and writable
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
 - \link ValNull\endlink
 - is_null()
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
 - `const This& asconst() const`
 - `bool null() const`
 - `bool empty() const`
 - `Size size() const`
 - `bool shared() const`
 - `This& set()`
 - `This& clear()`
 - `This& unshare()`
 - `ulong hash(ulong seed=0) const`
 .

Evo containers support C++11 features:
 - Range-based for loops
 - Initializer lists
 - Move semantics and `std::move()`

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
 - Var
 .

Related:
 - \link evo::StringBase StringBase\endlink
 - Pair
 - SubStringMapList
 - \link BitArraySubset\endlink
 - is_null()
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
 - implement `hash()` method to compute a hash value for a key, with a seed for chaining a previous hash method
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
 - Lookups are fast -- binary search is used to find the enum string in a pre-sorted list with SubStringMapList
 - This is done using EVO_ENUM_MAP_PREFIXED() or EVO_ENUM_CLASS_MAP(), or a related variant
 - \b Caution: This requires string values to be _pre-sorted_, and _no gaps_ between enum values
 .

This example defines an enum, then uses the EVO_ENUM_MAP_PREFIXED() helper to create string mappings.
 - This requires the enum to have the expected first/last guard values (`UNKNOWN`, `ENUM_END` -- with a prefix where applicable) to deduce the mappings
 - These enum values have a lowercase prefix to avoid collisions with other enums -- referred to as a "prefixed" enum
 - The enum map also has a nested `Iter` type for iterating through enum values
 .

\code
#include <evo/enum.h>
#include <evo/io.h>
using namespace evo;

enum Color {
    cUNKNOWN = 0,   // Must be first
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
    // Enum map lookups
    Color     color_val1 = ColorEnum::get_enum("green");    // set to cGREEN
    Color     color_val2 = ColorEnum::get_enum(2);          // set to cGREEN
    SubString color_str  = ColorEnum::get_string(cGREEN);   // set to "green"
    int       color_num  = ColorEnum::get_int(cGREEN);      // set to 2

    // Iterate through enum values
    Console& c = con();
    for (ColorEnum::Iter iter; iter; ++iter)
        c.out << "num:" << iter.value_num() << ", str:" << iter.value_str() << NL;

    return 0;
}
\endcode

Output (from iterator):
\code{.unparsed}
num:1, str:blue
num:2, str:green
num:3, str:red
\endcode

\par Unsorted Enum Values

This example is similar but uses an enum with unsorted values, which must be "remapped" to match the sorted string values.
 - This is useful when the enum values must be in a specific order that doesn't match the pre-sorted strings
 .

\code
#include <evo/enum.h>
using namespace evo;

enum Color {
    cUNKNOWN = 0,   // Must be first
    cRED,           // Unsorted values
    cBLUE,
    cGREEN,
    cENUM_END       // Must be last
};

static const Color COLOR_REMAP[] = {
    // Must match string order below
    cBLUE,
    cGREEN,
    cRED
};

EVO_ENUM_REMAP_PREFIXED(Color, c, COLOR_REMAP,
    // Must be sorted, and must match "remapped" enum values
    "blue",
    "green",
    "red"
);

int main() {
    Color     color_val1 = ColorEnum::get_enum("green");    // set to cGREEN
    Color     color_val2 = ColorEnum::get_enum(2);          // set to cBLUE
    SubString color_str  = ColorEnum::get_string(cGREEN);   // set to "green"
    int       color_num  = ColorEnum::get_int(cGREEN);      // set to 3
    return 0;
}
\endcode

\par Enum Traits

This example creates traits for the above `Color` enum using EVO_ENUM_TRAITS():
 - This requires the enum to have the expected first guard value (`UNKNOWN` -- with prefix where applicable) to deduce the mappings
 - The above enum map example and the below enum traits example may be combined on the same enum, creating `ColorEnum` and `ColorEnumTraits` helper types
 .

\code
#include <evo/enum.h>
#include <evo/substring.h>
using namespace evo;

enum Color {
    cUNKNOWN = 0,   // Must be first (required for traits)
    cBLUE,
    cGREEN,
    cRED,
    cENUM_END       // Must be last (not required for traits)
};

struct ColorTraits {
    int index;
    SubString code;
};

EVO_ENUM_TRAITS(Color, ColorTraits, cUNKNOWN,
    // Initialize ColorTraits for each enum value starting at cUNKNOWN, must match enum values
    { 0, "?" },
    { 1, "B" },
    { 2, "G" },
    { 3, "R" }
);

int main() {
    const ColorTraits& unknown_traits = ColorEnumTraits::get(cUNKNOWN); // index:0, code:?
    const ColorTraits& green_traits   = ColorEnumTraits::get(cGREEN);   // index:2, code:G
    return 0;
}
\endcode

\par Enum Class (C++11)

This is simplied further with C++11 enum class and EVO_ENUM_CLASS_MAP(), as long as the enum has the expected first/last guard values (`UNKNOWN`, `ENUM_END`).

\code
#include <evo/enum.h>
#include <evo/io.h>
using namespace evo;

enum class Color {
    UNKNOWN = 0,    // Must be first
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
    // Enum map lookups
    Color     color_val1 = ColorEnum::get_enum("green");        // set to Color::GREEN
    Color     color_val2 = ColorEnum::get_enum(2);              // set to Color::GREEN
    SubString color_str  = ColorEnum::get_string(Color::GREEN); // set to "green"
    int       color_num  = ColorEnum::get_int(Color::GREEN);    // set to 2

    // Iterate through enum values
    Console& c = con();
    for (ColorEnum::Iter iter; iter; ++iter)
        c.out << "num:" << iter.value_num() << ", str:" << iter.value_str() << NL;

    return 0;
}
\endcode

Output (from iterator):
\code{.unparsed}
num:1, str:blue
num:2, str:green
num:3, str:red
\endcode

\par Unsorted Enum Class Values (C++11)

This example is similar but uses an enum class with unsorted values, which must be "remapped" to match the sorted string values.
 - This is useful when the enum values must be in a specific order that doesn't match the pre-sorted strings
 .

\code
#include <evo/substring.h>
using namespace evo;

enum class Color {
    UNKNOWN = 0,    // Must be first
    RED,            // Unsorted values
    BLUE,
    GREEN,
    ENUM_END        // Must be last
};

static const Color COLOR_REMAP[] = {
    // Must match string order below
    Color::BLUE,
    Color::GREEN,
    Color::RED
};

EVO_ENUM_CLASS_REMAP(Color, COLOR_REMAP,
    // Must be sorted, and must match "remapped" enum values
    "blue",
    "green",
    "red"
);

int main() {
    Color     color_val1 = ColorEnum::get_enum("green");        // set to Color::GREEN
    Color     color_val2 = ColorEnum::get_enum(2);              // set to Color::BLUE
    SubString color_str  = ColorEnum::get_string(Color::GREEN); // set to "green"
    int       color_num  = ColorEnum::get_int(Color::GREEN);    // set to 3
    return 0;
}
\endcode

\par Enum Class Traits (C++11)

This example creates traits for the above `Color` enum class using EVO_ENUM_CLASS_TRAITS():
 - This requires the enum class to have the expected first guard value (`UNKNOWN`) to deduce the mappings
 - The above enum class map example and the below enum traits example may be combined on the same enum, creating `ColorEnum` and `ColorEnumTraits` helper types
 .

\code
#include <evo/enum.h>
#include <evo/substring.h>
using namespace evo;

enum class Color {
    UNKNOWN = 0,    // Must be first
    BLUE,
    GREEN,
    RED,
    ENUM_END        // Must be last
};

struct ColorTraits {
    int index;
    SubString code;
};

EVO_ENUM_CLASS_TRAITS(Color, ColorTraits,
    // Initialize ColorTraits for each enum value starting at UNKNOWN, must match enum values
    { 0, "?" },
    { 1, "B" },
    { 2, "G" },
    { 3, "R" }
);

int main() {
    const ColorTraits& unknown_traits = ColorEnumTraits::get(Color::UNKNOWN);   // index:0, code:?
    const ColorTraits& green_traits   = ColorEnumTraits::get(Color::GREEN);     // index:2, code:G
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

_Note: Streams use synchronous I/O_ -- see also: \ref Async "Asynchronous I/O"

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

/** \page Async Asynchronous I/O

Evo supports asynchronous I/O for high performance clients and servers:
 - AsyncClient
 - AsyncServer

See also: \ref Streams "I/O Streams & Sockets"

Evo async client and server classes are under namespace \ref async

_Alpha: Evo Async I/O classes should be considerd a Work In Progress_

\par Dependencies

Evo async I/O requires libevent 2.0 or newer:
 - Linux/Unix/Cygwin: Link with `-levent_core` or `-levent`
   - To use libevent in multiple threads `#``define EVO_ASYNC_MULTI_THREAD 1` (before evo includes) and also link with `-levent_pthreads` -- _otherwise libevent is not thread safe, even with separate instances_
 - Windows/MSVC: Link with `libevent_core.lib` or `event_core.lib` in project configuration:
   - `Configuration -> VC++ Directories -> Include Directories`
   - `Configuration -> VC++ Directories -> Library Directories`
   - `Configuration -> Linker -> Input -> Additional Dependencies`
   - To use libevent in multiple threads `#``define EVO_ASYNC_MULTI_THREAD 1` (before evo includes) -- _otherwise libevent is not thread safe, even with separate instances_
 - See also: \ref BuildDependencies
 .

Near the beginning of a program using sockets, call Socket::sysinit() for best portability (required in Windows).

\par Supported Protocols

Evo async I/O is designed to support clients and servers using different protocol implementations.

 - %Memcached: \link async::MemcachedClient MemcachedClient\endlink, \link async::MemcachedServer MemcachedServer\endlink
 - Comming soon: %Redis, %HTTP
 .

\par Client

The async client classes are named `ProtocolClient`, where `Protocol` is the protocol used.
 - Create a `ProtocolClient` instance, and setup methods as needed:
   - Call \link AsyncClient::set_timeout() client.set_timeout()\endlink to set socket timeouts (optional but recommended)
   - Call \link AsyncClient::set_logger() client.set_logger()\endlink to enable logging
 - Call a connect method like \link AsyncClient::connect_ip() connect_ip()\endlink -- this starts a new connection and returns immediately (non-blocking)
 - Call client methods to make requests (methods are protocol specific) -- the requests are queued while the client is connecting (non-blocking)
 - An event-loop is required to handle async events with one of the following approaches (examples are below):
   - \b Blocking: Call \link AsyncClient::runlocal() client.runlocal()\endlink to run a local event-loop in the current thread. This blocks until all queued requests are processed and receive responses.
     - Multiple clients can share the same event-loop, call \link AsyncClient::attach_to() client.attach_to()\endlink to attach client to another client-event loop
       - When attached, only the root client (the one all others attached to) can run an event-loop
     - This is useful for pipelining multiple requests then waiting for them to finish, and doing this with multiple clients allows some concurrency (in a single thread)
     - Not thread safe -- Only one thread may call this at a time
   - \b Non-blocking \b Background \b %Thread: _Not yet implemented_
   - \b Non-blocking \b Server \b Back-end: Call \link AsyncClient::attach_to() client.attach_to()\endlink to attach client to a server event-loop
     - This is done when a server is using the client to call another server -- one or more clients can share the server event-loop
     - Not to share between threads -- Only safe to use from server callbacks or other clients using the same event-loop, i.e. the same server thread
     - Works with multi-threaded server, as long as each server thread has it's own separate back-end client
 .

Client callback types:
 - \link AsyncClient::OnConnect OnConnect\endlink
 - \link AsyncClient::OnError OnError\endlink
 - Some clients may provide `OnEvent`, which combines multiple events
 - Other callbacks depend on the `Protocol`
 .

Here's an example using \link async::MemcachedClient MemcachedClient\endlink (Memcached protocol):

\code
#include <evo/async/memcached_client.h>
#include <evo/io.h>
using namespace evo;

struct OnEvent : async::MemcachedClient::OnEvent {
    void on_connect() {
        con().out << "on_connect()" << NL;
    }

    void on_store(const SubString& key, Memcached::StoreResult result) {
        con().out << "on_store() " << key << ' ' << Memcached::StoreResultEnum::get_string(result) << NL;
    }

    void on_get(const SubString& key, const SubString& value, uint32 flags) {
        con().out << "on_get() " << key << " '" << value << "' " << flags << NL;
    }
};

int main() {
    Socket::sysinit();

    const ushort MEMC_PORT = 11211;
    OnEvent on_event;

    async::MemcachedClient memc;
    memc.set_on_connect(&on_event);
    memc.connect_ip("127.0.0.1", MEMC_PORT);

    memc.set("key1", "value1", on_event);
    memc.set("key2", "value2", on_event);
    memc.runlocal();

    memc.get("key1", on_event);
    memc.get("key2", on_event);
    memc.runlocal();

    return 0;
}
\endcode

\par Server

An async server class is created using a template class implementing a `PROTOCOL` and passing it a user-defined `HANDLER` class that implements the protocol event callbacks.
 - You implement a `HANDLER` class with the event (callback) methods expected by the `PROTOCOL`
   - `HANDLER` is usually a `struct` (public members) and must:
     - inherit `ProtocolServerHandler` (where `Protocol` is the `PROTOCOL` name), which itself inherits AsyncServerHandler
     - optionally define nested struct types: `Global` and/or `Shared` -- if not defined then defaults from AsyncServerHandler are used:
       - `Global` normally stores configuration info that is read-only from requests, and is shared by all requests and all server threads
         - This must be thread safe if server is multi-threaded (optional if single-threaded) -- read-only objects usually are thread-safe
         - Any writable objects here should be lock-free -- any blocking (mutex locks, synchronous I/O, sleep, etc) should be avoided since the server is asynchronous
         - Note that if you define `Global` then you must also define `Shared` since it references `Global` (see below)
         - See AsyncServer::get_global() to access this from outside the handler code
       - `Shared` stores state and back-end client connections, and is shared by all requests in the same thread
         - This does not need to be thread safe since it's not shared between threads
         - This is the place to put back-end clients used by request handlers (which must also be asynchronous)
         - Required methods:
           - `bool on_init(AsyncBase&, Global&)` -- called during startup before first connection is accepted
             - Initialize back-end clients here -- they can be attached to the server event-loop here -- see examples below
           - `void on_uninit()` -- called on shutdown after last connection is handled
         - For simple cases this can inherit \link AsyncServerHandler::SimpleSharedBase SimpleSharedBase\endlink for a default implementation -- must pass `Global` type via template argument
         - Note that this is not accessible from outside the handler code
         - See also methods in AsyncServerHandler::Shared
     - define a constructor that accepts `Global` and `Shared` references and stores them for request handler access, like this:
       \code
        Global& global;     // holds global data -- must be thread safe if server is multi-threaded
        Shared& shared;     // holds shared data

        HANDLER(Global& global, Shared& shared) : global(global), shared(shared) {
        }
       \endcode
       - Note that you only need to store references you will actually use, so you can ignore any of these arguments if desired (but still must accept them)
   - The server creates a `HANDLER` instance for each connection, and destroys it when the connection closes
 - Next, define a server type to use (via typedef) by combining `Protocol` and `HANDLER` using: `ProtocolServer<HANDLER>::Server` (where `Protocol` is the protocol name)
   - The above is a shortcut, the full definition looks like this: `AsyncServer< ProtocolServer<HANDLER> >` (where `Protocol` is the protocol name)
 - Create a listener Socket to use with the server
 - Instantiate the `server` and then:
   - Call \link AsyncServer::get_global() server.get_global()\endlink and populate configuration info and state, as required
   - Call \link AsyncServer::set_timeout() server.set_timeout()\endlink to set connection timeouts (optional but recommended)
   - Call \link AsyncServer::set_logger() server.set_logger()\endlink to enable logging
   - Call \link AsyncServer::run() server.run()\endlink to run the server event-loop and handle connections -- this won't return until the server is shut down
   - A handler or another thread may call \link AsyncServer::shutdown() server.shutdown()\endlink to stop the server, causing the `server.run*()` event-loop method used to return
 - Note that `Shared::on_init()` is the place to start back-end connections
 .

\dot
digraph "AsyncServerType" {
  edge [fontname="Helvetica",fontsize="10",labelfontname="Helvetica",labelfontsize="10"];
  node [fontname="Helvetica",fontsize="10",shape=record];

  AsyncServer [label="AsyncServer\<T\>" URL="\ref AsyncServer"];
  ProtocolServer [label="ProtocolServer\<T\>"];
  HANDLER [style=filled fillcolor=lightgrey];
  ProtocolServerHandler;
  AsyncServerHandler [URL="\ref AsyncServerHandler"];

  AsyncServer -> ProtocolServer [style="dashed" label=" T"];
  ProtocolServer -> HANDLER [style="dashed" label=" T"];
  HANDLER -> ProtocolServerHandler;
  ProtocolServerHandler -> AsyncServerHandler;

  label="Async Server Type";
  labelloc=top;
}
\enddot

<center><i>Note: Dashed line shows template parameter type used for member variable, solid line shows inheritance</i></center>

Here's an example using Memcached `PROTOCOL` to create a simple async single-threaded memcached server:
\code
#include <evo/async/memcached_server.h>
#include <evo/maphash.h>
using namespace evo;

// Define a Handler type to handle memcached server request events
struct Handler : async::MemcachedServerHandlerBase {
    struct Shared : SimpleSharedBase<> {
        StrHash map;
    };

    Shared& shared;

    Handler(Global& global, Shared& shared) : shared(shared) {
    }

    StoreResult on_store(StoreParams& params, SubString& value, Command command, uint64 cas_id) {
        switch(command) {
            case cSET:
                shared.map[params.key] = value;
                break;
            default:
                send_error("Not supported");
                return rtHANDLED;
        }
        return Memcached::srSTORED;
    }

    ResponseType on_get(const SubString& key, GetAdvParams* adv_params) {
        const String* val = shared.map.find(key);
        if (val != NULL)
            send_value(key, *val);
        return rtHANDLED;
    }
};

// Create Memcached Server class using Handler
typedef async::MemcachedServer<Handler>::Server Server;

int main() {
    Socket::sysinit();

    const ushort PORT = 11211;
    const ulong RD_TIMEOUT_MS = 5000;
    const ulong WR_TIMEOUT_MS = 1000;

    Socket listener;
    try {
        listener.listen_ip(PORT);
    } EVO_CATCH(return 1)

    Server server;
    server.set_timeout(RD_TIMEOUT_MS, WR_TIMEOUT_MS);
    server.run(listener);

    return 0;
}
\endcode

\par Server - Deferred Response

If a server handler has to wait on something to get a response, this is called a _Deferred Response_.

 - The server handler must not block so it creates a \link evo::AsyncServerHandler::DeferredContextT::ReplyBase ProtocolServer::DeferredReply\endlink and returns `rtDEFERRED` indicating the response will be sent later
   - Only one `DeferredReply` is allowed per handler event -- for complex event chains you'll need to keep state in the event object and send the response at the end
 - An instance of \link evo::AsyncServerHandler::DeferredContextT::ReplyBase ProtocolServer::DeferredReply\endlink is created to send this deferred reply, and is inherited by or associated with an external event object
 - When the wait is over, one of the `DeferredReply::deferred_reply_*()` methods must be called to send a response, then the `DeferredReply` should be destroyed
 - Example use case: A request requires a back-end server call to send a response (like a proxy server):
   - Example chain of events:
     - Server: Receive request 1
       - Client: Send back-end request for request 1
     - Client: Back-end response for request 1 (or client error occurred)
       - Server: Send response for request 1
 - Deferred responses on the same connection may be "out of order", which the framework handles with a queue so responses are always sent in the right order (matching request order)
   - Simple example:
     - Response 1: Deferred
     - Response 2: Not deferred, sent but queued until response 1 is sent first
     - Response 1: Sent, and response 2 from queue is then sent too
 .

Examples when deferred response is required:
 - Proxy or mid-tier server calling another back-end server (as mentioned above)
 - Processing in another thread, which may be expensive or use blocking I/O
 .

Here's a more complex example using Memcached `PROTOCOL` to create a simple async single-threaded memcached proxy server that uses a client to call a back-end memcached server:
\code
#include <evo/async/memcached.h>
using namespace evo;

// Define a Handler type to handle memcached server request events
struct ServerHandler : public async::MemcachedServerHandlerBase {
    // Global configuration for all requests
    struct Global {
        String proxy_address;
        ushort proxy_port;

        Global() : proxy_port(0) {
        }
    };

    // Shared state per thread
    struct Shared {
        async::MemcachedClient client;  // client for calling back-end server

        bool on_init(AsyncBase& server, Global& global) {
            String tmp;
            client.attach_to(server); // attach client to server event-loop
            if (!client.connect_ip(global.proxy_address.cstr(tmp), global.proxy_port))
                return false;
            return true;
        }
    };

    // Used to make a back-end request via client -- this either gets a response or an error occurs
    // - After the response (or error) this deletes itself to cleanup
    struct OnClientEvent : DeferredReply, async::MemcachedClient::OnEvent, async::MemcachedClient::OnError {
        OnClientEvent(ServerHandler& parent, ulong id) : DeferredReply(parent, id) {
        }

        void on_store(const SubString& key, Memcached::StoreResult result) {
            deferred_reply_store(result);
            delete this; // deferred response complete, delete callback
        }

        void on_get(const SubString& key, const SubString& value, uint32 flags) {
            deferred_reply_get(key, value, flags);
        }

        void on_get_end(const SubString&) {
            deferred_reply_get_end();
            delete this; // deferred response complete, delete callback
        }

        void on_error(AsyncError error) {
            deferred_reply_error("Backend client error");
            delete this; // deferred response aborted on error, delete callback
        }
    };

    Global& global;
    Shared& shared;

    ServerHandler(Global& global, Shared& shared) : global(global), shared(shared) {
    }

    StoreResult on_store(StoreParams& params, SubString& value, Command command, uint64 cas_id) {
        if (noreply) {
            // Call back-end via client, no reply expected
            shared.client.set(params.key, value, params.flags, params.expire);
            return Memcached::srSTORED;
        } else {
            // Call back-end via client, response is deferred until client gets response (or an error occurs)
            OnClientEvent* on_event = new OnClientEvent(*this, id);
            if (!shared.client.set(params.key, value, params.flags, params.expire, NULL, on_event, on_event)) {
                delete on_event;
                send_error("Error calling back-end");
                return rtHANDLED; // error sent, not deferred
            }
            return rtDEFERRED;
        }
    }

    ResponseType on_get(const SubString& key, GetAdvParams* adv_params) {
        // Call back-end via client, response is deferred until client gets response (or an error occurs)
        OnClientEvent* on_event = new OnClientEvent(*this, id);
        if (!shared.client.get(key, *on_event, on_event)) {
            delete on_event;
            send_error("Error calling back-end");
            return rtHANDLED;
        }
        return rtDEFERRED;
    }
};

// Create Memcached Server class using ServerHandler
typedef async::MemcachedServer<ServerHandler>::Server Server;

int main() {
    Socket::sysinit();

    const ushort PORT = 11210;
    const String BACKEND_ADDRESS = "127.0.0.1";
    const ushort BACKEND_PORT = 11211;
    const ulong RD_TIMEOUT_MS = 5000;
    const ulong WR_TIMEOUT_MS = 1000;

    Socket listener;
    try {
        listener.listen_ip(PORT);
    } EVO_CATCH(return 1)

    Server server;
    {
        Server::Global& global = server.get_global();
        global.proxy_address = BACKEND_ADDRESS;
        global.proxy_port = BACKEND_PORT;
    }
    server.set_timeout(RD_TIMEOUT_MS, WR_TIMEOUT_MS);
    server.run(listener);
}
\endcode

\par Implementation Detail

AsyncClient and AsyncServer both use an AsyncEventLoop to wait for I/O and call the appropriate callbacks.

 - A number of AsyncClient objects can "attach" to an AsyncServer to share the same event-loop
 - The event-loop itself uses a low-level API (libevent) to wait for I/O and trigger low-level callbacks, which call the appropriate higher-level callbacks
 .

\dot
digraph "AsyncClientServer" {
  edge [fontname="Helvetica",fontsize="10",labelfontname="Helvetica",labelfontsize="10"];
  node [fontname="Helvetica",fontsize="10",shape=record];

  AsyncClient [URL="\ref AsyncClient"];
  AsyncServer [URL="\ref AsyncServer"];
  AsyncBase   [URL="\ref AsyncBase"];
  AsyncEventLoop [URL="\ref AsyncEventLoop"];
  AsyncClient -> AsyncBase;
  AsyncServer -> AsyncBase;
  AsyncBase -> AsyncEventLoop [style="dashed" label=" evloop_"];

  label="Async Client/Server";
  labelloc=top;
}
\enddot

\par Implementation Detail - Client

Client request internal details:
 - Sending a request:
   - Write request to socket buffer
   - Add request response info to queue (if reply expected)
 - On socket read-ready (data received):
   - Pop request response info from queue
     - If queue is empty this is usually a protocol error and if so close socket (details depend on protocol)
   - Async wait for more data if needed (continue on next socket read ready)
   - Read request response from socket buffer
     - On error, invoke response info `on_error` callback (if not null) then close socket
   - Invoke client callback for response

\par Implementation Detail - Server

When a server accepts an incoming connection:
 - An internal `Connection` object is created to represent the client connection, which has:
   - a `ProtocolServer` instance that receives socket events for this connection and implements the protocol layer:
     - this has an instance of the server-defined `HANDLER` which implements the protocol events and sends responses back to the connected client
     - the `HANDLER` has an instance of \link evo::AsyncServerReplyT AsyncServerReply\endlink (via inheritance)
   - a \link evo::AsyncServerHandler::DeferredContextT HANDLER::DeferredContext\endlink instance for sending deferred responses
     - this has a pending deferred response reference count and a pointer to the server-defined `HANDLER` for sending deferred responses, which is set to `NULL` when the connection is closed
     - this is destroyed when the connection is closed and when no more deferred responses are in progress
     - more on deferred responses below
   - a `HANDLER` instance with event implementations, which has an \link AsyncServerReply\endlink instance (via inheritance) for sending responses
     - a `HANDLER` event creates a \link evo::AsyncServerHandler::DeferredContextT::ReplyBase HANDLER::DeferredReply\endlink to make a deferred response (more on this below)
 - The `Connection` object is destroyed when the connection closes

\dot
digraph "AsyncServer" {
  edge [fontname="Helvetica",fontsize="10",labelfontname="Helvetica",labelfontsize="10"];
  node [fontname="Helvetica",fontsize="10",shape=record];

  AsyncServer [URL="\ref AsyncServer"];
  Connection;
  DeferredContext [URL="\ref evo::AsyncServerHandler::DeferredContextT"];
  ProtocolServer;
  HANDLER;
  DeferredReply [URL="\ref evo::AsyncServerReplyT"];
  AsyncServerReply [URL="\ref evo::AsyncServerReplyT"];

  AsyncServer -> Connection [label=" creates per connection"];
  Connection -> DeferredContext [label=" creates ptr"];
  Connection -> ProtocolServer [label=" has"];
  ProtocolServer -> HANDLER [label=" has"];
  DeferredContext -> HANDLER [label=" has ptr"];
  HANDLER -> AsyncServerReply [label=" has"];
  HANDLER -> DeferredReply [label=" creates"];
  DeferredReply -> DeferredContext [label=" has ref"];

  label="Async Server";
  labelloc=top;
}
\enddot

Response types:
 - Normal responses are sent immediately by the `HANDLER` (details depend on the protocol)
 - Deferred responses are more complex (see diagram below) and are required when the server must wait for a response (without blocking):
   - The `HANDLER` creates a \link evo::AsyncServerHandler::DeferredContextT::ReplyBase HANDLER::DeferredReply\endlink event/object, starts an async (non-blocking) operation, then returns `rtDEFERRED` result
   - The server handles other requests and events while the deferred response is in progress
   - When a response is ready, an async callback uses the \link evo::AsyncServerHandler::DeferredContextT::ReplyBase HANDLER::DeferredReply\endlink object to send the response

<center><b>Deferred Response</b></center>
\msc
  hscale="1.5",
  wordwraparcs="true";

  sock [label="Socket"],
  proto [label="ProtocolServer"],
  handler [label="HANDLER"],
  external [label="ExternalAsync"];

  sock note sock [label="request"];
  sock  => proto   [label="socket event"];
  proto => handler [label="request event"];
  handler => external [label="create DeferredReply, async call"];
  handler => proto [label="handler returns deferred result"];
  sock note sock [label="other requests/events are handled"];
  external => proto [label="async result via DeferredReply"];
  proto => sock [label="response via AsyncServerReply"];
\endmsc

The framework handles these scenarios with deferred responses:
 - Out of order response: Async events don't have a predictable order so responses may happen out of order
   - The framework accounts for this by sending responses through \link AsyncServerReply\endlink, which queues responses when they're out of order
   - Ordering is corrected for any mixture of deferred and non-deferred responses, so the response order always matches the request order
 - Connection closed unexpectedly while deferred responses are pending:
   - The framework handles this using \link evo::AsyncServerHandler::DeferredContextT DeferredContext\endlink, which is detached from the `Connection` when closed and a deferred response is pending
   - When the connection is closed the deferred responses for it become no-ops, but still must be cleaned up
   - After all pending deferred responses are complete the `DeferredContext` is destroyed
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

Linking:
 - Linux/Unix: `-pthread`
 - Cygwin: `-lpthread`
 - Windows: Usually multithreaded by default -- MSVC project settings: `C/C++ -> Code Generation -> Runtime Library`

Note: Before C++ 11, thread safety depends on the compiler. Be especially careful with static and global variables on pre-C++11 compilers. See notes in: \ref CppCompilers "C++ Compilers".
*/

///////////////////////////////////////////////////////////////////////////////

/** \page Metaprogramming Metaprogramming & Macros

Evo uses some metaprogramming internally -- these features are available for advanced use.

\#include <evo/type.h> -- included by most evo headers already

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
 - IsNullable
 - IsEvoContainer
 .

\par Type Conversion

 - RemoveConst
 - RemoveConstVol
 - RemoveExtents
 - RemoveExtentsConstVol
 - RemoveVolatile
 - Convert
 .

\par Static Expressions

 - STATIC_ASSERT()
 - \link STATIC_ASSERT_FUNC_UNUSED\endlink
 - STATIC_ASSERT_FUNC_UNUSED_RET()
 - EVO_STATIC_JOIN()
 - StaticIf
 .

\par Macros

Evo also implements these helper macros

\#include <evo/macro.h>

 - EVO_STRINGIFY()
 - EVO_CONCAT()
 - EVO_EXPAND()
 - EVO_COUNT_ARGS()
 - EVO_MAP_FIELDS(), EVO_MAP_FIELDS_KEY()
 .

\par Compiler Helpers

 - EVO_PARAM_UNUSED()
 - \link EVO_ATTRIB_UNUSED\endlink
 - EVO_MSVC_NOWARN_START(), \link EVO_MSVC_NOWARN_END\endlink
 .

 - \link EVO_CPP11\endlink, EVO_ONCPP11()
 - \link EVO_CPP14\endlink
 - \link EVO_CPP17\endlink
 .

 - \link EVO_64\endlink, \link EVO_ARCH_BITS\endlink
 - \link EVO_COMPILER\endlink
 - \link EVO_COMPILER_VER\endlink
 .

*/

///////////////////////////////////////////////////////////////////////////////

/** \page SmartQuoting Smart Quoting

When handling delimited text, Evo prefers smart quoting over escape characters:
 - Escape characters require the parser to mutate the input (remove escape prefixes), which prevents string \ref Sharing and affects performance
 - Escape characters also require the writer to mutate the output (insert escape prefixes), which increases the data size as well

Smart quoting avoids having to escape characters by using a quoting type adapted to the text being quoted.

_Note: Manually applying smart quoting to text can be tricky in certain cases._

\par Quoting Types

A "field" is the original text, which may contain whitespace, delimiters, and/or quote characters:
 - Binary data is not supported here as it isn't text and may be impossible to correctly quote
 - However, a quoted field may contain unprintable characters as well as UTF-8 multibyte characters

A "token" is a field as quoted or unquoted text, which is often followed by a delimiter acting as a separator for the next token:
 - This text may contain delimiters and quote characters as well as any valid plain text
 - If the field text _contains the delimiter_ then it must be quoted, which effectively "escapes" the delimiter character
 - If the field text _begins or ends with literal quotes_ then it must be quoted, which effectively "escapes" those quotes
 - If the field text _begins or ends with either whitespace or unprintable characters_ then it must be quoted to correctly preserve the beginning and end
 - If any combination of the above cases apply, then the field must be quoted with a quoting type that doesn't confuse the parser
 - If none of the above cases apply, then quoting isn't required to parse the field

Single-char quoting:
 - Single-quotes: `'foo bar'`
 - Double-quotes: `"foo bar"`
 - Backtick-quotes: <code>\`foo bar\`</code>

Triple quoting (inspired by the Python language):
 - Triple single-quotes: <code> '''foo bar''' </code>
 - Triple double-quotes: <code> """foo bar""" </code>
 - Triple backtick-quotes: <code> \`\`\`foo bar\`\`\` </code>

_Following the above rules will correctly quote and escape text fields_ -- see below for edge cases.

The non-single-quoting types (above) and edge cases (mentioned below) are rare with normal text, but must be handled correctly when using smart quoting.

No quoting:
 - If a token doesn't begin _and_ end with quotes then it's _treated as unquoted_
   - Quoting at _end_ is determined by context: a quote followed by a delimiter, or followed by end of input
   - There may be whitespace (spaces, tabs, newlines) between the quote and the delimiter (or end of input), in which case the field is still _end-quoted_ -- see _Formatting_ section below for example
 - The parser doesn't get confused when quote characters (or apostrophes) appear inside _unquoted_ tokens -- in this case with unquoted text, the parser just splits by delimiter
 - Example comma-delimited fields that aren't quoted at all -- these particular cases don't confuse the parser:
   - <code> can't,won't,'bout </code>
     - <code> can't </code>
     - <code> won't </code>
     - <code> 'bout </code> -- _dangerous if unquoted due to beginning apostrophe (single quote)_
   - <code> 'not' quoted,also not 'quoted' </code>
     - <code> 'not' quoted </code> -- _dangerous if unquoted due to beginning single quote_
     - <code> also not 'quoted' </code> -- _dangerous if unquoted due to ending single quote_
 - Example that _will_ confuse the parser with words that begin or end with an apostrophe (single quote):
   - <code> can't,'bout,runnin',jumpin' </code>
     - <code> can't </code>
     - <code> bout,runnin </code> -- _considered quoted, fix with actual quoting_
     - <code> jumpin' </code> -- _dangerous if unquoted, the ending apostrophe will confuse a reverse-parser_
   - _Reverse-Parser:_
     - <code> bout,runnin',jumpin </code> -- _gets different tokens due to the ending apostrophe_
     - <code> can't </code>
   - _Fixed:_ <code> can't,"'bout","runnin'","jumpin'" </code>
     - <code> can't </code>
     - <code> 'bout </code>
     - <code> runnin' </code>
     - <code> jumpin' </code>
 - Tricky examples -- quoting like this should be avoided:
   - <code> 'one'two','three' </code> -- _the second quote isn't an end-quote because it isn't followed by a delimiter (or end of input)_
     - <code> one'two </code>
     - <code> three </code>
   - <code> ''' </code> -- _1 quote char, not triple-quoted since there's no end quote_
     - <code> ' </code>
   - <code> '''' </code> -- _2 quote chars, not triple-quoted since there's no end quote_
     - <code> '' </code>

Backtick-DEL quoting:
 - This is a fallback when no other quoting will work (which is very rare) and uses the `DEL` char (ASCII code `7F`, normally not printable but shown here as `DEL` or `␡`)
 - Here a backtick followed by a `DEL` char is used as a quote, and this pair is used at the beginning and the end, like with other quote types
   - Example: <code> \`␡foo bar\`␡ </code>
 - This assumes the quoted text is valid plain text (not binary data) -- normal plain text doesn't use the `DEL` char at all, and is very unlikely to include a backtick-DEL pair followed by a delimiter (or end of input)
 - This (and other quoting types) can still be used in combination with the `DEL` char as a delimiter
   - Example with `DEL` delimiter and `backtick-DEL` quoting:
     - <code> \`␡foo bar\`␡␡\`␡stuff things\`␡ </code>
       - <code> foo bar </code>
       - <code> stuff things </code>

\par Parsing

 - \link StrTokQ\endlink, \link StrTokQR\endlink
 - StrTok::nextq(), StrTokR::nextq()
 - str_scan_endq(), str_scan_endq_r()

Parsers will look for a beginning quote, and if found try to find a matching end-quote followed by a delimiter or end of input.
 - If no beginning quote, or if the end-quote isn't found, then the input is treated as unquoted (see above examples under "No quoting")
   - Input that begins with a triple quote and ends with a single quote is treated as unquoted
 - With triple end-quotes, any extra ending quote characters are kept as-is, i.e. included in field text
   - Example with comma delim: <code> '''one'''',two </code>
     - <code> one' </code>
     - <code> two </code>
   - The same applies to beginning quotes: <code> ''''one''',two </code>
     - <code> 'one </code>
     - <code> two </code>
 - The parser must be aware of the delimiter that may follow the input
 - A reverse-parser does the same thing in reverse, and either way will result in the same tokens as long fields are quoted correctly when required

\par Formatting

 - String::writequoted()
 - Stream::writequoted()
 - StreamOut::writequoted()
 - StrQuoting::get()

Formatters will check the field text and write it with the appropriate quoting type.
 - Quoting type preference order: `single, double, backtick, triple-single, triple-double, triple-backtick, backtick-DEL`
 - Beginning and/or ending quote characters should not be used as the quoting type if possible, even though it would still parse correctly
   - Example input: <code> 'foo,bar\" </code>
     - Quoted (preferred): <code> `'foo,bar\"` </code>
     - Quoted (avoid): <code> ''foo,bar\"' </code> -- _parsers should still handle this correctly_
 - Field text that contains a quoting type followed by the delimiter (or end of input) cannot be quoted with that quoting type
   - Example with comma delim: <code> foo''',bar </code> -- _cannot be quoted with_ <code> ' </code> _or_ <code> ''' </code>
 - Any whitespace (spaces, tabs, newlines) between a quote character and a delimiter is ignored when determining the quoting type to use
   - Examples with comma delim:
     - <code> foo', bar </code> -- _this cannot be quoted with a single-quote char since it contains this quote char followed by a delim_
       - Quoted: <code> "foo', bar" </code>
     - <code> foo' , bar </code> -- _this cannot be quoted with a single-quote char since it contains this quote char followed by some whitespace then a delim_
       - Quoted: <code> "foo' , bar" </code>
 - Example using a comma delimiter that requires `backtick-DEL` quoting: <code> foo''',\"\"\",```,bar </code>
   - Quoted: <code> `␡foo''',\"\"\",\`\`\`,bar\`␡ </code>
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
 - C++11: Evo has support for UTF-16 string literals (`u` prefix, via `char16_t`), see UnicodeString and String

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

/** \page ErrorHandling Error Handling

_Work in progress_

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
 - Evo Nullable is similar to C++17 `std::optional` (but is only for primitive types)
 - Most Evo containers implement iterator methods for STL compatibility: `begin(), end(), cbegin(), cend()`

\par Initializer Lists (C++11)

Evo containers support C++11 initializer lists.

Example:
\code
#include <evo/list.h>
#include <evo/maplist.h>
using namespace evo;

int main() {
    List<int> list = {1, 2, 3};

    MapList<int,int> map = {{1, 100}, {2, 200}};

    return 0;
}
\endcode

\par Range-Based For Loops (C++11)

Evo containers support C++11 range-based for loops.

 - For best performance use `const` containers if possible -- Evo containers have an `asconst()` method to make this easy
 - See \ref PrimitivesContainers and \ref Slicing

Example:
\code
#include <evo/list.h>
#include <evo/io.h>
using namespace evo;

int main() {
    Console& c = con();

    // Make a list and a copy (uses Copy-On-Write)
    List<int> list = {1, 2, 3};
    List<int> list2(list);

    // Const iteration allows list2 to use Copy-On-Write for best performance
    for (auto& item : list.asconst())
        c.out << item << NL;

    // Mutable iteration forces list2 to make a copy so items can be modified
    for (auto& item : list) {
        item *= 10;
        c.out << item << NL;
    }

    return 0;
}
\endcode

\par STL String

Evo supports implicitly converting STL strings (`const std::string&` and `const std::string*`) to \link evo::StringBase StringBase\endlink so STL strings
can be passed to functions that accept Evo string types. Simply include STL string header first and Evo will try to auto-detect and enable `std::string` support.
C++17 `std::string_view` is also supported as well, when supported by the compiler.

If you want to be explicit about passing an STL string to a function using Evo strings, wrap it like this: `evo::StringBase(stdstring)` -- see example below.
 * Note that \ref StringBase is a common base class for all Evo strings and works as a primitive form of SubString

See \ref StringPassing

Note that in some cases you may run into problems with "too many implicit conversions." Here's an example:
\code
#include <string>
#include <evo/string.h>

int main() {
    std::string stdstring = "test";                     // constructs via temporary, ok here
    evo::String string    = stdstring;                  // constructs via temporary, possible compiler error: too many implicit conversions

    evo::String string(stdstring);                      // ok, explicit constructor (no temporary) -- preferred
    evo::String string = evo::StringBase(stdstring);    // ok, explicit temporary
    evo::String string(( evo::StringBase(stdstring) )); // ok, explicit temporary -- extra parenthesis prevent the C++ "Most Vexing Parse" issue

    return 0;
}
\endcode

You may also enable STL string compatibility directly with a preprocessor define, before including any Evo headers:
\code
#define EVO_STD_STRING 1
#define EVO_STD_STRING_VIEW 1
#include <evo/string.h>
\endcode

For more info on enabling STL string compatibility see: \ref EVO_STD_STRING and \ref EVO_STD_STRING_VIEW

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
 - Passing a string by value works but is not ideal (less efficient):
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

These suggestions also apply in general to Evo containers like List, and other complex objects.

For passing read-only strings it's often useful to use SubString to show how the string is used. A function taking a SubString argument
explicitly shows the string is read-only and prevents further \ref Sharing when copied to another String (i.e. sharing stops here),
and is often useful in multithreaded situations (with sychronization as needed).

 - Pass substring as an argument (by const reference):
   \code
    #include <evo/substring.h>
    using namespace evo;

    struct MyData {
        String data;

        // Takes a string reference, "disables" sharing
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
 - As with String, passing SubString by value works but is not ideal (less efficient)
 - Returning a SubString in any way is often dangerous, unless returning one of the arguments
   \code
    const SubString& foo(const SubString& str) {
        ...
        return str;
    }
   \endcode
 .

Alternatively, for passing read-only strings with \ref StlCompatibility use \ref StringBase. This works the same way as SubString.

 - Pass substring as an argument (by const reference) using \ref StringBase :
   \code
    #include <string> // std::string
    #include <evo/substring.h>
    using namespace evo;

    struct MyData {
        String data;
        Int num;

        // Takes a string reference, "disables" sharing
        void store(const StringBase& str) {
            data = str;                     // Store a unique copy
            num  = SubString(str).num();    // Use SubString methods, like num()
        }
    };

    int main() {
        String str("test");
        MyData data;

        // Pass str
        data.store(str);

        // Works with std::string too
        std::string stdstr("testing");
        data.store(stdstr);

        return 0;
    }
   \endcode
 - \ref StringBase is only used for passing strings to functions (and only by const-reference)
 - See \ref StlCompatibility

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

\par Whitespace

Many Evo methods skip or strip whitespace. This may or may not include newlines -- this will be noted in the method documentation.

 - Newlines are often used to define structure with plain text so it doesn't always make sense to skip them
 - When performance is critical, code that only skips spaces and tabs is generally faster than a version that also skips newlines (even with SSE optimization)
 - Tokenizers usually don't skip newlines
 - Methods like String::strip() have a variant like String::strip2() that also strips newlines

\par Check Null or Empty

\code
#include <evo/string.h>
using namespace evo;

int main() {
    String str;

    // Null string
    if (str.null())
        { }

    // Null string is also considered empty
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
#include <evo/strtok.h>
using namespace evo;

int main() {
    String str("1,2,3");

    // Split into list
    List<Int> nums;
    str.split<StrTok>(nums);

    // Join back into list
    nums.set();
    str.join(nums);  // set back to: 1,2,3

    return 0;
}
\endcode

Splitting to a list with generic conversion and alternate tokenizer:
\code
#include <evo/string.h>
#include <evo/strtok.h>
using namespace evo;

int main() {
    String str("1,'2,2',3");

    // Split into list, with tokenizer supporting quoting
    List<String> nums;
    str.split<StrTokQ>(nums);

    // Join back into list, with quoting
    nums.set();
    str.joinq(nums);  // set back to: 1,'2,2',3

    return 0;
}
\endcode

\par Tokenizing

String and SubString have methods for extracting tokens:
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
#include <evo/strtok.h>
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

\link StrTokQ\endlink and StrTok::nextq() can parse quoted tokens.

See \ref SmartQuoting

\par Low-Level Scanning

 - str_scan_nws(), str_scan_nws_r()
 - str_scan_ndelim(), str_scan_ndelim_r()
 - str_scan_delim(), str_scan_delim_r()
 - str_scan_endq(), str_scan_endq_r()
 - str_scan_to()
 - str_scan_decimal()
 - str_scan_hex()
 - StrQuoting

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
 - FmtChar, FmtString, FmtStringWrap
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

See also: \ref StringConversion and \ref SmartQuoting
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
 - FmtChar, FmtString, FmtStringWrap
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

\par Smart Quoting

See also: \ref SmartQuoting
*/

////

/** \page AdditionalFormatting Additional Formatting

Additional text formatting helpers, compatible with String, Stream, and StreamOut types.

\par Tables

FmtTable is used to setup table column information and FmtTableOut (or a helper that returns this) is used to format and write the table to a string or stream.

The easiest way to format a table is with fmt_table(), which caches the output internally in order to determine the best column alignment.

\code
#include <evo/fmt.h>
#include <evo/io.h>
using namespace evo;

int main() {
    Console& c = con();

    const SubString NAMES[] = {
        "Name",
        "ID",
        "Balance",
        "Location",
        ""
    };

    FmtTable table(NAMES, 0);
    fmt_table(c.out, table)
        << "John Smith" << 1001 << 9.50 << "Los Angeles" << NL
        << "Jane Doe" << 1002 << 19.75 << "New York" << NL
        << fFLUSH;

    return 0;
}
\endcode

Output:
\code{.unparsed}
Name        ID    Balance  Location
John Smith  1001  9.5      Los Angeles
Jane Doe    1002  19.75    New York
\endcode

To avoid caching overhead use fmt_table_nocache() to write table output directly. To keep columns aligned, FmtTableAttribs is useful for updating column widths first.

\code
#include <evo/fmt.h>
#include <evo/io.h>
using namespace evo;

int main() {
    Console& c = con();

    const SubString NAMES[] = {
        "Name",
        "ID",
        "Balance",
        "Location",
        ""
    };

    FmtTable table(NAMES, 0);
    FmtTableAttribs(table) << 10 << 5 << 11;

    fmt_table_nocache(c.out, table)
        << "John Smith" << 1001 << 9.50 << "Los Angeles" << NL
        << "Jane Doe" << 1002 << 19.75 << "New York" << NL;

    return 0;
}
\endcode

Output:
\code{.unparsed}
Name        ID    Balance  Location
John Smith  1001  9.5      Los Angeles
Jane Doe    1002  19.75    New York
\endcode
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

/** \page FullServer Full Server Example

This gives a full server example, including:
 - Logging with Logger
 - Signal handling for graceful shutdown
 - Daemonizing in Linux/Unix
 .

See: \ref Async

\par Example

\code
#include <evo/string.h>
#include <evo/maphash.h>
#include <evo/logger.h>
#include <evo/process.h>
#include <evo/commandline.h>
#include <evo/async/memcached_server.h>
using namespace evo;

// Basic memcache server
struct Handler : async::MemcachedServerHandlerBase {
    struct Shared : SimpleSharedBase<> {
        StrHash map;
    };

    Shared& shared;

    Handler(Global& global, Shared& shared) : shared(shared) {
    }

    StoreResult on_store(DeferredContext& context, StoreParams& params, SubString& value, Command command, uint64 cas_id) {
        switch(command) {
            case cSET:
                shared.map[params.key] = value;
                break;
            default:
                send_error("Not supported");
                return rtHANDLED;
        }
        return Memcached::srSTORED;
    }

    ResponseType on_get(DeferredContext& context, const SubString& key, GetAdvParams* adv_params) {
        const String* val = shared.map.find(key);
        if (val != NULL)
            send_value(key, *val);
        return rtHANDLED;
    }
};

typedef async::MemcachedServer<Handler>::Server Server;

int main(int argc, const char* argv[]) {
    Socket::sysinit();

    const ulong RD_TIMEOUT_MS = 5000;
    const ulong WR_TIMEOUT_MS = 1000;

    // Command-line
    StrHash args;
    String tmpstr;
    {
        List<SubString> log_level_choices;
        for (LogLevelEnum::Iter iter; iter; ++iter)
            log_level_choices.add(iter.value_str());

        CommandLine cmdline("Test memcached server");
        cmdline.addver("Test Memcached Server 1.0");
        cmdline.addopt("-p, --port", "Server port number to use")
            .default_value("11211")
            .numeric()
            .maxlen(UShort::MAXSTRLEN);
        cmdline.addopt("-l, --log", "Log file to use")
            .default_value("server.log");
        cmdline.addopt("-L, --loglevel", tmpstr.set("Log level to use ${default}, one of:\n ").join(log_level_choices, ','))
            .default_value("info")
            .addchoices(log_level_choices);

        if (!cmdline.parse(args, argc, argv))
            return 0;
    }

    // Init logger, use local time in log
    Logger<> logger;
    logger.set_local_time(true);
    logger.set_level(LogLevelEnum::get_enum(lookupsub(args, "loglevel")));

    // Listen on socket early and open log, stop on error such as "port in use" or permissions issue
    Socket listener;
    const ushort port = lookupsub(args, "port").getnum<ushort>();
    try {
        listener.listen_ip(port);
        logger.open(lookupsub(args, "log"));
    } EVO_CATCH(return 1);

    // Init server
    Server server;
    server.set_logger(&logger);
    server.set_timeout(RD_TIMEOUT_MS, WR_TIMEOUT_MS);

    // Install shutdown signal handler using helper for servers
    Signal::MainServer<Server> signal_main(server);

    // Uncomment to daemonize (run in background) -- Linux/Unix only
    //daemonize(DAEMONIZE_USE_STDERR);

    // Run server -- logger now used, error before this point goes to stderr
    logger.start_thread();
    EVO_LOG_INFO(logger, tmpstr.set() << "Startup: Listening on port " << port);
    server.run(listener);
    EVO_LOG_INFO(logger, "Shutdown");
    return 0;
}
\endcode
*/

///////////////////////////////////////////////////////////////////////////////

/** \page Configuration Configuration

Evo library can be customized using define directives before any Evo headers are included.

_Work in progress_
*/

///////////////////////////////////////////////////////////////////////////////
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
