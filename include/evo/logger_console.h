// Evo C++ Library
/* Copyright 2019 Justin Crowell
Distributed under the BSD 2-Clause License -- see included file LICENSE.txt for details.
*/
///////////////////////////////////////////////////////////////////////////////
/** \file logger_console.h Evo LoggerConsole. */
#pragma once
#ifndef INCL_evo_logger_console_h
#define INCL_evo_logger_console_h

#include "logger.h"
#include "iothread.h"

namespace evo {
/** \addtogroup EvoTools */
//@{

///////////////////////////////////////////////////////////////////////////////

/** Logger that writes to console.
 - Use to log to stderr instead of a file
 - This is thread safe -- uses ConsoleMT
 - This blocks while writing to stderr so when performance is important use Logger instead
 .
*/
class LoggerConsole : public LoggerBase {
public:
    /** Constructor.
     \param  level  Log level to use -- see LogLevel
    */
    LoggerConsole(LogLevel level=LOG_LEVEL_WARN) : c_(con_mt()) {
        level_.store(level);
    }

    void log_direct(LogLevel level, const SubString& msg) {
        const char* LEVEL_STR[] = { "[ALRT]", "[ERRR]", "[WARN]", "[INFO]", "[dbug]", "[dbgl]" };
        const StrSizeT LEVEL_LEN = 6;
        const SubString level_str(LEVEL_STR[(int)level - 1], LEVEL_LEN);
        EVO_IO_MT(c_.err, << level_str << ' ' << msg << NL);
    }

private:
    ConsoleMT& c_;
};

///////////////////////////////////////////////////////////////////////////////
//@}
}
#endif
