// Evo C++ Library
/* Copyright 2019 Justin Crowell
Distributed under the BSD 2-Clause License -- see included file LICENSE.txt for details.
*/
///////////////////////////////////////////////////////////////////////////////
/** \file ioasync_base.h Evo AsyncBase. */
#pragma once
#ifndef INCL_evo_ioasync_base_h
#define INCL_evo_ioasync_base_h

#include "iosock.h"
#include "event.h"
#include "thread.h"
#include "string.h"
#include "substring.h"
#include "atomic_buffer_queue.h"
#include "logger.h"

// Requires libevent 2.0+
#if !defined(LIBEVENT_VERSION_NUMBER)
    #include <event2/event.h>
    #include <event2/buffer.h>
    #include <event2/bufferevent.h>
    #include <event2/thread.h>
#endif
#define EVO_LIBEVENT_MINVER 0x02000000
#if LIBEVENT_VERSION_NUMBER < EVO_LIBEVENT_MINVER
    #error Evo Async I/O requires libevent 2.0+
#endif
#if !defined(EVO_ASYNC_MULTI_THREAD)    
    /** Enable multi-threaded support for internal async I/O code.
     - Define as `EVO_ASYNC_MULTI_THREAD 1` to enable this
     .
    */
    #define EVO_ASYNC_MULTI_THREAD 0
#endif

#if defined(EVO_MSVC_YEAR)
    #pragma comment(lib, "libevent_core.lib")
    #pragma comment(lib, "advapi32.lib")
#endif

namespace evo {
/** \addtogroup EvoIO */
//@{

///////////////////////////////////////////////////////////////////////////////

/** Manages an event-loop for async I/O.
*/
class AsyncEventLoop {
public:
    typedef struct event_base* Handle;

    /** Constructor. */
    AsyncEventLoop() {
        static FirstInitHelper first_init_helper;
        evbase_ = ::event_base_new();
        if (evbase_ == NULL)
            abort(); // This shouldn't happen
    }

    /** Destructor. */
    ~AsyncEventLoop() {
        ::event_base_free(evbase_);
    }

    /** Get event loop handle.
     \return  Handle
    */
    Handle handle()
        { return evbase_; }

    /** Get whether event-loop is active.
     \return  Whether active
    */
    bool active() const {
        return !shutdown_.load(EVO_ATOMIC_ACQUIRE);
    }

    /** Run event loop with one pass and process I/O events.
     - This waits until some events are active then processes all active events and returns
     - Use notify1() to wake this up if it's blocking/waiting so the caller can do other processing
     - \b Caution: `run*()` methods must only be called from 1 thread at a time, otherwise results are undefined
     .
     \return  Whether successfull, false on internal error
    */
    bool run1() {
        int result = ::event_base_loop(evbase_, EVLOOP_ONCE);
        if (result < 0)
            return false;
        else if (result > 0)
            shutdown_.store(1, EVO_ATOMIC_RELEASE);
        return true;
    }

    /** Shutdown event loop. */
    void shutdown() {
        if (shutdown_.compare_set(0, 1, EVO_ATOMIC_ACQ_REL, EVO_ATOMIC_ACQUIRE)) {
            if (::event_base_loopexit(evbase_, NULL) != 0)
                abort(); // This should never happen
        }
    }

private:
    struct FirstInitHelper {
        FirstInitHelper() {
            static Mutex mutex;
            Mutex::Lock lock(mutex);
            if (::event_get_version_number() < EVO_LIBEVENT_MINVER)
                abort(); // Abort if libevent is too old
        #if EVO_ASYNC_MULTI_THREAD
            int result =
            #if defined(_WIN32)
                ::evthread_use_windows_threads();
            #else
                ::evthread_use_pthreads();
            #endif
            assert( result == 0 );
            if (result != 0)
                abort(); // Abort if can't initialize libevent for multithreaded
        #endif
        }
    };

