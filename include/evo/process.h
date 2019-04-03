// Evo C++ Library
/* Copyright 2019 Justin Crowell
Distributed under the BSD 2-Clause License -- see included file LICENSE.txt for details.
*/
///////////////////////////////////////////////////////////////////////////////
/** \file process.h Evo process helpers. */
#pragma once
#ifndef INCL_evo_process_h
#define INCL_evo_process_h

#include "impl/sys.h"
#include "maplist.h"
#if defined(_WIN32)
    #include "thread.h"
#else
    #include <sys/types.h>
    #include <sys/stat.h>
    #include <fcntl.h>
    #include <signal.h>
    #include <syslog.h>
#endif

namespace evo {
/** \addtogroup EvoTools */
//@{

///////////////////////////////////////////////////////////////////////////////

#if defined(_WIN32)
    typedef DWORD ProcessId;
#else
    typedef pid_t ProcessId;    ///< Process ID type
#endif

/** Get current process ID.
 - \#include <evo/process.h>
 .
 \return  Current process ID
*/
inline ProcessId get_pid() {
    #if defined(_WIN32)
        return ::GetCurrentProcessId();
    #else
        return ::getpid();
    #endif
}

#if !defined(_WIN32)
static const uint DAEMONIZE_NO_CD      = 0x01;  ///< Flag to skip changing current directory -- see daemonize()
static const uint DAEMONIZE_NO_REDIR   = 0x02;  ///< Flag to skip std in/out/err redirects -- see daemonize()
static const uint DAEMONIZE_NO_EXIT    = 0x04;  ///< Flag to return false on error instead of calling `exit(1)` -- see daemonize()
static const uint DAEMONIZE_USE_STDERR = 0x08;  ///< Flag to use `stderr` instead of `syslog` for writing errors -- see daemonize()

/** Daemonize current process to run in background as a service (Linux/Unix).
 - \#include <evo/process.h>
 - This writes errors to `syslog` by default, or writes errors to `stderr` if `flags & DAEMONIZE_USE_STDERR`
   - This is useful when `stderr` has been redirected to a log file
 - By default, this never returns on error and instead does `exit(1)` on error, unless `flags & DAEMONIZE_NO_EXIT`
 - Not supported in Windows
 .
 \param  flags  Flags combined with bitwise `OR`:
                 - `DAEMONIZE_NO_CD` to skip changing current directory to `/`
                 - `DAEMONIZE_NO_REDIR` to skip redirecting std in/out/err to `/dev/null`
                 - `DAEMONIZE_NO_EXIT` to skip `exit()` on error and return `false` instead
                 - `DAEMONIZE_USE_STDERR` to use `stderr` instead of `syslog` for error messages, and not redirect stderr if `flags & DAEMONIZE_NO_REDIR`
 \return  Whether successful, false on error if `flags & DAEMONIZE_NO_EXIT`, otherwise on error this calls `exit(1)` and never returns
*/
inline bool daemonize(uint flags=0) {
    #define EVO_TEMP_LOG_ERR(MSG, ...) { \
        if (use_stderr) \
            fprintf(stderr, "daemonize: " MSG "\n", __VA_ARGS__); \
        else \
            syslog(LOG_ERR, "fork() failed: %s", strerror(errno)); \
    }

    bool use_stderr = (flags & DAEMONIZE_USE_STDERR);
    if (!use_stderr)
        openlog("daemonize", LOG_PID, LOG_DAEMON);

    bool result = false;
    for (;;) {
        // Fork to detach from parent
        pid_t pid = fork();
        if (pid == -1) {
            EVO_TEMP_LOG_ERR("fork() failed: %s", strerror(errno));
            break;
        }
        if (pid > 0)
            _exit(0); // parent

        // New session
        setsid();

        // Fork again to completely detatch from terminal
        pid = fork();
        if (pid == -1) {
            EVO_TEMP_LOG_ERR("fork() failed: %s", strerror(errno));
            break;
        }
        if (pid > 0)
            ::_exit(0); // parent

        // Reset umask, cd root
        umask(0);
        if ((flags & DAEMONIZE_NO_CD) == 0 && chdir("/") != 0)
            EVO_TEMP_LOG_ERR("chdir() on / failed: %s", strerror(errno));

        // Redirect std in/out/err to /dev/null
        if (flags & DAEMONIZE_NO_REDIR) {
            close(STDIN_FILENO);
            close(STDOUT_FILENO);
            if (!use_stderr)
                close(STDERR_FILENO);
            if (open("/dev/null", O_RDWR) != 0) {
                EVO_TEMP_LOG_ERR("open() on /dev/null failed: %s", strerror(errno));
                exit(1);
            }
            if (dup(STDIN_FILENO) == -1)
                EVO_TEMP_LOG_ERR("dup() failed for stdout: %s", strerror(errno));
            if (!use_stderr && dup(STDIN_FILENO) == -1)
                EVO_TEMP_LOG_ERR("dup() failed for stderr: %s", strerror(errno));
        }

        // Success
        result = true;
        break;
    }

