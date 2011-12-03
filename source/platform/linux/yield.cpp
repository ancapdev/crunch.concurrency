// Copyright (c) 2011, Christian Rorvik
// Distributed under the Simplified BSD License (See accompanying file LICENSE.txt)

#include "crunch/concurrency/yield.hpp"

#include <sched.h>
#include <time.h>

namespace Crunch { namespace Concurrency {

void ThreadYield()
{
    sched_yield();
}

void ThreadSleep(Duration duration)
{
    if (duration.IsNegative())
        return;

    std::uint64_t const NanoSecondsPerSecond = 1000000000ull;
    std::uint64_t const totalNs = static_cast<std::uint64_t>(duration.GetTotalNanoseconds());
    std::uint64_t const seconds = totalNs / NanoSecondsPerSecond;
    std::uint64_t const nanoseconds = totalNs - seconds * NanoSecondsPerSecond;
    timespec const tsDuration =
    { 
        static_cast<time_t>(seconds),
        static_cast<long>(nanoseconds)
    };

    nanosleep(&tsDuration, nullptr);
}

}}
