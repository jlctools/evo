// Evo C++ Library
/* Copyright 2019 Justin Crowell
Distributed under the BSD 2-Clause License -- see included file LICENSE.txt for details.
*/
///////////////////////////////////////////////////////////////////////////////
/** \file sys.h Evo implementation detail for system portability -- this is included by most Evo headers, include this via: include <evo/type.h>. */
#pragma once
#ifndef INCL_evo_impl_sys_h
#define INCL_evo_impl_sys_h

// Compiler detection -- most version integers are calculated as (except where noted): (major * 100) + minor
#if defined(__INTEL_COMPILER)
    // Intel C++ -- version 900 for 9.0, 1700 for 17.0, etc
    #define EVO_COMPILER "Intel"
    #define EVO_INTEL_VER __INTEL_COMPILER
    #define EVO_COMPILER_VER EVO_INTEL_VER
#elif defined(_MSC_VER)
    // MS Visual C++ version as Visual Studio version -- 1503 for Visual Studio 15.3 (2017), 1400 for 14.0 (2015), etc
    #define EVO_COMPILER "MSVC"
    #if _MSC_VER >= 1910
        #define EVO_MSVC_YEAR 2017
        #define EVO_MSVC_TOOLSET 141
        #if _MSC_VER >= 1916
            #define EVO_MSVS_VER 1509
        #elif _MSC_VER >= 1912
            #define EVO_MSVS_VER (1505 + (_MSC_VER - 1912))
        #elif _MSC_VER >= 1911
            #define EVO_MSVS_VER 1503
        #elif _MSC_VER >= 1910
            #define EVO_MSVS_VER 1500
        #endif
    #elif _MSC_VER >= 1900
        #define EVO_MSVC_YEAR 2015
        #define EVO_MSVC_TOOLSET 140
        #define EVO_MSVS_VER 1400
    #elif _MSC_VER >= 1800
        #define EVO_MSVC_YEAR 2013
        #define EVO_MSVC_TOOLSET 120
        #define EVO_MSVS_VER 1200
    #elif _MSC_VER >= 1700
        #define EVO_MSVC_YEAR 2012
        #define EVO_MSVC_TOOLSET 110
        #define EVO_MSVS_VER 1100
    #elif _MSC_VER >= 1600
        #define EVO_MSVC_YEAR 2010
        #define EVO_MSVC_TOOLSET 100
        #define EVO_MSVS_VER 1000
    #elif _MSC_VER >= 1500
        #define EVO_MSVC_YEAR 2008
        #define EVO_MSVC_TOOLSET 90
        #define EVO_MSVS_VER 900
    #elif _MSC_VER >= 1400
        #define EVO_MSVC_YEAR 2005
        #define EVO_MSVC_TOOLSET 80
        #define EVO_MSVS_VER 800
    #elif _MSC_VER >= 1310
        #define EVO_MSVC_YEAR 2003
        #define EVO_MSVC_TOOLSET 71
        #define EVO_MSVS_VER 701
    #elif _MSC_VER >= 1300
        #define EVO_MSVC_YEAR 2002
        #define EVO_MSVC_TOOLSET 70
        #define EVO_MSVS_VER 700
    #else
        #error "This MSVC compiler is too old (before 2002) and not supported by Evo"
    #endif
    #define EVO_COMPILER_VER EVO_MSVS_VER
#elif defined(__clang__)
    #if defined(__APPLE__) || defined(__apple_build_version__)
        #define EVO_COMPILER "Apple clang"
        #define EVO_APPLE_CLANG_VER ((__clang_major__ * 100) + __clang_minor__)
        #define EVO_CLANG_VER    EVO_APPLE_CLANG_VER        // Apple forked clang versions after 3.1, common EVO_CLANG_VER is useful on versions before then
        #define EVO_COMPILER_VER EVO_APPLE_CLANG_VER
        #if EVO_CLANG_VER < 500
            #define EVO_OLDCC
        #endif
    #else
        #define EVO_COMPILER "clang"
        #define EVO_LLVM_CLANG_VER ((__clang_major__ * 100) + __clang_minor__)
        #define EVO_CLANG_VER    EVO_LLVM_CLANG_VER
        #define EVO_COMPILER_VER EVO_LLVM_CLANG_VER
        #if EVO_CLANG_VER < 303
            #define EVO_OLDCC
        #endif
    #endif
    #include <ciso646> // used to identify LLVM libc++
    #if defined(_LIBCPP_VERSION)
        #define EVO_CLANG_LIBCPP _LIBCPP_VERSION
    #else
        #if defined(__GLIBCXX__)
            #define EVO_GLIBCPP __GLIBCXX__
        #endif
    #endif
#elif defined(__GNUC__)
    #define EVO_COMPILER "gcc"
    #define EVO_GCC_VER ((__GNUC__ * 100) + __GNUC_MINOR__)
    #define EVO_COMPILER_VER EVO_GCC_VER
    #if EVO_GCC_VER < 409
        #define EVO_OLDCC
        #if EVO_GCC_VER < 407
            #define EVO_OLDCC2
        #endif
    #endif
    #include <limits.h> // used to identify GNU libc++
    #if defined(__GLIBCXX__)
        #define EVO_GLIBCPP __GLIBCXX__
    #endif        
#elif defined(__CODEGEARC__) && __CODEGEARC__ >= 0x0630     // Not supported before C++ Builder XE
    // C++ Builder -- calculated version (100 for 10, 101 for 10.1, etc) for latest recognized version, XE number for older compiler (1 for XE, 2 for XE2, etc)
    #define EVO_COMPILER "C++ Builder"
    #if __CODEGEARC__ >= 0x0710     // 10.0 - 10.2
        #define EVO_CBUILDER_VER (100 + (((__CODEGEARC__ & 0x0FF0) - 0x0710) / 0x0010))
    #else                           // XE 1-8
        #define EVO_CBUILDER_VER (((__CODEGEARC__ & 0x0FF0) - 0x0620) / 0x0010)
    #endif
    #define EVO_COMPILER_VER EVO_CBUILDER_VER
#elif defined(__BORLANDC__)
    #error "This Borland compiler is too old (before C++ Builder XE) and not supported by Evo"
