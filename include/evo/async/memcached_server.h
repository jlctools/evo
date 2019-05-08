// Evo C++ Library
/* Copyright 2019 Justin Crowell
Distributed under the BSD 2-Clause License -- see included file LICENSE.txt for details.
*/
///////////////////////////////////////////////////////////////////////////////
/** \file memcached_server.h Evo Async Memached API, server classes. */
#pragma once
#ifndef INCL_evo_api_memcached_server_h
#define INCL_evo_api_memcached_server_h

#include "memcached_common.h"
#include "../ioasync_server.h"
#include "../strtok.h"

namespace evo {
namespace async {
/** \addtogroup EvoAsyncAPI */
//@{

///////////////////////////////////////////////////////////////////////////////

/** Base class for user defined %Memcached server handler.
 - See: MemcachedServer
 .
Request event methods:
 - on_error()
 - on_store()
 - on_increment()
 - on_delete()
 - on_touch()
 - on_get_start(), on_get(), on_get_end()
 - on_flush_all()
 - on_stats()
 - on_version()
 - on_command()
 .
*/
struct MemcachedServerHandlerBase : public AsyncServerHandler {
    typedef evo::async::Memcached Memcached;    ///< Alias for `evo::async::Memcached`

    /** Command value. */
    enum Command {
        cUNKNOWN = 0,       ///< Unknown command (always first)
        cADD,               ///< Add value if not found
        cAPPEND,            ///< Append to existing value
        cCAS,               ///< %Compare and swap
        cDECREMENT,         ///< Decrement numeric value for key
        cDELETE,            ///< Delete by key
        cGAT,               ///< Get value and update expiratiom time
        cGATS,              ///< Get value for compare and swap and update expiratiom time
        cGET,               ///< Get value
        cGETS,              ///< Get value for compare and swap
        cINCREMENT,         ///< Increment numeric value for key
        cPREPEND,           ///< Prepend to existing value
        cQUIT,              ///< Quit command to close connection
        cREPLACE,           ///< Replace value if found
        cSET,               ///< Set new value
        cSTATS,             ///< Get server stats
        cTOUCH,             ///< Touch key by updating expiration time
        cVERSION,           ///< Get server version
        cENUM_END           ///< Enum guard value (always last)
    };

    /** Command enum mappings. */
    EVO_ENUM_MAP_PREFIXED(Command, c,
        "add",
        "append",
        "cas",
        "decr",
        "delete",
        "gat",
        "gats",
        "get",
        "gets",
        "incr",
        "prepend",
        "quit",
        "replace",
        "set",
        "stats",
        "touch",
        "version"
    );

    /** STORE command parameters. */
    struct StoreParams {
        String key;         ///< Key to store
        uint32 flags;       ///< Flags to store, returned with GET
        int64  expire;      ///< Expiration time in seconds from now, 0 for no expiration, negative to expire now, or a value greater than 2592000 (30 days) means a Unix timestamp to expire on
        ulong  size;        ///< Value size to store
        uint64 cas_id;      ///< CAS ID -- only used for CAS command

        StoreParams() : flags(0), expire(0), size(0), cas_id(0) { }
    };

    /** GET command result from on_get_start(). */
    enum GetStartResultValue {
        gsrCONTINUE,    ///< Continue to on_get() events
        gsrSKIP         ///< Skip on_get() events
    };

    /** General result used by some commands. */
    enum GeneralResultValue {
        grOK,           ///< Success
        grNOT_FOUND     ///< Key not found
    };

    typedef ResponseResult<Memcached::StoreResult> StoreResult;     ///< STORE command result returned by on_store()
    typedef ResponseResult<GetStartResultValue>    GetStartResult;  ///< GET command result returned by on_get_start()
    typedef ResponseResult<UInt64>                 IncrementResult; ///< INCR/DECR command result
    typedef ResponseResult<GeneralResultValue>     DeleteResult;    ///< DELETE command result returned by on_delete()
    typedef ResponseResult<GeneralResultValue>     TouchResult;     ///< TOUCH command result returned by on_touch()

    /** Additional parameters for advanced GET request variants.
     - GET request variants:
       - GET is the normal request to get a value -- below are advanced variants that add additional functionality
       - GAT (get and update) to get and update expiration time at the same time
         - `expire` will have the new expiration time from the request
       - GETS (get for Compare-And-Swap) to get and return a cas_id for atomic %Compare And Swap
         - `cas` will be set to true
       - GATS (get for Compare-And-Swap and update) combines both above variants (GAT and GETS)
    */
    struct GetAdvParams {
        Int64 expire;   ///< Expiration time in seconds from now, null if not updating expiration, 0 for no expiration, negative to expire now, or a value greater than 2592000 (30 days) means a Unix timestamp to expire on
        bool  cas;      ///< Whether CAS is enabled, if true the handler must include a `cas_id` when sending response values with `send_value()`
    };

    /** Deferred context helper -- used by AsyncServer. */
    typedef DeferredContextT<MemcachedServerHandlerBase> DeferredContext;

    /** Deferred reply helper -- deferred event objects should hold or inherit this, and use to send deferred response. */
    struct DeferredReply : DeferredContext::ReplyBase {
        /** Constructor.
         \param  context  Context to use
         \param  id       Request ID to use
        */
        DeferredReply(DeferredContext& context, ulong id) : DeferredContext::ReplyBase(context, id) {
        }

        /** Destructor. */
        ~DeferredReply() {
            if (!finished && context.handler != NULL) {
                context.handler->logger.log(LOG_LEVEL_ERROR, "MemcServer DeferredReply left unfinished");
                deferred_reply_error("Internal handler error: DeferredReply left unfinished");
            }
        }

        /** Finish deferred response with an error.
         \param  msg  Error message to send back to client -- must not have any newlines
        */
        void deferred_reply_error(const SubString& msg) {
            if (context.handler != NULL) {
                String buf;
                buf.reserve(15 + msg.size());
                buf.set("SERVER_ERROR ", 13);
                buf << msg;
                buf.add("\r\n", 2);
                DeferredContext::Handler& handler = *context.handler;
                handler.reply.deferred_send(id, buf, true);
                if (handler.logger.check(LOG_LEVEL_DEBUG_LOW))
                    handler.logger.log_direct(LOG_LEVEL_DEBUG_LOW, String().reserve(40 + msg.size()) << "MemcServer deferred reply " << id << " error: " << msg);
                if (handler.reply.deferred_end(context))
                    handler.logger.log_direct(LOG_LEVEL_DEBUG_LOW, "MemcServer cleanup");
            }
            finished = true;
        }

