// Evo C++ Library
/* Copyright 2018 Justin Crowell
Distributed under the BSD 2-Clause License -- see included file LICENSE.txt for details.
*/
///////////////////////////////////////////////////////////////////////////////
/** \file ustring.h Evo UnicodeString container. */
#pragma once
#ifndef INCL_evo_ustring_h
#define INCL_evo_ustring_h

#include "string.h"

namespace evo {
/** \addtogroup EvoContainers */
//@{

///////////////////////////////////////////////////////////////////////////////

/** Unicode string container using UTF-16.
 - This is useful for converting String (UTF-8) to UTF-16 for use with APIs that require it
   - Conversion on invalid UTF-8 defaults to \ref umREPLACE_INVALID - Invalid UTF-8 bytes are each replaced with \ref UNICODE_REPLACEMENT_CHAR
 - See Unicode page: \ref Unicode
 - \b Caution: Setting from a raw wchar16 pointer will use \ref UnsafePtrRef "Unsafe Pointer Referencing"
 .

\par Example

\code
#include <evo/ustring.h>
using namespace evo;

int main() {
    // Convert UTF-8 string literal to UTF-16
    UnicodeString ustr("test123");

    // Convert UTF-16 string to UTF-8
    String str(ustr);

    // Modify str then convert it to UTF-16
    str = "newtest";
    ustr = str;

    return 0;
}
\endcode
*/
class UnicodeString : public List<wchar16,StrSizeT> {
public:
    typedef UnicodeString          ThisType;        ///< This string type
    typedef List<wchar16,StrSizeT> ListType;        ///< %List type
    typedef StrSizeT               Size;            ///< Size type

    /** Default constructor sets as null. */
    UnicodeString()
        { }

    /** Copy constructor.
     - Makes shared copy if possible -- see \ref Sharing "Sharing"
     .
     \param  str  %String to copy
    */
    UnicodeString(const UnicodeString& str) : List<wchar16,Size>(str)
        { }

    /** Constructor for string pointer.
     - \b Caution: Uses \ref UnsafePtrRef "Unsafe Pointer Referencing"
     - See UnicodeString(const PtrBase<wchar16>&,Size) for best safety, use \ref Ptr "Ptr<wchar16>" to wrap raw pointer
     .
     \param  str   %String pointer to use
     \param  size  %String size as wchar16 count
    */
    UnicodeString(const wchar16* str, Size size)
        { set(str, size); }

    /** Constructor for null terminated string.
     - \b Caution: Uses \ref UnsafePtrRef "Unsafe Pointer Referencing"
     - See UnicodeString(const PtrBase<wchar16>&) for best safety, use \ref Ptr "Ptr<wchar16>" to wrap raw pointer
     .
     \param  str  %String pointer, must be terminated, initialized as null if NULL
    */
    UnicodeString(const wchar16* str)
        { set(str); }

    /** Constructor to convert string from UTF-8.
     \param  str  %String to convert
    */
    UnicodeString(const StringBase& str)
        { set(str); }

    /** Constructor to convert string from UTF-8.
     \param  str   %String to convert
     \param  size  %String size in bytes
    */
    UnicodeString(const char* str, Size size)
        { set(str, size); }

    /** Constructor to convert terminated string from UTF-8.
     \param  str  %String to convert, must be terminated
    */
    UnicodeString(const char* str)
        { set(str); }

    /** Constructor to copy null terminated string from managed pointer.
     - This always makes an unshared copy
     - For best performance (and less safety) reference substring instead with UnicodeString(const wchar16*)
     - Use \ref Ptr "Ptr<wchar16>" to wrap raw pointer
     .
     \param  str  %String pointer, must be terminated, initialized as null if NULL
    */
    UnicodeString(const PtrBase<wchar16>& str) {
        if (str.ptr_ != NULL) {
            const ulong len = utf16_strlen(str.ptr_);
            assert( len < IntegerT<Size>::MAX );
            copy(str.ptr_, (Size)len);
        }
    }

    // SET

    /** \copydoc String::operator=(const String&) */
    UnicodeString& operator=(const UnicodeString& str)
        { return set(str); }

