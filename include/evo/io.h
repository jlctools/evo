// Evo C++ Library
/* Copyright (c) 2016 Justin Crowell
 This Source Code Form is subject to the terms of the Mozilla Public
 License, v. 2.0. If a copy of the MPL was not distributed with this
 file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
///////////////////////////////////////////////////////////////////////////////
/** \file io.h Evo Input/Output streams. */
#pragma once
#ifndef INCL_evo_io_h
#define INCL_evo_io_h

// Includes
#include "substring.h"
#include "impl/sysio.h"

// Namespace: evo
namespace evo {

/** \addtogroup EvoIO */
//@{

///////////////////////////////////////////////////////////////////////////////

enum Flush { fFLUSH=0 };

/** Signals an output stream flush */
static const Flush FLUSH = fFLUSH;

///////////////////////////////////////////////////////////////////////////////

// TODO: move to string.h, support in String <<
// TODO: examples
#if 0

/** Formatting integer base.
 - Used with operator<<()
 - Integer formatting settings only apply to integers and don't expire, they apply until changed
*/
enum FmtBase {
    fBaseCurrent = 0,   ///< Current base (no change)
    fOct = 8,           ///< Base 8: octal
    fDec = 10,          ///< Base 10: decimal (default)
    fHex = 16           ///< Base 16: hexadecimal
};

/** Formatting integer base prefix type.
 - Used with operator<<()
 - Integer formatting settings only apply to integers and don't expire, they apply until changed
*/
enum FmtBasePrefix {
    fPrefixCurrent,     ///< Current prefix (no change)
    fNoPrefix,          ///< No base prefix (default)
    fPrefix,            ///< Standard base prefix (0x, 0)
    fPrefixCh           ///< Single character base prefix (x, 0)
};

/** Formatting alignment.
 - Used with operator<<()
 - Alignment settings don't expire, they remain until changed
*/
enum FmtAlign {
    fAlignCurrent,      ///< Current alignment (no change)
    fLeft,              ///< Align left
    fCenter,            ///< Align center
    fRight              ///< Align right
};

/** Holds string to use when formatting a null value.
 - This stores a reference to string pointer, which must remain valid -- best to use a literal
 - Used with operator<<()
 - Null value doesn't expire, it applies until changed

\par Example

\code
#include <evo/io.h>
using namespace evo;
static Console& c = con();

int main() {
    Int foo;
    c.out << FmtNull("null") << foo << NL;
    return 0;
}
\endcode

Output
\verbatim
null
\endverbatim
*/
struct FmtNull {
    SubString null;     ///< String for formatting null values

    /** Constructor. */
    FmtNull()
        { }

    /** Constructor.
     \param  null  Null string to set, must be terminated
    */
    FmtNull(const char* null) : null(null)
        { }

    /** Constructor.
     \param  null  Null string to set
    */
    FmtNull(const SubString& null) : null(null)
        { }
};

/** Formatting integer padding detail.
 - This sets width and padding character for formatting integers
 - Used with operator<<()
 - Integer formatting settings only apply to integers and don't expire, they apply until changed

\par Example

\code
#include <evo/io.h>
using namespace evo;
static Console& c = con();

int main() {
    c.out << 1 << ',' << FmtIntPad(2) << 1 << ',' << 20 << ',' << 0 << NL;
    c.out << FmtIntPad(3, '_') << 1 << ',' << 20 << ',' << 0 << NL;
    return 0;
}
\endcode

Output
\verbatim
1,01,20,00
__1,_20,__0
\endverbatim
*/
struct FmtIntPad {
    int  width;         ///< Width to fill to, 0 for none, -1 to ignore (leave current width) (default: 0)
    char ch;            ///< Fill character, 0 to ignore (leave current fill character) (default: space)

    /** Constructor.
     \param  width  Width to fill to, 0 for none, -1 to ignore (leave current width) (default: 0)
     \param  ch     Fill character
    */
    FmtIntPad(int width=-1, char ch=0) : width(width), ch(ch)
        { }

    /** Copy constructor.
     \param  src  Source to copy
    */
    FmtIntPad(const FmtIntPad& src) : width(src.width), ch(src.ch)
        { }

    /** Assignment operator.
     \param  src  Source to copy (width<0 and ch=0 are skipped)
     \return      This
    */
    FmtIntPad& operator=(const FmtIntPad& src) {
        if (src.width >= 0)
            width = src.width;
        if (ch != 0)
            ch = src.ch;
        return *this;
    }

    /** Reset to defaults.
     \return  This
    */
    FmtIntPad& reset() {
        width = 0;
        ch    = ' ';
        return *this;
    }
};

/** Formatting detail for integers.
 - Padding is added before the number and after prefix (if applicable), prefix doesn't count toward padding width
 - Used with operator<<()
 - Integer formatting settings only apply to integers and don't expire, they apply until changed

\par Example

\code
#include <evo/io.h>
using namespace evo;
static Console& c = con();

int main() {
    c.out << 123 << FmtInt(fHex, fPrefix, 4, '0') << 10 << ',' << 0x14 << ',' << 0x1ACFF << NL;
    c.out << FmtInt(fDec, 5, '_')  << 123 << NL;
    c.out << fOct << fNoPrefix << 5 << ',' << 456 << NL;
    return 0;
}
\endcode

Output
\verbatim
123,0x000A,0x0014,0x1ACFF
__123
5,710
\endverbatim
*/
struct FmtInt {
    int           base;     ///< Base for formatting (default: fDec)
    FmtBasePrefix prefix;   ///< Formatting prefix type (default: fNoPrefix)
    FmtIntPad     pad;      ///< Padding settings (default: none, spaces)

    /** Constructor.
     \param  base    Formatting base, fBaseCurrent to ignore (default: fDec)
     \param  prefix  Formatting prefix, fPrefixCurrent to ignore (default: fNoPrefix)
     \param  width   Padding Width to fill to, 0 for none, -1 to ignore (leave current width) (default: 0)
     \param  ch      Padding fill character, 0 to ignore (leave current fill character)
    */
    FmtInt(int base=fBaseCurrent, FmtBasePrefix prefix=fPrefixCurrent, int width=-1, char ch=0) : base(base), prefix(prefix), pad(width, ch)
        { }

    /** Constructor.
     \param  base    Formatting base, fBaseCurrent to ignore (default: fDec)
     \param  width   Padding Width to fill to, 0 for none, -1 to ignore (leave current width) (default: 0)
     \param  ch      Padding fill character, 0 to ignore (leave current fill character)
    */
    FmtInt(int base, int width, char ch=0) : base(fBaseCurrent), prefix(fPrefixCurrent), pad(width, ch)
        { }

    /** Copy constructor.
     \param  src  Source to copy
    */
    FmtInt(const FmtInt& src) :
        base(  src.base),
        prefix(src.prefix),
        pad(   src.pad)
        { }

