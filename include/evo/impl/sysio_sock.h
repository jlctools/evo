// Evo C++ Library
/* Copyright 2019 Justin Crowell
Distributed under the BSD 2-Clause License -- see included file LICENSE.txt for details.
*/
///////////////////////////////////////////////////////////////////////////////
/** \file sysio_sock.h Evo system I/O socket implementation. */
#pragma once
#ifndef INCL_evo_impl_sysio_sock_h
#define INCL_evo_impl_sysio_sock_h

#include "sysio.h"
#include "../string.h"

#if defined(_WIN32)
    // Windows
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "Ws2_32.lib")
#else
    // Linux/Unix
    #include <sys/socket.h>
    #include <sys/un.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <netdb.h>
    #if defined(__APPLE__) && !defined(SOCK_NONBLOCK)
        #define SOCK_NONBLOCK O_NONBLOCK
    #endif
#endif

namespace evo {
/** \addtogroup EvoIO */
//@{

///////////////////////////////////////////////////////////////////////////////

#if defined(_WIN32)
    // Windows
    struct SysWindows {
        static void set_timeval_ms(struct timeval& tm, ulong ms) {
            const ulong MSEC_PER_SEC  = 1000;
            const ulong USEC_PER_MSEC = 1000;
            tm.tv_sec  = ms / MSEC_PER_SEC;
            tm.tv_usec = (ms - (tm.tv_sec * MSEC_PER_SEC)) * USEC_PER_MSEC;
        }
    };
#endif

///////////////////////////////////////////////////////////////////////////////

/** Get socket error message for error code.
 - This gives more socket specific error messages where applicable, and for general errors uses errormsg()
 .
 \param  error  %Error code -- see Error
 \return        Message for error code (null terminated, always an immutable string literal)
*/
inline const char* errormsg_socket(Error error) {
    switch (error) {
        case EAccess:   return "Permission denied for socket (EAccess)";
        case EClosed:   return "Socket is closed (EClosed)";
        case EExist:    return "Socket address/port in use or not available (EExist)";
        case ESize:     return "Message too long (ESize)";
        case ELimit:    return "No more socket descriptors available (ELimit)";
        case ELength:   return "Host or address string too long (ELength)";
        case EFail:     return "Connection refused or reset (EFail)";
        case ENotFound: return "Address not found (ENotFound)";
        default: break;
    }
    return evo::errormsg(error);
}

/** %Socket config exception for getopt() or setopt() errors, see Exception. */
class ExceptionSocketConfig : public ExceptionStream
    { EVO_CREATE_EXCEPTION_IMPL_2(ExceptionSocketConfig, ExceptionStream, errormsg_socket) };

/** %Socket open exception for socket connect/bind/listen errors, see Exception. */
class ExceptionSocketOpen : public ExceptionStreamOpen
    { EVO_CREATE_EXCEPTION_IMPL_2(ExceptionSocketOpen, ExceptionStreamOpen, errormsg_socket) };

/** %Socket input stream exception for socket read errors, see Exception. */
class ExceptionSocketIn : public ExceptionStreamIn
    { EVO_CREATE_EXCEPTION_IMPL_2(ExceptionSocketIn, ExceptionStreamIn, errormsg_socket) };

/** %Socket output stream exception for socket write errors, see Exception. */
class ExceptionSocketOut : public ExceptionStreamOut
    { EVO_CREATE_EXCEPTION_IMPL_2(ExceptionSocketOut, ExceptionStreamOut, errormsg_socket) };

///////////////////////////////////////////////////////////////////////////////

/** Base socket address.
 - This is essentially a C++ wrapper for the C sockaddr structures
   - Any derived class pointer/reference can be cast to a generic SocketAddress
 - Though this is a base structure/class, the common interface (virtual methods) is minimal
*/
struct SocketAddressBase {
    socklen_t addrlen;          ///< Address length

    /** %Set addrlen to max size for socket address.
     - Low-level socket functions that store a socket address need to know the max socket address length
     - Derived class must implement this and set the proper max size
    */
    virtual void set_maxsize()
        { addrlen = 0; }
};

///////////////////////////////////////////////////////////////////////////////

/** Generic socket address (used internally).
 - This is essentially a C++ wrapper for the C sockaddr structures
   - This overlaps with all concrete socket address types since they all start with a union on "addr"
   - So a method can accept SocketAddressBase* and cast it to SocketAddress*
*/
struct SocketAddress : public SocketAddressBase {
    struct sockaddr addr;       ///< Generic address structure
};

///////////////////////////////////////////////////////////////////////////////

/** Resolves socket name/address to socket address info.
 - This wraps socket getaddrinfo() and addrinfo structures
   - A name/address may resolve to multiple interfaces
 - You can set hints directly, or use helpers like tcp() or udp()
 - Call resolve() or convert() to resolve a host/port
*/
struct SocketAddressInfo {
    struct addrinfo  hints;     ///< Hints used as input to resolve()
    struct addrinfo* ptr;       ///< Pointer to first address in resolve results
    int              code;      ///< Internal return code from resolve()

    static const int NUMERIC = AI_NUMERICHOST | AI_NUMERICHOST; ///< Flags for numeric host address and port

    /** Constructor
     \param  family  Address family to use, common values: AF_INET for IPv4, AF_INET6 for IPv6
    */
    SocketAddressInfo(int family=AF_INET) {
        memset(&hints, 0, sizeof(hints));
        hints.ai_family = family;
        ptr = NULL;
        code = 0;
        tcp();
    }

    /** Destructor */
    ~SocketAddressInfo()
        { free(); }

    /** Reset data
     \param  family  Address family to use, common values: AF_INET for IPv4, AF_INET6 for IPv6
     \return         This
    */
    SocketAddressInfo& reset(int family=AF_INET) {
        memset(&hints, 0, sizeof(hints));
        hints.ai_family = family;
        ptr  = NULL;
        code = 0;
        return *this;
    }

    /** Setup for resolving to TCP address
     \return  This
    */
    SocketAddressInfo& tcp() {
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;
        return *this;
    }

    /** Setup for resolving to UDP address
     \return  This
    */
    SocketAddressInfo& udp() {
        hints.ai_socktype = SOCK_DGRAM;
        hints.ai_protocol = IPPROTO_UDP;
        return *this;
    }

