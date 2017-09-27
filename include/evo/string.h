// Evo C++ Library
/* Copyright (c) 2016 Justin Crowell
 This Source Code Form is subject to the terms of the Mozilla Public
 License, v. 2.0. If a copy of the MPL was not distributed with this
 file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
///////////////////////////////////////////////////////////////////////////////
/** \file string.h Evo String container. */
#pragma once
#ifndef INCL_evo_string_h
#define INCL_evo_string_h

// Includes
#include "list.h"
#include "impl/str.h"

// Namespace: evo
namespace evo {

/** \addtogroup EvoContainers */
//@{

///////////////////////////////////////////////////////////////////////////////

/** %String container.

\par Features

 - Similar to std::string
 - Built-in formatting support -- see operator<<()
 - Built-in conversions -- see num(), numu(), numf(), boolval()
 - Not always terminated, call cstr() for terminated string

 - Preallocates extra memory when buffer grows -- see capacity(), resize(), capacity(Size)
 - No memory allocated by new empty string
 - Inherits efficiency from List

 - \ref Sharing "Sharing" and \ref Slicing "Slicing" make for simple and efficient string parsing
 - \b Caution: Copying from a raw pointer will use \ref UnsafePtrRef "Unsafe Pointer Referencing"
 - See also: SubString
 .

\par Iterators

 - String::Iter -- Read-Only Iterator (IteratorRa)
 - String::IterM -- Mutable Iterator (IteratorRa)

\par Constructors

 - String()
 - String(const String&), ...
 - String(const ListType&,Key,Key), ...
 - String(const ListBaseType&,Key,Key)
 - String(const char*,Size)
 - String(const PtrBase<char>&,Size)
 - String(const char*)
 - String(const PtrBase<char>&)
 .

\par Read Access

 - size()
   - null(), empty()
   - capacity()
   - shared()
 - cstr(String&) const
 - data()
   - item()
   - operator[]()
   - first(), last(), iend()
   - hash()
 - split(C&,char) const
 - split(char,T1&,T2&) const
   - split(char,T1&) const
   - split(char,ValNull,T2&) const
 - splitr(char,T1&,T2&) const
   - splitr(char,T1&) const
   - splitr(char,ValNull,T2&) const
 - splitat(Key,T1&,T2&) const
   - splitat(Key,T1&) const
   - splitat(Key,ValNull,T2&) const
 - find()
   - findr()
   - findany()
   - findanyr()
   - contains(char) const
   - contains(const char*,Size) const
 - compare(const ListType&) const, ...
   - operator==(const String&) const, ...
   - operator==(const char*) const
   - operator!=(const String& data) const, ...
   - operator!=(const char*) const
 - starts(char) const
   - starts(const char*,Size) const
   - ends(char) const
   - ends(const char*,Size) const
 - convert()
 - boolval()
   - getbool() const, getbool(Error&) const
 - num(int) const
   - numl(int) const, numll(int) const
   - numu(int) const, numul(int) const, numull(int) const
   - getnum(int) const, getnum(Error&,int) const
 - numf()
   - numfd(), numfl()
   - getnumf() const, getnumf(Error&) const
 .

\par Slicing

 - slice(Key)
   - slice(Key,Size), slice2()
   - truncate()
   - triml(), trimr()
 - strip()
   - stripl(), stripr()
   - strip(char), stripl(char), stripr(char)
 - pop(), popq()
 - unslice()
 .

\par Modifiers

 - cstr()
 - dataM()
   - itemM()
   - operator()()
 - resize()
   - reserve()
   - capacity(Size)
   - capacitymin(), capacitymax()
   - unshare()
 - set()
   - set(const ListType&)
   - set(const ListType&,Key,Key), ...
   - set(const ListBaseType&,Key,Key)
   - set(const char*,Size)
   - set(const PtrBase<char>&,Size)
   - set(const char*)
   - set(const PtrBase<char>&)
   - set2(const ListType&,Key,Key), ...
   - setn(int,int), setn(uint,int), setn(float,int), ...
   - setempty()
   - clear()
   - operator=(const String&), ...
   - operator=(const ListBaseType&)
   - operator=(const char*)
   - operator=(const PtrBase<char>&)
   - operator=(const ValNull&)
   - operator=(const ValEmpty&)
 - copy(const ListType&)
   - copy(const ListBaseType&)
   - copy(const char*,Size)
   - copy(const char*)
 - convert_set()
   - convert_add()
   - join()
 - add(char)
   - add(const ListType&), ...
   - add(const char*,Size)
   - add(const char*)
   - addsep()
   - addn(int,int), addn(uint,int), addn(float,int), ...
   - operator<<(char)
   - operator<<(const ListType&), ...
   - operator<<(const char*)
   - operator<<(const ValNull&)
   - operator<<(const ValEmpty&)
   - operator<<(int), operator<<(uint), operator<<(float), ...
 - prepend(char)
   - prepend(const ListType&), ...
   - prepend(const char*,Size)
   - prepend(const char*)
   - prependsep()
   - prependn(int,int), prependn(uint,int), prependn(float,int), ...
 - insert(Key,char)
   - insert(Key,const ListType&), ...
   - insert(Key,const char*,Size)
   - insert(Key,const char*)
   - insertn(Key,int,int), insertn(Key,uint,int), insertn(Key,float,int), ...
 - replace(Key,Size,const String&)
   - replace(Key,Size,const char*,Size)
   - replace(Key,Size,const char*)
   - fillch()
 - remove()
   - replace()
 - pop(char&)
   - pop(char&,Key)
   - popq(char&)
 - move(Key,Key)
   - move(Key,ListType&,Key,Size)
   - swap()
 - reverse()
 .

\par Helpers

 - digits()
 - letters()
   - lettersu()
   - lettersl()
 - whitespace()
 .

\par Advanced

 - advItem(Key)
 - advResize()
 - advBuffer(Size)
   - advBuffer()
   - advSize()
 - advEdit()
   - advEditDone()
 - advSwap()
 .

\par Example

\code
#include <evo/string.h>
using namespace evo;

int main() {
    // Create string
    String str = "test";

    // operator[] provides read-only (const) access
    char value = str[0];
    //str[0] = 0;       // Error: operator[] is read-only

    // operator() provides write (mutable) access
    str(0) = 'T';

    // Iterate and print characters (read-only)
    for (String::Iter iter(str); iter; ++iter)
        printf("Ch: %c\n", *iter);

    // Reformat string
    str.clear() << "foo" << ',' << 123;

    // Split string into 2 substrings
    String sub1, sub2;
    str.split(',', sub1, sub2);

    // Print sub1 as string and sub2 as number (dereference Int to int)
    printf("1: %s\n2: %i\n", sub1.cstr(), *sub2.num());

    return 0;
}
\endcode

Output:
\verbatim
Ch: T
Ch: e
Ch: s
Ch: t
1: foo
2: 123
\endverbatim
*/
class String : public List<char,StrSizeT>
{
public:
    typedef String                        ThisType;        ///< This string type
    typedef List<char,StrSizeT>           ListType;        ///< List type

    /** Default constructor sets as null. */
    //[tags: self, set_null! ]
    String()
        { }

    /** Copy constructor.
     - Makes shared copy if possible -- see \ref Sharing "Sharing"
     .
     \param  str  %String to copy
    */
    //[tags: self, set_list!, slice(), unshare() ]
    String(const String& str) : List<char,Size>(str)
        { }

    /** Copy constructor.
     - Makes shared copy if possible -- see \ref Sharing "Sharing"
     .
     \param  str  %String to copy
    */
    //[tags: self, set_list!, slice(), unshare() ]
    String(const ListType& str) : List<char,Size>(str)
        { }

    /** Copy constructor.
     - Makes shared copy if possible -- see \ref Sharing "Sharing"
     .
     \param  str  %String pointer to copy from, ignored if NULL
    */
    //[tags: self, set_list!, slice(), unshare() ]
    String(const ListType* str)
        { if (str != NULL) set(*str); }

    /** Extended copy constructor.
     - Makes shared copy if possible -- see \ref Sharing "Sharing"
     .
     \param  str    %String to copy
     \param  index  Start index of string to copy, END to set as empty
     \param  size   Size as character count, ALL for all from index
    */
    //[tags: self, set_list!, slice(), unshare() ]
    String(const ListType& str, Key index, Key size=ALL) : List<char,Size>(str, index, size)
        { }

    /** Constructor to copy substring data.
     - This always makes an unshared copy
     - For best performance (and less safety) reference substring instead with String(const char*,Size)
     .
     \param  str    %SubString to copy
     \param  index  Start index of string to copy, END to set as empty
     \param  size   Size as character count, ALL for all from index
    */
    //[tags: self, set_list!, slice(), unshare() ]
    String(const ListBaseType& str, Key index=0, Key size=ALL) : List<char,Size>(str, index, size)
        { }

    /** Constructor for string pointer.
     - \b Caution: Uses \ref UnsafePtrRef "Unsafe Pointer Referencing"
     - See String(const PtrBase<char>&,Size) for best safety, use \ref Ptr "Ptr<char>" to wrap raw pointer
     .
     \param  str   %String pointer to use
     \param  size  %String size as character count
    */
    //[tags: self, set_ptr!, add_ptr!, slice(), unshare() ]
    String(const char* str, Size size) : List<char,Size>(str, size)
        { }

    /** Constructor for managed string pointer.
     - This always makes an unshared copy
     - For best performance (and less safety) reference substring instead with String(const char*,Size)
     - Use \ref Ptr "Ptr<char>" to wrap raw pointer
     .
     \param  str   %String pointer, calls set() if NULL
     \param  size  %String size as character count
    */
    //[tags: self, set_ptr!, add_ptr!, slice(), unshare() ]
    String(const PtrBase<char>& str, Size size) {
        if (str.ptr_ != NULL) {
            if (size > 0)
                copy(str.ptr_, size);
            else
                setempty();
        }
    }

    /** Constructor for null terminated string.
     - \b Caution: Uses \ref UnsafePtrRef "Unsafe Pointer Referencing"
     - See String(const PtrBase<char>&) for best safety, use \ref Ptr "Ptr<char>" to wrap raw pointer
     .
     \param  str  %String pointer, calls set() if NULL -- must be null-terminated
    */
    //[tags: self, set_ptr!, add_ptr!, slice(), unshare() ]
    String(const char* str) : List<char,Size>(str, str?strlen(str):0) {
        #if EVO_LIST_OPT_REFTERM
            terminated_ = true;
        #endif
    }

    /** Constructor to copy null terminated string from managed pointer.
     - This always makes an unshared copy
     - For best performance (and less safety) reference substring instead with String(const char*)
     - Use \ref Ptr "Ptr<char>" to wrap raw pointer
     .
     \param  str  %String pointer, calls set() if NULL -- must be null-terminated
    */
    //[tags: self, set_ptr!, add_ptr!, slice(), unshare() ]
    String(const PtrBase<char>& str) {
        if (str.ptr_ != NULL)
            copy(str.ptr_, strlen(str.ptr_));
    }

    // SET

    /** Assignment operator.
     - Makes shared copy if possible -- see \ref Sharing "Sharing"
     .
     \param  str  %String to copy
     \return      This
    */
    //[tags: self, set_list, add_list!, slice(), unshare() ]
    String& operator=(const String& str)
        { set(str); return *this; }

    /** Assignment operator.
     - Makes shared copy if possible -- see \ref Sharing "Sharing"
     .
     \param  str  %String to copy
     \return      This
    */
    //[tags: self, set_list, add_list!, slice(), unshare() ]
    String& operator=(const ListType& str)
        { set(str); return *this; }

    /** Assignment operator for pointer.
     - Makes shared copy if possible -- see \ref Sharing "Sharing"
     .
     \param  str  %String pointer to copy from, calls set() if NULL
     \return      This
    */
    //[tags: self, set_list, add_list!, slice(), unshare() ]
    String& operator=(const ListType* str) {
        if (str != NULL)
            set(*str);
        else
            set();
        return *this;
    }

