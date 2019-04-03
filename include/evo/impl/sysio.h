// Evo C++ Library
/* Copyright 2019 Justin Crowell
Distributed under the BSD 2-Clause License -- see included file LICENSE.txt for details.
*/
///////////////////////////////////////////////////////////////////////////////
/** \file sysio.h Evo system I/O implementation */
#pragma once
#ifndef INCL_evo_impl_sysio_h
#define INCL_evo_impl_sysio_h

#include "rawbuffer.h"
#include "../string.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#if defined(_WIN32)
    // Windows
    #include <io.h>
#endif

// Disable certain MSVC warnings for this file
#if defined(_MSC_VER)
    #pragma warning(push)
    #pragma warning(disable:4100)
#endif

namespace evo {
/** \addtogroup EvoIO */
//@{

///////////////////////////////////////////////////////////////////////////////

#if !defined(_WIN32)
    // Linux/Unix
    struct SysLinuxIo {
        static bool read_wait(Error& err, int handle, ulong timeout_ms, bool autoresume) {
            assert(handle >= 0);
            assert(timeout_ms > 0);

            fd_set read_set;
            FD_ZERO(&read_set);
            FD_SET(handle, &read_set);

            struct timeval timeout;
            SysLinux::set_timeval_ms(timeout, timeout_ms);
            for (;;) {
                int waitresult = ::select(handle+1, &read_set, NULL, NULL, &timeout);
                if (waitresult < 0) {
                    switch (errno) {
                        case EINTR:
                            if (autoresume) {
                                #if !defined(__linux__) // Linux select() updates timeout, others need to set it again
                                    SysLinux::set_timeval_ms(timeout, timeout_ms);
                                #endif
                                continue;
                            }
                            err = ESignal;
                            break;
                        case EBADF: err = EClosed;  break;
                        default:    err = EUnknown; break;
                    }
                    return false;
                } else if (waitresult == 0) {
                    err = ETimeout;
                    return false;
                }
                break;
            }
            return true;
        }

        static ulong read(Error& err, int handle, void* buf, ulong size, ulong timeout_ms, bool autoresume) {
            if (handle == -1) {
                errno = EBADF;
                err   = EClosed;
                return 0;
            }
            if (size > SSIZE_MAX)
                size = SSIZE_MAX;
            ssize_t result;
            for (;;) {
                if (timeout_ms > 0 && !SysLinuxIo::read_wait(err, handle, timeout_ms, autoresume))
                    return 0;
                result = ::read(handle, buf, size);
                if (result < 0) {
                    switch (errno) {
                        case EINTR:
                            if (autoresume)
                                continue;
                            err = ESignal; break;
                        case ENOSPC:      err = ESpace;  break;
                        case EFBIG:       err = ESize;   break;
                        case EFAULT:      err = EPtr;    break;
                        case EBADF:       err = EClosed; break;
                    #if EAGAIN != EWOULDBLOCK
                        case EAGAIN:      // fallthrough
                    #endif
                        case EWOULDBLOCK: err = ENonBlock; break;
                        default:          err = ERead;     break;
                    }
                    return 0;
                }
                break;
            }
            err = ENone;
            return result;
        }

        static bool write_wait(Error& err, int handle, ulong timeout_ms, bool autoresume) {
            fd_set write_set;
            FD_ZERO(&write_set);
            FD_SET(handle, &write_set);

            struct timeval timeout;
            SysLinux::set_timeval_ms(timeout, timeout_ms);
            for (;;) {
                int waitresult = ::select(handle+1, NULL, &write_set, NULL, &timeout);
                if (waitresult < 0) {
                    switch (errno) {
                        case EINTR:
                            if (autoresume) {
                                #if !defined(__linux__) // Linux select() updates timeout, others need to set it again
                                    SysLinux::set_timeval_ms(timeout, timeout_ms);
                                #endif
                                continue;
                            }
                            err = ESignal;
                            break;
                        case EBADF: err = EClosed;  break;
                        default:    err = EUnknown; break;
                    }
                    return false;
                } else if (waitresult == 0) {
                    err = ETimeout;
                    return false;
                }
                break;
            }
            return true;
        }

        static ulong write(Error& err, int handle, const void* buf, ulong size, ulong timeout_ms, bool autoresume) {
            if (handle == -1) {
                errno = EBADF;
                err   = EClosed;
                return 0;
            }
            if (size > SSIZE_MAX)
                size = SSIZE_MAX;
            ssize_t result;
            for (;;) {
                if (timeout_ms > 0 && !SysLinuxIo::write_wait(err, handle, timeout_ms, autoresume))
                    return 0;
                result = ::write(handle, buf, size);
                if (result == 0) {
                    err = EFail;
                    return 0;
                } else if (result < 0) {
                    switch (errno) {
                        case EINTR:
                            if (autoresume)
                                continue;
                            err = ESignal; break;
                        case ENOSPC:      err = ESpace;  break;
                        case EFBIG:       err = ESize;   break;
                        case EFAULT:      err = EPtr;    break;
                        case EBADF:       err = EClosed; break;
                    #if EAGAIN != EWOULDBLOCK
                        case EAGAIN:      // fallthrough
                    #endif
                        case EWOULDBLOCK: err = ENonBlock; break;
                        default:          err = EWrite;  break;
                    }
                    return 0;
                }
                break;
            }
            err = ENone;
            return result;
        }
    };
#endif

///////////////////////////////////////////////////////////////////////////////

/** %Open mode for files and streams.
 - See File::open()
*/
enum Open {
    oREAD            = O_RDONLY,                                ///< Read only
    oREAD_WRITE      = O_RDWR,                                  ///< Read and write
    oREAD_WRITE_NEW  = O_RDWR | O_CREAT | O_TRUNC,              ///< Read and write, create/replace
    oREAD_APPEND     = O_RDWR | O_APPEND,                       ///< Read and write/append
    oREAD_APPEND_NEW = O_RDWR | O_APPEND | O_CREAT | O_TRUNC,   ///< Read and write/append, create/replace
    oWRITE           = O_WRONLY,                                ///< Write only
    oWRITE_NEW       = O_WRONLY | O_CREAT | O_TRUNC,            ///< Write only, create/replace
    oAPPEND          = O_WRONLY | O_CREAT | O_APPEND,           ///< Write/append only, created if needed
    oAPPEND_NEW      = O_WRONLY | O_APPEND | O_CREAT | O_TRUNC  ///< Write/append only, create/replace
};

/** %Seek starting position. */
enum Seek {
    sBegin   = SEEK_SET,        ///< Seek from beginning
    sCurrent = SEEK_CUR,        ///< Seek from current position
    sEnd     = SEEK_END         ///< Seek to end
};

/** Check whether open mode is readable.
 \param  open  Open mode
 \return       Whether mode is readable
*/
inline bool open_readable(Open open)
    { return (open & O_RDWR || !(open & (O_WRONLY | O_APPEND))); }  // O_RDONLY is 0 so check for absence of write flags

/** Check whether open mode is writable.
 \param  open  Open mode
 \return       Whether mode is writable
*/
inline bool open_writable(Open open)
    { return ((open & (O_RDWR | O_WRONLY | O_APPEND)) != 0); }

///////////////////////////////////////////////////////////////////////////////

/** I/O device base class for streams.
 - This interface is used to define an I/O device concept for stream I/O
   - This doesn't need virtual methods because the derived class is used as a template parameter to a Stream class
 - Do not throw exceptions here, use Error code
*/
class IoDevice {
public:
    typedef ExceptionStreamIn  ExceptionInT;    ///< Input exception type for device (may be overridden by derived)
    typedef ExceptionStreamOut ExceptionOutT;   ///< Output exception type for device (may be overridden by derived)

