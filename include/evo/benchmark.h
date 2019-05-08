// Evo C++ Library
/* Copyright 2019 Justin Crowell
Distributed under the BSD 2-Clause License -- see included file LICENSE.txt for details.
*/
///////////////////////////////////////////////////////////////////////////////
/** \file benchmark.h Evo simple micro benchmarking. */
#pragma once
#ifndef INCL_evo_benchmark_h
#define INCL_evo_benchmark_h

#include "fmt.h"
#include "io.h"
#include "thread.h"
#include "timer.h"

///////////////////////////////////////////////////////////////////////////////

/** Shortcut setting up a Benchmark instance.
 - Call EVO_BENCH_RUN() or EVO_BENCH_RUN2() for each function or functor to benchmark
 - See \link evo::Benchmark Benchmark\endlink
 .
 \param  F       Function or functor to use to setup scale (for repeat count) -- passed to Benchmark::scale()
 \param  WARMUP  Warmup count to use (passed to Benchmark constructor), use `0` for default
*/
#define EVO_BENCH_SETUP(F, WARMUP) Benchmark bench(0, WARMUP); bench.scale(F)

/** Shortcut for running a benchmark on given function or functor.
 - Use EVO_BENCH_SETUP() to setup first
 - See \link evo::Benchmark Benchmark\endlink
 .
 \param  F  Function or functor to benchmark -- passed to Benchmark::run(), also used for the benchmark name
*/
#define EVO_BENCH_RUN(F) bench.run(#F, F)

/** Shortcut for running a benchmark on given parameterized functor.
 - This builds a string with the benchmark name and calls `F.get_name()` to add a suffix to the benchmark name
   - This allows the functor to append parameters to the benchmark name
   - Expected method signature `void get_name(String&) const`
 - Use EVO_BENCH_SETUP() to setup first
 - See \link evo::Benchmark Benchmark\endlink
 .
 \param  F  Function or functor to benchmark -- passed to Benchmark::run(), also used for the benchmark name
*/
#define EVO_BENCH_RUN2(F) { String evo_bench_name_(#F); F.get_name(evo_bench_name_); bench.run(evo_bench_name_, F); }

///////////////////////////////////////////////////////////////////////////////

namespace evo {
/** \addtogroup EvoTools */
//@{

///////////////////////////////////////////////////////////////////////////////

/** Micro benchmarking class.
 - Used to benchmark related blocks of code for comparison
 - This can benchmark a function or functor (object with `operator()()` -- note that this method must be const)
   - Functors are useful for "parameterizing" the code being benchmarked (via methods/properties)
 - Each benchmark runs a given function/functor repeatedly according to a repeat count and measures time elapsed
 - Usage:
   - Call scale() first to automatically set a default repeat count (optional)
   - Call run() for each benchmark to run
   - Call report() to show results and clear stored report -- the destructor calls this if there's pending report data
     - Report includes a "DiffBest" column showing the difference between AvgCPU of current call and the best (fastest) call in this report -- lower value is better
   - Repeat if needed
 .

Shortcut helpers:
 - EVO_BENCH_SETUP()
 - EVO_BENCH_RUN()
 - EVO_BENCH_RUN2()
 .

\par Example

\code
#include <evo/benchmark.h>
using namespace evo;

// Function to benchmark
inline void bm_function() {
    for (ulong i = 0; i < 100; ++i)
        printf("%s", "");
}

// Functor to benchmark
struct BM_Class {
    void operator()() const {
        for (ulong i = 0; i < 50; ++i)
            printf("%s", "");
    }
};

int main() {
    // Setup and scale using bm_function
    Benchmark bench;
    bench.scale(bm_function);

    // Benchmark bm_function()
    bench.run("bm_function", bm_function);

    // Benchmark instance of BM_Class
    bench.run("BM_Class", BM_Class());

    return 0;
}
\endcode

Example output:
\code{.unparsed}
Name         Time(nsec)  CPU(nsec)  Count    AvgTime(nsec)  AvgCPU(nsec)  DiffBest(nsec)
bm_function  204529700   203125000  1000000  204.5297       203.125       109.375
BM_Class     96745700    93750000   1000000  96.7457        93.75         0
\endcode

This is the same benchmarking code using the shortcut macros:

\code
int main() {
    // Setup and scale using bm_function
    EVO_BENCH_SETUP(bm_function, 0);

    // Benchmark bm_function()
    EVO_BENCH_RUN(bm_function);

    // Benchmark an instance of BM_Class
    EVO_BENCH_RUN(BM_Class());

    return 0;
}
\endcode

\par Parameterized Example

Here's an example that benchmarks parameterized code using a functor:

\code
#include <evo/benchmark.h>
using namespace evo;

struct BM_Class {
    ulong count;

    // Start with default count=50
    BM_Class() : count(50) {
    }

    // Called by EVO_BENCH_RUN2() to finish benchmark name
    void get_name(String& name) const {
        name << '/' << count;
    }

    // Code to benchmark
    void operator()() const {
        for (ulong i = 0; i < count; ++i)
            printf("%s", "");
    }
};

int main() {
    BM_Class bm_obj;
    EVO_BENCH_SETUP(bm_obj, 0);

    // Benchmark with defaults first
    EVO_BENCH_RUN2(bm_obj);

    // Benchmark other parameter variants
    bm_obj.count = 60;
    EVO_BENCH_RUN2(bm_obj);

    return 0;
}
\endcode

Example output:

\code{.unparsed}
Name       Time(nsec)  CPU(nsec)  Count    AvgTime(nsec)  AvgCPU(nsec)  DiffBest(nsec)
bm_obj/50  83509600    78125000   1000000  83.5096        78.125        0
bm_obj/60  112968900   109375000  1000000  112.9689       109.375       31.25
\endcode
*/
class Benchmark {
public:
    /** Constructor.
     \param  default_count         Default repeat count to use
     \param  default_warmup_count  Default warmup count to use
    */
    Benchmark(ulongl default_count=0, ulongl default_warmup_count=0) : default_count_(default_count), default_warmup_count_(default_warmup_count) {
    }