    /** Assignment operator to reference substring.
     - This always makes an unshared copy
     - For best performance (and less safety) reference substring instead with set(const char*,Size)
     .
     \param  data  Data to copy
     \return       This
    */
    //[tags: self, set_list, add_list!, slice(), unshare() ]
    String& operator=(const ListBaseType& data)
        { ListType::operator=(data); return *this; }

    /** Assignment operator for null terminated string.
     - \b Caution: Uses \ref UnsafePtrRef "Unsafe Pointer Referencing"
     - See operator=(const PtrBase<char>&) for best safety, use \ref Ptr "Ptr<char>" to wrap raw pointer
     .
     \param  str  %String pointer, calls set() if NULL -- must be null-terminated
     \return      This
    */
    //[tags: self, set_ptr!, add_ptr!, slice(), unshare() ]
    String& operator=(const char* str) {
        if (str == NULL)
            set();
        else
            ref(str, strlen(str), true);
        return *this;
    }

    /** Assignment operator to copy from managed pointer with null terminated string.
     - This always makes an unshared copy
     - For best performance (and less safety) reference substring instead with operator=(const char*)
     - Use \ref Ptr "Ptr<char>" to wrap raw pointer
     .
     \param  str  %String pointer, calls set() if NULL -- must be null-terminated
     \return      This
    */
    //[tags: self, set_ptr!, add_ptr!, slice(), unshare() ]
    String& operator=(const PtrBase<char>& str) {
        if (str.ptr_ == NULL)
            set();
        else
            copy(str.ptr_, strlen(str.ptr_));
        return *this;
    }

    /** Assignment operator to set as null and empty.
     - Same as set()
     .
    Example:
    \code
str = vNull;
    \endcode
     \return  This
    */
    //[tags: self, set_null! ]
    String& operator=(const ValNull&)
        { set(); return *this; }

    /** Assignment operator to set as empty but not null.
     - Same as setempty()
     .
    Example:
    \code
str = vEmpty;
    \endcode
     \return  This
    */
    //[tags: self, set_empty! ]
    String& operator=(const ValEmpty&)
        { setempty(); return *this; }

    /** Set as empty but not null.
     - Append operators can be chained\n
       Example:
       \code
// Set as empty then append two characters
str.setempty() << 'a' << 'b';
       \endcode
     .
     \return  This
    */
    //[tags: self, set_empty! ]
    String& setempty()
        { ListType::setempty(); return *this; }

    /** Set as null and empty.
     - Append operators can be chained\n
       Example:
       \code
// Clear string and append two characters
str.set() << 'a' << 'b';
       \endcode
     .
     \return  This
    */
    //[tags: self, set_null! ]
    String& set()
        { ListType::set(); return *this; }

    /** Set from another string.
     - Makes shared copy if possible -- see \ref Sharing "Sharing"
     .
     \param  str  %String to copy
     \return      This
    */
    //[tags: self, set_list, set, setn(int,int), slice(), unshare() ]
    String& set(const ListType& str)
        { ListType::set(str); return *this; }

    /** Set from substring of another string.
     - Makes shared copy if possible -- see \ref Sharing "Sharing"
     .
     \param  str    %String to copy
     \param  index  Start index of data, END to set as empty
     \param  size   Data size as item count, ALL for all from index
     \return        This
    */
    //[tags: self, set_list, set, setn(int,int), slice(), unshare() ]
    String& set(const ListType& str, Key index, Key size=ALL)
        { ListType::set(str, index, size); return *this; }

    /** Set as copy of substring.
     - This always makes an unshared copy
     - For best performance (and less safety) reference substring instead with set(const char*,Size)
     .
     \param  data   Data to copy
     \param  index  Start index of substring data, END to set as empty
     \param  size   Data size as item count, ALL for all from index
     \return        This
    */
    //[tags: self, set_list, set, slice(), unshare() ]
    String& set(const ListBaseType& data, Key index=0, Key size=ALL)
        { ListType::set(data, index, size); return *this; }

    /** Set from string pointer.
     - \b Caution: Uses \ref UnsafePtrRef "Unsafe Pointer Referencing"
     - See set(const PtrBase<char>&,Size) for best safety, use \ref Ptr "Ptr<char>" to wrap raw pointer
     .
     \param  str   %String pointer, calls set() if NULL
     \param  size  %String size as character count
     \return       This
    */
    //[tags: self, set_ptr, set, setn(int,int), slice(), unshare() ]
    String& set(const char* str, Size size)
        { ListType::set(str, size); return *this; }

    /** Set from managed string pointer.
     - This always makes an unshared copy
     - For best performance (and less safety) reference substring instead with set(const char*,Size)
     - Use \ref Ptr "Ptr<char>" to wrap raw pointer
     .
     \param  str   %String pointer, calls set() if NULL
     \param  size  %String size as character count
   */
    String& set(const PtrBase<char>& str, Size size) {
        if (str.ptr_ == NULL)
            ListType::set();
        else
            ListType::copy(str.ptr_, size);
        return *this;
    }

    /** Set from null terminated string.
     - \b Caution: Uses \ref UnsafePtrRef "Unsafe Pointer Referencing"
     - See set(const PtrBase<char>&) for best safety, use \ref Ptr "Ptr<char>" to wrap raw pointer
     .
     \param  str  %String pointer, calls set() if NULL -- must be null-terminated
     \return      This
    */
    //[tags: self, set_ptr, set, setn(int,int), slice(), unshare() ]
    String& set(const char* str) {
        if (str == NULL)
            ListType::set();
        else
            ref(str, strlen(str), true);
        return *this;
    }

    /** Set as copy of null terminated string from managed pointer.
     - This always makes an unshared copy
     - For best performance (and less safety) reference substring instead with set(const char*)
     - Use \ref Ptr "Ptr<char>" to wrap raw pointer
     .
     \param  str  %String pointer, calls set() if NULL -- must be null-terminated
    */
    //[tags: self, set_ptr!, add_ptr!, slice(), unshare() ]
    String& set(const PtrBase<char>& str) {
        if (str.ptr_ == NULL)
            ListType::set();
        else
            ListType::copy(str.ptr_, strlen(str.ptr_));
        return *this;
    }

    /** Set from substring of another string using start/end positions.
     - Makes shared copy if possible -- see \ref Sharing "Sharing"
     - If index2 < index1 then index2 will be set to index1 (empty sublist)
     - Use unshare() afterwards to make a full (unshared) copy
     .
     \param  str     %String to copy
     \param  index1  Start index of data, END to set as empty
     \param  index2  End index of data (this item not included), END for all after index1
     \return         This
    */
    //[tags: self, set_list, set, setn(int,int), slice(), unshare() ]
    String& set2(const ListType& str, Key index1, Key index2)
        { ListType::set2(str, index1, index2); return *this; }

    /** Set and reference sublist using start/end positions.
     - \b Caution: This will reference the same pointer as given sublist, so pointer must remain valid
     - Use unshare() afterwards to make a full (unshared) copy
     .
     \param  data    Data to reference
     \param  index1  Start index of sublist data, END to set as empty
     \param  index2  End index of sublist data (this item not included), END for all after index1
     \return         This
    */
    //[tags: self, set_list, set, slice(), unshare() ]
    String& set2(const ListBaseType& data, Key index1, Key index2)
        { ListType::set2(data, index1, index2); return *this; }

    /** Set as formatted signed number (modifier).
     \param  num   Number to set
     \param  base  Base to use for formatting
     \return       This
    */
    //[tags: self, set_num, set!, slice(), unshare() ]
    String& setn(int num, int base=10)
        { setnum(num, base); return *this; }

    /** Set as formatted signed number (modifier).
     \param  num   Number to set
     \param  base  Base to use for formatting
     \return       This
    */
    //[tags: self, set_num, set!, slice(), unshare() ]
    String& setn(long num, int base=10)
        { setnum(num, base); return *this; }

    /** Set as formatted signed number (modifier).
     \param  num   Number to set
     \param  base  Base to use for formatting
     \return       This
    */
    //[tags: self, set_num, set!, slice(), unshare() ]
    String& setn(longl num, int base=10)
        { setnum(num, base); return *this; }

    /** Set as formatted unsigned number (modifier).
     \param  num   Number to set
     \param  base  Base to use for formatting
     \return       This
    */
    //[tags: self, set_numu, set!, slice(), unshare() ]
    String& setn(uint num, int base=10)
        { setnumu(num, base); return *this; }

    /** Set as formatted unsigned number (modifier).
     \param  num   Number to set
     \param  base  Base to use for formatting
     \return       This
    */
    //[tags: self, set_numu, set!, slice(), unshare() ]
    String& setn(ulong num, int base=10)
        { setnumu(num, base); return *this; }

    /** Set as formatted unsigned number (modifier).
     \param  num   Number to set
     \param  base  Base to use for formatting
     \return       This
    */
    //[tags: self, set_numu, set!, slice(), unshare() ]
    String& setn(ulongl num, int base=10)
        { setnumu(num, base); return *this; }

    /** Set as formatted floating point number (modifier).
     \param  num        Number to set
     \param  precision  Formatting precision (number of fractional digits), 0 for none, PREC_AUTO for automatic
     \return            This
    */
    //[tags: self, set_numf, set!, slice(), unshare() ]
    String& setn(float num, int precision=PREC_AUTO)
        { setnumf(num, precision); return *this; }

    /** Set as formatted floating point number (modifier).
     \param  num        Number to set
     \param  precision  Formatting precision (number of fractional digits), 0 for none, PREC_AUTO for automatic
     \return            This
    */
    //[tags: self, set_numf, set!, slice(), unshare() ]
    String& setn(double num, int precision=PREC_AUTO)
        { setnumf(num, precision); return *this; }

    /** Set as formatted floating point number (modifier).
     \param  num        Number to set
     \param  precision  Formatting precision (number of fractional digits), 0 for none, PREC_AUTO for automatic
     \return            This
    */
    //[tags: self, set_numf, set!, slice(), unshare() ]
    String& setn(ldouble num, int precision=PREC_AUTO)
        { setnumf(num, precision); return *this; }

    // INFO

    /** Get string pointer (const).
     - In some cases string may already be terminated but DO NOT expect this -- use cstr() to get terminated string
     - \b Caution: Calling any modifier/mutable method like unshare() after this may (will) invalidate the returned pointer
     .
     \return  %String pointer as read-only, NULL if null, may be invalid if string empty (const)
    */
    //[tags: info_item, cstr ]
    const char* data() const
        { return data_; }

    /** Get terminated string pointer, using given string buffer if needed (const).
     - This is useful when a temporary null-terminated pointer to a const String is needed
     - Same as data() when already internally terminated, otherwise provided buffer is used to store terminated string
     - \b Caution: Calling any modifier/mutable method like unshare() after this may (will) invalidate the returned pointer
     - \b Caution: Modifying the buffer also may (will) invalidate the returned pointer
     - Example:
       \code
void print(const String& str1, const String& str2) {
    String temp;
    printf("String1: '%s'\n", str1.cstr(temp));
    printf("String2: '%s'\n", str2.cstr(temp));
}
       \endcode
     - BAD Example -- results undefined:
       \code
void bad_print(const String& str1, const String& str2) {
    String temp;
    // Error: Undefined results: pointer from str1.cstr() is likely invalidated by str2.cstr()!
    printf("String1: '%s'\nString2: '%s'\n", str1.cstr(temp), str2.cstr(temp));
}
     \endcode
     .
     \param  buffer  Buffer to use, if needed
     \return         Terminated string pointer
    */
    //[tags: self, cstr ]
    const char* cstr(String& buffer) const
        { return (size_ > 0 ? buffer.set(*this).cstr() : ""); }

