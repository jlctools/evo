// Evo C++ Library
/* Copyright 2019 Justin Crowell
Distributed under the BSD 2-Clause License -- see included file LICENSE.txt for details.
*/
///////////////////////////////////////////////////////////////////////////////
/** \file logger.h Evo logging. */
#pragma once
#ifndef INCL_evo_logger_h
#define INCL_evo_logger_h

#include "io.h"
#include "file.h"
#include "substring.h"
#include "atomic_buffer_queue.h"
#include "thread.h"
#include "time.h"
#include "enum.h"

///////////////////////////////////////////////////////////////////////////////

/** Helper macro for logging an alert with Logger.
 - This calls `LOGGER.check()` to check if a message at this level matches the current log level, and if so then calls `LOGGER.log_direct()` to log the message
 - Formatting can be done inline with `MSG`, for example:
   \code
    String msg;
    ...
    EVO_LOG_ALERT(logger, 0, msg.set() << "Hello " << 123)
   \endcode
 - `MSG` is only evaluated if it will be logged, which saves from allocating and formatting messages that won't be logged
 - See \link evo::Logger Logger\endlink and \link evo::LOG_LEVEL_ALERT LOG_LEVEL_ALERT\endlink
 .
 \param  LOGGER  The logger to use
 \param  MSG     Message string to log, passed as `const SubString&`
*/
#define EVO_LOG_ALERT(LOGGER, MSG) { if (LOGGER.check(LOG_LEVEL_ALERT)) LOGGER.log_direct(LOG_LEVEL_ALERT, MSG); }

/** Helper macro for logging an error with Logger.
 - This calls `LOGGER.check()` to check if a message at this level matches the current log level, and if so then calls `LOGGER.log_direct()` to log the message
 - Formatting can be done inline with `MSG`, for example:
   \code
    String msg;
    ...
    EVO_LOG_ERROR(logger, 0, msg.set() << "Hello " << 123)
   \endcode
 - `MSG` is only evaluated if it will be logged, which saves from allocating and formatting messages that won't be logged
 - See \link evo::Logger Logger\endlink and \link evo::LOG_LEVEL_ERROR LOG_LEVEL_ERROR\endlink
 .
 \param  LOGGER  The logger to use
 \param  MSG     Message string to log, passed as `const SubString&`
*/
#define EVO_LOG_ERROR(LOGGER, MSG) { if (LOGGER.check(LOG_LEVEL_ERROR)) LOGGER.log_direct(LOG_LEVEL_ERROR, MSG); }

/** Helper macro for logging a warning with Logger.
 - This calls `LOGGER.check()` to check if a message at this level matches the current log level, and if so then calls `LOGGER.log_direct()` to log the message
 - Formatting can be done inline with `MSG`, for example:
   \code
    String msg;
    ...
    EVO_LOG_WARN(logger, 0, msg.set() << "Hello " << 123)
   \endcode
 - `MSG` is only evaluated if it will be logged, which saves from allocating and formatting messages that won't be logged
 - See \link evo::Logger Logger\endlink and \link evo::LOG_LEVEL_WARN LOG_LEVEL_WARN\endlink
 .
 \param  LOGGER  The logger to use
 \param  MSG     Message string to log, passed as `const SubString&`
*/
#define EVO_LOG_WARN(LOGGER, MSG) { if (LOGGER.check(LOG_LEVEL_WARN)) LOGGER.log_direct(LOG_LEVEL_WARN, MSG); }

/** Helper macro for logging an informational message with Logger.
 - This calls `LOGGER.check()` to check if a message at this level matches the current log level, and if so then calls `LOGGER.log_direct()` to log the message
 - Formatting can be done inline with `MSG`, for example:
   \code
    String msg;
    ...
    EVO_LOG_INFO(logger, 0, msg.set() << "Hello " << 123)
   \endcode
 - `MSG` is only evaluated if it will be logged, which saves from allocating and formatting messages that won't be logged
 - See \link evo::Logger Logger\endlink and \link evo::LOG_LEVEL_INFO LOG_LEVEL_INFO\endlink
 .
 \param  LOGGER  The logger to use
 \param  MSG     Message string to log, passed as `const SubString&`
*/
#define EVO_LOG_INFO(LOGGER, MSG) { if (LOGGER.check(LOG_LEVEL_INFO)) LOGGER.log_direct(LOG_LEVEL_INFO, MSG); }

