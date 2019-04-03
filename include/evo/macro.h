// Evo C++ Library
/* Copyright 2019 Justin Crowell
Distributed under the BSD 2-Clause License -- see included file LICENSE.txt for details.
*/
///////////////////////////////////////////////////////////////////////////////
/** \file macro.h Evo advanced preprocessor macros. */
#pragma once
#ifndef INCL_evo_macro_h
#define INCL_evo_macro_h

#include "impl/sys.h"

///////////////////////////////////////////////////////////////////////////////

/** \cond impl */
#define EVO_STRINGIFY_IMPL(X) #X
#define EVO_CONCAT_IMPL(A, B) A##B

#if defined(_MSC_VER)
    #define EVO_COUNT_ARGS_IMPL_AUGMENT(...) DUMMY, __VA_ARGS__
    #define EVO_COUNT_ARGS_IMPL_2(A1,A2,A3, A4,A5,A6,A7,A8,A9,A10,A11,A12,A13,A14,A15,A16,A17,A18,A19,A20, \
                                    A21,A22,A23,A24,A25,A26,A27,A28,A29,A30,A31,A32,A33,A34,A35,A36,A37,A38,A39,A40, \
                                    A41,A42,A43,A44,A45,A46,A47,A48,A49,A50,A51,A52,A53,A54,A55,A56,A57,A58,A59,A60,N, ...) N
    #define EVO_COUNT_ARGS_IMPL(...) EVO_EXPAND(EVO_COUNT_ARGS_IMPL_2(__VA_ARGS__, \
                                    59,58,57,56,55,54,53,52,51,50,49,48,47,46,45,44,43,42,41,40, \
                                    39,38,37,36,35,34,33,32,31,30,29,28,27,26,25,24,23,22,21,20, \
                                    19,18,17,16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0))
#else
    #define EVO_COUNT_ARGS_IMPL(A0,A1,A2,A3, A4,A5,A6,A7,A8,A9,A10,A11,A12,A13,A14,A15,A16,A17,A18,A19,A20, \
                                A21,A22,A23,A24,A25,A26,A27,A28,A29,A30,A31,A32,A33,A34,A35,A36,A37,A38,A39,A40, \
                                A41,A42,A43,A44,A45,A46,A47,A48,A49,A50,A51,A52,A53,A54,A55,A56,A57,A58,A59,A60,N, ...) N
#endif

#define EVO_MAP_FIELDS_IMPL_BASE_DEF(T, N, K) T N
#define EVO_MAP_FIELDS_IMPL_BASE_LOAD(T, N, K) N = evo::Convert<evo::SubString,T>::value(evo::lookupsub(map, K))
#define EVO_MAP_FIELDS_IMPL_BASE_SAVE(T, N, K) evo::Convert<evo::String,T>::set(map[K], N)
#define EVO_MAP_FIELDS_IMPL_BASE_DUMP(T, N, K) out << K << ':' << N << evo::NL
#define EVO_MAP_FIELDS_IMPL_BASE_DUMP2(T, N, K) out << EVO_STRINGIFY_IMPL(N) << ':' << N << evo::NL

