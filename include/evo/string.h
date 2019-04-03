// Evo C++ Library
/* Copyright 2019 Justin Crowell
Distributed under the BSD 2-Clause License -- see included file LICENSE.txt for details.
*/
///////////////////////////////////////////////////////////////////////////////
/** \file string.h Evo String container. */
#pragma once
#ifndef INCL_evo_string_h
#define INCL_evo_string_h

#include "list.h"
#include "strscan.h"

namespace evo {
/** \addtogroup EvoContainers */
//@{

///////////////////////////////////////////////////////////////////////////////

/** %String fixed-size buffer for formatting an integer.
 - This is used for efficiently formatting a number to a string buffer, without allocating memory
 - Useful for setting up a terminated string pointer to pass to a low-level function
 .
 \tparam  T        Integer type
 \tparam  PADDING  Additional padding for buffer, defaults to 1 for terminator

\par Example

This example uses `printf()` as a function that requires a terminated string.

\code
#include <evo/string.h>
#include <stdio.h>
using namespace evo;

int main() {
    StringInt<int> str1(12345);
    printf("%s\n", str1.data());

    StringInt<int> str2;
    printf("%s\n", str2.set(6677));

    return 0;
}
\endcode

Output:
\code{.unparsed}
12345
6677
\endcode
*/
template<class T, int PADDING=1>
struct StringInt : public ListBase<char,StrSizeT> {
    typedef StringInt<T,PADDING> ThisType;                          ///< This type
    typedef StrSizeT             Size;                              ///< Size type

    static const int BUF_SIZE = IntegerT<T>::MAXSTRLEN + PADDING;   ///< Buffer size

    char buffer[BUF_SIZE];      ///< %String buffer

    using ListBase<char,StrSizeT>::data_;
    using ListBase<char,StrSizeT>::size_;

    /** Constructor intializes to null. */
    StringInt()
        { }

    /** Constructor intializes with set().
     - Formatting is done in reverse (least digit first) so additional padding after terminator is normally before the returned pointer
     .
     \param  num          Number to set/format
     \param  base         Number base to format with (2 - 36)
     \param  terminated   Whether to add terminator after number
     \param  end_padding  Padding to leave at end (`PADDING` template param must make room for this)
    */
    StringInt(T num, int base=fDEC, bool terminated=true, uint end_padding=0)
        { set(num, base, terminated, end_padding); }

    /** Get formatted string pointer.
     - This is null until set(T,int,bool,uint) is called
     .
     \return  %String pointer, NULL if null
    */
    char* data()
        { return data_; }

    /** Get formatting string size.
     - Padding is not included in size
     .
     \return  %string size in bytes, 0 if null
    */
    Size size() const
        { return size_; }

    /** %Set as null.
     \return  This
    */
    ThisType& set() {
        data_ = NULL;
        size_ = 0;
        return *this;
    }

    /** %Set as formatted integer.
     - Note that formatting is done in reverse (least digit first) so any extra space is at the beginning of the buffer
     .
     \param  num          Number to set/format
     \param  base         Number base to format with (2 - 36)
     \param  terminated   Whether to add terminator after number
     \param  end_padding  Padding in buffer to leave after string value (`PADDING` template param must make room for this)
     \return              Pointer to formatted string in buffer
    */
    char* set(T num, int base=fDEC, bool terminated=true, uint end_padding=0) {
        assert( base >= 2 );
        assert( base <= 36 );
        assert( end_padding <= BUF_SIZE - IntegerT<T>::MAXSTRLEN ); // Must have room for formatting and padding
        char* endptr = buffer + BUF_SIZE - end_padding;
        if (terminated) {
            assert( BUF_SIZE > IntegerT<T>::MAXSTRLEN );    // Terminator requires padding > 0
            *--endptr = '\0';
        }
        if (IsSigned<T>::value)
            size_ = impl::fnum(endptr, num, base);
        else
            size_ = impl::fnumu(endptr, num, base);
        data_ = endptr - size_;
        return data_;
    }

    /** Append additional data to buffer.
     - Results are undefined if null -- call StringInt(T,int,bool,uint) or set(T,int,bool,uint) first
     - This assumes there's enough buffer padding via `PADDING` template param, and via `end_padding` argument when set
     .
     \param  data  Data pointer to copy from
     \param  size  %Size to copy and add
     \return       This
    */
    ThisType& add(const char* data, Size size) {
        assert( data != NULL );
        assert( data_ != NULL );
        memcpy(data_ + size_, data, size);
        size_ += size;
        return *this;
    }

    /** Add additional space to buffer and return pointer to new uninitialized space in buffer.
     - This increases the data size used and returns a pointer to the newly added data to write to, which should be considered uninitialized
     - Results are undefined if null -- call StringInt(T,int,bool,uint) or set(T,int,bool,uint) first
     - This assumes there's enough buffer padding via `PADDING` template param, and via `end_padding` argument when set
     .
     \param  size  %Size to add
     \return       Pointer to newly added data (uninitialized)
    */
    char* addnew(Size size) {
        assert( data_ != NULL );
        char* p = data_ + size_;
        size_ += size;
        return p;
    }

private:
    // Disable copying
    StringInt(StringInt&);
    StringInt& operator=(StringInt&);
};

///////////////////////////////////////////////////////////////////////////////

/** %String fixed-size buffer for formatting a floating point number.
 - This is used for efficiently formatting a floating point number to a string buffer, without allocating memory
 - Useful for setting up a terminated string pointer to pass to a low-level function
 .
 \tparam  T        Floating point type
 \tparam  PADDING  Additional padding for buffer, defaults to 1 for terminator

\par Example

This example uses `printf()` as a function that requires a terminated string.

\code
#include <evo/string.h>
#include <stdio.h>
using namespace evo;

int main() {
    StringFlt<double> str1(1.23);
    printf("%s\n", str1.data());

    StringFlt<double> str2;
    printf("%s\n", str2.set(66.77));

    return 0;
}
\endcode

Output:
\code{.unparsed}
1.23
66.77
\endcode
*/
template<class T=double, int PADDING=1>
struct StringFlt : public ListBase<char,StrSizeT> {
    typedef StringFlt<T,PADDING> ThisType;                              ///< This type
    typedef StrSizeT             Size;                                  ///< Size type

    static const int BUF_SIZE = FloatT<T>::MAXDIGITS_AUTO + PADDING;    ///< Buffer size

    char buffer[BUF_SIZE];      ///< %String buffer

    using ListBase<char,StrSizeT>::data_;
    using ListBase<char,StrSizeT>::size_;

    /** Advanced: Special structure used to avoid automatically allocating memory when a bigger buffer is needed.
     - This is used to optimize a special case: After set(T,int,bool,NumInfo*) returns NULL, call format() and provide a large enough buffer to finish formatting
    */
    struct NumInfo {
        Size size;          ///< Buffer size required to format number (including PADDING), modified by format() to formatted size (excluding PADDING)
        T    number;        ///< Normalized floating point number to format
        int  exponent;      ///< Exponent for normalized floating point number to format
        int  precision;     ///< Precision to format
        bool terminated;    ///< Whether to add terminator

        /** Constructor. */
        NumInfo()
            { memset(this, 0, sizeof(NumInfo)); }

        /** Format number using given buffer.
         - This sets the `size` member to the formatted size (excluding terminator and PADDING)
         .
         \param  ptr  Buffer pointer to format to, must have enough space, must not be NULL
         \return      Formatted data pointer (`ptr`)
        */
        char* format(char* ptr) {
            const ulong len = impl::fnumf(ptr, number, exponent, precision);
            if (terminated)
                ptr[len] = '\0';
            assert( len < IntegerT<Size>::MAX );
            size = (Size)len;
            return ptr;
        }
    };

    /** Constructor intializes to null. */
    StringFlt() : dbuffer_(NULL), dbuffer_size_(0)
        { }

    /** Constructor intializes with set().
     - For special cases where stack buffer isn't big enough, this allocates the needed memory internally (which is freed by destructor)
     .
     \param  num          Number to set/format
     \param  precision    Formatting precision (number of fractional digits), 0 for none, fPREC_AUTO for automatic
     \param  terminated   Whether to add terminator after number
    */
    StringFlt(T num, int precision=fPREC_AUTO, bool terminated=true) : dbuffer_(NULL), dbuffer_size_(0)
        { set(num, precision, terminated); }

    /** Destructor. */
    ~StringFlt()
        { free(); }

    /** Get formatted string pointer.
     - This is null until set(T,int,bool,uint) is called
     .
     \return  %String pointer, NULL if null
    */
    char* data()
        { return data_; }

    /** Get formatting string size.
     - Padding is not included in size
     .
     \return  %string size in bytes, 0 if null
    */
    Size size() const
        { return size_; }

    /** %Set as null.
     \return  This
    */
    ThisType& set() {
        data_ = NULL;
        size_ = 0;
        return *this;
    }

    /** %Set floating point number and format to string.
     - For optimizing a special case where more memory is needed:
       - 1. Pass `info=NULL` (the default): this allocates the needed memory internally (which is freed by destructor) and formatting always succeeds
       - 2. Advanced: Pass an object pointer for `info`: this populates the `info` object and returns NULL, call info->format() to finish formatting with your own buffer
         - This allows optimization with your own buffer to minimize memory allocs
       - These cases don't apply when `precision=fPREC_AUTO` as the stack buffer is always big enough for automatic precision formatting (which uses scientific notation if needed)
     .
     \param  num           Number to set/format
     \param  precision     Formatting precision (number of fractional digits), 0 for none, \ref fPREC_AUTO for automatic
     \param  info          Used when more memory is needed: NULL to alloc internally and free in destructor, otherwise this is populated and NULL is returned
     \param  terminated    Whether to add terminator after number
     \param  info          NULL by default, otherwise a non-NULL value enables optimization for a special case (see above)
     \return               Pointer to formatted string in buffer, NULL if `info != NULL` and a buffer alloc is needed (see above)
    */
    char* set(T num, int precision=fPREC_AUTO, bool terminated=true, NumInfo* info=NULL) {
        ulong len;
        int exp = 0;
        if (precision < 0) {
            // fPREC_AUTO
            data_ = buffer;
            num = FloatT<T>::fexp10(exp, num);
            len = impl::fnumfe(data_, num, exp, false);
            assert( len <= (ulong)BUF_SIZE );
        } else {
            // Explicit precision may require allocating memory
            num = FloatT<T>::fexp10(exp, impl::fnumf_weight(num, precision));
            ulong maxsize = (ulong)FloatT<T>::maxdigits_prec(exp, precision) + PADDING;
            assert( maxsize > 0 );
            assert( maxsize < IntegerT<Size>::MAX );
            if (maxsize > (ulong)BUF_SIZE) {
                if (maxsize > dbuffer_size_) {
                    if (info != NULL) {
                        // No alloc, populate info to defer formatting
                        info->size       = maxsize;
                        info->number     = num;
                        info->exponent   = exp;
                        info->precision  = precision;
                        info->terminated = terminated;
                        return NULL;
                    }
                    free();
                    assert( maxsize < IntegerT<size_t>::MAX );
                    dbuffer_      = (char*)::malloc((size_t)maxsize);
                    dbuffer_size_ = (Size)maxsize;
                }
                data_ = dbuffer_;
            } else
                data_ = buffer;
            len = impl::fnumf(data_, num, exp, precision);
        }
        if (terminated) {
            assert( PADDING > 0 );    // Terminator requires padding > 0
            data_[len] = '\0';
        }
        assert( len < IntegerT<Size>::MAX );
        size_ = (Size)len;
        return data_;
    }

private:
    // Disable copying
    StringFlt(StringFlt&);
    StringFlt& operator=(StringFlt&);

    // Dynamic buffer
    char* dbuffer_;
    Size dbuffer_size_;

    void free() {
        if (dbuffer_ != NULL)
            ::free(dbuffer_);
    }
};

///////////////////////////////////////////////////////////////////////////////

/** %String container.

\par Features

 - Similar to STL `std::string`
 - Built-in formatting support -- see operator<<()
 - Built-in conversions -- see num(), numu(), numf(), boolval()
 - Searching and splitting with split() and find() methods
 - Not always terminated, call cstr() for terminated string
 .
 - Preallocates extra memory when buffer grows -- see capacity(), resize(), capacity(Size)
 - No memory allocated by new empty string
 - Inherits efficiency from List
 .
 - \ref Sharing "Sharing" and \ref Slicing "Slicing" make for simple and efficient string parsing
 - \b Caution: Setting from a raw pointer will use \ref UnsafePtrRef "Unsafe Pointer Referencing"
 - See also: SubString, StrTok, UnicodeString
 .

C++11:
 - Range-based for loop -- see \ref StlCompatibility
   \code
    String str;
    for (auto ch : str.asconst()) {
    }
   \endcode
 - Initialization lists
   \code
    String str = {'a', 'b', 'c'};
   \endcode
 - Move semantics
 - UTF-16 string literal, see UnicodeString
   \code
    String str = u"Hello"; // converts from UTF-16 literal to UTF-8
   \endcode

\par Iterators

 - String::Iter -- Read-Only Iterator (IteratorRa)
 - String::IterM -- Mutable Iterator (IteratorRa)

\par Constructors

 - String()
 - String(const String&), ...
 - String(const StringBase&,Key,Key)
 - String(const char*,Size)
 - String(const PtrBase<char>&,Size)
 - String(const char*)
 - String(const PtrBase<char>&)
 - String(const char16_t*,Size,UtfMode) [C++11]
 - String(const char16_t*,UtfMode) [C++11]
 - String(const std::initializer_list<char>&) [C++11]
 - String(String&&) [C++11]
 .

\par Read Access

 - asconst()
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
 .
 - split(C&,char) const
 - split(char,T1&,T2&) const
   - split(char,T1&) const
   - split(char,ValNull,T2&) const
 - splitr(char,T1&,T2&) const
   - splitr(char,T1&) const
   - splitr(char,ValNull,T2&) const
 - \ref List::splitat(Key,T1&,T2&) const       "splitat(Key,T1&,T2&) const"
   - \ref List::splitat(Key,T1&) const         "splitat(Key,T1&) const"
   - \ref List::splitat(Key,ValNull,T2&) const "splitat(Key,ValNull,T2&) const"
 - cbegin(), cend()
   - begin() const, end() const
 - find(char,Key,Key) const, ...
   - find(const char*,uint,Key,Key) const, ...
   - find(const StringBase&,Key,Key) const, ...
 - findr(char,Key,Key) const, ...
   - findr(const char*,uint,Key,Key) const, ...
   - findr(const StringBase&,Key,Key) const, ...
 - findany(), findanyr()
   - findanybut(), findanybutr()
 - findword(), findwordr()
   - findnonword(), findnonwordr()
 - contains(char) const
   - contains(const char*,Size) const
   - contains(const StringBase&) const
 .
 - \ref List::compare(const ListBaseType&) const "compare(const StringBase&) const", ...
   - operator==(const String&) const, ...
   - operator==(const char*) const
   - operator!=(const String& data) const, ...
   - operator!=(const char*) const
 - starts(char) const
   - \ref List::starts(const Item*,Size) const "starts(const char*,Size) const"
 - ends(char) const
   - \ref List::ends(const Item*,Size) const "ends(const char*,Size) const"
 .
 - convert()
   - splitmap()
   - toupper(), tolower()
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

 - token(), tokenr()
   - token_any(), tokenr_any()
   - token_line(), tokenr_line()
 - slice(Key)
   - slice(Key,Size), slice2()
 - truncate()
   - triml(), trimr()
 - strip()
   - strip(char)
   - stripl(), stripr()
   - stripl(char,Size), stripr(char,Size)
   - stripl(const char*,Size,Size), stripr(const char*,Size,Size)
   - stripl_newlines(), stripr_newlines()
 - strip2()
   - stripl2()
   - stripr2()
 - strip_newlines()
   - stripl_newlines(), stripl_newlines(Size)
   - stripr_newlines(), stripr_newlines(Size)
 - pop(), popq()
 - unslice()
 .

\par Modifiers

 - cstr()
 - dataM()
   - itemM()
   - operator()()
 - begin(), end()
 - resize()
   - reserve()
   - capacity(Size)
   - capacitymin(), capacitymax()
   - unshare()
 - set()
   - set(const ListType&)
   - set(const StringBase&,Key,Key)
   - set(const char*,Size)
   - set(const PtrBase<char>&,Size)
   - set(const char*)
   - set(const PtrBase<char>&)
   - set2(const StringBase&,Key,Key), ...
   - setn(int,int), setn(uint,int), setn(float,int), ...
   - set_unicode(const wchar16*,Size,UtfMode)
     - set_unicode(const wchar16*,UtfMode)
     - set_unicode(const char16_t*,Size,UtfMode) [C++11]
     - set_unicode(const char16_t*,UtfMode) [C++11]
   - setempty()
   - clear()
   - operator=(const String&), ...
   - operator=(const StringBase&)
   - operator=(String&&) [C++11]
   - operator=(const char16_t*) [C++11]
   - operator=(const char*)
   - operator=(const PtrBase<char>&)
   - operator=(const ValNull&)
   - operator=(const ValEmpty&)
 - copy(const ListType&)
   - copy(const StringBase&)
   - copy(const char*,Size)
   - copy(const char*)
 - convert_set()
   - convert_add()
   - join()
 - add(char)
   - add(char,Size)
   - add(const ListType&), ...
   - add(const char*,Size)
   - add(const char*)
   - addsep()
   - addn(int,int), addn(uint,int), addn(float,int), ...
   - operator<<(bool)
   - operator<<(char)
   - operator<<(const ListType&), ...
   - operator<<(const char*)
   - operator<<(const ValNull&)
   - operator<<(const ValEmpty&)
   - operator<<(const FmtChar&), operator<<(const FmtString&), operator<<(const FmtStringWrap&)
   - operator<<(int), operator<<(uint), operator<<(float), ...
   - operator<<(const FmtInt&), operator<<(const FmtUInt&), operator<<(const FmtFloat&), ...
 - prepend(char)
   - prepend(char,Size)
   - prepend(const ListType&), ...
   - prepend(const char*,Size)
   - prepend(const char*)
   - prependsep()
   - prependn(int,int), prependn(uint,int), prependn(float,int), ...
 - insert(Key,char)
   - insert(Key,char,Size)
   - insert(Key,const ListType&), ...
   - insert(Key,const char*,Size)
   - insert(Key,const char*)
   - insertn(Key,int,int), insertn(Key,uint,int), insertn(Key,float,int), ...
 - replace(Key,Size,const StringBase&)
   - replace(Key,Size,const char*,Size)
   - replace(Key,Size,const char*)
   - fillch()
 - findreplace(char,const char*,Size,Size)
   - findreplace(char,const StringBase&,Size)
 - findreplace(const char*,Size,const char*,Size,Size)
   - findreplace(const char*,Size,const StringBase&,Size)
   - findreplace(const StringBase&,const StringBase&,Size)
   - findreplace(const char*,Size,const StringBase&,Size)
 - remove()
 - pop(char&)
   - \ref List::pop(T&,Key) "pop(char&,Key)"
   - popq(char&)
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

 - \ref List::advItem(Key) "advItem(Key)"
 - advResize()
 - \ref List::advBuffer(Size) "advBuffer(Size)"
   - advBuffer()
   - advSize()
 - advWrite()
   - advWriteDone()
 - advEdit()
   - advEditDone()
 - advSwap()
 .

\par Example

\code
#include <evo/string.h>
#include <evo/io.h>
using namespace evo;
static Console& c = con();

int main() {
    // Create string
    String str("test");

    // operator[] provides read-only (const) access
    char value = str[0];
    //str[0] = 0;       // Error: operator[] is read-only

    // operator() provides write (mutable) access
    str(0) = 'T';

    // Iterate and print characters (read-only)
    for (String::Iter iter(str); iter; ++iter)
        c.out << "Ch: " << *iter << NL;

    // Reformat string
    str.clear() << "foo" << ',' << 123;

    // Split string into 2 substrings
    String sub1, sub2;
    str.split(',', sub1, sub2);

    // Print sub1, and sub2 as number (dereference Int to int)
    c.out << "1: " << sub1 << NL
          << "2: " << *sub2.num() << NL;

    // Print sub1 as terminated string (just for example)
    c.out << sub1.cstr() << NL;

    return 0;
}
\endcode

Output:
\code{.unparsed}
Ch: T
Ch: e
Ch: s
Ch: t
1: foo
2: 123
foo
\endcode
*/
class String : public List<char,StrSizeT> {
public:
    typedef String                        ThisType;     ///< This string type
    typedef List<char,StrSizeT>           ListType;     ///< List type
    typedef String                        Out;          ///< Type returned by write_out()

