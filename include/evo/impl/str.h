// Evo C++ Library
/* Copyright (c) 2016 Justin Crowell
 This Source Code Form is subject to the terms of the Mozilla Public
 License, v. 2.0. If a copy of the MPL was not distributed with this
 file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
///////////////////////////////////////////////////////////////////////////////
/** \file str.h Evo implementation detail: String helpers. */
#pragma once
#ifndef INCL_evo_impl_str_h
#define INCL_evo_impl_str_h

// Includes
#include "sys.h"
#include "../meta.h"

// Namespace: evo
namespace evo {

/** \addtogroup EvoContainers */
//@{
///////////////////////////////////////////////////////////////////////////////

static const char NULL_DELIM = '\x7F';  ///< Used in some cases for NULL delimiter -- this char generally doesn't appear in any valid text

/** Internal: Helper for converting strings with quoting as needed. */
struct StrQuoting {
    /** Quoting type for string returned by quoting(). */
    enum Type {  // Note: Order matters here, see addq() below
        tNONE = 0,          ///< No quoting needed
        tSINGLE,            ///< Single-quotes: '
        tDOUBLE,            ///< Double-quotes: "
        tBACKTICK,          ///< Backtick: `
        tTRIPLE_SINGLE,     ///< Triple single-quotes: '''
        tTRIPLE_DOUBLE,     ///< Triple double-quotes: """
        tTRIPLE_BACKTICK,   ///< Triple backtick: ```
        tBACKTICK_DEL       ///< Backtick followed by DEL char (7F) -- last resort
    };

    /** Scan string data and determine required quoting type to make it parsable with a delimiter.
     - Quoting is needed to preserve existing quoting or given delim
     .
     \param  str    String pointer
     \param  size   String size
     \param  delim  Delimiter used for parsing, \link NULL_DELIM\endlink for none
     \return        Quoting type for string
    */
    static Type get(const char* str, ulong size, char delim=NULL_DELIM) {
        if (size > 0) {
            const char QS  = '\'';
            const char QD  = '"';
            const char QB  = '`';
            const char* end = str + size;

            bool d  = false,    // set when delimiter found
                 qs = false,    // set when single-quote found
                 qd = false,    // set when double-quote found
                 qb = false,    // set when backtick found
                 f  = false;    // set when first char is a quote (any type)
            const char* p = str;

            // First char
            if (*p == delim)
                d = true;
            else if (*p == QS)
                f = qs = true;
            else if (*p == QD)
                f = qd = true;
            else if (*p == QB)
                f = qb = true;
            ++p;

            // Remaining chars
            while (p < end) {
                if (*p == delim) {
                    if (!d)
                        d = true;
                } else if (*p == QS) {
                    if (!qs)
                        qs = true;
                } else if (*p == QD) {
                    if (!qd)
                        qd = true;
                } else if (*p == QB && !qb)
                    qb = true;
                ++p;
            }

            // Check if quoting required
            if (d || f) {
                if (!qs)
                    return tSINGLE;
                if (!qd)
                    return tDOUBLE;
                if (!qb)
                    return tBACKTICK;

                // Rescan for triple quoting
                ulong count = 0;
                char  last  = 0;
                qs = false;     // set when triple single-quotes found
                qd = false;     // set when triple double-quotes found
                qb = false;     // set when triple backticks found
                for (p=str; p < end; ++p) {
                    if (*p == last && last != 0) {
                        // Consecutive quote
                        if (++count == 3) {
                            switch (last) {
                                case QS:
                                    qs = true;
                                    break;
                                case QD:
                                    qd = true;
                                    break;
                                case QB:
                                    qb = true;
                                    break;
                            }
                            last = 0;
                        }
                    } else {
                        switch (*p) {
                            case QS:
                            case QD:
                            case QB:
                                last  = *p;
                                count = 1;
                                break;
                            default:
                                if (last != 0)
                                    last = 0;
                                break;
                        }
                    }
                }

                // Determine quoting needed
                if (!qs)
                    return tTRIPLE_SINGLE;
                if (!qd)
                    return tTRIPLE_DOUBLE;
                if (!qb)
                    return tTRIPLE_BACKTICK;
                return tBACKTICK_DEL;  // last resort
            }
        }
        return tNONE;
    }

