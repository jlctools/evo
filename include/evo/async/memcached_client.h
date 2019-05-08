// Evo C++ Library
/* Copyright 2019 Justin Crowell
Distributed under the BSD 2-Clause License -- see included file LICENSE.txt for details.
*/
///////////////////////////////////////////////////////////////////////////////
/** \file memcached_client.h Evo Async Memached API, client class. */
#pragma once
#ifndef INCL_evo_api_memcached_client_h
#define INCL_evo_api_memcached_client_h

#include "memcached_common.h"
#include "../impl/systime.h"
#include "../ioasync_client.h"
#include "../strtok.h"
#include "../pair.h"

namespace evo {
namespace async {
/** \addtogroup EvoAsyncAPI */
//@{

///////////////////////////////////////////////////////////////////////////////

/** \cond impl */
namespace impl_memc {
    // Used in queue to track expected responses to client
    struct ClientQueueItem {
        enum Type {
            tNONE = 0,
            tSTORE,
            tINCREMENT,
            tDELETE,
            tTOUCH,
            tGET,
            tGET_CAS
        };

        Type   type;
        void*  on_reply;
        void*  on_error;
        String data;
        UInt64 data_num;
        bool   track_notfound;

        ClientQueueItem() : type(tNONE), on_reply(NULL), on_error(NULL), track_notfound(false)
            { }
        ClientQueueItem(const ClientQueueItem& src) : type(src.type), on_reply(src.on_reply), on_error(src.on_error), data(src.data), data_num(src.data_num), track_notfound(src.track_notfound)
            { }
        ClientQueueItem& operator=(const ClientQueueItem& src) {
            type     = src.type;
            on_reply = src.on_reply;
            on_error = src.on_error;
            swap(data, (String&)src.data);  // swap for thread safety and speed (unshared)
            data_num = src.data_num;
            track_notfound = src.track_notfound;
            return *this;
        }

        bool null() const {
            return (type == tNONE);
        }

        void set() {
            type = tNONE;
            on_reply = NULL;
            on_error = NULL;
            data.set();
            data_num.set();
            track_notfound = false;
        }
    };
}
/** \endcond */

///////////////////////////////////////////////////////////////////////////////

/** Implements %Memcached protocol for an async client.
 - See \ref Async for general overview
 .
Usage:
 - Implement handlers for desired response events:
   - This can be a general handler implementing async::MemcachedClient::OnEvent
   - or can be separate handlers for each request type: \link AsyncClient::OnConnect OnConnect\endlink, OnStore, OnIncrement, OnRemove, OnGet
   - Implement an \link AsyncClient::OnError OnError\endlink handler if desired
 - Instantiate a MemcachedClient
   - Call setup methods as needed: set_timeout(), set_on_connect(), set_on_error(), set_logger()
   - For non-blocking client: _Not yet implemented_
 - Call a connect method like connect_ip() to start a connection
   - Note that this doesn't block and requests are queued while connecting
 - Use requests methods to make requests, create and pass event handlers as needed -- not that some methods have alternate variants:
   - set(const SubString&, const SubString&, uint32, int64, uint64*, OnStore*, OnError*)
     - set(const SubString&, const SubString&, OnStore&, OnError*, uint32, int64, uint64*)
   - set_cas()
   - set_append(), set_prepend()
   - set_add(), set_replace()
   - increment(), incr(), decr()
   - remove()
   - touch()
   - get(), get_cas()
   - get_touch(), get_touch_cas()
 - For blocking client: call runlocal() to process all pending requests
 .

\par Example

\code
#include <evo/async/memcached_client.h>
#include <evo/io.h>
using namespace evo;

struct OnEvent : async::MemcachedClient::OnEvent {
    void on_connect() {
        con().out << "on_connect()" << NL;
    }

    void on_store(const SubString& key, Memcached::StoreResult result) {
        con().out << "on_store() " << key << ' ' << Memcached::StoreResultEnum::get_string(result) << NL;
    }

    void on_get(const SubString& key, const SubString& value, uint32 flags) {
        con().out << "on_get() " << key << " '" << value << "' " << flags << NL;
    }
};

int main() {
    Socket::sysinit();

    const ushort MEMC_PORT = 11211;
    OnEvent on_event;

    async::MemcachedClient memc;
    memc.set_on_connect(&on_event);
    memc.connect_ip("127.0.0.1", MEMC_PORT);

    memc.set("key1", "value1", on_event);
    memc.set("key2", "value2", on_event);
    memc.runlocal();

    memc.get("key1", on_event);
    memc.get("key2", on_event);
    memc.runlocal();

    return 0;
}
\endcode
*/
class MemcachedClient : public AsyncClient<MemcachedClient, impl_memc::ClientQueueItem> {
public:
    static const SizeT  DEFAULT_QUEUE_SIZE = 256;
    static const size_t DEFAULT_MAX_READ   = 524288;  // 512 KB
    static const size_t MIN_INITIAL_READ   = 0;

    typedef AsyncClient<MemcachedClient, impl_memc::ClientQueueItem> Base;
    using Base::OnConnect;
    using Base::OnError;

    /** Base interface for on_store() event. */
    struct OnStore {
        typedef async::Memcached Memcached;

        virtual ~OnStore() {
        }

        virtual void on_store(const SubString& key, Memcached::StoreResult result) {
            EVO_PARAM_UNUSED(key);
            EVO_PARAM_UNUSED(result);
        }
    };

    /** Base interface for on_increment() event. */
    struct OnIncrement {
        virtual ~OnIncrement() {
        }

        virtual void on_increment(const SubString& key, const UInt64& count) {
            EVO_PARAM_UNUSED(key);
            EVO_PARAM_UNUSED(count);
        }
    };

    /** Base interface for on_remove() event. */
    struct OnRemove {
        virtual ~OnRemove() {
        }

        virtual void on_remove(const SubString& key, bool removed) {
            EVO_PARAM_UNUSED(key);
            EVO_PARAM_UNUSED(removed);
        }
    };

    /** Base interface for on_touch() event. */
    struct OnTouch {
        virtual ~OnTouch() {
        }

        virtual void on_touch(const SubString& key, bool touched) {
            EVO_PARAM_UNUSED(key);
            EVO_PARAM_UNUSED(touched);
        }
    };

    /** Base interface for on_get() and on_get_end() events. */
    struct OnGet {
        virtual ~OnGet() {
        }

