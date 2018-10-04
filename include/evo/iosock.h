// Evo C++ Library
/* Copyright 2018 Justin Crowell
Distributed under the BSD 2-Clause License -- see included file LICENSE.txt for details.
*/
///////////////////////////////////////////////////////////////////////////////
/** \file iosock.h Evo Input/Output Socket streams. */
#pragma once
#ifndef INCL_evo_iosock_h
#define INCL_evo_iosock_h

#include "io.h"
#include "impl/sysio_sock.h"
#include "substring.h"
#if !defined(_WIN32)
    #include <net/if.h>
	#include <sys/select.h>
#endif

namespace evo {
/** \addtogroup EvoIO */
//@{

///////////////////////////////////////////////////////////////////////////////

/** TCP/IP socket address.
 - Holds either IPv4 or IPv6 address
*/
struct SocketAddressIp : public SocketAddressBase {
    static const uint MAX_INET4_STRLEN = 22;        ///< Max IPv4 string length with port num + terminator
    static const uint MAX_INET6_STRLEN = 65;        ///< Max IPv6 string length with IPv4 tunneling, brackets, zone/scope ID, and port num + terminator

    static const socklen_t MAX_SIZE = sizeof(sockaddr_in6);    ///< Max socket address size used here

    union {
        struct sockaddr     addr;       ///< Generic address structure
        struct sockaddr_in  addr_ip4;   ///< IPv4 address structure (union with addr)
        struct sockaddr_in6 addr_ip6;   ///< IPv6 address structure (union with addr)
    };

    /** Default constructor sets as empty/invalid. */
    SocketAddressIp()
        { clear(); }

    /** Copy constructor.
     \param  src  Source IP address to copy
    */
    SocketAddressIp(const SocketAddressIp& src) {
        memcpy(&addr_ip6, &src.addr_ip6, sizeof(sockaddr_in6));
        addrlen = src.addrlen;
    }

    /** Constructor to copy from an IPv4 address.
     \param  ptr  IP address pointer
    */
    SocketAddressIp(struct sockaddr_in* ptr) {
        if (ptr == NULL)
            clear();
        else
            memcpy(&addr, ptr, (addrlen=sizeof(sockaddr)));
    }

    /** Constructor to copy from an IPv6 address.
     \param  ptr  IP address pointer
    */
    SocketAddressIp(struct sockaddr_in6* ptr) {
        if (ptr == NULL)
            clear();
        else
            memcpy(&addr_ip6, ptr, (addrlen=sizeof(sockaddr_in6)));
    }

    /** Constructor for wildcard address.
     \param  port  Port number
     \param  ip6   Whether to use IPv6, false for IPv4
    */
    SocketAddressIp(ushort port, bool ip6=false) {
        clear();
        if (ip6) {
            addr_ip6.sin6_family = AF_INET6;
            addr_ip6.sin6_addr   = in6addr_any;
            addr_ip6.sin6_port   = htons(port);
        } else {
            addr_ip4.sin_family      = AF_INET;
            addr_ip4.sin_addr.s_addr = INADDR_ANY;
            addr_ip4.sin_port        = htons(port);
        }
    }

    /** Constructor to copy from addrinfo structure.
     \param  ptr  Address info pointer to get IP address
    */
    SocketAddressIp(struct addrinfo* ptr)
        { set(ptr); }

    /** Constructor to copy IP address from sockaddr structure.
     - This is set to null if ptr isn't a valid and supported IP address
     .
     \param  ptr  Address pointer to get IP address
    */
    SocketAddressIp(struct sockaddr* ptr) {
        clear();
        if (ptr != NULL) {
            switch (ptr->sa_family) {
                case AF_INET:  set((struct sockaddr_in*)ptr);  break;
                case AF_INET6: set((struct sockaddr_in6*)ptr); break;
            }
        }
    }

    // Documented by parent
    void set_maxsize()
        { addrlen = MAX_SIZE; }

    /** Get whether valid.
     \return  Whether valid
    */
    bool valid() const
        { return (addr.sa_family != 0); }

    /** Clear current address.
     \return  This
    */
    SocketAddressIp& clear() {
        memset(&addr, 0, (addrlen=sizeof(MAX_SIZE)));
        return *this;
    }

    /** %Set as copy of IP address.
     \param  src  Source IP address to copy
     \return      This
    */
    SocketAddressIp& set(const SocketAddressIp& src) {
        memcpy(&addr_ip6, &src.addr_ip6, sizeof(struct sockaddr_in6));
        addrlen = src.addrlen;
        return *this;
    }

    /** %Set as copy of an IPv4 address.
     \param  ptr  IP address pointer
     \return      This
    */
    SocketAddressIp& set(struct sockaddr_in* ptr) {
        if (ptr == NULL)
            clear();
        else
            memcpy(&addr_ip4, ptr, (addrlen=sizeof(struct sockaddr_in)));
        return *this;
    }

    /** %Set as copy of an IPv6 address.
     \param  ptr  IP address pointer
     \return      This
    */
    SocketAddressIp& set(struct sockaddr_in6* ptr) {
        if (ptr == NULL)
            clear();
        else
            memcpy(&addr_ip6, ptr, (addrlen=sizeof(struct sockaddr_in6)));
        return *this;
    }

    /** %Set to address from addrinfo structure.
     \param  ptr  Address info pointer to get IP address
     \return      This
    */
    SocketAddressIp& set(struct addrinfo* ptr) {
        clear();
        if (ptr != NULL && ptr->ai_addr != NULL && ptr->ai_addrlen > 0) {
            switch (ptr->ai_family) {
                case AF_INET:   memcpy(&addr_ip4, ptr->ai_addr, (addrlen=sizeof(struct sockaddr_in)));  break;
                case AF_INET6:  memcpy(&addr_ip6, ptr->ai_addr, (addrlen=sizeof(struct sockaddr_in6))); break;
            }
        }
        return *this;
    }

    /** %Set port on current address.
     - This is ignored if no address is set
     .
     \param  port  Port number
     \return       This
    */
    SocketAddressIp& setport(ushort port) {
        switch (addr.sa_family) {
            case AF_INET:  addr_ip4.sin_port  = htons(port);  break;
            case AF_INET6: addr_ip6.sin6_port = htons(port);  break;
        }
        return *this;
    }

