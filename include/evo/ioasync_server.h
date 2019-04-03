// Evo C++ Library
/* Copyright 2019 Justin Crowell
Distributed under the BSD 2-Clause License -- see included file LICENSE.txt for details.
*/
///////////////////////////////////////////////////////////////////////////////
/** \file ioasync_server.h Evo AsyncServer. */
#pragma once
#ifndef INCL_evo_ioasync_server_h
#define INCL_evo_ioasync_server_h

#include "ioasync_base.h"

namespace evo {
/** \addtogroup EvoIO */
//@{

///////////////////////////////////////////////////////////////////////////////

/** Handles sending server replies, and accounts for potentially out of order responses.
 - This writes directly to the socket when possible -- out of order responses are queued to maintain correct response order
 - This is used by AsyncServerHandler, the base class for handlers used with AsyncServer
*/
template<class T=AsyncBuffers>
class AsyncServerReplyT {
public:
    typedef T OutBuffer;                ///< Output buffer type
    typedef AsyncServerReplyT<T> This;  ///< This type

    /** Flags used with Writer. */
    enum WriterFlags {
        wfNONE = 0,
        wfDEFERRED = 0x01,      ///< Deferred but not the last part of this response
        wfDEFERRED_LAST = 0x03  ///< Deferred and the last part of this response
    };

    /** Response writer used to group multiple writes together for best performance.
     - This is similar to send(), but allows batched writing directly to the socket when possible
     - This requires knowing the batch size before actually writing
     - See write methods in AsyncBuffers::BulkWrite
     - The destructor commits the written data
     - \b Caution: Flags must be correct for deferred or non-deferred response, otherwise results are undefined
    */
    struct Writer : public OutBuffer::BulkWrite {
        using OutBuffer::BulkWrite::init;

        /** Constructor.
         \param  parent    Parent reply object to use
         \param  id        Request ID for response
         \param  buf_size  Data size that will be written -- must exactly match data that will be written
         \param  flags     Flags used for deferred response, use:
                            - wfDEFERRED_LAST for last deferred write
                            - wfDEFERRED for part of deferred write, but more writes required for this response
                            - wfNONE for non-deferred
        */
        Writer(This& parent, ulong id, SizeT buf_size, WriterFlags flags=wfNONE) {
            if (int(flags) & wfDEFERRED) {
                // Deferred
                parent.send_end();
                if (id == parent.next_id_) {
                    // This is the current reply
                    init(parent.buf_, buf_size);
                    if (flags == wfDEFERRED_LAST)
                        ++parent.next_id_;
                } else {
                    // Either append to existing reply in queue, or insert new reply -- keep queue ordered by ID
                    ReplyItem* rsp;
                    parent.deferred_get_queue_item(rsp, id);
                    init(rsp->data, buf_size);
                }
            } else if (id == parent.prev_id_) {
                // Append to previous send
                if (parent.prev_ == NULL)
                    init(parent.buf_, buf_size);
                else
                    init(parent.prev_->data, buf_size);
            } else {
                // New send
                parent.send_end();
                if (id == parent.next_id_) {
                    init(parent.buf_, buf_size);
                    ++parent.next_id_;
                    parent.prev_ = NULL;
                } else {
                    parent.prev_ = parent.queue_.addnew().advLast();
                    parent.prev_->id = id;
                    init(parent.prev_->data, buf_size);
                }
                parent.prev_id_ = id;
            }
        }
    };

    /** Constructor.
     \param  bufs  Buffer to use for writes
    */
    AsyncServerReplyT(T& bufs) : buf_(bufs), deferred_count_(0), gen_id_(1), next_id_(1), prev_id_(0), prev_(NULL) {
    }

    /** Generate a new request ID.
     \return  New request ID
    */
    ulong gen_id() {
        return gen_id_++;
    }

    /** Get current number of deferred responses in progress.
     \return  Number of current deferred responses in progress, 0 for none
    */
    ulong deferred_active() const {
        return deferred_count_;
    }

    /** Call when deferred response is started.
     - This is used to track the current number of deferred events are being waited on
     .
     \param  context  `DeferredContext` for current connection
     \return          Whether this was the last context reference, meaning context was destroyed, false if not destroyed
    */
    template<class U>
    void deferred_start(U& context) {
        ++deferred_count_;
        context.addref();
    }

