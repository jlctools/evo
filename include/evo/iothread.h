// Evo C++ Library
/* Copyright 2019 Justin Crowell
Distributed under the BSD 2-Clause License -- see included file LICENSE.txt for details.
*/
///////////////////////////////////////////////////////////////////////////////
/** \file iothread.h Evo Input/Output streams with multithreading. */
#pragma once
#ifndef INCL_evo_iothread_h
#define INCL_evo_iothread_h

#include "io.h"
#include "thread.h"
#include "file.h"

///////////////////////////////////////////////////////////////////////////////

/** Shortcut to define a ConsoleMT for I/O.
 - This defines "static ConsoleMT& c" in current scope
 - This is normally used at the top of each function using ConsoleMT streams
   - This may be used globally (not recommended), but you'll run into "redefinition of 'c'" errors if used (or included) in multiple source files
     - Note: Evo examples often use a static global for simplicity
   - This won't work in a class or struct outside of a function (must initialize static member out of line)
 - For \b non-thread-safe variant see \link EVO_CONSOLE EVO_CONSOLE\endlink

\par Example

\code
#include <evo/iothread.h>
using namespace evo;

void hello() {
    EVO_CONSOLE_MT;
    EVO_IO_MT(c.out, << "Hello" << NL)
}

int main() {
    EVO_CONSOLE_MT;
    EVO_IO_MT(c.out, << "Calling hello()" << NL)
    hello();

    return 0;
}
\endcode
*/
#define EVO_CONSOLE_MT static evo::ConsoleMT& c = evo::con_mt()

/** Lock mutex and run stream operation code.
 - This is a shortcut for making a stream operation thread-safe
 - Note: This operates in a new scope so an ending semi-colon isn't required (though won't hurt)
 .
 \param  STREAM  StreamMT or StreamExtMT object to use
 \param  CODE    Code to use on stream, this code goes right after the string object name, it may start with a dot (.) or operator like <<

\par Example

\code
#include <evo/iothread.h>
using namespace evo;
static ConsoleMT& c = con_mt();

int main() {
    // Thread safe console output
    EVO_IO_MT(c.out, << "Hello World " << 123 << NL)    // ending semicolon not required

    // Thread safe console output without macro
    { Mutex::Lock lock(c.out.mutex); c.out.stream << "Hello World " << 123 << NL; }
}
\endcode
*/
#define EVO_IO_MT(STREAM, CODE) { Mutex::Lock evo_streamlock_(STREAM .mutex); STREAM.stream CODE; }

/** Catch Evo Exception and print thread-safe error message to stderr.
 - This does not terminate the program when the exception is caught, pass abort() (or similar) in CODE to do that
 - Use after a try or catch block, where a "catch" would normally go (see example below)
 - For \b non-thread-safe variant see EVO_CATCH()
 .
 \param  CODE  Code to run if exception is caught, abort() to terminate process, use just a semi-colon for none

\par Example

\code
#include <evo/iothread.h>

int main() {
    try {
        // ...
        return 0;
    } EVO_CATCH_MT(abort())
}
\endcode
*/
#if EVO_CATCH_DEBUG == 0
    #define EVO_CATCH_MT(CODE) catch (const evo::Exception& e) { \
        { \
            evo::ConsoleMT& c = evo::con_mt(); \
            Mutex::Lock lock(c.err.mutex); \
            c.err.stream << evo::NL << e.msg() << " -- "; \
            evo::errormsg_out(c.err.stream, e.error()); \
            c.err.stream << evo::NL; \
        } \
        CODE; \
    }
#else
    #define EVO_CATCH_MT(CODE) catch (const evo::Exception& e) { \
        { \
            evo::ConsoleMT& c = evo::con_mt(); \
            Mutex::Lock lock(c.err.mutex); \
            c.err.stream << evo::NL; \
            if (e.file()) \
                c.err.stream << e.file() << '(' << e.line() << evo::SubString("): ", 3); \
            c.err.stream << e.msg() << evo::SubString(" -- ", 4); \
            evo::errormsg_out(c.err.stream, e.error()); \
            c.err.stream << evo::NL; \
        } \
        CODE; \
    }
#endif

///////////////////////////////////////////////////////////////////////////////

