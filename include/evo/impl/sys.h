// Evo C++ Library
/* Copyright (c) 2016 Justin Crowell
 This Source Code Form is subject to the terms of the Mozilla Public
 License, v. 2.0. If a copy of the MPL was not distributed with this
 file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
///////////////////////////////////////////////////////////////////////////////
/** \file sys.h Evo implementation detail: System portability handling. */
#pragma once
#ifndef INCL_evo_impl_sys_h
#define INCL_evo_impl_sys_h

// Includes - System Specific
#if defined(__linux) || defined(__CYGWIN__)
    #define _FILE_OFFSET_BITS 64

    #include <unistd.h>
    #include <malloc.h>
    #if defined(__linux)
        #include <stdint.h>
    #endif
    #if defined(__CYGWIN__)
        #include <sys/time.h>
    #endif
#elif defined(_WIN32)
    #include <windows.h>
    #include <malloc.h>
#elif defined(__APPLE__) && defined(__MACH__)
    #include <sys/time.h>
#else
    #error "This system is not supported by Evo"
#endif

// Includes - System
#include <stdlib.h>
#include <stdio.h>
#include <limits>
#include <limits.h>
#include <float.h>
#include <math.h>
#include <string.h>
#include <assert.h>
#include <errno.h>

// Evo Config
#include "../evo_config.h"

// STL Compatibility
#if (EVO_STD_STRING || defined(_BASIC_STRING_H) || defined(_STRING_)) && EVO_STD_STRING != -1
    #include <string> // std::string
    #define EVO_STD_STRING_ENABLED 1
#endif

/** \addtogroup EvoCore */
//@{
///////////////////////////////////////////////////////////////////////////////
/** \cond impl */

// C++ Version
#if __cplusplus >= 201103L
    #define EVO_CPP11
#endif

// 64 bit environment
#if defined(__LP64__) || defined(_LP64) || defined(_WIN64)
    #define EVO_64 1
#else
    #define EVO_32 1
#endif

// Primitive aliases
typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned long long int ulongl;
typedef long long int longl;
typedef long double ldouble;
#if defined(__APPLE__)
    typedef unsigned int uint;
    typedef unsigned long ulong;
#endif

#if defined(_WIN32)
    typedef unsigned int uint;
    typedef unsigned long ulong;
    typedef __int8  int8;
    typedef __int16 int16;
    typedef __int32 int32;
    typedef __int64 int64;
    typedef unsigned __int8  uint8;
    typedef unsigned __int16 uint16;
    typedef unsigned __int32 uint32;
    typedef unsigned __int64 uint64;
#else
    typedef int8_t  int8;
    typedef int16_t int16;
    typedef int32_t int32;
    typedef int64_t int64;
    typedef uint8_t  uint8;
    typedef uint16_t uint16;
    typedef uint32_t uint32;
    typedef uint64_t uint64;
#endif

// Some macros under Windows cause problems
#if defined(_WIN32)
    #if defined(min)
        #undef min
    #endif
    #if defined(max)
        #undef max
    #endif
#endif

// Used to ignore certain warnings
#if defined(_MSC_VER)
    #define EVO_PARAM_UNUSED(NAME)       __pragma(warning(suppress:4100)) NAME
    #define EVO_MSVC_NOWARN_START(CODES) __pragma(warning(push)) __pragma(warning(disable:CODES))
    #define EVO_MSVC_NOWARN_END          __pragma(warning(pop))
#else
    #define EVO_PARAM_UNUSED(NAME)
    #define EVO_MSVC_NOWARN_START(CODE)
    #define EVO_MSVC_NOWARN_END
#endif
#if defined(__GNUC__)
    #define EVO_ATTRIB_UNUSED __attribute__((unused))
#else
    #define EVO_ATTRIB_UNUSED
#endif

/** \endcond */
///////////////////////////////////////////////////////////////////////////////

/** Shortcut for NULL as const-char pointer. */
#define EVO_CNULL ((const char*)0)

/** Shortcut for NULL as void-pointer. */
#define EVO_VNULL ((void*)0)

///////////////////////////////////////////////////////////////////////////////

// Floating point functions
/** \cond impl */

// Windows -- has overloads for all floating point types
#if defined _WIN32
    #define evo_pow pow
    #define evo_modf modf
    #define evo_fabs fabs