/** Helper macro for logging a debug message with Logger.
 - This calls `LOGGER.check()` to check if a message at this level matches the current log level, and if so then calls `LOGGER.log_direct()` to log the message
 - Formatting can be done inline with `MSG`, for example:
   \code
    String msg;
    ...
    EVO_LOG_DEBUG(logger, 0, msg.set() << "Hello " << 123)
   \endcode
 - `MSG` is only evaluated if it will be logged, which saves from allocating and formatting messages that won't be logged
 - See \link evo::Logger Logger\endlink and \link evo::LOG_LEVEL_DEBUG LOG_LEVEL_DEBUG\endlink
 .
 \param  LOGGER  The logger to use
 \param  MSG     Message string to log, passed as `const SubString&`
*/
#define EVO_LOG_DEBUG(LOGGER, MSG) { if (LOGGER.check(LOG_LEVEL_DEBUG)) LOGGER.log_direct(LOG_LEVEL_DEBUG, MSG); }

/** Helper macro for logging a low-level debug message with Logger.
 - This calls `LOGGER.check()` to check if a message at this level matches the current log level, and if so then calls `LOGGER.log_direct()` to log the message
 - Formatting can be done inline with `MSG`, for example:
   \code
    String msg;
    ...
    EVO_LOG_DEBUG_LOW(logger, 0, msg.set() << "Hello " << 123)
   \endcode
 - `MSG` is only evaluated if it will be logged, which saves from allocating and formatting messages that won't be logged
 - See \link evo::Logger Logger\endlink and \link evo::LOG_LEVEL_DEBUG_LOW LOG_LEVEL_DEBUG_LOW\endlink
 .
 \param  LOGGER  The logger to use
 \param  MSG     Message string to log, passed as `const SubString&`
*/
#define EVO_LOG_DEBUG_LOW(LOGGER, MSG) { if (LOGGER.check(LOG_LEVEL_DEBUG_LOW)) LOGGER.log_direct(LOG_LEVEL_DEBUG_LOW, MSG); }

///////////////////////////////////////////////////////////////////////////////

namespace evo {
/** \addtogroup EvoTools */
//@{

/** Logger exception, see Exception. */
class ExceptionLogger : public Exception
    { EVO_CREATE_EXCEPTION_IMPL(ExceptionLogger, Exception) };

///////////////////////////////////////////////////////////////////////////////

/** Log severity level used with Logger. */
enum LogLevel {
    LOG_LEVEL_DISABLED = 0, ///< Logging disabled
    LOG_LEVEL_ALERT,        ///< Alert message for critical alert that needs immediate attention, program may be unstable (`ALRT`)
    LOG_LEVEL_ERROR,        ///< Error message showing something isn't working as expected, program may be able to work around it (`ERRR`)
    LOG_LEVEL_WARN,         ///< Warning message that can indicate a potential issue, this may lead to an error or alert (`WARN`)
    LOG_LEVEL_INFO,         ///< Informational message for showing notices and context (`INFO`)
    LOG_LEVEL_DEBUG,        ///< High-level debug message, used for showing debug info for higher-level behavior (`DBUG`)
    LOG_LEVEL_DEBUG_LOW     ///< Low-level debug message, used for showing debug info for lower-level internal or library details (`DBGL`)
};

/** Log level remapping for EVO_ENUM_REMAP(). */
static const LogLevel LOG_LEVEL_REMAP[] = {
    LOG_LEVEL_ALERT,
    LOG_LEVEL_DEBUG,
    LOG_LEVEL_DEBUG_LOW,
    LOG_LEVEL_ERROR,
    LOG_LEVEL_INFO,
    LOG_LEVEL_WARN
};

/** \ref LogLevel enum helper created with EVO_ENUM_REMAP(). */
EVO_ENUM_REMAP(LogLevel, LOG_LEVEL_ALERT, LOG_LEVEL_DEBUG_LOW, LOG_LEVEL_DISABLED, LOG_LEVEL_REMAP,
    "alert",
    "debug",
    "debug_low",
    "error",
    "info",
    "warn"
);

///////////////////////////////////////////////////////////////////////////////

/** \cond impl */
namespace impl {
    template<uint SZ>
    struct LoggerMsg {
        typedef LoggerMsg<SZ> This;

        static const uint BUF_SIZE = SZ;

        SysNativeTimeStamp timestamp;
        LogLevel level;
        uint32 size;
        uint32 aux_size;
        char* aux_buf;
        char buf[BUF_SIZE];

