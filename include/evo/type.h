// Evo C++ Library
/* Copyright 2018 Justin Crowell
Distributed under the BSD 2-Clause License -- see included file LICENSE.txt for details.
*/
///////////////////////////////////////////////////////////////////////////////
/** \file type.h Evo basic types and traits. */
#pragma once
#ifndef INCL_evo_type_h
#define INCL_evo_type_h

#include "impl/container.h"

namespace evo {
/** \addtogroup EvoCore */
//@{

///////////////////////////////////////////////////////////////////////////////

/** Macro returning lowest argument.
 \param  A  First number
 \param  B  Second number
 \return    Max number
*/
#define EVO_MIN(A, B) ((B) < (A) ? (B) : (A))

/** Macro returning highest argument.
 \param  A  First number
 \param  B  Second number
 \return    Max number
*/
#define EVO_MAX(A, B) ((B) > (A) ? (B) : (A))

///////////////////////////////////////////////////////////////////////////////

/** Safe bool base class.
This implements "safe" boolean evaluation, without the side-effects of implicit conversions from the bool type.
 - Use by deriving the intended class from SafeBool
 - The deriving class must implement the logical negation operator (as const) for boolean evaluation
   \code
    bool operator!() const
   \endcode
 - Safe bool comparisons (\c obj1 \c == \c obj2 or \c obj1 \c != \c obj1 \c ) will not compile unless the deriving class implements operator==() or operator!=(), respectively
 - No virtual methods are used even though this is a base class
 .
 \tparam  T  The deriving type being used.

\par Example:

\code
#include <evo/type.h>

// Class supporting "safe bool evaluation"
struct Foo : public evo::SafeBool<Foo> {
    bool operator!() const
        { return false; }
};

void func() {
    const Foo foo;

    // Safe bool evaluation -- calls !operator!()
    if (foo) {
        // ...
    }

    // Negative bool evaluation -- calls operator!()
    if (!foo) {
        // ...
    }
}
\endcode
*/
template<class T> class SafeBool {
protected:
    /** \cond impl */
    typedef void (SafeBool::*SafeBoolType)() const;
    void This_type_does_not_support_this_comparison() const { }
    /** \endcond */

public:
    /** Constructor. */
    SafeBool() { }
    /** \cond impl */
    SafeBool(const SafeBool&) { }
    /** \endcond */

    /** Safe (explicit) evaluation as bool type.
     - This is called when object is directly evaluated as a bool, and is equivalent to: !operator!()
     - See \link SafeBool\endlink
     .
    */
    operator SafeBoolType() const
        { return (!(static_cast<const T*>(this))->operator!() ? &SafeBool::This_type_does_not_support_this_comparison : 0); }
};

/** \cond impl */
template<typename T, typename U> bool operator==(const SafeBool<T>& l, const SafeBool<U>& r)
    { l.This_type_does_not_support_this_comparison(); return false; }
template<typename T, typename U> bool operator!=(const SafeBool<T>& l, const SafeBool<U>& r)
    { l.This_type_does_not_support_this_comparison(); return false; }
/** \endcond */

/** \cond impl */
namespace impl {
    struct SafeBoolTestCov : SafeBool<SafeBoolTestCov> {
        void test()
            { This_type_does_not_support_this_comparison(); }
    };
}
/** \endcond */

///////////////////////////////////////////////////////////////////////////////

/** %Nullable primitive base type.
 - This is normally not used directly, see derived types or below primitives using this
 - See: Bool, Char, \link Int\endlink, \link Long\endlink, \link UInt\endlink, \link ULong\endlink, \link Float\endlink, \link FloatD\endlink, \link FloatL\endlink
*/
template<class T>
class Nullable : public SafeBool<Nullable<T> > {
public:
    /** Constructor. */
    Nullable() : value_((T)0), null_(true)
        { }

    /** Copy constructor.
     \param  src  Source to copy
    */
    Nullable(const Nullable<T>& src) : value_(src.value_), null_(src.null_)
        { }

    /** Constructor to init with num.
     \param  num  Number to use
    */
    Nullable(T num) : value_(num), null_(false)
        { }

    /** Assignment/Copy operator.
     \param  src  Source to copy
     \return      This
    */
    Nullable<T>& operator=(const Nullable<T>& src)
        { null_ = src.null_; value_ = src.value_; return *this; }

    /** Constructor to init with value.
     \param  value  Number to use
     \return        This
    */
    Nullable<T>& operator=(T value)
        { null_ = false; value_ = value; return *this; }

    /** Assignment operator to set as null.
     - Use param vNULL to set as null
     .
     \return  This
    */
    Nullable<T>& operator=(ValNull)
        { null_ = true; value_ = (T)0; return *this; }

    /** Negation operator returns whether null or 0.
     \return  Whether null or 0
    */
    bool operator!() const
        { return (null_ || value_ == (T)0); }

    /** Equality operator.
     \param  val  Value to compare to
     \return      Whether equal
    */
    bool operator==(const Nullable<T>& val) const
        { return (null_ ? val.null_ : !val.null_ && value_ == val.value_); }

    /** Equality operator.
     - If null, the result is always false as null is not equal to any non-null value
     .
     \param  val  Value to compare to
     \return      Whether equal, false if null
    */
    bool operator==(T val) const
        { return (null_ ? false : value_ == val); }

    /** Inequality operator.
     \param  val  Value to compare to
     \return      Whether inequal
    */
    bool operator!=(const Nullable<T>& val) const
        { return (null_ ? !val.null_ : val.null_ || value_ != val.value_); }

    /** Inequality operator.
     - If null, the result is always true as null is not equal to any non-null value
     .
     \param  val  Value to compare to
     \return      Whether inequal, true if null
    */
    bool operator!=(T val) const
        { return (null_ ? true : value_ != val); }