    /** Resolve or convert host name/address and port to one or more socket addresses.
     - By default this may block while calling external services like DNS, unless flags disables this
     .
     \param  host   Host name or address to resolve (terminated string)
     \param  port   Port number or service name to resolve (terminated string), NULL to skip
     \param  flags  Flags, use default or NUMERIC, or see getaddrinfo() for more
     \return        ENone for success, ENotFound if host not found, otherwise error code
    */
    Error resolve(const char* host, const char* port=NULL, int flags=AI_NUMERICSERV) {
        free();
        hints.ai_flags = flags;
        code = ::getaddrinfo(host, port, &hints, &ptr);
        Error err;
        switch (code) {
            case 0:                     err = ENone;      break;
            case EAI_NONAME:            err = ENotFound;  break;
            case EAI_FAIL:              err = EFail;      break;
            case EAI_SERVICE:           // fallthrough
            case EAI_SOCKTYPE:          // fallthrough
            case EAI_FAMILY:            // fallthrough
            case EAI_BADFLAGS:          err = EInval;     break;
            case EAI_MEMORY:            err = ESpace;     break;
            case EAI_AGAIN:             err = ERetry;     break;
        #if defined(EAI_NODATA) && !defined(_WIN32) // win32: same as EAI_NONAME
            case EAI_NODATA:            err = ENotFound;  break;
        #endif
        #if defined(EAI_ADDRFAMILY)
            case EAI_ADDRFAMILY:        err = ENotFound;  break;
        #endif
            default:                    err = EUnknown;   break;
        }
        return err;
    }

    /** Resolve host name/address and port to one or more socket addresses.
     - By default this may block while calling external services (like DNS)
     .
     \param  host   Host name or address to resolve (terminated string)
     \param  port   Port number to use
     \param  flags  Flags, use default or NUMERIC, or see getaddrinfo() for more
     \return        ENone for success, ENotFound if host not found, otherwise error code
    */
    Error resolve(const char* host, ushort port, int flags=0) {
        StringInt<ushort> port_str(port);
        return resolve(host, port_str.data_, flags | AI_NUMERICSERV);
    }

    /** Resolve or convert host name/address and port to one or more socket addresses.
     - By default this may block while calling external services like DNS, unless flags disables this
     .
     \param  host   Host name or address to resolve
     \param  port   Port number to use, 0 for none
     \param  flags  Flags, use default or NUMERIC, or see getaddrinfo() for more
     \return        ENone for success, ENotFound if host not found, otherwise error code
    */
    Error resolve(const SubString& host, ushort port=0, int flags=0) {
        const uint MAX_HOST_SIZE = 256; // See IETF RFC 1123
        char hostbuf[MAX_HOST_SIZE];

        const SubString::Size hostlen = host.size();
        if (hostlen < MAX_HOST_SIZE) {
            memcpy(hostbuf, host.data(), hostlen);
            hostbuf[hostlen] = '\0';

            flags |= AI_NUMERICSERV;
            if (port > 0) {
                StringInt<ushort> port_str(port);
                return resolve(hostbuf, port_str.data_, flags);
            } else
                return resolve(hostbuf, EVO_CNULL, flags);
        }
        errno = ENAMETOOLONG;
        return ELength;
    }

    /** %Convert host address to one or more socket addresses.
     - This is like resolve() but doesn't call an external service (like DNS) so doesn't block
     .
     \param  host  Host address to convert/resolve (terminated string)
     \return       ENone for success, ENotFound if host not found, otherwise error code
    */
    Error convert(const char* host)
        { return resolve(host, EVO_CNULL, NUMERIC); }

    /** %Convert host address to one or more socket addresses.
     - This is like resolve() but doesn't call an external service (like DNS) so doesn't block
     .
     \param  host  Host address to resolve (terminated string)
     \param  port  Port number to use
     \return       ENone for success, ENotFound if host not found, otherwise error code
    */
    Error convert(const char* host, ushort port)
        { return resolve(host, port, NUMERIC); }

    /** %Convert host address to one or more socket addresses.
     - This is like resolve() but doesn't call an external service (like DNS) so doesn't block
     .
     \param  host  Host address to resolve (terminated string)
     \param  port  Port number to use, 0 for none
     \return       ENone for success, ENotFound if host not found, otherwise error code
    */
    Error convert(const SubString& host, ushort port=0)
        { return resolve(host, port, NUMERIC); }

    /** Free results allocated from resolve() or resolve_address.
     - This is called automatically by destructor or next resolve() call
    */
    void free() {
        if (ptr != NULL) {
            ::freeaddrinfo(ptr);
            ptr = NULL;
        }
    }

private:
    // Disable copying
    SocketAddressInfo(const SocketAddressInfo&);
    SocketAddressInfo& operator=(const SocketAddressInfo&);
};

///////////////////////////////////////////////////////////////////////////////

/** %Socket I/O device (used internally).
 - This is an internal low-level interface with public members, use Socket instead
 - Members are public for quick access and simple low-level interface
 - This does not do any read/write buffering
 - Implementation is OS specific, low-level error codes can be read from errno
   - Windows: This sets errno on error -- from WSAGetLastError()
 - IoSocket::init() (or Socket::sysinit()) should be called as early as possible before using this
*/
struct IoSocket : public IoDevice {
    static const bool  STREAM_SEEKABLE = false;     ///< Socket streams are not seekable with Stream
    static const ulong TIMEOUT_DEFAULT = 30000;     ///< Default timeout used in milliseconds

    typedef ExceptionSocketIn  ExceptionInT;        ///< Input exception type for socket stream
    typedef ExceptionSocketOut ExceptionOutT;       ///< Output exception type for socket stream

    ulong timeout_ms;       ///< Current timeout in milliseconds, 0 for indefinite

#if defined(_WIN32)
    // Windows

    enum Shutdown {
        sIN    = SD_RECEIVE,
        sOUT   = SD_SEND,
        sINOUT = SD_BOTH
    };

    struct Init {
        bool    active;
        WSADATA data;

        Init() : active(false)
            { init(); }

        ~Init()
            { cleanup(); }

        void init() {
            if (!active)
                active = (::WSAStartup(MAKEWORD(2, 2), &data) == 0);
        }

        void cleanup() {
            if (active && ::WSACleanup() == 0)
                active = false;
        }
    };

    typedef SOCKET Handle;
    typedef DWORD  OptNum;

    static const Handle INVALID    = INVALID_SOCKET;
    static const int    SOCK_ERROR = SOCKET_ERROR;

    IoSocket() {
        timeout_ms = TIMEOUT_DEFAULT;
        handle     = INVALID;
        nonblock   = false;
    }

    IoSocket(Handle socket) {
        timeout_ms = TIMEOUT_DEFAULT;
        handle     = socket;
        nonblock   = false;
    }

    bool shutdown(Shutdown how=sINOUT) {
        if (handle == INVALID)
            errno = WSAENOTCONN;
        else if (::shutdown(handle, how) != 0)
            errno = WSAGetLastError();
        else
            return true;
        return false;
    }