        /** Finish set/store request and report result.
         \param  result  Command result -- see Memcached::StoreResult
        */
        void deferred_reply_store(Memcached::StoreResult result) {
            if (context.handler != NULL) {
                String buf;
                switch (result) {
                    case Memcached::srSTORED:     buf.set("STORED\r\n", 8);      break;
                    case Memcached::srNOT_STORED: buf.set("NOT_STORED\r\n", 12); break;
                    case Memcached::srEXISTS:     buf.set("EXISTS\r\n", 8);      break;
                    case Memcached::srNOT_FOUND:  buf.set("NOT_FOUND\r\n", 11);  break;
                    default: buf.set("SERVER_ERROR Backend error\r\n", 28); break;
                }
                DeferredContext::Handler& handler = *context.handler;
                if (handler.logger.check(LOG_LEVEL_DEBUG_LOW))
                    handler.logger.log_direct(LOG_LEVEL_DEBUG_LOW, String().reserve(60 + buf.size()) << "MemcServer on_store send deferred response " << id << ": " << SubString(buf.data(), buf.size() - 2));
                handler.reply.deferred_send(id, buf, true);
                if (handler.reply.deferred_end(context))
                    handler.logger.log_direct(LOG_LEVEL_DEBUG_LOW, "MemcServer cleanup");
            }
            finished = true;
        }

        /** Finish increment/decrement request and report result.
         \param  value  Result value after increment/decrement, null if key not found
        */
        void deferred_reply_increment(UInt64 value) {
            if (context.handler != NULL) {
                String buf;
                if (value.null()) {
                    buf.set("NOT_FOUND\r\n", 11);
                } else {
                    buf.reserve(UInt64::MAXSTRLEN + 2);
                    buf << *value << SubString("\r\n", 2);
                }
                DeferredContext::Handler& handler = *context.handler;
                if (handler.logger.check(LOG_LEVEL_DEBUG_LOW))
                    handler.logger.log_direct(LOG_LEVEL_DEBUG_LOW, String().reserve(60 + buf.size()) << "MemcServer on_increment send deferred response " << id << ": " << SubString(buf.data(), buf.size() - 2));
                handler.reply.deferred_send(id, buf, true);
                if (handler.reply.deferred_end(context))
                    handler.logger.log_direct(LOG_LEVEL_DEBUG_LOW, "MemcServer cleanup");
            }
            finished = true;
        }

        /** Finish delete request and report result.
         \param  success  Whether successful, false if key not found
        */
        void deferred_reply_delete(bool success) {
            if (context.handler != NULL) {
                String buf;
                if (success)
                    buf.set("DELETED\r\n", 9);
                else
                    buf.set("NOT_FOUND\r\n", 11);
                DeferredContext::Handler& handler = *context.handler;
                if (handler.logger.check(LOG_LEVEL_DEBUG_LOW))
                    handler.logger.log_direct(LOG_LEVEL_DEBUG_LOW, String().reserve(60 + buf.size()) << "MemcServer on_delete send deferred response " << id << ": " << SubString(buf.data(), buf.size() - 2));
                handler.reply.deferred_send(id, buf, true);
                if (handler.reply.deferred_end(context))
                    handler.logger.log_direct(LOG_LEVEL_DEBUG_LOW, "MemcServer cleanup");
            }
            finished = true;
        }

        /** Finish touch request and report result.
         \param  success  Whether successful, false if key not found
        */
        void deferred_reply_touch(bool success) {
            if (context.handler != NULL) {
                String buf;
                if (success)
                    buf.set("TOUCHED\r\n", 9);
                else
                    buf.set("NOT_FOUND\r\n", 11);
                DeferredContext::Handler& handler = *context.handler;
                if (handler.logger.check(LOG_LEVEL_DEBUG_LOW))
                    handler.logger.log_direct(LOG_LEVEL_DEBUG_LOW, String().reserve(60 + buf.size()) << "MemcServer on_touch send deferred response " << id << ": " << SubString(buf.data(), buf.size() - 2));
                handler.reply.deferred_send(id, buf, true);
                if (handler.reply.deferred_end(context))
                    handler.logger.log_direct(LOG_LEVEL_DEBUG_LOW, "MemcServer cleanup");
            }
            finished = true;
        }

        /** Send get request value for key.
         - This may be called multiple times, once per requested key
         - Keys not found are skipped and don't have a response value
         - After last response value call deferred_reply_get_end() to finish the response
         .
         \param  key     Key for value to send
         \param  value   Value to send for key
         \param  flags   User flags stored with value
         \param  cas_id  Compare-And-Swap ID to send, NULL for none -- this must only be non-null for CAS GET request variants, see GetAdvParams
        */
        void deferred_reply_get(const SubString& key, const SubString& value, uint32 flags, uint64* cas_id=NULL) {
            if (context.handler != NULL) {
                DeferredContext::Handler& handler = *context.handler;
                handler.send_value_internal(id, key, value, flags, cas_id, AsyncServerReply::wfDEFERRED);
                if (handler.logger.check(LOG_LEVEL_DEBUG_LOW))
                    handler.logger.log_direct(LOG_LEVEL_DEBUG_LOW, String().reserve(68 + key.size()) << "MemcServer on_get sent deferred value " << id << ": '" << key << "' (size: " << value.size() << ')');
            }
        }

        /** Finish response for get request. */
        void deferred_reply_get_end() {
            if (context.handler != NULL) {
                String buf("END\r\n", 5);
                DeferredContext::Handler& handler = *context.handler;
                handler.reply.deferred_send(id, buf, true);
                if (handler.logger.check(LOG_LEVEL_DEBUG_LOW))
                    handler.logger.log_direct(LOG_LEVEL_DEBUG_LOW, String().reserve(48) << "MemcServer on_get end deferred response " << id);
                if (handler.reply.deferred_end(context))
                    handler.logger.log_direct(LOG_LEVEL_DEBUG_LOW, "MemcServer cleanup");
            }
            finished = true;
        }
    };

    LoggerPtr<> logger;     ///< Logger to use (set by AsyncServer)
    bool noreply;           ///< Whether no-reply mode is enabled (set by parent protocol class)
    bool enable_gat;        ///< Derived constructor must set to true to enable "get and touch" (gat/gats command)
    bool enable_cas;        ///< Derived constructor must set to true to enable "compare and swap" (gets/gats command)

    /** Constructor. */
    MemcachedServerHandlerBase() : noreply(false), enable_gat(false), enable_cas(false) {
    }

    // Config

