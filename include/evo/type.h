// Evo C++ Library
/* Copyright (c) 2016 Justin Crowell
 This Source Code Form is subject to the terms of the Mozilla Public
 License, v. 2.0. If a copy of the MPL was not distributed with this
 file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
///////////////////////////////////////////////////////////////////////////////
/** \file type.h Evo basic types and traits. */
#pragma once
#ifndef INCL_evo_type_h
#define INCL_evo_type_h

// Includes
#include "meta.h"
#include "impl/container.h"

// Namespace: evo
namespace evo {

/** \addtogroup EvoCore */
//@{

///////////////////////////////////////////////////////////////////////////////

/** Safe bool base class.
This implements "safe" boolean evaluation, without the side-effects of implicit conversions from the bool type.
 - Use by deriving the intended class from SafeBool.
 - The deriving class must implement the logical negation operator (as const) for boolean evaluation.
   \code
bool operator!() const
   \endcode
 - Safe bool comparisons (\c obj1 \c == \c obj2 or \c obj1 \c != \c obj1 \c ) will not compile unless the deriving class implements operator==() or operator!=(), respectively.
 - No virtual methods are used even though this is a base class.
 .
 \tparam  T  The deriving type being used.
\par Example:
\code
// Class Foo is a safe bool
struct Foo : public SafeBool<Foo>
{
    bool operator!() const
        { return false; }
};

void func()
{
    const Foo foo;
    // Safe bool evaluation (calls !operator!())
    if (foo)
    {
        // ...
    }
    // Negative bool evaluation (calls operator!())
    if (!foo)
    {
        // ...
    }
}
\endcode
*/
template<class T> class SafeBool {
protected:
    /** \cond impl */
    typedef void (SafeBool::*SafeBoolType)() const;
    void This_type_does_not_support_comparisons() const { }
    /** \endcond */

public:
    /** Constructor. */
    SafeBool() { }
    /** \cond impl */
    SafeBool(const SafeBool&) { }
    SafeBool& operator=(const SafeBool&) { return *this; }
    /** \endcond */

    /** Safe (explicit) evaluation as bool type.
     - This is called when object is directly evaluated as a bool.
     - This calls !operator!() on object.
     - See SafeBool description for more information.
     .
    */
    operator SafeBoolType() const
        { return (!(static_cast<const T*>(this))->operator!() ? &SafeBool::This_type_does_not_support_comparisons : 0); }
};

/** \cond impl */
template<typename T, typename U> bool operator==(const SafeBool<T>& l, const SafeBool<U>& r)
    { l.This_type_does_not_support_comparisons(); return false; }
template<typename T, typename U> bool operator!=(const SafeBool<T>& l, const SafeBool<U>& r)
    { l.This_type_does_not_support_comparisons(); return false; }
/** \endcond */

/** \cond impl */
namespace impl
{
    struct SafeBoolTestCov : SafeBool<SafeBoolTestCov> {
        void test()
            { This_type_does_not_support_comparisons(); }
    };
}
/** \endcond */

///////////////////////////////////////////////////////////////////////////////

/** %Nullable primitive base type (used internally).
See: Bool, Char, \link evo::Int Int\endlink, \link evo::Long Long\endlink, \link evo::UInt UInt\endlink,
\link evo::ULong ULong\endlink, \link evo::Float Float\endlink, \link evo::FloatD FloatD\endlink, \link evo::FloatL FloatL\endlink
*/
template<class T>
class Nullable {
public:
    Nullable()
        { null_ = true; value_ = (T)0; }
    Nullable(const Nullable<T>& src)
        { null_ = src.null_; value_ = src.value_; }
    Nullable(T num)
        { null_ = false; value_ = num; }
    Nullable<T>& operator=(const Nullable<T>& src)
        { null_ = src.null_; value_ = src.value_; return *this; }
    Nullable<T>& operator=(T num)
        { null_ = false; value_ = num; return *this; }
    Nullable<T>& operator=(ValNull)
        { null_ = true; value_ = (T)0; return *this; }

    /** Implicit conversion to underlying type.
     \return  Underlying type value
     */
    operator T() const
        { return value_; }
    /** Dereference for explicit conversion to underlying type. Alternative to value().
     \return  Underlying type value
     */
    const T& operator*() const
        { return value_; }

    /** Get whether null.
     \return  Whether null
     */
    bool null() const
        { return null_; }
    /** Get whether valid (not null).
     \return  Whether valid
     */
    bool valid() const
        { return !null_; }
    /** Get underlying value. Alternative to operator*().
     \return  Whether valid
     */
    T value() const
        { return value_; }

    /** Set as null.
     \return  This
     */
    Nullable<T>& set()
        { null_ = true; value_ = (T)0; return *this; }
    /** Set as copy of given value.
     \param  src  Source to copy
     \return      This
     */
    Nullable<T>& set(const Nullable<T>& src)
        { null_ = src.null_; value_ = src.value_; return *this; }
    /** Set as given value.
     \param  src  Source to set
     \return      This
     */
    Nullable<T>& set(T src)
        { null_ = false; value_ = src; return *this; }

private:
    T    value_;
    bool null_;
};
// Specialized for minimum size
/** \cond impl */
template<> class Nullable<bool> {
public:
    Nullable()
        { value_ = (uchar)nvNull; }
    Nullable(const Nullable<bool>& src)
        { value_ = src.value_; }
    Nullable(bool val)
        { value_ = (uchar)(val ? nvTrue : nvFalse); }
    Nullable<bool>& operator=(const Nullable<bool>& src)
        { value_ = src.value_; return *this; }
    Nullable<bool>& operator=(bool val)
        { value_ = (uchar)(val ? nvTrue : nvFalse); return *this; }
    Nullable<bool>& operator=(ValNull)
        { value_ = (uchar)nvNull; return *this; }

    operator bool() const
        { return (value_ == (uchar)nvTrue); }
    bool operator*() const
        { return (value_ == (uchar)nvTrue); }

    bool null() const
        { return (value_ == (uchar)nvNull); }
    bool valid() const
        { return (value_ != (uchar)nvNull); }
    bool value() const
        { return (value_ == (uchar)nvTrue); }

    // TODO
    Nullable<bool>& set()
        { value_ = (uchar)nvNull; return *this; }
    Nullable<bool>& set(const Nullable<bool>& src)
        { value_ = src.value_; return *this; }
    Nullable<bool>& set(bool val)
        { value_ = (uchar)(val ? nvTrue : nvFalse); return *this; }

private:
    enum NullableValue {
        nvNull=0,
        nvFalse,
        nvTrue
    };

    uchar value_;
};
/** \endcond */

///////////////////////////////////////////////////////////////////////////////

/** Basic boolean type. A basic Evo container for boolean values.
 - Assigned like standard bool\n
   \code
Bool val = true;  // set to true
   \endcode
 - Can hold null value\n
   \code
Bool val;         // null by default
val = false;      // set to false, no longer null
val = vNull;      // set to null
   \endcode
 - Best to dereference to get standard bool value, though operator bool() supports implicit conversion\n
   \code
Bool val1;        // null by default
if (*val1) {      // dereference for explicit bool value, best practice for clarity
                  // false when null
}
if (val1) {       // implicit conversion works too but may cause type ambiguity
                  // false when null
}
bool val2 = val1; // implicit conversion to bool (false when null)
val2 = *val1;     // best practice to dereference for explicit conversion to bool
   \endcode
 */
struct Bool : public Nullable<bool>
{
    /** Wrapped type (bool). */
    typedef bool Type;