    typedef ListBaseType           StringBase;          ///< Alias for ListBaseType
    typedef ListBase<wchar16,Size> UnicodeStringBase;   ///< Base for UnicodeString

    /** Default constructor sets as null. */
    String()
        { }

    /** Copy constructor.
     - Makes shared copy if possible -- see \ref Sharing "Sharing"
     .
     \param  str  %String to copy
    */
    String(const String& str) : List<char,Size>(str)
        { }

    /** Copy constructor.
     - Makes shared copy if possible -- see \ref Sharing "Sharing"
     .
     \param  str  %String to copy
    */
    String(const ListType& str) : List<char,Size>(str)
        { }

    /** Copy constructor.
     - Makes shared copy if possible -- see \ref Sharing "Sharing"
     .
     \param  str  %String pointer to copy from, ignored if null
    */
    String(const ListType* str)
        { if (str != NULL) set(*str); }

    /** Extended copy constructor.
     - Makes shared copy if possible -- see \ref Sharing "Sharing"
     .
     \param  str    %String to copy
     \param  index  Start index of string to copy, END to set as empty
     \param  size   Size as character count, ALL for all from index
    */
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
    String(const StringBase& str, Key index=0, Key size=ALL) : List<char,Size>(str, index, size)
        { }

    /** Constructor for string pointer.
     - \b Caution: Uses \ref UnsafePtrRef "Unsafe Pointer Referencing"
     - See String(const PtrBase<char>&,Size) for best safety, use \ref Ptr "Ptr<char>" to wrap raw pointer
     .
     \param  str   %String pointer to use
     \param  size  %String size as character count
    */
    String(const char* str, Size size) : List<char,Size>(str, size)
        { }

    /** Constructor for managed string pointer.
     - This always makes an unshared copy
     - For best performance (and less safety) reference substring instead with String(const char*,Size)
     - Use \ref Ptr "Ptr<char>" to wrap raw pointer
     .
     \param  str   %String pointer, calls set() if null
     \param  size  %String size as character count
    */
    String(const PtrBase<char>& str, Size size) {
        if (str.ptr_ != NULL) {
            if (size > 0)
                copy(str.ptr_, size);
            else
                setempty();
        }
    }

    /** Constructor to convert from UTF-16 string to UTF-8 string.
     - This calls set_unicode()
     .
     \param  str  UTF-16 string to convert from
     \return      This
    */
    String(const UnicodeStringBase& str)
        { set_unicode(str.data_, str.size_); }

    /** Constructor for null terminated string.
     - \b Caution: Uses \ref UnsafePtrRef "Unsafe Pointer Referencing"
     - See String(const PtrBase<char>&) for best safety, use \ref Ptr "Ptr<char>" to wrap raw pointer
     .
     \param  str  %String pointer, calls set() if null -- must be terminated
    */
    String(const char* str) : List<char,Size>(str, str?(Size)strlen(str):0) {
        #if EVO_LIST_OPT_REFTERM
            terminated_ = true;
        #endif
    }

    /** Constructor to copy null terminated string from managed pointer.
     - This always makes an unshared copy
     - For best performance (and less safety) reference substring instead with String(const char*)
     - Use \ref Ptr "Ptr<char>" to wrap raw pointer
     .
     \param  str  %String pointer, calls set() if null -- must be terminated
    */
    String(const PtrBase<char>& str) {
        if (str.ptr_ != NULL) {
            const size_t len = strlen(str.ptr_);
            assert( len < IntegerT<Size>::MAX );
            copy(str.ptr_, (Size)len);
        }
    }

#if defined(EVO_CPP11)
    /** Constructor to initialize and convert from UTF-16 string literal (using `u` prefix).
     - This calls set_unicode(const char16_t*,Size,UtfMode)
     .
     \param  str   %String literal, NULL to set as null
     \param  size  %String size in UTF-16 chars
     \param  mode  How to handle invalid UTF-16 values, see set_unicode()
    */
    String(const char16_t* str, Size size, UtfMode mode=umREPLACE_INVALID) {
        set_unicode(str, size, mode);
    }

    /** Constructor to initialize and convert from terminated UTF-16 string literal (using `u` prefix).
     - This calls set_unicode(const char16_t*,UtfMode)
     .
     \param  str   UTF-16 string, NULL to set as null, otherwise must be terminated
     \param  mode  How to handle invalid UTF-16 values, see set_unicode()
    */
    String(const char16_t* str, UtfMode mode=umREPLACE_INVALID) {
        set_unicode(str, mode);
    }

    /** Sequence constructor (C++11).
     \param  init  Initializer list, passed as comma-separated values in braces `{ }`
    */
    String(const std::initializer_list<char>& init) {
        assert( init.size() < IntegerT<Size>::MAX );
        ListType::advAdd((Size)init.size());
        char* p = data_;
        for (auto ch : init)
            *(p++) = ch;
    }

    /** Move constructor (C++11).
     \param  src  Source to move
    */
    String(String&& src) : List<char>(std::move(src)) {
    }

    /** Move assignment operator (C++11).
     \param  src  Source to move
     \return      This
    */
    String& operator=(String&& src) {
        List<char>::operator=(std::move(src));
        return *this;
    }

    /** Assignment operator to set and convert from terminated UTF-16 string.
     - This calls set_unicode(const char16_t*,UtfMode)
     .
     \param  str  UTF-16 string, NULL to set as null
     \return      This
    */
    String& operator=(const char16_t* str) {
        set_unicode(str);
        return *this;
    }
#endif

    /** \copydoc List::asconst() */
    const String& asconst() const {
        return *this;
    }

    // SET

    /** Assignment operator.
     - Makes shared copy if possible -- see \ref Sharing "Sharing"
     .
     \param  str  %String to copy
     \return      This
    */
    String& operator=(const String& str)
        { set(str); return *this; }

    /** Assignment operator.
     - Makes shared copy if possible -- see \ref Sharing "Sharing"
     .
     \param  str  %String to copy
     \return      This
    */
    String& operator=(const ListType& str)
        { set(str); return *this; }

    /** Assignment operator for pointer.
     - Makes shared copy if possible -- see \ref Sharing "Sharing"
     .
     \param  str  %String pointer to copy from, calls set() if null
     \return      This
    */
    String& operator=(const ListType* str) {
        if (str != NULL)
            set(*str);
        else
            set();
        return *this;
    }

    /** Assignment operator to copy from base list type.
     - This always makes an unshared copy
     - For best performance (and less safety) reference substring instead with set(const char*,Size)
     .
     \param  data  Data to copy
     \return       This
    */
    String& operator=(const StringBase& data)
        { ListType::operator=(data); return *this; }

    /** Assignment operator for null terminated string.
     - \b Caution: Uses \ref UnsafePtrRef "Unsafe Pointer Referencing"
     - See operator=(const PtrBase<char>&) for best safety, use \ref Ptr "Ptr<char>" to wrap raw pointer
     .
     \param  str  %String pointer, calls set() if null -- must be terminated
     \return      This
    */
    String& operator=(const char* str) {
        if (str == NULL)
            set();
        else
            ref(str, (Size)strlen(str), true);
        return *this;
    }

    /** Assignment operator to copy from managed pointer with null terminated string.
     - This always makes an unshared copy
     - For best performance (and less safety) reference substring instead with operator=(const char*)
     - Use \ref Ptr "Ptr<char>" to wrap raw pointer
     .
     \param  str  %String pointer, calls set() if null -- must be terminated
     \return      This
    */
    String& operator=(const PtrBase<char>& str) {
        if (str.ptr_ == NULL)
            set();
        else
            copy(str.ptr_, (Size)strlen(str.ptr_));
        return *this;
    }

    /** Assignment operator to convert from UTF-16 string to UTF-8 string.
     - This calls set_unicode()
     .
     \param  str  UTF-16 string to convert from
     \return      This
    */
    template<class T>
    String& operator=(const ListBase<wchar16,T>& str) {
        assert( str.size_ < IntegerT<Size>::MAX );
        set_unicode(str.data_, str.size_);
        return *this;
    }

    /** Assignment operator to set as null and empty.
     - Same as set()
     .
    Example:
    \code
    str = vNULL;
    \endcode
     \return  This
    */
    String& operator=(const ValNull&)
        { set(); return *this; }

    /** Assignment operator to set as empty but not null.
     - Same as setempty()
     .
    Example:
    \code
    str = vEMPTY;
    \endcode
     \return  This
    */
    String& operator=(const ValEmpty&)
        { setempty(); return *this; }

    /** %Set as empty but not null.
     - Append operators can be chained\n
       Example:
       \code
        // Set as empty then append two characters
        str.setempty() << 'a' << 'b';
       \endcode
     .
     \return  This
    */
    String& setempty()
        { ListType::setempty(); return *this; }

    /** %Set as null and empty.
     - Append operators can be chained\n
       Example:
       \code
        // Clear string and append two characters
        str.set() << 'a' << 'b';
       \endcode
     .
     \return  This
    */
    String& set()
        { ListType::set(); return *this; }

    /** %Set from another string.
     - Makes shared copy if possible -- see \ref Sharing "Sharing"
     .
     \param  str  %String to copy
     \return      This
    */
    String& set(const ListType& str)
        { ListType::set(str); return *this; }

    /** %Set from substring of another string.
     - Makes shared copy if possible -- see \ref Sharing "Sharing"
     .
     \param  str    %String to copy
     \param  index  Start index of data, END to set as empty
     \param  size   Data size as item count, ALL for all from index
     \return        This
    */
    String& set(const ListType& str, Key index, Key size=ALL)
        { ListType::set(str, index, size); return *this; }

    /** %Set as copy of substring.
     - This always makes an unshared copy
     - For best performance (and less safety) reference substring instead with set(const char*,Size)
     .
     \param  data   Data to copy
     \param  index  Start index of substring data, END to set as empty
     \param  size   Data size as item count, ALL for all from index
     \return        This
    */
    String& set(const StringBase& data, Key index=0, Key size=ALL)
        { ListType::set(data, index, size); return *this; }

    /** %Set from string pointer.
     - \b Caution: Uses \ref UnsafePtrRef "Unsafe Pointer Referencing"
     - For best safety use copy(const char*,Size)
     .
     \param  str   %String pointer, calls set() if null
     \param  size  %String size as character count
     \return       This
    */
    String& set(const char* str, Size size)
        { ListType::set(str, size); return *this; }

    /** %Set from managed string pointer.
     - This always makes an unshared copy
     - For best performance (and less safety) reference substring instead with set(const char*,Size)
     - Use \ref Ptr "Ptr<char>" to wrap raw pointer
     .
     \param  str   %String pointer, calls set() if null
     \param  size  %String size as character count
   */
    String& set(const PtrBase<char>& str, Size size) {
        if (str.ptr_ == NULL)
            ListType::set();
        else
            ListType::copy(str.ptr_, size);
        return *this;
    }

    /** %Set from terminated string.
     - \b Caution: Uses \ref UnsafePtrRef "Unsafe Pointer Referencing"
     - For best safety use set(const PtrBase<char>&,Size) or use \ref Ptr "Ptr<char>" to wrap raw pointer
     .
     \param  str  %String pointer, calls set() if null -- must be terminated
     \return      This
    */
    String& set(const char* str) {
        if (str == NULL)
            ListType::set();
        else
            ref(str, (Size)strlen(str), true);
        return *this;
    }

    /** %Set as copy of null terminated string from managed pointer.
     - This always makes an unshared copy
     - For best performance (and less safety) reference substring instead with set(const char*)
     - Use \ref Ptr "Ptr<char>" to wrap raw pointer
     .
     \param  str  %String pointer, calls set() if null -- must be terminated
    */
    String& set(const PtrBase<char>& str) {
        if (str.ptr_ == NULL)
            ListType::set();
        else
            ListType::copy(str.ptr_, (Size)strlen(str.ptr_));
        return *this;
    }

    /** %Set from substring of another string using start/end positions.
     - Makes shared copy if possible -- see \ref Sharing "Sharing"
     - If index2 < index1 then index2 will be set to index1 (empty sublist)
     - Use unshare() afterwards to make a full (unshared) copy
     .
     \param  str     %String to copy
     \param  index1  Start index of data, END to set as empty
     \param  index2  End index of data (this item not included), END for all after index1
     \return         This
    */
    String& set2(const ListType& str, Key index1, Key index2)
        { ListType::set2(str, index1, index2); return *this; }

    /** %Set and reference sublist using start/end positions.
     - \b Caution: This will reference the same pointer as given sublist, so pointer must remain valid
     - Use unshare() afterwards to make a full (unshared) copy
     .
     \param  data    Data to reference
     \param  index1  Start index of sublist data, END to set as empty
     \param  index2  End index of sublist data (this item not included), END for all after index1
     \return         This
    */
    String& set2(const StringBase& data, Key index1, Key index2)
        { ListType::set2(data, index1, index2); return *this; }

    /** %Set as formatted signed number (modifier).
     \param  num   Number to set
     \param  base  Base to use for formatting
     \return       This
    */
    String& setn(int num, int base=fDEC)
        { setnum(num, base); return *this; }

    /** %Set as formatted signed number (modifier).
     \param  num   Number to set
     \param  base  Base to use for formatting
     \return       This
    */
    String& setn(long num, int base=fDEC)
        { setnum(num, base); return *this; }

    /** %Set as formatted signed number (modifier).
     \param  num   Number to set
     \param  base  Base to use for formatting
     \return       This
    */
    String& setn(longl num, int base=fDEC)
        { setnum(num, base); return *this; }

    /** %Set as formatted unsigned number (modifier).
     \param  num   Number to set
     \param  base  Base to use for formatting
     \return       This
    */
    String& setn(uint num, int base=fDEC)
        { setnumu(num, base); return *this; }

    /** %Set as formatted unsigned number (modifier).
     \param  num   Number to set
     \param  base  Base to use for formatting
     \return       This
    */
    String& setn(ulong num, int base=fDEC)
        { setnumu(num, base); return *this; }

    /** %Set as formatted unsigned number (modifier).
     \param  num   Number to set
     \param  base  Base to use for formatting
     \return       This
    */
    String& setn(ulongl num, int base=fDEC)
        { setnumu(num, base); return *this; }

    /** %Set as formatted floating point number (modifier).
     \param  num        Number to set
     \param  precision  Formatting precision (number of fractional digits), 0 for none, fPREC_AUTO for automatic
     \return            This
    */
    String& setn(float num, int precision=fPREC_AUTO)
        { setnumf(num, precision); return *this; }

    /** %Set as formatted floating point number (modifier).
     \param  num        Number to set
     \param  precision  Formatting precision (number of fractional digits), 0 for none, fPREC_AUTO for automatic
     \return            This
    */
    String& setn(double num, int precision=fPREC_AUTO)
        { setnumf(num, precision); return *this; }

    /** %Set as formatted floating point number (modifier).
     \param  num        Number to set
     \param  precision  Formatting precision (number of fractional digits), 0 for none, fPREC_AUTO for automatic
     \return            This
    */
    String& setn(ldouble num, int precision=fPREC_AUTO)
        { setnumf(num, precision); return *this; }

    /** %Set as normal UTF-8 string converted from a raw UTF-16 string.
     - Windows: See also: set_win32(const WCHAR*,int)
     .
     \param  str   Pointer to UTF-16 string to convert from, NULL to set as null
     \param  size  %String size to convert from, as wchar16 length (not bytes), ignored if `str` is NULL
     \param  mode  How to handle invalid UTF-16 values:
                    - \ref umINCLUDE_INVALID - Invalid UTF-16 values are converted as-is (1 character each)
                    - \ref umREPLACE_INVALID - Invalid UTF-16 values are each replaced with the Unicode Replacement Character (code: 0xFFFD)
                    - \ref umSKIP_INVALID - Invalid UTF-16 values are skipped and ignored
                    - \ref umSTRICT - Stop on invalid input with an error
     \return       Whether successful, false if stopped on invalid input with umSTRICT
    */
    bool set_unicode(const wchar16* str, Size size, UtfMode mode=umREPLACE_INVALID) {
        if (str == NULL) {
            set();
        } else {
            clear();
            const wchar16* p = str;
            const wchar16* end = str + size;
            ulong bytes = utf16_to8(p, end, NULL, 0, mode);
            if (bytes == END)
                return false;
            if (bytes > 0) {
                assert( bytes < IntegerT<Size>::MAX );
                char* buf = advWrite(bytes + 1);
                bytes = utf16_to8(str, end, buf, bytes, mode);
                advWriteDone((Size)bytes);
            }
        }
        return true;
    }

    /** %Set as normal UTF-8 string converted from a raw terminated UTF-16 string.
     - Windows: See also: set_win32(const WCHAR*)
     .
     \param  str   Pointer to UTF-16 string to convert from, NULL to set as null, otherwise must be terminated
     \param  mode  How to handle invalid UTF-16 values:
                    - \ref umINCLUDE_INVALID - Invalid UTF-16 values are converted as-is (1 character each)
                    - \ref umREPLACE_INVALID - Invalid UTF-16 values are each replaced with the Unicode Replacement Character (code: 0xFFFD)
                    - \ref umSKIP_INVALID - Invalid UTF-16 values are skipped and ignored
                    - \ref umSTRICT - Stop on invalid input with an error
     \return       Whether successful, false if stopped on invalid input with umSTRICT
    */
    bool set_unicode(const wchar16* str, UtfMode mode=umREPLACE_INVALID) {
        return set_unicode(str, utf16_strlen(str), mode);
    }

#if defined(EVO_CPP11)
    /** \copydoc set_unicode(const wchar16*,Size,UtfMode) */
    bool set_unicode(const char16_t* str, Size size, UtfMode mode=umREPLACE_INVALID) {
        static_assert( sizeof(char16_t) == sizeof(wchar16), "ERROR: char16_t type larger than 16 bits" );
        return set_unicode((const wchar16*)str, size, mode);
    }

    /** \copydoc set_unicode(const wchar16*,UtfMode) */
    bool set_unicode(const char16_t* str, UtfMode mode=umREPLACE_INVALID) {
        static_assert( sizeof(char16_t) == sizeof(wchar16), "ERROR: char16_t type larger than 16 bits" );
        return set_unicode((const wchar16*)str, mode);
    }
#endif

#if defined(_WIN32) || defined(DOXYGEN)
    /** %Set as normal (UTF-8) string converted from a Windows UTF-16 (WCHAR) string (Windows only).
     - This uses the Win32 API to do the conversion
     - Invalid UTF-16 values are replaced with \ref UNICODE_REPLACEMENT_CHAR
     - For terminated UTF-16 strings see: set_win32(const WCHAR*)
     .
     \param  str   UTF-16 string to convert, NULL to set as null
     \param  size  %String size in wide chars, 0 for empty
     \return       This
    */
    String& set_win32(const WCHAR* str, int size) {
        if (str == NULL) {
            set();
        } else if (size <= 0) {
            setempty();
        } else {
            const int newsize = ::WideCharToMultiByte(CP_UTF8, 0, str, size, NULL, 0, NULL, NULL);
            if (newsize > 0) {
                clear();
                char* buf = advBuffer(newsize+1);
                int written = ::WideCharToMultiByte(CP_UTF8, 0, str, size, buf, newsize, NULL, NULL);
                if (written >= 0) {
                    buf[written] = '\0';
                    assert( written < IntegerT<Size>::MAX );
                    advSize(written);
                }
            } else
                set();
        }
        return *this;
    }