    /** Get terminated string pointer (modifier).
     - Use cstr(String&) instead for const instance
     - This effectively calls unshare() and adds an internal null terminator, if needed
     - \b Caution: Calling any modifier method like add(char) after this may (will) invalidate the returned pointer
     .
     \return  Terminated string pointer
    */
    //[tags: self, cstr ]
    const char* cstr() {
        const char* result = "";
        if (size_ > 0) {
            // Not empty
            if (buf_.ptr != NULL) {
                assert( buf_.header != NULL );
                assert( data_ >= buf_.ptr );
                assert( data_ < buf_.ptr+buf_.header->used );
                if (size_ + (data_-buf_.ptr) < buf_.header->used)
                    // End is sliced off, jump to add terminator
                    goto add_term;
            }

            // End not sliced
            #if EVO_LIST_OPT_REFTERM
            if (terminated_) {
                // Already terminated
                result = data_;
            } else
            #endif
            {
                // Add temporary terminator
                add_term:
                reserve(1);
                data_[size_] = '\0';
                result = data_;
            }
        }
        return result;
    }

    /** Get terminated string pointer (modifier).
     - This is potentially unsafe and should be avoided if possible, use cstr() or cstr(String&) instead
     - \b Caution: This defeats "const" and may reallocate or modify the buffer if needed
     - \b Caution: Calling any modifier method like add(char) after this may (will) invalidate the returned pointer
    */
    const char* cstrM() const
        { return (size_ > 0 ? const_cast<String*>(this)->cstr() : ""); }

    // COMPARE

    using ListType::operator==;
    using ListType::operator!=;

    /** Equality operator.
     \param  str  %String to compare to
     \return      Whether equal
    */
    //[tags: self, compare ]
    bool operator==(const String& str) const
        { return ListType::operator==(str); }

    /** Equality operator.
     \param  str  %String to compare to -- must be null terminated
     \return      Whether equal
    */
    //[tags: self, compare ]
    bool operator==(const char* str) const
        { return ( str == NULL ? null() : (strncmp(str, data_, size_) == 0 && str[size_] == '\0') ); }

    /** Inequality operator.
     \param  str  %String to compare to
     \return      Whether inequal
    */
    //[tags: self, compare ]
    bool operator!=(const String& str) const
        { return ListType::operator!=(str); }

    /** Inequality operator.
     \param  str  %String to compare to -- must be null terminated
     \return      Whether inequal
    */
    //[tags: self, compare ]
    bool operator!=(const char* str) const
        { return !( str == NULL ? null() : (strncmp(str, data_, size_) == 0 && str[size_] == '\0') ); }

    // FIND

    /** Find first occurrence of character with forward search.
     - Search stops before reaching end index or end of list
     .
     \param  ch     Character to find
     \param  start  Starting index for search
     \param  end    End index for search, END for end of list
     \return        Found character index or NONE if not found
    */
    //[tags: self, find_item, split! ]
    Key find(char ch, Key start=0, Key end=END) const {
        if (end > size_)
            end = size_;
        for (; start<end; ++start)
            if (data_[start] == ch)
                return start;
        return (Key)NONE;
    }

    /** Find last occurrence of character with reverse search.
     - Same as find() but does reverse search starting right before end index, or at last character if end of list
     - As with find(), character at end index is not checked
     .
     \param  ch     Character to find
     \param  start  Starting index for search range -- last character checked in reverse search
     \param  end    End index for search range (reverse search starting point), END for end of list
     \return        Found character index or NONE if not found
    */
    //[tags: self, find_item, split! ]
    Key findr(char ch, Key start=0, Key end=END) const {
        if (end > size_)
            end = size_;
        while (end>start)
            if (data_[--end] == ch)
                return end;
        return (Key)NONE;
    }

    // TODO
    // find(str)
    // findmap()

    /** Check whether contains given character.
     \param  ch  Character to check for
     \return     Whether character was found
    */
    //[tags: self, compare_sub, find(), compare() ]
    bool contains(char ch) const
        { return ListType::contains(ch); }

    /** Check if starts with given character.
     \param  ch  Character to check
     \return     Whether starts with character
    */
    //[tags: self, compare_sub, find(), compare() ]
    bool starts(char ch) const
        { return ListType::starts(ch); }

    /** Check if ends with given character.
     \param  ch  Character to check
     \return     Whether ends with character
    */
    //[tags: self, compare_sub, find(), compare() ]
    bool ends(char ch) const
        { return ListType::ends(ch); }

    // SPLIT

    /** Split at first occurrence of delimiter into left/right substrings.
     - Template types are automatically deduced from arguments
     - For more advanced parsing see StrTok
     .
     \tparam  T1    %String type to store left substring
     \tparam  T2    %String type to store right substring

     \param  delim  Delimiter to find
     \param  left   Set to substring before delim, set to this if not found [out]
     \param  right  Set to substring after delim, null if not found [out]
     \return        Whether successful, false if delim not found
    */
    //[tags: self, find_item!, split ]
    template<class T1,class T2>
    bool split(char delim, T1& left, T2& right) const {
        for (Key i=0; i<size_; ++i) {
            if (data_[i] == delim) {
                left.set(*this).slice(0, i);
                right.set(*this).slice(i+1, ALL);
                return true;
            }
        }
        left.set(*this);
        right.set();
        return false;
    }

    /** Split at first occurrence of delimiter into left substring.
     - Template types are automatically deduced from arguments
     - For more advanced parsing see StrTok
     .
     \tparam  T1  %String type to store left substring

     \param  delim  Delimiter to find
     \param  left   Set to substring before delim, set to this if not found [out]
     \return        Whether successful, false if delim not found
    */
    //[tags: self, find_item!, split ]
    template<class T1>
    bool split(char delim, T1& left) const {
        for (Key i=0; i<size_; ++i) {
            if (data_[i] == delim) {
                left.set(*this).slice(0, i);
                return true;
            }
        }
        left.set(*this);
        return false;
    }

    /** Split at first occurrence of delimiter into right substring.
     - Template types are automatically deduced from arguments
     - For more advanced parsing see StrTok
     .
     \tparam  T2  %String type to store right substring

     \param  delim  Delimiter to find
     \param  left   vNull (ignored)
     \param  right  Set to substring after delim, null if not found [out]
     \return        Whether successful, false if delim not found
    */
    //[tags: self, find_item!, split ]
    template<class T2>
    bool split(char delim, ValNull left, T2& right) const {
        EVO_PARAM_UNUSED(left);
        for (Key i=0; i<size_; ++i) {
            if (data_[i] == delim) {
                right.set(*this).slice(i+1, ALL);
                return true;
            }
        }
        right.set();
        return false;
    }

    /** Split at last occurrence of delimiter into left/right substrings.
     - Template types are automatically deduced from arguments
     - For more advanced parsing see StrTok
     .
     \tparam  T1    %String type to store left substring
     \tparam  T2    %String type to store right substring

     \param  delim  Delimiter to find
     \param  left   Set to substring before delim, set to this if not found [out]
     \param  right  Set to substring after delim, null if not found [out]
     \return        Whether successful, false if delim not found
    */
    //[tags: self, find_item!, split, set() ]
    template<class T1,class T2>
    bool splitr(char delim, T1& left, T2& right) const {
        for (Key i=size_; i>0; ) {
            if (data_[--i] == delim) {
                left.set(*this).slice(0, i);
                right.set(*this).slice(i+1, ALL);
                return true;
            }
        }
        left.set(*this);
        right.set();
        return false;
    }

    /** Split at last occurrence of delimiter into left substring.
     - Template types are automatically deduced from arguments
     - For more advanced parsing see StrTok
     .
     \tparam  T1    %String type to store left substring

     \param  delim  Delimiter to find
     \param  left   Set to substring before delim, set to this if not found [out]
     \return        Whether successful, false if delim not found
    */
    //[tags: self, find_item!, split, set() ]
    template<class T1>
    bool splitr(char delim, T1& left) const {
        for (Key i=size_; i>0; ) {
            if (data_[--i] == delim) {
                left.set(*this).slice(0, i);
                return true;
            }
        }
        left.set(*this);
        return false;
    }

    /** Split at last occurrence of delimiter into right substring.
     - Template types are automatically deduced from arguments
     - For more advanced parsing see StrTok
     .
     \tparam  T2    %String type to store right substring

     \param  delim  Delimiter to find
     \param  left   vNull (ignored)
     \param  right  Set to substring after delim, null if not found [out]
     \return        Whether successful, false if delim not found
    */
    //[tags: self, find_item!, split, set() ]
    template<class T2>
    bool splitr(char delim, ValNull left, T2& right) const {
        EVO_PARAM_UNUSED(left);
        for (Key i=size_; i>0; ) {
            if (data_[--i] == delim) {
                right.set(*this).slice(i+1, ALL);
                return true;
            }
        }
        right.set();
        return false;
    }

    // TODO category??

    // SPLIT_SET

    // TODO: split_setl()
    // TODO: split_setr()

    // TODO: splitr_setl()
    // TODO: splitr_setr()

    // TRIM/STRIP

    // TODO include newlines as whitespace

    /** Strip left (beginning) and right (ending) whitespace (spaces and tabs).
     - This non-destructively removes whitespace so data isn't modified -- see slice()
     .
     \return  This
    */
    //[tags: self, trim, slice, unshare() ]
    String& strip() {
        char ch;
        while ( size_ > 0 && ((ch=data_[size_-1]) == ' ' || ch == '\t') )
            --size_;
        Size count = 0;
        while ( count < size_ && ((ch=data_[count]) == ' ' || ch == '\t') )
            ++count;
        triml(count);
        return *this;
    }

    /** Strip left (beginning) and right (ending) occurences of character.
     - This non-destructively removes characters so data isn't modified -- see slice()
     .
     \param  ch  Character to strip
     \return     This
    */
    //[tags: self, trim, slice, unshare() ]
    String& strip(char ch) {
        while (size_ > 0 && data_[size_-1] == ch)
            --size_;
        Size count = 0;
        while (count < size_ && data_[count] == ch)
            ++count;
        triml(count);
        return *this;
    }

    /** Strip left (beginning) whitespace (spaces and tabs).
     - This non-destructively removes whitespace so data isn't modified -- see slice()
     .
     \return  This
    */
    //[tags: self, trim, slice, unshare() ]
    String& stripl() {
        char ch;
        Size count = 0;
        while ( count < size_ && ((ch=data_[count]) == ' ' || ch == '\t') )
            ++count;
        triml(count);
        return *this;
    }

    /** Strip left (beginning) occurrences of character.
     - This non-destructively removes characters so data isn't modified -- see slice()
     .
     \param  ch  Character to strip
     \return     This
    */
    //[tags: self, trim, slice, unshare() ]
    String& stripl(char ch) {
        Size count = 0;
        while (count < size_ && data_[count] == ch)
            ++count;
        triml(count);
        return *this;
    }

    /** Strip right (ending) whitespace (spaces and tabs).
     - This non-destructively removes whitespace so data isn't modified -- see slice()
     .
     \return  This
    */
    //[tags: self, trim, slice, unshare() ]
    String& stripr() {
        char ch;
        while ( size_ > 0 && ((ch=data_[size_-1]) == ' ' || ch == '\t') )
            --size_;
        return *this;
    }

    /** Strip right (ending) occurences of character.
     - This non-destructively removes characters so data isn't modified -- see slice()
     .
     \return  This
    */
    //[tags: self, trim, slice, unshare() ]
    String& stripr(char ch) {
        while (size_ > 0 && data_[size_-1] == ch)
            --size_;
        return *this;
    }

    // COPY

    /** Set as full (unshared) copy of another string (modifier).
     \param  str  %String to copy
     \return      This
    */
    //[tags: self, set_list, set, slice(), unshare() ]
    String& copy(const ListType& str)
        { ListType::copy(str); return *this; }

