// Evo C++ Library
/* Copyright 2019 Justin Crowell
Distributed under the BSD 2-Clause License -- see included file LICENSE.txt for details.
*/
///////////////////////////////////////////////////////////////////////////////
/** \file ioasync_client.h Evo AsyncClient. */
#pragma once
#ifndef INCL_evo_ioasync_client_h
#define INCL_evo_ioasync_client_h

#include "ioasync_base.h"

namespace evo {
/** \addtogroup EvoIO */
//@{

///////////////////////////////////////////////////////////////////////////////

/** Template class for an async I/O client.
 - This is inherited by a client implementing a given protocol -- not used directly
 - See \ref Async
 .
 \tparam  T  Protocol handler type, which must inherit from AsyncClient
 \tparam  Q  Response queue type that works with the protocol used
              - Must have `null()` and `set()` methods, and must default construct as null
*/
template<class T, class Q>
class AsyncClient : public AsyncBase {
public:
    typedef T ProtocolHandler;      ///< Derived protocol hander type (must inherit from AsyncClient)
    typedef Q QueueItem;            ///< Response queue item type
    typedef AsyncClient<T,Q> This;  ///< %This type

    /** Client connected event. */
    struct OnConnect {
        /** Destructor. */
        virtual ~OnConnect() { }

        /** Called when client is connected. */
        virtual void on_connect() { }
    };

    /** Client error event. */
    struct OnError {
        /** Destructor. */
        virtual ~OnError() { }

        /** Called on an error that breaks the connection.
         \param  error  Error value
        */
        virtual void on_error(AsyncError error) {
            EVO_PARAM_UNUSED(error);
        }
    };

    /** Client state. */
    enum State {
        sNONE = 0,      ///< No connection
        sCONNECTING,    ///< Connection in progress
        sCONNECTED      ///< Connected
    };

    /** Constructor.
     \param  max_queue_size  Max size for pending response queue
     \param  max_read_size   Max read buffer size, 0 for unlimited -- this is used to limit the read buffer size
    */
    AsyncClient(SizeT max_queue_size, SizeT max_read_size) : queue_(max_queue_size), id_(get_next_id()), state_(sNONE), bev_(NULL), on_connect_(NULL), on_error_(NULL), max_read_size_(max_read_size) {
    }

    /** Destructor. */
    ~AsyncClient() {
        close();
    }
    
    /** Get current client ID.
     \return  Client ID
    */
    ulong get_id() const {
        return id_;
    }

    /** Get current state.
     \return  Current state
    */
    State get_state() const {
        return state_;
    }

    /** Attach to a parent AsyncClient or AsyncServer and use the same event-loop as the parent.
     - This must be called _before_ any `connect*()` method, otherwise this is ignored
     .
     \param  parent  Parent to attach to
     \return         This
    */
    This& attach_to(AsyncBase& parent) {
        init_attach(parent);
        return *this;
    }

    /** Close connection.
     - This must not be called from an event handler
    */
    void close() {
        if (state_ > sNONE) {
            bufs_.reset();
            ::bufferevent_free(bev_);
            bev_   = NULL;
            state_ = sNONE;
            queue_.clear();
            ((ProtocolHandler&)*this).on_close();
            if (logger.check(LOG_LEVEL_DEBUG_LOW))
                logger.log(LOG_LEVEL_DEBUG_LOW, String().reserve(32) << "AsyncClient " << id_ << " closed");
        }
    }

    /** %Set general handler to call when a connection is established.
     \param  cb  OnConnect event to use, NULL for none
     \return     This
    */
    This& set_on_connect(OnConnect* cb) {
        on_connect_ = cb;
        return *this;
    }

    /** %Set general handler to call when an unexpected error occurs.
     \param  cb  OnError event to use, NULL for none
     \return     This
    */
    This& set_on_error(OnError* cb) {
        on_error_ = cb;
        return *this;
    }