    Handle evbase_;
    AtomicInt shutdown_;
};

///////////////////////////////////////////////////////////////////////////////

/** Holds data for async I/O buffers (used internally with AsyncServer and protocol implementations).
*/
class AsyncBuffers {
public:
    /** Use to group multiple writes for efficiency.
     - The constructor pre-allocates space: call add() or addchar() until all data is added and the pre-allocated space is full
     - Once the space is full (i.e. reserved size is reached), this automatically commits (writes) the data
     - \b Caution: Results are undefined if `add*()` calls don't reach the exact reserved size
     .
    */
    class BulkWrite {
    public:
        BulkWrite() {
            parent_ = NULL;
            ptr_ = end_ = NULL;
        }

        BulkWrite(String& str, SizeT size) {
            init(str, size);
        }

        BulkWrite(AsyncBuffers& parent, size_t size) {
            init(parent, size);
        }

        ~BulkWrite() {
            assert( ptr_ == end_ );
        }

        BulkWrite& init(String& str, SizeT size) {
            const SizeT used = str.used();
            ptr_ = str.advBuffer(used + size) + used;
            end_ = ptr_ + size;
            str.advSize(size);
            return *this;
        }

        BulkWrite& init(AsyncBuffers& parent, size_t size) {
            if (parent.output_ != NULL) {
                int result = ::evbuffer_reserve_space(parent.output_, size, &data_, 1);
                if (result != 1)
                    abort(); // This should never happen
                assert( data_.iov_base != NULL );
                assert( data_.iov_len >= size );
                data_.iov_len = size;
                ptr_ = (char*)data_.iov_base;
                end_ = ptr_ + size;
                parent_ = &parent;
            } else {
                parent_ = NULL;
                ptr_ = end_ = NULL;
                abort(); // This should never happen
            }
            return *this;
        }

        bool error() const {
            return (ptr_ == NULL);
        }

        char* ptr() {
            return ptr_;
        }

        BulkWrite& addsize(size_t size) {
            ptr_ += size;
            assert( ptr_ <= end_ );
            if (parent_ != NULL && ptr_ >= end_) {
                int result = ::evbuffer_commit_space(parent_->output_, &data_, 1);
                if (result != 0)
                    abort(); // This should never happen
                ptr_ = end_ = NULL;
            }
            return *this;
        }

        BulkWrite& add(const char* data, size_t size) {
            assert( ptr_ != NULL );
            memcpy(ptr_, data, size);
            addsize(size);
            return *this;
        }

        BulkWrite& add(char ch) {
            assert( ptr_ != NULL );
            *ptr_ = ch;
            addsize(1);
            return *this;
        }

    private:
        BulkWrite(const BulkWrite&);
        BulkWrite& operator=(const BulkWrite&);

        AsyncBuffers*         parent_;
        struct evbuffer_iovec data_;
        char*                 ptr_;
        char*                 end_;
    };

    /** Constructor (used internally). */
    AsyncBuffers() {
        bev_    = NULL;
        input_  = NULL;
        output_ = NULL;
        read_offset_ = 0;
    }

    /** Reset buffer pointers (used internally). */
    void reset() {
        bev_    = NULL;
        input_  = NULL;
        output_ = NULL;
        read_offset_ = 0;
    }

    /** Attach to active buffers (used internally).
     \param  bev  Buffers to attach to
    */
    void attach(struct bufferevent* bev) {
        bev_    = bev;
        input_  = ::bufferevent_get_input(bev);
        output_ = ::bufferevent_get_output(bev);
    }

    /** Attach to active buffers for writing (used internally).
     \param  bev  Buffers to attach to
    */
    void attach_write(struct bufferevent* bev) {
        bev_    = bev;
        output_ = ::bufferevent_get_output(bev);
    }

    /** Attach to current write buffers for reading too (used internally). */
    void attach_read() {
        input_ = ::bufferevent_get_input(bev_);
    }

    /** Get read buffer data size in bytes.
     \return  Buffered data size
    */
    size_t read_size()
        { return ::evbuffer_get_length(input_); }

