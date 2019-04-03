// Evo C++ Library
/* Copyright 2019 Justin Crowell
Distributed under the BSD 2-Clause License -- see included file LICENSE.txt for details.
*/
///////////////////////////////////////////////////////////////////////////////
/** \file sysio_dir.h Evo system I/O implementation for directories */
#pragma once
#ifndef INCL_evo_impl_sysio_dir_h
#define INCL_evo_impl_sysio_dir_h

#include "sys.h"
#include <sys/types.h>

#if defined(_WIN32)
    // Windows
    #include <io.h>
#else
    // Linux/Unix
    #include <dirent.h>
    #if !defined(EVO_USE_READDIR_R)
        #if (defined(__GNUC__) && __GNUC__ >= 6) || (defined(__clang_major__) && __clang_major__ >= 4)
            #define EVO_USE_READDIR_R 0     // Use thread-safe readdir()
        #else
            #define EVO_USE_READDIR_R 1     // Use thread-safe variation readdir_r() up until deprecated by glibc 2.24+
        #endif
    #endif
#endif

namespace evo {
/** \addtogroup EvoIO */
//@{

///////////////////////////////////////////////////////////////////////////////

/** System directory reader (used internally).
 - This is an internal low-level interface with public members, use Directory instead
 - Members are public for quick access and simple low-level interface
 - Implementation is OS specific and handle is public
*/
struct SysDir {
#if defined(_WIN32)
    // Windows
    typedef intptr_t Handle;

    struct _finddata_t context;
    char*              filepath;
    const char*        firstfile;

    SysDir() {
        handle    = -1;
        filepath  = NULL;
        firstfile = NULL;
    }

    ~SysDir()
        { close(); }

    Error open(const char* path) {
        close();
        if (path != NULL) { // NULL used by seek() to re-open same filepath
            // Append wildcard to path, save in case of seek()
            const size_t pathlen = strlen(path);
            filepath = (char*)::malloc(pathlen+3);
            memcpy(filepath, path, pathlen);
            memcpy(filepath+pathlen, "\\*", 3);
        }

        handle = _findfirst(filepath, &context);
        Error err;
        if (handle == -1) {
            switch (errno) {
                case ENOENT:  err = ENotFound; break;
                default:      err = EFail;     break;
            }
            firstfile = NULL;
        } else {
            err       = ENone;
            firstfile = context.name;
        }
        return err;
    }

    void close() {
        if (handle != -1) {
            _findclose(handle);
            handle = -1;
        }
        if (filepath != NULL) {
            ::free(filepath);
            filepath = NULL;
        }
        firstfile = NULL;
    }

    void seek() {
        if (handle != -1) {
            _findclose(handle);
            handle = -1;
        }
        firstfile = NULL;
        open(NULL);
    }

    template<class TStr>
    bool read(TStr& entry) {
        if (handle != -1) {
            if (firstfile != NULL) {
                entry.set(firstfile);
                firstfile = NULL;
                return true;
            } else {
                for (;;) {
                    if (_findnext(handle, &context) != 0) {
                        close();
                        break;
                    }
                    if (context.name[0] == '.' &&
                         (context.name[1] == '\0' ||
                           (context.name[1] == '.' && context.name[2] == '\0')
                         )
                       )
                        continue; // skip current/parent dir
                    entry.set(context.name);
                    return true;
                }
            }
        }
        entry.set();
        return false;
    }

#else
    // Linux/Unix
    typedef DIR* Handle;        ///< System directory handle

    /** Constructor. */
    SysDir() {
    #if EVO_USE_READDIR_R
        buffer = NULL;
    #endif
        handle = NULL;
    }

    /** Destructor. */
    ~SysDir()
        { close(); }

    /** Open directory for reading.
     \param  path  Directory path to open, must be terminated
     \return       ENone on success, error code on other error
    */
    Error open(const char* path) {
        close();
        handle = ::opendir(path);
        Error err;
        if (handle == NULL) {
            switch (errno) {
                case EACCES:  err = EAccess;   break;
                case ENOTDIR: // fallthrough
                case ENOENT:  err = ENotFound; break;
                default:      err = EFail;     break;
            }
        } else {
            err = ENone;
        #if EVO_USE_READDIR_R
            // See readdir_r() manpage for size calculation
            buffer = (char*)::malloc(offsetof(struct dirent, d_name) + pathconf(path, _PC_NAME_MAX) + 1);
        #endif
        }
        return err;
    }

    /** Close currently open directory, if any. */
    void close() {
    #if EVO_USE_READDIR_R
        if (buffer != NULL) {
            ::free(buffer);
            buffer = NULL;
        }
    #endif
        if (handle) {
            ::closedir(handle);
            handle = NULL;
        }
    }

    /** Seek to beginning of directory.
     - After this read() will return entries from the beginning
    */
    void seek()
        { if (handle) ::rewinddir(handle); }

    /** Read next directory entry.
     - Call seek() to move back to the first entry
     - \b Caution: Result string will reference internal memory that will be invalid on next read() or on close() (even with String type)
     .
     \tparam  TStr  %String type to store entry (SubString preferred), inferred from argument
     \param  entry  SubString to store the entry, set to null after last entry [out]
     \return        Whether entry read, false if no more entries
    */
    template<class TStr>
    bool read(TStr& entry) {
        if (handle) {
        #if EVO_USE_READDIR_R
            assert( buffer != NULL );
            struct dirent* result = NULL;
            if (::readdir_r(handle, (struct dirent*)buffer, &result) == 0 && result != NULL) {
                entry = result->d_name;
                return true;
            }
        #else
            struct dirent* result = ::readdir(handle);
            if (result != NULL) {
                entry = result->d_name;
                return true;
            }
        #endif
        }
        entry.set();
        return false;
    }

    /** Change current working directory to currently open directory.
     \return  Whether successful, false if directory not open or permission denied
    */
    bool chdir() {
        if (handle != NULL) {
            const int fd = ::dirfd(handle);
            return (fd >= 0 && ::fchdir(fd) == 0);
        }
        return false;
    }

private:
#if EVO_USE_READDIR_R
    char* buffer;
#endif

public:
#endif

    Handle handle;        ///< System directory handle
};

///////////////////////////////////////////////////////////////////////////////
//@}
}
#endif
