// Evo C++ Library
/* Copyright 2019 Justin Crowell
Distributed under the BSD 2-Clause License -- see included file LICENSE.txt for details.
*/
///////////////////////////////////////////////////////////////////////////////
/** \file io.h Evo I/O streams and Console I/O. */
#pragma once
#ifndef INCL_evo_io_h
#define INCL_evo_io_h

#include "iobase.h"
#include "thread_inert.h"

///////////////////////////////////////////////////////////////////////////////

/** Shortcut to define a Console for I/O.
 - This defines "static Console& c" in current scope
 - This is normally used at the top of each function using Console streams
   - This may be used globally (not recommended), but you'll run into "redefinition of 'c'" errors if used (or included) in multiple source files
     - Note: Evo examples often use a static global for simplicity
   - This won't work in a class or struct outside of a function (must initialize static member out of line)
 - For \b thread-safe variant see \link EVO_CONSOLE_MT EVO_CONSOLE_MT\endlink

\par Example

\code
#include <evo/io.h>
using namespace evo;

void hello() {
    EVO_CONSOLE;
    c.out << "Hello" << NL;
}

int main() {
    EVO_CONSOLE;
    c.out << "Calling hello()" << NL;
    hello();

    return 0;
}
\endcode
*/
#define EVO_CONSOLE static evo::Console& c = evo::con()

/** Catch Evo Exception and print error message to stderr.
 - This does not terminate the program when the exception is caught, pass abort() (or `return` if applicable) in CODE to do that
 - Use after a try or catch block, where a "catch" would normally go (see example below)
 - For \b thread-safe variant see EVO_CATCH_MT()
 .
 \param  CODE  Code to run if exception is caught, abort() to terminate process, use just a semi-colon for none

\par Example

\code
#include <evo/io.h>

int main() {
    try {
        // ...
        return 0;
    } EVO_CATCH(abort())
}
\endcode
*/
#if EVO_CATCH_DEBUG == 0
    #define EVO_CATCH(CODE) catch (const evo::Exception& e) { \
        evo::Console& c = evo::con(); \
        c.err << evo::NL << e.msg() << evo::SubString(" -- ", 4); \
        evo::errormsg_out(c.err, e.error()); \
        c.err << evo::NL; \
        CODE; \
    }
#else
    #define EVO_CATCH(CODE) catch (const evo::Exception& e) { \
        evo::Console& c = evo::con(); \
        c.err << evo::NL; \
        if (e.file()) \
            c.err << e.file() << '(' << e.line() << evo::SubString("): ", 3); \
        c.err << e.msg() << evo::SubString(" -- ", 4); \
        evo::errormsg_out(c.err, e.error()); \
        c.err << evo::NL; \
        CODE; \
    }
#endif

///////////////////////////////////////////////////////////////////////////////

// Namespace: evo
namespace evo {

/** \addtogroup EvoIO
Evo input/output streams
*/
//@{

///////////////////////////////////////////////////////////////////////////////

/** Input stream implementation.
 - This is similar to Stream but only supports reading input
 - This implements binary and text input with buffering
   - Binary intput (no newline conversion): readbin()
   - Text intput (automatic newline conversion): readtext(), readline()
   .
 - Call error() to check error code from last operation
 - Evaluating as bool (via SafeBool) checks whether last operation was successful by calling ! operator!()\n
   Example:
   \code
if (stream) {
    // last operation successful
}
if (!stream) {
    // last operation failed
}
   \endcode
 .
 \tparam  T  Low-level I/O stream implementing IoDevice to use
*/
template<class T>
class StreamIn : public StreamBase {
public:
    typedef typename T::Handle Handle;  ///< Low-level handle type (OS dependent)
    typedef StreamIn<T> This;           ///< This stream type

    typedef typename T::ExceptionInT  ExceptionInT;     ///< %Stream input exception type

    /** Constructor.
     \param  newlines  Newline value for text reading to convert newlines to (defaults to NL_SYS), doesn't affect reading by line
    */
    StreamIn(Newline newlines=NL_SYS) :
        owned_(false),
        bufrd_(0, newlines)
        { }

    /** \copydoc evo::Stream::handle() */
    Handle handle() const
        { return device_.handle; }

    /** \copydoc evo::Stream::bufread() */
    RawBuffer& bufread()
        { return bufrd_.readbuf; }

    /** Attach existing input stream.
     \param  handle  Handle to attach, must be valid
     \param  owned   Whether to take ownership and close handle, false detaches on close()
    */
    void attach(Handle handle, bool owned=true) {
        close();
        device_.handle = handle;
        if (device_.isopen()) {
            owned_ = owned;
            init();
        }
    }