// Unix/Linux -- has separate functions for float and long-double
#else
    // Determine whether long double math functions are supported
    #if defined(__CYGWIN__)
        // This indicates a possible precision loss on long-double due to missing math functions
        #define EVO_LDBL_NOMATH
    #endif

    // evo_pow()
    inline float evo_pow(float x, float y)
        { return powf(x, y); }
    inline double evo_pow(double x, double y)
        { return pow(x, y); }
    #if defined EVO_LDBL_NOMATH
        // Use pow() -- possible precision loss
        inline long double evo_pow(long double x, long double y)
            { return pow((double)x, (double)y); }
    #else
        inline long double evo_pow(long double x, long double y)
            { return powl(x, y); }
    #endif

    // evo_modf()
    inline float evo_modf(float x, float* iptr)
        { return modff(x, iptr); }
    inline double evo_modf(double x, double* iptr)
        { return modf(x, iptr); }
    #if defined EVO_LDBL_NOMATH
        // Use modf() -- possible precision loss
        inline long double evo_modf(long double x, long double* iptr)
            { return modf((double)x, (double*)iptr); }
    #else
        inline long double evo_modf(long double x, long double* iptr)
            { return modfl(x, iptr); }
    #endif

    // evo_fabs()
    inline float evo_fabs(float x)
        { return fabsf(x); }
    inline double evo_fabs(double x)
        { return fabs(x); }
    #if defined EVO_LDBL_NOMATH
        inline long double evo_fabs(long double x)
            { return (x<0.0 ? -x : x); }
    #else
        inline long double evo_fabs(long double x)
            { return fabsl(x); }
    #endif

#endif
/** \endcond */

///////////////////////////////////////////////////////////////////////////////

/** Make argument a string literal.
 \param  X  Argument to make a string literal
*/
#define EVO_STRINGIFY(X) #X

///////////////////////////////////////////////////////////////////////////////

// Namespace: evo
namespace evo {

///////////////////////////////////////////////////////////////////////////////

/** Default Evo container size type.
 - Always unsigned
 - This may be overridden in any Evo container template
*/
typedef EVO_SIZE_TYPE SizeT;

/** Default Evo string size type.
 - Always unsigned
*/
typedef EVO_STR_SIZE_TYPE StrSizeT;

///////////////////////////////////////////////////////////////////////////////

/** Newline type.
 - This gives the current system newline type: NL
 - NL also is used with Stream operator<<() to write a newline and flush console output
 .
*/
enum Newline {
    // Expected to be first:
    nLF=0,            ///< Line Feed "\n" (Unix/Linux, Mac OS X)
    nCR,            ///< Carriage Return "\r" (Mac OS)
    nCRLF,            ///< Carriage Return + Line Feed "\r\n" (Windows, DOS, Internet)
    // Expected to be last:
    nLFCR            ///< Line Feed + Carriage Return "\n\r" (RISC OS)
};

#if defined(_WIN32)
    // Windows
    /** Current system newline type. */
    static const Newline NL = nCRLF;
#else
    // Linux/Posix
    /** Current system newline type. */
    static const Newline NL = nLF;
#endif

/** Get newline string for given type.
 \param  newline  Newline type (default is newline for current platform)
 \return          Newline string
*/
inline const char* getnewline(Newline newline=NL) {
    const char* NEWLINES[] = { "\n", "\r", "\r\n", "\n\r" };
    return NEWLINES[(int)newline];
}

/** Get newline string size for given type.
 \param  newline  Newline type (default is newline for current platform)
 \return          Newline string size (length)
*/
inline uint getnewlinesize(Newline newline=NL) {
    const uint SIZES[] = { 1, 1, 2, 2 };
    return SIZES[(int)newline];
}

///////////////////////////////////////////////////////////////////////////////

/** Base for all Evo list types (used internally).
 - This provides a common base type so all list types have some compatibility with each other
 - List of char is the base type for Evo string types, see \link evo::StringBase StringBase\endlink
 - See list types: Array, List, SubList, String, SubString
*/
template<class T,class TSize=SizeT>
struct ListBase {
    typedef ListBase<T,TSize> ListBaseType;       ///< List base type (this)

    T*    data_;            ///< Data pointer, NULL if null
    TSize size_;            ///< Data size as item count, 0 if empty or null