#define EVO_MAP_FIELDS_IMPL_2(BASE, T, N) BASE(T, N, EVO_STRINGIFY(N));
#define EVO_MAP_FIELDS_IMPL_4(BASE, T, N, ...) BASE(T, N, EVO_STRINGIFY(N)); EVO_EXPAND(EVO_MAP_FIELDS_IMPL_2(BASE, __VA_ARGS__))
#define EVO_MAP_FIELDS_IMPL_6(BASE, T, N, ...) BASE(T, N, EVO_STRINGIFY(N)); EVO_EXPAND(EVO_MAP_FIELDS_IMPL_4(BASE, __VA_ARGS__))
#define EVO_MAP_FIELDS_IMPL_8(BASE, T, N, ...) BASE(T, N, EVO_STRINGIFY(N)); EVO_EXPAND(EVO_MAP_FIELDS_IMPL_6(BASE, __VA_ARGS__))
#define EVO_MAP_FIELDS_IMPL_10(BASE, T, N, ...) BASE(T, N, EVO_STRINGIFY(N)); EVO_EXPAND(EVO_MAP_FIELDS_IMPL_8(BASE, __VA_ARGS__))
#define EVO_MAP_FIELDS_IMPL_12(BASE, T, N, ...) BASE(T, N, EVO_STRINGIFY(N)); EVO_EXPAND(EVO_MAP_FIELDS_IMPL_10(BASE, __VA_ARGS__))
#define EVO_MAP_FIELDS_IMPL_14(BASE, T, N, ...) BASE(T, N, EVO_STRINGIFY(N)); EVO_EXPAND(EVO_MAP_FIELDS_IMPL_12(BASE, __VA_ARGS__))
#define EVO_MAP_FIELDS_IMPL_16(BASE, T, N, ...) BASE(T, N, EVO_STRINGIFY(N)); EVO_EXPAND(EVO_MAP_FIELDS_IMPL_14(BASE, __VA_ARGS__))
#define EVO_MAP_FIELDS_IMPL_18(BASE, T, N, ...) BASE(T, N, EVO_STRINGIFY(N)); EVO_EXPAND(EVO_MAP_FIELDS_IMPL_16(BASE, __VA_ARGS__))
#define EVO_MAP_FIELDS_IMPL_20(BASE, T, N, ...) BASE(T, N, EVO_STRINGIFY(N)); EVO_EXPAND(EVO_MAP_FIELDS_IMPL_18(BASE, __VA_ARGS__))
#define EVO_MAP_FIELDS_IMPL_22(BASE, T, N, ...) BASE(T, N, EVO_STRINGIFY(N)); EVO_EXPAND(EVO_MAP_FIELDS_IMPL_20(BASE, __VA_ARGS__))
#define EVO_MAP_FIELDS_IMPL_24(BASE, T, N, ...) BASE(T, N, EVO_STRINGIFY(N)); EVO_EXPAND(EVO_MAP_FIELDS_IMPL_22(BASE, __VA_ARGS__))
#define EVO_MAP_FIELDS_IMPL_26(BASE, T, N, ...) BASE(T, N, EVO_STRINGIFY(N)); EVO_EXPAND(EVO_MAP_FIELDS_IMPL_24(BASE, __VA_ARGS__))
#define EVO_MAP_FIELDS_IMPL_28(BASE, T, N, ...) BASE(T, N, EVO_STRINGIFY(N)); EVO_EXPAND(EVO_MAP_FIELDS_IMPL_26(BASE, __VA_ARGS__))
#define EVO_MAP_FIELDS_IMPL_30(BASE, T, N, ...) BASE(T, N, EVO_STRINGIFY(N)); EVO_EXPAND(EVO_MAP_FIELDS_IMPL_28(BASE, __VA_ARGS__))
#define EVO_MAP_FIELDS_IMPL_32(BASE, T, N, ...) BASE(T, N, EVO_STRINGIFY(N)); EVO_EXPAND(EVO_MAP_FIELDS_IMPL_30(BASE, __VA_ARGS__))
#define EVO_MAP_FIELDS_IMPL_34(BASE, T, N, ...) BASE(T, N, EVO_STRINGIFY(N)); EVO_EXPAND(EVO_MAP_FIELDS_IMPL_32(BASE, __VA_ARGS__))
#define EVO_MAP_FIELDS_IMPL_36(BASE, T, N, ...) BASE(T, N, EVO_STRINGIFY(N)); EVO_EXPAND(EVO_MAP_FIELDS_IMPL_34(BASE, __VA_ARGS__))
#define EVO_MAP_FIELDS_IMPL_38(BASE, T, N, ...) BASE(T, N, EVO_STRINGIFY(N)); EVO_EXPAND(EVO_MAP_FIELDS_IMPL_36(BASE, __VA_ARGS__))
#define EVO_MAP_FIELDS_IMPL_40(BASE, T, N, ...) BASE(T, N, EVO_STRINGIFY(N)); EVO_EXPAND(EVO_MAP_FIELDS_IMPL_38(BASE, __VA_ARGS__))