#else
    /** C++ compiler name.
     - See \link EVO_COMPILER_VER\endlink for compiler names and versions
    */
    #define EVO_COMPILER "Unknown"

    /** C++ compiler version integer.
     - Most version integers are calculated as: (major * 100) + minor
     - Some compilers will have additional macros with more info
     - Note: Not all compilers listed here are fully supported
     .
    Compiler versions:
    | Compiler Name | Version | Additional Macros |
    | ------------- | ------- | ----------------- |
    | GCC           | 504 for GCC 5.4 | \link EVO_GCC_VER\endlink |
    | Clang         | 308 for Clang 3.8 | \link EVO_LLVM_CLANG_VER\endlink / \link EVO_CLANG_VER\endlink |
    | Apple Clang   | 901 for Apple Clang 9.1 -- Apple forked clang versions after 3.1 | \link EVO_APPLE_CLANG_VER\endlink / \link EVO_CLANG_VER\endlink |
    | MSVC          | Visual Studio version: 1507 for 2017 15.7, 1400 for 2015 14.0 | \link EVO_MSVC_YEAR\endlink / \link EVO_MSVC_TOOLSET\endlink |
    | Intel         | 1700 for Intel C++ 17.00 | |
    | C++ Builder   | 101 for C++ Builder 10.1, older: 1-8 for C++ Builder XE - XE8 | |
    */
    #define EVO_COMPILER_VER 0

    #if defined(DOXYGEN)
        /** GCC compiler version as integer. */
        #define EVO_GCC_VER 504

        /** Clang compiler version as integer (LLVM or Apple).
         - Note: Apple forked clang versions after clang 3.1
        */
        #define EVO_CLANG_VER 301

        /** LLVM clang compiler version as integer. */
        #define EVO_LLVM_CLANG_VER 308

        /** Apple clang compiler version as integer. */
        #define EVO_APPLE_CLANG_VER 901

        /** MSVC compiler year (4 digits). */
        #define EVO_MSVC_YEAR 2017

        /** MSVC toolset version as integer (3 digits, 2 digits before MSVC 2010). */
        #define EVO_MSVC_TOOLSET 141
    #endif
#endif

// System includes
#if defined(_WIN32)
    #define _WINSOCKAPI_            // exclude winsock from windows.h
    #if !defined(WIN32_LEAN_AND_MEAN)
        #define WIN32_LEAN_AND_MEAN // minimize windows.h include
    #endif
    #if !defined(_CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES)
        #define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES 1
    #endif
    #if !defined(_CRT_NONSTDC_NO_WARNINGS)
        #define _CRT_NONSTDC_NO_WARNINGS 1
    #endif
    #define NOMINMAX                // disable windows min/max macros

    #include <windows.h>
    #include <malloc.h>
#elif defined(__APPLE__) && defined(__MACH__)
    #include <unistd.h>
    #include <sys/time.h>
#elif defined(__linux) || defined(__unix__) || defined(__CYGWIN__)
    #define _FILE_OFFSET_BITS 64
    #if defined(__linux) && defined(__GLIBC__)
        #define __GNU_SOURCE
    #endif

    #include <unistd.h>
    #include <time.h>
    #include <sys/time.h>
    #if defined(__linux)
        #include <malloc.h>
        #include <stdint.h>
    #else
        #include <stdlib.h>
    #endif
#elif !defined(DOXYGEN)
    #error "This system is not supported by Evo"
#endif

/** \cond impl */
#include <stdlib.h>
#include <stdio.h>
#include <limits>
#include <limits.h>
#include <float.h>
#include <math.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
/** \endcond */

// Evo Config
#include "../evo_config.h"

// C++ Version
#if __cplusplus >= 201103L || (defined(EVO_MSVC_YEAR) && EVO_MSVC_YEAR >= 2017) || defined(DOXYGEN)
    /** Defined when compiler C++11 supported is enabled. */
    #define EVO_CPP11
#endif
#if __cplusplus >= 201402L || (defined(_MSVC_LANG) && _MSVC_LANG >= 201402L) || defined(DOXYGEN)
    /** Defined when compiler C++14 supported is enabled. */
    #define EVO_CPP14

	#if !defined(EVO_MSVC_YEAR) || EVO_MSVC_YEAR > 2015
		#define EVO_CPP14_FULL // MSVC 2015 has issues, use to detect full C++14 support
	#endif
#endif
#if __cplusplus >= 201703L || (defined(_MSVC_LANG) && _MSVC_LANG >= 201703L) || defined(DOXYGEN)
    /** Defined when compiler C++17 supported is enabled. */
    #define EVO_CPP17
#endif

#if defined(EVO_CPP11)
    /** Compile `EXPR` only if C++11 support is detected, otherwise this is a no-op.
     - \#include <evo/type.h>
     - This is useful to make code using C++11 features portable with and without C++11 support
     - Examples:
       \code
        #include <evo/type.h>

        // Function default template param not supported before C++11
        template<class T EVO_ONCPP11(=ulong)> void foo(T num) {
        };

        class Bar {
        public:
            Bar() { }
        private:
            // Disable copy constructor
            Bar(const Bar&) EVO_ONCPP11(= delete);
        };
       \endcode
     .
     \param  EXPR  Expression to use if C++11 is supported
    */
    #define EVO_ONCPP11(EXPR) EXPR

    /** Compile to `EXPR1` if C++11 support is detected, otherwise compile to `EXPR2`.
     - \#include <evo/type.h>
     - This is useful to make code using C++11 features portable with and without C++11 support
     .
     \param  EXPR1  Expression to use if C++11 is supported
     \param  EXPR2  Expression to use if C++11 is not supported
    */
    #define EVO_ONCPP11_ELSE(EXPR1, EXPR2) EXPR1
#else
    #define EVO_ONCPP11(EXPR)
    #define EVO_ONCPP11_ELSE(EXPR1, EXPR2) EXPR2
#endif

#if defined(EVO_CPP14)
    /** Compile `EXPR` only if C++14 support is detected, otherwise this is a no-op.
     - This is useful to specify `constexpr` when it improves code with C++14 support
     .
     \param  EXPR  Expression to use if C++14 is supported
    */
    #define EVO_ONCPP14(EXPR) EXPR
#else
    #define EVO_ONCPP14(EXPR)
#endif

#if defined(EVO_CPP14_FULL)
    /** Compile `EXPR` only if "full" C++14 support is detected, otherwise this is a no-op.
     - This is useful to specify `constexpr` when it improves code with C++14 support
	 - This excludes MSVC before 2017 due to C++14 issues
     .
     \param  EXPR  Expression to use if C++14 is fully supported
    */
	#define EVO_ONCPP14_FULL(EXPR) EXPR
#else
	#define EVO_ONCPP14_FULL(EXPR)
#endif

#if defined(EVO_CPP17)
    /** Compile `EXPR` only if C++17 support is detected, otherwise this is a no-op.
     \param  EXPR  Expression to use if C++17 is supported
    */
    #define EVO_ONCPP17(EXPR) EXPR
#else
    #define EVO_ONCPP17(EXPR)
#endif

// STL Compatibility
#if (EVO_STD_STRING || defined(_BASIC_STRING_H) || defined(_STRING_) || defined(_LIBCPP_STRING) || defined(_GLIBCXX_STRING) || (defined(_MSC_VER) && defined(_XSTRING_))) && EVO_STD_STRING != -1
    #include <string> // std::string
    #define EVO_STD_STRING_ENABLED 1