    /** Append string value to string with quoting as needed.
     - This calls get() to determine quoting needed
     - This calls dest.add() to append needed quotes and string value
     - When applicable, strings are scanned to determine how to quote:
       - no quoting if delim not preset and doesn't start with any quotes (containing quotes is ok)
       - single-quoting ( ' ), double-quoting ( " ), backtick-quoting ( \` ): 'foo bar'
       - triple-quoting ( ''' or """ or \`\`\` ): '''foo bar'''
       - backtick-DEL (backtick then code 7F) quoting as a rare last resort ( \`␡ ): \`␡foo bar\`␡
       .
     - This is used by Convert::addq() when applicable
     .
     \param  dest   Destination string to append to
     \param  value  String value to appened
     \param  delim  Delimiter to use for determining quoting, must not be a letter or digit
    */
    template<class T, class C>
    static void addq(C& dest, T value, typename C::Value delim) {
        typedef typename T::ListBaseType BaseType;
        const BaseType& value_base = (const BaseType&)value;
        const Type type = StrQuoting::get(value_base.data_, value_base.size_, delim);
        if (type == tNONE) {
            // No quoting needed
            dest.add(value);
        } else if (type < tTRIPLE_SINGLE) {
            // Single char quotes
            const char* const Q_CH = "'\"`";
            const char q = Q_CH[(int)type - (int)tSINGLE];
            dest.reserve(value_base.size_+2);
            dest.add(q).add(value).add(q);
        } else if (type < tBACKTICK_DEL) {
            // Triple char quotes
            const char* const Q_STR = "'''\"\"\"```";
            const char* const q = Q_STR + (((int)type - (int)tTRIPLE_SINGLE) * 3);
            dest.reserve(value_base.size_+6);
            dest.add(q, 3).add(value).add(q, 3);
        } else {
            // Double char quotes
            const char* const Q_STR = "`\x7F";
            dest.reserve(value_base.size_+4);
            dest.add(Q_STR, 2).add(value).add(Q_STR, 2);
        }
    }
};

///////////////////////////////////////////////////////////////////////////////

// Implementation
/** \cond impl */
namespace impl
{
    // Convert string to integer value
    template<class T>
    static T tonum(const char* str, ulong size, Error& error, int base) {
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

        // Detect base, if needed
        if (base == 0) {
            if ((ch=*str) == '0') {
                if (++str < end && ((ch=*str) == 'x' || ch == 'X')) {
                    // Hex
                    base = 16;
                    if (++str == end)
                        { error = EInval; return 0; }
                } else
                    // Octal
                    base = 8;
            } else if (ch == 'x') {
                // Hex
                base = 16;
                if (++str == end)
                    { error = EInval; return 0; }
            } else
                // Decimal
                base = 10;
        } else if (base == 16) {
            // Skip hex prefix
            if (*str == 'x')
                ++str;
            else if (*str == '0' && str+1 < end && ((ch=str[1]) == 'x' || ch == 'X'))
                str += 2;
            if (str == end)
                { error = EInval; return 0; }
        }

        // Limits
        const typename ToUnsigned<T>::Type limitnum = (neg ? IntegerT<T>::minabslimit() : IntegerT<T>::max());
        const typename ToUnsigned<T>::Type limitbase = limitnum / (typename ToUnsigned<T>::Type)base;

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
            if (num > limitbase || (num == limitbase && ch > (limitnum % base)))
                { error = EOutOfBounds; return (neg ? IntegerT<T>::min() : IntegerT<T>::max()); }
            num *= (typename ToUnsigned<T>::Type)base;
            num += ch;
        }
        return (neg ? -(typename ToSigned<T>::Type)num : num);
    }

