// Evo C++ Library
/* Copyright 2019 Justin Crowell
Distributed under the BSD 2-Clause License -- see included file LICENSE.txt for details.
*/
///////////////////////////////////////////////////////////////////////////////
/** \file iobase.h Evo base I/O stream classes. */
#pragma once
#ifndef INCL_evo_iobase_h
#define INCL_evo_iobase_h

#include "substring.h"
#include "impl/sysio.h"

// Namespace: evo
namespace evo {

/** \addtogroup EvoIO
Evo input/output streams
*/
//@{

///////////////////////////////////////////////////////////////////////////////

/** Base binary stream interface.
 - Interface for generic binary I/O, the next level interface adds text I/O: StreamBase
 - Reading a write-only stream will always return end-of-stream (0)
 - Writing to a read-only stream will return error ENotImpl
 .
*/
struct IoBase : public SafeBool<IoBase> {
    /** Constructor. */
    IoBase() : error_(ENone), excep_(EVO_EXCEPTIONS) {
    }

    /** Destructor. */
    virtual ~IoBase() {
    }

    /** Negation operator checks whether an error was set by a previous operation.
     - Call error() to get last error code
     - Alternatively, use SafeBool evaluation to check whether previous operations were successful
     .
     \return  Whether last operation set an error
    */
    bool operator!() const {
        return (error_ != ENone);
    }

    /** Get whether exceptions are enabled.
     \return  Whether exceptions enabled
    */
    bool excep() const {
        return excep_;
    }

    /** %Set whether exceptions are enabled.
     \param  val  Whether to enable exceptions
    */
    void excep(bool val) {
        excep_ = val;
    }

    /** Get error code from last operation.
     \return  Error code, ENone for success (no error)
    */
    Error error() const {
        return error_;
    }

    /** Get whether stream is open.
     \return  Whether open
    */
    virtual bool isopen() const {
        return false;
    }

    /** Read binary input from stream.
     - This does a binary read -- no conversion on newlines
     - Depending on the stream type, this may:
       - be a blocking call
       - read at least 1 byte, but less than requested
       - return a read error, some stream types may timeout
       .
     - Call error() to check error code
     - This never throws any exception
     .
     \param  buf   Buffer to store data read
     \param  size  Size to read
     \return       Bytes read and stored in buf, 0 if end-of-stream or error
    */
    virtual ulong readbin(void* buf, ulong size) {
        EVO_PARAM_UNUSED(buf);
        EVO_PARAM_UNUSED(size);
        error_ = ENone;
        return 0;
    }

    /** Write binary output to stream.
     - This does a binary write -- no conversion on newlines
     - Depending on the stream type, this may:
       - be a blocking call
       - write at least 1 byte, but less than requested
         - StreamBase: always writes all data on success
         .
       - return a write error, some stream types may timeout
       .
     - Call error() to check error code
     .
     \param  buf   Data to write
     \param  size  Size to write
     \return       Bytes written, 0 on error
    */
    virtual ulong writebin(const void* buf, ulong size) {
        EVO_PARAM_UNUSED(buf);
        EVO_PARAM_UNUSED(size);
        error_ = ENotImpl;
        EVO_THROW_ERR_CHECK(evo::ExceptionStreamOut, "Stream doesn't support writebin()", error_, excep_);
        return 0;
    }

    /** Flush any pending output in stream write buffer, if buffered.
     - This is a no-op if writes aren't buffered
     - Call error() to check error code
     .
     \return  Whether successful, false on error
    */
    virtual bool flush() {
        error_ = ENone;
        return true;
    }

protected:
    Error error_;        ///< Last error code
    bool  excep_;        ///< Whether to throw exceptions
};

///////////////////////////////////////////////////////////////////////////////

/** Base text and binary stream interface.
 - This extends IoBase and adds reading and writing text
 - Text I/O automatically handles newline conversion
 - For absolute best performance use binary I/O, if possible
 .
*/
struct StreamBase : public IoBase {
    typedef ulong Size;     ///< Data size type (ulong)

    /** Constructor. */
    StreamBase() {
    }

    /** Destructor. */
    virtual ~StreamBase() {
    }

    /** Read text input from stream.
     - This does a text read, converting newlines as needed
     - Depending on the stream type, this may:
       - be a blocking call
       - read at least 1 byte, but less than requested
       - return a read error
       .
     - Call error() to check error code
     - After calling this with size=1 (not recommended), calling readline() next may trigger a special case error in certain conditions -- see readline() error code ELoss
     .
     \param  buf   Buffer to store data read
     \param  size  Size in bytes to read from file (must be positive)
     \return       Bytes read, 0 if end-of-file or error
    */
    virtual ulong readtext(char* buf, ulong size) {
        EVO_PARAM_UNUSED(buf);
        EVO_PARAM_UNUSED(size);
        error_ = ENone;
        return 0;
    }

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
    virtual bool readline(String& str, ulong maxlen=0) {
        EVO_PARAM_UNUSED(str);
        EVO_PARAM_UNUSED(maxlen);
        error_ = ENone;
        return false;
    }

    /** Write repeat character as text output to stream.
     - This does a text write, converting newlines as needed
       - Note that writing newline text characters 1 char at a time or by string will give the same end result either way -- the edge cases are covered
       .
     - Depending on the stream type, this may be a blocking call
     - If exceptions are disabled, call error() to check error code
     .
     \param  ch     Character to write
     \param  count  Character count to write, must be positive
     \return        Size actually written (should be the same as count), 0 on error
    */
    virtual ulong writechar(char ch, ulong count=1) {
        EVO_PARAM_UNUSED(ch);
        EVO_PARAM_UNUSED(count);
        error_ = ENotImpl;
        EVO_THROW_ERR_CHECK(evo::ExceptionStreamOut, "Stream doesn't support writechar()", error_, excep_);
        return 0;
    }