    if (!use_stderr)
        closelog();
    if (!result && (flags & DAEMONIZE_NO_EXIT) == 0)
        exit(1);
    return result;

    #undef EVO_TEMP_LOG_ERR
}
#endif

///////////////////////////////////////////////////////////////////////////////

/** Process signal handling.
 - This works with Linux/Unix signals as well as Windows signals (which are more limited)
 - \b Caution: %Signal handlers should only be set in the _main thread_ of a process, i.e. in `main()`
 - \b Caution: %Signal handlers are _sensitive to race conditions_ and should be minimal, and _should not_ modify `errno` -- especially in Linux/Unix
   - Ideally for best safety, limit signal handlers to setting status variable (even better with Atomic)
 .

\par Example

Example using Signal::Main helper for a common case:

\code
#include <evo/process.h>

static void on_shutdown(Signal::SigNumType, Signal::Type) {
    // shutdown code goes here
}

int main() {
    // Setup shutdown handler and call Signal::shutdown_sync() from destructor
    Signal::Main signal_main(on_shutdown);

    // main code goes here

    return 0;
}
\endcode

Here's the same code without the helper:

\code
#include <evo/process.h>

static void on_shutdown(Signal::SigNumType, Signal::Type) {
    // shutdown code goes here
}

int main() {
    Signal::set_on_shutdown(on_shutdown);

    // main code goes here

    // Required for Windows
    Signal::shutdown_sync();
    return 0;
}
\endcode
*/
class Signal {
public:
#if defined(_WIN32)
    typedef DWORD SigNumType;
#else
    typedef int SigNumType;     ///< Signal number value type
#endif

    /** %Signal handling action. */
    enum Action {
        aDEFAULT,       ///< Use default handler, i.e. remove user handler
        aIGNORE         ///< Ignore signal
    };

    /** %Signal type. */
    enum Type {
        tUNKNOWN = 0,   ///< Unknown signal type (used internally)
        // Portable (Linux/Unix/Windows)
        tINTERRUPT,     ///< Interrupted by Control-C (SIGINT) [Portable: Linux/Unix/Windows]
        tTERMINATE,     ///< General terminate (SIGTERM) [Portable: Linux/Unix/Windows]
        // Linux/Unix only
        tPIPE,          ///< Write on broken pipe (SIGPIPE) [Linux/Unix]
        tCHILD,         ///< Child process terminated (SIGCHLD), ignored by default [Linux/Unix]
        tHUP,           ///< Terminal hangup, or daemon reload (SIGHUP) [Linux/Unix]
        tUSER1,         ///< User defined signal 1 (SIGUSR1) [Linux/Unix]
        tUSER2,         ///< User defined signal 2 (SIGUSR2) [Linux/Unix]
        // Windows only
        tWIN_BREAK,     ///< Control-Break (CTRL_BREAK_EVENT) [Windows]
        tWIN_CLOSE,     ///< Process closed by user (CTRL_CLOSE_EVENT) [Windows]
        tWIN_LOGOFF,    ///< User logging off, though which user is undefined (CTRL_LOGOFF_EVENT) [Windows, services only]
        tWIN_SHUTDOWN   ///< System shutting down (CTRL_SHUTDOWN_EVENT) [Windows, services only]
    };

    typedef void (*Handler)(SigNumType, Type);      ///< %Signal handler function type
    typedef void (*OnShutdown)(SigNumType, Type);   ///< General shutdown handler type

    /** Helper for common shutdown signal handling in program `main()`.
     - Create an instance in program `main()` to setup a shutdown handler
     - Destructor calls Signal::shutdown_sync(), as required for portability
    */
    struct Main {
        bool error;     ///< Error flag, set to false on success

        /** Default constructor. */
        Main() : error(false) {
        }

        /** Constructor that sets up shutdown handler.
         - This calls Signal::set_on_shutdown()
         .
         \param  on_shutdown  Shutdown handler to use
         \param  except       Whether to throw an exception on error, default set by Evo config: EVO_EXCEPTIONS
        */
        Main(Signal::OnShutdown on_shutdown, bool except=EVO_EXCEPTIONS) : error(false) {
            if (!Signal::set_on_shutdown(on_shutdown)) {
                if (except) {
                    EVO_THROW(Exception, "Signal::set_on_shutdown() failed");
                } else
                    fprintf(stderr, "Signal::set_on_shutdown() failed\n");
                error = true;
            }
        }