        LoggerMsg() : size(0), aux_size(0), aux_buf(NULL) {
        }

        ~LoggerMsg() {
            if (aux_buf != NULL)
                ::free(aux_buf);
        }

        void set(LogLevel msglevel, const SubString& msg) {
            assert( msg.size() <= UInt32::MAX );

            timestamp.set_utc();
            level = msglevel;

            const char* p = msg.data();
            uint32 remain_size = msg.size();
            if (remain_size > BUF_SIZE) {
                ::memcpy(buf, p, BUF_SIZE);
                p += BUF_SIZE;
                remain_size -= BUF_SIZE;

                if (aux_buf != NULL) {
                    if (remain_size > aux_size)
                        ::free(aux_buf); // old buffer too small
                    else
                        goto copy; // old buffer ok
                }

                // New aux_buf
                aux_size = remain_size;
                aux_buf = (char*)::malloc(aux_size);

            copy:
                // Copy remaining part to aux_buf
                ::memcpy(aux_buf, p, remain_size);
                size = msg.size();
            } else {
                if (aux_buf != NULL) {
                    ::free(aux_buf);
                    aux_buf  = NULL;
                    aux_size = 0;
                }
                ::memcpy(buf, p, remain_size);
                size = remain_size;
            }
        }

        void get_buf_sizes(uint32& buf1_size, uint32& buf2_size) {
            if (size <= BUF_SIZE) {
                buf1_size = size;
                buf2_size = 0;
            } else {
                buf1_size = BUF_SIZE;
                buf2_size = size - BUF_SIZE;
            }
        }

        // aux_buf is swapped, buf is copied from src (not swapped), src.size is set to 0
        This& operator=(const This& src) {
            This& src_mutable = const_cast<This&>(src); // assign operator arg must be const, overridding for performance

            // Copy timestamp and level
            timestamp = src.timestamp;
            level = src.level;

            // Copy main buffer up to size from src
            size = src.size;
            if (src.size > 0) {
                ::memcpy(buf, src.buf, size);
                src_mutable.size = 0;
            }

            // Swap aux buffers
            swap(aux_buf, src_mutable.aux_buf);
            swap(aux_size, src_mutable.aux_size);
            return *this;
        }
    };
}
/** \endcond */

///////////////////////////////////////////////////////////////////////////////

/** Base class for Logger.
 - This gives a common logging interface
 - Call set_level() to set logging level -- messages less severe than this level are ignored (not logged)
 - Call log() or log_direct() to log messages from any thread, or use the helper macros:
   - EVO_LOG_ALERT()
   - EVO_LOG_ERROR()
   - EVO_LOG_WARN()
   - EVO_LOG_INFO()
   - EVO_LOG_DEBUG()
   - EVO_LOG_DEBUG_LOW()
 - See: Logger, LoggerConsole
 .
*/
class LoggerBase {
public:
    /** Destructor. */
    virtual ~LoggerBase() {
    }

    /** Get last error that occurred.
     - This resets the error message so calling again returns false, unless another error occurred
     .
     \param  msg  Stores last error message, null if none  [out]
     \return      Whether an error occurred, true if error message stored in `msg`
    */
    virtual bool get_error(String& msg) {
        EVO_PARAM_UNUSED(msg);
        return false;
    }

    /** %Set current log level.
     - Log messages with a less severe level than this are ignored (not logged)
     .
     \param  level  Log level to use -- see LogLevel
    */
    virtual void set_level(LogLevel level) {
        level_.store(level);
    }

    /** %Set log rotation flag.
     - This tells the logging thread to close and re-open the log file between messages
     - Linux: Usually called from SIGHUP handler triggered by something like the standard `logrotate` tool
     .
    */
    virtual void rotate() {
    }

    /** Log a message with given log level directly without checking the current log level.
     - This assumes check() was called first to check whether message will be logged
     - Consider using helper macros:
       - EVO_LOG_ALERT()
       - EVO_LOG_ERROR()
       - EVO_LOG_WARN()
       - EVO_LOG_INFO()
       - EVO_LOG_DEBUG()
       - EVO_LOG_DEBUG_LOW()
     .
     \param  level  Message severity level to use
     \param  msg    Message to log
    */
    virtual void log_direct(LogLevel level, const SubString& msg) = 0;

