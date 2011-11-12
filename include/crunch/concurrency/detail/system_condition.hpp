// Copyright (c) 2011, Christian Rorvik
// Distributed under the Simplified BSD License (See accompanying file LICENSE.txt)

#ifndef CRUNCH_CONCURRENCY_DETAIL_SYSTEM_CONDITION_HPP
#define CRUNCH_CONCURRENCY_DETAIL_SYSTEM_CONDITION_HPP

#include "crunch/base/platform.hpp"
#include "crunch/concurrency/detail/system_mutex.hpp"

#if defined (CRUNCH_PLATFORM_WIN32)
#   include <type_traits>
#elif defined (CRUNCH_PLATFORM_LINUX)
#   include <pthread.h>
#else
#   error "Unsupported platform"
#endif

namespace Crunch { namespace Concurrency { namespace Detail {

class SystemCondition
{
public:
    SystemCondition();
    ~SystemCondition();

    void Wait(SystemMutex& lock);
    void WakeOne();
    void WakeAll();

private:
#if defined (CRUNCH_PLATFORM_WIN32)
    typedef std::aligned_storage<8, 8>::type ConditionVariableStorageType;
    ConditionVariableStorageType mConditionVariableStorage;
#else
    pthread_cond_t mCondition;
#endif
};

}}}

#endif