    /** %Set as wildcard IPv4 address.
     \param  port  Port number
     \return       This
    */
    SocketAddressIp& setany4(ushort port=0) {
        clear();
        addr_ip4.sin_family      = AF_INET;
        addr_ip4.sin_addr.s_addr = INADDR_ANY;
        addr_ip4.sin_port        = htons(port);
        return *this;
    }

    /** %Set as wildcard IPv6 address.
     \param  port  Port number
     \return       This
    */
    SocketAddressIp& setany6(ushort port=0) {
        clear();
        addr_ip6.sin6_family = AF_INET6;
        addr_ip6.sin6_addr   = in6addr_any;
        addr_ip6.sin6_port   = htons(port);
        return *this;
    }

    bool parse(const SubString& str, ushort port=0, int family=AF_UNSPEC) {
        clear();
        if (str.size() == 0)
            return false;
        const char* pstr = str.data();
        const char* pend = pstr + str.size();

        if (family == AF_UNSPEC) {
            // Auto-detect: Check first few chars for IPv4 syntax, fallback to IPv6
            int i = 0;
            for (const char* p=pstr; p < pend; ++p, ++i) {
                if (*p == '.') {
                    --i;
                    break;
                }
                if (*p < '0' || *p > '9') {
                    i = -1;
                    break;
                }
            }
            family = (i < 0 ? AF_INET6 : AF_INET);
        }

        switch (family) {
            case AF_INET: {
                if (str.size() >= MAX_INET4_STRLEN)
                    return false;
                uint32 ip = 0;
                ushort num, shift = 32;
                const char* p = pstr;
                for (uint i=0; i < 4; ++i) {
                    num = 0;
                    p = str_scan_decimal(num, p, pend);
                    if (p == NULL)
                        return false;
                    if (i < 3) {
                        if (p == pend || *p != '.')
                            return false;
                        ++p;
                    }
                    if (num > 0xFF)
                        return false;
                    shift -= 8;
                    ip |= (num << shift);
                }
                addr_ip4.sin_addr.s_addr = htonl(ip);
                if (p != pend) {
                    if (*p != ':')
                        return false;
                    num = 0;
                    p = str_scan_decimal(num, p+1, pend);
                    if (p != pend)
                        return false;
                    addr_ip4.sin_port = htons(num);
                }
                if (port > 0)
                    addr_ip4.sin_port = htons(port);
                addrlen = sizeof(struct sockaddr_in);
                break;
            }
            case AF_INET6: {
                if (str.size() >= MAX_INET6_STRLEN)
                    return false;   // too long
                const char* p = pstr;

                bool brackets = false;
                if (*p == '[') {
                    brackets = true;
                    if (++p == pend)
                        return false;   // bad syntax
                }

                const uint MAX_NUMS  = 8;
                const uint MAX_IPLEN = MAX_NUMS * 2;
                uint head_len = MAX_IPLEN;
                uint i = 1;
                if (*p == ':') {
                    if (++p == pend || *p != ':')
                        return false;       // bad syntax
                    if (++p < pend && *p == ']') {
                        if (!brackets)
                            return false;   // unexpected ']'
                        i = MAX_NUMS;       // stop parse loop
                    }
                    head_len = 0;
                }

                // Parsing from first token
                const char* tokp;
                uchar* ip = addr_ip6.sin6_addr.s6_addr;
                uchar* p_ip = ip;
                ushort num;
                for (; p < pend && i <= MAX_NUMS; ++i) {
                    if (*p == ':') {
                        if (head_len < MAX_IPLEN)
                            return false;   // only 1 "::" allowed
                        head_len = (uint)(p_ip - ip);
                        if (++p == pend)
                            break;  // ended with "::"
                    }

                    tokp = p;
                    num  = 0;
                    if ((p = str_scan_hex(num, p, pend)) == NULL)
                        return false;   // bad num

                    if (p == pend) {
                        if (i < MAX_NUMS && head_len == MAX_IPLEN)
                            return false;   // too short, no "::"
                    } else if (*p == '%') {
                        i = MAX_NUMS;
                    } else if (i < MAX_NUMS) {
                        // Not last token, check delim
                        if (*p == '.') {
                            // IPv4 mapped, re-parse from current token
                            p = tokp;
                            if ((uint)(pend - p) > MAX_INET4_STRLEN + 1) // +1 for possible end bracket
                                return false;   // too long
                            const uint MAX_IP4LEN = 4;
                            for (uint j=1; j <= MAX_IP4LEN; ++j) {
                                num = 0;
                                if ((p = str_scan_decimal(num, p, pend)) == NULL)
                                    return false;
                                if (j < MAX_IP4LEN) {
                                    if (p == pend || *p != '.')
                                        return false;   // too short or bad delim
                                    ++p;
                                }
                                if (num > 0xFF)
                                    return false;   // num out of range
                                *p_ip = (uchar)num;
                                ++p_ip;
                            }
                            break;
                        } else if (*p == ']') {
                            if (!brackets)
                                return false;   // unexpected ']'
                            *p_ip   = (uchar)((num >> 8) & 0xFF);
                            *++p_ip = (uchar)(num & 0xFF);
                            ++p_ip;
                            break;
                        }  else if (*p != ':')
                            return false;   // bad delim
                        ++p;
                    }
                    *p_ip   = (uchar)((num >> 8) & 0xFF);
                    *++p_ip = (uchar)(num & 0xFF);
                    ++p_ip;
                }

                // Adjust for 0-compression
                if (head_len < MAX_IPLEN) {
                    uint ip_mov_len = (uint)(p_ip - ip);
                    if (ip_mov_len >= MAX_IPLEN)
                        return false;   // can't use "::" on full IP
                    ip_mov_len -= head_len;
                    if (ip_mov_len > 0)
                        memmove(ip + MAX_IPLEN - ip_mov_len, ip + head_len, ip_mov_len);
                    memset(ip + head_len, 0, MAX_IPLEN - ip_mov_len - head_len);
                }

                // Zone/Scope ID
                if (p < pend && *p == '%') {
                    if (++p == pend)
                        return false;   // missing scope ID after delim
                #if !defined(_WIN32)
                    // Linux/Unix: Map interface name to ID
                    if (*p < '0' || *p > '9') {
                        const char* p0 = str_scan_to(p, pend, ']', ':', IF_NAMESIZE-1);
                        if (p0 == NULL)
                            return false; // interface name too long
                        const uint len = p0 - p;
                        char buf[IF_NAMESIZE];
                        memcpy(buf, p, len);
                        if ((i = if_nametoindex(buf)) != 0)
                            return false;   // bad interface name
                        addr_ip6.sin6_scope_id = i;
                        p = p0;
                    } else
                #endif
                    if ((p = str_scan_decimal(addr_ip6.sin6_scope_id, p, pend)) == NULL)
                        return false;   // bad scope ID
                }

                // End bracket
                if (brackets) {
                    if (p == pend || *p != ']')
                        return false;   // missing end ']'
                    ++p;
                }

                // Port number
                if (p < pend) {
                    if (*p != ':' || ++p == pend)
                        return false;
                    num = 0;
                    if ((p = str_scan_decimal(num, p, pend)) != pend)
                        return false;
                    addr_ip6.sin6_port = htons(num);
                }
                if (port > 0)
                    addr_ip6.sin6_port = htons(port);

                addrlen = sizeof(struct sockaddr_in6);
                break;
            }
            default:
                return false;
        }
        addr.sa_family = (ushort)family;
        return true;
    }

