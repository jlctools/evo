// Evo C++ Library
/* Copyright 2019 Justin Crowell
Distributed under the BSD 2-Clause License -- see included file LICENSE.txt for details.
*/
///////////////////////////////////////////////////////////////////////////////
/** \file enum.h Evo enum helpers. */
#pragma once
#ifndef INCL_evo_enum_h
#define INCL_evo_enum_h

#include "substring.h"

namespace evo {
/** \addtogroup EvoContainers */
//@{

///////////////////////////////////////////////////////////////////////////////

/** Expanded EnumIterator used with enum traits.
 - This type is not normally used directly as enum traits will have a typedef for it with `T::Iter`
 - Create enum traits type with a helper macro like: EVO_ENUM_MAP_PREFIXED() and EVO_ENUM_CLASS_MAP()
 .
 \tparam  T  Enum traits type to use

\par Example

\code
#include <evo/enum.h>
#include <evo/io.h>
using namespace evo;

enum Color {
    cUNKNOWN,
    cBLUE,
    cGREEN,
    cRED,
    cENUM_END
};

EVO_ENUM_MAP_PREFIXED(Color, c,
    // Must be sorted and match enum values above
    "blue",
    "green",
    "red"
);

int main() {
    Console& c = con();
    for (ColorEnum::Iter iter; iter; ++iter)
        c.out << iter.value_str() << NL;

    return 0;
}
\endcode

Output:
\code{.unparsed}
blue
green
red
\endcode
*/
template<class T>
struct EnumMapIterator : EnumIterator<typename T::Type, (int)T::FIRST, (int)T::LAST> {
    typedef EnumMapIterator<T> This;
    typedef EnumIterator<typename T::Type, (int)T::FIRST, (int)T::LAST> BaseType;
#if defined(EVO_OLDCC2)
	typedef typename BaseType::EnumType EnumType;
#else
    using typename BaseType::EnumType;
#endif

    // Documented by parents

    EnumMapIterator() {
    }

    EnumMapIterator(const This& src) : EnumIterator<typename T::Type, (int)T::FIRST, (int)T::LAST>(src) {
    }

    EnumMapIterator(EnumType value) : EnumIterator<typename T::Type, (int)T::FIRST, (int)T::LAST>(value) {
    }

    EnumMapIterator(IteratorPos pos) : EnumIterator<typename T::Type, (int)T::FIRST, (int)T::LAST>(pos) {
    }

    This& operator=(const This& src) {
        BaseType::operator=(src);
        return *this;
    }

    This& operator=(EnumType value) {
        BaseType::operator=(value);
        return *this;
    }

    This& operator=(IteratorPos pos) {
        BaseType::operator=(pos);
        return *this;
    }

    /** Get current enum string value.
     \return  Current enum string value, null if at end
    */
    SubString value_str() const {
        SubString result;
        if (!BaseType::end_)
            result = T::get_string(BaseType::value_);
        return result;
    }
};

///////////////////////////////////////////////////////////////////////////////

/** Helper for creating enum string/value mappers with explicit first/last/unknown values.
 - \#include <evo/enum.h>
 - See better alternative: EVO_ENUM_MAP_PREFIXED()
 - This creates a struct with traits that uses \link evo::SubStringMapList::find_enum() find_enum()\endlink and \link evo::SubStringMapList::get_enum_string() get_enum_string()\endlink
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
        typedef EnumMapIterator< ENUM ## Enum > Iter; \
        static const ENUM FIRST   = FIRST_VAL; \
        static const ENUM LAST    = LAST_VAL; \
        static const ENUM UNKNOWN = UNKNOWN_VAL; \
        static const evo::SubStringMapList& map() { \
            static const evo::SubString LIST[] = { __VA_ARGS__ }; \
            static const evo::SubStringMapList MAP(LIST, evo::fixed_array_size(LIST)); \
            return MAP; \
        } \
        static ENUM get_enum(const evo::SubString& key) \
            { return map().find_enum<ENUM>(key, FIRST, LAST, UNKNOWN); } \
        static ENUM get_enum(int val) \
            { return (val < (int)FIRST || val > (int)LAST ? UNKNOWN : (ENUM)val); } \
        static int get_int(ENUM val) \
            { return (int)val; } \
        static SubString get_string(ENUM val) \
            { return map().get_enum_string(val, FIRST, LAST); } \
    }

/** Helper for creating enum string/value mappers with explicit first/last/unknown values, with unsorted enum remapped to sorted values.
 - \#include <evo/enum.h>
 - This is a variant of EVO_ENUM_MAP() for an unsorted enum
 - This uses `REMAP_ARRAY` to map an unsorted enum to sorted values -- the data should be constant and static
 .
 \param  ENUM         Enum type to create traits for
 \param  FIRST_VAL    First enum value to map to, maps to first string
 \param  LAST_VAL     Last enum value to map to, maps to last string -- must be >= first_enum
 \param  UNKNOWN_VAL  Unknown enum value to use if key not found or result out of range
 \param  REMAP_ARRAY  Pointer to array of `ENUM` values sorted so they match the sorted string literals that follow
 \param  ...          _Sorted_ list of string literals to map to each enum value -- ex: `"a", "b", "c"`
*/
#define EVO_ENUM_REMAP(ENUM, FIRST_VAL, LAST_VAL, UNKNOWN_VAL, REMAP_ARRAY, ...) \
    struct ENUM ## Enum { \
        typedef ENUM Type; \
        typedef EnumMapIterator< ENUM ## Enum > Iter; \
        static const ENUM FIRST   = FIRST_VAL; \
        static const ENUM LAST    = LAST_VAL; \
        static const ENUM UNKNOWN = UNKNOWN_VAL; \
        static const ENUM* get_remap_array() { \
            return REMAP_ARRAY; \
        } \
        static const SizeT* get_reverse_remap_array() { \
            static const SubStringMapList::ReverseRemap< ENUM ## Enum > REVERSE(get_remap_array()); \
            return REVERSE.array; \
        } \
        static const evo::SubStringMapList& map() { \
            static const evo::SubString LIST[] = { __VA_ARGS__ }; \
            static const evo::SubStringMapList MAP(LIST, evo::fixed_array_size(LIST)); \
            return MAP; \
        } \
        static ENUM get_enum(const evo::SubString& key) \
            { return map().find_enum_remap<Type>(get_remap_array(), key, FIRST, LAST, UNKNOWN); } \
        static ENUM get_enum(int val) \
            { return (val < (int)FIRST || val > (int)LAST ? UNKNOWN : (ENUM)val); } \
        static int get_int(ENUM val) \
            { return (int)val; } \
        static SubString get_string(ENUM val) \
            { return map().get_enum_string_remap(get_reverse_remap_array(), val, FIRST, LAST); } \
    }