    /** Write text output to stream.
     - This does a text write, converting newlines as needed
     - Depending on the stream type, this may be a blocking call
     - If exceptions are disabled, call error() to check error code
     .
     \param  buf   Data to write
     \param  size  Size to write
     \return       Size actually written, 0 on error
    */
    virtual ulong writetext(const char* buf, ulong size) {
        EVO_PARAM_UNUSED(buf);
        EVO_PARAM_UNUSED(size);
        error_ = ENotImpl;
        EVO_THROW_ERR_CHECK(evo::ExceptionStreamOut, "Stream doesn't support writetext()", error_, excep_);
        return 0;
    }

    /** Write quoted text output to string.
     - Newlines and unprintable characters are written as-is
     - This uses Smart Quoting -- see \ref SmartQuoting
     .
     \param  buf       Data to quote and write
     \param  size      Data size to write
     \param  delim     Delimiter for next field to escape via quoting
     \param  optional  Whether quoting is optional, true to avoid quoting if possible
     \return           Size actually written, 0 on error (write error or unquotable text)
    */
    virtual Size writequoted(const char* buf, Size size, char delim, bool optional=false) {
        EVO_PARAM_UNUSED(buf);
        EVO_PARAM_UNUSED(size);
        EVO_PARAM_UNUSED(delim);
        EVO_PARAM_UNUSED(optional);
        error_ = ENotImpl;
        EVO_THROW_ERR_CHECK(evo::ExceptionStreamOut, "Stream doesn't support writequoted()", error_, excep_);
        return 0;
    }

    /** Write text line output to stream.
     - This always writes the whole line followed by a newline on success
     - This does a text write, converting newlines as needed, which could turn into multiple lines
     - Depending on the stream type, this may be a blocking call
     - If exceptions are disabled, call error() to check error code
     .
     \param  buf   Data buffer to write from
     \param  size  Data size to write in bytes, must be positive
     \return       Size actually written (including newline), 0 on error
    */
    virtual ulong writeline(const char* buf, ulong size) {
        EVO_PARAM_UNUSED(buf);
        EVO_PARAM_UNUSED(size);
        error_ = ENotImpl;
        EVO_THROW_ERR_CHECK(evo::ExceptionStreamOut, "Stream doesn't support writeline()", error_, excep_);
        return 0;
    }

    /** Get pointer for writing directly to buffer to append data.
     - Call write_direct_finish() to commit written data, or don't to cancel
     - This will flush buffer to make room, if needed
     - Newlines in data written aren't converted
     .
     \param  size  Requred size in bytes to reserve
     \return       Buffer to write to (at append position), NULL on error or if buffer not large enough or if not supported
    */
    virtual char* write_direct(Size size) {
        EVO_PARAM_UNUSED(size);
        error_ = ENotImpl;
        EVO_THROW_ERR_CHECK(evo::ExceptionStreamOut, "Stream doesn't support write_direct()", error_, excep_);
        return NULL;
    }

    /** Get pointer for writing directly to buffer to append data and allow multiple passes for larger sizes.
     - Call write_direct_flush() or write_direct_finish() to commit written data, or neither to cancel
     - If `reserve_size` is 0 then this does nothing and returns a non-NULL but invalid pointer
     - This will flush buffer to make room, if needed
     - Newlines in data written aren't converted
     .
     \param  available     Stores available size reserved in bytes, may be less than `reserve_size`, 0 if `reserve_size` was 0  [out]
     \param  reserve_size  Requred size in bytes to reserve
     \return               Buffer to write to (at append position), NULL on error or if not supported
    */
    char* write_direct_multi(Size& available, Size reserve_size) {
        EVO_PARAM_UNUSED(available);
        EVO_PARAM_UNUSED(reserve_size);
        error_ = ENotImpl;
        EVO_THROW_ERR_CHECK(evo::ExceptionStreamOut, "Stream doesn't support write_direct_multi()", error_, excep_);
        return NULL;
    }

    /** Flush data written directly to buffer and get pointer for appending more.
     - This commits data written directly after previous call or write_direct_multi(), which must be called first
     - If `reserve_size` is 0 then this does the same as write_direct_finish() and returns a non-NULL but invalid pointer on success
     - Newlines in data written aren't converted
     .
     \param  available     Stores available size reserved in bytes, may be less than `reserve_size`, 0 if `reserve_size` was 0  [out]
     \param  written_size  Size written in bytes to flush, must not be greater than `available` size from previous call to this or write_direct_multi()
     \param  reserve_size  Requred size in bytes to reserve, 0 to finish
     \return               Buffer to write to (at append position), NULL on error or if not supported
    */
    char* write_direct_flush(Size& available, Size written_size, Size reserve_size) {
        EVO_PARAM_UNUSED(available);
        EVO_PARAM_UNUSED(written_size);
        EVO_PARAM_UNUSED(reserve_size);
        error_ = ENotImpl;
        EVO_THROW_ERR_CHECK(evo::ExceptionStreamOut, "Stream doesn't support write_direct_flush()", error_, excep_);
        return NULL;
    }

    /** Finish writing directly to buffer.
     - This commits data written directly after calling write_direct() or write_direct_multi(), one of which must be called first
     - Newlines in data written aren't converted
     .
     \param  size  Size written in bytes, must not be greater than `size` passed to write_direct()
     \return       Whether successful, false if not supported
    */
    virtual bool write_direct_finish(Size size) {
        EVO_PARAM_UNUSED(size);
        error_ = ENotImpl;
        EVO_THROW_ERR_CHECK(evo::ExceptionStreamOut, "Stream doesn't support write_direct_finish()", error_, excep_);
        return false;
    }
};

///////////////////////////////////////////////////////////////////////////////

/** %Stream output formatter with state.
 - This is associated with a Stream object and supports stateful (i.e. sticky) output formatting with "<<" operator on this object
   - Use Stream::Format (where Stream is the stream type used)
 - Formatting attributes include:
   - Aligned fields for formatting text "columns"
   - Integer base, prefix, and padding (in addition to field padding)
   - Floating point precision and padding (in additon to field padding)
   - %String to use for "null" values (null char*, String, \link evo::Int Int\endlink, etc)
   .
 - Note: Single character (char) and Newline (NL) values are not formatted as fields (i.e. not padded) since they're usually delimiters
 - See: \ref StreamFormatting "Stream Formatting"

\par Examples

Example using File:
\code
#include <evo/io.h>
using namespace evo;

int main() {
    File file("file.txt", oWRITE_NEW);

    // Use a temporary formatter to write line: 7B,1C8
    File::Format(file) << fHEX << 123 << ',' << 456 << NL;

    // Create a formatter and use to write line: 001,002
    File::Format out(file);
    out << FmtSetInt(fDEC, 3) << 1 << ',' << 2 << NL;

    return 0;
}
\endcode

Example using Console (stdout):
\code
#include <evo/io.h>
using namespace evo;

int main() {
    Console& c = con();

    // Use a temporary formatter to write line: 7B,1C8
    Console::Format(c.out) << fHEX << 123 << ',' << 456 << NL;

    // Create a formatter and use to write line: 001,002
    Console::Format out(c.out);
    out << FmtSetInt(fDEC, 3) << 1 << ',' << 2 << NL;

    return 0;
}
\endcode

For more examples see: \ref StreamFormatting "Stream Formatting"
*/
template<class T>
struct StreamFormatter {
    typedef typename T::Size   Size;    ///< Data size type (ulong)
    typedef T                  Out;     ///< Associated output stream type, type returned by write_out()
    typedef StreamFormatter<T> This;    ///< This type