    /** Detach current stream.
     - Never throws an exception
     .
     \return  Detached handle
    */
    Handle detach() {
        if (device_.isopen()) {
            bufrd_.close();
            owned_ = false;
        }
        return device_.detach();
    }

    /** Close stream.
     - Never throws an exception, even if close fails
     .
     \return  Whether successful, always true
    */
    bool close() {
        if (device_.isopen()) {
            bufrd_.close();
            if (owned_) {
                device_.close();
                owned_ = false;
            } else
                device_.detach();
        }
        return true;
    }

    bool isopen() const {
        return device_.isopen();
    }

    // Read methods copied from Stream, with some modifications -- it's done this way to avoid more virtual method overhead

    ulong readbin(void* buf, ulong size) {
        size = bufrd_.readbin(error_, device_, buf, size);
        EVO_THROW_ERR_CHECK(ExceptionInT, "Stream binary read failed", error_, (excep_ && size == 0 && error_ != ENone && error_ != EEnd));
        return size;
    }

    ulong readtext(char* buf, ulong size) {
        size = bufrd_.readtext(error_, device_, buf, size);
        EVO_THROW_ERR_CHECK(ExceptionInT, "Stream text read failed", error_, (excep_ && size == 0 && error_ != ENone && error_ != EEnd));
        return size;
    }

    bool readline(String& str, ulong maxlen=0) {
        error_ = bufrd_.readline(str, device_, maxlen);
        if (error_ != ENone && error_ != EEnd) {
            EVO_THROW_ERR_CHECK(ExceptionInT, "Stream text line read failed", error_, excep_);
            return false;
        }
        return true;
    }

protected:
    T        device_;       ///< I/O handle
    bool     owned_;        ///< Whether handle is owned (to be closed here)
    IoReader bufrd_;        ///< Buffered reader

    /** Initialize and reset buffers for a new stream. */
    void init() {
        bufrd_.open();
    }

private:
    StreamIn(const This&);
    This& operator=(const This&);
};

///////////////////////////////////////////////////////////////////////////////

/** Output stream implementation.
 - This is similar to Stream but only supports writing output
 - This implements binary and text output with buffering
   - Binary output (no newline conversion): writebin()
   - Text output (automatic newline conversion): writetext(), writeline()
   - Use flush() to flush write buffer (text and binary)
   .
 - This also implements formatted output using operator<<()\n
   Note: Output formatting stops (is skipped) when an error is set, allows chaining << calls then checking error\n
   Example:
   \code
stream << "test" << 123 << ',' << 12.3 << NL;
if (!stream) {
    // write error
}
   \endcode
 - Call error() to check error code from last operation
 - Evaluating as bool (via SafeBool) checks whether last operation was successful by calling ! operator!()\n
   Example:
   \code
if (stream) {
    // last operation successful
}
if (!stream) {
    // last operation failed
}
   \endcode
 .
 \tparam  T  Low-level I/O stream implementing IoDevice to use
*/
template<class T>
class StreamOut : public StreamBase {
public:
    typedef typename T::Handle Handle;                  ///< Low-level handle type (OS dependent)
    typedef StreamOut<T>       This;                    ///< This stream type
    typedef This               Out;                     ///< Type returned by write_out()

    typedef StreamFormatter<This> Format;               ///< \copydoc evo::StreamFormatter

    typedef typename T::ExceptionOutT ExceptionOutT;    ///< %Stream output exception type

    /** Constructor.
     \param  newlines  Newline type to use for text writing
    */
    StreamOut(Newline newlines=NL_SYS) :
        owned_(false),
        bufwr_(0, newlines)
        { }

    /** Destructor. */
    ~StreamOut()
        { close(); }

    /** \copydoc evo::Stream::handle() */
    Handle handle() const
        { return device_.handle; }

    /** \copydoc evo::Stream::bufwrite() */
    IoWriter& bufwrite()
        { return bufwr_; }

    /** Attach existing output stream.
     \param  handle      Handle to attach, must be valid
     \param  owned       Whether to take ownership and close handle, false detaches on close()
     \param  flushlines  Whether to flush text output on newlines (line buffering)
    */
    void attach(Handle handle, bool owned=true, bool flushlines=false) {
        close();
        device_.handle = handle;
        if (device_.isopen()) {
            owned_ = owned;
            init(flushlines);
        }
    }