    /** Set as full (unshared) copy of substring (modifier).
     \param  str  %String to copy
     \return      This
    */
    //[tags: self, set_list, set, slice(), unshare() ]
    String& copy(const ListBaseType& str)
        { ListType::copy(str); return *this; }

    /** Set as full (unshared) copy using string pointer (modifier).
     \param  str   %String to copy
     \param  size  %String size as character count
     \return       This
    */
    //[tags: self, set_ptr, set, slice(), unshare() ]
    String& copy(const char* str, Size size)
        { ListType::copy(str, size); return *this; }

    /** Set as full (unshared) copy of null terminated string (modifier).
     \param  str  %String to copy -- must be null-terminated
     \return      This
    */
    //[tags: self, set_ptr, set, slice(), unshare() ]
    String& copy(const char* str) {
        if (str == NULL)
            ListType::set();
        else
            ListType::copy(str, strlen(str));
        return *this;
    }

    // ADD

    /** Append character (modifier).
     \param  ch  Character to append
     \return     This
    */
    //[tags: self, add, add_item, add(int,int), addrem_item, set_item, remove(), reserve() ]
    String& add(char ch)
        { ListType::add(ch); return *this; }

    /** Append from another string (modifier).
     - For best performance use set(const String&) when this is empty
     .
     \param  str  %String to append
     \return      This
    */
    //[tags: self, add, add(int,int), addrem_list, set_list, remove(), reserve() ]
    String& add(const ListType& str)
        { ListType::add(str); return *this; }

    /** Append from another string (modifier).
     - For best performance use set(const String&) when this is empty
     .
     \param  str  %String to append
     \return      This
    */
    //[tags: self, add, add(int,int), addrem_list, set_list, remove(), reserve() ]
    String& add(const ListBaseType& str)
        { ListType::add(str); return *this; }

    /** Append from string pointer (modifier).
     - For best performance use set(const char*,Size) when this is empty
     .
     \param  str   %String to append
     \param  size  %String size as character count to append
     \return       This
    */
    //[tags: self, add, add(int,int), addrem_ptr, set_ptr, remove(), reserve() ]
    String& add(const char* str, Size size)
        { ListType::add(str, size); return *this; }

    /** Append null terminated string (modifier).
     - For best performance use set(const char*) when this is empty
     .
     \param  str  %String to append -- must be null-terminated
     \return      This
    */
    //[tags: self, add, add(int,int), addrem_ptr, set_ptr, remove(), reserve() ]
    String& add(const char* str)
        { if (str) add(str, strlen(str)); return *this; }

    /** Append separator/delimiter if needed (modifier).
     - This will only append given delim if not empty and not already ending with delim
     .
     \param  delim  Delimiter to append
     \return        This
    */
    //[tags: self, add, add_item, add(int,int), addrem_item, set_item, remove(), reserve() ]
    String& addsep(char delim=',')
        { if (size_ > 0 && data_[size_-1] != delim) add(delim); return *this; }

    /** Append formatted signed number (modifier).
     \param  num   Number to append
     \param  base  Base to use for formatting
     \return       This
    */
    //[tags: self, add_int, add!, remove(), reserve() ]
    String& addn(int num, int base=10)
        { addnum(num, base); return *this; }

    /** Append formatted signed number (modifier).
     \param  num   Number to append
     \param  base  Base to use for formatting
     \return       This
    */
    //[tags: self, add_long, add!, remove(), reserve() ]
    String& addn(long num, int base=10)
        { addnum(num, base); return *this; }

    /** Append formatted signed number (modifier).
     \param  num   Number to append
     \param  base  Base to use for formatting
     \return       This
    */
    //[tags: self, add_longl, add!, remove(), reserve() ]
    String& addn(longl num, int base=10)
        { addnum(num, base); return *this; }

    /** Append formatted unsigned number (modifier).
     \param  num   Number to append
     \param  base  Base to use for formatting
     \return       This
    */
    //[tags: self, add_uint, add!, remove(), reserve() ]
    String& addn(uint num, int base=10)
        { addnumu(num, base); return *this; }

    /** Append formatted unsigned number (modifier).
     \param  num   Number to append
     \param  base  Base to use for formatting
     \return       This
    */
    //[tags: self, add_ulong, add!, remove(), reserve() ]
    String& addn(ulong num, int base=10)
        { addnumu(num, base); return *this; }

    /** Append formatted unsigned number (modifier).
     \param  num   Number to append
     \param  base  Base to use for formatting
     \return       This
    */
    //[tags: self, add_ulongl, add!, remove(), reserve() ]
    String& addn(ulongl num, int base=10)
        { addnumu(num, base); return *this; }

    /** Append formatted floating point number (modifier).
     \param  num        Number to append
     \param  precision  Formatting precision (number of fractional digits), 0 for none, PREC_AUTO for automatic
     \return            This
    */
    //[tags: self, add_float, add!, remove(), reserve() ]
    String& addn(float num, int precision=PREC_AUTO)
        { addnumf(num, precision); return *this; }

    /** Append formatted floating point number (modifier).
     \param  num        Number to append
     \param  precision  Formatting precision (number of fractional digits), 0 for none, PREC_AUTO for automatic
     \return            This
    */
    //[tags: self, add_double, add!, remove(), reserve() ]
    String& addn(double num, int precision=PREC_AUTO)
        { addnumf(num, precision); return *this; }

    /** Append formatted floating point number (modifier).
     \param  num        Number to append
     \param  precision  Formatting precision (number of fractional digits), 0 for none, PREC_AUTO for automatic
     \return            This
    */
    //[tags: self, add_ldouble, add!, remove(), reserve() ]
    String& addn(ldouble num, int precision=PREC_AUTO)
        { addnumf(num, precision); return *this; }

    /** Append operator.
     - Same as add(char)
     - Append operators can be chained\n
       Example:
       \code
// Clear string and append two characters
str << vEmpty << 'a' << 'b';
       \endcode
     .
     \param  ch  Character to append
     \return     This
    */
    //[tags: self, add_item!, set_item ]
    String& operator<<(char ch)
        { return add(ch); }

    /** Append operator.
     - Same as add(const ListType&)
     - Append operators can be chained\n
       Example:
       \code
// Clear string and append two strings
str << vEmpty << str1 << str2;
       \endcode
     .
     \param  str  %String to append
     \return      This
    */
    //[tags: self, add_list!, set_list ]
    String& operator<<(const ListType& str)
        { return add(str); }

    /** Append operator.
     - Same as add(const ListBaseType&)
     - Append operators can be chained\n
       Example:
       \code
// Clear string and append two strings
str << vEmpty << str1 << str2;
       \endcode
     .
     \param  str  %String to append

     \return      This
    */
    //[tags: self, add_list!, set_list ]
    String& operator<<(const ListBaseType& str)
        { return add(str); }

    /** Append operator.
     - Same as add(const char*)
     - Append operators can be chained\n
       Example:
       \code
// Clear string and append two strings
str << vEmpty << "foo" << "bar";
       \endcode
     .
     \param  str  %String pointer to append -- must be null terminated
     \return      This
    */
    //[tags: self, add_ptr!, set_ptr ]
    String& operator<<(const char* str)
        { return add(str); }

    /** Append operator to set as null and empty.
     - Same as set()
     - Append operators can be chained\n
       Example:
       \code
// Clear string and append two strings
str << vNull << "foo" << "bar";
       \endcode
     .
     \param  val  vNull
     \return      This
    */
    //[tags: self, set_null! ]
    String& operator<<(const ValNull& val) {
        EVO_PARAM_UNUSED(val);
        return set();
    }

    /** Append operator to set as empty but not null.
     - Same as setempty()
     - Append operators can be chained\n
       Example:
       \code
// Clear string and append two strings
str << vEmpty << "foo" << "bar";
       \endcode
     .
     \param  val  vEmpty
     \return      This
    */
    //[tags: self, set_empty! ]
    String& operator<<(const ValEmpty& val) {
        EVO_PARAM_UNUSED(val);
        return setempty();
    }

    /** Append operator to append newline.
     - Append operators can be chained\n
       Example:
       \code
// Clear string and append string and newline
str << vEmpty << "foo" << NL;
       \endcode
     .
     \param  nl  Newline type, NL for system newline
     \return     This
    */
    //[tags: self ]
    String& operator<<(Newline nl) {
        add(getnewline(nl), getnewlinesize(nl));
        return *this;
    }

    /** Append operator to append formatted number.
     - Same as addn(int,int) with base=10
     - Append operators can be chained\n
       Example:
       \code
// Clear string and append a number
str << vEmpty << 123;
       \endcode
     .
     \param  num  Number to format and append
     \return      This
    */
    //[tags: self, add_int, add!, remove(), reserve() ]
    String& operator<<(int num)
        { addnum(num); return *this; }

    /** Append operator to append formatted number.
     - Same as addn(long,int) with base=10
     - Append operators can be chained\n
       Example:
       \code
// Clear string and append a number
str << vEmpty << 123;
       \endcode
     .
     \param  num  Number to format and append
     \return      This
    */
    //[tags: self, add_long, add!, remove(), reserve() ]
    String& operator<<(long num)
        { addnum(num); return *this; }

    /** Append operator to append formatted number.
     - Same as addn(longl,int) with base=10
     - Append operators can be chained\n
       Example:
       \code
// Clear string and append a number
str << vEmpty << 123;
       \endcode
     .
     \param  num  Number to format and append
     \return      This
    */
    //[tags: self, add_longl, add!, remove(), reserve() ]
    String& operator<<(longl num)
        { addnum(num); return *this; }

    /** Append operator to append formatted number.
     - Same as addn(uint,int) with base=10
     - Append operators can be chained\n
       Example:
       \code
// Clear string and append a number
str << vEmpty << 123;
       \endcode
     .
     \param  num  Number to format and append
     \return      This
    */
    //[tags: self, add_uint, add!, remove(), reserve() ]
    String& operator<<(uint num)
        { addnumu(num); return *this; }

    /** Append operator to append formatted number.
     - Same as addn(ulong,int) with base=10
     - Append operators can be chained\n
       Example:
       \code
// Clear string and append a number
str << vEmpty << 123;
       \endcode
     .
     \param  num  Number to format and append
     \return      This
    */
    //[tags: self, add_ulong, add!, remove(), reserve() ]
    String& operator<<(ulong num)
        { addnumu(num); return *this; }

    /** Append operator to append formatted number.
     - Same as addn(ulongl,int) with base=10
     - Append operators can be chained\n
       Example:
       \code
// Clear string and append a number
str << vEmpty << 123;
       \endcode
     .
     \param  num  Number to format and append
     \return      This
    */
    //[tags: self, add_ulongl, add!, remove(), reserve() ]
    String& operator<<(ulongl num)
        { addnumu(num); return *this; }

    /** Append operator to append formatted number.
     - Same as addn(float,int) with precision=PREC_AUTO
     - Append operators can be chained\n
       Example:
       \code
// Clear string and append a number
str << vEmpty << 123.4;
       \endcode
     .
     \param  num  Number to format and append
     \return      This
    */
    //[tags: self, add_float, add!, remove(), reserve() ]
    String& operator<<(float num)
        { addnumf(num); return *this; } // TODO, precision -1

    /** Append operator to append formatted number.
     - Same as addn(double,int) with precision=PREC_AUTO
     - Append operators can be chained\n
       Example:
       \code
// Clear string and append a number
str << vEmpty << 123.4;
       \endcode
     .
     \param  num  Number to format and append
     \return      This
    */
    //[tags: self, add_double, add!, remove(), reserve() ]
    String& operator<<(double num)
        { addnumf(num); return *this; }

    /** Append operator to append formatted number.
     - Same as addn(ldouble,int) with precision=PREC_AUTO
     - Append operators can be chained\n
       Example:
       \code
// Clear string and append a number
str << vEmpty << 123.4;
       \endcode
     .
     \param  num  Number to format and append
     \return      This
    */
    //[tags: self, add_ldouble, add!, remove(), reserve() ]
    String& operator<<(ldouble num)
        { addnumf(num); return *this; }

