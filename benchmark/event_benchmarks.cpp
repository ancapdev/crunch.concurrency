// Copyright (c) 2011, Christian Rorvik
// Distributed under the Simplified BSD License (See accompanying file LICENSE.txt)

#include "crunch/concurrency/event.hpp"

#include "crunch/benchmarking/stopwatch.hpp"
#include "crunch/benchmarking/statistical_profiler.hpp"
#include "crunch/benchmarking/result_table.hpp"

#include "crunch/test/framework.hpp"

namespace Crunch { namespace Concurrency {

BOOST_AUTO_TEST_SUITE(EventBenchmarks)

BOOST_AUTO_TEST_CASE(RemoveMe)
{
    using namespace Benchmarking;

    Stopwatch stopwatch;

    StatisticalProfiler profiler(0.01, 100, 1000, 10);


    ResultTable<std::tuple<int32, double, double, double, double, double>> results(
        "Event",
        1,
        std::make_tuple("count", "min", "max", "mean", "median", "stddev"));


    Event event;

    int const maxCount = 10;
    auto nullFunc = []{};
    Waiter::Typed<decltype(nullFunc)>* waiters[maxCount];
    for (int i = 0; i < maxCount; ++i)
        waiters[i] = Waiter::Create(nullFunc, false);

    int const reps = 100;
    for (int32 count = 1; count < maxCount; ++count)
    {
        profiler.Reset();

        while (!profiler.IsDone())
        {
            stopwatch.Start();
            for (int i = 0; i < reps; ++i)
            {
                for (int j = 0; j < count; ++j)
                    event.AddWaiter(waiters[j]);
                for (int j = 0; j < count; ++j)
                    event.RemoveWaiter(waiters[j]);
            }
            stopwatch.Stop();
            profiler.AddSample(stopwatch.GetElapsedNanoseconds() / (reps * count));
        }

        results.Add(std::make_tuple(
            count,
            profiler.GetMin(),
            profiler.GetMax(),
            profiler.GetMean(),
            profiler.GetMedian(),
            profiler.GetStdDev()));
    }

    for (int i = 0; i < maxCount; ++i)
        waiters[i]->Destroy();
}

BOOST_AUTO_TEST_SUITE_END()

}} 