    /** Detach current stream.
    - This will flush output before detaching
    - Never throws an exception, even if flush fails
    .
    \return  Detached handle
    */
    Handle detach() {
        if (device_.isopen()) {
            if (bufwr_.used > 0)
                bufwr_.flush(device_);
            bufwr_.close();
            owned_ = false;
        }
        return device_.detach();
    }

    /** Close stream.
     - This will flush output before closing
     - Never throws an exception, even if flush or close fails
     .
     \return  Whether successful, false on flush error (stream will still close)
    */
    bool close() {
        if (device_.isopen()) {
            if (bufwr_.used > 0)
                error_ = bufwr_.flush(device_);
            else
                error_ = ENone;
            bufwr_.close();
            if (owned_) {
                device_.close();
                owned_ = false;
            } else
                device_.detach();
            return (error_ == ENone);
        }
        return true;
    }

    bool isopen() const
        { return device_.isopen(); }

    // Write methods copied from Stream, with some modifications -- it's done this way to avoid more virtual method overhead

    bool flush() {
        error_ = bufwr_.flush(device_);
        if (error_ != ENone) {
            EVO_THROW_ERR_CHECK(ExceptionOutT, "Stream flush failed", error_, (excep_ && error_ != ENone));
            return false;
        }
        return true;
    }

    ulong writebin(const void* buf, ulong size) {
        size = bufwr_.writebin(error_, device_, buf, size);
        EVO_THROW_ERR_CHECK(ExceptionOutT, "Stream binary write failed", error_, (excep_ && size == 0 && error_ != ENone));
        return size;
    }

    ulong writechar(char ch, ulong count=1) {
        count = bufwr_.writetext_char(error_, device_, ch, count);
        EVO_THROW_ERR_CHECK(ExceptionOutT, "Stream text write failed", error_, (excep_ && count == 0 && error_ != ENone));
        return count;
    }

    ulong writetext(const char* buf, ulong size) {
        size = bufwr_.writetext(error_, device_, buf, size);
        EVO_THROW_ERR_CHECK(ExceptionOutT, "Stream text write failed", error_, (excep_ && size == 0 && error_ != ENone));
        return size;
    }

    Size writequoted(const char* buf, Size size, char delim, bool optional=false) {
        bool quote_optional = false;
        const StrQuoting::Type type = StrQuoting::get(quote_optional, buf, size, delim);
        switch (type) {
            case StrQuoting::tSINGLE:
                if (quote_optional && optional)
                    return writebin(buf, size);
            case StrQuoting::tDOUBLE:
            case StrQuoting::tBACKTICK: {
                const char ch = "'\"`"[(uint)type];
                const Size result = size + 2;
                if (writebin(&ch, 1) == 1 && writebin(buf, size) == size && writebin(&ch, 1) == 1)
                    return result;
                break;
            }
            case StrQuoting::tSINGLE3:
            case StrQuoting::tDOUBLE3:
            case StrQuoting::tBACKTICK3: {
                const char* str = &"'''\"\"\"```"[(uint(type) - 3) * 3];
                const Size result = size + 6;
                if (writebin(str, 3) == 3 && writebin(buf, size) == size && writebin(str, 3) == 3)
                    return result;
                break;
            }
            case StrQuoting::tBACKTICK_DEL: {
                const char* str = "`\x7F";
                const Size result = size + 4;
                if (writebin(str, 2) != 2 || writebin(buf, size) != size || writebin(str, 2) != 2)
                    return result;
                break;
            }
            case StrQuoting::tERROR:
                error_ = EInval;
                EVO_THROW_ERR_CHECK(ExceptionOutT, "Stream text write failed on unquotable text", error_, excep_);
                break;
        }
        return 0;
    }

    ulong writeline(const char* buf, ulong size) {
        ulong writtensize = bufwr_.writetext(error_, device_, buf, size);
        bufwr_.partnl = 0;
        if (writtensize == 0 || !bufwr_.writebin(error_, device_, bufwr_.newline, bufwr_.newlinesize) ) {
            EVO_THROW_ERR_CHECK(ExceptionOutT, "Stream text line write failed", error_, excep_);
            return 0;
        }
        return writtensize + bufwr_.newlinesize;
    }

    Out& write_out()
        { return *this; }

    char* write_direct(Size size) {
        if (size > bufwr_.size) {
            EVO_THROW_ERR_CHECK(ExceptionOutT, "Stream not large enough for write_direct()", error_, excep_);
            return NULL;
        }
        if (bufwr_.avail() < size) {
            error_ = bufwr_.flush(device_);
            if (error_ != ENone) {
                EVO_THROW_ERR_CHECK(ExceptionOutT, "Stream flush failed", error_, excep_);
                return NULL;
            }
        }
        bufwr_.partnl = 0;
        return bufwr_.data + bufwr_.used;
    }