    // PREPEND

    /** Prepend character (modifier).
     \param  ch  Character to prepend
     \return     This
    */
    //[tags: self, prepend, prepend(int,int), addrem_item, set_item, remove(), reserve() ]
    String& prepend(char ch)
        { ListType::prepend(ch); return *this; }

    /** Prepend from another string (modifier).
     \param  str  %String to prepend
     \return      This
    */
    //[tags: self, prepend, prepend(int,int), addrem_list, set_list, remove(), reserve() ]
    String& prepend(const ListType& str)
        { ListType::prepend(str); return *this; }

    /** Prepend from another string (modifier).
     \param  str  %String to prepend
     \return      This
    */
    //[tags: self, prepend, prepend(int,int), addrem_list, set_list, remove(), reserve() ]
    String& prepend(const ListBaseType& str)
        { ListType::prepend(str); return *this; }

    /** Prepend from string pointer (modifier).
     \param  str   %String to prepend
     \param  size  %String size as character count to prepend
     \return       This
    */
    //[tags: self, prepend, prepend(int,int), addrem_ptr, set_ptr, remove(), reserve() ]
    String& prepend(const char* str, Size size)
        { ListType::prepend(str, size); return *this; }

    /** Prepend null terminated string (modifier).
     \param  str  %String to prepend -- must be null-terminated
     \return      This
    */
    //[tags: self, prepend, prepend(int,int), addrem_ptr, set_ptr, remove(), reserve() ]
    String& prepend(const char* str)
        { if (str) prepend(str, strlen(str)); return *this; }

    /** Prepend separator/delimiter if needed (modifier).
     - This will only prepend given delim if not empty and not already starting with delim
     .
     \param  delim  Delimiter to prepend
     \return        This
    */
    //[tags: self, prepend, prepend(int,int), addrem_item, set_item, remove(), reserve() ]
    String& prependsep(char delim=',')
        { if (size_ > 0 && data_[0] != delim) prepend(delim); return *this; }

    /** Prepend formatted signed number (modifier).
     \param  num   Number to prepend
     \param  base  Base to use for formatting
     \return       This
    */
    //[tags: self, prepend_int, prepend!, remove(), reserve() ]
    String& prependn(int num, int base=10)
        { prependnum(num, base); return *this; }

    /** Prepend formatted signed number (modifier).
     \param  num   Number to prepend
     \param  base  Base to use for formatting
     \return       This
    */
    //[tags: self, prepend_long, prepend!, remove(), reserve() ]
    String& prependn(long num, int base=10)
        { prependnum(num, base); return *this; }

    /** Prepend formatted signed number (modifier).
     \param  num   Number to prepend
     \param  base  Base to use for formatting
     \return       This
    */
    //[tags: self, prepend_longl, prepend!, remove(), reserve() ]
    String& prependn(longl num, int base=10)
        { prependnum(num, base); return *this; }

    /** Prepend formatted unsigned number (modifier).
     \param  num   Number to prepend
     \param  base  Base to use for formatting
     \return       This
    */
    //[tags: self, prepend_uint, prepend!, remove(), reserve() ]
    String& prependn(uint num, int base=10)
        { prependnum(num, base); return *this; }

    /** Prepend formatted unsigned number (modifier).
     \param  num   Number to prepend
     \param  base  Base to use for formatting
     \return       This
    */
    //[tags: self, prepend_ulong, prepend!, remove(), reserve() ]
    String& prependn(ulong num, int base=10)
        { prependnum(num, base); return *this; }

    /** Prepend formatted unsigned number (modifier).
     \param  num   Number to prepend
     \param  base  Base to use for formatting
     \return       This
    */
    //[tags: self, prepend_ulongl, prepend!, remove(), reserve() ]
    String& prependn(ulongl num, int base=10)
        { prependnum(num, base); return *this; }

    /** Prepend formatted floating point number (modifier).
     \param  num        Number to prepend
     \param  precision  Formatting precision (number of fractional digits), 0 for none, PREC_AUTO for automatic
     \return            This
    */
    //[tags: self, prepend_float, prepend!, remove(), reserve() ]
    String& prependn(float num, int precision=PREC_AUTO)
        { prependnumf(num, precision); return *this; }

    /** Prepend formatted floating point number (modifier).
     \param  num        Number to prepend
     \param  precision  Formatting precision (number of fractional digits), 0 for none, PREC_AUTO for automatic
     \return            This
    */
    //[tags: self, prepend_double, prepend!, remove(), reserve() ]
    String& prependn(double num, int precision=PREC_AUTO)
        { prependnumf(num, precision); return *this; }

    /** Prepend formatted floating point number (modifier).
     \param  num        Number to prepend
     \param  precision  Formatting precision (number of fractional digits), 0 for none, PREC_AUTO for automatic
     \return            This
    */
    //[tags: self, prepend_ldouble, prepend!, remove(), reserve() ]
    String& prependn(ldouble num, int precision=PREC_AUTO)
        { prependnumf(num, precision); return *this; }

    // INSERT

    /** Insert character (modifier).
     \param  index  Insert index, END to append
     \param  ch     Character to insert
     \return        Inserted index
    */
    //[tags: self, insert, insert_item, insertn(Key,int,int), addrem_item, set_item, remove(), reserve() ]
    Size insert(Key index, char ch)
        { return ListType::insert(index, ch); }

    /** Insert from another string (modifier).
     \param  index  Insert index, END to append
     \param  str    %String to insert
     \return        Inserted index
    */
    //[tags: self, insert, insertn(Key,int,int), addrem_list, set_list, remove(), reserve() ]
    Size insert(Key index, const ListType& str)
        { return ListType::insert(index, str); }

    /** Insert from another string (modifier).
     \param  index  Insert index, END to append
     \param  str    %String to insert
     \return        Inserted index
    */
    //[tags: self, insert, insertn(Key,int,int), addrem_list, set_list, remove(), reserve() ]
    Size insert(Key index, const ListBaseType& str)
        { return ListType::insert(index, str); }

    /** Insert string data (modifier).
     \param  index  Insert index, END to append
     \param  str    %String to insert
     \param  size   %String size as character count to insert
     \return        Inserted index
    */
    //[tags: self, insert, insertn(Key,int,int), addrem_ptr, set_ptr, remove(), reserve() ]
    Size insert(Key index, const char* str, Size size)
        { return ListType::insert(index, str, size); }

    /** Insert null terminated string (modifier).
     \param  index  Insert index, END to append
     \param  str    %String to insert -- must be null-terminated
     \return        This
    */
    //[tags: self, insert, insertn(Key,int,int), addrem_ptr, set_ptr, remove(), reserve() ]
    String& insert(Key index, const char* str)
        { if (str) insert(index, str, strlen(str)); return *this; }

    /** Insert formatted signed number (modifier).
     \param  index  Insert index, END to append
     \param  num    Number to insert
     \param  base   Base to use for formatting
     \return        This
    */
    //[tags: self, insert_int, insert!, remove(), reserve() ]
    String& insertn(Key index, int num, int base=10)
        { insertnum(index, num, base); return *this; }

    /** Insert formatted signed number (modifier).
     \param  index  Insert index, END to append
     \param  num    Number to insert
     \param  base   Base to use for formatting
     \return        This
    */
    //[tags: self, insert_long, insert!, remove(), reserve() ]
    String& insertn(Key index, long num, int base=10)
        { insertnum(index, num, base); return *this; }

    /** Insert formatted signed number (modifier).
     \param  index  Insert index, END to append
     \param  num    Number to insert
     \param  base   Base to use for formatting
     \return        This
    */
    //[tags: self, insert_longl, insert!, remove(), reserve() ]
    String& insertn(Key index, longl num, int base=10)
        { insertnum(index, num, base); return *this; }

    /** Insert formatted unsigned number (modifier).
     \param  index  Insert index, END to append
     \param  num    Number to insert
     \param  base   Base to use for formatting
     \return        This
    */
    //[tags: self, insert_uint, insert!, remove(), reserve() ]
    String& insertn(Key index, uint num, int base=10)
        { insertnum(index, num, base); return *this; }

    /** Insert formatted unsigned number (modifier).
     \param  index  Insert index, END to append
     \param  num    Number to insert
     \param  base   Base to use for formatting
     \return        This
    */
    //[tags: self, insert_ulong, insert!, remove(), reserve() ]
    String& insertn(Key index, ulong num, int base=10)
        { insertnum(index, num, base); return *this; }

    /** Insert formatted unsigned number (modifier).
     \param  index  Insert index, END to append
     \param  num    Number to insert
     \param  base   Base to use for formatting
     \return        This
    */
    //[tags: self, insert_ulongl, insert!, remove(), reserve() ]
    String& insertn(Key index, ulongl num, int base=10)
        { insertnum(index, num, base); return *this; }

    /** Insert formatted floating point number (modifier).
     \param  index      Insert index, END to append
     \param  num        Number to insert
     \param  precision  Formatting precision (number of fractional digits), 0 for none, PREC_AUTO for automatic
     \return            This
    */
    //[tags: self, insert_float, insert!, remove(), reserve() ]
    String& insertn(Key index, float num, int precision=PREC_AUTO)
        { insertnumf(index, num, precision); return *this; }

    /** Insert formatted floating point number (modifier).
     \param  index      Insert index, END to append
     \param  num        Number to insert
     \param  precision  Formatting precision (number of fractional digits), 0 for none, PREC_AUTO for automatic
     \return            This
    */
    //[tags: self, insert_double, insert!, remove(), reserve() ]
    String& insertn(Key index, double num, int precision=PREC_AUTO)
        { insertnumf(index, num, precision); return *this; }

    /** Insert formatted floating point number (modifier).
     \param  index      Insert index, END to append
     \param  num        Number to insert
     \param  precision  Formatting precision (number of fractional digits), 0 for none, PREC_AUTO for automatic
     \return            This
    */
    //[tags: self, insert_ldouble, insert!, remove(), reserve() ]
    String& insertn(Key index, ldouble num, int precision=PREC_AUTO)
        { insertnumf(index, num, precision); return *this; }

    // FILL

    /** Fill with copies of character (modifier).
     \param  ch     Character to fill with
     \param  index  Start index, END to start at end and append
     \param  size   Size to fill as character count from index, ALL for all items from index, 0 to do nothing
    */
    //[tags: self, replace, resize() ]
    String& fillch(char ch, Key index=0, Size size=ALL)
        { ListType::fill(ch, index, size); return *this; }

    // REPLACE

    /** Replace characters with string (modifier).
     \param  index  Start index to replace
     \param  rsize  Size as item count from index to replace, ALL for all items from index
     \param  str    Replacement string to copy
     \return        This
    */
    //[tags: self, replace, set_ptr!, add_ptr!, remove(), resize() ]
    String& replace(Key index, Size rsize, const String& str)
        { ListType::replace(index, rsize, str.data_, str.size_); return *this; }

    /** Replace characters with string (modifier).
     \param  index  Start index to replace
     \param  rsize  Size as item count from index to replace, ALL for all items from index
     \param  str    Replacement string to copy
     \param  size   Replacement string size as character count
     \return        This
    */
    //[tags: self, replace, set_ptr!, add_ptr!, remove(), resize() ]
    String& replace(Key index, Size rsize, const char* str, Size size)
        { ListType::replace(index, rsize, str, size); return *this; }

    /** Replace characters with string (modifier).
     \param  index  Start index to replace
     \param  rsize  Size as item count from index to replace, ALL for all items from index
     \param  str    Replacement string to copy -- must be null-terminated
     \return        This
    */
    //[tags: self, replace, set_ptr!, add_ptr!, remove(), resize() ]
    String& replace(Key index, Size rsize, const char* str)
        { ListType::replace(index, rsize, str, str?strlen(str):0); return *this; }