    /** Read fixed size data from read buffer.
    - This references buffered data directly (no copy)
    - If not enough data available, call again on next read event -- see AsyncServer
      - If the first call fails, this will adjust internal read thresholds so the next read event has enough data
    - On success, must call read_flush() to actually consume the data (removing it from read buffer),
      then if it took 2 calls (previous read_fixed() call failed), must call read_reset() to reset read thresholds
    .
     \param  data      %Set to reference data from read buffer on success  [out]
     \param  size      Data size to read
     \param  max_size  Max read buffer size, 0 for no limit, must be at least enough for size
     \return           Whether successful, false if not enough received yet so call again on next read event
    */
    bool read_fixed(SubString& data, SizeT size, SizeT max_size=0) {
        assert( max_size == 0 || max_size >= size );
        if (::evbuffer_get_length(input_) < size) {
            ::bufferevent_setwatermark(bev_, EV_READ, size, max_size);
            return false;
        }
        read_offset_ = size;
        data.set((char*)::evbuffer_pullup(input_, size), size);
        return true;
    }

    /** Helper for reading fixed size data from read buffer from a ProtocolHandler `on_read()` event.
     - This helps properly implement a pattern where an `on_read()` event needs to read fixed size data
       - This is a bit tricky because I/O is asynchronous and we may or may not need to wait for more data, creating 2 code paths
       - The idea here is to use `parent.on_fixed_read()` to handle both code paths
       - See AsyncServer for more on this
     - If enough data has been received then this:
       - Reads the data with read_fixed(), calls `parent.on_read_fixed()` to consume it, then calls read_flush() to flush this data
       - The above is repeated in a loop as long as enough data is available, and `parent.on_read_fixed()` sets `next_size` to read another chunk of fixed data
     - If enough data is not available, this sets `fixed_size` and returns true
     - If this returns true and:
       - `fixed_size > 0`: the calling `on_read()` function should immediately return true to wait for more data
       - else: the data has been read and processed, and the calling `on_read()` function may continue reading and processing data
     - If a call to `parent.on_read_fixed()` returns false, this will immediately return false,
       and the calling `on_read()` function should also immediately return false to close the connection
     .
     \tparam  T  Parent ProtocolHandler type with `on_read()` and `on_read_fixed()` event methods (inferred from parent)

     \param  parent      Parent ProtocolHandler calling this from `on_read()` event method
     \param  fixed_size  Reference to `fixed_size` param passed to `on_read()`
     \param  size        Data size to read
     \param  max_size    Max read buffer size, 0 for no limit, must be at least enough for `size`
     \param  context     Context pointer for deferred server reply (used with servers), NULL for clients -- passed to `on_read_fixed()`
     \return             Whether successful, true and `fixed_size > 0` if need to wait for more data, otherwise true if data processed, false to close connection immediately

    \par Example

    This example reads a line with a data size, then does a fixed-size read on that data size.

    \code
    bool on_read(size_t& fixed_size, AsyncBuffers& buffers, void* context) {
        SubString line;
        while (buffers.read_line(line)) {
            uint data_size = line.numu().value();
            buffers.read_flush();

            if (data_size > 0) {
                if (!buffers.read_fixed_helper(*this, fixed_size, data_size, 0, context))
                    return false;
                if (fixed_size > 0)
                    return true; // wait for more data
            }
        }
    }
    \endcode
    */
    template<class T>
    bool read_fixed_helper(T& parent, SizeT& fixed_size, SizeT size, SizeT max_size=0, void* context=NULL) {
        for (;;) {
            SubString data;
            if (!read_fixed(data, size, max_size)) {
                fixed_size = size;
                return true; // wait for more data
            }
            fixed_size = 0;
            if (!parent.on_read_fixed(fixed_size, data, context))
                return false;
            read_flush();
            if (fixed_size <= 0)
                break;
        }
        return true;
    }