    char* write_direct_multi(Size& available, Size reserve_size) {
        if (reserve_size > bufwr_.avail()) {
            error_ = bufwr_.flush(device_);
            if (error_ != ENone) {
                EVO_THROW_ERR_CHECK(ExceptionOutT, "Stream flush failed", error_, excep_);
                return NULL;
            }
            if (reserve_size > bufwr_.size)
                available = (Size)bufwr_.size;
            else
                available = reserve_size;
        } else {
            available = reserve_size;
            if (reserve_size == 0)
                return (char*)1; // finished
        }
        bufwr_.partnl = 0;
        return bufwr_.data + bufwr_.used;
    }

    char* write_direct_flush(Size& available, Size written_size, Size reserve_size) {
        bufwr_.used += written_size;
        assert( bufwr_.used <= bufwr_.size );

        error_ = bufwr_.flush(device_);
        if (error_ != ENone) {
            EVO_THROW_ERR_CHECK(ExceptionOutT, "Stream flush failed", error_, excep_);
            return NULL;
        }

        if (reserve_size > bufwr_.size) {
            available = (Size)bufwr_.size;
        } else {
            available = reserve_size;
            if (reserve_size == 0)
                return (char*)1; // finished
        }

        return bufwr_.data;
    }

    bool write_direct_finish(Size size) {
        bufwr_.used += size;
        assert( bufwr_.used <= bufwr_.size );
        return true;
    }

    /** \copydoc evo::Stream::writenum() */
    template<class TNum>
    bool writenum(TNum num, int base=fDEC) {
        if (error_ == ENone) {
            error_ = bufwr_.writenum(device_, num, base);
            if (error_ != ENone) {
                EVO_THROW_ERR_CHECK(ExceptionOutT, "Stream text write number failed", error_, excep_);
                return false;
            }
            return true;
        }
        EVO_THROW_ERR_CHECK(ExceptionOutT, "Stream text write number blocked by previous error", error_, excep_);
        return false;
    }

    /** \copydoc evo::Stream::writenumu() */
    template<class TNum>
    bool writenumu(TNum num, int base=fDEC) {
        if (error_ == ENone) {
            error_ = bufwr_.writenumu(device_, num, base);
            if (error_ != ENone) {
                EVO_THROW_ERR_CHECK(ExceptionOutT, "Stream text write number failed", error_, excep_);
                return false;
            }
            return true;
        }
        EVO_THROW_ERR_CHECK(ExceptionOutT, "Stream text write number blocked by previous error", error_, excep_);
        return false;
    }

    /** \copydoc evo::Stream::writenumf() */
    template<class TNum>
    bool writenumf(TNum num, int precision=fPREC_AUTO) {
        if (error_ == ENone) {
            error_ = bufwr_.writenumf(device_, num, precision);
            if (error_ != ENone) {
                EVO_THROW_ERR_CHECK(ExceptionOutT, "Stream text write number failed", error_, excep_);
                return false;
            }
            return true;
        }
        EVO_THROW_ERR_CHECK(ExceptionOutT, "Stream text write number blocked by previous error", error_, excep_);
        return false;
    }

    /** \copydoc evo::Stream::writefmtchar() */
    bool writefmtchar(char ch, ulong count, const FmtSetField& field) {
        if (error_ == ENone) {
            error_ = bufwr_.writefmtchar(device_, ch, count, field);
            EVO_THROW_ERR_CHECK(ExceptionOutT, "Stream text formatted write char failed", error_, (excep_ && error_ != ENone));
            return true;
        }
        EVO_THROW_ERR_CHECK(ExceptionOutT, "Stream text write formatted char blocked by previous error", error_, excep_);
        return false;
    }

    /** \copydoc evo::Stream::writefmtstr() */
    bool writefmtstr(const char* buf, ulong size, const FmtSetField& field) {
        if (error_ == ENone) {
            error_ = bufwr_.writefmtstr(device_, buf, size, field);
            EVO_THROW_ERR_CHECK(ExceptionOutT, "Stream text formatted write failed", error_, (excep_ && error_ != ENone));
            return true;
        }
        EVO_THROW_ERR_CHECK(ExceptionOutT, "Stream text formatted write blocked by previous error", error_, excep_);
        return false;
    }