    /** Close stream. */
    void close()
        { }

    /** Read input data from device.
     - On success this reads at least 1 byte and may read less than requested size
     - If not open/readable, returns end-of-file (0)
     .
     \param  err         Stores ENone on success, error code on error [out]
     \param  buf         Buffer to store data read
     \param  size        Size to read in bytes
     \param  timeout_ms  Read timeout in milliseconds, 0 for none (don't timeout)
     \return             Size actually read in bytes, 0 on end-of-file or error
    */
    ulong read(Error& err, void* buf, ulong size, ulong timeout_ms=0)
        { err = ENone; return 0; }

    /** Write output data to device.
     - On success at least 1 byte is written but may be less than requested size
     - If not open/writable, returns ENotImpl error
     .
     \param  err         Stores ENone on success, error code on error [out]
     \param  buf         Buffer to write from
     \param  size        Size to write in bytes
     \param  timeout_ms  Write timeout in milliseconds, 0 for none (don't timeout)
     \return             Size actually written in bytes, 0 on error (check err)
    */
    ulong write(Error& err, const void* buf, ulong size, ulong timeout_ms=0)
        { err = ENotImpl; return 0; }
};

///////////////////////////////////////////////////////////////////////////////

/** File I/O device (used internally).
 - This is an internal low-level interface with public members, use File instead
 - Members are public for quick access and simple low-level interface
 - This does not do any read/write buffering
 - Implementation is OS specific
*/
struct IoFile : public IoDevice {
    static const bool STREAM_SEEKABLE = true;   ///< File streams are seekable with Stream

    typedef ExceptionFileIn  ExceptionInT;      ///< Input exception type for file stream
    typedef ExceptionFileOut ExceptionOutT;     ///< Output exception type for file stream

    typedef int Handle;        ///< System file handle

    /** Invalid handle value. */
    static const Handle INVALID = -1;

    /** Destructor. Calls close(). */
    ~IoFile()
        { close(); }

    /** Get whether file is open.
     \return  Whether file is open
    */
    bool isopen() const
        { return (handle != INVALID); }

    /** Detach and return file handle.
     \return  File handle
    */
    Handle detach()
        { Handle result = handle; handle = INVALID; return result; }

#if defined(_WIN32)
    // Windows

    static const int DEFPERM  = _S_IREAD | _S_IWRITE;
    static const int READONLY = _S_IREAD;
    static const int USER_RD  = _S_IREAD;
    static const int USER_RW  = _S_IREAD | _S_IWRITE;

    IoFile()
        { handle = INVALID; }

    Error open(const char* path, Open mode, int perm=DEFPERM) {
        close();
        handle = ::_open(path, mode | _O_BINARY, perm);
        Error err;
        if (handle >= 0) {
            err = ENone;
        } else {
            switch (errno) {
                case EACCES:       err = EAccess;      break;
                case EEXIST:       err = EExist;       break;
                case ENOENT:       err = ENotFound;    break;
                case EMFILE:       err = EOutOfBounds; break;
                default:           err = EFail;        break;
            }
            handle = INVALID;
        }
        return err;
    }

    Error open_dup(Handle other, Handle target=INVALID) {
        close();
        if (target == INVALID) {
            const int result = ::_dup(other);
            if (result >= 0) {
                handle = result;
                return ENone;
            }
        } else if (::_dup2(other, target) == 0) {
            handle = target;
            return ENone;
        }
        Error err;
        switch (errno) {
            case EBADF: err = EClosed; break;
            default:    err = EFail;   break;
        }
        return err;
    }

    void close() {
        if (handle != INVALID) {
            ::_close(handle);
            handle = INVALID;
        }
    }

    ulongl pos(Error& err) {
        __int64 result = _lseeki64(handle, 0, SEEK_CUR);
        if (result < 0) {
            switch (errno) {
                case EBADF:     err = EClosed; break;
                default:        err = EFail;   break;
            }
            return 0;
        }
        err = ENone;
        return (ulongl)result;
    }

    ulongl seek(Error& err, ulongl offset, Seek start=sBegin) {
        if (offset > (ulongl)std::numeric_limits<__int64>::max())
            { err = ESize; return 0; }
        __int64 result = _lseeki64(handle, offset, (int)start);
        if (result < 0) {
            switch (errno) {
                case EBADF:     err = EClosed; break;
                default:        err = EFail;   break;
            }
            return 0;
        }
        err = ENone;
        return (ulongl)result;
    }

    ulong read(Error& err, void* buf, ulong size, ulong timeout_ms=0) {
        if (handle == INVALID) {
            err = EClosed;
            return 0;
        }
        if (size > std::numeric_limits<uint>::max())
            size = std::numeric_limits<uint>::max();
        int result;
        for (;;) {
            result = ::_read(handle, buf, size);
            if (result < 0) {
                switch (errno) {
                    case ENOSPC: err = ESpace;  break;
                    case EFBIG:  err = ESize;   break;
                    case EFAULT: err = EPtr;    break;
                    case EBADF:  err = EClosed; break;
                    default:     err = ERead;   break;
                }
                return 0;
            }
            break;
        }
        err = ENone;
        return (ulong)result;
    }

    ulong write(Error& err, const void* buf, ulong size, ulong timeout_ms=0) {
        if (handle == INVALID) {
            err = EClosed;
            return 0;
        }
        if (size > std::numeric_limits<uint>::max())
            size = std::numeric_limits<uint>::max();
        int result;
        for (;;) {
            result = ::_write(handle, buf, size);
            if (result == 0) {
                err = EFail;
                return 0;
            } else if (result < 0) {
                switch (errno) {
                    case ENOSPC: err = ESpace;  break;
                    case EINVAL: err = EPtr;    break;
                    case EBADF:  err = EClosed; break;
                    default:     err = EWrite;  break;
                }
                return 0;
            }
            break;
        }
        err = ENone;
        return result;
    }

#else
    // Linux/Unix

    /** Default permissions (used when creating new file). */
    static const int DEFPERM  = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH;
    /** Read-only permissions. */
    static const int READONLY = S_IRUSR | S_IRGRP | S_IROTH;
    /** User read-only permissions. */
    static const int USER_RD = S_IRUSR;
    /** User read-write permissions. */
    static const int USER_RW = S_IRUSR | S_IWUSR;

    /** Constructor. */
    IoFile()
        { handle = INVALID; autoresume = true; }

    /** Open file for access.
     \param  path  File path to use
     \param  mode  Access mode to use
     \param  perm  Permissions for new files [Linux/Unix]
     \return       ENone on success, error code on other error
    */
    Error open(const char* path, Open mode, int perm=DEFPERM) {
        close();
        handle = ::open(path, mode, perm);
        Error err;
        if (handle >= 0) {
            err = ENone;
        } else {
            switch (errno) {
                case EISDIR:       // fallthrough
                case EROFS:        // fallthrough
                case ETXTBSY:      // fallthrough
                case EACCES:       err = EAccess;      break;
                case EEXIST:       err = EExist;       break;
                case EFAULT:       err = EPtr;         break;
                case ELOOP:        // fallthrough
                case ENAMETOOLONG: err = ESize;        break;
                case ENOTDIR:      // fallthrough
                case ENOENT:       err = ENotFound;    break;
                case ENOSPC:       err = ESpace;       break;
                case EFBIG:        // fallthrough
                case EOVERFLOW:    err = EOutOfBounds; break;
                default:           err = EFail;        break;
            }
            handle = INVALID;
        }
        return err;
    }