    // TODO
    //replace(Key index, Size rsize, char ch, Size copies=1)

    // GETBOOL

    /** Convert to bool value for given boolean type. Fails on bad format, empty, or number overflow.

    Format: [WHITESPACE] ["on"|"off"|"yes"|"no"|"true"|"false"|"t"|"f"|DIGITS] [WHITESPACE]
     - Result is true if non-zero number -- checked using getnum(Error&,int) with T=long, base=0
     - Case insensitive
     .
    Error codes:
     - ENone: success
     - EInval: bad format, invalid character, or no digits
     - EOutOfBounds: number overflow
     .
     \param  error  Stores conversion error code, ENone on success [out]
     \return        Converted value on success, partially converted value or 0 on failure
     */
    //[tags: self, boolerr ]
    bool getbool(Error& error) const {
        assert( data_ > (char*)1 || size_ == 0 );
        return impl::tobool(data_, size_, error);
    }

    /** Convert to bool value for given boolean type. Fails on bad format, empty, or number overflow

    Format: [WHITESPACE] ["on"|"off"|"yes"|"no"|"true"|"false"|"t"|"f"|DIGITS] [WHITESPACE]
     - Result is true if non-zero number -- checked using getnum(Error&,int) with T=long, base=0
     - Case insensitive
     .
     \tparam  T  Boolean type to convert to -- can be Bool or bool
     \return  Converted value on success, Bool set as null on failure, primitive is false on failure
     */
    //[tags: self, bool ]
    template<class T> T getbool() const {
        assert( data_ > (char*)1 || size_ == 0 );
        return StaticIf< IsPodType<T>::value, impl::ToBoolPod<T>, impl::ToBool<T> >::Type::getbool(data_, size_);
    }

    // GETNUM

    /** Convert to number value for given integer type. Fails on bad format, no digits, or overflow.

    Format: [WHITESPACE] ["+"|"-"] ["0x"|"0X"|"x"|"0"] DIGITS [WHITESPACE]
     - Base 2-36 supported -- Base 0 autodetects hex or octal based on prefix, defaults to decimal
     - Base 16 (hex) may use "0x", "0X", or "x" prefix
     - Base 8 (octal) may use "0" prefix
     - Letters in digits are not case sensitive
     .
    Error codes:
     - ENone: success
     - EInval: bad format, invalid character, or no digits
     - EOutOfBounds: number overflow
     .
     \tparam  T  Basic integer type to convert to -- must be primitive like: int32, int, long, uint32, uint, ulong, etc
     \param  error  Stores conversion error code, ENone on success [out]
     \param  base   Conversion base, 0 for autodetect
     \return        Converted value on success, partially converted value or 0 on failure
    */
    //[tags: self, numerr ]
    template<class T> T getnum(Error& error, int base=0) const {
        STATIC_ASSERT( IsPodType<T>::value, getnum_POD_Type_Required );
        assert( data_ > (char*)1 || size_ == 0 );
        return impl::tonum<T>(data_, size_, error, base);
    }

    /** Convert to number value for given integer type. Fails on bad format, no digits, or overflow.

    Format: [WHITESPACE] ["+"|"-"] ["0x"|"0X"|"x"|"0"] DIGITS [WHITESPACE]
     - Base 2-36 supported -- Base 0 autodetects hex or octal based on prefix, defaults to decimal
     - Base 16 (hex) may use "0x", "0X", or "x" prefix
     - Base 8 (octal) may use "0" prefix
     - Letters in digits are not case sensitive
     .
     \tparam  T  Integer type to convert to -- can be IntegerT type like Int32, Int, ULong, etc or primitive like int32, int, ulong, etc
     \param  base  Conversion base, 0 for autodetect
     \return       Converted value on success, IntegerT set as null on failure, primitive is 0 on failure
    */
    //[tags: self, num ]
    template<class T> T getnum(int base=0) const {
        assert( data_ > (char*)1 || size_ == 0 );
        return StaticIf< IsPodType<T>::value, impl::ToNumPod<T>, impl::ToNum<T> >::Type::getnum(data_, size_, base);
    }

    /** Convert to floating point number value for given type. Fails on bad format or no digits.

    Format: [WHITESPACE] ("nan" | NUMBER) [WHITESPACE]\n
     - NUMBER: ["+"|"-"] ("inf" | DIGITS ["." DIGITS] [EXPONENT])\n
     - EXPONENT: ["+"|"-"] ("e"|"E") DIGITS
     - Only decimal (base 10) digits supported
     - Not-A-Number ("nan") and infinite ("inf") values are case insensitive
     .
    Error codes:
     - ENone: success
     - EInval: bad format, invalid character, or no digits
     .
     \tparam  T  Basic floating point type to convert to -- must be primitive like: float, double, ldouble
     \param  error  Stores conversion error code, ENone on success [out]
     \return        Converted value, partially converted value or 0 on failure
    */
    //[tags: self, numerr_float, numerr ]
    template<class T> T getnumf(Error& error) const {
        STATIC_ASSERT( IsPodType<T>::value, getnumf_POD_Type_Required );
        assert( data_ > (char*)1 || size_ == 0 );
        return impl::tonumf<T>(data_, size_, error);
    }

    /** Convert to floating point number value for given type. Fails on bad format or no digits.

    Format: [WHITESPACE] ("nan" | NUMBER) [WHITESPACE]\n
     - NUMBER: ["+"|"-"] ("inf" | DIGITS ["." DIGITS] [EXPONENT])\n
     - EXPONENT: ["+"|"-"] ("e"|"E") DIGITS
     - Only decimal (base 10) digits supported
     - Not-A-Number ("nan") and infinite ("inf") values are case insensitive
     .
     \tparam  T  Floating point type to convert to -- can be FloatT type like Float, FloatD, etc or primitive like float, double, etc
     \return  Converted value on success, FloatT set as null on failure, primitive is 0 on failure
    */
    //[tags: self, numerr_float, numerr ]
    template<class T> T getnumf() const {
        assert( data_ > (char*)1 || size_ == 0 );
        return StaticIf< IsPodType<T>::value, impl::ToNumfPod<T>, impl::ToNumf<T> >::Type::getnum(data_, size_);
    }

    // BOOLVAL

    /** Convert to bool value. Fails on bad format, empty, or number overflow.

    Format: [WHITESPACE] ["on"|"off"|"yes"|"no"|"true"|"false"|"t"|"f"|DIGITS] [WHITESPACE]
     - Result is true if non-zero number -- checked using getnum(Error&,int) with T=long, base=0
     - Case insensitive
     .
     \return  Converted value, set to null on failure
    */
    //[tags: self, bool ]
    Bool boolval() const
        { return impl::ToBool<Bool>::getbool(data_, size_); }

    // NUM

    /** Convert to number value (signed). Fails on bad format, no digits, or overflow.

    Format: [WHITESPACE] ["+"|"-"] ["0x"|"0X"|"x"|"0"] DIGITS [WHITESPACE]
     - Base 2-36 supported -- Base 0 autodetects hex or octal based on prefix, defaults to decimal
     - Base 16 (hex) may use "0x", "0X", or "x" prefix
     - Base 8 (octal) may use "0" prefix
     - Letters in digits are not case sensitive
     .
     \param  base  Conversion base, 0 for autodetect
     \return       Converted value, set as null on failure
    */
    //[tags: self, num_signed, num ]
    Int num(int base=0) const
        { return impl::ToNum<Int>::getnum(data_, size_, base); }

    /** Convert to number value (signed long). Fails on bad format, no digits, or overflow.

    Format: [WHITESPACE] ["+"|"-"] ["0x"|"0X"|"x"|"0"] DIGITS [WHITESPACE]
     - Base 2-36 supported -- Base 0 autodetects hex or octal based on prefix, defaults to decimal
     - Base 16 (hex) may use "0x", "0X", or "x" prefix
     - Base 8 (octal) may use "0" prefix
     - Letters in digits are not case sensitive
     .
     \param  base  Conversion base, 0 for autodetect
     \return       Converted value, set as null on failure
    */
    //[tags: self, num_signed, num ]
    Long numl(int base=0) const
        { return impl::ToNum<Long>::getnum(data_, size_, base); }

    /** Convert to number value (signed long-long). Fails on bad format, no digits, or overflow.

    Format: [WHITESPACE] ["+"|"-"] ["0x"|"0X"|"x"|"0"] DIGITS [WHITESPACE]
     - Base 2-36 supported -- Base 0 autodetects hex or octal based on prefix, defaults to decimal
     - Base 16 (hex) may use "0x", "0X", or "x" prefix
     - Base 8 (octal) may use "0" prefix
     - Letters in digits are not case sensitive
     .
     \param  base  Conversion base, 0 for autodetect
     \return       Converted value, set as null on failure
    */
    //[tags: self, num_signed, num ]
    LongL numll(int base=0) const
        { return impl::ToNum<LongL>::getnum(data_, size_, base); }

    // NUMU

    /** Convert to number value (unsigned). Fails on bad format, no digits, or overflow.

    Format: [WHITESPACE] ["+"|"-"] ["0x"|"0X"|"x"|"0"] DIGITS [WHITESPACE]
     - Base 2-36 supported -- Base 0 autodetects hex or octal based on prefix, defaults to decimal
     - Base 16 (hex) may use "0x", "0X", or "x" prefix
     - Base 8 (octal) may use "0" prefix
     - Letters in digits are not case sensitive
     - Negative values are converted to unsigned equivalent
     .
     \param  base  Conversion base, 0 for autodetect
     \return       Converted value, set as null on failure
    */
    //[tags: self, num_unsigned, num ]
    UInt numu(int base=0) const
        { return impl::ToNum<UInt>::getnum(data_, size_, base); }

    /** Convert to number value (unsigned long). Fails on bad format, no digits, or overflow.

    Format: [WHITESPACE] ["+"|"-"] ["0x"|"0X"|"x"|"0"] DIGITS [WHITESPACE]
     - Base 2-36 supported -- Base 0 autodetects hex or octal based on prefix, defaults to decimal
     - Base 16 (hex) may use "0x", "0X", or "x" prefix
     - Base 8 (octal) may use "0" prefix
     - Letters in digits are not case sensitive
     - Negative values are converted to unsigned equivalent
     .
     \param  base  Conversion base, 0 for autodetect
     \return       Converted value, set as null on failure
    */
    //[tags: self, num_unsigned, num ]
    ULong numul(int base=0) const
        { return impl::ToNum<ULong>::getnum(data_, size_, base); }

    /** Convert to number value (unsigned long-long). Fails on bad format, no digits, or overflow.

    Format: [WHITESPACE] ["+"|"-"] ["0x"|"0X"|"x"|"0"] DIGITS [WHITESPACE]
     - Base 2-36 supported -- Base 0 autodetects hex or octal based on prefix, defaults to decimal
     - Base 16 (hex) may use "0x", "0X", or "x" prefix
     - Base 8 (octal) may use "0" prefix
     - Letters in digits are not case sensitive
     - Negative values are converted to unsigned equivalent
     .
     \param  base  Conversion base, 0 for autodetect
     \return       Converted value, set as null on failure
    */
    //[tags: self, num_unsigned, num ]
    ulongl numull(int base=0) const
        { return impl::ToNum<ULongL>::getnum(data_, size_, base); }

    // NUMF

    /** Convert to number value (floating point). Fails on bad format or no digits.

    Format: [WHITESPACE] ("nan" | NUMBER) [WHITESPACE]\n
     - NUMBER: ["+"|"-"] ("inf" | DIGITS ["." DIGITS] [EXPONENT])\n
     - EXPONENT: ["+"|"-"] ("e"|"E") DIGITS
     - Only decimal (base 10) digits supported
     - Not-A-Number ("nan") and infinite ("inf") values are case insensitive
     .
     \return  Converted value, set as null on failure
    */
    //[tags: self, num_float, num ]
    Float numf() const
        { return impl::ToNumf<Float>::getnum(data_, size_); }