    /** \copydoc evo::Stream::writefmtnum() */
    template<class TNum>
    bool writefmtnum(TNum num, const FmtSetInt& fmt, const FmtSetField* field=NULL) {
        if (error_ == ENone) {
            error_ = bufwr_.writefmtnum(device_, num, fmt, field);
            if (error_ != ENone) {
                EVO_THROW_ERR_CHECK(ExceptionOutT, "Stream text write formatted number failed", error_, excep_);
                return false;
            }
            return true;
        }
        EVO_THROW_ERR_CHECK(ExceptionOutT, "Stream text write formatted number blocked by previous error", error_, excep_);
        return false;
    }

    /** \copydoc evo::Stream::writefmtnumu() */
    template<class TNum>
    bool writefmtnumu(TNum num, const FmtSetInt& fmt, const FmtSetField* field=NULL) {
        if (error_ == ENone) {
            error_ = bufwr_.writefmtnumu(device_, num, fmt, field);
            if (error_ != ENone) {
                EVO_THROW_ERR_CHECK(ExceptionOutT, "Stream text write formatted number failed", error_, excep_);
                return false;
            }
            return true;
        }
        EVO_THROW_ERR_CHECK(ExceptionOutT, "Stream text write formatted number blocked by previous error", error_, excep_);
        return false;
    }

    /** \copydoc evo::Stream::writefmtnumf() */
    template<class TNum>
    bool writefmtnumf(TNum num, const FmtSetFloat& fmt, const FmtSetField* field=NULL) {
        if (error_ == ENone) {
            error_ = bufwr_.writefmtnumf(device_, num, fmt, field);
            if (error_ != ENone) {
                EVO_THROW_ERR_CHECK(ExceptionOutT, "Stream text write formatted number failed", error_, excep_);
                return false;
            }
            return true;
        }
        EVO_THROW_ERR_CHECK(ExceptionOutT, "Stream text write formatted number blocked by previous error", error_, excep_);
        return false;
    }

    /** \copydoc evo::Stream::writefmtdump() */
    bool writefmtdump(const FmtDump& fmt) {
        if (error_ == ENone) {
            error_ = bufwr_.writefmtdump(device_, fmt, bufwr_.newline, bufwr_.newlinesize);
            if (error_ != ENone) {
                EVO_THROW_ERR_CHECK(ExceptionOutT, "Stream text write hex dump failed", error_, excep_);
                return false;
            }
            return true;
        }
        EVO_THROW_ERR_CHECK(ExceptionOutT, "Stream text write hex dump blocked by previous error", error_, excep_);
        return false;
    }

    // operator<<() overloads copied from Stream

    /** \copydoc evo::Stream::operator<<(This&) */
    This& operator<<(This&)
        { return *this; }

    /** \copydoc evo::Stream::operator<<(Newline) */
    This& operator<<(Newline nl) {
        if (error_ == ENone) {
            writebin(getnewline(nl), getnewlinesize(nl));
            flush();
        }
        return *this;
    }

    /** \copydoc evo::Stream::operator<<(NewlineDefault) */
    This& operator<<(NewlineDefault nl) {
        EVO_PARAM_UNUSED(nl);
        if (error_ == ENone) {
            writebin(bufwr_.newline, bufwr_.newlinesize);
            flush();
        }
        return *this;
    }

    /** \copydoc evo::Stream::operator<<(const NewlineValue&) */
    This& operator<<(const NewlineValue& nl) {
        if (error_ == ENone) {
            uint nl_size;
            const char* nl_str = nl.getnewline(nl_size, bufwr_.newline, bufwr_.newlinesize);
            writebin(nl_str, nl_size);
            flush();
        }
        return *this;
    }

    /** \copydoc evo::Stream::operator<<(Flush) */
    This& operator<<(Flush) {
        if (error_ == ENone)
            flush();
        return *this;
    }

    /** \copydoc evo::Stream::operator<<(bool) */
    This& operator<<(bool val) {
        if (error_ == ENone) {
            if (val)
                bufwr_.writetext(error_, device_, "true", 4);
            else
                bufwr_.writetext(error_, device_, "false", 5);
        }
        return *this;
    }

    /** \copydoc evo::Stream::operator<<(char) */
    This& operator<<(char ch) {
        if (error_ == ENone)
            writechar(ch);
        return *this;
    }

    /** \copydoc evo::Stream::operator<<(const char*) */
    This& operator<<(const char* str) {
        if (error_ == ENone && str != NULL)
            bufwr_.writetext(error_, device_, str, (ulong)strlen(str));
        return *this;
    }