        virtual void on_get(const SubString& key, const SubString& value, uint32 flags) {
            EVO_PARAM_UNUSED(key);
            EVO_PARAM_UNUSED(value);
            EVO_PARAM_UNUSED(flags);
        }

        virtual void on_get_cas(const SubString& key, const SubString& value, uint32 flags, uint64 cas_id) {
            EVO_PARAM_UNUSED(key);
            EVO_PARAM_UNUSED(value);
            EVO_PARAM_UNUSED(flags);
            EVO_PARAM_UNUSED(cas_id);
        }

        virtual void on_get_end(const SubString& keys_notfound) {
            EVO_PARAM_UNUSED(keys_notfound);
        }
    };

    /** Base interface used as a shortcut that inherits all the non-error event interfaces: \link AsyncClient::OnConnect OnConnect\endlink, OnStore, OnIncrement, OnTouch, OnRemove, OnGet */
    struct OnEvent : OnConnect, OnStore, OnIncrement, OnTouch, OnRemove, OnGet {
    };

    /** Constructor to initialize client.
     \param  max_queue_size  Max size for pending response queue
     \param  max_read_size   Max read buffer size, 0 for unlimited -- this is used to limit the read buffer size
    */
    MemcachedClient(SizeT max_queue_size=DEFAULT_QUEUE_SIZE, SizeT max_read_size=DEFAULT_MAX_READ) :
        AsyncClient<MemcachedClient, impl_memc::ClientQueueItem>(max_queue_size, max_read_size), cur_type_(QueueItem::tNONE) {
    }

    /** Send a request to set a key and value.
     - This replaces the previous value for given key, and overwrites `flags` and `expire` time for that key
     - If `on_store=NULL` then this sends a "noreply" request to tell the server not to respond to this command (reducing overhead)
     .
     \param  key       Key to store under -- must not have any spaces
     \param  value     New value to store
     \param  flags     User defined flags to store under key, 0 for none
     \param  expire    Expiration time in seconds from now, 0 for no expiration, negative to expire now, or a value greater than 2592000 (30 days) means a Unix timestamp to expire on
                        - See calc_expire_time() helper
     \param  cas_id    Pointer to Compare-And-Swap ID returned from get_cas() request, NULL if not doing Compare-And-Swap, otherwise this new value is stored unless:
                        - If the value has been modified since get_cas() returned this `cas_id` then response will be Memcached::srEXISTS
                        - If key doesn't exist then response will be Memcached::srNOT_FOUND
     \param  on_store  OnStore handler to receive response event, NULL for none
     \param  on_error  OnError handler to use for unexpected errors (not used for normal fail response), NULL for none -- ignored if `on_store=NULL`
     \return           Whether successful, false if unable to send request due to internal error (usually means not connected and no connection in progress)
    */
    bool set(const SubString& key, const SubString& value, uint32 flags=0, int64 expire=0, uint64* cas_id=NULL, OnStore* on_store=NULL, OnError* on_error=NULL) {
        if (get_state() == sNONE)
            return false;

        StringInt<uint32,0> flags_str(flags, fDEC, false);
        StringInt<int64,0>  expire_str(expire, fDEC, false);
        StringInt<StrSizeT,0> val_size_str(value.size(), fDEC, false);
        StringInt<uint64,0> cas_id_str;

        size_t buf_size = 7 + key.size() + flags_str.size() + expire_str.size() + val_size_str.size() + NEWLINE_LEN + value.size() + NEWLINE_LEN;
        if (cas_id != NULL) {
            cas_id_str.set(*cas_id, fDEC, false);
            buf_size += 1 + cas_id_str.size();
        }
        if (on_store == NULL)
            buf_size += 8;

        RequestWriter writer(*this, buf_size);
        if (writer.error())
            return false;
        const char* dbg_str = writer.ptr();
        writer.add((cas_id == NULL ? "set " : "cas "), 4);
        writer.add(key.data(), key.size());
        writer.add(' ').add(flags_str.data(), flags_str.size());
        writer.add(' ').add(expire_str.data(), expire_str.size());
        writer.add(' ').add(val_size_str.data(), val_size_str.size());
        if (cas_id != NULL)
            writer.add(' ').add(cas_id_str.data(), cas_id_str.size());
        const StrSizeT dbg_str_len = (StrSizeT)(writer.ptr() - dbg_str);
        if (on_store == NULL)
            writer.add(" noreply", 8);
        writer.add("\r\n", NEWLINE_LEN);
        writer.add(value.data(), value.size());
        writer.add("\r\n", NEWLINE_LEN);
        if (logger.check(LOG_LEVEL_DEBUG))
            logger.log_direct(LOG_LEVEL_DEBUG, String().reserve(32 + dbg_str_len) << "MemcClient " << get_id() << ' ' << SubString(dbg_str, dbg_str_len) << " (write: " << buf_size << ')');

        if (on_store != NULL) {
            QueueItem& item = writer.pq.item;
            item.type     = QueueItem::tSTORE;
            item.on_reply = (void*)on_store;
            item.on_error = on_error;
            item.data     = key;
        }
        return true;
    }

    /** Send a request to set a key and value.
     - This replaces the previous value for given key, and overwrites `flags` and `expire` time for that key
     .
     \param  key       Key to store under -- must not have any spaces
     \param  value     New value to store
     \param  on_store  OnStore handler to receive response event
     \param  on_error  OnError handler to use for unexpected errors (not used for normal fail response), NULL for none
     \param  flags     User defined flags to store under key, 0 for none
     \param  expire    Expiration time in seconds from now, 0 for no expiration, negative to expire now, or a value greater than 2592000 (30 days) means a Unix timestamp to expire on
                        - See calc_expire_time() helper
     \param  cas_id    Pointer to Compare-And-Swap ID returned from get_cas() request, NULL if not doing Compare-And-Swap, otherwise this new value is stored unless:
                        - If the value has been modified since get_cas() returned this `cas_id` then response will be Memcached::srEXISTS
                        - If key doesn't exist then response will be Memcached::srNOT_FOUND
     \return           Whether successful, false if unable to send request due to internal error (usually means not connected and no connection in progress)
    */
    bool set(const SubString& key, const SubString& value, OnStore& on_store, OnError* on_error=NULL, uint32 flags=0, int64 expire=0, uint64* cas_id=NULL) {
        return set(key, value, flags, expire, cas_id, &on_store, on_error);
    }

