// Copyright (c) 2011, Christian Rorvik
// Distributed under the Simplified BSD License (See accompanying file LICENSE.txt)

#include "crunch/concurrency/atomic.hpp"

#include "crunch/benchmarking/stopwatch.hpp"
#include "crunch/benchmarking/statistical_profiler.hpp"
#include "crunch/benchmarking/result_table.hpp"

#include "crunch/test/framework.hpp"

namespace Crunch { namespace Concurrency {

using namespace Benchmarking;

struct AtomicTester
{
    typedef ResultTable<std::tuple<int32, std::string, std::string, double, double, double, double, double>> ResultTableType;

    AtomicTester()
        : results("Concurrency_AtomicOperations", 1, std::make_tuple("size", "op", "ordering", "min", "max", "mean", "median", "stddev"))
        , profiler(0.01, 100, 10000, 100)
    {}

    template<typename OpFunc>
    void Run(std::size_t size, char const* opName, OpFunc const& op)
    {
        Run(size, opName, op, "relaxed", MEMORY_ORDER_RELAXED);
        Run(size, opName, op, "acquire", MEMORY_ORDER_ACQUIRE);
        Run(size, opName, op, "release", MEMORY_ORDER_RELEASE);
        Run(size, opName, op, "acq_rel", MEMORY_ORDER_ACQ_REL);
        Run(size, opName, op, "seq_cst", MEMORY_ORDER_SEQ_CST);
        Run(size, opName, op, "consume", MEMORY_ORDER_CONSUME);
    }

    template<typename OpFunc>
    void Run(std::size_t size, char const* opName, OpFunc const& op, char const* orderingName, MemoryOrder ordering)
    {
        int const reps = 1000;
        int const unrolled = 10;
        profiler.Reset();
        while (!profiler.IsDone())
        {
            stopwatch.Start();
            for (int i = 0; i < reps; ++i)
                for (int j = 0; j < unrolled; ++j)
                    op(ordering);
            stopwatch.Stop();
            profiler.AddSample(stopwatch.GetElapsedNanoseconds() / (reps * unrolled));
        }

        results.Add(std::make_tuple(
            static_cast<int32>(size),
            opName,
            orderingName,
            profiler.GetMin(),
            profiler.GetMax(),
            profiler.GetMean(),
            profiler.GetMedian(),
            profiler.GetStdDev()));
    }

    template<typename T>
    void RunAll()
    {
        Atomic<T> a(0);
        Run(sizeof(T), "CompareAndSwap", [&] (MemoryOrder ordering) { T b = 0; a.CompareAndSwap(b, b, ordering); });
        Run(sizeof(T), "Swap", [&] (MemoryOrder ordering) { a.Swap(1, ordering); });
        Run(sizeof(T), "And", [&] (MemoryOrder ordering) { a.And(1, ordering); });
        Run(sizeof(T), "Or", [&] (MemoryOrder ordering) { a.Or(1, ordering); });
        Run(sizeof(T), "Xor", [&] (MemoryOrder ordering) { a.Xor(1, ordering); });
        Run(sizeof(T), "Inc", [&] (MemoryOrder ordering) { a.Increment(ordering); });
        Run(sizeof(T), "Dec", [&] (MemoryOrder ordering) { a.Decrement(ordering); });
        Run(sizeof(T), "Add", [&] (MemoryOrder ordering) { a.Add(1, ordering); });
        Run(sizeof(T), "Sub", [&] (MemoryOrder ordering) { a.Sub(1, ordering); });
        Run(sizeof(T), "Load", [&] (MemoryOrder ordering) { a.Load(ordering); });
        Run(sizeof(T), "Store", [&] (MemoryOrder ordering) { a.Store(1, ordering); });
    }

    ResultTableType results;
    StatisticalProfiler profiler;
    Stopwatch stopwatch;
};

BOOST_AUTO_TEST_SUITE(AtomicBenchmarks)

BOOST_AUTO_TEST_CASE(AtomicOperationsBenchmark)
{
    AtomicTester tester;
    tester.RunAll<uint32>();
    tester.RunAll<uint64>();
}

BOOST_AUTO_TEST_SUITE_END()
}}