    /** Assignment operator to copy from base list type.
     - This always makes an unshared copy
     - For best performance (and less safety) reference string instead with set(const wchar16*,Size)
     .
     \param  str  %String data to assign
     \return      This
    */
    UnicodeString& operator=(const ListBaseType& str)
        { return set(str); }

    /** Assignment operator for terminated UTF-16 string.
     - \b Caution: Uses \ref UnsafePtrRef "Unsafe Pointer Referencing"
     - See operator=(const PtrBase<wchar16>&) for best safety, use \ref Ptr "Ptr<wchar16>" to wrap raw pointer
     .
     \param  str  %String pointer, must be terminated, calls set() if null
     \return      This
    */
    UnicodeString& operator=(const wchar16* str)
        { return set(str); }

    /** Assignment operator to convert from UTF-8 string base type.
     \param  str  %String str to convert from
     \return      This
    */
    UnicodeString& operator=(const StringBase& str)
        { return set(str); }

    /** Assignment operator to convert from terminated UTF-8 string.
     \param  str  %String str to convert from, must be terminated
     \return      This
    */
    UnicodeString& operator=(const char* str)
        { return set(str); }

    /** ASsignment operator for null terminated string from managed pointer.
     - This always makes an unshared copy
     - For best performance (and less safety) reference substring instead with operator=(const wchar16*)
     - Use \ref Ptr "Ptr<wchar16>" to wrap raw pointer
     .
     \param  str  %String pointer, must be terminated, set to null if NULL
     \return      This
    */
    UnicodeString& operator=(const PtrBase<wchar16>& str)
        { return set(str); }

    /** %Set as empty but not null.
     \return  This
    */
    UnicodeString& setempty()
        { ListType::setempty(); return *this; }

    /** %Set as null and empty.
     \return  This
    */
    UnicodeString& set()
        { ListType::set(); return *this; }

    /** %Set to new string.
     - \b Caution: Uses \ref UnsafePtrRef "Unsafe Pointer Referencing"
     - For best safety use copy(const wchar16*,Size)
     .
     \param  str  %String to set from
     \return      This
    */
    UnicodeString& set(const ListType& str)
        { ListType::set(str); return *this; }

    /** %Set from base list type.
     - This always makes an unshared copy
     - For best performance (and less safety) reference string instead with set(const wchar16*,Size)
     .
     \param  str  %String to assign
     \return      This
    */
    UnicodeString& set(const ListBaseType& str)
        { ListType::set(str); return *this; }

    /** %Set from raw string pointer.
     - \b Caution: Uses \ref UnsafePtrRef "Unsafe Pointer Referencing"
     - For best safety use copy(const wchar16*,Size)
     .
     \param  str   %String pointer to set from
     \param  size  %String size as character (wchar16) count
     \return       This
    */
    UnicodeString& set(const wchar16* str, Size size)
        { ListType::set(str, size); return *this; }

    /** %Set from terminated raw string pointer.
     - \b Caution: Uses \ref UnsafePtrRef "Unsafe Pointer Referencing"
     - For best safety use set(const PtrBase<wchar16>&,Size) or use \ref Ptr "Ptr<wchar16>" to wrap raw pointer
     .
     \param  str  %String pointer to set from, must be terminated
     \return      This
    */
    UnicodeString& set(const wchar16* str) {
        if (str == NULL)
            ListType::set();
        else
            set(str, utf16_strlen(str));
        return *this;
    }

    /** %Set and convert from UTF-8 string base type.
     \param  str  %String str to convert from
     \return      This
    */
    UnicodeString& set(const StringBase& str) {
        set(str.data_, str.size_);
        return *this;
    }

    /** %Set and convert from UTF-8 string.
     \param  str   %String str to convert from
     \param  size  %String size as character count
     \param  mode  How to handle invalid UTF-8 bytes and values that convert to reserved UTF-16 surrogate values:
                    - \ref umINCLUDE_INVALID - Invalid UTF-8 bytes are still converted, but invalid UTF-16 surrogate values that can't be represented are skipped
                    - \ref umREPLACE_INVALID - Invalid UTF-8 bytes are each replaced with \ref UNICODE_REPLACEMENT_CHAR
                    - \ref umSKIP_INVALID - Invalid input is skipped and ignored
                    - \ref umSTRICT - Stop on invalid input with an error
     \return       This
    */
    UnicodeString& set(const char* str, Size size, UtfMode mode=umREPLACE_INVALID) {
        if (str == NULL) {
            set();
        } else {
            setempty();
            if (size > 0) {
                const char* p = str;
                const char* end = p + size;
                ulong newsize = utf8_to16(p, end, NULL, 0, mode);
                if (newsize > 0) {
                    wchar16* buf = advBuffer(++newsize);
                    const ulong written = utf8_to16(str, end, buf, newsize, mode);
                    if (written > 0)
                        advSize(written);
                }
            }
        }
        return *this;
    }