    Out&       out;                     ///< Associated output stream
    FmtAttribs fmt;                     ///< Formatting attributes (state)

    /** Constructor.
     \param  out  Output stream to associate and format to
    */
    StreamFormatter(Out& out) : out(out) {
    }

    /** Copy constructor.
     - This will reference the same stream as src
     .
     \param  src  Source to copy
    */
    StreamFormatter(const This& src) : out(src.out), fmt(src.fmt) {
    }

    /** Assignment operator copies attributes.
     - This does not copy the referenced string
     .
     \param  src  Source to copy attributes from
     \return      This
    */
    This& operator=(const This& src) {
        memcpy(&fmt, &src.fmt, sizeof(FmtAttribs));
        return *this;
    }

    /** Get parent output string.
     \return  Parent output string (this)
    */
    Out& write_out()
        { return *this; }

    // Field
        
    /** %Set field alignment type to use.
     \param  align  Alignment type
     \return        This
    */
    This& operator<<(FmtAlign align)
        { fmt.field.align = align; return *this; }

    /** %Set field width to use.
     \param  width  Field width to use
     \return        This
    */
    This& operator<<(FmtWidth width)
        { fmt.field.width = width; return *this; }

    /** %Set field attributes to use.
     \param  field  Field attributes
     \return        This
    */
    This& operator<<(const FmtSetField& field)
        { fmt.field.merge(field); return *this; }

    // Newlines

    /** Write an explicit newline and flush stream.
     - Use \link evo::NL NL\endlink for default newline value, where default is set by the stream
     .
     \param  nl  Newline value to write, \link evo::NL_SYS NL_SYS\endlink for system default
     \return     This
    */
    This& operator<<(Newline nl)
        { out << nl; return *this; }

    /** Write default newline and flush stream.
     - This overloads operator<<(Newline)
     .
     \param  nl  Default newline value (\link evo::NL NL\endlink), where default is set by the stream
     \return     This
    */
    This& operator<<(NewlineDefault nl)
        { out << nl; return *this; }

    /** Write newline value and flush stream.
     \param  nl  Newline value to use, use current stream default if null
     \return     This
    */
    This& operator<<(const NewlineValue& nl)
        { out << nl; return *this; }

    /** Flush buffer by writing to stream.
     - This calls flush()
     - This is useful when writing a console prompt, flush so the prompt is immediately visible
     .
     \return  This
    */
    This& operator<<(Flush) {
        if (out.error() == ENone)
            out.flush();
        return *this;
    }

    // Null

    /** %Set attributes for null values.
     \param  null  Null attributes
     \return       This
    */
    This& operator<<(const FmtSetNull& null)
        { fmt.null = null; return *this; }

    // Bools

    /** Append a bool value.
     - Bool value is formatted as either "true" or "false" (without quotes)
     - This calls writetext()
     .
     \param  val  Bool value to append
     \return      This
    */
    This& operator<<(bool val) {
        if (val)
            out.writetext("true", 4);
        else
            out.writetext("false", 5);
        return *this;
    }

    // Chars

    /** Append a character.
     \param  ch  Character to append
     \return     This
    */
    This& operator<<(char ch) {
        if (out.error() == ENone)
            out.writechar(ch, 1);
        return *this;
    }

    /** Append a repeated character.
     \param  ch  Character info to append
     \return     This
    */
    This& operator<<(const FmtChar& ch) {
        if (out.error() == ENone)
            out.writefmtchar(ch.ch, ch.count, fmt.field);
        return *this;
    }

    // Strings

    /** Append a formatted string.
     \param  str  %String info to append
     \return      This
    */
    This& operator<<(const FmtString& str) {
        if (out.error() == ENone) {
            FmtSetField fmtfield(fmt.field);
            fmtfield.merge(str.fmt);
            out.writefmtstr(str.str.data_, str.str.size_, fmtfield);
        }
        return *this;
    }

    /** Append a terminated string.
     - Field attributes apply
     .
     \param  val  %String pointer, must be terminated, NULL for null string
     \return      This
    */
    This& operator<<(const char* val) {
        if (out.error() == ENone) {
            if (val == NULL) {
                if (fmt.null.size > 0)
                    out.writefmtstr(fmt.null.str, fmt.null.size, fmt.field);
            } else if (*val != '\0')
                out.writefmtstr(val, (ulong)strlen(val), fmt.field);
        }
        return *this;
    }

    /** Append a string.
     - Field attributes apply
     .
     \param  str  %String value
     \return      This
    */
    template<class TSize>
    This& operator<<(const ListBase<char,TSize>& str) {
        if (out.error() == ENone) {
            if (str.data_ == NULL) {
                if (fmt.null.size > 0)
                    out.writefmtstr(fmt.null.str, fmt.null.size, fmt.field);
            } else if (str.size_ != '\0')
                out.writefmtstr(str.data_, str.size_, fmt.field);
        }
        return *this;
    }