    /** %Set as normal (UTF-8) string converted from a terminated Windows UTF-16 (WCHAR) string (Windows only).
     - This uses the Win32 API to do the conversion, and so is only supported in Windows
     - Invalid UTF-16 values are replaced with \ref UNICODE_REPLACEMENT_CHAR
     - For non-terminated UTF-16 strings see: set_win32(const WCHAR*,int)
     .
     \param  str  UTF-16 string to convert (must be terminated), NULL to set as null
     \return      This
    */
    String& set_win32(const WCHAR* str) {
        if (str == NULL) {
            set();
        } else if (*str == 0) {
            setempty();
        } else {
            const int newsize = ::WideCharToMultiByte(CP_UTF8, 0, str, -1, NULL, 0, NULL, NULL);
            if (newsize > 0) {
                clear();
                int written = ::WideCharToMultiByte(CP_UTF8, 0, str, -1, advBuffer(newsize), newsize, NULL, NULL);
                if (written > 0)
                    --written; // chop terminator
                assert( written < IntegerT<Size>::MAX );
                advSize(written);
            } else
                set();
        }
        return *this;
    }
#endif

    /** Extract next token from string.
     - If delim is found, the token value up to that delim is extracted
     - If delim isn't found, the whole string is extracted
     - The extracted token is removed from this string, including the delim (if found)
     - See also: StrTok (and variants)
     .
     \tparam  StringT  Output string type to store line (String or SubString), inferred from first parameter
     \param  value  %Set to next token value, or null if none  [out]
     \param  delim  Delimiter to tokenize on
     \return        Whether next token was found, false if current string is empty
    */
    template<class StringT>
    bool token(StringT& value, char delim) {
        if (size_ > 0) {
            const char* ptr = (char*)memchr(data_, delim, size_);
            if (ptr != NULL) {
                Key i = (Key)(ptr - data_);
                value.set(data_, i);
                ++i;
                data_ += i;
                size_ -= i;
                return true;
            }
            value.set(data_, size_);
            clear();
            return true;
        }
        value.set();
        return false;
    }

    /** Extract next token from string in reverse (from end of string).
     - If delim is found, the token value up to that delim is extracted
     - If delim isn't found, the whole string is extracted
     - The extracted token is removed from this string, including the delim (if found)
     - See also: StrTokR (and variants)
     .
     \tparam  StringT  Output string type to store line (String or SubString), inferred from first parameter
     \param  value  %Set to next token value, or null if none  [out]
     \param  delim  Delimiter to tokenize on
     \return        Whether next token was found, false if current string is empty
    */
    template<class StringT>
    bool tokenr(StringT& value, char delim) {
        if (size_ > 0) {
        #if defined(EVO_GLIBC_MEMRCHR)
            const char* ptr = (char*)memrchr(data_, delim, size_);
            if (ptr != NULL) {
                const Key i = (Key)(ptr - data_);
                const Size len = size_ - i;
                value.set(data_ + i + 1, len - 1);
                size_ -= len;
                return true;
            }
        #else
            for (Key i=size_; i > 0; ) {
                if (data_[--i] == delim) {
                    Size len = size_ - i;
                    value.set(data_ + i + 1, len - 1);
                    size_ -= len;
                    return true;
                }
            }
        #endif
            value.set(data_, size_);
            clear();
            return true;
        }
        value.set();
        return false;
    }

    /** Extract next token from string using any of given delimiters.
     - If a delim is found, the token value up to that delim is extracted
     - If a delim isn't found, the whole string is extracted
     - The extracted token is removed from this string, including the delim (if found)
     - See also: StrTok (and variants)
     .
     \tparam  StringT  Output string type to store line (String or SubString), inferred from first parameter
     \param  value        %Set to next token value, or null if none  [out]
     \param  found_delim  %Set to delimited found, null if no delim found  [out]
     \param  delims       Delimiters to search for
     \param  count        Count of delimiters to search for, must be positive
     \return              Whether next token was found, false if current string is empty
    */
    template<class StringT>
    bool token_any(StringT& value, Char& found_delim, const char* delims, Size count) {
        assert( count > 0 );
        if (size_ > 0) {
            Size j;
            for (Key i=0; i < size_; ++i) {
                for (j=0; j < count; ++j) {
                    if (data_[i] == delims[j]) {
                        value.set(data_, i);
                        found_delim = data_[i];
                        ++i;
                        data_ += i;
                        size_ -= i;
                        return true;
                    }
                }
            }
            value.set(data_, size_);
            found_delim.set();
            clear();
            return true;
        }
        value.set();
        found_delim.set();
        return false;
    }

    /** Extract next token from string in reverse (from end of string) using any of given delimiters.
     - If a delim is found, the token value up to that delim is extracted
     - If a delim isn't found, the whole string is extracted
     - The extracted token is removed from this string, including the delim (if found)
     - See also: StrTokR (and variants)
     .
     \tparam  StringT  Output string type to store line (String or SubString), inferred from first parameter
     \param  value        %Set to next token value, or null if none  [out]
     \param  found_delim  %Set to delimited found, null if no delim found  [out]
     \param  delims       Delimiters to search for
     \param  count        Count of delimiters to search for, must be positive
     \return              Whether next token was found, false if current string is empty
    */
    template<class StringT>
    bool tokenr_any(StringT& value, Char& found_delim, const char* delims, Size count) {
        assert( count > 0 );
        if (size_ > 0) {
            Size j;
            for (Key i=size_; i > 0; ) {
                --i;
                for (j=0; j < count; ++j) {
                    if (data_[i] == delims[j]) {
                        Size len = size_ - i;
                        value.set(data_ + i + 1, len - 1);
                        found_delim = data_[i];
                        size_ -= len;
                        return true;
                    }
                }
            }
            value.set(data_, size_);
            found_delim.set();
            clear();
            return true;
        }
        value.set();
        found_delim.set();
        return false;
    }

    /** Extract next line from string.
     - The extracted line is removed from this string, along with the ending newline
     - This recognizes all the main newlines types, see \ref Newline
     .
     \tparam  StringT  Output string type to store line (String or SubString), inferred from parameter
     \param  line  %Set to extracted line (newline removed), null if no line extracted
     \return       Whether line extracted, false if this is empty
    */
    template<class StringT>
    bool token_line(StringT& line) {
        const char* NEWLINE_CHARS = "\n\r";
        Size i = findany(NEWLINE_CHARS, 2);
        if (i == END) {
            if (size_ > 0) {
                line.set(data_, size_);
                setempty();
                return true;
            }
            line.set();
            return false;
        }
        line.set(data_, i);

        if (++i < size_) {
            char ch1 = data_[i - 1];
            char ch2 = data_[i];
            if ((ch1 == '\n' && ch2 == '\r') || (ch1 == '\r' && ch2 == '\n'))
                ++i;
        }
        data_ += i;
        size_ -= i;
        return true;
    }

    /** Extract next line from string in reverse (from end of string).
     - The extracted line is removed from this string, along with the ending newline
     - This recognizes all the main newlines types, see \ref Newline
     .
     \tparam  StringT  Output string type to store line (String or SubString), inferred from parameter
     \param  line  %Set to extracted line (newline removed), null if no line extracted
     \return       Whether line extracted, false if this is empty
    */
    template<class StringT>
    bool tokenr_line(StringT& line) {
        const char* NEWLINE_CHARS = "\n\r";
        Size i = findanyr(NEWLINE_CHARS, 2);
        if (i == END) {
            if (size_ > 0) {
                line.set(data_, size_);
                setempty();
                return true;
            }
            line.set();
            return false;
        }
        line.set(data_ + i + 1, size_ - i - 1);

        if (i > 1) {
            char ch1 = data_[i - 1];
            char ch2 = data_[i];
            if ((ch1 == '\n' && ch2 == '\r') || (ch1 == '\r' && ch2 == '\n'))
                --i;
        }
        size_ = i;
        return true;
    }

    // INFO

    /** Get string pointer (const).
     - In some cases string may already be terminated but DO NOT expect this -- use cstr() to get terminated string
     - \b Caution: Calling any modifier/mutable method like unshare() after this may (will) invalidate the returned pointer
     .
     \return  %String pointer as read-only, NULL if null, may be invalid if string empty (const)
    */
    const char* data() const
        { return data_; }

    /** Get terminated string pointer, using given string buffer if needed (const).
     - This is useful when a temporary terminated string pointer is needed
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
    const char* cstr(String& buffer) const
        { return (size_ > 0 ? buffer.set(*this).cstr() : ""); }

    /** Get terminated string pointer (modifier).
     - Use cstr(String&) instead for const instance
     - This effectively calls unshare() and adds an internal null terminator, if needed
     - \b Caution: Calling any modifier method like add(char) after this may (will) invalidate the returned pointer
     .
     \return  Terminated string pointer
    */
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

    using ListType::compare;

    /** Comparison against UTF-16 string.
     \tparam  T  Inferred from argument
     \param  str  %String to compare to
     \return      Result (<0 if this is less, 0 if equal, >0 if this is greater)
    */
    template<class T>
    int compare(const ListBase<wchar16,T>& str) {
        assert( str.size_ < ULong::MAX );
        return -utf16_compare8(str.data_, str.size_, data_, size_);
    }

    using ListType::operator==;
    using ListType::operator!=;

    /** Equality operator.
     \param  str  %String to compare to
     \return      Whether equal
    */
    bool operator==(const String& str) const
        { return ListType::operator==(str); }

    /** Equality operator.
     \param  str  %String to compare to -- must be null terminated
     \return      Whether equal
    */
    bool operator==(const char* str) const {
        return (utf8_compare(data_, size_, str) == 0);
    }

    /** Equality operator to compare against UTF-16 string.
     \tparam  T  Inferred from argument
     \param  str  %String to compare to
     \return      Whether equal
    */
    template<class T>
    bool operator==(const ListBase<wchar16,T>& str) {
        assert( str.size_ < ULong::MAX );
        return (utf16_compare8(str.data_, str.size_, data_, size_) == 0);
    }

    /** Inequality operator.
     \param  str  %String to compare to
     \return      Whether inequal
    */
    bool operator!=(const String& str) const
        { return ListType::operator!=(str); }

    /** Inequality operator.
     \param  str  %String to compare to -- must be null terminated
     \return      Whether inequal
    */
    bool operator!=(const char* str) const {
        return (utf8_compare(data_, size_, str) != 0);
    }

    /** Inequality operator to compare against UTF-16 string.
     \tparam  T  Inferred from argument
     \param  str  %String to compare to
     \return      Whether inequal
    */
    template<class T>
    bool operator!=(const ListBase<wchar16,T>& str) {
        assert( str.size_ < ULong::MAX );
        return (utf16_compare8(str.data_, str.size_, data_, size_) != 0);
    }

    using ListType::starts;
    using ListType::ends;

    /** Check if this starts with given character.
     \param  ch  Character to check
     \return     Whether starts with character
    */
    bool starts(char ch) const
        { return ListType::starts(ch); }

    /** Check if this ends with given character.
     \param  ch  Character to check
     \return     Whether ends with character
    */
    bool ends(char ch) const
        { return ListType::ends(ch); }

    // FIND

    /** Get iterator at first item (const).
     - This allows compatibility with range-based for loops and other libraries, otherwise use container Iter directly
     .
     \return  Iterator at first item, or at end position if empty
     \see Iter, cend(), begin(), end()
    */
    Iter cbegin() const
        { return Iter(*this); }

    /** Get iterator at end (const).
     - This allows compatibility with range-based for loops and other libraries, otherwise use container Iter directly
     - This really just creates an empty iterator
     .
     \return  Iterator at end position
     \see Iter, cbegin(), begin(), end()
    */
    Iter cend() const
        { return Iter(); }

    /** Get iterator at first item (mutable).
     - This allows compatibility with range-based for loops and other libraries, otherwise use container Iter directly
     - cbegin() is more efficient, since this effectively calls unshare() to make chars mutable
     .
     \return  Iterator at first item, or at end position if empty
     \see IterM, end(), cbegin(), cend()
    */
    IterM begin()
        { return IterM(*this); }

    /** Get iterator at first item (const).
     - This allows compatibility with range-based for loops and other libraries, otherwise use container Iter directly
     .
     \return  Iterator at first item, or at end position if empty
     \see end() const, cbegin()
    */
    Iter begin() const
        { return Iter(*this); }

    /** Get iterator at end.
     - This allows compatibility with range-based for loops and other libraries, otherwise use container Iter directly
     - This really just creates an empty iterator
     .
     \return  Iterator at end position
     \see IterM, begin(), cbegin(), cend()
    */
    IterM end()
        { return IterM(); }

    /** Get iterator at end.
     - This allows compatibility with range-based for loops and other libraries, otherwise use container Iter directly
     - This really just creates an empty iterator
     .
     \return  Iterator at end position
     \see begin() const, cend()
    */
    Iter end() const
        { return Iter(); }

    /** Find first occurrence of character with forward search.
     \param  ch  Character to find
     \return     Found character index, NONE if not found
    */
    Key find(char ch) const {
        if (size_ > 0) {
            const char* ptr = (char*)memchr(data_, ch, size_);
            if (ptr != NULL)
                return (Key)(ptr - data_);
        }
        return NONE;
    }

    /** Find first occurrence of character with forward search.
     - Search stops before reaching end index or end of string
     - Character at end index is not checked
     .
     \param  ch     Character to find
     \param  start  Starting index for search
     \param  end    End index for search, END for end of string
     \return        Found character index, NONE if not found
    */
    Key find(char ch, Key start, Key end=END) const {
        if (start < size_) {
            if (end > size_)
                end = size_;
            if (start < end) {
                const char* ptr = (char*)memchr(data_ + start, ch, end - start);
                if (ptr != NULL)
                    return (Key)(ptr - data_);
            }
        }
        return NONE;
    }

    /** Find first occurrence of pattern string.
     - Search stops before reaching end index or end of string
     - Character at end index is not checked
     - Specify search algorithm with: find(StringSearchAlg,const char*,uint,Key,Key) const
     .
     \param  pattern       Pointer to pattern to look for, must not be NULL
     \param  pattern_size  Pattern size in bytes
     \param  start         Starting index for search
     \param  end           End index for search, END for end of string
     \return               Found pattern index, NONE if not found or if `pattern_size=0`
    */
    Key find(const char* pattern, uint pattern_size, Key start=0, Key end=END) const {
        if (start < size_ && start < end) {
            assert( pattern != NULL );
            if (end > size_)
                end = size_;
            return impl::string_search(pattern, pattern_size, data_ + start, end - start, start);
        }
        return NONE;
    }

    /** Find first occurrence of pattern string using specified algorithm.
     - Search stops before reaching end index or end of string
     - Character at end index is not checked
     .
     \param  alg           Search algorithm to use, see \ref StringSearchAlg
     \param  pattern       Pointer to pattern to look for, must not be NULL
     \param  pattern_size  Pattern size in bytes
     \param  start         Starting index for search
     \param  end           End index for search, END for end of string
     \return               Found pattern index, NONE if not found or if `pattern_size=0`
    */
    Key find(StringSearchAlg alg, const char* pattern, uint pattern_size, Key start=0, Key end=END) const {
        if (start < size_ && start < end) {
            assert( pattern != NULL );
            if (end > size_)
                end = size_;
            return impl::string_search(alg, pattern, pattern_size, data_ + start, end - start, start);
        }
        return NONE;
    }

    /** Find first occurrence of pattern string.
     - Search stops before reaching end index or end of string
     - Character at end index is not checked
     - Specify search algorithm with: find(StringSearchAlg,const StringBase&,Key,Key) const
     .
     \param  pattern  Pattern to look for
     \param  start    Starting index for search
     \param  end      End index for search, END for end of string
     \return          Found pattern index, NONE if not found or if `pattern` is empty
    */
    Key find(const StringBase& pattern, Key start=0, Key end=END) const {
        if (start < size_ && start < end) {
            if (end > size_)
                end = size_;
            return impl::string_search(pattern.data_, pattern.size_, data_ + start, end - start, start);
        }
        return NONE;
    }

    /** Find first occurrence of pattern string using specified algorithm.
     - Search stops before reaching end index or end of string
     - Character at end index is not checked
     .
     \param  alg      Search algorithm to use, see \ref StringSearchAlg
     \param  pattern  Pattern to look for
     \param  start    Starting index for search
     \param  end      End index for search, END for end of string
     \return          Found pattern index, NONE if not found or if `pattern_size=0`
    */
    Key find(StringSearchAlg alg, const StringBase& pattern, Key start=0, Key end=END) const {
        if (start < size_ && start < end) {
            if (end > size_)
                end = size_;
            return impl::string_search(alg, pattern.data_, pattern.size_, data_ + start, end - start, start);
        }
        return NONE;
    }

    /** Find last occurrence of character with reverse search.
     - This does a reverse search starting right before end index
     - Character at end index is not checked
     .
     \param  ch  Character to find
     \return     Found character index, NONE if not found
    */
    Key findr(char ch) const {
    #if !defined(EVO_NO_MEMRCHR) && defined(EVO_GLIBC_MEMRCHR)
        if (size_ > 0) {
            const char* ptr = (char*)memrchr(data_, ch, size_);
            if (ptr != NULL)
                return (Key)(ptr - data_);
        }
    #else
        const char* ptr = data_ + size_;
        while (ptr > data_)
            if (*--ptr == ch)
                return (Key)(ptr - data_);
    #endif
        return NONE;
    }

    /** Find last occurrence of character with reverse search.
     - This does a reverse search starting right before end index
     - Character at end index is not checked
     .
     \param  ch     Character to find
     \param  start  Starting index for search range -- last character checked in reverse search
     \param  end    End index for search range (reverse search starting point), END for end of string
     \return        Found character index or NONE if not found
    */
    Key findr(char ch, Key start, Key end=END) const {
    #if !defined(EVO_NO_MEMRCHR) && defined(EVO_GLIBC_MEMRCHR)
        if (start < size_) {
            if (end > size_)
                end = size_;
            if (start < end) {
                const char* ptr = (char*)memrchr(data_ + start, ch, end - start);
                if (ptr != NULL)
                    return (Key)(ptr - data_);
            }
        }
    #else
        if (end > size_)
            end = size_;
        while (end>start)
            if (data_[--end] == ch)
                return end;
    #endif
        return NONE;
    }

    /** Find last occurrence of pattern string with reverse search.
     - This does a reverse search starting right before end index
     - Character at end index is not checked
     - Specify search algorithm with: findr(StringSearchAlg,const char*,uint,Key,Key) const
     .
     \param  pattern       Pointer to pattern to look for, must not be NULL
     \param  pattern_size  Pattern size in bytes
     \param  start         Starting index for search
     \param  end           End index for search, END for end of string
     \return               Found pattern index, NONE if not found or if `pattern` is empty
    */
    Key findr(const char* pattern, uint pattern_size, Key start=0, Key end=END) const {
        if (start < size_ && start < end) {
            assert( pattern != NULL );
            if (end > size_)
                end = size_;
            return impl::string_search_reverse(pattern, pattern_size, data_ + start, end - start, start);
        }
        return NONE;
    }