    void close() {
        if (handle == INVALID)
            errno = WSAENOTCONN;
        else if (::closesocket(handle) == SOCKET_ERROR)
            errno = WSAGetLastError();
        else
            errno = 0;
    }

    Error set_nonblock(bool enable=true) {
        if (handle != INVALID) {
            ulong flag = (enable ? 1 : 0);
            if (::ioctlsocket(handle, FIONBIO, &flag) != 0) {
                Error err;
                const int last_error = WSAGetLastError();
                errno = last_error;
                switch (last_error) {
                    case WSANOTINITIALISED: // fallthrough
                    case WSAENOTSOCK: err = EClosed;  break;
                    default:          err = EUnknown; break;
                }
                return err;
            }
        }
        nonblock = enable;
        return ENone;
    }

    bool accept(Error& err, IoSocket& client_socket, SocketAddressBase* client_address=NULL) {
        if (handle == INVALID) {
            err   = EClosed;
            errno = WSAENOTCONN;
        }  else {
            if (timeout_ms == 0 || read_wait(err, timeout_ms)) {
                Handle client_handle;
                if (client_address != NULL)
                    client_handle = ::accept(handle, &((SocketAddress*)client_address)->addr, &client_address->addrlen);
                else
                    client_handle = ::accept(handle, NULL, NULL);
                if (client_handle != INVALID) {
                    client_socket.attach(client_handle);
                    err = ENone;
                    return true;
                }
                err = get_socket_error();
            }
        }
        return false;
    }

    bool accept_nonblock(Error& err, IoSocket& client_socket, SocketAddressBase* client_address=NULL) {
        if (accept(err, client_socket, client_address)) {
            err = set_nonblock();
            return (err == ENone);
        }
        return false;
    }

    bool read_wait(Error& err, ulong read_timeout_ms) {
        fd_set read_set;
        FD_ZERO(&read_set);
        FD_SET(handle, &read_set);

        struct timeval timeout;
        SysWindows::set_timeval_ms(timeout, read_timeout_ms);

        int waitresult = ::select(0, &read_set, NULL, NULL, &timeout);
        if (waitresult == SOCK_ERROR) {
            const int last_error = WSAGetLastError();
            errno = last_error;
            switch (last_error) {
                case WSANOTINITIALISED: // fallthrough
                case WSAENOTSOCK: err = EClosed;  break;
                default:          err = EUnknown; break;
            }
            return false;
        } else if (waitresult == 0) {
            errno = WSAETIMEDOUT;
            err   = ETimeout;
            return false;
        }
        return true;
    }

    ulong read(Error& err, void* buf, ulong size) {
        if (handle == INVALID) {
            err   = EClosed;
            errno = ENOTCONN;
            return 0;
        }
        if (size > (ulong)std::numeric_limits<int>::max())
            size = (ulong)std::numeric_limits<int>::max();
        if (timeout_ms > 0 && !read_wait(err, timeout_ms))
            return 0;
        int result = ::recv(handle, (char*)buf, size, 0);
        if (result == SOCK_ERROR) {
            const int last_error = WSAGetLastError();
            errno = last_error;
            switch (last_error) {
                case WSANOTINITIALISED: // fallthrough
                case WSAENOTSOCK:       // fallthrough
                case WSAEINVAL:         // fallthrough
                case WSAENOTCONN:    err = EClosed;   break;
                case WSAEMSGSIZE:    err = ESize;     break;
                case WSAEFAULT:      err = EPtr;      break;
                case WSAEWOULDBLOCK: err = ENonBlock; break;
                default:             err = ERead;     break;
            }
            return 0;
        }
        err = ENone;
        return (ulong)result;
    }

    ulong readfrom(Error& err, void* buf, ulong size, struct sockaddr* address=NULL, socklen_t* address_len=NULL, int flags=0) {
        if (handle == INVALID) {
            err   = EClosed;
            errno = ENOTCONN;
            return 0;
        }
        if (size > (ulong)std::numeric_limits<int>::max())
            size = (ulong)std::numeric_limits<int>::max();
        if (timeout_ms > 0 && !read_wait(err, timeout_ms))
            return 0;
        int result = ::recvfrom(handle, (char*)buf, size, flags, address, address_len);
        if (result == 0) {
            err   = EClosed;
            errno = ENOTCONN;
            return 0;
        } else if (result < 0) {
            const int last_error = WSAGetLastError();
            errno = last_error;
            switch (last_error) {
                case WSANOTINITIALISED: // fallthrough
                case WSAENOTSOCK:       // fallthrough
                case WSAEINVAL:         // fallthrough
                case WSAENOTCONN:       err = EClosed;   break;
                case WSAEMSGSIZE:       err = ESize;     break;
                case WSAEFAULT:         err = EPtr;      break;
                case WSAEWOULDBLOCK:    err = ENonBlock; break;
                case WSAECONNRESET:     err = EFail;     break;
                default:                err = ERead;     break;
            }
            return 0;
        }
        err = ENone;
        return (ulong)result;
    }

    bool write_wait(Error& err, ulong write_timeout_ms) {
        fd_set write_set;
        FD_ZERO(&write_set);
        FD_SET(handle, &write_set);

        struct timeval timeout;
        SysWindows::set_timeval_ms(timeout, write_timeout_ms);

        int waitresult = ::select(0, NULL, &write_set, NULL, &timeout);
        if (waitresult == SOCK_ERROR) {
            const int last_error = WSAGetLastError();
            errno = last_error;
            switch (last_error) {
                case WSANOTINITIALISED: // fallthrough
                case WSAENOTSOCK: err = EClosed;  break;
                default:          err = EUnknown; break;
            }
            return false;
        } else if (waitresult == 0) {
            errno = WSAETIMEDOUT;
            err   = ETimeout;
            return false;
        }
        return true;
    }

    ulong write(Error& err, const void* buf, ulong size) {
        if (handle == INVALID) {
            err   = EClosed;
            errno = ENOTCONN;
            return 0;
        }
        if (size > (ulong)std::numeric_limits<int>::max())
            size = (ulong)std::numeric_limits<int>::max();
        if (timeout_ms > 0 && !write_wait(err, timeout_ms))
            return 0;
        int result = ::send(handle, (const char*)buf, (int)size, 0);
        if (result == SOCK_ERROR) {
            const int last_error = WSAGetLastError();
            errno = last_error;
            switch (last_error) {
                case WSANOTINITIALISED: // fallthrough
                case WSAENOTSOCK:       // fallthrough
                case WSAEINVAL:         // fallthrough
                case WSAENOTCONN:    err = EClosed;   break;
                case WSAENOBUFS:     err = ESpace;    break;
                case WSAEMSGSIZE:    err = ESize;     break;
                case WSAEFAULT:      err = EPtr;      break;
                case WSAEWOULDBLOCK: err = ENonBlock; break;
                default:             err = EWrite;    break;
            }
            return 0;
        }
        err = ENone;
        return (ulong)result;
    }