    /** %Set and convert from terminated UTF-8 string.
     \param  str  %String str to convert from, must be terminated, NULL to call set()
     \return      This
    */
    UnicodeString& set(const char* str) {
        if (str == NULL)
            ListType::set();
        else
            set(str, (Size)strlen(str));
        return *this;
    }

    /** %Set to null terminated string from managed pointer.
     - This always makes an unshared copy
     - For best performance (and less safety) reference substring instead with set(const wchar16*)
     - Use \ref Ptr "Ptr<wchar16>" to wrap raw pointer
     .
     \param  str  %String pointer, must be terminated, set to null if NULL
     \return      This
    */
    UnicodeString& set(const PtrBase<wchar16>& str) {
        if (str.ptr_ == NULL) {
            set();
        } else {
            const ulong len = utf16_strlen(str.ptr_);
            assert( len < IntegerT<Size>::MAX );
            copy(str.ptr_, (Size)len);
        }
        return *this;
    }

#if defined(_WIN32) || defined(DOXYGEN)
    /** Assignment operator for terminated Windows `WCHAR` string (Windows only).
     - \b Caution: Uses \ref UnsafePtrRef "Unsafe Pointer Referencing"
     - For best safety use copy(const WCHAR*)
     .
     \param  str  Pointer to string to set, must be terminated, NULL to set as null
     \return      This
    */
    UnicodeString& operator=(const WCHAR* str)
        { return set((const wchar16*)str); }

    /** %Set from Windows `WCHAR` string pointer (Windows only).
     - \b Caution: Uses \ref UnsafePtrRef "Unsafe Pointer Referencing"
     - For best safety use copy(const WCHAR*,Size)
     .
     \param  str   Pointer to string to set, NULL to set as null
     \param  size  %String size as character (WCHAR) count
     \return       This
    */
    UnicodeString& set(const WCHAR* str, Size size)
        { return set((const wchar16*)str, size); }

    /** %Set from terminated Windows `WCHAR` string pointer (Windows only).
     - \b Caution: Uses \ref UnsafePtrRef "Unsafe Pointer Referencing"
     - For best safety use copy(const WCHAR*)
     .
     \param  str  Pointer to string to set, must be terminated, NULL to set as null
     \return      This
    */
    UnicodeString& set(const WCHAR* str)
        { return set((const wchar16*)str); }

    /** %Copy from terminated Windows `WCHAR` string pointer (Windows only).
     - This always makes an unshared copy
     - For best performance (and less safety) reference string instead with set(const WCHAR*,Size)
     .
     \param  str   Pointer to string to copy from, NULL to set as null
     \param  size  %String size as character (WCHAR) count
     \return       This
    */
    UnicodeString& copy(const WCHAR* str, Size size)
        { return copy((const wchar16*)str, size); }

    /** %Copy from Windows `WCHAR` string pointer (Windows only).
     - This always makes an unshared copy
     - For best performance (and less safety) reference string instead with set(const WCHAR*)
     .
     \param  str  Pointer to string to copy from, must be terminated, NULL to set as null
     \return      This
    */
    UnicodeString& copy(const WCHAR* str)
        { return copy((const wchar16*)str); }

