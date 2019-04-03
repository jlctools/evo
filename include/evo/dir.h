// Evo C++ Library
/* Copyright 2019 Justin Crowell
Distributed under the BSD 2-Clause License -- see included file LICENSE.txt for details.
*/
///////////////////////////////////////////////////////////////////////////////
/** \file dir.h Evo Directory helpers. */
#pragma once
#ifndef INCL_evo_dir_h
#define INCL_evo_dir_h

#include "impl/sysio_dir.h"
#include "filepath.h"

#if defined(_WIN32)
    #include "ustring.h"
#endif

// Namespace: evo
namespace evo {
/** \addtogroup EvoIO */
//@{

///////////////////////////////////////////////////////////////////////////////

/** Store current working directory in string.
 - \#include <evo/dir.h>
 - Windows: \b Caution: This is _not_ thread safe in Windows, and not safe in shared libraries
 - See also: get_abspath(), set_cwd()
 .
 \param  outpath  %String to store current working directory [out]
 \return          Reference to `outpath` with absolute path to current working directory, set to null if this somehow fails (unlikely)
*/
inline String& get_cwd(String& outpath) {
#if defined(_WIN32)
    outpath.set();
    DWORD sz = ::GetCurrentDirectory(0, NULL);
    if (sz > 0 && (sz = ::GetCurrentDirectory(sz, outpath.advBuffer(sz))) > 0)
        outpath.advSize(sz);
    else
        outpath = "C:\\"; // unknown so guess C drive root
#else
    const size_t DEFAULT_BUF_SIZE = 512;
    for (size_t sz = DEFAULT_BUF_SIZE;; sz *= 2) {
        outpath.set().reserve(sz);
        const char* p = ::getcwd(outpath.advBuffer(sz), sz);
        if (p != NULL) {
            assert( p == outpath.data() );
            outpath.advSize(strlen(p));
            break;
        } else if (errno != ERANGE) {
            outpath = "/"; // unknown so use root
            break;
        }
    }
#endif
    return outpath;
}

/** %Set current working directory.
 - \#include <evo/dir.h>
 - This is generally not thread safe since it affects the whole process
 - Windows: \b Caution: This is _not_ thread safe in Windows, and not safe in shared libraries
 - See also: get_cwd()
 .
 \param  path   Directory path to set
 \param  excep  Whether to enable exceptions on error, default set by Evo config: EVO_EXCEPTIONS
 \return        Error code, ENone for success, throws exception on error if `excep=true`
*/
inline Error set_cwd(const char* path, bool excep=EVO_EXCEPTIONS) {
    Error err = ENone;
#if defined(_WIN32)
    if (::SetCurrentDirectory(path) != 0)
        return ENone;
    err = EFail;
#else
    if (::chdir(path) == 0)
        return ENone;
    const int code = errno;
    switch (code) {
        case EACCES:       err = EAccess;    break;
        case EFAULT:       err = EPtr;       break;
        case ELOOP:        // fallthrough
        case ENAMETOOLONG: err = ESize;      break;
        case ENOENT:       err = ENotFound;  break;
        case ENOTDIR:      err = ENotDir;    break;
        default:           err = EFail;      break;
    }
#endif
    EVO_THROW_ERR_CHECK(evo::Exception, "Error setting working directory", err, excep);
    return err;
}

/** Get absolute path for given input path.
 - \#include <evo/dir.h>
 - This calls FilePath::abs() (with `strict=false`) to see if input `path` is an absolute path, if so this is the output path
 - If input path is relative, this joins it with the current working directory to make it an absolute path
 - This calls FilePath::normalize() to normalize the result absolute path
   - If you want to contain the input path to the current directory for security reasons, call FilePath::normalize() on it first
 - Windows: \b Caution: This is _not_ thread safe in Windows, and not safe in shared libraries
 - See also: get_cwd()
 .
 \param  outpath  Stores output absolute path [out]
 \param  path     Path to get absolute path for
 \return          Reference to `outpath`, set to null if `path` is null
*/
inline String& get_abspath(String& outpath, SubString& path) {
    if (path.null()) {
        return outpath.set();
    } else if (FilePath::abs(path, false)) {
        outpath = path;
    } else {
        get_cwd(outpath);
        FilePath::join(outpath, path);
    }
    return FilePath::normalize(outpath, outpath);
}

///////////////////////////////////////////////////////////////////////////////

/** Directory open exception for errors opening a directory for reading entries, see Exception. */
class ExceptionDirOpen : public Exception
    { EVO_CREATE_EXCEPTION_IMPL(ExceptionDirOpen, Exception) };

///////////////////////////////////////////////////////////////////////////////

/** %Directory reader.
 - Use this open a directory and step through the files
 .

\par Example

\code
#include <evo/dir.h>
#include <evo/io.h>
using namespace evo;

int main() {
    SubString filename;
    try {
        Directory dir(".");
        while (dir.read(filename))
            con().out << filename << NL;
    } EVO_CATCH(return 1);

    return 0;
}
\endcode
*/
class Directory : public SafeBool<Directory> {
public:
    /** Constructor.
     \param  excep  Whether to enable exceptions on error, default set by Evo config: EVO_EXCEPTIONS
    */
    Directory(bool excep=EVO_EXCEPTIONS) : excep_(excep) {
    }

    /** Constructor to open directory.
     - This calls open()
     .
     \param  path   Directory path to open
     \param  excep  Whether to enable exceptions on error, default set by Evo config: EVO_EXCEPTIONS
    */
    Directory(const char* path, bool excep=EVO_EXCEPTIONS) : excep_(excep) {
        open(path);
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

    /** Get whether directory is open.
     \return  Whether open
    */
    bool isopen() const {
        return (dir_.handle != NULL);
    }

    /** Open directory for reading entries.
     - Current directory is closed first
     - Throws ExceptionStreamOpen on error, if exceptions enabled
     .
     \param  path  Directory path to open
     \return       Whether successful, false on error -- call error() for error code
    */
    bool open(const char* path) {
        error_ = dir_.open(path);
        EVO_THROW_ERR_CHECK(evo::ExceptionDirOpen, "Directory::open() failed", error_, (excep_ && error_ != ENone));
        return error_ == ENone;
    }

    /** Close currently open directory, if any. */
    void close() {
        dir_.close();
        error_ = ENone;
    }

    /** Seek to beginning of directory.
     - After this read() will return entries from the beginning
    */
    void seek() {
        dir_.seek();
    }

    /** Read next directory entry.
     - Call seek() to move back to the first entry
     - \b Caution: Result string will reference internal memory that will be invalid on next read() or on close()
     .
     \param  entry  SubString to store the entry, set to null after last entry [out]
     \return        Whether entry read, false if no more entries
    */
    bool read(SubString& entry) {
        return dir_.read(entry);
    }

#if !defined(_WIN32) || defined(DOXYGEN)
    /** Change current working directory to currently open directory (Linux/Unix only).
     \return  Whether successful, false if directory not open or permission denied
    */
    bool chdir() {
        return dir_.chdir();
    }
#endif

private:
    // Disable copying
    Directory(const Directory&) EVO_ONCPP11(= delete);
    Directory& operator=(const Directory&) EVO_ONCPP11(= delete);

    SysDir dir_;        ///< System directory reader
    Error error_;       ///< Last error code
    bool  excep_;       ///< Whether to throw exceptions
};

///////////////////////////////////////////////////////////////////////////////
//@}
}
#endif