    /** Convert to number value (double floating point). Fails on bad format or no digits.

    Format: [WHITESPACE] ("nan" | NUMBER) [WHITESPACE]\n
     - NUMBER: ["+"|"-"] ("inf" | DIGITS ["." DIGITS] [EXPONENT])\n
     - EXPONENT: ["+"|"-"] ("e"|"E") DIGITS
     - Only decimal (base 10) digits supported
     - Not-A-Number ("nan") and infinite ("inf") values are case insensitive
     .
     \return  Converted value, set as null on failure
    */
    //[tags: self, num_float, num ]
    FloatD numfd() const
        { return impl::ToNumf<FloatD>::getnum(data_, size_); }

    /** Convert to number value (ldouble floating point). Fails on bad format or no digits.

    Format: [WHITESPACE] ("nan" | NUMBER) [WHITESPACE]\n
     - NUMBER: ["+"|"-"] ("inf" | DIGITS ["." DIGITS] [EXPONENT])\n
     - EXPONENT: ["+"|"-"] ("e"|"E") DIGITS
     - Only decimal (base 10) digits supported
     - Not-A-Number ("nan") and infinite ("inf") values are case insensitive
     .
     \return  Converted value, set as null on failure
    */
    //[tags: self, num_float, num ]
    FloatL numfl() const
        { return impl::ToNumf<FloatL>::getnum(data_, size_); }

    // CONVERT

    /** Convert string to value of given type.
     - Advanced: Custom conversions can be defined by specializing a template instance of Convert -- see \ref StringCustomConversion "Custom String Conversion/Formatting"
     \return  Converted value
     */
    //[tags: convert ]
    template<class C> C convert() const
        // An "undefined reference" compiler error pointing here means the given conversion isn't implemented/supported
        { return Convert<String,C>::value(*this); }

    /** Convert value to string, replacing current string.
     - Advanced: Custom conversions can be defined by specializing a template instance of Convert -- see \ref StringCustomConversion "Custom String Conversion/Formatting"
     \param  value  Value to convert
    */
    //[tags: convert ]
    template<class C> String& convert_set(C value)
        // An "undefined reference" compiler error pointing here means the given conversion isn't implemented/supported
        { Convert<String,C>::set(*this, value); return *this; }

    /** Convert value to string, appending to current string.
     - Advanced: Custom conversions can be defined by specializing a template instance of Convert -- see \ref StringCustomConversion "Custom String Conversion/Formatting"
     \param  value  Value to convert
    */
    //[tags: convert ]
    template<class C> String& convert_add(C value)
        // An "undefined reference" compiler error pointing here means the given conversion isn't implemented/supported
        { Convert<String,C>::add(*this, value); return *this; }

    /** Convert value to string with quoting as needed, appending to current string.
     - Advanced: Custom conversions can be defined by specializing a template instance of Convert -- see \ref StringCustomConversion "Custom String Conversion/Formatting"
     \param  value  Value to convert
     \param  delim  Delimiter to use
    */
    //[tags: convert ]
    template<class C> String& convert_addq(C value, char delim)
        // An "undefined reference" compiler error pointing here means the given conversion isn't implemented/supported
        { Convert<String,C>::addq(*this, value, delim); return *this; }

    /** Join list items into delimited string.
     - This adds each item to string with delimiters, effectively using convert_add() for conversion
     - Delimiter is added before each item except the first
     - List item type must be convertible to String via convert_add()
     .
     \param  items  List items to join
     \param  delim  Delimiter to use
    */
    //[tags: convert, split_list ]
    template<class C> String& join(const C& items, char delim=',') {
        typename C::Iter iter(items);
        for (typename C::Size i=0; iter; ++iter, ++i) {
            if (i > 0)
                add(delim);
            Convert<String,typename C::Item>::add(*this, *iter);
        }
        return *this;
    }

    /** Join list items into delimited string, with quoting as needed.
     - This adds each item to string with delimiters, effectively using convert_addq() for conversion
     - Delimiter is added before each item except the first
     - List item type must be convertible to String via convert_addq()
     - This analyzes string values to determine the type of quoting needed (if any) so can be slightly slower than join()
     .
     \param  items  List items to join
     \param  delim  Delimiter to use
    */
    //[tags: convert, split_list ]
    template<class C> String& joinq(const C& items, char delim=',') {
        typename C::Iter iter(items);
        for (typename C::Size i=0; iter; ++iter, ++i) {
            if (i > 0)
                add(delim);
            Convert<String,typename C::Item>::addq(*this, *iter, delim);
        }
        return *this;
    }

    /** Join map items into delimited string.
     - This adds each item to string with delimiters, using convert_add() for conversion
     - Item delimiter is added before each item except the first, Key/Value delimiter is added between keys and values
     - Map item key and types must be convertible to String via convert_add()
     .
     \param  map      Map to join
     \param  delim    Item delimiter to use
     \param  kvdelim  Key/Value delimiter to use
    */
    //[tags: convert, split_list ]
    template<class C> String& joinmap(const C& map, char delim=',', char kvdelim='=') {
        typename C::Iter iter(map);
        for (typename C::Size i=0; iter; ++iter, ++i) {
            if (i > 0)
                add(delim);
            Convert<String,typename C::Key>::add(*this, iter->key);
            add(kvdelim);
            Convert<String,typename C::Value>::add(*this, iter->value);
        }
        return *this;
    }

    /** Join map items into delimited string, with quoting as needed.
     - This adds each item to string with delimiters, using convert_addq() for conversion --
       key and value are converted separately, and either may be quoted
     - Item delimiter is added before each item except the first, Key/Value delimiter is added between keys and values
     - Map item key and types must be convertible to String via convert_add()
     .
     \param  map      Map to join
     \param  delim    Item delimiter to use
     \param  kvdelim  Key/Value delimiter to use
    */
    //[tags: convert, split_list ]
    template<class C> String& joinmapq(const C& map, char delim=',', char kvdelim='=') {
        typename C::Iter iter(map);
        for (typename C::Size i=0; iter; ++iter, ++i) {
            if (i > 0)
                add(delim);
            Convert<String,typename C::Key>::addq(*this, iter->key, kvdelim);
            add(kvdelim);
            Convert<String,typename C::Value>::addq(*this, iter->value, delim);
        }
        return *this;
    }

    // TODO: test with Set
    /** Split delimited string into item list using given tokenizer.
     - This tokenizes and adds each item to list, using convert() for conversion to list item type
     - String must be convertible to list item type via convert()
     - See join() to join list back into string
     .
     \tparam  Tok  Tokenizer to use, such as StrTok
     \tparam  C    List container for items -- inferred from items parameter
     \param  items  List to add items to [in/out]
     \param  delim  Delimiter to use
    */
    template<class Tok,class C> typename C::Size split(C& items, char delim=',') const {
        typename C::Size count = 0;
        Tok tok(*this);
        for (; tok.next(delim); ++count)
            items.add(tok.value().template convert<typename C::Item>());
        return count;
    }

    // TODO: quoted fields, splitmapq()?
    /** Split delimited string into map key/value items.
     - This parses/tokenizes str and adds each item to map, using convert() for conversion to map key and value types
     - Map key and value types must be convertible from String via convert()
     - See joinmap() to join map back into string
     .
     \tparam  C       List container for items -- inferred from map parameter
     \param  map      List to add items to [in/out]
     \param  delim    Item delimiter to use
     \param  kvdelim  Key/Value delimiter to use
     \return          Number of items added to map
    */
    template<class C> typename C::Size splitmap(C& map, char delim=',', char kvdelim='=') const
        { return map.addsplit(*this, delim, kvdelim); }

    // OVERRIDES

    //[props:doxyparent] {
    Size capacity()
        { return ListType::capacity(); }
    String& capacity(Size size)
        { ListType::capacity(size); return *this; }
    String& capacitymin(Size min)
        { ListType::capacitymin(min); return *this; }
    String& capacitymax(Size max)
        { ListType::capacitymax(max); return *this; }
    String& reserve(Size size)
        { ListType::reserve(size); return *this; }
    String& unslice()
        { ListType::unslice(); return *this; }
    String& resize(Size size)
        { ListType::resize(size); return *this; }
    String& unshare()
        { ListType::unshare(); return *this; }
    String& remove(Key index, Size size=1)
        { ListType::remove(index, size); return*this; }
    String& reverse()
        { ListType::reverse(); return *this; }
    String& clear()
        { ListType::clear(); return *this; }
    String& slice(Key index)
        { ListType::slice(index); return *this; }
    String& slice(Key index, Size size)
        { ListType::slice(index, size); return *this; }
    String& slice2(Key index1, Key index2)
        { ListType::slice2(index1, index2); return *this; }
    String& triml(Size size)
        { ListType::triml(size); return *this; }
    String& trimr(Size size)
        { ListType::trimr(size); return *this; }
    String& truncate(Size size)
        { ListType::truncate(size); return *this; }
    String& advResize(Size size)
        { ListType::advResize(size); return *this; }
    //[] }

    // STATIC

    /** Get string of all alphanumeric digits (0-9, A-Z).
     \return  %String of digits
    */
    static const String& digits()
        { static const String str("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"); return str; }

    /** Get string of letters (A-z).
     \return  %String of letters
    */
    static const String& letters()
        { static const String str("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"); return str; }

    /** Get string of uppercase letters (A-Z).
     \return  %String of letters
    */
    static const String& lettersu()
        { static const String str("ABCDEFGHIJKLMNOPQRSTUVWXYZ"); return str; }

    /** Get string of lowercase letters (a-z).
     \return  %String of letters
    */
    static const String& lettersl()
        { static const String str("abcdefghijklmnopqrstuvwxyz"); return str; }

    /** Get string of whitespace characters (space, tab).
     \return  %String of whitespace
    */
    static const String& whitespace()
        { static const String str(" \t"); return str; }

    // Doxygen: Inherited methods
    #ifdef DOXYGEN
    //[props:doxyparent] {
    bool operator==(const ListType& data) const;
    bool operator==(const ListBaseType& data) const;
    bool operator!=(const ListType& data) const;
    bool operator!=(const ListBaseType& data) const;
    bool contains(const char* data,Size size) const;
    bool starts(const char* items,Size size) const;
    bool ends(const char* items,Size size) const;
    Size capacity() const;
    bool null() const;
    bool empty() const;
    Size size() const;
    bool shared() const;
    const char& operator[](Key index) const;
    const char& item(Key index) const;
    const char* first() const;
    const char* last() const;
    Key iend(Size offset) const;
    ulong hash(ulong seed) const;
    int compare(const ListType& data) const;
    int compare(const ListBaseType& data) const;
    Key findany(const char* items,Size count,Key start,Key end) const;
    Key findanyr(const char* items,Size count,Key start,Key end) const;
    template<class T1,class T2> bool splitat(Key index,T1& left,T2& right) const;
    template<class T1>          bool splitat(Key index,T1& left) const;
    template<class T2>          bool splitat(Key index,ValNull left,T2& right) const;
    bool splitat_setl(Key index);
    template<class T2> bool splitat_setl(Key index,T2& right);
    bool splitat_setr(Key index);
    template<class T1> bool splitat_setr(Key index,T1& left);
    char* dataM();
    char& operator()(Key index);
    char& itemM(Key index);
    bool pop(char& item,Key index);
    bool pop(char& item);
    const char* pop();
    bool popq(char& item);
    const char* popq();
    void move(Key dest,Key index);
    Size move(Key dest,ListType& src,Key srcindex,Size size);
    void swap(Key index1,Key index2);
    void swap(ListType& list);
    char* advBuffer(Size size);
    char& advItem(Key index);
    void advSwap(Key index1,Key index2);
    //[] }
    #endif

protected:
    // Doxygen: Inherited methods
    #ifdef DOXYGEN
    //[props:doxyparent] {
    void ref(const ListType& data);
    void ref(const char* data,Size size,bool term);
    //[] }
    #endif

private:
    // Hide unused parent members
    ListType& fill(const char& item, Key index=0, Size size=ALL);
    String& addnew(Size);
    String& prependnew(Size);
    Size insertnew(Key, Size);
    void advAdd(Size);
    void advPrepend(Size);
    void advInsert(Key, Size);
    void advRemove(Key, Size);