    static size_t get_max_initial_read() {
        const size_t MAX_INITIAL_READ_VALUE = 524288;  // 512 KB
        return MAX_INITIAL_READ_VALUE;
    }

    // Events

    /** Called on STORE request to store a value.
     - How to store depends on the exact command:
       - cSET:      Store new value, replace if already exists
       - cAPPEND:   Store value, append to existing value, or set new value if doesn't exist
       - cPREPEND:  Store value, prepend to existing value, or set new value if doesn't exist
       - cADD:      Store new value, only if key doesn't already exist
       - cREPLACE:  Store new value, only if key already exists
       - cCAS:      Store with atomic swap operation, fails if item was modified since it was last retrieved -- not enabled unless `enable_cas=true`
       .
     - On error call send_error() then return `rtHANDLED` or `rtCLOSE`
     - If the store `command` is not supported do this:
       \code
        send_error("Not implemented");
        return rtHANDLED;
       \endcode
     .
     \param  context  Context for creating DeferredReply for deferred response
     \param  params   Storage parameters to use
     \param  value    Value to store
     \param  command  Exact store command used, one of: cSET, cAPPEND, cPREPEND, cADD, cREPLACE, cCAS
                       - cAPPEND and cPREPEND commands should only update values and ignore `params.flags` and `params.exptime`
     \param  cas_id   %Compare And Swap ID from previous "gets" or "gats" command -- positive for cCAS command, otherwise always 0
     \return          Store result:
                       - `Memcached::srSTORED` if value successfully stored, or if 'noreply' flag is set
                       - `Memcached::srNOT_STORED` if not stored due to unmet condition (cADD, cREPLACE, cAPPEND, cPREPEND)
                         - cADD: Used when key already exists (can't add new key)
                         - cREPLACE: Used when key doesn't already exist (nothing to replace)
                         - cAPPEND/cPREPEND: Used when key doesn't already exist (nothing to append/prepend)
                       - CAS:
                         - `Memcached::srEXISTS` if %Compare And Swap failed due to value being modified since last retrieved with `cas_id` (cCAS only)
                         - `Memcached::srNOT_FOUND` if %Compare And Swap failed due to value not existing (cCAS only)
                       - `rtDEFERRED` for deferred response (i.e. must wait for another event)
                         - Not valid when 'noreply' flag is set (meaning the client doesn't want a reply)
                         - When deferred, an event using DeferredReply must be created and the event must call:
                           - DeferredReply::deferred_reply_store() to send the deferred response
                           - or DeferredReply::deferred_reply_error() to finish with an error
                       - `rtHANDLED` if response already sent -- use if error was sent
                       - `rtCLOSE` to stop this request and immediately close connection
    */
    virtual StoreResult on_store(DeferredContext& context, StoreParams& params, SubString& value, Command command, uint64 cas_id) {
        EVO_PARAM_UNUSED(context);
        EVO_PARAM_UNUSED(params);
        EVO_PARAM_UNUSED(value);
        EVO_PARAM_UNUSED(command);
        EVO_PARAM_UNUSED(cas_id);
        send_error("Not implemented");
        return rtHANDLED;
    }

    /** Called on INCR or DECR request to increment or decrement a numeric value.
     - Decrementing below 0 should result in 0
     - On error call send_error() then return `rtHANDLED` or `rtCLOSE`
     - If value for key is not numeric then this should be considered an error
     .
     \param  context    Context for creating DeferredReply for deferred response
     \param  key        Key to increment or decrement
     \param  count      Count to increment or decrement
     \param  decrement  Whether to decrement instead of increment, false to increment, true to decrement
     \return            Result:
                         - UInt64 value for new value after increment/decrement
                         - Null UInt64 value if key not found
                         - `rtDEFERRED` for deferred response (i.e. must wait for another event)
                           - Not valid when 'noreply' flag is set (meaning the client doesn't want a reply)
                           - When deferred, an event using DeferredReply must be created and the event must call:
                             - DeferredReply::deferred_reply_increment() to send the deferred response
                             - or DeferredReply::deferred_reply_error() to finish with an error
                         - `rtHANDLED` if response already sent -- use if error was sent
                         - `rtCLOSE` to stop this request and immediately close connection
    */
    virtual IncrementResult on_increment(DeferredContext& context, const SubString& key, uint64 count, bool decrement) {
        EVO_PARAM_UNUSED(context);
        EVO_PARAM_UNUSED(key);
        EVO_PARAM_UNUSED(count);
        EVO_PARAM_UNUSED(decrement);
        send_error("Not implemented");
        return rtHANDLED;
    }

    /** Called on DELETE request to delete key and value.
     - On error call send_error() then return `rtHANDLED` or `rtCLOSE`
     .
     \param  context  Context for creating DeferredReply for deferred response
     \param  key      Key to delete
     \return          Delete result:
                       - `grOK` if deleted successfully
                       - `drNOT_FOUND` if key not found
                       - `rtDEFERRED` for deferred response (i.e. must wait for another event)
                         - Not valid when 'noreply' flag is set (meaning the client doesn't want a reply)
                         - When deferred, an event using DeferredReply must be created and the event must call:
                           - DeferredReply::deferred_reply_delete() to send the deferred response
                           - or DeferredReply::deferred_reply_error() to finish with an error
                       - `rtHANDLED` if response already sent -- use if error was sent
                       - `rtCLOSE` to stop this request and immediately close connection
    */
    virtual DeleteResult on_delete(DeferredContext& context, const SubString& key) {
        EVO_PARAM_UNUSED(context);
        EVO_PARAM_UNUSED(key);
        send_error("Not implemented");
        return rtHANDLED;
    }

    /** Called on TOUCH request to update expiration of existing key without retrieving the value.
     - On error call send_error() then return `rtHANDLED` or `rtCLOSE`
     .
     \param  context  Context for creating DeferredReply for deferred response
     \param  key      Key to delete
     \param  expire   New expiration time in seconds from now, 0 for no expiration, negative to expire now, or a value greater than 2592000 (30 days) means a Unix timestamp to expire on
     \return          Touch result:
                       - `grOK` if successful
                       - `drNOT_FOUND` if key not found
                       - `rtDEFERRED` for deferred response (i.e. must wait for another event)
                         - Not valid when 'noreply' flag is set (meaning the client doesn't want a reply)
                         - When deferred, an event using DeferredReply must be created and the event must call:
                           - DeferredReply::deferred_reply_touch() to send the deferred response
                           - or DeferredReply::deferred_reply_error() to finish with an error
                       - `rtHANDLED` if response already sent -- use if error was sent
                       - `rtCLOSE` to stop this request and immediately close connection
    */
    virtual TouchResult on_touch(DeferredContext& context, const SubString& key, int64 expire) {
        EVO_PARAM_UNUSED(context);
        EVO_PARAM_UNUSED(key);
        EVO_PARAM_UNUSED(expire);
        send_error("Not implemented");
        return rtHANDLED;
    }