    /** Call when deferred response is finished.
     - This is used to track the current number of deferred events are being waited on
     - Call this only after response is fully sent with send()
     - This calls send_end()
     .
     \param  context  `DeferredContext` for current connection
     \return          Whether this was the last context reference, meaning context was destroyed, false if not destroyed
    */
    template<class U>
    bool deferred_end(U& context) {
        --deferred_count_;
        send_end();
        return context.endref();
    }

    /** Send deferred response for given request ID.
     - Call gen_id() before response is deferred to get an ID to use
     - Do not call this with non-deferred response, use send() instead
     - Multiple calls with the same ID are effectively appended together, but must set `last=true` on last part of response
     - \b Caution: Results are undefined if `last` isn't set correctly
     .
     \param  id    Request ID for response
     \param  data  Response data to send
     \param  last  Whether this is the last part of the response, true to allow processing of next request response
     \return       This
    */
    This& deferred_send(ulong id, String& data, bool last) {
        send_end();
        if (id == next_id_) {
            // This is the current reply
            buf_.write(data.data(), data.size());
            if (last)
                ++next_id_;
        } else {
            // Either append to existing reply in queue, or insert new reply -- keep queue ordered by ID
            ReplyItem* rsp;
            if (deferred_get_queue_item(rsp, id))
                rsp->data.add(data);
            else
                rsp->data = data;
        }
        return *this;
    }

    /** Send response for given request ID.
     - Call gen_id() to get an ID to use
     - Do not call this with deferred response, use deferred_send() instead
     - Multiple calls with the same ID are effectively appended together
     - Responses sent out of order are queued so they're actually sent in the right order
     .
     \param  id    Request ID for response
     \param  data  Response data to send
    */
    void send(ulong id, String& data) {
        if (id == prev_id_) {
            // Append to previous send
            if (prev_ == NULL)
                buf_.write(data.data(), data.size());
            else
                prev_->data.add(data);
        } else {
            // New send
            send_end();
            if (id == next_id_) {
                buf_.write(data.data(), data.size());
                ++next_id_;
            } else {
                prev_ = queue_.addnew().advLast();
                prev_->id = id;
                prev_->data = data;
            }
            prev_id_ = id;
        }
    }

    /** End current response.
     - Call after last send() for current response
     - Not needed for deferred responses, deferred_end() calls this
    */
    void send_end() {
        ReplyItem* rsp;
        while ((rsp = queue_.advFirst()) != NULL && rsp->id == next_id_) {
            buf_.write(rsp->data.data(), rsp->data.size());
            queue_.popq();
            ++next_id_;
        }
    }

    /** Cancel current ID since current request doesn't have a response.
     - This must be called before the next ID is created (i.e. before next request is processed), otherwise the connection will likely hang
     .
     \param  id  ID to cancel
    */
    void nosend(ulong id) {
        if (id + 1 == gen_id_)
            --gen_id_;
    }

private:
    // Disable copying
    AsyncServerReplyT(const This&);
    This& operator=(const This&);

    // Reply item in queue
    struct ReplyItem {
        ulong id;
        String data;

        ReplyItem() : id(0) {
        }
        ReplyItem(const ReplyItem& src) : id(src.id), data(src.data) {
        }
        ReplyItem& operator=(const ReplyItem& src) {
            id = src.id;
            data = src.data;
            return *this;
        }
    };

    OutBuffer& buf_;        // write buffer
    ulong deferred_count_;  // deferred replies in progress, 0 for none
    ulong gen_id_;          // next ID to generate
    ulong next_id_;         // next ID to send
    List<ReplyItem> queue_; // reply queue, only used while replies are out of order
    ulong prev_id_;         // previously queued/sent request ID -- not used by deferred_send()
    ReplyItem* prev_;       // previously queued item, NULL if none -- not used by deferred_send()

