// Evo C++ Library
/* Copyright 2019 Justin Crowell
Distributed under the BSD 2-Clause License -- see included file LICENSE.txt for details.
*/
///////////////////////////////////////////////////////////////////////////////
/** \file str.h Evo implementation detail: String helpers. */
#pragma once
#ifndef INCL_evo_impl_str_h
#define INCL_evo_impl_str_h

#include "sys.h"
#include "../meta.h"

// Disable certain MSVC warnings for this file
#if defined(_MSC_VER)
    #pragma warning(push)
    #pragma warning(disable:4146)
#endif

namespace evo {
/** \addtogroup EvoContainers */
//@{

///////////////////////////////////////////////////////////////////////////////

/** \cond impl */
namespace impl {
    static const char CHARMAP_TYPE_MASK = 0x07;
    inline const char* CHARMAP_TYPE()
        { return "@@@@@@@@@AAAAA@@@@@@@@@@@@@@@@@@ABBBBBBBBBBBBBBBCCCCCCCCCCBBBBBBBDDDDDDDDDDDDDDDDDDDDDDDDDDBBBBBBEEEEEEEEEEEEEEEEEEEEEEEEEEBBBB@"; }
    inline const char* CHARMAP_BREAK_TYPE()
        { return "@@@@@@@@@AAAAA@@@@@@@@@@@@@@@@@@ADB@@D@BCD@@DDDDEEEEEEEEEE@DC@DD@EEEEEEEEEEEEEEEEEEEEEEEEEEC@D@EBEEEEEEEEEEEEEEEEEEEEEEEEEEC@D@@"; }

    static const int CHARMAP_ALPHA_LEN = 36;

    static const int CHARMAP_UPPER_FIRST = 65;
    static const int CHARMAP_UPPER_LAST  = 90;
    inline const char* CHARMAP_UPPER()
        { return "ABCDEFGHIJKLMNOPQRSTUVWXYZ"; }

