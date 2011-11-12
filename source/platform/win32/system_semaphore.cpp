// Copyright (c) 2011, Christian Rorvik
// Distributed under the Simplified BSD License (See accompanying file LICENSE.txt)

#include "crunch/concurrency/detail/system_semaphore.hpp"
#include "crunch/base/assert.hpp"

#include <limits>

#include <windows.h>

namespace Crunch { namespace Concurrency { namespace Detail {

SystemSemaphore::SystemSemaphore(uint32 initialCount)
    : mCount(static_cast<int32>(initialCount))
    , mSemaphore(::CreateSemaphore(NULL, static_cast<LONG>(initialCount), std::numeric_limits<LONG>::max(), NULL))
{}

SystemSemaphore::~SystemSemaphore()
{
    ::CloseHandle(mSemaphore);
}

void SystemSemaphore::Post()
{
    if (mCount.Increment() < 0)
        CRUNCH_ASSERT_ALWAYS(::ReleaseSemaphore(mSemaphore, 1, NULL) == TRUE);
}

void SystemSemaphore::Wait()
{
    if (mCount.Decrement() <= 0)
        CRUNCH_ASSERT_ALWAYS(::WaitForSingleObject(mSemaphore, INFINITE) == WAIT_OBJECT_0);
}

bool SystemSemaphore::TryWait()
{
    int32 count = mCount.Load(MEMORY_ORDER_RELAXED);
    for (;;)
    {
        if (count <= 0)
            return false;

        if (mCount.CompareAndSwap(count - 1, count))
            return true;
    }
}

}}}