    bool deferred_get_queue_item(ReplyItem*& rsp, ulong id) {
        SizeT i = 0, sz = queue_.size();
        if (sz > 0 && id > queue_[sz-1].id) {
            i = sz; // ID not in queue, insert at end
        } else {
            for (; i < sz; ++i) {
                rsp = &queue_.advItem(i);
                if (id < rsp->id)
                    break; // ID not in queue, insert here
                if (id == rsp->id) {
                    // Use existing reply in queue
                    return true;
                }
            }
        }

        // Insert new reply in queue
        rsp = &queue_.advItem(queue_.insertnew(i));
        rsp->id = id;
        return false;
    }
};

typedef AsyncServerReplyT<> AsyncServerReply;   ///< Handles sending server replies -- see AsyncServerReplyT

///////////////////////////////////////////////////////////////////////////////

/** Base async I/O server handler.
 - See \ref Async
*/
struct AsyncServerHandler {
    /** Holds a context for deferred responses in progress.
     - This is detached from the handler when the connection is destroyed, but will persist until the last deferred response so responses are cleaned up
     - A protocol handler should typedef this as `DeferredContenxt` and implement `DeferredReply` inheriting ReplyBase
     - How it works:
       - When the server creates a connection supports deferred responses it must also create a `DeferredContext` (using operator `new`) and call detach() when connection is destroyed
       - This holds a reference count, which starts at 1 for the connection
       - detach() sets `handler=NULL` and decrements the reference count -- deferred responses after that should be no-ops and just cleanup
       - When a response is deferred then addref() must be called to increment the reference count
       - When a deferred response is sent then endref() must be called to decrement the reference count (which will `delete this` after last deferred response if connection was destroyed)
     .
     \tparam  T  Protocol base handler type to use, i.e. `ProtocolServerHandlerBase`
    */
    template<class T>
    struct DeferredContextT {
        typedef DeferredContextT<T> Context;    ///< Alias for this context

        /** Base class for deferred reply.
         - Implemented as DeferredReply by protocol handler
         - Derived destructor should send an error response if `!finished` to keep server stable
           - Example:
             \code
                if (!finished && context.handler != NULL) {
                    context.handler->logger.log(LOG_LEVEL_ERROR, "AsyncServer DeferredReply left unfinished");
                    deferred_reply_error("Internal handler error: DeferredReply left unfinished");
                }
             \endcode
        */
        struct ReplyBase {
            Context& context;   ///< Reference to context used for reply
            ulong id;           ///< Response ID to use for reply

            ReplyBase(Context& context, ulong id) : context(context), id(id), finished(false) {
                context.handler->reply.deferred_start(context);
            }

            ~ReplyBase() {
                if (!finished && context.handler != NULL)
                    context.handler->logger.log(LOG_LEVEL_ERROR, "AsyncServer DeferredReply left unfinished");
            }

        protected:
            bool finished;      ///< Whether deferred response is finished
        };

        typedef T Handler;  ///< Handler type
        Handler* handler;   ///< Pointer to handler for sending deferred reply, NULL when connection is destroyed

        /** Constructor.
         \param  handler  `HANDLER` used
        */
        DeferredContextT(Handler& handler) : handler(&handler), refcount(1) {
        }

        /** Get current pending deferred response count.
         \return  Pending deferred response count
        */
        ulong count() const {
            return refcount;
        }

        /** Call when a deferred response is started. */
        void addref() {
            ++refcount;
        }

        /** Call to cleanup after deferred response is sent/finished.
         \return  Whether this was the last reference, meaning this is destroyed, false if not destroyed
        */
        bool endref() {
            if (--refcount == 0) {
                assert( handler == NULL ); // failure here indicates deferred start/end call mismatch (shouldn't happen)
                delete this;
                return true;
            }
            return false;
        }

        /** Call when server connection is destroyed.
         - Called from server Connection destructor
         .
         \return  Whether this was the last reference, meaning this is destroyed, false if not destroyed
        */
        bool detach() {
            handler = NULL;
            return endref();
        }

    private:
        ulong refcount;
    };

    /** Handler response type -- used with ResponseResult. */
    enum ResponseType {
        rtNORMAL,       ///< Normal response
        rtDEFERRED,     ///< Defer response while waiting for an event -- an error if deferred response not supported under current conditions
        rtHANDLED,      ///< Response already sent so request is handled, use if error was sent
        rtCLOSE         ///< Close connection
    };

    /** Handler response result.
     - This pairs a ResponseType enum with a templated enum (ResultType) to create a handler response type
     - Handlers can return either:
       - a ResponseType to indicate a non-normal result (`result` defaults to: `(T)0`)
       - a `T` value to indicate a normal result (`type` defaults to: `rtNORMAL`)
    */
    template<class T>
    struct ResponseResult {
        typedef T ResultType;   ///< Result type for rtNORMAL response

