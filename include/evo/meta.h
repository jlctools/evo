// Evo C++ Library
/* Copyright 2019 Justin Crowell
Distributed under the BSD 2-Clause License -- see included file LICENSE.txt for details.
*/
///////////////////////////////////////////////////////////////////////////////
/** \file meta.h Evo Metaprogramming. */
#pragma once
#ifndef INCL_evo_meta_h
#define INCL_evo_meta_h

#include "impl/sys.h"

namespace evo {
/** \addtogroup EvoMeta
Evo metaprogramming types and helpers
*/
//@{

///////////////////////////////////////////////////////////////////////////////

/** \cond impl */
template<bool b> struct StaticAssertTrue;
template<>       struct StaticAssertTrue<true> { };
template<int N>  struct StaticAssertTestSize { };
/** \endcond */

/** Join (concatenate) compile-time symbols after resolving macros.
 - This resolves A and B if they're macros, then joins (concatenates) them together as one symbol
 .
 \param  A  First symbol/macro to join
 \param  B  Second symbol/macro to join
 \return    Joined symbol
*/
#define EVO_STATIC_JOIN(A,B) IMPL_EVO_STATIC_JOIN1(A,B)
/** \cond impl */
#define IMPL_EVO_STATIC_JOIN1(A,B) IMPL_EVO_STATIC_JOIN2(A,B)
#define IMPL_EVO_STATIC_JOIN2(A,B) A##B
/** \endcond */

#if defined(EVO_CPP11)
    /** Assert compile-time expression is true or trigger compiler error.
     - This uses some template magic to cause a custom compiler error if given expression is false
     - With C++11 this just calls static_assert()
     .
     \param  EXP    Compile-time expression to evaluate.
     \param  TOKEN  Error message token in the form of a type name that explains the error (not a string).

    \par Example
    \code
    STATIC_ASSERT(true, Assert_failed__Test_message);   // ok, no-op
    STATIC_ASSERT(false, Assert_failed__Test_message);  // compiler error: assert fails
    \endcode
    */
    #define STATIC_ASSERT(EXP,TOKEN) static_assert(EXP,#TOKEN)
#else
    #define STATIC_ASSERT(EXP,TOKEN) typedef ::evo::StaticAssertTestSize< sizeof(::evo::StaticAssertTrue<(bool)(EXP)>) > \
        EVO_STATIC_JOIN(TOKEN, __LINE__) EVO_ATTRIB_UNUSED
#endif

#if defined(EVO_CPP11)
    /** Assert a function is unused at compile-time.
     - This marks a function so that compiling will fail if the function is used (called)
     - This replaces the function implementation -- see example below
     - This should only be used with function defined in a class (or struct) or as inline
     - With C++11 this marks the function as deleted
     - Without C++11, some compilers (clang) won't show an error until linking phase (undefined function)
     .

    \par Example
    \code
    struct Foo {
        void foo()
            STATIC_ASSERT_FUNC_UNUSED
    };

    int foo()
        STATIC_ASSERT_FUNC_UNUSED_RET(0)
    \endcode
    */
    #define STATIC_ASSERT_FUNC_UNUSED =delete;