    /** Format IP address to given string (appended).
     \param  str  %String to format to, unchanged if IP not valid
     \return      Whether successful, false if IP not valid
    */
    bool format(String& str) const
        { return SocketAddressIp::format_addr(str, this); }

    /** Format IP address to given string (appended).
     \param  str      %String to format to, unchanged if IP not valid
     \param  address  Address to format
     \return          Whether successful, false if not a valid IP address
    */
    static bool format_addr(String& str, const SocketAddressBase* address) {
        if (address != NULL) {
            const SocketAddress* address_base = (SocketAddress*)address;
            switch (address_base->addr.sa_family) {
                case AF_INET: {
                    struct sockaddr_in* sock_addr = (struct sockaddr_in*)&address_base->addr;
                    if (sock_addr->sin_port > 0) {
                        const ushort port = ntohs(sock_addr->sin_port);
                        char* buf = str.advWrite(MAX_INET4_STRLEN); // Add port and delims: addr:port
                        if (inet_ntop(AF_INET, (void*)&sock_addr->sin_addr, buf, MAX_INET4_STRLEN) != NULL) {
                            String::Size addrlen = (String::Size)strlen(buf);
                            buf[addrlen++] = ':';
                            addrlen += impl::fnum(buf + addrlen + IntegerT<ushort>::digits(port, 10), port, 10);
                            str.advWriteDone(addrlen);
                            return true;
                        }
                    } else {
                        char* buf = str.advWrite(MAX_INET4_STRLEN);
                        if (inet_ntop(AF_INET, (void*)&sock_addr->sin_addr, buf, MAX_INET4_STRLEN) != NULL) {
                            str.advWriteDone((String::Size)strlen(buf));
                            return true;
                        }
                    }
                    break;  // cov: can't mock error
                }
                case AF_INET6: {
                    struct sockaddr_in6* sock_addr = (struct sockaddr_in6*)&address_base->addr;
                    if (sock_addr->sin6_port > 0) {
                        const ushort port = ntohs(sock_addr->sin6_port);
                        char* buf = str.advWrite(MAX_INET6_STRLEN); // Add port and delims: [addr]:port
                        *buf = '[';
                        if (inet_ntop(AF_INET6, (void*)&sock_addr->sin6_addr, buf+1, MAX_INET6_STRLEN-1) != NULL) {
                            String::Size addrlen = (String::Size)strlen(buf+1) + 1; // includes '['
                            buf[addrlen++] = ']';
                            buf[addrlen++] = ':';
                            addrlen += impl::fnum(buf + addrlen + IntegerT<ushort>::digits(port, 10), port, 10);
                            str.advWriteDone(addrlen);
                            return true;
                        }
                    } else {
                        char* buf = str.advWrite(MAX_INET6_STRLEN);
                        if (inet_ntop(AF_INET6, (void*)&sock_addr->sin6_addr, buf, MAX_INET6_STRLEN) != NULL) {
                            str.advWriteDone((String::Size)strlen(buf));
                            return true;
                        }
                    }
                    break;  // cov: can't mock error
                }
            }
        }
        return false;
    }

    /** Check if address struct holds a supported IP address type.
     \param  addr  Address pointer, NULL for none
     \return       Whether addr is valid and is a supported IP address type
    */
    static bool check(struct sockaddr* addr) {
        if (addr != NULL) {
            switch (addr->sa_family) {
                case AF_INET:  return true;
                case AF_INET6: return true;
            }
        }
        return false;
    }
};

///////////////////////////////////////////////////////////////////////////////

#if !defined(_WIN32)
/** Unix Domain socket address (linux/unix).
 - Not supported in Windows
*/
struct SocketAddressUnix : public SocketAddressBase {
    static const socklen_t MAX_SIZE = sizeof(sockaddr_un);  ///< Max socket address size used here

    union {
        struct sockaddr    addr;        ///< Generic address structure
        struct sockaddr_un addr_unix;   ///< Unix Domain address structure (union with addr)
    };

    /** Default constructor sets as empty/invalid. */
    SocketAddressUnix()
        { clear(); }

    /** Copy constructor.
     \param  src  Source address to copy
    */
    SocketAddressUnix(const SocketAddressUnix& src) {
        memcpy(&addr_unix, &src.addr_unix, sizeof(sockaddr_un));
        addrlen = src.addrlen;
    }

    /** Constructor to copy from address.
     \param  ptr  Address pointer
    */
    SocketAddressUnix(struct sockaddr_un* ptr)
        { set(ptr); }