    /** Called at the beginning of a GET request with all requested keys.
     - GET request variants:
       - GET is the normal request to get a value -- below are advanced variants that add additional functionality
       - GAT (get and update) to get and update expiration time at the same time -- these requests are rejected unless `enable_gat=true`
       - GETS (get for Compare-And-Swap) to get and return a cas_id for atomic %Compare And Swap -- these requests are rejected unless `enable_cas=true`
       - GATS (get for Compare-And-Swap and update) combines both above variants -- these requests are rejected unless `enable_gat=true` and `enable_cas=true`
     - This allows handling all keys at once as a batch (if desired) -- by default this returns `gsrCONTINUE` if not overridden
       - If desired, use StrTokWord() to tokenize keys on space character
     - Call send_value() to send a value per requested key -- skip for keys not found
     - It's ok to call send_value() for some values then defer the rest by returning `rtDEFERRED`
     .
     \param  context     Context for creating DeferredReply for deferred response
     \param  keys        Requested keys (space separated)
     \param  adv_params  Pointer to advanced params, NULL for normal GET request, non-null if this is an advanced GET variant request (see above)
                          - This same pointer is also passed to `on_get()` events for this request
     \return             Get result:
                          - `gsrCONTINUE` to continue to `on_get()` events for each key -- default if not overridden
                          - `gsrSKIP` to skip `on_get()` events -- `on_get_end()` is still called
                          - This indicates all requested keys were retrieved and send_value() was called as needed
                          - `rtDEFERRED` to skip `on_get()` events and setup for deferred response -- `on_get_end()` is still called
                            - This indicates that all requested values were sent (if any) and at least one is deferred
                            - When deferred, an event using DeferredReply must be created and the event must call:
                              - DeferredReply::deferred_reply_get() for each value per key (skip for keys not found)
                              - DeferredReply::deferred_reply_get_end() to finish the deferred response
                              - or DeferredReply::deferred_reply_error() to finish with an error
                          - `rtHANDLED` if response already sent -- use if error was sent -- `on_get()` and `on_get_end()` will not be called for this response
                          - `rtCLOSE` to stop this request and immediately close connection
    */
    virtual GetStartResult on_get_start(DeferredContext& context, const SubString& keys, GetAdvParams* adv_params) {
        EVO_PARAM_UNUSED(context);
        EVO_PARAM_UNUSED(keys);
        EVO_PARAM_UNUSED(adv_params);
        return gsrCONTINUE;
    }

    /** Called for each key in GET request.
     - GET request variants:
       - GET is the normal request to get a value -- below are advanced variants that add additional functionality
       - GAT (get and update) to get and update expiration time at the same time -- these requests are rejected unless `enable_gat=true`
       - GETS (get for Compare-And-Swap) to get and return a cas_id for atomic %Compare And Swap -- these requests are rejected unless `enable_cas=true`
       - GATS (get for Compare-And-Swap and update) combines both above variants -- these requests are rejected unless `enable_gat=true` and `enable_cas=true`
     - Call send_value() to send a value for key -- skip if key not found
     - This is called multiple times if a GET request has multiple keys
     - This is only called if `on_get_start()` returned `grCONTINUE` for this request
     - If any part of this response is deferred then this or `on_get_start()` or `on_get_end()` must return `rtDEFERRED` to indiciate this, otherwise results are undefined
     .
     \param  context     Context for creating DeferredReply for deferred response
     \param  key         Requested key to get
     \param  adv_params  Pointer to advanced params, NULL for normal GET request, non-null if this is an advanced GET variant request (see above)
     \return             Response result:
                          - `rtNORMAL` to continue to next event
                          - `rtDEFERRED` to continue, but also mark the response as deferred (i.e. must wait for another event)
                            - When deferred, an event using DeferredReply must be created and the event must call:
                              - DeferredReply::deferred_reply_get() to send value for key (skip if not found)
                              - DeferredReply::deferred_reply_get_end() to finish the deferred response
                              - or DeferredReply::deferred_reply_error() to finish with an error
                            - When on_get() is called multiple times for multiple keys, each call returning `rtDEFERRED` must have a matching DeferredReply to finish that deferred response
                          - `rtHANDLED` if response already sent -- use if error was sent
                          - `rtCLOSE` to stop this request and immediately close connection
    */
    virtual ResponseType on_get(DeferredContext& context, const SubString& key, GetAdvParams* adv_params) {
        EVO_PARAM_UNUSED(context);
        EVO_PARAM_UNUSED(key);
        EVO_PARAM_UNUSED(adv_params);
        return rtNORMAL;
    }

    /** Called at end of GET request.
     - This is called after on_get_start() and after the last on_get() call for this request, and gives an oppurtunity for cleanup
       - This is not called if on_get_start() or on_get() returned `rtHANDLED` or `rtCLOSE`
     - This marks the end of the GET request inputs, but deferred responses for this request may still be pending -- the request is not complete until all deferred responses for it are completed
     .
     \param  context  Context for creating DeferredReply for deferred response
     \return          Response result:
                       - `rtNORMAL` to complete the GET request -- default if method not implemented
                       - `rtDEFERRED` to mark the response as deferred (i.e. must wait for another event)
                         - When deferred, an event using DeferredReply must be created and the event must call:
                           - DeferredReply::deferred_reply_get() for each additional value per key (skip for keys not found)
                           - DeferredReply::deferred_reply_get_end() to finish the deferred response
                           - or DeferredReply::deferred_reply_error() to finish with an error
                       - `rtHANDLED` if response already sent -- use if error was sent
                       - `rtCLOSE` to stop this request and close connection
    */
    virtual ResponseType on_get_end(DeferredContext& context) {
        EVO_PARAM_UNUSED(context);
        return rtNORMAL;
    }

    /** Called on FLUSH_ALL request to expire all keys.
     \param  delay_sec  Number of seconds to delay the flush, 0 for immediate
    */
    virtual void on_flush_all(ulong delay_sec) {
        EVO_PARAM_UNUSED(delay_sec);
    }

    /** Called on STATS request for statistics.
     - Use send_reply() for response 
     .
     \param  params  Parameter string received
    */
    virtual void on_stats(SubString& params) {
        EVO_PARAM_UNUSED(params);
    }