    /** Find last occurrence of pattern string with reverse search.
     - This does a reverse search starting right before end index
     - Character at end index is not checked
     .
     \param  alg           Search algorithm to use, see \ref StringSearchAlg
     \param  pattern       Pointer to pattern to look for, must not be NULL
     \param  pattern_size  Pattern size in bytes
     \param  start         Starting index for search
     \param  end           End index for search, END for end of string
     \return               Found pattern index, NONE if not found or if `pattern` is empty
    */
    Key findr(StringSearchAlg alg, const char* pattern, uint pattern_size, Key start=0, Key end=END) const {
        if (start < size_ && start < end) {
            assert( pattern != NULL );
            if (end > size_)
                end = size_;
            return impl::string_search_reverse(alg, pattern, pattern_size, data_ + start, end - start, start);
        }
        return NONE;
    }

    /** Find last occurrence of pattern string with reverse search.
     - This does a reverse search starting right before end index
     - Character at end index is not checked
     - Specify search algorithm with: findr(StringSearchAlg,const StringBase&,Key,Key) const
     .
     \param  pattern  Pattern to look for
     \param  start    Starting index for search
     \param  end      End index for search, END for end of string
     \return          Found pattern index, NONE if not found or if `pattern` is empty
    */
    Key findr(const StringBase& pattern, Key start=0, Key end=END) const {
        if (start < size_ && start < end) {
            if (end > size_)
                end = size_;
            return impl::string_search_reverse(pattern.data_, pattern.size_, data_ + start, end - start, start);
        }
        return NONE;
    }

    /** Find last occurrence of pattern string with reverse search.
     - This does a reverse search starting right before end index
     - Character at end index is not checked
     .
     \param  alg      Search algorithm to use, see \ref StringSearchAlg
     \param  pattern  Pattern to look for
     \param  start    Starting index for search
     \param  end      End index for search, END for end of string
     \return          Found pattern index, NONE if not found or if `pattern` is empty
    */
    Key findr(StringSearchAlg alg, const StringBase& pattern, Key start=0, Key end=END) const {
        if (start < size_ && start < end) {
            if (end > size_)
                end = size_;
            return impl::string_search_reverse(alg, pattern.data_, pattern.size_, data_ + start, end - start, start);
        }
        return NONE;
    }

    /** Find first occurrence of any given characters with forward search.
     - This searches for any one of the given characters
     - Search stops before reaching end index or end of string
     - Character at end index is not checked
     .
     \param  chars  Characters to search for, must not contain multi-byte chars
     \param  count  Character count to search for
     \param  start  Starting index for search
     \param  end    End index for search, END for end of list
     \return        Found character index , NONE if not found or if `count=0`
    */
    Key findany(const char* chars, Size count, Key start=0, Key end=END) const {
        if (start < size_ && start < end && count > 0) {
            if (end > size_)
                end = size_;
            if (count == 1) {
                // Special case for single char
                const char* ptr = (char*)memchr(data_ + start, *chars, end - start);
                if (ptr != NULL)
                    return (Key)(ptr - data_);
            } else {
                const char* pend = data_ + end;
                const char* ptr  = data_ + start;
                for (; ptr < pend; ++ptr)
                    if (memchr(chars, *ptr, count) != NULL)
                        return (Key)(ptr - data_);
            }
        }
        return NONE;
    }

    /** Find first occurrence of any given characters with forward search.
     - This searches for any one of the given characters
     - Search stops before reaching end index or end of string
     - Character at end index is not checked
     .
     \param  chars  Characters to search for, must not contain multi-byte chars
     \param  start  Starting index for search
     \param  end    End index for search, END for end of list
     \return        Found character index, NONE if not found or if `chars` is empty
    */
    Key findany(const StringBase& chars, Key start=0, Key end=END) const
        { return findany(chars.data_, chars.size_, start, end); }

    /** Find last occurrence of any given characters with reverse search.
     - This searches for any one of the given characters
     - This does a reverse search starting right before end index
     - Character at end index is not checked
     .
     \param  chars  Characters to search for, must not contain multi-byte chars
     \param  count  Character count to search for
     \param  start  Starting index for search range -- last character checked in reverse search
     \param  end    End index for search range (reverse search starting point), END for end of list
     \return        Found character index, NONE if not found or if `count=0`
    */
    Key findanyr(const char* chars, Size count, Key start=0, Key end=END) const {
        if (count == 1)
            return findr(*chars, start, end);
        if (start < size_ && start < end && count > 0) {
            if (end > size_)
                end = size_;
            const char* pstart = data_ + start;
            const char* ptr    = data_ + end;
            while (ptr > pstart)
                if (memchr(chars, *--ptr, count) != NULL)
                    return (Key)(ptr - data_);
        }
        return NONE;
    }

    /** Find last occurrence of any given characters with reverse search.
     - This searches for any one of the given characters
     - This does a reverse search starting right before end index
     - Character at end index is not checked
     .
     \param  chars  Characters to search for, must not contain multi-byte chars
     \param  start  Starting index for search range -- last character checked in reverse search
     \param  end    End index for search range (reverse search starting point), END for end of list
     \return        Found character index, NONE if not found or if `chars` is empty
    */
    Key findanyr(const StringBase& chars, Key start=0, Key end=END) const
        { return findanyr(chars.data_, chars.size_, start, end); }

    /** Find first occurrence of any character not listed with forward search.
     - This searches for any character not in `chars`
     - Search stops before reaching end index or end of string
     - Character at end index is not checked
     .
     \param  chars  Excluded characters, must not contain multi-byte chars
     \param  count  Excluded character count
     \param  start  Starting index for search
     \param  end    End index for search, END for end of list
     \return        Found character index, NONE if not found
    */
    Key findanybut(const char* chars, Size count, Key start=0, Key end=END) const {
        if (start < size_ && start < end) {
            if (count == 0)
                return start;
            if (end > size_)
                end = size_;
            const char* pend = data_ + end;
            const char* ptr  = data_ + start;
            for (; ptr < pend; ++ptr)
                if (memchr(chars, *ptr, count) == NULL)
                    return (Key)(ptr - data_);
        }
        return NONE;
    }

    /** Find first occurrence of any character not listed with forward search.
     - This searches for any character not in `chars`
     - Search stops before reaching end index or end of string
     - Character at end index is not checked
     .
     \param  chars  Excluded characters, must not contain multi-byte chars
     \param  start  Starting index for search
     \param  end    End index for search, END for end of list
     \return        Found character index, NONE if not found
    */
    Key findanybut(const StringBase& chars, Key start=0, Key end=END) const
        { return findanybut(chars.data_, chars.size_, start, end); }

    /** Find last occurrence of any character not listed with reverse search.
     - This searches for any character not in `chars`
     - This does a reverse search starting right before end index
     - Character at end index is not checked
     .
     \param  chars  Excluded characters, must not contain multi-byte chars
     \param  count  Excluded character count
     \param  start  Starting index for search range -- last character checked in reverse search
     \param  end    End index for search range (reverse search starting point), END for end of list
     \return        Found character index, NONE if not found
    */
    Key findanybutr(const char* chars, Size count, Key start=0, Key end=END) const {
        if (start < size_ && start < end) {
            if (end > size_)
                end = size_;
            if (count == 0)
                return end - 1;
            const char* pstart = data_ + start;
            const char* ptr    = data_ + end;
            while (ptr > pstart)
                if (memchr(chars, *--ptr, count) == NULL)
                    return (Key)(ptr - data_);
        }
        return NONE;
    }

    /** Find last occurrence of any character not listed with reverse search.
     - This searches for any character not in `chars`
     - This does a reverse search starting right before end index
     - Character at end index is not checked
     .
     \param  chars  Excluded characters, must not contain multi-byte chars
     \param  start  Starting index for search range -- last character checked in reverse search
     \param  end    End index for search range (reverse search starting point), END for end of list
     \return        Found character index, NONE if not found
    */
    Key findanybutr(const StringBase& chars, Key start=0, Key end=END) const
        { return findanybutr(chars.data_, chars.size_, start, end); }

    /** Find first word character.
     - A word character is a letter, number, or underscore -- see \link cbtWORD\endlink
     .
     \param  start  Starting index for search
     \param  end    End index for search, END for end of list
     \return        Found character index, NONE if not found
    */
    Key findword(Key start=0, Key end=END) const {
        if (start < size_ && start < end) {
            if (end > size_)
                end = size_;
            for (; start < end; ++start)
                if (ascii_breaktype(data_[start]) == cbtWORD)
                    return start;
        }
        return NONE;
    }

    /** Find last word character with reverse search.
     - A word character is a letter, number, or underscore -- see \link cbtWORD\endlink
     .
     \param  start  Starting index for search range -- last character checked in reverse search
     \param  end    End index for search range (reverse search starting point), END for end of list
     \return        Found character index, NONE if not found
    */
    Key findwordr(Key start=0, Key end=END) const {
        if (start < size_ && start < end) {
            if (end > size_)
                end = size_;
            while (end > start)
                if (ascii_breaktype(data_[--end]) == cbtWORD)
                    return end;
        }
        return NONE;
    }

    /** Find first non-word word character.
     - This finds the first character that is NOT a word character
     - A word character is a letter, number, or underscore -- see \link cbtWORD\endlink
     .
     \param  start  Starting index for search
     \param  end    End index for search, END for end of list
     \return        Found character index, NONE if not found
    */
    Key findnonword(Key start=0, Key end=END) const {
        if (start < size_ && start < end) {
            if (end > size_)
                end = size_;
            for (; start < end; ++start)
                if (ascii_breaktype(data_[start]) != cbtWORD)
                    return start;
        }
        return NONE;
    }

    /** Find last non-word character with reverse search.
     - This finds the last character that is NOT a word character (with reverse search)
     - A word character is a letter, number, or underscore -- see \link cbtWORD\endlink
     .
     \param  start  Starting index for search range -- last character checked in reverse search
     \param  end    End index for search range (reverse search starting point), END for end of list
     \return        Found character index, NONE if not found
    */
    Key findnonwordr(Key start=0, Key end=END) const {
        if (start < size_ && start < end) {
            if (end > size_)
                end = size_;
            while (end > start)
                if (ascii_breaktype(data_[--end]) != cbtWORD)
                    return end;
        }
        return NONE;
    }

    /** Check whether this contains given character.
     \param  ch  Character to check for
     \return     Whether character was found
    */
    bool contains(char ch) const
        { return find(ch) != NONE; }

    /** Check whether this contains given string.
     \param  str   Pointer to string to check for
     \param  size  %String size in bytes
     \return       Whether string was found
    */
    bool contains(const char* str, Size size) const 
        { return find(str, size) != NONE; }

    /** Check whether contains given string.
     \param  str  %String to check for
     \return      Whether string was found
    */
    bool contains(const StringBase& str) const 
        { return find(str) != NONE; }

    // SPLIT

    /** Split at first occurrence of delimiter into left/right substrings.
     - Template types are automatically deduced from arguments
     - For more advanced parsing see StrTok
     .
     \tparam  T1    %String type to store left substring
     \tparam  T2    %String type to store right substring

     \param  delim  Delimiter to find
     \param  left   %Set to substring before delim, set to this if not found [out]
     \param  right  %Set to substring after delim, null if not found [out]
     \return        Whether successful, false if delim not found
    */
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
     \param  left   %Set to substring before delim, set to this if not found [out]
     \return        Whether successful, false if delim not found
    */
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
     \param  left   vNULL (ignored)
     \param  right  %Set to substring after delim, null if not found [out]
     \return        Whether successful, false if delim not found
    */
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
     \param  left   %Set to substring before delim, set to this if not found [out]
     \param  right  %Set to substring after delim, null if not found [out]
     \return        Whether successful, false if delim not found
    */
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
     \param  left   %Set to substring before delim, set to this if not found [out]
     \return        Whether successful, false if delim not found
    */
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
     \param  left   vNULL (ignored)
     \param  right  %Set to substring after delim, null if not found [out]
     \return        Whether successful, false if delim not found
    */
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

    // TRIM/STRIP

    /** Strip left (beginning) and right (ending) whitespace (spaces and tabs).
     - This non-destructively removes whitespace so data isn't modified -- see \ref Slicing
     - This doesn't strip newlines -- see strip2()
     .
     \return  This
    */
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
     - This non-destructively removes characters so data isn't modified -- see \ref Slicing
     .
     \param  ch  Character to strip
     \return     This
    */
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
     - This non-destructively removes whitespace so data isn't modified -- see \ref Slicing
     - This doesn't strip newlines -- see stripl2()
     .
     \return  This
    */
    String& stripl() {
        char ch;
        Size count = 0;
        while ( count < size_ && ((ch=data_[count]) == ' ' || ch == '\t') )
            ++count;
        triml(count);
        return *this;
    }

    /** Strip left (beginning) occurrences of character.
     - This non-destructively removes characters so data isn't modified -- see \ref Slicing
     .
     \param  ch   Character to strip
     \param  max  Max count to strip, ALL for all
     \return      This
    */
    String& stripl(char ch, Size max=ALL) {
        Size count = 0;
        while (count < size_ && data_[count] == ch && count < max)
            ++count;
        triml(count);
        return *this;
    }

    /** Strip left (beginning) occurrences of string.
     - This non-destructively removes characters so data isn't modified -- see \ref Slicing
     .
     \param  str      Pointer to string to strip
     \param  strsize  %String length to strip
     \param  max      Max number of occurences to strip, ALL for all
     \return          This
    */
    String& stripl(const char* str, Size strsize, Size max=ALL) {
        if (strsize > 0 && strsize <= size_ && max > 0) {
            Size i, j = 0, count = 0;
            do {
                for (i=0; i < strsize; ++i, ++j)
                    if (j >= size_ || str[i] != data_[j])
                        goto break_all;
                ++count;
            } while (j < size_ && count < max);
        break_all:
            if (count > 0) {
                count *= strsize;
                size_ -= count;
                data_ += count;
            }
        }
        return *this;
    }

    /** Strip right (ending) whitespace (spaces and tabs).
     - This non-destructively removes whitespace so data isn't modified -- see \ref Slicing
     - This doesn't strip newlines -- see stripr2()
     .
     \return  This
    */
    String& stripr() {
        char ch;
        while ( size_ > 0 && ((ch=data_[size_-1]) == ' ' || ch == '\t') )
            --size_;
        return *this;
    }

    /** Strip right (ending) occurences of character.
     - This non-destructively removes characters so data isn't modified -- see \ref Slicing
     .
     \param  ch   Character to strip
     \param  max  Max count to strip, ALL for all
     \return      This
    */
    String& stripr(char ch, Size max=ALL) {
        for (Size i=0; size_ > 0 && data_[size_-1] == ch && i < max; ++i)
            --size_;
        return *this;
    }

    /** Strip right (ending) occurences of string.
     - This non-destructively removes characters so data isn't modified -- see \ref Slicing
     .
     \param  str      Pointer to string to strip
     \param  strsize  %String length to strip
     \param  max      Max number of occurences to strip, ALL for all
     \return          This
    */
    String& stripr(const char* str, Size strsize, Size max=ALL) {
        if (strsize > 0 && strsize <= size_ && max > 0) {
            Size i, j = size_, count = 0;
            do {
                for (i=strsize; i > 0; )
                    if (j == 0 || str[--i] != data_[--j])
                        goto break_all;
                ++count;
            } while (j > 0 && count < max);
        break_all:
            if (count > 0)
                size_ -= (count * strsize);
        }
        return *this;
    }

    /** Strip left (beginning) and right (ending) whitespace from string, including newlines.
     - This non-destructively removes characters so data isn't modified -- see \ref Slicing
     - This is CPU-optimized where possible -- see \ref CppCompilers
     - See also strip()
     .
     \return  This
    */
    String& strip2() {
        if (size_ > 0) {
            const char* end = data_ + size_;
            data_ = (char*)str_scan_nws(data_, end);
            size_ = (Size)(str_scan_nws_r(data_, end) - data_);
        }
        return *this;
    }

    /** Strip left (beginning) whitespace from string, including newlines.
     - This non-destructively removes characters so data isn't modified -- see \ref Slicing
     - This is CPU-optimized where possible -- see \ref CppCompilers
     - See also stripl()
     .
     \return  This
    */
    String& stripl2() {
        if (size_ > 0) {
            const char* end = data_ + size_;
            data_ = (char*)str_scan_nws(data_, end);
            size_ = (Size)(end - data_);
        }
        return *this;
    }

    /** Strip right (ending) whitespace (including newlines) from string.
     - This non-destructively removes characters so data isn't modified -- see \ref Slicing
     - This is CPU-optimized where possible -- see \ref CppCompilers
     - See also stripr()
     .
     \return  This
    */
    String& stripr2() {
        if (size_ > 0) {
            const char* end = data_ + size_;
            size_ = (Size)(str_scan_nws_r(data_, end) - data_);
        }
        return *this;
    }

    /** Strip all left (beginning) newlines from string.
     - This non-destructively removes characters so data isn't modified -- see \ref Slicing
     - This removes all beginning newline chars (CR and LF) so this effectively recognizes all the main newlines types, see \ref Newline
     .
     \return  This
    */
    String& stripl_newlines() {
        char ch;
        Size count = 0;
        while ( count < size_ && ((ch=data_[count]) == '\n' || ch == '\r') )
            ++count;
        if (count > 0) {
            size_ -= count;
            data_ += count;
        }
        return *this;
    }

    /** Strip left (beginning) newlines from string.
     - This non-destructively removes characters so data isn't modified -- see \ref Slicing
     - This recognizes all the main newlines types, see \ref Newline
     - Note that a single newline may be 2 characters, so a `max` of 1 may remove 2 characters, and so on
     .
     \param  max  Max number of newlines to strip, ALL for all
     \return      This
    */
    String& stripl_newlines(Size max) {
        char ch;
        Size count = 0;
        for (; count < size_ && max > 0; --max) {
            ch = data_[count];
            if (ch == '\n') {
                if (count + 1 < size_ && data_[count + 1] == '\r') {
                    count += 2;
                    continue;
                }
            } else if (ch == '\r') {
                if (count + 1 < size_ && data_[count + 1] == '\n') {
                    count += 2;
                    continue;
                }
            } else
                break;
            ++count;
        }
        triml(count);
        return *this;
    }

    /** Strip all right (ending) newlines from string.
     - This non-destructively removes characters so data isn't modified -- see \ref Slicing
     - This removes all ending newline chars (CR and LF) so this effectively recognizes all the main newlines types, see \ref Newline
     .
     \return  This
    */
    String& stripr_newlines() {
        char ch;
        while ( size_ > 0 && ((ch=data_[size_-1]) == '\n' || ch == '\r') )
            --size_;
        return *this;
    }

    /** Strip right (ending) newlines from string.
     - This non-destructively removes characters so data isn't modified -- see \ref Slicing
     - This recognizes all the main newlines types, see \ref Newline
     .
     \param  max  Max number of newlines to strip, ALL for all
     \return      This
    */
    String& stripr_newlines(Size max) {
        char ch;
        for (; size_ > 0 && max > 0; --max) {
            ch = data_[size_ - 1];
            if (ch == '\n') {
                if (size_ > 1 && data_[size_ - 2] == '\r') {
                    size_ -= 2;
                    continue;
                }
            } else if (ch == '\r') {
                if (size_ > 1 && data_[size_ - 2] == '\n') {
                    size_ -= 2;
                    continue;
                }
            } else
                break;
            --size_;
        }
        return *this;
    }

    /** Strip left (beginning) and right (ending) newlines from string.
     - This non-destructively removes characters so data isn't modified -- see \ref Slicing
     - This removes all beginning and ending newline chars (CR and LF) so this effectively recognizes all the main newlines types, see \ref Newline
     .
     \return  This
    */
    String& strip_newlines() {
        stripl_newlines();
        stripr_newlines();
        return *this;
    }

    // COPY

    /** %Set as full (unshared) copy of another string (modifier).
     \param  str  %String to copy
     \return      This
    */
    String& copy(const ListType& str)
        { ListType::copy(str); return *this; }