    /** Assignment operator.
     \param  src  Source to copy (ignored values are skipped)
     \return      This
    */
    FmtInt& operator=(const FmtInt& src) {
        if (src.base > fBaseCurrent)
            base = src.base;
        if (src.prefix > fPrefixCurrent)
            prefix = src.prefix;
        pad = src.pad;
        return *this;
    }

    /** Reset to defaults.
     \return  This
    */
    FmtInt& reset() {
        base   = fDec;
        prefix = fNoPrefix;
        pad.reset();
        return *this;
    }
};

/** Formatting floating-point number padding detail.
 - Used with operator<<()
*/
struct FmtFloatPad {
    int  width;         ///< Width to fill to, 0 for none, -1 to ignore (leave current width) (default: 0)
    char ch;            ///< Fill character, 0 to ignore (leave current fill character) (default: space)

    /** Constructor.
     \param  width  Width to fill to, 0 for none, -1 to ignore (leave current width) (default: 0)
     \param  ch     Fill character
    */
    FmtFloatPad(int width=-1, char ch=0) : width(width), ch(ch)
        { }

    /** Copy constructor.
     \param  src  Source to copy
    */
    FmtFloatPad(const FmtFloatPad& src) : width(src.width), ch(src.ch)
        { }

    /** Assignment operator.
     \param  src  Source to copy (width<0 and ch=0 are skipped)
     \return      This
    */
    FmtFloatPad& operator=(const FmtFloatPad& src) {
        if (src.width >= 0)
            width = src.width;
        if (ch != 0)
            ch = src.ch;
        return *this;
    }

    /** Reset to defaults.
     \return  This
    */
    FmtFloatPad& reset() {
        width = 0;
        ch    = ' ';
        return *this;
    }
};

/** Formatting detail for floating-point numbers.
 - Used with operator<<()
*/
struct FmtFloat {
    int         precision;
    FmtFloatPad pad; // TODO

    FmtFloat(int precision=PREC_AUTO) : precision(precision)
        { }
};

/** Formatting as field detail.
 - Used with operator<<()
*/
struct FmtField {
    FmtAlign align;
    char     fill;

    FmtField(FmtAlign align=fLeft, char fill=' ') :
        align(align), fill(fill)
        { }
};

/** Set formatting width for next field.
 - This applies only to the next formatted value, after that width is reset to 0 (no width)
 - Used with operator<<()
*/
struct FmtWidth {
    int width;

    FmtWidth(int width) : width(width)
        { }
};

/** Formatting detail.
 - Used with operator<<()
*/
struct FmtOut {
    FmtNull  null;
    FmtInt   num_int;
    FmtFloat num_flt;
    FmtField field;
};

//template<class T>
//fmtwidth

#endif
///////////////////////////////////////////////////////////////////////////////

/** Base binary stream interface.
 - Interface for generic binary I/O, the next level interface adds text I/O: StreamBase
 - Reading a write-only stream will always return end-of-stream (0)
 - Writing to a read-only stream will return error EInval
 .
*/
class StreamBinBase : public SafeBool<StreamBinBase>
{
public:
    /** Constructor. */
    StreamBinBase() : error_(ENone), excep_(EVO_EXCEPTIONS)
        { }

    /** Destructor. */
    virtual ~StreamBinBase()
        { }

    /** Negation operator checks whether last operation set an error.
     - Call error() to get last error code
     .
     \return
    */
    bool operator!() const
        { return (error_ != ENone); }

    /** Get whether exceptions are enabled.
     \return  Whether exceptions enabled
    */
    bool excep() const
        { return excep_; }

    /** Set whether exceptions are enabled.
     \param  val  Whether to enable exceptions
    */
    void excep(bool val)
        { excep_ = val; }

    /** Get error code from last operation.
     \return  Error code, ENone for success (no error)
    */
    Error error() const
        { return error_; }

    /** Get whether stream is open.
     \return  Whether open
    */
    virtual bool isopen() const
        { return false; }

    // TODO: peek()?
    // TODO: skip()?

    /** Read binary input from stream.
     - This does a binary read -- no conversion on newlines
     - Depending on the stream type, this may:
       - be a blocking call
       - read at least 1 byte, but less than requested
       - return a read error
       .
     - Call error() to check error code
     - This never throws any exception
     .
     \param  buf   Buffer to store data read
     \param  size  Size to read
     \return       Bytes read and stored in buf, 0 if end-of-stream or error
    */
    virtual ulong readbin(char* buf, ulong size)
        { error_ = ENone; return 0; }

    /** Write binary output to stream.
     - This does a binary write -- no conversion on newlines
     - Depending on the stream type, this may:
       - be a blocking call
       - write at least 1 byte, but less than requested
         - StreamBase: always writes all data on success
         .
       - return a write error
       .
     - Call error() to check error code
     - This never throws any exception
     .
     \param  buf   Data to write
     \param  size  Size to write
     \return       Bytes written, 0 on error
    */
    virtual ulong writebin(const char* buf, ulong size)
        { error_ = EInval; EVO_THROW_ERR_CHECK(evo::ExceptionStreamOut, "Stream doesn't support writebin()", error_, excep_); return 0; }

    /** Flush any pending output in stream write buffer, if buffered.
     - This is a no-op if writes aren't buffered
     - Call error() to check error code
     .
     \return  Whether successful, false on error
    */
    virtual bool flush()
        { error_ = ENone; return true; }

protected:
    Error error_;        ///< Last error code
    bool  excep_;        ///< Whether to throw exceptions
};

///////////////////////////////////////////////////////////////////////////////

/** Base text and binary stream interface.
 - This extends StreamBinBase and adds reading and writing text
 - Text I/O automatically handles newline conversion
 - For absolute best performance use binary I/O, if possible
 .
*/
class StreamBase : public StreamBinBase
{
public:
    /** Destructor. */
    virtual ~StreamBase()
        { }

    /** Read text input from stream.
     - This does a text read, converting newlines as needed
     - Depending on the stream type, this may:
       - be a blocking call
       - read at least 1 byte, but less than requested
       - return a read error
       .
     - Call error() to check error code
     - After calling this with size=1 (not recommended), calling readline() next may trigger a special case error in certain conditions -- see readline() error code ELoss
     - This never throws any exception
     .
     \param  buf   Buffer to store data read
     \param  size  Size in bytes to read from file (must be positive)
     \return       Bytes read, 0 if end-of-file or error
    */
    virtual ulong readtext(char* buf, ulong size)
        { error_ = ENone; return 0; }