    ulong writeto(Error& err, const void* buf, ulong size, const struct sockaddr* address, socklen_t address_len, int flags=0) {
        if (handle == INVALID) {
            err   = EClosed;
            errno = ENOTCONN;
            return 0;
        }
        if (size > (ulong)std::numeric_limits<int>::max()) {
            err   = ESize;
            errno = WSAEMSGSIZE;
            return 0;
        }
        if (timeout_ms > 0 && !write_wait(err, timeout_ms))
            return 0;
        int result = ::sendto(handle, (const char*)buf, (int)size, flags, address, address_len);
        if (result < 0) {
            const int last_error = WSAGetLastError();
            errno = last_error;
            switch (last_error) {
                case WSANOTINITIALISED: // fallthrough
                case WSAENOTSOCK:       // fallthrough
                case WSAEINVAL:         // fallthrough
                case WSAENOTCONN:       err = EClosed;   break;
                case WSAEACCES:         err = EAccess;   break;
                case WSAENOBUFS:        err = ESpace;    break;
                case WSAEMSGSIZE:       err = ESize;     break;
                case WSAEFAULT:         err = EPtr;      break;
                case WSAEWOULDBLOCK:    err = ENonBlock; break;
                case WSAECONNABORTED:   // fallthrough
                case WSAECONNRESET:     err = EFail;     break;
                case WSAEADDRNOTAVAIL:  // fallthrough
                case WSAEDESTADDRREQ:   // fallthrough
                case WSAEHOSTUNREACH:   err = ENotFound; break;
                case WSAEAFNOSUPPORT:   err = EInvalOp;  break;
                default:                err = EWrite;    break;
            }
            return 0;
        }
        err = ENone;
        return (ulong)result;
    }

    Error getopt(int level, int optname, void* buf, uint& size) {
        if (size > (uint)INT_MAX)
            size = (uint)INT_MAX;
        int optlen = (int)size;
        int result = ::getsockopt(handle, level, optname, (char*)buf, &optlen);
        if (result < 0) {
            Error err;
            const int last_error = WSAGetLastError();
            errno = last_error;
            switch (last_error) {
                case WSANOTINITIALISED: // fallthrough
                case WSAENOTSOCK:       // fallthrough
                case WSAENOTCONN:       err = EClosed;  break;
                case WSAEFAULT:         err = EPtr;     break;
                case WSAEINVAL:         err = EInval;   break;
                case WSAENOPROTOOPT:    err = EInvalOp; break;
                default:                err = EUnknown; break;
            }
            return err;
        }
        size = (optlen > 0) ? (uint)optlen : 0;
        return ENone;
    }

    Error setopt(int level, int optname, const void* buf, uint size) {
        if (size > (uint)INT_MAX) {
            errno = WSAEMSGSIZE;
            return ESize;
        }
        int result = ::setsockopt(handle, level, optname, (const char*)buf, (int)size);
        if (result < 0) {
            Error err;
            const int last_error = WSAGetLastError();
            errno = last_error;
            switch (last_error) {
                case WSANOTINITIALISED: // fallthrough
                case WSAENOTSOCK:       // fallthrough
                case WSAENOTCONN:       err = EClosed;  break;
                case WSAEFAULT:         err = EPtr;     break;
                case WSAEINVAL:         err = EInval;   break;
                case WSAENOPROTOOPT:    err = EInvalOp; break;
                default:                err = EUnknown; break;
            }
            return err;
        }
        return ENone;
    }

    template<class TOut>
    static TOut& errormsg_out(TOut& out, Error err) {
        const int lasterror = errno;
        out << errormsg_socket(err) << ListBase<char>(" (errno:", 8) << lasterror << ')';
        if (err != ENone && lasterror != 0) {
            const int BUF_SIZE = 256;
            char buf[BUF_SIZE];
            DWORD len = ::FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, lasterror, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), buf, BUF_SIZE, NULL);
            if (len > 0)
                out << ListBase<char>(": ", 2) << ListBase<char>(buf, len);
        }
        return out;
    }