    /** Open duplicate handle from source handle.
     \param  src     Source handle to duplicate
     \param  target  Target handle to open under (closed first, if open), INVALID to ignore and open a new handle
     \return         ENone on success, error code on other error
    */
    Error open_dup(Handle src, Handle target=INVALID) {
        close();
        if (target == INVALID) {
            const int result = ::dup(src);
            if (result >= 0) {
                handle = result;
                return ENone;
            }
        } else if (::dup2(src, target) == 0) {
            handle = target;
            return ENone;
        }
        Error err;
        switch (errno) {
            case EBADF: err = EClosed; break;
            default:    err = EFail;   break;
        }
        return err;
    }

    // Documented by parent
    void close() {
        if (handle != INVALID) {
            ::close(handle);
            handle = INVALID;
        }
    }

    /** Get current file position.
     \param  err  Stores ENone on success, error code on error [out]
     \return      File byte position, 0 for beginning or error
    */
    ulongl pos(Error& err) {
        off_t result = lseek(handle, 0, SEEK_CUR);
        if (result < 0) {
            switch (errno) {
                case EBADF:     err = EClosed; break;
                case EOVERFLOW: err = ESize;   break;
                case ESPIPE:    err = EInval;  break;
                default:        err = EFail;   break;
            }
            return 0;
        }
        err = ENone;
        return result;
    }

    /** Seek to file position.
     \param  err     Stores ENone on success, error code on error [out]
     \param  offset  Offset from start to seek to
     \param  start   Start position to seek from
     \return         New file position after seek, 0 for beginning or error
    */
    ulongl seek(Error& err, ulongl offset, Seek start=sBegin) {
        if (offset > (ulongl)std::numeric_limits<off_t>::max())
            { err = ESize; return 0; }
        off_t result = lseek(handle, offset, (int)start);
        if (result < 0) {
            switch (errno) {
                case EBADF:     err = EClosed; break;
                case EOVERFLOW: err = ESize;   break;
                case ESPIPE:    err = EInval;  break;
                default:        err = EFail;   break;
            }
            return 0;
        }
        err = ENone;
        return result;
    }

    // Documented by parent
    ulong read(Error& err, void* buf, ulong size, ulong timeout_ms=0)
        { return SysLinuxIo::read(err, handle, buf, size, timeout_ms, autoresume); }

    // Documented by parent
    ulong write(Error& err, const void* buf, ulong size, ulong timeout_ms=0)
        { return SysLinuxIo::write(err, handle, buf, size, timeout_ms, autoresume); }

    static Error mkdir(const char* path, int perm=0) {
        if (perm == 0)
            perm = DEFPERM;
        Error err;
        if (::mkdir(path, perm) == 0) {
            err = ENone;
        } else {
            switch (errno) {
                case EACCES:       // fallthrough
                case EPERM:        // fallthrough
                case EROFS:        err = EAccess; break;
                case EEXIST:       err = EExist;  break;
                case EFAULT:       err = EPtr;    break;
                case ELOOP:        // fallthrough
                case EMLINK:       // fallthrough
                case ENAMETOOLONG: err = ESize;   break;
                case ENOTDIR:      // fallthrough
                case ENOENT:       err = ENotFound; break;
                case ENOSPC:       err = ESpace;  break;
                default:           err = EFail;   break;
            }
        }
        return err;
    }

    static Error rmdir(const char* path) {
        Error err;
        if (::rmdir(path) == 0) {
            err = ENone;
        } else {
            switch (errno) {
                case EACCES:       // fallthrough
                case EPERM:        // fallthrough
                case EBUSY:        // fallthrough
                case EROFS:        err = EAccess; break;
                case ENOTEMPTY:    err = EExist;  break;
                case EFAULT:       err = EPtr;    break;
                case EINVAL:       err = EInval;  break;
                case ELOOP:        // fallthrough
                case ENAMETOOLONG: err = ESize;   break;
                case ENOTDIR:      // fallthrough
                case ENOENT:       err = ENotFound; break;
                case ENOSPC:       err = ESpace;  break;
                default:           err = EFail;   break;
            }
        }
        return err;
    }

    static Error rm(const char* path) {
        Error err;
        if (::unlink(path) == 0) {
            err = ENone;
        } else {
            switch (errno) {
                case EACCES:       // fallthrough
                case EPERM:        // fallthrough
                case EBUSY:        // fallthrough
                case EROFS:        err = EAccess; break;
                case EFAULT:       err = EPtr;    break;
                case ELOOP:        // fallthrough
                case ENAMETOOLONG: err = ESize;   break;
                case EISDIR:       // fallthrough
                case ENOTDIR:      // fallthrough
                case ENOENT:       err = ENotFound; break;
                default:           err = EFail;   break;
            }
        }
        return err;
    }

    bool   autoresume;    ///< Whether to auto-resume I/O operation after signal received [Linux/Unix]
#endif

    Handle handle;        ///< System handle/descriptor
};

///////////////////////////////////////////////////////////////////////////////

/** Buffered reader for IoDevice (used internally).
 - This is an internal low-level interface, see: File, Pipe, Console, Socket
 - Filtering is enabled by setting 'filters' -- note that the pointer should be owned (freed) by the IoDevice
 - Use readbuf.resize() to resize or disable buffer -- note that filters require buffering
 - Members are public for quick access and simple low-level interface
*/
struct IoReader {
    static const ulong DEFSIZE = 8192;      ///< Default buffer size (8KB, power of 2 and multiple of common filesystem block size 4KB)

    RawBuffer  readbuf;         ///< Primary read buffer -- filtering may involve additional buffers
    RawBuffer* curbuf;          ///< Pointer to current buffer, either primary buffer or from last filter applied
    ulong      curbuf_offset;   ///< Bytes read from curbuf, i.e. buffer start offset

    ulong       timeout_ms;     ///< Read timeout in milliseconds, 0 for none (don't timeout)
    const char* newline;        ///< Newline string to convert to when reading text -- do not modify
    uint        newlinesize;    ///< Size of newlines string to use, i.e. strlen(newline) -- do not modify
    char        rd_partnl;      ///< Used by readtext() in special case, holds end of converted newline that didn't fit in buf or 0
    char        rl_partnl;      ///< Used by readline() on partial newlines, holds next expected char for newline pair or 0

    /** Constructor to set new buffer size.
     \param  newsize  New buffer size, 0 for default
     \param  nl       Newline value for text reading to convert newlines to (defaults to NL_SYS), doesn't affect reading by line
    */
    IoReader(ulong newsize=0, Newline nl=NL_SYS) {
        if (newsize > 0)
            readbuf.resize(newsize);
        curbuf        = &readbuf;
        curbuf_offset = 0;
        timeout_ms  = 0;
        newline     = getnewline(nl);
        newlinesize = getnewlinesize(nl);
        rd_partnl   = 0;
        rl_partnl   = 0;
    }

    /** Initialize and open for input (reading).
     - This creates the read buffer, if needed
    */
    void open() {
        readbuf.used = curbuf_offset = 0;
        if (readbuf.size == 0)
            readbuf.resize(IoReader::DEFSIZE);
        //if (filters != NULL)
        //    filters->onread_open(readbuf);
    }

    /** Close input. */
    void close() {
        //if (filters != NULL)
        //    filters->onread_close();
    }