        /** Destructor.
         - This calls Signal::set_on_shutdown() to unset shutdown handler, then calls Signal::shutdown_sync()
        */
        ~Main() {
            Signal::set_on_shutdown(NULL);
            Signal::shutdown_sync();
        }
    };

    /** Helper for common server shutdown signal handling in program `main()`.
     - This expands on Signal::Main for use with servers like AsyncServer and provides a default on_shutdown() callback for shutting down the server
     - This uses a static variable for current server, so shutdown handler can do it's job without a global variable
     - This should be used in the same scope as the given server instance
     .
     \tparam  T  Server type to use, usually an AsyncServer -- must have `shutdown()` method (with no params)
    */
    template<class T>
    struct MainServer : Main {
        /** Constructor that sets default shutdown handler for server.
         - This calls Signal::set_on_shutdown()
         .
         \param  server  Server instance to use
         \param  except  Whether to throw an exception on error, default set by Evo config: EVO_EXCEPTIONS
        */
        MainServer(T& server, bool except=EVO_EXCEPTIONS) : Main(on_shutdown, except) {
            global_server() = &server;
        }

        /** Default shutdown handler, called on shutdown signal. */
        static void on_shutdown(Signal::SigNumType, Signal::Type) {
            global_server()->shutdown();
        }

    private:
        static T*& global_server() {
            static T* server = NULL;
            return server;
        }
    };

    /** %Set shutdown handler.
     - This sets a general shutdown handler that is called on tINTERRUPT or tTERMINATE signals
     - When this is set, shutdown_sync() should be called by `main()` right before it returns -- this is required for graceful shutdown to work correctly in Windows
     - Other handlers that were set via set_handler() are called before `on_shutdown`
     .
    */
    static bool set_on_shutdown(OnShutdown on_shutdown) {
        State& state = get_state();
        #if defined(_WIN32)
            Condition::Lock lock(state.condmutex); // locked until return
            if (on_shutdown != NULL && !state.registered) {
                if (::SetConsoleCtrlHandler(main_handler, TRUE) != TRUE)
                    return false;
                state.registered = true;
            }
        #else
            set_handler(tINTERRUPT, NULL);
            set_handler(tTERMINATE, NULL);
        #endif
        state.on_shutdown = on_shutdown;
        return true;
    }

    /** Sync shutdown with signal handler.
     - This should be called right before `main()` exits, to notify that the process has shut down
     - Windows signal handlers work differently and run in another thread, and this notifies that thread that shutdown is complete so it may finish and kill the process
     - This is a no-op if a shutdown handler wasn't set, and is always a no-op in Linux/Unix
     .
    */
    static void shutdown_sync() {
        #if defined(_WIN32)
            State& state = get_state();
            Condition::Lock lock(state.condmutex);
            state.shutdown_flag = true;
            state.condmutex.notify_all();
        #endif
    }

    /** %Set signal handling option.
     \param  type    %Signal type
     \param  action  Action for signal
     \return         Whether successful, false if action couldn't be set
    */
    static bool set_handler(Type type, Action action) {
        #if defined(_WIN32)
            if (action == aDEFAULT) {
                // Set to default by removing previously added handler
                State& state = get_state();
                Condition::Lock lock(state.condmutex);
                state.handlers.remove(get_signum(type));
                if (state.registered && state.on_shutdown == NULL && state.handlers.empty()) {
                    ::SetConsoleCtrlHandler(main_handler, FALSE);
                    state.registered = false;
                }
                return true;
            } else {
                // Windows can only ignore tINTERRUPT
                if (type != tINTERRUPT)
                    return false;
                return (::SetConsoleCtrlHandler(NULL, TRUE) == TRUE);
            }
        #else
            const int signum = get_signum(type);
            if (signum == INVALID_SIGNUM)
                return false;
            get_state().handlers.remove(signum);

            struct sigaction action_data;
            action_data.sa_handler = (action == aDEFAULT ? SIG_DFL : SIG_IGN);
            action_data.sa_flags = 0;
            sigemptyset(&action_data.sa_mask);

            return (::sigaction(signum, &action_data, NULL) == 0);
        #endif
    }

    /** %Set signal handler.
     \param  type     %Signal type
     \param  handler  Handler for signal type, NULL to make sure main handler is installed without setting a user defined handler
     \return          Whether successful, false if action couldn't be set
    */
    static bool set_handler(Type type, Handler handler) {
        State& state = get_state();
        #if defined(_WIN32)
            const DWORD signum = get_signum(type);
            if (signum == INVALID_SIGNUM)
                return false;
            if (handler != NULL) {
                Condition::Lock lock(state.condmutex);
                state.handlers[signum] = handler;
                if (state.registered)
                    return true;
            }
            if (::SetConsoleCtrlHandler(main_handler, TRUE) == TRUE) {
                Condition::Lock lock(state.condmutex);
                state.registered = true;
            }
            return false;
        #else
            const int signum = get_signum(type);
            if (signum == INVALID_SIGNUM)
                return false;
            if (handler != NULL)
                state.handlers[signum] = handler;

            struct sigaction action_data;
            action_data.sa_handler = main_handler;
            action_data.sa_flags = 0;
            sigfillset(&action_data.sa_mask); // block all signals while handling signal

            return (::sigaction(signum, &action_data, NULL) == 0);
        #endif
    }