    /** Check whether a message with given level will actually be logged.
     - This is used before calling log_direct() with the message
     - Consider using helper macros:
       - EVO_LOG_ALERT()
       - EVO_LOG_ERROR()
       - EVO_LOG_WARN()
       - EVO_LOG_INFO()
       - EVO_LOG_DEBUG()
       - EVO_LOG_DEBUG_LOW()
     .
     \param  level  Message severity level to check
     \return        Whether given severity level passes current level check, false if a message at this level will be ignored
    */
    bool check(LogLevel level) const {
        return (level <= level_.load());
    }

    /** Log a message with given severity level.
     - This calls check() to check whether this message will be logged
     - For best performance use check() and log_direct() instead to avoid formatting messages that will not be logged
     - Consider using helper macros:
       - EVO_LOG_ALERT()
       - EVO_LOG_ERROR()
       - EVO_LOG_WARN()
       - EVO_LOG_INFO()
       - EVO_LOG_DEBUG()
       - EVO_LOG_DEBUG_LOW()
     .
     \param  level  Message severity level to use
     \param  msg    Message to log
     \return        Whether message was logged, false if message will be ignored due to current log level
    */
    bool log(LogLevel level, const SubString& msg) {
        if (level <= level_.load()) {
            log_direct(level, msg);
            return true;
        }
        return false;
    }

protected:
    AtomicInt level_;   ///< Log level, messages less severe than this are ignored (not logged)
};

///////////////////////////////////////////////////////////////////////////////

/** Wraps a logger pointer that can reference a logger to use or be disabled.
 - This is useful for making logging optional in certain classes or functions -- leaving or setting the pointer as `NULL` disables logging
 - This provides the same logging methods as LoggerBase: check(), log_direct() and log()
 - \b Caution: This is not thread safe
 - See: Logger, LoggerConsole
 .
 \tparam  T  %Logger to use -- use LoggerBase (the default) for any logger
*/
template<class T=LoggerBase>
struct LoggerPtr {
    typedef T LoggerType;   ///< Logger type used from template

    LoggerType* ptr;    ///< %Logger pointer, NULL to disable logging with this

    /** Constructor sets as null. */
    LoggerPtr() : ptr(NULL) {
    }

    /** Constructor to set pointer. */
    LoggerPtr(LoggerType* newptr) : ptr(newptr) {
    }

    /** Copy constructor copies logger pointer.
     \param  src  Source to copy
    */
    LoggerPtr(const LoggerPtr& src) : ptr(src.ptr) {
    }

    /** Assignment operator copies logger pointer.
     \param  src  Source to copy
     \return      This
    */
    LoggerPtr& operator=(const LoggerPtr& src) {
        ptr = src.ptr;
        return *this;
    }

    /** %Set as null. */
    void set() {
        ptr = NULL;
    }

    /** %Set to new pointer.
     \param  newptr  New pointer to set
    */
    void set(LoggerType* newptr) {
        ptr = newptr;
    }

    /** \copydoc LoggerBase::check() */
    bool check(LogLevel level) const {
        return (ptr != NULL && ptr->check(level));
    }

    /** \copydoc LoggerBase::log_direct() */
    void log_direct(LogLevel level, const SubString& msg) {
        ptr->log_direct(level, msg);
    }