    /** Start IP connection.
     - This is non-blocking and returns immediately while connecting
    */
    bool connect_ip(const char* host, ushort port, int family=AF_INET) {
        close();
        SocketAddressInfo address_info(family);
        Error err = address_info.convert(host, port);
        if (err == ENone) {
            assert( address_info.ptr->ai_addrlen <= (size_t)Int::MAX );
            if (connect_new(address_info.ptr->ai_addr, (int)address_info.ptr->ai_addrlen)) {
                if (logger.check(LOG_LEVEL_DEBUG)) {
                    const SubString host_str(host);
                    logger.log_direct(LOG_LEVEL_DEBUG, String().reserve(50 + host_str.size()) << "AsyncClient " << id_ << " connect_ip: '" << host_str << "' port " << port);
                }
                return true;
            }
            if (logger.check(LOG_LEVEL_ERROR))
                logger.log_direct(LOG_LEVEL_ERROR, String().reserve(44) << "AsyncClient " << id_ << " connect_new() failed");
        } else if (logger.check(LOG_LEVEL_ERROR)) {
            const SubString host_str(host);
            logger.log_direct(LOG_LEVEL_ERROR, String().reserve(58 + host_str.size()) << "AsyncClient " << id_ << " connect_ip() failed on bad host: '" << host_str << "'");
        }
        return false;
    }

protected:
    /** Request data for prequeue. */
    struct PreQueueItem {
        String buf;         ///< Request output data buffer -- only used if not writing directly to socket
        QueueItem item;     ///< Request response data for main queue -- this is added to the main queue when the output data is written to the socket

        /** Constructor. */
        PreQueueItem() {
        }

        /** Copy constructor.
         \param  src  Source to copy
        */
        PreQueueItem(const PreQueueItem& src) : buf(src.buf), item(src.item) {
        }

        /** Assignment operator.
         \param  src  Source to copy/assign
         \return      This
        */
        PreQueueItem& operator=(const PreQueueItem& src) {
            buf = src.buf;
            item = src.item;
            return *this;
        }
    };

    /** Used by the protocol implementation to write a request to an AsyncClient.
     - This builds output request data and either writes it directly to the socket or prequeues it to be written by the event handler thread
     - The request output size must be calculated in advance and passed to constructor
     - Use inherited `add()` methods to write and build up the request data
     - The destructor finishes the processes and sends and/or queues the request as appropriate
     .
    */
    struct RequestWriter : public AsyncBuffers::BulkWrite {
        typedef This Parent;    ///< Parent AsyncClient type

        Parent& parent;         ///< Parent AsyncClient for request
        PreQueueItem pq;        ///< Request data for prequeue: write buffer, main queue response data

        /** Constructor sets up request writer for parent AsyncClient.
         \param  parent    Parent AsyncClient to use
         \param  buf_size  Buffer size to use -- must be the exact size of the output data to write
        */
        RequestWriter(Parent& parent, size_t buf_size) : parent(parent) {
            init(parent.bufs_, buf_size);
        }

        /** Destructor writes and/or queues the request data (as applicable). */
        ~RequestWriter() {
            if (!pq.item.null())
                parent.queue_.add(pq.item);
        }
    };

    AtomicBufferQueue<QueueItem> queue_;    ///< Queue where each item represents an expected response from server

private:
    // Disable copy constructor
    AsyncClient(const This&) EVO_ONCPP11(= delete);

    AsyncBuffers bufs_;

    ulong id_;
    State state_;
    struct bufferevent* bev_;

    OnConnect* on_connect_;
    OnError*   on_error_;
    SizeT read_fixed_size_;
    SizeT max_read_size_;

    bool check_client_active() {
        return (!queue_.empty() || bufs_.write_size() > 0);
    }

    bool connect_new(struct sockaddr* addr, int addr_len) {
        init();
        bev_ = ::bufferevent_socket_new(evloop_->handle(), -1, BEV_OPT_CLOSE_ON_FREE);
        ::bufferevent_setcb(bev_, on_read, NULL, on_event, this);
        ::bufferevent_setwatermark(bev_, EV_READ, ProtocolHandler::MIN_INITIAL_READ, max_read_size_);
        if (read_timeout_ms_ > 0 || write_timeout_ms_ > 0) {
            struct timeval read_timeout, write_timeout;
            ::bufferevent_set_timeouts(bev_,
                get_timeout_ptr(read_timeout, read_timeout_ms_),
                get_timeout_ptr(write_timeout, write_timeout_ms_)
            );
        }

        read_fixed_size_ = 0;
        if (::bufferevent_enable(bev_, EV_READ | EV_WRITE) == 0) {
            bufs_.attach_write(bev_);
            if (::bufferevent_socket_connect(bev_, addr, addr_len) == 0) {
                state_ = sCONNECTING;
                return true;
            } else
                logger.log(LOG_LEVEL_ERROR, "AsyncClient libevent error: bufferevent_socket_connect() failed");
        } else
            logger.log(LOG_LEVEL_ERROR, "AsyncClient libevent error: bufferevent_enable() failed");
        close();
        return false;
    }