    ListBase() {
        data_ = NULL;
        size_ = 0;
    }

    ListBase(const char* str) {
        if (str == NULL) {
            data_ = NULL;
            size_ = 0;
        } else {
            data_ = (char*)str;
            size_ = (TSize)strlen(str);
        }
    }

    ListBase(const char* str, TSize size) {
        if (size > 0) {
            data_ = (char*)str;
            size_ = size;
        } else {
            data_ = (char*)(str == NULL ? NULL : "");
            size_ = 0;
        }
    }
};

// If enabled, implicitly convert "const std::string&" and "const std::string*" to StringBase
/** \cond impl */
#if EVO_STD_STRING_ENABLED
template<class TSize>
struct ListBase<char,TSize> {
    typedef ListBase<char,TSize> ListBaseType;

    char* data_;
    TSize size_;

    ListBase() {
        data_ = NULL;
        size_ = 0;
    }

    ListBase(const char* str) {
        if (str == NULL) {
            data_ = NULL;
            size_ = 0;
        } else {
            data_ = (char*)str;
            size_ = (TSize)strlen(str);
        }
    }

    ListBase(const char* str, TSize size) {
        if (size > 0) {
            data_ = (char*)str;
            size_ = size;
        } else {
            data_ = (char*)(str == NULL ? NULL : "");
            size_ = 0;
        }
    }

    ListBase(const std::basic_string<char>& str) {
        size_ = (TSize)str.length();
        data_ = (char*)(size_ > 0 ? str.data() : "");
    }

    ListBase(const std::basic_string<char>* str) {
        if (str == NULL) {
            data_ = NULL;
            size_ = 0;
        } else {
            data_ = (char*)str->data();
            size_ = (TSize)str->length();
        }
    }
};
#endif
/** \endcond */

///////////////////////////////////////////////////////////////////////////////

/** Null value type */
enum ValNull {
    vNull=0         ///< Null value
};

/** Empty value type */
enum ValEmpty {
    vEmpty=0        ///< Empty value
};

/** Value type to specify reverse algorithm. */
enum ValAlgReverse {
    vReverse=0      ///< Value to reverse algorithm
};

///////////////////////////////////////////////////////////////////////////////

/** General error code. Used with or without exceptions. */
enum Error {
    // Non-errors first
    ENone = 0,          ///< No error
    EEnd,               ///< Reached end of resource (not an error)
    EMoreInput,         ///< Operation needs more input data
    EMoreOutput,        ///< Operation has more output data
    // Errors
    EUnknown,           ///< Unknown or unspecified error
    EInval,             ///< Invalid operation or data error
    EFail,              ///< Operation failed
    EOutOfBounds,       ///< Out of bounds error
    EClosed,            ///< %Stream or resource is closed
    ETimeout,           ///< Operation timed out
    ESignal,            ///< System call interrupted by signal (Linux/Posix), auto-resume disabled
    EPtr,               ///< Invalid pointer used
    ESpace,             ///< Not enough space/memory available
    EAccess,            ///< Permission denied
    EExist,             ///< Resource already exists
    ENotFound,          ///< Resource not found
    ESize,              ///< Size limit exceeded
    ELoss,              ///< Operation aborted to avoid data loss (used in special cases)
    // I/O Errors
    ERead,              ///< General stream or resource read error
    EWrite,             ///< General stream or resource write error
    EInput              ///< Truncated or incomplete input (error version of EEnd)
    // EInput expected to be last in errormsg()
};

/** Get general error message for error code. */
inline const char* errormsg(Error error) {
    const char* msgs[] = {
        // Non-errors first
        "No error",                         // ENone
        "End of resource",
        "Operation requires more input data",
        "Operation has more output data",
        // Errors
        "Unknown error",                    // EUnknown
        "Invalid operation/data",
        "Operation failed",
        "Data out of bounds",
        "Resource is closed",
        "Operation timed out",              // ETimeout
        "Signal interrupt",
        "Invalid pointer",
        "Not enough space/memory available",
        "Permission denied",
        "Resource already exists",          // EExist
        "Resource not found",
        "Size limit exceeded",
        "Failed to prevent data loss",
        // I/O Errors
        "General read error",               // ERead
        "General write error",
        "Truncated or incomplete input"
    };
    if (error < 0 || error > EWrite)
        return "Bad error code";
    return msgs[(int)error];
}

///////////////////////////////////////////////////////////////////////////////

/** Evo base exception.
 - Use EVO_THROW() or EVO_THROW_E() macro to throw exception
*/
class Exception {
public:
    /** Constructor.
     \param  file   Exception file name
     \param  line   Exception line number
     \param  msg    Exception message
     \param  error  Error code, EUnknown if unknown
    */
    Exception(const char* file, ulong line, const char* msg, Error error=EUnknown) : file_(file), line_(line), msg_(NULL), error_(error)
        { setmsg("Exception", msg, strlen(msg)); }