    static const int CHARMAP_LOWER_FIRST = 97;
    static const int CHARMAP_LOWER_LAST  = 122;
    inline const char* CHARMAP_LOWER()
        { return "abcdefghijklmnopqrstuvwxyz"; }
};
/** \endcond */

/** Character type returned by ascii_type(). */
enum CharType {
    ctOTHER = 0,    ///< Other character type
    ctWSPACE,       ///< Whitespace character
    ctPUNCT,        ///< Punctuation or symbol character
    ctDIGIT,        ///< Numeric digit character (0-9)
    ctUPPER,        ///< Uppercase alphabetic character (A-Z)
    ctLOWER         ///< Lowercase alphabetic character (a-z)
};

/** Character break type returned by ascii_breaktype(). */
enum CharBreakType {
    cbtOTHER = 0,   ///< Other charcater type
    cbtWSPACE,      ///< Whitespace character, used between words
    cbtQUOTE,       ///< Quote character, break word before or after this depending on whether begin or end quote ('"`)
    cbtBEGIN,       ///< Punctuation used before a word begins, break words before this ( ([{< )
    cbtEND,         ///< Punctuation used after a word ends, break words after this ( )]}>!%;,./ )
    cbtWORD         ///< Word character (A-Z, a-z, 0-9, _)
};

/** Get ASCII character type.
 - \#include <evo/strscan.h> or <evo/string.h> or <evo/substring.h>
 - This recognizes standard ASCII codes 0-127
 - This does not use any locale information
 .
 \param  ch  Character to check
 \return     Character type, see CharType
*/
inline CharType ascii_type(char ch) {
    if (ch & 0x80)
        return ctOTHER;
    return (CharType)(impl::CHARMAP_TYPE()[(int)ch] & impl::CHARMAP_TYPE_MASK);
}

/** Get ASCII character word-break type.
 - \#include <evo/strscan.h> or <evo/string.h> or <evo/substring.h>
 - This is used to help find a good place to insert a word-break
 - This recognizes standard ASCII codes 0-127
 - This does not use any locale information
 .
 \param  ch  Character to check
 \return     Character break type, see CharBreakType
*/
inline CharBreakType ascii_breaktype(char ch) {
    if (ch & 0x80)
        return cbtOTHER;
    return (CharBreakType)(impl::CHARMAP_BREAK_TYPE()[(int)ch] & impl::CHARMAP_TYPE_MASK);
}

/** %Convert ASCII character to uppercase.
 - \#include <evo/strscan.h> or <evo/string.h> or <evo/substring.h>
 - This recognizes standard ASCII codes 0-127
 - This does not use any locale information
 .
 \param  ch  Character to check
 \return     Converted character
*/
inline char ascii_toupper(char ch) {
    if (ch < impl::CHARMAP_LOWER_FIRST || ch > impl::CHARMAP_LOWER_LAST)
        return ch;
    return impl::CHARMAP_UPPER()[ch - impl::CHARMAP_LOWER_FIRST];
}

/** %Convert ASCII character to lowercase.
 - \#include <evo/strscan.h> or <evo/string.h> or <evo/substring.h>
 - This recognizes standard ASCII codes 0-127
 - This does not use any locale information
 .
 \param  ch  Character to check
 \return     Converted character
*/
inline char ascii_tolower(char ch) {
    if (ch < impl::CHARMAP_UPPER_FIRST || ch > impl::CHARMAP_UPPER_LAST)
        return ch;
    return impl::CHARMAP_LOWER()[ch - impl::CHARMAP_UPPER_FIRST];
}

///////////////////////////////////////////////////////////////////////////////

/** Unicode code point for "Replacement Character" used when decoding invalid UTF bytes or values.
 - This character often shows as a question mark in a box
 - Unicode code point: FFFD
 - UTF-8: Encoded with 3 bytes as 0xEF 0xBF 0xBD
 - UTF-16 & UTF-32: Encoded directly as 0xFFFD
*/
static const wchar16 UNICODE_REPLACEMENT_CHAR = 0xFFFD;

/** UTF decoding mode used to set how to handle invalid character values. */
enum UtfMode {
    umINCLUDE_INVALID,  ///< Include invalid characters -- try to use as-is (dangerous)
    umREPLACE_INVALID,  ///< Replace invalid characters with \ref UNICODE_REPLACEMENT_CHAR
    umSKIP_INVALID,     ///< Skip invalid characters
    umSTRICT            ///< Stop or error on invalid character
};

///////////////////////////////////////////////////////////////////////////////

/** Scan for next Unicode character in UTF-8 string.
 - \#include <evo/strscan.h> or <evo/string.h> or <evo/substring.h>
 - Each UTF-8 character may span 1-4 bytes, depending on the character
 - Call this in a loop to iterate through Unicode characters in a UTF-8 string, stop when NULL is returned
 .
 \param  code  %Set to full Unicode character code scanned, or error code on error (see return value)  [out]
 \param  str   Pointer to string to scan, must not be NULL
 \param  end   Pointer to end of string to scan, must be >= `str`
 \param  mode  How to handle invalid UTF-8 bytes:
                - \ref umINCLUDE_INVALID - Invalid UTF-8 bytes are each used as-is (1 byte each)
                - \ref umREPLACE_INVALID - Invalid UTF-8 bytes are each replaced with \ref UNICODE_REPLACEMENT_CHAR
                - \ref umSKIP_INVALID - Invalid UTF-8 values are skipped (ignored)
                - \ref umSTRICT - Stop with error on invalid UTF-8 value, return NULL and set code=1
 \return       Pointer after scanned Unicode character, NULL with code=0 if stopped at end of string, NULL with code=1 on error (if mode=umSTRICT)
*/
inline const char* utf8_scan(wchar32& code, const char* str, const char* end, UtfMode mode=umREPLACE_INVALID) {
    assert( str != NULL );
    assert( str <= end );
    const uchar LBITS_1      = 0x80;
    const uchar LBITS_11     = 0xC0;
    const uchar LBITS_111    = 0xE0;
    const uchar LBITS_1111   = 0xF0;
    const uchar LBITS_11111  = 0xF8;
    const uchar RBITS_111111 = 0x3F;
    const uchar RBITS_11111  = 0x1F;
    const uchar RBITS_1111   = 0x0F;
    const uchar RBITS_111    = 0x07;
    for (const uchar* p; str < end; ) {
        p = (const uchar*)str;
        if ((*p & LBITS_1) != 0) {
            if ((*p & LBITS_111) == LBITS_11) {
                // 2 byte char
                if (str+1 < end && (p[1] & LBITS_11) == LBITS_1) {
                    code = ((uint32)(p[0] & RBITS_11111) << 6) |
                           (uint32)(p[1] & RBITS_111111);
                    return str + 2;
                }
            } else if ((*p & LBITS_1111) == LBITS_111) {
                // 3 byte char
                if (str+2 < end && (p[1] & LBITS_11) == LBITS_1 && (p[2] & LBITS_11) == LBITS_1) {
                    code = ((uint32)(p[0] & RBITS_1111)   << 12) |
                           ((uint32)(p[1] & RBITS_111111) << 6) |
                           (uint32)(p[2] & RBITS_111111);
                    return str + 3;
                }
            } else if ((*p & LBITS_11111) == LBITS_1111) {
                // 4 byte char
                if (str+3 < end && (p[1] & LBITS_11) == LBITS_1 && (p[2] & LBITS_11) == LBITS_1 && (p[3] & LBITS_11) == LBITS_1) {
                    code = ((uint32)(p[0] & RBITS_111)    << 18) |
                           ((uint32)(p[1] & RBITS_111111) << 12) |
                           ((uint32)(p[2] & RBITS_111111) << 6) |
                           (uint32)(p[3] & RBITS_111111);
                    return str + 4;
                }
            }
            // Invalid multi-byte char
            if (mode == umREPLACE_INVALID) {
                code = UNICODE_REPLACEMENT_CHAR;
                return str + 1;
            } else if (mode == umSKIP_INVALID) {
                ++str;
                continue;
            } else if (mode == umSTRICT) {
                code = 1;
                return NULL;
            }
        }
        // 1 byte char
        code = (wchar32)*p;
        return str + 1;
    }
    code = 0;
    return NULL;
}

/** Scan for next Unicode character in terminated UTF-8 string.
 - \#include <evo/strscan.h> or <evo/string.h> or <evo/substring.h>
 - Each UTF-8 character may span 1-4 bytes, depending on the character
 - Call this in a loop to iterate through Unicode characters in a UTF-8 string, stop when NULL is returned
 .
 \param  code  %Set to full Unicode character code scanned, or error code on error (see return value)  [out]
 \param  str   Pointer to string to scan, must be terminated, must not be NULL
 \param  mode  How to handle invalid UTF-8 bytes:
                - \ref umINCLUDE_INVALID - Invalid UTF-8 bytes are each used as-is (1 byte each)
                - \ref umREPLACE_INVALID - Invalid UTF-8 bytes are each replaced with \ref UNICODE_REPLACEMENT_CHAR
                - \ref umSKIP_INVALID - Invalid UTF-8 values are skipped (ignored)
                - \ref umSTRICT - Stop with error on invalid UTF-8 value, return NULL and set code=1
 \return       Pointer after scanned Unicode character, NULL with code=0 if stopped at end of string, NULL with code=1 on error (if mode=umSTRICT)
*/
inline const char* utf8_scan_term(wchar32& code, const char* str, UtfMode mode=umREPLACE_INVALID) {
    assert( str != NULL );
    const uchar LBITS_1      = 0x80;
    const uchar LBITS_11     = 0xC0;
    const uchar LBITS_111    = 0xE0;
    const uchar LBITS_1111   = 0xF0;
    const uchar LBITS_11111  = 0xF8;
    const uchar RBITS_111111 = 0x3F;
    const uchar RBITS_11111  = 0x1F;
    const uchar RBITS_1111   = 0x0F;
    const uchar RBITS_111    = 0x07;
    for (const uchar* p; *str != 0; ) {
        p = (const uchar*)str;
        if ((*p & LBITS_1) != 0) {
            if ((*p & LBITS_111) == LBITS_11) {
                // 2 byte char
                if (str[1] != 0 && (p[1] & LBITS_11) == LBITS_1) {
                    code = ((uint32)(p[0] & RBITS_11111) << 6) |
                           (uint32)(p[1] & RBITS_111111);
                    return str + 2;
                }
            } else if ((*p & LBITS_1111) == LBITS_111) {
                // 3 byte char
                if (str[1] != 0 && str[2] != 0 &&
                    (p[1] & LBITS_11) == LBITS_1 && (p[2] & LBITS_11) == LBITS_1) {
                    code = ((uint32)(p[0] & RBITS_1111)   << 12) |
                           ((uint32)(p[1] & RBITS_111111) << 6) |
                           (uint32)(p[2] & RBITS_111111);
                    return str + 3;
                }
            } else if ((*p & LBITS_11111) == LBITS_1111) {
                // 4 byte char
                if (str[1] != 0 && str[2] != 0 && str[3] != 0 &&
                    (p[1] & LBITS_11) == LBITS_1 && (p[2] & LBITS_11) == LBITS_1 && (p[3] & LBITS_11) == LBITS_1) {
                    code = ((uint32)(p[0] & RBITS_111)    << 18) |
                           ((uint32)(p[1] & RBITS_111111) << 12) |
                           ((uint32)(p[2] & RBITS_111111) << 6) |
                           (uint32)(p[3] & RBITS_111111);
                    return str + 4;
                }
            }
            // Invalid multi-byte char
            if (mode == umREPLACE_INVALID) {
                code = UNICODE_REPLACEMENT_CHAR;
                return str + 1;
            } else if (mode == umSKIP_INVALID) {
                ++str;
                continue;
            } else if (mode == umSTRICT) {
                code = 1;
                return NULL;
            }
        }
        // 1 byte char
        code = (wchar32)*p;
        return str + 1;
    }
    code = 0;
    return NULL;
}

/** %Compare two non-terminated UTF-8 strings.
 - \#include <evo/strscan.h> or <evo/string.h> or <evo/substring.h>
 - See also terminated variants: utf8_compare(const char*,ulong,const char*), utf16_compare(const char*,const char*)
 .
 \param  str1  First string to compare, can be NULL
 \param  len1  First string length
 \param  str2  Second string to compare, can be NULL
 \param  len2  Second string length
 \return       Result (<0 if this is less, 0 if equal, >0 if this is greater)
*/
inline int utf8_compare(const char* str1, ulong len1, const char* str2, ulong len2) {
    if (str1 == NULL) {
        if (str2 != NULL)
            return -1;
    } else if (str2 == NULL) {
        return 1;
    } else if (str1 == str2) {
        if (len1 < len2)
            return -1;
        else if (len1 > len2)
            return 1;
    } else {
        const char* end1 = str1 + len1;
        const char* end2 = str2 + len2;
        for (;;) {
            if (str1 >= end1) {
                if (str2 >= end2)
                    break;
                return -1;
            } else if (str2 >= end2)
                return 1;
            else if (*str1 < *str2)
                return -1;
            else if (*str1 > *str2)
                return 1;
            ++str1;
            ++str2;
        }
    }
    return 0;
}

/** %Compare non-terminated and terminated UTF-8 strings.
 - \#include <evo/strscan.h> or <evo/string.h> or <evo/substring.h>
 - See also non-terminated and terminated variants: utf8_compare(const char*,ulong,const char*,ulong), utf16_compare(const char*,const char*)
 .
 \param  str1  First string to compare, can be NULL
 \param  len1  First string length
 \param  str2  Second string to compare, must be terminated, can be NULL
 \return       Result (<0 if this is less, 0 if equal, >0 if this is greater)
*/
inline int utf8_compare(const char* str1, ulong len1, const char* str2) {
    if (str1 == NULL) {
        if (str2 != NULL)
            return -1;
    } else if (str2 == NULL) {
        return 1;
    } else {
        const char* end1 = str1 + len1;
        for (;;) {
            if (str1 >= end1) {
                if (*str2 == 0)
                    break;
                return -1;
            } else if (*str2 == 0)
                return 1;
            else if (*str1 < *str2)
                return -1;
            else if (*str1 > *str2)
                return 1;
            ++str1;
            ++str2;
        }
    }
    return 0;
}

/** %Compare two terminated UTF-8 strings.
 - \#include <evo/strscan.h> or <evo/string.h> or <evo/substring.h>
 - See also non-terminated variants: utf8_compare(const char*,ulong,const char*,ulong), utf16_compare(const char*,ulong,const char*)
 .
 \param  str1  First string to compare, must be terminated, can be NULL
 \param  str2  Second string to compare, must be terminated, can be NULL
 \return       Result (<0 if this is less, 0 if equal, >0 if this is greater)
*/
inline int utf8_compare(const char* str1, const char* str2) {
    if (str1 == NULL) {
        if (str2 != NULL)
            return -1;
    } else if (str2 == NULL) {
        return 1;
    } else {
        for (;;) {
            if (*str1 == 0) {
                if (*str2 == 0)
                    break;
                return -1;
            } else if (*str2 == 0)
                return 1;
            else if (*str1 < *str2)
                return -1;
            else if (*str1 > *str2)
                return 1;
            ++str1;
            ++str2;
        }
    }
    return 0;
}

/** Scan for UTF-8 multi-byte characters of at least minsize.
 - \#include <evo/strscan.h> or <evo/string.h> or <evo/substring.h>
 - Multi-byte characters are used for higher Unicode code points
 - Valid UTF-8 single-byte characters are also valid ASCII characters
 - Use `minsize` to set the multi-byte characters to stop at -- character sizes smaller than this are ignored
 .
 \param  str       Pointer to string to search, must not be NULL
 \param  end       Pointer to end of string to scan, must be >= `str`
 \param  strict    Whether to stop counting on invalid UTF-8 byte, false to skip/ignore invalid UTF-8 bytes
 \param  mincount  Minimum number of multi-byte values to look for, must be positive
 \param  minsize   Minimum number of bytes to count as multi-byte value, must be 2-4 (inclusive)
 \return           Pointer to UTF-8 character that met mincount, NULL if not enough multi-byte values found
*/
inline const char* utf8_min(const char* str, const char* end, bool strict=false, ulong mincount=1, uint minsize=2) {
    assert( str != NULL );
    assert( str <= end );
    assert( mincount > 0 );
    assert( minsize >= 2 );
    assert( minsize <= 4 );
    const uchar LBITS_1     = 0x80;
    const uchar LBITS_11    = 0xC0;
    const uchar LBITS_111   = 0xE0;
    const uchar LBITS_1111  = 0xF0;
    const uchar LBITS_11111 = 0xF8;
    ulong count = 0;
    for (const uchar* p; str < end; ) {
        p = (const uchar*)str;
        if ((*p & LBITS_1) != 0) {
            if ((*p & LBITS_111) == LBITS_11) {
                // 2 byte char
                if (str+1 < end && (p[1] & LBITS_11) == LBITS_1) {
                    if (minsize <= 2 && ++count >= mincount)
                        return str;
                    str += 2;
                    continue;
                }
            } else if ((*p & LBITS_1111) == LBITS_111) {
                // 3 byte char
                if (str+2 < end && (p[1] & LBITS_11) == LBITS_1 && (p[2] & LBITS_11) == LBITS_1) {
                    if (minsize <= 3 && ++count >= mincount)
                        return str;
                    str += 3;
                    continue;
                }
            } else if ((*p & LBITS_11111) == LBITS_1111) {
                // 4 byte char
                if (str+3 < end && (p[1] & LBITS_11) == LBITS_1 && (p[2] & LBITS_11) == LBITS_1 && (p[3] & LBITS_11) == LBITS_1) {
                    if (minsize <= 4 && ++count >= mincount)
                        return str;
                    str += 4;
                    continue;
                }
            }
            // Invalid multi-byte char
            if (strict)
                break;
        }
        // 1 byte char
        ++str;
    }
    return NULL;
}

/** Count Unicode character values in UTF-8 string.
 - \#include <evo/strscan.h> or <evo/string.h> or <evo/substring.h>
 - Each UTF-8 character may span 1-4 bytes, depending on the character
 .
 \param  str   Pointer to string to scan, must not be NULL
 \param  end   Pointer to end of string to scan, must be >= `str`
 \param  mode  How to handle invalid UTF-8 bytes:
                - \ref umINCLUDE_INVALID - Invalid UTF-8 values are counted as 1 character each
                - \ref umREPLACE_INVALID - Invalid UTF-8 values are counted as 1 character each (\ref UNICODE_REPLACEMENT_CHAR)
                - \ref umSKIP_INVALID - Invalid UTF-8 values are skipped and not counted
                - \ref umSTRICT - Stop counting on an invalid UTF-8 value and return an error
 \return       Number of Unicode characters counted, NONE on error (if mode=umSTRICT)
*/
inline ulong utf8_count(const char* str, const char* end, UtfMode mode=umREPLACE_INVALID) {
    assert( str != NULL );
    assert( str <= end );
    const uchar LBITS_1     = 0x80;
    const uchar LBITS_11    = 0xC0;
    const uchar LBITS_111   = 0xE0;
    const uchar LBITS_1111  = 0xF0;
    const uchar LBITS_11111 = 0xF8;
    ulong count = 0;
    for (const uchar* p; str < end; ) {
        p = (const uchar*)str;
        if ((*p & LBITS_1) != 0) {
            if ((*p & LBITS_111) == LBITS_11) {
                // 2 byte char
                if (str+1 < end && (p[1] & LBITS_11) == LBITS_1) {
                    ++count;
                    str += 2;
                    continue;
                }
            } else if ((*p & LBITS_1111) == LBITS_111) {
                // 3 byte char
                if (str+2 < end && (p[1] & LBITS_11) == LBITS_1 && (p[2] & LBITS_11) == LBITS_1) {
                    ++count;
                    str += 3;
                    continue;
                }
            } else if ((*p & LBITS_11111) == LBITS_1111) {
                // 4 byte char
                if (str+3 < end && (p[1] & LBITS_11) == LBITS_1 && (p[2] & LBITS_11) == LBITS_1 && (p[3] & LBITS_11) == LBITS_1) {
                    ++count;
                    str += 4;
                    continue;
                }
            }
            // Invalid multi-byte char
            if (mode == umSKIP_INVALID) {
                ++str;
                continue;
            } else if (mode == umSTRICT)
                return NONE;
            // umINCLUDE_INVALID & umREPLACE_INVALID count as 1 char
        }
        // 1 byte char
        ++count;
        ++str;
    }
    return count;
}

/** %Convert UTF-8 string to UTF-16 string.
 - \#include <evo/strscan.h> or <evo/string.h> or <evo/substring.h>
 \param  str      Pointer to input UTF-8 string to convert, set to stopping point on return, must not be NULL  [in/out]
 \param  end      Pointer to stop input at (end of input, exclusive)
 \param  outbuf   Output buffer to write UTF-16 string to, NULL to scan without writing
 \param  outsize  Output buffer size as wchar16 count (not bytes), ignored if outbuf=NULL
 \param  mode     How to handle invalid UTF-8 bytes and values that convert to reserved UTF-16 surrogate values:
                   - \ref umINCLUDE_INVALID - Invalid UTF-8 bytes are still converted, but invalid UTF-16 surrogate values that can't be represented are skipped
                   - \ref umREPLACE_INVALID - Invalid UTF-8 bytes are each replaced with \ref UNICODE_REPLACEMENT_CHAR
                   - \ref umSKIP_INVALID - Invalid input is skipped and ignored
                   - \ref umSTRICT - Stop on invalid input with an error
 \return          Count written to outbuf as wchar16 count (not bytes), END on invalid input (if mode=umSTRICT)
*/
inline ulong utf8_to16(const char*& str, const char* end, wchar16* outbuf=NULL, ulong outsize=0, UtfMode mode=umREPLACE_INVALID) {
    assert( str != NULL );
    assert( str <= end );
    ulong written = 0;
    wchar32 code;
    const char* p;
    if (outbuf == NULL) {
        // Count UTF-16 size required (no writes)
        for (;;) {
            if ((p = utf8_scan(code, str, end, mode)) == NULL) {
                if (code == 1)
                    return END; // Invalid input with mode umSTRICT
                break;
            }
            if (code < 0x10000) {
                // Single char
                if (code >= 0xD800 && code <= 0xDFFF) {
                    if (mode == umSKIP_INVALID) {
                        str = p;
                        continue;
                    } else if (mode == umSTRICT)
                        return END; // Invalid input using reserved surrogate value with mode umSTRICT
                    // umINCLUDE_INVALID & umREPLACE_INVALID count as 1 char
                }
                ++written;
            } else
                // Surrogate pair
                written += 2;
            str = p;
        }
    } else {
        // Write UTF-16
        for (;;) {
            if ((p = utf8_scan(code, str, end, mode)) == NULL) {
                if (code == 1)
                    return END; // Invalid input with mode umSTRICT
                break;
            }
            if (code < 0x10000) {
                // Single char
                if (code >= 0xD800 && code <= 0xDFFF) {
                    if (mode == umREPLACE_INVALID) {
                        code = UNICODE_REPLACEMENT_CHAR;
                    } else if (mode == umSKIP_INVALID) {
                        str = p;
                        continue;
                    } else if (mode == umSTRICT)
                        return END; // Invalid input using reserved surrogate value with mode umSTRICT
                }
                if (written >= outsize)
                    break;
                outbuf[written] = (wchar16)code;
                ++written;
            } else {
                // Surrogate pair
                if (written + 1 >= outsize)
                    break;
                code -= 0x10000;
                outbuf[written]   = (wchar16)((code >> 10) & 0x3FF) + 0xD800;
                outbuf[written+1] = (wchar16)(code & 0x3FF) + 0xDC00;
                written += 2;
            }
            str = p;
        }
    }
    return written;
}

///////////////////////////////////////////////////////////////////////////////

/** Scan for next Unicode character in UTF-16 string.
 - \#include <evo/strscan.h> or <evo/string.h> or <evo/substring.h>
 - A UTF-16 character may span 1 or 2 16-bit values (2 or 4 bytes), depending on the character
   - 2 byte values are the raw Unicode 16-bit code value
   - 4 byte values are pairs of reserved 16-bit values called surrogate pairs
 - Call this in a loop to iterate through Unicode characters in a UTF-16 string, stop when NULL is returned
 .
 \param  code  %Set to full Unicode character code scanned, or error code on error (see return value)  [out]
 \param  str   Pointer to string to scan, must not be ULL
 \param  end   Pointer to end of string to scan
 \param  mode  How to handle invalid UTF-16 values:
                - \ref umINCLUDE_INVALID - Invalid UTF-16 values are each returned as-is (1 character each)
                - \ref umREPLACE_INVALID - Invalid UTF-16 bytes are each replaced with \ref UNICODE_REPLACEMENT_CHAR
                - \ref umSKIP_INVALID - Invalid UTF-16 values are skipped (ignored)
                - \ref umSTRICT - Stop with error on invalid UTF-16 value, return NULL and set code=1
 \return       Pointer after scanned Unicode character, NULL with code=0 if stopped at end of string, NULL with code=1 on error (if mode=umSTRICT)
*/
inline const wchar16* utf16_scan(wchar32& code, const wchar16* str, const wchar16* end, UtfMode mode=umREPLACE_INVALID) {
    assert( str != NULL );
    assert( str <= end );
    while (str < end) {
        uint32 ch1 = (uint16)*str;
        if (!(ch1 < 0xD800 || ch1 > 0xDFFF)) {
            if (ch1 <= 0xDBFF && str+1 < end) {
                uint32 ch2 = (uint16)str[1];
                if (!(ch2 < 0xDC00 || ch2 > 0xDFFF)) {
                    // Valid surrogate pair
                    ch1 = ((ch1 - 0xD800) & 0x3FF) << 10;
                    ch2 = (ch2 - 0xDC00) & 0x3FF;
                    code = (ch1 | ch2) + 0x10000;
                    return str + 2;
                }
            }
            // Invalid surrogate pair
            if (mode == umREPLACE_INVALID) {
                code = UNICODE_REPLACEMENT_CHAR;
                return str + 1;
            } else if (mode == umSKIP_INVALID) {
                ++str;
                continue;
            } else if (mode == umSTRICT) {
                code = 1;
                return NULL;
            }
        }
        // Single char
        code = ch1;
        return str + 1;
    }
    code = 0;
    return NULL;
}

/** Scan for next Unicode character in terminated UTF-16 string.
 - \#include <evo/strscan.h> or <evo/string.h> or <evo/substring.h>
 - A UTF-16 character may span 1 or 2 16-bit values (2 or 4 bytes), depending on the character
   - 2 byte values are the raw Unicode 16-bit code value
   - 4 byte values are pairs of reserved 16-bit values called surrogate pairs
 - Call this in a loop to iterate through Unicode characters in a terminated UTF-16 string, stop when NULL is returned
 .
 \param  code  %Set to full Unicode character code scanned, or error code on error (see return value)  [out]
 \param  str   Pointer to string to scan, muts be terminated, must not be ULL
 \param  mode  How to handle invalid UTF-16 values:
                - \ref umINCLUDE_INVALID - Invalid UTF-16 values are each returned as-is (1 character each)
                - \ref umREPLACE_INVALID - Invalid UTF-16 bytes are each replaced with \ref UNICODE_REPLACEMENT_CHAR
                - \ref umSKIP_INVALID - Invalid UTF-16 values are skipped (ignored)
                - \ref umSTRICT - Stop with error on invalid UTF-16 value, return NULL and set code=1
 \return       Pointer after scanned Unicode character, NULL with code=0 if stopped at end of string, NULL with code=1 on error (if mode=umSTRICT)
*/
inline const wchar16* utf16_scan_term(wchar32& code, const wchar16* str, UtfMode mode=umREPLACE_INVALID) {
    assert( str != NULL );
    for (;;) {
        uint32 ch1 = (uint16)*str;
        if (ch1 == 0)
            break;
        if (!(ch1 < 0xD800 || ch1 > 0xDFFF)) {
            if (ch1 <= 0xDBFF && str[1] != 0) {
                uint32 ch2 = (uint16)str[1];
                if (!(ch2 < 0xDC00 || ch2 > 0xDFFF)) {
                    // Valid surrogate pair
                    ch1 = ((ch1 - 0xD800) & 0x3FF) << 10;
                    ch2 = (ch2 - 0xDC00) & 0x3FF;
                    code = (ch1 | ch2) + 0x10000;
                    return str + 2;
                }
            }
            // Invalid surrogate pair
            if (mode == umREPLACE_INVALID) {
                code = UNICODE_REPLACEMENT_CHAR;
                return str + 1;
            } else if (mode == umSKIP_INVALID) {
                ++str;
                continue;
            } else if (mode == umSTRICT) {
                code = 1;
                return NULL;
            }
        }
        // Single char
        code = ch1;
        return str + 1;
    }
    code = 0;
    return NULL;
}

/** %Compare two non-terminated UTF-16 strings.
 - \#include <evo/strscan.h> or <evo/string.h> or <evo/substring.h>
 - See also terminated variants: utf16_compare(const wchar16*,ulong,const wchar16*), utf16_compare(const wchar16*,const wchar16*)
 .
 \param  str1  First string to compare, must not be NULL
 \param  len1  First string length
 \param  str2  Second string to compare, must not be NULL
 \param  len2  Second string length
 \return       Result (<0 if this is less, 0 if equal, >0 if this is greater)
*/
inline int utf16_compare(const wchar16* str1, ulong len1, const wchar16* str2, ulong len2) {
    if (str1 == NULL) {
        if (str2 != NULL)
            return -1;
    } else if (str2 == NULL) {
        return 1;
    } else if (str1 == str2) {
        if (len1 < len2)
            return -1;
        else if (len1 > len2)
            return 1;
    } else {
        const wchar16* end1 = str1 + len1;
        const wchar16* end2 = str2 + len2;
        wchar32 code1, code2;
        for (;;) {
            str1 = utf16_scan(code1, str1, end1, umINCLUDE_INVALID);
            str2 = utf16_scan(code2, str2, end2, umINCLUDE_INVALID);
            if (str1 == NULL) {
                if (str2 == NULL)
                    break;
                return -1;
            } else if (str2 == NULL)
                return 1;
            else if (code1 < code2)
                return -1;
            else if (code1 > code2)
                return 1;
        }
    }
    return 0;
}

/** %Compare a non-terminated UTF-16 string with a terminated UTF-16 string.
 - \#include <evo/strscan.h> or <evo/string.h> or <evo/substring.h>
 - See also non-terminated variant: utf16_compare(const wchar16*,ulong,const wchar16*,ulong)
 .
 \param  str1  First string to compare, must not be NULL
 \param  len1  First string length
 \param  str2  Second string to compare, must be terminated, must not be NULL
 \return       Result (<0 if this is less, 0 if equal, >0 if this is greater)
*/
inline int utf16_compare(const wchar16* str1, ulong len1, const wchar16* str2) {
    if (str1 == NULL) {
        if (str2 != NULL)
            return -1;
    } else if (str2 == NULL) {
        return 1;
    } else {
        const wchar16* end1 = str1 + len1;
        wchar32 code1, code2;
        for (;;) {
            str1 = utf16_scan(code1, str1, end1, umINCLUDE_INVALID);
            str2 = utf16_scan_term(code2, str2, umINCLUDE_INVALID);
            if (str1 == NULL) {
                if (str2 == NULL)
                    break;
                return -1;
            } else if (str2 == NULL)
                return 1;
            else if (code1 < code2)
                return -1;
            else if (code1 > code2)
                return 1;
        }
    }
    return 0;
}

/** %Compare two terminated UTF-16 strings.
 - \#include <evo/strscan.h> or <evo/string.h> or <evo/substring.h>
 - See also non-terminated variant: utf16_compare(const wchar16*,ulong,const wchar16*,ulong)
 .
 \param  str1  First string to compare, must be terminated, must not be NULL
 \param  str2  Second string to compare, must be terminated, must not be NULL
 \return       Result (<0 if this is less, 0 if equal, >0 if this is greater)
*/
inline int utf16_compare(const wchar16* str1, const wchar16* str2) {
    if (str1 == NULL) {
        if (str2 != NULL)
            return -1;
    } else if (str2 == NULL) {
        return 1;
    } else {
        wchar32 code1, code2;
        for (;;) {
            str1 = utf16_scan_term(code1, str1, umINCLUDE_INVALID);
            str2 = utf16_scan_term(code2, str2, umINCLUDE_INVALID);
            if (str1 == NULL) {
                if (str2 == NULL)
                    break;
                return -1;
            } else if (str2 == NULL)
                return 1;
            else if (code1 < code2)
                return -1;
            else if (code1 > code2)
                return 1;
        }
    }
    return 0;
}

/** %Compare a non-terminated UTF-16 string to a non-terminated UTF-8 string.
 - \#include <evo/strscan.h> or <evo/string.h> or <evo/substring.h>
 - See also terminated variants: utf16_compare(const wchar16*,ulong,const char*), utf16_compare(const wchar16*,const char*)
 .
 \param  str1  First string to compare, must not be NULL
 \param  len1  First string length
 \param  str2  Second string to compare, must not be NULL
 \param  len2  Second string length
 \return       Result (<0 if this is less, 0 if equal, >0 if this is greater)
*/
inline int utf16_compare8(const wchar16* str1, ulong len1, const char* str2, ulong len2) {
    if (str1 == NULL) {
        if (str2 != NULL)
            return -1;
    } else if (str2 == NULL) {
        return 1;
    } else {
        const wchar16* end1 = str1 + len1;
        const char*    end2 = str2 + len2;
        wchar32 code1, code2;
        for (;;) {
            str1 = utf16_scan(code1, str1, end1, umINCLUDE_INVALID);
            str2 = utf8_scan(code2, str2, end2, umINCLUDE_INVALID);
            if (str1 == NULL) {
                if (str2 == NULL)
                    break;
                return -1;
            } else if (str2 == NULL)
                return 1;
            else if (code1 < code2)
                return -1;
            else if (code1 > code2)
                return 1;
        }
    }
    return 0;
}

/** %Compare a non-terminated UTF-16 string to a terminated UTF-8 string.
 - \#include <evo/strscan.h> or <evo/string.h> or <evo/substring.h>
 - See also non-terminated and terminated variants: utf16_compare(const wchar16*,ulong,const char*,ulong), utf16_compare(const wchar16*,const char*)
 .
 \param  str1  First string to compare, must not be NULL
 \param  len1  First string length
 \param  str2  Second string to compare, must be terminated, must not be NULL
 \return       Result (<0 if this is less, 0 if equal, >0 if this is greater)
*/
inline int utf16_compare8(const wchar16* str1, ulong len1, const char* str2) {
    if (str1 == NULL) {
        if (str2 != NULL)
            return -1;
    } else if (str2 == NULL) {
        return 1;
    } else {
        const wchar16* end1 = str1 + len1;
        wchar32 code1, code2;
        for (;;) {
            str1 = utf16_scan(code1, str1, end1, umINCLUDE_INVALID);
            str2 = utf8_scan_term(code2, str2, umINCLUDE_INVALID);
            if (str1 == NULL) {
                if (str2 == NULL)
                    break;
                return -1;
            } else if (str2 == NULL)
                return 1;
            else if (code1 < code2)
                return -1;
            else if (code1 > code2)
                return 1;
        }
    }
    return 0;
}

/** %Compare a terminated UTF-16 string to a terminated UTF-8 string.
 - \#include <evo/strscan.h> or <evo/string.h> or <evo/substring.h>
 - See also non-terminated variants: utf16_compare(const wchar16*,ulong,const char*,ulong), utf16_compare(const wchar16*,ulong,const char*)
 .
 \param  str1  First string to compare, must be terminated, must not be NULL
 \param  str2  Second string to compare, must be terminated, must not be NULL
 \return       Result (<0 if this is less, 0 if equal, >0 if this is greater)
*/
inline int utf16_compare8(const wchar16* str1, const char* str2) {
    if (str1 == NULL) {
        if (str2 != NULL)
            return -1;
    } else if (str2 == NULL) {
        return 1;
    } else {
        wchar32 code1, code2;
        for (;;) {
            str1 = utf16_scan_term(code1, str1, umINCLUDE_INVALID);
            str2 = utf8_scan_term(code2, str2, umINCLUDE_INVALID);
            if (str1 == NULL) {
                if (str2 == NULL)
                    break;
                return -1;
            } else if (str2 == NULL)
                return 1;
            else if (code1 < code2)
                return -1;
            else if (code1 > code2)
                return 1;
        }
    }
    return 0;
}

/** Find terminated UTF-16 string length.
 - \#include <evo/strscan.h> or <evo/string.h> or <evo/substring.h>
 .
 \param  str  Pointer to string, must be NULL terminated, NULL for none
 \return      %String length in UTF-16 values (not bytes), 0 if str=NULL
*/
inline ulong utf16_strlen(const wchar16* str) {
    if (str == NULL)
        return 0;
    const wchar16* p = str;
    while (*p != 0)
        ++p;
    return (ulong)(p - str);
}

/** Scan for UTF-16 surrogate pairs, which each require a pair of wchar16 values (4 bytes).
 - \#include <evo/strscan.h> or <evo/string.h> or <evo/substring.h>
 - Surrogate pairs are used for Unicode code points 0x10000 to 0x10FFFF, which are less common
 - Valid UTF-16 without surrogate pairs is also valid UCS-2 (a precursor to UTF-16)
 .
 \param  str       Pointer to string to search, must not be NULL
 \param  end       Pointer to end of string to scan, must be >= `str`
 \param  strict    Whether to stop counting on invalid UTF-16 value, false to skip/ignore invalid UTF-16 values
 \param  mincount  Minimum number of surrogate pairs to look for, must be positive
 \return           Pointer to UTF-16 character that met mincount, NULL if less than `mincount` surrogate pairs found
*/
inline const wchar16* utf16_min(const wchar16* str, const wchar16* end, bool strict=false, uint mincount=1) {
    assert( str != NULL );
    assert( str <= end );
    assert( mincount > 0 );
    uint count = 0;
    while (str < end) {
        uint32 ch1 = (uint16)*str;
        if (!(ch1 < 0xD800 || ch1 > 0xDFFF)) {
            if (ch1 <= 0xDBFF && str+1 < end) {
                uint32 ch2 = (uint16)str[1];
                if (!(ch2 < 0xDC00 || ch2 > 0xDFFF)) {
                    // Valid surrogate pair
                    if (++count >= mincount)
                        return str;
                    str += 2;
                    continue;
                }
            }
            // Invalid surrogate pair
            if (strict)
                break;
        }
        // Single char
        ++str;
    }
    return NULL;
}

/** Count Unicode character values in UTF-16 string.
 - \#include <evo/strscan.h> or <evo/string.h> or <evo/substring.h>
 - Each UTF-16 character may span 1 or 2 16-bit values (2 or 4 bytes), depending on the character
   - 2 byte values are the raw Unicode 16-bit code value
   - 4 byte values are pairs of reserved 16-bit values called surrogate pairs
 .
 \param  str   Pointer to string to scan, must not be NULL
 \param  end   Pointer to end of string to scan, must be >= `str`
 \param  mode  How to handle invalid UTF-16 values:
                - \ref umINCLUDE_INVALID - Invalid UTF-16 values are counted as 1 character each
                - \ref umREPLACE_INVALID - Invalid UTF-16 values are counted as 1 character each (\ref UNICODE_REPLACEMENT_CHAR)
                - \ref umSKIP_INVALID - Invalid UTF-16 values are skipped and not counted
                - \ref umSTRICT - Stop counting on invalid UTF-16 value and return an error
 \return       Number of Unicode characters counted, NONE on error (if mode=umSTRICT)
*/
inline ulong utf16_count(const wchar16* str, const wchar16* end, UtfMode mode=umREPLACE_INVALID) {
    assert( str != NULL );
    assert( str <= end );
    ulong count = 0;
    while (str < end) {
        uint32 ch1 = (uint16)*str;
        if (!(ch1 < 0xD800 || ch1 > 0xDFFF)) {
            if (ch1 <= 0xDBFF && str+1 < end) {
                uint32 ch2 = (uint16)str[1];
                if (!(ch2 < 0xDC00 || ch2 > 0xDFFF)) {
                    // Valid surrogate pair
                    ++count;
                    str += 2;
                    continue;
                }
            }
            // Invalid surrogate pair
            if (mode == umSKIP_INVALID) {
                ++str;
                continue;
            } else if (mode == umSTRICT)
                return NONE;
        }
        // Single char
        ++count;
        ++str;
    }
    return count;
}

/** %Convert UTF-16 string to UTF-8 string.
 - \#include <evo/strscan.h> or <evo/string.h> or <evo/substring.h>
 .
 \param  str      Pointer to input UTF-16 string to convert, set to stopping point on return, must not be NULL  [in/out]
 \param  end      Pointer to stop input at (end of input, exclusive), must be >= `str`
 \param  outbuf   Output buffer to write UTF-8 string to, NULL to scan without writing
 \param  outsize  Output buffer size in bytes, ignored if outbuf=NULL
 \param  mode     How to handle invalid UTF-16 values:
                   - \ref umINCLUDE_INVALID - Invalid UTF-16 values are converted as-is (1 character each)
                   - \ref umREPLACE_INVALID - Invalid UTF-16 values are each replaced with \ref UNICODE_REPLACEMENT_CHAR
                   - \ref umSKIP_INVALID - Invalid UTF-16 values are skipped and ignored
                   - \ref umSTRICT - Stop on invalid input with an error
 \return          Count written to outbuf in bytes, END on invalid input (if mode=umSTRICT)
*/
inline ulong utf16_to8(const wchar16*& str, const wchar16* end, char* outbuf=NULL, ulong outsize=0, UtfMode mode=umREPLACE_INVALID) {
    assert( str != NULL );
    assert( str <= end );
    ulong written = 0;
    wchar32 code;
    const wchar16* p;
    if (outbuf == NULL) {
        // Count UTF-8 size required (no writes)
        for (;;) {
            if ((p = utf16_scan(code, str, end, mode)) == NULL) {
                if (code == 1)
                    return END; // Invalid input with mode umSTRICT
                break;
            }
            str = p;
            if (code < 0x0080) {
                // 1 byte char
                ++written;
            } else if (code < 0x0800) {
                // 2 byte char
                written += 2;
            } else if (code < 0x10000) {
                // 3 byte char
                written += 3;
            } else {
                // 4 byte char
                assert( code <= 0x10FFFF );
                written += 4;
            }
        }
    } else {
        // Write UTF-16
        const uchar LBITS_1      = 0x80;
        const uchar LBITS_11     = 0xC0;
        const uchar LBITS_111    = 0xE0;
        const uchar LBITS_1111   = 0xF0;
        const uchar RBITS_111111 = 0x3F;
        const uchar RBITS_11111  = 0x1F;
        const uchar RBITS_1111   = 0x0F;
        const uchar RBITS_111    = 0x07;
        uchar* out = (uchar*)outbuf;
        for (;;) {
            if ((p = utf16_scan(code, str, end, mode)) == NULL) {
                if (code == 1)
                    return END; // Invalid input with mode umSTRICT
                break;
            }
            if (code < 0x0080) {
                // 1 byte char
                if (written >= outsize)
                    break;
                out[written] = (uchar)code;
                ++written;
            } else if (code < 0x0800) {
                // 2 byte char
                if (written + 1 >= outsize)
                    break;
                outbuf[written]   = ((uchar)(code >> 6) & RBITS_11111) | LBITS_11;
                outbuf[written+1] = ((uchar)code & RBITS_111111) | LBITS_1;
                written += 2;
            } else if (code < 0x10000) {
                // 3 byte char
                if (written + 2 >= outsize)
                    break;
                outbuf[written]   = ((uchar)(code >> 12) & RBITS_1111) | LBITS_111;
                outbuf[written+1] = ((uchar)(code >> 6) & RBITS_111111) | LBITS_1;
                outbuf[written+2] = ((uchar)code & RBITS_111111) | LBITS_1;
                written += 3;
            } else {
                // 4 byte char
                assert( code <= 0x10FFFF );
                if (written + 3 >= outsize)
                    break;
                outbuf[written]   = ((uchar)(code >> 18) & RBITS_111) | LBITS_1111;
                outbuf[written+1] = ((uchar)(code >> 12) & RBITS_111111) | LBITS_1;
                outbuf[written+2] = ((uchar)(code >> 6) & RBITS_111111) | LBITS_1;
                outbuf[written+3] = ((uchar)code & RBITS_111111) | LBITS_1;
                written += 4;
            }
            str = p;
        }
    }
    return written;
}

///////////////////////////////////////////////////////////////////////////////

/** Evo implementation of memrchr() to search for character in reverse.
 - \#include <evo/strscan.h> or <evo/string.h> or <evo/substring.h>
 - This searches for a character in reverse and returns a pointer to it, or `NULL` if not found
 - This calls `memrchr()` (GNU C extension) when possible, otherwise this falls back to an equivalent implementation
 .
 \param  str   %String pointer to search
 \param  ch    Character to search for
 \param  size  %String size to search, search starts at the end
 \return       Pointer to last occurence of `ch`, NULL if not found
*/
inline const char* string_memrchr(const char* str, char ch, size_t size) {
    #if !defined(EVO_NO_MEMRCHR) && defined(EVO_GLIBC_MEMRCHR)
        return (char*)::memrchr(str, ch, size);
    #else
        for (const char* p = str + size; p > str; )
            if (*--p == ch)
                return p;
        return NULL;
    #endif
}

///////////////////////////////////////////////////////////////////////////////

/** %String search algorithm selection. */
enum StringSearchAlg {
    ssaDEFAULT=0,   ///< Default, uses system memmem() if possible (define EVO_NO_MEMMEM to prevent this), otherwise same as ssaKMP
    ssaKMP,         ///< Knuth-Morris-Pratt using partial match table
    ssaBASIC        ///< Basic search using memchr() and memcmp()
};

// Implementation
/** \cond impl */
namespace impl {
    // String search alg helpers

