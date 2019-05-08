// Evo C++ Library
/* Copyright 2019 Justin Crowell
Distributed under the BSD 2-Clause License -- see included file LICENSE.txt for details.
*/
///////////////////////////////////////////////////////////////////////////////
/** \file memcached_common.h Evo Async Memached API, common client/server types. */
#pragma once
#ifndef INCL_evo_api_memcached_common_h
#define INCL_evo_api_memcached_common_h

#include "../enum.h"

namespace evo {
namespace async {
/** \addtogroup EvoAsyncAPI
Evo Async I/O client and server APIs
*/
//@{

///////////////////////////////////////////////////////////////////////////////

/** General types used when implementing MemcachedClient or server callbacks. */
struct Memcached {
    /** Memcached protocol error result. */
    enum ErrorResult {
        erUNKNOWN = 0,
        erCLIENT_ERROR,
        erERROR,
        erSERVER_ERROR,
        erENUM_END
    };

    /** Memcached store command result. */
    enum StoreResult {
        srUNKNOWN = 0,  ///< Unknown/invalid result (always first)
        srEXISTS,       ///< Item modified, interrupting compare-and-swap command (CAS command only)
        srNOT_FOUND,    ///< Item not found, can't compare-and-swap (CAS command only)
        srNOT_STORED,   ///< Not stored due to unmet condtion for append, prepend, add, or replace command
        srSTORED,       ///< Successfully stored
        srENUM_END      ///< Enum guard value (always last)
    };

    /** ErrorResult enum conversion helper. */
    EVO_ENUM_MAP_PREFIXED(ErrorResult, er,
        "CLIENT_ERROR",
        "ERROR",
        "SERVER_ERROR"
    );

    /** StoreResult enum conversion helper. */
    EVO_ENUM_MAP_PREFIXED(StoreResult, sr,
        "EXISTS",
        "NOT_FOUND",
        "NOT_STORED",
        "STORED"
    );
};

///////////////////////////////////////////////////////////////////////////////
//@}
}
}
#endif
