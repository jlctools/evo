// Evo C++ Library
/* Copyright 2018 Justin Crowell
Distributed under the BSD 2-Clause License -- see included file LICENSE.txt for details.
*/
///////////////////////////////////////////////////////////////////////////////
/** \file net_interface.h Evo network adapter interface wrappers. */
#pragma once
#ifndef INCL_net_interface_h
#define INCL_net_interface_h

#include "substring.h"
#include "impl/iter.h"
#if defined(_WIN32)
    #include <stdlib.h>
    #include <malloc.h>
    #include <Iphlpapi.h>

    #pragma comment(lib, "IPHLPAPI.lib")
#else
    #include <ifaddrs.h>
    #include <net/if.h>
#endif

namespace evo {
/** \addtogroup EvoIO */
//@{

///////////////////////////////////////////////////////////////////////////////

/** Query network adapter interfaces from OS.
 - Use Iter to iterate through interfaces and addresses
 - The same interface may be returned multiple times with different address (IPv4, IPv6, etc)
 - Destroying this invalidates all iterators using it

\par Example

\code
#include <evo/net_interface.h>
#include <evo/iosock.h>
#include <evo/string.h>
#include <evo/io.h>
using namespace evo;
static Console& c = con();

int main() {
    String ipstr;
    NetworkInterfaces interfaces;
    for (NetworkInterfaces::Iter iter(interfaces); iter; ++iter) {
        if (SocketAddressIp::check(iter->addr)) {
            SocketAddressIp addr(iter->addr);
            addr.format(ipstr.clear());
            c.out << iter->name << '\t' << ipstr << NL;
        }
    }
}
\endcode
*/
class NetworkInterfaces {
public:
#if defined(_WIN32)
    // Windows
    typedef IP_ADAPTER_ADDRESSES SysItem;
#else
    // Linux/Unix
    typedef struct ifaddrs SysItem;   ///< System interface item
#endif

    /** Network iterface item. */
    struct Item {
        SubString        id;            ///< Interface ID string (terminated)
        SubString        name;          ///< Interface name (terminated)
        const SysItem*   detail;        ///< Interface system details (system specific)
        #if defined(_WIN32)
            // Windows only
            IP_ADAPTER_UNICAST_ADDRESS* detail_unicast; ///< Current interface unicast address (used internally)
            String                      detail_name;    ///< String for storing converted friendly name as UTF-8 (used internally)
        #endif
        uint32           scope_id;      ///< Interface scope ID for IPv6
        struct sockaddr* addr;          ///< Interface address
        bool             active;        ///< Whether interface is active

        /** Constructor. */
        Item() {
            detail   = NULL;
            #if defined(_WIN32)
                detail_unicast = NULL;
            #endif
            scope_id = 0;
            addr     = NULL;
            active   = false;
        }
    
    private:
        // Disable copying
        Item(const Item&);
        Item& operator=(const Item&);
    };

    /** Constructor queries OS for interfaces.
     - Use Iter to iterate through interfaces, which will reference allocated interface data
     .
     \param  family  Filter by address family, AF_UNSPEC for all
     \param  active  Find active interfaces, false for all interfaces
    */
    NetworkInterfaces(int family=AF_UNSPEC, bool active=true) {
        ptr_    = NULL;
        family_ = family;
        active_ = active;
        init();
    }

    /** Destructor frees interface data.
     - Any iterators referencing interface data are invalidated
    */
    ~NetworkInterfaces()
        { free(); }

    // Iterator support types
    /** \cond impl */
    typedef Item IterKey;
    typedef Item IterItem;
    typedef uint Size;
    /** \endcond */

    typedef IteratorFw<NetworkInterfaces>::Const Iter;      ///< Iterator (const) - IteratorFw

    // Iterator support methods
    /** \cond impl */
    void iterInitMutable()
        { }

    const IterItem* iterFirst(IterKey& key) const {
        const IterItem* result;
        if (ptr_ == NULL) {
            key.detail = NULL;
            result     = NULL;
        } else {
            result = item_get(key, ptr_);
            key.detail = ptr_;
            if (family_ != AF_UNSPEC && (key.addr == NULL || key.addr->sa_family != family_))
                result = iterNext(key);
            else
                result = &key;
        }
        return result;
    }