    /** Called on VERSION request for server version.
     - Use send_reply() for response 
     .
     \param  version  Set to version string to return (no newline chars)  [out]
    */
    virtual void on_version(String& version) {
        version = "Unknown";
    }

    /** Called on any other command.
     - Use send_reply() or send_error() for response 
     .
     \param  handled      Set to true if command handled, otherwise an error response is sent (unknown command)
     \param  command_str  %Command string to use
     \param  params       Parameters string received, usually space delimited
     \return              Whether successful, false to close connection
    */
    virtual bool on_command(bool& handled, SubString& command_str, SubString& params) {
        EVO_PARAM_UNUSED(handled);
        EVO_PARAM_UNUSED(command_str);
        EVO_PARAM_UNUSED(params);
        return true;
    }

    /** Called on error.
     \param  err  Error code -- see AsyncError
    */
    virtual void on_error(AsyncError err) {
        EVO_PARAM_UNUSED(err);
    }

    // Helpers

    /** Helper to send a reply message.
     \param  msg  Reply message to send
    */
    void send_reply(const SubString& msg) {
        if (!noreply) {
            AsyncServerReply::Writer writer(reply, id, msg.size() + 2);
            writer.add(msg.data(), msg.size());
            writer.add("\r\n", 2);
            if (logger.check(LOG_LEVEL_DEBUG_LOW))
                logger.log_direct(LOG_LEVEL_DEBUG_LOW, String().reserve(32 + msg.size()) << "MemcServer -- send_reply: " << msg);
        }
    }

    /** Helper to send a client error response.
     \param  msg  Error message to send
    */
    void send_client_error(const SubString& msg) {
        if (!noreply) {
            AsyncServerReply::Writer writer(reply, id, 15 + msg.size());
            writer.add("CLIENT_ERROR ", 13);
            writer.add(msg.data(), msg.size());
            writer.add("\r\n", 2);
            if (logger.check(LOG_LEVEL_DEBUG_LOW))
                logger.log_direct(LOG_LEVEL_DEBUG_LOW, String().reserve(40 + msg.size()) << "MemcServer -- send_error: CLIENT_ERROR " << msg);
        }
    }

    /** Helper to send a server error response.
     \param  msg  Error message to send
    */
    void send_error(const SubString& msg) {
        if (!noreply) {
            AsyncServerReply::Writer writer(reply, id, 15 + msg.size());
            writer.add("SERVER_ERROR ", 13);
            writer.add(msg.data(), msg.size());
            writer.add("\r\n", 2);
            if (logger.check(LOG_LEVEL_DEBUG_LOW))
                logger.log_direct(LOG_LEVEL_DEBUG_LOW, String().reserve(40 + msg.size()) << "MemcServer -- send_error: SERVER_ERROR " << msg);
        }
    }

    /** Helper for sending statistics.
     - Call from on_stats()
     - Call repeatedly for each statistic
     .
     \param  name   Stats name
     \param  value  Stats value
    */
    void send_stat(const SubString& name, const SubString& value) {
        AsyncServerReply::Writer writer(reply, id, 8 + name.size() + value.size());
        writer.add("STAT ", 5);
        writer.add(name.data(), name.size());
        writer.add(' ');
        writer.add(value.data(), value.size());
        writer.add("\r\n", 2);
    }

    /** Helper to send value for get response.
     - Call from on_get_start() or on_get() if key and value found
     .
     \param  key     Key for value
     \param  value   Value to send
     \param  flags   User defined flags stored with value
     \param  cas_id  Compare-And-Swap ID to send, NULL for none -- this must only be non-null for CAS GET request variants, see GetAdvParams
    */
    void send_value(const SubString& key, const SubString& value, uint32 flags=0, uint64* cas_id=NULL) {
        send_value_internal(id, key, value, flags, cas_id, AsyncServerReply::wfNONE);
        if (logger.check(LOG_LEVEL_DEBUG_LOW))
            logger.log_direct(LOG_LEVEL_DEBUG_LOW, String().reserve(42 + key.size()) << "MemcServer -- send_value '" << key << "' (size: " << value.size() << ')');
    }

private:
    /** Helper to send value for get response (used internally).
     - This is used by send_value() and deferred_reply_get() -- not used directly
     .
     \param  req_id        Request ID for response
     \param  key           Key for value
     \param  value         Value to send
     \param  flags         User defined flags stored with value
     \param  cas_id        Compare-And-Swap ID to send, NULL for none -- this must only be non-null for CAS GET request variants, see GetAdvParams
     \param  writer_flags  Flags passed to response writer
    */
    void send_value_internal(ulong req_id, const SubString& key, const SubString& value, uint32 flags, uint64* cas_id, AsyncServerReply::WriterFlags writer_flags) {
        StringInt<uint32> flags_str(flags);
        StringInt<StrSizeT> value_size_str(value.size());
        StringInt<uint64> cas_id_str;

        SizeT write_size = 12 + key.size() + flags_str.size() + value_size_str.size() + value.size();
        if (cas_id != NULL) {
            cas_id_str.set(*cas_id);
            write_size += 1 + cas_id_str.size();
        }

        AsyncServerReply::Writer writer(reply, req_id, write_size, writer_flags);
        writer.add("VALUE ", 6);
        writer.add(key.data(), key.size());
        writer.add(' ').add(flags_str.data(), flags_str.size());
        writer.add(' ').add(value_size_str.data(), value_size_str.size());
        if (cas_id != NULL)
            writer.add(' ').add(cas_id_str.data(), cas_id_str.size());
        writer.add("\r\n", 2);
        writer.add(value.data(), value.size());
        writer.add("\r\n", 2);
    }
};

///////////////////////////////////////////////////////////////////////////////

/** Implements %Memcached protocol for an async server.
 - See \ref Async for general overview
 .
Usage:
 - Implement a `HANDLER` class with the event (callback) methods
   - inherit MemcachedServerHandlerBase and implement desired methods -- see event methods there
   - implement `Global` and `Shared` nested structs if needed -- see \ref Async for details
     - Shared::on_init() is the place to start back-end connections
   - implement constructor with expected arguments: `Global&, Shared&`
 - Define a server type (via typedef) using `MemcachedServer<HANDLER>::%Server` -- a shortcut for `AsyncServer< MemcachedServer<HANDLER> >`
 - Create a listener Socket to use with the server
 - Instantiate a server and then:
   - Call setup methods as needed: \link AsyncServer::get_global() get_global()\endlink, \link AsyncServer::set_timeout() set_timeout()\endlink, \link AsyncServer::set_logger() set_logger()\endlink
 - Call \link AsyncServer::run() run()\endlink to run the server
 - A handler or another thread may call shutdown() to stop the server
 .

\par Example

A simple single-threaded memcached server:
\code
#include <evo/async/memcached_server.h>
#include <evo/maphash.h>
using namespace evo;

// Define a Handler type to handle memcached server events
struct Handler : async::MemcachedServerHandlerBase {
    struct Shared : SimpleSharedBase<> {
        StrHash map;
    };

    Shared& shared;

    Handler(Global& global, Shared& shared) : shared(shared) {
    }

    StoreResult on_store(StoreParams& params, SubString& value, Command command, uint64 cas_id) {
        switch(command) {
            case cSET:
                shared.map[params.key] = value;
                break;
            default:
                send_error("Not supported");
                return rtHANDLED;
        }
        return Memcached::srSTORED;
    }

    ResponseType on_get(const SubString& key, GetAdvParams* adv_params) {
        const String* val = shared.map.find(key);
        if (val != NULL)
            send_value(key, *val);
        return rtHANDLED;
    }
};

// Create Memcached Server class using Handler
typedef async::MemcachedServer<Handler>::Server Server;

int main() {
    Socket::sysinit();

    const ushort PORT = 11211;
    const ulong RD_TIMEOUT_MS = 5000;
    const ulong WR_TIMEOUT_MS = 1000;

    Socket listener;
    try {
        listener.listen_ip(PORT);
    } EVO_CATCH(return 1)

    Server server;
    server.set_timeout(RD_TIMEOUT_MS, WR_TIMEOUT_MS);
    server.run(listener);

    return 0;
}
\endcode
*/
template<class T>
struct MemcachedServer {
    static const size_t MIN_INITIAL_READ = 0;
    static const size_t NEWLINE_LEN = 2;