    /** Reset and fill buffer by reading from file.
     - This will first reset buffer moving data at pos to beginning (and set pos to 0), data before pos is lost
     - Next this will read from file and fill buffer until full, minsize reached, end-of-file is reached, or an error occurs
     - Data may still be in buffer after end-of-file is reached
     - The minsize parameter is useful when input is trickling in (slow network or user typing)
     .
     \tparam  T  IoDevice to read from

     \param  in       File to read from
     \param  minsize  Minimum size to fill (capped to buffer size if greater), 0 for whole buffer
     \return          ENone on success, EEnd if end-of-file reached, error code on other error
    */
    template<class T>
    Error fill(T& in, ulong minsize=0) {
        if (minsize == 0 || minsize > readbuf.size)
            minsize = readbuf.size;
        Error err;
        ulong readsize;
        {
            // No filters, read from stream
            readbuf.flush(curbuf_offset);
            while (readbuf.used < minsize) {
                readsize = in.read(err, readbuf.data+readbuf.used, readbuf.size-readbuf.used);
                if (err != ENone)
                    return err;
                if (readsize > 0)
                    readbuf.used += readsize;
                else
                    return EEnd;
            }
        }
        return ENone;
    }

    /** Read from file using buffer.
     - May return less than requested
     - This does a binary read -- no conversion on newlines
     .
     \tparam  T  IoDevice to read from

     \param  err      Stores ENone on success, error code on error [out]
     \param  in       File to read from
     \param  buf      Buffer to store data read
     \param  bufsize  Size in bytes to read from file (must be positive)
     \return          Bytes read, 0 if end-of-file or error (check err)
    */
    template<class T>
    ulong readbin(Error& err, T& in, void* buf, ulong bufsize) {
        assert( bufsize > 0 );
        assert( curbuf->used <= curbuf->size );
        assert( curbuf_offset <= curbuf->used );

        ulong readtotal = 0, usedleft = curbuf->used - curbuf_offset;
        if (bufsize <= usedleft) {
            // Read from buffer, enough is there
            memcpy(buf, curbuf->data+curbuf_offset, bufsize);
            readtotal = bufsize;
            curbuf_offset += bufsize;
        } else {
            // Read from buffer first
            if (usedleft > 0) {
                memcpy(buf, curbuf->data+curbuf_offset, usedleft);
                (char*&)buf += usedleft;
                bufsize     -= usedleft;
                readtotal   = usedleft;
            }

            // Read data larger than buffer directly
            ulong readsize;
            if (bufsize >= curbuf->size) {
                readsize = in.read(err, buf, bufsize);
                if (err != ENone)
                    return 0;
                (char*&)buf += readsize;
                bufsize     -= readsize;
                readtotal   += readsize;
            }

            // Read more if needed
            if (bufsize > 0) {
                // Fill buffer for next read
                readsize = in.read(err, curbuf->data, curbuf->size);
                if (err != ENone)
                    return 0;
                curbuf->used = readsize;

                // Read more from buffer if needed
                if (curbuf->used > 0) {
                    curbuf_offset = (bufsize > curbuf->used ? curbuf->used : bufsize);
                    memcpy(buf, curbuf->data, curbuf_offset);
                    readtotal += curbuf_offset;
                } else
                    curbuf_offset = 0;
            }
        }
        err = ENone;
        return readtotal;
    }

    /** Read from file using buffer.
     - May return less than requested
     - This does a text read, converting newlines per newline member
     - This recognizes '\\n', '\\r', or either combination of the two as a newline
     - Note: This will try to avoid stopping in the middle of a newline pair by reading 1 less byte, if possible
       - If forced to break up a newline pair due to bufsize=1 (not recommended), the remaining newline byte is saved until the next call to readtext()
       - However, calling readline() after readtext() in this special case will cause readline() to return an ELoss error
       - If this happens the solution is to read the remaining newline byte with bufsize=1, to detect this case:
         - readline() returns ELoss
         - after readtext(): rd_partnl != 0
         .
       - This is only a problem if you mix readtext() (with bufsize=1) and readline() calls under these conditions
       .
     .
     \tparam  T  IoDevice to read from

     \param  err      Stores ENone on success, error code on error [out]
     \param  in       File to read from
     \param  buf      Buffer to store data read
     \param  bufsize  Size in bytes to read from file (must be positive)
     \return          Bytes read, 0 if end-of-file or error (check err)
    */
    template<class T>
    ulong readtext(Error& err, T& in, char* buf, ulong bufsize) {
        assert( curbuf->size >= 2 );
        assert( curbuf->used <= curbuf->size );
        assert( curbuf_offset <= curbuf->used );
        assert( bufsize > 0 );
        ulong bytesread = 0, len, len2;

        if (rd_partnl != 0) {
            // Special case, newline pair broken up from last readtext(), complete newline pair using saved char
            buf[0] = rd_partnl;
            ++bytesread;
            rd_partnl = 0;
        }

        for (;;) {
            if (curbuf_offset+1 >= curbuf->used) {
                // Fill buffer -- need at least 2 bytes to handle newlines
                err = fill(in);
                if (err != ENone && err != EEnd)
                    return 0;
                if (curbuf->used == 0) {
                    err = (bytesread > 0 ? ENone : EEnd);
                    return (ulong)bytesread;
                }
            }
            if (rl_partnl != 0) {
                // Ignore end of partial newline from readline()
                if (curbuf->data[curbuf_offset] == rl_partnl)
                    ++curbuf_offset;
                rl_partnl = 0;
            }

            len  = curbuf->used - curbuf_offset;
            len2 = bufsize - bytesread;
            const char* start = curbuf->data + curbuf_offset;
            const char* end1  = start + (len2 < len ? len2 : len);
            const char* end2  = start + len;
            const char* p     = start;
            uint read_nl_size = 0;
            char checknext    = 0;

            // Find newlines, copy up to newline then write newline, repeat until end
            while (p < end1) {
                if (*p == '\n')
                    checknext = '\r';
                else if (*p == '\r')
                    checknext = '\n';
                else
                    { ++p; continue; }

                if (p+1 >= end2) {
                    break; // can't checknext, need to refill buffer
                } else if (p[1] == checknext) {
                    // Found newline pair
                    if (newline[1] != '\0' && p+1 >= end1) {
                        read_nl_size = 0; // no room for full newline, stop here
                    } else {
                        if (checknext == newline[1])
                            { p += 2; continue; } // no need to convert, continue
                        read_nl_size = 2;
                    }
                } else {
                    // Found newline char
                    if (newline[1] == '\0' && *p == newline[0])
                        { ++p; continue; } // no need to convert, continue
                    read_nl_size = 1;
                }

                // Copy to newline
                if (p > start) {
                    memcpy(buf+bytesread, start, (len=(ulong)(p-start)));
                    bytesread += len;
                    curbuf_offset += (ulong)len;
                }
                if (read_nl_size == 0 || bytesread+newlinesize > bufsize) {
                    if (bufsize == 1) {
                        // Special case, buffer too small so have to read first part of newline pair now, save remaining part for next call
                        buf[0]    = newline[0];
                        rd_partnl = newline[1];
                        bytesread = 1;
                        curbuf_offset += read_nl_size;
                    }
                    goto done;  // no room for full newline, stop here
                }

                curbuf_offset += read_nl_size;
                memcpy(buf+bytesread, newline, newlinesize);
                bytesread += newlinesize;
                start = (p += read_nl_size);
            }

            // Copy remaining until end
            if (p > start) {
                memcpy(buf+bytesread, start, (len=(ulong)(p-start)));
                bytesread += len;
                curbuf_offset += (ulong)len;
            }

            // Stop here if desired size read
            if (bytesread >= bufsize)
                break;
        }

    done:
        err = ENone;
        assert( bytesread > 0 );
        return (ulong)bytesread;
    }

