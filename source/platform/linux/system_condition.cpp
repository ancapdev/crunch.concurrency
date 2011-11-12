// Copyright (c) 2011, Christian Rorvik
// Distributed under the Simplified BSD License (See accompanying file LICENSE.txt)

#include "crunch/concurrency/detail/system_condition.hpp"
#include "crunch/base/assert.hpp"

namespace Crunch { namespace Concurrency { namespace Detail {

SystemCondition::SystemCondition()
{
    CRUNCH_ASSERT_ALWAYS(pthread_cond_init(&mCondition, nullptr) == 0);
}

SystemCondition::~SystemCondition()
{
    pthread_cond_destroy(&mCondition);
}

void SystemCondition::Wait(SystemMutex& lock)
{
    CRUNCH_ASSERT_ALWAYS(pthread_cond_wait(&mCondition, &lock.mMutex) == 0);
}

void SystemCondition::WakeOne()
{
    CRUNCH_ASSERT_ALWAYS(pthread_cond_signal(&mCondition) == 0);
}

void SystemCondition::WakeAll()
{
    CRUNCH_ASSERT_ALWAYS(pthread_cond_broadcast(&mCondition) == 0);
}

}}}