#endif
#if defined(EVO_CPP17) && (EVO_STD_STRING_VIEW || defined(_LIBCPP_STRING_VIEW) || defined(_GLIBCXX_STRING_VIEW) || (defined(_MSC_VER) && defined(_XSTRING_))) && EVO_STD_STRING_VIEW != -1
    #include <string_view> // std::string_view
    #define EVO_STD_STRING_VIEW_ENABLED 1
#endif

// Exception support
#if defined(__cpp_exceptions) || defined(__EXCEPTIONS) || defined(_CPPUNWIND) || defined(DOXYGEN)
    #define EVO_EXCEPTIONS_ENABLED 1
#endif

/** No-exceptions specification before and after C++11.
 - This is replaced with either "noexcept" (C++11) or "throw()" (before C++11)
*/
#if defined(EVO_CPP11)
    #define EVO_NOEXCEPT noexcept
#else
    #define EVO_NOEXCEPT throw()
#endif

// 64 bit environment
#if defined(__LP64__) || defined(_LP64) || defined(_WIN64) || defined(DOXYGEN)
    /** Defined when current architecture is 64-bit, if not then EVO_32 is defined for 32-bit instead. */
    #define EVO_64 1
    /** Number of bits for current architecture. */
    #define EVO_ARCH_BITS 64
#else
    #define EVO_32 1
    #define EVO_ARCH_BITS 32
#endif

// SSE optimization, use SSE 4.2 if AVX enabled
#if !defined(EVO_NO_CPU) && !defined(EVO_NO_SSE)
    #if !defined(EVO_NO_SSE42) && ( defined(EVO_USE_SSE42) || defined(__SSE4_2__) || defined(__AVX__) )
        #define EVO_IMPL_SSE42
        #define EVO_CPU "SSE 4.2"
    #elif !defined(EVO_NO_SSE2) && ( defined(EVO_USE_SSE2) || defined(__SSE2__) || defined(__AVX__) )
        #define EVO_IMPL_SSE2
        #define EVO_CPU "SSE 2"
    #elif defined(_MSC_VER) // no MSVC macro for SSE 4.2
        #if !defined(EVO_NO_SSE2) && ( defined(EVO_USE_SSE2) || defined(_M_X64) || (defined(_M_IX86) && _M_IX86_FP == 2) || defined(__AVX__) )
            #define EVO_IMPL_SSE2
            #define EVO_CPU "SSE 2"
        #endif
    #endif
#endif

/** Whether char is signed -- usually true, but unsigned on some systems. */
#if !defined(EVO_CHAR_SIGNED)
    #if defined(EVO_MSVC_YEAR) || defined(DOXYGEN)
        #define EVO_CHAR_SIGNED 1
    #elif (((char)-1) < 0)
        #define EVO_CHAR_SIGNED 1
    #else
        #define EVO_CHAR_SIGNED 0
    #endif
#endif

///////////////////////////////////////////////////////////////////////////////

// Whether to use glibc memmem()
#if !defined(EVO_NO_MEMMEM) && defined(__linux) && defined(__GNU_LIBRARY__) && defined(__GLIBC__) && defined(__GLIBC_MINOR__) && __GNU_LIBRARY__ >= 6 && __GLIBC__ >= 2
    // Requires at least glibc 2.12 -- see: https://bugzilla.redhat.com/show_bug.cgi?id=641128
    #if __GLIBC__ > 2 || __GLIBC_MINOR__ >= 12
        #define EVO_GLIBC_MEMMEM
    #endif
#endif

// Whether to use glibc memrchr()
#if !defined(EVO_NO_MEMRCHR) && defined(__linux) && defined(__GNU_LIBRARY__) && defined(__GLIBC__) && defined(__GLIBC_MINOR__) && __GNU_LIBRARY__ >= 6 && __GLIBC__ >= 2
    // Requires at least glibc 2.2
    #if __GLIBC__ > 2 || __GLIBC_MINOR__ >= 2
        #define EVO_GLIBC_MEMRCHR
    #endif
#endif

///////////////////////////////////////////////////////////////////////////////

// Some macros under Windows cause problems
/** \cond impl */
#if defined(_WIN32)
    #if defined(min)
        #undef min
    #endif
    #if defined(max)
        #undef max
    #endif
#endif
/** \endcond */

// Used to ignore certain warnings
#if defined(_MSC_VER)
    #define EVO_PARAM_UNUSED(NAME)       __pragma(warning(suppress:4100)) NAME
    #define EVO_MSVC_NOWARN_START(CODES) __pragma(warning(push)) __pragma(warning(disable:CODES))
    #define EVO_MSVC_NOWARN_END          __pragma(warning(pop))
#else
    /** Mark function parameter as unused to suppress "unreferenced parameter" compiler warnings on it.
     - Use this at the top of the function implementation, end with a semicolon
     - This is mainly supported by MSVC, and is ignored by other compilers
     .
     \param  NAME  Parameter name to mark as "unused"

    \par Example
    \code
    #include <evo/type.h>

    void foo(int arg1, int arg2) {
        EVO_PARAM_UNUSED(arg1);
        EVO_PARAM_UNUSED(arg2);
    }

    int main() {
        foo(1);
        return 0;
    }
    \endcode
    */
    #define EVO_PARAM_UNUSED(NAME)

    /** %Set starting point to ignore MSVC warnings.
     - This must be followed by a matching \link EVO_MSVC_NOWARN_END\endlink at some point in the same file
     - Semicolon not needed after this
     - This is MSVC specific and is ignored by other compilers
     .
     \param  CODES  Space separated list of warning codes to disable (without the 'C' prefix) -- see <a href="https://docs.microsoft.com/en-us/cpp/error-messages/compiler-warnings">MSVC warning codes</a>

    \par Example
    \code
    #include <evo/type.h>

    // Disable unused parameter and unused variable warnings in MSVC
    EVO_MSVC_NOWARN_START(4100 4101)

    int main(int arg, char** argv) {
        int myvar;
        return 0;
    }

    EVO_MSVC_NOWARN_END
    \endcode
    */
    #define EVO_MSVC_NOWARN_START(CODES)

    /** %Set end point for ignoring MSVC warnings.
     - See EVO_MSVC_NOWARN_START()
     - Semicolon not needed after this
     - This is MSVC specific and is ignored by other compilers
    */
    #define EVO_MSVC_NOWARN_END
#endif

#if defined(__GNUC__)
    #define EVO_ATTRIB_UNUSED __attribute__((unused))
#else
    /** Mark variable with "unused" attribute to suppress "unused variable" compiler warnings on it.
     - Use this after the variable name in the variable declaration
     - This is mainly supported by GCC and Clang compilers, and is ignored by other compilers
     .
    \par Example
    \code
    #include <evo/type.h>

    int main() {
        int myvar1 EVO_ATTRIB_UNUSED;
        int myvar2 EVO_ATTRIB_UNUSED = 0;
        return 0;
    }
    \endcode
    */
    #define EVO_ATTRIB_UNUSED