    typedef T Handler;                          ///< User defined handler type
    typedef MemcachedServer<Handler> This;      ///< This type
    typedef AsyncServer<This> Server;           ///< Server type
    typedef typename Handler::DeferredContext DeferredContext;  ///< Alias for Handler::DeferredContext
    typedef typename Handler::Global Global;    ///< Alias for Handler::Global
    typedef typename Handler::Shared Shared;    ///< Alias for Handler::Shared

    typedef MemcachedServerHandlerBase HandlerBase; ///< Alias for MemcachedServerHandlerBase

    LoggerPtr<> logger;                     ///< Logger to use
    Handler handler;                        ///< Handler instance

    /** Constructor to create server protocol instance.
     \param  global  Global data for requests
     \param  shared  Shared data for requests
     \param  logger  Logger to use, NULL for none
    */
    MemcachedServer(Global& global, Shared& shared, LoggerBase* logger) : logger(logger), handler(global, shared), command(HandlerBase::cUNKNOWN) {
        handler.logger.set(logger);
    }

    // Read events

    // Helper for common deferred count checks -- undef'd below
    #define EVO_HELPER_HANDLER_DEFCHECK(EVENT_NAME, EVENT_MSG_SUFFIX) \
        if (handler.noreply) { \
            logger.log(LOG_LEVEL_ERROR, "MemcServer " EVENT_NAME " error: Handler returned rtDEFERRED on 'noreply' request"); \
            return false; /* invalid result, close connection */ \
        } \
        if (++expected_deferred_count != context_ref.count()) { \
            if (expected_deferred_count > context_ref.count()) \
                logger.log(LOG_LEVEL_ERROR, "MemcServer " EVENT_NAME " error: Handler returned rtDEFERRED without creating a DeferredReply"); \
            else \
                logger.log(LOG_LEVEL_ERROR, "MemcServer " EVENT_NAME " error: Handler created more than one DeferredReply, only 1 allowed per handler"); \
            return false; /* invalid result, close connection */ \
        } \
        logger.log(LOG_LEVEL_DEBUG_LOW, "MemcServer -- " EVENT_NAME " response deferred" EVENT_MSG_SUFFIX);

    // Helper for common result.type cases -- undef'd below
    #define EVO_HELPER_HANDLER_CASES(EVENT_NAME) \
        case HandlerBase::rtDEFERRED: \
            EVO_HELPER_HANDLER_DEFCHECK(EVENT_NAME, "") \
            break; \
        case HandlerBase::rtHANDLED: break; \
        default: return false; /* rtCLOSE */

    // Helper for common handler end deferred-check -- undef'd below
    #define EVO_HELPER_HANDLER_END_DEFCHECK(EVENT_NAME) \
        if (expected_deferred_count != context_ref.count()) { \
            logger.log(LOG_LEVEL_ERROR, "MemcServer " EVENT_NAME " error: Handler created DeferredReply without returning rtDEFERRED"); \
            return false; /* invalid result, close connection */ \
        }

    bool on_read_fixed(SizeT& next_size, SubString& data, void* context) {
        // Read storage value
        EVO_PARAM_UNUSED(next_size);
        assert( context != NULL );
        String logstr;
        if (logger.check(LOG_LEVEL_DEBUG)) {
            logstr.set().reserve(96 + storage_params.key.size())
                << "MemcServer on_store " << HandlerBase::CommandEnum::get_string(command)
                << " '" << storage_params.key << "' fl:" << storage_params.flags << " exp:" << storage_params.expire;
            if (command == HandlerBase::cCAS)
                logstr << " id:" << storage_params.cas_id;
            logstr << " (size: " << storage_params.size << ')';
            logger.log_direct(LOG_LEVEL_DEBUG, logstr);
        }
        data.stripr("\r\n", NEWLINE_LEN, 1);

        DeferredContext& context_ref = *(DeferredContext*)context;
        ulong expected_deferred_count = context_ref.count();
        
        HandlerBase::StoreResult result = handler.on_store(context_ref, storage_params, data, command, storage_params.cas_id);
        switch (result.type) {
            case HandlerBase::rtNORMAL:
                switch (result.result) {
                    case Memcached::srSTORED:     reply("STORED\r\n", 8);      break;
                    case Memcached::srNOT_STORED: reply("NOT_STORED\r\n", 12); break;
                    case Memcached::srEXISTS:     reply("EXISTS\r\n", 8);      break;
                    case Memcached::srNOT_FOUND:  reply("NOT_FOUND\r\n", 11);  break;
                    default:
                        if (logger.check(LOG_LEVEL_ERROR))
                            logger.log_direct(LOG_LEVEL_ERROR, logstr.set().reserve(56) << "MemcServer on_store error: Invalid handler result: " << (int)result.result);
                        return false; // invalid result, close connection
                }
                break;
            EVO_HELPER_HANDLER_CASES("on_store");
        }
        EVO_HELPER_HANDLER_END_DEFCHECK("on_store");
        return true;
    }