#define EVO_MAP_FIELDS_IMPL2_3(BASE, T, N, K) BASE(T, N, K);
#define EVO_MAP_FIELDS_IMPL2_6(BASE, T, N, K, ...) BASE(T, N, K); EVO_EXPAND(EVO_MAP_FIELDS_IMPL2_3(BASE, __VA_ARGS__))
#define EVO_MAP_FIELDS_IMPL2_9(BASE, T, N, K, ...) BASE(T, N, K); EVO_EXPAND(EVO_MAP_FIELDS_IMPL2_6(BASE, __VA_ARGS__))
#define EVO_MAP_FIELDS_IMPL2_12(BASE, T, N, K, ...) BASE(T, N, K); EVO_EXPAND(EVO_MAP_FIELDS_IMPL2_9(BASE, __VA_ARGS__))
#define EVO_MAP_FIELDS_IMPL2_15(BASE, T, N, K, ...) BASE(T, N, K); EVO_EXPAND(EVO_MAP_FIELDS_IMPL2_12(BASE, __VA_ARGS__))
#define EVO_MAP_FIELDS_IMPL2_18(BASE, T, N, K, ...) BASE(T, N, K); EVO_EXPAND(EVO_MAP_FIELDS_IMPL2_15(BASE, __VA_ARGS__))
#define EVO_MAP_FIELDS_IMPL2_21(BASE, T, N, K, ...) BASE(T, N, K); EVO_EXPAND(EVO_MAP_FIELDS_IMPL2_18(BASE, __VA_ARGS__))
#define EVO_MAP_FIELDS_IMPL2_24(BASE, T, N, K, ...) BASE(T, N, K); EVO_EXPAND(EVO_MAP_FIELDS_IMPL2_21(BASE, __VA_ARGS__))
#define EVO_MAP_FIELDS_IMPL2_27(BASE, T, N, K, ...) BASE(T, N, K); EVO_EXPAND(EVO_MAP_FIELDS_IMPL2_24(BASE, __VA_ARGS__))
#define EVO_MAP_FIELDS_IMPL2_30(BASE, T, N, K, ...) BASE(T, N, K); EVO_EXPAND(EVO_MAP_FIELDS_IMPL2_27(BASE, __VA_ARGS__))
#define EVO_MAP_FIELDS_IMPL2_33(BASE, T, N, K, ...) BASE(T, N, K); EVO_EXPAND(EVO_MAP_FIELDS_IMPL2_30(BASE, __VA_ARGS__))
#define EVO_MAP_FIELDS_IMPL2_36(BASE, T, N, K, ...) BASE(T, N, K); EVO_EXPAND(EVO_MAP_FIELDS_IMPL2_33(BASE, __VA_ARGS__))
#define EVO_MAP_FIELDS_IMPL2_39(BASE, T, N, K, ...) BASE(T, N, K); EVO_EXPAND(EVO_MAP_FIELDS_IMPL2_36(BASE, __VA_ARGS__))
#define EVO_MAP_FIELDS_IMPL2_42(BASE, T, N, K, ...) BASE(T, N, K); EVO_EXPAND(EVO_MAP_FIELDS_IMPL2_39(BASE, __VA_ARGS__))
#define EVO_MAP_FIELDS_IMPL2_45(BASE, T, N, K, ...) BASE(T, N, K); EVO_EXPAND(EVO_MAP_FIELDS_IMPL2_42(BASE, __VA_ARGS__))
#define EVO_MAP_FIELDS_IMPL2_48(BASE, T, N, K, ...) BASE(T, N, K); EVO_EXPAND(EVO_MAP_FIELDS_IMPL2_45(BASE, __VA_ARGS__))
#define EVO_MAP_FIELDS_IMPL2_51(BASE, T, N, K, ...) BASE(T, N, K); EVO_EXPAND(EVO_MAP_FIELDS_IMPL2_48(BASE, __VA_ARGS__))
#define EVO_MAP_FIELDS_IMPL2_54(BASE, T, N, K, ...) BASE(T, N, K); EVO_EXPAND(EVO_MAP_FIELDS_IMPL2_51(BASE, __VA_ARGS__))
#define EVO_MAP_FIELDS_IMPL2_57(BASE, T, N, K, ...) BASE(T, N, K); EVO_EXPAND(EVO_MAP_FIELDS_IMPL2_54(BASE, __VA_ARGS__))
#define EVO_MAP_FIELDS_IMPL2_60(BASE, T, N, K, ...) BASE(T, N, K); EVO_EXPAND(EVO_MAP_FIELDS_IMPL2_57(BASE, __VA_ARGS__))
/** \endcond */

