// Evo C++ Library
/* Copyright 2018 Justin Crowell
Distributed under the BSD 2-Clause License -- see included file LICENSE.txt for details.
*/
///////////////////////////////////////////////////////////////////////////////
/** \file alg.h Evo general algorithms. */
#pragma once
#ifndef INCL_evo_alg_h
#define INCL_evo_alg_h

namespace evo {
/** \addtogroup EvoAlgs */
//@{

///////////////////////////////////////////////////////////////////////////////

/** Returns lowest of given values.
 - This requires operator<() for comparing values.
 .
 \param  a  Value a.
 \param  b  Value b.
 \return    Lowest value (a or b).
*/
template<class T> inline T& min(T& a, T&b)
    { return (a<b ? a : b); }

/** Returns lowest of given values.
 - This requires operator<() for comparing values.
 .
 \param  a  Value a.
 \param  b  Value b.
 \param  c  Value c.
 \return    Lowest value (a, b, or c).
*/
template<class T> inline T& min(T& a, T& b, T& c)
    { return (a<b ? (a<c ? a : c) : (b<c ? b : c)); }

/** Returns highest of given values.
 - This requires operator<() for comparing values.
 .
 \param  a  Value a.
 \param  b  Value b.
 \return    Highest value (a or b).
*/
template<class T> inline T& max(T& a, T& b)
    { return (a<b ? b : a); }

/** Returns highest of given values.
 - This requires operator<() for comparing values.
 .
 \param  a  Value a.
 \param  b  Value b.
 \param  c  Value c.
 \return    Highest value (a, b, or c).
*/
template<class T> inline T& max(T& a, T& b, T& c)
    { return (a<b ? (b<c ? c : b) : (a<c ? c : a)); }

template<class T> inline T& applyminmax(T& val, T& min, T& max)
    { if (val < min) val = min; }

template<class T> inline T& applymin(T& val, T& min)
    { if (val < min) val = min; }

template<class T> inline T& applymax(T& val, T& max)
    { if (val > max) val = max; }

template<class T> inline T& constrain(T& val, T& min, T& max)
    { return ( val<min ? min : (val>max?max:val) ); }

template<class T> inline T& constrainmin(T& val, T& min)
    { return ( val<min ? min : val ); }

template<class T> inline T& constrainmax(T& val, T& max)
    { return ( val>max ? max : val ); }

template<class T> inline bool validate(T& val, T& min, T& max)
    { return (val >= min && val <= max); }

template<class T> inline T& validate(T& val, T& min, T& max, T& invalid)
    { return ( (val<min || val>max) ? invalid : val ); }

template<class T> inline T first(T val1, T val2)
    { return ( val1 ? val1 : val2 ); }

template<class T> inline T first(T val1, T val2, T val3)
    { return ( val1 ? val1 : (val2?val2:val3) ); }

///////////////////////////////////////////////////////////////////////////////
//@}
}
#endif
