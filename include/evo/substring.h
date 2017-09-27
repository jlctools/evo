// Evo C++ Library
/* Copyright (c) 2016 Justin Crowell
 This Source Code Form is subject to the terms of the Mozilla Public
 License, v. 2.0. If a copy of the MPL was not distributed with this
 file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
///////////////////////////////////////////////////////////////////////////////
/** \file substring.h Evo SubString container. */
#pragma once
#ifndef INCL_evo_substring_h
#define INCL_evo_substring_h

// Includes
#include "sublist.h"
#include "string.h"

// Namespace: evo
namespace evo {

/** \addtogroup EvoContainers */
//@{

///////////////////////////////////////////////////////////////////////////////

/** Reference and access existing string data.

\par Features

 - This provides read-only methods similar to String, but with reduced overhead
   - This is useful for referencing static/immutable data, or when performance or size is critical
   - String is generally safer, though SubString is smaller and slightly faster

 - This does not allocate or free any resources
 - \b Caution: SubString can be unsafe since it references a pointer, which must remain valid
 .

\par Iterators

 - SubString::Iter -- Read-Only Iterator (IteratorRa)
 .

\par Constructors

 - SubString()
 - SubString(const SubString&), ...
 - SubString(const char*,Size)
 - SubString(const char*)
 .

\par Read Access

 - size()
   - null(), empty()
 - cstr()
 - data()
   - item()
   - operator[]()
   - first(), last(), iend()
   - hash()
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
   - contains(ItemVal) const
   - contains(const char*,Size) const
   - contains(const char*) const
 - compare(const ListBaseType&) const
   - operator==(const ListBaseType&) const
   - operator==(const char*) const
   - operator!=(const ListBaseType&) const
   - operator!=(const char*) const
 - starts(ItemVal) const
   - starts(const char*,Size) const
   - starts(const char*) const
   - ends(ItemVal) const
   - ends(const char*,Size) const
   - ends(const char*) const
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
   - set(const ListBaseType&), ...
   - set(const ListBaseType&,Key,Key), ...
   - set(const char*,Size)
   - set2(const ListBaseType&,Key,Key), ...
   - clear()
   - operator=(const ListBaseType&), ...
   - operator=(const char*)
   - operator=(const ValNull&)
   - operator=(const ValEmpty&)
 - splitat_setl(Key)
   - splitat_setl(Key,T2&)
 - splitat_setr(Key)
   - splitat_setr(Key,T1&)
 - truncate()
   - triml(), trimr()
 - swap()
 .

\par Example

\code
#include <evo/substring.h>
using namespace evo;

int main() {
    // Create substring from string literal
    SubString str = "foo,123";

    // operator[] provides read-only (const) access
    char value = str[0];
    //str[0] = 0;       // Error: operator[] is read-only

    // Iterate and print characters (read-only)
    for (SubString::Iter iter(str); iter; ++iter)
        printf("Ch: %c\n", *iter);

    // Split substring into 2 substrings
    SubString sub1, sub2;
    str.split(',', sub1, sub2);

    // Use for formatting substring as terminated string
    String tmp;

    // Print sub1 as string and sub2 as number (dereference Int to int)
    printf("1: %s\n2: %i\n", sub1.cstr(tmp), *sub2.num());

    return 0;
}
\endcode

Output:
\verbatim
Ch: f
Ch: o
Ch: o
Ch: ,
Ch: 1
Ch: 2
Ch: 3
1: foo
2: 123
\endverbatim
*/
struct SubString : public SubList<char,StrSizeT>
{
    typedef SubString ThisType;            ///< This string type

    //[props:doxyparent]
    SubString()
        { }

    //[props:doxycopy=evo::SubList<class T,class TSize>::SubList(const ListBaseType&)]
    SubString(const ThisType& data) : SubList<char,StrSizeT>(data)
        { }

    //[props:doxycopy=evo::SubList<class T,class TSize>::SubList(const ListBaseType&)]
    SubString(const ListBaseType& data) : SubList<char,StrSizeT>(data)
        { }

    //[props:doxycopy=evo::SubList<class T,class TSize>::SubList(const ListBaseType*)]
    SubString(const ListBaseType* data)
        { if (data != NULL) set(*data);  }