    // Knuth-Morris-Pratt based search -- requires table[pattern_size]
    template<class T>
    inline ulong string_search_impl_kmp(T* table, const char* pattern, uint pattern_size, const char* data, ulong data_size, ulong offset=0) {
        assert( pattern_size > 1 );
        assert( data_size > pattern_size );

        // Build partial match table
        table[0] = 0;
        for (uint len = 0, i = 1; i < pattern_size; ) {
            assert( len < pattern_size );
            if (pattern[i] == pattern[len]) {
                table[i] = (T)++len;
                ++i;                    
            } else if (len != 0) {
                len = table[len - 1];
            } else {
                table[i] = 0;
                ++i;
            }
        }

        // Do search
        for (ulong i = 0, j = 0; i < data_size; ) {
            if (data[i] == pattern[j]) {
                ++i;
                if (++j == pattern_size)
                    return offset + (i - j);
            } else if (j == 0) {
                ++i;
            } else
                j = table[j - 1];
        }
        return NONE;
    }

    // Knuth-Morris-Pratt based reverse search -- requires table[pattern_size]
    template<class T>
    inline ulong string_search_impl_kmp_reverse(T* table, const char* pattern, uint pattern_size, const char* data, ulong data_size, ulong offset=0) {
        // Mostly the same as the forward version, arrays are just indexed in reverse (so 0 is last char, 1 is 2nd last, etc)
        assert( pattern_size > 1 );
        assert( data_size > pattern_size );
        const uint pend = pattern_size - 1;

        // Build partial match table
        table[0] = 0;
        for (uint len = 0, i = 1; i < pattern_size; ) {
            assert( len < pattern_size );
            if (pattern[pend - i] == pattern[pend - len]) {
                table[i] = (T)++len;
                ++i;                    
            } else if (len != 0) {
                len = table[len - 1];
            } else {
                table[i] = 0;
                ++i;
            }
        }

        // Do search
        const uint dend = data_size - 1;
        for (ulong i = 0, j = 0; i < data_size; ) {
            if (data[dend - i] == pattern[pend - j]) {
                ++i;
                if (++j == pattern_size)
                    return offset + (data_size - (i - j) - pattern_size);
            } else if (j == 0) {
                ++i;
            } else
                j = table[j - 1];
        }
        return NONE;
    }

