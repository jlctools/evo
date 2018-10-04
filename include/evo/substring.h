// Evo C++ Library
/* Copyright 2018 Justin Crowell
Distributed under the BSD 2-Clause License -- see included file LICENSE.txt for details.
*/
///////////////////////////////////////////////////////////////////////////////
/** \file substring.h Evo SubString container. */
#pragma once
#ifndef INCL_evo_substring_h
#define INCL_evo_substring_h

#include "sublist.h"
#include "string.h"

namespace evo {
/** \addtogroup EvoContainers */
//@{

///////////////////////////////////////////////////////////////////////////////

/** Reference and access existing string data.

Use to explictly reference string data, split/tokenize string data, and to convert to other types.

\par Features

 - Similar to STL string_view from C++17
 - This provides read-only methods similar to String, but with reduced overhead
   - This is useful for referencing static/immutable data, or when performance or size is critical
   - String is generally safer, though SubString is smaller and slightly faster

 - This does not own, allocate, or free any resources
 - \b Caution: SubString can be unsafe since it references a string data pointer, which must remain valid
 - See also: StrTok, String, UnicodeString
 .

Note: including this automatically includes evo/string.h

\par Iterators

 - SubString::Iter -- Read-Only Iterator (IteratorRa)
 .

\par Constructors

 - SubString()
 - SubString(const SubString&)
 - SubString(const StringBase&)
 - SubString(const StringBase*)
 - SubString(const char*,Size)
 - SubString(const char*)
 .

\par Read Access

 - size()
   - null(), empty()
   - shared()
 - cstr()
 - data()
   - item()
   - operator[]()
   - first(), last(), iend()
   - hash()
 .
 - split(char,T1&,T2&) const
   - split(char,T1&) const
   - split(char,ValNull,T2&) const
 - splitr(char,T1&,T2&) const
   - splitr(char,T1&) const
   - splitr(char,ValNull,T2&) const
 - splitat(Key,T1&,T2&) const
   - splitat(Key,T1&) const
   - splitat(Key,ValNull,T2&) const
 - find(char,Key,Key) const, ...
   - find(const char*,uint,Key,Key) const, ...
   - find(const StringBase&,Key,Key) const, ...
 - findr(char,Key,Key) const, ...
   - findr(const char*,uint,Key,Key) const, ...
   - findr(const StringBase&,Key,Key) const, ...
 - findany()
   - findanyr()
 - findanybut()
   - findanybutr()
 - contains(char) const
   - contains(const char*,Size) const
   - contains(const StringBase&) const
 .
 - compare(const StringBase&) const
   - operator==(const StringBase&) const
   - operator==(const char*) const
   - operator!=(const StringBase&) const
   - operator!=(const char*) const
 - starts(ItemVal) const
   - starts(const char*,Size) const
   - starts(const char*) const
   - ends(ItemVal) const
   - ends(const char*,Size) const
   - ends(const char*) const
 .
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

\par Modifiers

 - set()
   - set(const StringBase&)
   - set(const StringBase&,Key,Key)
   - set(const StringBase*)
   - set(const char*,Size)
   - set(const char*)
   - set2(const StringBase&,Key,Key)
   - clear()
   - operator=(const StringBase&)
   - operator=(const StringBase*)
   - operator=(const char*)
   - operator=(const ValNull&)
   - operator=(const ValEmpty&)
 - convert_set()
 - token()
   - tokenr()
   - token_any()
   - tokenr_any()
 - truncate()
   - triml(), trimr()
 - strip()
   - strip(char)
   - stripl(), stripr()
   - stripl(char,Size), stripr(char,Size)
   - stripl(const char*,Size,Size), stripr(const char*,Size,Size)
 - splitat_setl(Key)
   - splitat_setl(Key,T2&)
 - splitat_setr(Key)
   - splitat_setr(Key,T1&)
 - slice(Key)
   - slice(Key,Size), slice2()
 - unshare()
 - swap()
 .

\par Example

\code
#include <evo/substring.h>
#include <evo/io.h>
using namespace evo;
static Console& c = con();

int main() {
    // Create substring from string literal
    SubString str("foo,123");

    // operator[] provides read-only (const) access
    char value = str[0];
    //str[0] = 0;       // Error: operator[] is read-only

    // Iterate and print characters (read-only)
    for (SubString::Iter iter(str); iter; ++iter)
        c.out << "Ch: " << *iter << NL;

    // Split substring into 2 substrings
    SubString sub1, sub2;
    str.split(',', sub1, sub2);

    // Print sub1, and sub2 as number (dereference Int to int)
    c.out << "1: " << sub1 << NL
          << "2: " << *sub2.num() << NL;

    // Use for formatting substring as terminated string
    String tmp;

    // Print sub1 as terminated string (just for example)
    c.out << sub1.cstr(tmp) << NL;

    return 0;
}
\endcode

Output:
\code{.unparsed}
Ch: f
Ch: o
Ch: o
Ch: ,
Ch: 1
Ch: 2
Ch: 3
1: foo
2: 123
foo
\endcode
*/
struct SubString : public SubList<char,StrSizeT> {
    typedef SubString ThisType;             ///< This string type
    typedef ListBaseType StringBase;        ///< Alias for ListBaseType

    /** Default constructor sets as null. */
    SubString()
        { }

    /** Copy constructor.
     \param  str  %String to copy
    */
    SubString(const ThisType& str) : SubList<char,StrSizeT>(str)
        { }

    /** Copy constructor.
     \param  str  %String to copy
    */
    SubString(const StringBase& str) : SubList<char,StrSizeT>(str)
        { }

    /** Copy constructor.
     \param  str  Pointer to string to copy, NULL to set as null
    */
    SubString(const StringBase* str) {
        if (str != NULL)
            set(*str);
    }

    /** Constructor to reference string data.
     \param  data  %String data to reference
     \param  size  %String size in bytes
    */
    SubString(const char* data, Size size) : SubList<char,StrSizeT>(data, size)
        { }

    /** Constructor to reference terminated string.
     \param  data  String data to reference -- must be terminated
    */
    SubString(const char* data) : SubList<char,StrSizeT>(data, data?(Size)strlen(data):0)
        { }

    // SET

    /** \copydoc evo::SubList::operator=(const ListBaseType&) */
    SubString& operator=(const StringBase& data)
        { set(data); return *this; }

    /** \copydoc evo::SubList::operator=(const ListBaseType*) */
    SubString& operator=(const StringBase* data)
        { if (data != NULL) set(*data); else set(); return *this; }