    /** Destructor.
     - This calls report() if it hasn't been called yet
    */
    ~Benchmark() {
        if (report_.size() > 0)
            report();
    }

    /** Scale the current default repeat count using given function/functor.
     - This figures out a useful repeat count by calling the given function/functor repeatedly until some measurable time has elapsed
     - This sets the default repeat count, overriding the constructor argument
     .
     \param  func    Function or functor to use to scale
     \param  factor  Factor to scale (multiply) the decided repeat count
     \return         New repeat count that was set
    */
    template<class T>
    ulongl scale(const T& func, uint factor=1) {
        const ulong THRESHOLD_MSEC  = 100;
        const ulong THRESHOLD_COUNT = 1000000000;
        const ulongl MAX_COUNT = ULongL::MAX;
        assert( factor > 0 );

        Timer timer;
        timer.start();

        ulongl count = 1;
        func();
        while (timer.msec() < THRESHOLD_MSEC && count < MAX_COUNT) {
            const ulongl target = count * (count > THRESHOLD_COUNT ? 2 : 10);
            while (count < target) {
                EVO_ATOMIC_FENCE(EVO_ATOMIC_SYNC);
                func();
                ++count;
            }
        }

        if (count > MAX_COUNT / factor)
            count = MAX_COUNT;
        else
            count *= factor;

        default_count_ = count;
        return count;
    }

    /** Run benchmark on given function/functor.
     - This does the benchmark and records the result
     - Call report() to print out results
     .
     \tparam  T  Function/Functor type -- inferred by `func` argument
     \param  name          Name to use for report
     \param  func          Function or functor to benchmark
     \param  count         Repeat count to use as the number of times to call `func` during benchmark
     \param  warmup_count  Number of time to call `func` as a warmup before doing the benchmark
     \return               This
    */
    template<class T>
    Benchmark& run(const SubString& name, const T& func, ulongl count, ulongl warmup_count=0) {
        const ulongl WARMUP_COUNT = 100;
        if (count < 1)
            count = 1;
        const ulongl start_count = count;

        if (warmup_count == 0)
            warmup_count = WARMUP_COUNT;
        for (; warmup_count > 0; --warmup_count)
            func(); // warmup

        // Start a monitor thread to keep run from taking too long
        AtomicInt monitor_flag;
        monitor_flag.store(0);
        Thread monitor(monitor_thread, &monitor_flag);
        monitor.thread_start();

        Timer walltimer;
        TimerCpu cputimer;
        walltimer.start();
        cputimer.start();
        for (; count > 0 && monitor_flag.load() == 0; --count)
            func();
        cputimer.stop();
        walltimer.stop();

        // Stop thread, if needed
        monitor_flag.store(1);
        monitor.thread_join();

        report_.add(ReportItem(name, walltimer.nsec(), cputimer.nsec(), start_count - count));
        return *this;
    }

