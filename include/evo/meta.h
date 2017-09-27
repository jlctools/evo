// Evo C++ Library
/* Copyright (c) 2016 Justin Crowell
 This Source Code Form is subject to the terms of the Mozilla Public
 License, v. 2.0. If a copy of the MPL was not distributed with this
 file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
///////////////////////////////////////////////////////////////////////////////
/** \file meta.h Evo Metaprogramming. */
#pragma once
#ifndef INCL_evo_meta_h
#define INCL_evo_meta_h

// Includes
#include "impl/sys.h"

// Namespace: evo
namespace evo {

/** \addtogroup EvoMeta */
//@{

///////////////////////////////////////////////////////////////////////////////

// EVO_STATIC_JOIN()
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

// STATIC_ASSERT()
/** \cond impl */
template<bool b> struct StaticAssertTrue;
template<>       struct StaticAssertTrue<true> { };
template<int N>  struct StaticAssertTestSize { };
/** \endcond */
/** Assert compile-time expression is true or trigger compiler error.
 - This uses some template magic to cause a custom compiler error if given expression is false
 - With C++11 this just calls static_assert()
 .
\code
STATIC_ASSERT(true, Assert_failed__Test_message);   // ok, no-op
STATIC_ASSERT(false, Assert_failed__Test_message);  // compiler error: assert fails
\endcode
 \param  EXP    Compile-time expression to evaluate.
 \param  TOKEN  Error message token in the form of a type name that explains the error (not a string).
*/
#if defined(EVO_CPP11)
    #define STATIC_ASSERT(EXP,TOKEN) static_assert(EXP,#TOKEN)
#else
    #define STATIC_ASSERT(EXP,TOKEN) typedef ::evo::StaticAssertTestSize< sizeof(::evo::StaticAssertTrue<(bool)(EXP)>) > \
        EVO_STATIC_JOIN(TOKEN, __LINE__) EVO_ATTRIB_UNUSED
#endif

// STATIC_ASSERT_UNUSED(), STATIC_ASSERT_FUNC_UNUSED_RET()
#if defined(EVO_CPP11)
    #define STATIC_ASSERT_FUNC_UNUSED =delete;
    #define STATIC_ASSERT_FUNC_UNUSED_RET(RET) =delete;
#elif defined(__clang__)
    #define STATIC_ASSERT_FUNC_UNUSED ;
    #define STATIC_ASSERT_FUNC_UNUSED_RET(RET) ;
#else
    #define STATIC_ASSERT_FUNC_UNUSED          { ::evo::StaticAssertTrue<false> EVO_STATIC_JOIN(_ASSERT_FUNC_UNUSED_, __LINE__); }
    #define STATIC_ASSERT_FUNC_UNUSED_RET(RET) { ::evo::StaticAssertTrue<false> EVO_STATIC_JOIN(_ASSERT_FUNC_UNUSED_, __LINE__); return RET; }
#endif

/** \def STATIC_ASSERT_FUNC_UNUSED
 Assert a function is unused at compile-time.
 - This marks a function so that compiling will fail if the function is used (called)
 - This replaces the function implementation -- see example below
 - This should only be used with function defined in a class (or struct) or as inline
 - With C++11 this marks the function as deleted
 - Without C++11, some compilers (clang) won't show an error until linking phase (undefined function)
 .
\code
struct Foo {
    void foo()
        STATIC_ASSERT_FUNC_UNUSED
};

inline int foo()
    STATIC_ASSERT_FUNC_UNUSED_RET(0)
\endcode
*/

/** \def STATIC_ASSERT_FUNC_UNUSED_RET(RET)
 Assert a function is unused at compile-time (with return value).
 - Same as STATIC_ASSERT_FUNC_UNUSED, but includes a return value to suppress warnings on missing return value
 .
\code
struct Foo {
    int bar()
        STATIC_ASSERT_FUNC_UNUSED_RET(0)
};

inline int bar()
    STATIC_ASSERT_FUNC_UNUSED_RET(0)
\endcode
 \param  RET  Return value for function
*/

///////////////////////////////////////////////////////////////////////////////

/** Static bool value. This holds a bool result value to be used at compile-time.
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

/** Static conditional type. This returns one of two types depending on evaluation of given expression. Result is in Type member.
 \tparam  b  Expression to evaluate (must be valid compile-time expression).
 \tparam  T  Type to use if true.
 \tparam  F  Type to use if false.
\par Example:
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
\par Example:
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
\par Example:
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
\par Example:
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
\par Example:
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
\par Example:
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
\par Example:
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
\par Example:
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
\par Example:
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
\par Example:
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
template<> struct IsSigned<unsigned char>  : public StaticBoolF { };
template<> struct IsSigned<unsigned short> : public StaticBoolF { };
template<> struct IsSigned<unsigned int>   : public StaticBoolF { };
template<> struct IsSigned<unsigned long>  : public StaticBoolF { };
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

// TODO
/*template<class B, class T>
struct IsBaseOf {
    typedef typename RemoveConst<B>::Type Base;
    typedef typename RemoveConst<T>::Type Derived;
    static const bool value = (IsSame<Base,Derived>::value || IsConvertible<Derived,Base>::value);
};*/

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
/** \endcond */

/** Check if type is a Plan Old Data type. Value member holds result.
 \tparam  T  Type to check.
\par Example:
\code
struct Foo { };
struct Bar { };
PODTYPE(Foo);
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
 \tparam  T  Type to check.
\par Example:
\code
struct Foo { };
struct Bar { };
BCTYPE(Foo);
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
 \tparam  T  Type to check.
\par Example:
\code
struct Foo { };
struct Bar { };
BCTYPE(Foo);
bool b1 = IsNormalType<char>::value; // false
bool b2 = IsNormalType<Foo*>::value; // false
bool b3 = IsNormalType<Foo>::value;  // false
bool b4 = IsNormalType<Bar*>::value; // false
bool b5 = IsNormalType<Bar>::value;  // true
\endcode
*/
template<class T> struct IsNormalType : public StaticBool<!IsByteCopyType<T>::value> { };

// Type ID (POD, Basic, Normal)
/** Type ID for POD type - see TypeId */
struct TIdPod { };
/** Type ID for ByteCopy type - see TypeId */
struct TIdByteCopy { };
/** Type ID for normal type - see TypeId */
struct TIdNormal { };
/** Type ID enum -- see TypeId. */
enum TIdType {
    itPOD,        ///< POD type
    itBCOPY,    ///< ByteCopy type
    itNORMAL    ///< Normal type
};

/** Get type ID for given type. Type member holds type ID type.
 \tparam  T  Type to get type ID for.
\par Example:
\code
struct Foo { };
struct Bar { };
BCTYPE(Foo);
TypeId<char>::Type foo1; // type is TIdPod
TypeId<Foo>::Type  foo2; // type is TIdByteCopy
TypeId<Bar>::Type  foo3; // type is TIdNormal
\endcode
*/
template<class T> struct TypeId {
    /** Result type (TIdPod, TIdByteCopy, or TIdNormal). */
    typedef typename
        StaticIf<
            IsPodType<T>::value,
            TIdPod,
            typename StaticIf<IsByteCopyType<T>::value, TIdByteCopy, TIdNormal>::Type
        >::Type Type;

    /** Result type as enum (itPOD, itBCOPY, itNORMAL). */
    static const TIdType type = (IsPodType<T>::value ? itPOD : (IsByteCopyType<T>::value ? itBCOPY : itNORMAL));
};

// PODTYPE()
/** Identify given type as a Plain Old Data Type.
 - POD types do not use any constructors, assignment operators, or destructor; and may be directly byte-copied.
 - All built-in types and pointer types are considered POD types.
 - Containers take advantage of these rules with performance optimizations.
 - This can only be used in global scope.
 .
 \param  Type  Type name to set as POD type.
 \see BCTYPE()
\par Example:
\code
struct Foo { };
PODTYPE(Foo);
\endcode
*/
#define PODTYPE(Type) namespace evo { template<> struct PodType<Type> : public StaticBoolT { }; }

// BCTYPE()
/** Identify the given type as a Byte-Copy type.
 - Byte-Copy types may be directly byte-copied for improved performance.
 - Containers take advantage of these rules with performance optimizations.
 - This can only be used in global scope.
 .
 \param  Type  Type name to set as ByteCopy type.
 \see PODTYPE()
\par Example:
\code
struct Foo { };
BCTYPE(Foo);
\endcode
*/
#define BCTYPE(Type) namespace evo { template<> struct ByteCopyType<Type> : public StaticBoolT { }; }

// EVO_TYPE_SELECT()
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

// TODO?
/* Get most efficient type for passing as parameter (const-reference or POD value).
 - POD types are best passed by value
 - Other types are usually best passed by const-reference
 .
*/
//template<class T> struct PassType { /** Translated type. */ typedef typename StaticIf<IsPodType::value,T,const T&>::Type Type; };

// TODO?
//#define EVO_PASS_BYVAL(ValType) namespace evo { template<> struct PassType<ValType> { typedef ValType Type;}; }

///////////////////////////////////////////////////////////////////////////////

/** Create template to check if type has a member variable matching given signature.
 - This defines a template class that can detect the given member variable signature
 .
 \param  TypeName  Template type name to create
 \param  VarType   Member function return type
 \param  VarName   Member function name
\par Example:
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
\par Example:
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
\par Example:
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
// TODO
//#define EVO_CONTAINERTYPE_T(Type) template<class U> struct ContainerType<Type<U> > : public StaticBoolT { };
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
\par Example:
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
\par Example:
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

/** Trait to identify Evo item types.
Evo items:
 - are optimized for storage in Evo containers -- see EvoContainer
 - implement proper public default constructor, copy constructor, and assignment operator
 - can be null and implement the following methods:
   - Item is null/not-null
     - bool null() const
     - bool valid() const
   - Set as null:
        - ThisType& set()
   - Set as 0/empty if not null, leaving null status unchanged:
     - clear()
 - implement the following compare/hash methods:
   - Comparison operator:
     - bool operator==(const T&) const
   - %Compare to another item of same type:
     - int compare(const T& oth) const
   - Compute hash for item:
     - ulong hash(ulong seed=0) const
 .
 \tparam  T  Type for trait
 \see EVO_ITEM_TYPE, EVO_ITEM(), EvoContainer
*/
EVO_TRAIT_CREATE(EvoItem)

/** Identify given type as an EvoItem.
 - The type must meet the requirements of EvoItem, otherwise results are undefined
 - Must use at global scope (no namespace) since this modifies evo namespace
 .
 \param  Type  Type to identify as EvoItem
*/
#define EVO_ITEM(Type) EVO_TRAIT_NSET(evo,EvoItem,Type)

/** Identify current class/struct as an EvoItem.
 - The type must meet the requirements of EvoItem, otherwise results are undefined
 - Use in class/struct scope, anywhere a typedef is valid
 .
\par Example:
\code
class Foo {
    EVO_ITEM_TYPE;
    // ...
};
class Bar { };

void main() {
    bool b1 = IsEvoItem<Foo>::value;    // true
    bool b2 = IsEvoItem<Bar>::value;    // false
}
\endcode
*/
#define EVO_ITEM_TYPE typedef void EvoItemType

/** Check if type is an EvoItem. Value member holds result.
 \tparam  T  Type to check
*/
template<class T> class IsEvoItem {
    template<class U> struct SFINAE { };
    template<class U> static char Test(SFINAE<typename U::EvoItemType>*);
    template<class U> static long Test(...);
public:
    typedef IsEvoItem<T> Type;    ///< This type
    static const bool value = (!IsPodType<T>::value && (sizeof(Test<T>(0)) == sizeof(char) || EvoItem<T>::value)); ///< Result value
};

///////////////////////////////////////////////////////////////////////////////

/** Trait to identify Evo container types.
Evo containers:
 - hold a collection of items of any type
   - if item isn't an EvoItem then it will be treated as raw bytes for compare/hash
 - are also Evo Items -- see EvoItem
 - define the following general subtypes:
   - ThisType: This container type
   - Key:      Key type for getting an item/value
   - Value:    Value type for key
   - Item:     Item type used by iterators use (same as Value for list types, Key/Value pair for map types)
 - define iterators:
   - Iter:  const iterator
   - IterM: mutable iterator
 - can be empty or null and implement the following methods, in addition to EvoItem methods:
   - empty():    Container is empty or null
   - setempty(): Remove all items and set as not null
   - clear():    Remove all items but leave null status unchanged
 .
*/
EVO_TRAIT_CREATE(EvoContainer)

/** Identify given type as an EvoContainer.
 - The type must meet the requirements of EvoContainer, otherwise results are undefined
 - Must use at global scope (no namespace) since this modifies evo namespace
 .
 \param  Type  Type to identify as EvoContainer
*/
#define EVO_CONTAINER(Type) EVO_TRAIT_NSET(evo,EvoContainer,Type)

/** Identify current class/struct as an EvoContainer.
 - The type must meet the requirements of EvoContainer, otherwise results are undefined
 - Use in class/struct scope, anywhere a typedef is valid
 .
\par Example:
\code
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
#define EVO_CONTAINER_TYPE typedef void EvoItemType; typedef void EvoContainerType

/** Check if type is an EvoContainer. Value member holds result.
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
//@}
} // Namespace: evo
#endif