    // Doxygen: Hide unused overidden parent members from documentation
    #ifdef DOXYGEN
    bool contains(ItemVal) const;
    bool starts(ItemVal) const;
    bool ends(ItemVal) const;
    ListType& operator<<(const char&);
    ListType& add(const char&);
    ListType& prepend(const char&);
    Size insert(Key,const char&);

    Size used() const;
    #if EVO_LIST_OPT_REFTERM
    bool terminated() const;
    #endif
    Size refs() const;
    const char* buffer() const;
    void iterInitMutable();
    const char* iterFirst(IterKey& key) const;
    const char* iterNext(IterKey& key) const;
    const char* iterNext(Size count,IterKey& key) const;
    const char* iterLast(IterKey& key) const;
    const char* iterPrev(IterKey& key) const;
    const char* iterPrev(Size count,IterKey& key) const;
    Size iterCount() const;
    const char* iterSet(IterKey key) const;
    #endif

    // Set number
    template<class T>
    void setnum(T num, int base=10) {
        clear().capacitymin(IntegerT<T>::maxlen(base)+1).unshare();
        const Size len = impl::fnum(data_+IntegerT<T>::digits(num,base), num, base);
        buf_.header->used += len;
        size_             += len;
    }
    template<class T>
    void setnumu(T num, int base=10) {
        clear().capacitymin(IntegerT<T>::maxlen(base)+1).unshare();
        const Size len = impl::fnumu(data_+IntegerT<T>::digits(num,base), num, base);
        buf_.header->used += len;
        size_             += len;
    }
    template<class T>
    void setnumf(T num, int precision=PREC_AUTO) {
        int exp = 0;
        Size len;
        if (precision < 0) {
            num = FloatT<T>::fexp10(exp, num);
            clear().capacitymin(FloatT<T>::maxdigits_auto).unshare();
            len = impl::fnumfe(data_, num, exp, false);
        } else {
            num = FloatT<T>::fexp10(exp, impl::fnumf_weight(num, precision));
            clear().capacitymin(FloatT<T>::maxdigits_prec(exp, precision)).unshare();
            len = impl::fnumf(data_, num, exp, precision);
        }
        buf_.header->used += len;
        size_             += len;
    }

    // Append number
    template<class T>
    void addnum(T num, int base=10) {
        reserve(IntegerT<T>::maxlen(base)+1);
        const Size len = impl::fnum(data_+size_+IntegerT<T>::digits(num,base), num, base);
        buf_.header->used += len;
        size_             += len;
    }
    template<class T>
    void addnumu(T num, int base=10) {
        reserve(IntegerT<T>::maxlen(base)+1);
        const Size len = impl::fnumu(data_+size_+IntegerT<T>::digits(num,base), num, base);
        buf_.header->used += len;
        size_             += len;
    }
    template<class T>
    void addnumf(T num, int precision=PREC_AUTO) {
        int exp = 0;
        Size len;
        if (precision < 0) {
            num = FloatT<T>::fexp10(exp, num);
            reserve(FloatT<T>::maxdigits_auto);
            len = impl::fnumfe(data_+size_, num, exp, false);
        } else {
            num = FloatT<T>::fexp10(exp, impl::fnumf_weight(num, precision));
            reserve(FloatT<T>::maxdigits_prec(exp, precision));
            len = impl::fnumf(data_+size_, num, exp, precision);
        }
        buf_.header->used += len;
        size_             += len;
    }

    // Prepend number
    template<class T>
    void prependnum(T num, int base=10) {
        // TODO - temp buffer
        String str;
        str.setn(num, base);
        prepend(str);
    }
    template<class T>
    void prependnumf(T num, int precision=PREC_AUTO) {
        // TODO - temp buffer??
        String str;
        str.setnumf(num, precision);
        prepend(str);
    }

    // Insert number
    template<class T>
    void insertnum(Key index, T num, int base=10) {
        // TODO - temp buffer
        String str;
        str.setn(num, base);
        insert(index, str);
    }
    template<class T>
    void insertnumf(Key index, T num, int precision=PREC_AUTO) {
        // TODO - temp buffer??
        String str;
        str.setnumf(num, precision);
        insert(index, str);
    }
};

// String comparison
/** \cond impl */
inline bool operator==(const char* str1, const String& str2)
    { return str2 == str1; }
inline bool operator!=(const char* str1, const String& str2)
    { return str2 != str1; }
/** \endcond */

// String conversion
/** \cond impl */
// Common base types for string conversion -- used internally
template<class T> struct Convert_StringToIntBase {
    template<class V> static void set(String& dest, V value)
        { dest.setn(value); }
    template<class V> static void add(String& dest, V value)
        { dest.addn(value); }
    template<class V> static void addq(String& dest, V value, char)
        { dest.addn(value); }
    static T value(const String& src)
        { return src.getnum<T>(); }
};
template<class T> struct Convert_StringToFltBase {
    template<class V> static void set(String& dest, V value)
        { dest.setn(value); }
    template<class V> static void add(String& dest, V value)
        { dest.addn(value); }
    template<class V> static void addq(String& dest, V value, char)
        { dest.addn(value); }
    static T value(const String& src)
        { return src.getnumf<T>(); }
};
template<class T> struct Convert_StringToCIntBase {
    template<class V> static void set(String& dest, const V& value)
        { if (value.valid()) dest.setn(*value); }
    template<class V> static void add(String& dest, const V& value)
        { if (value.valid()) dest.addn(*value); }
    template<class V> static void addq(String& dest, const V& value, char)
        { if (value.valid()) dest.addn(*value); }
    static T value(const String& src)
        { return src.getnum<T>(); }
};
template<class T> struct Convert_StringToCFltBase {
    template<class V> static void set(String& dest, const V& value)
        { if (value.valid()) dest.setn(*value); }
    template<class V> static void add(String& dest, const V& value)
        { if (value.valid()) dest.addn(*value); }
    template<class V> static void addq(String& dest, const V& value, char)
        { if (value.valid()) dest.addn(*value); }
    static T value(const String& src)
        { return src.getnumf<T>(); }
};
// Conversion templates
template<> struct Convert<const char*,String> {
    // TODO?
    static void set(String& dest, const char* value)
        { dest.set(value); }
    static void add(String& dest, const char* value)
        { dest.add(value); }
    static String value(const char* src)
        { return src; }
};
template<> struct Convert<char*,String> {
    // TODO?
    static void set(String& dest, const char* value)
        { dest.set(value); }
    static void add(String& dest, const char* value)
        { dest.add(value); }
    static String value(const char* src)
        { return src; }
};
template<> struct Convert<String,String> {
    static void set(String& dest, const String& value)
        { dest.set(value); }
    static void add(String& dest, const String& value)
        { dest.add(value); }
    static void addq(String& dest, const String& value, char delim)
        { StrQuoting::addq(dest, value, delim); }
    static const String& value(const String& src)
        { return src; }
};
template<> struct Convert<String,const char*> {
    static void set(String& dest, const char* value)
        { dest.set(value); }
    static void add(String& dest, const char* value)
        { dest.add(value); }
    static void addq(String& dest, const char* value, char delim)
        { StrQuoting::addq(dest, String::ListBaseType(value, strlen(value)), delim); }
    // Unsafe: Converting String to const char*
    //template<class U> static const char* value(U&)
};
template<> struct Convert<String,char*> {
    static void set(String& dest, const char* value)
        { dest.set(value); }
    static void add(String& dest, const char* value)
        { dest.add(value); }
    static void addq(String& dest, const char* value, char delim)
        { StrQuoting::addq(dest, String::ListBaseType(value), delim); }
    // Unsafe: Converting String to char*
    //template<class U> static const char* value(U&)
};
template<> struct Convert<String,bool> {
    static void set(String& dest, bool value) {
        if (value) dest.set("true", 4);
        else       dest.set("false", 5);
    }
    static void add(String& dest, bool value) {
        if (value) dest.add("true", 4);
        else       dest.add("false", 5);
    }
    static void addq(String& dest, bool value, char) {
        if (value) dest.add("true", 4);
        else       dest.add("false", 5);
    }
    static bool value(const String& src)
        { return src.getbool<bool>(); }
};
template<> struct Convert<String,short>   : public Convert_StringToIntBase<short>   { };
template<> struct Convert<String,int>     : public Convert_StringToIntBase<int>     { };
template<> struct Convert<String,long>    : public Convert_StringToIntBase<long>    { };
template<> struct Convert<String,longl>   : public Convert_StringToIntBase<longl>   { };
template<> struct Convert<String,ushort>  : public Convert_StringToIntBase<ushort>  { };
template<> struct Convert<String,uint>    : public Convert_StringToIntBase<uint>    { };
template<> struct Convert<String,ulong>   : public Convert_StringToIntBase<ulong>   { };
template<> struct Convert<String,ulongl>  : public Convert_StringToIntBase<ulongl>  { };
template<> struct Convert<String,float>   : public Convert_StringToFltBase<float>   { };
template<> struct Convert<String,double>  : public Convert_StringToFltBase<double>  { };
template<> struct Convert<String,ldouble> : public Convert_StringToFltBase<ldouble> { };
template<> struct Convert<String,Bool> {
    static void set(String& dest, Bool value) {
        if (value.null()) dest.set();
        else if (*value)  dest.set("true", 4);
        else              dest.set("false", 5);
    }
    static void add(String& dest, Bool value) {
        if (value.valid()) {
            if (*value) dest.add("true", 4);
            else        dest.add("false", 5);
        }
    }
    static void addq(String& dest, Bool value, char) {
        if (value.valid()) {
            if (*value) dest.add("true", 4);
            else        dest.add("false", 5);
        }
    }
    static Bool value(const String& src)
        { return src.getbool<Bool>(); }
};
template<> struct Convert<String,Short>  : public Convert_StringToCIntBase<Short>     { };
template<> struct Convert<String,Int>    : public Convert_StringToCIntBase<Int>     { };
template<> struct Convert<String,Long>   : public Convert_StringToCIntBase<Long>     { };
template<> struct Convert<String,LongL>  : public Convert_StringToCIntBase<LongL>     { };
template<> struct Convert<String,UShort> : public Convert_StringToCIntBase<UShort>     { };
template<> struct Convert<String,UInt>   : public Convert_StringToCIntBase<UInt>    { };
template<> struct Convert<String,ULong>  : public Convert_StringToCIntBase<ULong>   { };
template<> struct Convert<String,ULongL> : public Convert_StringToCIntBase<ULongL>  { };
template<> struct Convert<String,Float>  : public Convert_StringToCFltBase<Float>   { };
template<> struct Convert<String,FloatD> : public Convert_StringToCFltBase<FloatD>  { };
template<> struct Convert<String,FloatL> : public Convert_StringToCFltBase<FloatL>     { };

/** \endcond */

///////////////////////////////////////////////////////////////////////////////

/** Base string typedef.
 - Use to explicitly cast to a base string type
   - Use on char* or STL string (if STL compatiblity enabled)
   - Not needed with string literals like: "test"
   .
 - This can solve compiler errors creating a temporary when "too many implicit conversions" are available\n
   Example:
   \code
   std::string stdstr("test");
   evo::String str(StringBase(stdstr));
   \endcode
 - See also: SubString
 .
*/
typedef String::ListBaseType StringBase;

///////////////////////////////////////////////////////////////////////////////
//@}
} // Namespace: evo
#endif
