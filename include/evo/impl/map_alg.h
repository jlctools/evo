// Evo C++ Library
/* Copyright 2018 Justin Crowell
Distributed under the BSD 2-Clause License -- see included file LICENSE.txt for details.
*/
///////////////////////////////////////////////////////////////////////////////
/** \file map_alg.h Evo implementation detail: Internal map algorithms. */
#pragma once
#ifndef INCL_evo_impl_map_alg_h
#define INCL_evo_impl_map_alg_h

#include "../strtok.h"

namespace evo {
/** \addtogroup EvoContainers */
//@{

///////////////////////////////////////////////////////////////////////////////

// Implementation
/** \cond impl */
namespace impl
{
    template<class TMap, class TStr>
    typename TMap::Size map_addsplit(TMap& map, const TStr& str, char delim=',', char kvdelim='=') {
        char delims_buf[2];
        delims_buf[0] = delim;
        delims_buf[1] = kvdelim;
        const SubString delims(delims_buf, 2);
        const SubString EMPTY;

        typename TMap::Size count = 0;
        bool created;
        StrTok tok(str);
        for (; tok.nextany(delims); ++count) {
            typename TMap::Value& val = map.get(tok.value().convert<typename TMap::Key>(), &created);
            if (tok.delim().null() || *tok.delim() == delim) {
                if (!created)
                    DataInit<typename TMap::Value>::init(val);
            } else {
                if (tok.next(delim))
                    val = tok.value().convert<typename TMap::Value>();
                else if (!created)
                    DataInit<typename TMap::Value>::init(val);
            }
        }
    }
}
/** \endcond */

///////////////////////////////////////////////////////////////////////////////
//@}
}
#endif