    //[props:doxycopy=evo::SubList<class T,class TSize>::SubList(const Item*,Size)]
    SubString(const char* data, Size size) : SubList<char,StrSizeT>(data, size)
        { }

    /** Constructor to reference terminated string.
     \param  data  String data to reference -- must be terminated
    */
    //[tags: self, set_term, set_list! ]
    SubString(const char* data) : SubList<char,StrSizeT>(data, data?strlen(data):0)
        { }

    // SET

    //[props:doxyparent] {
    SubString& operator=(const ListBaseType& data)
        { set(data); return *this; }
    //[] }

    //[props:doxycopy=evo::SubList<class T,class TSize>::operator=(const ListBaseType*)]
    SubString& operator=(const ListBaseType* data)
        { if (data != NULL) set(*data); else set(); return *this; }

    /** Assignment operator sets as reference to terminated string.
     - This will reference the same pointer as given data
     - \b Caution: Source data pointer must remain valid
     .
     \param  data  Data to reference
     \return       This
    */
    //[tags: self, set_list! ]
    SubString& operator=(const char* data) {
        data_ = (char*)data;
        size_ = data ? strlen(data) : 0;
        return *this;
    }

    using SubListType::set;

    /** Set as reference to terminated string.
     - This will reference the same pointer as given data
     - \b Caution: Source data pointer must remain valid
     .
     \param  data  Data to reference -- must be terminated
     \return       This
    */
    //[tags: self, set_list! ]
    SubString& set(const char* data) {
        data_ = (char*)data;
        size_ = data ? strlen(data) : 0;
        return *this;
    }

    //[props:doxyparent] {
    SubString& operator=(const ValNull& val) {
        EVO_PARAM_UNUSED(val);
        set(); return *this;
    }
    SubString& operator=(const ValEmpty& val) {
        EVO_PARAM_UNUSED(val);
        setempty(); return *this;
    }
    //[] }

    // INFO

    /** Get terminated string pointer, using given string buffer if needed (const).
     - \b Caution: Modifying buffer may (will) invalidate the returned pointer
     .
     \param  buffer  Buffer to use, if needed
     \return         Terminated string pointer
    */
    //[tags: self, cstr ]
    const char* cstr(String& buffer) const
        { return (size_ > 0 ? buffer.set(data_, size_).cstr() : ""); }

    // COMPARE

    using SubListType::operator==;
    using SubListType::operator!=;

    //[props:doxycopy=evo::String::operator==(const char*) const]
    bool operator==(const char* str) const
        { return ( str == NULL ? (data_ == NULL) : (data_ != NULL && strncmp(str, data_, size_) == 0 && str[size_] == '\0') ); }

    //[props:doxycopy=evo::String::operator==(const char*) const]
    bool operator!=(const char* str) const
        { return ( str == NULL ? (data_ != NULL) : (data_ == NULL || strncmp(str, data_, size_) != 0 || str[size_] != '\0') ); }

    // FIND

    using SubListType::contains;

    /** Check whether contains given string.
     - This does a simple linear search for given string
     .
     \param  str  %String to check for
     \return      Whether string was found
    */
    //=[tags: self, find_item, compare_sub, compare() ]
    bool contains(const char* str) const {
        bool result = false;
        if (str != NULL) {
            const SizeT size = strlen(str);
            if (size > 0 && size_ >= size) {
                const Size end = size_ - size;
                for (Key i=0; i<=end; ++i)
                    if (DataOp<Item>::equal(data_+i, str, size))
                        { result = true; break; }
            }
        }
        return result;
    }

    using SubListType::starts;

    /** Check if starts with given terminated string.
     \param  str  %String to check for -- must be terminated
     \return      Whether starts with string
    */
    //=[tags: self, compare_sub, find(), compare() ]
    bool starts(const char* str) const {
        if (str == NULL)
            return false;
        const SizeT size = strlen(str);
        return (size > 0 && size_ >= size && DataOp<Item>::equal(data_, str, size));
    }

    using SubListType::ends;

    /** Check if ends with given terminated string.
     \param  str  %String to check for -- must be terminated
     \return      Whether ends with string
    */
    //=[tags: self, compare_sub, find(), compare() ]
    bool ends(const char* str) const {
        if (str == NULL)
            return false;
        const SizeT size = strlen(str);
        return (size > 0 && size_ >= size && DataOp<Item>::equal(data_+size_-size, str, size));
    }