    bool on_read(SizeT& fixed_size, AsyncBuffers& buffers, void* context) {
        // New command
        assert( context != NULL );

        DeferredContext& context_ref = *(DeferredContext*)context;
        ulong expected_deferred_count = context_ref.count();

        const char DELIM = ' ';
        String logstr;
        SubString line;
        while (buffers.read_line(line)) {
            SubString command_str, params_str;
            line.split(' ', command_str, params_str);
            handler.set_id();
            handler.noreply = false;

            command = Handler::CommandEnum::get_enum(command_str);
            switch (command) {
                // Storage
                case HandlerBase::cCAS:
                    if (!handler.enable_cas) {
                        handler.send_error("Not implemented");
                        break;
                    } // fallthrough
                case HandlerBase::cADD:      // fallthrough
                case HandlerBase::cAPPEND:   // fallthrough
                case HandlerBase::cPREPEND:  // fallthrough
                case HandlerBase::cREPLACE:  // fallthrough
                case HandlerBase::cSET: {
                    storage_params.key.set();
                    storage_params.flags  = 0;
                    storage_params.expire = 0;
                    storage_params.size   = 0;

                    StrTokWord tok(params_str);
                    for (;;) {
                        EVO_TOK_NEXT_OR_BREAK(tok, DELIM);  storage_params.key    = tok.value();
                        EVO_TOK_NEXT_OR_BREAK(tok, DELIM);  storage_params.flags  = tok.value().getnum<uint32>(fDEC);
                        EVO_TOK_NEXT_OR_BREAK(tok, DELIM);  storage_params.expire = tok.value().getnum<longl>(fDEC);
                        EVO_TOK_NEXT_OR_BREAK(tok, DELIM);  storage_params.size   = tok.value().getnum<ulong>(fDEC);
                        if (command == HandlerBase::cCAS) {
                            EVO_TOK_NEXT_OR_BREAK(tok, DELIM); storage_params.cas_id = tok.value().getnum<uint64>();
                        }
                        EVO_TOK_NEXT_OR_BREAK(tok, DELIM);  handler.noreply       = (tok.value() == "noreply");
                        break;
                    }
                    if (storage_params.key.empty()) {
                        handler.send_client_error("Missing parameter, expected key");
                        break;
                    }
                    buffers.read_flush();
                    if (handler.noreply)
                        handler.reply.nosend(handler.id);

                    if (!buffers.read_fixed_helper(*this, fixed_size, storage_params.size + NEWLINE_LEN, 0, context))
                        return false;
                    if (fixed_size > 0)
                        return true;
                    continue; // next command (while loop at top)
                }

                // Increment
                case HandlerBase::cINCREMENT:
                case HandlerBase::cDECREMENT: {
                    SubString key;
                    uint64 count = 0;
                    StrTokWord tok(params_str);
                    for (;;) {
                        EVO_TOK_NEXT_OR_BREAK(tok, DELIM);  key             = tok.value();
                        EVO_TOK_NEXT_OR_BREAK(tok, DELIM);  count           = tok.value().getnum<uint64>();
                        EVO_TOK_NEXT_OR_BREAK(tok, DELIM);  handler.noreply = (tok.value() == "noreply");
                        break;
                    }
                    if (key.empty()) {
                        handler.send_client_error("Missing parameter, expected key");
                        break;
                    }
                    if (logger.check(LOG_LEVEL_DEBUG))
                        logger.log_direct(LOG_LEVEL_DEBUG, logstr.set().reserve(42 + key.size()) << "MemcServer on_increment '" << key << "' " << (command == HandlerBase::cDECREMENT ? '-' : '+') << count);
                    if (handler.noreply)
                        handler.reply.nosend(handler.id);
                    HandlerBase::IncrementResult result = handler.on_increment(context_ref, key, count, (command == HandlerBase::cDECREMENT));
                    switch (result.type) {
                        case HandlerBase::rtNORMAL:
                            if (!result.result.null()) {
                                StringInt<uint64,NEWLINE_LEN> result_str(*result.result, fDEC, false, NEWLINE_LEN);
                                result_str.add("\r\n", NEWLINE_LEN);
                                reply(result_str.data(), result_str.size());
                            } else
                                reply("NOT_FOUND\r\n", 11);
                            break;
                        EVO_HELPER_HANDLER_CASES("on_increment");
                    }
                    EVO_HELPER_HANDLER_END_DEFCHECK("on_increment");
                    break;
                }

                // Delete
                case HandlerBase::cDELETE: {
                    SubString key;
                    StrTokWord tok(params_str);
                    for (;;) {
                        EVO_TOK_NEXT_OR_BREAK(tok, DELIM);  key             = tok.value();
                        EVO_TOK_NEXT_OR_BREAK(tok, DELIM);  handler.noreply = (tok.value() == "noreply");
                        break;
                    }
                    if (key.empty()) {
                        handler.send_client_error("Missing parameter, expected key(s)");
                        break;
                    }
                    if (logger.check(LOG_LEVEL_DEBUG))
                        logger.log_direct(LOG_LEVEL_DEBUG, logstr.set().reserve(24 + key.size()) << "MemcServer on_delete '" << key << '\'');
                    if (handler.noreply)
                        handler.reply.nosend(handler.id);
                    HandlerBase::DeleteResult result = handler.on_delete(context_ref, key);
                    switch (result.type) {
                        case HandlerBase::rtNORMAL:
                            if (result.result == HandlerBase::grOK)
                                reply("DELETED\r\n", 9);
                            else
                                reply("NOT_FOUND\r\n", 11);
                            break;
                        EVO_HELPER_HANDLER_CASES("on_delete");
                    }
                    EVO_HELPER_HANDLER_END_DEFCHECK("on_delete");
                    break;
                }

                // Touch
                case HandlerBase::cTOUCH: {
                    SubString key;
                    int64 expire = 0;
                    StrTokWord tok(params_str);
                    for (;;) {
                        EVO_TOK_NEXT_OR_BREAK(tok, DELIM);  key             = tok.value();
                        EVO_TOK_NEXT_OR_BREAK(tok, DELIM);  expire          = tok.value().getnum<int64>(fDEC);
                        EVO_TOK_NEXT_OR_BREAK(tok, DELIM);  handler.noreply = (tok.value() == "noreply");
                        break;
                    }
                    if (key.empty()) {
                        handler.send_client_error("Missing parameter, expected key(s)");
                        break;
                    }
                    if (logger.check(LOG_LEVEL_DEBUG))
                        logger.log_direct(LOG_LEVEL_DEBUG, logstr.set().reserve(24 + key.size()) << "MemcServer on_touch '" << key << '\'');
                    if (handler.noreply)
                        handler.reply.nosend(handler.id);
                    HandlerBase::TouchResult result = handler.on_touch(context_ref, key, expire);
                    switch (result.type) {
                        case HandlerBase::rtNORMAL:
                            if (result.result == HandlerBase::grOK)
                                reply("TOUCHED\r\n", 9);
                            else
                                reply("NOT_FOUND\r\n", 11);
                            break;
                        EVO_HELPER_HANDLER_CASES("on_touch");
                    }
                    EVO_HELPER_HANDLER_END_DEFCHECK("on_touch");
                    break;
                }

                // Get
                case HandlerBase::cGAT:
                case HandlerBase::cGATS:
                case HandlerBase::cGETS:
                case HandlerBase::cGET: {
                    bool handled  = false;
                    HandlerBase::GetAdvParams adv_params;
                    HandlerBase::GetAdvParams* adv_params_ptr = NULL;
                    switch (command) {
                        case HandlerBase::cGETS:
                            // CAS variant
                            if (!handler.enable_cas) {
                                handler.send_error("Not implemented");
                                handled = true;
                                break;
                            }
                            adv_params.cas = true;
                            adv_params_ptr = &adv_params;
                            break;
                        case HandlerBase::cGATS:
                            // GAT and CAS variant
                            if (!handler.enable_cas) {
                                handler.send_error("Not implemented");
                                handled = true;
                                break;
                            }
                            adv_params.cas = true; // fallthrough
                        case HandlerBase::cGAT: {
                            // GAT variant
                            if (!handler.enable_gat) {
                                handler.send_error("Not implemented");
                                handled = true;
                                break;
                            }
                            SubString expire_str;
                            if (!params_str.token(expire_str, DELIM) || expire_str.empty() || params_str.stripl(DELIM).empty()) {
                                handler.send_client_error("Missing parameter, expected expire value and key(s)");
                                handled = true;
                                break;
                            }
                            adv_params.expire = expire_str.getnum<uint64>();
                            adv_params_ptr = &adv_params;
                            break;
                        }
                        default:
                            if (params_str.empty()) {
                                handler.send_client_error("Missing parameter, expected key(s)");
                                handled = true;
                                break;
                            }
                            break;
                    }
                    if (handled)
                        break;

                    if (adv_params_ptr != NULL && logger.check(LOG_LEVEL_DEBUG_LOW)) {
                        logstr.set().reserve(48) << "MemcServer get adv:" << command_str;
                        if (!adv_params.expire.null())
                            logstr << " exp:" << *adv_params.expire;
                        logger.log_direct(LOG_LEVEL_DEBUG_LOW, logstr);
                    }

                    uint deferred = 0;
                    HandlerBase::GetStartResult result = handler.on_get_start(context_ref, params_str, adv_params_ptr);
                    switch (result.type) {
                        case HandlerBase::rtNORMAL:
                            switch (result.result) {
                                case HandlerBase::gsrCONTINUE: {
                                    // Call on_get() for each key
                                    StrTokWord tok(params_str);
                                    while (tok.nextw(DELIM)) {
                                        if (logger.check(LOG_LEVEL_DEBUG))
                                            logger.log_direct(LOG_LEVEL_DEBUG, logstr.set().reserve(24 + tok.value().size()) << "MemcServer on_get '" << tok.value() << '\'');
                                        switch (handler.on_get(context_ref, tok.value(), adv_params_ptr)) {
                                            case HandlerBase::rtNORMAL:   break;
                                            case HandlerBase::rtHANDLED:  handled = true; break;
                                            case HandlerBase::rtDEFERRED: ++deferred; EVO_HELPER_HANDLER_DEFCHECK("on_get", " from on_get()"); break;
                                            default: return false; // rtCLOSE
                                        }
                                        EVO_HELPER_HANDLER_END_DEFCHECK("on_get");
                                        if (handled)
                                            break;
                                    }
                                }
                                case HandlerBase::gsrSKIP:
                                    break;
                            }
                            break;
                        case HandlerBase::rtDEFERRED: ++deferred; EVO_HELPER_HANDLER_DEFCHECK("on_get", " from on_get_start()"); break;
                        case HandlerBase::rtHANDLED:  handled = true; break;
                        default: return false; // rtCLOSE
                    }
                    EVO_HELPER_HANDLER_END_DEFCHECK("on_get_start");

                    if (!handled) {
                        switch (handler.on_get_end(context_ref)) {
                            case HandlerBase::rtNORMAL:   break;
                            case HandlerBase::rtHANDLED:  handled = true; break;
                            case HandlerBase::rtDEFERRED: ++deferred; EVO_HELPER_HANDLER_DEFCHECK("on_get", " from on_get_end()"); break;
                            default: return false; // rtCLOSE
                        }
                        EVO_HELPER_HANDLER_END_DEFCHECK("on_get_end");

                        if (!handled && deferred == 0) {
                            String buf("END\r\n", 5);
                            handler.reply.send(handler.id, buf);
                            handler.reply.send_end();
                        }
                    }
                    break;
                }

                // Misc
                case HandlerBase::cSTATS: handler.on_stats(params_str); break;
                case HandlerBase::cQUIT:  return false;

                // Unknown
                default: {
                    bool handled = false;
                    if (!handler.on_command(handled, command_str, params_str))
                        return false;
                    if (!handled)
                        reply("ERROR\r\n", 7);
                    break;
                }
            }
            buffers.read_flush();
        }
        return true;
    }

    void on_error(AsyncError err) {
        handler.on_error(err);
    }

    #undef EVO_HELPER_HANDLER_END_DEFCHECK
    #undef EVO_HELPER_HANDLER_CASES
    #undef EVO_HELPER_HANDLER_DEFCHECK

private:
    HandlerBase::Command     command;
    HandlerBase::StoreParams storage_params;

    // String literals only
    void reply(const char* data, StrSizeT size) {
        if (!handler.noreply) {
            String data_str(data, size);
            handler.reply.send(handler.id, data_str);
            handler.reply.send_end();
        }
    }
};

///////////////////////////////////////////////////////////////////////////////
//@}
}
}
#endif