    /** Read text line input from stream.
     - This will read and return the next line as a string, not including the newline
     - Depending on the stream type, this may:
       - be a blocking call
       - read at least 1 byte, but less than requested
       - return a read error
       .
     - Call error() to check error code
     - Error codes:
       - EEnd if no more lines (end-of-file)
       - EOutOfBounds if line exceeds maxlen
       - ELoss if previous readtext() call left a pending partial newline pair that must be read first -- this prevents data loss on the newline pair
         - Only occurs when readtext() was used with size=1 while reading a newline pair (and newline conversion to 2-byte newline pairs is used),
           and next readline() is called in the middle of the newline pair, triggering this special case
         - Resolve by calling readtext() again with size=1 to read the pending newline char, then resume reading as desired
         - Calling readtext() with size=1 followed by readline() on same stream is best avoided so this special case never happens
         .
       - Other codes for read errors
     - This never throws any exception
     .
     \param  str     String to store line (cleared first) [out]
     \param  maxlen  Maximum line length, 0 for no limit
     \return         Whether successful, false if no more lines (end-of-file) or error
    */
    virtual bool readline(String& str, ulong maxlen=0)
        { error_ = ENone; return false; }

    /** Write text output to stream.
     - This does a text write, converting newlines as needed
     - Depending on the stream type, this may be a blocking call
     - Call error() to check error code
     - This never throws any exception
     .
     \param  buf   Data to write
     \param  size  Size to write
     \return       Size actually written, 0 on error
    */
    virtual ulong writetext(const char* buf, ulong size)
        { error_ = EInval; EVO_THROW_ERR_CHECK(evo::ExceptionStreamOut, "Stream doesn't support writetext()", error_, excep_); return 0; }

    /** Write text line output to stream.
     - This always writes the whole line followed by a newline on success
     - This does a text write, converting newlines as needed, which could turn into multiple lines
     - Depending on the stream type, this may be a blocking call
     - Call error() to check error code
     - This never throws any exception
     .
     \param  buf   Data buffer to write from
     \param  size  Data size to write in bytes
     \return       Size actually written (including newline), 0 on error
    */
    virtual ulong writeline(const char* buf, ulong size)
        { error_ = EInval; EVO_THROW_ERR_CHECK(evo::ExceptionStreamOut, "Stream doesn't support writeline()", error_, excep_); return 0; }

    /** Write a newline to stream.
     - This always writes the whole newline on success
     - Depending on the stream type, this may be a blocking call
     - Call error() to check error code
     - This never throws any exception
     .
     \return  Size actually written, 0 on error
    */
    virtual ulong writenewline()
        { error_ = EInval; EVO_THROW_ERR_CHECK(evo::ExceptionStreamOut, "Stream doesn't support writenewline()", error_, excep_); return 0; }
};

///////////////////////////////////////////////////////////////////////////////

/** Input/Output stream implementation.
 - This is not normally instantiated directly, see: File, Pipe, Console
 - This implements binary and text input/output with buffering
   - Binary I/O (no newline conversion): readbin(), writebin()
   - Text I/O (automatic newline conversion): readtext(), readline(), writetext(), writeline()
   - Use flush() to flush write buffer (text and binary)
   .
 - This also implements formatted output using operator<<()\n
   Note: Output formatting stops (is skipped) when an error is set, this allows chaining multiple << calls before checking for error\n
   Example:\code
stream << "test" << 123 << ',' << 12.3 << NL;
if (!stream) {
    // write error
}
   \endcode
 - Call error() to check error code from last operation
 - Evaluating as bool (via SafeBool) checks whether last operation was successful using ! operator!()\n
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
 \tparam  T  Low-level I/O stream implementing SysIoDevice to use
*/
template<class T>
class Stream : public StreamBase
{
public:
    typedef typename T::Handle Handle;      ///< Low-level handle type (OS dependent)
    typedef Stream<T>          This;        ///< This stream type

    // TODO
    //Iter iterlines()

    /** Constructor.
     \param  newlines  Newline type to use for text reading/writing
    */
    Stream(Newline newlines=NL) :
        owned_(false),
        bufrd_(0, newlines),
        bufwr_(0, newlines),
        savepos_(0),
        rwlast_(rwlNONE)
        { }

    /** Destructor.
     - Calls close()
     - Never throws an exception, even if flush fails
    */
    ~Stream()
        { close(); }

    /** Advanced: Access primary read buffer.
     - \b Caution: This returns a low-level interface that must be used properly
     - This never throws any exception
     .
     \return  Read buffer
    */
    RawBuffer& bufread()
        { return bufrd_.readbuf; }

    /** Advanced: Access primary write buffer.
     - \b Caution: This returns a low-level interface that must be used properly
     - This never throws any exception
     .
     \return  Write buffer
    */
    SysWriter& bufwrite()
        { return bufwr_; }

    /** Detach current stream.
     \return  Detached handle
    */
    Handle detach()
        { owned_ = false; return device_.detach(); }

    /** Close stream.
     - This will flush output before closing
     - Never throws an exception, even if flush fails
     .
     \return  Whether successful, false on flush error (stream will still close)
    */
    bool close() {
        if (bufwr_.used > 0)
            error_ = bufwr_.flush(device_);
        else
            error_ = ENone;
        if (owned_)
            device_.close();
        else
            device_.detach();
        owned_ = false;
        return (error_ == ENone);
    }

    bool isopen() const
        { return device_.isopen(); }

    ulong readbin(char* buf, ulong size) {
        if (rwlast_ != rwlREAD && !readprep())
            return 0;
        size = bufrd_.readbin(error_, device_, buf, size);
        EVO_THROW_ERR_CHECK(evo::ExceptionStreamIn, "Stream binary read failed", error_, (excep_ && error_ != ENone && error_ != EEnd));
        return size;
    }

    ulong readtext(char* buf, ulong size) {
        if (rwlast_ != rwlREAD && !readprep())
            return 0;
        size = bufrd_.readtext(error_, device_, buf, size);
        EVO_THROW_ERR_CHECK(evo::ExceptionStreamIn, "Stream text read failed", error_, (excep_ && error_ != ENone && error_ != EEnd));
        return size;
    }

    bool readline(String& str, ulong maxlen=0) {
        if (rwlast_ != rwlREAD && !readprep())
            return false;
        error_ = bufrd_.readline(str, device_, maxlen);
        EVO_THROW_ERR_CHECK(evo::ExceptionStreamIn, "Stream text line read failed", error_, (excep_ && error_ != ENone && error_ != EEnd));
        return (error_ == ENone);
    }

    // TODO readprevline()

    bool flush() {
        error_ = bufwr_.flush(device_);
        EVO_THROW_ERR_CHECK(evo::ExceptionStreamOut, "Stream flush failed", error_, (excep_ && error_ != ENone));
        return (error_ == ENone);
    }

    ulong writebin(const char* buf, ulong size) {
        if (rwlast_ != rwlWRITE && !writeprep())
            return 0;
        size = bufwr_.writebin(error_, device_, buf, size);
        EVO_THROW_ERR_CHECK(evo::ExceptionStreamOut, "Stream binary write failed", error_, (excep_ && error_ != ENone));
        return size;
    }

    ulong writetext(const char* buf, ulong size) {
        if (rwlast_ != rwlWRITE && !writeprep())
            return 0;
        size = bufwr_.writetext(error_, device_, buf, size);
        EVO_THROW_ERR_CHECK(evo::ExceptionStreamOut, "Stream text write failed", error_, (excep_ && error_ != ENone));
        return size;
    }