    /** Read a line from file using buffer.
     - This will read and return the next line from file as a string, not including the newline
     - This recognizes '\\n', '\\r', or either combination of the two as a newline
     .
     \tparam  T  IoDevice to read from

     \param  str     String to store line (cleared first) [out]
     \param  in      File to read from
     \param  maxlen  Maximum line length, 0 for no limit
     \return         ENone on success, EEnd if no more lines (end-of-file), EOutOfBounds if line exceeds maxlen, or error code on error
    */
    template<class T>
    Error readline(String& str, T& in, ulong maxlen=0) {
        assert( curbuf->used <= curbuf->size );
        assert( curbuf_offset <= curbuf->used );
        if (rd_partnl != 0)
            { str.clear(); return ELoss; }

        Error err;
        ulong len = 0;
        char  checknext;
        str.clear();
        for (;;) {
            if (curbuf_offset >= curbuf->used) {
                // Fill buffer
                err = fill(in, 1);
                if (err != ENone && err != EEnd)
                    return err;
                if (curbuf->used == 0)
                    return (len > 0 ? ENone : EEnd);
            }
            if (rl_partnl != 0) {
                // Previous line ended with potential partial newline
                if (curbuf->data[curbuf_offset] == rl_partnl) {
                    // Skip rest of newline, continue on
                    if (++curbuf_offset >= curbuf->used) {
                        rl_partnl = 0;
                        continue;
                    }
                }
                rl_partnl = 0;
            }

            const char* start = curbuf->data + curbuf_offset;
            const char* end   = curbuf->data + curbuf->used;
            for (const char* p=start; p < end; ++p) {
                if (*p == '\n')
                    checknext = '\r';
                else if (*p == '\r')
                    checknext = '\n';
                else
                    continue;

                // Found newline
                len = (ulong)(p - start);
                if (maxlen > 0 && str.size()+len > maxlen)
                    return EOutOfBounds;
                str.add(start, len);
                curbuf_offset += len + 1;

                // Check next byte for remaining newline, if not available set rl_partnl to check later
                if (++p < end) {
                    if (curbuf->data[curbuf_offset] == checknext)
                        ++curbuf_offset; // skip rest of newline
                } else
                    rl_partnl = checknext; // save partial to check later
                return ENone;
            }

            // Newline not found
            len = (ulong)(end - start);
            if (maxlen > 0 && str.size()+len > maxlen)
                break;
            str.add(start, len);
            curbuf_offset += len;
        }
        return EOutOfBounds;
    }
};

///////////////////////////////////////////////////////////////////////////////

/** Buffered writer for IoDevice (used internally).
 - This is an internal low-level interface, see: File, Pipe, Console, Socket
 - Filtering is enabled by setting 'filters' -- note that the pointer should be owned (freed) by the IoDevice
 - Use writebuf.resize() to resize or disable buffer -- note that filters require buffering
 - Members are public for quick access and simple low-level interface
*/
struct IoWriter : public RawBuffer {
    static const ulong DEFSIZE = 16384;    ///< Default buffer size (16KB, power of 2 and multiple of common filesystem block size 4KB)

    ulong       timeout_ms;     ///< Write timeout in milliseconds, 0 for none (no timeout)
    const char* newline;        ///< Default newline string for formatting -- do not modify
    uint        newlinesize;    ///< Size of default newline string to use, i.e. strlen(newline) -- do not modify
    bool        flushlines;     ///< Whether to flush after each line (aka line buffering) -- only applies to text writes, i.e. writetext()
    char        partnl;         ///< Used internally for handling partial newlines between writetext() calls

    /** Constructor to set new buffer size and newline value.
     \param  newsize  New buffer size
     \param  nl       Default newline value for formatting (defaults to NL_SYS)
    */
    IoWriter(ulong newsize=0, Newline nl=NL_SYS) {
        if (newsize > 0)
            resize(newsize);
        timeout_ms  = 0;
        newline     = getnewline(nl);
        newlinesize = getnewlinesize(nl);
        partnl      = 0;
        flushlines  = false;
    }

    /** Initialize and open for output (writing).
     - This creates the write buffer, if needed
     .
     \param  flushlines_val  Whether to flush on each newline
    */
    void open(bool flushlines_val=false) {
        used = 0;
        if (size == 0)
            resize(IoWriter::DEFSIZE);
        flushlines = flushlines_val;
    }

    /** Close output. */
    void close()
        { }

    /** Flush buffer by writing to device.
     - This will clear out buffered data
     .
     \tparam  T  IoDevice to flush and write to

     \param  out  Device to write to
     \return      ENone on success, error code on error
    */
    template<class T>
    Error flush(T& out) {
        assert( used <= size );
        Error err;
        ulong flushpos = 0, writesize;
        while (flushpos < used) {
            writesize = out.write(err, data+flushpos, used-flushpos);
            if (err != ENone)
                return err;
            assert( writesize > 0 );
            flushpos += writesize;
        }
        used = 0;
        return ENone;
    }

    /** Write data to device using buffer.
     - This will flush the buffer when full
     - Note: If writetext() was previously called then set partnl=0 before calling writetext() again
     .
     \tparam  T  IoDevice to write to

     \param  err      Stores ENone on success, error code on error [out]
     \param  out      Device to write to
     \param  buf      Data buffer to write from
     \param  bufsize  Data size to write in bytes
     \return          Size actually written, 0 on error, see err
    */
    template<class T>
    ulong writebin(Error& err, T& out, const void* buf, ulong bufsize) {
        assert( used <= size );
        if (bufsize > 0) {
            ulong writesize = size - used;
            if (bufsize < writesize) {
                // Copy to buffer, partial fill
                memcpy(data+used, buf, bufsize);
                used += bufsize;
            } else {
                // Fill buffer and flush, if enabled
                ulong bufleft = bufsize;
                if (size > 0) {
                    if (writesize > 0) {
                        memcpy(data+used, buf, writesize);
                        used        += writesize;
                        (char*&)buf += writesize;
                        bufleft     -= writesize;
                    }
                    if ( (err=flush(out)) != ENone )
                        return 0;
                }

                if (bufleft >= size) {
                    // Remaining data larger than buffer, write directly
                    do {
                        writesize = out.write(err, buf, bufleft);
                        if (err != ENone)
                            return 0;
                        assert( writesize > 0 );
                        (char*&)buf += writesize;
                        bufleft     -= writesize;
                    } while (bufleft > 0);
                } else if (bufleft > 0) {
                    // Copy to buffer, partial fill
                    memcpy(data, buf, bufleft);
                    used = bufleft;
                }
            }
        }
        assert( size == 0 || used < size );
        err = ENone;
        return bufsize;
    }

