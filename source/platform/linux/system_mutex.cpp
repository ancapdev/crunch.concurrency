// Copyright (c) 2011, Christian Rorvik
// Distributed under the Simplified BSD License (See accompanying file LICENSE.txt)

#include "crunch/concurrency/detail/system_mutex.hpp"
#include "crunch/base/assert.hpp"

namespace Crunch { namespace Concurrency { namespace Detail {

SystemMutex::SystemMutex()
{
    int result = pthread_mutex_init(&mMutex, nullptr);
    CRUNCH_ASSERT_ALWAYS(result == 0);
}

SystemMutex::~SystemMutex()
{
    int result = pthread_mutex_destroy(&mMutex);
    CRUNCH_ASSERT_ALWAYS(result == 0);
}

bool SystemMutex::TryLock()
{
    return pthread_mutex_trylock(&mMutex) == 0;
}

void SystemMutex::Lock()
{
    int result = pthread_mutex_lock(&mMutex);
    CRUNCH_ASSERT_ALWAYS(result == 0);
}

void SystemMutex::Unlock()
{
    int result = pthread_mutex_unlock(&mMutex);
    CRUNCH_ASSERT_ALWAYS(result == 0);
}

}}}
