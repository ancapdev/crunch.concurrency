// Copyright (c) 2011, Christian Rorvik
// Distributed under the Simplified BSD License (See accompanying file LICENSE.txt)

#ifndef CRUNCH_CONCURRENCY_DETAIL_SYSTEM_SEMAPHORE_HPP
#define CRUNCH_CONCURRENCY_DETAIL_SYSTEM_SEMAPHORE_HPP

#include "crunch/base/platform.hpp"
#include "crunch/concurrency/yield.hpp"

#if defined (CRUNCH_PLATFORM_WIN32)
#   include "crunch/base/platform/win32/wintypes.hpp"
#   include "crunch/concurrency/atomic.hpp"
#elif defined (CRUNCH_PLATFORM_LINUX)
#   include <semaphore.h>
#else
#   error "Unsupported platform"
#endif

#include <cstdint>

namespace Crunch { namespace Concurrency { namespace Detail {

class SystemSemaphore
{
public:
    SystemSemaphore(std::uint32_t initialCount);
    ~SystemSemaphore();

    void Post();
    void Wait();
    bool TryWait();
    void SpinWait(std::uint32_t spinCount);

private:
#if defined (CRUNCH_PLATFORM_WIN32)
    Atomic<std::int32_t> mCount;
    HANDLE mSemaphore;
#else
    sem_t mSemaphore;
#endif
};

inline void SystemSemaphore::SpinWait(std::uint32_t spinCount)
{
    while (spinCount--)
    {
        if (TryWait())
            return;

        CRUNCH_PAUSE();
    }

    Wait();
}

}}}

#endif