    // Forward search

    // Find pattern in string, offset is added to result if found
    inline ulong string_search(const char* pattern, uint pattern_size, const char* data, ulong data_size, ulong offset) {
        if (pattern_size > 0 && pattern_size <= data_size) {
            if (pattern_size == 1) {
                // Special case for single char
                const char* ptr = (char*)memchr(data, *pattern, data_size);
                if (ptr != NULL)
                    return offset + (ulong)(ptr - data);
            } else if (pattern_size == data_size) {
                // Special case for pattern and key being same size
                if (memcmp(data, pattern, data_size) == 0)
                    return offset;
            } else {
            #if !defined(EVO_NO_MEMMEM) && defined(EVO_GLIBC_MEMMEM)
                // memmem()
                const char* ptr = (char*)memmem(data, data_size, pattern, pattern_size);
                if (ptr != NULL)
                    return offset + (ulong)(ptr - data);
            #else
                // KMP
                const uint8 STACK_BUF_SIZE = 128;
                if (pattern_size <= STACK_BUF_SIZE) {
                    // Smaller patterns use stack table
                    uint8 table[STACK_BUF_SIZE];
                    return string_search_impl_kmp<uint8>(table, pattern, pattern_size, data, data_size, offset);
                } else {
                    // Larger patterns must allocate a table
                    uint* table = (uint*)::malloc(pattern_size * sizeof(uint));
                    const ulong result = string_search_impl_kmp<uint>(table, pattern, pattern_size, data, data_size, offset);
                    ::free(table);
                    return result;
                }
            #endif
            }
        }
        return NONE;
    }

    // string_search() variant
    inline ulong string_search_kmp(const char* pattern, uint pattern_size, const char* data, ulong data_size, ulong offset) {
        if (pattern_size > 0 && pattern_size <= data_size) {
            if (pattern_size == 1) {
                // Special case for single char
                const char* ptr = (char*)memchr(data, *pattern, data_size);
                if (ptr != NULL)
                    return offset + (ulong)(ptr - data);
            } else if (pattern_size == data_size) {
                // Special case for pattern and key being same size
                if (memcmp(data, pattern, data_size) == 0)
                    return offset;
            } else {
                const uint8 STACK_BUF_SIZE = 128;
                if (pattern_size <= STACK_BUF_SIZE) {
                    // Smaller patterns use stack table
                    uint8 table[STACK_BUF_SIZE];
                    return string_search_impl_kmp<uint8>(table, pattern, pattern_size, data, data_size, offset);
                } else {
                    // Larger patterns must allocate a table
                    uint* table = (uint*)::malloc(pattern_size * sizeof(uint));
                    const ulong result = string_search_impl_kmp<uint>(table, pattern, pattern_size, data, data_size, offset);
                    ::free(table);
                    return result;
                }
            }
        }
        return NONE;
    }

    // string_search() variant
    inline ulong string_search_basic(const char* pattern, uint pattern_size, const char* data, ulong data_size, ulong offset) {
        if (pattern_size > 0 && pattern_size <= data_size) {
            if (pattern_size == 1) {
                // Special case for single char
                const char* ptr = (char*)memchr(data, *pattern, data_size);
                if (ptr != NULL)
                    return offset + (ulong)(ptr - data);
            } else if (pattern_size == data_size) {
                // Special case for pattern and key being same size
                if (memcmp(data, pattern, data_size) == 0)
                    return offset;
            } else {
                data_size = data_size + 1 - pattern_size; // no space for pattern after here
                const uchar pattern_first = (uchar)*pattern;
                const char* start = data;
                const char* end   = data + data_size;
                while (data < end) {
                    if ((data = (char*)memchr(data, pattern_first, data_size)) == NULL)
                        break;
                    if (memcmp(data, pattern, pattern_size) == 0)
                        return offset + (ulong)(data - start);
                    ++data;
                    data_size = (ulong)(end - data);
                }
            }
        }
        return NONE;
    }

    // Select a string_search() variant dynamically
    inline ulong string_search(StringSearchAlg alg, const char* pattern, uint pattern_size, const char* data, ulong data_size, ulong offset) {
        switch (alg) {
            case ssaKMP:
                return string_search_kmp(pattern, pattern_size, data, data_size, offset);
            case ssaBASIC:
                return string_search_basic(pattern, pattern_size, data, data_size, offset);
            default:
                return string_search(pattern, pattern_size, data, data_size, offset);
        }
    }

    // Reverse search

    // Find pattern in string with reverse search, offset is added to result if found -- default uses KMP
    inline ulong string_search_reverse(const char* pattern, uint pattern_size, const char* data, ulong data_size, ulong offset) {
        if (pattern_size > 0 && pattern_size <= data_size) {
            if (pattern_size == 1) {
                // Special case for single char
                #if !defined(EVO_NO_MEMRCHR) && defined(EVO_GLIBC_MEMRCHR)
                    const char* ptr = (char*)memrchr(data, *pattern, data_size);
                    if (ptr != NULL)
                        return offset + (ulong)(ptr - data);
                #else
                    const uchar pattern_first = (uchar)*pattern;
                    for (const char* ptr = data + data_size; ptr > data; )
                        if (*--ptr == pattern_first)
                            return offset + (ulong)(ptr - data);
                #endif
            } else if (pattern_size == data_size) {
                // Special case for pattern and key being same size
                if (memcmp(data, pattern, data_size) == 0)
                    return offset;
            } else {
                const uint8 STACK_BUF_SIZE = 128;
                if (pattern_size <= STACK_BUF_SIZE) {
                    // Smaller patterns use stack table
                    uint8 table[STACK_BUF_SIZE];
                    return string_search_impl_kmp_reverse<uint8>(table, pattern, pattern_size, data, data_size, offset);
                } else {
                    // Larger patterns must allocate a table
                    uint* table = (uint*)::malloc(pattern_size * sizeof(uint));
                    const ulong result = string_search_impl_kmp_reverse<uint>(table, pattern, pattern_size, data, data_size, offset);
                    ::free(table);
                    return result;
                }
            }
        }
        return NONE;
    }

    // string_search_reverse() variant
    inline ulong string_search_reverse_basic(const char* pattern, uint pattern_size, const char* data, ulong data_size, ulong offset) {
        if (pattern_size > 0 && pattern_size <= data_size) {
            if (pattern_size == 1) {
                // Special case for single char
                #if !defined(EVO_NO_MEMRCHR) && defined(EVO_GLIBC_MEMRCHR)
                    const char* ptr = (char*)memrchr(data, *pattern, data_size);
                    if (ptr != NULL)
                        return offset + (ulong)(ptr - data);
                #else
                    const uchar pattern_first = (uchar)*pattern;
                    for (const char* ptr = data + data_size; ptr > data; )
                        if (*--ptr == pattern_first)
                            return offset + (ulong)(ptr - data);
                #endif
            } else if (pattern_size == data_size) {
                // Special case for pattern and key being same size
                if (memcmp(data, pattern, data_size) == 0)
                    return offset;
            } else {
                data_size = data_size + 1 - pattern_size; // no space for pattern after here
                const uchar pattern_first = (uchar)*pattern;
                const char* ptr;
            #if !defined(EVO_NO_MEMRCHR) && defined(EVO_GLIBC_MEMRCHR)
                for (;;) {
                    if ((ptr = (char*)memrchr(data, pattern_first, data_size)) == NULL)
                        break;
                    data_size = (ulong)(ptr - data);
                    if (memcmp(ptr, pattern, pattern_size) == 0)
                        return offset + data_size;
                    if (ptr == data)
                        break;
                }
            #else
                ptr = data + data_size; // data_size adjusted above so there's no space for pattern after here
                while (ptr > data)
                    if (*--ptr == pattern_first && memcmp(ptr, pattern, pattern_size) == 0)
                        return offset + (ulong)(ptr - data);
            #endif
            }
        }
        return NONE;
    }