    /** Send a request to set a key and value using Compare-And-Swap.
     - This replaces the previous value for given key, and overwrites `flags` and `expire` time for that key
     - get_cas() should be called first to check the current value and `cas_id`, then if needed this is used to replace that value (if it hasn't changed since)
     .
     \param  key       Key to store under -- must not have any spaces
     \param  value     New value to store
     \param  cas_id    Compare-And-Swap ID returned from get_cas() request, this new value is stored unless:
                        - If the value has been modified since get_cas() returned this `cas_id` then response will be Memcached::srEXISTS
                        - If key doesn't exist then response will be Memcached::srNOT_FOUND
     \param  on_store  OnStore handler to receive response event
     \param  on_error  OnError handler to use for unexpected errors (not used for normal fail response), NULL for none
     \param  flags     User defined flags to store under key, 0 for none
     \param  expire    Expiration time in seconds from now, 0 for no expiration, negative to expire now, or a value greater than 2592000 (30 days) means a Unix timestamp to expire on
                        - See calc_expire_time() helper
     \return           Whether successful, false if unable to send request due to internal error (usually means not connected and no connection in progress)
    */
    bool set_cas(const SubString& key, const SubString& value, uint64 cas_id, OnStore& on_store, OnError* on_error=NULL, uint32 flags=0, int64 expire=0) {
        return set(key, value, flags, expire, &cas_id, &on_store, on_error);
    }

    /** Send a request to append to existing value for key.
     - This appends to the current value for given key
     - If key doesn't exist then the response will be Memcached::srNOT_STORED
     .
     \param  key       Key to store under -- must not have any spaces
     \param  value     New value to append to existing value
     \param  on_store  OnStore handler to receive response event
     \param  on_error  OnError handler to use for unexpected errors (not used for normal fail response), NULL for none
     \return           Whether successful, false if unable to send request due to internal error (usually means not connected and no connection in progress)
    */
    bool set_append(const SubString& key, const SubString& value, OnStore* on_store=NULL, OnError* on_error=NULL) {
        if (get_state() == sNONE)
            return false;

        StringInt<StrSizeT,0> val_size_str(value.size(), fDEC, false);
        size_t buf_size = 12 + key.size() + val_size_str.size() + NEWLINE_LEN + value.size() + NEWLINE_LEN;
        if (on_store == NULL)
            buf_size += 8;

        RequestWriter writer(*this, buf_size);
        if (writer.error())
            return false;
        writer.add("append ", 7);
        writer.add(key.data(), key.size());
        writer.add(" 0 0 ", 5);
        writer.add(val_size_str.data(), val_size_str.size());
        if (on_store == NULL)
            writer.add(" noreply", 8);
        writer.add("\r\n", NEWLINE_LEN);
        writer.add(value.data(), value.size());
        writer.add("\r\n", NEWLINE_LEN);
        if (logger.check(LOG_LEVEL_DEBUG))
            logger.log_direct(LOG_LEVEL_DEBUG, String().reserve(54 + key.size()) << "MemcClient " << get_id() << " set_append '" << key << "' (write: " << buf_size << ')');

        if (on_store != NULL) {
            QueueItem& item = writer.pq.item;
            item.type     = QueueItem::tSTORE;
            item.on_reply = (void*)on_store;
            item.on_error = on_error;
            item.data     = key;
        }
        return true;
    }

    /** Send a request to prepend to existing value for key.
     - This prepends to the current value for given key
     - If key doesn't exist then the response will be Memcached::srNOT_STORED
     .
     \param  key       Key to store under -- must not have any spaces
     \param  value     New value to prepend to existing value
     \param  on_store  OnStore handler to receive response event
     \param  on_error  OnError handler to use for unexpected errors (not used for normal fail response), NULL for none
     \return           Whether successful, false if unable to send request due to internal error (usually means not connected and no connection in progress)
    */
    bool set_prepend(const SubString& key, const SubString& value, OnStore* on_store=NULL, OnError* on_error=NULL) {
        if (get_state() == sNONE)
            return false;

        StringInt<StrSizeT,0> val_size_str(value.size(), fDEC, false);
        size_t buf_size = 13 + key.size() + val_size_str.size() + NEWLINE_LEN + value.size() + NEWLINE_LEN;
        if (on_store == NULL)
            buf_size += 8;

        RequestWriter writer(*this, buf_size);
        if (writer.error())
            return false;
        writer.add("prepend ", 8);
        writer.add(key.data(), key.size());
        writer.add(" 0 0 ", 5);
        writer.add(val_size_str.data(), val_size_str.size());
        if (on_store == NULL)
            writer.add(" noreply", 8);
        writer.add("\r\n", NEWLINE_LEN);
        writer.add(value.data(), value.size());
        writer.add("\r\n", NEWLINE_LEN);
        if (logger.check(LOG_LEVEL_DEBUG))
            logger.log_direct(LOG_LEVEL_DEBUG, String().reserve(54 + key.size()) << "MemcClient " << get_id() << " set_append '" << key << "' (write: " << buf_size << ')');

        if (on_store != NULL) {
            QueueItem& item = writer.pq.item;
            item.type     = QueueItem::tSTORE;
            item.on_reply = (void*)on_store;
            item.on_error = on_error;
            item.data     = key;
        }
        return true;
    }

    /** Send a request to add a new key and value and fail if key already exists.
     - If key already exists then nothing is stored and the response will be Memcached::srNOT_STORED
     - If `on_store=NULL` then this sends a "noreply" request to tell the server not to respond to this command (reducing overhead)
     .
     \param  key       Key to store under -- must not have any spaces
     \param  value     New value to store
     \param  flags     User defined flags to store under key, 0 for none
     \param  expire    Expiration time in seconds from now, 0 for no expiration, negative to expire now, or a value greater than 2592000 (30 days) means a Unix timestamp to expire on
                        - See calc_expire_time() helper
     \param  on_store  OnStore handler to receive response event, NULL for none
     \param  on_error  OnError handler to use for unexpected errors (not used for normal fail response), NULL for none -- ignored if `on_store=NULL`
     \return           Whether successful, false if unable to send request due to internal error (usually means not connected and no connection in progress)
    */
    bool set_add(const SubString& key, const SubString& value, uint32 flags=0, int64 expire=0, OnStore* on_store=NULL, OnError* on_error=NULL) {
        if (get_state() == sNONE)
            return false;

        StringInt<uint32,0> flags_str(flags, fDEC, false);
        StringInt<int64,0>  expire_str(expire, fDEC, false);
        StringInt<StrSizeT,0> val_size_str(value.size(), fDEC, false);

        size_t buf_size = 7 + key.size() + flags_str.size() + expire_str.size() + val_size_str.size() + NEWLINE_LEN + value.size() + NEWLINE_LEN;
        if (on_store == NULL)
            buf_size += 8;

        RequestWriter writer(*this, buf_size);
        if (writer.error())
            return false;
        writer.add("add ", 4);
        writer.add(key.data(), key.size());
        writer.add(' ').add(flags_str.data(), flags_str.size());
        writer.add(' ').add(expire_str.data(), expire_str.size());
        writer.add(' ').add(val_size_str.data(), val_size_str.size());
        if (on_store == NULL)
            writer.add(" noreply", 8);
        writer.add("\r\n", NEWLINE_LEN);
        writer.add(value.data(), value.size());
        writer.add("\r\n", NEWLINE_LEN);
        if (logger.check(LOG_LEVEL_DEBUG))
            logger.log_direct(LOG_LEVEL_DEBUG, String().reserve(44 + key.size()) << "MemcClient " << get_id() << " set_add '" << key << "' (write: " << buf_size << ')');

        if (on_store != NULL) {
            QueueItem& item = writer.pq.item;
            item.type     = QueueItem::tSTORE;
            item.on_reply = (void*)on_store;
            item.on_error = on_error;
            item.data     = key;
        }
        return true;
    }

