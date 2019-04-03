// Evo C++ Library
/* Copyright 2019 Justin Crowell
Distributed under the BSD 2-Clause License -- see included file LICENSE.txt for details.
*/
///////////////////////////////////////////////////////////////////////////////
/** \file pair.h Evo Pair class. */
#pragma once
#ifndef INCL_evo_pair_h
#define INCL_evo_pair_h

#include "impl/container.h"

namespace evo {
/** \addtogroup EvoContainers */
//@{

///////////////////////////////////////////////////////////////////////////////

/** Stores a key/value pair of independent objects or values.
 - POD types are initialzied to 0 by default
 - Access first item directly, or with a() or key()
 - Access second item directly, or with b() or value()
 .

C++11:
 - Move semantics

\tparam  TA  First (key) type
\tparam  TB  Second (value) type
*/
template<class TA, class TB>
struct Pair {
    typedef Pair<TA, TB> This;  ///< This type

    typedef TA A;               ///< First value type (same as Key)
    typedef TA First;           ///< First value type (same as Key)
    typedef TA Key;             ///< %Pair key type (same as A)
    typedef TB B;               ///< Second value type (same as Value)
    typedef TB Second;          ///< Second value type (same as Value)
    typedef TB Value;           ///< %Pair value type (same as B)

    TA first;       ///< First value (same as a() and key())
    TB second;      ///< Second value (same as b() and value())

    /** Constructor. */
    Pair() {
        DataCopy<TA>::set_default_pod(first);
        DataCopy<TB>::set_default_pod(second);
    }

    /** Constructor.
     \param  a  First value (pair key)
    */
    Pair(const TA& a) : first(a) {
        DataCopy<TB>::set_default_pod(second);
    }

    /** Constructor.
     \param  a  First value (pair key)
     \param  b  Second value (pair value)
    */
    Pair(const TA& a, const TB& b) : first(a), second(b) {
    }

    /** Copy constructor.
     \param  src  %Pair to copy
    */
    Pair(const This& src) : first(src.first), second(src.second) {
    }

    /** Assignment operator.
     \param  src  %Pair to copy
     \return      This
    */
    This& operator=(const This& src) {
        first  = src.first;
        second = src.second;
        return *this;
    }

#if defined(EVO_CPP11)
    /** Move constructor (C++11).
     \param  src  Source to move
    */
    Pair(This&& src) : first(std::move(src.first)), second(std::move(src.second)) {
    }

    /** Move assignment operator (C++11).
     \param  src  Source to move
     \return      This
    */
    This& operator=(This&& src) {
        first = std::move(src.first);
        second = std::move(src.second);
        return *this;
    }
#endif

    /** Get first value (const).
     \return  First value
    */
    const A& a() const
        { return first; }

    /** Get first value.
     \return  First value
    */
    A& a()
        { return first; }

    /** Get key for pair (first value) (const).
     \return  First value
    */
    const Key& key() const
        { return first; }

    /** Get key for pair (first value).
     \return  First value
    */
    Key& key()
        { return first; }

    /** Get second value (const).
     \return  Second value
    */
    const B& b() const
        { return second; }

    /** Get second value.
     \return  Second value
    */
    B& b()
        { return second; }

    /** Get value for pair (second value) (const).
     \return  Second value
    */
    const Value& value() const
        { return second; }

    /** Get value for pair (second value).
     \return  Second value
    */
    Value& value()
        { return second; }

    /** Get hash value for both values in pair.
     \param  seed  Seed value for hashing multiple values, 0 if none
     \return       Hash value
    */
    ulong hash(ulong seed=0) const {
        seed = DataHash<TA>::hash(first, seed);
        return DataHash<TB>::hash(second, seed);
    }

    /** %Compare with another instance.
     - Values are compared in order as a group: This first compares a, and if a is equal then compares b for final result
     .
     \param  src  %Pair to compare to
     \return      Result (<0 if this is less, 0 if equal, >0 if this is greater)
    */
    int compare(const This& src) const {
        int result = DataCompare<A>::compare(first, src.first);
        if (result == 0)
            result = DataCompare<B>::compare(second, src.second);
        return result;
    }

    /** %Compare for equality.
     - Values are compared as a group: They are equal if a and b match with `src`
     .
     \param  src  %Pair to compare to
     \return      Whether equal
    */
    bool operator==(const This& src) const
        { return (first == src.first && second == src.second); }

    /** %Compare for inequality.
     - Values are compared as a group: They are inequal if either a or b don't match
     .
     \param  src  %Pair to compare to
     \return      Whether not equal
    */
    bool operator!=(const This& src) const
        { return (first != src.first || second != src.second); }

    /** %Compare whether less than given pair.
     - Values are compared in order as a group: This first compares a, and if a is equal then compares b for final result
     .
     \param  src  %Pair to compare to
     \return      Whether less than src
    */
    bool operator<(const This& src) const
        { return (first < src.first || (first == src.first && second < src.second)); }

    /** %Compare whether less than or equal to given pair.
     - Values are compared in order as a group: This first compares a, and if a is equal then compares b for final result
     .
     \param  src  %Pair to compare to
     \return      Whether less than or equal to src
    */
    bool operator<=(const This& src) const
        { return (first < src.first || (first == src.first && second <= src.second)); }

    /** %Compare whether greater than given pair.
     - Values are compared in order as a group: This first compares a, and if a is equal then compares b for final result
     .
     \param  src  %Pair to compare to
     \return      Whether greater than src
    */
    bool operator>(const This& src) const
        { return (first > src.first || (first == src.first && second > src.second)); }

    /** %Compare whether greater than or equal to given pair.
     - Values are compared in order as a group: This first compares a, if a is equal then compares b instead
     \param  src  %Pair to compare to
     \return      Whether greater than or equal to src
    */
    bool operator>=(const This& src) const
        { return (first > src.first || (first == src.first && second >= src.second)); }
};

///////////////////////////////////////////////////////////////////////////////
//@}
}
#endif