    static void on_read(struct bufferevent* bev, void* self_handler) {
        EVO_PARAM_UNUSED(bev);
        String logstr;
        ProtocolHandler& self = *(ProtocolHandler*)self_handler;
        AsyncBuffers& bufs = self.bufs_;
        bufs.attach_read();
        if (self.read_fixed_size_ > 0) {
            if (self.logger.check(LOG_LEVEL_DEBUG_LOW))
                self.logger.log_direct(LOG_LEVEL_DEBUG_LOW, logstr.set().reserve(64) << "AsyncClient " << self.id_ << " fixed read: " << self.read_fixed_size_);
            for (;;) {
                SubString data;
                if (!bufs.read_fixed(data, self.read_fixed_size_))
                    return; // wait for more data
                self.read_fixed_size_ = 0;
                if (!self.on_read_fixed(self.read_fixed_size_, data, NULL)) {
                    if (self.logger.check(LOG_LEVEL_DEBUG_LOW))
                        self.logger.log_direct(LOG_LEVEL_DEBUG_LOW, logstr.set().reserve(64) << "AsyncClient " << self.id_ << " on_read_fixed() returned false to close");
                    self.close();
                    return;
                }
                bufs.read_flush();
                if (self.read_fixed_size_ <= 0)
                    break;
            }
            bufs.read_reset(ProtocolHandler::MIN_INITIAL_READ, self.max_read_size_);
            if (bufs.read_size() == 0)
                return;
        }
        if (self.logger.check(LOG_LEVEL_DEBUG_LOW))
            self.logger.log_direct(LOG_LEVEL_DEBUG_LOW, logstr.set().reserve(64) << "AsyncClient " << self.id_ << " read: " << bufs.read_size());
        if (!self.on_read(self.read_fixed_size_, bufs, NULL)) {
            if (self.logger.check(LOG_LEVEL_DEBUG_LOW))
                self.logger.log_direct(LOG_LEVEL_DEBUG_LOW, logstr.set().reserve(64) << "AsyncClient " << self.id_ << " on_read() returned false to close");
            self.close();
            return;
        }
    }

    static void on_event(struct bufferevent* bev, short events, void* self_ptr) {
        EVO_PARAM_UNUSED(bev);
        String logstr;
        This& self = *(This*)self_ptr;
        if (events & BEV_EVENT_CONNECTED && self.state_ == sCONNECTING) {
            self.logger.log(LOG_LEVEL_DEBUG_LOW, logstr.set().reserve(34) << "AsyncClient " << self.id_ << " connected");
            self.state_ = sCONNECTED;
            ((ProtocolHandler&)self).on_connect();
            if (self.on_connect_ != NULL)
                self.on_connect_->on_connect();
        } else {
            AsyncError err;
            if (events & BEV_EVENT_EOF)
                err = aeCLOSED;
            else if (events & BEV_EVENT_TIMEOUT)
                err = aeTIMEOUT;
            else if (events & BEV_EVENT_READING)
                err = (self.state_ == sCONNECTING ? aeCONNECT : aeIO_READ);
            else if (events & BEV_EVENT_WRITING)
                err = (self.state_ == sCONNECTING ? aeCONNECT : aeIO_WRITE);
            else if (events & BEV_EVENT_CONNECTED) {
                self.logger.log(LOG_LEVEL_ERROR, logstr.set().reserve(64) << "AsyncClient " << self.id_ << " error: Unexpected 'connected' event");
                err = aeIO;
            } else
                err = aeIO;
            if (self.logger.check(LOG_LEVEL_ERROR)) {
                const SubString errmsg(async_error_msg(err));
                self.logger.log_direct(LOG_LEVEL_ERROR, logstr.set().reserve(42 + errmsg.size()) << "AsyncClient " << self.id_ << " error: " << errmsg << " (code: " << (int)err << ')');
            }

            self.close();
            ((ProtocolHandler&)self).on_error(err);
            if (self.on_error_ != NULL)
                self.on_error_->on_error(err);
        }
    }

    static ulong get_next_id() {
        static AtomicULong id;
        return ++id;
    }
};

///////////////////////////////////////////////////////////////////////////////
//@}
}
#endif