    /** %Set as UTF-16 string converted from UTF-8 string (Windows only).
     - This uses the Win32 API to do the conversion, and so is only supported in Windows
     - Invalid UTF-8 characters are replaced with \ref UNICODE_REPLACEMENT_CHAR
     - For terminated strings see: set_unicode_win32(const char*)
     .
     \param  str   %String to convert from, NULL to set as null by calling set()
     \param  size  %String size as character count
     \return       This
    */
    UnicodeString& set_win32(const char* str, int size) {
        if (str == NULL) {
            set();
        } else {
            setempty();
            if (size > 0) {
                assert( size < IntegerT<Size>::MAX );
                int newsize = ::MultiByteToWideChar(CP_UTF8, 0, str, size, NULL, 0);
                if (newsize > 0) {
                    WCHAR* buf = (WCHAR*)advBuffer(++newsize); // leave a space for terminator
                    const int written = ::MultiByteToWideChar(CP_UTF8, 0, str, size, buf, newsize);
                    if (written >= 0)
                        advSize(written);
                }
            }
        }
        return *this;
    }

    /** %Set as UTF-16 string converted from terminated UTF-8 string (Windows only).
     - This uses the Win32 API to do the conversion, and so is only supported in Windows
     - Invalid UTF-8 characters are replaced with \ref UNICODE_REPLACEMENT_CHAR
     - For non-terminated strings see: set_unicode_win32(const char*,int)
     .
     \param  str  %String to convert from, must be terminated, NULL to set as null by calling set()
     \return      This
    */
    UnicodeString& set_win32(const char* str) {
        if (str == NULL) {
            set();
        } else {
            setempty();
            if (*str != NULL) {
                ulong size = (ulong)strlen(str);
                assert( size < IntegerT<Size>::MAX );
                int newsize = ::MultiByteToWideChar(CP_UTF8, 0, str, size, NULL, 0);
                if (newsize > 0) {
                    WCHAR* buf = (WCHAR*)advBuffer(++newsize); // leave a space for terminator
                    const int written = ::MultiByteToWideChar(CP_UTF8, 0, str, size, buf, newsize);
                    if (written >= 0)
                        advSize(written);
                }
            }
        }
        return *this;
    }
#endif

    // INFO

    /** Get string pointer (const).
     - In some cases string may already be terminated but DO NOT expect this -- use cstr() to get terminated string
     - \b Caution: Calling any modifier/mutable method like unshare() after this may (will) invalidate the returned pointer
     .
     \return  %String pointer as read-only, NULL if null, may be invalid if string empty (const)
    */
    const wchar16* data() const
        { return data_; }

    /** Get terminated string pointer, using given string buffer if needed (const).
     - This is useful when a temporary terminated string pointer is needed
     - Same as data() when already internally terminated, otherwise provided buffer is used to store terminated string
     - \b Caution: Calling any modifier/mutable method like unshare() after this may (will) invalidate the returned pointer
     - \b Caution: Modifying the buffer also may (will) invalidate the returned pointer
     .
     \param  buffer  Buffer to use, if needed
     \return         Terminated string pointer
    */
    const wchar16* cstr(UnicodeString& buffer) const {
        static const wchar16 EMPTY = 0;
        return (size_ > 0 ? buffer.set(*this).cstr() : &EMPTY);
    }