#endif

///////////////////////////////////////////////////////////////////////////////

/** Mark a variable for thread-local storage.
 - With C++11 this is the same as the "thread_local" keyword, but this also works on older compilers
 - Older compilers may only support POD or basic types as thread-local, depending on compiler version
 .

\par Example
\code
EVO_THREAD_LOCAL static int var;
\endcode
*/
#if defined(EVO_CPP11)
    #define EVO_THREAD_LOCAL thread_local
#elif defined(EVO_MSVC_YEAR)
    #define EVO_THREAD_LOCAL __declspec(thread)
#else
    #define EVO_THREAD_LOCAL __thread
#endif

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
/** \addtogroup EvoCore */
//@{
///////////////////////////////////////////////////////////////////////////////

// Primitive aliases
/** \cond impl */
typedef unsigned char          uchar;   ///< Alias for unsigned char
typedef unsigned short         ushort;  ///< Alias for unsigned short
typedef unsigned long long int ulongl;  ///< Alias for unsigned long long int
typedef long long int          longl;   ///< Alias for long long int
typedef long double            ldouble; ///< Alias for long double
#if defined(__unix__) || defined(__APPLE__) || defined(DOXYGEN)
    typedef unsigned int  uint;         ///< Alias for unsigned int
    typedef unsigned long ulong;        ///< Alias for unsigned long
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
    typedef int8_t   int8;      ///< 8-bit signed integer
    typedef int16_t  int16;     ///< 16-bit signed integer
    typedef int32_t  int32;     ///< 32-bit signed integer
    typedef int64_t  int64;     ///< 64-bit signed integer
    typedef uint8_t  uint8;     ///< 8-bit unsigned integer
    typedef uint16_t uint16;    ///< 16-bit unsigned integer
    typedef uint32_t uint32;    ///< 32-bit unsigned integer
    typedef uint64_t uint64;    ///< 64-bit unsigned integer
#endif

typedef uint16 wchar16;     ///< Wide UTF-16 character
typedef uint32 wchar32;     ///< Wide UTF-32 character
/** \endcond */

///////////////////////////////////////////////////////////////////////////////

/** Shortcut for NULL as const-char pointer. */
#define EVO_CNULL ((const char*)0)

/** Shortcut for NULL as void-pointer. */
#define EVO_VNULL ((void*)0)

///////////////////////////////////////////////////////////////////////////////

// Namespace: evo
namespace evo {

///////////////////////////////////////////////////////////////////////////////

#if defined(_WIN32)
/** \cond impl */
namespace impl {
    // High preceision sleep in 100 nsec units
    inline bool nano100sleep(ulongl nsec100) {
        bool result;
        HANDLE timer = ::CreateWaitableTimer(NULL, TRUE, NULL);
        if (timer == NULL) {
            result = false;
        } else {
            LARGE_INTEGER tm;
            if (nsec100 > (ulongl)std::numeric_limits<LONGLONG>::max())
                tm.QuadPart = std::numeric_limits<LONGLONG>::min();
            else
                tm.QuadPart = -(LONGLONG)nsec100;
            result = (::SetWaitableTimer(timer, &tm, 0, NULL, NULL, FALSE) != 0 && ::WaitForSingleObject(timer, INFINITE) != WAIT_FAILED);
            ::CloseHandle(timer);
        }
        return result;
    }
}
/** \endcond */
#endif

/** Sleep for number of milliseconds.
 - \#include <evo/type.h> (included by most evo headers)
 - \b Caution: Do not sleep in an async callback as it will block other callbacks
 .
 \param  msec  Milliseconds to sleep, must be > 0
 \return       Whether successful, false if sleep failed (shouldn't happen)
*/
inline bool sleepms(ulong msec) {
    assert( msec > 0 );
#if defined(_WIN32)
    ::Sleep(msec);
#else
    const ulong MSEC_PER_SEC = 1000;
    const long NSEC_PER_MSEC = 1000000;
    timespec tm;
    tm.tv_sec  = (msec / MSEC_PER_SEC);
    tm.tv_nsec = (long)(msec - (MSEC_PER_SEC * tm.tv_sec)) * NSEC_PER_MSEC;
    while (nanosleep(&tm, &tm) == -1)
        if (errno != EINTR)
            return false;
#endif
    return true;
}

/** Sleep for number of microseconds.
 - \#include <evo/type.h> (included by most evo headers)
 - On UNIX/Linux systems this will ignore signals
 - \b Caution: Do not sleep in an async callback as it will block other callbacks
 .
 \param  usec  Microseconds to sleep, must be > 0
 \return       Whether successful, false if sleep failed (shouldn't happen)
*/
inline bool sleepus(ulongl usec) {
    assert( usec > 0 );
#if defined(_WIN32)
    const ulongl NSEC100_PER_USEC = 10;
    return impl::nano100sleep(usec * NSEC100_PER_USEC);
#else
    const ulongl USEC_PER_SECOND = 1000000;
    const long NSEC_PER_USEC = 1000;
    timespec tm;
    tm.tv_sec  = (usec / USEC_PER_SECOND);
    tm.tv_nsec = (long)(usec - (USEC_PER_SECOND * tm.tv_sec)) * NSEC_PER_USEC;
    while (nanosleep(&tm, &tm) == -1)
        if (errno != EINTR)
            return false;
    return true;
#endif
}

/** Sleep for number of nanoseconds.
 - \#include <evo/type.h> (included by most evo headers)
 - On UNIX/Linux systems this will ignore signals
 - On Windows the lowest precision is in 100-nanosecond units so this will sleep at least 100 nanoseconds
 - \b Caution: Do not sleep in an async callback as it will block other callbacks
 .
 \param  nsec  Nanoseconds to sleep, must be > 0
 \return       Whether successful, false if sleep failed (shouldn't happen)
*/
inline bool sleepns(ulongl nsec) {
    assert( nsec > 0 );
#if defined(_WIN32)
    const ulongl NSEC_PER_NSEC100 = 100;
    return impl::nano100sleep(nsec < NSEC_PER_NSEC100 ? 1 : nsec / NSEC_PER_NSEC100);
#else
    const ulongl NSEC_PER_SECOND = 1000000000ULL;
    timespec tm;
    tm.tv_sec  = (nsec / NSEC_PER_SECOND);
    tm.tv_nsec = (long)(nsec - (NSEC_PER_SECOND * tm.tv_sec));
    while (nanosleep(&tm, &tm) == -1)
        if (errno != EINTR)
            return false;
    return true;
#endif
}

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

/** Signals an output stream to flush pending data. */
enum Flush { fFLUSH=0 };

///////////////////////////////////////////////////////////////////////////////

/** Newline type.
 - Use \link evo::NL NL\endlink for default newline type, or \link evo::NL_SYS NL_SYS\endlink for explicit system newline
 - Newline values are supported by Stream and String operator<<()
 .
*/
enum Newline {  // The order here is important and must align with functions below
    nLF=0,          ///< Line Feed "\n" (Linux/Unix/MacOS)
    nCR,            ///< Carriage Return "\r" (Classic MacOS)
    nCRLF,          ///< Carriage Return + Line Feed "\r\n" (Windows, DOS, Internet)
    nLFCR           ///< Line Feed + Carriage Return "\n\r" (RISC OS)
};

/** Current system newline type.
 - When used with Stream operator<<() this will usually trigger a flush as well
*/
#if defined(_WIN32)
    // Windows
    static const Newline NL_SYS = nCRLF;
#else
    // Linux/Unix
    static const Newline NL_SYS = nLF;
#endif

/** Default newline type, implicitly converts to \ref NL_SYS (system default newline).
 - This is used to define the \ref NL (default newline) constant, which implicitly converts to \ref NL_SYS (system default newline) with functions taking a \ref Newline type
 - This is useful for having a class differentiate between \ref NL (default newline defined by class) and explicit \ref Newline values like \ref NL_SYS (system default) or \ref nLF
   - Stream does this, ex: an instance of File or Socket can define it's own default newline type used when \ref NL is passed as \ref Newline
   - Passing \ref NL_SYS explicitly uses the system default newline type rather than the class default, or passing \ref nLF explicitly uses that newline value
 - See also: NewlineValue
*/
struct NewlineDefault {
    /** Implicit conversion to system newline value.
     \return  System newline value (\ref NL_SYS)
    */
    operator Newline() const
        { return NL_SYS; }
};

/** Default newline type.
 - By default this is the same as the system newline (\ref NL_SYS), though Stream instances may override with their own default
 - When used with Stream operator<<() this will usually trigger a flush as well
*/
static const NewlineDefault& NL = NewlineDefault();

/** Get newline string for given type.
 - \#include <evo/type.h> (included by most evo headers)
 .
 \param  newline  Newline type (default is newline for current platform)
 \return          Newline string
*/
inline const char* getnewline(Newline newline=NL) {
    const char* NEWLINES[] = { "\n", "\r", "\r\n", "\n\r" };
    return NEWLINES[(int)newline];
}

/** Get newline string size for given type.
 - \#include <evo/type.h> (included by most evo headers)
 .
 \param  newline  Newline type (default is newline for current platform)
 \return          Newline string size (length)
*/
inline uint getnewlinesize(Newline newline=NL) {
    const uint SIZES[] = { 1, 1, 2, 2 };
    return SIZES[(int)newline];
}

/** Holds a Newline value that can be null, which implicitly converts to \ref NL_SYS (system default newline).
 - This is useful for storing and forwarding a newline choice to an object that may define it's own default newline (like Stream and StreamOut)
 - See Newline and NewlineDefault
*/
class NewlineValue {
public:
    /** Constructor initializes as null. */
    NewlineValue() : null_(true), nl_(NL_SYS) {
    }