    /** Copy constructor.
     \param  e  Exception to copy
    */
    Exception(const Exception& e) : file_(e.file_), line_(e.line_), msg_(NULL), error_(e.error_)
        { setmsg(e.msg_, strlen(e.msg_)); }

    /** Destructor */
    ~Exception()
        { if (msg_ != NULL) free(msg_); }

    /** Get exception file name.
     \return  File name
    */
    const char* file() const
        { return file_; }

    /** Get exception line number.
     \return  Line number
    */
    ulong line() const
        { return line_; }

    /** Get exception message.
     \return  Message
    */
    const char* msg() const
        { return (msg_ == NULL) ? "" : msg_; }

    /** Get error code.
     \return  Error code, 0 if unknown
    */
    Error error() const
        { return error_; }

protected:
    /** Constructor with exception type. Use EVO_THROW() or EVO_THROW_E() macro to throw exception.
     \param  file   Exception file name
     \param  line   Exception line number
     \param  type   Exception type as string, used as msg prefix
     \param  msg    Exception message
     \param  error  Error code, EUnknown if unknown
    */
    Exception(const char* file, ulong line, const char* type, const char* msg, Error error) : file_(file), line_(line), msg_(NULL), error_(error)
        { setmsg(type, msg, strlen(msg)); }

private:
    const char* file_;        ///< File name
    ulong       line_;        ///< Line number
    char*       msg_;        ///< Message
    Error       error_;        ///< Error code, EUnknown if unknown

    void setmsg(const char* type, const char* msg, ulong len) {
        ulong type_len  = strlen(type);
        ulong total_len = type_len + 2 + len;
        if (total_len > 0) {
            msg_ = (char*)malloc(total_len+1);
            memcpy(msg_, type, type_len);
            memcpy(msg_+type_len, ": ", 2);
            memcpy(msg_+type_len+2, msg, len);
            msg_[total_len] = '\0';
        }
    }