    // Convert string to floating-point value
    template<class T>
    static T tonumf(const char* str, ulong size, Error& error) {
        static const int maxdigits = FloatT<T>::maxdigits + 2;
        static const int base = 10;
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
                if ( FloatT<T>::nanok && end-str >= 3 &&
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
                        exp *= base;
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
                if (sig_digits > maxdigits) {
                    ++exp;
                } else {
                    num *= base;
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
        assert( base >= 0 && base <= 36 );
        static const char* digits = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
        char* ptr = endptr;
        if (num == 0)
            *--ptr = '0';
        else if (num > 0) {
            while (num != 0) {
                *--ptr = digits[num % base];
                num /= base;
            }
        } else {
            typename ToUnsigned<T>::Type unum = (typename ToUnsigned<T>::Type)-num;
            while (unum != 0) {
                *--ptr = digits[unum % base];
                unum /= base;
            }
            *--ptr = '-';
        }
        return (endptr - ptr);
    }

    // Format unsigned integer as string
    template<class T>
    static ulong fnumu(char* endptr, T num, int base) {
        assert( endptr != NULL );
        assert( base >= 0 && base <= 36 );
        static const char* digits = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
        char* ptr = endptr;
        if (num == 0)
            *--ptr = '0';
        else
            while (num != 0) {
                *--ptr = digits[num % base];
                num /= base;
            }
        return (endptr - ptr);
    }

    // Weight floating point number so it's rounded to given precision
    template<class T>
    static T fnumf_weight(T num, int precision) {
        T dummy;
        if (num != 0.0 && evo_modf(num, &dummy) != 0.0) {
            T weight = 0.5;
            for (int i=0; i<precision; ++i)
                weight /= 10.0;
            if (num < 0.0)
                num -= weight;
            else
                num += weight;
        }
        return num;
    }

    // Format normalized floating point number as string using given precision
    template<class T>
    static ulong fnumf(char* ptr, T num, int exp, int precision) {
        static const T flt_precision = FloatT<T>::precision();
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
            return (ptr - startptr);
        }

        // Fraction leading zeroes
        if (exp <= 0) {
            *ptr = '0'; ++ptr;
            if (precision <= 0)
                return (ptr - startptr); // Done, ignore fraction
            *ptr = '.'; ++ptr;
            while (exp < 0 && precision > 0) {
                *ptr = '0'; ++ptr;
                ++exp; --precision;
            }
        }

        // Significant digits
        int digit = 0, count = 0;
        while (num > flt_precision) {
            // Get next digit
            num *= 10.0;
            digit = (int)(num + 0.03);
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
            if (++count > FloatT<T>::maxdigits)
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
        return (ptr - startptr);
    }

    // Format normalized floating point number as string using normal or e notation
    template<class T>
    static ulong fnumfe(char* ptr, T num, int exp, bool cap) {
        static const T flt_precision = FloatT<T>::precision();
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
            return (ptr - startptr);
        }

        // Adjust for exponent notation
        static const int e_threshold = (FloatT<T>::maxdigits > 6 ? 6 : FloatT<T>::maxdigits);
        bool on_fraction = false;
        bool show_e      = false;
        if (exp < -2 || exp > e_threshold) {
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

        // Significant digits
        int digit = 0, count = 0, zero_count = 0;
        while (num > flt_precision) {
            // Get next digit
            num *= 10.0;
            digit = (int)(num + 0.03);
            if (digit > 9)
                digit = 9;

            // Remove next digit
            num -= (T)digit;

            // Add digit
            if (++count > FloatT<T>::maxdigits)
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
        return (ptr - startptr);
    }

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

/** Use to specify floating point formatting with automatic precision -- either normal decimal notation or E notation, whichever is shorter. */
static const int PREC_AUTO = -1;

// TODO -- needed?
/* Special floating point precision value for E notation */
//static const int PREC_E = -2;

///////////////////////////////////////////////////////////////////////////////
} // Namespace: evo
//@}
#endif