    static const int bytes = sizeof(bool);            ///< Type size in bytes
    static const int bits  = 1;                        ///< Type size in bits

    /** Constructor. */
    Bool()
        { }
    /** Copy constructor.
     \param  val  Value to copy
    */
    Bool(const Bool& val) : Nullable<bool>(val)
        { }
    /** Constructor.
     \param  val  Value to set
    */
    Bool(bool val) : Nullable<bool>(val)
        { }
    /** Assignment operator.
     \param  val  Value to copy
     \return      This
    */
    Bool& operator=(const Bool& val)
        { Nullable<bool>::operator=(val); return *this; }
    /** Assignment operator.
     \param  val  Value to set
     \return      This
    */
    Bool& operator=(bool val)
        { Nullable<bool>::operator=(val); return *this; }
    /** Assignment operator to set as null.
     \return  This
    */
    Bool& operator=(ValNull)
        { Nullable<bool>::set(); return *this; }
};

///////////////////////////////////////////////////////////////////////////////

/** %Nullable primitive character base type.
See: Char
 \tparam  T  Character type
*/
template<class T>
struct CharT : public Nullable<T>
{
    /** Wrapped type. */
    typedef T Type;

    static const int bytes = sizeof(T);                ///< Type size in bytes
    static const int bits  = bytes * 8;                ///< Type size in bits

    /** Character category.
     - Overlapping categories (ex: alphanumeric) can be checked with greater-than (>) comparison:
       \code
Char::Category cat = Char::category(ch);
bool space     = (cat == Char::cSpace);     // Whitespace (space, tab)
bool symbol    = (cat == Char::cSymbol);    // Symbols (comma, precent, dollar, etc)
bool digit     = (cat == Char::cDigit);     // Decimal digit (0-9)
bool uppercase = (cat == Char::cAlphaU);    // Alphabet uppercase (A-Z)
bool lowercase = (cat == Char::cAlphaL);    // Alphabet lowercase (a-z)
bool alphanum  = (cat > Char::cAlphaNum);   // Alphanumeric (0-9, A-Z, a-z)
bool alpha     = (cat > Char::cAlpha);      // Alphabet (A-Z, a-z)
bool printable = (cat > Char::cNone);       // Any printable character
bool visible   = (cat > Char::cSpace);      // Any visible character (printable and not whitespace)
       \endcode
     .
    */
    enum Category {
        cNone=0,        ///< Non printable char
        cSpace,            ///< Whitespace (space, tab)
        cSymbol,        ///< Symbol character (printable but not alphanumeric)
        cAlphaNum,        ///< Alpha-numeric -- categories greater than this are alphanumeric
        cDigit,            ///< Decimal digit (0-9)
        cAlpha,            ///< Alphabet -- categories greater than this are alphabetic
        cAlphaU,        ///< Alphabet uppercase (A-Z)
        cAlphaL            ///< Alphabet lowercase (a-z)
    };

    /** Character digit type.
     - Digit types overlap so check with greater-than-or-equal (>=) comparison:
       \code
Char::Digit dig = Char::digit(ch);
bool b36 = (dig >= Char::dBase36);  // base 36 digit (0-9, A-Z)
bool hex = (dig >= Char::dHex);     // hex digit (0-9, A-F)
bool dec = (dig >= Char::dDecimal); // decimal digit (0-9)
bool oct = (dig >= Char::dOctal);   // octal digit (0-7)
bool err = (dig == Char::dHex);     // error: must use operator >= here
       \endcode
     .
    */
    enum Digit {
        dNone=0,            ///< Not a digit
        dBase36,            ///< Base 36 character
        dHex,                ///< Hexadecimal character
        dDecimal,            ///< Decimal character
        dOctal                ///< Octal character
    };

    /** Constructor. */
    CharT()
        { }
    /** Copy constructor.
     \param  val  Value to copy
    */
    CharT(const CharT<T>& val) : Nullable<T>(val)
        { }
    /** Constructor.
     \param  val  Value to set
    */
    CharT(T val) : Nullable<T>(val)
        { }
    /** Assignment operator.
     \param  val  Value to copy
     \return      This
    */
    CharT<T>& operator=(const CharT<char>& val)
        { Nullable<T>::operator=(val); return *this; }
    /** Assignment operator.
     \param  val  Value to set
     \return      This
    */
    CharT<T>& operator=(char val)
        { Nullable<T>::operator=(val); return *this; }
    /** Assignment operator to set as null.
     \return  This
    */
    CharT<T>& operator=(ValNull val)
        { Nullable<T>::set(); return *this; }
};

/** Basic character type. A basic Evo container for character values.
 - Assigned like standard char\n
   \code
Char val = 'A';             // set to 'A'
   \endcode
 - Can hold null value\n
   \code
Char val;                   // null by default -- null converts to char as 0
val = 'B';                  // set to 'B', no longer null
val = vNull;                // set to null
   \endcode
 - Best to dereference to get standard char value, though operator char() supports implicit conversion\n
   \code
Char val1;                  // null by default
if (*val1 == 'A') {         // dereference for explicit char value, best practice for clarity
                            // false: 0 when null
}
if (val1 == 'A') {          // implicit conversion works too but may cause type ambiguity
                            // false: 0 when null
}
char val2 = val1;           // implicit conversion to char (0 when null)
val2 = *val1;               // best practice to dereference for explicit conversion to char
   \endcode
 - Traits and helpers\n
   \code
int num_bits = Char::bits;                   // get number of bits per char (answer depends on char size type, usually 8 bits)
if (Char::category('1') > Char::cAlphaNum) { // check character category (alpha-numeric)
                                             // true
}
if (Char::isupper('A') {                     // check if upper-case character (static helper)
                                             // true
}
   \endcode
*/
struct Char : public CharT<char>
{
    /** Get character category.
     - Extended ASCII characters are considered unprintable
     .
     \param  ch  Character to check
     \return     Category
    */
    static Category category(char ch) {
        const char chmap[128] = {
            0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0,            // 0-15
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,            // 16-31
            1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,            // 32-47
            4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 2, 2, 2, 2, 2, 2,            // 48-63
            2, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,            // 64-79
            6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 2, 2, 2, 2, 2,            // 80-95
            2, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,            // 96-111
            7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 2, 2, 2, 2, 0            // 112-127
        };
        return (Category)((uchar)ch < 128 ? chmap[(uint)ch] : 0);
    }