    void setmsg(const char* msg, ulong len) {
        if (len > 0) {
            msg_ = (char*)malloc(len+1);
            memcpy(msg_, msg, len);
            msg_[len] = '\0';
        }
    }
};

///////////////////////////////////////////////////////////////////////////////

/** Create an Evo exception implementation.
 - See also EVO_CREATE_EXCEPTION()
 - This is used to create an exception so doxygen will document it\n
   Example:\code
class ExceptionFoo : public Exception
    { EVO_CREATE_EXCEPTION_IMPL(ExceptionFoo,Exception) };
   \endcode
 .
 \param  NAME  Name of new exception type
 \param  BASE  Base exception type -- Exception or derived from it
*/
#define EVO_CREATE_EXCEPTION_IMPL(NAME,BASE) \
    public: \
        NAME(const char* file, ulong line, const char* msg, Error error=EUnknown) : BASE(file, line, #NAME, msg, error) { } \
        NAME(const NAME& e) : BASE(e) { } \
    protected: \
        NAME(const char* file, ulong line, const char* type, const char* msg, Error error) : BASE(file, line, type, msg, error) { }

/** Create an Evo exception using base exception.
 - Doxygen will not track this type, see EVO_CREATE_EXCEPTION_IMPL()
 .
 \param  NAME  Name of new exception type
 \param  BASE  Base exception type -- Exception or derived from it
*/
#define EVO_CREATE_EXCEPTION(NAME,BASE) \
    class NAME : public BASE \
        { EVO_CREATE_EXCEPTION_IMPL(NAME,BASE) }

// Basics

// TODO: needed?
/** Invalid operation or data exception. Same interface as Exception. */
struct ExceptionInval : public Exception
    { EVO_CREATE_EXCEPTION_IMPL(ExceptionInval,Exception) };

// TODO: needed?
/** Out of bounds value or index exception. Same interface as Exception. */
struct ExceptionOutOfBounds : public Exception
    { EVO_CREATE_EXCEPTION_IMPL(ExceptionOutOfBounds,Exception) };

// TODO: needed?
/** Allocation exception. */
struct ExceptionAlloc : public Exception
    { EVO_CREATE_EXCEPTION_IMPL(ExceptionAlloc,Exception) };

// IO

/** Base stream exception. Same interface as Exception. */
class ExceptionStream : public Exception
    { EVO_CREATE_EXCEPTION_IMPL(ExceptionStream,Exception) };

/** %Stream open exception. Same interface as Exception. */
class ExceptionStreamOpen : public ExceptionStream
    { EVO_CREATE_EXCEPTION_IMPL(ExceptionStreamOpen,ExceptionStream) };

/** Input stream exception. Same interface as Exception. */
class ExceptionStreamIn : public ExceptionStream
    { EVO_CREATE_EXCEPTION_IMPL(ExceptionStreamIn,ExceptionStream) };

/** Output stream exception. Same interface as Exception. */
class ExceptionStreamOut : public ExceptionStream
    { EVO_CREATE_EXCEPTION_IMPL(ExceptionStreamOut,ExceptionStream) };

///////////////////////////////////////////////////////////////////////////////

/** Throw an Evo exception.
 - See also: EVO_THROW_EX(), EVO_THROW_ERR()
 .
 \param  TYPE  %Exception type (Exception or derived from it).
 \param  MSG   %Exception message string (const char*).
*/
#define EVO_THROW(TYPE,MSG) throw TYPE(__FILE__, __LINE__, MSG)

/** Throw an Evo base exception.
 - See also: EVO_THROW(), EVO_THROW_ERR()
 .
 \param  MSG  %Exception message string (const char*).
*/
#define EVO_THROW_EX(MSG) throw evo::Exception(__FILE__, __LINE__, MSG)

/** Throw an Evo exception with error code.
 - See also: EVO_THROW(), EVO_THROW_EX()
 .
 \param  TYPE   %Exception type (Exception or derived from it).
 \param  MSG    %Exception message string (const char*).
 \param  ERROR  %Error code
*/
#define EVO_THROW_ERR(TYPE,MSG,ERROR) throw TYPE(__FILE__, __LINE__, MSG, ERROR)

/** Throw an Evo exception with error code if COND is true.
 \param  TYPE   %Exception type (Exception or derived from it).
 \param  MSG    %Exception message string (const char*).
 \param  ERROR  %Error code
 \param  COND   Error condition to check
*/
#define EVO_THROW_ERR_CHECK(TYPE,MSG,ERROR,COND) { if (COND) throw TYPE(__FILE__, __LINE__, MSG, ERROR); }

///////////////////////////////////////////////////////////////////////////////

// TODO
/** Output formatter base class.
 - Implement to support custom output via operator<<() on String and StreamOut objects
*/
class OutFmtBase {
    virtual ~OutFmtBase()
        { }

//    virtual ulong write(char* buf, ulong size)
//        { return 0; }
};

///////////////////////////////////////////////////////////////////////////////

#if defined(_WIN32)
    // Windows
    // TODO
#else
    // Linux/Posix
    struct SysLinux {
        static void set_timeval_ms(struct timeval& tm, ulong ms) {
            const ulong MSEC_PER_SEC  = 1000;
            const ulong USEC_PER_MSEC = 1000;
            tm.tv_sec  = ms / MSEC_PER_SEC;
            tm.tv_usec = (ms - (tm.tv_sec * MSEC_PER_SEC)) * USEC_PER_MSEC;
        }
    };
#endif

///////////////////////////////////////////////////////////////////////////////

/** Swap contents of given objects.
 - The object memory is swapped directly, so this works with almost any object type
 .
 \tparam  T  Object type, inferred from parameters
 \param  a  First object to swap
 \param  b  Second object to swap
*/
template<class T>
void swap(T& a, T& b) {
    char tmp[sizeof(T)];
    memcpy(tmp, &a,  sizeof(T));
    memcpy(&a,  &b,  sizeof(T));
    memcpy(&b,  tmp, sizeof(T));
}

///////////////////////////////////////////////////////////////////////////////
} // Namespace: evo
//@}
#endif