        ResponseType type;  ///< Response type -- see ResponseType
        ResultType result;  ///< Normal response result -- ignored unless `type=rtNORMAL`

        /** Default constructor initializes as rtCLOSE -- other constructors preferred. */
        ResponseResult() : type(rtCLOSE), result((ResultType)0) {
        }

        /** Copy constructor.
         \param  src  Source to copy
        */
        ResponseResult(const ResponseResult& src) : type(src.type), result(src.result) {
        }

        /** Constructor for ResponseResult -- use for non rtNORMAL response.
         \param  type  Response type, should not be rtNORMAL -- use ResponseResult(ResultType) for rtNORMAL result
        */
        ResponseResult(ResponseType type) : type(type), result((ResultType)0) {
        }

        /** Constructor for ResultType -- use for rtNORMAL response.
         \param  result  Result value -- use ResponseResult(ResponseType) for non rtNORMAL result
        */
        ResponseResult(ResultType result) : type(rtNORMAL), result(result) {
        }

        /** Assignment operator for ResponseResult -- use for non rtNORMAL response.
         \param  newtype  Response type, should not be rtNORMAL -- use ResponseResult(ResultType) for rtNORMAL result
         \return          This
        */
        ResponseResult& operator=(ResponseType newtype) {
            type = newtype;
            result = (T)0;
            return *this;
        }

        /** Assignment operator for ResponseResult -- use for non rtNORMAL response.
         \param  newresult  Result value -- use ResponseResult(ResponseType) for non rtNORMAL result
         \return            This
        */
        ResponseResult& operator=(ResultType newresult) {
            type = rtNORMAL;
            result = newresult;
            return *this;
        }

        /** Assignment operator to copy.
         \param  src  Source to copy
         \return      This
        */
        ResponseResult& operator=(const ResponseResult& src) {
            type = src.type;
            result = src.result;
            return * this;
        }
    };

    /** Max initial read size.
     - Default is 8 KB, but the protocol handler may override with a new default
     - Override to tune
     - Use 0 for unlimited (not recommended)
    */
    static const size_t MAX_INITIAL_READ = 8192;   // 8 KB

    /** Default global data (empty).
     - When overriding this `Global` struct, `Shared` must also be overridden since it
    */
    struct Global {
    };

    /** Default shared data (empty) with template parameter for global data type.
     - The user defined `ServerHandler` may define a `Shared` type that inherits this
     - \b Caution: This must match the `Global` type used, otherwise you'll get compiler errors
     .
     \tparam  T  `Global` type to use
    */
    template<class T=Global>
    struct SimpleSharedBase {
        typedef T Global;   ///< Global data type used

        /** Called when server is initialized, before any connections are accepted.
         - When overriding this `Shared` struct, this is the place to attach any async clients so they use the server event-loop
         .
         \param  server  Server AsyncBase instance
         \param  global  Reference to global data
         \return         Whether successfull, false on critical error (which prevents server startup)
        */
        bool on_init(AsyncBase& server, Global& global) {
            EVO_PARAM_UNUSED(server);
            EVO_PARAM_UNUSED(global);
            return true;
        }

        /** Called when server is shutting down, after last request has completed. */
        void on_uninit() {
        }
    };

    /** Default shared data (empty) using default global data type.
     - The user defined `ServerHandler` may define a `Shared` type that inherits this
     - \b Caution: Only use this if your `ServerHandler` does _not_ define a `Global` type (or use SimpleSharedBase), otherwise you'll get compiler errors
    */
    struct Shared : SimpleSharedBase<> {
    };

    AsyncBuffers buffers;   ///< Buffers for async I/O
    AsyncServerReply reply; ///< Server reply manager, used to track deferred events and queue out of order replies
    ulong id;               ///< Request/reply ID, used by reply manager (set by parent protocol class)

    /** Constructor. */
    AsyncServerHandler() : reply(buffers), id(0) {
    }