    /** Run benchmark on given function/functor with default repeat and wramup counts.
     - This does the benchmark and records the result
     - Call report() to print out results
     .
     \tparam  T  Function/Functor type -- inferred by `func` argument
     \param  name  Name to use for report
     \param  func  Function or functor to benchmark
     \return       This
    */
    template<class T>
    Benchmark& run(const SubString& name, const T& func) {
        return run(name, func, default_count_, default_warmup_count_);
    }

    /** Clear current report. */
    Benchmark& clear() {
        report_.clear();
        return *this;
    }

    /** Write benchmark report to output stream or string.
     - Call run() first to run each benchmarks, then call this to show the report -- repeat if needed
     - This clears current benchmark data to setup for another set of benchmarks
     .
     \tparam  T  Output stream/string type (inferred by argument)
     \param  out   Output stream/string to write to
     \param  type  Output format type -- see FmtTable::Type
     \return       This
    */
    template<class T>
    Benchmark& report_out(T& out, FmtTable::Type type=FmtTable::tTEXT) {
        if (!report_.empty()) {
            const SubString COLUMN_NAMES[] = {
                "Name",
                "Time(nsec)",
                "CPU(nsec)",
                "Count",
                "AvgTime(nsec)",
                "AvgCPU(nsec)",
                "DiffBest(nsec)",
                ""
            };

            // Find lowest cpu time
            const ReportItem* fastest = NULL;
            for (ReportList::Iter iter(report_); iter; ++iter) {
                const ReportItem& item = *iter;
                if (fastest == NULL || item.cputime_nsec < fastest->cputime_nsec)
                    fastest = &item;
            }

            // Tests
            FmtTable table(COLUMN_NAMES, 0);
            FmtTableOut<T> table_out(out, table, type);
            for (ReportList::Iter iter(report_); iter; ++iter) {
                const ReportItem& item = *iter;
                const double cpu_avg_diff = (fastest == NULL ? 0.0 : ((double)item.cputime_nsec / item.count) - ((double)fastest->cputime_nsec / fastest->count));
                table_out
                    << item.name << item.walltime_nsec << item.cputime_nsec << item.count
                    << ((double)item.walltime_nsec / item.count) << ((double)item.cputime_nsec / item.count)
                    << cpu_avg_diff << NL;
            }
            table_out << fFLUSH;
            out << NL;

            report_.clear();
        }
        return *this;
    }

    /** Write benchmark report to stdout.
     - Call run() first to run each benchmarks, then call this to show the report -- repeat if needed
     - This clears current benchmark data to setup for another set of benchmarks
     .
     \param  type  Output format type -- see FmtTable::Type
    */
    Benchmark& report(FmtTable::Type type=FmtTable::tTEXT) {
        return report_out(con().out, type);
    }

private:
    // Disable copying
    Benchmark(const Benchmark&) EVO_ONCPP11(= delete);
    Benchmark& operator=(const Benchmark&) EVO_ONCPP11(= delete);

    struct ReportItem {
        String name;
        ulongl walltime_nsec;
        ulongl cputime_nsec;
        ulongl count;

        ReportItem() : walltime_nsec(0), cputime_nsec(0), count(0) {
        }

        ReportItem(const SubString& name, ulongl walltime_nsec, ulongl cputime_nsec, ulongl count) : name(name), walltime_nsec(walltime_nsec), cputime_nsec(cputime_nsec), count(count) {
        }

        ReportItem(const ReportItem& src) : name(src.name), walltime_nsec(src.walltime_nsec), cputime_nsec(src.cputime_nsec), count(src.count) {
        }
    };

    typedef List<ReportItem> ReportList;

    ulongl default_count_;
    ulongl default_warmup_count_;
    ReportList report_;

    static void monitor_thread(void* arg) {
        const uint WAIT_TIME = 5000;
        const uint WAIT_INC  = 200;
        AtomicInt& flag = *(AtomicInt*)arg;
        for (uint msec = 0; msec < WAIT_TIME && flag.load() == 0; msec += WAIT_INC)
            sleepms(WAIT_INC);
        flag.store(1);
    }
};

///////////////////////////////////////////////////////////////////////////////
//@}
}
#endif