    // Select a string_search_reverse() variant dynamically
    inline ulong string_search_reverse(StringSearchAlg alg, const char* pattern, uint pattern_size, const char* data, ulong data_size, ulong offset) {
        switch (alg) {
            case ssaBASIC:
                return string_search_reverse_basic(pattern, pattern_size, data, data_size, offset);
            case ssaKMP:
            default:
                return string_search_reverse(pattern, pattern_size, data, data_size, offset);
        }
    }
}
/** \endcond */

///////////////////////////////////////////////////////////////////////////////

// Implementation
/** \cond impl */
namespace impl {
    // Convert string to integer value
    template<class T>
    T tonum(const char* str, ulong size, Error& error, int base) {
        const char* const end = str + size;
        bool neg  = false;
        typename ToUnsigned<T>::Type num = 0;
        uchar ch;

        // Prefix
        while (str < end && ((ch=*str) == ' ' || ch == '\t'))
            ++str;
        if (str < end) {
            if ((ch=*str) == '+')
                ++str;
            else if (ch == '-')
                { neg = true; ++str; }
        }
        if (str == end)
            { error = EInval; return 0; }

        switch (base) {
            case 0:
                // Detect base
                switch (*str) {
                    case '0':
                        if (++str < end) {
                            switch (*str) {
                                case 'X': // fallthrough
                                case 'x':
                                    // Hex
                                    base = 16;
                                    if (++str == end)
                                        { error = EInval; return 0; }
                                    break;
                                case 'O': // fallthrough
                                case 'o':
                                    // Octal
                                    base = 8;
                                    if (++str == end)
                                        { error = EInval; return 0; }
                                    break;
                                case 'B': // fallthrough
                                case 'b':
                                    // Binary
                                    base = 2;
                                    if (++str == end)
                                        { error = EInval; return 0; }
                                    break;
                                default:
                                    // Octal
                                    base = 8;
                            }
                        } else
                            // Decimal
                            base = 10;
                        break;
                    case 'x':
                        // Hex
                        base = 16;
                        if (++str == end)
                            { error = EInval; return 0; }
                        break;
                    case 'o':
                        // Octal
                        base = 8;
                        if (++str == end)
                            { error = EInval; return 0; }
                        break;
                    case 'b':
                        // Binary
                        base = 2;
                        if (++str == end)
                            { error = EInval; return 0; }
                        break;
                    default:
                        // Decimal
                        base = 10;
                        break;
                }
                break;
            case 16:
                // Skip hex prefix
                if (*str == 'x')
                    ++str;
                else if (*str == '0' && str+1 < end && (str[1] == 'x' || str[1] == 'X'))
                    str += 2;
                if (str == end)
                    { error = EInval; return 0; }
                break;
            case 8:
                // Skip octal prefix
                if (*str == 'o')
                    ++str;
                else if (*str == '0' && str+1 < end && (str[1] == 'o' || str[1] == 'O'))
                    str += 2;
                if (str == end)
                    { error = EInval; return 0; }
                break;
            case 2:
                // Skip binary prefix
                if (*str == 'b')
                    ++str;
                else if (*str == '0' && str+1 < end && (str[1] == 'b' || str[1] == 'B'))
                    str += 2;
                if (str == end)
                    { error = EInval; return 0; }
                break;
            default:
                break;
        }

        // Limits
        const typename ToUnsigned<T>::Type limitnum = (neg ? (IntegerT<T>::SIGN ? -(typename ToSigned<T>::Type)IntegerT<T>::MIN : IntegerT<T>::MAX) : IntegerT<T>::MAX);
        const typename ToUnsigned<T>::Type limitbase = limitnum / (typename ToUnsigned<T>::Type)base;

    #if defined(EVO_OLDCC)
        const T MIN = IntegerT<T>::MIN;
        const T MAX = IntegerT<T>::MAX;
    #endif

        // Number
        error = ENone;
        for (; str < end; ++str) {
            ch = *str;
            if (ch >= '0' && ch <= '9')
                ch -= '0';
            else if (ch >= 'A' && ch <= 'Z')
                ch = ch - 'A' + 10;
            else if (ch >= 'a' && ch <= 'z')
                ch = ch - 'a' + 10;
            else {
                // Not a digit
                if (str < end && *str == '.' && base == 10) {
                    // Ignore fraction
                    ++str;
                    while (str < end && (ch=*str) >= '0' && ch <= '9')
                        ++str;
                }
                while (str < end && ((ch=*str) == ' ' || ch == '\t'))
                    ++str;
                if (str == end)
                    break; // ending whitespace ok
                error = EInval;
                break;
            }
            if (ch >= base)
                { error = EInval; break; }
            assert( ch < base );
            if (num > limitbase || (num == limitbase && ch > (limitnum % base))) {
                error = EOutOfBounds;
            #if defined(EVO_OLDCC) // fixes undefined reference on older compilers
                return (neg ? MIN : MAX);
            #else
                return (neg ? IntegerT<T>::MIN : IntegerT<T>::MAX);
            #endif
            }
            num *= (typename ToUnsigned<T>::Type)base;
            num += ch;
        }
        return (neg ? -(typename ToSigned<T>::Type)num : num);
    }

    // Convert string to floating-point value
    template<class T>
    T tonumf(const char* str, ulong size, Error& error) {
        const int MAXDIGITS = FloatT<T>::MAXDIGITS + 2;
        const int BASE = 10;
        const char* end = str + size;
        bool neg = false;
        uchar ch;

        // Trim ending whitespace
        while ( str < end && ((ch=end[-1]) == ' ' || ch == '\t') )
            --end;

        // Prefix
        while ( str < end && ((ch=*str) == ' ' || ch == '\t') )
            ++str;
        if (str < end) {
            switch (*str) {
                case '+': ++str; break;
                case '-': neg = true; ++str; break;
            }
        } else {
            error = EInval;
            return 0.0;
        }

        // INF, NaN
        error = ENone;
        switch (*str) {
            case 'i':
            case 'I':
                if ( end-str >= 3 &&
                     ((ch=str[1]) == 'n' || ch == 'N') &&
                     ((ch=str[2]) == 'f' || ch == 'F') )
                    return (neg ? -FloatT<T>::inf() : FloatT<T>::inf());
                break;
            case 'n':
            case 'N':
                if ( FloatT<T>::NANOK && end-str >= 3 &&
                     ((ch=str[1]) == 'a' || ch == 'A') &&
                     ((ch=str[2]) == 'n' || ch == 'N') )
                    return FloatT<T>::nan();
                break;
        }

        // Working data
        bool   found_point = false;
        ulongl num         = 0;
        int    exp         = 0;
        int    exp_digits  = 0;
        int    sig_digits  = 0;
        int    digits      = 0;

        // Significant digits
        while (str < end) {
            ch = *str;
            if (ch >= '0' && ch <= '9')
                // Decimal digit
                ch -= '0';
            else if (ch == '.') {
                // Decimal point
                if (found_point)
                    { error = EInval; return 0.0; }
                found_point = true;
                ++str;
                continue;
            } else if (ch == 'e' || ch == 'E') {
                // Exponent
                if (digits == 0)
                    { error = EInval; break; }
                if (++str < end) {
                    bool exp_neg = false;
                    switch (*str) {
                        case '+': ++str; break;
                        case '-': exp_neg = true; ++str; break;
                    }
                    for (; str < end; ++str) {
                        if ((ch=*str) >= '0' && ch <= '9')
                            ch -= '0';
                        else
                            break;
                        exp *= BASE;
                        exp += ch;
                    }
                    if (exp_neg)
                        exp = -exp;
                }
                break;
            } else if (ch == '#') {
                // MSVC inf/nan
                ++str;
                if ( end-str >= 3 &&
                    ((ch=str[0]) == 'I' || ch == 'i') &&
                    ((ch=str[1]) == 'N' || ch == 'n') &&
                    ((ch=str[2]) == 'F' || ch == 'f') )
                    return (neg ? -FloatT<T>::inf() : FloatT<T>::inf());
                return FloatT<T>::nan();
            } else {
                // Invalid character
                error = EInval;
                break;
            }

            // Don't start counting digits until first significant digit
            if (found_point)
                ++exp_digits;
            if (sig_digits != 0 || ch != 0) {
                // Apply next digit
                if (sig_digits > MAXDIGITS) {
                    ++exp;
                } else {
                    num *= BASE;
                    num += ch;
                }
                ++sig_digits;
            }
            ++digits; ++str;
        }
        if (digits == 0 || str != end)
            error = EInval;

        // Result
        T result;
        if (exp > FloatT<T>::maxexp())
            { result = FloatT<T>::inf(); error = EOutOfBounds; }
        else if (exp < FloatT<T>::minexp())
            { result = -FloatT<T>::inf(); error = EOutOfBounds; }
        else
            result = FloatT<T>::exp10((T)num, -exp_digits+exp);
        if (neg)
            result = -result;
        return result;
    }

    // Convert string to bool value
    template<class Size>
    bool tobool(const char* str, Size size, Error& error) {
        #define EVO_TMP_GET_CHAR_TOUPPER(INDEX) ch = str[INDEX]; if (ch >= 'a') ch -= 32;
        #define EVO_TMP_ELSE_INVALID else { error = EInval; return false; }
        #define EVO_TMP_TRUE_IF_CHAR(VAL) if (ch == VAL) { error = ENone; return true; } EVO_TMP_ELSE_INVALID

        char ch;
        while (size > 0 && ( (ch=*str) == ' ' || ch == '\t' ))
            { ++str; --size; }
        while (size > 0 && ( (ch=str[size-1]) == ' ' || ch == '\t' ))
            --size;

        switch (size) {
            case 1:
                EVO_TMP_GET_CHAR_TOUPPER(0);
                if ( ch == 'T' || ch == 'Y' || (ch >= '1' && ch <= '9') ) {
                    error = ENone;
                    return true;
                } else if ( ch == 'F' || ch == 'N' || ch == '0' )
                    error = ENone;
                else
                    error = EInval;
                return false;
            case 2:
                EVO_TMP_GET_CHAR_TOUPPER(0);
                if (ch == 'O') {
                    EVO_TMP_GET_CHAR_TOUPPER(1);
                    EVO_TMP_TRUE_IF_CHAR('N');
                }
                break;
            case 3:
                EVO_TMP_GET_CHAR_TOUPPER(0);
                if (ch == 'O') {
                    EVO_TMP_GET_CHAR_TOUPPER(1);
                    if (ch == 'F') {
                        EVO_TMP_GET_CHAR_TOUPPER(2);
                        if (ch == 'F') {
                            error = ENone;
                            return false;
                        } EVO_TMP_ELSE_INVALID;
                    } EVO_TMP_ELSE_INVALID;
                } else if (ch == 'Y') {
                    EVO_TMP_GET_CHAR_TOUPPER(1);
                    if (ch == 'E') {
                        EVO_TMP_GET_CHAR_TOUPPER(2);
                        EVO_TMP_TRUE_IF_CHAR('S');
                    } EVO_TMP_ELSE_INVALID;
                }
                break;
            case 4:
                EVO_TMP_GET_CHAR_TOUPPER(0);
                if (ch == 'T') {
                    EVO_TMP_GET_CHAR_TOUPPER(1);
                    if (ch == 'R') {
                        EVO_TMP_GET_CHAR_TOUPPER(2);
                        if (ch == 'U') {
                            EVO_TMP_GET_CHAR_TOUPPER(3);
                            EVO_TMP_TRUE_IF_CHAR('E');
                        } EVO_TMP_ELSE_INVALID;
                    } EVO_TMP_ELSE_INVALID;
                }
                break;
            case 5:
                EVO_TMP_GET_CHAR_TOUPPER(0);
                if (ch == 'F') {
                    EVO_TMP_GET_CHAR_TOUPPER(1);
                    if (ch == 'A') {
                        EVO_TMP_GET_CHAR_TOUPPER(2);
                        if (ch == 'L') {
                            EVO_TMP_GET_CHAR_TOUPPER(3);
                            if (ch == 'S') {
                                EVO_TMP_GET_CHAR_TOUPPER(4);
                                if (ch == 'E') {
                                    error = ENone;
                                    return false;
                                } EVO_TMP_ELSE_INVALID;
                            } EVO_TMP_ELSE_INVALID;
                        } EVO_TMP_ELSE_INVALID;
                    } EVO_TMP_ELSE_INVALID;
                }
                break;
            default:
                break;
        }
        return (tonum<ulong>(str, size, error, 0) != 0);

        #undef EVO_TMP_GET_CHAR_TOUPPER
        #undef EVO_TMP_ELSE_INVALID
        #undef EVO_TMP_TRUE_IF_CHAR
    }

    // Format signed integer as string
    template<class T>
    static ulong fnum(char* endptr, T num, int base) {
        assert( endptr != NULL );
        assert( base >= 0 );

        const char* digits;
        if (base >= 100) {
            assert( base <= 136 );
            base -= 100;
            digits = "0123456789abcdefghijklmnopqrstuvwxyz";
        } else {
            assert( base <= 36 );
            digits = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
        }

        char* ptr = endptr;
        if (num == 0)
            *--ptr = '0';
        else if (num > 0) {
            while (num != 0) {
                *--ptr = digits[num % (T)base];
                num /= (T)base;
            }
        } else {
            typename ToUnsigned<T>::Type unum = (typename ToUnsigned<T>::Type)-num;
            while (unum != 0) {
                *--ptr = digits[unum % (T)base];
                unum /= (T)base;
            }
            *--ptr = '-';
        }
        return (ulong)(endptr - ptr);
    }
    
    // Format unsigned integer as string
    template<class T>
    static ulong fnumu(char* endptr, T num, int base) {
        assert( endptr != NULL );
        assert( base >= 0 );

        const char* digits;
        if (base >= 100) {
            assert( base <= 136 );
            base -= 100;
            digits = "0123456789abcdefghijklmnopqrstuvwxyz";
        } else {
            assert( base <= 36 );
            digits = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
        }

        char* ptr = endptr;
        if (num == 0)
            *--ptr = '0';
        else
            while (num != 0) {
                *--ptr = digits[num % (T)base];
                num /= (T)base;
            }
        return (ulong)(endptr - ptr);
    }

    // Weight floating point number so it's rounded to given precision
    template<class T>
    static T fnumf_weight(T num, int precision) {
        T dummy;
        if (num != 0.0 && evo_modf(num, &dummy) != 0.0) {
            T weight;
            if (precision < 0 || precision > 6) {
                weight = 0.5;
                for (int i=0; i<precision; ++i)
                    weight /= 10.0;
            } else {
                const T TABLE[] = { (T)0.5, (T)0.05, (T)0.005, (T)0.0005, (T)0.00005, (T)0.000005, (T)0.0000005 };
                weight = TABLE[precision];
            }

            if (num < 0.0)
                num -= weight;
            else
                num += weight;
        }
        return num;
    }

    // Used by fnumf*() below
    template<class T> struct FloatNumF {
        static T dig_roundup()       { return 0.00001f; }
        static T dig_roundup_limit() { return 0.1f; }
    };
    template<> struct FloatNumF<double> {
        static double dig_roundup()       { return 0.00001; }
        static double dig_roundup_limit() { return 0.1; }
    };
    template<> struct FloatNumF<ldouble> {
        static ldouble dig_roundup()       { return 0.00001L; }
        static ldouble dig_roundup_limit() { return 0.1L; }
    };

    // Format normalized floating point number as string using given precision
    template<class T>
    static ulong fnumf(char* ptr, T num, int exp, int precision) {
        const T PRECISION = FloatT<T>::precision();
        const char* startptr = ptr;

        // NaN
        if (FloatT<T>::nan(num)) {
            ptr[0] = 'n';
            ptr[1] = 'a';
            ptr[2] = 'n';
            return 3;
        }

        // Negative
        if (num < 0.0) {
            *ptr = '-'; ++ptr;
            num = -num;
        }

        // inf or -inf
        if (FloatT<T>::inf(num)) {
            ptr[0] = 'i';
            ptr[1] = 'n';
            ptr[2] = 'f';
            ptr += 3;
            return (ulong)(ptr - startptr);
        }

        // Fraction leading zeroes
        if (exp <= 0) {
            *ptr = '0'; ++ptr;
            if (precision <= 0)
                return (ulong)(ptr - startptr); // Done, ignore fraction
            *ptr = '.'; ++ptr;
            while (exp < 0 && precision > 0) {
                *ptr = '0'; ++ptr;
                ++exp; --precision;
            }
        }

        // This prevents some cases where formatting comes out rounded down
        num += FloatT<T>::eps();

        // Significant digits
        T digit_roundup = FloatNumF<T>::dig_roundup(); // Used to round out float noise on each digit
        int digit = 0, count = 0, digit_roundup_counter = FloatT<T>::MAXDIGITS - 1;
        while (num > PRECISION) {
            // Round out float noise more aggressively when reaching precision limit
            if (--digit_roundup_counter == 0)
                digit_roundup = FloatNumF<T>::dig_roundup_limit();

            // Get next digit
            num *= 10.0;
            digit = (int)(num + digit_roundup);
            if (digit > 9)
                digit = 9;

            // Precision check
            if (exp <= 0) {
                if (precision <= 0)
                    break;
                --precision;
            }

            // Remove next digit
            num -= digit;

            // Add digit
            if (++count > FloatT<T>::MAXDIGITS)
                *ptr = '0';    // Exceeded max significant digits, use '0'
            else
                *ptr = '0' + (char)digit;
            ++ptr;

            // Decimal point
            if (--exp == 0) {
                if (precision <= 0)
                    break; // Done, ignore fraction
                *ptr = '.'; ++ptr;
            }
        }

        // Whole number trailing zeroes
        if (exp > 0) {
            do {
                *ptr = '0'; ++ptr;
            } while (--exp > 0);
            if (precision > 0)
                { *ptr = '.'; ++ptr; }
        }

        // Fraction trailing zeroes
        while (precision > 0) {
            *ptr = '0'; ++ptr;
            --precision;
        }

        // Done
        return (ulong)(ptr - startptr);
    }