    /** \copydoc evo::Stream::operator<<(const SubString&) */
    This& operator<<(const SubString& str) {
        if (error_ == ENone)
            bufwr_.writetext(error_, device_, str.data_, str.size_);
        return *this;
    }

    /** \copydoc evo::Stream::operator<<(const ListBase<char,TSize>&) */
    template<class TSize>
    This& operator<<(const ListBase<char,TSize>& str) {
        if (error_ == ENone) {
            bufwr_.writetext(error_, device_, str.data_, str.size_);
            EVO_THROW_ERR_CHECK(evo::ExceptionStreamOut, "Stream text write (ListBase) failed", error_, (excep_ && error_ != ENone));
        }
        return *this;
    }

    /** \copydoc evo::Stream::operator<<(int) */
    This& operator<<(int num)
        { writenum(num); return *this; }

    /** \copydoc evo::Stream::operator<<(long) */
    This& operator<<(long num)
        { writenum(num); return *this; }

    /** \copydoc evo::Stream::operator<<(longl) */
    This& operator<<(longl num)
        { writenum(num); return *this; }

    /** \copydoc evo::Stream::operator<<(uint) */
    This& operator<<(uint num)
        { writenumu(num); return *this; }

    /** \copydoc evo::Stream::operator<<(ulong) */
    This& operator<<(ulong num)
        { writenumu(num); return *this; }

    /** \copydoc evo::Stream::operator<<(ulongl) */
    This& operator<<(ulongl num)
        { writenumu(num); return *this; }

    /** \copydoc evo::Stream::operator<<(const IntegerT<U>&) */
    template<class U>
    This& operator<<(const IntegerT<U>& num) {
        if (!num.null()) {
            if (IntegerT<U>::SIGN)
                writenum(num.value());
            else
                writenumu(num.value());
        }
        return *this;
    }

    /** \copydoc evo::Stream::operator<<(float) */
    This& operator<<(float num)
        { writenumf(num); return *this; }

    /** \copydoc evo::Stream::operator<<(double) */
    This& operator<<(double num)
        { writenumf(num); return *this; }

    /** \copydoc evo::Stream::operator<<(ldouble) */
    This& operator<<(ldouble num)
        { writenumf(num); return *this; }

    /** \copydoc evo::Stream::operator<<(const FloatT<U>&) */
    template<class U>
    This& operator<<(const FloatT<U>& num) {
        if (!num.null())
            writenumf(num);
        return *this;
    }

    /** \copydoc evo::Stream::operator<<(const FmtChar&) */
    This& operator<<(const FmtChar& fmt)
        { writechar(fmt.ch, fmt.count); return *this; }

    /** \copydoc evo::Stream::operator<<(const FmtString&) */
    This& operator<<(const FmtString& fmt)
        { writefmtstr(fmt.str.data_, fmt.str.size_, fmt.fmt); return *this; }

    This& operator<<(const FmtStringWrap& fmt) {
        uint newline_size;
        const char* newline_str = fmt.newline.getnewline(newline_size, bufwr_.newline, bufwr_.newlinesize);

        SubString str(fmt.str), line, substr;
        for (uint notfirst=0; str.token_line(line); ) {
            for (;;) {
                if (line.empty()) {
                    if (writebin(newline_str, newline_size) == 0)
                        return *this;
                } else {
                    if (notfirst == 0)
                        ++notfirst;
                    else if (fmt.indent > 0 && writechar(' ', fmt.indent) == 0)
                        return *this;
                    if (fmt.width > 1 && line.size() > (Size)fmt.width) {
                        // line too long, try to find a word break
                        Size i = line.findnonwordr(0, (Size)fmt.width + 1);
                        if (i == NONE) {
                            i = (Size)fmt.width;
                        } else {
                            while (i > 0 && ascii_breaktype(line[i]) == cbtBEGIN)
                                --i;
                            if (i < (Size)fmt.width)
                                ++i;
                        }

                        substr.set(line.data(), i).stripr();
                        if (!substr.empty() && (writebin(substr.data(), substr.size()) == 0 || writebin(newline_str, newline_size) == 0))
                            return *this;

                        line.triml(i).stripl();
                        continue;
                    }
                    if (writebin(line.data(), line.size()) == 0 || writebin(newline_str, newline_size) == 0)
                        return *this;
                }
                break;
            }
        }
        return *this;
    }

    /** \copydoc evo::Stream::operator<<(const FmtShort&) */
    This& operator<<(const FmtShort& fmt)
        { writefmtnum(fmt.num, fmt.fmt); return *this; }