    /** Send a request to replace an existing value under key and fail if key doesn't exist.
     - If key doesn't exist then nothing is stored and the response will be Memcached::srNOT_STORED
     - If `on_store=NULL` then this sends a "noreply" request to tell the server not to respond to this command (reducing overhead)
     .
     \param  key       Key to store under -- must not have any spaces
     \param  value     New value to store
     \param  flags     User defined flags to store under key, 0 for none
     \param  expire    Expiration time in seconds from now, 0 for no expiration, negative to expire now, or a value greater than 2592000 (30 days) means a Unix timestamp to expire on
                        - See calc_expire_time() helper
     \param  on_store  OnStore handler to receive response event, NULL for none
     \param  on_error  OnError handler to use for unexpected errors (not used for normal fail response), NULL for none -- ignored if `on_store=NULL`
     \return           Whether successful, false if unable to send request due to internal error (usually means not connected and no connection in progress)
    */
    bool set_replace(const SubString& key, const SubString& value, uint32 flags=0, int64 expire=0, OnStore* on_store=NULL, OnError* on_error=NULL) {
        if (get_state() == sNONE)
            return false;

        StringInt<uint32,0> flags_str(flags, fDEC, false);
        StringInt<int64,0>  expire_str(expire, fDEC, false);
        StringInt<StrSizeT,0> val_size_str(value.size(), fDEC, false);

        size_t buf_size = 11 + key.size() + flags_str.size() + expire_str.size() + val_size_str.size() + NEWLINE_LEN + value.size() + NEWLINE_LEN;
        if (on_store == NULL)
            buf_size += 8;

        RequestWriter writer(*this, buf_size);
        if (writer.error())
            return false;
        writer.add("replace ", 8);
        writer.add(key.data(), key.size());
        writer.add(' ').add(flags_str.data(), flags_str.size());
        writer.add(' ').add(expire_str.data(), expire_str.size());
        writer.add(' ').add(val_size_str.data(), val_size_str.size());
        if (on_store == NULL)
            writer.add(" noreply", 8);
        writer.add("\r\n", NEWLINE_LEN);
        writer.add(value.data(), value.size());
        writer.add("\r\n", NEWLINE_LEN);
        if (logger.check(LOG_LEVEL_DEBUG))
            logger.log_direct(LOG_LEVEL_DEBUG, String().reserve(48 + key.size()) << "MemcClient " << get_id() << " set_replace '" << key << "' (write: " << buf_size << ')');

        if (on_store != NULL) {
            QueueItem& item = writer.pq.item;
            item.type     = QueueItem::tSTORE;
            item.on_reply = (void*)on_store;
            item.on_error = on_error;
            item.data     = key;
        }
        return true;
    }

    /** Send a request to increment or decrement value for given key.
     - If key doesn't exist then the response event `count` argument will be null
     - If value for key is not an integer, the server will return a CLIENT_ERROR and on_error will be called (if not NULL)
     - If `on_increment=NULL` then this sends a "noreply" request to tell the server not to respond to this command (reducing overhead)
     .
     \param  key           Key to use -- must not have any spaces
     \param  count         Count to increment or decrement by
     \param  decrement     Whether to decrement, false to increment
     \param  on_increment  OnIncrement handler to receive response event, NULL for none
     \param  on_error      OnError handler to use for unexpected errors (not used for normal fail response), NULL for none -- ignored if `on_increment=NULL`
     \return               Whether successful, false if unable to send request due to internal error (usually means not connected and no connection in progress)
    */
    bool increment(const SubString& key, uint64 count=1, bool decrement=false, OnIncrement* on_increment=NULL, OnError* on_error=NULL) {
        if (get_state() == sNONE)
            return false;

        StringInt<uint64> count_str(count, fDEC, true);
        StrSizeT buf_size = 6 + key.size() + count_str.size() + NEWLINE_LEN;
        if (on_increment == NULL)
            buf_size += 8;

        RequestWriter writer(*this, buf_size);
        if (writer.error())
            return false;
        SubString msg_str(writer.ptr(), buf_size - NEWLINE_LEN);
        writer.add((decrement ? "decr " : "incr "), 5);
        writer.add(key.data(), key.size());
        writer.add(' ').add(count_str.data(), count_str.size());
        if (on_increment == NULL)
            writer.add(" noreply", 8);
        writer.add("\r\n", NEWLINE_LEN);
        if (logger.check(LOG_LEVEL_DEBUG))
            logger.log_direct(LOG_LEVEL_DEBUG, String().reserve(22 + msg_str.size()) << "MemcClient " << get_id() << ' ' << msg_str);

        if (on_increment != NULL) {
            QueueItem& item = writer.pq.item;
            item.type     = QueueItem::tINCREMENT;
            item.on_reply = (void*)on_increment;
            item.on_error = on_error;
            item.data     = key;
        }
        return true;
    }