    /** \copydoc LoggerBase::log() */
    bool log(LogLevel level, const SubString& msg) {
        if (ptr != NULL && ptr->check(level)) {
            ptr->log_direct(level, msg);
            return true;
        }
        return false;
    }
};

///////////////////////////////////////////////////////////////////////////////

/** High performance message logger.
 - Efficient lock-free and thread-safe logging
 - This uses a background thread that pulls messages from the logger queue and writes to the log file
 - Call set_level() to set logging level -- messages less severe than this level are ignored (not logged)
 - Call start() to open/create the log file and start the background thread
 - Call log() or log_direct() to log messages from any thread, or use the helper macros:
   - EVO_LOG_ALERT()
   - EVO_LOG_ERROR()
   - EVO_LOG_WARN()
   - EVO_LOG_INFO()
   - EVO_LOG_DEBUG()
   - EVO_LOG_DEBUG_LOW()
 - See: LoggerConsole, LoggerBase
 .

Notes:
 - Default logging level: `LOG_LEVEL_WARN`
 - Each message follows this form: `[DATE:TIME LEVEL] MESSAGE`
   - `DATE` is the message date in the form: `YYYY-MM-DD`
   - `TIME` is the message time in the form: `HH:MM:SS`
   - `LEVEL` is the 4 character message severity, one of: `ALRT, ERRR, WARN, INFO, DBUG`
   - `MESSAGE` is the message being logged, which may contain newlines
 - Messages should _not_ end with a newline
 - Each newline in messages _should_ be followed by a space or tab to distinguish multi-line messages
   - Message newlines are written as-is, i.e. are _not_ converted for current platform
 - An empty line is written each time a log file is opened (or re-opened)
 .

\tparam  MSG_BUF_SIZE  Preallocated size to use for each message -- messages within this size don't allocate memory when queued
                        - Use \link MESSAGE_BUFFER_SIZE\endlink or call get_message_buffer_size() to get this size
                        - See constructor for setting queue size

\par Example

This example starts a logger and logs some messages -- the `INFO` level message is not logged (and not wastefully formatted) since the log level here is `WARN`.

\code
#include <evo/logger.h>
using namespace evo;

int main() {
    Logger<> logger;
    logger.set_local_time(true);

    try {
        logger.start("out.log");
    } EVO_CATCH(return 1) // shows exception message if logger can't open file or start thread

    String msg;
    msg.reserve(logger.get_message_buffer_size()); // preallocate message buffer to minimize memory allocations

    EVO_LOG_ERROR(logger, 0, msg.set() << "This is an error -- test " << 123);
    EVO_LOG_WARN(logger, 0, msg.set() << "This is a warning -- test " << 123);
    EVO_LOG_INFO(logger, 0, msg.set() << "This is an info message -- test " << 123);

    return 0; // Logger destructor will flush queued messages and shutdown the background thread
}
\endcode

The above program appends the following to `out.log`:

\code{.unparsed}

[2000-01-01:00:00:00 ERRR] This is an error -- test 123
[2000-01-01:00:00:00 WARN] This is a warning -- test 123
\endcode

Note that the above example pre-allocates a string buffer for formatting messages so no memory is allocated at all when formatting and logging messages within that size.
*/
template<uint MSG_BUF_SIZE=512>
class Logger : public LoggerBase {
public:
    typedef Logger<MSG_BUF_SIZE> This;      ///< This Logger type

    static const uint MESSAGE_BUFFER_SIZE = MSG_BUF_SIZE;   ///< Preallocated buffer size per message -- messages within this size don't allocate memory when queued
    static const SizeT DEFAULT_QUEUE_SIZE = 256;            ///< Default queue size, override with constructor

    /** Constructor.
     \param  queue_size  Queue size to use
    */
    Logger(SizeT queue_size=DEFAULT_QUEUE_SIZE) : queue_(queue_size), outfile_(NL_SYS, false), thread_(consumer, this), local_time_(false) {
        level_.store(LOG_LEVEL_WARN);
    }

    /** Destructor, calls shutdown(). */
    ~Logger() {
        shutdown();
    }

    /** Get message buffer size.
     - Messages within this size don't allocate memory when queued
     - This is useful for pre-allocating a string buffer to minimize memory allocations when logging
     .
     \return  Message buffer size -- same as MESSAGE_BUFFER_SIZE
    */
    uint get_message_buffer_size() const {
        return MESSAGE_BUFFER_SIZE;
    }

    /** %Set wheter to convert log date/time values to local time.
     - This conversion adds some small overhead to log writing
     .
    */
    void set_local_time(bool local_time) {
        local_time_ = local_time;
    }

    bool get_error(String& msg) {
        Condition::Lock lock(condmutex_);
        if (errmsg_.null())
            return false;
        msg = errmsg_;
        errmsg_.set();
        return true;
    }

    void rotate() {
        rotate_.store(1);
    }

    void log_direct(LogLevel level, const SubString& msg) {
        Msg qitem;
        qitem.set(level, msg);
        queue_.add(qitem);
        if (condmutex_.trylock()) { // non-blocking
            condmutex_.notify();
            condmutex_.unlock();
        }
    }
    