    /** \copydoc evo::Stream::operator<<(const FmtInt&) */
    This& operator<<(const FmtInt& fmt)
        { writefmtnum(fmt.num, fmt.fmt); return *this; }

    /** \copydoc evo::Stream::operator<<(const FmtLong&) */
    This& operator<<(const FmtLong& fmt)
        { writefmtnum(fmt.num, fmt.fmt); return *this; }

    /** \copydoc evo::Stream::operator<<(const FmtLongL&) */
    This& operator<<(const FmtLongL& fmt)
        { writefmtnum(fmt.num, fmt.fmt); return *this; }

    /** \copydoc evo::Stream::operator<<(const FmtUShort&) */
    This& operator<<(const FmtUShort& fmt)
        { writefmtnumu(fmt.num, fmt.fmt); return *this; }

    /** \copydoc evo::Stream::operator<<(const FmtUInt&) */
    This& operator<<(const FmtUInt& fmt)
        { writefmtnumu(fmt.num, fmt.fmt); return *this; }

    /** \copydoc evo::Stream::operator<<(const FmtULong&) */
    This& operator<<(const FmtULong& fmt)
        { writefmtnumu(fmt.num, fmt.fmt); return *this; }

    /** \copydoc evo::Stream::operator<<(const FmtULongL&) */
    This& operator<<(const FmtULongL& fmt)
        { writefmtnumu(fmt.num, fmt.fmt); return *this; }

    /** \copydoc evo::Stream::operator<<(const FmtFloat&) */
    This& operator<<(const FmtFloat& fmt)
        { writefmtnumf(fmt.num, fmt.fmt); return *this; }

    /** \copydoc evo::Stream::operator<<(const FmtFloatD&) */
    This& operator<<(const FmtFloatD& fmt)
        { writefmtnumf(fmt.num, fmt.fmt); return *this; }

    /** \copydoc evo::Stream::operator<<(const FmtFloatL&) */
    This& operator<<(const FmtFloatL& fmt)
        { writefmtnumf(fmt.num, fmt.fmt); return *this; }

    /** \copydoc evo::Stream::operator<<(const FmtShort&) */
    template<class U>
    This& operator<<(const FmtFieldNum<U>& fmt) {
        if (IntegerT<U>::SIGN)
            writefmtnum(fmt.num.num, fmt.num.fmt, &fmt.field);
        else
            writefmtnumu(fmt.num.num, fmt.num.fmt, &fmt.field);
        return *this;
    }

    /** \copydoc evo::Stream::operator<<(const FmtShort&) */
    template<class U>
    This& operator<<(const FmtFieldFloat<U>& fmt)
        { writefmtnumf(fmt.num.num, fmt.num.fmt, &fmt.field); return *this; }

    /** \copydoc evo::Stream::operator<<(const FmtPtr&) */
    This& operator<<(const FmtPtr& fmtptr)
        { writefmtnumu((ulong)fmtptr.ptr, fmtptr.fmt); return *this; }

    /** \copydoc evo::Stream::operator<<(const FmtDump&) */
    This& operator<<(const FmtDump& fmt)
        { writefmtdump(fmt); return *this; }

protected:
    T        device_;       ///< I/O handle
    bool     owned_;        ///< Whether handle is owned (to be closed here)
    IoWriter bufwr_;        ///< Write buffer

    /** Initialize and reset buffers for a new stream. */
    void init(bool flushlines=false) {
        bufwr_.open(flushlines);
    }

private:
    StreamOut(const This&);
    This& operator=(const This&);
};

///////////////////////////////////////////////////////////////////////////////

/** Input stream for reading from pipe.
*/
class PipeIn : public StreamIn<IoFile> {
public:
    using StreamIn<IoFile>::Handle;

    /** Constructor.
     \param  exceptions  Whether to enable exceptions on error, default set by Evo config: EVO_EXCEPTIONS
    */
    PipeIn(bool exceptions=EVO_EXCEPTIONS) {
        excep(exceptions);
    }

private:
    PipeIn(const PipeIn&);
    PipeIn& operator=(const PipeIn&);
};

///////////////////////////////////////////////////////////////////////////////

/** Output stream for writing to pipe.
*/
class PipeOut : public StreamOut<IoFile> {
public:
    using StreamOut<IoFile>::Handle;