// Namespace: evo
namespace evo {

/** \addtogroup EvoIO */
//@{

///////////////////////////////////////////////////////////////////////////////

/** %Stream with mutex for multithreaded synchronization.
 - The stream is only thread safe when protected by this mutex
*/
template<class T>
struct StreamMT {
    typedef StreamMT<T> This;    ///< This type

    Mutex mutex;    ///< Stream mutex
    T     stream;   ///< Stream object

    /** Constructor.
     \param  exceptions  Whether to enable exceptions on error, default set by Evo config: EVO_EXCEPTIONS
    */
    StreamMT(bool exceptions=EVO_EXCEPTIONS) : stream(exceptions)
        { }

private:
    StreamMT(This&);
    This& operator=(This&);
};

/** %Stream with mutex for multithreaded synchronization, specialized for File.
 - This supports the same constructor overloads as File
 - The stream is only thread safe when protected by this mutex
*/
template<>
struct StreamMT<File> {
    typedef StreamMT<File> This;    ///< This type

    Mutex mutex;    ///< Stream mutex
    File  stream;   ///< Stream object

    /** \copydoc File::File(Newline, bool) */
    StreamMT(Newline nl=NL, bool exceptions=EVO_EXCEPTIONS) : stream(nl, exceptions)
        { }

    /** \copydoc File::File(const char*, Open, bool, Newline, bool) */
    StreamMT(const char* path, Open mode=oREAD, bool flushlines=false, Newline nl=NL_SYS, bool exceptions=EVO_EXCEPTIONS) :
        stream(path, mode, flushlines, nl, exceptions)
        { }

private:
    StreamMT(This&);
    This& operator=(This&);
};

typedef StreamMT<PipeIn>  PipeInMT;       ///< Input pipe with mutex for synchonization -- see StreamMT
typedef StreamMT<PipeOut> PipeOutMT;      ///< Output pipe with mutex for synchonization -- see StreamMT
typedef StreamMT<File>    FileMT;         ///< %File with mutex for synchonization -- see File and StreamMT<File>

///////////////////////////////////////////////////////////////////////////////

/** Multithreaded console I/O.
 - This wraps existing Console streams with synchronization to support use by multiple threads
 - Each stream operation must use the EVO_IO_MT() macro (or equivalent) in order to be thread-safe
 - Call ConsoleMT::get() or con_mt() (shortcut) to get a console reference, or use \link EVO_CONSOLE_MT\endlink (shortcut macro)
 - \b Compiler: ConsoleMT::get() and con_mt() aren't thread safe without compiler support for "magic statics", which requires at least:
   - <a href="https://msdn.microsoft.com/en-us/library/hh567368.aspx">Microsoft Visual C++ 2015</a>
   - <a href="http://en.cppreference.com/w/cpp/compiler_support#cpp11">gcc 4.3, clang 2.9</a>
 - See also: ConsoleNotMT
 
\par Example

\code
#include <evo/iothread.h>
using namespace evo;
static ConsoleMT& c = con_mt();

int main() {
    // Thread safe console output
    EVO_IO_MT(c.out, << "Hello World " << 123 << NL)

    // Thread safe console output without macro
    { Mutex::Lock lock(c.out.mutex); c.out.stream << "Hello World " << 123 << NL; }
}
\endcode 
*/
struct ConsoleMT {
    typedef PipeOut::Format Format;             ///< \copydoc evo::StreamFormatter
    typedef StreamExtMT<PipeIn,Mutex>  InT;     ///< Input type
    typedef StreamExtMT<PipeOut,Mutex> OutT;    ///< Output type
    typedef Mutex::Lock Lock;                   ///< Mutex lock type

    InT  in;        ///< Read console input
    OutT out;       ///< Write to console, normal output
    OutT err;       ///< Write to console, error output

    /** Get console instance to use.
     \return  Console instance
    */
    static ConsoleMT& get()
        { static ConsoleMT console(con()); return console; }

private:
    ConsoleMT(Console& c) : in(c.in), out(c.out), err(c.err)
        { }

    ConsoleMT(const ConsoleMT&);
    ConsoleMT& operator=(const ConsoleMT&);
};

/** Shortcut for ConsoleMT::get().
 \return  ConsoleMT reference
*/
inline ConsoleMT& con_mt()
    { return ConsoleMT::get(); }

///////////////////////////////////////////////////////////////////////////////
//@}
}
#endif