    /** Send a request to increment value for given key.
     - If key doesn't exist then the response event `count` argument will be null
     - If value for key is not an integer, the server will return a CLIENT_ERROR and on_error will be called (if not NULL)
     - If `on_increment=NULL` then this sends a "noreply" request to tell the server not to respond to this command (reducing overhead)
     .
     \param  key           Key to use -- must not have any spaces
     \param  count         Count to increment by
     \param  on_increment  OnIncrement handler to receive response event, NULL for none
     \param  on_error      OnError handler to use for unexpected errors (not used for normal fail response), NULL for none -- ignored if `on_increment=NULL`
     \return               Whether successful, false if unable to send request due to internal error (usually means not connected and no connection in progress)
    */
    bool incr(const SubString& key, uint64 count=1, OnIncrement* on_increment=NULL, OnError* on_error=NULL) {
        return increment(key, count, false, on_increment, on_error);
    }

    /** Send a request to decrement value for given key.
     - If key doesn't exist then the response event `count` argument will be null
     - If value for key is not an integer, the server will return a CLIENT_ERROR and on_error will be called (if not NULL)
     - If `on_increment=NULL` then this sends a "noreply" request to tell the server not to respond to this command (reducing overhead)
     .
     \param  key           Key to use -- must not have any spaces
     \param  count         Count to decrement by
     \param  on_increment  OnIncrement handler to receive response event, NULL for none
     \param  on_error      OnError handler to use for unexpected errors (not used for normal fail response), NULL for none -- ignored if `on_increment=NULL`
     \return               Whether successful, false if unable to send request due to internal error (usually means not connected and no connection in progress)
    */
    bool decr(const SubString& key, uint64 count=1, OnIncrement* on_increment=NULL, OnError* on_error=NULL) {
        return increment(key, count, true, on_increment, on_error);
    }

    /** Send a request to delete key and value
     - If `on_remove=NULL` then this sends a "noreply" request to tell the server not to respond to this command (reducing overhead)
     .
     \param  key        Key to use -- must not have any spaces
     \param  on_remove  OnRemove handler to receive response event, NULL for none
     \param  on_error   OnError handler to use for unexpected errors (not used for normal fail response), NULL for none -- ignored if `on_remove=NULL`
     \return            Whether successful, false if unable to send request due to internal error (usually means not connected and no connection in progress)
    */
    bool remove(const SubString& key, OnRemove* on_remove=NULL, OnError* on_error=NULL) {
        if (get_state() == sNONE)
            return false;

        StrSizeT buf_size = 7 + key.size() + NEWLINE_LEN;
        if (on_remove == NULL)
            buf_size += 8;

        RequestWriter writer(*this, buf_size);
        if (writer.error())
            return false;
        SubString msg_str(writer.ptr(), buf_size - NEWLINE_LEN);
        writer.add("delete ", 7);
        writer.add(key.data(), key.size());
        if (on_remove == NULL)
            writer.add(" noreply", 8);
        writer.add("\r\n", NEWLINE_LEN);
        if (logger.check(LOG_LEVEL_DEBUG))
            logger.log_direct(LOG_LEVEL_DEBUG, String().reserve(22 + msg_str.size()) << "MemcClient " << get_id() << ' ' << msg_str);

        if (on_remove != NULL) {
            QueueItem& item = writer.pq.item;
            item.type     = QueueItem::tDELETE;
            item.on_reply = (void*)on_remove;
            item.on_error = on_error;
            item.data     = key;
        }
        return true;
    }

    /** Send a request to touch (update) the expiration time for given key.
     - If `on_touch=NULL` then this sends a "noreply" request to tell the server not to respond to this command (reducing overhead)
     .
     \param  key       Key to store under -- must not have any spaces
     \param  expire    New expiration time in seconds from now, 0 for no expiration, negative to expire now, or a value greater than 2592000 (30 days) means a Unix timestamp to expire on
                        - See calc_expire_time() helper
     \param  on_touch  OnTouch handler to receive response event, NULL for none
     \param  on_error  OnError handler to use for unexpected errors (not used for normal fail response), NULL for none -- ignored if `on_store=NULL`
     \return           Whether successful, false if unable to send request due to internal error (usually means not connected and no connection in progress)
    */
    bool touch(const SubString& key, int64 expire, OnTouch* on_touch, OnError* on_error=NULL) {
        if (get_state() == sNONE)
            return false;

        StringInt<uint64,0> expire_str(expire, fDEC, false);
        StrSizeT buf_size = 7 + key.size() + expire_str.size() + NEWLINE_LEN;
        if (on_touch == NULL)
            buf_size += 8;

        RequestWriter writer(*this, buf_size);
        if (writer.error())
            return false;
        SubString msg_str(writer.ptr(), buf_size - NEWLINE_LEN);
        writer.add("touch ", 6);
        writer.add(key.data(), key.size());
        writer.add(' ').add(expire_str.data(), expire_str.size());
        if (on_touch == NULL)
            writer.add(" noreply", 8);
        writer.add("\r\n", NEWLINE_LEN);
        if (logger.check(LOG_LEVEL_DEBUG))
            logger.log_direct(LOG_LEVEL_DEBUG, String().reserve(22 + msg_str.size()) << "MemcClient " << get_id() << ' ' << msg_str);

        if (on_touch != NULL) {
            QueueItem& item = writer.pq.item;
            item.type     = QueueItem::tTOUCH;
            item.on_reply = (void*)on_touch;
            item.on_error = on_error;
            item.data     = key;
        }
        return true;
    }

    /** Send a request to get value for one or more keys.
     - The server only sends back responses for keys that were found, so noting which keys weren't found (cache misses) requires some overhead:
       - Use `track_notfound` to have the client build a list of keys not found, which is passed to OnGet::on_get_end()
     .
     \param  key             Key(s) to get, multiple keys are space-separated
     \param  on_get          OnGet handler to receive response events
     \param  on_error        OnError handler to use for unexpected errors (not used for normal fail response), NULL for none
     \param  track_notfound  Whether to track keys not found (i.e. cache misses), true to enable logic that builds a list of keys not found (passed to OnGet::on_get_end()) -- this adds some overhead
     \param  expire          Pointer to expiration time used to "touch" while getting the value by updating the expiration time, NULL to disable this
                              - Expiration time is in seconds from now, 0 for no expiration, negative to expire now, or a value greater than 2592000 (30 days) means a Unix timestamp to expire on
                              - See calc_expire_time() helper
     \return                 Whether successful, false if unable to send request due to internal error (usually means not connected and no connection in progress)
    */
    bool get(const SubString& key, OnGet& on_get, OnError* on_error=NULL, bool track_notfound=false, int64* expire=NULL) {
        if (get_state() == sNONE)
            return false;

        StringInt<uint64> expire_str;
        StrSizeT buf_size = 4 + key.size() + NEWLINE_LEN;
        if (expire != NULL) {
            expire_str.set(*expire);
            buf_size += 1 +  expire_str.size();
        }

        RequestWriter writer(*this, buf_size);
        if (writer.error())
            return false;
        SubString msg_str(writer.ptr(), buf_size - NEWLINE_LEN);
        if (expire != NULL) {
            writer.add("gat ", 4);
            writer.add(expire_str.data(), expire_str.size());
            writer.add(" ", 1);
        } else
            writer.add("get ", 4);
        writer.add(key.data(), key.size());
        writer.add("\r\n", NEWLINE_LEN);
        if (logger.check(LOG_LEVEL_DEBUG))
            logger.log_direct(LOG_LEVEL_DEBUG, String().reserve(22 + msg_str.size()) << "MemcClient " << get_id() << ' ' << msg_str);

        QueueItem& item = writer.pq.item;
        item.type     = QueueItem::tGET;
        item.on_reply = (void*)&on_get;
        item.on_error = on_error;
        item.data     = key;
        item.track_notfound = track_notfound;
        return true;
    }

