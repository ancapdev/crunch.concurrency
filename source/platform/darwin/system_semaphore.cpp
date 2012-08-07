// Copyright (c) 2011, Christian Rorvik
// Distributed under the Simplified BSD License (See accompanying file LICENSE.txt)

#include "crunch/concurrency/detail/system_semaphore.hpp"
#include "crunch/base/assert.hpp"

#include <mach/mach_init.h>
#include <mach/semaphore.h>
#include <mach/task.h>

namespace Crunch { namespace Concurrency { namespace Detail {

SystemSemaphore::SystemSemaphore(std::uint32_t initialCount)
    : mCount(static_cast<std::int32_t>(initialCount))
{
    CRUNCH_ASSERT_ALWAYS(
        semaphore_create(
             mach_task_self(),
             &mSemaphore,
             SYNC_POLICY_FIFO,
             static_cast<int>(initialCount)) == KERN_SUCCESS);
}

SystemSemaphore::~SystemSemaphore()
{
    semaphore_destroy(mach_task_self(), mSemaphore);
}

void SystemSemaphore::Post()
{
    if (mCount.Increment() < 0)
        CRUNCH_ASSERT_ALWAYS(semaphore_signal(mSemaphore) == KERN_SUCCESS);
}

void SystemSemaphore::Wait()
{
    if (mCount.Decrement() <= 0)
        CRUNCH_ASSERT_ALWAYS(semaphore_wait(mSemaphore) == KERN_SUCCESS);
}

bool SystemSemaphore::TryWait()
{
    std::int32_t count = mCount.Load(MEMORY_ORDER_RELAXED);
    for (;;)
    {
        if (count <= 0)
            return false;

        if (mCount.CompareAndSwap(count, count - 1))
            return true;
    }
}

}}}