    /** Constructor initializes with a Newline value.
     \param  nl  Newline value
    */
    NewlineValue(Newline nl) : null_(false), nl_(nl) {
    }

    /** Constructor initializes as null via NewlineDefault. */
    NewlineValue(const NewlineDefault&) : null_(true), nl_(NL_SYS) {
    }

    /** Copy constructor.
     \param  src  Source to copy
    */
    NewlineValue(const NewlineValue& src) : null_(src.null_), nl_(src.nl_) {
    }

    /** Get whether null.
     \return  Whether null
    */
    bool null() {
        return null_;
    }

    /** Assignment operator sets to Newline value.
     \param  nl  Newline value
     \return     This
    */
    NewlineValue& operator=(Newline nl) {
        null_ = false;
        nl_   = nl;
        return *this;
    }

    /** Assignment operator sets as null via NewlineDefault.
     \return  This
    */
    NewlineValue& operator=(NewlineDefault) {
        null_ = true;
        nl_   = NL_SYS;
        return *this;
    }

    /** Assignment operator to set as copy.
     \param  src  Newline value to copy
     \return      This
    */
    NewlineValue& operator=(const NewlineValue& src) {
        null_ = src.null_;
        nl_   = src.nl_;
        return *this;
    }

    /** Implicit conversion to a Newline value.
     - If null this returns \ref NL_SYS
     .
     \return  Newline value
    */
    operator Newline() const
        { return nl_; }

    /** Get newline string pointer for current value.
     - If null this uses \ref NL_SYS
     - Call getnewlinesize() to get newline string size
     .
     \return  Newline string pointer
    */
    const char* getnewline() const {
        return evo::getnewline(nl_);
    }

    /** Get newline string pointer and size for current value, or use default string if null.
     \param  size             %Set to newline string size  [out]
     \param  default_newline  Default newline string to use if null
     \param  default_size     Default newline string size to use if null
     \return                  Newline string pointer
    */
    const char* getnewline(uint& size, const char* default_newline, uint default_size) const {
        if (null_) {
            size = default_size;
            return default_newline;
        }
        size = evo::getnewlinesize(nl_);
        return evo::getnewline(nl_);
    }

    /** Get newline string size for current value.
     - If null this uses \ref NL_SYS
     .
     \return  Newline string size
    */
    uint getnewlinesize() const {
        return evo::getnewlinesize(nl_);
    }

private:
    bool null_;
    Newline nl_;
};

///////////////////////////////////////////////////////////////////////////////

/** Working data buffer.
 - This manages a working buffer and frees it in destructor
 - Pass to certain methods to optimize memory allocations with repeated calls by reusing the buffer
 - If desired, call get() to pre-allocate a buffer
*/
template<class T>
class WorkBuffer {
public:
    /** Constructor. */
    WorkBuffer() : buf_(NULL), size_(0)
        { }

    /** Destructor. */
    ~WorkBuffer()
        { free(); }

    /** Get buffer pointer.
     - The call allocates the initial buffer
     - Additional calls with a larger size grow (reallocate) the buffer
     .
     \param  size  Minimum required buffer size
     \return       Buffer pointer
    */
    T* get(ulong size) {
        if (buf_ == NULL) {
            buf_  = (T*)::malloc(size * sizeof(T));
            size_ = size;
        } else if (size > size_) {
            buf_  = (T*)::realloc(buf_, size * sizeof(T));
            size_ = size;
        }
        return buf_;
    }

private:
    // Disable copying
    WorkBuffer(const WorkBuffer&);
    WorkBuffer& operator=(const WorkBuffer&);

    T*    buf_;
    ulong size_;

    void free() {
        if (buf_ != NULL)
            ::free(buf_);
    }
};

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