    /** Create and set new ID for current request/response.
     - Called by parent protocol class
    */
    void set_id() {
        id = reply.gen_id();
    }
};

///////////////////////////////////////////////////////////////////////////////

/** Async I/O server for receiving and handling requests.

This is a template class that uses a `ProtocolServer` (`T`) which implements the server request handling via async I/O event methods.
 - The `ProtocolServer` is usually also a template class that implements the protocol and uses a user-defined, protocol-specific event-handler
 .

See Server section in: \ref Async "Asynchronous I/O"

\par ProtocolServer Requirements

The `ProtocolServer` (`T`) must define this nested type:
 - `T::Handler`: An event handler inheriting AsyncServerHandler (directly or indirectly) and implementing protocol-specific event methods
 .

The `ProtocolServer` (`T`) must implement these event methods:
 - \code
   bool on_read(size_t& fixed_size, AsyncBuffers& buffers)
   \endcode
   - Called when data is available to read
   - If not enough data is available, return true to wait for more
   - Return false to immediately close the connection
   - This will usually have a loop like this to read line by line:
     \code
     SubString line;
     while (buffers.read_line(line)) {
        // ...
        buffers.read_flush();
     }
     \endcode
   - To read fixed size data (i.e. read an additional `N` bytes): call \link AsyncBuffers::read_fixed() buffers.read_fixed()\endlink, and if that fails (i.e. returns `false`):
     - This means more data is needed from the client
     - To wait for more data: set `fixed_size` (out param) to the desired read size and return `true`
     - Then `on_read_fixed()` will be called when enough data arrives
     - Use \link AsyncBuffers::read_fixed_helper() buffers.read_fixed_helper()\endlink to make this easier in most cases, so all fixed-size reads go through the `on_read_fixed()` event method
     - \b Caution: Do not modify `fixed_size` unless \link AsyncBuffers::read_fixed() buffers.read_fixed()\endlink has been called and returned false   
   .
 - \code
   bool on_read_fixed(size_t& next_size, SubString& data)
   \endcode
   - Called instead of `on_read()` when reading fixed-size data
     - This is only called if `on_read()` set a `fixed_size`
     - If not used this will not be called -- just return false
   - Return false to immediately close connection
   - Memory referenced by `data` param will not be valid after this returns
   .
 - \code
   void on_error(AsyncError err)
   \endcode
   - Called when the client has closed the connection, or when a socket error ocurred
   .
 .

The ProtocolServer (`T`) must define this constant:
 - `MIN_INITIAL_READ`: Minimum size (in bytes) needed before calling `on_read()` the first time for a request, 0 for any amount
   \code
   static const size_t MIN_INITIAL_READ = 0;
   \endcode
 .

\tparam  T  ProtocolServer type to use (see above)
*/
template<class T>
class AsyncServer : public AsyncBase {
public:
    typedef T ProtocolServer;
    typedef AsyncServer<ProtocolServer> This;
    typedef typename ProtocolServer::Handler::DeferredContext DeferredContext;
    typedef typename ProtocolServer::Handler::Global Global;
    typedef typename ProtocolServer::Handler::Shared Shared;

    struct Stats {
        ulong active_connections;
        ulong accept_ok;
        ulong accept_err;
        ulong event_err;
        ulong reads;

        Stats() {
            active_connections = 0;
            accept_ok  = 0;
            accept_err = 0;
            event_err  = 0;
            reads      = 0;
        }
    };

    /** Constructor. */
    AsyncServer() : last_id_(0) {
        init();
    }

    /** Get reference to global data used by all requests and all threads in this server.
     - Use this to populate configuration data before running the server
     .
     \return  Reference to global data
    */
    Global& get_global() {
        return global_;
    }

    /** Run server event handling and handle connections until shutdown.
     - This initializes (calls `Shared::on_init()`) and runs the event-loop to accept and handle connections
     - This keeps running until the listener socket is closed or shutdown() is called
     - \b Caution: Only 1 thread may handle connections at a time, otherwise results are undefined
     .
     \param  listener  Listener socket handle to accept connections on
     \return           Whether successful, false if initialization failed (`Shared::on_init()` or internal error)
    */
    bool run(IoSocket::Handle listener) {
        {
            IoSocket listener_socket(listener);
            Error err = listener_socket.set_nonblock();
            if (err != ENone) {
                if (logger.check(LOG_LEVEL_ALERT)) {
                    String msg("AsyncServer listener error setting as non-blocking: ");
                    IoSocket::errormsg_out(msg, err);
                    logger.log_direct(LOG_LEVEL_ALERT, msg);
                }
                return false;
            }
            listener_socket.detach();
        }

        struct event* ev = ::event_new(evloop_->handle(), listener, EV_READ | EV_PERSIST, on_listener_ready, this);
        if (ev == NULL) {
            logger.log(LOG_LEVEL_ALERT, "AsyncServer libevent event_new() failed on listener -- this shouldn't happen");
            return false;
        }

        if (::event_add(ev, NULL) != 0) {
            logger.log(LOG_LEVEL_ALERT, "AsyncServer libevent event_add() failed on listener -- this shouldn't happen");
            return false;
        }

        if (!shared_.on_init(*this, global_)) {
            logger.log(LOG_LEVEL_ALERT, "AsyncServer Shared on_init() returned an error, indicating a bad configuration");
            return false;
        } else
            logger.log(LOG_LEVEL_DEBUG, "AsyncServer ready");

        run_eventloop();
        ::event_free(ev);
        return true;
    }