    // SPLIT

    //[props:doxycopy=evo::String::split(char, T1&, T2&) const]
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

    //[props:doxycopy=evo::String::split(char, T1&) const]
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

    //[props:doxycopy=evo::String::split(char, ValNull, T2&) const]
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

    //[props:doxycopy=evo::String::splitr(char, T1&, T2&) const]
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

    //[props:doxycopy=evo::String::splitr(char, T1&) const]
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

    //[props:doxycopy=evo::String::splitr(char, ValNull, T2&) const]
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

    // TODO include newlines as whitespace

    //[props:doxycopy=evo::String::strip()]
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

    //[props:doxycopy=evo::String::strip(char)]
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

    //[props:doxycopy=evo::String::stripl()]
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

    //[props:doxycopy=evo::String::stripl(char)]
    SubString& stripl(char ch) {
        Size count = 0;
        while (count < size_ && data_[count] == ch)
            ++count;
        if (count > 0) {
            size_ -= count;
            data_ += count;
        }
        return *this;
    }

    //[props:doxycopy=evo::String::stripr()]
    SubString& stripr() {
        char ch;
        while ( size_ > 0 && ((ch=data_[size_-1]) == ' ' || ch == '\t') )
            --size_;
        return *this;
    }

    //[props:doxycopy=evo::String::stripr(char)]
    SubString& stripr(char ch) {
        while (size_ > 0 && data_[size_-1] == ch)
            --size_;
        return *this;
    }

    // OVERRIDES

    //[props:doxyparent] {
    SubString& clear()
        { SubListType::clear(); return *this; }
    SubString& set()
        { SubListType::set(); return *this; }
    SubString& set(const ListBaseType& data)
        { SubListType::set(data); return *this; }
    SubString& set(const ListBaseType& data, Key index, Key size=ALL)
        { SubListType::set(data, index, size); return *this; }
    SubString& set(const char* data, Size size)
        { SubListType::set(data, size); return *this; }
    SubString& set2(const ListBaseType& data, Key index1, Key index2)
        { SubListType::set2(data, index1, index2); return *this; }
    SubString& setempty()
        { SubListType::setempty(); return *this; }
    SubString& triml(Size size)
        { SubListType::triml(size); return *this; }
    SubString& trimr(Size size)
        { SubListType::trimr(size); return *this; }
    SubString& truncate(Size size)
        { SubListType::truncate(size); return *this; }
    SubString& slice(Key index)
        { SubListType::slice(index); return *this; }
    SubString& slice(Key index, Size size)
        { SubListType::slice(index, size); return *this; }
    SubString& slice2(Key index1, Key index2)
        { SubListType::slice2(index1, index2); return *this; }
    //[] }

    // GETBOOL

    //[props:doxycopy=evo::String::getbool(Error&) const]
    bool getbool(Error& error) const {
        assert( data_ > (char*)1 || size_ == 0 );
        return impl::tobool(data_, size_, error);
    }

    //[props:doxycopy=evo::String::getbool() const]
    template<class T> T getbool() const {
        assert( data_ > (char*)1 || size_ == 0 );
        return StaticIf< IsPodType<T>::value, impl::ToBoolPod<T>, impl::ToBool<T> >::Type::getbool(data_, size_);
    }

    // GETNUM

    //[props:doxycopy=evo::String::getnum(Error&,int) const]
    //[tags: self, numerr ]
    template<class T> T getnum(Error& error, int base=0) const {
        STATIC_ASSERT( IsPodType<T>::value, getnum_POD_Type_Required );
        assert( data_ > (char*)1 || size_ == 0 );
        return impl::tonum<T>(data_, size_, error, base);
    }

    //[props:doxycopy=evo::String::getnum(int) const]
    //[tags: self, num ]
    template<class T> T getnum(int base=0) const {
        assert( data_ > (char*)1 || size_ == 0 );
        return StaticIf< IsPodType<T>::value, impl::ToNumPod<T>, impl::ToNum<T> >::Type::getnum(data_, size_, base);
    }

    //[props:doxycopy=evo::String::getnumf(Error&) const]
    //[tags: self, numerr_float, numerr ]
    template<class T> T getnumf(Error& error) const {
        STATIC_ASSERT( IsPodType<T>::value, getnumf_POD_Type_Required );
        assert( data_ > (char*)1 || size_ == 0 );
        return impl::tonumf<T>(data_, size_, error);
    }

