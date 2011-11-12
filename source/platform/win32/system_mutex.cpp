// Copyright (c) 2011, Christian Rorvik
// Distributed under the Simplified BSD License (See accompanying file LICENSE.txt)

#include "crunch/concurrency/detail/system_mutex.hpp"

#include <windows.h>

namespace Crunch { namespace Concurrency { namespace Detail {

SystemMutex::SystemMutex()
{
    static_assert(sizeof(CriticalSectionStorageType) >= sizeof(CRITICAL_SECTION), "Insufficient storage space for CRITICAL_SECTION");

    InitializeCriticalSection(reinterpret_cast<CRITICAL_SECTION*>(&mCriticalSectionStorage));
}

SystemMutex::~SystemMutex()
{
    DeleteCriticalSection(reinterpret_cast<CRITICAL_SECTION*>(&mCriticalSectionStorage));
}

bool SystemMutex::TryLock()
{
    return TryEnterCriticalSection(reinterpret_cast<CRITICAL_SECTION*>(&mCriticalSectionStorage)) == TRUE;
}

void SystemMutex::Lock()
{
    EnterCriticalSection(reinterpret_cast<CRITICAL_SECTION*>(&mCriticalSectionStorage));
}

void SystemMutex::Unlock()
{
    LeaveCriticalSection(reinterpret_cast<CRITICAL_SECTION*>(&mCriticalSectionStorage));
}

}}}