    /** Run server and handle connections until shutdown.
     - This runs the event-loop to accept and handle connections
     - This keeps running until the listener socket is closed or shutdown() is called
     - \b Caution: Only 1 thread may handle connections at a time, otherwise results are undefined
     .
     \param  listener  Listener socket to accept connections on
     \return           Whether successful, false if initialization failed (`Shared::on_init()` or internal error)
    */
    bool run(Socket& listener) {
        return run(listener.device().handle);
    }

    /** Shut down server.
     - The server will stop accepting new requests and will finish handling current requests
     - This doesn't block and can be called from any thread or event handler
    */
    void shutdown() {
        evloop_->shutdown();
    }

private:
    Global global_;
    Shared shared_;
    Stats stats_;
    ulong last_id_;

    using AsyncBase::runlocal;

    struct Connection {
        This& server;                       ///< AsyncServer reference
        DeferredContext* deferred_context;  ///< Deferred context pointer, handles deferred replies
        ProtocolServer   protocol_server;   ///< Protocol server instance for connection
        struct bufferevent* bev;            ///< Low-level event object for connection
        SizeT  read_fixed_size_;            ///< Size for fixed-size read
        ulong id;                           ///< Connection ID

        Connection(This& async_server, struct bufferevent* bev, ulong id) :
                server(async_server), protocol_server(async_server.global_, async_server.shared_, async_server.logger.ptr), bev(bev), read_fixed_size_(0), id(id) {
            deferred_context = new DeferredContext(protocol_server.handler);

            ::bufferevent_setcb(bev, on_read, NULL, on_error, this);
            ::bufferevent_setwatermark(bev, EV_READ, T::MIN_INITIAL_READ, T::Handler::MAX_INITIAL_READ);
            if (server.read_timeout_ms_ > 0 || server.write_timeout_ms_ > 0) {
                struct timeval read_timeout, write_timeout;
                const int result = ::bufferevent_set_timeouts(bev,
                    get_timeout_ptr(read_timeout, server.read_timeout_ms_),
                    get_timeout_ptr(write_timeout, server.write_timeout_ms_)
                );
                if (result != 0)
                    server.logger.log(LOG_LEVEL_ERROR, "AsyncServer libevent bufferevent_set_timeouts() returned an error -- this shouldn't happen");
            }
        }

        ~Connection() {
            ::bufferevent_free(bev);
            --server.stats_.active_connections;
            if (!deferred_context->detach())
                server.logger.log(LOG_LEVEL_DEBUG_LOW, "AsyncServer cleanup, deferred pending");
        }

        bool enable() {
            if (::bufferevent_enable(bev, EV_READ | EV_WRITE) != 0) {
                server.logger.log(LOG_LEVEL_ALERT, "AsyncServer libevent bufferevent_enable() returned an error -- this shouldn't happen");
                return false;
            }
            return true;
        }
    };