    /** Constructor to copy Unix Domain socket address from sockaddr structure.
     - This is set to null if ptr isn't a valid and supported Unix Domain socket address
     .
     \param  ptr  Address pointer to get IP address
    */
    SocketAddressUnix(struct sockaddr* ptr) {
        clear();
        if (ptr != NULL && ptr->sa_family == AF_UNIX)
            set((struct sockaddr_un*)ptr);
    }

    // Documented by parent
    void set_maxsize()
        { addrlen = MAX_SIZE; }

    /** Get whether valid.
     \return  Whether valid
    */
    bool valid() const
        { return (addr.sa_family != 0); }

    /** Clear current address.
     \return  This
    */
    SocketAddressUnix& clear() {
        memset(&addr, 0, (addrlen=sizeof(MAX_SIZE)));
        return *this;
    }

    /** %Set as copy of address.
     \param  src  Source address to copy
     \return      This
    */
    SocketAddressUnix& set(const SocketAddressUnix& src) {
        memcpy(&addr_unix, &src.addr_unix, (addrlen=src.addrlen));
        return *this;
    }

    /** %Set as copy of address.
     \param  ptr  Address pointer
     \return      This
    */
    SocketAddressUnix& set(struct sockaddr_un* ptr) {
        if (ptr == NULL || ptr->sun_family != AF_UNIX)
            clear();
        else
            memcpy(&addr_unix, ptr, (addrlen=offsetof(struct sockaddr_un, sun_path) + strlen(ptr->sun_path)));
        return *this;
    }

    /** Parse Unix Domain socket address.
     - The address is just a path, so this really just copies the path
     .
     \param  path  Unix Domain socket path
     \return       Whether successful, false if path is too long
    */
    bool parse(const SubString& path) {
        const SubString::Size pathlen = path.size();
        if (pathlen+1 < sizeof(addr_unix.sun_path)) {
            bzero((char*)&addr_unix, sizeof(addr_unix));
            addr_unix.sun_family = AF_UNIX;

            memcpy(addr_unix.sun_path, path.data(), pathlen);
            addr_unix.sun_path[pathlen] = '\0';
            
            addrlen = pathlen + offsetof(struct sockaddr_un, sun_path);
            return true;
        }
        return false;
    }

    /** Format Unix Domain socket address to given string (appended).
     - The address is just a path, so this really just copies the path
     .
     \param  str  %String to format to, unchanged on error
     \return      Whether successful, false on error (address not valid)
    */
    bool format(String& str) const
        { return SocketAddressUnix::format_addr(str, this); }

    /** Format Unix Domain socket address to given string (appended).
     - The address is just a path, so this really just copies the path
     .
     \param  str      %String to format to, unchanged on error
     \param  address  Unix Domain socket address to format
     \return          Whether successful, false if not a valid Unix Domain socket address
    */
    static bool format_addr(String& str, const SocketAddressBase* address) {
        if (address != NULL) {
            const SocketAddress* address_base = (SocketAddress*)address;
            if (address_base->addr.sa_family == AF_UNIX) {
                const struct sockaddr_un* address_unix = (struct sockaddr_un*)&address_base->addr;
                str.copy(address_unix->sun_path, address_base->addrlen - offsetof(struct sockaddr_un, sun_path));
                return true;
            }
        }
        return false;
    }

    /** Check if address struct holds a supported Unix Domain socket address type.
     \param  addr  Address pointer, NULL for none
     \return       Whether addr is valid and is a supported IP address type
    */
    static bool check(struct sockaddr* addr) {
        if (addr != NULL && addr->sa_family == AF_UNIX)
            return true;
        return false;
    }
};
#endif

///////////////////////////////////////////////////////////////////////////////

/** %Socket I/O stream.
 - Use to read and write socket streams
 - Supports internet (TCP/IP) sockets
   - Linux/Unix: Supports Unix Domain sockets
 - This throws an ExceptionStream derived exception on error if exceptions are enabled, otherwise use operator!() or error() to check for error
   - Exceptions: ExceptionSocketIn, ExceptionSocketOut
 - See also: SocketCast

\par Methods

 - Constructor:
   - Socket(Newline, bool), Socket(bool)
 - TCP/IP Sockets:
   - listen_ip(const SubString&, ushort, int, int)
   - listen_ip(ushort, int, int)
   - connect_ip(const SubString&, ushort, int)
 - Unix Domain Sockets:
   - listen_ud()
   - connect_ud()
 - Connections:
   - isopen()
   - accept()
   - finish(), finish_in(), finish_out()
   - close()
 - Data:
   - readline(), readtext(), readbin()
     - bufread()
   - writeline(), writetext(), writechar(), writebin()
     - flush()
     - bufwrite()
   - \ref StreamFormatting "Stream Formatting"
 - Options:
   - get_timeout(), set_timeout()
   - get_resolve(), set_resolve()
   - get_opt(), get_opt_num()
   - set_opt(), set_opt_num()
 - Error handling:
   - ExceptionStream
   - operator!()
   - error()
   - errormsg_out()

\par Example for client

\code
#include <evo/iosock.h>
using namespace evo;

int main() {
    const ushort PORT = 12345;

    Socket client;
    client.connect_ip("localhost", PORT);
    // ...

    return 0;
}
\endcode

\par Example for server

\code
#include <evo/iosock.h>
using namespace evo;

int main() {
    const ushort PORT = 12345;

    Socket server, client;
    server.listen_ip(PORT);

    while (server.accept(client)) {
        // ...
        client.close();
    }

    return 0;
}
\endcode
*/
class Socket : public Stream<IoSocket> {
public:
    typedef Stream<IoSocket> Base;              ///< Base class alias (used internally)

    static const int BACKLOG_DEFAULT = 5;   ///< Default backlog queue size

    /** Constructor.
     - This initializes without opening a socket
       - Server: Use listen_ip() or listen_ud(), and accept() methods to receive connections
       - Client: Use connect_ip() or connect_ud() to connect to server
     .
     \param  nl          Default newline value to use for text reads/writes
     \param  exceptions  Whether to enable exceptions on error, default set by Evo config: EVO_EXCEPTIONS
    */
    Socket(Newline nl=NL_SYS, bool exceptions=EVO_EXCEPTIONS) : Base(nl), resolve_enable_(true)
        { excep(exceptions); }