    // Format normalized floating point number as string using normal or e notation
    template<class T>
    static ulong fnumfe(char* ptr, T num, int exp, bool cap) {
        const T PRECISION = FloatT<T>::precision();
        const char* startptr = ptr;

        // NaN
        if (FloatT<T>::nan(num)) {
            ptr[0] = 'n';
            ptr[1] = 'a';
            ptr[2] = 'n';
            return 3;
        }

        // Zero
        if (num == 0.0) {
            *ptr = '0';
            return 1;
        }

        // Negative
        if (num < 0.0) {
            *ptr = '-'; ++ptr;
            num = -num;
        }

        // inf or -inf
        if (FloatT<T>::inf(num)) {
            ptr[0] = 'i';
            ptr[1] = 'n';
            ptr[2] = 'f';
            ptr += 3;
            return (ulong)(ptr - startptr);
        }

        // Adjust for exponent notation
        const int E_THRESHOLD = (FloatT<T>::MAXDIGITS > 6 ? 6 : FloatT<T>::MAXDIGITS);
        bool on_fraction = false;
        bool show_e      = false;
        if (exp < -2 || exp > E_THRESHOLD) {
            show_e = true;
            --exp;
        } else {
            // Fraction leading zeroes
            if (exp <= 0) {
                on_fraction = true;
                ptr[0] = '0';
                ptr[1] = '.';
                ptr += 2;
                while (exp < 0)
                    { *ptr = '0';  ++ptr;  ++exp; }
            }
        }

        // This prevents some cases where formatting comes out rounded down
        num += FloatT<T>::eps();

        // Significant digits
        T digit_roundup = FloatNumF<T>::dig_roundup(); // Used to round out float noise on each digit
        int digit = 0, count = 0, zero_count = 0, digit_roundup_counter = FloatT<T>::MAXDIGITS - 1;
        while (num > PRECISION) {
            // Round out float noise more aggressively when reaching precision limit
            if (--digit_roundup_counter == 0)
                digit_roundup = FloatNumF<T>::dig_roundup_limit();

            // Get next digit
            num *= 10.0;
            digit = (int)(num + digit_roundup);
            if (digit > 9)
                digit = 9;

            // Remove next digit
            num -= (T)digit;

            // Add digit
            if (++count > FloatT<T>::MAXDIGITS)
                break;
            *ptr = '0' + (char)digit;
            ++ptr;

            // Decimal point
            if (show_e) {
                // E notation has decimal point after 1st digit
                if (count == 1) {
                    on_fraction = true;
                    *ptr = '.';
                    ++ptr;
                }
            } else if (--exp == 0) {
                // Normal notation decimal
                on_fraction = true;
                *ptr = '.';
                ++ptr;
            }

            // Track ending zeros for trimming
            if (on_fraction) {
                if (digit == 0)
                    ++zero_count;
                else if (zero_count > 0)
                    zero_count = 0;
            }
        }

        // Remove unnecessary ending zeros
        if (zero_count > 0)
            ptr -= zero_count;
        if (ptr[-1] == '.')
            --ptr;

        // Additional values
        if (show_e) {
            // Exponent
            if (exp != 0) {
                *ptr = (cap?'E':'e');  ++ptr;
                if (exp < 0)
                    { *ptr = '-';  ++ptr;  exp = -exp; }
                else
                    { *ptr = '+';  ++ptr; }
                if (exp >= 100) {
                    ptr[0] = '0' + ((exp / 100) % 10);
                    ptr[1] = '0' + ((exp / 10) % 10);
                    ptr[2] = '0' + (exp % 10);
                    ptr += 3;
                } else if (exp >= 10) {
                    ptr[0] = '0' + ((exp / 10) % 10);
                    ptr[1] = '0' + (exp % 10);
                    ptr += 2;
                } else
                    { *ptr = '0' + (char)exp;  ++ptr; }
            }
        } else {
            // Whole number trailing zeroes
            if (exp > 0) {
                do {
                    *ptr = '0'; ++ptr;
                } while (--exp > 0);
            }
        }

        // Done
        return (ulong)(ptr - startptr);
    }

#if defined(EVO_FNUMF_SPRINTF)
    static const uint FNUMF_SPRINTF_BUF_SIZE = UInt::MAXSTRLEN + 6;

    static ulong fnumf_sprintf_setup(char*& fmt, char* buf, double num, int precision) {
        if (precision < 0) {
            fmt = buf;
            fmt[0] = '%';
            fmt[1] = 'g';
            fmt[2] = '\0';
        } else {
            fmt = buf + FNUMF_SPRINTF_BUF_SIZE - 1;
            *--fmt = '\0';
            *--fmt = 'f';
            fmt -= fnumu(fmt, (uint)(precision < 0 ? 0 : precision), 10);
            *--fmt = '.';
            *--fmt = '%';
        }
        int result = ::snprintf(NULL, 0, fmt, num);
        return (ulong)(result < 0 ? 0 : result);
    }

    static ulong fnumf_sprintf_setup(char*& fmt, char* buf, float num, int precision) {
        return fnumf_sprintf_setup(fmt, buf, (double)num, precision);
    }

    static ulong fnumf_sprintf_setup(char*& fmt, char* buf, ldouble num, int precision) {
        if (precision < 0) {
            fmt = buf;
            fmt[0] = '%';
            fmt[1] = 'L';
            fmt[2] = 'g';
            fmt[3] = '\0';
        } else {
            fmt = buf + FNUMF_SPRINTF_BUF_SIZE - 1;
            *--fmt = '\0';
            *--fmt = 'f';
            *--fmt = 'L';
            fmt -= fnumu(fmt, (uint)(precision < 0 ? 0 : precision), 10);
            *--fmt = '.';
            *--fmt = '%';
        }
        int result = ::snprintf(NULL, 0, fmt, num);
        return (ulong)(result < 0 ? 0 : result);
    }
#endif

    // Conversion helpers
    template<class T> struct ToBool {
        template<class Size> static T getbool(const char* data, Size size) {
            STATIC_ASSERT( IsBool<typename T::Type>::value, ToBool_Bool_Type_Required );
            T result;
            if (size > 0) {
                Error error;
                typename T::Type value = impl::tobool(data, size, error);
                if (error == ENone)
                    result = value;
            }
            return result;
        }
    };
    template<class T>struct ToBoolPod {
        template<class Size> static T getbool(const char* data, Size size) {
            STATIC_ASSERT( IsBool<T>::value && IsPodType<T>::value, ToBool_POD_Type_Required );
            Error error;
            T result = impl::tobool(data, size, error);
            if (error != ENone)
                result = 0;
            return result;
        }
    };
    template<class T> struct ToNum {
        template<class Size> static T getnum(const char* data, Size size, int base) {
            STATIC_ASSERT( IsInt<typename T::Type>::value, ToNum_IntegerT_Type_Required );
            T result;
            if (size > 0) {
                Error error;
                typename T::Type value = impl::tonum<typename T::Type>(data, size, error, base);
                if (error == ENone)
                    result = value;
            }
            return result;
        }
    };
    template<class T>struct ToNumPod {
        template<class Size> static T getnum(const char* data, Size size, int base) {
            STATIC_ASSERT( IsInt<T>::value && IsPodType<T>::value, ToNum_POD_Type_Required );
            Error error;
            T result = impl::tonum<T>(data, size, error, base);
            if (error != ENone)
                result = 0;
            return result;
        }
    };
    template<class T> struct ToNumf {
        template<class Size> static T getnum(const char* data, Size size) {
            STATIC_ASSERT( IsFloat<typename T::Type>::value, ToNumf_FloatT_Type_Required );
            T result;
            if (size > 0) {
                Error error;
                typename T::Type value = impl::tonumf<typename T::Type>(data, size, error);
                if (error == ENone)
                    result = value;
            }
            return result;
        }
    };
    template<class T> struct ToNumfPod {
        template<class Size> static T getnum(const char* data, Size size) {
            STATIC_ASSERT( IsFloat<T>::value, ToNumf_POD_Type_Required );
            Error error;
            T result = impl::tonumf<T>(data, size, error);
            if (error != ENone)
                result = 0;
            return result;
        }
    };
}
/** \endcond */

///////////////////////////////////////////////////////////////////////////////

/** Formatting integer base.
 - Used with FmtSetInt and operator<<() on String::Format, Stream, or StreamOut
*/
enum FmtBase {
    fbCURRENT = 0,      ///< Current base (i.e. unspecified/default)
    fbAUTO = 0,         ///< Auto base detection
    fBIN  = 2,          ///< Base 2: binary
    fOCT  = 8,          ///< Base 8: octal
    fDEC  = 10,         ///< Base 10: decimal (default)
    fHEX  = 16,         ///< Base 16: hexadecimal
    fHEXL = 116         ///< Base 16: hexadecimal (lowercase)
};

/** Formatting integer base prefix type.
 - Used with FmtSetInt and operator<<() on String::Format, Stream, or StreamOut
*/
enum FmtBasePrefix {
    fbpCURRENT = 0,     ///< Current prefix (i.e. unspecified/default)
    fPREFIX0,           ///< No base prefix (default)
    fPREFIX1,           ///< Single character base prefix (x for hex, o for octal, b for binary)
    fPREFIX2            ///< Double character base prefix (0x for hex, 0o for octal, 0b for binary)
};

/** Formatting floating point precision type.
 - Used with FmtSetFloat and operator<<() on String::Format, Stream, or StreamOut
*/
enum FmtPrecision {
    fpCURRENT  = -2,    ///< Current floating point precision (i.e. unspecified/default)
    fPREC_AUTO = -1,    ///< Automatic floating point precision -- either normal decimal notation or E notation, whichever is shorter (default)
    fPREC0 = 0,         ///< No floating point precision (whole numbers)
    fPREC1,             ///< 1-digit floating point precision
    fPREC2,             ///< 2-digit floating point precision
    fPREC3,             ///< 3-digit floating point precision
    fPREC4,             ///< 4-digit floating point precision
    fPREC5,             ///< 5-digit floating point precision
    fPREC6              ///< 6-digit floating point precision
};

/** Formatting alignment.
 - Used with FmtSetField and operator<<() on String::Format, Stream, or StreamOut
*/
enum FmtAlign {
    faCURRENT = 0,      ///< Current alignment (i.e. unspecified/default)
    fLEFT,              ///< Align left by adding filler on right side
    fCENTER,            ///< Align center by adding filler on left and right sides
    fRIGHT              ///< Align right by adding filler on left side
};

/** Formatting field width.
 - Used with operator<<() on String::Format or Stream::Format
   - See \ref StringFormatting "String Formatting" and \ref StreamFormatting "Stream Formatting"
*/
enum FmtWidth {
    fWIDTH0 = 0
};

///////////////////////////////////////////////////////////////////////////////

/** Holds string to use when formatting null values.
 - This stores a reference to string pointer, which must remain valid -- best to use a literal
 - Used with operator<<() on String::Format or Stream::Format
   - See \ref StringFormatting "String Formatting" and \ref StreamFormatting "Stream Formatting"

\par Example

\code
#include <evo/io.h>
#include <evo/string.h>
using namespace evo;
static Console& c = con();

int main() {
    String null_str;
    Int null_int;

    // Format to stream
    Stream::Format(c.out) << FmtSetNull("(null)") << null_str << ',' << null_int << NL;

    // Format same output to a string
    String str;
    String::Format(str) << FmtSetNull("(null)") << null_str << ',' << null_int << NL;

    return 0;
}
\endcode

Output:
\code{.unparsed}
(null),(null)
\endcode
*/
struct FmtSetNull {
    const char* str;    ///< Pointer to string for formatting null values, ignored if size is 0
    StrSizeT    size;   ///< %String (str) size for formatting null values, 0 for none/empty

    /** Constructor. */
    FmtSetNull() : str(NULL), size(0)
        { }

    /** Constructor.
     \param  null  Null string to set, must be terminated
    */
    FmtSetNull(const char* null) : str(null), size((StrSizeT)strlen(null))
        { }

    /** Constructor.
     \param  null  Null string to set
     \param  size  Null string size in bytes
    */
    FmtSetNull(const char* null, StrSizeT size) : str(null), size(size)
        { }

    /** Constructor.
     \param  null  Null string to set
    */
    FmtSetNull(const ListBase<char,StrSizeT>& null) : str(null.data_), size(null.size_)
        { }

    /** Reset to defaults.
     \return  This
    */
    FmtSetNull& reset() {
        str  = NULL;
        size = 0;
        return *this;
    }
};

/** Holds field and alignment formatting attributes.
 - This is useful for formatting rows of text aligned by columns
   - Fields that are smaller than the field width will be filled with padding up to the width
   - Only numeric values (int, float, etc) and strings count as fields
 - Used with operator<<() on String::Format or Stream::Format
   - See \ref StringFormatting "String Formatting" and \ref StreamFormatting "Stream Formatting"
 - Note: Only numeric valus (int, float, etc) and strings count as fields, this doesn't apply to others

\par Example

\code
#include <evo/io.h>
#include <evo/string.h>
using namespace evo;
static Console& c = con();

int main() {
    // Format to stream
    Stream::Format(c.out) << FmtSetField(fLEFT, 6);
        << "Col A" << "Col B" << NL;
        << 1 << 2 << NL;

    // Format same output to a string
    String str;
    String::Format(str) << FmtSetField(fLEFT, 6)
        << "Col A" << "Col B" << NL;
        << 1 << 2 << NL;

    return 0;
}
\endcode

Output:
\code{.unparsed}
Col A Col B 
1     2     
\endcode
*/
struct FmtSetField {
    FmtAlign align;     ///< Field alignment type (default: fLEFT)
    int      width;     ///< Field width to align in (default: 0)
    char     fill;      ///< Field fill character to pad up to width (default: ' ')

    /** Main constructor.
     \param  align  Alignment type to use: fLEFT, fRIGHT, or fCENTER (faCURRENT for unspecified/default)
     \param  width  Field character width, (-1 for unspecified/default)
     \param  fill   Field fill character to pad up to width (0 for unspecified/default)
    */
    FmtSetField(FmtAlign align=faCURRENT, int width=-1, char fill=0) : align(align), width(width), fill(fill)
        { }

