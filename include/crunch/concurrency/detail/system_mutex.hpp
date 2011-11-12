// Copyright (c) 2011, Christian Rorvik
// Distributed under the Simplified BSD License (See accompanying file LICENSE.txt)

#ifndef CRUNCH_CONCURRENCY_DETAIL_SYSTEM_MUTEX_HPP
#define CRUNCH_CONCURRENCY_DETAIL_SYSTEM_MUTEX_HPP

#include "crunch/base/platform.hpp"
#include "crunch/concurrency/lock_guard.hpp"

#if defined (CRUNCH_PLATFORM_WIN32)
#   include <type_traits>
#elif defined (CRUNCH_PLATFORM_LINUX)
#   include <pthread.h>
#else
#   error "Unsupported platform"
#endif

namespace Crunch { namespace Concurrency { namespace Detail {

class SystemMutex
{
public:
    typedef LockGuard<SystemMutex> ScopedLock;

    SystemMutex();

    ~SystemMutex();

    bool TryLock();

    void Lock();

    void Unlock();

private:
    friend class SystemCondition;

#if defined (CRUNCH_PLATFORM_WIN32)
    typedef std::aligned_storage<40, 8>::type CriticalSectionStorageType;
    CriticalSectionStorageType mCriticalSectionStorage;
#else
    pthread_mutex_t mMutex;
#endif
};


}}}

#endif