    /** Less than operator.
     - If null, null is less than any non-null value
     .
     \param  val  Value to compare to
     \return      Whether less than `val`
    */
    bool operator<(const Nullable<T>& val) const
        { return (null_ ? !val.null_ : value_ < val.value_); }

    /** Less than operator.
     - If null, null is less than any non-null value
     .
     \param  val  Value to compare to
     \return      Whether less than `val`
    */
    bool operator<(T val) const
        { return (null_ ? true : value_ < val); }

    /** Less than or equal operator.
     - If null, null is less than or equal to any value
     .
     \param  val  Value to compare to
     \return      Whether less than or equal to `val`, true if null
    */
    bool operator<=(const Nullable<T>& val) const
        { return (null_ ? true : (val.null_ ? false : value_ <= val.value_)); }

    /** Less than or equal operator.
     - If null, null is less than or equal to any value
     .
     \param  val  Value to compare to
     \return      Whether less than or equal to `val`, true if null
    */
    bool operator<=(T val) const
        { return (null_ ? true : value_ <= val); }

    /** Greater than operator.
     - If null, null is not greater than any non-null value
     .
     \param  val  Value to compare to
     \return      Whether greater than `val`, false if null
    */
    bool operator>(const Nullable<T>& val) const
        { return (null_ ? false : (val.null_ ? true : value_ > val.value_)); }

    /** Greater than operator.
     - If null, null is not greater than any non-null value
     .
     \param  val  Value to compare to
     \return      Whether greater than `val`, false if null
    */
    bool operator>(T val) const
        { return (null_ ? false : value_ > val); }

    /** Greater than or equal operator.
     - If null, null is not greater than any non-null value
     .
     \param  val  Value to compare to
     \return      Whether greater than or equal to `val`
    */
    bool operator>=(const Nullable<T>& val) const
        { return (null_ ? val.null_ : (val.null_ ? true : value_ >= val.value_)); }

    /** Greater than or equal operator.
     - If null, null is not greater than any non-null value
     .
     \param  val  Value to compare to
     \return      Whether greater than or equal to `val`
    */
    bool operator>=(T val) const
        { return (null_ ? false : value_ >= val); }

    /** Comparison.
     \param  val  Value to compare to
     \return      Result (<0 if this is less, 0 if equal, >0 if this is greater)
    */
    int compare(const Nullable<T>& val) const {
        if (null_) {
            if (val.null_)
                return 0;
            return -1;
        } else if (val.null_)
            return 1;
        return (value_ == val.value_ ? 0 : (value_ < val.value_ ? -1 : 1));
    }

    /** Comparison.
     - If null, the result is always -1 as null compares as less than any non-null value
     .
     \param  val  Value to compare to
     \return      Result (<0 if this is less, 0 if equal, >0 if this is greater)
    */
    int compare(T val) const {
        if (null_)
            return -1;
        return (value_ == val ? 0 : (value_ < val ? -1 : 1));
    }

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
    const T& value() const
        { return value_; }

    /** %Set as null.
     \return  This
    */
    Nullable<T>& set()
        { null_ = true; value_ = (T)0; return *this; }

    /** %Set as copy of given value.
     \param  src  Source to copy
     \return      This
    */
    Nullable<T>& set(const Nullable<T>& src)
        { null_ = src.null_; value_ = src.value_; return *this; }

    /** %Set as given value.
     \param  src  Source to set
     \return      This
    */
    Nullable<T>& set(T src)
        { null_ = false; value_ = src; return *this; }