    /** Short constructor with default alignment.
     \param  width  Field character width, (-1 for unspecified/default)
     \param  fill   Field fill character to pad up to width (0 for unspecified/default)
    */
    FmtSetField(int width, char fill=0) : align(faCURRENT), width(width), fill(fill)
        { }

    /** Reset attributes to defaults (not unspecified). */
    FmtSetField& reset() {
        align = fLEFT;
        width = 0;
        fill  = ' ';
        return *this;
    }

    /** Merge source attributes (used internally).
     - This copies attributes from src that aren't "unspecified"
     .
     \param  src  Source to merge from
    */
    void merge(const FmtSetField& src) {
        if (src.align > faCURRENT)
            align = src.align;
        if (src.width >= 0)
            width = src.width;
        if (src.fill != 0)
            fill = src.fill;
    }

    /** Used to setup and calculate alignment padding (used internally).
     \param  align_padleft   %Set to left-padding size  [out]
     \param  align_padright  %Set to right-padding size  [out]
     \param  align_padding   Padding width to use
     \param  field           Field attribuets to use for alignment type (field->align), NULL for no alignment
    */
    static void setup_align(int& align_padleft, int& align_padright, int align_padding, const FmtSetField* field) {
        if (align_padding > 0) {
            switch (field->align) {
                default:        // fallthrough
                case faCURRENT: // fallthrough
                case fLEFT:
                    align_padleft  = 0;
                    align_padright = align_padding;
                    break;
                case fCENTER:
                    align_padleft  = (align_padding / 2);
                    align_padright = align_padding - align_padleft;
                    break;
                case fRIGHT:
                    align_padleft  = align_padding;
                    align_padright = 0;
                    break;
            }
        } else
            align_padleft = align_padright = 0;
    }
};

/** Holds integer formatting attributes.
 - Used with operator<<() on String, Stream, and StreamOut
   - Each integer counts as a "field" when formatted to output object with field alignment state (like String::Format or Stream::Format)
   - See \ref StringFormatting "String Formatting" and \ref StreamFormatting "Stream Formatting"
 - This uses "unspecified" defaults, which means default attributes depend on the output object used, which uses configured formatting state or real defaults if no formatting state kept

\par Example

\code
#include <evo/io.h>
#include <evo/string.h>
using namespace evo;
static Console& c = con();

int main() {
    // Format to stream
    Stream::Format(c.out) << 123 << ',' << FmtSetInt(fHEX, fPREFIX2, 4) << 10 << ',' << 20 << NL
        << FmtSetInt(fDEC, 5, '_')  << 123 << NL
        << fOCT << fPREFIX0 << 5 << ',' << 456 << NL;

    // Format same output to a string
    String str;
    String::Format(str) << 123 << ',' << FmtSetInt(fHEX, fPREFIX2, 4) << 10 << NL
        << FmtSetInt(fDEC, 5, '_')  << 123 << NL
        << fOCT << fPREFIX0 << 5 << ',' << 456 << NL;

    return 0;
}
\endcode

Output:
\code{.unparsed}
123,0x000A,0x0014
__123
5,710
\endcode
*/
struct FmtSetInt {
    int           base;         ///< Base for formatting (default: fDEC)
    FmtBasePrefix prefix;       ///< Formatting prefix type (default: fPREFIX0)
    int           pad_width;    ///< Width to fill to, 0 for none, -1 if unspecified (use baseline or default) (default: 0)
    char          pad_ch;       ///< Fill character, 0 if unspecified (use baseline or default) (default: '0')

    /** Main constructor with all attributes.
     \param  base    Formatting base to use (see FmtBase), fbCURRENT for unspecified (default: fDEC)
     \param  prefix  Formatting prefix to use (see FmtBasePrefix), fbpCURRENT for unspecified (default: fPREFIX0)
     \param  width   Width to pad to, -1 for unspecified (default: 0)
     \param  ch      Padding character to use, 0 for unspecified (default: '0')
    */
    FmtSetInt(int base=fbCURRENT, FmtBasePrefix prefix=fbpCURRENT, int width=-1, char ch=0) : base(base), prefix(prefix), pad_width(width), pad_ch(ch)
        { }

    /** Short constructor without prefix.
     \param  base    Formatting base to use (see FmtBase), fbCURRENT for unspecified (default: fDEC)
     \param  width   Width to pad to, -1 for unspecified (default: 0)
     \param  ch      Padding character to use, 0 for unspecified (default: '0')
    */
    FmtSetInt(int base, int width, char ch=0) : base(base), prefix(fbpCURRENT), pad_width(width), pad_ch(ch)
        { }

    /** Reset attributes to defaults (not unspecified). */
    FmtSetInt& reset() {
        base      = fDEC;
        prefix    = fPREFIX0;
        pad_width = 0;
        pad_ch    = '0';
        return *this;
    }

    /** Merge source attributes (used internally).
     - This copies attributes from src that aren't "unspecified"
     .
     \param  src  Source to merge from
    */
    void merge(const FmtSetInt& src) {
        if (src.base > fbCURRENT)
            base = src.base;
        if (src.prefix > fbpCURRENT)
            prefix = src.prefix;
        if (src.pad_width >= 0)
            pad_width = src.pad_width;
        if (src.pad_ch != 0)
            pad_ch = src.pad_ch;
    }

    // Internal helpers
    /** \cond impl */
    void impl_prefix_info(char& prefix_ch, uint& prefix_len) const {
        switch (prefix) {
            case fPREFIX1:
                switch (base) {
                    case fHEXL: // fallthrough
                    case fHEX:
                        prefix_len = 1;
                        prefix_ch  = 'x';
                        break;
                    case fOCT:
                        prefix_len = 1;
                        prefix_ch  = 'o';
                        break;
                    case fBIN:
                        prefix_len = 1;
                        prefix_ch  = 'b';
                        break;
                    default:
                        break;
                }
                break;
            case fPREFIX2:
                switch (base) {
                    case fHEXL: // fallthrough
                    case fHEX:
                        prefix_len = 2;
                        prefix_ch  = 'x';
                        break;
                    case fOCT:
                        prefix_len = 2;
                        prefix_ch  = 'o';
                        break;
                    case fBIN:
                        prefix_len = 2;
                        prefix_ch  = 'b';
                        break;
                    default:
                        break;
                }
                break;
            default:
                break;
        }
    }
    static void impl_prefix_write(char*& p, char prefix_ch, uint prefix_len) {
        switch (prefix_len) {
            case 1:
                *p = prefix_ch;
                ++p;
                break;
            case 2:
                *p = '0';
                p[1] = prefix_ch;
                p += 2;
                break;
            default:
                break;
        }
    }
    template<class T>
    void impl_num_write(char* p, T num, int digits, int width, int align_padding, const FmtSetField* field) const {
        int align_padleft, align_padright;
        FmtSetField::setup_align(align_padleft, align_padright, align_padding, field);

        if (align_padleft > 0) {
            memset(p, (int)(uchar)field->fill, align_padleft);
            p += align_padleft;
        }

        if (num < 0) {
            if (digits < width) {
                *p = '-';
                ++p;

                char* p0 = p;
                const uint padlen = width - digits;
                p += padlen + digits - 1;
                impl::fnum(p, num, fDEC);

                const int ch = (pad_ch == 0 ? '0' : (int)(uchar)pad_ch);
                memset(p0, ch, padlen);
            } else {
                p += digits;
                impl::fnum(p, num, fDEC);
            }
        } else {
            if (digits < width) {
                const uint padlen = width - digits;
                const int ch = (pad_ch == 0 ? '0' : (int)(uchar)pad_ch);
                memset(p, ch, padlen);
                p += padlen;
            }
            p += digits;
            impl::fnum(p, num, fDEC);
        }
                
        if (align_padright > 0)
            memset(p, (int)(uchar)field->fill, align_padright);
    }
    /** \endcond */
};

/** Holds floating point formatting attributes.
 - Used with operator<<() on String, Stream, and StreamOut
   - Each floating point number counts as a "field" when formatted to output object with field alignment state (like String::Format or Stream::Format)
   - See \ref StringFormatting "String Formatting" and \ref StreamFormatting "Stream Formatting"
 - This uses "unspecified" defaults, which means default attributes depend on the output object used, which uses configured formatting state or real defaults if no formatting state kept

\par Example

\code
#include <evo/io.h>
#include <evo/string.h>
using namespace evo;
static Console& c = con();

int main() {
    // Format to stream
    Stream::Format(c.out) << 12.3 << ',' << FmtSetFloat(fPREC2, 6) << 10 << ',' << 20 << NL
        << fPREC3 << 1.23 << NL;

    // Format same output to a string
    String str;
    String::Format(str) << 12.3 << ',' << FmtSetFloat(fPREC2, 6) << 10 << ',' << 20 << NL
        << fPREC3 << 1.23 << NL;

    return 0;
}
\endcode

Output:
\code{.unparsed}
12.3,010.00,020.00
12.300
\endcode
*/
struct FmtSetFloat {
    int            precision;       ///< Floating point precision (number of digits after decimal), fPREC_AUTO for automatic (default: fPREC_AUTO)
    int            pad_width;       ///< Width to fill to, 0 for none, -1 to ignore (leave current width) (default: 0)
    char           pad_ch;          ///< Padding character, 0 to ignore (leave current fill character) (default: '0')
    char           pad_ch_sp;       ///< Padding character used with special value (inf or nan), 0 to ignore (leave current fill character) (default: ' ')

    /** Constructor with all attributes.
     \param  precision  Precision to use (number of digits after decimal), fPREC_AUTO for automatic (default: fPREC_AUTO)
     \param  width      Width to pad to, -1 for unspecified (default: 0)
     \param  ch         Padding character to use, 0 for unspecified (default: '0')
     \param  ch_sp      Special padding character to use when formatting special values like "inf" or "nan" (default: ' ')
    */
    FmtSetFloat(int precision=fpCURRENT, int width=-1, char ch=0, char ch_sp=0) : precision(precision), pad_width(width), pad_ch(ch), pad_ch_sp(ch_sp)
        { }

    /** Reset to defaults.
     \return  This
    */
    FmtSetFloat& reset() {
        precision = fPREC_AUTO;
        pad_width = 0;
        pad_ch    = '0';
        pad_ch_sp = ' ';
        return *this;
    }

    /** Merge from source (used internally).
     - This copies explicit values from src, but ignores "leave current" values
     .
     \param  src  Source to copy
    */
    void merge(const FmtSetFloat& src) {
        if (src.precision > fpCURRENT)
            precision = src.precision;
        if (src.pad_width >= 0)
            pad_width = src.pad_width;
        if (src.pad_ch != 0)
            pad_ch = src.pad_ch;
        if (src.pad_ch_sp != 0)
            pad_ch_sp = src.pad_ch_sp;
    }

    // Internal helpers
    /** \cond impl */
    template<class T>
    void impl_info(T& num, int& exp, int& maxlen, int align_width) const {
        if (precision < 0) {
            num    = FloatT<T>::fexp10(exp, num);
            maxlen = FloatT<T>::MAXDIGITS_AUTO + 1;                     // add 1 for sign
        } else {
            num    = FloatT<T>::fexp10(exp, impl::fnumf_weight(num, precision));
            maxlen = FloatT<T>::maxdigits_prec(exp, precision) + 1;     // add 1 for sign
        }
        if (pad_width > maxlen)
            maxlen = pad_width;
        if (align_width > maxlen)
            maxlen = align_width;
    }
    template<class T>
    ulong impl_write(char* buf, T num, int exp, int align_width, const FmtSetField* field) const {
        char* p = buf;
        long len;
        if (precision < 0)
            len = (long)impl::fnumfe(p, num, exp, false);
        else
            len = (long)impl::fnumf(p, num, exp, precision);

        const int width         = (pad_width > len ? pad_width : len);
        const int align_padding = (align_width > width ? align_width - width : 0);

        int align_padleft, align_padright;
        FmtSetField::setup_align(align_padleft, align_padright, align_padding, field);

        char* p1 = p + align_padleft;
        if (len < pad_width) {
            const int padlen = pad_width - len;
            if (FloatT<T>::nan(num) || FloatT<T>::inf(num)) {
                memmove(p1 + padlen, p, len);
                memset(p1, (int)(uchar)pad_ch_sp, padlen);
            } else if (num < 0.0) {
                memmove(p1 + 1 + padlen, p+1, len-1);
                memset(p1 + 1, (int)(uchar)pad_ch, padlen);
                *p1 = '-';
            } else {
                memmove(p1 + padlen, p, len);
                memset(p1, (int)(uchar)pad_ch, padlen);
            }
            if (align_padleft > 0)
                memset(p, (int)(uchar)field->fill, align_padleft);
            p = p1 + len + padlen;
        } else if (align_padleft > 0) {
            memmove(p1, p, len);
            memset(p, (int)(uchar)field->fill, align_padleft);
            p = p1 + len;
        } else
            p += len;

        if (align_padright > 0) {
            memset(p, (int)(uchar)field->fill, align_padright);
            p += align_padright;
        }

        return (ulong)(p - buf);
    }
    /** \endcond */
};

/** Formatting attributes (used internally).
 - Used by String::Format and Stream::Format
 - See \ref StringFormatting "String Formatting" and \ref StreamFormatting "Stream Formatting"
*/
struct FmtAttribs {
    FmtSetNull  null;       ///< String to use for null values from strings (like String) or primitives (like Int)
    FmtSetInt   num_int;    ///< Integer attributes
    FmtSetFloat num_flt;    ///< Floating point attributes
    FmtSetField field;      ///< Field alignment attributes

    /** Constructor. */
    FmtAttribs() : num_int(fbAUTO, fPREFIX0, 0, '0'), num_flt(fPREC_AUTO, 0, '0', ' '), field(fLEFT, 0, ' ')
        { }

    /** Reset to defaults.
     \return  This
    */
    FmtAttribs& reset() {
        null.reset();
        num_int.reset();
        num_flt.reset();
        field.reset();
        return *this;
    }
};

///////////////////////////////////////////////////////////////////////////////

/** Explicitly format a repeated character.
 - Used with operator<<() on String, Stream, and StreamOut
   - See \ref StringFormatting "String Formatting" and \ref StreamFormatting "Stream Formatting"
*/
struct FmtChar {
    char        ch;     ///< Character to format
    uint        count;  ///< Character repeat count

    /** Constructor.
     \param  ch     Character to format
     \param  count  Character repeat count
    */
    FmtChar(char ch, ulong count) : ch(ch), count(count)
        { }
};

/** Explicitly format a string.
 - Used with operator<<() on String, Stream, and StreamOut
   - See \ref StringFormatting "String Formatting" and \ref StreamFormatting "Stream Formatting"
 
*/
struct FmtString {
    typedef FmtString               This;           ///< %This type
    typedef ListBase<char,StrSizeT> StringBase;     ///< StringBase type
    typedef FmtString               FmtFieldType;   ///< This type paired with field info