/** Make argument token a string literal using the preprocessor.
 \param  X  Argument to make a string literal
*/
#define EVO_STRINGIFY(X) EVO_STRINGIFY_IMPL(X)

/** Concatenate two tokens into a single token using the preprocessor.
 \param  A  Token beginning
 \param  B  Token end
*/
#define EVO_CONCAT(A, B) EVO_CONCAT_IMPL(A, B)

/** Expand argument to itself.
 - This is a trick used to make sure the argument is used as-is, where in some cases it might get mangled from nesting macros (which often happens with MSVC, especially when passing __VA_ARGS__ to another macro)
 .
 \param  X  Parameter to expand
*/
#define EVO_EXPAND(X) X

/** Count number of arguments passed to macro.
 - This usually requires at least 1 argument, otherwise some compilers (gcc, clang) have issues when C++11 (or newer) is enabled
 - This can be used to construct a macro name with the number of arguments used, ex: `EVO_CONCAT(FOO_, EVO_COUNT_ARGS(a))` resolves to `FOO_1`
*/
#if defined(_MSC_VER)
    #define EVO_COUNT_ARGS(...) EVO_COUNT_ARGS_IMPL(EVO_COUNT_ARGS_IMPL_AUGMENT(__VA_ARGS__))
#else
    #define EVO_COUNT_ARGS(...) EVO_EXPAND(EVO_COUNT_ARGS_IMPL(0, ## __VA_ARGS__, \
                                      60,59,58,57,56,55,54,53,52,51,50,49,48,47,46,45,44,43,42,41,40, \
                                      39,38,37,36,35,34,33,32,31,30,29,28,27,26,25,24,23,22,21,20, \
                                      19,18,17,16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0))
#endif

/** Create a list of fields and templated load/save/dump helper methods.
 - This is normally used inside a `struct` and defines member variables and methods for it
 - This defines the fields listed, then defines these helper methods:
   - `template<class T> void load(const T& map)` -- load from map into fields
   - `template<class T> void save(T& map) const` -- save from fields to map
   - `template<class T> void dump(T& out) const` -- dump fields to output stream/string
 - This supports a maximum of 20 fields (any more will give compiler errors) -- for more use multiple `structs`
 - If you get a bunch of compiler errors using this, double check:
   - There's a _pair of arguments for each field_
     - The first field argument is a valid type, and the second is a valid field (variable) name
   - Any map passed to load() or save() <i>must use the \link evo::String String\endlink type for both keys and values</i>
   - Any stream or string passed to dump() supports all field types with operator<<()
   - All arguments are comma separated
   - Last argument doesn't end with a comma
 - See also: EVO_MAP_FIELDS2()
 .
 \param  ...  Fields to define, where each field is a pair of arguments as: type, name

\par Example

\code{.cpp}
#include <evo/macro.h>
#include <evo/string.h>
#include <evo/maplist.h>
#include <evo/io.h>
using namespace evo;

struct MyFields {
    EVO_MAP_FIELDS(
        bool, flag,
        int, num,
        ULong, unum,
        String, str
    )
};

int main() {
    Console& c = con();

    // Populate map with fields
    StrMapList map;
    map["flag"] = "true";
    map["num"]  = "123";
    map["unum"] = "999";
    map["str"]  = "Hello";

    // Load from map to struct
    MyFields data;
    data.load(map);
    data.dump(c.out << "Dump:" << NL);

    // Clear map and repopulate from struct
    map.clear();
    data.save(map);

    // Print map items
    c.out << NL << "Map:" << NL;
    for (StrMapList::Iter iter(map); iter; ++iter)
        c.out << iter->key() << " = " << iter->value() << NL;

    return 0;
}
\endcode

\code{.unparsed}
Dump:
flag:true
num:123
unum:999
str:Hello

Map:
flag = true
num = 123
str = Hello
unum = 999
\endcode
*/
#define EVO_MAP_FIELDS(...) \
    EVO_EXPAND(EVO_CONCAT(EVO_MAP_FIELDS_IMPL_, EVO_COUNT_ARGS(__VA_ARGS__))(EVO_MAP_FIELDS_IMPL_BASE_DEF, __VA_ARGS__)) \
    template<class C> void load(const C& map) { \
        EVO_EXPAND(EVO_CONCAT(EVO_MAP_FIELDS_IMPL_, EVO_COUNT_ARGS(__VA_ARGS__))(EVO_MAP_FIELDS_IMPL_BASE_LOAD, __VA_ARGS__)) \
    } \
    template<class C> void save(C& map) const { \
        EVO_EXPAND(EVO_CONCAT(EVO_MAP_FIELDS_IMPL_, EVO_COUNT_ARGS(__VA_ARGS__))(EVO_MAP_FIELDS_IMPL_BASE_SAVE, __VA_ARGS__)) \
    } \
    template<class C> void dump(C& out) const { \
        EVO_EXPAND(EVO_CONCAT(EVO_MAP_FIELDS_IMPL_, EVO_COUNT_ARGS(__VA_ARGS__))(EVO_MAP_FIELDS_IMPL_BASE_DUMP, __VA_ARGS__)) \
    }

