// Evo C++ Library
/* Copyright 2019 Justin Crowell
Distributed under the BSD 2-Clause License -- see included file LICENSE.txt for details.
*/
///////////////////////////////////////////////////////////////////////////////
/** \file evo_config.h Evo Library Configuration. */
#pragma once
#ifndef INCL_evo_evo_config_h
#define INCL_evo_evo_config_h

/** \addtogroup EvoCore */
//@{

///////////////////////////////////////////////////////////////////////////////
// Version

/** Evo version number (major.minor). */
#define EVO_VERSION 0.5

/** Evo version patch number. */
#define EVO_VERSION_PATCH 0

/** Evo version string (major.minor.patch). */
#define EVO_VERSION_STRING "0.5.0"

///////////////////////////////////////////////////////////////////////////////
// Global

// Exceptions
#if !defined(EVO_EXCEPTIONS)
    /** Whether to throw exceptions on error by default.
     - Default: 1
     - Disabling exceptions can help performance when critical, however error codes should then be checked directly
    */
    #define EVO_EXCEPTIONS 1
#endif

#if !defined(EVO_CATCH_DEBUG)
    /** Enable additional debug info when printing exceptions from EVO_CATCH() and EVO_CATCH_MT().
     - Default: 0
     - Enabling this changes the EVO_CATCH() helpers to include source file path and line number in the exception error message
    */
    #define EVO_CATCH_DEBUG 0
#endif

///////////////////////////////////////////////////////////////////////////////
// Containers

#if !defined(EVO_STD_STRING)
    /** Whether to support std::string compatibility in Evo string classes (String, SubString).
     - Default: 0 (auto-detect, enable if std::string included)
     - By default this will try to auto-detect whether STL strings have been included already, and if so enable std::string support
     - Set to 1 to explicitly enable (and auto-include std::string), -1 to explicitly disable and skip auto-detect
     - Set this BEFORE including any Evo headers
     - When enabled, any function that takes "const StringBase&" will automatically support "const std::string&" and "const std::string*"
       - Such functions should use "const SubString&" instead of the base type though
    */
    #define EVO_STD_STRING 0
#endif

// Work-In-Progress: Do not change from defaults at this time

// Size Type
#if !defined(EVO_SIZE_TYPE)
    /** Sets default size type for Evo containers.
     - Default: uint32
     - Always unsigned
     - This becomes type: SizeT
    */
    #define EVO_SIZE_TYPE uint32
#endif

// String Size Type
#if !defined(EVO_STR_SIZE_TYPE)
    /** Sets default string size type for Evo string containers.
     - Default: uint32
     - Always unsigned
     - This becomes type: StrSizeT
    */
    #define EVO_STR_SIZE_TYPE uint32
#endif

// Allocators
#if !defined(EVO_ALLOCATORS)
    /** Enable container allocators -- NOT YET SUPPORTED.
     - Default: 0
    */
    #define EVO_ALLOCATORS 0
#endif

// List
#if !defined(EVO_LIST_REALLOC)
    /** List: Enable using realloc to resize buffer.
     - This could improve performance in some systems
     - Default: 1
    */
    #define EVO_LIST_REALLOC 1
#endif
#if !defined(EVO_LIST_OPT_EXTREF)
    /** List: Enable optimization for referencing external lists rather than copying.
     - When enabled, each String variable uses a little more memory
     - Disabling this also disables EVO_LIST_OPT_LAZYBUF and EVO_LIST_OPT_EXTREF (?)
     - Default: 1
    */
    #define EVO_LIST_OPT_EXTREF 1
#endif
#if !defined(EVO_LIST_OPT_LAZYREM)
    /** List: Enable lazy removal optimization when removing items.
     - When enabled, List::remove() will trim begining/end items, to be actually removed later
     - Can improve performance when beginning and/or end items are added and removed a lot
     - Default: 0
    */
    #define EVO_LIST_OPT_LAZYREM 0
#endif
#if !defined(EVO_LIST_OPT_LAZYBUF)
    /** List: Enable lazy buffer optimization for keeping buffer when not used.
     - When enabled, list buffer is not immediately freed when unused in case it's needed again
     - Default: 1
    */
    #define EVO_LIST_OPT_LAZYBUF 1
#endif
#if !defined(EVO_LIST_OPT_REFTERM)
    /** List: Enable optimization when referencing terminated lists.
     - When enabled, each String variable uses a little more memory (4 bytes)
     - Can improve performance when list referencing external terminated list and using cstr()
     - Default: 1
    */
    #define EVO_LIST_OPT_REFTERM 0
#endif

// List Option Dependencies
#if !EVO_LIST_OPT_EXTREF
    #define EVO_LIST_OPT_LAZYBUF 0
    #define EVO_LIST_OPT_REFTERM 0
#endif

///////////////////////////////////////////////////////////////////////////////
//@}
#endif