    /** Open log file but don't start logging thread yet.
     - This is useful when about to `fork()` or daemonize(), but want to open the log first to make sure it works
     - Once opened, call start_thread() to start the logging thread
     .
     \param  path   Log file path to use, can be absolute or relative to current directory
     \param  excep  Whether to throw an exception on error, true to always return success else throw exception on error
     \return        Whether successful, false on error opening log file or thread already active (when `excep=false`)
    */
    bool open(const SubString& path, bool excep=EVO_EXCEPTIONS) {
        if (thread_.thread_active()) {
            const SubString MSG("Logger can't open a file while thread already active");
            Condition::Lock lock(condmutex_);
            errmsg_.set().reserve(MSG.size() + filepath_.size()) << MSG << filepath_;
            return false;
        } else {
            // No mutex locks needed while thread is inactive
            filepath_ = path;
            if (filepath_.empty()) {
                errmsg_ = "Logger can't open empty file path";
                EVO_THROW_ERR_CHECK(evo::ExceptionLogger, errmsg_, EInval, excep);
                return false;
            }
        #if defined(_WIN32)
            if (filepath_.ends('/') || filepath_.ends('\\') || filepath_.ends(':')) {
        #else
            if (filepath_.ends('/')) {
        #endif
                const SubString MSG("Logger can't open invalid file path, must be a file not a directory: ");
                errmsg_.set().reserve(MSG.size() + filepath_.size()) << MSG << filepath_;
                EVO_THROW_ERR_CHECK(evo::ExceptionLogger, errmsg_, EInval, excep);
                return false;
            }
            if (!outfile_.open(filepath_.cstr(), oAPPEND)) {
                const Error err = outfile_.error();
                errmsg_.set() << "Logger can't open: " << filepath_;
                EVO_THROW_ERR_CHECK(evo::ExceptionLogger, errmsg_, err, excep);
                errormsg_out(errmsg_ << " -- ", err);
                return false;
            }
            outfile_ << NL;
        }
        errmsg_.set();
        return true;
    }

    /** Start logging thread for already open file, which consumes the queue and actually writes to file.
     - open() must be called first to open the log file
     - Once started, an error in the logging thread will set an error message retrievable with get_error()
     - On success, and if thread wasn't active, this resets the last error state
     - \b Caution: The logging thread will not survive a `fork()` or daemonize() -- results are undefined in this case if thread is running (Linux/Unix only)
     .
     \param  excep  Whether to throw an exception on error, true to always return success or throw exception on error
     \return        Whether successful, true if thread started or already running, false on either error opening log file or failed to start thread (when `excep=false`)
    */
    bool start_thread(bool excep=EVO_EXCEPTIONS) {
        if (!thread_.thread_active()) {
            // No mutex locks needed while thread is inactive
            errmsg_.set();
            if (!outfile_.isopen()) {
                const SubString MSG("Logger file not open, must open() first before start_thread()");
                errmsg_.set().reserve(MSG.size() + filepath_.size()) << MSG << filepath_;
                EVO_THROW_ERR_CHECK(evo::ExceptionLogger, errmsg_, EInvalOp, excep);
                return false;
            } else if (!thread_.thread_start()) {
                errmsg_ = "Logger thread failed to start";
                EVO_THROW_ERR_CHECK(evo::ExceptionLogger, errmsg_, EFail, excep);
                return false;
            }
        }
        return true;
    }

    /** Open log file and start logging thread, which consumes the queue and actually writes to file.
     - This opens the log file and starts the thread that writes queued messages to it
     - Once started, if an error occurs in the logging thread get the error message with get_error()
     - On success this resets the last error state
     - \b Caution: The logging thread will not survive a `fork()` or daemonize() -- results are undefined in this case if thread is running (Linux/Unix only)
       - To work around this either call this after `fork()`, or instead call open() before `fork()` and start_thread() afterwards
     .
     \param  path   Log file path to use, can be absolute or relative to current directory
     \param  excep  Whether to throw an exception on error, true to always return success or throw exception on error
     \return        Whether successful, true if thread started or already running, false on either error opening log file or failed to start thread (when `excep=false`)
    */
    bool start(const SubString& path, bool excep=EVO_EXCEPTIONS) {
        return (open(path, excep) && start_thread(excep));
    }

    /** Shutdown logging thread. */
    void shutdown() {
        if (thread_.thread_active()) {
            shutdown_.store(1);
            condmutex_.lock_notify();
            thread_.thread_join();
        }
    }

private:
    typedef impl::LoggerMsg<MSG_BUF_SIZE> Msg;

    AtomicBufferQueue<Msg> queue_;

    String errmsg_;
    String filepath_;
    File   outfile_;
    Thread thread_;
    Condition condmutex_;
    AtomicInt shutdown_;
    AtomicInt rotate_;
    bool local_time_;

    static void consumer(void* arg) {
        const char BEGIN_DELIM = '[';
        const SubString MSG_DELIM("] ", 2);
        const char* LEVEL_STR[] = { "ALRT", "ERRR", "WARN", "INFO", "dbug", "dbgl" };
        const StrSizeT LEVEL_LEN = 4;

        const ulong WAKE_TIMEOUT_MS = 500;
        Logger& logger = *(Logger*)arg;
        bool closed = false;
        ulong drop_count = 0;

        DateTime dt;
        Msg msg;
        uint32 buf1_size, buf2_size;
        for (;;) {
            // Consume all messages in queue
            while (logger.queue_.pop(msg)) {
                if (closed) {
                    ++drop_count;
                } else {
                    if (logger.local_time_)
                        msg.timestamp.convert_local_dt_notz(dt);
                    else
                        msg.timestamp.convert_utc_dt(dt);
                    dt.tz.set();

                    logger.outfile_ << BEGIN_DELIM;
                    dt.format(logger.outfile_, ':') << ' ';
                    if (msg.level > LOG_LEVEL_DISABLED && msg.level <= LOG_LEVEL_DEBUG_LOW)
                        logger.outfile_ << SubString(LEVEL_STR[(int)msg.level - 1], LEVEL_LEN);
                    else
                        logger.outfile_ << FmtInt(msg.level).width(LEVEL_LEN, ' ');

                    logger.outfile_ << MSG_DELIM;

                    bool error = false;
                    msg.get_buf_sizes(buf1_size, buf2_size);
                    if (buf1_size > 0 && logger.outfile_.writebin(msg.buf, buf1_size) < buf1_size)
                        error = true;
                    else if (buf2_size > 0 && logger.outfile_.writebin(msg.aux_buf, buf2_size) < buf2_size)
                        error = true;
                    else
                        logger.outfile_ << NL;
                    if (error || !logger.outfile_) {
                        // Unable to write to file
                        ++drop_count;
                        logger.outfile_.close();
                        closed = true;

                        Condition::Lock lock(logger.condmutex_);
                        logger.errmsg_.set() << "Logger file write error: " << logger.filepath_ << " -- ";
                        errormsg_out(logger.errmsg_, logger.outfile_.error());
                        break;
                    }
                }
            }
            if (logger.shutdown_.load())
                break;

            if (closed || logger.rotate_.load()) {
                // Log re-open (rotation or log closed from error)
                {
                    Condition::Lock lock(logger.condmutex_);
                    logger.outfile_.close();
                    if (logger.outfile_.open(logger.filepath_.cstr(), oAPPEND)) {
                        logger.outfile_ << NL;
                        if (closed)
                            logger.outfile_ << "[] Logger recovered from error (lost: " << drop_count << "): " << logger.errmsg_ << NL;
                        logger.errmsg_.set();
                        closed = false;
                        drop_count = 0;
                    } else if (!closed) {
                        logger.errmsg_.set() << "Logger can't open: " << logger.filepath_ << " -- ";
                        errormsg_out(logger.errmsg_, logger.outfile_.error());
                        closed = true;
                    }
                }
                logger.rotate_.store(0);
            }

            // Wait for more messages -- logging is lock-free (non-blocking) so this must wake up regularly in case a message signal was missed
            logger.condmutex_.wait(WAKE_TIMEOUT_MS, false);
            logger.condmutex_.unlock();
        }
        logger.shutdown_.store(0);
    }
};

///////////////////////////////////////////////////////////////////////////////

/** Inert logger implementing the same interface as Logger.
 - This is useful for passing a Logger type via template parameter that doesn't log anything
 - Everything here is a no-op: start() always returns true, and check() and log() always return false
 - See Logger
 .
*/
class LoggerInert {
public:
    LoggerInert(SizeT queue_size=0) {
        EVO_PARAM_UNUSED(queue_size);
    }

    bool get_error(String&)
        { return false; }

    uint get_message_buffer_size() const
        { return 0; }

    void set_level(LogLevel) { }

    void set_local_time(bool) { }

    void rotate() { }

    bool check(LogLevel) const
        { return false; }

    void log_direct(LogLevel, const SubString&) { }

    bool log(LogLevel, const SubString&)
        { return false; }

    bool start(const SubString&, bool excep=EVO_EXCEPTIONS) {
        EVO_PARAM_UNUSED(excep);
        return true;
    }

    void shutdown() { }
};

///////////////////////////////////////////////////////////////////////////////
//@}
}
#endif