    /** Constructor.
     - This initializes without opening a socket
       - Server: Use listen_ip() or listen_ud(), and accept() methods to receive connections
       - Client: Use connect_ip() or connect_ud() to connect to server
     .
     \param  exceptions  Whether to enable exceptions on error, default set by Evo config: EVO_EXCEPTIONS
    */
    Socket(bool exceptions) : Base(NL_SYS), resolve_enable_(true)
        { excep(exceptions); }
    
    /** Access low-level I/O device for socket.
     \return  Reference to underlying IoSocket
    */
    IoSocket& device()
        { return device_; }

    /** Get timeout for socket operations.
     - This applies to read, write, and accept calls, but doesn't apply to connect calls
     .
     \return  Timeout in milliseconds, 0 for indefinite
    */
    ulong get_timeout() const
        { return device_.timeout_ms; }

    /** %Set timeout for socket operations.
     - This applies to read, write, and accept calls, but doesn't apply to connect calls
     .
     \param  timeout_ms  Timeout in milliseconds, 0 for indefinite
     \return             This
    */
    Socket& set_timeout(ulong timeout_ms)
        { device_.timeout_ms = timeout_ms; return *this; }

    /** Get whether full resolver is used when resolving addresses.
     - This sets how listen and connect methods resolve addresses:
       - Enabled means listen and connect methods may call external services (like DNS) to resolve an address, which may block
       - Otherwise addresses passed to listen and connect methods are assumed to be numeric, so simply need conversion to resolve
     - Full resolving is enabled by default
     .
     \return  Whether full resolving enabled
    */
    bool get_resolve() const
        { return resolve_enable_; }

    /** %Set whether full resolver is used when resolving addresses.
     - This sets how listen and connect methods resolve addresses:
       - Enabled means listen and connect methods may call external services (like DNS) to resolve an address, which may block
       - Otherwise addresses passed to listen and connect methods are assumed to be numeric, so simply need conversion to resolve
     - Full resolving is enabled by default
     .
     \param  enable  True to enable full resolving, false to disable
     \return         This
    */
    Socket& set_resolve(bool enable) {
        resolve_enable_ = enable;
        return *this;
    }

    /** Get socket option value.
     - This calls getsockopt() to store option value in given buffer
       - Linux:   http://man7.org/linux/man-pages/man2/getsockopt.2.html
       - Windows: https://msdn.microsoft.com/en-us/library/windows/desktop/ms738544.aspx
     .
     \param  level    Protocol level ID (SOL_SOCKET, IPPROTO_IP, etc)
     \param  optname  Option name ID
     \param  buf      Buffer to store option value
     \return          Value pointer, NULL on error -- must be correct type for level and optname
     \tparam  T  Option value type -- results are system dependent on incorrect type for option, may return an error or invalid value
    */
    template<class T>
    T* get_opt(int level, int optname, T* buf) {
        uint size = sizeof(T);
        error_ = device_.getopt(level, optname, buf, size);
        if (error_ == ENone)
            return buf;
        EVO_THROW_ERR_CHECK(ExceptionSocketConfig, "Socket getopt() failed", error_, excep_);
        return NULL;
    }

    /** Get socket option flag or numeric value.
     - This calls getsockopt()
       - Linux:   http://man7.org/linux/man-pages/man2/getsockopt.2.html
       - Windows: https://msdn.microsoft.com/en-us/library/windows/desktop/ms738544.aspx
     - Flags should be 0 for false, non-zero (usually 1) for true
     - Results are system dependent if option is not bool or numeric -- may return an error or invalid value like 0
     .
     \param  level    Protocol level ID (SOL_SOCKET, IPPROTO_IP, etc)
     \param  optname  Option name ID
     \return          Value, null on error
    */
    Long get_opt_num(int level, int optname) {
        IoSocket::OptNum num = 0;
        uint size = sizeof(num);
        error_ = device_.getopt(level, optname, &num, size);

        Long result;
        if (error_ == ENone)
            result = (long)num;
        else
            { EVO_THROW_ERR_CHECK(ExceptionSocketConfig, "Socket getopt() failed", error_, excep_); }
        return result;
    }

    /** %Set socket option value.
     - This calls setsockopt() to set new option value
       - Linux:   http://man7.org/linux/man-pages/man2/getsockopt.2.html
       - Windows: https://msdn.microsoft.com/en-us/library/windows/desktop/ms740476.aspx
     .
     \param  level    Protocol level ID (SOL_SOCKET, IPPROTO_IP, etc)
     \param  optname  Option name ID
     \param  val      Value to set -- must be correct type for level and optname
     \return          Whether successful, false on error
     \tparam  T  Option value type
    */
    template<class T>
    bool set_opt(int level, int optname, const T& val) {
        error_ = device_.setopt(level, optname, (T*)&val, sizeof(T));
        if (error_ != ENone) {
            EVO_THROW_ERR_CHECK(ExceptionSocketConfig, "Socket setopt() failed", error_, excep_);
            return false;
        }
        return true;
    }

    /** %Set socket option flag or numeric value.
     - This calls setsockopt()
       - Linux:   http://man7.org/linux/man-pages/man2/getsockopt.2.html
       - Windows: https://msdn.microsoft.com/en-us/library/windows/desktop/ms740476.aspx
       - Flags should be 0 for false, non-zero (usually 1) for true
     .
     \param  level    Protocol level ID (SOL_SOCKET, IPPROTO_IP, etc)
     \param  optname  Option name ID
     \param  val      Value to set
     \return          Whether successful, false on error
    */
    bool set_opt_num(int level, int optname, long val) {
        IoSocket::OptNum num = (IoSocket::OptNum)val;
        return set_opt(level, optname, num);
    }