    /** Get terminated string pointer (const).
     - Use cstr(UnicodeString&) instead for const instance
     - This effectively calls unshare() and adds an internal null terminator, if needed
     - \b Caution: Calling any modifier method like add(wchar16) after this may (will) invalidate the returned pointer
     .
     \return  Terminated string pointer
    */
    const wchar16* cstr() {
        static const wchar16 EMPTY = 0;
        const wchar16* result = &EMPTY;
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
                data_[size_] = 0;
                result = data_;
            }
        }
        return result;
    }

    /** Get terminated string pointer (modifier).
     - This is potentially unsafe and should be avoided if possible, use cstr() or cstr(String&) instead
     - \b Caution: This defeats "const" and may reallocate or modify the buffer if needed
     - \b Caution: Calling any modifier method like add(wchar16) after this may (will) invalidate the returned pointer
    */
    const wchar16* cstrM() const {
        static const wchar16 EMPTY = 0;
        return (size_ > 0 ? const_cast<UnicodeString*>(this)->cstr() : &EMPTY);
    }

    // COMPARE

    /** Comparison.
     \param  data  Data to compare to
     \return       Result (<0 if this is less, 0 if equal, >0 if this is greater)
    */
    int compare(const ListBaseType& data) const
        { return utf16_compare(data_, size_, data.data_, data.size_); }

    /** Comparison against UTF-8 string.
     \param  data  Data to compare to
     \return       Result (<0 if this is less, 0 if equal, >0 if this is greater)
    */
    int compare(const StringBase& data) const
        { return utf16_compare8(data_, size_, data.data_, data.size_); }

    using ListType::operator==;
    using ListType::operator!=;

    /** Equality operator.
     \param  str  %String to compare to
     \return      Whether equal
    */
    bool operator==(const UnicodeString& str) const
        { return ListType::operator==(str); }

    /** Equality operator.
     \param  str  %String to compare to, must be terminated
     \return      Whether equal
    */
    bool operator==(const wchar16* str) const
        { return (utf16_compare(data_, size_, str) == 0); }

    /** Equality operator to compare against UTF-8 string.
     \param  str  %String to compare to, must be terminated
     \return      Whether equal
    */
    bool operator==(const StringBase& str) const
        { return (utf16_compare8(data_, size_, str.data_, str.size_) == 0); }

    /** Equality operator to compare against terminated UTF-8 string.
     \param  str  %String to compare to, must be terminated
     \return      Whether equal
    */
    bool operator==(const char* str) const
        { return (utf16_compare8(data_, size_, str) == 0); }

    /** Inequality operator.
     \param  str  %String to compare to
     \return      Whether inequal
    */
    bool operator!=(const UnicodeString& str) const
        { return ListType::operator!=(str); }

    /** Inequality operator.
     \param  str  %String to compare to, must be terminated
     \return      Whether inequal
    */
    bool operator!=(const wchar16* str) const
        { return (utf16_compare(data_, size_, str) != 0); }

    /** Inequality operator to compare against UTF-8 string.
     \param  str  %String to compare to, must be terminated
     \return      Whether inequal
    */
    bool operator!=(const StringBase& str) const
        { return (utf16_compare8(data_, size_, str.data_, str.size_) != 0); }

    /** Inequality operator to compare against terminated UTF-8 string.
     \param  str  %String to compare to, must be terminated
     \return      Whether inequal
    */
    bool operator!=(const char* str) const
        { return (utf16_compare8(data_, size_, str) != 0); }

    // COPY

    /** %Copy from base list type.
     - This always makes an unshared copy
     - For best performance (and less safety) reference string instead with set(const ListBaseType&)
     .
     \param  str  %String to copy from
     \return      This
    */
    UnicodeString& copy(const ListBaseType& str)
        { ListType::copy(str); return *this; }

    /** %Copy from raw string pointer.
     - This always makes an unshared copy
     - For best performance (and less safety) reference string instead with set(const wchar16*,Size)
     .
     \param  str   %String to copy from
     \param  size  %String size as character (wchar16) count
     \return       This
    */
    UnicodeString& copy(const wchar16* str, Size size)
        { ListType::copy(str, size); return *this; }

    /** %Copy from terminated raw string pointer.
     - This always makes an unshared copy
     - For best performance (and less safety) reference string instead with set(const wchar16*)
     .
     \param  str  %String to copy from, must be terminated, NULL to set as null by calling set()
     \return      This
    */
    UnicodeString& copy(const wchar16* str) {
        if (str == NULL)
            ListType::set();
        else
            ListType::copy(str, utf16_strlen(str));
        return *this;
    }

    // OVERRIDES

    /** \copydoc ListType::clear() */
    UnicodeString& clear()
        { ListType::clear(); return *this; }

    /** \copydoc ListType::set2(const ListType&,Key,Key) */
    UnicodeString& set2(const ListType& data, Key index1, Key index2)
        { ListType::set2(data, index1, index2); return *this; }

    /** \copydoc ListType::set2(const ListBaseType&,Key,Key) */
    UnicodeString& set2(const ListBaseType& data, Key index1, Key index2)
        { ListType::set2(data, index1, index2); return *this; }

    /** \copydoc ListType::triml(Size) */
    UnicodeString& triml(Size size)
        { ListType::triml(size); return *this; }

    /** \copydoc ListType::trimr(Size) */
    UnicodeString& trimr(Size size)
        { ListType::trimr(size); return *this; }

    /** \copydoc ListType::truncate(Size) */
    UnicodeString& truncate(Size size=0)
        { ListType::truncate(size); return *this; }

    /** \copydoc ListType::slice(Key) */
    UnicodeString& slice(Key index)
        { ListType::slice(index); return *this; }

    /** \copydoc ListType::slice(Key,Size) */
    UnicodeString& slice(Key index, Size size)
        { ListType::slice(index, size); return *this; }

    /** \copydoc ListType::slice2(Key,Key) */
    UnicodeString& slice2(Key index1, Key index2)
        { ListType::slice2(index1, index2); return *this; }

    /** \copydoc ListType::unslice() */
    UnicodeString& unslice()
        { ListType::unslice(); return *this; }

    /** \copydoc ListType::capacity(Size) */
    UnicodeString& capacity(Size size)
        { ListType::capacity(size); return *this; }

    /** \copydoc ListType::capacitymin(Size) */
    UnicodeString& capacitymin(Size min)
        { ListType::capacitymin(min); return *this; }

    /** \copydoc ListType::capacitymax(Size) */
    UnicodeString& capacitymax(Size max)
        { ListType::capacitymax(max); return *this; }

    /** \copydoc ListType::compact() */
    UnicodeString& compact()
        { ListType::compact(); return *this; }

    /** \copydoc ListType::reserve(Size,bool) */
    UnicodeString& reserve(Size size, bool prefer_realloc=false)
        { ListType::reserve(size, prefer_realloc); return *this; }

    /** \copydoc ListType::unshare() */
    UnicodeString& unshare()
        { ListType::unshare(); return *this; }

    /** \copydoc ListType::resize(Size) */
    UnicodeString& resize(Size size)
        { ListType::resize(size); return *this; }

    /** \copydoc ListType::addnew(Size) */
    UnicodeString& addnew(Size size=1)
        { ListType::addnew(size); return *this; }

    /** \copydoc ListType::add(const Item*,Size) */
    UnicodeString& add(const Item* data, Size size)
        { ListType::add(data, size); return *this; }

    /** \copydoc ListType::add(const ListBaseType&) */
    UnicodeString& add(const ListBaseType& data)
        { ListType::add(data); return *this; }

    /** \copydoc ListType::add(const Item&) */
    UnicodeString& add(const Item& data)
        { ListType::add(data); return *this; }

    /** \copydoc ListType::operator<<(const Item&) */
    UnicodeString& operator<<(const Item& data)
        { ListType::operator<<(data); return *this; }

    /** \copydoc ListType::operator<<(const ListBaseType&) */
    UnicodeString& operator<<(const ListBaseType& data)
        { ListType::operator<<(data); return *this; }

    /** \copydoc ListType::operator<<(const ValNull&) */
    UnicodeString& operator<<(const ValNull& val)
        { ListType::operator<<(val); return *this; }

    /** \copydoc ListType::operator<<(const ValEmpty&) */
    UnicodeString& operator<<(const ValEmpty& val)
        { ListType::operator<<(val); return *this; }

    /** \copydoc ListType::prependnew(Size) */
    UnicodeString& prependnew(Size size=1)
        { ListType::prependnew(size); return *this; }

    /** \copydoc ListType::prepend(const Item*,Size) */
    UnicodeString& prepend(const Item* data, Size size)
        { ListType::prepend(data, size); return *this; }

    /** \copydoc ListType::prepend(const ListBaseType&) */
    UnicodeString& prepend(const ListBaseType& data)
        { ListType::prepend(data); return *this; }

    /** \copydoc ListType::prepend(const Item&) */
    UnicodeString& prepend(const Item& data)
        { ListType::prepend(data); return *this; }

    /** \copydoc ListType::fill(const Item&,Key,Size) */
    UnicodeString& fill(const Item& item, Key index=0, Size size=ALL)
        { ListType::fill(item, index, size); return *this; }

    /** \copydoc ListType::replace(Key,Size,const Item*,Size) */
    UnicodeString& replace(Key index, Size rsize, const Item* data, Size size)
        { ListType::replace(index, rsize, data, size); return *this; }

    /** \copydoc ListType::advResize(Size) */
    UnicodeString& advResize(Size size)
        { ListType::advResize(size); return *this; }

private:
    UnicodeString& reverse() EVO_ONCPP11(= delete);
};

///////////////////////////////////////////////////////////////////////////////
//@}
}
#endif