    /** Send a request to get value for one or more keys for Compare-And-Swap.
     - Responses here will include a `cas_id`, which can be passed to set_cas() or set() to complete the Compare-And-Swap
     - The server only sends back responses for keys that were found, so noting which keys weren't found (cache misses) requires some overhead:
       - Use `track_notfound` to have the client build a list of keys not found, which is passed to OnGet::on_get_end()
     .
     \param  key             Key(s) to get, multiple keys are space-separated
     \param  on_get          OnGet handler to receive response events
     \param  on_error        OnError handler to use for unexpected errors (not used for normal fail response), NULL for none
     \param  track_notfound  Whether to track keys not found (i.e. cache misses), true to enable logic that builds a list of keys not found (passed to OnGet::on_get_end()) -- this adds some overhead
     \param  expire          Pointer to expiration time used to "touch" while getting the value by updating the expiration time, NULL to disable this
                              - Expiration time is in seconds from now, 0 for no expiration, negative to expire now, or a value greater than 2592000 (30 days) means a Unix timestamp to expire on
                              - See calc_expire_time() helper
     \return                 Whether successful, false if unable to send request due to internal error (usually means not connected and no connection in progress)
    */
    bool get_cas(const SubString& key, OnGet& on_get, OnError* on_error=NULL, bool track_notfound=false, int64* expire=NULL) {
        if (get_state() == sNONE)
            return false;

        StringInt<uint64> expire_str;
        StrSizeT buf_size = 5 + key.size() + NEWLINE_LEN;
        if (expire != NULL) {
            expire_str.set(*expire);
            buf_size += 1 +  expire_str.size();
        }

        RequestWriter writer(*this, buf_size);
        if (writer.error())
            return false;
        SubString msg_str(writer.ptr(), buf_size - NEWLINE_LEN);
        if (expire != NULL) {
            writer.add("gats ", 5);
            writer.add(expire_str.data(), expire_str.size());
            writer.add(" ", 1);
        } else
            writer.add("gets ", 5);
        writer.add(key.data(), key.size());
        writer.add("\r\n", NEWLINE_LEN);
        if (logger.check(LOG_LEVEL_DEBUG))
            logger.log_direct(LOG_LEVEL_DEBUG, String().reserve(22 + msg_str.size()) << "MemcClient " << get_id() << ' ' << msg_str);
 
        QueueItem& item = writer.pq.item;
        item.type     = QueueItem::tGET_CAS;
        item.on_reply = (void*)&on_get;
        item.on_error = on_error;
        item.data     = key;
        item.track_notfound = track_notfound;
        return true;
    }

    /** Send a request to get value for one or more keys and touch (update) the stored expiration time.
     - The server only sends back responses for keys that were found, so noting which keys weren't found (cache misses) requires some overhead:
       - Use `track_notfound` to have the client build a list of keys not found, which is passed to OnGet::on_get_end()
     .
     \param  key             Key(s) to get, multiple keys are space-separated
     \param  expire          New expiration time in seconds from now, 0 for no expiration, negative to expire now, or a value greater than 2592000 (30 days) means a Unix timestamp to expire on
                              - See calc_expire_time() helper
     \param  on_get          OnGet handler to receive response events
     \param  on_error        OnError handler to use for unexpected errors (not used for normal fail response), NULL for none
     \param  track_notfound  Whether to track keys not found (i.e. cache misses), true to enable logic that builds a list of keys not found (passed to OnGet::on_get_end()) -- this adds some overhead
     \return                 Whether successful, false if unable to send request due to internal error (usually means not connected and no connection in progress)
    */
    bool get_touch(const SubString& key, int64 expire, OnGet& on_get, OnError* on_error=NULL, bool track_notfound=false) {
        return get(key, on_get, on_error, track_notfound, &expire);
    }

    /** Send a request to get value for one or more keys for Compare-And-Swap and touch (update) the stored expiration time.
     - The server only sends back responses for keys that were found, so noting which keys weren't found (cache misses) requires some overhead:
       - Use `track_notfound` to have the client build a list of keys not found, which is passed to OnGet::on_get_end()
     .
     \param  key             Key(s) to get, multiple keys are space-separated
     \param  expire          New expiration time in seconds from now, 0 for no expiration, negative to expire now, or a value greater than 2592000 (30 days) means a Unix timestamp to expire on
                              - See calc_expire_time() helper
     \param  on_get          OnGet handler to receive response events
     \param  on_error        OnError handler to use for unexpected errors (not used for normal fail response), NULL for none
     \param  track_notfound  Whether to track keys not found (i.e. cache misses), true to enable logic that builds a list of keys not found (passed to OnGet::on_get_end()) -- this adds some overhead
     \return                 Whether successful, false if unable to send request due to internal error (usually means not connected and no connection in progress)
    */
    bool get_touch_cas(const SubString& key, int64 expire, OnGet& on_get, OnError* on_error=NULL, bool track_notfound=false) {
        return get_cas(key, on_get, on_error, track_notfound, &expire);
    }