    /** Reset read buffer thresholds.
     - This sets the read buffer min/max thresholds (also known as watermarks) as conditions for calling the next read event
     - Call to prepare for next read
     - See read_fixed()
     .
     \param  max_size  Maximum read buffer size, stop reading from socket when buffer hits this size, 0 for no max
     \param  min_size  Minimum read buffer size, read event isn't called until this size is reached, 0 for any size
    */
    void read_reset(size_t max_size, size_t min_size=0) {
        ::bufferevent_setwatermark(bev_, EV_READ, min_size, max_size);
    }

    /** Read next line from read buffer.
     - This references buffered data directly (no copy)
     - On success, must call read_flush() to actually consume the data (removing it from read buffer)
     - If not enough data available, call again on next read event
     .
     \param  data  %Set to reference next line from read buffer  [out]
     \return       Whether successful, false if no newline received yet so call again on next read event
    */
    bool read_line(SubString& data) {
        size_t block_len = ::evbuffer_get_contiguous_space(input_);
        if (block_len > 0) {
            size_t tmp_len;
            for (;;) {
                char* ptr = (char*)::evbuffer_pullup(input_, block_len);
                while (read_offset_ < block_len) {
                    if (ptr[read_offset_] == '\n') {
                        assert( read_offset_ <= IntegerT<SizeT>::MAX );
                        if (read_offset_ > 0 && ptr[read_offset_-1] == '\r')
                            data.set(ptr, (SizeT)(read_offset_-1));
                        else
                            data.set(ptr, (SizeT)read_offset_);
                        ++read_offset_;
                        return true;
                    }
                    ++read_offset_;
                }
                if (block_len < (tmp_len=::evbuffer_get_length(input_))) {
                    block_len = tmp_len;
                } else
                    break;
            }
        }
        return false;
    }

    /** Flush and consume next line from read buffer.
     - Must call read_line() first to get next line
     - Only call this after read_line() returns true
    */
    void read_flush() {
        if (read_offset_ > 0) {
            if (::evbuffer_drain(input_, read_offset_) != 0)
                abort(); // This should never happen
            read_offset_ = 0;
        }
    }

    size_t write_size() const
        { return (output_ == NULL ? 0 : ::evbuffer_get_length(output_)); }

    void write_clear() {
        if (output_ != NULL)
            ::evbuffer_drain(output_, ::evbuffer_get_length(output_));
    }

    void write_reserve(size_t size)
        { ::evbuffer_expand(output_, size); }

    void write(const char* data, size_t size)
        { ::evbuffer_add(output_, data, size); }

private:
    struct bufferevent* bev_;
    struct evbuffer* input_;
    struct evbuffer* output_;
    size_t read_offset_;
};

///////////////////////////////////////////////////////////////////////////////

/** Async I/O read results used by protocol events. */
enum AsyncReadResult {
    arrERROR = 0,   ///< An error occurred
    arrDONE,        ///< Done reading request
    arrMORE         ///< More to read for request
};

/** Async I/O error type. */
enum AsyncError {
    aeNONE = 0,     ///< No error
    aeCONNECT,      ///< Connection refused (clients only)
    aeCLOSED,       ///< Socket closed by other side
    aeIO,           ///< I/O unrecoverable error
    aeIO_READ,      ///< I/O read error
    aeIO_WRITE,     ///< I/O write error
    aeTIMEOUT,      ///< I/O timeout
    aeCLIENT        ///< Client protocol error (set by protocol implementation)
};

/** Get error message for AsyncError code.
 \param  err  Error code to get message for
 \return      Error message string pointer -- always terminated, never empty or null
*/
inline const char* async_error_msg(AsyncError err) {
    if (err <= aeNONE || err > aeCLIENT)
        return "Unknown error";
    const char* MSG[] = {
        "Connection refused",
        "Socket closed by other side",
        "Unrecoverable I/O error",
        "Read error",
        "Write error",
        "Timed out",
        "Client protocol error"
    };
    return MSG[(int)err - 1];
}

///////////////////////////////////////////////////////////////////////////////

/** Base class for Async I/O.
 - This is inherited by either AsyncServer or AsyncClient
*/
class AsyncBase {
public:
    LoggerPtr<> logger; ///< Logger for protocol and debug messages, set to enable logging -- see set_logger()