    /** Assignment operator sets as reference to terminated string.
     - This will reference the same pointer as given data
     - \b Caution: Source data pointer must remain valid
     .
     \param  data  Data to reference
     \return       This
    */
    SubString& operator=(const char* data) {
        data_ = (char*)data;
        size_ = data ? (Size)strlen(data) : 0;
        return *this;
    }

    using SubListType::set;

    /** %Set as reference to terminated string.
     - This will reference the same pointer as given data
     - \b Caution: Source data pointer must remain valid
     .
     \param  data  Data to reference -- must be terminated
     \return       This
    */
    SubString& set(const char* data) {
        data_ = (char*)data;
        size_ = data ? (Size)strlen(data) : 0;
        return *this;
    }

    /** \copydoc evo::SubList::operator=(const ValNull&) */
    SubString& operator=(const ValNull& val) {
        EVO_PARAM_UNUSED(val);
        set(); return *this;
    }

    /** \copydoc evo::SubList::operator=(const ValEmpty&) */
    SubString& operator=(const ValEmpty& val) {
        EVO_PARAM_UNUSED(val);
        setempty(); return *this;
    }

    /** Extract next token from string.
     - If delim is found, the token value up to that delim is extracted
     - If delim isn't found, the whole string is extracted
     - The extracted token is removed from this string, including the delim (if found)
     - See also: StrTok (and variants)
     .
     \param  value  %Set to next token value, or null if none  [out]
     \param  delim  Delimiter to tokenize on
     \return        Whether next token was found, false if current string is empty
    */
    bool token(SubString& value, char delim) {
        if (size_ > 0) {
            for (Key i=0; i < size_; ++i) {
                if (data_[i] == delim) {
                    value.set(data_, i);
                    ++i;
                    data_ += i;
                    size_ -= i;
                    return true;
                }
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
     \param  value  %Set to next token value, or null if none  [out]
     \param  delim  Delimiter to tokenize on
     \return        Whether next token was found, false if current string is empty
    */
    bool tokenr(SubString& value, char delim) {
        if (size_ > 0) {
            for (Key i=size_; i > 0; ) {
                if (data_[--i] == delim) {
                    Size len = size_ - i;
                    value.set(data_ + i + 1, len - 1);
                    size_ -= len;
                    return true;
                }
            }
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
     \param  value        %Set to next token value, or null if none  [out]
     \param  found_delim  %Set to delimited found, null if no delim found  [out]
     \param  delims       Delimiters to search for
     \param  count        Count of delimiters to search for, must be positive
     \return              Whether next token was found, false if current string is empty
    */
    bool token_any(SubString& value, Char& found_delim, const char* delims, Size count) {
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
     \param  value        %Set to next token value, or null if none  [out]
     \param  found_delim  %Set to delimited found, null if no delim found  [out]
     \param  delims       Delimiters to search for
     \param  count        Count of delimiters to search for, must be positive
     \return              Whether next token was found, false if current string is empty
    */
    bool tokenr_any(SubString& value, Char& found_delim, const char* delims, Size count) {
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

    // INFO

    /** Get terminated string pointer, using given string buffer if needed (const).
     - \b Caution: Modifying buffer may (will) invalidate the returned pointer
     .
     \param  buffer  Buffer to use, if needed
     \return         Terminated string pointer
    */
    const char* cstr(String& buffer) const
        { return (size_ > 0 ? buffer.set(data_, size_).cstr() : ""); }

    // COMPARE

    using SubListType::compare;

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

    using SubListType::operator==;
    using SubListType::operator!=;

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

    /** \copydoc evo::String::operator==(const char*) const */
    bool operator==(const char* str) const {
        return (utf8_compare(data_, size_, str) == 0);
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

    /** \copydoc evo::String::operator!=(const char*) const */
    bool operator!=(const char* str) const {
        return (utf8_compare(data_, size_, str) != 0);
    }

    using SubListType::starts;

    /** Check if starts with given terminated string.
     \param  str  %String to check for -- must be terminated
     \return      Whether starts with string
    */
    bool starts(const char* str) const {
        if (str == NULL)
            return false;
        const SizeT size = (Size)strlen(str);
        return (size > 0 && size_ >= size && DataEqual<Item>::equal(data_, str, size));
    }

    using SubListType::ends;

    /** Check if ends with given terminated string.
     \param  str  %String to check for -- must be terminated
     \return      Whether ends with string
    */
    bool ends(const char* str) const {
        if (str == NULL)
            return false;
        const SizeT size = (Size)strlen(str);
        return (size > 0 && size_ >= size && DataEqual<Item>::equal(data_+size_-size, str, size));
    }

    // FIND

    /** \copydoc evo::String::find(char) const */
    Key find(char ch) const {
        if (size_ > 0) {
            const char* ptr = (char*)memchr(data_, ch, size_);
            if (ptr != NULL)
                return (Key)(ptr - data_);
        }
        return NONE;
    }

    /** \copydoc evo::String::find(char,Key,Key) const */
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

    /** \copydoc evo::String::find(const char*,uint,Key,Key) const */
    Key find(const char* pattern, uint pattern_size, Key start=0, Key end=END) const {
        if (start < size_ && start < end) {
            assert( pattern != NULL );
            if (end > size_)
                end = size_;
            return impl::string_search(pattern, pattern_size, data_ + start, end - start, start);
        }
        return NONE;
    }

    /** \copydoc evo::String::find(StringSearchAlg,const char*,uint,Key,Key) const */
    Key find(StringSearchAlg alg, const char* pattern, uint pattern_size, Key start=0, Key end=END) const {
        if (start < size_ && start < end) {
            assert( pattern != NULL );
            if (end > size_)
                end = size_;
            return impl::string_search(alg, pattern, pattern_size, data_ + start, end - start, start);
        }
        return NONE;
    }

    /** \copydoc evo::String::find(const StringBase&,Key,Key) const */
    Key find(const StringBase& pattern, Key start=0, Key end=END) const {
        if (start < size_ && start < end) {
            if (end > size_)
                end = size_;
            return impl::string_search(pattern.data_, pattern.size_, data_ + start, end - start, start);
        }
        return NONE;
    }

    /** \copydoc evo::String::find(StringSearchAlg,const StringBase&,Key,Key) const */
    Key find(StringSearchAlg alg, const StringBase& pattern, Key start=0, Key end=END) const {
        if (start < size_ && start < end) {
            if (end > size_)
                end = size_;
            return impl::string_search(alg, pattern.data_, pattern.size_, data_ + start, end - start, start);
        }
        return NONE;
    }

    /** \copydoc evo::String::findr(char) const */
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

    /** \copydoc evo::String::findr(char,Key,Key) const */
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

    /** \copydoc evo::String::findr(const char*,uint,Key,Key) const */
    Key findr(const char* pattern, uint pattern_size, Key start=0, Key end=END) const {
        if (start < size_ && start < end) {
            assert( pattern != NULL );
            if (end > size_)
                end = size_;
            return impl::string_search_reverse(pattern, pattern_size, data_ + start, end - start, start);
        }
        return NONE;
    }

    /** \copydoc evo::String::findr(StringSearchAlg,const char*,uint,Key,Key) const */
    Key findr(StringSearchAlg alg, const char* pattern, uint pattern_size, Key start=0, Key end=END) const {
        if (start < size_ && start < end) {
            assert( pattern != NULL );
            if (end > size_)
                end = size_;
            return impl::string_search_reverse(alg, pattern, pattern_size, data_ + start, end - start, start);
        }
        return NONE;
    }

    /** \copydoc evo::String::findr(const StringBase&,Key,Key) const */
    Key findr(const StringBase& pattern, Key start=0, Key end=END) const {
        if (start < size_ && start < end) {
            if (end > size_)
                end = size_;
            return impl::string_search_reverse(pattern.data_, pattern.size_, data_ + start, end - start, start);
        }
        return NONE;
    }

    /** \copydoc evo::String::findr(StringSearchAlg,const StringBase&,Key,Key) const */
    Key findr(StringSearchAlg alg, const StringBase& pattern, Key start=0, Key end=END) const {
        if (start < size_ && start < end) {
            if (end > size_)
                end = size_;
            return impl::string_search_reverse(alg, pattern.data_, pattern.size_, data_ + start, end - start, start);
        }
        return NONE;
    }

    /** \copydoc evo::String::findany(const char*,Size,Key,Key) const */
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

    /** \copydoc evo::String::findany(const StringBase&,Key,Key) const */
    Key findany(const StringBase& chars, Key start=0, Key end=END) const
        { return findany(chars.data_, chars.size_, start, end); }

    /** \copydoc evo::String::findanyr(const char*,Size,Key,Key) const */
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

    /** \copydoc evo::String::findanyr(const StringBase&,Key,Key) const */
    Key findanyr(const StringBase& chars, Key start=0, Key end=END) const
        { return findanyr(chars.data_, chars.size_, start, end); }

    /** \copydoc evo::String::findanybut(const char*,Size,Key,Key) const */
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

    /** \copydoc evo::String::findanybut(const StringBase&,Key,Key) const */
    Key findanybut(const StringBase& chars, Key start=0, Key end=END) const
        { return findanybut(chars.data_, chars.size_, start, end); }

    /** \copydoc evo::String::findanybutr(const char*,Size,Key,Key) const */
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

    /** \copydoc evo::String::findanybutr(const StringBase&,Key,Key) const */
    Key findanybutr(const StringBase& chars, Key start=0, Key end=END) const
        { return findanybutr(chars.data_, chars.size_, start, end); }

    /** \copydoc evo::String::contains(char) const */
    bool contains(char ch) const
        { return find(ch) != NONE; }

    /** \copydoc evo::String::contains(const char*,Size) const */
    bool contains(const char* str, Size size) const 
        { return find(str, size) != NONE; }

    /** \copydoc evo::String::contains(const StringBase&) const */
    bool contains(const StringBase& str) const 
        { return find(str) != NONE; }
    
    // SPLIT

    /** \copydoc evo::String::split(char,T1&,T2&) const */
    template<class T1,class T2>
    bool split(char delim, T1& left, T2& right) const {
        for (Key i=0; i<size_; ++i) {
            if (data_[i] == delim) {
                left.set(*this, 0, i);
                right.set(*this, i+1, ALL);
                return true;
            }
        }
        left.set(*this);
        right.set();
        return false;
    }

    /** \copydoc evo::String::split(char,T1&) const */
    template<class T1>
    bool split(char delim, T1& left) const {
        for (Key i=0; i<size_; ++i) {
            if (data_[i] == delim) {
                left.set(*this, 0, i);
                return true;
            }
        }
        left.set(*this);
        return false;
    }

    /** \copydoc evo::String::split(char,ValNull,T2&) const */
    template<class T2>
    bool split(char delim, ValNull left, T2& right) const {
        EVO_PARAM_UNUSED(left);
        for (Key i=0; i<size_; ++i) {
            if (data_[i] == delim) {
                right.set(*this, i+1, ALL);
                return true;
            }
        }
        right.set();
        return false;
    }

    /** \copydoc evo::String::splitr(char,T1&,T2&) const */
    template<class T1,class T2>
    bool splitr(char delim, T1& left, T2& right) const {
        for (Key i=size_; i>0; ) {
            if (data_[--i] == delim) {
                left.set(*this, 0, i);
                right.set(*this, i+1, ALL);
                return true;
            }
        }
        left.set(*this);
        right.set();
        return false;
    }

    /** \copydoc evo::String::splitr(char,T1&) const */
    template<class T1>
    bool splitr(char delim, T1& left) const {
        for (Key i=size_; i>0; ) {
            if (data_[--i] == delim) {
                left.set(*this, 0, i);
                return true;
            }
        }
        left.set(*this);
        return false;
    }

    /** \copydoc evo::String::splitr(char,ValNull,T2&) const */
    template<class T2>
    bool splitr(char delim, ValNull left, T2& right) const {
        EVO_PARAM_UNUSED(left);
        for (Key i=size_; i>0; ) {
            if (data_[--i] == delim) {
                right.set(*this, i+1, ALL);
                return true;
            }
        }
        right.set();
        return false;
    }

    // TRIM/STRIP

    /** \copydoc evo::String::strip() */
    SubString& strip() {
        char ch;
        while ( size_ > 0 && ((ch=data_[size_-1]) == ' ' || ch == '\t') )
            --size_;
        Size count = 0;
        while ( count < size_ && ((ch=data_[count]) == ' ' || ch == '\t') )
            ++count;
        if (count > 0) {
            size_ -= count;
            data_ += count;
        }
        return *this;
    }

    /** \copydoc evo::String::strip(char) */
    SubString& strip(char ch) {
        while (size_ > 0 && data_[size_-1] == ch)
            --size_;
        Size count = 0;
        while (count < size_ && data_[count] == ch)
            ++count;
        if (count > 0) {
            size_ -= count;
            data_ += count;
        }
        return *this;
    }

    /** \copydoc evo::String::stripl() */
    SubString& stripl() {
        char ch;
        Size count = 0;
        while ( count < size_ && ((ch=data_[count]) == ' ' || ch == '\t') )
            ++count;
        if (count > 0) {
            size_ -= count;
            data_ += count;
        }
        return *this;
    }

    /** \copydoc evo::String::stripl(char,Size) */
    SubString& stripl(char ch, Size max=ALL) {
        Size count = 0;
        while (count < size_ && data_[count] == ch && count < max)
            ++count;
        if (count > 0) {
            size_ -= count;
            data_ += count;
        }
        return *this;
    }

    /** \copydoc evo::String::stripl(const char*,Size,Size) */
    SubString& stripl(const char* str, Size strsize, Size max=ALL) {
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

    /** \copydoc evo::String::stripr() */
    SubString& stripr() {
        char ch;
        while ( size_ > 0 && ((ch=data_[size_-1]) == ' ' || ch == '\t') )
            --size_;
        return *this;
    }

    /** \copydoc evo::String::stripr(char,Size) */
    SubString& stripr(char ch, Size max=ALL) {
        for (Size i=0; size_ > 0 && data_[size_-1] == ch && i < max; ++i)
            --size_;
        return *this;
    }

    /** \copydoc evo::String::stripr(const char*,Size,Size) */
    SubString& stripr(const char* str, Size strsize, Size max=ALL) {
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

    // OVERRIDES

    /** \copydoc evo::SubList::clear() */
    SubString& clear()
        { SubListType::clear(); return *this; }

    /** \copydoc evo::SubList::set() */
    SubString& set()
        { SubListType::set(); return *this; }

    /** \copydoc evo::SubList::set(const ListBaseType&) */
    SubString& set(const StringBase& data)
        { SubListType::set(data); return *this; }

    /** \copydoc evo::SubList::set(const ListBaseType&,Key,Key) */
    SubString& set(const StringBase& data, Key index, Key size=ALL)
        { SubListType::set(data, index, size); return *this; }

    /** \copydoc evo::SubList::set(const ListBaseType*) */
    SubString& set(const StringBase* data)
        { SubListType::set(data); return *this; }

    /** \copydoc evo::SubList::set(const Item*,Size) */
    SubString& set(const char* data, Size size)
        { SubListType::set(data, size); return *this; }

    /** \copydoc evo::SubList::set2(const ListBaseType&,Key,Key) */
    SubString& set2(const StringBase& data, Key index1, Key index2)
        { SubListType::set2(data, index1, index2); return *this; }

    /** \copydoc evo::SubList::setempty() */
    SubString& setempty()
        { SubListType::setempty(); return *this; }

    /** \copydoc evo::SubList::triml(Size) */
    SubString& triml(Size size)
        { SubListType::triml(size); return *this; }

    /** \copydoc evo::SubList::trimr(Size) */
    SubString& trimr(Size size)
        { SubListType::trimr(size); return *this; }

    /** \copydoc evo::SubList::truncate(Size) */
    SubString& truncate(Size size)
        { SubListType::truncate(size); return *this; }

    /** \copydoc evo::SubList::slice(Key) */
    SubString& slice(Key index)
        { SubListType::slice(index); return *this; }

    /** \copydoc evo::SubList::slice(Key,Size) */
    SubString& slice(Key index, Size size)
        { SubListType::slice(index, size); return *this; }

    /** \copydoc evo::SubList::slice2(Key,Key) */
    SubString& slice2(Key index1, Key index2)
        { SubListType::slice2(index1, index2); return *this; }

    /** \copydoc evo::SubList::unshare() */
    SubString& unshare()
        { return *this; }

    // GETBOOL

    /** \copydoc evo::String::getbool(Error&) const */
    bool getbool(Error& error) const {
        assert( data_ > (char*)1 || size_ == 0 );
        return impl::tobool(data_, size_, error);
    }

    /** \copydoc evo::String::getbool() const */
    template<class T> T getbool() const {
        assert( data_ > (char*)1 || size_ == 0 );
        return StaticIf< IsPodType<T>::value, impl::ToBoolPod<T>, impl::ToBool<T> >::Type::getbool(data_, size_);
    }

    // GETNUM

    /** \copydoc evo::String::getnum(Error&,int) const */
    template<class T> T getnum(Error& error, int base=0) const {
        STATIC_ASSERT( IsPodType<T>::value, getnum_POD_Type_Required );
        assert( data_ > (char*)1 || size_ == 0 );
        return impl::tonum<T>(data_, size_, error, base);
    }

    /** \copydoc evo::String::getnum(int) const */
    template<class T> T getnum(int base=0) const {
        assert( data_ > (char*)1 || size_ == 0 );
        return StaticIf< IsPodType<T>::value, impl::ToNumPod<T>, impl::ToNum<T> >::Type::getnum(data_, size_, base);
    }

    /** \copydoc evo::String::getnumf(Error&) const */
    template<class T> T getnumf(Error& error) const {
        STATIC_ASSERT( IsPodType<T>::value, getnumf_POD_Type_Required );
        assert( data_ > (char*)1 || size_ == 0 );
        return impl::tonumf<T>(data_, size_, error);
    }

    /** \copydoc evo::String::getnumf() const */
    template<class T> T getnumf() const {
        assert( data_ > (char*)1 || size_ == 0 );
        return StaticIf< IsPodType<T>::value, impl::ToNumfPod<T>, impl::ToNumf<T> >::Type::getnum(data_, size_);
    }

    // BOOLVAL

    /** \copydoc evo::String::boolval() const */
    Bool boolval() const
        { return impl::ToBool<Bool>::getbool(data_, size_); }

    // NUM

    /** \copydoc evo::String::num(int) const */
    Int num(int base=0) const
        { return impl::ToNum<Int>::getnum(data_, size_, base); }

    /** \copydoc evo::String::numl(int) const */
    Long numl(int base=0) const
        { return impl::ToNum<Long>::getnum(data_, size_, base); }

    /** \copydoc evo::String::numll(int) const */
    LongL numll(int base=0) const
        { return impl::ToNum<LongL>::getnum(data_, size_, base); }

    // NUMU

    /** \copydoc evo::String::numu(int) const */
    UInt numu(int base=0) const
        { return impl::ToNum<UInt>::getnum(data_, size_, base); }

    /** \copydoc evo::String::numul(int) const */
    ULong numul(int base=0) const
        { return impl::ToNum<ULong>::getnum(data_, size_, base); }

    /** \copydoc evo::String::numull(int) const */
    ULongL numull(int base=0) const
        { return impl::ToNum<ULongL>::getnum(data_, size_, base); }

    // NUMF

    /** \copydoc evo::String::numf() const */
    Float numf() const
        { return impl::ToNumf<Float>::getnum(data_, size_); }

    /** \copydoc evo::String::numfd() const */
    FloatD numfd() const
        { return impl::ToNumf<FloatD>::getnum(data_, size_); }

    /** \copydoc evo::String::numfl() const */
    FloatL numfl() const
        { return impl::ToNumf<FloatL>::getnum(data_, size_); }

    // CONVERT

    /** \copydoc evo::String::convert() const */
    template<class C> C convert() const
        // An "undefined reference" compiler error pointing here means the given conversion isn't implemented/supported
        { return Convert<SubString,C>::value(*this); }

    /** \copydoc evo::String::convert_set() */
    template<class C> SubString& convert_set(C value)
        // An "undefined reference" compiler error pointing here means the given conversion isn't implemented/supported
        { Convert<SubString,C>::set(*this, value); return *this; }

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

    // Doxygen: Inherited methods
#ifdef DOXYGEN
    /** \copydoc evo::SubList::data() const */
    const char* data() const;

    /** \copydoc evo::SubList::operator[](Key) const */
    const char& operator[](Key index) const;

    /** \copydoc evo::SubList::item(Key) const */
    const char& item(Key index) const;

    /** \copydoc evo::SubList::operator==(const ListBaseType&) const */
    bool operator==(const StringBase& data) const;

    /** \copydoc evo::SubList::operator!=(const ListBaseType&) const */
    bool operator!=(const StringBase& data) const;

    /** \copydoc evo::SubList::null() const */
    bool null() const;

    /** \copydoc evo::SubList::empty() const */
    bool empty() const;

    /** \copydoc evo::SubList::size() const */
    Size size() const;

    /** \copydoc evo::SubList::shared() const */
    bool shared() const;

    /** \copydoc evo::SubList::first() const */
    const char* first() const;

    /** \copydoc evo::SubList::last() const */
    const char* last() const;

    /** \copydoc evo::SubList::iend(Size) const */
    Key iend(Size offset) const;

    /** \copydoc evo::SubList::hash(ulong) const */
    ulong hash(ulong seed) const;

    /** \copydoc evo::SubList::compare(const ListBaseType&) const */
    int compare(const StringBase& data) const;

    /** \copydoc evo::SubList::find(ItemVal,Key,Key) const */
    Key find(ItemVal item,Key start,Key end) const;

    /** \copydoc evo::SubList::findr(ItemVal,Key,Key) const */
    Key findr(ItemVal item,Key start,Key end) const;
    
    /** \copydoc evo::SubList::findany(const Item*,Size,Key,Key) const */
    Key findany(const char* items,Size count,Key start,Key end) const;
    
    /** \copydoc evo::SubList::findanyr(const Item*,Size,Key,Key) const */
    Key findanyr(const char* items,Size count,Key start,Key end) const;

    /** \copydoc evo::SubList::splitat(Key,T1&,T2&) const */
    template<class T1,class T2> bool splitat(Key index,T1& left,T2& right) const;

    /** \copydoc evo::SubList::splitat(Key,T1&) const */
    template<class T1> bool splitat(Key index,T1& left) const;

    /** \copydoc evo::SubList::splitat(Key,ValNull,T2&) const */
    template<class T2> bool splitat(Key index,ValNull left,T2& right) const;

    /** \copydoc evo::SubList::splitat_setl(Key) */
    bool splitat_setl(Key index);

    /** \copydoc evo::SubList::splitat_setl(Key,T2&) */
    template<class T2> bool splitat_setl(Key index,T2& right);

    /** \copydoc evo::SubList::splitat_setr(Key) */
    bool splitat_setr(Key index);

    /** \copydoc evo::SubList::splitat_setr(Key,T1&) */
    template<class T1> bool splitat_setr(Key index,T1& left);

    /** \copydoc evo::SubList::swap(SubListType&) */
    void swap(StringBase& list);
#endif

private:
    // Doxygen: Hide unused overridden parent members from documentation
    #ifdef DOXYGEN
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
};

// SubString comparison
/** \cond impl */
inline bool operator==(const char* str1, const SubString& str2)
    { return str2 == str1; }
inline bool operator!=(const char* str1, const SubString& str2)
    { return str2 != str1; }
/** \endcond */

// SubString conversion
/** \cond impl */
// Common base types for string conversion -- used internally
template<class T> struct Convert_SubStringToIntBase {
    // Invalid: Converting Num to SubString
    template<class U> static void set(U&, const SubString&)
        STATIC_ASSERT_FUNC_UNUSED
    template<class U> static void add(U&, const SubString&)
        STATIC_ASSERT_FUNC_UNUSED
    template<class U> static void addq(U&, const SubString&, char)
        STATIC_ASSERT_FUNC_UNUSED
    static T value(const SubString& src)
        { return src.getnum<T>(); }
};
template<class T> struct Convert_SubStringToFltBase {
    // Invalid: Converting Flt to SubString
    template<class U> static void set(U&, const SubString&)
        STATIC_ASSERT_FUNC_UNUSED
    template<class U> static void add(U&, const SubString&)
        STATIC_ASSERT_FUNC_UNUSED
    template<class U> static void addq(U&, const SubString&, char)
        STATIC_ASSERT_FUNC_UNUSED
    static T value(const SubString& src)
        { return src.getnumf<T>(); }
};
// Conversion templates
template<> struct Convert<const char*,SubString> {
    // Invalid: Converting char* to SubString
    template<class U> static void set(U&, const SubString&)
        STATIC_ASSERT_FUNC_UNUSED
    template<class U> static void add(U&, const SubString&)
        STATIC_ASSERT_FUNC_UNUSED
    template<class U> static void addq(U&, const SubString&, char)
        STATIC_ASSERT_FUNC_UNUSED
    static SubString value(const char* src)
        { return src; }
};
template<> struct Convert<char*,SubString> {
    // Invalid: Converting char* to SubString
    template<class U> static void set(U&, const SubString&)
        STATIC_ASSERT_FUNC_UNUSED
    template<class U> static void add(U&, const SubString&)
        STATIC_ASSERT_FUNC_UNUSED
    template<class U> static void addq(U&, const SubString&, char)
        STATIC_ASSERT_FUNC_UNUSED
    static SubString value(const char* src)
        { return src; }
};
template<> struct Convert<String,SubString> {
    static void set(String& dest, const SubString& value)
        { dest = value; }
    static void add(String& dest, const SubString& value)
        { dest.add(value); }
    static void addq(String& dest, const SubString& value, char delim)
        { StrQuoting::addq(dest, value, delim); }
    static SubString value(const String& src)
        { return src; }
};
template<> struct Convert<SubString,String> {
    static void set(SubString& dest, const String& value)
        { dest = value; }
    // Invalid: Add String to SubString
    template<class U> static void add(U& dest, const String& value)
        STATIC_ASSERT_FUNC_UNUSED
    template<class U> static void addq(U&, const SubString&, char)
        STATIC_ASSERT_FUNC_UNUSED
    static String value(const SubString& src)
        { return src; }
};
template<> struct Convert<SubString,SubString> {
    static void set(SubString& dest, const SubString& value)
        { dest.set(value); }
    // Invalid: Add SubString to SubString
    template<class U> static void add(U& dest, const SubString& value)
        STATIC_ASSERT_FUNC_UNUSED
    template<class U> static void addq(U&, const SubString&, char)
        STATIC_ASSERT_FUNC_UNUSED
    static const SubString& value(const SubString& src)
        { return src; }
};
template<> struct Convert<SubString,bool> {
    static void set(SubString& dest, bool value) {
        if (value) dest.set("true", 4);
        else       dest.set("false", 5);
    }
    // Invalid: Add bool to SubString
    template<class U> static void add(U& dest, const SubString& value)
        STATIC_ASSERT_FUNC_UNUSED
    template<class U> static void addq(U&, const SubString&, char)
        STATIC_ASSERT_FUNC_UNUSED
    static bool value(const SubString& src)
        { return src.getbool<bool>(); }
};
template<> struct Convert<SubString,short>   : public Convert_SubStringToIntBase<short>   { };
template<> struct Convert<SubString,int>     : public Convert_SubStringToIntBase<int>     { };
template<> struct Convert<SubString,long>    : public Convert_SubStringToIntBase<long>    { };
template<> struct Convert<SubString,longl>   : public Convert_SubStringToIntBase<longl>   { };
template<> struct Convert<SubString,ushort>  : public Convert_SubStringToIntBase<ushort>  { };
template<> struct Convert<SubString,uint>    : public Convert_SubStringToIntBase<uint>    { };
template<> struct Convert<SubString,ulong>   : public Convert_SubStringToIntBase<ulong>   { };
template<> struct Convert<SubString,ulongl>  : public Convert_SubStringToIntBase<ulongl>  { };
template<> struct Convert<SubString,float>   : public Convert_SubStringToFltBase<float>   { };
template<> struct Convert<SubString,double>  : public Convert_SubStringToFltBase<double>  { };
template<> struct Convert<SubString,ldouble> : public Convert_SubStringToFltBase<ldouble> { };
template<> struct Convert<SubString,Bool> {
    static void set(SubString& dest, Bool value) {
        if (value.null()) dest.set();
        else if (*value)  dest.set("true", 4);
        else              dest.set("false", 5);
    }
    // Invalid: Add Bool to SubString
    template<class U> static void add(U& dest, const SubString& value)
        STATIC_ASSERT_FUNC_UNUSED
    template<class U> static void addq(U&, const SubString&, char)
        STATIC_ASSERT_FUNC_UNUSED
    static Bool value(const SubString& src)
        { return src.getbool<Bool>(); }
};
template<> struct Convert<SubString,Short>  : public Convert_SubStringToIntBase<Short>  { };
template<> struct Convert<SubString,Int>    : public Convert_SubStringToIntBase<Int>    { };
template<> struct Convert<SubString,Long>   : public Convert_SubStringToIntBase<Long>   { };
template<> struct Convert<SubString,LongL>  : public Convert_SubStringToIntBase<LongL>  { };
template<> struct Convert<SubString,UShort> : public Convert_SubStringToIntBase<UShort> { };
template<> struct Convert<SubString,UInt>   : public Convert_SubStringToIntBase<UInt>   { };
template<> struct Convert<SubString,ULong>  : public Convert_SubStringToIntBase<ULong>  { };
template<> struct Convert<SubString,ULongL> : public Convert_SubStringToIntBase<ULongL> { };
template<> struct Convert<SubString,Float>  : public Convert_SubStringToFltBase<Float>  { };
template<> struct Convert<SubString,FloatD> : public Convert_SubStringToFltBase<FloatD> { };
template<> struct Convert<SubString,FloatL> : public Convert_SubStringToFltBase<FloatL> { };

/** \endcond */

///////////////////////////////////////////////////////////////////////////////

/** References a list of ordered substrings for fast lookup.
 - Item order must already be ordered
 - Lookups are done with binary search
 - \b Caution: %String list _must be sorted_ for `find` methods to work
 - See also: \ref EnumConversion "Enum Conversion"
 .

\par Example

\code
#include <evo/substring.h>
using namespace evo;

int main() {
    // Pre-sorted string list (no mem allocs)
    static const SubString LIST[] = {
        "bar",
        "foo",
        "stuff"
    };

    // Efficient map from string list (no mem allocs, binary search)
    static const SubStringMapList LISTMAP(LIST, fixed_array_size(LIST));

    int i1 = LISTMAP.find("foo");       // set to 1
    int i2 = LISTMAP.find("baz");       // set to NONE (not found)

    return 0;
}
\endcode
*/
class SubStringMapList {
public:
    /** Constructor for null and empty SubString list. */
    SubStringMapList() : data_(NULL), size_(0) {
    }

    /** Constructor for referencing an existing SubString list.
     - This references list data and does not allocate any memory
     .
     \param  data          List data, NULL to set as null
     \param  size          Size as number of strings in list, 0 to set as empty
     \param  verify_order  Whether to verify string order with verify() and call abort() if this fails
    */
    SubStringMapList(const SubString* data, SizeT size, bool verify_order=false) : data_((SubString*)data), size_(size) {
        if (verify_order && !verify())
            abort();    // verify failed, string out of order
    }

    /** Get whether empty.
     \return  Whether empty
    */
    bool empty() const {
        return (size_ == 0);
    }

    /** Get whether null.
     \return  Whether null
    */
    bool null() const {
        return (data_ == NULL);
    }

    /** Find key string in list.
     - This uses a binary search so list must be sorted, otherwise results are accurate
     - If there are duplicate strings in list, which one is found first is undefined
     - \b Caution: String values _must have been sorted_
     .
     \param  key  Key string to look for
     \return      Found key index (0 for first), NONE if not found
    */
    SizeT find(const SubString& key) const {
        int cmp;
        SizeT left = 0, right = size_, mid = 0;
        while (left < right) {
            mid = left + ((right-left) / 2);
            cmp = key.compare(data_[mid]);
            if (cmp < 0)
                right = mid;
            else if (cmp == 0)
                return mid;
            else
                left = mid + 1;
        }
        return NONE;
    }

    /** Find key string in list and convert to enum value.
     - This uses a binary search so list must be sorted, otherwise results are inaccurate
     - This calls assert() to check the result and that the number of enum values matches the string list size
     - \b Caution: String values _must have been sorted_
     - See also: \ref EnumConversion "Enum Conversion"
     .
     \tparam  T  Enum type to convert to, inferred from arguments
     \param  key         Key string to look for
     \param  first_enum  First enum value to map to, maps to first string
     \param  last_enum   Last enum value to map to, maps to last string -- must be >= `first_enum`
     \param  unknown     Unknown enum value to use if key not found or result out of range
     \return             Found enum value, `unknown` if not found or out of range

    \par Example
    \code
    #include <evo/substring.h>
    using namespace evo;

    enum Color {
        cUNKNOWN = 0,
        cBLUE,
        cGREEN,
        cRED
    };

    int main() {
        static const SubString COLORS[] = {
            "blue",
            "green",
            "red"
        };
        static const SubStringMapList COLOR_MAP(COLORS, fixed_array_size(COLORS));

        Color color = COLOR_MAP.find_enum("green", cBLUE, cRED, cUNKNOWN);
        return 0;
    }
    \endcode
    */
    template<class T>
    T find_enum(const SubString& key, T first_enum, T last_enum, T unknown) const {
        assert( (SizeT)last_enum >= (SizeT)first_enum );
        assert( (SizeT)last_enum - (SizeT)first_enum + 1 == size_ );
        SizeT i = find(key);
        if (i == NONE)
            return unknown;
        i += (SizeT)first_enum;
        assert( i <= (SizeT)last_enum );
        if (i > (SizeT)last_enum)
            return unknown;
        return (T)i;
    }

    /** Find key string in list and convert to enum value using a traits class.
     - This uses a binary search so list must be sorted, otherwise results are inaccurate
     - This calls assert() to check the number of enum values matches the string list size
     - This uses a traits class to call find_enum()
     - \b Caution: String values _must have been sorted_
     - See also: \ref EnumConversion "Enum Conversion"
     .
     \tparam  T  Traits class with enum type `Type`, and constants: `FIRST`, `LAST`, `UNKNOWN`
     \param  key  Key string to look for

    \par Example
    \code
    #include <evo/substring.h>
    using namespace evo;

    enum Color {
        cUNKNOWN = 0,
        cBLUE,
        cGREEN,
        cRED
    };

    struct ColorTraits {
        typedef Color Type;
        static const Color FIRST   = cBLUE;
        static const Color LAST    = cRED;
        static const Color UNKNOWN = cUNKNOWN;

        static const SubStringMapList& map() {
            static const SubString LIST[] = {
                "blue",
                "green",
                "red"
            };
            static const SubStringMapList MAP(LIST, fixed_array_size(LIST));
            return MAP;
        }

        static Color get_enum(const SubString& key) {
            return map().find_enum_traits<ColorTraits>(key);
        }

        static SubString get_string(Color value) {
            return map().get_enum_traits_string<ColorTraits>(value);
        }
    };

    int main() {
        Color     color_val = ColorTraits::get_enum("green");
        SubString color_str = ColorTraits::get_string(cGREEN);
        return 0;
    }
    \endcode
    */
    template<class T>
    typename T::Type find_enum_traits(const SubString& key) const {
        return find_enum<typename T::Type>(key, T::FIRST, T::LAST, T::UNKNOWN);
    }

    /** Convert enum value to key string from list.
     - This is the reverse of find_enum()
     - See also: \ref EnumConversion "Enum Conversion"
     .
     \tparam  T  Enum type to convert from, inferred from arguments
     \param  enum_value  Enum value to convert from
     \param  first_enum  First enum value to map to, maps to first string
     \param  last_enum   Last enum value to map to, maps to last string -- must be >= `first_enum`
     \return             %String for enum value, null if unknown
    */
    template<class T>
    SubString get_enum_string(T enum_value, T first_enum, T last_enum) const {
        if ((SizeT)enum_value < (SizeT)first_enum || (SizeT)enum_value > (SizeT)last_enum)
            return SubString();
        SizeT i = (SizeT)enum_value - (SizeT)first_enum;
        assert( i < size_ );
        if (i >= size_)
            return SubString();
        return data_[i];
    }

    /** Convert enum value to key string from list using a traits class.
     - This is the reverse of find_enum_traits()
     - See also: \ref EnumConversion "Enum Conversion"
     .
     \tparam  T  Traits class with enum type `Type`, and constants: `FIRST`, `LAST`
     \param  enum_value  Enum value to convert from
     \return             %String for enum value, null if unknown
    */
    template<class T>
    SubString get_enum_traits_string(typename T::Type enum_value) const {
        return get_enum_string(enum_value, T::FIRST, T::LAST);
    }

#if defined(EVO_CPP11) || defined(DOXYGEN)
    /** Find key string in list and convert to enum class value (C++11).
     - This uses a binary search so list must be sorted, otherwise results are inaccurate
     - This calls assert() to check the number of enum values matches the string list size
     - This assumes the enum class has guard values: starts with `T::UNKNOWN`, ends with `T::ENUM_END`, with at least 1 mapped value in between
     - See also: \ref EnumConversion "Enum Conversion"
     .
     \tparam  T  Enum class type to use
     \param  key  Key string to look for
     \return      Found enum value for key, `T::UNKNOWN` if not found

    \par Example
    \code
    #include <evo/substring.h>
    using namespace evo;

    enum class Color {
        UNKNOWN = 0,
        BLUE,
        GREEN,
        RED,
        ENUM_END
    };

    int main() {
        static const SubString COLORS[] = {
            SubString("blue"),
            SubString("green"),
            SubString("red")
        };
        static const SubStringMapList COLOR_MAP(COLORS, fixed_array_size(COLORS));

        Color color = COLOR_MAP.find_enum_class<Color>("green");
        return 0;
    }
    \endcode
    */
    template<class T>
    T find_enum_class(const SubString& key) const {
        return find_enum<T>(key, (T)((SizeT)T::UNKNOWN + 1), (T)((SizeT)T::ENUM_END - 1), T::UNKNOWN);
    }

    /** Convert enum class value to key string from list (C++11).
     - This is the reverse of find_enum_class()
     - This assumes the enum class has guard values: starts with `T::UNKNOWN`, ends with `T::ENUM_END`, with at least 1 mapped value in between
     - See also: \ref EnumConversion "Enum Conversion"
     .
     \tparam  T  Enum class type to use, inferred from argument
     \param  enum_value  Enum value to convert from
     \return             %String for enum value, `T::UNKNOWN` if unknown
    */
    template<class T>
    SubString get_enum_class_string(T enum_value) const {
        return get_enum_string(enum_value, (T)((SizeT)T::UNKNOWN + 1), (T)((SizeT)T::ENUM_END - 1));
    }
#endif

    /** Verify strings are in correct order.
     - This loops through and compares all strings to verify ordering
       - This takes linear time
     - Duplicates are not considered valid
     - Call during initialization or from unit tests
     .
     \return  Whether strings are correctly ordered
    */
    bool verify() const {
        for (SizeT i = 1; i < size_; ++i)
            if (data_[i-1].compare(data_[i]) >= 0)
                return false;
        return true;
    }

private:
    SubString* data_;
    SizeT size_;
};

///////////////////////////////////////////////////////////////////////////////

/** Helper for creating enum string/value mappers with explicit first/last/unknown values.
 - See better alternative: EVO_ENUM_MAP_PREFIXED()
 - This creates a struct with traits that uses SubStringMapList::find_enum_traits() and SubStringMapList::get_enum_traits_string()
 - The created struct type is named after ENUM with suffix "Enum", and has helper functions:
   - static ENUM get_enum(const evo::SubString& key):
     - Map string key to ENUM value
   - static SubString get_string(ENUM value):
     - Map ENUM value to string key, null if unknown
   .
 - \b Caution: The string values _must match ENUM and must be sorted_
 - See: \ref EnumConversion "Enum Conversion"
 .
 \param  ENUM         Enum type to create traits for
 \param  FIRST_VAL    First enum value to map to, maps to first string
 \param  LAST_VAL     Last enum value to map to, maps to last string -- must be >= first_enum
 \param  UNKNOWN_VAL  Unknown enum value to use if key not found or result out of range
 \param  ...          _Sorted_ list of string literals to map to each enum value -- ex: `"a", "b", "c"`
*/
#define EVO_ENUM_MAP(ENUM, FIRST_VAL, LAST_VAL, UNKNOWN_VAL, ...) \
    struct ENUM ## Enum { \
        typedef ENUM Type; \
        static const ENUM FIRST   = FIRST_VAL; \
        static const ENUM LAST    = LAST_VAL; \
        static const ENUM UNKNOWN = UNKNOWN_VAL; \
        static const evo::SubStringMapList& map() { \
            static const evo::SubString LIST[] = { __VA_ARGS__ }; \
            static const evo::SubStringMapList MAP(LIST, evo::fixed_array_size(LIST)); \
            return MAP; \
        } \
        static ENUM get_enum(const evo::SubString& key) \
            { return map().find_enum_traits< ENUM ## Enum >(key); } \
        static SubString get_string(ENUM val) \
            { return map().get_enum_traits_string< ENUM ## Enum >(val); } \
    }

/** Helper for creating enum string/value mappers with prefixed enum values.
 - This creates a struct with traits that uses SubStringMapList::find_enum_class() and SubStringMapList::get_enum_class_string()
 - The created struct type is named after ENUM with suffix "Enum", and has helper functions:
   - static ENUM get_enum(const evo::SubString& key):
     - Map string key to ENUM value
   - static SubString get_string(ENUM value):
     - Map ENUM value to string key, null if unknown
   .
 - This requires ENUM type to have the following value names defined, each name beginning with PREFIX:
   - UNKNOWN -- must be first
   - ENUM_END -- must be last
   - and there _must not_ be any gaps between the above values
   .
 - \b Caution: The string values _must match ENUM and must be sorted_
 - See example here: \ref EnumConversion "Enum Conversion"
 - See also: EVO_ENUM_MAP()
 .
 \param  ENUM    Enum type to create mappings for
 \param  PREFIX  Prefix for enum values, used to find UNKNOWN and ENUM_END values
 \param  ...     _Sorted_ list of string literals to map to each enum value -- ex: `"a", "b", "c"`
*/
#define EVO_ENUM_MAP_PREFIXED(ENUM, PREFIX, ...) \
    struct ENUM ## Enum { \
        typedef ENUM Type; \
        static const ENUM FIRST   = (ENUM)((int)(PREFIX ## UNKNOWN) + 1); \
        static const ENUM LAST    = (ENUM)((int)(PREFIX ## ENUM_END) - 1); \
        static const ENUM UNKNOWN = PREFIX ## UNKNOWN; \
        static const evo::SubStringMapList& map() { \
            static const evo::SubString LIST[] = { __VA_ARGS__ }; \
            static const evo::SubStringMapList MAP(LIST, evo::fixed_array_size(LIST)); \
            return MAP; \
        } \
        static ENUM get_enum(const evo::SubString& key) \
            { return map().find_enum_traits< ENUM ## Enum >(key); } \
        static SubString get_string(ENUM val) \
            { return map().get_enum_traits_string< ENUM ## Enum >(val); } \
    }

#if defined(EVO_CPP11) || defined(DOXYGEN)
    /** Helper for creating enum class string/value mappers (C++11).
     - This creates a struct with traits that uses SubStringMapList::find_enum_class() and SubStringMapList::get_enum_class_string()
     - The created struct type is named after ENUM with suffix "Enum", and has helper functions:
       - static ENUM get_enum(const evo::SubString& key):
         - Map string key to ENUM value
       - static SubString get_string(ENUM value):
         - Map ENUM value to string key, null if unknown
       .
     - \b Caution: The string values _must match ENUM and must be sorted_
     - See example here: \ref EnumConversion "Enum Conversion"
     .
     \param  ENUM  Enum type to create mappings for
     \param  ...   _Sorted_ list of string literals to map to each enum value -- ex: `"a", "b", "c"`
    */
    #define EVO_ENUM_CLASS_MAP(ENUM, ...) \
        struct ENUM ## Enum { \
            static const evo::SubStringMapList& map() { \
                static const evo::SubString LIST[] = { __VA_ARGS__ }; \
                static const evo::SubStringMapList MAP(LIST, evo::fixed_array_size(LIST)); \
                return MAP; \
            } \
            static ENUM get_enum(const evo::SubString& key) \
                { return map().find_enum_class<ENUM>(key); } \
            static SubString get_string(ENUM val) \
                { return map().get_enum_class_string(val); } \
        }
#endif

///////////////////////////////////////////////////////////////////////////////
//@}
}
#endif