    /** Assert a function is unused at compile-time (with return value).
     - Same as STATIC_ASSERT_FUNC_UNUSED, but includes a return value to suppress warnings on missing return value
     .
    \code
    struct Foo {
        int bar()
            STATIC_ASSERT_FUNC_UNUSED_RET(0)
    };

    int bar()
        STATIC_ASSERT_FUNC_UNUSED_RET(0)
    \endcode
     \param  RET  Return value for function
    */
    #define STATIC_ASSERT_FUNC_UNUSED_RET(RET) =delete;
#elif defined(__clang__)
    #define STATIC_ASSERT_FUNC_UNUSED ;
    #define STATIC_ASSERT_FUNC_UNUSED_RET(RET) ;
#else
    #define STATIC_ASSERT_FUNC_UNUSED          { ::evo::StaticAssertTrue<false> EVO_STATIC_JOIN(_ASSERT_FUNC_UNUSED_, __LINE__); }
    #define STATIC_ASSERT_FUNC_UNUSED_RET(RET) { ::evo::StaticAssertTrue<false> EVO_STATIC_JOIN(_ASSERT_FUNC_UNUSED_, __LINE__); return RET; }
#endif

///////////////////////////////////////////////////////////////////////////////

/** Static bool value.
This holds a bool result value to be used at compile-time.
 \tparam  val  Value.
*/
template<bool val> struct StaticBool {
    typedef typename evo::StaticBool<val> Type;    ///< This type
    static const bool value = val;                ///< Result value
};

/** Static bool value (false). */
typedef StaticBool<false> StaticBoolF;

/** Static bool value (true). */
typedef StaticBool<true> StaticBoolT;

/** Static conditional type.
This returns one of two types depending on evaluation of given expression. Result is in Type member.
 \tparam  b  Expression to evaluate (must be valid compile-time expression).
 \tparam  T  Type to use if true.
 \tparam  F  Type to use if false.

\par Example
\code
StaticIf<true,int,char>::Type  foo1; // type is int
StaticIf<false,int,char>::Type foo2; // type is char
\endcode
*/
template<bool b, class T, class F> struct StaticIf { /** Result type (T or F) */ typedef T Type; };
/** \cond impl */
template<class T, class F> struct StaticIf<false,T,F> { typedef F Type; };
template<class T, class F> struct StaticIf<true,T,F> { typedef T Type; };
/** \endcond */

///////////////////////////////////////////////////////////////////////////////

/** Check if types are exactly the same. Value member holds result.
 \tparam  T1  First type to compare.
 \tparam  T2  Second type to compare.

\par Example
\code
bool b1 = IsSame<char,char>::value; // true
bool b2 = IsSame<char,int>::value;  // false
\endcode
*/
template<class T1, class T2> struct IsSame : public StaticBoolF { };
/** \cond impl */
template<class T> struct IsSame<T,T> : public StaticBoolT { };
/** \endcond */

/** Check if type is a pointer. Value member holds result.
 \tparam  T  Type to check.

\par Example
\code
bool b1 = IsPointer<char*>::value; // true
bool b2 = IsPointer<char>::value;  // false
\endcode
*/
template<class T> struct IsPointer : public StaticBoolF { };
/** \cond impl */
template<class T> struct IsPointer<T*> : public StaticBoolT { };
/** \endcond */

/** Check if type is an array. Value member holds result.
 \tparam  T  Type to check.

\par Example
\code
bool b1 = IsArray<char[2]>::value; // true
bool b2 = IsArray<char*>::value;   // false
bool b3 = IsArray<char>::value;    // false
\endcode
*/
template<class T> struct IsArray : public StaticBoolF { };
/** \cond impl */
template<class T> struct IsArray<T[]> : public StaticBoolT { };
template<class T, ulong N> struct IsArray<T[N]> : public StaticBoolT { };
/** \endcond */

/** Check if type is a reference. Value member holds result.
 \tparam  T  Type to check.

\par Example
\code
bool b1 = IsReference<char&>::value; // true
bool b2 = IsReference<char*>::value; // false
bool b3 = IsReference<char>::value;  // false
\endcode
*/
template<class T> struct IsReference : public StaticBoolF { };
/** \cond impl */
template<class T> struct IsReference<T&> : public StaticBoolT { };
/** \endcond */

/** Check if type is const. Value member holds result.
 \tparam  T  Type to check.

\par Example
\code
bool b1 = IsConst<const char>::value;  // true
bool b2 = IsConst<const char*>::value; // false
bool b3 = IsConst<char* const>::value; // true
bool b4 = IsConst<char>::value;        // false
\endcode
*/
template<class T> struct IsConst : public StaticBoolF { };
/** \cond impl */
template<class T> struct IsConst<const T> : public StaticBoolT { };
/** \endcond */

/** Check if type is a boolean (true/false) type. Value member holds result.
 \tparam  T  Type to check.

\par Example
\code
bool b1 = IsBool<bool>::value;         // true
bool b2 = IsBool<int>::value;          // false
\endcode
 */
template<class T> struct IsBool : public StaticBoolF { };
/** \cond impl */
template<> struct IsBool<bool> : public StaticBoolT { };
/** \endcond */

/** Check if type is an integer (whole number) type. Value member holds result.
 - Integer types (signed and unsigned): char, short, int, long
 .
 \tparam  T  Type to check.

\par Example
\code
bool b1 = IsInt<char>::value;         // true
bool b2 = IsInt<int>::value;          // true
bool b3 = IsInt<float>::value;        // false
bool b4 = IsInt<unsigned int>::value; // true
bool b5 = IsInt<double>::value;       // false
\endcode
*/
template<class T> struct IsInt : public StaticBoolF { };
/** \cond impl */
template<> struct IsInt<char>                : public StaticBoolT { };
template<> struct IsInt<signed char>         : public StaticBoolT { };
template<> struct IsInt<signed short>        : public StaticBoolT { };
template<> struct IsInt<signed int>          : public StaticBoolT { };
template<> struct IsInt<signed long>         : public StaticBoolT { };
template<> struct IsInt<signed long long>    : public StaticBoolT { };
template<> struct IsInt<unsigned char>       : public StaticBoolT { };
template<> struct IsInt<unsigned short>      : public StaticBoolT { };
template<> struct IsInt<unsigned int>        : public StaticBoolT { };
template<> struct IsInt<unsigned long>       : public StaticBoolT { };
template<> struct IsInt<unsigned long long>  : public StaticBoolT { };
/** \endcond */

/** Check if type is a floating point type. Value member holds result.
 - Floating point types: float, double, long double
 .
 \tparam  T  Type to check.

\par Example
\code
bool b1 = IsFloat<char>::value;         // false
bool b2 = IsFloat<int>::value;          // false
bool b3 = IsFloat<float>::value;        // true
bool b4 = IsFloat<unsigned int>::value; // false
bool b5 = IsFloat<double>::value;       // true
\endcode
*/
template<class T> struct IsFloat : public StaticBoolF { };
/** \cond impl */
template<> struct IsFloat<float>       : public StaticBoolT { };
template<> struct IsFloat<double>      : public StaticBoolT { };
template<> struct IsFloat<long double> : public StaticBoolT { };
/** \endcond */

/** Check if integer type is unsigned. Value member holds result.
 \tparam  T  Type to check.

\par Example
\code
bool b1 = IsSigned<unsigned char>::value; // false
bool b2 = IsSigned<int>::value;           // true
bool b3 = IsSigned<float>::value;         // true
bool b4 = IsSigned<unsigned int>::value;  // false
bool b5 = IsSigned<double>::value;        // true
\endcode
*/
template<class T> struct IsSigned : public StaticBoolT { };
/** \cond impl */
template<> struct IsSigned<unsigned char>      : public StaticBoolF { };
template<> struct IsSigned<unsigned short>     : public StaticBoolF { };
template<> struct IsSigned<unsigned int>       : public StaticBoolF { };
template<> struct IsSigned<unsigned long>      : public StaticBoolF { };
template<> struct IsSigned<unsigned long long> : public StaticBoolF { };
/** \endcond */

/** Check if type T is convertible to type U. Value member holds result.
 \tparam  T  Type to check conversion on
 \tparam  U  Type to check conversion to
*/
template<class T, class U>
class IsConvertible {
private:
    typedef char SmallType;
    class LargeType { char dummy[2]; };
    static SmallType check(const U&);
    static LargeType check(...);
    static T make();

public:
    static const bool value = (sizeof(check(make())) == sizeof(SmallType)); ///< Result value
};

///////////////////////////////////////////////////////////////////////////////

/** Add const to type. Type member holds translated type. */
template<class T> struct AddConst { /** Translated type. */ typedef const T Type; };
/** \cond impl */
template<class T> struct AddConst<const T> { typedef T Type; };
/** \endcond */

/** Remove const from type. Type member holds translated type. */
template<class T> struct RemoveConst { /** Translated type. */ typedef T Type; };
/** \cond impl */
template<class T> struct RemoveConst<const T> { typedef T Type; };
/** \endcond */

/** Remove volatile from type. Type member holds translated type. */
template<class T> struct RemoveVolatile { /** Translated type. */ typedef T Type; };
/** \cond impl */
template<class T> struct RemoveVolatile<volatile T> { typedef T Type; };
/** \endcond */

/** Remove const & volatile from type. Type member holds translated type. */
template<class T> struct RemoveConstVol { /** Translated type. */ typedef T Type; };
/** \cond impl */
template<class T> struct RemoveConstVol<const T>    { typedef typename RemoveConstVol<T>::Type Type; };
template<class T> struct RemoveConstVol<volatile T> { typedef typename RemoveConstVol<T>::Type Type; };
/** \endcond */

/** Remove extents (pointer and array parts) from type. Type member holds translated type. */
template<class T> struct RemoveExtents { /** Translated type. */ typedef T Type; };
/** \cond impl */
template<class T> struct RemoveExtents<T*>            { typedef typename RemoveExtents<T>::Type Type; };
template<class T> struct RemoveExtents<T[]>           { typedef typename RemoveExtents<T>::Type Type; };
template<class T, ulong N> struct RemoveExtents<T[N]> { typedef typename RemoveExtents<T>::Type Type; };
/** \endcond */

/** Remove extents (pointer and array parts) and then const & volatile from type. Type member holds translated type. */
template<class T> struct RemoveExtentsConstVol { /** Translated type. */ typedef typename RemoveConstVol<typename RemoveExtents<T>::Type>::Type Type; };

///////////////////////////////////////////////////////////////////////////////

/** Translate integer type to signed. */
template<class T> struct ToSigned { /** Translated type. */ typedef T Type; };
/** \cond impl */
template<> struct ToSigned<unsigned char>      { typedef char Type; };
template<> struct ToSigned<unsigned short>     { typedef short Type; };
template<> struct ToSigned<unsigned int>       { typedef int   Type; };
template<> struct ToSigned<unsigned long>      { typedef long  Type; };
template<> struct ToSigned<unsigned long long> { typedef long  Type; };
/** \endcond */

/** Translate integer type to unsigned. */
template<class T> struct ToUnsigned { /** Translated type. */ typedef T Type; };
/** \cond impl */
template<> struct ToUnsigned<signed char> { typedef unsigned char  Type; };
template<> struct ToUnsigned<char>        { typedef unsigned char  Type; };
template<> struct ToUnsigned<short>       { typedef unsigned short Type; };
template<> struct ToUnsigned<int>         { typedef unsigned int   Type; };
template<> struct ToUnsigned<long>        { typedef unsigned long  Type; };
template<> struct ToUnsigned<long long>   { typedef unsigned long long Type; };
/** \endcond */

///////////////////////////////////////////////////////////////////////////////

/** Helper for creating a boolean trait type.
 - The trait value will be true for types explicitly set, otherwise defaults to false
 - Use EVO_TRAIT_SET() or EVO_TRAIT_NSET() to set trait values
 - Use only at namespace scope, not struct/class scope (this uses template specialization)
 .
 \param  Name  Name for trait type
*/
#define EVO_TRAIT_CREATE(Name) template<class T> struct Name : StaticBoolF { };

/** Helper for setting a boolean trait for given type.
 - This sets the trait value to true for given type
 - Must create trait first with EVO_TRAIT_CREATE()
 - Use only in same namespace as created
   - If different namespace from created see: EVO_TRAIT_NSET()
 .
 \param  Name  Name for trait type -- same name created with EVO_TRAIT_CREATE()
 \param  Type  Type to set trait for

\par Example
\code
EVO_TRAIT_CREATE(IsSignedInt);
EVO_TRAIT_SET(IsSignedInt,int);
EVO_TRAIT_SET(IsSignedInt,long);

void main() {
    bool b1 = IsSignedInt<int>::value;      // true
    bool b2 = IsSignedInt<long>::value;     // true
    bool b3 = IsSignedInt<char>::value;     // false
}
\endcode
*/
#define EVO_TRAIT_SET(Name,Type) template<> struct Name< Type > : public StaticBoolT { };

/** Helper for setting a boolean trait for given type.
 - This sets the trait value to true for given type
 - Must create trait first with EVO_TRAIT_CREATE()
 - Use only at global scope (not inside a namespace)
   - If in same namespace as created can also use: EVO_TRAIT_SET()
 .
 \param  Namespace  Namespace for trait
 \param  Name       Name for trait type -- same name created with EVO_TRAIT_CREATE()
 \param  Type       Type to set trait for

\par Example
\code
namespace foo {
    EVO_TRAIT_CREATE(IsSignedInt);
}
EVO_TRAIT_NSET(foo,IsSignedInt,int);
EVO_TRAIT_NSET(foo,IsSignedInt,long);

void main() {
    bool b1 = foo::IsSignedInt<int>::value;     // true
    bool b2 = foo::IsSignedInt<long>::value;    // true
    bool b3 = foo::IsSignedInt<char>::value;    // false
}
\endcode
*/
#define EVO_TRAIT_NSET(Namespace,Name,Type) namespace Namespace { template<> struct Name< Type > : public StaticBoolT { }; }

///////////////////////////////////////////////////////////////////////////////

/** Trait to identify Evo container types.
 - See \ref PrimitivesContainers
*/
EVO_TRAIT_CREATE(EvoContainer)

/** Identify given type as an EvoContainer.
 - The type must meet the requirements of EvoContainer, otherwise results are undefined
 - See \ref PrimitivesContainers
 - Must use at global scope (no namespace) since this modifies evo namespace
 .
 \param  Type  Type to identify as EvoContainer
*/
#define EVO_CONTAINER(Type) EVO_TRAIT_NSET(evo,EvoContainer,Type)

/** Identify current class/struct as an EvoContainer.
 - The type must meet the requirements of EvoContainer, otherwise results are undefined
 - Use in class/struct scope, anywhere a typedef is valid
 .

\par Example
\code
#include <evo/type.h>

class Foo {
    EVO_CONTAINER_TYPE;
    // ...
};
class Bar { };

void main() {
    bool b1 = IsEvoContainer<Foo>::value;   // true
    bool b2 = IsEvoContainer<Bar>::value;   // false
}
\endcode
*/
#define EVO_CONTAINER_TYPE typedef void EvoContainerType

///////////////////////////////////////////////////////////////////////////////

/** \cond impl */
// Applies (calls) f(type) for each builtin type
#define EVO_APPLY_BUILTINS(f) \
    f(bool); \
    f(char); \
    f(signed char); \
    f(unsigned char); \
    f(short); \
    f(unsigned short); \
    f(int); \
    f(unsigned int); \
    f(long); \
    f(unsigned long); \
    f(long long); \
    f(unsigned long long); \
    f(float); \
    f(double); \
    f(long double);

// PodType & ByteCopyType
template<class T> struct PodType : StaticBoolF { };
template<class T> struct ByteCopyType : StaticBoolF { };
#define EVO_PODTYPE(Type) template<> struct PodType<Type> : public StaticBoolT { };
EVO_APPLY_BUILTINS(EVO_PODTYPE);
#undef EVO_PODTYPE // redefined below
/** \endcond */

/** Check if type is a Plan Old Data type. Value member holds result.
 \tparam  T  Type to check

\par Example
\code
struct Foo { };
struct Bar { };
EVO_PODTYPE(Foo);
bool b1 = IsPodType<char>::value; // true
bool b2 = IsPodType<Foo*>::value; // true
bool b3 = IsPodType<Foo>::value;  // true
bool b4 = IsPodType<Bar*>::value; // true
bool b5 = IsPodType<Bar>::value;  // false
\endcode
*/
template<class T> struct IsPodType :
    public StaticBool<IsPointer<T>::value || PodType<typename RemoveExtentsConstVol<T>::Type>::value> { };

/** Check if type is a ByteCopy type. Value member holds result.
 \tparam  T  Type to check

\par Example
\code
struct Foo { };
struct Bar { };
EVO_BCTYPE(Foo);
bool b1 = IsByteCopyType<char>::value; // true
bool b2 = IsByteCopyType<Foo*>::value; // true
bool b3 = IsByteCopyType<Foo>::value;  // true
bool b4 = IsByteCopyType<Bar*>::value; // true
bool b5 = IsByteCopyType<Bar>::value;  // false
\endcode
*/
template<class T> struct IsByteCopyType :
    public StaticBool<IsPodType<T>::value || ByteCopyType<typename RemoveExtentsConstVol<T>::Type>::value> { };

/** Check if type is a normal type (not POD and not ByteCopy type). Value member holds result.
 \tparam  T  Type to check

\par Example
\code
struct Foo { };
struct Bar { };
EVO_BCTYPE(Foo);
bool b1 = IsNormalType<char>::value; // false
bool b2 = IsNormalType<Foo*>::value; // false
bool b3 = IsNormalType<Foo>::value;  // false
bool b4 = IsNormalType<Bar*>::value; // false
bool b5 = IsNormalType<Bar>::value;  // true
\endcode
*/
template<class T> struct IsNormalType : public StaticBool<!IsByteCopyType<T>::value> { };

///////////////////////////////////////////////////////////////////////////////

/** Check if type is an EvoContainer.
 - `value` member holds result
 - See \ref PrimitivesContainers
 .
 \tparam  T  Type to check
*/
template<class T> class IsEvoContainer {
    template<class U> struct SFINAE { };
    template<class U> static char Test(SFINAE<typename U::EvoContainerType>*);
    template<class U> static long Test(...);
public:
    typedef IsEvoContainer<T> Type;    ///< This type
    static const bool value = (!IsPodType<T>::value && (sizeof(Test<T>(0)) == sizeof(char) || EvoContainer<T>::value)); ///< Result value
};

///////////////////////////////////////////////////////////////////////////////

/** Check if type is nullable.
 - `value` member holds result
 - Recognized nullable types include: 
   - Any type inheriting Nullable
   - Any EvoContainer type, see: \ref PrimitivesContainers
*/
template<class T> class IsNullable {
    template<class U> struct SFINAE { };
    template<class U> static char Test(SFINAE<typename U::EvoNullableType>*);
    template<class U> static long Test(...);
public:
    typedef IsNullable<T> Type;    ///< This type
    static const bool value = (!IsPodType<T>::value && sizeof(Test<T>(0)) == sizeof(char)) || IsEvoContainer<T>::value; ///< Result value
};

/** \cond impl */
namespace impl {
    template<class T, bool P=IsPointer<T>::value, bool N=IsNullable<T>::value>
    struct IsNull {
        static bool check(const T&)
            { return false; }
    };
    template<class T> struct IsNull<T,true,false> {
        static bool check(const T val)
            { return (val == NULL); }
    };
    template<class T> struct IsNull<T,false,true> {
        static bool check(const T& val)
            { return val.null(); }
    };
}
/** \endcond */

/** Check whether object or value is null.
 - \#include <evo/meta.h>
 - This works with any type, non-nullable types always return false
 - Recognized nullable types include:
   - Any type inheriting Nullable
   - Any EvoContainer type, see: \ref PrimitivesContainers
   - Any managed pointer type, see: \ref ManagedPtr
   - Any raw pointer, where `0` or `NULL` is considered null
 .
 \tparam  T  Type to check, inferred from argument
 \param  val  Object or value to check if null
 \return      Whether object or value is null
*/
template<class T>
inline bool is_null(const T& val)
    { return impl::IsNull<T>::check(val); }

///////////////////////////////////////////////////////////////////////////////

/** Type ID info (POD, %ByteCopy, %Normal).
 - This is used to find out whether a given type is a POD (Plain Old Data) type, a %ByteCopy type, or a %Normal type
 - Usually used to specialize a class on each of these IDs
 - Categories of type IDs:
   - TypeId::Get has normal types (Pod/ByteCopy/Normal)
   - TypeId::GetFill has array/buffer fill types (memset/memcpy/assign)
   .
 .

\#include <evo/type.h>

\par Example

\code
#include <evo/type.h>
#include <evo/io.h>
using namespace evo;

// Example item types, declare Foo as a ByteCopy type
struct Foo { };
struct Bar { };
EVO_BCTYPE(Foo);

// Create MyClass specializations -- U is inferred from T
template<class T, class U=typename TypeId::Get<T>::Id>
struct MyClass {
    static void print() {
        con().out << "MyClass with Normal type" << NL;
    }
};

template<class T>
struct MyClass<T, TypeId::Pod> {
    static void print() {
        con().out << "MyClass with POD type" << NL;
    }
};

template<class T>
struct MyClass<T, TypeId::ByteCopy> {
    static void print() {
        con().out << "MyClass with ByteCopy type" << NL;
    }
};

int main() {
    MyClass<int>::print();      // POD type
    MyClass<Foo>::print();      // ByteCopy type
    MyClass<Bar>::print();      // Normal type
    return 0;
};
\endcode

Output:
\code{.unparsed}
MyClass with POD type
MyClass with ByteCopy type
MyClass with Normal type
\endcode
*/
struct TypeId {
    /** Used to identify POD (Plain Old Data) types. */
    struct Pod { };

