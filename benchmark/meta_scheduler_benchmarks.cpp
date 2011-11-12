// Copyright (c) 2011, Christian Rorvik
// Distributed under the Simplified BSD License (See accompanying file LICENSE.txt)

#include "crunch/concurrency/meta_scheduler.hpp"
#include "crunch/concurrency/processor_affinity.hpp"
#include "crunch/benchmarking/stopwatch.hpp"
#include "crunch/benchmarking/statistical_profiler.hpp"
#include "crunch/benchmarking/result_table.hpp"
#include "crunch/benchmarking/stream_result_sink.hpp"
#include "crunch/test/framework.hpp"

#include <array>
#include <tuple>

namespace Crunch { namespace Concurrency {

struct NullWaitable : IWaitable
{
    virtual void AddWaiter(Waiter* waiter) { waiter->Notify(); }
    virtual bool RemoveWaiter(Waiter*) { return false; }
    virtual bool IsOrderDependent() const { return false; }
};

BOOST_AUTO_TEST_SUITE(MetaSchedulerBenchmarks)

BOOST_AUTO_TEST_CASE(SingleThreadedWaitForReadyWaitableBenchmark)
{
    using namespace Benchmarking;

    MetaScheduler::Configuration configuration;
    MetaScheduler ms(configuration);
    MetaScheduler::Context& context = ms.AcquireContext();
    SetCurrentThreadAffinity(ProcessorAffinity(0));
    int const reps = 1000;
    NullWaitable waitable;
    Stopwatch stopwatch;

    ResultTable<std::tuple<char const*, double, double, double, double, double>> results(
        "WaitForReady",
        1,
        std::make_tuple("wait_mode", "min", "max", "mean", "median", "stddev"));

    typedef std::tuple<char const*, WaitMode> NamedWaitMode;
    std::array<NamedWaitMode, 3> const waitModes =
    {
        std::make_tuple("poll", WaitMode::Poll()),
        std::make_tuple("block", WaitMode::Block()),
        std::make_tuple("run", WaitMode::Run())
    };

    StatisticalProfiler profiler(0.01, 1000, 10000, 100);

    std::for_each(waitModes.begin(), waitModes.end(), [&] (NamedWaitMode const& waitMode)
    {
        profiler.Reset();
        while (!profiler.IsDone())
        {
            stopwatch.Start();
            for (int i = 0; i < reps; ++i)
                WaitFor(waitable, std::get<1>(waitMode));
            stopwatch.Stop();
            profiler.AddSample(stopwatch.GetElapsedNanoseconds() / reps);
        }

        results.Add(std::make_tuple(
            std::get<0>(waitMode),
            profiler.GetMin(),
            profiler.GetMax(),
            profiler.GetMean(),
            profiler.GetMedian(),
            profiler.GetStdDev()));

    });

    context.Release();
}

BOOST_AUTO_TEST_SUITE_END()

}}