    //[props:doxycopy=evo::String::getnumf() const]
    //[tags: self, numerr_float, numerr ]
    template<class T> T getnumf() const {
        assert( data_ > (char*)1 || size_ == 0 );
        return StaticIf< IsPodType<T>::value, impl::ToNumfPod<T>, impl::ToNumf<T> >::Type::getnum(data_, size_);
    }

    // BOOLVAL

    //[props:doxycopy=evo::String::boolval() const]
    //[tags: self, bool ]
    Bool boolval() const
        { return impl::ToBool<Bool>::getbool(data_, size_); }

    // NUM

    //[props:doxycopy=evo::String::num(int) const]
    //[tags: self, num_signed, num ]
    Int num(int base=0) const
        { return impl::ToNum<Int>::getnum(data_, size_, base); }

    //[props:doxycopy=evo::String::numl(int) const]
    //[tags: self, num_signed, num ]
    Long numl(int base=0) const
        { return impl::ToNum<Long>::getnum(data_, size_, base); }

    //[props:doxycopy=evo::String::numll(int) const]
    //[tags: self, num_signed, num ]
    LongL numll(int base=0) const
        { return impl::ToNum<LongL>::getnum(data_, size_, base); }

    // NUMU

    //[props:doxycopy=evo::String::numu(int) const]
    //[tags: self, num_unsigned, num ]
    UInt numu(int base=0) const
        { return impl::ToNum<UInt>::getnum(data_, size_, base); }

    //[props:doxycopy=evo::String::numul(int) const]
    //[tags: self, num_unsigned, num ]
    ULong numul(int base=0) const
        { return impl::ToNum<ULong>::getnum(data_, size_, base); }

    //[props:doxycopy=evo::String::numull(int) const]
    //[tags: self, num_unsigned, num ]
    ULongL numull(int base=0) const
        { return impl::ToNum<ULongL>::getnum(data_, size_, base); }

    // NUMF

    //[props:doxycopy=evo::String::numf() const]
    //[tags: self, num_float, num ]
    Float numf() const
        { return impl::ToNumf<Float>::getnum(data_, size_); }

    //[props:doxycopy=evo::String::numfd() const]
    //[tags: self, num_float, num ]
    FloatD numfd() const
        { return impl::ToNumf<FloatD>::getnum(data_, size_); }

    //[props:doxycopy=evo::String::numfl() const]
    //[tags: self, num_float, num ]
    FloatL numfl() const
        { return impl::ToNumf<FloatL>::getnum(data_, size_); }

    // CONVERT

    //[props:doxycopy=evo::String::convert() const]
    //[tags: convert, split_list, split ]
    template<class C> C convert() const
        // An "undefined reference" compiler error pointing here means the given conversion isn't implemented/supported
        { return Convert<SubString,C>::value(*this); }

    //[props:doxycopy=evo::String::convert_set(C)]
    //[tags: convert, split_list, split ]
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
    //[props:doxyparent] {
    const char* data() const;
    const char& operator[](Key index) const;
    const char& item(Key index) const;

    bool operator==(const ListBaseType& data) const;
    bool operator!=(const ListBaseType& data) const;
    bool null() const;
    bool empty() const;
    Size size() const;
    const char* first() const;
    const char* last() const;
    Key iend(Size offset) const;
    ulong hash(ulong seed) const;
    int compare(const ListBaseType& data) const;
    Key find(ItemVal item,Key start,Key end) const;
    Key findr(ItemVal item,Key start,Key end) const;
    Key findany(const char* items,Size count,Key start,Key end) const;
    Key findanyr(const char* items,Size count,Key start,Key end) const;
    template<class T1,class T2> bool splitat(Key index,T1& left,T2& right) const;
    template<class T1>          bool splitat(Key index,T1& left) const;
    template<class T2>          bool splitat(Key index,ValNull left,T2& right) const;
    bool splitat_setl(Key index);
    template<class T2> bool splitat_setl(Key index,T2& right);
    bool splitat_setr(Key index);
    template<class T1> bool splitat_setr(Key index,T1& left);
    void swap(SubListType& list);
    //[] }
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
//@}
} // Namespace: evo
#endif