    /** Calculate memcached expiration time for given number of seconds from now.
     - This returns an expiration value understood by memcached servers: number of seconds from now, or a value greater than 2592000 (30 days) means a Unix timestamp to expire on
     .
     \param  seconds  Number of seconds now for expiration time
     \param  base     Base timestamp to use, 0 for current time -- only used if needed
     \return          Expiration time value for memcached
    */
    static int64 calc_expire_time(int64 seconds, int64 base=0) {
        const int64 TIMESTAMP_THRESHOLD = 2592000; // 30 days
        if (seconds > TIMESTAMP_THRESHOLD) {
            if (base <= 0) {
                SysNativeTimeStamp ts;
                ts.set_utc();
                base = ts.get_unix_timestamp();
            }
            return base + seconds;
        }
        return seconds;
    }

private:
    static const size_t NEWLINE_LEN = 2;

    typedef impl_memc::ClientQueueItem QueueItem;

    struct ValueParams {
        typedef Pair<SubString,bool> KeyFlagPair;
        typedef List<KeyFlagPair> KeyFlags;

        SubString key;
        uint32   flags;
        ulong    size;
        uint64   cas_id;
        KeyFlags key_flags; // Flags for keys received -- used with track_notfound

        ValueParams() : flags(0), size(0), cas_id(0) {
        }

        ValueParams& clear() {
            key.set();
            flags  = 0;
            size   = 0;
            cas_id = 0;
            key_flags.set();
            return *this;
        }

        void parse(const SubString& params_str) {
            const char DELIM = ' ';
            StrTokWord tok(params_str);
            for (;;) {
                EVO_TOK_NEXT_OR_BREAK(tok, DELIM);  key    = tok.value();
                EVO_TOK_NEXT_OR_BREAK(tok, DELIM);  flags  = tok.value().getnum<uint32>(fDEC);
                EVO_TOK_NEXT_OR_BREAK(tok, DELIM);  size   = tok.value().getnum<ulong>(fDEC);
                EVO_TOK_NEXT_OR_BREAK(tok, DELIM);  cas_id = tok.value().getnum<uint64>();
                break;
            }
        }

        void init_key_flags(const SubString& data) {
            key_flags.clear();
            StrTok tok(data);
            while (tok.nextw(' '))
                key_flags.add(KeyFlagPair(tok.value(), false));
        }

        void no_key_flags() {
            key_flags.set();
        }

        void set_key_flag() {
            for (KeyFlags::IterM iter(key_flags); iter; ++iter) {
                KeyFlagPair& item(*iter);
                if (item.first == key && !item.second) {
                    item.second = true;
                    break;
                }
            }
        }
    };

    QueueItem::Type cur_type_;
    QueueItem cur_item_;
    ValueParams value_params_;

    friend class AsyncClient<MemcachedClient, QueueItem>;
    friend class evo::AsyncBuffers;

    void on_connect() {
        cur_type_ = QueueItem::tNONE;
    }

    void on_error(AsyncError err) {
        if (cur_type_ != QueueItem::tNONE && cur_item_.on_error != NULL) {
            ((OnError*)cur_item_.on_error)->on_error(err);
            cur_item_.on_error = NULL;
        }
    }

    void on_close() {
    }

    bool on_read_fixed(SizeT& next_size, SubString& data, void* context) {
        EVO_PARAM_UNUSED(next_size);
        EVO_PARAM_UNUSED(context);

        // Value data
        data.stripr("\r\n", NEWLINE_LEN, 1);
        switch (cur_type_) {
            case QueueItem::tGET_CAS:
                if (logger.check(LOG_LEVEL_DEBUG))
                    logger.log_direct(LOG_LEVEL_DEBUG, String().reserve(64 + value_params_.key.size()) << "MemcClient " << get_id() << " on_get_cas '" << value_params_.key << "' " << value_params_.cas_id);
                ((OnGet*)cur_item_.on_reply)->on_get_cas(value_params_.key, data, value_params_.flags, value_params_.cas_id);
                break;
            case QueueItem::tGET:
                if (logger.check(LOG_LEVEL_DEBUG))
                    logger.log_direct(LOG_LEVEL_DEBUG, String().reserve(34 + value_params_.key.size()) << "MemcClient " << get_id() << " on_get '" << value_params_.key << '\'');
                ((OnGet*)cur_item_.on_reply)->on_get(value_params_.key, data, value_params_.flags);
                break;
            default:
                assert( false ); // shouldn't happen
                break;
        }
        return true;
    }

    // Helper for common response error checks, used in on_read() -- undef'd below
    #define EVO_HELPER_RESPONSE_ERROR_CHECKS(EVENT_NAME) \
        if (reply_str.ends(STR_ERROR)) { \
            line.truncate(MAX_ERROR_LENGTH); \
            if (logger.check(LOG_LEVEL_ERROR)) \
                logger.log_direct(LOG_LEVEL_ERROR, logstr.set().reserve(30 + line.size()) << "MemcClient " << get_id() << ": " << line); \
            return false; \
        } else { \
            logger.log(LOG_LEVEL_ERROR, "MemcClient protocol error on " EVENT_NAME); \
            return false; \
        }