    static void on_listener_ready(evutil_socket_t listener, short event, void* self_ptr) {
        EVO_PARAM_UNUSED(event);
        This& self = *(This*)self_ptr;
        Error err;
        IoSocket listener_socket(listener), client_socket;
        if (!listener_socket.accept_nonblock(err, client_socket)) {
            if (self.logger.check(LOG_LEVEL_ALERT)) {
                String msg;
                msg = "AsyncServer socket accept failed: ";
                IoSocket::errormsg_out(msg, err);
                self.logger.log_direct(LOG_LEVEL_ALERT, msg);
            }
            ++self.stats_.accept_err;
            return;
        }
        listener_socket.detach();

        struct bufferevent* bev = ::bufferevent_socket_new(self.evloop_->handle(), client_socket.detach(), BEV_OPT_CLOSE_ON_FREE);
        if (bev == NULL) {
            self.logger.log(LOG_LEVEL_ALERT, "AsyncServer libevent bufferevent_socket_new() returned an error -- this shouldn't happen");
            ++self.stats_.accept_err;
            return;
        }

        // Connection takes ownership of bev, and is freed by on_read() or on_error() (when connection is closed)
        Connection* conn = new Connection(self, bev, ++self.last_id_);
        ++self.stats_.active_connections;
        if (!conn->enable()) {
            delete conn; // failed to enable connection event handling
            ++self.stats_.accept_err;
        } else
            ++self.stats_.accept_ok;
    }

    static void on_read(struct bufferevent* bev, void* conn_ptr) {
        Connection* conn = (Connection*)conn_ptr;
        LoggerPtr<>& logger = conn->server.logger;
        ++conn->server.stats_.reads;
        conn->protocol_server.handler.buffers.attach(bev);

        String logstr;
        AsyncBuffers* bufs = &conn->protocol_server.handler.buffers;
        if (conn->read_fixed_size_ > 0) {
            if (logger.check(LOG_LEVEL_DEBUG_LOW))
                logger.log_direct(LOG_LEVEL_DEBUG_LOW, logstr.set().reserve(72) << "AsyncServer connection " << conn->id << " fixed read: " << conn->read_fixed_size_);
            for (;;) {
                SubString data;
                if (!bufs->read_fixed(data, conn->read_fixed_size_))
                    return; // wait for more data
                conn->read_fixed_size_ = 0;
                if (!conn->protocol_server.on_read_fixed(conn->read_fixed_size_, data, conn->deferred_context)) {
                    delete conn;
                    if (logger.check(LOG_LEVEL_DEBUG_LOW))
                        logger.log(LOG_LEVEL_DEBUG_LOW, logstr.set().reserve(72) << "AsyncServer connection " << conn->id << " on_read_fixed() returned false to close");
                    return;
                }
                bufs->read_flush();
                if (conn->read_fixed_size_ <= 0)
                    break;
            }
            bufs->read_reset(ProtocolServer::MIN_INITIAL_READ, ProtocolServer::Handler::MAX_INITIAL_READ);
            if (bufs->read_size() == 0)
                return;
        }

        if (logger.check(LOG_LEVEL_DEBUG_LOW))
            logger.log_direct(LOG_LEVEL_DEBUG_LOW, logstr.set().reserve(72) << "AsyncServer connection " << conn->id << " read: " << bufs->read_size());
        if (!conn->protocol_server.on_read(conn->read_fixed_size_, *bufs, conn->deferred_context)) {
            delete conn;
            if (logger.check(LOG_LEVEL_DEBUG_LOW))
                logger.log_direct(LOG_LEVEL_DEBUG_LOW, logstr.set().reserve(72) << "AsyncServer connection " << conn->id << " on_read() returned false to close");
        }
    }

    static void on_error(struct bufferevent* bev, short error, void* conn_ptr) {
        EVO_PARAM_UNUSED(bev);
        Connection* conn = (Connection*)conn_ptr;
        AsyncError err;
        if (error & BEV_EVENT_EOF)
            err = aeCLOSED;
        else if (error & BEV_EVENT_TIMEOUT)
            err = aeTIMEOUT;
        else if (error & BEV_EVENT_READING)
            err = aeIO_READ;
        else if (error & BEV_EVENT_WRITING)
            err = aeIO_WRITE;
        else
            err = aeIO;
        ++conn->server.stats_.event_err;
        conn->protocol_server.on_error(err);

        const ulong conn_id = conn->id;
        LoggerPtr<>& logger = conn->server.logger;
        delete conn;

        if (logger.check(LOG_LEVEL_DEBUG_LOW)) {
            const SubString errmsg(async_error_msg(err));
            logger.log_direct(LOG_LEVEL_DEBUG, String().reserve(48 + errmsg.size()) << "AsyncServer connection " << conn_id << " error: " << errmsg);
        }
    }
};

///////////////////////////////////////////////////////////////////////////////
//@}
}
#endif
