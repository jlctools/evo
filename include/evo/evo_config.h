// Evo C++ Library
/* Copyright (c) 2016 Justin Crowell
 This Source Code Form is subject to the terms of the Mozilla Public
 License, v. 2.0. If a copy of the MPL was not distributed with this
 file, You can obtain one at http://mozilla.org/MPL/2.0/.
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
#define EVO_VERSION 0.3

/** Evo version patch number. */
#define EVO_VERSION_PATCH 0

///////////////////////////////////////////////////////////////////////////////
// Global

// Exceptions
#ifndef EVO_EXCEPTIONS
    /** Whether to throw exceptions on error by default.
     - Default: 1
     - Disabling exceptions can help performance when critical, however error codes should then be checked directly
    */
    #define EVO_EXCEPTIONS 1
#endif

///////////////////////////////////////////////////////////////////////////////
// Containers

#ifndef EVO_STD_STRING
    /** Whether to support std::string compatibility in Evo string classes (String, SubString).
     - Default: 0 (auto-detect, enable if std::string included)
     - By default this will try to auto-detect whether STL strings have been included already, and if so enable std::string support
     - Set to 1 to explicitly enable (and auto-include std::string), -1 to explicitly disable and skip auto-detect
     - Set this BEFORE including any Evo headers
     - When enabled, any function that takes "const ListBase<char>&" will automatically support "const std::string&" and "const std::string*"
    */
    #define EVO_STD_STRING 0
#endif

// TODO -- Work-In-Progress: Do not change from defaults at this time

// Size Type
#ifndef EVO_SIZE_TYPE
    /** Sets default size type for Evo containers.
     - Default: uint32
     - Always unsigned
     - This becomes type: SizeT
    */
    #define EVO_SIZE_TYPE uint32
#endif

// String Size Type
#ifndef EVO_STR_SIZE_TYPE
    /** Sets default string size type for Evo string containers.
     - Default: uint32
     - Always unsigned
     - This becomes type: StrSizeT
    */
    #define EVO_STR_SIZE_TYPE uint32
#endif

// Allocators
#ifndef EVO_ALLOCATORS
    /** Enable container allocators -- NOT YET SUPPORTED.
     - Default: 0
    */
    #define EVO_ALLOCATORS 0
#endif

// List
#ifndef EVO_LIST_REALLOC
    /** List: Enable using realloc to resize buffer.
     - This could improve performance in some systems
     - Default: 1
    */
    #define EVO_LIST_REALLOC 1
#endif
#ifndef EVO_LIST_OPT_EXTREF
    /** List: Enable optimization for referencing external lists rather than copying.
     - When enabled, each String variable uses a little more memory
     - Disabling this also disables EVO_LIST_OPT_LAZYBUF and EVO_LIST_OPT_EXTREF (?)
     - Default: 1
    */
    #define EVO_LIST_OPT_EXTREF 1
#endif
#ifndef EVO_LIST_OPT_LAZYREM
    /** List: Enable lazy removal optimization when removing items.
     - When enabled, List::remove() will trim begining/end items, to be actually removed later
     - Can improve performance when beginning and/or end items are added and removed a lot
     - Default: 0
    */
    #define EVO_LIST_OPT_LAZYREM 0 // TODO: 1
#endif
#ifndef EVO_LIST_OPT_LAZYBUF
    /** List: Enable lazy buffer optimization for keeping buffer when not used.
     - When enabled, list buffer is not immediately freed when unused in case it's needed again
     - Default: 1
    */
    #define EVO_LIST_OPT_LAZYBUF 1
#endif
#ifndef EVO_LIST_OPT_REFTERM
    /** List: Enable optimization when referencing terminated lists.
     - When enabled, each String variable uses a little more memory (4 bytes)
     - Can improve performance when list referencing external terminated list and using cstr()
     - Default: 1
    */
    #define EVO_LIST_OPT_REFTERM 0 // TODO: 1
#endif

// List Option Dependencies
#if !EVO_LIST_OPT_EXTREF
    #define EVO_LIST_OPT_LAZYBUF 0
    #define EVO_LIST_OPT_REFTERM 0
#endif

// Map
#ifndef EVO_MAP_VIRTUAL
    /** Map: Enable virtual members in base Map -- NOT YET SUPPORTED.
     - This allows Map to be used as a polymorphic interface -- in other words a MapList can be passed generically as a Map
     - However using this feature adds small overhead via "virtual" methods -- this overhead can be avoided by passing concrete types such as MapList rather than base type Map
     - Disabling this will force code to avoid the above mentioned overhead, giving compiler errors when Map is used too generically
     - Default: 1
    */
    #define EVO_MAP_VIRTUAL 1 // TODO: tests build/pass when 0
#endif

///////////////////////////////////////////////////////////////////////////////
//@}
#endif