    /** %Set as full (unshared) copy of substring (modifier).
     \param  str  %String to copy
     \return      This
    */
    String& copy(const StringBase& str)
        { ListType::copy(str); return *this; }

    /** %Set as full (unshared) copy using string pointer (modifier).
     \param  str   %String to copy
     \param  size  %String size as character count
     \return       This
    */
    String& copy(const char* str, Size size)
        { ListType::copy(str, size); return *this; }

    /** %Set as full (unshared) copy of null terminated string (modifier).
     \param  str  %String to copy -- must be terminated
     \return      This
    */
    String& copy(const char* str) {
        if (str == NULL)
            ListType::set();
        else
            ListType::copy(str, (Size)strlen(str));
        return *this;
    }

    // ADD

    /** Append character (modifier).
     \param  ch  Character to append
     \return     This
    */
    String& add(char ch)
        { ListType::add(ch); return *this; }

    /** Append copies of the same character (modifier).
     \param  ch     Character to append
     \param  count  Character copies to append
     \return        This
    */
    String& add(char ch, Size count) {
        ListType::advAdd(count);
        if (count > 0)
            memset(data_ + size_ - count, (int)(uchar)ch, count);
        return *this;
    }

    /** Append from another string (modifier).
     - For best performance use set(const String&) when this is empty
     .
     \param  str  %String to append
     \return      This
    */
    String& add(const ListType& str)
        { ListType::add(str); return *this; }

    /** Append from another string (modifier).
     - For best performance use set(const String&) when this is empty
     .
     \param  str  %String to append
     \return      This
    */
    String& add(const StringBase& str)
        { ListType::add(str); return *this; }

    /** Append from string pointer (modifier).
     - For best performance use set(const char*,Size) when this is empty
     .
     \param  str   %String to append
     \param  size  %String size as character count to append
     \return       This
    */
    String& add(const char* str, Size size)
        { ListType::add(str, size); return *this; }

    /** Append null terminated string (modifier).
     - For best performance use set(const char*) when this is empty
     .
     \param  str  %String to append -- must be terminated
     \return      This
    */
    String& add(const char* str)
        { if (str) add(str, (Size)strlen(str)); return *this; }

    /** Append separator/delimiter if needed (modifier).
     - This will only append given delim if not empty and not already ending with delim
     .
     \param  delim  Delimiter to append
     \return        This
    */
    String& addsep(char delim=',')
        { if (size_ > 0 && data_[size_-1] != delim) add(delim); return *this; }

    /** Append formatted signed number (modifier).
     \param  num   Number to append
     \param  base  Base to use for formatting
     \return       This
    */
    String& addn(int num, int base=fDEC)
        { writenum(num, base); return *this; }

    /** Append formatted signed number (modifier).
     \param  num   Number to append
     \param  base  Base to use for formatting
     \return       This
    */
    String& addn(long num, int base=fDEC)
        { writenum(num, base); return *this; }

    /** Append formatted signed number (modifier).
     \param  num   Number to append
     \param  base  Base to use for formatting
     \return       This
    */
    String& addn(longl num, int base=fDEC)
        { writenum(num, base); return *this; }

    /** Append formatted unsigned number (modifier).
     \param  num   Number to append
     \param  base  Base to use for formatting
     \return       This
    */
    String& addn(uint num, int base=fDEC)
        { writenumu(num, base); return *this; }

    /** Append formatted unsigned number (modifier).
     \param  num   Number to append
     \param  base  Base to use for formatting
     \return       This
    */
    String& addn(ulong num, int base=fDEC)
        { writenumu(num, base); return *this; }

    /** Append formatted unsigned number (modifier).
     \param  num   Number to append
     \param  base  Base to use for formatting
     \return       This
    */
    String& addn(ulongl num, int base=fDEC)
        { writenumu(num, base); return *this; }

    /** Append formatted floating point number (modifier).
     \param  num        Number to append
     \param  precision  Formatting precision (number of fractional digits), 0 for none, fPREC_AUTO for automatic
     \return            This
    */
    String& addn(float num, int precision=fPREC_AUTO)
        { writenumf(num, precision); return *this; }

    /** Append formatted floating point number (modifier).
     \param  num        Number to append
     \param  precision  Formatting precision (number of fractional digits), 0 for none, fPREC_AUTO for automatic
     \return            This
    */
    String& addn(double num, int precision=fPREC_AUTO)
        { writenumf(num, precision); return *this; }

    /** Append formatted floating point number (modifier).
     \param  num        Number to append
     \param  precision  Formatting precision (number of fractional digits), 0 for none, fPREC_AUTO for automatic
     \return            This
    */
    String& addn(ldouble num, int precision=fPREC_AUTO)
        { writenumf(num, precision); return *this; }

    /** Append operator.
     - Bool value is formatted as either "true" or "false" (without quotes)
     .
     \param  val  Bool value to append
     \return      This
    */
    String& operator<<(bool val) {
        if (val)
            add("true", 4);
        else
            add("false", 5);
        return *this;
    }

    /** Append operator.
     - Same as add(char)
     - Append operators can be chained\n
       Example:
       \code
        // Clear string and append two characters
        str << vEMPTY << 'a' << 'b';
       \endcode
     .
     \param  ch  Character to append
     \return     This
    */
    String& operator<<(char ch)
        { return add(ch); }

    /** Append operator.
     - Same as add(const ListType&)
     - Append operators can be chained\n
       Example:
       \code
        // Clear string and append two strings
        str << vEMPTY << str1 << str2;
       \endcode
     .
     \param  str  %String to append
     \return      This
    */
    String& operator<<(const ListType& str)
        { return add(str); }

    /** Append operator.
     - Same as add(const StringBase&)
     - Append operators can be chained\n
       Example:
       \code
        // Clear string and append two strings
        str << vEMPTY << str1 << str2;
       \endcode
     .
     \param  str  %String to append

     \return      This
    */
    String& operator<<(const StringBase& str)
        { return add(str); }

    /** Append operator.
     - Same as add(const char*)
     - Append operators can be chained\n
       Example:
       \code
        // Clear string and append two strings
        str << vEMPTY << "foo" << "bar";
       \endcode
     .
     \param  str  %String pointer to append -- must be null terminated
     \return      This
    */
    String& operator<<(const char* str)
        { return add(str); }

    /** Append operator to set as null and empty.
     - Same as set()
     - Append operators can be chained\n
       Example:
       \code
        // Clear string and append two strings
        str << vNULL << "foo" << "bar";
       \endcode
     .
     \param  val  vNULL
     \return      This
    */
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
        str << vEMPTY << "foo" << "bar";
       \endcode
     .
     \param  val  vEMPTY
     \return      This
    */
    String& operator<<(const ValEmpty& val) {
        EVO_PARAM_UNUSED(val);
        return setempty();
    }

    /** Append operator to append newline.
     - Append operators can be chained\n
       Example:
       \code
        // Clear string and append string and newline
        str << vEMPTY << "foo" << NL;
       \endcode
     .
     \param  nl  Newline type, NL for system newline
     \return     This
    */
    String& operator<<(Newline nl) {
        add(getnewline(nl), getnewlinesize(nl));
        return *this;
    }
    
    /** Flush output buffer -- no-op for string.
     - Use param: fFLUSH
     .
     \return  This
    */
    String& operator<<(Flush)
        { return *this; }

    /** Append operator to append formatted number.
     - Same as addn(int,int) with base=fDEC
     - Append operators can be chained\n
       Example:
       \code
        // Clear string and append a number
        str << vEMPTY << 123;
       \endcode
     .
     \param  num  Number to format and append
     \return      This
    */
    String& operator<<(int num)
        { writenum(num); return *this; }

    /** Append operator to append formatted number.
     - Same as addn(long,int) with base=fDEC
     - Append operators can be chained\n
       Example:
       \code
        // Clear string and append a number
        str << vEMPTY << 123;
       \endcode
     .
     \param  num  Number to format and append
     \return      This
    */
    String& operator<<(long num)
        { writenum(num); return *this; }

    /** Append operator to append formatted number.
     - Same as addn(longl,int) with base=fDEC
     - Append operators can be chained\n
       Example:
       \code
        // Clear string and append a number
        str << vEMPTY << 123;
       \endcode
     .
     \param  num  Number to format and append
     \return      This
    */
    String& operator<<(longl num)
        { writenum(num); return *this; }

    /** Append operator to append formatted number.
     - Same as addn(uint,int) with base=fDEC
     - Append operators can be chained\n
       Example:
       \code
        // Clear string and append a number
        str << vEMPTY << 123;
       \endcode
     .
     \param  num  Number to format and append
     \return      This
    */
    String& operator<<(uint num)
        { writenumu(num); return *this; }

    /** Append operator to append formatted number.
     - Same as addn(ulong,int) with base=fDEC
     - Append operators can be chained\n
       Example:
       \code
        // Clear string and append a number
        str << vEMPTY << 123;
       \endcode
     .
     \param  num  Number to format and append
     \return      This
    */
    String& operator<<(ulong num)
        { writenumu(num); return *this; }

    /** Append operator to append formatted number.
     - Same as addn(ulongl,int) with base=fDEC
     - Append operators can be chained\n
       Example:
       \code
        // Clear string and append a number
        str << vEMPTY << 123;
       \endcode
     .
     \param  num  Number to format and append
     \return      This
    */
    String& operator<<(ulongl num)
        { writenumu(num); return *this; }

    /** Append operator to append formatted number.
     - Same as addn(float,int) with precision=fPREC_AUTO
     - Append operators can be chained\n
       Example:
       \code
        // Clear string and append a number
        str << vEMPTY << 123.4;
       \endcode
     .
     \param  num  Number to format and append
     \return      This
    */
    String& operator<<(float num)
        { writenumf(num); return *this; }

    /** Append operator to append formatted number.
     - Same as addn(double,int) with precision=fPREC_AUTO
     - Append operators can be chained\n
       Example:
       \code
        // Clear string and append a number
        str << vEMPTY << 123.4;
       \endcode
     .
     \param  num  Number to format and append
     \return      This
    */
    String& operator<<(double num)
        { writenumf(num); return *this; }

    /** Append operator to append formatted number.
     - Same as addn(ldouble,int) with precision=fPREC_AUTO
     - Append operators can be chained\n
       Example:
       \code
        // Clear string and append a number
        str << vEMPTY << 123.4;
       \endcode
     .
     \param  num  Number to format and append
     \return      This
    */
    String& operator<<(ldouble num)
        { writenumf(num); return *this; }

    /** Append operator to append formatted character field.
     \param  fmt  Character info
     \return      This
    */
    String& operator<<(const FmtChar& fmt)
        { add(fmt.ch, fmt.count); return *this; }

    /** Append operator to append formatted string field.
     \param  fmt  %String formatting info
     \return      This
    */
    String& operator<<(const FmtString& fmt)
        { writefmtstr(fmt.str.data_, fmt.str.size_, fmt.fmt); return *this; }

    /** Append operator to append formatted wrapped string column.
     \param  fmt  %String formatting info
     \return      This
    */
    String& operator<<(const FmtStringWrap& fmt) {
        const char* newline_str = fmt.newline.getnewline();
        const uint newline_size = fmt.newline.getnewlinesize();

        const Size EST_LINES = (fmt.width > 0 ? fmt.str.size_ / fmt.width : 1) + 1;
        reserve(fmt.str.size_ + ((fmt.indent + newline_size + 1) * EST_LINES));

        String str(fmt.str.data_, fmt.str.size_), line, substr;
        for (uint notfirst=0; str.token_line(line); ) {
            for (;;) {
                if (line.empty()) {
                    add(newline_str, newline_size);
                } else {
                    if (notfirst == 0)
                        ++notfirst;
                    else if (fmt.indent > 0)
                        add(' ', fmt.indent);
                    if (fmt.width > 1 && line.size() > (Size)fmt.width) {
                        // line too long, try to find a word break
                        Size i = line.findnonwordr(0, (Size)fmt.width + 1);
                        if (i == NONE) {
                            i = (Size)fmt.width;
                        } else {
                            while (i > 0 && ascii_breaktype(line[i]) == cbtBEGIN)
                                --i;
                            if (i < (Size)fmt.width)
                                ++i;
                        }

                        substr.set(line.data(), i).stripr();
                        if (!substr.empty()) {
                            add(substr);
                            add(newline_str, newline_size);
                        }

                        line.triml(i).stripl();
                        continue;
                    }
                    add(line);
                    add(newline_str, newline_size);
                }
                break;
            }
        }
        return *this;
    }

    /** Append operator to append formatted number field.
     \param  fmt  Number info
     \return      This
    */
    String& operator<<(const FmtShort& fmt)
        { writefmtnum(fmt.num, fmt.fmt); return *this; }

    /** \copydoc operator<<(const FmtShort&) */
    String& operator<<(const FmtInt& fmt)
        { writefmtnum(fmt.num, fmt.fmt); return *this; }

    /** \copydoc operator<<(const FmtShort&) */
    String& operator<<(const FmtLong& fmt)
        { writefmtnum(fmt.num, fmt.fmt); return *this; }

    /** \copydoc operator<<(const FmtShort&) */
    String& operator<<(const FmtLongL& fmt)
        { writefmtnum(fmt.num, fmt.fmt); return *this; }

    /** \copydoc operator<<(const FmtShort&) */
    String& operator<<(const FmtUShort& fmt)
        { writefmtnumu(fmt.num, fmt.fmt); return *this; }

    /** \copydoc operator<<(const FmtShort&) */
    String& operator<<(const FmtUInt& fmt)
        { writefmtnumu(fmt.num, fmt.fmt); return *this; }

    /** \copydoc operator<<(const FmtShort&) */
    String& operator<<(const FmtULong& fmt)
        { writefmtnumu(fmt.num, fmt.fmt); return *this; }

    /** \copydoc operator<<(const FmtShort&) */
    String& operator<<(const FmtULongL& fmt)
        { writefmtnumu(fmt.num, fmt.fmt); return *this; }

    /** \copydoc operator<<(const FmtShort&) */
    String& operator<<(const FmtFloat& fmt)
        { writefmtnumf(fmt.num, fmt.fmt); return *this; }

    /** \copydoc operator<<(const FmtShort&) */
    String& operator<<(const FmtFloatD& fmt)
        { writefmtnumf(fmt.num, fmt.fmt); return *this; }

    /** \copydoc operator<<(const FmtShort&) */
    String& operator<<(const FmtFloatL& fmt)
        { writefmtnumf(fmt.num, fmt.fmt); return *this; }

    /** Append operator to append formatted pointer field.
     \param  fmtptr  Pointer info
     \return         This
    */
    String& operator<<(const FmtPtr& fmtptr)
        { writefmtnumu((size_t)fmtptr.ptr, fmtptr.fmt); return *this; }

    /** Append operator to append data dump.
     \param  fmt  Dump info
     \return      This
    */
    String& operator<<(const FmtDump& fmt)
        { writefmtdump(fmt, getnewline(), getnewlinesize()); return *this; }

    /** %String formatter with state.
     - This is associated with a String object and supports stateful (i.e. sticky) output formatting with "<<" operator on this object
     - Formatting attributes include:
       - Aligned fields for formatting text "columns"
       - Integer base, prefix, and padding (in addition to field padding)
       - Floating point precision and padding (in additon to field padding)
       - %String to use for "null" values (null char*, String, \ref Int, etc)
       .
     - Note: Single character (char) and Newline (NL) values are not formatted as fields (i.e. not padded) since they're usually delimiters
     - See: \ref StringFormatting "String Formatting"

    \par Example

    \code
    #include <evo/string.h>
    using namespace evo;

    int main() {
        String str;

        // Use a temporary formatter to append line to str: 7B,1C8
        String::Format(str) << fHEX << 123 << ',' << 456 << NL;

        // Create a formatter and use to append line to str: 001,002
        String::Format out(str);
        out << FmtSetInt(fDEC, 3) << 1 << ',' << 2 << NL;

        return 0;
    }
    \endcode

    For more examples see: \ref StringFormatting "String Formatting"
    */
    struct Format {
        typedef String::Size Size;  ///< %String data size type
        typedef String Out;         ///< Associated output string type, type returned by write_out()
        typedef Format This;        ///< This type

        Out&       out;             ///< Associated output string
        FmtAttribs fmt;             ///< Formatting attributes (state)

        /** Constructor.
         \param  out  Output string to associate and format to
        */
        Format(Out& out) : out(out)
            { }

        /** Copy constructor.
         - This will reference the same string as src
         .
         \param  src  Source to copy
        */
        Format(const This& src) : out(src.out), fmt(src.fmt)
            { }

        /** Assignment operator copies attributes.
         - This does not copy the referenced string
         .
         \param  src  Source to copy attributes from
         \return      This
        */
        This& operator=(const This& src)
            { memcpy(&fmt, &src.fmt, sizeof(FmtAttribs)); return *this; }

        /** Get parent output string.
         \return  Parent output string
        */
        Out& write_out()
            { return out; }

        // Field
        
        /** %Set field alignment type to use.
         \param  align  Alignment type
         \return        This
        */
        This& operator<<(FmtAlign align)
            { fmt.field.align = align; return *this; }

        /** %Set field width to use.
         \param  width  Field width to use
         \return        This
        */
        This& operator<<(FmtWidth width)
            { fmt.field.width = width; return *this; }

        /** %Set field attributes to use.
         \param  field  Field attributes
         \return        This
        */
        This& operator<<(const FmtSetField& field)
            { fmt.field.merge(field); return *this; }

        // Newlines/Flush

        /** Append a newline.
         \param  nl  %Newline type to append, NL for system default
         \return     This
        */
        This& operator<<(Newline nl)
            { out.add(getnewline(nl), getnewlinesize(nl)); return *this; }

        /** Flush output buffer -- no-op for string.
         - Use param: fFLUSH
         .
         \return  This
        */
        This& operator<<(Flush)
            { return *this; }

        // Null

        /** %Set attributes for null values.
         \param  null  Null attributes
         \return       This
        */
        This& operator<<(const FmtSetNull& null)
            { fmt.null = null; return *this; }

        // Bools

        /** Append operator.
         - Bool value is formatted as either "true" or "false" (without quotes)
         .
         \param  val  Bool value to append
         \return      This
        */
        This& operator<<(bool val) {
            if (val)
                out.add("true", 4);
            else
                out.add("false", 5);
            return *this;
        }

        // Chars

        /** Append a character.
         \param  ch  Character to append
         \return     This
        */
        This& operator<<(char ch)
            { out.add(ch); return *this; }

        /** Append a repeated character.
         \param  ch  Character info to append
         \return     This
        */
        This& operator<<(const FmtChar& ch)
            { out.writefmtchar(ch.ch, ch.count, fmt.field); return *this; }

        // Strings

        /** Append a terminated string.
         - Field attributes apply
         .
         \param  val  %String pointer, must be terminated, NULL for null string
         \return      This
        */
        This& operator<<(const char* val) {
            if (val == NULL) {
                if (fmt.null.size > 0)
                    out.writefmtstr(fmt.null.str, fmt.null.size, fmt.field);
            } else if (*val != '\0')
                out.writefmtstr(val, (Size)strlen(val), fmt.field);
            return *this;
        }

        /** Append a string.
         - Field attributes apply
         .
         \param  val  %String value
         \return      This
        */
        This& operator<<(const StringBase& val) {
            if (val.data_ == NULL) {
                if (fmt.null.size > 0)
                    out.writefmtstr(fmt.null.str, fmt.null.size, fmt.field);
            } else if (val.size_ > 0)
                out.writefmtstr(val.data_, val.size_, fmt.field);
            return *this;
        }

        // Integers

        /** %Set base for formatting integers.
         \param  base  Base to use, see FmtBase
         \return       This
        */
        This& operator<<(FmtBase base)
            { fmt.num_int.base = base; return *this; }