    ListBase(const ListBaseType& src) {
        data_ = src.data_;
        size_ = src.size_;
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

// If enabled, implicitly convert "const std::string&" and "const std::string*" to StringBase (ListBase<char>)
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

    ListBase(const ListBaseType& src) {
        data_ = src.data_;
        size_ = src.size_;
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

#if EVO_STD_STRING_VIEW_ENABLED
    ListBase(const std::basic_string_view<char>& str) {
        size_ = (TSize)str.length();
        data_ = (char*)(size_ > 0 ? str.data() : "");
    }

    ListBase(const std::basic_string_view<char>* str) {
        if (str == NULL) {
            data_ = NULL;
            size_ = 0;
        } else {
            data_ = (char*)str->data();
            size_ = (TSize)str->length();
        }
    }
#endif
};
#endif
/** \endcond */

///////////////////////////////////////////////////////////////////////////////

/** %Null type. */
struct Null { };

/** Unique null value type and value (vNULL).
 - This gives a consistent null type/value before and after C++11, and is distinct from pointer types like C++11 `nullptr` (doesn't conflict with overloads using pointers)
*/
enum ValNull {
    vNULL=0         ///< %Null value with unique type, same as `nullptr` with C++11, otherwise a unique enum value before C++11
};

/** Special empty value type, pass as vEMPTY. */
enum ValEmpty {
    vEMPTY=0        ///< Special empty value passed with type ValEmpty
};

/** Special value type to reverse algorithm, pass as vREVERSE. */
enum ValAlgReverse {
    vREVERSE=0      ///< Special value to reverse an algorithm passed with type ValAlgReverse
};

///////////////////////////////////////////////////////////////////////////////

/** General Evo error code stored in exceptions, or used directly when exceptions are disabled. */
enum Error {
    // Non-errors first
    ENone = 0,          ///< No error
    EEnd,               ///< Reached end of resource (not an error)
    EMoreOutput,        ///< More pending output to flush (not an error)
    ENonBlock,          ///< Operation would block (non-blocking I/O)
    // General errors
    EUnknown,           ///< Unknown or unspecified error
    ENotImpl,           ///< Function not supported/implemented
    EInvalOp,           ///< Invalid or unsupported operation
    EInval,             ///< Invalid argument or data
    EFail,              ///< Operation failed
    ELoss,              ///< Operation aborted to prevent data loss (used in special cases)
    ERetry,             ///< Temporary failure occurred, try again
    EBufSize,           ///< Buffer too small error
    // More specific errors
    EOutOfBounds,       ///< Out of bounds error
    EClosed,            ///< %Stream or resource is closed
    ETimeout,           ///< Operation timed out
    ESignal,            ///< System call interrupted by signal (Linux/Unix), auto-resume disabled
    EPtr,               ///< Invalid pointer used
    ESpace,             ///< Not enough space/memory available
    EAccess,            ///< Permission denied
    EExist,             ///< Resource already exists
    ENotFound,          ///< Resource not found
    ENotDir,            ///< Path component is not a directory
    ESize,              ///< Size limit exceeded
    ELimit,             ///< System resource limit reached
    ELength,            ///< %String length limit exceeded
    // I/O Errors
    ERead,              ///< General stream or resource read error
    EWrite,             ///< General stream or resource write error
    EInput              ///< Truncated or incomplete input (error version of EEnd)
    // EInput expected to be last in errormsg()
};

/** Get general error message for error code.
 - \#include <evo/type.h> (included by most evo headers)
 .
 \param  error  %Error code -- see Error
 \return        Message for error code (null terminated, always an immutable string literal)
*/
inline const char* errormsg(Error error) {
    const char* msgs[] = {
        // Non-errors first
        "No error (ENone)",
        "End of resource (EEnd)",
        "More pending output to flush (EMoreOutput)",
        "Operation would block (non-blocking I/O) (ENonBlock)",
        // General errors
        "Unknown error (EUnknown)",
        "Function not supported/implemented (ENotImpl)",
        "Invalid or unsupported operation (EInvalOp)",
        "Invalid argument or data (EInval)",
        "Operation failed (EFail)",
        "Aborted to prevent data loss (ELoss)",
        "Temporary failure, try again (ERetry)",
        "Buffer too small (EBufSize)",
        // More specific errors
        "Data out of bounds (EOutOfBounds)",
        "Resource is closed (EClosed)",
        "Operation timed out (ETimeout)",
        "Interrupted by signal (ESignal)",
        "Invalid pointer (EPtr)",
        "Not enough space/memory available (ESpace)",
        "Permission denied (EAccess)",
        "Resource already exists or in use (EExist)",
        "Resource not found (ENotFound)",
        "Path component is not a directory (ENotDir)",
        "Size or resource limit exceeded (ESize)",
        "System resource limit reached (ELimit)",
        "String too long (ELength)",
        // I/O Errors
        "General read error (ERead)",
        "General write error (EWrite)",
        "Truncated or incomplete input (EInput)"
    };
    if (error < 0 || error > EInput)
        return "Bad error code";
    return msgs[(int)error];
}

/** Write error message with errno to output stream/string.
 - \#include <evo/type.h> (included by most evo headers)
 - Must call right after the error, otherwise errno may be overwritten
 .
 \param  out  Stream or String to write output to
 \param  err  Error code to format message for
*/
template<class TOut>
inline TOut& errormsg_out(TOut& out, Error err) {
    out << errormsg(err) << ListBase<char>(" (errno:", 8) << errno << ')';
    return out;
}

///////////////////////////////////////////////////////////////////////////////

/** Evo base exception class.
 - Use EVO_THROW() or EVO_THROW_ERR() macro to throw an Evo exception
 - See EVO_CREATE_EXCEPTION() for creating a custom Evo exception type
*/
class Exception {
public:
    typedef ListBase<char,StrSizeT> StringBase; ///< Alias for evo::StringBase

    /** Constructor with file/line info.
     - Use EVO_THROW() or EVO_THROW_ERR() macro to throw exception and automatically populate the file/line info
     .
     \param  file   Exception file name (use __FILE__ macro)
     \param  line   Exception line number (use __LINE__ macro)
     \param  msg    Exception message
     \param  error  Error code, EUnknown if unknown
    */
    Exception(const char* file, ulong line, const StringBase& msg, Error error=EUnknown) : file_(file), line_(line), msg_(NULL), error_(error), errormsg_func_(evo::errormsg)
        { init("Exception", msg.data_, msg.size_); }

    /** Copy constructor.
     \param  e  Exception to copy
    */
    Exception(const Exception& e) : file_(e.file_), line_(e.line_), msg_(NULL), error_(e.error_), errormsg_func_(e.errormsg_func_)
        { init(e.msg_, strlen(e.msg_)); }

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
        { return (msg_ == NULL) ? "Unexpected Exception" : msg_; }

    /** Get error code.
     \return  Error code, 0 if unknown
    */
    Error error() const
        { return error_; }

    /** Write error message with errno to output stream/string.
     - Must call right after the error, otherwise errno may be overwritten
     .
     \tparam  TOut  Output stream or string type

     \param  out  Stream or String to write output to
     \return      This
    */
    template<class TOut>
    TOut& errormsg_out(TOut& out) {
        out << errormsg_func_(error_) << ListBase<char>(" (errno:", 8) << errno << ')';
        return out;
    }

protected:
    typedef const char* (*ErrorMsgFunc)(Error);     ///< Pointer type to function returning error message from error code

    /** Constructor with exception type. Use EVO_THROW() or EVO_THROW_E() macro to throw exception.
     \param  file   Exception file name
     \param  line   Exception line number
     \param  type   Exception type as string, used as msg prefix
     \param  msg    Exception message
     \param  error  Error code, EUnknown if unknown
     \param  errmsg_func  %Error message function to use
    */
    Exception(const char* file, ulong line, const char* type, const StringBase& msg, Error error, ErrorMsgFunc errmsg_func) : file_(file), line_(line), msg_(NULL), error_(error), errormsg_func_(errmsg_func)
        { init(type, msg.data_, msg.size_); }

private:
    const char*  file_;             ///< File name
    ulong        line_;             ///< Line number
    char*        msg_;              ///< Message
    Error        error_;            ///< Error code, EUnknown if unknown
    ErrorMsgFunc errormsg_func_;    ///< %Error message function to use

    void init(const char* type, const char* msg, size_t len) {
        size_t type_len  = strlen(type);
        size_t total_len = type_len + 2 + len;
        if (total_len > 0) {
            msg_ = (char*)malloc(total_len+1);
            memcpy(msg_, type, type_len);
            memcpy(msg_+type_len, ": ", 2);
            memcpy(msg_+type_len+2, msg, len);
            msg_[total_len] = '\0';
        }
    }

    void init(const char* msg, size_t len) {
        if (len > 0) {
            msg_ = (char*)malloc(len+1);
            memcpy(msg_, msg, len);
            msg_[len] = '\0';
        }
    }
};

///////////////////////////////////////////////////////////////////////////////

/** Create an Evo exception implementation with custom errormsg() function.
 - This works the same as EVO_CREATE_EXCEPTION_IMPL() but has an additional parameter to specify the errormsg() function to use
 .
 \param  NAME         Name of new exception type
 \param  BASE         Base exception type -- \link evo::Exception Exception\endlink or derived from it
 \param  ERRMSG_FUNC  %Error message function to use to format \link evo::Error Error\endlink code

\par Example

\code
#include <evo/type.h>

class ExceptionFoo : public Exception {
    EVO_CREATE_EXCEPTION_IMPL(ExceptionFoo, Exception)
};
\endcode
*/
#define EVO_CREATE_EXCEPTION_IMPL_2(NAME, BASE, ERRMSG_FUNC) \
    public: \
        using BASE::StringBase; \
        NAME(const char* file, ulong line, const StringBase& msg, Error error=EUnknown) : BASE(file, line, #NAME, msg, error, ERRMSG_FUNC) { } \
        NAME(const NAME& e) : BASE(e) { } \
    protected: \
        NAME(const char* file, ulong line, const char* type, const StringBase& msg, Error error, ErrorMsgFunc errormsg_func) : BASE(file, line, type, msg, error, errormsg_func) { }

/** Create an Evo exception implementation.
 - This "pastes" the implementation for an Evo exception class
 - Use this to be explicit about creating the exception class, and so tools like doxygen see (and document) the class
 - This uses the default errormsg() function to format \link evo::Error Error\endlink codes, use EVO_CREATE_EXCEPTION_IMPL_2() to override this
 - Alternatively, use EVO_CREATE_EXCEPTION() to create an Evo exception
 .
 \param  NAME  Name of new exception type
 \param  BASE  Base exception type -- \link evo::Exception Exception\endlink or derived from it

\par Example

\code
#include <evo/type.h>

class ExceptionFoo : public Exception {
    EVO_CREATE_EXCEPTION_IMPL(ExceptionFoo, Exception)
};
\endcode
*/
#define EVO_CREATE_EXCEPTION_IMPL(NAME, BASE) EVO_CREATE_EXCEPTION_IMPL_2(NAME, BASE, evo::errormsg)

/** Create an Evo exception from a base exception.
 - This defines the exception class and the implementation
 - Note that tools like doxygen may not notice classes defined via macro, see EVO_CREATE_EXCEPTION_IMPL() for an alternative
 .
 \param  NAME  Name of new exception type
 \param  BASE  Base exception type -- \link evo::Exception Exception\endlink or derived from it

\par Example

\code
#include <evo/type.h>

EVO_CREATE_EXCEPTION(ExceptionFoo, Exception);
\endcode
*/
#define EVO_CREATE_EXCEPTION(NAME, BASE) \
    class NAME : public BASE \
        { EVO_CREATE_EXCEPTION_IMPL(NAME, BASE) }

// I/O

/** Base stream exception for all stream errors, see Exception. */
class ExceptionStream : public Exception
    { EVO_CREATE_EXCEPTION_IMPL(ExceptionStream, Exception) };

/** %Stream open exception for errors opening a stream, see Exception. */
class ExceptionStreamOpen : public ExceptionStream
    { EVO_CREATE_EXCEPTION_IMPL(ExceptionStreamOpen, ExceptionStream) };

/** Input stream exception for stream read errors, see Exception. */
class ExceptionStreamIn : public ExceptionStream
    { EVO_CREATE_EXCEPTION_IMPL(ExceptionStreamIn, ExceptionStream) };

/** Output stream exception for stream write errors, see Exception. */
class ExceptionStreamOut : public ExceptionStream
    { EVO_CREATE_EXCEPTION_IMPL(ExceptionStreamOut, ExceptionStream) };


/** File open exception for errors opening a file, see Exception. */
class ExceptionFileOpen : public ExceptionStreamOpen
    { EVO_CREATE_EXCEPTION_IMPL(ExceptionFileOpen, ExceptionStreamOpen) };

/** File input stream exception for file read errors, see Exception. */
class ExceptionFileIn : public ExceptionStreamIn
    { EVO_CREATE_EXCEPTION_IMPL(ExceptionFileIn, ExceptionStreamIn) };

/** File output stream exception for file write errors, see Exception. */
class ExceptionFileOut : public ExceptionStreamOut
    { EVO_CREATE_EXCEPTION_IMPL(ExceptionFileOut, ExceptionStreamOut) };

///////////////////////////////////////////////////////////////////////////////

#if defined(EVO_EXCEPTIONS_ENABLED)
    /** Throw an Evo exception.
     - This passes the current source filename and line number to the exception
     - Alternatively, use EVO_THROW_ERR() to throw with an error code
     - If exceptions are disabled this calls `abort()`
     .
     \param  TYPE  %Exception type (Exception or derived from it)
     \param  MSG   %Exception message string (const char*)

    \par Example

    \code
    #include <evo/type.h>
    #include <evo/io.h>

    class ExceptionCustom : public Exception {
        EVO_CREATE_EXCEPTION_IMPL(ExceptionCustom, Exception)
    };

    int main() {
        try {
            EVO_THROW(ExceptionCustom, "Error message");
        } EVO_CATCH()
        return 0;
    }
    \endcode
    */
    #define EVO_THROW(TYPE, MSG) throw TYPE(__FILE__, __LINE__, MSG)

    /** Throw an Evo exception with error code.
     - This passes the current source filename and line number to the exception
     - Alternatively, use EVO_THROW_ERR_CHECK() to throw on an error condition
     - If exceptions are disabled this calls `abort()`
     .
     \param  TYPE   %Exception type (Exception or derived from it)
     \param  MSG    %Exception message string (const char*)
     \param  ERROR  %Error code -- see Error

    \par Example

    \code
    #include <evo/type.h>
    #include <evo/io.h>

    class ExceptionCustom : public Exception {
        EVO_CREATE_EXCEPTION_IMPL(ExceptionCustom, Exception)
    };

    int main() {
        try {
            EVO_THROW_ERR(ExceptionCustom, "Error message", EUnknown);
        } EVO_CATCH()
        return 0;
    }
    \endcode
    */
    #define EVO_THROW_ERR(TYPE, MSG, ERROR) throw TYPE(__FILE__, __LINE__, MSG, ERROR)

    /** Throw an Evo exception with error code if COND is true.
     - This passes the current source filename and line number to the exception
     - This only throws if COND evaluates to true
       - If exceptions are disabled this calls `abort()` instead of throw
     .
     \param  TYPE   %Exception type (Exception or derived from it)
     \param  MSG    %Exception message string (const char*)
     \param  ERROR  %Error code -- see Error
     \param  COND   %Error condition to check

    \par Example

    \code
    #include <evo/type.h>
    #include <evo/io.h>
    using namespace evo;

    class ExceptionCustom : public Exception {
        EVO_CREATE_EXCEPTION_IMPL(ExceptionCustom, Exception)
    };

    // Function that returns an error code
    Error foo() {
        // ...
        return ENone;
    }

    int main() {
        try {
            Error err = foo();
            EVO_THROW_ERR_CHECK(ExceptionCustom, "Error message", EUnknown, (err != ENone));
        } EVO_CATCH()
        return 0;
    }
    \endcode
    */
    #define EVO_THROW_ERR_CHECK(TYPE, MSG, ERROR, COND) { if (COND) throw TYPE(__FILE__, __LINE__, MSG, ERROR); }
#else
    // Exceptions disabled, use abort()
    #define EVO_THROW(TYPE, MSG) abort()
    #define EVO_THROW_ERR(TYPE, MSG, ERROR) abort()
    #define EVO_THROW_ERR_CHECK(TYPE, MSG, ERROR, COND) { if (COND) abort(); }
#endif

///////////////////////////////////////////////////////////////////////////////

#if !defined(_WIN32)
    // Linux/Unix
    struct SysLinux {
        static void set_timeval_ms(struct timeval& tm, ulong ms) {
            const ulong MSEC_PER_SEC  = 1000;
            const ulong USEC_PER_MSEC = 1000;
            tm.tv_sec  = ms / MSEC_PER_SEC;
            tm.tv_usec = (ms - (tm.tv_sec * MSEC_PER_SEC)) * USEC_PER_MSEC;
        }

        static void set_timespec_tv(struct timespec& tm, struct timeval& tv) {
            const long NSEC_PER_USEC = 1000;
            tm.tv_sec = tv.tv_sec;
            tm.tv_nsec = tv.tv_usec * NSEC_PER_USEC;
        }

        static void set_timespec_ms(struct timespec& tm, ulong ms) {
            const ulong MSEC_PER_SEC  = 1000;
            const ulong NSEC_PER_MSEC = 1000000;
            tm.tv_sec  = ms / MSEC_PER_SEC;
            tm.tv_nsec = (ms - (tm.tv_sec * MSEC_PER_SEC)) * NSEC_PER_MSEC;
        }

        static void add_timespec_ms(struct timespec& tm, ulong ms) {
            const ulong MSEC_PER_SEC  = 1000;
            const ulong NSEC_PER_MSEC = 1000000;
            const long NSEC_PER_SEC   = 1000000000;
            {
                const time_t add_sec = (time_t)(ms / MSEC_PER_SEC);
                tm.tv_sec += add_sec;
                tm.tv_nsec += ((ms - (add_sec * MSEC_PER_SEC)) * NSEC_PER_MSEC);
            }
            const long add_sec = (tm.tv_nsec / NSEC_PER_SEC);
            tm.tv_sec += add_sec;
            tm.tv_nsec -= (add_sec * NSEC_PER_SEC);
        }

        static void set_timespec_now(struct timespec& tm) {
        #if defined(_POSIX_TIMERS) && defined(CLOCK_REALTIME) && !defined(EVO_USE_GETTIMEOFDAY)
            #if defined(CLOCK_MONOTONIC_RAW)
                ::clock_gettime(CLOCK_MONOTONIC_RAW, &tm);
            #elif defined(CLOCK_MONOTONIC)
                ::clock_gettime(CLOCK_MONOTONIC, &tm);
            #else
                ::clock_gettime(CLOCK_REALTIME, &tm);
            #endif
        #else
            struct timeval tv;
            ::gettimeofday(&tv, NULL);
            SysLinux::set_timespec_tv(tm, tv);
        #endif
        }

        static int compare_timespec(const struct timespec& a, const struct timespec& b) {
            if (a.tv_sec < b.tv_sec)
                return -1;
            else if (a.tv_sec > b.tv_sec)
                return 1;
            else if (a.tv_nsec < b.tv_nsec)
                return -1;
            else if (a.tv_nsec > b.tv_nsec)
                return 1;
            return 0;
        }
    };
#endif

///////////////////////////////////////////////////////////////////////////////

/** Swap contents of given objects.
 - \#include <evo/type.h> (included by most evo headers)
 - The object memory is swapped directly, so this works with any relocatable object type
 .
 \tparam  T  Object type, inferred from parameters

 \param  a  First object to swap
 \param  b  Second object to swap
*/
template<class T>
inline void swap(T& a, T& b) {
    char tmp[sizeof(T)];
    memcpy(tmp, &a,  sizeof(T));
    memcpy(&a,  &b,  sizeof(T));
    memcpy(&b,  tmp, sizeof(T));
}

///////////////////////////////////////////////////////////////////////////////
}
//@}
#endif