    // Integers

    /** %Set base for formatting integers.
     \param  base  Base to use, see FmtBase
     \return       This
    */
    This& operator<<(FmtBase base)
        { fmt.num_int.base = base; return *this; }

    /** %Set prefix for formatting integers.
     \param  prefix  Integer prefix to use, see FmtBasePrefix
     \return         This
    */
    This& operator<<(FmtBasePrefix prefix)
        { fmt.num_int.prefix = prefix; return *this; }

    /** %Set integer formatting attributes.
     \param  fmt_int  Integer formatting attributes
     \return          This
    */
    This& operator<<(const FmtSetInt& fmt_int)
        { fmt.num_int.merge(fmt_int); return *this; }

    /** Append a formatted signed integer.
     - Integer and field attributes apply
     .
     \param  num  Integer to format
     \return      This
    */
    This& operator<<(short num)
        { out.writefmtnum(num, fmt.num_int, &fmt.field); return *this; }

    /** \copydoc operator<<(short) */
    This& operator<<(int num)
        { out.writefmtnum(num, fmt.num_int, &fmt.field); return *this; }

    /** \copydoc operator<<(short) */
    This& operator<<(long num)
        { out.writefmtnum(num, fmt.num_int, &fmt.field); return *this; }

    /** \copydoc operator<<(short) */
    This& operator<<(longl num)
        { out.writefmtnum(num, fmt.num_int, &fmt.field); return *this; }

    /** Append a formatted unsigned integer.
     - Integer and field attributes apply
     .
     \param  num  Integer to format
     \return      This
    */
    This& operator<<(ushort num)
        { out.writefmtnumu(num, fmt.num_int, &fmt.field); return *this; }

    /** \copydoc operator<<(ushort) */
    This& operator<<(uint num)
        { out.writefmtnumu(num, fmt.num_int, &fmt.field); return *this; }

    /** \copydoc operator<<(ushort) */
    This& operator<<(ulong num)
        { out.writefmtnumu(num, fmt.num_int, &fmt.field); return *this; }

    /** \copydoc operator<<(ushort) */
    This& operator<<(ulongl num)
        { out.writefmtnumu(num, fmt.num_int, &fmt.field); return *this; }

    /** Append a formatted integer class.
     - Integer, field, and null attributes apply
     .
     \tparam  T  Integer POD type, deduced from arguments

     \param  num  Integer to format (Int, UInt, etc)
     \return      This
    */
    template<class U>
    This& operator<<(const IntegerT<U>& num) {
        if (num.null()) {
            if (fmt.null.size > 0)
                out.writefmtstr(fmt.null.str, fmt.null.size, fmt.field);
        } else if (IntegerT<U>::SIGN)
            out.writefmtnum(num.value(), fmt.num_int, &fmt.field);
        else
            out.writefmtnumu(num.value(), fmt.num_int, &fmt.field);
        return *this;
    }

    // Floats

    /** %Set floating point formatting precision.
     \param  prec  Precision value, see FmtPrecision
     \return       This
    */
    This& operator<<(FmtPrecision prec)
        { fmt.num_flt.precision = prec; return *this; }

    /** %Set floating point formatting attributes.
     \param  fmt_flt  Floating point formatting attributes
     \return          This
    */
    This& operator<<(const FmtSetFloat& fmt_flt)
        { fmt.num_flt.merge(fmt_flt); return *this; }

    /** Append a formatting floating point number.
     - Floating point and field attributes apply
     .
     \param  num  Number to format
     \return      This
    */
    This& operator<<(float num)
        { out.writefmtnumf(num, fmt.num_flt, &fmt.field); return *this; }

    /** \copydoc operator<<(float) */
    This& operator<<(double num)
        { out.writefmtnumf(num, fmt.num_flt, &fmt.field); return *this; }

    /** \copydoc operator<<(float) */
    This& operator<<(ldouble num)
        { out.writefmtnumf(num, fmt.num_flt, &fmt.field); return *this; }

    /** Append a formatted integer class.
     - Floating point, field, and null attributes apply
     .
     \tparam  T  Floating point POD type, deduced from arguments
     \param  num  Number to format (Float, FloatD, etc)
     \return      This
    */
    template<class U>
    This& operator<<(const FloatT<U>& num) {
        if (num.null()) {
            if (fmt.null.size > 0)
                out.writefmtstr(fmt.null.str, fmt.null.size, fmt.field);
        } else
            out.writefmtnumf(num.value(), fmt.num_flt, &fmt.field);
        return *this;
    }

    // Pointer

    /** Writer formatted pointer field to stream.
     \param  fmtptr  Pointer info
     \return         This
    */
    This& operator<<(const FmtPtr& fmtptr)
        { out.writefmtnumu((ulong)fmtptr.ptr, fmtptr.fmt, &fmt.field); return *this; }

    // Dump

    /** Write formatted data dump to stream.
     \param  fmtdump  Dump info
     \return          This
    */
    This& operator<<(const FmtDump& fmtdump)
        { out.writefmtdump(fmtdump); return *this; }
};

///////////////////////////////////////////////////////////////////////////////

/** Input/Output stream implementation.
 - This is not normally instantiated directly, see: File, Pipe, Console, Socket
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
\tparam  T  Low-level I/O stream implementing IoDevice to use
*/
template<class T>
class Stream : public StreamBase {
public:
    typedef typename T::Handle Handle;                  ///< Low-level handle type (OS dependent)
    typedef Stream<T>          This;                    ///< This stream type
    typedef This               Out;                     ///< Type returned by write_out()

    typedef StreamFormatter<This> Format;               ///< \copydoc evo::StreamFormatter

    typedef typename T::ExceptionInT  ExceptionInT;     ///< %Stream input exception type
    typedef typename T::ExceptionOutT ExceptionOutT;    ///< %Stream output exception type

    /** Constructor.
     \param  newlines  Newline type to use for text reading/writing
    */
    Stream(Newline newlines=NL_SYS) :
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
    ~Stream() {
        close();
    }