    /** Used to identify %ByteCopy types. */
    struct ByteCopy { };

    /** Used to identify normal types (not %POD, and not %ByteCopy). */
    struct Normal { };

    /** Type ID enum values. */
    enum Enum {
        POD = 0,    ///< POD (Plain Old Data) type ID value
        BCOPY,      ///< Byte-copy type ID value
        NORMAL      ///< %Normal type ID value
    };

    /** Use to get type ID info for type T.
     - This is used by containers for things like copy optimization
     - See TypeId
     .
     \tparam  T  %Type to get type ID info on
    */
    template<class T> struct Get {
        /** %Type ID for type T (Pod, ByteCopy, Normal). */
        typedef typename StaticIf<IsPodType<T>::value, Pod,
            typename StaticIf<IsByteCopyType<T>::value, ByteCopy, Normal>::Type
        >::Type Id;

        /** %Enum value for type T (POD, BCOPY, NORMAL). */
        static const Enum ENUM = (IsPodType<T>::value ? POD : (IsByteCopyType<T>::value ? BCOPY : NORMAL));
    };

    /** Use to get type ID info for type T as for filling arrays/buffers.
     - This is used by containers for filling arrays and buffers (memset, memcpy, assignment)
     - See TypeId
     .
     \tparam  T  %Type to get type ID info on
    */
    template<class T> struct GetFill {
        /** %Type ID for type T (Pod for memset, ByteCopy for memcpy, Normal for assignment). */
        typedef typename StaticIf< IsPodType<T>::value || IsByteCopyType<T>::value,
            typename StaticIf<sizeof(T) == 1, Pod, ByteCopy>::Type,
            Normal
        >::Type Id;

        /** %Enum value for type T (POD for memset, BCOPY for memcpy, NORMAL for assignment). */
        static const Enum ENUM = (IsPodType<T>::value ? POD : (IsByteCopyType<T>::value ? BCOPY : NORMAL));
    };
};

/** Identify given type as a Plain Old Data Type.
 - POD types do not use any constructors, assignment operators, or destructor; and may be directly byte-copied
 - All built-in types and pointer types are considered POD types -- <i>it's unlikely you'll need to use this directly</i>
 - Containers take advantage of these rules with performance optimizations
 - This can only be used in global scope
 .
 \param  Type  Type name to set as POD type.
 \see EVO_BCTYPE()

\par Example

\code
struct Foo { };
EVO_PODTYPE(Foo);
\endcode
*/
#define EVO_PODTYPE(Type) namespace evo { template<> struct PodType<Type> : public StaticBoolT { }; }

/** Identify the given type as a Byte-Copy type.
 - Byte-Copy types may be directly byte-copied for improved performance.
 - Containers take advantage of these rules with performance optimizations.
 - This can only be used in global scope.
 .
 \param  Type  Type name to set as ByteCopy type.
 \see EVO_PODTYPE()

\par Example
\code
struct Foo { };
EVO_BCTYPE(Foo);
\endcode
*/
#define EVO_BCTYPE(Type) namespace evo { template<> struct ByteCopyType<Type> : public StaticBoolT { }; }

/** Select a type depending on whether Type is POD, ByteCopy, or Normal type.
 - See: IsPodType(), IsByteCopyType(), IsNormalType()
 .
 \param  T       Type to check
 \param  Pod     Type to use if POD type
 \param  Bc      Type to use if ByteCopy type
 \param  Normal  Type to use if Normal type
 \return         Selected type (Pod, Bc, or Normal)
*/
#define EVO_TYPE_SELECT(T,Pod,Bc,Normal) typename StaticIf<IsPodType<T>::value,Pod,typename StaticIf<IsByteCopyType<T>::value,Bc,Normal>::Type>::Type

///////////////////////////////////////////////////////////////////////////////

/** Create template to check if type has a member variable matching given signature.
 - This defines a template class that can detect the given member variable signature
 .
 \param  TypeName  Template type name to create
 \param  VarType   Member function return type
 \param  VarName   Member function name

\par Example
\code
struct Class1 {
    int foo;
};
struct Class2 {
    const int foo;
};
struct Class3 {
    long foo;
};

EVO_CREATE_HAS_VAR(HasIntFoo, int, foo);
EVO_CREATE_HAS_VAR(HasCIntFoo, const int, foo);
EVO_CREATE_HAS_VAR(HasLongFoo, long, foo);

void main() {
    bool b1  = HasIntFoo<Class1>::value;        // true
    bool b2 = HasCIntFoo<Class2>::value;        // true
    bool b3 = HasLongFoo<Class3>::value;        // true
    bool b4 = HasLongFoo<Class1>::value;        // false
}
\endcode
*/
#define EVO_CREATE_HAS_VAR(TypeName, VarType, VarName) \
    template<class T> class TypeName { \
        template<class U, VarType U::*> struct SFINAE { }; \
        template<class U> static char Test(SFINAE<U, &U::VarName>*); \
        template<class U> static long Test(...); \
    public: \
        static const bool value = (sizeof(Test<T>(0)) == sizeof(char)); \
    }

/** Create template to check if type has a member function matching given signature.
 - This defines a template class that can detect the given member function signature
 - To check for const member function see: EVO_CREATE_HAS_METHOD_CONST()
 .
 \param  TypeName    Template type name to create
 \param  ReturnType  Member function return type
 \param  Func        Member function name
 \param  ...         Member function parameter types (0 or more)

\par Example
\code
struct Class1 {
    void foo();
};
struct Class2 {
    void foo(int);
};
struct Class3 {
    int foo(int);
};

EVO_CREATE_HAS_METHOD(HasFoo, void, foo);
EVO_CREATE_HAS_METHOD(HasFooInt, void, foo, int);
EVO_CREATE_HAS_METHOD(HasIntFooInt, int, foo, int);

void main() {
    bool b1  = HasFoo<Class1>::value;           // true
    bool b2 = HasFooInt<Class2>::value;         // true
    bool b3 = HasIntFooInt<Class3>::value;      // true
    bool b4 = HasIntFooInt<Class1>::value;      // false
}
\endcode
*/
#define EVO_CREATE_HAS_METHOD(TypeName, ReturnType, Func, ...) \
    template<class T> class TypeName { \
        template<class U, ReturnType (U::*)(__VA_ARGS__)> struct SFINAE { }; \
        template<class U> static char Test(SFINAE<U, &U::Func>*); \
        template<class U> static long Test(...); \
    public: \
        static const bool value = (sizeof(Test<T>(0)) == sizeof(char)); \
    }

/** Create template to check if type has a const member function matching given signature.
 - This defines a template class that can detect the given member function signature
 - To check for non-const member function see: EVO_CREATE_HAS_METHOD()
 .
 \param  TypeName    Template type name to create
 \param  ReturnType  Member function return type
 \param  Func        Member function name
 \param  ...         Member function parameter types (0 or more)

\par Example
\code
struct Class1 {
    void foo() const;
};
struct Class2 {
    void foo(int) const;
};
struct Class3 {
    int foo(int) const;
};

EVO_CREATE_HAS_METHOD_CONST(HasFoo, void, foo);
EVO_CREATE_HAS_METHOD_CONST(HasFooInt, void, foo, int);
EVO_CREATE_HAS_METHOD_CONST(HasIntFooInt, int, foo, int);

void main() {
    bool b1 = HasFoo<Class1>::value;            // true
    bool b2 = HasFooInt<Class2>::value;         // true
    bool b3 = HasIntFooInt<Class3>::value;      // true
    bool b4 = HasIntFooInt<Class1>::value;      // false
}
\endcode
*/
#define EVO_CREATE_HAS_METHOD_CONST(TypeName, ReturnType, Func, ...) \
    template<class T> class TypeName { \
        template<typename U, ReturnType (U::*)(__VA_ARGS__) const> struct SFINAE { }; \
        template<typename U> static char Test(SFINAE<U, &U::Func>*); \
        template<typename U> static long Test(...); \
    public: \
        static const bool value = (sizeof(Test<T>(0)) == sizeof(char)); \
    }

///////////////////////////////////////////////////////////////////////////////

// ContainerType trait (used internally)
/** \cond impl */
template<class T> struct ContainerType : StaticBoolF { };
#define EVO_CONTAINERTYPE(Type) template<> struct ContainerType<Type> : public StaticBoolT { };
/** \endcond */

///////////////////////////////////////////////////////////////////////////////
//@}
}
#endif