    /** Create and bind TCP socket on host interface and listen for connections.
     - Use SocketAddressIp* for client_address with accept()
     .
     \param  host     Host address string to bind to (terminated string)
     \param  port     Port number to bind to and listen on
     \param  family   Protocol family to use: AF_INET for IPv4, AF_INET6 for IPv6
     \param  backlog  Listener queue backlog size
     \return          Whether successful, false on error
    */
    bool listen_ip(const SubString& host, ushort port, int family=AF_INET, int backlog=BACKLOG_DEFAULT) {
        SocketAddressInfo address_info(family);
        if (resolve_enable_)
            error_ = address_info.resolve(host, port);
        else
            error_ = address_info.convert(host, port);
        if (error_ == ENone && device_.listen(error_, address_info.ptr, backlog)) {
            owned_ = true;
            return true;
        }
        errno = address_info.code;
        EVO_THROW_ERR_CHECK(ExceptionSocketOpen, "Socket listen_ip() failed", error_, excep_);
        return false;
    }

    /** Create and bind TCP socket on all interfaces and listen for connections.
     \param  port     Port number to bind to and listen on
     \param  family   Protocol family to use: AF_INET for IPv4, AF_INET6 for IPv6
     \param  backlog  Listener queue backlog size
     \return          Whether successful, false on error
    */
    bool listen_ip(ushort port, int family=AF_INET, int backlog=BACKLOG_DEFAULT) {
        SocketAddressInfo address_info(family);
        error_ = address_info.resolve(NULL, port, AI_PASSIVE | AI_NUMERICSERV);
        if (error_ == ENone && device_.listen(error_, address_info.ptr, backlog)) {
            owned_ = true;
            return true;
        }
        errno = address_info.code;
        EVO_THROW_ERR_CHECK(ExceptionSocketOpen, "Socket listen_ip() failed", error_, excep_);
        return false;
    }

    /** Create and bind Unix Domain socket to file path and listen for connections (linux/unix).
     - Use SocketAddressUnix* for client_address with accept()
     - Not supported in Windows -- fails with EInval
     - Note that the path length limit is lower than normal file paths
       - Limit is usually between 92 - 108 (inclusive), depending on the system
       - This returns error ESize if path is too long
     .
     \param  path     File path for Unix Domain socket
     \param  backlog  Listener queue backlog size
     \return          Whether successful, false on error
    */
    bool listen_ud(const SubString& path, int backlog=BACKLOG_DEFAULT) {
        assert( path.size_ > 0 );
    #if defined(_WIN32)
        (void)path;     // prevent compiler warnings
        (void)backlog;
        errno  = EINVAL;
        error_ = EInval;
    #else
        SocketAddressUnix address;
        if (address.parse(path)) {
            if (device_.listen(error_, &address.addr, address.addrlen, backlog))
                return true;
        } else {
            errno  = ENAMETOOLONG;
            error_ = ESize;
        }
    #endif
        EVO_THROW_ERR_CHECK(ExceptionSocketOpen, "Socket listen_ud() failed", error_, excep_);
        return false;
    }

    /** Accept connection from listening socket.
     - Socket must be in listen mode, see: listen_ip(), listen_ud()
     - If client_address is specified, it must be the correct type for the socket, otherwise results are implementation dependent (may truncated the address or return an error)
     - See the listen method used for the expected socket address type
     .
     \param  client_socket   Attached to connected client socket on success (unchanged on error)
     \param  client_address  Pointer used to set connected client address on success (invalidated on error), NULL to skip
     \return                 Whether connection successfully accepted, false on error
    */
    bool accept(Socket& client_socket, SocketAddressBase* client_address=NULL) {
        client_socket.close();
        if (client_address != NULL) {
            ((SocketAddress*)client_address)->addr.sa_family = 0;
            client_address->set_maxsize();
        }
        if (device_.accept(error_, client_socket.device_, client_address)) {
            client_socket.owned_ = true;
            return true;
        }
        EVO_THROW_ERR_CHECK(ExceptionSocketOpen, "Socket accept() failed", error_, excep_);
        return false;
    }

    /** Connect TCP socket to host address.
     - This blocks while connecting, and could take a while for the implementation to timeout
     - This resolves the host address (without lookup) and port and connects a socket to it
     .
     \param  host    Host address to resolve (terminated string)
     \param  port    Port number to use
     \param  family  Protocol family to use: AF_INET for IPv4, AF_INET6 for IPv6
     \return         Whether successful, false on error
    */
    bool connect_ip(const char* host, ushort port, int family=AF_INET) {
        close();
        SocketAddressInfo address_info(family);
        if (resolve_enable_)
            error_ = address_info.resolve(host, port);
        else
            error_ = address_info.convert(host, port);
        if (error_ == ENone && device_.connect(error_, address_info.ptr)) {
            owned_ = true;
            return true;
        }
        errno = address_info.code;
        EVO_THROW_ERR_CHECK(ExceptionSocketOpen, "Socket connect_ip() failed", error_, excep_);
        return false;
    }

    /** Connect TCP socket to host address.
     - This blocks while connecting, and could take a while for the implementation to timeout
     - This resolves the host address (without lookup) and port and connects a socket to it
     .
     \param  host    Host address to resolve
     \param  port    Port number to use
     \param  family  Protocol family to use: AF_INET for IPv4, AF_INET6 for IPv6
     \return         Whether successful, false on error
    */
    bool connect_ip(const SubString& host, ushort port, int family=AF_INET) {
        close();
        SocketAddressInfo address_info(family);
        if (resolve_enable_)
            error_ = address_info.resolve(host, port);
        else
            error_ = address_info.convert(host, port);
        if (error_ == ENone && device_.connect(error_, address_info.ptr)) {
            owned_ = true;
            return true;
        }
        errno = address_info.code;
        EVO_THROW_ERR_CHECK(ExceptionSocketOpen, "Socket connect_ip() failed", error_, excep_);
        return false;
    }

    /** Connect to Unix Domain socket at file path (linux/unix).
     - Not supported in Windows -- fails with EInval
     - Note that the path length limit is lower than normal file paths
       - Limit is usually between 92 - 108 (exclusive), depending on the system
       - This returns error ESize if path is too long
     .
     \param  path     File path for Unix Domain socket
     \return          Whether successful, false on error
    */
    bool connect_ud(const SubString& path) {
        assert( path.size_ > 0 );
        close();
    #if defined(_WIN32)
        (void)path;     // prevent compiler warning
        errno  = EINVAL;
        error_ = EInval;
    #else
        SocketAddressUnix address;
        if (address.parse(path)) {
            if (device_.connect(error_, &address.addr, address.addrlen))
                return true;
        } else {
            errno  = ENAMETOOLONG;
            error_ = ESize;
        }
    #endif
        EVO_THROW_ERR_CHECK(ExceptionSocketOpen, "Socket connect_ud() failed", error_, excep_);
        return false;
    }