/** Create a list of fields and templated load/save/dump helper methods.
 - This is the same as EVO_MAP_FIELDS() but takes an additional argument per field to specify the map key string for the field
 .
 \param  ...  Fields to define, where each field is 3 arguments as: type, name, key

\par Example

\code{.cpp}
#include <evo/macro.h>
#include <evo/string.h>
#include <evo/maplist.h>
#include <evo/io.h>
using namespace evo;

struct MyFields {
    EVO_MAP_FIELDS_KEY(
        bool, flag, "flag-key",
        int, num, "num-key",
        ULong, unum, "unum-key",
        String, str, "str-key"
    )
};

int main() {
    Console& c = con();

    // Populate map with fields
    StrMapList map;
    map["flag-key"] = "true";
    map["num-key"]  = "123";
    map["unum-key"] = "999";
    map["str-key"]  = "Hello";

    // Load from map to struct
    MyFields data;
    data.load(map);
    data.dump(c.out << "Dump:" << NL);

    // Clear map and repopulate from struct
    map.clear();
    data.save(map);

    // Print map items
    c.out << NL << "Map:" << NL;
    for (StrMapList::Iter iter(map); iter; ++iter)
        c.out << iter->key() << " = " << iter->value() << NL;

    return 0;
}
\endcode

\code{.unparsed}
Dump:
flag:true
num:123
unum:999
str:Hello

Map:
flag-key = true
num-key = 123
str-key = Hello
unum-key = 999
\endcode
*/
#define EVO_MAP_FIELDS_KEY(...) \
    EVO_EXPAND(EVO_CONCAT(EVO_MAP_FIELDS_IMPL2_, EVO_COUNT_ARGS(__VA_ARGS__))(EVO_MAP_FIELDS_IMPL_BASE_DEF, __VA_ARGS__)) \
    template<class M> void load(const M& map) { \
        EVO_EXPAND(EVO_CONCAT(EVO_MAP_FIELDS_IMPL2_, EVO_COUNT_ARGS(__VA_ARGS__))(EVO_MAP_FIELDS_IMPL_BASE_LOAD, __VA_ARGS__)) \
    } \
    template<class M> void save(M& map) const { \
        EVO_EXPAND(EVO_CONCAT(EVO_MAP_FIELDS_IMPL2_, EVO_COUNT_ARGS(__VA_ARGS__))(EVO_MAP_FIELDS_IMPL_BASE_SAVE, __VA_ARGS__)) \
    } \
    template<class C> void dump(C& out) const { \
        EVO_EXPAND(EVO_CONCAT(EVO_MAP_FIELDS_IMPL2_, EVO_COUNT_ARGS(__VA_ARGS__))(EVO_MAP_FIELDS_IMPL_BASE_DUMP2, __VA_ARGS__)) \
    }

///////////////////////////////////////////////////////////////////////////////
#endif
