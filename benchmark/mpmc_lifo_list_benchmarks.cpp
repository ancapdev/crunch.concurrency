// Copyright (c) 2011, Christian Rorvik
// Distributed under the Simplified BSD License (See accompanying file LICENSE.txt)

#include "crunch/concurrency/mpmc_lifo_list.hpp"
#include "crunch/concurrency/constant_backoff.hpp"
#include "crunch/concurrency/null_backoff.hpp"
#include "crunch/concurrency/processor_affinity.hpp"
#include "crunch/concurrency/processor_topology.hpp"
#include "crunch/concurrency/spin_barrier.hpp"
#include "crunch/concurrency/thread.hpp"

#include "crunch/benchmarking/stopwatch.hpp"
#include "crunch/benchmarking/statistical_profiler.hpp"
#include "crunch/benchmarking/result_table.hpp"

#include "crunch/test/framework.hpp"

namespace Crunch { namespace Concurrency {

namespace
{
    struct TestNode
    {
        TestNode* next;
    };

    void SetNext(TestNode& node, TestNode* next)
    {
        node.next = next;
    }

    TestNode* GetNext(TestNode& node)
    {
        return node.next;
    }
}

// TODO:
// - Producer/Consumer
BOOST_AUTO_TEST_SUITE(MPMCLifoListBenchmarks)

BOOST_AUTO_TEST_CASE(PushPopBenchmark)
{
    using namespace Benchmarking;

    std::uint32_t const systemProcCount = GetSystemNumProcessors();

    int const reps = 10000;

    ResultTable<std::tuple<std::int32_t, char const*, double, double, double, double, double>> results(
        "Crunch.Concurrency.MPMCLifoList.PushPop",
        1,
        std::make_tuple("threads", "operation", "min", "max", "mean", "median", "stddev"));

    ProcessorAffinity const oldAffinity = SetCurrentThreadAffinity(ProcessorAffinity(0));

    bool push = true;
    do
    {
        MPMCLifoList<TestNode> list;
        TestNode node;
        if (!push)
        {
            // Create a cycle for infinite pop
            list.Push(&node);
            list.Push(&node);
        }
        for (std::uint32_t procCount = 1; procCount <= systemProcCount; ++procCount)
        {
            volatile bool done = false;
            SpinBarrier startBarrier(procCount);
            SpinBarrier finishBarrier(procCount);

            std::vector<double> threadResults;
            threadResults.resize(procCount, 0.0);

            auto benchmarkFunc = [&list, reps, push] (Stopwatch& stopwatch) -> double
            {
                stopwatch.Start();
                TestNode node;
                if (push)
                    for (int i = 0; i < reps; ++i)
                        list.Push(&node);
                else
                    for (int i = 0; i < reps; ++i)
                        list.Pop();
                stopwatch.Stop();

                return stopwatch.GetElapsedNanoseconds() / reps;
            };

            auto workerFunc = [&] (std::uint32_t index)
            {
                SetCurrentThreadAffinity(ProcessorAffinity(index));
                Benchmarking::Stopwatch stopwatch;
                for (;;)
                {
                    startBarrier.Wait();
                    if (done)
                        return;
                    threadResults[index] = benchmarkFunc(stopwatch);
                    finishBarrier.Wait();
                }
            };

            StatisticalProfiler profiler(0.01, 100, 1000, 10);
            std::vector<Thread> threads;
            for (std::uint32_t i = 1; i < procCount; ++i)
                threads.push_back(Thread([&, i] { workerFunc(i); }));

            Stopwatch stopwatch;
            while (!profiler.IsDone())
            {
                startBarrier.Wait();
                threadResults[0] = benchmarkFunc(stopwatch);
                finishBarrier.Wait();

                for (std::uint32_t i = 0; i < procCount; ++i)
                    profiler.AddSample(threadResults[i]);
            }

            done = true;
            startBarrier.Wait();
            for (std::uint32_t i = 1; i < procCount; ++i)
                threads[i - 1].Join();

            results.Add(std::make_tuple(
                procCount,
                push ? "push" : "pop",
                profiler.GetMin(),
                profiler.GetMax(),
                profiler.GetMean(),
                profiler.GetMedian(),
                profiler.GetStdDev()));
        }

        push = !push;
    } while (!push);

    SetCurrentThreadAffinity(oldAffinity);
}

BOOST_AUTO_TEST_SUITE_END()

}}