    StringBase  str;
    FmtSetField fmt;

    /** Constructor.
     \param  str    %String pointer to format, must be terminated
     \param  align  Alignment type for string -- see FmtAlign
    */
    FmtString(const char* str, FmtAlign align=fLEFT) : str(str), fmt(align)
        { }

    /** Constructor.
     \param  str    %String pointer to format
     \param  size   %String size to format
     \param  width  Width to format within, -1 for default
     \param  ch     Padding character to fill in `width` around string, 0 for default
    */
    FmtString(const char* str, StrSizeT size, int width=-1, char ch=0) : str(str, size), fmt(width, ch)
        { }

    /** Constructor.
     \param  str    %String pointer to format
     \param  size   %String size to format
     \param  align  Alignment type for string -- see FmtAlign
     \param  width  Width to format within, -1 for default
     \param  ch     Padding character to fill in `width` around string, 0 for default
    */
    FmtString(const char* str, StrSizeT size, FmtAlign align, int width=-1, char ch=0) : str(str, size), fmt(align, width, ch)
        { }

    /** Constructor.
     \param  str    %String to format
     \param  width  Width to format within, -1 for default
     \param  ch     Padding character to fill in `width` around string, 0 for default
    */
    FmtString(const StringBase& str, int width=-1, char ch=0) : str(str), fmt(width, ch)
        { }

    /** Constructor.
     \param  str    %String to format
     \param  align  Alignment type for string -- see FmtAlign
     \param  width  Width to format within, -1 for default
     \param  ch     Padding character to fill in `width` around string, 0 for default
    */
    FmtString(const StringBase& str, FmtAlign align, int width=-1, char ch=0) : str(str), fmt(align, width, ch)
        { }

    /** Constructor with override fields for compatibility with FmtFieldType.
     \param  str    %String to format
     \param  align  Alignment type for string -- see FmtAlign
     \param  width  Width to format within, -1 for default
     \param  ch     Padding character to fill in `width` around string, 0 for default
    */
    FmtString(const FmtString& str, FmtAlign align, int width, char ch=0) : str(str.str), fmt(align, width, ch)
        { }

    /** Helper for setting padding attributes.
     \param  width  Width to format within, -1 for default
     \param  ch     Padding character to fill in `width` around string, 0 for default
     \return        This
    */
    This& width(int width, char ch=0) {
        fmt.width = width;
        fmt.fill  = ch;
        return *this;
    }
};

struct FmtStringWrap {
    typedef ListBase<char,StrSizeT> StringBase;

    StringBase str;
    int width;
    int indent;
    NewlineValue newline;

    FmtStringWrap(const char* str, StrSizeT size, int width, int indent=0) : str(str, size), width(width), indent(indent)
        { }

    FmtStringWrap(const StringBase& str, int width) : str(str), width(width), indent(0)
        { }

    FmtStringWrap& set_indent(int new_indent=0) {
        indent = new_indent;
        return *this;
    }

    FmtStringWrap& set_newline(Newline nl) {
        newline = nl;
        return *this;
    }

    FmtStringWrap& set_newline(NewlineDefault nl) {
        newline = nl;
        return *this;
    }
};

// Implemented below
template<class T> struct FmtFieldNum;
template<class T> struct FmtFieldFloat;

/** Explicitly format an integer.
 - Used with operator<<() on String, Stream, and StreamOut
   - See \ref StringFormatting "String Formatting" and \ref StreamFormatting "Stream Formatting"
 - Don't use FmtIntT directly, use one of the aliases:
   - \ref FmtShort, \ref FmtInt, \ref FmtLong, \ref FmtLongL
   - \ref FmtInt8, \ref FmtInt16, \ref FmtInt32, \ref FmtInt64
   - \ref FmtUShort, \ref FmtUInt, \ref FmtULong, \ref FmtULongL
   - \ref FmtUInt8, \ref FmtUInt16, \ref FmtUInt32, \ref FmtUInt64
 .

\par Example

Example using \ref FmtInt

\code
#include <evo/string.h>
using namespace evo;

int main() {
    // Format string as: 07B,45,67
    String str;
    str << FmtInt(123, fHEX, 3) << ',' FmtInt(45, fDEC) << ',' 67;

    return 0;
}
\endcode
*/
template<class T>
struct FmtIntT {
    typedef FmtIntT<T>                 This;            ///< %This type
    typedef typename IntegerT<T>::This IntClass;        ///< Number class type
    typedef T                          IntPod;          ///< Number POD type
    typedef FmtFieldNum<T>             FmtFieldType;    ///< This type paired with field info

    T         num;          ///< Number to format
    FmtSetInt fmt;          ///< Formatting attributes

    /** Constructor for POD number type with all attributes.
     \param  num     Number to format
     \param  base    Formatting base to use (see FmtBase), fbCURRENT for unspecified (default: fDEC)
     \param  prefix  Formatting prefix to use (see FmtBasePrefix), fbpCURRENT for unspecified (default: fPREFIX0)
     \param  width   Width to pad to, -1 for unspecified (default: 0)
     \param  ch      Padding character to use, 0 for unspecified (default: '0')
    */
    FmtIntT(T num, int base=fbCURRENT, FmtBasePrefix prefix=fbpCURRENT, int width=-1, char ch=0) : num(num), fmt(base, prefix, width, ch)
        { }

    /** Constructor for class number type (Int, etc) with all attributes.
     \param  num     Number to format
     \param  base    Formatting base to use (see FmtBase), fbCURRENT for unspecified (default: fDEC)
     \param  prefix  Formatting prefix to use (see FmtBasePrefix), fbpCURRENT for unspecified (default: fPREFIX0)
     \param  width   Width to pad to, -1 for unspecified (default: 0)
     \param  ch      Padding character to use, 0 for unspecified (default: '0')
    */
    FmtIntT(const IntClass& num, int base=fbCURRENT, FmtBasePrefix prefix=fbpCURRENT, int width=-1, char ch=0) : num(num.value()), fmt(base, prefix, width, ch)
        { }

    /** Helper for setting padding attributes.
     \param  width   Width to pad to, -1 for unspecified (default: 0)
     \param  ch      Padding character to use, 0 for unspecified (default: '0')
     \return         This
    */
    This& width(int width, char ch=0) {
        fmt.pad_width = width;
        fmt.pad_ch    = ch;
        return *this;
    }
};

typedef FmtIntT<short>  FmtShort;   ///< \copydoc FmtIntT
typedef FmtIntT<int>    FmtInt;     ///< \copydoc FmtIntT
typedef FmtIntT<long>   FmtLong;    ///< \copydoc FmtIntT
typedef FmtIntT<longl>  FmtLongL;   ///< \copydoc FmtIntT
typedef FmtIntT<int8>   FmtInt8;    ///< \copydoc FmtIntT
typedef FmtIntT<int16>  FmtInt16;   ///< \copydoc FmtIntT
typedef FmtIntT<int32>  FmtInt32;   ///< \copydoc FmtIntT
typedef FmtIntT<int64>  FmtInt64;   ///< \copydoc FmtIntT

typedef FmtIntT<ushort> FmtUShort;  ///< \copydoc FmtIntT
typedef FmtIntT<uint>   FmtUInt;    ///< \copydoc FmtIntT
typedef FmtIntT<ulong>  FmtULong;   ///< \copydoc FmtIntT
typedef FmtIntT<ulongl> FmtULongL;  ///< \copydoc FmtIntT
typedef FmtIntT<uint8>  FmtUInt8;   ///< \copydoc FmtIntT
typedef FmtIntT<uint16> FmtUInt16;  ///< \copydoc FmtIntT
typedef FmtIntT<uint32> FmtUInt32;  ///< \copydoc FmtIntT
typedef FmtIntT<uint64> FmtUInt64;  ///< \copydoc FmtIntT

/** Explicitly format a floating pointer number.
 - Used with operator<<() on String, Stream, and StreamOut
   - See \ref StringFormatting "String Formatting" and \ref StreamFormatting "Stream Formatting"
 - Don't use FmtFloatT directly, use one of the aliases:
   - \ref FmtFloat, \ref FmtFloatD, \ref FmtFloatL

\par Example

Example using \ref FmtFloat

\code
#include <evo/string.h>
using namespace evo;

int main() {
    // Format string as: 01.230,4.56
    String str;
    str << FmtFloat(1.23, 3, 6) << ',' << 4.56;

    return 0;
}
\endcode
*/
template<class T>
struct FmtFloatT {
    typedef FmtFloatT<T>             This;          ///< This type
    typedef typename FloatT<T>::This FloatClass;    ///< Number class type
    typedef T                        FloatPod;      ///< Number POD type
    typedef FmtFieldFloat<T>         FmtFieldType;  ///< This type paired with field info

    bool        null;
    T           num;
    FmtSetFloat fmt;

    /** Constructor for POD number type with all attributes.
     \param  num        Number to format
     \param  precision  Precision to use (number of digits after decimal), fPREC_AUTO for automatic (default: fPREC_AUTO)
     \param  width      Width to pad to, -1 for unspecified (default: 0)
     \param  ch         Padding character to use, 0 for unspecified (default: '0')
     \param  ch_sp      Padding character to use with special value (inf or nan), 0 for unspecified (default: ' ')
    */
    FmtFloatT(T num, int precision=fpCURRENT, int width=-1, char ch='0', char ch_sp=' ') : null(false), num(num), fmt(precision, width, ch, ch_sp)
        { }

    /** Constructor for class number type (Float, etc) with all attributes.
     \param  num        Number to format
     \param  precision  Precision to use (number of digits after decimal), fPREC_AUTO for automatic (default: fPREC_AUTO)
     \param  width      Width to pad to, -1 for unspecified (default: 0)
     \param  ch         Padding character to use, 0 for unspecified (default: '0')
     \param  ch_sp      Padding character to use with special value (inf or nan), 0 for unspecified (default: ' ')
    */
    FmtFloatT(const FloatClass& num, int precision=fpCURRENT, int width=-1, char ch='0', char ch_sp=' ') : null(num.null()), num(num.value()), fmt(precision, width, ch, ch_sp)
        { }

    /** Helper for setting padding attributes.
     \param  width   Width to pad to, -1 for unspecified (default: 0)
     \param  ch      Padding character to use, 0 for unspecified (default: '0')
     \param  ch_sp   Padding character to use with special value (inf or nan), 0 for unspecified (default: ' ')
     \return         This
    */
    This& width(int width, char ch=0, char ch_sp=0) {
        fmt.pad_width = width;
        fmt.pad_ch    = ch;
        fmt.pad_ch_sp = ch_sp;
        return *this;
    }
};

typedef FmtFloatT<float>   FmtFloat;    ///< \copydoc FmtFloatT
typedef FmtFloatT<double>  FmtFloatD;   ///< \copydoc FmtFloatT
typedef FmtFloatT<ldouble> FmtFloatL;   ///< \copydoc FmtFloatT

///////////////////////////////////////////////////////////////////////////////

/** This pairs a FmtIntT type with FmtSetField for output formatting.
*/
template<class T>
struct FmtFieldNum {
    typedef FmtFieldNum<T> This;
    FmtIntT<T> num;
    FmtSetField field;

    FmtFieldNum(const FmtIntT<T>& num, FmtAlign align=fLEFT, int width=0, char fill=' ') : num(num), field(align, width, fill) {
    }

    FmtFieldNum(const This& src) : num(src.num), field(src.field) {
    }

    FmtFieldNum& operator=(const This& src) {
        num = src.num;
        field = src.field;
        return *this;
    }
};

/** This pairs a FmtFloatT type with FmtSetField for output formatting.
*/
template<class T>
struct FmtFieldFloat {
    typedef FmtFieldFloat<T> This;
    FmtFloatT<T> num;
    FmtSetField field;

    FmtFieldFloat(const FmtFloatT<T>& num, FmtAlign align=fLEFT, int width=0, char fill=' ') : num(num), field(align, width, fill) {
    }

    FmtFieldFloat(const This& src) : num(src.num), field(src.field) {
    }

    This& operator=(const This& src) {
        num = src.num;
        field = src.field;
        return *this;
    }
};

///////////////////////////////////////////////////////////////////////////////

/** Explicitly format a pointer.
 - Used with operator<<() on String, Stream, and StreamOut
 .

\par Example

\code
#include <evo/string.h>
using namespace evo;

int main() {
    int num = 1;

    // Format string as: 
    String str;
    str << FmtPtr(&num);

    return 0;
}
\endcode

*/
struct FmtPtr {
    const void* ptr;    ///< Pointer to format
    FmtSetInt fmt;      ///< Formatting attributes

    /** Constructor for formatting a pointer.
     \param  ptr     Pointer to format
     \param  prefix  Formatting prefix to use (see FmtBasePrefix), fbpCURRENT for unspecified (default: fPREFIX0)
     \param  width   Width to pad to, -1 for unspecified (default: 0)
     \param  ch      Padding character to use, 0 for unspecified (default: '0')
    */
    FmtPtr(const void* ptr, FmtBasePrefix prefix=fbpCURRENT, int width=-1, char ch=0) : ptr(ptr), fmt(fHEX, prefix, width, ch) {
    }

    /** Constructor for formatting a pointer.
     \param  ptr     Pointer to format
     \param  base    Formatting base to use (see FmtBase), fbCURRENT for unspecified (default: fDEC)
     \param  prefix  Formatting prefix to use (see FmtBasePrefix), fbpCURRENT for unspecified (default: fPREFIX0)
     \param  width   Width to pad to, -1 for unspecified (default: 0)
     \param  ch      Padding character to use, 0 for unspecified (default: '0')
    */
    FmtPtr(const void* ptr, int base, FmtBasePrefix prefix=fbpCURRENT, int width=-1, char ch=0) : ptr(ptr), fmt(base, prefix, width, ch) {
    }
};

///////////////////////////////////////////////////////////////////////////////

/** Explicitly format a hex dump from buffer.
 - May span multiple lines if maxline is positive
 - Includes an offsets and ASCII dump as well if not in compact mode
*/
struct FmtDump {
    const void* buf;        ///< Buffer to dump
    ulong       size;       ///< Buffer size in bytes to dump
    uint        maxline;    ///< Maximum bytes per line to dump, 0 for none (all 1 line)
    bool        compact;    ///< Whether to use compact mode (no address or ASCII output)
    bool        upper;      ///< Whether to use uppercase hex, false for lowercase

    /** Explicitly format a hex dump of given buffer.
     \param  buf      Buffer to dump
     \param  size     Buffer size in bytes to dump
     \param  maxline  Maximum bytes per line to dump, 0 for none (all 1 line)
     \param  compact  Whether to use compact mode (no address or ASCII output)
     \param  upper    Whether to use uppercase hex, false for lowercase
    */
    FmtDump(const void* buf, ulong size, uint maxline=24, bool compact=false, bool upper=false) : buf(buf), size(size), maxline(maxline), compact(compact), upper(upper)
        { }
};

///////////////////////////////////////////////////////////////////////////////
//@}
}
#if defined(_MSC_VER)
    #pragma warning(pop)
#endif
#endif