    /** Clears null flag and returns value reference.
     - Value is unchanged, POD types default to 0 when null
     - Useful for updating the underlying value by reference
     - Note: `Nullable<bool>` template specialization returns by value instead of reference
     .
     \return  Whether valid
    */
    T& denull() {
        if (null_)
            null_ = false;
        return value_;
    }

private:
    T    value_;
    bool null_;
};

// Specialized for minimum size
/** \cond impl */
template<> class Nullable<bool> : public SafeBool<Nullable<bool> > {
public:
    Nullable()
        { value_ = (uchar)nvNULL; }
    Nullable(const Nullable<bool>& src)
        { value_ = src.value_; }
    Nullable(bool val)
        { value_ = (uchar)(val ? nvTRUE : nvFALSE); }
    Nullable<bool>& operator=(const Nullable<bool>& src)
        { value_ = src.value_; return *this; }
    Nullable<bool>& operator=(bool val)
        { value_ = (uchar)(val ? nvTRUE : nvFALSE); return *this; }
    Nullable<bool>& operator=(ValNull)
        { value_ = (uchar)nvNULL; return *this; }

    bool operator!() const
        { return (value_ != (uchar)nvTRUE); }
    bool operator==(const Nullable<bool>& val) const
        { return (value_ == val.value_); }
    bool operator==(bool val) const
        { return (value_ != (uchar)nvNULL && val == (value_ == (uchar)nvTRUE)); }
    bool operator!=(const Nullable<bool>& val) const
        { return (value_ != val.value_); }
    bool operator!=(bool val) const
        { return (value_ == (uchar)nvNULL || val != (value_ == (uchar)nvTRUE)); }

    bool operator<(const Nullable<bool>& val) const
        { return (value_ < val.value_); }
    bool operator<(bool val) const
        { return (value_ < (uchar)(val ? nvTRUE : nvFALSE)); }
    bool operator<=(const Nullable<bool>& val) const
        { return (value_ <= val.value_); }
    bool operator<=(bool val) const
        { return (value_ <= (uchar)(val ? nvTRUE : nvFALSE)); }
    bool operator>(const Nullable<bool>& val) const
        { return (value_ > val.value_); }
    bool operator>(bool val) const
        { return (value_ > (uchar)(val ? nvTRUE : nvFALSE)); }
    bool operator>=(const Nullable<bool>& val) const
        { return (value_ >= val.value_); }
    bool operator>=(bool val) const
        { return (value_ >= (uchar)(val ? nvTRUE : nvFALSE)); }

    int compare(const Nullable<bool>& val) const
        { return (value_ == val.value_ ? 0 : (value_ < val.value_ ? -1 : 1)); }

    const bool operator*() const
        { return (value_ == (uchar)nvTRUE); }

    bool null() const
        { return (value_ == (uchar)nvNULL); }
    bool valid() const
        { return (value_ != (uchar)nvNULL); }
    const bool value() const
        { return (value_ == (uchar)nvTRUE); }

    Nullable<bool>& set()
        { value_ = (uchar)nvNULL; return *this; }
    Nullable<bool>& set(const Nullable<bool>& src)
        { value_ = src.value_; return *this; }
    Nullable<bool>& set(bool val)
        { value_ = (uchar)(val ? nvTRUE : nvFALSE); return *this; }

    bool denull() {
        if (value_ == nvNULL) {
            value_ = nvFALSE;
            return false;
        }
        return (value_ == nvTRUE);
    }

private:
    enum NullableValue {
        nvNULL=0,
        nvFALSE,
        nvTRUE
    };

    uchar value_;
};
/** \endcond */

/** Equality operator comparing value with Nullable value.
 - If val2 is null, the result is always false as null is not equal to any non-null value
 .
 \param  val1  Value to compare
 \param  val2  Value to compare to
 \return       Whether equal, false if null
*/
template<class T> inline bool operator==(T val1, const Nullable<T>& val2)
    { return val2 == val1; }

/** Inequality operator comparing value with Nullable value.
 - If val2 is null, the result is always true as null is not equal to any non-null value
 .
 \param  val1  Value to compare
 \param  val2  Value to compare to
 \return       Whether inequal, true if null
*/
template<class T> inline bool operator!=(T val1, const Nullable<T>& val2)
    { return val2 != val1; }

/** Less than operator comparing value with Nullable value.
 - If val2 is null, no non-null value is less than null
 .
 \param  val1  Value to compare
 \param  val2  Value to compare to
 \return       Whether `val1` is less than `val2`, false if `val2` is null
*/
template<class T> inline bool operator<(T val1, const Nullable<T>& val2)
    { return val2 > val1; }

/** Less than or equal operator comparing value with Nullable value.
 - If val2 is null, no non-null value is less than null
 .
 \param  val1  Value to compare
 \param  val2  Value to compare to
 \return       Whether `val1` is less than or equal to `val2`, false if `val2` is null
*/
template<class T> inline bool operator<=(T val1, const Nullable<T>& val2)
    { return val2 >= val1; }

/** Greater than operator comparing value with Nullable value.
 - If val2 is null, any non-null value is greater than null
 .
 \param  val1  Value to compare
 \param  val2  Value to compare to
 \return       Whether `val1` is greater than `val2`, true if `val2` is null
*/
template<class T> inline bool operator>(T val1, const Nullable<T>& val2)
    { return val2 < val1; }

/** Greater than or equal operator comparing value with Nullable value.
 - If val2 is null, any non-null value is greater than null
 .
 \param  val1  Value to compare
 \param  val2  Value to compare to
 \return       Whether `val1` is greater or equal to `val2`, true if `val2` is null
*/
template<class T> inline bool operator>=(T val1, const Nullable<T>& val2)
    { return val2 <= val1; }

///////////////////////////////////////////////////////////////////////////////

/** Basic boolean type. A basic Evo container for boolean values.
 - Assigned like a bool:
   \code
    Bool val(true);         // Set to true
    val = false;            // Set to false
   \endcode
 - Can hold null value:
   \code
    Bool val;               // Null by default
    val = false;            // Set to false, no longer null
    val.set();;             // Set to null
   \endcode
 - Evaluate as bool to check if non-null and non-false:
   \code
    Bool val;               // Null by default
    if (val) {              // false
    }
    if (!val) {             // true
    }
    val = false;
    if (val) {              // false
    }
    val = true;
    if (val) {              // true
    }
   \endcode
 - Dereference to get raw value:
   \code
    Bool val1;              // Null by default
    if (*val1) {            // Dereference for raw value -- false when null
    }
   \endcode
 - Comparison operator overloads:
   \code
    Bool val;               // Null by default
    if (*val == false) {    // true: raw value is false
    }
    if (val == false) {     // false: null is not false
    }
    if (val < false) {      // true: null is less than any value
    }
    if (*val < false) {     // false: raw value is false
    }
   \endcode
*/
struct Bool : public Nullable<bool> {
    typedef bool Type;                                  ///< Wrapped type (bool)

    static const int BYTES = sizeof(bool);              ///< Type size in bytes, usually 1
    static const int BITS  = 1;                         ///< Type size in bits (1)

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
 \tparam  T  Character type (char)

Character types:
 - \link Char\endlink

Examples:

 - Assigned like a char:
   \code
    Char val('A');          // Set to 'A'
    val = 'b';              // Set to 'b'
   \endcode
 - Can hold null value:
   \code
    Char val;               // Null by default
    val = 'B';              // Set to 'B', no longer null
    val.set();              // Set to null
   \endcode
 - Evaluate as bool to check non-null and non-zero:
   \code
    Char val;               // Null by default
    if (val) {              // false
    }
    if (!val) {             // true
    }
    val = '\0';
    if (val) {              // false
    }
    val = 'A';
    if (val) {              // true
    }
   \endcode
 - Use denull() to clear null flag and update value via reference -- useful for decrement/increment:
   \code
    Char val;               // Null by default
    ++val.denull();         // Remove null flag (value is 0) and increment so value is now 1
   \endcode
 - Dereference to get raw value:
   \code
    Char val;               // Null by default
    if (*val == 'A') {      // Dereference for raw value -- false: 0 when null
    }
   \endcode
 - Comparison operator overloads:
   \code
    Char val;               // Null by default, with raw value 0
    if (*val == '\0') {     // true: raw value is 0
    }
    if (val == '\0') {      // false: null is not 0
    }
    if (val < '\0') {       // true: null is less than any value
    }
    if (*val < '\0') {      // false: raw value is 0
    }
   \endcode
 - Some traits and helpers
   \code
    int num_bits = Char::BITS;                   // get number of bits per char (8 bits)
    if (Char::category('1') > Char::cALPHANUM) { // check character category (alpha-numeric) -- true
    }
    if (Char::isupper('A') {                     // check if upper-case character (static helper) -- true
    }
   \endcode
*/
template<class T>
struct CharT : public Nullable<T> {
    typedef CharT<T> This;          ///< %This non-POD type
    typedef T        Type;          ///< Wrapped POD type

    static const int BYTES = sizeof(T);                ///< Type size in bytes, usually 1
    static const int BITS  = BYTES * CHAR_BIT;         ///< Type size in bits (8)

    /** Character category.
     - Overlapping categories (ex: alphanumeric) can be checked with greater-than (>) comparison:
       \code
        Char::Category cat = Char::category(ch);
        bool space     = (cat == Char::cSPACE);     // Whitespace (space, tab)
        bool symbol    = (cat == Char::cSYMBOL);    // Symbols (comma, precent, dollar, etc)
        bool digit     = (cat == Char::cDIGIT);     // Decimal digit (0-9)
        bool uppercase = (cat == Char::cALPHA_U);   // Alphabet uppercase (A-Z)
        bool lowercase = (cat == Char::cALPHA_L);   // Alphabet lowercase (a-z)
        bool alphanum  = (cat > Char::cALPHANUM);   // Alphanumeric (0-9, A-Z, a-z)
        bool alpha     = (cat > Char::cALPHA);      // Alphabet (A-Z, a-z)
        bool printable = (cat > Char::cNONE);       // Any printable character
        bool visible   = (cat > Char::cSPACE);      // Any visible character (printable and not whitespace)
       \endcode
     .
    */
    enum Category {
        cNONE=0,            ///< Non printable char
        cSPACE,             ///< Whitespace (space, tab)
        cSYMBOL,            ///< Symbol character (printable but not alphanumeric)
        cALPHANUM,          ///< Alpha-numeric -- categories greater than this are alphanumeric
        cDIGIT,             ///< Decimal digit (0-9)
        cALPHA,             ///< Alphabet -- categories greater than this are alphabetic
        cALPHA_U,           ///< Alphabet uppercase (A-Z)
        cALPHA_L            ///< Alphabet lowercase (a-z)
    };

    /** Character digit type.
     - Digit types overlap so check with greater-than-or-equal (>=) comparison:
       \code
        Char::Digit dig = Char::digit(ch);
        bool b36 = (dig >= Char::dBASE36);  // base 36 digit (0-9, A-Z)
        bool hex = (dig >= Char::dHEX);     // hex digit (0-9, A-F)
        bool dec = (dig >= Char::dDECIMAL); // decimal digit (0-9)
        bool oct = (dig >= Char::dOCTAL);   // octal digit (0-7)
        bool err = (dig == Char::dHEX);     // error: must use operator >= here
       \endcode
     .
    */
    enum Digit {
        dNONE=0,            ///< Not a digit
        dBASE36,            ///< Base 36 character
        dHEX,               ///< Hexadecimal character
        dDECIMAL,           ///< Decimal character
        dOCTAL              ///< Octal character
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

    /** Assignment operator to set as null by passing \ref vNULL.
     \return  This
    */
    CharT<T>& operator=(ValNull)
        { Nullable<T>::set(); return *this; }
};

/** Basic character type (char) -- see CharT. A basic Evo container for single-byte character values.
\code
Char ch('A');
\endcode
*/
struct Char : public CharT<char> {
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
            7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 2, 2, 2, 2, 0             // 112-127
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
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0             // 112-127
        };
        return (Digit)((uchar)ch < 128 ? chmap[(uint)ch] : 0);
    }

    /** Check whether whitespace character (space, tab, newline, carrige return).
     \param  ch  Character to check
     \return     Whether whitespace
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
    Char(const Char& val) : CharT<char>(val)
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
    Char& operator=(const Char& val)
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

/** \cond impl */
namespace impl {
    template<int IntSize> struct IntMaxLen { };
    // Max string length by int size in bytes, including either sign or hex/oct prefix (0x/0), but not both
    template<> struct IntMaxLen<1> { static const int value = 4; };
    template<> struct IntMaxLen<2> { static const int value = 7; };
    template<> struct IntMaxLen<4> { static const int value = 12; };
    template<> struct IntMaxLen<8> { static const int value = 23; };
}
/** \endcond */

///////////////////////////////////////////////////////////////////////////////

// Disable constant truncation MSVC warnings for constants using bit manipulation
#if defined(_MSC_VER)
    #pragma warning(push)
    #pragma warning(disable:4309)
#endif

/** Basic integer type. A basic Evo container template for integer types.
 \tparam  T  Integer plain type (short, int, long, longl, ushort, uint, ulong, ulongl, etc)

Integer types:
 - \link Short\endlink, \link Int\endlink, \link Long\endlink, \link LongL\endlink
 - \link UShort\endlink, \link UInt\endlink, \link ULong\endlink, \link ULongL\endlink

Examples -- using Int type (and int type) here, other supported integer types work the same:

 - Assigned like an integer:
   \code
    Int val(123);               // Set to 123
    val = 12;                   // Set to 12
   \endcode
 - Can hold null value:
   \code
    Int val;                    // Null by default
    val = 567;                  // Set to 567, no longer null
    val.set();                  // Set to null
   \endcode
 - Evaluate as bool to check non-null and non-zero:
   \code
    Int val;                    // Null by default
    if (val) {                  // false
    }
    if (!val) {                 // true
    }
    val = 0;
    if (val) {                  // false
    }
    val = 1;
    if (val) {                  // true
    }
   \endcode
 - Use denull() to clear null flag and update value via reference -- useful for decrement/increment:
   \code
    Int val;                    // Null by default
    ++val.denull();             // Remove null flag (value is 0) and increment so value is now 1
   \endcode
 - Dereference to get raw value:
   \code
    Int val;                    // Null by default -- null converts to int as 0
    if (*val == 123) {          // Dereference for raw value -- false: 0 when null
    }
   \endcode
 - Comparison operator overloads:
   \code
    Int val;                    // Null by default, with raw value 0
    if (*val == 0) {            // true: raw value is 0
    }
    if (val == 0) {             // false: null is not 0
    }
    if (val < 0) {              // true: null is less than any value
    }
    if (*val < 0) {             // false: raw value is 0
    }
   \endcode
 - Some traits and helpers:
   \code
    int num_bits = Int::BITS;        // get number of bits per int (answer depends on int type size, often 32 bits)
    int digits = Int::digits(12345); // get number of digits in number 12345 (answer is 5) (static helper)
   \endcode
*/
template<class T>
struct IntegerT : public Nullable<T> {
    typedef IntegerT<T> This;       ///< %This non-POD type
    typedef T           Type;       ///< Wrapped POD type

    static const bool SIGN      = IsSigned<T>::value;                   ///< Whether type is signed
    static const int  BYTES     = sizeof(T);                            ///< Type size in bytes
    static const int  BITS      = sizeof(T) * 8;                        ///< Type size in bits
    static const int  MAXSTRLEN = impl::IntMaxLen<sizeof(T)>::value;    ///< Max formatted length, including either sign or hex/octal prefix (0x/0), but not both

    static const int  BITS_MINUS_1 = BITS - 1;                          ///< BITS minus 1, used by bit manipulation code
    static const T    RBIT         = 0x01;                              ///< Mask with right-most (least significant) bit set
    static const T    LBIT         = RBIT << BITS_MINUS_1;              ///< Mask with left-most (most significant) bit set
    static const T    ZERO         = 0;                                 ///< Mask with all bits cleared (no bits set)
    static const T    ALLBITS      = T(~ZERO);                          ///< Mask with all bits set

    static const T    MIN = (SIGN ? LBIT : 0);                          ///< Minimum integer value
    static const T    MAX = (SIGN ? ~LBIT : ALLBITS);                   ///< Maximum interger value

    /** Get maximum formatted length for type at base.
     - Includes either sign (if signed) or hex/octal prefix (if applicable), but not both
     .
     \param  base  Number base to use
     \return       Max number length for given base
    */
    static int maxlen(int base=10) {
        // Note: Newer compilers (gcc 4.9.2) give c++11 'narrowing conversion' warnings if 'values' are smaller than int, casting doesn't fix
        if (base >= 100)
            base -= 100;
        assert( base > 1 );
        assert( base <= 36 );
        static const int n = (SIGN ? 1 : 0);
        static const int VALUES[] = {
            digits(MAX,2)+n, digits(MAX,3)+n, digits(MAX,4)+n, digits(MAX,5)+n, digits(MAX,6)+n,
            digits(MAX,7)+n, digits(MAX,8)+1, digits(MAX,9)+n, digits(MAX,10)+n, digits(MAX,11)+n,
            digits(MAX,12)+n, digits(MAX,13)+n, digits(MAX,14)+n, digits(MAX,15)+n, digits(MAX,16)+n+1,
            digits(MAX,17)+n, digits(MAX,18)+n, digits(MAX,19)+n, digits(MAX,20)+n, digits(MAX,21)+n,
            digits(MAX,22)+n, digits(MAX,23)+n, digits(MAX,24)+n, digits(MAX,25)+n, digits(MAX,26)+n,
            digits(MAX,27)+n, digits(MAX,28)+n, digits(MAX,29)+n, digits(MAX,30)+n, digits(MAX,31)+n,
            digits(MAX,32)+n, digits(MAX,33)+n, digits(MAX,34)+n, digits(MAX,35)+n, digits(MAX,36)+n
        };
        return VALUES[base-2];
    }

    /** Get number of digits for given number and base. Includes sign if negative.
     \param  num   Number to use
     \param  base  Number base to use
     \return       Number of digits for given number and base
    */
    static int digits(T num, int base=10) {
        if (base >= 100)
            base -= 100;
        assert( base > 1 );
        assert( base < 100 );
        int result = 0;
        if (num == 0)
            result = 1;
        else {
            if (num < 0)
                ++result;
            while (num != 0) {
                num /= (int8)base;
                ++result; 
            }
        }
        return result;
    }

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

    /** Assignment operator to set as null by passing \ref vNULL.
     \return  This
    */
    IntegerT<T>& operator=(ValNull)
        { Nullable<T>::set(); return *this; }

    using Nullable<T>::value;

    /** Get underlying value or given default if null.
     \param  defval  Default value to use if null
     \return         Result value, or defval if null
    */
    T value(T defval) const
        { return (Nullable<T>::null() ? defval : Nullable<T>::value()); }
};

#if defined(_MSC_VER)
    #pragma warning(pop)
#endif

/** Basic integer type (short) -- see IntegerT. A basic Evo container for integer values.
\code
Short num(123);
\endcode
*/
typedef IntegerT<short> Short;

/** Basic integer type (int) -- see IntegerT. A basic Evo container for integer values.
\code
Int num(123);
\endcode
*/
typedef IntegerT<int> Int;

/** Basic integer type (long) -- see IntegerT. A basic Evo container for integer values.
\code
Long num(123);
\endcode
*/
typedef IntegerT<long> Long;

/** Basic integer type (long long) -- see IntegerT. A basic Evo container for integer values.
\code
LongL num(123);
\endcode
*/
typedef IntegerT<longl> LongL;

/** Basic integer type (int8) -- see IntegerT. A basic Evo container for integer values.
\code
Int8 num(123);
\endcode
*/
typedef IntegerT<int8> Int8;

/** Basic integer type (int16) -- see IntegerT. A basic Evo container for integer values.
\code
Int16 num(123);
\endcode
*/
typedef IntegerT<int16> Int16;

/** Basic integer type (int32) -- see IntegerT. A basic Evo container for integer values.
\code
Int32 num(123);
\endcode
*/
typedef IntegerT<int32> Int32;

/** Basic integer type (int64) -- see IntegerT. A basic Evo container for integer values.
\code
Int64 num(123);
\endcode
*/
typedef IntegerT<int64> Int64;

/** Basic integer type (unsigned short) -- see IntegerT. A basic Evo container for integer values.
\code
ushort num(123);
\endcode
*/
typedef IntegerT<ushort> UShort;

/** Basic integer type (unsigned int) -- see IntegerT. A basic Evo container for integer values.
\code
uint num(123);
\endcode
*/
typedef IntegerT<uint> UInt;

/** Basic integer type (unsigned long) -- see IntegerT. A basic Evo container for integer values.
\code
ulong num(123);
\endcode
*/
typedef IntegerT<ulong> ULong;

/** Basic integer type (unsigned long long) -- see IntegerT. A basic Evo container for integer values.
\code
ulongl num(123);
\endcode
*/
typedef IntegerT<ulongl> ULongL;

/** Basic integer type (uint8) -- see IntegerT. A basic Evo container for integer values.
\code
UInt8 num(123);
\endcode
*/
typedef IntegerT<uint8> UInt8;

/** Basic integer type (uint16) -- see IntegerT. A basic Evo container for integer values.
\code
UInt16 num(123);
\endcode
*/
typedef IntegerT<uint16> UInt16;

/** Basic integer type (uint32) -- see IntegerT. A basic Evo container for integer values.
\code
UInt32 num(123);
\endcode
*/
typedef IntegerT<uint32> UInt32;

/** Basic integer type (uint64) -- see IntegerT. A basic Evo container for integer values.
\code
UInt64 num(123);
\endcode
*/
typedef IntegerT<uint64> UInt64;

///////////////////////////////////////////////////////////////////////////////

/** \cond impl */
namespace impl {
    template<class T> struct ConstFloatT {
        static T point1() { return 0.1f; }
        static T ten()    { return 10.0f; }
    };
    template<> struct ConstFloatT<double> {
        static double point1() { return 0.1; }
        static double ten()    { return 10.0; }
    };
    template<> struct ConstFloatT<ldouble> {
        static ldouble point1() { return 0.1L; }
        static ldouble ten()    { return 10.0L; }
    };
};

/** \endcond */

/** %Nullable basic floating-point base type.
 \tparam  T  Floating-point type

Floating point types:
 - \link evo::Float Float\endlink, \link evo::FloatD FloatD\endlink, \link evo::FloatL FloatL\endlink

Examples -- using Float type (and float type) here, other supported floating point types work the same:

 - Assigned like a number:
   \code
    Float val(12.3f);       // Set to 12.3
    val = 1.23f;            // Set to 1.23
   \endcode
 - Can hold null value:
   \code
    Float val;              // Null by default
    val = 56.7f;            // Set to 56.7, no longer null
    val.set();              // Set to null
   \endcode
 - Evaluate as bool to check non-null and non-zero:
   \code
    Float val;              // Null by default
    if (val) {              // false
    }
    if (!val) {             // true
    }
    val = 0.0f;
    if (val) {              // false
    }
    val = 1.0f;
    if (val) {              // true
    }
   \endcode
 - Use denull() to clear null flag and update value via reference -- useful for decrement/increment:
   \code
    Float val;              // Null by default
    ++val.denull();         // Remove null flag (value is 0.0) and increment so value is now 1.0
   \endcode
 - Dereference to get raw value:
   \code
    Float val;              // Null by default
    if (*val == 1.23f) {    // Dereference for raw value -- false: 0.0 when null
    }
   \endcode
 - Comparison operator overloads:
   \code
    Float val;              // Null by default, with raw value 0
    if (*val == 0.0f) {     // true: raw value is 0.0
    }
    if (val == 0.0f) {      // false: null is not 0.0
    }
    if (val < 0.0f) {       // true: null is less than any value
    }
    if (*val < 0.0f) {      // false: raw value is 0.0
    }
   \endcode
 - Some traits and helpers
   \code
    bool equal = Float::eq(1.23, 1.23);     // check whether approximately equal, accounts for floating point rounding errors
   \endcode
*/
template<class T>
struct FloatT : public Nullable<T> {
    typedef FloatT<T> This;         ///< %This non-POD type
    typedef T         Type;         ///< Wrapped POD type

    static const bool IS        = IsFloat<T>::value;                        ///< Whether type is really a floating point type
    static const bool SIGN      = true;                                     ///< Whether type is signed
    static const int  BYTES     = sizeof(T);                                ///< Type size in bytes
    static const int  MAXDIGITS = std::numeric_limits<T>::digits10;         ///< Maximum significant digits without precision loss
    static const bool NANOK     = std::numeric_limits<T>::has_quiet_NaN;    ///< Whether Not-A-Number (NaN) is supported

    static const int MAXDIGITS_AUTO = MAXDIGITS + 15;       ///< Max formatting digits with auto precision (used internally)

    /** Get max formatting digits with given exponent and precision, including sign and any additional chars (used internally).
     \return  Max formatting digits
    */
    static int maxdigits_prec(int exp, int precision) {
        const int BASEDIGITS = MAXDIGITS + 9;
        return BASEDIGITS + (exp < 0 ? -exp : exp) + precision;
    }

    /** Get best precision value.
     \return  Value
    */
    static T precision() {
        static const T VAL = evo_pow(impl::ConstFloatT<T>::point1(), std::numeric_limits<T>::digits10);
        return VAL;
    }

    /** Get minimum normalized value.
     \return  Value
    */
    static T min()
        { return std::numeric_limits<T>::min(); }

    /** Get minimum allowed exponent.
     \return  Value
    */
    static int minexp()
        { return std::numeric_limits<T>::min_exponent10; }

    /** Get maximum normalized value.
     \return  Value
    */
    static T max()
        { return std::numeric_limits<T>::max(); }

    /** Get maximum allowed exponent.
     \return  Value
    */
    static int maxexp()
        { return std::numeric_limits<T>::max_exponent10; }

    /** Get infinity value.
     \return  Value
    */
    static T inf()
        { return std::numeric_limits<T>::infinity(); }

    /** Check whether value is infinite.
     \return  Value
    */
    static bool inf(T num) {
        if (num < 0.0)
            num = -num;
        return (std::numeric_limits<T>::has_infinity && num == std::numeric_limits<T>::infinity());
    }

    /** Get whether value is Not-A-Number (NaN).
     \return  Whether NaN, always false if NaN not supported
    */
    static bool nan(T num)
        { return (num != num); }

    /** Get Not-A-Number (NaN) value.
     - This value doesn't work for comparison. Use nan(T) instead
     .
     \return  NaN value, 0.0 if not supported
    */
    static T nan() {
        static const T val = (T)(NANOK ? std::numeric_limits<T>::quiet_NaN() : 0.0);
        return val;
    }

    /** Get machine epsilon. This is the difference between 1 and the least value greater than 1 that is representable.
     \return  Epsilon value
    */
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
        static const T EPSILON = std::numeric_limits<T>::epsilon();
        return ( (nan(val1) && nan(val2)) || val1 == val2 || evo_fabs(val1 - val2) <= EPSILON );
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
     \param  exp  Stores exponent value [out]
     \param  num  Number to use
     \return      Mantissa value
    */
    static T fexp10(int& exp, T num) {
        bool neg = false;
        if (num < 0.0)
            { neg = true; num = -num; }
        exp = 0;
        if (!nan(num) && !inf(num) && num != 0.0) {
            if (num >= 1.0) {
                static const int BIGNUM_DIGITS = std::numeric_limits<T>::digits10;
                static const T   BIGNUM        = evo_pow(impl::ConstFloatT<T>::ten(), BIGNUM_DIGITS);
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

    /** Get whether approximately equal to given value.
     - This uses the system epsilon value for current type to determine whether the values are close enough to be considered equal
     .
     \param  val  Value to compare to
     \return      Whether approximately equal
    */
    bool eq1(const Nullable<T>& val) {
        return ( (this->null() && val.null()) ||
                 (this->null() == val.null() && FloatT<T>::eq(this->value(), val.value())) );
    }

    /** Get whether approximately equal to given value.
     - This uses the system epsilon value for current type to determine whether the values are close enough to be considered equal
     .
     \param  val  Value to compare to
     \return      Whether approximately equal
    */
    bool eq1(T val)
        { return (!this->null() && FloatT<T>::eq(this->value(), val)); }

    using Nullable<T>::value;

    /** Get underlying value or given default if null.
     \param  defval  Default value to use if null
     \return         Result value, or defval if null
    */
    T value(T defval) const
        { return (Nullable<T>::null() ? defval : Nullable<T>::value()); }
};

/** Basic single-precision floating-point type (float) -- see FloatT. A basic Evo container for floating-point values.
\code
Float num(12.3f);
\endcode
*/
typedef FloatT<float> Float;

/** Basic double-precision floating-point type (double) -- see FloatT. A basic Evo container for floating-point values.
\code
FloatD num(12.3);
\endcode
*/
typedef FloatT<double> FloatD;

/** Basic long-double floating-point type (long double) -- see FloatT. A basic Evo container for floating-point values.
\code
FloatL num(12.3L);
\endcode
*/
typedef FloatT<long double> FloatL;

///////////////////////////////////////////////////////////////////////////////

/** Base managed pointer.
 \tparam  T  Type to use pointer to (not raw pointer type)
 \tparam  P  Actual pointer to store -- usually T* but could be something compatible like Atomic<T*>
*/
template<class T, class P=T*>
struct PtrBase : public SafeBool<PtrBase<T> > {
    typedef PtrBase<T,P> Base;      ///< This pointer base type

    P ptr_;            ///< Pointer

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
    bool operator==(const Base& ptr) const
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
    bool operator!=(const Base& ptr) const
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
    bool operator<(const Base& ptr) const
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
    bool operator<=(const Base& ptr) const
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
    bool operator>(const Base& ptr) const
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
    bool operator>=(const Base& ptr) const
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
    /** %Set target to value (reversed conversion).
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

// Special type for NONE, ALL, END values -- used internally.
/** \cond impl */
struct EndT {
    // Evo sizes are unsigned, use max value
    operator unsigned char() const
        { return IntegerT<unsigned char>::MAX; }
    operator unsigned short() const
        { return IntegerT<unsigned short>::MAX; }
    operator unsigned int() const
        { return IntegerT<unsigned int>::MAX; }
    operator unsigned long() const
        { return IntegerT<unsigned long>::MAX; }
    operator unsigned long long() const
        { return IntegerT<unsigned long long>::MAX; }

    EndT() { }
};

template<class T> bool operator==(T a, EndT)
    { return (a == IntegerT<T>::MAX); }
template<class T> bool operator==(EndT, T b)
    { return (b == IntegerT<T>::MAX); }

template<class T> bool operator!=(T a, EndT)
    { return (a != IntegerT<T>::MAX); }
template<class T> bool operator!=(EndT, T b)
    { return (b != IntegerT<T>::MAX); }
/** \endcond */

/** Special integer value for indicating no item or unknown item.
 - Returned by methods like \link evo::String::find() find()\endlink
 - Evo uses unsigned Size types, so this is used instead of the common special index value -1
 - Implicitly converts and compares with any Size (integer) type -- actual value is the max integer of the type converted/compared to
*/
static const EndT NONE;

/** Special integer value for indicating all items or all remaining items.
 - Passed to methods like \link evo::String::replace(Key,Size,const String&) replace()\endlink
 - Evo uses unsigned Size types, so this is used instead of the common special index value -1
 - Implicitly converts and compares with any Size (integer) type -- actual value is the max integer of the type converted/compared to
*/
static const EndT ALL;

/** Special integer value for indicating end of items or no item.
 - Passed as a position/index to methods like \link evo::String::find(char,Key,Key) const find()\endlink
 - Evo uses unsigned Size types, so this is used instead of the common special index value -1
 - Implicitly converts and compares with any Size (integer) type -- actual value is the max integer of the type converted/compared to
*/
static const EndT END;

///////////////////////////////////////////////////////////////////////////////

/** Special pointer value for default initialization (used in containers). 
 - This only works inside container classes with item type "T" defined
*/
#define EVO_PDEFAULT ((T*)IntegerT<std::size_t>::MAX)

/** Special pointer value for empty but not NULL (used in containers).
 - This only works inside container classes with item type "T" defined
*/
#define EVO_PEMPTY ((T*)1)

/** Special pointer value for empty but not NULL (used in containers).
 - This only works inside container classes with item type "T" defined
*/
#define EVO_PPEMPTY ((T**)1)

///////////////////////////////////////////////////////////////////////////////

// Implementation
/** \cond impl */
namespace impl {
    template<class T, int SZ=sizeof(T)>
    struct NextPow2 {
        static T next(T v) {
            T n = 1;
            while (n < v)
                n <<= 1;
            return n;
        }
    };

    template<class T> struct NextPow2<T,1> {
        static uint8 next(uint8 v) {
            --v;
            v |= v >> 1;
            v |= v >> 2;
            v |= v >> 4;
            return ++v;
        }
    };
    template<class T> struct NextPow2<T,2> {
        static uint16 next(uint16 v) {
            --v;
            v |= v >> 1;
            v |= v >> 2;
            v |= v >> 4;
            v |= v >> 8;
            return ++v;
        }
    };
    template<class T> struct NextPow2<T,4> {
        static uint32 next(uint32 v) {
            --v;
            v |= v >> 1;
            v |= v >> 2;
            v |= v >> 4;
            v |= v >> 8;
            v |= v >> 16;
            return ++v;
        }
    };
    template<class T> struct NextPow2<T,8> {
        static uint64 next(uint64 v) {
            --v;
            v |= v >> 1;
            v |= v >> 2;
            v |= v >> 4;
            v |= v >> 8;
            v |= v >> 16;
            v |= v >> 32;
            return ++v;
        }
    };
}
/** \endcond */

/** Get next power of 2 equal to or greater than given number.
 \tparam  T  Number type, inferred from param -- must be unsigned
 \param  v  Value to get next power of 2 from
 \return    Next power of 2 that's greater than or equal to v, 0 on overflow
 \see size_pow2()
*/
template<class T>
inline T next_pow2(T v) {
    assert( !IntegerT<T>::SIGN );
    v += (v == 0);
    return impl::NextPow2<T>::next(v);
}

/** Get size as power of 2.
 - This finds the lowest power of 2 that fits input size, and is at least minsize
 - On overflow the max power of 2 for Size is returned
 - Using powers of 2 allows a "faster modulus" on size using a mask (`size - 1`):
   - `i & (size - 1)` is equivalent to `i % size`
 .
 \tparam  Size  Size type to use, inferred from params
 \param  size     Input size to convert to power of 2
 \param  minsize  Minimum allowed size -- must be positive and a power of 2
 \return          Requested size as power of 2, always positive 0
 \see next_pow2()
*/
template<class Size>
inline Size size_pow2(Size size, Size minsize=2) {
    assert( !IntegerT<Size>::SIGN );
    if (size <= minsize)
        return minsize;
    const Size MAX_SIZE = (std::numeric_limits<Size>::max() >> 1) + 1;
    if (size >= MAX_SIZE)
        return MAX_SIZE;
    return impl::NextPow2<Size>::next(size);
}

/** Get whether a number is a power of 2.
 \tparam  T  Number type, inferred from param
 \param  num  Number to check
 \return      Whether num is a power of 2
*/
template<class T>
inline bool is_pow2(T num)
    { return (num && (num & (num - 1)) == 0); }

///////////////////////////////////////////////////////////////////////////////
//@}
}
#endif