    /** Constructor.
     \param  exceptions  Whether to enable exceptions on error, default set by Evo config: EVO_EXCEPTIONS
    */
    PipeOut(bool exceptions=EVO_EXCEPTIONS) {
        excep(exceptions);
    }

private:
    PipeOut(const PipeOut&);
    PipeOut& operator=(const PipeOut&);
};

///////////////////////////////////////////////////////////////////////////////

/** %Pipe stream access.
*/
struct Pipe {
    PipeIn  in;
    PipeOut out;
};

///////////////////////////////////////////////////////////////////////////////

/** %Console I/O.
 - Use to read from console (STDIN) and/or write to console (STDOUT, STDERR)
 - Call Console::get() or con() (shortcut) to get a console reference, or use \link EVO_CONSOLE\endlink (shortcut macro)
 - Evo text I/O uses <em>automatic newline conversion</em> and supports all the common newline types (CR, LF, CRLF, LFCR), see Stream
 - For a \b thread-safe console see ConsoleMT
 .

\par Example

\code
#include <evo/io.h>
using namespace evo;
static Console& c = con();

int main() {
    // Newline (NL) flushes to console
    c.out << "testing stdout " << 123 << NL;
    c.err << "testing stderr " << 123 << NL;

    // Need to FLUSH so prompt appears before input is read
    c.out << "Type something: " << fFLUSH;

    String str;
    c.in.readline(str);
    c.out << "You typed: " << str << NL;

    return 0;
}
\endcode
*/
struct Console {
    typedef PipeOut::Format Format;     ///< \copydoc evo::StreamFormatter
    typedef PipeIn  InT;                ///< Input type
    typedef PipeOut OutT;               ///< Output type

    PipeIn  in;     ///< Read console input
    PipeOut out;    ///< Write to console, normal output
    PipeOut err;    ///< Write to console, error output

    /** Get console instance to use.
     \return  Console instance
    */
    static Console& get() {
        static Console console;
        return console;
    }

private:
    static const PipeIn::Handle  HIN  = 0;
    static const PipeOut::Handle HOUT = 1;
    static const PipeOut::Handle HERR = 2;

    static const ulong IN_BUFSZ  = 4096;
    static const ulong OUT_BUFSZ = 4096;
    static const ulong ERR_BUFSZ = 1024;

    Console() {
        in.bufread().resize(  IN_BUFSZ);
        out.bufwrite().resize(OUT_BUFSZ);
        err.bufwrite().resize(ERR_BUFSZ);

        in.attach( HIN,  false);
        out.attach(HOUT, false, false);
        err.attach(HERR, false, true);
    }

    Console(const Console&);
    Console& operator=(const Console&);
};

/** Shortcut for Console::get().
 \return  Console reference
*/
inline Console& con() {
    return Console::get();
}

///////////////////////////////////////////////////////////////////////////////

/** Extends an existing stream with mutex for multithreaded synchronization.
 - The stream is only thread safe when protected by this mutex
*/
template<class T,class M>
struct StreamExtMT {
    typedef StreamExtMT<T,M> This;    ///< This type

    M  mutex;       ///< Stream mutex
    T& stream;      ///< Stream object

    /** Constructor.
     \param  stream  %Stream to extend/wrap
    */
    StreamExtMT(T& stream) : stream(stream)
        { }

private:
    StreamExtMT(This&);
    This& operator=(This&);
};

///////////////////////////////////////////////////////////////////////////////

/** Single threaded console I/O.
 - This wraps existing Console streams with _inert_ synchronization to support the same interface as ConsoleMT for single threaded use
 - This allows a template class to support using both single threaded and multithreaded behavior via template parameter
 - Use Lock to lock the _inert_ mutex -- see MutexInert
 - This is _not_ thread safe
*/
struct ConsoleNotMT {
    typedef PipeOut::Format Format;                 ///< \copydoc evo::StreamFormatter
    typedef StreamExtMT<PipeIn,MutexInert>  InT;    ///< Input type
    typedef StreamExtMT<PipeOut,MutexInert> OutT;   ///< Output type
    typedef MutexInert::Lock Lock;                  ///< Mutex lock type

    InT  in;        ///< Read console input
    OutT out;       ///< Write to console, normal output
    OutT err;       ///< Write to console, error output

    /** Get console instance to use.
     \return  Console instance
    */
    static ConsoleNotMT& get()
        { static ConsoleNotMT console(con()); return console; }

private:
    ConsoleNotMT(Console& c) : in(c.in), out(c.out), err(c.err)
        { }

    ConsoleNotMT(const ConsoleNotMT&);
    ConsoleNotMT& operator=(const ConsoleNotMT&);
};

///////////////////////////////////////////////////////////////////////////////
//@}
}
#endif