#else
    // Linux/Unix

    /** Used with shutdown(). */
    enum Shutdown {
        sIN    = SHUT_RD,       ///< Shutdown input (reads) on socket
        sOUT   = SHUT_WR,       ///< Shutdown output (writes) on socket
        sINOUT = SHUT_RDWR      ///< Shutdown both input and output (reads and writes) on socket
    };

    /** Used internally to initialize system socket API. */
    struct Init
        { };

    typedef int Handle;         ///< System socket handle
    typedef int OptNum;     ///< General number type for socket options

    static const Handle INVALID    = -1;        ///< Invalid handle value (used internally)
    static const int    SOCK_ERROR = -1;        ///< Socket error value (used internally)

    /** Constructor. */
    IoSocket() {
        timeout_ms = TIMEOUT_DEFAULT;
        handle     = INVALID;
        nonblock   = false;
        autoresume = true;
    }

    /** Constructor attaching to existing socket.
     - This is equivalent to using default constructor and calling attach()
     .
     \param  socket  Socket handle to attach
    */
    IoSocket(Handle socket) {
        timeout_ms = TIMEOUT_DEFAULT;
        handle     = socket;
        nonblock   = false;
        autoresume = true;
    }

    /** %Shutdown socket communication.
     - This is a gracefull way to shutdown input and/or output on socket so the other end is informated
     - Don't use on error, just close the socket (or let destructor close it)
     .
     \param  how  Whether to shutdown input, output, or both -- see Shutdown enum
     \return      Whether successful, false on error (not connected, invalid, etc)
    */
    bool shutdown(Shutdown how=sINOUT) {
        if (handle == INVALID)
            errno = ENOTCONN;
        else if (::shutdown(handle, (int)how) == 0)
            return true;
        return false;
    }

    /** Close connection and socket.
     - To gracefully shutdown call shutdown() first, unless there was an error
    */
    void close() {
        if (handle != INVALID) {
            ::close(handle);
            handle = INVALID;
        }
    }

    /** Enable/Disable non-blocking I/O.
     - This updates the nonblock flag, which persists even after socket is closed and created again
     - If socket is open, this updates non-blocking mode on it too
     - With non-blocking enabled, connect/read/write operations return error code ENonBlock when they would normally block
     .
     \param  enable  Whether to enable non-blocking mode, false to disable
     \return         ENone for success, otherwise error code
    */
    Error set_nonblock(bool enable=true) {
    #if defined(SOCK_NONBLOCK)
        if (handle != INVALID) {
            bool error_flag; // init below
            int flags = ::fcntl(handle, F_GETFL, NULL);
            if (flags >= 0) {
                if (enable)
                    flags |= O_NONBLOCK;
                else
                    flags &= ~(int)O_NONBLOCK;
                error_flag = (::fcntl(handle, F_SETFL, flags) != 0);
            } else
                error_flag = true;

            if (error_flag) {
                Error err;
                switch (errno) {
                    case EAGAIN: // fallthrough
                    case EACCES: err = EAccess;  break;
                    case EINVAL: err = EInval;   break;
                    case EBADF:  err = EClosed;  break;
                    default:     err = EUnknown; break;
                }
                return err;
            }
        }
        nonblock = enable;
        return ENone;
    #else
        // Non-blocking I/O not supported on this system
        errno = EINVAL;
        return EInval;
    #endif
    }

    /** Wait until socket connection is established and ready to write.
     - Used with non-blocking I/O, after connection is started
     - If connection failed this fails with err set to the error code
     .
     \param  err         %Set to error code on failure, ETimeout on timeout
     \param  timeout_ms  Wait timeout in milliseconds, 0 for no wait (returns immediately, useful for polling)
     \return             Whether connected, false on error
    */
    bool connect_wait(Error& err, ulong timeout_ms) {
        if (handle == INVALID) {
            err   = EClosed;
            errno = ENOTCONN;
        } else if (write_wait(err, timeout_ms)) {
            for (;;) {
                int value = 0;
                socklen_t len = sizeof(value);
                if (::getsockopt(handle, SOL_SOCKET, SO_ERROR, &value, &len) == 0) {
                    if (value == EINTR && autoresume) {
                        continue;
                    } else if (value == 0 || value == EISCONN) {
                        err = ENone;
                        return true;
                    }
                    err = get_socket_error(value);
                } else
                    err = get_socket_error();
                break;
            }
        }
        return false;
    }

    /** Wait until socket is ready to read.
     - Socket is ready to read when it's received some data to read
     - Used with non-blocking I/O, before calling read()
     .
     \param  err         %Set to error code on failure, ETimeout on timeout
     \param  timeout_ms  Wait timeout in milliseconds, 0 for indefinite
     \return             Whether ready to read, false on error
    */
    bool read_wait(Error& err, ulong timeout_ms)
        { return SysLinuxIo::read_wait(err, handle, timeout_ms, autoresume); }

    // Documented by parent
    ulong read(Error& err, void* buf, ulong size)
        { return SysLinuxIo::read(err, handle, buf, size, timeout_ms, autoresume); }

    /** Read message from socket device.
     - This is used with UDP sockets to receive a packet
     - UDP packet size limit is usually just under 64 KB, depending on the protocol used
     .
     \param  err          Stores ENone on success, error code on error [out]
     \param  buf          Buffer to store data read
     \param  size         Buffer size in bytes
     \param  address      Pointer to store source socket address, NULL to ignore
     \param  address_len  Pointer to address buffer length, updated to actual length used
     \param  flags        Low-level flags to use with socket
     \return              Size actually read in bytes, 0 on end-of-file or error
    */
    ulong readfrom(Error& err, void* buf, ulong size, struct sockaddr* address=NULL, socklen_t* address_len=NULL, int flags=0) {
        if (handle == INVALID) {
            err   = EClosed;
            errno = ENOTCONN;
            return 0;
        }
        if (size > SSIZE_MAX)
            size = SSIZE_MAX;
        ssize_t result;
        for (;;) {
            if (timeout_ms > 0 && !SysLinuxIo::read_wait(err, handle, timeout_ms, autoresume))
                return 0;
            result = ::recvfrom(handle, buf, size, flags, address, address_len);
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
        return (ulong)result;
    }

    /** Wait until socket is ready to write.
     - Socket is ready to write when it's connected and there's room in the write buffer
     - Normally used with non-blocking I/O
     .
     \param  err         %Set to error code on failure, ETimeout on timeout
     \param  timeout_ms  Wait timeout in milliseconds, 0 for indefinite
     \return             Whether ready to write, false on error
    */
    bool write_wait(Error& err, ulong timeout_ms)
        { return SysLinuxIo::write_wait(err, handle, timeout_ms, autoresume); }

    // Documented by parent
    ulong write(Error& err, const void* buf, ulong size)
        { return SysLinuxIo::write(err, handle, buf, size, timeout_ms, autoresume); }

    /** Write message to device and socket address.
     - This is used with UDP sockets to send a packet directly to socket address
     - UDP packet size limit is usually just under 64 KB, depending on the protocol used
     .
     \param  err          Stores ENone on success, error code on error [out]
     \param  buf          Buffer to write from
     \param  size         Size to write in bytes, 0 sends an empty packet
     \param  address      Socket address to write to, must not be NULL
     \param  address_len  Socket address length to write to, must be positive
     \param  flags        Low-level flags to use with socket
     \return              Size actually written in bytes (should match size), 0 on error (check err)
    */
    ulong writeto(Error& err, const void* buf, ulong size, const struct sockaddr* address, socklen_t address_len, int flags=0) {
        if (handle == INVALID) {
            err   = EClosed;
            errno = ENOTCONN;
            return 0;
        }
        if (size > SSIZE_MAX) {
            err   = ESize;
            errno = EMSGSIZE;
            return -1;
        }
        ssize_t result;
        for (;;) {
            if (timeout_ms > 0 && !SysLinuxIo::write_wait(err, handle, timeout_ms, autoresume))
                return 0;
            result = ::sendto(handle, buf, size, flags, address, address_len);
            if (result == 0) {
                err = EFail;
                return 0;
            } else if (result < 0) {
                switch (errno) {
                    case EINTR:
                        if (autoresume)
                            continue;
                        err = ESignal; break;
                    case EACCES:       err = EAccess;   break;
                    case ENOSPC:       err = ESpace;    break;
                    case EFBIG:        err = ESize;     break;
                    case EFAULT:       err = EPtr;      break;
                    case ENOTCONN:     // fallthrough
                    case ENOTSOCK:     // fallthrough
                    case EPIPE:        // fallthrough
                    case EBADF:        err = EClosed;   break;
                #if EAGAIN != EWOULDBLOCK
                    case EAGAIN:       // fallthrough
                #endif
                    case EWOULDBLOCK:  err = ENonBlock; break;
                    case EOPNOTSUPP:   err = EInvalOp;  break;
                    case ECONNRESET:   err = EFail;     break;
                    case EDESTADDRREQ: err = ENotFound; break;
                    default:           err = EWrite;    break;
                }
                return 0;
            }
            break;
        }
        err = ENone;
        return (ulong)result;
    }

    /** Accept connection from listening socket.
     - Socket must be in listen mode, see listen()
     .
     \param  err             %Set to error code on failure, ETimeout on timeout
     \param  client_socket   Attached to connected client socket on success (unchanged on error)
     \param  client_address  Pointer to store client socket address on success (ignored on error), NULL to skip
     \return                 Whether connection successfully accepted, false on error
    */
    bool accept(Error& err, IoSocket& client_socket, SocketAddressBase* client_address=NULL) {
        if (handle == INVALID) {
            err   = EClosed;
            errno = ENOTCONN;
        }  else {
            if (SysLinuxIo::read_wait(err, handle, timeout_ms, autoresume)) {
                for (;;) {
                    int client_handle;
                    if (client_address != NULL)
                        client_handle = ::accept(handle, &((SocketAddress*)client_address)->addr, &client_address->addrlen);
                    else
                        client_handle = ::accept(handle, NULL, NULL);

                    if (client_handle > 0) {
                        client_socket.attach(client_handle);
                        err = ENone;
                        return true;
                    } else {
                        const int last_error = errno;
                        if (last_error == EINTR && autoresume)
                            continue;
                        err = get_socket_error(last_error);
                    }
                    break;
                }
            }
        }
        return false;
    }

    /** Accept connection from listening socket and set the new connection as non-blocking.
     - Socket must be in listen mode, see listen()
     - This is equivalent to accept() then set_nonblock() on the new connection, except may be more efficient on some systems (Linux)
     .
     \param  err             %Set to error code on failure, ETimeout on timeout
     \param  client_socket   Attached to connected client socket on success (unchanged on error)
     \param  client_address  Pointer to store client socket address on success (ignored on error), NULL to skip
     \return                 Whether connection successfully accepted, false on error
    */
    bool accept_nonblock(Error& err, IoSocket& client_socket, SocketAddressBase* client_address=NULL) {
        if (handle == INVALID) {
            err   = EClosed;
            errno = ENOTCONN;
            return false;
        }

    #if defined(__linux) && defined(SOCK_NONBLOCK) && defined(__GLIBC__) && (__GLIBC__ > 2 || (__GLIBC__ == 2 && __GLIBC_MINOR__ >= 10)) && !defined(EVO_SOCKET_NO_ACCEPT4)
        // Use GNU accept4() and set as non-blocking at the same time (less system calls) -- avoid this by defining EVO_SOCKET_NO_ACCEPT4
        for (;;) {
            int client_handle;
            if (client_address != NULL)
                client_handle = ::accept4(handle, &((SocketAddress*)client_address)->addr, &client_address->addrlen, SOCK_NONBLOCK);
            else
                client_handle = ::accept4(handle, NULL, NULL, SOCK_NONBLOCK);
            if (client_handle > 0) {
                client_socket.attach(client_handle);
                client_socket.nonblock = true;
                err = ENone;
                return true;
            } else {
                const int last_error = errno;
                if (last_error == EINTR && autoresume)
                    continue;
                err = get_socket_error(last_error);
            }
            break;
        }
    #else
        if (accept(err, client_socket, client_address)) {
            err = set_nonblock();
            return (err == ENone);
        }
    #endif
        return false;
    }

    /** Get socket option value.
     - This calls getsockopt() on the socket
     .
     \param  level    Option level, ex: SOL_SOCKET, IPPROTO_IP
     \param  optname  Option identifier, depends on level
     \param  buf      Buffer to store value
     \param  size     Buffer size, updated with value size stored in buffer
     \return          ENone on success, EInval on invalid value or size, EInvalOP on unknown optname, otherwise error code
    */
    Error getopt(int level, int optname, void* buf, uint& size) {
        if (size > (uint)std::numeric_limits<socklen_t>::max())
            size = (uint)std::numeric_limits<socklen_t>::max();
        socklen_t optlen = (socklen_t)size;
        int result = ::getsockopt(handle, level, optname, buf, &optlen);
        if (result < 0) {
            Error err;
            switch (errno) {
                case EBADF:       // fallthrough
                case ENOTSOCK:    err = EClosed;  break;
                case EFAULT:      err = EPtr;     break;
                case EINVAL:      err = EInval;   break;
                case ENOPROTOOPT: err = EInvalOp; break;
                default:          err = EUnknown; break;
            }
            return err;
        }
        size = (optlen > 0) ? (uint)optlen : 0;
        return ENone;
    }

    /** %Set socket option with new value.
     - This calls setsockopt() on the socket
     .
     \param  level    Option level, ex: SOL_SOCKET, IPPROTO_IP
     \param  optname  Option identifier, depends on level
     \param  buf      Buffer with value to set
     \param  size     Value size in buffer
     \return          ENone on success, EInval on invalid value or size, EInvalOP on unknown optname, otherwise error code
    */
    Error setopt(int level, int optname, const void* buf, uint size) {
        if (size > (uint)std::numeric_limits<socklen_t>::max()) {
            errno = EMSGSIZE;
            return ESize;
        }
        const socklen_t optlen = (socklen_t)size;
        int result = ::setsockopt(handle, level, optname, buf, optlen);
        if (result < 0) {
            Error err;
            switch (errno) {
                case EBADF:       // fallthrough
                case ENOTSOCK:    err = EClosed;  break;
                case EFAULT:      err = EPtr;     break;
                case EINVAL:      err = EInval;   break;
                case ENOPROTOOPT: err = EInvalOp; break;
                default:          err = EUnknown; break;
            }
            return err;
        }
        return ENone;
    }

    /** Write detailed error message with errno to output stream/string.
     - Must call right after the error, otherwise errno may be overwritten
     - This includes the system formatted message for errno
     .
     \param  out  Stream or String to write output to
     \param  err  Error code to format message for
     \return      out
    */
    template<class TOut>
    static TOut& errormsg_out(TOut& out, Error err) {
        const int lasterror = errno;
        out << errormsg_socket(err) << ListBase<char>(" (errno:", 8) << lasterror << ')';
        if (err != ENone && lasterror != 0) {
            const int BUF_SIZE = 256;
            char buf[BUF_SIZE];
            char* p;
            #if defined(__linux)
                p = strerror_r(lasterror, buf, BUF_SIZE);
            #else
                p = (strerror_r(lasterror, buf, BUF_SIZE) == 0 ? buf : NULL);
            #endif
            if (p && *p)
                out << ListBase<char>(": ", 2) << p;
        }
        return out;
    }

    bool autoresume;        ///< Whether to auto-resume I/O operation after signal received [Linux/Unix]
#endif

    /** Destructor. Calls close(). */
    ~IoSocket()
        { close(); }

    /** Get whether socket is open.
     \return  Whether socket is open
    */
    bool isopen() const
        { return (handle != INVALID); }

    /** Create and bind socket using address info and listen for connections.
     \param  err           %Set to ENone on success, EExist if address/port already used, otherwise set to error code
     \param  address_info  Pointer to addrinfo structure to bind to (first in linked list)
     \param  backlog       Listener queue backlog size
     \param  all           Whether to try all addresses in addrinfo until successful, false to just try the first address
     \return               Whether successful, false on error
    */
    bool listen(Error& err, struct addrinfo* address_info, int backlog=SOMAXCONN, bool all=true) {
        assert( address_info != NULL );
        close();
        err = EInval;
        uint best_state = 0; // Used to return error from loop that got closest to success: 0=null, 1=socket(), 2=bind()
        int  best_errno = 0; // Used to set errno so it matches err
        for (ulong counter=0; (counter == 0 || all) && address_info != NULL; address_info=address_info->ai_next, ++counter) {
            Error sock_err = create_socket(address_info->ai_family, address_info->ai_socktype, address_info->ai_protocol);
            if (sock_err != ENone) {
                if (best_state == 0) {
                    err        = sock_err;
                    best_errno = errno;
                }
                continue;
            }

            uint cur_state = 1;
            if (::bind(handle, address_info->ai_addr, (int)address_info->ai_addrlen) != SOCK_ERROR) {
                ++cur_state;
                if (::listen(handle, backlog) != SOCK_ERROR) {
                    err = ENone;
                    return true;
                }
            }
            if (cur_state >= best_state) {
                err        = IoSocket::get_socket_error();
                best_errno = errno;
                best_state = cur_state;
            }
            close();
        }
        errno = best_errno;
        return false;
    }

    /** Create and bind socket to address and listen for connections.
     \param  err          %Set to ENone on success, EExist if address/port already used, otherwise set to error code
     \param  address      Socket address to bind to, must not be NULL
     \param  address_len  Socket address length to bind to, must be positive
     \param  backlog      Listener queue backlog size
     \param  socktype     Socket type, usually SOCK_STREAM
     \param  protocol     Network protocol, usually 0 for default
     \return              Whether successful, false on error
    */
    bool listen(Error& err, struct sockaddr* address, socklen_t address_len, int backlog=SOMAXCONN, int socktype=SOCK_STREAM, int protocol=0) {
        assert( address != NULL );
        assert( address_len > 0 );
        close();
        err = create_socket(address->sa_family, socktype, protocol);
        if (err == ENone) {
            if (::bind(handle, address, address_len) != SOCK_ERROR) {
                if (::listen(handle, backlog) != SOCK_ERROR) {
                    err = ENone;
                    return true;
                }
            }
            err = IoSocket::get_socket_error();
            const int last_errno = errno;
            close();
            errno = last_errno;
        }
        return false;
    }

    /** Create and bind datagram socket to address.
     \param  err          %Set to ENone on success, EExist if address/port already used, otherwise set to error code
     \param  address      Socket address to bind to, must not be NULL
     \param  address_len  Socket address length to bind to, must be positive
     \param  socktype     Socket type
     \param  protocol     Network protocol, 0 for default
     \return              Whether successful, false on error
    */
    bool bind(Error& err, struct sockaddr* address, socklen_t address_len, int socktype=SOCK_DGRAM, int protocol=0) {
        assert( address != NULL );
        assert( address_len > 0 );
        close();
        err = create_socket(address->sa_family, socktype, protocol);
        if (err == ENone) {
            if (::bind(handle, address, address_len) != SOCK_ERROR) {
                err = ENone;
                return true;
            }
            err = IoSocket::get_socket_error();
            const int last_errno = errno;
            close();
            errno = last_errno;
        }
        return false;
    }

    /** Create unbound datagram socket for casting.
     \param  err       %Set to ENone on success, otherwise set to error code
     \param  family    Socket family type (AF_INET, AF_INET6)
     \param  socktype  Socket type, SOCK_DGRAM for datagram socket
     \param  protocol  Network protocol, 0 for default
     \return           Whether successful, false on error
    */
    bool cast(Error& err, int family=AF_INET, int socktype=SOCK_DGRAM, int protocol=0) {
        close();
        err = create_socket(family, socktype, protocol);
        if (err == ENone)
            return true;
        err = IoSocket::get_socket_error();
        const int last_errno = errno;
        close();
        errno = last_errno;
        return false;
    }

    /** Create and connect socket using address info.
     - In blocking mode this blocks while connecting, and could take a while for the implementation to timeout
     - In non-blocking mode this returns with err=ENonBlock -- use write_wait() to wait/poll
     - This may try to create/connect multiple sockets/addresses, if all fail then err is set according to the "best" case (closest to success)
       - This also sets system "errno" accordingly (even in Windows), so it matches err
       .
     .
     \param  err           %Set to ENone on success, otherwise set to error code
     \param  address_info  Pointer to addrinfo structure to use (first in linked list)
     \param  all           Whether to try all addresses in addrinfo until successful, false to just try the first address
     \return               Whether successful, false on error
    */
    bool connect(Error& err, struct addrinfo* address_info, bool all=true) {
        close();
        err = EInval;
        uint best_state = 0; // Used to return error from loop that got closest to success: 0=null, 1=socket()
        int  best_errno = 0; // Used to set errno so it matches err
        for (ulong counter=0; (counter == 0 || all) && address_info != NULL; address_info=address_info->ai_next, ++counter) {
            Error sock_err = create_socket(address_info->ai_family, address_info->ai_socktype, address_info->ai_protocol);
            if (sock_err != ENone) {
                if (best_state == 0) {
                    err        = sock_err;
                    best_errno = errno;
                }
                continue;
            }
            best_state = 1; // use error from last connect

            const int result = ::connect(handle, address_info->ai_addr, (int)address_info->ai_addrlen);
            if (result != SOCK_ERROR) {
                err = ENone;
                return true;
            }
            err = IoSocket::get_socket_error();
            close();
        }
        errno = best_errno;
        return false;
    }

    /** Create and connect socket using address.
     - In blocking mode this blocks while connecting, and could take a while for the implementation to timeout
     - In non-blocking mode this returns with err=ENonBlock -- use write_wait() to wait/poll
     .
     \param  err          %Set to ENone on success, otherwise set to error code
     \param  address      Socket address to connect to, must not be NULL
     \param  address_len  Socket address length to connect to, must be positive
     \param  socktype     Socket type, usually SOCK_STREAM
     \param  protocol     Network protocol, usually 0 for default
     \return              Whether successful, false on error
    */
    bool connect(Error& err, struct sockaddr* address, socklen_t address_len, int socktype=SOCK_STREAM, int protocol=0) {
        assert( address != NULL );
        assert( address_len > 0 );
        close();
        err = create_socket(address->sa_family, socktype, protocol);
        if (err == ENone) {
            if (::connect(handle, address, address_len) != SOCK_ERROR) {
                err = ENone;
                return true;
            }
            err = IoSocket::get_socket_error();
            const int last_errno = errno;
            close();
            errno = last_errno;
        }
        return false;
    }

    /** Attach new socket handle.
     - If socktype is tUNKNOWN, you won't get a client IP address from accept()
     .
     \param  socket  Socket handle to attach
    */
    void attach(Handle socket) {
        close();
        handle = socket;
    }

    /** Detach and return socket handle.
     \return  File handle
    */
    Handle detach() {
        Handle result = handle;
        handle = INVALID;
        return result;
    }

    Handle handle;          ///< Socket handle/descriptor
    bool   nonblock;        ///< Whether non-blocking I/O is enabled

    /** Initialize socket library.
     - This should be called near the beginning of the process to load the socket library
     - Implementation is OS specific, used in Windows to load WinSock DLL
    */
    static const Init& init() {
        static Init data;
        return data;
    }

    /** Not supported with sockets.
     \param  err  %Set to EInval  [out]
     \return      Always 0
    */
    ulongl pos(Error& err) {
        errno = EINVAL;
        err   = EInval;
        return 0;
    }

    /** Not supported with sockets.
     \param  err  %Set to EInval  [out]
     \return      Always 0
    */
    ulongl seek(Error& err, ulongl, Seek) {
        errno = EINVAL;
        err   = EInval;
        return 0;
    }

private:
#if defined(_WIN32)
    // Windows

    Error create_socket(int domain, int socktype, int protocol) {
        if ((handle=::socket(domain, socktype, protocol)) == INVALID) {
            return IoSocket::get_socket_error();
        } else if (nonblock)
            return set_nonblock(true);
        return ENone;
    }

    static Error get_socket_error() {
        Error err;
        const int code = WSAGetLastError();
        switch (code) {
            case WSAEACCES:          err = EAccess;   break;
            case WSANOTINITIALISED:  // fallthrough
            case WSAENOTSOCK:        // fallthrough
            case WSAENOTCONN:        // fallthrough
            case WSAEPROTONOSUPPORT: // fallthrough
            case WSAEPROTOTYPE:      // fallthrough
            case WSAESOCKTNOSUPPORT: // fallthrough
            case WSAEAFNOSUPPORT:    // fallthrough
            case WSAEFAULT:          // fallthrough
            case WSAEINVAL:          // fallthrough
            case WSAEADDRNOTAVAIL:   err = EInval;    break;
            case WSAEOPNOTSUPP:      err = EInvalOp;  break;
            case WSAEADDRINUSE:      err = EExist;    break;
            case WSAENOBUFS:         err = ESpace;    break;
            case WSAEMFILE:          err = ELimit;    break;
            case WSAEHOSTUNREACH:    // fallthrough
            case WSAENETUNREACH:     // fallthrough
            case WSAECONNRESET:      // fallthrough
            case WSAECONNREFUSED:    err = EFail;     break;
            case WSAETIMEDOUT:       err = ETimeout;  break;
            case WSAEALREADY:        // fallthrough
            case WSAEWOULDBLOCK:     err = ENonBlock; break;
            case WSAEISCONN:         err = ENone;     break;
            default:                 err = EUnknown;  break;
        }
        errno = code;
        return err;
    }

#else
    // Linux/Unix

    Error create_socket(int domain, int type, int protocol) {
    #if defined(SOCK_NONBLOCK)
        const int flags = (nonblock ? SOCK_NONBLOCK : 0);
    #else
        // Non-blocking I/O not supported on this system
        if (nonblock) {
            errno = EINVAL;
            return EInval;
        }
        const int flags = 0;
    #endif
        if ((handle=::socket(domain, type | flags, protocol)) == INVALID)
            return IoSocket::get_socket_error();
        return ENone;
    }

    static Error get_socket_error(int code) {
        Error err;
        switch (code) {
            case EPERM:           // fallthrough
            case EACCES:          err = EAccess;   break;
            case EINVAL:          // fallthrough
            case EPROTONOSUPPORT: // fallthrough
            case EPROTOTYPE:      // fallthrough
            case EBADF:           // fallthrough
            case EFAULT:          // fallthrough
            case ENOTSOCK:        // fallthrough
            case EAFNOSUPPORT:    err = EInval;    break;
            case EOPNOTSUPP:      err = EInvalOp;  break;
            case EADDRNOTAVAIL:   // fallthrough
            case EADDRINUSE:      err = EExist;    break;
            case ENOMEM:          // fallthrough
            case ENOBUFS:         err = ESpace;    break;
            case ENFILE:          // fallthrough
            case EMFILE:          err = ELimit;    break;
            case ENETUNREACH:     // fallthrough
            case ECONNABORTED:    // fallthrough
            case ECONNREFUSED:    err = EFail;     break;
            case ETIMEDOUT:       err = ETimeout;  break;
            case EINPROGRESS:     // fallthrough
            case EAGAIN:          // fallthrough
        #if EAGAIN != EWOULDBLOCK
            case EWOULDBLOCK:     // fallthrough
        #endif
            case EALREADY:        err = ENonBlock; break;
            case EINTR:           err = ESignal;   break;
            default:              err = EUnknown;  break;
        }
        return err;
    }

    static Error get_socket_error()
        { return IoSocket::get_socket_error(errno); }
#endif

    static const char* errormsg(Error error) {
        switch (error) {
            case EAccess:   return "Permission denied for socket (EAccess)";
            case EClosed:   return "Socket is closed (EClosed)";
            case EExist:    return "Socket address/port in use or not available (EExist)";
            case ESize:     return "Message too long (ESize)";
            case ELimit:    return "No more socket descriptors available (ELimit)";
            case ELength:   return "Host or address string too long (ELength)";
            case EFail:     return "Connection refused or reset (EFail)";
            case ENotFound: return "Address not found (ENotFound)";
            default: break;
        }
        return evo::errormsg(error);
    }
};

///////////////////////////////////////////////////////////////////////////////
//@}
}
#endif