    ulong writeline(const char* buf, ulong size) {
        if (rwlast_ != rwlWRITE && !writeprep())
            return 0;
        ulong writtensize = bufwr_.writetext(error_, device_, buf, size);
        bufwr_.partnl = 0;
        if (writtensize == 0 || !bufwr_.writebin(error_, device_, bufwr_.newline, bufwr_.newlinesize) ) {
            EVO_THROW_ERR_CHECK(evo::ExceptionStreamOut, "Stream text line write failed", error_, excep_);
            return 0;
        }
        return writtensize + bufwr_.newlinesize;
    }

    ulong writenewline() {
        if (rwlast_ != rwlWRITE && !writeprep())
            return 0;
        bufwr_.partnl = 0;
        if (!bufwr_.writebin(error_, device_, bufwr_.newline, bufwr_.newlinesize)) {
            EVO_THROW_ERR_CHECK(evo::ExceptionStreamOut, "Stream newline write failed", error_, excep_);
            return 0;
        }
        return bufwr_.newlinesize;
    }

    // operator<<() overloads are copied to StreamOut
    // Note: It's tempting to move to base and call virtual writes, but then operator<<() returns base type and calls require VTABLE lookups

    /** Write a newline to stream and flush output.
     - This calls writenewline() then flush()
     .
     \return  This
    */
    This& operator<<(Newline)
        { if (error_ == ENone) { writenewline(); flush(); } return *this; }

    /** Flush buffer by writing to stream.
     - This calls flush()
     - This is useful when writing a console prompt, flush so the prompt is immediately visible
     .
     \return  This
    */
    This& operator<<(Flush)
        { if (error_ == ENone) flush(); return *this; }

    /** Write character to stream.
     - This calls writetext()
     .
     \param  ch  Character to write
     \return     This
    */
    This& operator<<(char ch)
        { if (error_ == ENone) writetext(&ch, 1); return *this; }

    /** Write terminated string to stream.
     - This calls writetext()
     .
     \param  str  String to write, must be terminated
     \return      This
    */
    This& operator<<(const char* str) {
        if (error_ == ENone && str != NULL) // TODO: write null indicator?
            writetext(str, strlen(str));
        return *this;
    }

    /** Write substring to stream.
     - This calls writetext()
     .
     \param  str  Substring to write
     \return      This
    */
    This& operator<<(const SubString& str) {
        if (error_ == ENone && str.size_ > 0) // TODO: write null indicator?
            writetext(str.data_, str.size_);
        return *this;
    }

    /** Write substring to stream.
     - This effectively calls writetext()
     - This handles all Evo character list or string types
     .
     \tparam  TSize  List size type (inferred from parameter)

     \param  str  Substring to write
     \return      This
    */
    template<class TSize>
    This& operator<<(const ListBase<char,TSize>& str) {
        if (error_ == ENone && str.size_ > 0) // TODO: write null indicator?
            writetext(str.data_, str.size_);
        return *this;
    }

    /** Write formatted number to stream.
     - This will flush current buffer if it's too full
     - This formats directly to write buffer so it must be large enough
     .
     \param  num  Number to format and write
     \return      This
    */
    This& operator<<(int num)
        { if (error_ == ENone) writenum(num); return *this; }

    /** Write formatted number to stream.
     - This will flush current buffer if it's too full
     - This formats directly to write buffer so it must be large enough
     .
     \param  num  Number to format and write
     \return      This
    */
    This& operator<<(long num)
        { if (error_ == ENone) writenum(num); return *this; }

    /** Write formatted number to stream.
     - This will flush current buffer if it's too full
     - This formats directly to write buffer so it must be large enough
     .
     \param  num  Number to format and write
     \return      This
    */
    This& operator<<(longl num)
        { if (error_ == ENone) writenum(num); return *this; }

    /** Write formatted number to stream.
     - This will flush current buffer if it's too full
     - This formats directly to write buffer so it must be large enough
     .
     \param  num  Number to format and write
     \return      This
    */
    This& operator<<(uint num)
        { if (error_ == ENone) writenumu(num); return *this; }

    /** Write formatted number to stream.
     - This will flush current buffer if it's too full
     - This formats directly to write buffer so it must be large enough
     .
     \param  num  Number to format and write
     \return      This
    */
    This& operator<<(ulong num)
        { if (error_ == ENone) writenumu(num); return *this; }

    /** Write formatted number to stream.
     - This will flush current buffer if it's too full
     - This formats directly to write buffer so it must be large enough
     .
     \param  num  Number to format and write
     \return      This
    */
    This& operator<<(ulongl num)
        { if (error_ == ENone) writenumu(num); return *this; }

    /** Write formatted floating-point number to stream.
     - This will flush current buffer if it's too full
     - This formats directly to write buffer so it must be large enough
     .
     \param  num  Number to format and write
     \return      This
    */
    This& operator<<(float num)
        { if (error_ == ENone) writenumf(num); return *this; }

    /** Write formatted floating-point number to stream.
     - This will flush current buffer if it's too full
     - This formats directly to write buffer so it must be large enough
     .
     \param  num  Number to format and write
     \return      This
    */
    This& operator<<(double num)
        { if (error_ == ENone) writenumf(num); return *this; }

    /** Write formatted floating-point number to stream.
     - This will flush current buffer if it's too full
     - This formats directly to write buffer so it must be large enough
     .
     \param  num  Number to format and write
     \return      This
    */
    This& operator<<(ldouble num)
        { if (error_ == ENone) writenumf(num); return *this; }

    // TODO: add Int, etc... uchar?

    // TODO
/*    This& operator<<(const FmtNull& fmt)
        { fmtout_.null = fmt; return *this; }

    This& operator<<(FmtBase base)
        { fmtout_.num_int.base = (int)base; return *this; }

    This& operator<<(const FmtIntPad& pad)
        { fmtout_.num_int.pad = pad; return *this; }

    This& operator<<(const FmtInt& fmt)
        { fmtout_.num_int = fmt; return *this; }

    This& operator<<(const FmtFloatPad& pad)
        { fmtout_.num_flt.pad = pad; return *this; }

    This& operator<<(const FmtFloat& fmt)
        { fmtout_.num_flt = fmt; return *this; }

    This& operator<<(const FmtField& fmt)
        { fmtout_.field = fmt; return *this; }

    This& operator<<(const FmtOut& fmt)
        { fmtout_ = fmt; return *this; }

    const FmtOut& fmtout() const
        { return fmtout_; }
*/
protected:
    enum RwLast {
        rwlNONE = 0,
        rwlREAD,
        rwlWRITE
    };

    T          device_;     ///< I/O device
    bool       owned_;      ///< Whether handle is owned (to be closed here)
    SysReader  bufrd_;      ///< Buffered reader
    SysWriter  bufwr_;      ///< Buffered writer
    ulongl     savepos_;    ///< Read/Write: Used to save buffered read position when switching between read/write
    RwLast     rwlast_;     ///< Read/Write: Used to track last operation when switching between read/write
    //FmtOut     fmtout_;     ///< Output formatting detail