        /** %Set prefix for formatting integers.
         \param  prefix  Integer prefix to use, see FmtBasePrefix
         \return         This
        */
        This& operator<<(FmtBasePrefix prefix)
            { fmt.num_int.prefix = prefix; return *this; }

        /** %Set integer formatting attributes.
         \param  fmt_int  Integer formatting attributes
         \return          This
        */
        This& operator<<(const FmtSetInt& fmt_int)
            { fmt.num_int.merge(fmt_int); return *this; }

        /** Append a formatted signed integer.
         - Integer and field attributes apply
         .
         \param  num  Integer to format
         \return      This
        */
        This& operator<<(short num)
            { out.writefmtnum(num, fmt.num_int, &fmt.field); return *this; }

        /** \copydoc operator<<(short) */
        This& operator<<(int num)
            { out.writefmtnum(num, fmt.num_int, &fmt.field); return *this; }

        /** \copydoc operator<<(short) */
        This& operator<<(long num)
            { out.writefmtnum(num, fmt.num_int, &fmt.field); return *this; }

        /** \copydoc operator<<(short) */
        This& operator<<(longl num)
            { out.writefmtnum(num, fmt.num_int, &fmt.field); return *this; }

        /** Append a formatted unsigned integer.
         - Integer and field attributes apply
         .
         \param  num  Integer to format
         \return      This
        */
        This& operator<<(ushort num)
            { out.writefmtnumu(num, fmt.num_int, &fmt.field); return *this; }

        /** \copydoc operator<<(ushort) */
        This& operator<<(uint num)
            { out.writefmtnumu(num, fmt.num_int, &fmt.field); return *this; }

        /** \copydoc operator<<(ushort) */
        This& operator<<(ulong num)
            { out.writefmtnumu(num, fmt.num_int, &fmt.field); return *this; }

        /** \copydoc operator<<(ushort) */
        This& operator<<(ulongl num)
            { out.writefmtnumu(num, fmt.num_int, &fmt.field); return *this; }

        /** Append a formatted integer class.
         - Integer, field, and null attributes apply
         .
         \tparam  T  Integer POD type, deduced from param
         \param  num  Integer to format (Int, UInt, etc)
         \return      This
        */
        template<class T>
        This& operator<<(const IntegerT<T>& num) {
            if (num.null()) {
                if (fmt.null.size > 0)
                    out.writefmtstr(fmt.null.str, fmt.null.size, fmt.field);
            } else if (IntegerT<T>::SIGN)
                out.writefmtnum(num.value(), fmt.num_int, &fmt.field);
            else
                out.writefmtnumu(num.value(), fmt.num_int, &fmt.field);
            return *this;
        }

        // Floats

        /** %Set floating point formatting precision.
         \param  prec  Precision value, see FmtPrecision
         \return       This
        */
        This& operator<<(FmtPrecision prec)
            { fmt.num_flt.precision = prec; return *this; }

        /** %Set floating point formatting attributes.
         \param  fmt_flt  Floating point formatting attributes
         \return          This
        */
        This& operator<<(const FmtSetFloat& fmt_flt)
            { fmt.num_flt.merge(fmt_flt); return *this; }

        /** Append a formatting floating point number.
         - Floating point and field attributes apply
         .
         \param  num  Number to format
         \return      This
        */
        This& operator<<(float num)
            { out.writefmtnumf(num, fmt.num_flt, &fmt.field); return *this; }

        /** \copydoc operator<<(float) */
        This& operator<<(double num)
            { out.writefmtnumf(num, fmt.num_flt, &fmt.field); return *this; }

        /** \copydoc operator<<(float) */
        This& operator<<(ldouble num)
            { out.writefmtnumf(num, fmt.num_flt, &fmt.field); return *this; }

        /** Append a formatted integer class.
         - Floating point, field, and null attributes apply
         .
         \tparam  T  Floating point POD type, deduced from param
         \param  num  Number to format (Float, FloatD, etc)
         \return      This
        */
        template<class T>
        This& operator<<(const FloatT<T>& num) {
            if (num.null()) {
                if (fmt.null.size > 0)
                    out.writefmtstr(fmt.null.str, fmt.null.size, fmt.field);
            } else
                out.writefmtnumf(num.value(), fmt.num_flt, &fmt.field);
            return *this;
        }

        // Dump

        This& operator<<(const FmtDump& fmtdump)
            { out.writefmtdump(fmtdump, getnewline(), getnewlinesize()); return *this; }
    };

    // PREPEND

    /** Prepend character (modifier).
     \param  ch  Character to prepend
     \return     This
    */
    String& prepend(char ch)
        { ListType::prepend(ch); return *this; }

    /** Prepend copies of the same character (modifier).
     \param  ch     Character to prepend
     \param  count  Character copies to prepend
     \return        This
    */
    String& prepend(char ch, Size count) {
        ListType::advPrepend(count);
        if (count > 0)
            memset(data_, (int)(uchar)ch, count);
        return *this;
    }

    /** Prepend from another string (modifier).
     \param  str  %String to prepend
     \return      This
    */
    String& prepend(const ListType& str)
        { ListType::prepend(str); return *this; }

    /** Prepend from another string (modifier).
     \param  str  %String to prepend
     \return      This
    */
    String& prepend(const StringBase& str)
        { ListType::prepend(str); return *this; }

    /** Prepend from string pointer (modifier).
     \param  str   %String to prepend
     \param  size  %String size as character count to prepend
     \return       This
    */
    String& prepend(const char* str, Size size)
        { ListType::prepend(str, size); return *this; }

    /** Prepend null terminated string (modifier).
     \param  str  %String to prepend -- must be terminated
     \return      This
    */
    String& prepend(const char* str)
        { if (str) prepend(str, (Size)strlen(str)); return *this; }

    /** Prepend separator/delimiter if needed (modifier).
     - This will only prepend given delim if not empty and not already starting with delim
     .
     \param  delim  Delimiter to prepend
     \return        This
    */
    String& prependsep(char delim=',')
        { if (size_ > 0 && data_[0] != delim) prepend(delim); return *this; }

    /** Prepend formatted signed number (modifier).
     \param  num   Number to prepend
     \param  base  Base to use for formatting
     \return       This
    */
    String& prependn(int num, int base=fDEC)
        { prependnum(num, base); return *this; }

    /** Prepend formatted signed number (modifier).
     \param  num   Number to prepend
     \param  base  Base to use for formatting
     \return       This
    */
    String& prependn(long num, int base=fDEC)
        { prependnum(num, base); return *this; }

    /** Prepend formatted signed number (modifier).
     \param  num   Number to prepend
     \param  base  Base to use for formatting
     \return       This
    */
    String& prependn(longl num, int base=fDEC)
        { prependnum(num, base); return *this; }

    /** Prepend formatted unsigned number (modifier).
     \param  num   Number to prepend
     \param  base  Base to use for formatting
     \return       This
    */
    String& prependn(uint num, int base=fDEC)
        { prependnum(num, base); return *this; }

    /** Prepend formatted unsigned number (modifier).
     \param  num   Number to prepend
     \param  base  Base to use for formatting
     \return       This
    */
    String& prependn(ulong num, int base=fDEC)
        { prependnum(num, base); return *this; }

    /** Prepend formatted unsigned number (modifier).
     \param  num   Number to prepend
     \param  base  Base to use for formatting
     \return       This
    */
    String& prependn(ulongl num, int base=fDEC)
        { prependnum(num, base); return *this; }

    /** Prepend formatted floating point number (modifier).
     \param  num        Number to prepend
     \param  precision  Formatting precision (number of fractional digits), 0 for none, fPREC_AUTO for automatic
     \return            This
    */
    String& prependn(float num, int precision=fPREC_AUTO)
        { prependnumf(num, precision); return *this; }

    /** Prepend formatted floating point number (modifier).
     \param  num        Number to prepend
     \param  precision  Formatting precision (number of fractional digits), 0 for none, fPREC_AUTO for automatic
     \return            This
    */
    String& prependn(double num, int precision=fPREC_AUTO)
        { prependnumf(num, precision); return *this; }

    /** Prepend formatted floating point number (modifier).
     \param  num        Number to prepend
     \param  precision  Formatting precision (number of fractional digits), 0 for none, fPREC_AUTO for automatic
     \return            This
    */
    String& prependn(ldouble num, int precision=fPREC_AUTO)
        { prependnumf(num, precision); return *this; }

    // INSERT

    /** Insert character (modifier).
     \param  index  Insert index, END to append
     \param  ch     Character to insert
     \return        Inserted index
    */
    Size insert(Key index, char ch)
        { return ListType::insert(index, ch); }

    /** Insert copies of the same character (modifier).
     \param  index  Insert index, END to append
     \param  ch     Character to insert
     \param  count  Character copies to insert
     \return        Inserted index
    */
    Size insert(Key index, char ch, Size count) {
        index = ListType::advInsert(index, count);
        if (count > 0)
            memset(data_ + index, (int)(uchar)ch, count);
        return index;
    }

    /** Insert from another string (modifier).
     \param  index  Insert index, END to append
     \param  str    %String to insert
     \return        Inserted index
    */
    Size insert(Key index, const ListType& str)
        { return ListType::insert(index, str); }

    /** Insert from another string (modifier).
     \param  index  Insert index, END to append
     \param  str    %String to insert
     \return        Inserted index
    */
    Size insert(Key index, const StringBase& str)
        { return ListType::insert(index, str); }

    /** Insert string data (modifier).
     \param  index  Insert index, END to append
     \param  str    %String to insert
     \param  size   %String size as character count to insert
     \return        Inserted index
    */
    Size insert(Key index, const char* str, Size size)
        { return ListType::insert(index, str, size); }

    /** Insert null terminated string (modifier).
     \param  index  Insert index, END to append
     \param  str    %String to insert -- must be terminated
     \return        This
    */
    String& insert(Key index, const char* str)
        { if (str) insert(index, str, (Size)strlen(str)); return *this; }

    /** Insert formatted signed number (modifier).
     \param  index  Insert index, END to append
     \param  num    Number to insert
     \param  base   Base to use for formatting
     \return        This
    */
    String& insertn(Key index, int num, int base=fDEC)
        { insertnum(index, num, base); return *this; }

    /** Insert formatted signed number (modifier).
     \param  index  Insert index, END to append
     \param  num    Number to insert
     \param  base   Base to use for formatting
     \return        This
    */
    String& insertn(Key index, long num, int base=fDEC)
        { insertnum(index, num, base); return *this; }

    /** Insert formatted signed number (modifier).
     \param  index  Insert index, END to append
     \param  num    Number to insert
     \param  base   Base to use for formatting
     \return        This
    */
    String& insertn(Key index, longl num, int base=fDEC)
        { insertnum(index, num, base); return *this; }

    /** Insert formatted unsigned number (modifier).
     \param  index  Insert index, END to append
     \param  num    Number to insert
     \param  base   Base to use for formatting
     \return        This
    */
    String& insertn(Key index, uint num, int base=fDEC)
        { insertnum(index, num, base); return *this; }

    /** Insert formatted unsigned number (modifier).
     \param  index  Insert index, END to append
     \param  num    Number to insert
     \param  base   Base to use for formatting
     \return        This
    */
    String& insertn(Key index, ulong num, int base=fDEC)
        { insertnum(index, num, base); return *this; }

    /** Insert formatted unsigned number (modifier).
     \param  index  Insert index, END to append
     \param  num    Number to insert
     \param  base   Base to use for formatting
     \return        This
    */
    String& insertn(Key index, ulongl num, int base=fDEC)
        { insertnum(index, num, base); return *this; }

    /** Insert formatted floating point number (modifier).
     \param  index      Insert index, END to append
     \param  num        Number to insert
     \param  precision  Formatting precision (number of fractional digits), 0 for none, fPREC_AUTO for automatic
     \return            This
    */
    String& insertn(Key index, float num, int precision=fPREC_AUTO)
        { insertnumf(index, num, precision); return *this; }

    /** Insert formatted floating point number (modifier).
     \param  index      Insert index, END to append
     \param  num        Number to insert
     \param  precision  Formatting precision (number of fractional digits), 0 for none, fPREC_AUTO for automatic
     \return            This
    */
    String& insertn(Key index, double num, int precision=fPREC_AUTO)
        { insertnumf(index, num, precision); return *this; }

    /** Insert formatted floating point number (modifier).
     \param  index      Insert index, END to append
     \param  num        Number to insert
     \param  precision  Formatting precision (number of fractional digits), 0 for none, fPREC_AUTO for automatic
     \return            This
    */
    String& insertn(Key index, ldouble num, int precision=fPREC_AUTO)
        { insertnumf(index, num, precision); return *this; }

    // FILL

    /** Fill with copies of character (modifier).
     \param  ch     Character to fill with
     \param  index  Start index, END to start at end and append
     \param  size   Size to fill as character count from index, ALL for all items from index, 0 to do nothing
    */
    String& fillch(char ch, Key index=0, Size size=ALL)
        { ListType::fill(ch, index, size); return *this; }

    // REPLACE

    /** Replace characters with string (modifier).
     \param  index  Start index to replace
     \param  rsize  Size as item count from index to replace, ALL for all items from index
     \param  str    Replacement string to copy
     \return        This
    */
    String& replace(Key index, Size rsize, const StringBase& str)
        { ListType::replace(index, rsize, str.data_, str.size_); return *this; }

    /** Replace characters with string (modifier).
     \param  index  Start index to replace
     \param  rsize  Size as item count from index to replace, ALL for all items from index
     \param  str    Replacement string to copy
     \param  size   Replacement string size as character count
     \return        This
    */
    String& replace(Key index, Size rsize, const char* str, Size size)
        { ListType::replace(index, rsize, str, size); return *this; }

    /** Replace characters with string (modifier).
     \param  index  Start index to replace
     \param  rsize  Size as item count from index to replace, ALL for all items from index
     \param  str    Replacement string to copy -- must be terminated
     \return        This
    */
    String& replace(Key index, Size rsize, const char* str)
        { ListType::replace(index, rsize, str, str?(Size)strlen(str):0); return *this; }

    /** Find character and replace with string (modifier).
     \param  ch    Character to find
     \param  str   Replacement string pointer
     \param  size  Replacement string size
     \param  max   Max occurrences to find and replace, ALL for all
     \return       Number of replacements made, 0 if none
    */
    Size findreplace(char ch, const char* str, Size size, Size max=ALL) {
        uint startmax = max;
        for (Size i = 0; max > 0; --max) {
            i = find(ch, i);
            if (i == NONE)
                break;
            ListType::replace(i, 1, str, size);
            i += size;
        }
        return startmax - max;
    }

    /** Find character and replace with string (modifier).
     \param  ch    Character to find
     \param  str   Replacement string
     \param  max   Max occurrences to find and replace, ALL for all
     \return       Number of replacements made, 0 if none
    */
    Size findreplace(char ch, const StringBase& str, Size max=ALL)
        { return findreplace(ch, str.data_, str.size_, max); }

    /** Find string and replace with string (modifier).
     \param  fstr   Pointer to string to find and replace
     \param  fsize  %String size to find and replace
     \param  str    Replacement string pointer
     \param  size   Replacement string size
     \param  max    Max occurrences to find and replace, ALL for all
     \return        Number of replacements made, 0 if none
    */
    Size findreplace(const char* fstr, Size fsize, const char* str, Size size, Size max=ALL) {
        uint startmax = max;
        for (Size i = 0; max > 0; --max) {
            i = find(fstr, fsize, i);
            if (i == NONE)
                break;
            ListType::replace(i, fsize, str, size);
            i += size;
        }
        return startmax - max;
    }

    /** Find string and replace with string (modifier).
     \param  fstr   Pointer to string to find and replace
     \param  fsize  %String size to find and replace
     \param  str    Replacement string
     \param  max    Max occurrences to find and replace, ALL for all
     \return        Number of replacements made, 0 if none
    */
    Size findreplace(const char* fstr, Size fsize, const StringBase& str, Size max=ALL)
        { return findreplace(fstr, fsize, str.data_, str.size_, max); }

    /** Find string and replace with string (modifier).
     \param  fstr   %String to find and replace
     \param  str    Replacement string
     \param  max    Max occurrences to find and replace, ALL for all
     \return        Number of replacements made, 0 if none
    */
    Size findreplace(const StringBase& fstr, const StringBase& str, Size max=ALL)
        { return findreplace(fstr.data_, fstr.size_, str.data_, str.size_, max); }

    /** Find string and replace with string (modifier).
     \param  fstr   %String to find and replace
     \param  str    Replacement string pointer
     \param  size   Replacement string size
     \param  max    Max occurrences to find and replace, ALL for all
     \return        Number of replacements made, 0 if none
    */
    Size findreplace(const StringBase& fstr, const char* str, Size size, Size max=ALL)
        { return findreplace(fstr.data_, fstr.size_, str, size, max); }

    // GETBOOL

    /** %Convert to bool value for given boolean type. Fails on bad format, empty, or number overflow.

    %Format: [WHITESPACE] ["on"|"off"|"yes"|"no"|"true"|"false"|"t"|"f"|DIGITS] [WHITESPACE]
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
    bool getbool(Error& error) const {
        assert( data_ > (char*)1 || size_ == 0 );
        return impl::tobool(data_, size_, error);
    }

    /** %Convert to bool value for given boolean type. Fails on bad format, empty, or number overflow

    %Format: [WHITESPACE] ["on"|"off"|"yes"|"no"|"true"|"false"|"t"|"f"|DIGITS] [WHITESPACE]
     - Result is true if non-zero number -- checked using getnum(Error&,int) with T=long, base=0
     - Case insensitive
     .
     \tparam  T  Boolean type to convert to -- can be Bool or bool
     \return  Converted value on success, Bool set as null on failure, primitive is false on failure
     */
    template<class T> T getbool() const {
        assert( data_ > (char*)1 || size_ == 0 );
        return StaticIf< IsPodType<T>::value, impl::ToBoolPod<T>, impl::ToBool<T> >::Type::getbool(data_, size_);
    }

    // GETNUM

    /** %Convert to number value for given integer type. Fails on bad format, no digits, or overflow.

    %Format: [WHITESPACE] ["+"|"-"] ["0x"|"0X"|"x"|"0"] DIGITS [WHITESPACE]
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
    template<class T> T getnum(Error& error, int base=0) const {
        STATIC_ASSERT( IsPodType<T>::value, getnum_POD_Type_Required );
        assert( data_ > (char*)1 || size_ == 0 );
        return impl::tonum<T>(data_, size_, error, base);
    }

    /** %Convert to number value for given integer type. Fails on bad format, no digits, or overflow.

    %Format: [WHITESPACE] ["+"|"-"] ["0x"|"0X"|"x"|"0"] DIGITS [WHITESPACE]
     - Base 2-36 supported -- Base 0 autodetects hex or octal based on prefix, defaults to decimal
     - Base 16 (hex) may use "0x", "0X", or "x" prefix
     - Base 8 (octal) may use "0" prefix
     - Letters in digits are not case sensitive
     .
     \tparam  T  Integer type to convert to -- can be IntegerT type like Int32, Int, ULong, etc or primitive like int32, int, ulong, etc
     \param  base  Conversion base, 0 for autodetect
     \return       Converted value on success, IntegerT set as null on failure, primitive is 0 on failure
    */
    template<class T> T getnum(int base=0) const {
        assert( data_ > (char*)1 || size_ == 0 );
        return StaticIf< IsPodType<T>::value, impl::ToNumPod<T>, impl::ToNum<T> >::Type::getnum(data_, size_, base);
    }

