// Copyright (c) 2011, Christian Rorvik
// Distributed under the Simplified BSD License (See accompanying file LICENSE.txt)

#include "crunch/concurrency/detail/system_semaphore.hpp"
#include "crunch/base/assert.hpp"

namespace Crunch { namespace Concurrency { namespace Detail {

SystemSemaphore::SystemSemaphore(uint32 initialCount)
{
    CRUNCH_ASSERT_ALWAYS(sem_init(&mSemaphore, 0, initialCount) == 0);
}

SystemSemaphore::~SystemSemaphore()
{
    sem_destroy(&mSemaphore);
}

void SystemSemaphore::Post()
{
    CRUNCH_ASSERT_ALWAYS(sem_post(&mSemaphore) == 0);
}

void SystemSemaphore::Wait()
{
    CRUNCH_ASSERT_ALWAYS(sem_wait(&mSemaphore) == 0);
}

bool SystemSemaphore::TryWait()
{
    return sem_trywait(&mSemaphore) == 0;
}

}}}