    /** Get digit type for character.
     - Extended ASCII characters aren't considered digits
     .
     \param  ch  Character to check
     \return     Digit type
    */
    static Digit digit(char ch) {
        const char chmap[128] = {
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,            // 0-15
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,            // 16-31
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,            // 32-47
            4, 4, 4, 4, 4, 4, 4, 4, 3, 3, 0, 0, 0, 0, 0, 0,            // 48-63
            0, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1,            // 64-79
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0,            // 80-95
            0, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1,            // 96-111
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0            // 112-127
        };
        return (Digit)((uchar)ch < 128 ? chmap[(uint)ch] : 0);
    }

    // TODO - use char maps
    // TODO - other checks: http://unixhelp.ed.ac.uk/CGI/man-cgi?isalpha+3

    /** Check whether uppercase letter (A-Z).
     \param  ch  Character to check
     \return     Whether uppercase
    */
    static bool isspace(char ch)
        { return (ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r'); }

    /** Check whether uppercase letter (A-Z).
     \param  ch  Character to check
     \return     Whether uppercase
    */
    static bool isupper(char ch)
        { return (ch >= 'A' && ch <= 'Z'); }

    /** Check whether lowercase letter (a-z).
     \param  ch  Character to check
     \return     Whether lowercase
    */
    static bool islower(char ch)
        { return (ch >= 'a' && ch <= 'z'); }

    /** Check whether alphabetic (A-Z, a-z).
     \param  ch  Character to check
     \return     Whether alphabetic
    */
    static bool isalpha(char ch)
        { return ((ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z')); }

    /** Check whether alphanumeric (A-Z, a-z, 0-9).
     \param  ch  Character to check
     \return     Whether alphanumeric
    */
    static bool isalnum(char ch)
        { return ((ch >= '0' && ch <= '9') || (ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z')); }

    /** Check whether a digit (0-9).
     \param  ch  Character to check
     \return     Whether a digit
    */
    static bool isdigit(char ch)
        { return (ch >= '0' && ch <= '9'); }

    /** Constructor. */
    Char()
        { }
    /** Copy constructor.
     \param  val  Value to copy
    */
    Char(const CharT<char>& val) : CharT<char>(val)
        { }
    /** Constructor.
     \param  val  Value to set
    */
    Char(char val) : CharT<char>(val)
        { }
    /** Assignment operator.
     \param  val  Value to copy
     \return      This
    */
    Char& operator=(const CharT<char>& val)
        { Nullable<char>::operator=(val); return *this; }
    /** Assignment operator.
     \param  val  Value to set
     \return      This
    */
    Char& operator=(char val)
        { Nullable<char>::operator=(val); return *this; }
    /** Assignment operator to set as null.
     \return  This
    */
    Char& operator=(ValNull)
        { Nullable<char>::set(); return *this; }
};

///////////////////////////////////////////////////////////////////////////////

/** Basic integer type. A basic Evo container template for integer types.
 \tparam  T  Integer plain type (short, int, long, longl, ushort, uint, ulong, ulongl, etc)

Integer types:
 - \link evo::Short Short\endlink, \link evo::Int Int\endlink, \link evo::Long Long\endlink, \link evo::LongL LongL\endlink
 - \link evo::UShort UShort\endlink, \link evo::UInt UInt\endlink, \link evo::ULong ULong\endlink, \link evo::ULongL ULongL\endlink

Note: Examples here use Int type (and int type) but the same applies to all supported integer types.

 - Assigned like standard integer\n
   \code
Int val = 123;              // set to 123
   \endcode
 - Can hold null value\n
   \code
Int val;                    // null by default
val = 567;                  // set to 567, no longer null
val = vNull;                // set to null
   \endcode
 - Best to dereference to get standard int value, though operator int() supports implicit conversion\n
   \code
Int val1;                   // null by default -- null converts to int as 0
if (*val1 == 123) {         // dereference for explicit int value, best practice for clarity
                            // false: 0 when null
}
if (val1 == 123) {          // implicit conversion works too but may cause type ambiguity
                            // false: 0 when null
}
int val2 = val1;            // implicit conversion to int (0 when null)
val2 = *val1;               // best practice to dereference for explicit conversion to int
   \endcode
 - Traits and helpers\n
   \code
int num_bits = Int::bits;        // get number of bits per int (answer depends on int type size, often 32 bits)
int digits = Int::digits(12345); // get number of digits in number 12345 (answer is 5) (static helper)
   \endcode
*/
template<class T>
struct IntegerT : public Nullable<T>
{
    /** Wrapped type. */
    typedef T Type;

    static const bool sign  = IsSigned<T>::value;        ///< Whether type is signed
    static const int  bytes = sizeof(T);                ///< Type size in bytes
    static const int  bits  = bytes * 8;                ///< Type size in bits

    /** Get value with right-most (first/least-significant) bit set.
     \return  Value. */
    static T bitr()
        { return 1; }

    /** Get value with left-most (last/most-significant) bit set.
     \return  Value. */
    static T bitl()
        { static const T val = (typename ToUnsigned<T>::Type)bitr() << (bits-1); return val; }

    /** Get minimum value.
     \return  Value. */
    static T min()
        { static const T val = (sign ? bitl() : 0); return val; }

    /** Get minimum value as absolute number (0 if unsigned).
     \return  Value. */
    static T minabs()
        { static const T val = (sign ? -min() : 0); return val; }

    static T minabslimit()
        { static const T val = (sign ? -(typename ToSigned<T>::Type)min() : max()); return val; }

    /** Get maximum value for type.
     \return  Value. */
    static T max()
        { static const T val = (sign ? ~bitl() : ~min()); return val; }

    /** Get maximum formatted length for type at base. Includes sign if signed.
     \return  Value. */
    static int maxlen(int base=10) {
        // Note: Newer compilers (gcc 4.9.2) give c++11 'narrowing conversion' warnings if 'values' are smaller than int, casting doesn't fix
        static const int n = (sign ? 1 : 0);
        static const int values[] = { 0, 0,
            digits(max(),2)+n,  digits(max(),3)+n,  digits(max(),4)+n,  digits(max(),5)+n,  digits(max(),6)+n,  digits(max(),7)+n,
            digits(max(),8)+n,  digits(max(),9)+n,  digits(max(),10)+n, digits(max(),11)+n, digits(max(),12)+n, digits(max(),13)+n,
            digits(max(),14)+n, digits(max(),15)+n, digits(max(),16)+n, digits(max(),17)+n, digits(max(),18)+n, digits(max(),19)+n,
            digits(max(),20)+n, digits(max(),21)+n, digits(max(),22)+n, digits(max(),23)+n, digits(max(),24)+n, digits(max(),25)+n,
            digits(max(),26)+n, digits(max(),27)+n, digits(max(),28)+n, digits(max(),29)+n, digits(max(),30)+n, digits(max(),31)+n,
            digits(max(),32)+n, digits(max(),33)+n, digits(max(),34)+n, digits(max(),35)+n, digits(max(),36)+n
        };
        return values[base];
    }

    /** Get number of digits for given number and base. Includes sign if negative.
     \return Value. */
    static int digits(T num, int base=10) {
        assert( base > 0 );
        int result = 0;
        if (num == 0)
            result = 1;
        else {
            if (num < 0)
                ++result;
            while (num != 0) {
                num /= base;
                ++result; 
            }
        }
        return result;
    }

    // TODO - basic bit manipulation methods?

    /** Constructor. */
    IntegerT()
        { }
    /** Copy constructor.
     \param  val  Value to copy
    */
    IntegerT(const IntegerT<T>& val) : Nullable<T>(val)
        { }
    /** Constructor.
     \param  val  Value to set
    */
    IntegerT(T val) : Nullable<T>(val)
        { }
    /** Assignment operator.
     \param  val  Value to copy
     \return      This
    */
    IntegerT<T>& operator=(const IntegerT<T>& val)
        { Nullable<T>::operator=(val); return *this; }
    /** Assignment operator.
     \param  val  Value to set
     \return      This
    */
    IntegerT<T>& operator=(T val)
        { Nullable<T>::operator=(val); return *this; }
    /** Assignment operator to set as null.
     \return  This
    */
    IntegerT<T>& operator=(ValNull val)
        { Nullable<T>::set(); return *this; }
};

/** Basic integer type (short) -- see IntegerT. A basic Evo container for integer values.
\code
Short num = 123;
\endcode
*/
typedef IntegerT<short> Short;
/** Basic integer type (int) -- see IntegerT. A basic Evo container for integer values.
\code
Int num = 123;
\endcode
*/
typedef IntegerT<int> Int;
/** Basic integer type (long) -- see IntegerT. A basic Evo container for integer values.
\code
Long num = 123;
\endcode
*/
typedef IntegerT<long> Long;
/** Basic integer type (long long) -- see IntegerT. A basic Evo container for integer values.
\code
LongL num = 123;
\endcode
*/
typedef IntegerT<longl> LongL;

/** Basic integer type (int8) -- see IntegerT. A basic Evo container for integer values.
\code
Int8 num = 123;
\endcode
*/
typedef IntegerT<int8> Int8;
/** Basic integer type (int16) -- see IntegerT. A basic Evo container for integer values.
\code
Int16 num = 123;
\endcode
*/
typedef IntegerT<int16> Int16;
/** Basic integer type (int32) -- see IntegerT. A basic Evo container for integer values.
\code
Int32 num = 123;
\endcode
*/
typedef IntegerT<int32> Int32;
/** Basic integer type (int64) -- see IntegerT. A basic Evo container for integer values.
\code
Int64 num = 123;
\endcode
*/
typedef IntegerT<int64> Int64;

/** Basic integer type (unsigned short) -- see IntegerT. A basic Evo container for integer values.
\code
ushort num = 123;
\endcode
*/
typedef IntegerT<ushort> UShort;
/** Basic integer type (unsigned int) -- see IntegerT. A basic Evo container for integer values.
\code
uint num = 123;
\endcode
*/
typedef IntegerT<uint> UInt;
/** Basic integer type (unsigned long) -- see IntegerT. A basic Evo container for integer values.
\code
ulong num = 123;
\endcode
*/
typedef IntegerT<ulong> ULong;
/** Basic integer type (unsigned long long) -- see IntegerT. A basic Evo container for integer values.
\code
ulongl num = 123;
\endcode
*/
typedef IntegerT<ulongl> ULongL;

/** Basic integer type (uint8) -- see IntegerT. A basic Evo container for integer values.
\code
UInt8 num = 123;
\endcode
*/
typedef IntegerT<uint8> UInt8;
/** Basic integer type (uint16) -- see IntegerT. A basic Evo container for integer values.
\code
UInt16 num = 123;
\endcode
*/
typedef IntegerT<uint16> UInt16;
/** Basic integer type (uint32) -- see IntegerT. A basic Evo container for integer values.
\code
UInt32 num = 123;
\endcode
*/
typedef IntegerT<uint32> UInt32;
/** Basic integer type (uint64) -- see IntegerT. A basic Evo container for integer values.
\code
UInt64 num = 123;
\endcode
*/
typedef IntegerT<uint64> UInt64;

///////////////////////////////////////////////////////////////////////////////

/** %Nullable basic floating-point base type.
See: \link evo::Float Float\endlink, \link evo::FloatD FloatD\endlink, \link evo::FloatL FloatL\endlink
 \tparam  T  Floating-point type
*/
template<class T>
struct FloatT : public Nullable<T>
{
    /** Wrapped type. */
    typedef T Type;

    static const bool is        = IsFloat<T>::value;                        ///< Whether type is a floating point type
    static const bool sign      = true;                                        ///< Whether type is signed
    static const int  bytes     = sizeof(T);                                ///< Type size in bytes
    static const int  maxdigits = std::numeric_limits<T>::digits10;            ///< Maximum significant digits without precision loss
    static const bool nanok     = std::numeric_limits<T>::has_quiet_NaN;    ///< Whether Not-A-Number (NaN) is supported

    static const int maxdigits_auto = maxdigits + 15;    ///< Max formatting digits with auto precision (used internally)

    /** Get max formatting digits with given exponent and precision, including sign and any additional chars (used internally).
     \return  Max formatting digits. */
    static int maxdigits_prec(int exp, int precision)
        { const int BASEDIGITS = maxdigits + 9; return BASEDIGITS + (exp<0?-exp:exp) + precision; }

    /** Get best precision value.
     \return  Value */
    static T precision()
        { static const T val = evo_pow((T)0.1, std::numeric_limits<T>::digits10); return val; }

    /** Get minimum normalized value.
     \return  Value */
    static T min()
        { return std::numeric_limits<T>::min(); }

    /** Get minimum allowed exponent.
     \return  Value */
    static int minexp()
        { return std::numeric_limits<T>::min_exponent10; }

    /** Get maximum normalized value.
     \return  Value */
    static T max()
        { return std::numeric_limits<T>::max(); }

    /** Get maximum allowed exponent.
     \return  Value */
    static int maxexp()
        { return std::numeric_limits<T>::max_exponent10; }

    /** Get infinity value.
     \return  Value */
    static T inf()
        { return std::numeric_limits<T>::infinity(); }

    /** Check whether value if infinite.
     \return  Value */
    static bool inf(T num) {
        if (num < 0.0)
            num = -num;
        return (std::numeric_limits<T>::has_infinity && num == std::numeric_limits<T>::infinity());
    }

    /** Get whether value is Not-A-Number (NaN).
     \return  Whether NaN, always false if NaN not supported */
    static bool nan(T num)
        { return (num != num); }

    /** Get Not-A-Number (NaN) value.
     - This value doesn't work for comparison. Use nan(T) instead.
     .
     \return  NaN value, 0.0 if not supported */
    static T nan() {
        static const T val = (T)(nanok ? std::numeric_limits<T>::quiet_NaN() : 0.0);
        return val;
    }

    /** Get machine epsilon. This is the difference between 1 and the least value greater than 1 that is representable.
     \return  Epsilon value */
    static T eps()
        { return std::numeric_limits<T>::epsilon(); }

    /** Get whether values are approximately equal.
     - This uses the system epsilon value for current type to determine whether the values are close enough to be considered equal
     - For better accuracy use eq(T,T,T) with a more relevant epsilon value
     .
     \param  val1  First value to compare
     \param  val2  Second value to compare
     \return       Whether approximately equal
    */
    static bool eq(T val1, T val2) {
        static const T epsilon = std::numeric_limits<T>::epsilon();
        return ( (nan(val1) && nan(val2)) || val1 == val2 || evo_fabs(val1 - val2) <= epsilon );
    }

    /** Get whether values are approximately equal using given epsilon value.
     \param  val1  First value to compare
     \param  val2  Second value to compare
     \param  eps   Epsilon value to use for comparison -- values are approximately equal if difference <= eps
     \return       Whether approximately equal
    */
    static bool eq(T val1, T val2, T eps)
        { return ( (nan(val1) && nan(val2)) || val1 == val2 || evo_fabs(val1 - val2) <= eps ); }

    /** Multiply number by 10 raised to exponent.
     \param  num  Number to multiply
     \param  exp  Exponent to use
     \return      Result
    */
    static T exp10(T num, int exp) {
        T result = 0.0;
        if (exp == 0)
            result = num;
        else if (num != 0.0) {
            bool neg = false;
            if (exp < 0) {
                neg = true;
                exp = -exp;
            }
            T power = 10.0;
            result = 1.0;
            for (int bit = 1; exp != 0; bit <<= 1) {
                if (exp & bit) {
                    exp ^= bit;
                    result *= power;
                    if (exp == 0)
                        break;
                }
                power *= power;
            }
            result = (neg ? num / result : num * result);
        }
        return result;
    }

    /** Extract normalized base 10 mantissa and exponent from number.
     \param  exp  Stores exponent value. [out]
     \param  num  Number to use.
     \return      Mantissa value.
    */
    static T fexp10(int& exp, T num) {
        bool neg = false;
        if (num < 0.0)
            { neg = true; num = -num; }
        exp = 0;
        if (!nan(num) && !inf(num) && num != 0.0) {
            if (num >= 1.0) {
                static const int BIGNUM_DIGITS = std::numeric_limits<T>::digits10;
                static const T   BIGNUM        = evo_pow((T)10.0, BIGNUM_DIGITS);
                while (num >= BIGNUM)
                    { num /= BIGNUM;  exp += BIGNUM_DIGITS; }
                while (num >= 1000.0)
                    { num /= 1000.0;  exp += 3; }
                do {
                    num /= 10.0;  ++exp;
                } while (num >= 1.0);
            } else {
                while (num < 0.001 && num > 0.0)
                    { num *= 1000.0;  exp -= 3; }
                if (num > 0.0) {
                    if (num < 0.01)
                        { num *= 100.0;  exp -= 2; }
                    else if (num < 0.1)
                        { num *= 10.0; --exp; }
                }
            }
        }
        return (neg ? -num : num);
    }

    /** Constructor. */
    FloatT()
        { }
    /** Copy constructor.
     \param  val  Value to copy
    */
    FloatT(const FloatT<T>& val) : Nullable<T>(val)
        { }
    /** Constructor.
     \param  val  Value to set
    */
    FloatT(T val) : Nullable<T>(val)
        { }
    /** Assignment operator.
     \param  val  Value to copy
     \return      This
    */
    FloatT<T>& operator=(const FloatT<T>& val)
        { Nullable<T>::operator=(val); return *this; }
    /** Assignment operator.
     \param  val  Value to set
     \return      This
    */
    FloatT<T>& operator=(T val)
        { Nullable<T>::operator=(val); return *this; }
    /** Assignment operator to set as null.
     \return  This
    */
    FloatT<T>& operator=(ValNull)
        { Nullable<T>::set(); return *this; }

    // TODO eq1?
    /** Get whether approximately equal to given value.
     - This uses the system epsilon value for current type to determine whether the values are close enough to be considered equal.
     .
     \param  val  Value to compare to
     \return      Whether approximately equal
    */
    bool eq1(const Nullable<T>& val) {
        return ( (this->null() && val.null()) ||
                 (this->null() == val.null() && FloatT<T>::eq(this->value(), val.value())) );
    }

    /** Get whether approximately equal to given value.
     - This uses the system epsilon value for current type to determine whether the values are close enough to be considered equal.
     .
     \param  val  Value to compare to
     \return      Whether approximately equal
    */
    bool eq1(T val)
        { return (!this->null() && FloatT<T>::eq(this->value(), val)); }
};

/** Basic single-precision floating-point type (float) -- see FloatT. A basic Evo container for floating-point values.
\code
Float num = 12.3f;
\endcode
*/
typedef FloatT<float> Float;

/** Basic double-precision floating-point type (double) -- see FloatT. A basic Evo container for floating-point values.
\code
FloatD num = 12.3;
\endcode
*/
typedef FloatT<double> FloatD;

/** Basic long-double floating-point type (long double) -- see FloatT. A basic Evo container for floating-point values.
\code
FloatL num = 12.3L;
\endcode
*/
typedef FloatT<long double> FloatL;

///////////////////////////////////////////////////////////////////////////////

/** Base managed pointer.
 \tparam  T  Type to use pointer to (not raw pointer type)
*/
template<class T> struct PtrBase : public SafeBool<PtrBase<T> > {
    typedef PtrBase<T> BaseType;        ///< This pointer base type

    T* ptr_;            ///< Pointer

    /** Dereference operator (const).
     - Results are undefined if pointer is NULL
     .
     \return  Dereferenced pointer
    */
    const T& operator*() const {
        assert(ptr_ != NULL);
        return *ptr_;
    }

    /** Dereference operator (mutable).
     - Results are undefined if pointer is NULL
     .
     \return  Dereferenced pointer
    */
    T& operator*() {
        assert(ptr_ != NULL);
        return *ptr_;
    }

    /** Member access operator (const).
     - Results are undefined if pointer is NULL
     .
     \return  Pointer
    */
    const T* operator->() const {
        assert(ptr_ != NULL);
        return ptr_;
    }

    /** Member access operator (mutable).
     - Results are undefined if pointer is NULL
     .
     \return  Pointer
    */
    T* operator->() {
        assert(ptr_ != NULL);
        return ptr_;
    }

    /** Array access operator (const).
     - Results are undefined if pointer is NULL or index is out of bounds
     .
     \return  Item reference
    */
    const T& operator[](ulong index) const {
        assert(ptr_ != NULL);
        return ptr_[index];
    }

    /** Array access operator (mutable).
     - Results are undefined if pointer is NULL or index is out of bounds
     .
     \return  Item reference
    */
    T& operator[](ulong index) {
        assert(ptr_ != NULL);
        return ptr_[index];
    }

    /** Negation operator checks if NULL.
     \return  Whether NULL
    */
    bool operator!() const
        { return ptr_ == NULL; }

    /** Equality operator.
     \param  ptr  Pointer to compare to
     \return      Whether equal
    */
    bool operator==(const BaseType& ptr) const
        { return ptr_ == ptr.ptr_; }

    /** Equality operator.
     \param  ptr  Pointer to compare to
     \return      Whether equal
    */
    bool operator==(void* ptr) const
        { return ptr_ == ptr; }

    /** Inequality operator.
     \param  ptr  Pointer to compare to
     \return      Whether not equal
    */
    bool operator!=(const BaseType& ptr) const
        { return ptr_ != ptr.ptr_; }

    /** Inequality operator.
     \param  ptr  Pointer to compare to
     \return      Whether not equal
    */
    bool operator!=(void* ptr) const
        { return ptr_ != ptr; }

    /** Less-than operator.
     \param  ptr  Pointer to compare to
     \return      Whether less than ptr
    */
    bool operator<(const BaseType& ptr) const
        { return ptr_ < ptr.ptr_; }

    /** Less-than operator.
     \param  ptr  Pointer to compare to
     \return      Whether less than ptr
    */
    bool operator<(void* ptr) const
        { return ptr_ < ptr; }

    /** Less-than-or-equals operator.
     \param  ptr  Pointer to compare to
     \return      Whether less than or equal to ptr
    */
    bool operator<=(const BaseType& ptr) const
        { return ptr_ <= ptr.ptr_; }

    /** Less-than-or-equals operator.
     \param  ptr  Pointer to compare to
     \return      Whether less than or equal to ptr
    */
    bool operator<=(void* ptr) const
        { return ptr_ <= ptr; }

    /** Greater-than operator.
     \param  ptr  Pointer to compare to
     \return      Whether greater than ptr
    */
    bool operator>(const BaseType& ptr) const
        { return ptr_ > ptr.ptr_; }

    /** Greater-than operator.
     \param  ptr  Pointer to compare to
     \return      Whether greater than ptr
    */
    bool operator>(void* ptr) const
        { return ptr_ > ptr; }

    /** Greater-than-or-equals operator.
     \param  ptr  Pointer to compare to
     \return      Whether greater than or equal to ptr
    */
    bool operator>=(const BaseType& ptr) const
        { return ptr_ >= ptr.ptr_; }

    /** Greater-than-or-equals operator.
     \param  ptr  Pointer to compare to
     \return      Whether greater than or equal to ptr
    */
    bool operator>=(void* ptr) const
        { return ptr_ >= ptr; }

    /** Get whether pointer is null.
     \return  Whether null
    */
    bool null() const
        { return (ptr_ == NULL); }

    /** Get whether pointer is valid (not null).
     \return  Whether valid
    */
    bool valid() const
        { return (ptr_ != NULL); }

    /** Get current pointer (const).
     - Caution: This does not release ownership of the pointer
     .
     \return  Current pointer, or NULL if none
    */
    const T* ptr() const
        { return ptr_; }

    /** Get current pointer (mutable).
     - Caution: This does not release ownership of the pointer
     .
     \return  Current pointer, or NULL if none
    */
    T* ptr()
        { return ptr_; }
};

///////////////////////////////////////////////////////////////////////////////

/** Generic value conversion template.
 - This is specialized to define supported automatic value conversions between types
 - This is used internally, though additional conversions may be added by adding more specializations
 - See \ref StringCustomConversion "Custom String Conversion/Formatting"
 .
 \tparam  T  Conversion source value type
 \tparam  C  Conversion target type
*/
template<class T, class C> struct Convert {
    /** Set target to value (reversed conversion).
     \param  dest   Destination to set
     \param  value  Value to set
    */
    static void set(C& dest, T value)
        STATIC_ASSERT_FUNC_UNUSED               // Override required to support conversion

    /** Add/Append value to target (reversed conversion).
     \param  dest   Destination to add to
     \param  value  Value to add
    */
    static void add(C& dest, T value)
        STATIC_ASSERT_FUNC_UNUSED               // Override required to support conversion

    /** Add/Append value to target with quoting when needed (reversed conversion).
     - When applicable, strings are scanned to determine how to quote:
       - no quoting if delim not preset and doesn't start with any quotes (containing quotes is ok)
       - single-quoting ( ' ), double-quoting ( " ), backtick-quoting ( \` ): 'foo bar'
       - triple-quoting ( ''' or """ or \`\`\` ): '''foo bar'''
       - backtick-DEL (backtick then code 7F) quoting as a rare last resort ( \`␡ ): \`␡foo bar\`␡
       .
     .
     \param  dest   Destination to add to
     \param  value  Value to add
     \param  delim  Delimiter to consider for quoting
    */
    static void addq(C& dest, T value, typename C::Value delim)
        STATIC_ASSERT_FUNC_UNUSED               // Override required to support conversion

    /** %Convert value to target.
     \param  src  Source value to convert
     \return      Converted value
    */
    static const T& value(const T& src)
        STATIC_ASSERT_FUNC_UNUSED_RET(src)      // Override required to support conversion
};

///////////////////////////////////////////////////////////////////////////////

/* Key/Value pair type. */
template<class TKey, class TVal> struct KeyVal {
    typedef typename evo::KeyVal<TKey,TVal> Type;    ///< Key/value type
    typedef TKey Key;                                ///< Key type
    typedef TVal Val;                                ///< Value type

    TKey key;        ///< Key data
    TVal value;        ///< Value data

    /** Constructor. */
    KeyVal()
        { DataOp<Key>::defvalpod(key);  DataOp<Val>::defvalpod(value); }

    /** Constructor.
     \param  k  Key
     */
    KeyVal(const TKey& k) : key(k)
        { DataOp<Val>::defvalpod(value); }

    /** Constructor.
     \param  k  Key
     \param  v  Value
     */
    KeyVal(const TKey& k, const TVal& v) : key(k), value(v) { }

    /** Constructor.
     \param  src  Source to copy
     */
    KeyVal(const Type& src) : key(src.key), value(src.value) { }

    /** Assignment operator.
     \param  src  Source to copy
     \return      This
     */
    Type& operator=(const Type& src)
        { this->key = src.key; this->value = src.value; return *this; }

    /** Equality operator.
     \param  src  Source to compare
     \return      Whether equal
     */
    bool operator==(const Type& src) const
        { return key==src.key && value==src.value; }

    /** Inequality operator.
     \param  src  Source to compare
     \return      Whether inequal
     */
    bool operator!=(const Type& src) const
        { return key!=src.key || value!=src.value; }
};

///////////////////////////////////////////////////////////////////////////////

// Special type for NONE, ALL, END values -- used internally.
/** \cond impl */
struct EndT {
    operator unsigned char() const
        { return IntegerT<unsigned char>::max(); }
    operator unsigned short() const
        { return IntegerT<unsigned short>::max(); }
    operator unsigned int() const
        { return IntegerT<unsigned int>::max(); }
    operator unsigned long() const
        { return IntegerT<unsigned long>::max(); }
    operator unsigned long long() const
        { return IntegerT<unsigned long long>::max(); }

    EndT() { }
};

template<class T> bool operator==(T a, EndT)
    { return (a == IntegerT<T>::max()); }
template<class T> bool operator==(EndT, T b)
    { return (b == IntegerT<T>::max()); }

template<class T> bool operator!=(T a, EndT)
    { return (a != IntegerT<T>::max()); }
template<class T> bool operator!=(EndT, T b)
    { return (b != IntegerT<T>::max()); }
/** \endcond */

/** Special value for indicating no item or unknown item. Converts and compares with any Size type. */
static const EndT NONE;

/** Special value for indicating all items or all remaining items. Converts and compares with any Size type. */
static const EndT ALL;

/** Special value for indicating end of items. Converts and compares with any Size type. */
static const EndT END;

///////////////////////////////////////////////////////////////////////////////

/** Special pointer value for default initialization (used in containers). */
#define EVO_PDEFAULT ((T*)IntegerT<std::size_t>::max())

/** Special pointer value for empty but not NULL (used in containers). */
#define EVO_PEMPTY ((T*)1)

/** Special pointer value for empty but not NULL (used in containers). */
#define EVO_PPEMPTY ((T**)1)

///////////////////////////////////////////////////////////////////////////////

// TBitArray
// TODO

// Implementation
/** \cond impl */
namespace impl
{
    template<class T>
    struct BitInfoT {
        static const int bytes        = sizeof(T);
        static const int bits         = bytes * 8;
        static const int bits_minus_1 = bits - 1;
        static const T   rbit         = 0x01;
        static const T   lbit         = rbit << bits_minus_1;
    };

    struct BitInfo {
        static const int           bits         = 8;
        static const int           bits_minus_1 = bits - 1;
        static const unsigned char rbit         = 0x01;
        static const unsigned char lbit         = rbit << bits_minus_1;

        static unsigned char maskM(register int start, register int count) {
            if (count <= 0)
                return 0;
            else if (start <= 0)
                return -(lbit >> (count-1));
            else
                return ( ~-(lbit >> (start-1)) & -(lbit >> (start+count-1)) );
        }

        template<class T>
        static T getVal(T& data, register unsigned char* ptr, register int offset, register int count) {
            data = 0;
            if (count > 0) {
                ptr = &(ptr[offset / bits]);
                offset = offset % bits;
                if (offset + count <= bits)
                    // Copy from single byte
                    data = (*ptr & maskM(offset, count)) >> (bits - (offset+count));
                else {
                    // Copy from more than one byte
                    if (count > BitInfoT<T>::bits)
                        count = BitInfoT<T>::bits;
                    // Copy first partial byte
                    if (offset > 0) {
                        data |= ( (*ptr & ~-(lbit >> (offset-1))) << (count-offset) );
                        count -= (bits - offset);
                        ++ptr;
                    }
                    // Copy whole bytes
                    while (count >= bits) {
                        count -= bits;
                        data |= (*ptr << count);
                        ++ptr;
                    }
                    // Copy end partial byte
                    if (count > 0)
                        data |= ( (*ptr & -(lbit >> (count-1))) >> (bits-count) );
                }
            }
            return data;
        }
    };
}

// TODO - make class with static inline funcs

// BIT_ARRAY_SIZE()
/** Get size of array for number of bits.
 \param  numbits  Number of bits.
 \return          List size in bytes.
*/
#define BIT_ARRAY_SIZE(numbits) ( ((numbits)+::evo::impl::BitInfo::bits_minus_1) / ::evo::impl::BitInfo::bits )

// BIT_ARRAY_INDEX()
/** Get byte index for bit offset.
 \param  offset  Bit offset.
 \return         List byte index.
*/
#define BIT_ARRAY_INDEX(offset) ( ((int)offset) / ::evo::impl::BitInfo::bits )

// BIT_ARRAY_MASK()
/** Get mask with given bit set.
 - Offset is automatically scaled down to byte offset.
 - Offset begins at highest-order bit (0x80).
.
 \param  offset  Bit offset.
 \return         Mask with given bit set.
*/
#define BIT_ARRAY_MASK(offset) ( ::evo::impl::BitInfo::lbit >> ((offset) % ::evo::impl::BitInfo::bits) )

// BIT_ARRAY_MASK_L()
/** Get mask with beginning (highest order) bits set.
 \param  count  Number of bits to set.
 \return        Bit mask with given bits set.
*/
#define BIT_ARRAY_MASK_L(count) (unsigned char)(-(::evo::impl::BitInfo::lbit >> ((count)-1)))

// BIT_ARRAY_MASK_R()
/** Get mask with ending (lowest order) bits set.
 \param  count  Number of bits to set.
 \return        Bit mask with given bits set.
*/
#define BIT_ARRAY_MASK_R(count) (unsigned char)(~-(::evo::impl::BitInfo::rbit << (count)))

// BIT_ARRAY_MASK_M()
/** Get mask with given bits set.
 \param  offset  Start bit offset (from left).
 \param  count   Number of bits to set from start.
 \return         Bit mask with given bits set.
*/
#define BIT_ARRAY_MASK_M(offset,count) ::evo::impl::BitInfo::maskM(offset, count)

// BIT_ARRAY_BYTE()
/** Get byte value at index.
 \param  ptr    Pointer to bit array.
 \param  index  Byte index.
 \return        Byte value.
*/
#define BIT_ARRAY_BYTE(ptr,index) ( ((unsigned char*)ptr)[index] )

// BIT_ARRAY_CHK()
/** Get bit at offset.
 - Offset is mapped to byte index and byte-level offset.
 - Byte offset begins at highest-order bit (0x80).
.
 \param  ptr     Pointer to bit array.
 \param  offset  Bit offset.
 \return         Bit value.
*/
#define BIT_ARRAY_CHK(ptr,offset) ( ((unsigned char*)ptr)[BIT_ARRAY_INDEX(offset)] & BIT_ARRAY_MASK(offset) )

// BIT_ARRAY_GET()
/** Get given bits as a number.
 - Extracts given bits as a number.
 - Offset is mapped to byte index and byte-level offset.
 - Byte offset begins at highest-order bit (0x80).
.
 \param  data   Set to result data (must be basic numeric type). [out]
 \param  ptr    Pointer to bit array.
 \param  start  Bit start offset.
 \param  count  Bit count.
 \return        Data value.
*/
#define BIT_ARRAY_GET(data,ptr,start,count) ::evo::impl::BitInfo::getVal(data, ptr, start, count)

// BIT_ARRAY_SET()
/** Set bit at offset.
 - Offset is mapped to byte index and byte-level offset.
 - Byte offset begins at highest-order bit (0x80).
.
 \param  ptr     Pointer to bit array.
 \param  offset  Bit offset.
*/
#define BIT_ARRAY_SET(ptr,offset) ( ((unsigned char*)ptr)[BIT_ARRAY_INDEX(offset)] |= BIT_ARRAY_MASK(offset) )

// BIT_ARRAY_CLR()
/** Clear bit at offset.
 - Offset is mapped to byte index and byte-level offset.
 - Byte offset begins at highest-order bit (0x80).
.
 \param  ptr     Pointer to bit array.
 \param  offset  Bit offset.
*/
#define BIT_ARRAY_CLR(ptr,offset) ( ((unsigned char*)ptr)[BIT_ARRAY_INDEX(offset)] &= ~BIT_ARRAY_MASK(offset) )

// BIT_ARRAY_SET_ALL()
/** Set all bits.
 - Bits are set at byte level so this may change padding bits at end.
.
 \param  ptr      Pointer to bit array.
 \param  numbits  Number of bits to set.
*/
#define BIT_ARRAY_SET_ALL(ptr,numbits) memset(ptr, 0xFF, BIT_ARRAY_SIZE(numbits))

// BIT_ARRAY_CLR_ALL()
/** Clear all bits.
 - Bits are set at byte level so this may change padding bits at end.
.
 \param  ptr      Pointer to bit array.
 \param  numbits  Number of bits to clear.
*/
#define BIT_ARRAY_CLR_ALL(ptr,numbits) memset(ptr, 0, BIT_ARRAY_SIZE(numbits))

/** \endcond */

///////////////////////////////////////////////////////////////////////////////
//@}
} // Namespace: evo
#endif