    /** %Convert to floating point number value for given type. Fails on bad format or no digits.

    %Format: [WHITESPACE] ("nan" | NUMBER) [WHITESPACE]\n
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
    template<class T> T getnumf(Error& error) const {
        STATIC_ASSERT( IsPodType<T>::value, getnumf_POD_Type_Required );
        assert( data_ > (char*)1 || size_ == 0 );
        return impl::tonumf<T>(data_, size_, error);
    }

    /** %Convert to floating point number value for given type. Fails on bad format or no digits.

    %Format: [WHITESPACE] ("nan" | NUMBER) [WHITESPACE]\n
     - NUMBER: ["+"|"-"] ("inf" | DIGITS ["." DIGITS] [EXPONENT])\n
     - EXPONENT: ["+"|"-"] ("e"|"E") DIGITS
     - Only decimal (base 10) digits supported
     - Not-A-Number ("nan") and infinite ("inf") values are case insensitive
     .
     \tparam  T  Floating point type to convert to -- can be FloatT type like Float, FloatD, etc or primitive like float, double, etc
     \return  Converted value on success, FloatT set as null on failure, primitive is 0 on failure
    */
    template<class T> T getnumf() const {
        assert( data_ > (char*)1 || size_ == 0 );
        return StaticIf< IsPodType<T>::value, impl::ToNumfPod<T>, impl::ToNumf<T> >::Type::getnum(data_, size_);
    }

    // BOOLVAL

    /** %Convert to bool value. Fails on bad format, empty, or number overflow.

    %Format: [WHITESPACE] ["on"|"off"|"yes"|"no"|"true"|"false"|"t"|"f"|DIGITS] [WHITESPACE]
     - Result is true if non-zero number -- checked using getnum(Error&,int) with T=long, base=0
     - Case insensitive
     .
     \return  Converted value, set to null on failure
    */
    Bool boolval() const
        { return impl::ToBool<Bool>::getbool(data_, size_); }

    // NUM

    /** %Convert to number value (signed). Fails on bad format, no digits, or overflow.

    %Format: [WHITESPACE] ["+"|"-"] ["0x"|"0X"|"x"|"0"] DIGITS [WHITESPACE]
     - Base 2-36 supported -- Base 0 autodetects hex or octal based on prefix, defaults to decimal
     - Base 16 (hex) may use "0x", "0X", or "x" prefix
     - Base 8 (octal) may use "0" prefix
     - Letters in digits are not case sensitive
     .
     \param  base  Conversion base, 0 for autodetect
     \return       Converted value, set as null on failure
    */
    Int num(int base=0) const
        { return impl::ToNum<Int>::getnum(data_, size_, base); }

    /** %Convert to number value (signed long). Fails on bad format, no digits, or overflow.

    %Format: [WHITESPACE] ["+"|"-"] ["0x"|"0X"|"x"|"0"] DIGITS [WHITESPACE]
     - Base 2-36 supported -- Base 0 autodetects hex or octal based on prefix, defaults to decimal
     - Base 16 (hex) may use "0x", "0X", or "x" prefix
     - Base 8 (octal) may use "0" prefix
     - Letters in digits are not case sensitive
     .
     \param  base  Conversion base, 0 for autodetect
     \return       Converted value, set as null on failure
    */
    Long numl(int base=0) const
        { return impl::ToNum<Long>::getnum(data_, size_, base); }

    /** %Convert to number value (signed long-long). Fails on bad format, no digits, or overflow.

    %Format: [WHITESPACE] ["+"|"-"] ["0x"|"0X"|"x"|"0"] DIGITS [WHITESPACE]
     - Base 2-36 supported -- Base 0 autodetects hex or octal based on prefix, defaults to decimal
     - Base 16 (hex) may use "0x", "0X", or "x" prefix
     - Base 8 (octal) may use "0" prefix
     - Letters in digits are not case sensitive
     .
     \param  base  Conversion base, 0 for autodetect
     \return       Converted value, set as null on failure
    */
    LongL numll(int base=0) const
        { return impl::ToNum<LongL>::getnum(data_, size_, base); }

    // NUMU

    /** %Convert to number value (unsigned). Fails on bad format, no digits, or overflow.

    %Format: [WHITESPACE] ["+"|"-"] ["0x"|"0X"|"x"|"0"] DIGITS [WHITESPACE]
     - Base 2-36 supported -- Base 0 autodetects hex or octal based on prefix, defaults to decimal
     - Base 16 (hex) may use "0x", "0X", or "x" prefix
     - Base 8 (octal) may use "0" prefix
     - Letters in digits are not case sensitive
     - Negative values are converted to unsigned equivalent
     .
     \param  base  Conversion base, 0 for autodetect
     \return       Converted value, set as null on failure
    */
    UInt numu(int base=0) const
        { return impl::ToNum<UInt>::getnum(data_, size_, base); }

    /** %Convert to number value (unsigned long). Fails on bad format, no digits, or overflow.

    %Format: [WHITESPACE] ["+"|"-"] ["0x"|"0X"|"x"|"0"] DIGITS [WHITESPACE]
     - Base 2-36 supported -- Base 0 autodetects hex or octal based on prefix, defaults to decimal
     - Base 16 (hex) may use "0x", "0X", or "x" prefix
     - Base 8 (octal) may use "0" prefix
     - Letters in digits are not case sensitive
     - Negative values are converted to unsigned equivalent
     .
     \param  base  Conversion base, 0 for autodetect
     \return       Converted value, set as null on failure
    */
    ULong numul(int base=0) const
        { return impl::ToNum<ULong>::getnum(data_, size_, base); }

    /** %Convert to number value (unsigned long-long). Fails on bad format, no digits, or overflow.

    %Format: [WHITESPACE] ["+"|"-"] ["0x"|"0X"|"x"|"0"] DIGITS [WHITESPACE]
     - Base 2-36 supported -- Base 0 autodetects hex or octal based on prefix, defaults to decimal
     - Base 16 (hex) may use "0x", "0X", or "x" prefix
     - Base 8 (octal) may use "0" prefix
     - Letters in digits are not case sensitive
     - Negative values are converted to unsigned equivalent
     .
     \param  base  Conversion base, 0 for autodetect
     \return       Converted value, set as null on failure
    */
    ULongL numull(int base=0) const
        { return impl::ToNum<ULongL>::getnum(data_, size_, base); }

    // NUMF

    /** %Convert to number value (floating point). Fails on bad format or no digits.

    %Format: [WHITESPACE] ("nan" | NUMBER) [WHITESPACE]\n
     - NUMBER: ["+"|"-"] ("inf" | DIGITS ["." DIGITS] [EXPONENT])\n
     - EXPONENT: ["+"|"-"] ("e"|"E") DIGITS
     - Only decimal (base 10) digits supported
     - Not-A-Number ("nan") and infinite ("inf") values are case insensitive
     .
     \return  Converted value, set as null on failure
    */
    Float numf() const
        { return impl::ToNumf<Float>::getnum(data_, size_); }

    /** %Convert to number value (double floating point). Fails on bad format or no digits.

    %Format: [WHITESPACE] ("nan" | NUMBER) [WHITESPACE]\n
     - NUMBER: ["+"|"-"] ("inf" | DIGITS ["." DIGITS] [EXPONENT])\n
     - EXPONENT: ["+"|"-"] ("e"|"E") DIGITS
     - Only decimal (base 10) digits supported
     - Not-A-Number ("nan") and infinite ("inf") values are case insensitive
     .
     \return  Converted value, set as null on failure
    */
    FloatD numfd() const
        { return impl::ToNumf<FloatD>::getnum(data_, size_); }

    /** %Convert to number value (ldouble floating point). Fails on bad format or no digits.

    %Format: [WHITESPACE] ("nan" | NUMBER) [WHITESPACE]\n
     - NUMBER: ["+"|"-"] ("inf" | DIGITS ["." DIGITS] [EXPONENT])\n
     - EXPONENT: ["+"|"-"] ("e"|"E") DIGITS
     - Only decimal (base 10) digits supported
     - Not-A-Number ("nan") and infinite ("inf") values are case insensitive
     .
     \return  Converted value, set as null on failure
    */
    FloatL numfl() const
        { return impl::ToNumf<FloatL>::getnum(data_, size_); }

    // STREAM FORMATTING

    /** Get parent output string.
     \return  Parent output string (this)
    */
    Out& write_out()
        { return *this; }

    /** Write (append) repeat character as text output to string.
     - This is the same as add(char,Size)
     .
     \param  ch     Character to write
     \param  count  Character count to write, must be positive
     \return        Size actually written (same as count)
    */
    Size writechar(char ch, Size count=1) {
        add(ch, count);
        return count;
    }

    /** Write (append) text output to string.
     - This is the same as add(const char*,Size)
     .
     \param  buf   Data to write
     \param  size  Size to write
     \return       Size actually written, 0 on error
    */
    Size writetext(const char* buf, Size size) {
        add(buf, size);
        return size;
    }

    /** Write (append) quoted output to string.
     - Newlines and unprintable characters are written as-is
     - This uses Smart Quoting -- see \ref SmartQuoting
     .
     \param  buf       Data to quote and write
     \param  size      Data size to write
     \param  delim     Delimiter for next field to escape via quoting
     \param  optional  Whether quoting is optional, true to avoid quoting if possible
     \return           Size actually written, 0 on error (unquotable text)
    */
    Size writequoted(const char* buf, Size size, char delim, bool optional=false) {
        bool quote_optional = false;
        const StrQuoting::Type type = StrQuoting::get(quote_optional, buf, size, delim);
        switch (type) {
            case StrQuoting::tSINGLE:
                if (quote_optional && optional) {
                    add(buf, size);
                    return size;
                }
            case StrQuoting::tDOUBLE:
            case StrQuoting::tBACKTICK: {
                const char ch = "'\"`"[(uint)type];
                const Size result = size + 2;
                reserve(result);
                add(ch, 1);
                add(buf, size);
                add(ch, 1);
                return result;
            }
            case StrQuoting::tSINGLE3:
            case StrQuoting::tDOUBLE3:
            case StrQuoting::tBACKTICK3: {
                const char* str = &"'''\"\"\"```"[(uint(type) - 3) * 3];
                const Size result = size + 6;
                reserve(result);
                add(str, 3);
                add(buf, size);
                add(str, 3);
                return result;
            }
            case StrQuoting::tBACKTICK_DEL: {
                const char* str = "`\x7F";
                const Size result = size + 4;
                reserve(result);
                add(str, 2);
                add(buf, size);
                add(str, 2);
                return result;
            }
            case StrQuoting::tERROR:
                break;
        }
        return 0;
    }

    /** Get pointer for writing directly to buffer to append data.
     - Call write_direct_finish() to commit written data, or don't to cancel
     - This always allocates enough space so doesn't fail with String
     .
     \param  size  Requred size in bytes to reserve
     \return       Buffer to write to (at append position), NULL on error
    */
    char* write_direct(Size size) {
        reserve(size);
        return buf_.ptr + used();
    }

    /** Get pointer for writing directly to buffer to append data and allow multiple passes for larger sizes.
     - Call write_direct_flush() or write_direct_finish() to commit written data, or neither to cancel
     - If `reserve_size` is 0 then this does nothing and returns a non-NULL but still invalid pointer
     - This always allocates enough space with String so multiple passes with write_direct_multi() aren't required, but the interface is still compatibille with Stream types
     - With String, this is the same as write_direct()
     .
     \param  available     Stores available size reserved in bytes, may be less than `reserve_size`, 0 if `reserve_size` was 0  [out]
     \param  reserve_size  Requred size in bytes to reserve
     \return               Buffer to write to (at append position), NULL on error
    */
    char* write_direct_multi(Size& available, Size reserve_size) {
        available = reserve_size;
        if (reserve_size == 0)
            return (char*)1; // finished
        reserve(reserve_size);
        return buf_.ptr + used();
    }

    /** Flush data written directly to buffer and get pointer for appending more.
     - This commits data written directly after previous call or write_direct_multi(), which must be called first
     - If `reserve_size` is 0 then this does the same as write_direct_finish() and returns a non-NULL but invalid pointer on success
     - This isn't normally needed with String but the interface is compatibille with Stream types
     .
     \param  available     Stores available size reserved in bytes, may be less than `reserve_size`, 0 if `reserve_size` was 0  [out]
     \param  written_size  Size written in bytes to flush, must not be greater than `available` size from previous call to this or write_direct_multi()
     \param  reserve_size  Requred size in bytes to reserve, 0 to finish
     \return               Buffer to write to (at append position), NULL on error
    */
    char* write_direct_flush(Size& available, Size written_size, Size reserve_size) {
        advSize(used() + written_size);
        available = reserve_size;
        if (reserve_size == 0)
            return (char*)1; // finished
        reserve(reserve_size);
        return buf_.ptr + used();
    }

    /** Finish writing directly to buffer.
     - This commits data written directly after calling write_direct() or write_direct_multi(), one of which must be called first
     - This always succeeds with String
     .
     \param  size  Size written in bytes, must not be greater than `size` passed to write_direct()
     \return       Whether successful, false on error
    */
    bool write_direct_finish(Size size) {
        advSize(used() + size);
        return true;
    }

    /** Write (append) formatted signed number.
     \tparam  TNum  Number type, inferred by param
     \param  num   Number to write
     \param  base  Base to use for formatting
    */
    template<class TNum>
    bool writenum(TNum num, int base=fDEC) {
        reserve(IntegerT<TNum>::maxlen(base)+1);
        const Size len = impl::fnum(data_+size_+IntegerT<TNum>::digits(num,base), num, base);
        buf_.header->used += len;
        size_             += len;
        return true;
    }

    /** Write (append) formatted unsigned number.
     \tparam  TNum  Number type, inferred by param
     \param  num   Number to write
     \param  base  Base to use for formatting
     \return       Whether successful, always true with String
    */
    template<class TNum>
    bool writenumu(TNum num, int base=fDEC) {
        reserve(IntegerT<TNum>::maxlen(base)+1);
        const Size len = impl::fnumu(data_+size_+IntegerT<TNum>::digits(num,base), num, base);
        buf_.header->used += len;
        size_             += len;
        return true;
    }

    /** Write (append) formatted floating-point number.
     \tparam  TNum  Number type, inferred by param
     \param  num        Number to write
     \param  precision  Formatting precision (number of fractional digits), 0 for none, fPREC_AUTO for automatic
     \return            Whether successful, always true with String
    */
    template<class TNum>
    bool writenumf(TNum num, int precision=fPREC_AUTO) {
        Size len;
    #if defined(EVO_FNUMF_SPRINTF)
        char buf[impl::FNUMF_SPRINTF_BUF_SIZE];
        char* fmt;
        len = impl::fnumf_sprintf_setup(fmt, buf, num, precision);
        if (len > 0) {
            reserve(len + 1);
            ::sprintf(data_ + size_, fmt, num);
        }
    #else
        int exp = 0;
        if (precision < 0) {
            num = FloatT<TNum>::fexp10(exp, num);
            reserve(FloatT<TNum>::MAXDIGITS_AUTO + 1);
            len = impl::fnumfe(data_ + size_, num, exp, false);
        } else {
            num = FloatT<TNum>::fexp10(exp, impl::fnumf_weight(num, precision));
            reserve(FloatT<TNum>::maxdigits_prec(exp, precision) + 1);
            len = impl::fnumf(data_ + size_, num, exp, precision);
        }
    #endif
        buf_.header->used += len;
        size_             += len;
        return true;
    }

    /** Write (append) formatted and/or repeated character.
     \param  ch     Character to write
     \param  count  Character repeat count to use
     \param  field  Field attributes to use
     \return        Whether successful, always true with String
    */
    bool writefmtchar(char ch, Size count, const FmtSetField& field) {
        const Size index = size_;
        if (field.width >= 0 && (Size)field.width > count) {
            const int fillchar = (int)(field.fill != 0 ? (uchar)field.fill : (uchar)' ');
            const uint padding = field.width - count;
            ListType::advAdd(count + padding);

            char* p = data_ + index;
            switch (field.align) {
                case faCURRENT: // fallthrough
                case fLEFT:
                    memset(p, (int)(uchar)ch, count);
                    if (padding > 0)
                        memset(p + count, fillchar, padding);
                    break;
                case fCENTER: {
                    const uint padleft = (padding / 2);
                    if (padleft > 0) {
                        memset(p, fillchar, padleft);
                        p += padleft;
                    }
                    memset(p, (int)(uchar)ch, count);
                    const uint padright = (padding - padleft);
                    if (padright > 0) {
                        p += count;
                        memset(p, fillchar, padright);
                    }
                    break;
                }
                case fRIGHT:
                    if (padding > 0) {
                        memset(p, fillchar, padding);
                        p += padding;
                    }
                    memset(p, (int)(uchar)ch, count);
                    break;
            };
        } else if (count > 0) {
            ListType::advAdd(count);
            memset(data_ + index, (int)(uchar)ch, count);
        }
        return true;
    }

    /** Write (append) text with field alignment.
     \param  str    %String buffer to write from
     \param  size   Size to write in bytes
     \param  field  Field attributes to use
     \return        Whether successful, always true with String
    */
    bool writefmtstr(const char* str, Size size, const FmtSetField& field) {
        const Size index = size_;
        if (field.width >= 0 && (Size)field.width > size) {
            const int fillchar = (int)(field.fill != 0 ? (uchar)field.fill : (uchar)' ');
            const uint padding = field.width - size;
            ListType::advAdd(size + padding);

            char* p = data_ + index;
            switch (field.align) {
                case faCURRENT: // fallthrough
                case fLEFT:
                    memcpy(p, str, size);
                    if (padding > 0)
                        memset(p + size, fillchar, padding);
                    break;
                case fCENTER: {
                    const uint padleft = (padding / 2);
                    if (padleft > 0) {
                        memset(p, fillchar, padleft);
                        p += padleft;
                    }
                    memcpy(p, str, size);
                    const uint padright = (padding - padleft);
                    if (padright > 0) {
                        p += size;
                        memset(p, fillchar, padright);
                    }
                    break;
                }
                case fRIGHT:
                    if (padding > 0) {
                        memset(p, fillchar, padding);
                        p += padding;
                    }
                    memcpy(p, str, size);
                    break;
            };
        } else if (size > 0) {
            ListType::advAdd(size);
            memcpy(data_ + index, str, size);
        }
        return true;
    }

    /** Write (append) formatted signed number with field alignment.
     \tparam  TNum  Number type, inferred by param
     \param  num    Number to write
     \param  fmt    Integer formatting attributes to use
     \param  field  Field formatting attributes to use, NULL for none
     \return        Whether successful, always true with String
    */
    template<class TNum>
    bool writefmtnum(TNum num, const FmtSetInt& fmt, const FmtSetField* field=NULL) {
        if (fmt.base <= 0 || fmt.base == fDEC) {
            const Size index        = size_;
            const int digits        = IntegerT<TNum>::digits(num, fDEC);
            const int width         = (fmt.pad_width > digits ? fmt.pad_width : digits);
            const int align_padding = (field != NULL && field->width > width ? field->width - width : 0);
            ListType::advAdd(width + align_padding);
            fmt.impl_num_write(data_ + index, num, digits, width, align_padding, field);
        } else
            writefmtnumu((typename ToUnsigned<TNum>::Type)num, fmt, field);
        return true;
    }

    /** Write (append) formatted unsigned number with field alignment.
     \tparam  TNum  Number type, inferred by param
     \param  num    Number to write
     \param  fmt    Integer formatting attributes to use
     \param  field  Field formatting attributes to use, NULL for none
     \return        Whether successful, always true with String
    */
    template<class TNum>
    bool writefmtnumu(TNum num, const FmtSetInt& fmt, const FmtSetField* field=NULL) {
        const int base = (fmt.base > 0 ? fmt.base : fDEC);

        char prefix_ch  = 0;
        uint prefix_len = 0;
        fmt.impl_prefix_info(prefix_ch, prefix_len);

        const int digits        = IntegerT<TNum>::digits(num, base);
        const int width         = (fmt.pad_width > digits ? fmt.pad_width : digits);
        const int full_width    = width + prefix_len;
        const int align_padding = (field != NULL && field->width > full_width ? field->width - full_width : 0);

        const Size index = size_;
        ListType::advAdd(full_width + align_padding);
        char* p = data_ + index;

        int align_padleft, align_padright;
        FmtSetField::setup_align(align_padleft, align_padright, align_padding, field);

        if (align_padleft > 0) {
            memset(p, (int)(uchar)field->fill, align_padleft);
            p += align_padleft;
        }

        FmtSetInt::impl_prefix_write(p, prefix_ch, prefix_len);

        if (digits < width) {
            const uint padlen = width - digits;
            const int ch = (fmt.pad_ch == 0 ? '0' : (int)(uchar)fmt.pad_ch);
            memset(p, ch, padlen);
            p += padlen;
        }
        p += digits;
        impl::fnumu(p, num, base);

        if (align_padright > 0)
            memset(p, (int)(uchar)field->fill, align_padright);
        return true;
    }