    /** Timer expired event.
     - When activated, the on_timer() is called after a given amount of time elapses (i.e. when timer expires)
     - Use AsyncBase::set_timer() to activate
     .
    */
    struct OnTimer {
        struct event* timer_handle;     ///< Internal handle for event
        ulong timer_msec;               ///< Timer value in milliseconds, set by timer_reset()

        /** Constructor. */
        OnTimer() : timer_handle(NULL) {
        }

        /** Destructor. */
        virtual ~OnTimer() {
            ::event_free(timer_handle);
        }

        /** Called when timer expires.
         - When the timer expires it is deactivated
         - To reset (reactivate) the timer call timer_reset(), use timer_msec for last timer value
        */
        virtual void on_timer() = 0;

        /** Reset and activate timer so the on_timer() event is called after given time elapses.
         - On success this sets timer_msec
         .
         \param  msec  Amount of timer to trigger timer in milliseconds
         \return       Whether successful, false on internal error
        */
        bool timer_reset(ulong msec) {
            struct timeval tv;
            if (::event_add(timer_handle, get_timeout_ptr(tv, msec)) != 0) {
                ::event_free(timer_handle);
                return false;
            }
            timer_msec = msec;
            return true;
        }
    };

    /** Constructor.
     - Initialize event-loop using init() or init_attach()
    */
    AsyncBase() : parent_base_(NULL), child_base_(NULL), local_(true), evloop_(NULL), read_timeout_ms_(0), write_timeout_ms_(0) {
    }

    /** Destructor.
     - This detaches from parent, if applicable
    */
    virtual ~AsyncBase() {
        if (parent_base_ == NULL) {
            if (child_base_ != NULL) {
                logger.log(LOG_LEVEL_ERROR, "AsyncBase internal cleanup error, parent destroyed before child");
                assert( false ); // this shouldn't happen
            }
            delete evloop_;
        } else {
            parent_base_->child_base_ = child_base_;
            if (child_base_ != NULL)
                child_base_->parent_base_ = parent_base_;
        }
    }

    /** %Set logger to use.
     - If set, a logger shows low-level details and error information from the framework
     - The logger pointer must remain valid as long as it's referenced here
     .
     \param  newlogger  Pointer to logger to set, NULL for none (no logging)
    */
    virtual void set_logger(LoggerBase* newlogger) {
        logger.ptr = newlogger;
    }

    /** %Set read/write timeouts to use.
     \param  read_timeout_ms   Socket read timeout in milliseconds, 0 for none (never timeout)
     \param  write_timeout_ms  Socket write timeout in milliseconds, 0 for none (never timeout)
    */
    void set_timeout(ulong read_timeout_ms=0, ulong write_timeout_ms=0) {
        read_timeout_ms_  = read_timeout_ms;
        write_timeout_ms_ = write_timeout_ms;
    }

    /** Activate timer so it expires after given time elapses.
     - When the timer expires the OnTimer::on_timer() event is called and the timer is deactivated
     - To reactivate the timer so it expires again the event should call OnTimer::timer_reset()
     .
     \param  on_timer  %Timer to use
     \param  msec      Expiration time in milliseconds
     \return           Whether successful, false on internal error
    */
    bool set_timer(OnTimer& on_timer, ulong msec) {
        if (on_timer.timer_handle != NULL)
            ::event_free(on_timer.timer_handle);
        on_timer.timer_handle = ::event_new(evloop_->handle(), -1, 0, on_timer_event, &on_timer);
        if (on_timer.timer_handle == NULL)
            return false;
        return on_timer.timer_reset(msec);
    }

