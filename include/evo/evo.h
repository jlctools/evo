// Evo C++ Library
/* Copyright (c) 2016 Justin Crowell
 This Source Code Form is subject to the terms of the Mozilla Public
 License, v. 2.0. If a copy of the MPL was not distributed with this
 file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
///////////////////////////////////////////////////////////////////////////////
/** \file evo.h Evo Core Library Documentation -- Includes all the core Evo headers. */
#pragma once
#ifndef INCL_evo_h
#define INCL_evo_h

///////////////////////////////////////////////////////////////////////////////

/** \addtogroup EvoCore
Evo C++ library. Implemented under namespace: evo */
//@{

/** Evo C++ Library namespace. */
namespace evo {

// TODO:
// - \ref ErrorHandling "Error Handling"
// - \ref Configuration "Configuration"

/** \mainpage Evo C++ Library
\par Evo

Evo is a modern high-performance C++ library designed to make programming fundamentals easy, including data structures, strings, and I/O.

Inspired by modern languages like Python and D, Evo is the result of many years of lessons learned from writing (and rewriting) high-performance,
object-oriented C++ server code. Even when performance isn't critical, Evo can still make tedious string parsing and data structures easy to read and manage.
Evo also aims to make complex I/O situations easier. Evo is intended to provide a solid foundation for applications and other libraries.

 - \ref WhatsNew "What's New?"
 - \ref DesignGoals "Design Goals"
 - \ref LicenseCredits "License and Credits"
 - \ref CppCompilers "C++ Compilers"
 .

\par Features & Concepts:

 - \ref ManagedPtr "Managed Pointers"
 - \ref Sharing "Sharing"
 - \ref Slicing "Slicing"
 - \ref UnsafePtrRef "Unsafe Pointer Referencing"
 - \ref PlainOldData "Plain Old Data & Copying"
 - \ref PrimitivesContainers "Primitives & Containers"
 - \ref Streams "I/O Streams"
 - \ref Metaprogramming "Metaprogramming"
 - \ref StlCompatibility "STL Compatibility"
 .

\par Mini Tutorials

 - \ref StringPassing "String Passing"
 - \ref StringConversion "String Conversion"
 - \ref StringParsing "String Parsing"
 - \ref StringFormatting "String Formatting"
 - \ref StringCustomConversion "Advanced: Custom String Conversion/Formatting"
 .

\par Primitives

 - Bool
 - CharT
    - Char
 - IntegerT
    - \link evo::Short Short\endlink, \link evo::Int Int\endlink, \link evo::Long Long\endlink, \link evo::LongL LongL\endlink, \link evo::UShort UShort\endlink, \link evo::UInt UInt\endlink, \link evo::ULong ULong\endlink, \link evo::ULongL ULongL\endlink
    - \link evo::Int8 Int8\endlink, \link evo::Int16 Int16\endlink, \link evo::Int32 Int32\endlink, \link evo::Int64 Int64\endlink, \link evo::UInt8 UInt8\endlink, \link evo::UInt16 UInt16\endlink, \link evo::UInt32 UInt32\endlink, \link evo::UInt64 UInt64\endlink
 - FloatT
    - \link evo::Float Float\endlink, \link evo::FloatD FloatD\endlink, \link evo::FloatL FloatL\endlink
 - SmartPtr, SharedPtr, Ptr
 .

\par Containers

 - Array, List, PtrList, SubList
 - String, SubString, \link evo::StringBase StringBase\endlink
 - Map
    - MapList, \link evo::StrMapList StrMapList\endlink
    - MapHash, \link evo::StrHash StrHash\endlink
    - lookupsub()
    .
 - SafeBool
 .

\par String Tokenizers

 - StrTok, StrTokR
   - \link evo::StrTokQ StrTokQ\endlink, \link evo::StrTokQR StrTokQR\endlink
   - \link evo::StrTokWord StrTokWord\endlink, \link evo::StrTokWordR StrTokWordR\endlink
 - StrTokS, StrTokRS
   - \link evo::StrTokWordS StrTokWordS\endlink, \link evo::StrTokWordRS StrTokWordRS\endlink
 .

\par Stream I/O

 - File
 - Pipe, PipeIn, PipeOut
 - Console

\par Constants

 - \link EVO_CNULL\endlink, \link EVO_VNULL\endlink
 - \link vNull\endlink, \link vEmpty\endlink, \link vReverse\endlink

\par Error Handling

 - \link evo::Error Error\endlink
 - Exception
 - ExceptionInval
 - ExceptionStream
 - EVO_THROW()
 - EVO_THROW_ERR(), EVO_THROW_ERR_CHECK()
 .

----

Copyright (C)  2016  Justin Crowell.\n
Permission is granted to copy, distribute and/or modify this document
under the terms of the GNU Free Documentation License, Version 1.3
or any later version published by the Free Software Foundation;
with no Invariant Sections, no Front-Cover Texts, and no Back-Cover Texts.
A copy of the license is included in the section entitled
\ref GFDL "\"GNU Free Documentation License\"".

*/

///////////////////////////////////////////////////////////////////////////////

/** \page WhatsNew What's New?

Evo change history.

\par Version 0.3 - Dec 2016
 - Bug fix: SharedPtr clear() crash
 - I/O %Stream refinements
 - Add STL compatibility: std::string
 - Support C++11 and clang compilers
 - Improved quoting with StrTokQ and Convert
 - Added StrTok nextanyq()
 - Many documentation updates, added Mini Tutorials

\par Version 0.2 - Jan 2016
 - Add I/O Streams
 - Add map lookupsub()
 - Bug fixes
 - Many minor refinements and updates
 - Many documentation updates

\par Version 0.15 - Mar 2015
 - First public release
*/

///////////////////////////////////////////////////////////////////////////////

/** \page DesignGoals Design Goals

General design goals.

Design:
 - Trivial operations are short and simple, like parsing a string
 - Advanced operations are supported, like writing directly to a string buffer
 - High performance, especially for critical systems like servers and game engines
 - Simple interfaces that are easy to understand and use, differentiate basic and advanced features
 - Headers-only library that's easy to integrate with projects
 - Good documentation and extensive unit test coverage
 - Categorical naming to make related things naturally sort together -- ex: MapList, MapHash, \link evo::String::split split()\endlink, \link evo::String::splitr splitr()\endlink, \link evo::String::splitat splitat()\endlink
 - Support multiple platforms: Linux, Unix, Windows, Cygwin

Source:
 - Solid code with thorough unit testing and 100% code coverage (or as close as possible)
 - See: \ref LicenseCredits "License and Credits"
*/

// TODO:
// - Make common network/internet features easy (HTTP, URL parsing, etc)
// - Support common compression and encryption libraries: zlib, libsnappy, openssl

///////////////////////////////////////////////////////////////////////////////

/** \page CppCompilers C++ Compilers

Evo works with C++ 2003 and 2011 compliant compilers.

Evo has extensive unit test coverage and is tested on the following systems and compilers.

\par Ubuntu 64-bit
 - 16.04: gcc 5.4.0, clang 3.8, valgrind 3.11.0
 - 14.04: gcc 4.8.4, clang 3.4, valgrind 3.10.1
 - 12.04: gcc 4.6.3, valgrind 3.7.0

\par CentOS 64-bit
 - 7.1: gcc 4.8.5, valgrind 3.11.0
 - 5.7: gcc 4.1.2, valgrind 3.5.0

\par Mac OS X 64-bit
 - 10.10.5: Apple LLVM 7.0.2 (clang 700-1.81)

\par Windows 10 64-bit
 - MSVC: 2015 Community Update 3

\par Windows 7 32-bit
 - MSVC: 2010 Express SP1
 - cygwin: gcc 4.9.2
*/

///////////////////////////////////////////////////////////////////////////////

/** \page ManagedPtr Managed Pointers

A managed pointer is a class that emulates a raw (built-in) pointer type.
Using managed pointers makes the intent and ownership of the pointer clear and reduces risk of pointer misuse and memory bugs.

 - SmartPtr is a smart pointer to a single object, meaning it takes ownership of a pointer and is responsible for freeing it.
   - Copying a SmartPtr copies the pointed object too.
 - SharedPtr is similar to SmartPtr except it can be shared among multiple owners and uses Reference Counting.
   - Copying a SharedPtr makes a shared copy (increments reference count) -- all copies share the same instance.
 - Ptr is a dumb pointer to a single object or array, meaning it only references the pointer and does not take ownership. This is useful to indicate the pointer is owned elsewhere.
   - One could use raw pointers as dumb pointers, however keep in mind that Evo containers copy differently from a raw pointer vs a managed pointer -- raw pointers may be considered immutable and "referenced", see \ref Sharing "Sharing"
   - Raw pointers are best used only for memory that is both static and immutable, such as string literals

All managed pointers are specialized for array types (<tt>T[]</tt>) to use <tt>delete[]</tt> to free memory (when applicable). Managed array pointers can't make a copy of the pointed objects (array size not known).
 - SmartPtr<T[]> (array version) cannot be copied
 - \link evo::SharedPtr<T[],TSize> SharedPtr<T[]>\endlink (array version) doesn't support unshare()

Managed pointers should be used instead of raw pointers as much as possible.
Dereference operators (operator*, operator->, operator[]) are overloaded so they work like raw pointers.

\par Quick Example

\code
// Smart pointer to int
SmartPtr<int> num(new int);
(*num) = 1; // parenthesis not required, only used to work around doxygen formatting bug

// Smart pointer to char array
SmartPtr<char[]> chars(new char[5]);
strcpy(chars.ptr(), "test");
chars[0] = 'b';
\endcode

\par Detailed Example

\code
#include <evo/ptr.h>
#include <stdio.h>
using namespace evo;

// Example class/object
struct Data {
    int val;
};

// Uses dumb Ptr so it's clear this function does not take pointer ownership
void print(Ptr<Data> ptr) {
    if (ptr) // managed pointers use SafeBool for easy not-null check
        printf("Val: %i\n", ptr->val);
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
} // ptr is null, ptr2 automatically freed
\endcode

Output
\verbatim
Val: 1
Val: 2
Val: 2
\endverbatim

*/

///////////////////////////////////////////////////////////////////////////////

/** \page Sharing Sharing

Sharing is a form of Copy-On-Write (COW) optimization with Reference Counting.

Containers like List and String use sharing:

 - This works in conjunction with \ref Slicing "Slicing" to improve performance by minimizing memory reallocation and copying
 - When a container is copied, a "shared" copy may be used, meaning the underlying data is shared between two (or more) objects
   - Some containers (such as String) may also share a raw pointer (read-only data) such as a string literal -- see example below
   - A shared copy will become a full copy when a write operation (modifier) is called
   - Copying from a managed pointer (SmartPtr, Ptr, etc) always makes a full (unshared) copy
   - An empty container is considered unshared (no data is shared)
   .
 - Containers that support sharing will have these methods:
   - Constructors: Same as set()
   - set(): Set as copy (shared copy if possible)
   - copy(): Set as full (unshared) copy
   - operator=(): Same as set()
   - shared(): Get whether shared
   - unshare(): If shared then make unshared, allocating a new buffer if needed -- calls unslice() (if present)
   - Modifier methods end with a capital 'M' to distinguish them, call unshare() to make the internal buffer unshared and writable
   .
 - Sharing is automatic, though being mindful of how "modifier" methods are used will improve performance

 - \b Caution: Sharing can impact thread safety, always make full (unshared) copies across separate threads
 .

Containers that support Sharing: List, String, PtrList, MapList, MapHash

Example:
\code
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
cout << left << ',' << right << endl;
\endcode

Example with sharing and slicing:
\code
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
cout << left << ',' << right << endl;
\endcode
*/

///////////////////////////////////////////////////////////////////////////////

/** \page Slicing Slicing

Slicing is a way for a container to reference a subset (a "slice") of sequential data.

 - This allows a kind of non-destructive editing where beginning and end items can be "trimmed" without actually modifying the data
 - This works in conjunction with \ref Sharing "Sharing" to improve performance by minimizing memory reallocation and copying
 - The "sliced out" items remain in memory and are automatically cleaned up later
 - Containers that support slicing will have these methods:
   - slice(): Slices object, trimming all items outside given subset
   - unslice(): Cleans up sliced out data -- called automatically by modifier methods
   - triml(): Trim (remove) items from left side (beginning)
   - trimr(): Trim (remove) items from right side (end)
   - truncate(): Truncate to given size by trimming (removing) items from end as needed
   .
 - Inspired by D %Array Slices: http://dlang.org/d-array-article.html
 .

Containers that support Slicing: List, String, MapList

Example:
\code
// Reference a string literal -- no memory allocated here
String str("_test_");

// Remove (slice out) underscore characters, same literal is still referenced
str.slice(1, 4);

// Append a space and number
//  - This is a write operation (modifier) so will allocate a buffer to hold "test 123" -- sliced out data is discarded
str << " 123";

// Prints: test 123
cout << str << endl;
\endcode
*/

///////////////////////////////////////////////////////////////////////////////

/** \page PlainOldData Plain Old Data & Copying

Evo uses these type categories for optimization.

\par Plain Old Data (POD)
These types are plain data primitives that only use default constructors, assignment operators, and destructor.
 - This generally means built-in types such as char or int
 - Containers use memcpy() for copying these types -- constructors, assignment operators, and destructors are skipped
 - You may declare a custom POD type with PODTYPE()

\par Byte-Copy Types
These types can be directly byte-copied without any copy constructor or assignment operator.
 - These types still use a default constructor and destructor
 - Containers use memcpy() for copying these types instead of copy constructor or assignment operator
 - You may declare a custom Byte-Copy type with BCTYPE()

\par Normal Types
These are normal types that use constructors, assignment operators and destructors.
 - This basically means non-POD and non-Byte-Copy types -- the default for custom types not recognized by Evo
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

Primitives:
 - CharT
    - Char
 - IntegerT
    - \link evo::Short Short\endlink, \link evo::Int Int\endlink, \link evo::Long Long\endlink, \link evo::LongL LongL\endlink, \link evo::UShort UShort\endlink, \link evo::UInt UInt\endlink, \link evo::ULong ULong\endlink, \link evo::ULongL ULongL\endlink
    - \link evo::Int8 Int8\endlink, \link evo::Int16 Int16\endlink, \link evo::Int32 Int32\endlink, \link evo::Int64 Int64\endlink, \link evo::UInt8 UInt8\endlink, \link evo::UInt16 UInt16\endlink, \link evo::UInt32 UInt32\endlink, \link evo::UInt64 UInt64\endlink
 - FloatT
    - \link evo::Float Float\endlink, \link evo::FloatD FloatD\endlink, \link evo::FloatL FloatL\endlink
 - SmartPtr, SharedPtr, Ptr
 .

\par Containers

Evo containers:
 - %Nullable: Can have an explicit null value
 - Containers have iterator subtypes:
   - Iter for read-only (const) access
   - IterM for read and write (mutable) access
   - See below for more on iterators
   .
 - Optimizations for handling different types efficiently -- see \ref PlainOldData "Plain Old Data & Copying"
 - May support \ref Sharing "Sharing" and/or \ref Slicing "Slicing"
 - New empty containers do not allocate memory until necessary
 - Advanced methods with "adv" prefix for more intrusive use
 .

Types:
 - Array, List, PtrList, SubList
 - String, SubString, \link evo::StringBase StringBase\endlink
 - Map
   - MapList, \link evo::StrMapList StrMapList\endlink
   - MapHash, \link evo::StrHash StrHash\endlink
   .
 .

\par Iterators

Evo iterators work similar to pointers.
 - Increment or decrement iterator to move to next or previous item
 - Dereference iterator to get value
 .

Iterator types:
 - IteratorFw: Forward iterator
 - IteratorBi: Bidirectional (forward and reverse) iterator
 - IteratorRa: Random access (forward, reverse, and random access) iterator
 - A container will have subtypes defining iterator types to use -- see List::Iter and List::IterM for example
*/

///////////////////////////////////////////////////////////////////////////////

/** \page Streams I/O Streams

Evo supports various types of I/O streams:
 - File
 - Pipe
 - Console

Features include:
 - Output formatting (operator<<)
 - Text newline conversion (LF, CRLF, etc)
 - High performance buffering and I/O filtering
 - Low-level I/O abstraction layer (OS files, pipes, etc) for supporting multiple platforms

Streams inherit from Stream (read/write), StreamIn (read-only), or StreamOut (write-only).
Note that a Stream (read/write) can still be opened in read-only or write-only mode.

\par Implementation Detail

Abstract stream interfaces:
 - StreamBinBase for binary only read/write
 - StreamBase for binary and/or text read/write

Abstract I/O device interface:
 - SysIoDevice

Low level layer for reference:
 - RawBuffer
 - SysFile
 - SysReader
 - SysWriter
*/

// TODO
/* Filtering model



*/

// TODO:
/*
 - Custom filters
 - Custom streams
\par Filtering model
 - Symmetrical implementation, read and write filters use the same interface with buffers: translate buffer input to output buffer
 - Asynchronous model, filters work on buffers rather than read/write calls so by design don't block on I/O

\par Custom Streams
*/

///////////////////////////////////////////////////////////////////////////////

/** \page Metaprogramming Metaprogramming

Evo uses some metaprogramming internally -- these features are available for advanced use.

\par Type Traits

 - PODTYPE()
 - BCTYPE()
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

/** \page ErrorHandling Error Handling

TODO -- work in progress

\par Data Errors

\par Errors and Exceptions

*/

///////////////////////////////////////////////////////////////////////////////

/** \page StlCompatibility STL Compatibility

Evo STL compatibility info.

\par STL String

Evo supports implicitly converting "const std::string&" and "const std::string*" to \link evo::StringBase StringBase\endlink so STL strings can be passed to most functions that
accept Evo string types. Simply include STL strings first and Evo will try to auto-detect and enable std::string support.

Note that in some cases you may run into problems with "too many implicit conversions." Here's an example:
\code
#include <string>
#include <evo/string.h>

std::string stdstring = "test";              // uses default constructor, assigns via temporary, ok here
evo::String string    = stdstring;           // uses default constructor, assigns via temporary, possible compiler error: too many implicit conversions

evo::String string(stdstring);               // ok, explicit constructor (no temporary) -- preferred
evo::String string = StringBase(stdstring);  // ok, explicit temporary
\endcode

You may also enable STL string compatibility directly with a #define, before including any Evo headers:
\code
#define EVO_STD_STRING 1
#include <evo/string.h>
\endcode

For more info on enabling STL string compatibility see: \link EVO_STD_STRING EVO_STD_STRING\endlink
*/

///////////////////////////////////////////////////////////////////////////////

/** \page StringPassing %String Passing

This shows the preferred ways of passing a string with Evo.

 - Passing a string as an argument (by const reference):
   \code
   void foo(const evo::String& str);

   ...
   // Call function, pass a string
   foo("test");
   \endcode
 - Best to "return" a string using an "out" parameter (by reference):
   \code
   void foo(evo::String& out);
   \endcode
 - You can also return a string by value, but this is not recommended (not as efficient):
   \code
   evo::String foo1();          // Return by value: not recommended
   void foo2(evo::String str);  // Pass by value: not recommended
   \endcode
 .

The same applies with general to Evo containers like List, and other complex objects.

*/

/** \page StringConversion %String Conversion

This goes over converting strings to simpler types like numbers. The reverse is covered by \ref StringFormatting "String Formatting".

 - Evo primitives like Bool and \link Int\endlink are useful for conversion. If the conversion fails the result will be null.
 - If conversion to a C++ primitive like bool or int fails, the result will be false or 0, respectively.
 - Evo primitives can be null so they help distinguish between conversion failure and conversion to 0.
 - Conversion failure doesn't throw an error or exception --
   though many conversion functions have a lower-level variant that can return an error code like
   \link String::getnum(Error&,int) const getnum()\endlink

\par Bool Conversion

%Convert strings to bool:
\code
Bool val1 = evo::String("true").boolval();
bool val2 = evo::String("true").getbool<bool>();
\endcode

\par Number Conversion

%Convert strings to numbers:
\code
Int    val1 = evo::String("123").num();
FloatD val2 = evo::String("1.2").numfd();
int    val3 = evo::String("123").getnum<int>();
double val4 = evo::String("1.2").getnumf<double>();
\endcode

\par Generic Conversion

Generic conversion is supported via templates:
\code
Bool      val1 = evo::String("1").convert<evo::Bool>();
Int       val2 = evo::String("123").convert<evo::Int>();
FloatD    val3 = evo::String("1.2").convert<evo::FloatD>();
int       val4 = evo::String("123").convert<int>();
double    val5 = evo::String("1.2").convert<double>();
SubString val6 = evo::String("1.2").convert<evo::SubString>();
\endcode

Reverse generic conversion is also supported. See \ref StringFormatting "String Formatting".

Note: Conversion to an incompatible type will trigger a compiler error.

*/

/** \page StringParsing %String Parsing

This shows different approaches for string parsing.

\par Check Null or Empty

\code
// Null string
if (str.null())
    { }

// Empty string is either null or ""
if (str.empty())
    { }

// Non-empty string
if (!str.empty())
    { }
\endcode

\par Splitting

Splitting to substrings:
\code
evo::String str("foo=bar"), left, right;

// Split to left/right on first occurrence of '='
str.split('=', left, right);

// Split to just left
str.split('=', left);

// Split to just right
str.split('=', vNull, right);
\endcode

Splitting to a list with generic conversion:
\code
evo::String str("1,2,3");

// Split into list
evo::List<evo::Int> nums;
str.split(nums);

// Join back into list
str.join(nums.set());
\endcode

Splitting to a list with generic conversion and alternate tokenizer:
\code
evo::String str("1,'2,2',3");

// Split into list, with tokenizer supporting quoting
evo::List<evo::String> nums;
str.split<evo::StrTokQ>(nums);

// Join back into list, with quoting
str.joinq(nums.set());
\endcode

\par Tokenizing

Parse into substring tokens:
\code
// Tokenize in order (left to right)
evo::StrTok tok1("one,two,three");
while (tok1.next(','))
    cout << tok1.value();

// Tokenize in reverse (right to left)
evo::StrTokR tok2("one,two,three");
while (tok2.next(','))
    cout << tok2.value();

// Tokenize with quoting -- using StrTokQ
evo::StrTokQ tok3("one,'two,two',three");
while (tok3.next(','))
    cout << tok3.value();

// Tokenize with quoting -- using nextq()
evo::StrTok tok4("one,'two,two',three");
while (tok4.nextq(','))
    cout << tok4.value();
\endcode

\par Quoting

For delimited text Evo uses quoting over escaping and avoids having to mutate tokens by adding or filtering escape characters (like backslash).
Multiple quoting types are supported to handle any text.

No quoting:
 - If a token doesn't begin and end with quotes then it's considered unquoted
 - The parser doesn't get confused when quotes appear in unquoted tokens: f'oo,b'ar parses to f'oo and b'ar

Single-char quoting:
 - Single-quotes: 'foo bar'
 - Double-quotes: "foo bar"
 - Backtick-quotes: \`foo bar\`

Triple quoting:
 - Triple single-quotes: '''foo bar'''
 - Triple double-quotes: """foo bar"""
 - Triple backtick-quotes: \`\`\`foo bar\`\`\`

Backtick-DEL quoting:
 - This is single-backtick quoting with a DEL char (code 7F, shown as ␡) added after each backtick: \`␡foo bar\`␡
 - The DEL char is not normally used at all with text, and can still be used as a delimiter
 - Only used if no other quoting is possible, which is rare

*/

/** \page StringFormatting %String Formatting

This shows different approaches for string formatting.

\par Direct Formatting

Methods for direct formatting:
\code
evo::String str;

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
\endcode

\par Stream Formatting

Strings can be appended using the stream operator (<<), similar to output streams:
\code
evo::String str;

// Append string and number
str << "Testing " << 123;

// Clear string and append new string and number
str.set() << "Foobar " << 1.23;
\endcode

\par Generic Conversion

Generic conversion and formatting is supported via templates:
\code
evo::String str;

// set
str.convert_set<evo::Bool>(true);
str.convert_set<evo::Int>(123);
str.convert_set<evo::FloatD>(1.2);
str.convert_set<int>(123);
str.convert_set<double>(1.2);

// append
str.convert_add<evo::Bool>(true);
str.convert_add<evo::Int>(123);
str.convert_add<evo::FloatD>(1.2);
str.convert_add<int>(123);
str.convert_add<double>(1.2);
\endcode

TODO

*/

/** \page StringCustomConversion Custom %String Conversion and Formatting

This shows ways to convert between types.

\par Conversion to Custom Types

Conversion is extendable for custom types by specializing the Convert template class. This is used to:
 - %Convert string to a value and return it -- used by methods like
   \link String::convert() convert()\endlink and \link String::split() split()\endlink.
 - Format string with value (reverse) -- used by methods like
   \link String::convert_set() convert_set()\endlink.
 - Format value and append to string (reverse) -- used by methods like
   \link String::convert_add() convert_add()\endlink and \link String::join() join()\endlink.

Example for converting String to bool:
\code
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

    static bool value(const String& src)
        { return src.getbool<bool>(); }
};
\endcode

Some conversions make sense one way but not the other. Here's an example for converting String to char*:
\code
template<> struct Convert<String,char*> {
    static void set(String& dest, const char* value)
        { dest.set(value); }

    static void add(String& dest, const char* value)
        { dest.add(value); }

    // Unsafe: Converting String to char*
    //template<class U> static char* value(U&)
};
\endcode

*/

///////////////////////////////////////////////////////////////////////////////

/** \page Configuration Configuration

Evo library can be customized using define directives before any Evo headers are included.

TODO -- work in progress
*/ /*
\par Containers

 - To disable container support for allocators\n
   \code
   #define EVO_ALLOCATORS 0
   \endcode
 .

*/

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

/** \page UnsafePtrRef Unsafe Pointer Referencing

Some Evo classes use Pointer Referencing to copy without allocating memory, improving performance.
This can be unsafe (dangerous) in certain situations so documentation for classes/methods that do this will have a warning and a link to this page.

When speed is less important these classes/methods may be avoided for maximum safety -- Evo will still be fast!

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
 - operator=
 - set()

Safety:
 - Referencing static data that is never freed is safe<br>
   Example:\code
eov::String str;
str = "test"; // Safe!
\endcode
 - Best: Avoid overloads that take a raw pointer, pass Ptr instead (forces an unshared copy)<br>
   Example:\code
eov::String str;
{
    char buffer[5];
    strcpy(buffer, "test");
    str = Ptr<char>(buffer);    // Safe (unshared copy)
}
\endcode
 - Call unshare() after Pointer Reference copy to force an unshared copy<br>
   Example:\code
eov::String str;
{
    char buffer[5];
    strcpy(buffer, "test");
    str = buffer;   // Unsafe without next line
    str.unshare();  // Safe (unshared copy)
}
\endcode
 - Use copy() to force a unshared copy<br>
   Example:\code
eov::String str;
{
    char buffer[5];
    strcpy(buffer, "test");
    str.copy(buffer);   // Safe (unshared copy)
}
\endcode

\par Classes

The following classes rely on Pointer Referencing:
 - SubString, SubList, Ptr

Safety:
 - Referencing static data that is never freed is safe<br>
   Example:\code
eov::SubString str;
str = "test"; // Safe!
\endcode
 - Beware referencing a local buffer with shorter life/scope<br>
   Example:\code
eov::SubString substr;
{
    char buffer[5];
    strcpy(buffer, "test");
    substr = buffer;    // BAD!
}
// substr now references an invalid pointer!
\endcode
 - Beware referencing an object with a shorter life/scope<br>
   Example:\code
eov::SubString substr;
{
    String str;
    str.copy("testing");
    substr = str;   // BAD!
}   // str is destroyed
// substr now references an invalid pointer!
\endcode
 - Beware of modifying (reallocating) an object while it's being referenced<br>
   Example:\code
eov::String str;
str.copy("test");
eov::SubString substr;
substr = str;
str.copy("test123");    // BAD! substr now references an invalid pointer since str was reallocated!
\endcode
 - Classes that support Sharing with each other (List, String) are safe to copy and modify in any order<br>
   Example:\code
eov::String str1;
str1.copy("test");
String str2(str1);      // str1 and str2 safely Share same data
str1.copy("test123");   // Safe since Sharing covers modifications on either side
\endcode
*/

///////////////////////////////////////////////////////////////////////////////
} // Namespace: evo
//@}
///////////////////////////////////////////////////////////////////////////////
// Includes
#include "core.h"
///////////////////////////////////////////////////////////////////////////////
#endif