    /** Write repeated data to device using buffer.
     - This will flush the buffer when full
     - Note: If writetext() was previously called then set partnl=0 before calling writetext() again
     .
     \tparam  T  IoDevice to write to

     \param  err      Stores ENone on success, error code on error [out]
     \param  out      Device to write to
     \param  buf      Data buffer to write repeatedly from
     \param  bufsize  Data size to write in bytes
     \param  count    Data repeat count (2 = 2*bufsize)
     \return          Size actually written, 0 on error, see err
    */
    template<class T>
    ulong writebin2(Error& err, T& out, const void* buf, ulong bufsize, ulong count) {
        assert( used <= size );
        const ulong countsize = count * bufsize;
        if (count > 0 && bufsize > 0) {
            if (size > 0) {
                const ulong BUF_THRESHOLD = size / 2;
                if (bufsize < BUF_THRESHOLD) {
                    // Fill buffer and flush, repeat as needed
                    char* const end = (data + size) - bufsize + 1;
                    char* p         = data + used;
                    for (ulong writecount=0;;) {
                        for (writecount=0; p < end && writecount < count; p+=bufsize, ++writecount)
                            memcpy(p, buf, bufsize);
                        used  += (writecount * bufsize);
                        count -= writecount;
                        if ( p >= end && (err=flush(out)) != ENone )
                            return 0;
                        if (count <= 0)
                            break;
                        p = data;
                    }
                } else
                    // bufsize too large, flush before writing directly
                    if ( (err=flush(out)) != ENone )
                        return 0;
            }

            // Write data directly (no buffer or buffer too small)
            if (count > 0) {
                ulong writesize, writeleft; const char* p;
                do {
                    // Write next repetition
                    for (writeleft=bufsize, p=(const char*)buf;; p+=writesize) {
                        writesize = out.write(err, p, writeleft);
                        if (err != ENone)
                            return 0;
                        assert( writesize > 0 );
                        writeleft -= writesize;
                        if (writeleft <= 0)
                            break;
                    }
                } while (--count > 0);
            }
        }
        assert( size == 0 || used < size );
        err = ENone;
        return countsize;
    }

    /** Write repeated character to device using buffer.
     - This will flush the buffer when full
     - Note: If writetext() was previously called then set partnl=0 before calling writetext() again
     .
     \tparam  T  IoDevice to write to

     \param  err    Stores ENone on success, error code on error [out]
     \param  out    Device to write to
     \param  ch     Character to write
     \param  count  Character repeat count to write
     \return        Size actually written, 0 on error, see err
    */
    template<class T>
    ulong writebin_char(Error& err, T& out, char ch, ulong count) {
        assert( used <= size );
        if (count > 0) {
            ulong writesize = size - used;
            if (count < writesize) {
                // Set in buffer, partial fill
                memset(data+used, ch, count);
                used += count;
            } else {
                ulong remain = count;
                if (size > 0) {
                    // Using buffer, fill and flush, repeat as needed
                    assert( writesize > 0 );
                    for (;;) {
                        if (used >= size) {
                            if ((err=flush(out)) != ENone)
                                return 0;
                            writesize = (remain < size ? remain : size);
                        }
                        if (remain <= 0)
                            break;
                        assert( writesize <= remain );
                        memset(data+used, ch, writesize);
                        used   += writesize;
                        remain -= writesize;
                    }
                } else {
                    // No buffer, write 1 character at a time (expensive!)
                    do {
                        out.write(err, &ch, 1);
                        if (err != ENone)
                            return 0;
                    } while (--remain > 0);
                }
            }
        }
        assert( size == 0 || used < size );
        err = ENone;
        return count;
    }

    /** Write data to device using buffer.
     - This will flush the buffer when full
     - This does a text write, converting newlines per newline member
     - Newline conversion may change the number of bytes written internally
     - If readtext() was used on buf (newline conversion already done) then use write() instead for best performance
     .
     \tparam  T  IoDevice to write to

     \param  err      Stores ENone on success, error code on error [out]
     \param  out      Device to write to
     \param  buf      Data buffer to write from
     \param  bufsize  Data size to write in bytes
     \return          Size actually written, 0 on error, see err
    */
    template<class T>
    ulong writetext(Error& err, T& out, const char* buf, ulong bufsize) {
        assert( used <= size );
        ulong writesize = 0;
        if (bufsize > 0) {
            const char* start = buf;
            if (partnl > 0) {
                if (*start == partnl)
                    ++start; // newline already written, skip remaining partial
                partnl = 0;
            }

            ulong flushlines_size = 0;
            ulong len, cur_newline_size; char checknext;
            const char* end = buf + bufsize;
            for (const char* p=start; p < end; ) {
                if (*p == '\n')
                    checknext = '\r';
                else if (*p == '\r')
                    checknext = '\n';
                else
                    { ++p; continue; }

                if (p+1 >= end) {
                    partnl = checknext; // can't checknext, note partial
                    cur_newline_size = 1;
                } else if (p[1] == checknext) {
                    if (newline[1] == checknext) {
                        p += 2; // no need to convert 2-char newline
                        flushlines_size = used + (ulong)(p - start);
                        continue;
                    }
                    cur_newline_size = 2;
                } else if (*p == *newline && newline[1] == '\0') {
                    ++p; // no need to convert 1-char newline
                    flushlines_size = used + (ulong)(p - start);
                    continue;
                } else
                    cur_newline_size = 1;

                len = (ulong)(p - start);
                if ( ( p > start && !writebin(err, out, start, len) ) ||
                     !writebin(err, out, newline, newlinesize)
                   )
                    return 0;
                start = (p += cur_newline_size);
                writesize += len + newlinesize;
                flushlines_size = used;
            }

            if (start < end) {
                len = (ulong)(end - start);
                if (!writebin(err, out, start, len))
                    return 0;
                writesize += len;
            }

            if (flushlines && flushlines_size > 0 && used > 0) {
                // Flush up to last newline
                assert( flushlines_size <= used );
                ulong flushpos = 0, flushwritesize;
                do {
                    flushwritesize = out.write(err, data+flushpos, flushlines_size-flushpos);
                    if (err != ENone)
                        return 0;
                    assert( flushwritesize > 0 );
                    flushpos += flushwritesize;
                } while (flushpos < flushlines_size);
                used -= flushlines_size;
                if (used > 0)
                    memmove(data, data+flushlines_size, used);
            }
        }
        err = ENone;
        return writesize;
    }

    /** Write repeated character data to device using buffer, formatted with field alignment.
     \tparam  T  IoDevice to write to

     \param  out    Device to write to
     \param  ch     Character to write
     \param  count  Character repeat count to use
     \param  field  Field attributes to use
     \return        ENone on success, ELength if string and/or field-width are too long for buffer, error code on other error [out]
    */
    template<class TOut>
    Error writefmtchar(TOut& out, char ch, ulong count, const FmtSetField& field) {
        assert( used <= size );
        Error err;
        if (field.width >= 0 && (uint)field.width > count) {
            if (size < (uint)field.width)
                return ELength;
            if (size-used < (uint)field.width && (err=flush(out)) != ENone)
                return err;

            const int fillchar = (int)(field.fill != 0 ? (uchar)field.fill : (uchar)' ');
            const uint padding = (uint)field.width - count;
            char* p = data + used;
            switch (field.align) {
                case faCURRENT: // fallthrough
                case fLEFT:
                    memset(p, (int)(uchar)ch, count);
                    if (padding > 0)
                        memset(p + count, fillchar, padding);
                    break;
                case fCENTER: {
                    const uint padleft = (padding / 2);
                    if (padleft > 0) {
                        memset(p, fillchar, padleft);
                        p += padleft;
                    }
                    memset(p, (int)(uchar)ch, count);
                    const uint padright = (padding - padleft);
                    if (padright > 0) {
                        p += count;
                        memset(p, fillchar, padright);
                    }
                    break;
                }
                case fRIGHT:
                    if (padding > 0) {
                        memset(p, fillchar, padding);
                        p += padding;
                    }
                    memset(p, (int)(uchar)ch, count);
                    break;
            };
            used += (uint)field.width;
        } else if (count > 0) {
            if (size < count)
                return ELength;
            if (size-used < count && (err=flush(out)) != ENone)
                return err;
            memset(data + used, (int)(uchar)ch, count);
            used += count;
        }
        return ENone;
    }