    /** Finish (shutdown) socket input and output (reads and writes).
     - This is a gracefull way to stop input and output at this point
       - Input already received is still readable
     - Only use under normal conditions (not on communication error)
     - This doesn't throw any exception (if enabled) on error since an error closing communication doesn't block progress
     .
     \return  Whether successfull, false on error (not connected, invalid, etc)
    */
    bool finish()
        { return device_.shutdown(); }

    /** Finish (shutdown) socket input (reads).
     - This is a gracefull way to stop input at this point
       - Input already received is still readable
     - Only use under normal conditions (not on communication error)
     - This doesn't throw any exception (if enabled) on error since an error closing communication doesn't block progress
     .
     \return  Whether successfull, false on error (not connected, invalid, etc)
    */
    bool finish_in()
        { return device_.shutdown(IoSocket::sIN); }

    /** Finish (shutdown) socket output (writes).
     - This is a gracefull way to stop output at this point
     - Only use under normal conditions (not on communication error)
     - This doesn't throw any exception (if enabled) on error since an error closing communication doesn't block progress
     .
     \return  Whether successfull, false on error (not connected, invalid, etc)
    */
    bool finish_out()
        { return device_.shutdown(IoSocket::sOUT); }

    /** Write detailed error message with errno to output stream/string.
     - Must call right after the error, otherwise errno may be out of date
     - This includes the system formatted message for errno, if applicable
     .
     \param  out  Stream or String to write output to
     \return      out
    */
    template<class TOut>
    TOut& errormsg_out(TOut& out)
        { return IoSocket::errormsg_out<>(out, error_); }

    /** Initialize socket library.
     - This should be called near the beginning of the process to load the socket library
     - Implementation is OS specific, used in Windows to load WinSock DLL
    */
    static void sysinit()
        { IoSocket::init(); }

private:
    bool resolve_enable_;

    // Disable copying
    Socket(const Socket&);
    Socket& operator=(const Socket&);
};

///////////////////////////////////////////////////////////////////////////////

/** %Socket for I/O casting (datagram/UDP).
 - Datagram protocols send messages directly in packets, no buffering is used
 - Message types include: unicast, multicast, and broadcast
 - See also: Socket

\par Methods

 - Constructor:
   - SocketCast(SocketAddressBase*, bool), SocketCast(bool)
 - State:
   - bind()
   - cast()
   - isopen()
   - close()
 - Data:
   - read(), readbin()
   - write(), writebin()
 - Options:
   - get_timeout(), set_timeout()
   - get_opt(), get_opt_num()
   - set_opt(), set_opt_num()
 - Error handling:
   - ExceptionStream
   - operator!()
   - error()
   - errormsg_out()
*/
class SocketCast : public IoBase {
public:
    /** Constructor.
     \param  exceptions  Whether to enable exceptions on error, default set by Evo config: EVO_EXCEPTIONS
    */
    SocketCast(bool exceptions=EVO_EXCEPTIONS) : target_address_(NULL)
        { excep(exceptions); }

    /** Constructor.
     \param  address     Default address to cast (write) to, NULL for none -- may be overridden by write()
     \param  exceptions  Whether to enable exceptions on error, default set by Evo config: EVO_EXCEPTIONS
    */
    SocketCast(SocketAddressBase* address, bool exceptions=EVO_EXCEPTIONS) : target_address_(address)
        { excep(exceptions); }

    /** Destructor. */
    ~SocketCast()
        { close(); }

    /** Get timeout for socket operations.
     - This applies to read, write, and accept calls, but doesn't apply to connect calls
     .
     \return  Timeout in milliseconds, 0 for indefinite
    */
    ulong get_timeout() const
        { return device_.timeout_ms; }

    /** %Set timeout for socket operations.
     - This applies to read, write, and accept calls, but doesn't apply to connect calls
     .
     \param  timeout_ms  Timeout in milliseconds, 0 for indefinite
     \return             This
    */
    SocketCast& set_timeout(ulong timeout_ms)
        { device_.timeout_ms = timeout_ms; return *this; }

    /** Get socket option value.
     - This calls getsockopt() to store option value in given buffer
       - Linux:   http://man7.org/linux/man-pages/man2/getsockopt.2.html
       - Windows: https://msdn.microsoft.com/en-us/library/windows/desktop/ms738544.aspx
     .
     \param  level    Protocol level ID (SOL_SOCKET, IPPROTO_IP, etc)
     \param  optname  Option name ID
     \param  buf      Buffer to store option value
     \return          Value pointer, NULL on error -- must be correct type for level and optname
     \tparam  T  Option value type -- results are system dependent on incorrect type for option, may return an error or invalid value
    */
    template<class T>
    T* get_opt(int level, int optname, T* buf) {
        uint size = sizeof(T);
        error_ = device_.getopt(level, optname, buf, size);
        if (error_ == ENone)
            return buf;
        else
            { EVO_THROW_ERR_CHECK(ExceptionSocketConfig, "SocketCast getopt() failed", error_, excep_); }
        return NULL;
    }

    /** Get socket option flag or numeric value.
     - This calls getsockopt()
       - Linux:   http://man7.org/linux/man-pages/man2/getsockopt.2.html
       - Windows: https://msdn.microsoft.com/en-us/library/windows/desktop/ms738544.aspx
     - Flags should be 0 for false, non-zero (usually 1) for true
     - Results are system dependent if option is not bool or numeric -- may return an error or invalid value like 0
     .
     \param  level    Protocol level ID (SOL_SOCKET, IPPROTO_IP, etc)
     \param  optname  Option name ID
     \return          Value, null on error
    */
    Long get_opt_num(int level, int optname) {
        IoSocket::OptNum num = 0;
        uint size = sizeof(num);
        error_ = device_.getopt(level, optname, &num, size);

        Long result;
        if (error_ == ENone)
            result = (long)num;
        else
            { EVO_THROW_ERR_CHECK(ExceptionSocketConfig, "SocketCast getopt() failed", error_, excep_); }
        return result;
    }