    /** Get stream handle for low-level calls.
     \return  %Stream handle
    */
    Handle handle() const {
        return device_.handle;
    }

    /** Advanced: Access primary read buffer.
     - \b Caution: This returns a low-level interface that must be used properly
     - Note: Other buffers may be involved if filters are used
     - This never throws any exception
     .
     \return  Read buffer
    */
    RawBuffer& bufread() {
        return bufrd_.readbuf;
    }

    /** Advanced: Access primary write buffer.
     - \b Caution: This returns a low-level interface that must be used properly
     - Note: Other buffers may be involved if filters are used
     - This never throws any exception
     .
     \return  Write buffer
    */
    IoWriter& bufwrite() {
        return bufwr_;
    }

    /** Attach existing stream.
     \param  mode        Access mode to use (oREAD, oREAD_WRITE, or oWRITE), must be correct for handle
     \param  handle      Handle to attach, must be valid
     \param  owned       Whether to take ownership and close handle, false detaches on close()
     \param  flushlines  Whether to flush text output on newlines (line buffering)
    */
    void attach(Open mode, Handle handle, bool owned=true, bool flushlines=false) {
        close();
        device_.handle = handle;
        if (device_.isopen()) {
            init(mode, flushlines);
            owned_ = owned;
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
            bufrd_.close();
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
            if (bufwr_.used > 0) {
                error_ = bufwr_.flush(device_);
            } else
                error_ = ENone;
            bufrd_.close();
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

    bool isopen() const {
        return device_.isopen();
    }

    ulong readbin(void* buf, ulong size) {
        if (T::STREAM_SEEKABLE && rwlast_ != rwlREAD && !readprep())
            return 0;
        size = bufrd_.readbin(error_, device_, buf, size);
        EVO_THROW_ERR_CHECK(ExceptionInT, "Stream binary read failed", error_, (excep_ && size == 0 && error_ != ENone && error_ != EEnd));
        return size;
    }

    ulong readtext(char* buf, ulong size) {
        if (T::STREAM_SEEKABLE && rwlast_ != rwlREAD && !readprep())
            return 0;
        size = bufrd_.readtext(error_, device_, buf, size);
        EVO_THROW_ERR_CHECK(ExceptionInT, "Stream text read failed", error_, (excep_ && size == 0 && error_ != ENone && error_ != EEnd));
        return size;
    }

    bool readline(String& str, ulong maxlen=0) {
        if (T::STREAM_SEEKABLE && rwlast_ != rwlREAD && !readprep())
            return false;
        error_ = bufrd_.readline(str, device_, maxlen);
        if (error_ != ENone && error_ != EEnd) {
            EVO_THROW_ERR_CHECK(ExceptionInT, "Stream text line read failed", error_, excep_);
            return false;
        }
        return true;
    }

    bool flush() {
        error_ = bufwr_.flush(device_);
        if (error_ != ENone) {
            EVO_THROW_ERR_CHECK(ExceptionOutT, "Stream flush failed", error_, excep_);
            return false;
        }
        return true;
    }

    ulong writebin(const void* buf, ulong size) {
        if (T::STREAM_SEEKABLE && rwlast_ != rwlWRITE && !writeprep())
            return 0;
        size = bufwr_.writebin(error_, device_, buf, size);
        EVO_THROW_ERR_CHECK(ExceptionOutT, "Stream binary write failed", error_, (excep_ && size == 0 && error_ != ENone));
        return size;
    }

    ulong writechar(char ch, ulong count=1) {
        if (T::STREAM_SEEKABLE && rwlast_ != rwlWRITE && !writeprep())
            return 0;
        count = bufwr_.writetext_char(error_, device_, ch, count);
        EVO_THROW_ERR_CHECK(ExceptionOutT, "Stream text write failed", error_, (excep_ && count == 0 && error_ != ENone));
        return count;
    }

    ulong writetext(const char* buf, ulong size) {
        if (T::STREAM_SEEKABLE && rwlast_ != rwlWRITE && !writeprep())
            return 0;
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
        if (T::STREAM_SEEKABLE && rwlast_ != rwlWRITE && !writeprep())
            return 0;
        ulong writtensize = bufwr_.writetext(error_, device_, buf, size), writtensize2;
        bufwr_.partnl = 0;
        if ( writtensize == 0 || (writtensize2=bufwr_.writebin(error_, device_, bufwr_.newline, bufwr_.newlinesize)) == 0 ) {
            EVO_THROW_ERR_CHECK(ExceptionOutT, "Stream text line write failed", error_, excep_);
            return 0;
        }
        return writtensize + writtensize2;
    }

    Out& write_out()
        { return *this; }

    char* write_direct(Size size) {
        if (T::STREAM_SEEKABLE && rwlast_ != rwlWRITE && !writeprep())
            return 0;
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
        if (T::STREAM_SEEKABLE && rwlast_ != rwlWRITE && !writeprep())
            return 0;
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

    /** Write formatted signed number.
     - Depending on the stream type, this may be a blocking call
     - If exceptions are disabled, call error() to check error code
     .
     \tparam  TNum  Number type, inferred by param
     \param  num   Number to write
     \param  base  Base to use for formatting
     \return       Whether successful, false on error
    */
    template<class TNum>
    bool writenum(TNum num, int base=fDEC) {
        if (error_ == ENone) {
            if (T::STREAM_SEEKABLE && rwlast_ != rwlWRITE && !writeprep())
                return false;
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

    /** Write formatted unsigned number.
     - Depending on the stream type, this may be a blocking call
     - If exceptions are disabled, call error() to check error code
     .
     \tparam  TNum  Number type, inferred by param
     \param  num   Number to write
     \param  base  Base to use for formatting
     \return       Whether successful, false on error
    */
    template<class TNum>
    bool writenumu(TNum num, int base=fDEC) {
        if (error_ == ENone) {
            if (T::STREAM_SEEKABLE && rwlast_ != rwlWRITE && !writeprep())
                return false;
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

    /** Write formatted floating-point number.
     - Depending on the stream type, this may be a blocking call
     - If exceptions are disabled, call error() to check error code
     .
     \tparam  TNum  Number type, inferred by param
     \param  num        Number to write
     \param  precision  Formatting precision (number of fractional digits), 0 for none, fPREC_AUTO for automatic
     \return            Whether successful, false on error
    */
    template<class TNum>
    bool writenumf(TNum num, int precision=fPREC_AUTO) {
        if (error_ == ENone) {
            if (T::STREAM_SEEKABLE && rwlast_ != rwlWRITE && !writeprep())
                return false;
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

    /** Write formatted and/or repeated character.
     \param  ch     Character to write
     \param  count  Character repeat count to use
     \param  field  Field attributes to use
     \return        Whether successful, false on error
    */
    bool writefmtchar(char ch, ulong count, const FmtSetField& field) {
        if (error_ == ENone) {
            if (T::STREAM_SEEKABLE && rwlast_ != rwlWRITE && !writeprep())
                return false;
            error_ = bufwr_.writefmtchar(device_, ch, count, field);
            EVO_THROW_ERR_CHECK(ExceptionOutT, "Stream text formatted write char failed", error_, (excep_ && error_ != ENone));
            return true;
        }
        EVO_THROW_ERR_CHECK(ExceptionOutT, "Stream text write formatted char blocked by previous error", error_, excep_);
        return false;
    }

    /** Write text with field alignment.
     - This does a text write, converting newlines as needed
     - Depending on the stream type, this may be a blocking call
     - If exceptions are disabled, call error() to check error code
     .
     \param  buf    Buffer to write from
     \param  size   Size to write in bytes
     \param  field  Field attributes to use
     \return        Whether successful, false on error
    */
    bool writefmtstr(const char* buf, ulong size, const FmtSetField& field) {
        if (error_ == ENone) {
            if (T::STREAM_SEEKABLE && rwlast_ != rwlWRITE && !writeprep())
                return false;
            error_ = bufwr_.writefmtstr(device_, buf, size, field);
            EVO_THROW_ERR_CHECK(ExceptionOutT, "Stream text formatted write string failed", error_, (excep_ && error_ != ENone));
            return true;
        }
        EVO_THROW_ERR_CHECK(ExceptionOutT, "Stream text write formatted string blocked by previous error", error_, excep_);
        return false;
    }

    /** Write formatted signed number with field alignment.
     - Depending on the stream type, this may be a blocking call
     - If exceptions are disabled, call error() to check error code
     .
     \tparam  TNum  Number type, inferred by param
     \param  num    Number to write
     \param  fmt    Integer formatting attributes to use
     \param  field  Field formatting attributes to use, NULL for none
     \return        Whether successful, false on error
    */
    template<class TNum>
    bool writefmtnum(TNum num, const FmtSetInt& fmt, const FmtSetField* field=NULL) {
        if (error_ == ENone) {
            if (T::STREAM_SEEKABLE && rwlast_ != rwlWRITE && !writeprep())
                return false;
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

    /** Write formatted unsigned number with field alignment.
     - Depending on the stream type, this may be a blocking call
     - If exceptions are disabled, call error() to check error code
     .
     \tparam  TNum  Number type, inferred by param
     \param  num    Number to write
     \param  fmt    Integer formatting attributes to use
     \param  field  Field formatting attributes to use, NULL for none
     \return        Whether successful, false on error
    */
    template<class TNum>
    bool writefmtnumu(TNum num, const FmtSetInt& fmt, const FmtSetField* field=NULL) {
        if (error_ == ENone) {
            if (T::STREAM_SEEKABLE && rwlast_ != rwlWRITE && !writeprep())
                return false;
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

    /** Write formatted floating point number with field alignment.
     - Depending on the stream type, this may be a blocking call
     - If exceptions are disabled, call error() to check error code
     .
     \tparam  TNum  Number type, inferred by param
     \param  num    Number to write
     \param  fmt    Floating point formatting attributes to use
     \param  field  Field formatting attributes to use, NULL for none
     \return        Whether successful, false on error
    */
    template<class TNum>
    bool writefmtnumf(TNum num, const FmtSetFloat& fmt, const FmtSetField* field=NULL) {
        if (error_ == ENone) {
            if (T::STREAM_SEEKABLE && rwlast_ != rwlWRITE && !writeprep())
                return false;
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

    /** Write formatted data dump.
     - Output may span multiple lines, and always ends with a newline (unless dump data is empty)
     - Depending on the stream type, this may be a blocking call
     - If exceptions are disabled, call error() to check error code
     .
     \param  fmt  Format data, including buffer to dump
     \return      Whether successful, false on error
    */
    bool writefmtdump(const FmtDump& fmt) {
        if (error_ == ENone) {
            if (T::STREAM_SEEKABLE && rwlast_ != rwlWRITE && !writeprep())
                return false;
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

    /** Write formatted data dump with explicit newline type.
     - Output may span multiple lines, and always ends with a newline (unless dump data is empty)
     - Depending on the stream type, this may be a blocking call
     - If exceptions are disabled, call error() to check error code
     .
     \param  fmt  Format data, including buffer to dump
     \param  nl   Newline type to use, \link evo::NL NL\endlink for current default or \link evo::NL_SYS NL_SYS\endlink for system default
     \return      Whether successful, false on error
    */
    bool writefmtdump(const FmtDump& fmt, Newline nl) {
        if (error_ == ENone) {
            if (T::STREAM_SEEKABLE && rwlast_ != rwlWRITE && !writeprep())
                return false;
            error_ = bufwr_.writefmtdump(device_, fmt, getnewline(nl), getnewlinesize(nl));
            if (error_ != ENone) {
                EVO_THROW_ERR_CHECK(ExceptionOutT, "Stream text write hex dump failed", error_, excep_);
                return false;
            }
            return true;
        }
        EVO_THROW_ERR_CHECK(ExceptionOutT, "Stream text write hex dump blocked by previous error", error_, excep_);
        return false;
    }

    /** No-op used by formatting helpers.
     \return  This
    */
    This& operator<<(This&)
        { return *this; }

    /** Write an explicit newline and flush stream.
     - Use \link evo::NL NL\endlink for default newline value, where default is set by the stream
     .
     \param  nl  Newline type to write, \link evo::NL NL\endlink for current default or \link evo::NL_SYS NL_SYS\endlink for system default
     \return     This
    */
    This& operator<<(Newline nl) {
        if (error_ == ENone) {
            writebin(getnewline(nl), getnewlinesize(nl));
            flush();
        }
        return *this;
    }

    /** Write default newline and flush stream.
     - This overloads operator<<(Newline)
     .
     \param  nl  Default newline value (\link evo::NL NL\endlink), where default is set by the stream
     \return     This
    */
    This& operator<<(NewlineDefault nl) {
        EVO_PARAM_UNUSED(nl);
        if (error_ == ENone) {
            writebin(bufwr_.newline, bufwr_.newlinesize);
            flush();
        }
        return *this;
    }

    /** Write newline value and flush stream.
     \param  nl  Newline value to use, use current stream default if null
     \return     This
    */
    This& operator<<(const NewlineValue& nl) {
        if (error_ == ENone) {
            uint nl_size;
            const char* nl_str = nl.getnewline(nl_size, bufwr_.newline, bufwr_.newlinesize);
            writebin(nl_str, nl_size);
            flush();
        }
        return *this;
    }

    /** Flush buffer by writing to stream.
     - This calls flush()
     - This is useful when writing a console prompt, flush so the prompt is immediately visible
     .
     \return  This
    */
    This& operator<<(Flush) {
        if (error_ == ENone)
            flush();
        return *this;
    }

    /** Append a bool value to stream.
     - Bool value is formatted as either "true" or "false" (without quotes)
     - This calls writetext()
     .
     \param  val  Bool value to append
     \return      This
    */
    This& operator<<(bool val) {
        if (error_ == ENone) {
            if (val)
                bufwr_.writetext(error_, device_, "true", 4);
            else
                bufwr_.writetext(error_, device_, "false", 5);
        }
        return *this;
    }

    /** Write character to stream.
     - This calls writetext()
     .
     \param  ch  Character to write
     \return     This
    */
    This& operator<<(char ch) {
        if (error_ == ENone)
            writechar(ch);
        return *this;
    }

    /** Write terminated string to stream.
     - This calls writetext()
     .
     \param  str  String to write, must be terminated
     \return      This
    */
    This& operator<<(const char* str) {
        if (error_ == ENone && str != NULL)
            writetext(str, (ulong)strlen(str));
        return *this;
    }

    /** Write substring to stream.
     - This calls writetext()
     .
     \param  str  Substring to write
     \return      This
    */
    This& operator<<(const SubString& str) {
        if (error_ == ENone && str.size_ > 0)
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
        if (error_ == ENone && str.size_ > 0)
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
        { writenum(num); return *this; }

    /** Write formatted number to stream.
     - This will flush current buffer if it's too full
     - This formats directly to write buffer so it must be large enough
     .
     \param  num  Number to format and write
     \return      This
    */
    This& operator<<(long num)
        { writenum(num); return *this; }

    /** Write formatted number to stream.
     - This will flush current buffer if it's too full
     - This formats directly to write buffer so it must be large enough
     .
     \param  num  Number to format and write
     \return      This
    */
    This& operator<<(longl num)
        { writenum(num); return *this; }

    /** Write formatted number to stream.
     - This will flush current buffer if it's too full
     - This formats directly to write buffer so it must be large enough
     .
     \param  num  Number to format and write
     \return      This
    */
    This& operator<<(uint num)
        { writenumu(num); return *this; }

    /** Write formatted number to stream.
     - This will flush current buffer if it's too full
     - This formats directly to write buffer so it must be large enough
     .
     \param  num  Number to format and write
     \return      This
    */
    This& operator<<(ulong num)
        { writenumu(num); return *this; }

    /** Write formatted number to stream.
     - This will flush current buffer if it's too full
     - This formats directly to write buffer so it must be large enough
     .
     \param  num  Number to format and write
     \return      This
    */
    This& operator<<(ulongl num)
        { writenumu(num); return *this; }

    /** Write formatted class number to stream.
     - This will flush current buffer if it's too full
     - This formats directly to write buffer so it must be large enough
     .
     \tparam  U  Number class type (Int, Uint, etc)

     \param  num  Number to format and write, no-op if null
     \return      This
    */
    template<class U>
    This& operator<<(const IntegerT<U>& num) {
        if (!num.null())
            writenum(num.value());
        return *this;
    }

    /** Write formatted floating-point number to stream.
     - This will flush current buffer if it's too full
     - This formats directly to write buffer so it must be large enough
     .
     \param  num  Number to format and write
     \return      This
    */
    This& operator<<(float num)
        { writenumf(num); return *this; }

    /** Write formatted floating-point number to stream.
     - This will flush current buffer if it's too full
     - This formats directly to write buffer so it must be large enough
     .
     \param  num  Number to format and write
     \return      This
    */
    This& operator<<(double num)
        { writenumf(num); return *this; }

    /** Write formatted floating-point number to stream.
     - This will flush current buffer if it's too full
     - This formats directly to write buffer so it must be large enough
     .
     \param  num  Number to format and write
     \return      This
    */
    This& operator<<(ldouble num)
        { writenumf(num); return *this; }

    /** Write formatted class floating-point number to stream.
     - This will flush current buffer if it's too full
     - This formats directly to write buffer so it must be large enough
     .
     \tparam  U  Number class type (Float, FloatD, etc)

     \param  num  Number to format and write, no-op if null
     \return      This
    */
    template<class U>
    This& operator<<(const FloatT<U>& num) {
        if (!num.null())
            writenumf(num.value());
        return *this;
    }

    /** Write repeated character to stream.
     - This will flush current buffer if it's too full
     - This formats directly to write buffer so it must be large enough
     .
     \param  fmt  Character info
     \return      This
    */
    This& operator<<(const FmtChar& fmt)
        { writechar(fmt.ch, fmt.count); return *this; }

    /** Write formatted string field to stream.
     \param  fmt  %String info
     \return      This
    */
    This& operator<<(const FmtString& fmt)
        { writefmtstr(fmt.str.data_, fmt.str.size_, fmt.fmt); return *this; }

    /** Write formatted number field to stream.
     - This will flush current buffer if it's too full
     - This formats directly to write buffer so it must be large enough
     .
     \param  fmt  Number info
     \return      This
    */
    This& operator<<(const FmtShort& fmt)
        { writefmtnum(fmt.num, fmt.fmt); return *this; }

    /** \copydoc operator<<(const FmtShort&) */
    This& operator<<(const FmtInt& fmt)
        { writefmtnum(fmt.num, fmt.fmt); return *this; }

    /** \copydoc operator<<(const FmtShort&) */
    This& operator<<(const FmtLong& fmt)
        { writefmtnum(fmt.num, fmt.fmt); return *this; }

    /** \copydoc operator<<(const FmtShort&) */
    This& operator<<(const FmtLongL& fmt)
        { writefmtnum(fmt.num, fmt.fmt); return *this; }

    /** \copydoc operator<<(const FmtShort&) */
    This& operator<<(const FmtUShort& fmt)
        { writefmtnumu(fmt.num, fmt.fmt); return *this; }

    /** \copydoc operator<<(const FmtShort&) */
    This& operator<<(const FmtUInt& fmt)
        { writefmtnumu(fmt.num, fmt.fmt); return *this; }

    /** \copydoc operator<<(const FmtShort&) */
    This& operator<<(const FmtULong& fmt)
        { writefmtnumu(fmt.num, fmt.fmt); return *this; }

    /** \copydoc operator<<(const FmtShort&) */
    This& operator<<(const FmtULongL& fmt)
        { writefmtnumu(fmt.num, fmt.fmt); return *this; }

    /** \copydoc operator<<(const FmtShort&) */
    This& operator<<(const FmtFloat& fmt)
        { writefmtnumf(fmt.num, fmt.fmt); return *this; }

    /** \copydoc operator<<(const FmtShort&) */
    This& operator<<(const FmtFloatD& fmt)
        { writefmtnumf(fmt.num, fmt.fmt); return *this; }

    /** \copydoc operator<<(const FmtShort&) */
    This& operator<<(const FmtFloatL& fmt)
        { writefmtnumf(fmt.num, fmt.fmt); return *this; }

    /** \copydoc operator<<(const FmtShort&) */
    template<class U>
    This& operator<<(const FmtFieldNum<U>& fmt) {
        if (IntegerT<U>::SIGN)
            writefmtnum(fmt.num.num, fmt.num.fmt, &fmt.field);
        else
            writefmtnumu(fmt.num.num, fmt.num.fmt, &fmt.field);
        return *this;
    }

    /** \copydoc operator<<(const FmtShort&) */
    template<class U>
    This& operator<<(const FmtFieldFloat<U>& fmt)
        { writefmtnumf(fmt.num.num, fmt.num.fmt, &fmt.field); return *this; }

    /** Writer formatted pointer field to stream.
     \param  fmtptr  Pointer info
     \return         This
    */
    This& operator<<(const FmtPtr& fmtptr)
        { writefmtnumu((ulong)fmtptr.ptr, fmtptr.fmt); return *this; }

    /** Write formatted data dump to stream.
     \param  fmt  Dump info
     \return      This
    */
    This& operator<<(const FmtDump& fmt)
        { writefmtdump(fmt); return *this; }

protected:
    /** Current read/write mode. */
    enum RwLast {
        rwlNONE = 0,
        rwlREAD,
        rwlWRITE
    };

    T        device_;       ///< I/O device
    bool     owned_;        ///< Whether handle is owned (to be closed here)
    IoReader bufrd_;        ///< Buffered reader
    IoWriter bufwr_;        ///< Buffered writer
    ulongl   savepos_;      ///< Read/Write: Used to save buffered read position when switching between read/write
    RwLast   rwlast_;       ///< Read/Write: Used to track last operation when switching between read/write

    /** Initialize and reset buffers for a new stream. */
    void init(Open mode, bool flushlines=false) {
        savepos_ = 0;
        rwlast_  = rwlNONE;
        if (open_readable(mode))
            bufrd_.open();
        if (open_writable(mode))
            bufwr_.open(flushlines);
    }

private:
    Stream(const This&);
    This& operator=(const This&);

    // Restore buffered read position (switching from write to read)
    bool readprep() {
        if (rwlast_ != rwlNONE) {
            if ((error_=bufwr_.flush(device_)) != ENone) {
                EVO_THROW_ERR_CHECK(ExceptionOutT, "Stream flush failed before switch to read mode", error_, excep_);
                return false;
            }
            ulongl newpos = device_.pos(error_);
            if (error_ != ENone) {
                EVO_THROW_ERR_CHECK(ExceptionInT, "Stream position read failed during switch to read mode", error_, excep_);
                return false;
            }
            if (newpos > savepos_ || bufrd_.readbuf.used > savepos_ || newpos < savepos_-bufrd_.readbuf.used) {
                bufrd_.readbuf.used = bufrd_.curbuf_offset = 0; // seeking outside buffered data
                device_.seek(error_, newpos, sBegin);
            } else {
                bufrd_.curbuf_offset = bufrd_.readbuf.used - (ulong)(savepos_ - newpos);
                device_.seek(error_, savepos_, sBegin);
            }
            if (error_ != ENone) {
                EVO_THROW_ERR_CHECK(ExceptionInT, "Stream seek failed during switch to read mode", error_, excep_);
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
                EVO_THROW_ERR_CHECK(ExceptionOutT, "Stream position read failed during switch to write mode", error_, excep_);
                return false;
            }
            device_.seek(error_, savepos_-(bufrd_.readbuf.used-bufrd_.curbuf_offset), sBegin);
            if (error_ != ENone) {
                EVO_THROW_ERR_CHECK(ExceptionOutT, "Stream seek failed during switch to write mode", error_, excep_);
                return false;
            }
        }
        rwlast_ = rwlWRITE;
        return true;
    }
};

///////////////////////////////////////////////////////////////////////////////
//@}
}
#endif