    /** Write string data to device using buffer, formatted with field alignment.
     - This will flush the buffer when full
     - This does a text write, converting newlines per newline member
     - Newline conversion may change the number of bytes written internally
     - If readtext() was used on buf (newline conversion already done) then use write() instead for best performance
     .
     \tparam  T  IoDevice to write to

     \param  out      Device to write to
     \param  str      %String data buffer to write from
     \param  strsize  %String data size to write in bytes
     \param  field    Field attributes to use
     \return          ENone on success, ELength if string and/or field-width are too long for buffer, error code on other error [out]
    */
    template<class TOut>
    Error writefmtstr(TOut& out, const char* str, ulong strsize, const FmtSetField& field) {
        assert( used <= size );
        if (size < strsize)
            return ELength;
        Error err;
        if (size-used < strsize && (err=flush(out)) != ENone)
            return err;

        if (field.width >= 0 && (ulong)field.width > strsize) {
            const int fillchar = (int)(field.fill != 0 ? (uchar)field.fill : (uchar)' ');
            const uint padding = field.width - strsize;
            char* p = data + used;
            used += strsize + padding;

            switch (field.align) {
                case faCURRENT: // fallthrough
                case fLEFT:
                    memcpy(p, str, strsize);
                    if (padding > 0)
                        memset(p + strsize, fillchar, padding);
                    break;
                case fCENTER: {
                    const uint padleft = (padding / 2);
                    if (padleft > 0) {
                        memset(p, fillchar, padleft);
                        p += padleft;
                    }
                    memcpy(p, str, strsize);
                    const uint padright = (padding - padleft);
                    if (padright > 0) {
                        p += strsize;
                        memset(p, fillchar, padright);
                    }
                    break;
                }
                case fRIGHT:
                    if (padding > 0) {
                        memset(p, fillchar, padding);
                        p += padding;
                    }
                    memcpy(p, str, strsize);
                    break;
            };
        } else if (strsize > 0) {
            memcpy(data + used, str, strsize);
            used += strsize;
        }
        return ENone;
    }

    /** Write data to device using buffer.
     - This will flush the buffer when full
     - This does a text write, converting newlines per newline member
     - Newline conversion may change the number of bytes written internally
       - Note that writing newline text characters 1 char at a time or by string will give the same end result either way -- the edge cases are covered
       .
     - This handles newline/newline-pair conversion consistently with calling writetext() with same data
     - If readtext() was used on buf (newline conversion already done) then use write() instead for best performance
     .
     \tparam  T  IoDevice to write to

     \param  err    Stores ENone on success, error code on error [out]
     \param  out    Device to write to
     \param  ch     Data buffer to write from
     \param  count  Data size to write in bytes
     \return        Size actually written (could be 0 in certain newline-pair cases with count=1), 0 on error, see err
    */
    template<class T>
    ulong writetext_char(Error& err, T& out, char ch, ulong count) {
        assert( used <= size );
        if (partnl > 0) {
            if (ch == partnl)
                --count; // newline already written, skip remaining partial
            partnl = 0;
        }
        if (count > 0) {
            if (ch == '\n')
                partnl = '\r';
            else if (ch == '\r')
                partnl = '\n';
            if (partnl == 0) {
                // Non-newline char
                count = writebin_char(err, out, ch, count);
            } else {
                // Newline char
                if (newlinesize == 1)
                    // 1 char newlines
                    count = writebin_char(err, out, *newline, count);
                else
                    // 2 char newlines
                    count = writebin2(err, out, newline, newlinesize, count);
            }
            if (err != ENone)
                return 0;
        }
        err = ENone;
        return count;
    }

    /** Write formatted signed number to device using buffer.
     - This formats directly to buffer and requires the buffer is large enough
     - If needed this will flush buffer to make room
     .
     \tparam  TOut  IoDevice to write to
     \tparam  TNum  Signed number type

     \param  out   Device to write to
     \param  num   Number to format and write
     \param  base  Base to use for formatting
     \return       ENone on success, error code on error
    */
    template<class TOut,class TNum>
    Error writenum(TOut& out, TNum num, int base=fDEC) {
        assert( used <= size );
        const uint len = IntegerT<TNum>::digits(num,base);
        if (size < len)
            return EBufSize;
        Error err;
        if (size-used < len && (err=flush(out)) != ENone)
            return err;
        used += len;
        impl::fnum(data+used, num, base);
        return ENone;
    }

    /** Write formatted unsigned number to device using buffer.
     - This formats directly to buffer and requires the buffer is large enough
     - If needed this will flush buffer to make room
     .
     \tparam  TOut  IoDevice to write to
     \tparam  TNum  Unsigned number type

     \param  out   Device to write to
     \param  num   Number to format and write
     \param  base  Base to use for formatting
     \return       ENone on success, error code on error
    */
    template<class TOut,class TNum>
    Error writenumu(TOut& out, TNum num, int base=fDEC) {
        assert( used <= size );
        const uint len = IntegerT<TNum>::digits(num,base);
        if (size < len)
            return EBufSize;
        Error err;
        if (size-used < len && (err=flush(out)) != ENone)
            return err;
        used += len;
        impl::fnumu(data+used, num, base);
        return ENone;
    }

    /** Write formatted floating-point number to device using buffer.
     - This formats directly to buffer and requires the buffer is large enough
     - If needed this will flush buffer to make room
     .
     \tparam  TOut  IoDevice to write to
     \tparam  TNum  Floating-point number number type

     \param  out        Device to write to
     \param  num        Number to format and write
     \param  precision  Formatting precision (number of fractional digits), 0 for none, fPREC_AUTO for automatic
     \return            ENone on success, error code on error
    */
    template<class TOut,class TNum>
    Error writenumf(TOut& out, TNum num, int precision=fPREC_AUTO) {
        assert( used <= size );
        int exp = 0;  uint maxlen;  Error err;
        if (precision < 0) {
            num    = FloatT<TNum>::fexp10(exp, num);
            maxlen = (uint)FloatT<TNum>::MAXDIGITS_AUTO;
            if (size < maxlen)
                return EBufSize;
            if (size-used < maxlen && (err=flush(out)) != ENone)
                return err;
            used += impl::fnumfe(data+used, num, exp, false);
        } else {
            num    = FloatT<TNum>::fexp10(exp, impl::fnumf_weight(num, precision));
            maxlen = (uint)FloatT<TNum>::maxdigits_prec(exp, precision);
            if (size < maxlen)
                return EBufSize;
            if (size-used < maxlen && (err=flush(out)) != ENone)
                return err;
            used += impl::fnumf(data+used, num, exp, precision);
        }
        return ENone;
    }

    /** Write formatted signed number to device using buffer, using field attributes.
     - This formats directly to buffer and requires the buffer is large enough
     - If needed this will flush buffer to make room
     .
     \tparam  TOut  IoDevice to write to
     \tparam  TNum  Signed number type

     \param  out    Device to write to
     \param  num    Number to format and write
     \param  fmt    Integer formatting attributes to use
     \param  field  Field formatting attributes to use, NULL for none
     \return        ENone on success, error code on error
    */
    template<class TOut,class TNum>
    Error writefmtnum(TOut& out, TNum num, const FmtSetInt& fmt, const FmtSetField* field=NULL) {
        if (fmt.base <= 0 || fmt.base == fDEC) {
            assert( used <= size );
            const int digits        = IntegerT<TNum>::digits(num, fDEC);
            const int width         = (fmt.pad_width > digits ? fmt.pad_width : digits);
            const int align_padding = (field != NULL && field->width > width ? field->width - width : 0);
            const uint len          = width + align_padding;

            if (size < len)
                return EBufSize;
            Error err;
            if (size-used < len && (err=flush(out)) != ENone)
                return err;

            fmt.impl_num_write(data + used, num, digits, width, align_padding, field);
            used += len;
        } else
            return writefmtnumu(out, (typename ToUnsigned<TNum>::Type)num, fmt, field);
        return ENone;
    }