    bool on_read(SizeT& fixed_size, AsyncBuffers& buffers, void* context) {
        const char DELIM = ' ';
        const SubString STR_NOTFOUND("NOT_FOUND", 9);
        const SubString STR_VALUE("VALUE", 5);
        const SubString STR_END("END", 3);
        const SubString STR_ERROR("ERROR", 5);
        const ulong MAX_ERROR_LENGTH = 200;

        String logstr;
        SubString line;
        while (buffers.read_line(line)) {
            SubString reply_str, params_str;
            if (cur_type_ == QueueItem::tGET || cur_type_ == QueueItem::tGET_CAS) {
                // Still reading GET response
                line.split(DELIM, reply_str, params_str);
                if (reply_str == STR_VALUE) {
                    value_params_.clear().parse(params_str);
                    value_params_.set_key_flag();
                    buffers.read_flush();
                    if (!buffers.read_fixed_helper(*this, fixed_size, value_params_.size + NEWLINE_LEN, 0, context))
                        return false;
                    if (fixed_size > 0)
                        return true;
                } else if (reply_str == STR_END) {
                    cur_type_ = QueueItem::tNONE;
                    buffers.read_flush();
                    if (cur_item_.track_notfound) {
                        String keys_notfound;
                        for (ValueParams::KeyFlags::Iter iter(value_params_.key_flags); iter; ++iter) {
                            const ValueParams::KeyFlagPair& item(*iter);
                            if (!item.second)
                                keys_notfound.addsep(' ').add(item.first);
                        }
                        if (logger.check(LOG_LEVEL_DEBUG_LOW)) {
                            if (keys_notfound.size() == 0)
                                logger.log_direct(LOG_LEVEL_DEBUG_LOW, logstr.set().reserve(48) << "MemcClient " << get_id() << " on_get_end, no notfound");
                            else
                                logger.log_direct(LOG_LEVEL_DEBUG_LOW, logstr.set().reserve(48 + keys_notfound.size()) << "MemcClient " << get_id() << " on_get_end, notfound: '" << keys_notfound << '\'');
                        }
                        ((OnGet*)cur_item_.on_reply)->on_get_end(keys_notfound);
                    } else {
                        if (logger.check(LOG_LEVEL_DEBUG_LOW))
                            logger.log_direct(LOG_LEVEL_DEBUG_LOW, logstr.set().reserve(34) << "MemcClient " << get_id() << " on_get_end");
                        ((OnGet*)cur_item_.on_reply)->on_get_end(SubString());
                    }
                } else EVO_HELPER_RESPONSE_ERROR_CHECKS("GET VALUE");
                continue;   // next line
            }

            // Next reply
            if (!queue_.pop(cur_item_)) {
                logger.log(LOG_LEVEL_ERROR, "MemcClient internal error: Unexpected empty queue for response");
                return false;
            }
            line.split(DELIM, reply_str, params_str);
            switch (cur_item_.type) {
                case QueueItem::tSTORE: {
                    Memcached::StoreResult result = Memcached::StoreResultEnum::get_enum(reply_str);
                    if (result != Memcached::srUNKNOWN) {
                        if (logger.check(LOG_LEVEL_DEBUG))
                            logger.log_direct(LOG_LEVEL_DEBUG, logstr.set().reserve(36 + reply_str.size()) << "MemcClient " << get_id() << " on_store " << reply_str);
                        OnStore* on_store = (OnStore*)cur_item_.on_reply;
                        on_store->on_store(cur_item_.data, result);
                    } else EVO_HELPER_RESPONSE_ERROR_CHECKS("STORE");
                    break;
                }

                case QueueItem::tINCREMENT: {
                    UInt64 count;
                    if (reply_str != STR_NOTFOUND) {
                        count = reply_str.getnum<uint64>();
                        if (count.null()) {
                            EVO_HELPER_RESPONSE_ERROR_CHECKS("INCR/DECR");
                        }
                    }

                    if (logger.check(LOG_LEVEL_DEBUG))
                        logger.log_direct(LOG_LEVEL_DEBUG, logstr.set().reserve(36 + reply_str.size()) << "MemcClient " << get_id() << " on_increment " << reply_str);
                    OnIncrement* on_increment = (OnIncrement*)cur_item_.on_reply;
                    on_increment->on_increment(cur_item_.data, count);
                    break;
                }

                case QueueItem::tDELETE: {
                    const SubString STR_DELETED("DELETED", 7);
                    bool removed;
                    if (reply_str == STR_DELETED) {
                        removed = true;
                    } else if (reply_str == STR_NOTFOUND) {
                        removed = false;
                    } else EVO_HELPER_RESPONSE_ERROR_CHECKS("DELETE");

                    if (logger.check(LOG_LEVEL_DEBUG))
                        logger.log_direct(LOG_LEVEL_DEBUG, logstr.set().reserve(36 + reply_str.size()) << "MemcClient " << get_id() << " on_remove " << reply_str);
                    OnRemove* on_remove = (OnRemove*)cur_item_.on_reply;
                    on_remove->on_remove(cur_item_.data, removed);
                    break;
                }

                case QueueItem::tTOUCH: {
                    const SubString STR_TOUCHED("TOUCHED", 7);
                    bool touched;
                    if (reply_str == STR_TOUCHED) {
                        touched = true;
                    } else if (reply_str == STR_NOTFOUND) {
                        touched = false;
                    } else EVO_HELPER_RESPONSE_ERROR_CHECKS("TOUCH");

                    if (logger.check(LOG_LEVEL_DEBUG))
                        logger.log_direct(LOG_LEVEL_DEBUG, logstr.set().reserve(36 + reply_str.size()) << "MemcClient " << get_id() << " on_touch " << reply_str);
                    OnTouch* on_touch = (OnTouch*)cur_item_.on_reply;
                    on_touch->on_touch(cur_item_.data, touched);
                    break;
                }

                case QueueItem::tGET_CAS:
                case QueueItem::tGET: {
                    if (reply_str == STR_VALUE) {
                        cur_type_ = cur_item_.type;
                        value_params_.clear();
                        if (cur_item_.track_notfound)
                            value_params_.init_key_flags(cur_item_.data); // track flags for whether each key is found
                        else
                            value_params_.no_key_flags();
                        value_params_.parse(params_str);
                        value_params_.set_key_flag();
                        buffers.read_flush();
                        if (!buffers.read_fixed_helper(*this, fixed_size, value_params_.size + NEWLINE_LEN, 0, context))
                            return false;
                        if (fixed_size > 0)
                            return true;
                        continue;   // next line
                    } else if (reply_str == STR_END) {
                        // No keys found
                        assert( cur_type_ == QueueItem::tNONE );
                        if (cur_item_.on_reply != NULL) {
                            if (cur_item_.track_notfound) {
                                if (logger.check(LOG_LEVEL_DEBUG_LOW))
                                    logger.log(LOG_LEVEL_DEBUG_LOW, logstr.set().reserve(48) << "MemcClient " << get_id() << " on_get_end, none found");
                                ((OnGet*)cur_item_.on_reply)->on_get_end(cur_item_.data);
                            } else {
                                if (logger.check(LOG_LEVEL_DEBUG_LOW))
                                    logger.log(LOG_LEVEL_DEBUG_LOW, logstr.set().reserve(48) << "MemcClient " << get_id() << " on_get_end");
                                ((OnGet*)cur_item_.on_reply)->on_get_end(SubString());
                            }
                        }
                    } else EVO_HELPER_RESPONSE_ERROR_CHECKS("GET");
                    break;
                }

                default:
                    if (logger.check(LOG_LEVEL_ERROR))
                        logger.log_direct(LOG_LEVEL_ERROR, logstr.set().reserve(52) << "MemcClient internal error: Bad queue item type: " << (int)cur_item_.type);
                    return false;
            }
            buffers.read_flush();
        }
        return true;
    }

    #undef EVO_HELPER_RESPONSE_ERROR_CHECKS

private:
    // Disable copying
    MemcachedClient(const MemcachedClient&);
    MemcachedClient& operator=(const MemcachedClient&);
};

///////////////////////////////////////////////////////////////////////////////
//@}
}
}
#endif
