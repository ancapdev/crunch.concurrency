// Copyright (c) 2011, Christian Rorvik
// Distributed under the Simplified BSD License (See accompanying file LICENSE.txt)

#ifndef CRUNCH_CONCURRENCY_SPIN_BARRIER_HPP
#define CRUNCH_CONCURRENCY_SPIN_BARRIER_HPP

#include "crunch/base/noncopyable.hpp"
#include "crunch/concurrency/atomic.hpp"

#include <cstdint>

namespace Crunch { namespace Concurrency {

class SpinBarrier : NonCopyable
{
public:
    SpinBarrier(std::uint32_t count)
        : mTotalCount(count)
        , mWaitCount(count, MEMORY_ORDER_RELAXED)
        , mReadyCount(0, MEMORY_ORDER_RELAXED)
    {}

    bool Wait()
    {
        // Decrement and wait for count to reach 0
        mWaitCount.Decrement(MEMORY_ORDER_RELAXED);
        while (mWaitCount.Load(MEMORY_ORDER_RELAXED) != 0)
            CRUNCH_PAUSE();

        // Increment wait count to signal that we are through barrier
        if (mReadyCount.Increment() == (mTotalCount - 1))
        {
            // Last waiter to go through barrier resets counts
            mReadyCount.Store(0, MEMORY_ORDER_RELAXED);
            mWaitCount.Store(mTotalCount, MEMORY_ORDER_RELEASE);
            return true;
        }

        return false;
    }

private:
    std::uint32_t const mTotalCount;
    Atomic<std::uint32_t> mWaitCount;
    Atomic<std::uint32_t> mReadyCount;
};

}}

#endif