    /** Write formatted unsigned number to device using buffer, using field attributes.
     - This formats directly to buffer and requires the buffer is large enough
     - If needed this will flush buffer to make room
     .
     \tparam  TOut  IoDevice to write to
     \tparam  TNum  Signed number type

     \param  out    Device to write to
     \param  num    Number to format and write
     \param  fmt    Integer formatting attributes to use
     \param  field  Field formatting attributes to use, NULL for none
     \return        ENone on success, error code on error
    */
    template<class TOut,class TNum>
    Error writefmtnumu(TOut& out, TNum num, const FmtSetInt& fmt, const FmtSetField* field=NULL) {
        assert( used <= size );
        const int base = (fmt.base > 0 ? fmt.base : fDEC);

        char prefix_ch  = 0;
        uint prefix_len = 0;
        fmt.impl_prefix_info(prefix_ch, prefix_len);

        const int digits        = IntegerT<TNum>::digits(num, base);
        const int width         = (fmt.pad_width > digits ? fmt.pad_width : digits);
        const int full_width    = width + prefix_len;
        const int align_padding = (field != NULL && field->width > full_width ? field->width - full_width : 0);
        const uint len          = full_width + align_padding;

        if (size < len)
            return EBufSize;
        Error err;
        if (size-used < len && (err=flush(out)) != ENone)
            return err;

        char* p = data + used;
        used += len;

        int align_padleft, align_padright;
        FmtSetField::setup_align(align_padleft, align_padright, align_padding, field);

        if (align_padleft > 0) {
            memset(p, (int)(uchar)field->fill, align_padleft);
            p += align_padleft;
        }

        FmtSetInt::impl_prefix_write(p, prefix_ch, prefix_len);

        if (digits < width) {
            const uint padlen = width - digits;
            const int ch = (fmt.pad_ch == 0 ? '0' : (int)(uchar)fmt.pad_ch);
            memset(p, ch, padlen);
            p += padlen;
        }
        p += digits;
        impl::fnumu(p, num, base);

        if (align_padright > 0)
            memset(p, (int)(uchar)field->fill, align_padright);

        return ENone;
    }

    /** Write formatted floating point number to device using buffer, using field attributes.
     - This formats directly to buffer and requires the buffer is large enough
     - If needed this will flush buffer to make room
     .
     \tparam  TOut  IoDevice to write to
     \tparam  TNum  Signed number type

     \param  out    Device to write to
     \param  num    Number to format and write
     \param  fmt    Floating point formatting attributes to use
     \param  field  Field formatting attributes to use, NULL for none
     \return        ENone on success, error code on error
    */
    template<class TOut,class TNum>
    Error writefmtnumf(TOut& out, TNum num, const FmtSetFloat& fmt, const FmtSetField* field=NULL) {
        const int align_width = (field != NULL ? field->width : 0);
        int exp = 0, maxlen;
        fmt.impl_info(num, exp, maxlen, align_width);   // sets maxlen

        if (size < (uint)maxlen)
            return EBufSize;
        Error err;
        if (size-used < (uint)maxlen && (err=flush(out)) != ENone)
            return err;

        used += fmt.impl_write(data + used, num, exp, align_width, field);
        return ENone;
    }

    /** Write formatted buffer dump in hex.
     - Output may span multiple lines, and always ends with a newline (unless dump data is empty)
     - This flushes the buffer as needed to make room
     .
     \tparam  TOut  IoDevice to write to

     \param  out          Device to write to
     \param  fmt          Format data, including buffer to dump
     \param  newline      Newline string to use
     \param  newlinesize  Newline string size
     \return              ENone on success, error code on error
    */
    template<class TOut>
    Error writefmtdump(TOut& out, const FmtDump& fmt, const char* newline, uint newlinesize) {
        const char* DIGITS     = (fmt.upper ? "0123456789ABCDEF" : "0123456789abcdef");
        const ulong LINE_SIZE  = (fmt.maxline > 0 ? fmt.maxline : fmt.size);
        const ulong FLUSH_SIZE = size - 3;  // Flush if not enough room for at least 3 bytes (2-digit hex and space, or newline)

        Error err;
        const uchar* ptr     = (uchar*)fmt.buf;
        const uchar* ptr_end = ptr + fmt.size;
        const uchar* ptr_nl;

        FmtSetInt offset_fmt(fHEX, 0);
        ulong offset = 0;
        if (fmt.maxline > 0 && !fmt.compact)
            offset_fmt.pad_width = Int::digits(fmt.size, fHEX);

        // Loop for each line
        for (const uchar* ptr2; ptr < ptr_end; ) {
            // Show offset
            if (fmt.maxline > 0 && !fmt.compact) {
                if ((err = writefmtnumu(out, offset, offset_fmt)) != ENone)
                    return err;
                offset += fmt.maxline;

                if (used >= FLUSH_SIZE && (err=flush(out)) != ENone)
                    return err;
                data[used++] = ':';
                data[used++] = ' ';
                data[used++] = ' ';
            }

            // Figure newline position
            ptr_nl = ptr + LINE_SIZE;
            if (ptr_nl > ptr_end)
                ptr_nl = ptr_end;

            // Hex dump line
            ptr2 = ptr;
            for (; ptr < ptr_nl; ++ptr) {
                if (used >= FLUSH_SIZE && (err=flush(out)) != ENone)
                    return err;
                data[used++] = DIGITS[(*ptr >> 4) & 0x0F];
                data[used++] = DIGITS[*ptr & 0x0F];
                data[used++] = ' ';
            }

            if (fmt.compact) {
                assert( used > 0 );
                --used; // trim extra space from last byte
            } else {
                if (ptr_nl >= ptr_end && fmt.maxline > 0 && ptr2 != (uchar*)fmt.buf) {
                    // Pad last line, add separator
                    const ulong remainder = fmt.size % fmt.maxline;
                    ulong avail, wrlen, count = (remainder > 0 ? ((fmt.maxline - remainder) * 3) + 1 : 1);
                    while (count > 0) {
                        if (used >= FLUSH_SIZE && (err=flush(out)) != ENone)
                            return err;
                        avail = size - used;
                        wrlen = (count > avail ? avail : count);
                        memset(data + used, ' ', wrlen);
                        count -= wrlen;
                        used  += wrlen;
                    }
                } else {
                    // Separator
                    if (used >= FLUSH_SIZE && (err=flush(out)) != ENone)
                        return err;
                    data[used++] = ' ';
                }

                // ASCII dump
                for (; ptr2 < ptr_nl; ++ptr2) {
                    if (used >= FLUSH_SIZE && (err=flush(out)) != ENone)
                        return err;
                    if (*ptr2 < ' ' || *ptr2 > '~')
                        data[used++] = '.';
                    else
                        data[used++] = (char)*ptr2;
                }
            }

            // Newlie
            if (used >= FLUSH_SIZE && (err=flush(out)) != ENone)
                return err;
            memcpy(data + used, newline, newlinesize);
            used += newlinesize;
        }
        return ENone;
    }
};

///////////////////////////////////////////////////////////////////////////////
//@}
}
#if defined(_MSC_VER)
    #pragma warning(pop)
#endif
#endif
