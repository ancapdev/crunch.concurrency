// Copyright (c) 2011, Christian Rorvik
// Distributed under the Simplified BSD License (See accompanying file LICENSE.txt)

#include "crunch/concurrency/detail/system_condition.hpp"
#include "crunch/base/assert.hpp"

#include <windows.h>

namespace Crunch { namespace Concurrency { namespace Detail {

SystemCondition::SystemCondition()
{
    static_assert(sizeof(ConditionVariableStorageType) >= sizeof(CONDITION_VARIABLE), "Insufficient storage space for CONDITION_VARIABLE");

    InitializeConditionVariable(reinterpret_cast<CONDITION_VARIABLE*>(&mConditionVariableStorage));
}

SystemCondition::~SystemCondition()
{}

void SystemCondition::Wait(SystemMutex& lock)
{
    BOOL result = SleepConditionVariableCS(
        reinterpret_cast<CONDITION_VARIABLE*>(&mConditionVariableStorage),
        reinterpret_cast<CRITICAL_SECTION*>(&lock.mCriticalSectionStorage),
        INFINITE);

    CRUNCH_ASSERT_ALWAYS(result == TRUE);
}

void SystemCondition::WakeOne()
{
    WakeConditionVariable(reinterpret_cast<CONDITION_VARIABLE*>(&mConditionVariableStorage));
}

void SystemCondition::WakeAll()
{
    WakeAllConditionVariable(reinterpret_cast<CONDITION_VARIABLE*>(&mConditionVariableStorage));
}


}}}