    const IterItem* iterNext(IterKey& key) const {
        for (;;) {
            if (key.detail != NULL) {
                if (item_next_address(key)) {
                    if (family_ != AF_UNSPEC && (key.addr == NULL || key.addr->sa_family != family_))
                        continue;
                    return &key;
                }
                if (item_next(key)) {
                    item_get(key, key.detail);
                    if (family_ != AF_UNSPEC && (key.addr == NULL || key.addr->sa_family != family_))
                        continue;
                    if (active_ && !item_active(key.detail))
                        continue;
                    return &key;
                }
            }
            break;
        }
        return NULL;
    }
    /** \endcond */

private:
    SysItem* ptr_;
    int      family_;
    bool     active_;

#if defined(_WIN32)
    // Windows
    void init() {
        const ULONG flags = GAA_FLAG_SKIP_ANYCAST | GAA_FLAG_SKIP_MULTICAST | GAA_FLAG_SKIP_DNS_SERVER;
        ULONG family = AF_UNSPEC;
        if (family_ == AF_INET || family_ == AF_INET6)
            family = family_;

        const uint MAX_TRIES = 6;   // try a few times since required size can change between queries
        ULONG size = 15360;         // 15 KB to start, grow if needed
        ptr_ = (SysItem*)::malloc(size);

        ULONG result = GetAdaptersAddresses(family, flags, NULL, ptr_, &size);
        for (uint i=0; result == ERROR_BUFFER_OVERFLOW && i < MAX_TRIES; ++i) {
            ptr_ = (SysItem*)::realloc(ptr_, size);
            result = GetAdaptersAddresses(AF_UNSPEC, 0, NULL, ptr_, &size);
        }

        if (result != ERROR_SUCCESS)
            free();
    }

    void free() {
        if (ptr_ != NULL) {
            ::free(ptr_);
            ptr_ = NULL;
        }
    }

    static bool item_next_address(Item& item) {
        if (item.detail_unicast != NULL && (item.detail_unicast = item.detail_unicast->Next) != NULL) {
            item.addr = item.detail_unicast->Address.lpSockaddr;
            return true;
        }
        return false;
    }

    static bool item_next(Item& item) {
        if (item.detail != NULL)
            return ((item.detail = item.detail->Next) != NULL);
        return false;
    }

    static bool item_active(const SysItem* ptr)
        { return ptr->OperStatus & IfOperStatusUp; }

    static const Item* item_get(Item& item, const SysItem* ptr) {
        item.detail      = ptr;
        item.id          = ptr->AdapterName;
        item.detail_name.set_win32(ptr->FriendlyName);
        item.name        = item.detail_name;
        item.scope_id    = ptr->Ipv6IfIndex;
        item.active      = item_active(ptr);
        if ((item.detail_unicast = ptr->FirstUnicastAddress) != NULL)
            item.addr = item.detail_unicast->Address.lpSockaddr;
        else
            item.addr = NULL;
        return &item;
    }
#else
    // Linux/Unix
    void init() {
        if (::getifaddrs(&ptr_) != 0)
            ptr_ = NULL;
    }

    void free() {
        if (ptr_ != NULL) {
            ::freeifaddrs(ptr_);
            ptr_ = NULL;
        }
    }

    static bool item_next_address(Item&)
        { return false; }

    static bool item_next(Item& item) {
        if (item.detail != NULL)
            return ((item.detail = item.detail->ifa_next) != NULL);
        return false;
    }

    static bool item_active(const SysItem* ptr)
        { return ptr->ifa_flags & IFF_UP; }

    static const Item* item_get(Item& item, const SysItem* ptr) {
        item.detail   = ptr;
        item.id       = ptr->ifa_name;
        item.name     = ptr->ifa_name;
        item.scope_id = if_nametoindex(ptr->ifa_name);
        item.addr     = ptr->ifa_addr;
        item.active   = item_active(ptr);
        return &item;
    }
#endif
};

///////////////////////////////////////////////////////////////////////////////
//@}
}
#endif