    /** Initialize and reset buffers for a new stream. */
    void init() {
        savepos_ = 0;
        rwlast_  = rwlNONE;
    }

    /** Initialize buffer for writing. */
    void initwrite(bool flushlines=false) {
        if (bufwr_.size == 0)
            bufwr_.resize(SysWriter::DEFSIZE);
        bufwr_.flushlines = flushlines;
    }

    /** Write formatted signed number. */
    template<class TNum>
    bool writenum(TNum num, int base=10) {
        if (rwlast_ != rwlWRITE && !writeprep())
            return false;
        error_ = bufwr_.writetext_num(device_, num, base);
        EVO_THROW_ERR_CHECK(evo::ExceptionStreamOut, "Stream text write number failed", error_, (excep_ && error_ != ENone));
        return (error_ == ENone);
    }

    /** Write formatted unsigned number. */
    template<class TNum>
    bool writenumu(TNum num, int base=10) {
        if (rwlast_ != rwlWRITE && !writeprep())
            return false;
        error_ = bufwr_.writetext_numu(device_, num, base);
        EVO_THROW_ERR_CHECK(evo::ExceptionStreamOut, "Stream text write number failed", error_, (excep_ && error_ != ENone));
        return (error_ == ENone);
    }

    /** Write formatted floating-point number. */
    template<class TNum>
    bool writenumf(TNum num, int precision=PREC_AUTO) {
        if (rwlast_ != rwlWRITE && !writeprep())
            return false;
        error_ = bufwr_.writetext_numf(device_, num, precision);
        EVO_THROW_ERR_CHECK(evo::ExceptionStreamOut, "Stream text write number failed", error_, (excep_ && error_ != ENone));
        return (error_ == ENone);
    }

private:
    Stream(const This&);
    This& operator=(const This&);

    // Restore buffered read position (switching from write to read)
    bool readprep() {
        if (rwlast_ != rwlNONE) {
            if ((error_=bufwr_.flush(device_)) != ENone) {
                EVO_THROW_ERR_CHECK(evo::ExceptionStreamOut, "Stream flush failed before switch to read mode", error_, excep_);
                return false;
            }
            ulongl newpos = device_.pos(error_);
            if (error_ != ENone) {
                EVO_THROW_ERR_CHECK(evo::ExceptionStreamIn, "Stream position read failed during switch to read mode", error_, excep_);
                return false;
            }
            // TODO: account for read filters??
            if (newpos > savepos_ || bufrd_.readbuf.used > savepos_ || newpos < savepos_-bufrd_.readbuf.used) {
                bufrd_.readbuf.used = bufrd_.curbuf_offset = 0; // seeking outside buffered data
                device_.seek(error_, newpos, sBegin);
            } else {
                bufrd_.curbuf_offset = bufrd_.readbuf.used - (ulong)(savepos_ - newpos);
                device_.seek(error_, savepos_, sBegin);
            }
            if (error_ != ENone) {
                EVO_THROW_ERR_CHECK(evo::ExceptionStreamIn, "Stream seek failed during switch to read mode", error_, excep_);
                return false;
            }
            savepos_ = 0;
        }
        rwlast_ = rwlREAD;
        return true;
    }

    // Save buffered read position (switching from read to write)
    bool writeprep() {
        if (rwlast_ != rwlNONE) {
            savepos_ = device_.pos(error_);
            if (error_ != ENone) {
                EVO_THROW_ERR_CHECK(evo::ExceptionStreamOut, "Stream position read failed during switch to write mode", error_, excep_);
                return false;
            }
            // TODO: account for read filters??
            device_.seek(error_, savepos_-(bufrd_.readbuf.used-bufrd_.curbuf_offset), sBegin);
            if (error_ != ENone) {
                EVO_THROW_ERR_CHECK(evo::ExceptionStreamOut, "Stream seek failed during switch to write mode", error_, excep_);
                return false;
            }
        }
        rwlast_ = rwlWRITE;
        return true;
    }
};

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
 \tparam  T  Low-level I/O stream implementing SysIoDevice to use
*/
template<class T>
class StreamIn : public StreamBase
{
public:
    typedef typename T::Handle Handle;    ///< Low-level handle type (OS dependent)
    typedef StreamIn<T> This;            ///< This stream type

    /** Constructor.
     \param  newlines  Newline type to use for text reading
    */
    StreamIn(Newline newlines=NL) :
        owned_(false),
        bufrd_(0, newlines)
        { }

    /** \copydoc Stream::bufread() */
    RawBuffer& bufread()
        { return bufrd_.readbuf; }

    /** Attach existing stream.
     \param  handle  Handle to attach
     \param  owned   Whether to take ownership and close handle, false detaches on close()
    */
    void attach(Handle handle, bool owned=true) {
        close();
        device_.handle = handle;
        owned_         = owned;
        init();
    }

    /** \copydoc Stream::detach() */
    Handle detach()
        { owned_ = false; return device_.detach(); }

    /** Close stream.
     \return  Whether successful, always true
    */
    bool close() {
        if (owned_)
            device_.close();
        else
            device_.detach();
        owned_ = false;
        return true;
    }

    ulong readbin(char* buf, ulong size)
        { return bufrd_.readbin(error_, device_, buf, size); }

    ulong readtext(char* buf, ulong size)
        { return bufrd_.readtext(error_, device_, buf, size); }

    bool readline(String& str, ulong maxlen=0)
        { error_ = bufrd_.readline(str, device_, maxlen); return (error_ == ENone); }

protected:
    T         device_;      ///< I/O handle
    bool      owned_;       ///< TODO
    SysReader bufrd_;       ///< Buffered reader