/** Helper for creating enum string/value mappers with prefixed enum values.
 - \#include <evo/enum.h>
 - This creates a struct with traits that uses \link evo::SubStringMapList::find_enum_class() find_enum_class()\endlink and \link evo::SubStringMapList::get_enum_class_string() get_enum_class_string()\endlink
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
    EVO_ENUM_MAP(ENUM, (ENUM)((int)(PREFIX ## UNKNOWN) + 1), (ENUM)((int)(PREFIX ## ENUM_END) - 1), PREFIX ## UNKNOWN, __VA_ARGS__)

/** Helper for creating enum string/value mappers with prefixed enum values, with unsorted enum remapped to sorted values.
 - \#include <evo/enum.h>
 - This is a variant of EVO_ENUM_MAP_PREFIXED() for an unsorted enum
 - This uses `REMAP_ARRAY` to map an unsorted enum to sorted values -- the data should be constant and static
 .
 \param  ENUM         Enum type to create mappings for
 \param  PREFIX       Prefix for enum values, used to find UNKNOWN and ENUM_END values
 \param  REMAP_ARRAY  Pointer to array of `ENUM` values sorted so they match the sorted string literals that follow
 \param  ...          _Sorted_ list of string literals to map to each enum value -- ex: `"a", "b", "c"`
*/
#define EVO_ENUM_REMAP_PREFIXED(ENUM, PREFIX, REMAP_ARRAY, ...) \
    EVO_ENUM_REMAP(ENUM, (ENUM)((int)(PREFIX ## UNKNOWN) + 1), (ENUM)((int)(PREFIX ## ENUM_END) - 1), PREFIX ## UNKNOWN, REMAP_ARRAY, __VA_ARGS__)

#if defined(EVO_CPP11)
    /** Helper for creating enum class string/value mappers (C++11).
     - \#include <evo/enum.h>
     - This creates a struct with traits that uses \link evo::SubStringMapList::find_enum_class() find_enum_class()\endlink and \link evo::SubStringMapList::get_enum_class_string() get_enum_class_string()\endlink
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
            typedef ENUM Type; \
            typedef EnumMapIterator< ENUM ## Enum > Iter; \
            static const ENUM FIRST = (ENUM)((int)(ENUM::UNKNOWN) + 1); \
            static const ENUM LAST = (ENUM)((int)(ENUM::ENUM_END) - 1); \
            static const evo::SubStringMapList& map() { \
                static const evo::SubString LIST[] = { __VA_ARGS__ }; \
                static const evo::SubStringMapList MAP(LIST, evo::fixed_array_size(LIST)); \
                return MAP; \
            } \
            static ENUM get_enum(const evo::SubString& key) \
                { return map().find_enum_class<ENUM>(key); } \
            static ENUM get_enum(int val) \
                { return (val <= (int)ENUM::UNKNOWN || val >= (int)ENUM::ENUM_END ? ENUM::UNKNOWN : (ENUM)val); } \
            static int get_int(ENUM val) \
                { return (int)val; } \
            static SubString get_string(ENUM val) \
                { return map().get_enum_class_string(val); } \
        }

    /** Helper for creating enum class string/value mappers, with unsorted enum remapped to sorted values. (C++11).
     - \#include <evo/enum.h>
     - This is a variant of EVO_ENUM_CLASS_MAP() for an unsorted enum
     - This uses `REMAP_ARRAY` to map an unsorted enum to sorted values -- the data should be constant and static
     .
     \param  ENUM         Enum type to create mappings for
     \param  REMAP_ARRAY  Pointer to array of `ENUM` values sorted so they match the sorted string literals that follow
     \param  ...          _Sorted_ list of string literals to map to each enum value -- ex: `"a", "b", "c"`
    */
    #define EVO_ENUM_CLASS_REMAP(ENUM, REMAP_ARRAY, ...) \
        struct ENUM ## Enum { \
            typedef ENUM Type; \
            typedef EnumMapIterator< ENUM ## Enum > Iter; \
            static const ENUM FIRST = (ENUM)((int)(ENUM::UNKNOWN) + 1); \
            static const ENUM LAST = (ENUM)((int)(ENUM::ENUM_END) - 1); \
            static const ENUM* get_remap_array() { \
                return REMAP_ARRAY; \
            } \
            static const SizeT* get_reverse_remap_array() { \
                static const SubStringMapList::ReverseRemap< ENUM ## Enum > REVERSE(get_remap_array()); \
                return REVERSE.array; \
            } \
            static const evo::SubStringMapList& map() { \
                static const evo::SubString LIST[] = { __VA_ARGS__ }; \
                static const evo::SubStringMapList MAP(LIST, evo::fixed_array_size(LIST)); \
                return MAP; \
            } \
            static ENUM get_enum(const evo::SubString& key) \
                { return map().find_enum_remap<Type>(get_remap_array(), key, FIRST, LAST, Type::UNKNOWN); } \
            static ENUM get_enum(int val) \
                { return (val <= (int)ENUM::UNKNOWN || val >= (int)ENUM::ENUM_END ? ENUM::UNKNOWN : (ENUM)val); } \
            static int get_int(ENUM val) \
                { return (int)val; } \
            static SubString get_string(ENUM val) \
                { return map().get_enum_string_remap(get_reverse_remap_array(), val, FIRST, LAST); } \
        }
#endif

///////////////////////////////////////////////////////////////////////////////

/** Helper for creating enum trait mappers.
 - \#include <evo/enum.h>
 - This creates a struct with a static array of `TRAITS` values mapped to enum values
 - The created struct type is named after ENUM with suffix "EnumTraits", and has the helper function:
     - static const TRAITS& get(ENUM value)
       - Get traits for enum value
   .
 - See: \ref EnumConversion "Enum Conversion"
 .
 \param  ENUM         Enum type to create traits for
 \param  TRAITS       Traits type to create for each enum value, usually named after `ENUM` with suffix `Traits`
 \param  START_VAL    Start enum value used as the starting point of enum values -- usually this is the "UNKNOWN" enum value
 \param  ...          %List of initializers for TRAITS values matching enum values starting at `START_VAL`
*/
#define EVO_ENUM_TRAITS(ENUM, TRAITS, START_VAL, ...) \
    struct ENUM ## EnumTraits { \
        typedef ENUM Type; \
        static const ENUM START = START_VAL; \
        static const TRAITS& get(ENUM value) { \
            return data()[(int)value - (int)START]; \
        } \
    private: \
        static const TRAITS* data() { \
            static const TRAITS DATA[] = { __VA_ARGS__ }; \
            return DATA; \
        } \
    };

#if defined(EVO_CPP11)
    /** Helper for creating enum class trait mappers (C++11).
     - \#include <evo/enum.h>
     - This creates a struct with a static array of `TRAITS` values mapped to enum values
     - The created struct type is named after ENUM with suffix "EnumTraits", and has the helper function:
         - static const TRAITS& get(ENUM value)
           - Get traits for enum value
       .
     - If you want to specify a different starting value use EVO_ENUM_TRAITS(), which works on enum class too
     - See: \ref EnumConversion "Enum Conversion"
     .
     \param  ENUM    Enum type to create traits for
     \param  TRAITS  Traits type to create for each enum value, usually named after `ENUM` with suffix `Traits`
     \param  ...     %List of initializers for TRAITS values matching enum class values starting at `ENUM::UNKNOWN`
    */
    #define EVO_ENUM_CLASS_TRAITS(ENUM, TRAITS, ...) \
        struct ENUM ## EnumTraits { \
            typedef ENUM Type; \
            static const ENUM START = ENUM::UNKNOWN; \
            static const TRAITS& get(ENUM value) { \
                return data()[(int)value - (int)START]; \
            } \
        private: \
            static const TRAITS* data() { \
                static const TRAITS DATA[] = { __VA_ARGS__ }; \
                return DATA; \
            } \
        };
#endif

///////////////////////////////////////////////////////////////////////////////
//@}
}
#endif