    /** Send signal to current process.
     \param  pid     Process ID or process group ID to send to
     \param  signal  %Signal type to send
    */
    static bool send_signal(ProcessId pid, Type signal) {
        const SigNumType signum = get_signum(signal);
        if (signal == tUNKNOWN)
            return false;

        #if defined(_WIN32)
            return (::GenerateConsoleCtrlEvent(signum, pid) == TRUE);
        #else
            return (::kill(pid, signum) == 0);
        #endif
    }

private:
    // Internal state
    struct State {
        #if defined(_WIN32)
            Condition condmutex;
            bool registered;
        #endif

        bool shutdown_flag;
        OnShutdown on_shutdown;
        MapList<SigNumType,Handler> handlers;

        State() {
            #if defined(_WIN32)
                registered = false;
            #endif
            shutdown_flag = false;
            on_shutdown = NULL;
        }
    };

    static State& get_state() {
        static State state;
        return state;
    }

#if defined(_WIN32)
    static const DWORD INVALID_SIGNUM = 0xFFFFFFFF;

    static DWORD get_signum(Type type) {
        switch (type) {
            case tINTERRUPT: return CTRL_C_EVENT;
            case tWIN_BREAK: return CTRL_BREAK_EVENT;
            case tWIN_CLOSE: return CTRL_CLOSE_EVENT;
            case tWIN_LOGOFF: return CTRL_LOGOFF_EVENT;
            case tWIN_SHUTDOWN: return CTRL_SHUTDOWN_EVENT;
            default: break;
        }
        return INVALID_SIGNUM;
    }

    static Type get_sigtype(SigNumType value) {
        switch (value) {
            case CTRL_C_EVENT: return tINTERRUPT;
            case CTRL_BREAK_EVENT: return tWIN_BREAK;
            case CTRL_CLOSE_EVENT: return tWIN_CLOSE;
            case CTRL_LOGOFF_EVENT: return tWIN_LOGOFF;
            case CTRL_SHUTDOWN_EVENT: return tWIN_SHUTDOWN;
            default: break;
        }
        return tUNKNOWN;
    }

    static BOOL WINAPI main_handler(DWORD value) {
        const Type type = get_sigtype(value);
        if (type != tUNKNOWN) {
            State& state = get_state();
            const Handler* h;
            {
                Condition::Lock lock(state.condmutex);
                h = state.handlers.find(value);
            }
            if (h != NULL)
                (*h)(value, type);

            Condition::Lock lock(state.condmutex);
            if (state.registered && state.on_shutdown != NULL) {
                lock.unlock();
                (*state.on_shutdown)(value, type);
                lock.lock();
                while (!state.shutdown_flag)
                    state.condmutex.wait(Condition::INF, true);
            }
        }
        return FALSE;
    }

#else
    static const int INVALID_SIGNUM = -1;

    static int get_signum(Type type) {
        switch (type) {
            case tINTERRUPT: return SIGINT;
            case tTERMINATE: return SIGTERM;
            case tPIPE:  return SIGPIPE;
            case tCHILD: return SIGCHLD;
            case tHUP:   return SIGHUP;
            case tUSER1: return SIGUSR1;
            case tUSER2: return SIGUSR2;
            default: break;
        }
        return INVALID_SIGNUM;
    }

    static Type get_sigtype(bool& shutdown, SigNumType value) {
        shutdown = false;
        switch (value) {
            case SIGINT:  shutdown = true; return tINTERRUPT;
            case SIGTERM: shutdown = true; return tTERMINATE;
            case SIGPIPE: return tPIPE;
            case SIGCHLD: return tCHILD;
            case SIGHUP:  return tHUP;
            case SIGUSR1: return tUSER1;
            case SIGUSR2: return tUSER2;
            default: break;
        }
        return tUNKNOWN;
    }

    static void main_handler(int value) {
        bool shutdown;
        const Type type = get_sigtype(shutdown, value);

        State& state = get_state();
        const Handler* h = state.handlers.find(value);
        if (h != NULL)
            (*h)(value, type);

        if (shutdown && state.on_shutdown != NULL)
            (*state.on_shutdown)(value, type);
    }
#endif
};

///////////////////////////////////////////////////////////////////////////////
//@}
}
#endif