    /** Initialize and reset buffers for a new stream. */
    void init() {
        bufrd_.readbuf.used = bufrd_.curbuf_offset = 0;
        if (bufrd_.readbuf.size == 0)
            bufrd_.readbuf.resize(SysReader::DEFSIZE);
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
 \tparam  T  Low-level I/O stream implementing SysIoDevice to use
*/
template<class T>
class StreamOut : public StreamBase
{
public:
    typedef typename T::Handle Handle;    ///< Low-level handle type (OS dependent)
    typedef StreamOut<T>       This;    ///< This stream type

    /** Constructor.
     \param  newlines  Newline type to use for text writing
    */
    StreamOut(Newline newlines=NL) :
        owned_(false),
        bufwr_(0, newlines)
        { }

    /** Destructor. */
    ~StreamOut()
        { close(); }

    /** \copydoc Stream::bufwrite() */
    SysWriter& bufwrite()
        { return bufwr_; }

    /** Attach existing stream.
     \param  handle      Handle to attach
     \param  owned       Whether to take ownership and close handle, false detaches on close()
     \param  flushlines  Whether to flush text output on newlines (line buffering)
    */
    void attach(Handle handle, bool owned=true, bool flushlines=false) {
        close();
        device_.handle = handle;
        owned_         = owned;
        init(flushlines);
    }

    /** \copydoc Stream::detach() */
    Handle detach()
        { owned_ = false; return device_.detach(); }

    /** Close stream.
     - This will flush output before closing
     .
     \return  Whether successful, false on flush error (stream will still close)
    */
    bool close() {
        if (bufwr_.used > 0)
            error_ = bufwr_.flush(device_);
        else
            error_ = ENone;
        if (owned_)
            device_.close();
        else
            device_.detach();
        owned_ = false;
        return (error_ == ENone);
    }

    bool isopen() const
        { return device_.isopen(); }

    bool flush()
        { return ( (error_=bufwr_.flush(device_)) == ENone ); }

    ulong writebin(const char* buf, ulong size) {
        return bufwr_.writebin(error_, device_, buf, size);
    }

    ulong writetext(const char* buf, ulong size) {
        return bufwr_.writetext(error_, device_, buf, size);
    }

    ulong writeline(const char* buf, ulong size) {
        ulong writtensize = bufwr_.writetext(error_, device_, buf, size);
        bufwr_.partnl = 0;
        if (writtensize == 0 || !bufwr_.writebin(error_, device_, bufwr_.newline, bufwr_.newlinesize) )
            return 0;
        return writtensize + bufwr_.newlinesize;
    }

    ulong writenewline() {
        bufwr_.partnl = 0;
        if (!bufwr_.writebin(error_, device_, bufwr_.newline, bufwr_.newlinesize))
            return 0;
        return bufwr_.newlinesize;
    }

    // operator<<() overloads must match Stream
    // Note: It's tempting to move to base and call virtual writes, but then operator<<() returns base type and calls require VTABLE lookups and

    /** Write a newline to stream and flush output.
     - This calls writenewline() then flush()
     .
     \return  This
    */
    This& operator<<(Newline)
        { if (error_ == ENone) { writenewline(); flush(); } return *this; }

    /** Flush buffer by writing to stream.
     - This calls flush()
     - This is useful when writing a console prompt, flush so the prompt is immediately visible
     .
     \return  This
    */
    This& operator<<(Flush)
        { if (error_ == ENone) flush(); return *this; }

    /** Write character to stream.
     - This effectively calls writetext()
     .
     \param  ch  Character to write
     \return     This
    */
    This& operator<<(char ch) {
        if (error_ == ENone) {
            bufwr_.writetext(error_, device_, &ch, 1);
            EVO_THROW_ERR_CHECK(evo::ExceptionStreamOut, "Stream text write (char) failed", error_, (excep_ && error_ != ENone));
        }
        return *this;
    }

    /** Write terminated string to stream.
     - This effectively calls writetext()
     .
     \param  str  String to write, must be terminated
     \return      This
    */
    This& operator<<(const char* str) {
        if (error_ == ENone && str != NULL) { // TODO: write null indicator?
            bufwr_.writetext(error_, device_, str, strlen(str));
            EVO_THROW_ERR_CHECK(evo::ExceptionStreamOut, "Stream text write (char*) failed", error_, (excep_ && error_ != ENone));
        }
        return *this;
    }

    /** Write substring to stream.
     - This effectively calls writetext()
     .
     \param  str  Substring to write
     \return      This
    */
    This& operator<<(const SubString& str) {
        if (error_ == ENone) {
            bufwr_.writetext(error_, device_, str.data_, str.size_);
            EVO_THROW_ERR_CHECK(evo::ExceptionStreamOut, "Stream text write (SubString) failed", error_, (excep_ && error_ != ENone));
        }
        return *this;
    }

    /** Write substring to stream.
     - This effectively calls writetext()
     - This handles all Evo character list or string types
     .
     \tparam  TSize  List size type (inferred from parameter)

     \param  str  Substring to write
     \return      This
    */
    template<class TSize>
    This& operator<<(const ListBase<char,TSize>& str) {
        if (error_ == ENone) {
            bufwr_.writetext(error_, device_, str.data_, str.size_);
            EVO_THROW_ERR_CHECK(evo::ExceptionStreamOut, "Stream text write (ListBase) failed", error_, (excep_ && error_ != ENone));
        }
        return *this;
    }

    /** Write formatted number to stream.
     - This will flush current buffer if it's too full
     - This formats directly to write buffer so it must be large enough
     .
     \param  num  Number to format and write
     \return      This
    */
    This& operator<<(int num) {
        if (error_ == ENone) {
            error_ = bufwr_.writetext_num(device_, num, 10);
            EVO_THROW_ERR_CHECK(evo::ExceptionStreamOut, "Stream text write (int) failed", error_, (excep_ && error_ != ENone));
        }
        return *this;
    }

    /** Write formatted number to stream.
     - This will flush current buffer if it's too full
     - This formats directly to write buffer so it must be large enough
     .
     \param  num  Number to format and write
     \return      This
    */
    This& operator<<(long num) {
        if (error_ == ENone) {
            error_ = bufwr_.writetext_num(device_, num, 10);
            EVO_THROW_ERR_CHECK(evo::ExceptionStreamOut, "Stream text write (long) failed", error_, (excep_ && error_ != ENone));
        }
        return *this;
    }

    /** Write formatted number to stream.
     - This will flush current buffer if it's too full
     - This formats directly to write buffer so it must be large enough
     .
     \param  num  Number to format and write
     \return      This
    */
    This& operator<<(longl num) {
        if (error_ == ENone) {
            error_ = bufwr_.writetext_num(device_, num, 10);
            EVO_THROW_ERR_CHECK(evo::ExceptionStreamOut, "Stream text write (longl) failed", error_, (excep_ && error_ != ENone));
        }
        return *this;
    }

    /** Write formatted number to stream.
     - This will flush current buffer if it's too full
     - This formats directly to write buffer so it must be large enough
     .
     \param  num  Number to format and write
     \return      This
    */
    This& operator<<(uint num) {
        if (error_ == ENone) {
            error_ = bufwr_.writetext_numu(device_, num, 10);
            EVO_THROW_ERR_CHECK(evo::ExceptionStreamOut, "Stream text write (uint) failed", error_, (excep_ && error_ != ENone));
        }
        return *this;
    }

    /** Write formatted number to stream.
     - This will flush current buffer if it's too full
     - This formats directly to write buffer so it must be large enough
     .
     \param  num  Number to format and write
     \return      This
    */
    This& operator<<(ulong num) {
        if (error_ == ENone) {
            error_ = bufwr_.writetext_numu(device_, num, 10);
            EVO_THROW_ERR_CHECK(evo::ExceptionStreamOut, "Stream text write (ulong) failed", error_, (excep_ && error_ != ENone));
        }
        return *this;
    }

    /** Write formatted number to stream.
     - This will flush current buffer if it's too full
     - This formats directly to write buffer so it must be large enough
     .
     \param  num  Number to format and write
     \return      This
    */
    This& operator<<(ulongl num) {
        if (error_ == ENone) {
            error_ = bufwr_.writetext_numu(device_, num, 10);
            EVO_THROW_ERR_CHECK(evo::ExceptionStreamOut, "Stream text write (ulongl) failed", error_, (excep_ && error_ != ENone));
        }
        return *this;
    }

    /** Write formatted floating-point number to stream.
     - This will flush current buffer if it's too full
     - This formats directly to write buffer so it must be large enough
     .
     \param  num  Number to format and write
     \return      This
    */
    This& operator<<(float num) {
        if (error_ == ENone) {
            error_ = bufwr_.writetext_numf(device_, num, PREC_AUTO);
            EVO_THROW_ERR_CHECK(evo::ExceptionStreamOut, "Stream text write (float) failed", error_, (excep_ && error_ != ENone));
        }
        return *this;
    }

    /** Write formatted floating-point number to stream.
     - This will flush current buffer if it's too full
     - This formats directly to write buffer so it must be large enough
     .
     \param  num  Number to format and write
     \return      This
    */
    This& operator<<(double num) {
        if (error_ == ENone) {
            error_ = bufwr_.writetext_numf(device_, num, PREC_AUTO);
            EVO_THROW_ERR_CHECK(evo::ExceptionStreamOut, "Stream text write (double) failed", error_, (excep_ && error_ != ENone));
        }
        return *this;
    }

    /** Write formatted floating-point number to stream.
     - This will flush current buffer if it's too full
     - This formats directly to write buffer so it must be large enough
     .
     \param  num  Number to format and write
     \return      This
    */
    This& operator<<(ldouble num) {
        if (error_ == ENone) {
            error_ = bufwr_.writetext_numf(device_, num, PREC_AUTO);
            EVO_THROW_ERR_CHECK(evo::ExceptionStreamOut, "Stream text write (ldouble) failed", error_, (excep_ && error_ != ENone));
        }
        return *this;
    }

protected:
    T         device_;      ///< I/O handle
    bool      owned_;       ///< Whether handle is owned (to be closed here)
    SysWriter bufwr_;       ///< Write buffer

    /** Initialize and reset buffers for a new stream. */
    void init(bool flushlines=false) {
        bufwr_.used = 0;
        if (bufwr_.size == 0)
            bufwr_.resize(SysWriter::DEFSIZE);
        bufwr_.flushlines = flushlines;
    }

private:
    StreamOut(const This&);
    This& operator=(const This&);
};

///////////////////////////////////////////////////////////////////////////////

/** %File I/O stream.
 - Use to read and write files
 - This is a Stream (read/write) using I/O device SysFile
 - This throws an ExceptionStream on error if exceptions are enabled, otherwise use error() to check for error

\par Example

\code
#include <evo/io.h>
using namespace evo;
static Console& c = con();

int main() {
    const char* filename = "tmpfile.txt";

    // Write new file
    {
        File file(filename, oWriteNew);
        file << "line one" << NL;
        file << "line " << 2 << NL;
        file << "line three" << NL;
    }

    // Read and print file
    {
        File file(filename);
        for (String line; file.readline(line); )
            c.out << "> " << line << NL;
    }

    return 0;
}
\endcode
*/
class File : public Stream<SysFile>
{
public:
    typedef Stream<SysFile> Base;    ///< Base class alias

    /** Constructor.
     - This initializes without opening a file, use open() to open a file
     .
     \param  newlines    Newline type to use for text reads/writes
     \param  exceptions  Whether to enable exceptions on error, default set by Evo config: EVO_EXCEPTIONS
    */
    File(Newline newlines=NL, bool exceptions=EVO_EXCEPTIONS) : Base(newlines)
        { excep(exceptions); }

    /** Constructor to open file.
     - If exceptions disabled: Call error() to check for error
     - Throws ExceptionStreamOpen on error, if exceptions enabled
     .
     \param  path        File path to use
     \param  mode        Access mode to use
     \param  flushlines  Whether to flush text output on newlines (line buffering)
     \param  newlines    Newline type to use for text reads/writes
     \param  exceptions  Whether to enable exceptions on error, default set by Evo config: EVO_EXCEPTIONS
    */
    File(const char* path, Open mode=oRead, bool flushlines=false, Newline newlines=NL, bool exceptions=EVO_EXCEPTIONS) : Base(newlines)
        { excep(exceptions); open(path, mode, flushlines); }

    /** Open file for read and/or writing.
     - Current file is closed first
     - Throws ExceptionStreamOpen on error, if exceptions enabled
     .
     \param  path        File path to use
     \param  mode        Access mode to use
     \param  flushlines  Whether to flush text output on newlines (line buffering)
     \return             Whether successful, false on error -- call error() for error code
    */
    bool open(const char* path, Open mode=oRead, bool flushlines=false) {
        const bool result = ((error_=device_.open(path, mode)) == ENone);
        if (result) {
            Base::init();
            switch (mode) {
                case oRead:
                    bufrd_.open();
                    break;
                case oWrite:
                case oWriteNew:
                case oAppend:
                case oAppendNew:
                    Base::initwrite(flushlines);
                    break;
                case oReadWrite:
                case oReadWriteNew:
                case oReadAppend:
                case oReadAppendNew:
                    bufrd_.open();
                    Base::initwrite(flushlines);
                    break;
            }
            owned_ = true;
        } else if (excep_ && error_ != ENone) {
            EVO_THROW_ERR(evo::ExceptionStreamOpen, "File::open() failed", error_);
        }
        return result;
    }

    /** Get current file position.
     - Call error() to check error code
     .
     \return  File position on success, 0 on error
    */
    ulongl pos() {
        assert( bufrd_.curbuf_offset <= bufrd_.readbuf.used );
        if (savepos_ > 0)
            return device_.pos(error_) + bufwr_.used;
        return device_.pos(error_) - (bufrd_.readbuf.used-bufrd_.curbuf_offset) + (bufwr_.used);
    }

    /** Seek to file position.
     - Call error() to check error code
     .
     \param  offset  Offset from start to seek to
     \param  start   Start position to seek from
     \return         New file position after seek, 0 for beginning or error
    */
    ulongl seek(ulongl offset, Seek start=sBegin) {
        if (bufwr_.used > 0) {
            if (!flush())
                return 0;
            savepos_ = 0;
        }
        if (bufrd_.readbuf.used > 0)
            bufrd_.readbuf.used = bufrd_.curbuf_offset = 0;
        return device_.seek(error_, offset, start);
    }

/*    static Error mkdir(const char* path)
        { return SysFile::mkdir(path); }

    static Error rmdir(const char* path)
        { return SysFile::mkdir(path); }*/

private:
    File(const File&);
    File& operator=(const File&);
};

///////////////////////////////////////////////////////////////////////////////

/** Input stream for reading from pipe.
*/
class PipeIn : public StreamIn<SysFile>
{
public:
    using StreamIn<SysFile>::Handle;

    /** Constructor.
     \param  exceptions  Whether to enable exceptions on error, default set by Evo config: EVO_EXCEPTIONS
    */
    PipeIn(bool exceptions=EVO_EXCEPTIONS)
        { excep(exceptions); }

private:
    PipeIn(const PipeIn&);
    PipeIn& operator=(const PipeIn&);
};

///////////////////////////////////////////////////////////////////////////////

/** Output stream for writing to pipe.
*/
class PipeOut : public StreamOut<SysFile>
{
public:
    using StreamOut<SysFile>::Handle;

    /** Constructor.
     \param  exceptions  Whether to enable exceptions on error, default set by Evo config: EVO_EXCEPTIONS
    */
    PipeOut(bool exceptions=EVO_EXCEPTIONS)
        { excep(exceptions); }

private:
    PipeOut(const PipeOut&);
    PipeOut& operator=(const PipeOut&);
};

///////////////////////////////////////////////////////////////////////////////

// TODO: Needed? Add constructor/create()?
/** Pipe stream access.
*/
struct Pipe
{
    PipeIn  in;
    PipeOut out;
};

///////////////////////////////////////////////////////////////////////////////

/** %Console I/O.
 - Use to read from console (STDIN) and/or write to console (STDOUT, STDERR)
 - Implemented as a singleton, call Console::get() or con() to get a reference
 - Evo text I/O uses automatic newline conversion and supports all the big newline types (CR, LF, CRLF, LFCR), see Stream
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
    c.out << "Type something: " << FLUSH;

    String str;
    c.in.readline(str);
    c.out << "You typed: " << str << NL;

    return 0;
}
\endcode
*/
struct Console
{
    PipeIn  in;     ///< Read console input
    PipeOut out;    ///< Write console normal output
    PipeOut err;    ///< Write console error output

    // TODO: redirect err -> out, and back

    /** Get console instance to use.
     \return  Console instance
    */
    static Console& get()
        { static Console console; return console; }

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
inline Console& con()
    { return Console::get(); }

///////////////////////////////////////////////////////////////////////////////

// TODO: Use Array?
// TODO: Useful? Yes for MemStream
#if 0
/** %Buffer for holding and streaming data.
*/
class Buffer : public Stream
{
public:
    /** Constructor creates empty buffer.
     - Call resize() to set size
    */
    Buffer() {
        data_ = NULL;
        size_ = 0;
        pos_  = 0;
        used_ = 0;
    }

    /** Constructor creates empty buffer.
     \param  size  Buffer size to allocate
    */
    Buffer(ulong size) {
        data_ = (char*)malloc(size);
        size_ = size;
        pos_  = 0;
        used_ = 0;
    }

    /** Destructor. */
    ~Buffer() {
        if (data_ != NULL)
            free(data_);
    }

    /** Get buffer size allocated.
     \return  Buffer size
    */
    ulong size() const
        { return size_; }

    /** Get buffer size used.
     \return  Buffer used size, 0 if unused
    */
    ulong used() const
        { return used_; }

    /** Get buffer size available (not used).
     \return  Buffer size available
    */
    ulong avail() const
        { return (size_ - used_); }

    /** Resize buffer.
     - This resizes by allocating a new buffer and preserves existing data, if possible
     .
     \param  size  New size, 0 to free buffer and leave empty
    */
    void resize(ulong size) {
        if (size != size_) {
            if (size > 0) {
                char* olddata = data_;
                data_ = (char*)malloc(size);
                if (used_ > 0) {
                    if (used_ > size)
                        used_ = size;
                    memcpy(data_, olddata+pos_, used_);
                }
                size_ = size;
                pos_  = 0;
                free(olddata);
            } else if (data_ != NULL) {
                free(data_);
                data_ = NULL;
                size_ = 0;
                pos_  = 0;
                used_ = 0;
            }
        }
    }

    /** Get buffer pointer for read/write.
     - After writing to buffer, call bufupd() to set new size used
     .
     \return  Buffer pointer, NULL if none
    */
    char* buf() {
        assert( used_ <= size_ );
        if (used_ > 0 && pos_ > 0) {
            memmove(data_, data_+pos_, used_);
            pos_ = 0;
        }
        return data_;
    }

    /** Set new buffer size used after update.
     - Call this to set new size used after writing to buffer using buf()
     .
     \param  size  New size used
    */
    void bufupd(ulong size)
        { used_ = (size > size_ ? size_ : size); }

    /** Peek at read buffer (const).
     - Call used() to get buffer size used
     - Call read() with buf=NULL to "consume" peeked data, with up to size=used()
     .
     \return  Buffer data
    */
    const char* peek() const
        { return (data_ + pos_); }

    using Stream::read;
    using Stream::write;

    // Documented in parent
    ulong read(Error& err, char* buf, ulong size) {
        assert( used_ <= size_ );
        err = ENone;
        if (size > used_)
            size = used_;
        if (size > 0) {
            if (buf != NULL)
                memcpy(buf, data_+pos_, size);
            pos_  += size;
            used_ -= size;
        }
        return size;
    }

    // Documented in parent
    ulong write(Error& err, const char* buf, ulong size) {
        assert( used_ <= size_ );
        err = ENone;
        const ulong avail = (size_ - used_);
        if (size > avail)
            size = avail;
        if (size > 0) {
            if (size > (avail - pos_)) {
                memmove(data_, data_+pos_, used_);
                pos_ = 0;
            }
            memcpy(data_+used_, buf, size);
            used_ += size;
        }
        return size;
    }

    /** Fill buffer by reading from stream.
     - This will read from given stream until the buffer is full, end-of-stream reached, or error occurs
     .
     \param  in  Stream to read from
     \return     Bytes read from stream and stored, 0 if end-of-stream or error
    */
    ulong fill(Stream& in) {
        assert( used_ <= size_ );
        if (used_ > 0 && pos_ > 0) {
            memmove(data_, data_+pos_, used_);
            pos_ = 0;
        }

        ulong readtotal = 0;
        ulong readsize;
        while (used_ < size_) {
            readsize = in.read(err, data_+used_, (size_-used_));
            if (readsize == 0 || err != ENone)
                break;
            used_     += readsize;
            readtotal += readsize;
        }
        return readtotal;
    }

    /** Fill buffer by reading from stream with exceptions.
     - This calls fill(Error&,Stream&) and throws exception ExceptionIoRead on error
     - For best performance use fill(Error&,Stream&) since exceptions add overhead
     .
     \param  in  Stream to read from
     \return     Bytes read from stream and stored, 0 if end-of-stream
    */
    ulong fill(Stream& in) {
        Error err;
        ulong result = fill(err, in);
        if (err != ENone)
            THROW_T(ExceptionIoRead, error_msg(err));
        return result;
    }

private:
    char* data_;
    ulong size_;
    ulong pos_;
    ulong used_;
};
#endif

///////////////////////////////////////////////////////////////////////////////
//@}
} // Namespace: evo
#endif