    /** Run the event-loop locally in current thread until all pending requests are handled (client only).
     - This blocks while client requests are pending
     - This returns false immediately if this does not own an event-loop (i.e. was attached to a parent) -- only the top parent can run an event-loop
     .
     \return  Whether successful, false if attached to another AsyncBase or on internal error
    */
    bool runlocal() {
        if (parent_base_ != NULL || evloop_ == NULL)
            return false;
        if (evloop_->active() && !run_eventloop_once())
            return false;
        for (AsyncBase* p = this; p != NULL && evloop_->active(); p = p->child_base_)
            while (p->check_client_active())
                if (!run_eventloop_once())
                    return false;
        return true;
    }

protected:
    AsyncBase* parent_base_;    ///< Pointer to parent in AsyncBase chain, NULL if this is the main parent (and owns evloop_ pointer)
    AsyncBase* child_base_;     ///< Pointer to child in AsyncBase chain (always an AsyncClient), NULL for none
    bool local_;                ///< Whether event-loop is local (same thread), false if separate thread

    AsyncEventLoop* evloop_;    ///< Event loop pointer, either owned by this or a parent
    ulong read_timeout_ms_;     ///< Socket read timeout in milliseconds, 0 for none (never timeout)
    ulong write_timeout_ms_;    ///< Socket write timeout in milliseconds, 0 for none (never timeout)

    /** Initialize event-loop.
     - Ignored if already initialized
     .
    */
    void init() {
        if (evloop_ == NULL)
            evloop_ = new AsyncEventLoop;

    }

    /** Initialize and attach to a parent event-loop.
     - Ignored if already initialized
     .
     \param  parent  Parent AsyncBase to attach to
    */
    void init_attach(AsyncBase& parent) {
        if (evloop_ == NULL && parent_base_ == NULL) {
            // Always attach at end of chain
            AsyncBase* p = &parent;
            while (p->child_base_ != NULL) {
                p = p->child_base_;
                assert( p->parent_base_ != NULL );
            }

            if (p->evloop_ == NULL) {
                // Init parent since this hasn't been done yet
                assert( p->parent_base_ == NULL );
                p->init();
            }

            parent_base_ = p;
            p->child_base_ = this;
            evloop_ = p->evloop_;
        }
    }

    /** Called during client event-loop to check whether any client requests are active (client only).
     - This is used to determine if all client requests were handled
     - Only AsyncClient should override this, AsyncServer should leave the default
     .
     \return  Whether any client requests are active, true if a request is in progress (waiting for response or write is pending)
    */
    virtual bool check_client_active() {
        return false;
    }

    /** Run event loop with one pass and process all events.
     - This waits until some events are active then processes all active events and returns
     .
     \return  Whether successfull, false on internal error
    */
    bool run_eventloop_once() {
        if (parent_base_ != NULL || !evloop_->run1())
            return false;
        return true;
    }

    /** Run event loop and process all events and repeat until shutdown.
     \return  Whether successfull, false on internal error
    */
    bool run_eventloop() {
        while (evloop_->active()) {
            if (!run_eventloop_once())
                return false;
        }
        return true;
    }

    /** Get timeval struct pointer from timeout in milliseconds.
     \param  out  Struct to store timeout
     \param  ms   Time in milliseconds to use
     \return      Pointer to `out`, or NULL if `ms` is 0
    */
    static struct timeval* get_timeout_ptr(struct timeval& out, ulong ms) {
        if (ms > 0) {
            #if defined(_WIN32)
                SysWindows::set_timeval_ms(out, ms);
            #else
                SysLinux::set_timeval_ms(out, ms);
            #endif
            return &out;
        }
        return NULL;
    }

private:
    // Disable copying
    AsyncBase(const AsyncBase&);
    AsyncBase& operator=(const AsyncBase&);

    static void on_timer_event(evutil_socket_t, short, void * arg) {
        ((OnTimer*)arg)->on_timer();
    }
};

///////////////////////////////////////////////////////////////////////////////
//@}
}
#endif