    /** %Set socket option value.
     - This calls setsockopt() to set new option value
       - Linux:   http://man7.org/linux/man-pages/man2/getsockopt.2.html
       - Windows: https://msdn.microsoft.com/en-us/library/windows/desktop/ms740476.aspx
     .
     \param  level    Protocol level ID (SOL_SOCKET, IPPROTO_IP, etc)
     \param  optname  Option name ID
     \param  val      Value to set -- must be correct type for level and optname
     \return          Whether successful, false on error
     \tparam  T  Option value type
    */
    template<class T>
    bool set_opt(int level, int optname, const T& val) {
        error_ = device_.setopt(level, optname, (T*)&val, sizeof(T));
        if (error_ != ENone) {
            EVO_THROW_ERR_CHECK(ExceptionSocketConfig, "SocketCast setopt() failed", error_, excep_);
            return false;
        }
        return true;
    }

    /** %Set socket option flag or numeric value.
     - This calls setsockopt()
       - Linux:   http://man7.org/linux/man-pages/man2/getsockopt.2.html
       - Windows: https://msdn.microsoft.com/en-us/library/windows/desktop/ms740476.aspx
       - Flags should be 0 for false, non-zero (usually 1) for true
     .
     \param  level    Protocol level ID (SOL_SOCKET, IPPROTO_IP, etc)
     \param  optname  Option name ID
     \param  val      Value to set
     \return          Whether successful, false on error
    */
    bool set_opt_num(int level, int optname, long val) {
        IoSocket::OptNum num = (IoSocket::OptNum)val;
        return set_opt(level, optname, num);
    }

    /** Create and bind datagram socket to address (read/write).
     \param  address   Address to bind to -- must be correct type for socket
     \param  socktype  Socket type value, defaults to standard UDP
     \param  protocol  Socket protocol, 0 for default
     \return           Whether successful, false on error
    */
    bool bind(const SocketAddressBase& address, int socktype=SOCK_DGRAM, int protocol=0) {
        if (!device_.bind(error_, &((SocketAddress&)address).addr, address.addrlen, socktype, protocol)) {
            EVO_THROW_ERR_CHECK(ExceptionSocketOpen, "SocketCast bind failed", error_, excep_);
            return false;
        }
        return true;
    }

    /** Create unbound datagram socket for casting (write only).
     - If address is specified, this will reference that pointer so it must remain valid
     - Unbound sockets can only cast (write only), use bind() for read/write
     .
     \param  address   Default address to cast (write) to, NULL for none -- may be overridden by write()
     \param  family    Socket family type (AF_INET, AF_INET6), AF_UNSPEC to use family from address (if not NULL), or default to AF_INET (IPv4)
     \param  socktype  Socket type value, defaults to standard UDP
     \param  protocol  Socket protocol, 0 for default
     \return           Whether successful, false on error
    */
    bool cast(const SocketAddressBase* address=NULL, int family=AF_UNSPEC, int socktype=SOCK_DGRAM, int protocol=0) {
        if (family == AF_UNSPEC) {
            if (address != NULL)
                family = ((SocketAddress*)address)->addr.sa_family;
            else
                family = AF_INET;
        }
        target_address_ = address;
        if (!device_.cast(error_, family, socktype, protocol)) {
            EVO_THROW_ERR_CHECK(ExceptionSocketOpen, "SocketCast create failed", error_, excep_);
            return false;
        }
        return true;
    }

    /** Close socket. */
    void close()
        { device_.close(); }

    /** Read message from socket.
     \param  buf      Buffer to store message
     \param  size     Buffer size in bytes
     \param  flags    Flags passed to recvfrom(), 0 for none
     \param  address  Pointer to store source address (must be correct type for socket), NULL to skip
     \return          Message size read, 0 on error
    */
    ulong read(void* buf, ulong size, int flags=0, SocketAddressBase* address=NULL) {
        if (address == NULL)
            return device_.readfrom(error_, buf, size, NULL, 0, flags);
        address->set_maxsize();
        const ulong result = device_.readfrom(error_, buf, size, &((SocketAddress*)address)->addr, &address->addrlen, flags);
        EVO_THROW_ERR_CHECK(ExceptionSocketIn, "SocketCast read failed", error_, (excep_ && result == 0 && error_ != ENone));
        return result;
    }

    // Documented by parent
    ulong readbin(void* buf, ulong size)
        { return read(buf, size); }

    /** Write message to socket.
     \param  buf      Buffer with message
     \param  size     Message size in bytes
     \param  flags    Flags passed to sendto(), 0 for none
     \param  address  Target address to write to, NULL for default set by set_target()
     \return          Size sent (should match size), 0 on error
    */
    ulong write(const void* buf, ulong size, int flags=0, const SocketAddressBase* address=NULL) {
        if (address == NULL) {
            if (target_address_ == NULL) {
                error_ = EInval;
                errno  = EINVAL;
                return 0;
            }
            address = target_address_;
        }
        const ulong result = device_.writeto(error_, buf, size, &((SocketAddress*)address)->addr, address->addrlen, flags);
        EVO_THROW_ERR_CHECK(ExceptionSocketOut, "SocketCast write failed", error_, (excep_ && result == 0 && error_ != ENone));
        return result;
    }

    // Documented by parent
    ulong writebin(const void* buf, ulong size)
        { return write(buf, size); }

    /** Write detailed error message with errno to output stream/string.
     - Must call right after the error, otherwise errno may be out of date
     - This includes the system formatted message for errno, if applicable
     .
     \param  out  Stream or String to write output to
     \return      out
    */
    template<class TOut>
    TOut& errormsg_out(TOut& out)
        { return IoSocket::errormsg_out<>(out, error_); }

private:
    const SocketAddressBase* target_address_;   ///< Paired default target address writes, NULL for none
    IoSocket                 device_;           ///< I/O device
};

///////////////////////////////////////////////////////////////////////////////
//@}
}
#endif