    /** Write (append) formatted floating point number with field alignment.
     \tparam  TNum  Number type, inferred by param
     \param  num    Number to write
     \param  fmt    Floating point formatting attributes to use
     \param  field  Field formatting attributes to use, NULL for none
     \return       Whether successful, always true with String
    */
    template<class TNum>
    bool writefmtnumf(TNum num, const FmtSetFloat& fmt, const FmtSetField* field=NULL) {
        const int align_width = (field != NULL ? field->width : 0);
        int exp = 0, maxlen;
        fmt.impl_info(num, exp, maxlen, align_width);   // sets maxlen
        reserve(maxlen);

        const ulong len = fmt.impl_write(data_ + size_, num, exp, align_width, field);
        buf_.header->used += len;
        size_             += len;
        return true;
    }

    /** Write formatted data dump.
     - Output may span multiple lines, and always ends with a newline (unless dump data is empty)
     .
     \param  fmt          Format data, including buffer to dump
     \param  newline      Newline string to use
     \param  newlinesize  Size of newline string in bytes (max 2)
     \return       Whether successful, always true with String
    */
    bool writefmtdump(const FmtDump& fmt, const char* newline, uint newlinesize) {
        assert( newlinesize <= 2 );
        if (fmt.size > 0) {
            const char* DIGITS     = (fmt.upper ? "0123456789ABCDEF" : "0123456789abcdef");
            const ulong LINE_SIZE  = (fmt.maxline > 0 ? fmt.maxline : fmt.size);

            const uchar* ptr     = (uchar*)fmt.buf;
            const uchar* ptr_end = ptr + fmt.size;
            const uchar* ptr_nl;

            FmtSetInt offset_fmt(fHEX, 0);
            ulong offset = 0;
            if (fmt.maxline > 0 && !fmt.compact) {
                offset_fmt.pad_width = Int::digits(fmt.size, fHEX);
                const ulong lines = (fmt.size / fmt.maxline) + 1;
                reserve( ((offset_fmt.pad_width + 4 + (fmt.maxline * 4) + newlinesize) * lines) + 1 );
            } else
                reserve( (fmt.size * 4) + newlinesize + 1 );

            // Loop for each line
            int padding;
            for (const uchar* ptr2; ptr < ptr_end; ) {
                // Show offset
                if (fmt.maxline > 0 && !fmt.compact) {
                    padding = offset_fmt.pad_width - IntegerT<ulong>::digits(offset, fHEX);
                    if (padding > 0)
                        memset(data_ + size_, '0', padding);
                    size_ += offset_fmt.pad_width;
                    impl::fnumu(data_ + size_, offset, fHEX);

                    offset += fmt.maxline;
                    data_[size_++] = ':';
                    data_[size_++] = ' ';
                    data_[size_++] = ' ';
                }

                // Figure newline position
                ptr_nl = ptr + LINE_SIZE;
                if (ptr_nl > ptr_end)
                    ptr_nl = ptr_end;

                // Hex dump line
                ptr2 = ptr;
                for (; ptr < ptr_nl; ++ptr) {
                    data_[size_++] = DIGITS[(*ptr >> 4) & 0x0F];
                    data_[size_++] = DIGITS[*ptr & 0x0F];
                    data_[size_++] = ' ';
                }

                if (fmt.compact) {
                    assert( size_ > 0 );
                    --size_; // trim extra space from last byte
                } else {
                    if (ptr_nl >= ptr_end && fmt.maxline > 0 && ptr2 != (uchar*)fmt.buf) {
                        // Pad last line, add separator
                        const ulong remainder = fmt.size % fmt.maxline;
                        const ulong count = (remainder > 0 ? ((fmt.maxline - remainder) * 3) + 1 : 1);
                        if (count > 0) {
                            memset(data_ + size_, ' ', count);
                            size_ += count;
                        }
                    } else
                        // Separator
                        data_[size_++] = ' ';

                    // ASCII dump line
                    for (; ptr2 < ptr_nl; ++ptr2) {
                        if (*ptr2 < ' ' || *ptr2 > '~')
                            data_[size_++] = '.';
                        else
                            data_[size_++] = (char)*ptr2;
                    }
                }

                // Newline
                memcpy(data_ + size_, newline, newlinesize);
                size_ += newlinesize;
            }
            buf_.header->used = size_;
            assert( buf_.header->used <= buf_.header->size );
        }
        return true;
    }

    /** Write formatted data dump.
     - Output may span multiple lines, and always ends with a newline (unless dump data is empty)
     .
     \param  fmt  Format data, including buffer to dump
     \param  nl   Newline type to use, \ref NL or \ref NL_SYS for system default
     \return      Whether successful, always true with String
    */
    bool writefmtdump(const FmtDump& fmt, Newline nl=NL_SYS) {
        writefmtdump(fmt, getnewline(nl), getnewlinesize(nl));
        return true;
    }

    // CONVERT

    /** %Convert string to value of given type.
     - Advanced: Custom conversions can be defined by specializing a template instance of Convert -- see \ref StringCustomConversion "Custom String Conversion/Formatting"
     .
     \tparam  C  Type to convert to
     \return  Converted value
     */
    template<class C> C convert() const
        // An "undefined reference" compiler error pointing here means the given conversion isn't implemented/supported
        { return Convert<String,C>::value(*this); }

    /** %Convert value to string, replacing current string.
     - Advanced: Custom conversions can be defined by specializing a template instance of Convert -- see \ref StringCustomConversion "Custom String Conversion/Formatting"
     .
     \tparam  C  Type to convert from, inferred from argument
     \param  value  Value to convert
    */
    template<class C> String& convert_set(C value)
        // An "undefined reference" compiler error pointing here means the given conversion isn't implemented/supported
        { Convert<String,C>::set(*this, value); return *this; }

    /** %Convert value to string, appending to current string.
     - Advanced: Custom conversions can be defined by specializing a template instance of Convert -- see \ref StringCustomConversion "Custom String Conversion/Formatting"
     .
     \tparam  C  Type to convert from, inferred from argument
     \param  value  Value to convert
    */
    template<class C> String& convert_add(C value)
        // An "undefined reference" compiler error pointing here means the given conversion isn't implemented/supported
        { Convert<String,C>::add(*this, value); return *this; }

    /** %Convert value to string with quoting as needed, appending to current string.
     - Advanced: Custom conversions can be defined by specializing a template instance of Convert -- see \ref StringCustomConversion "Custom String Conversion/Formatting"
     .
     \tparam  C  Type to convert from, inferred from argument
     \param  value  Value to convert
     \param  delim  Delimiter to use to determine quoting needed -- this is not added to string
     \return        Whether successful, false if input is a string and is not quotable (invalid text)
    */
    template<class C> bool convert_addq(C value, char delim=',')
        // An "undefined reference" compiler error pointing here means the given conversion isn't implemented/supported
        { return Convert<String,C>::addq(*this, value, delim); }

    /** Join list items into delimited string.
     - This adds each item to string with delimiters, effectively using convert_add() for conversion
     - Delimiter is added before each item except the first
     - List item type must be convertible to String via convert_add()
     .
     \param  items  List items to join
     \param  delim  Delimiter to use
    */
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
    template<class C> String& joinmap(const C& map, char delim=',', char kvdelim='=') {
        typename C::Iter iter(map);
        for (typename C::Size i=0; iter; ++iter, ++i) {
            if (i > 0)
                add(delim);
            Convert<String,typename C::Key>::add(*this, iter->first);
            add(kvdelim);
            Convert<String,typename C::Value>::add(*this, iter->second);
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
    template<class C> String& joinmapq(const C& map, char delim=',', char kvdelim='=') {
        typename C::Iter iter(map);
        for (typename C::Size i=0; iter; ++iter, ++i) {
            if (i > 0)
                add(delim);
            Convert<String,typename C::Key>::addq(*this, iter->first, kvdelim);
            add(kvdelim);
            Convert<String,typename C::Value>::addq(*this, iter->second, delim);
        }
        return *this;
    }

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

    // CASE

    /** %Convert all lowercase characters in string to uppercase (modifier).
     - This recognizes standard ASCII codes 0-127
     - This does not use any locale information
     .
     \return  This
    */
    ThisType& toupper() {
        bool found = false;
        Size index = 0;
        for (; index < size_; ++index) {
            if (ascii_type(data_[index]) == ctLOWER) {
                found = true;
                break;
            }
        }
        if (found) {
            unshare();
            for (; index < size_; ++index)
                data_[index] = ascii_toupper(data_[index]);
        }
        return *this;
    }

    /** %Convert all uppercase characters in string to lowercase (modifier).
     - This recognizes standard ASCII codes 0-127
     - This does not use any locale information
     .
     \return  This
    */
    ThisType& tolower() {
        bool found = false;
        Size index = 0;
        for (; index < size_; ++index) {
            if (ascii_type(data_[index]) == ctUPPER) {
                found = true;
                break;
            }
        }
        if (found) {
            unshare();
            for (; index < size_; ++index)
                data_[index] = ascii_tolower(data_[index]);
        }
        return *this;
    }

    // OVERRIDES

    /** \copydoc ListType::clear() */
    String& clear()
        { ListType::clear(); return *this; }

    /** \copydoc ListType::triml(Size) */
    String& triml(Size size)
        { ListType::triml(size); return *this; }

    /** \copydoc ListType::trimr(Size) */
    String& trimr(Size size)
        { ListType::trimr(size); return *this; }

    /** \copydoc ListType::truncate(Size) */
    String& truncate(Size size=0)
        { ListType::truncate(size); return *this; }

    /** \copydoc ListType::slice(Key) */
    String& slice(Key index)
        { ListType::slice(index); return *this; }

    /** \copydoc ListType::slice(Key,Size) */
    String& slice(Key index, Size size)
        { ListType::slice(index, size); return *this; }

    /** \copydoc ListType::slice2(Key,Key) */
    String& slice2(Key index1, Key index2)
        { ListType::slice2(index1, index2); return *this; }

    /** \copydoc ListType::unslice() */
    String& unslice()
        { ListType::unslice(); return *this; }

    /** \copydoc evo::List::capacity() const */
    Size capacity() const
        { return ListType::capacity(); }

    /** \copydoc ListType::capacity(Size) */
    String& capacity(Size size)
        { ListType::capacity(size); return *this; }

    /** \copydoc ListType::capacitymin(Size) */
    String& capacitymin(Size min)
        { ListType::capacitymin(min); return *this; }

    /** \copydoc ListType::capacitymax(Size) */
    String& capacitymax(Size max)
        { ListType::capacitymax(max); return *this; }

    /** \copydoc ListType::compact() */
    String& compact()
        { ListType::compact(); return *this; }

    /** \copydoc ListType::reserve(Size,bool) */
    String& reserve(Size size, bool prefer_realloc=false)
        { ListType::reserve(size, prefer_realloc); return *this; }

    /** \copydoc ListType::unshare() */
    String& unshare()
        { ListType::unshare(); return *this; }

    /** \copydoc ListType::resize(Size) */
    String& resize(Size size)
        { ListType::resize(size); return *this; }

    /** \copydoc ListType::reverse() */
    String& reverse()
        { ListType::reverse(); return *this; }

    /** \copydoc ListType::advResize(Size) */
    String& advResize(Size size)
        { ListType::advResize(size); return *this; }

    // STATIC

    /** Get string of all ASCII alphanumeric digits with uppercase letters (0-9, A-Z).
     \return  %String of digits
    */
    static const String& digits()
        { static const String str("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"); return str; }

    /** Get string of all ASCII alphanumeric digits with lowercase letters (0-9, a-z).
     \return  %String of digits
    */
    static const String& digitsl()
        { static const String str("0123456789abcdefghijklmnopqrstuvwxyz"); return str; }

    /** Get string of ASCII letters (A-z).
     \return  %String of letters
    */
    static const String& letters()
        { static const String str("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"); return str; }

    /** Get string of ASCII uppercase letters (A-Z).
     \return  %String of letters
    */
    static const String& lettersu()
        { static const String str("ABCDEFGHIJKLMNOPQRSTUVWXYZ"); return str; }

    /** Get string of ASCII lowercase letters (a-z).
     \return  %String of letters
    */
    static const String& lettersl()
        { static const String str("abcdefghijklmnopqrstuvwxyz"); return str; }

    /** Get string of ASCII whitespace characters (space, tab).
     \return  %String of whitespace
    */
    static const String& whitespace()
        { static const String str(" \t"); return str; }

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
#if defined(DOXYGEN)
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
    void setnum(T num, int base=fDEC) {
        clear().capacitymin(IntegerT<T>::maxlen(base)+1).unshare();
        const Size len = impl::fnum(data_+IntegerT<T>::digits(num,base), num, base);
        buf_.header->used += len;
        size_             += len;
    }
    template<class T>
    void setnumu(T num, int base=fDEC) {
        clear().capacitymin(IntegerT<T>::maxlen(base)+1).unshare();
        const Size len = impl::fnumu(data_+IntegerT<T>::digits(num,base), num, base);
        buf_.header->used += len;
        size_             += len;
    }
    template<class T>
    void setnumf(T num, int precision=fPREC_AUTO) {
        Size len;
    #if defined(EVO_FNUMF_SPRINTF)
        char buf[impl::FNUMF_SPRINTF_BUF_SIZE];
        char* fmt;
        len = impl::fnumf_sprintf_setup(fmt, buf, num, precision);
        if (len > 0) {
            clear().capacitymin(len + 1).unshare();
            ::sprintf(data_, fmt, num);
        }
    #else
        int exp = 0;
        if (precision < 0) {
            num = FloatT<T>::fexp10(exp, num);
            clear().capacitymin(FloatT<T>::MAXDIGITS_AUTO).unshare();
            len = impl::fnumfe(data_, num, exp, false);
        } else {
            num = FloatT<T>::fexp10(exp, impl::fnumf_weight(num, precision));
            clear().capacitymin(FloatT<T>::maxdigits_prec(exp, precision)).unshare();
            len = impl::fnumf(data_, num, exp, precision);
        }
    #endif
        buf_.header->used += len;
        size_             += len;
    }

    // Prepend number
    template<class T>
    void prependnum(T num, int base=fDEC) {
        StringInt<T> str(num, base, false);
        prepend(str.data(), str.size());
    }
    template<class T>
    void prependnumf(T num, int precision=fPREC_AUTO) {
        StringFlt<T> str(num, precision, false);
        prepend(str.data(), str.size());
    }

    // Insert number
    template<class T>
    void insertnum(Key index, T num, int base=fDEC) {
        StringInt<T> str(num, base, false);
        insert(index, str.data(), str.size());
    }
    template<class T>
    void insertnumf(Key index, T num, int precision=fPREC_AUTO) {
        StringFlt<T> str(num, precision, false);
        insert(index, str.data(), str.size());
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
    template<class V> static bool addq(String& dest, V value, char)
        { dest.addn(value); return true; }
    static T value(const String& src)
        { return src.getnum<T>(); }
};
template<class T> struct Convert_StringToFltBase {
    template<class V> static void set(String& dest, V value)
        { dest.setn(value); }
    template<class V> static void add(String& dest, V value)
        { dest.addn(value); }
    template<class V> static bool addq(String& dest, V value, char)
        { dest.addn(value); return true; }
    static T value(const String& src)
        { return src.getnumf<T>(); }
};
template<class T> struct Convert_StringToCIntBase {
    template<class V> static void set(String& dest, const V& value)
        { if (value.valid()) dest.setn(*value); }
    template<class V> static void add(String& dest, const V& value)
        { if (value.valid()) dest.addn(*value); }
    template<class V> static bool addq(String& dest, const V& value, char)
        { if (value.valid()) dest.addn(*value); return true; }
    static T value(const String& src)
        { return src.getnum<T>(); }
};
template<class T> struct Convert_StringToCFltBase {
    template<class V> static void set(String& dest, const V& value)
        { if (value.valid()) dest.setn(*value); }
    template<class V> static void add(String& dest, const V& value)
        { if (value.valid()) dest.addn(*value); }
    template<class V> static bool addq(String& dest, const V& value, char)
        { if (value.valid()) dest.addn(*value); return true; }
    static T value(const String& src)
        { return src.getnumf<T>(); }
};
// Conversion templates
template<> struct Convert<const char*,String> {
    static void set(String& dest, const char* value)
        { dest.set(value); }
    static void add(String& dest, const char* value)
        { dest.add(value); }
    static String value(const char* src)
        { return src; }
};
template<> struct Convert<char*,String> {
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
    static bool addq(String& dest, const String& value, char delim)
        { return (value.size() == 0 || dest.writequoted(value.data(), value.size(), delim, true) > 0); }
    static const String& value(const String& src)
        { return src; }
};
template<> struct Convert<String,const char*> {
    static void set(String& dest, const char* value)
        { dest.set(value); }
    static void add(String& dest, const char* value)
        { dest.add(value); }
    static bool addq(String& dest, const char* value, char delim) {
        const StrSizeT len = (StrSizeT)strlen(value);
        return (len == 0 || dest.writequoted(value, (StrSizeT)strlen(value), delim, true) > 0);
    }
    // Unsafe: Converting String to const char*
    //template<class U> static const char* value(U&)
};
template<> struct Convert<String,char*> {
    static void set(String& dest, const char* value)
        { dest.set(value); }
    static void add(String& dest, const char* value)
        { dest.add(value); }
    static bool addq(String& dest, const char* value, char delim) {
        const StrSizeT len = (StrSizeT)strlen(value);
        return (len == 0 || dest.writequoted(value, (StrSizeT)strlen(value), delim, true) > 0);
    }
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
    static bool addq(String& dest, bool value, char) {
        if (value) dest.add("true", 4);
        else       dest.add("false", 5);
        return true;
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
    static bool addq(String& dest, Bool value, char) {
        if (value.valid()) {
            if (*value) dest.add("true", 4);
            else        dest.add("false", 5);
        }
        return true;
    }
    static Bool value(const String& src)
        { return src.getbool<Bool>(); }
};
template<> struct Convert<String,Short>  : public Convert_StringToCIntBase<Short>   { };
template<> struct Convert<String,Int>    : public Convert_StringToCIntBase<Int>     { };
template<> struct Convert<String,Long>   : public Convert_StringToCIntBase<Long>    { };
template<> struct Convert<String,LongL>  : public Convert_StringToCIntBase<LongL>   { };
template<> struct Convert<String,UShort> : public Convert_StringToCIntBase<UShort>  { };
template<> struct Convert<String,UInt>   : public Convert_StringToCIntBase<UInt>    { };
template<> struct Convert<String,ULong>  : public Convert_StringToCIntBase<ULong>   { };
template<> struct Convert<String,ULongL> : public Convert_StringToCIntBase<ULongL>  { };
template<> struct Convert<String,Float>  : public Convert_StringToCFltBase<Float>   { };
template<> struct Convert<String,FloatD> : public Convert_StringToCFltBase<FloatD>  { };
template<> struct Convert<String,FloatL> : public Convert_StringToCFltBase<FloatL>  { };

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
}
#endif
