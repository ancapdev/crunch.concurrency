// Copyright (c) 2011, Christian Rorvik
// Distributed under the Simplified BSD License (See accompanying file LICENSE.txt)

#ifndef CRUNCH_CONCURRENCY_DETAIL_SYSTEM_EVENT_HPP
#define CRUNCH_CONCURRENCY_DETAIL_SYSTEM_EVENT_HPP

#include "crunch/base/platform.hpp"
#include "crunch/concurrency/api.hpp"

#if defined (CRUNCH_PLATFORM_WIN32)
#   include "crunch/base/platform/win32/wintypes.hpp"
#elif defined (CRUNCH_PLATFORM_LINUX) || defined (CRUNCH_PLATFORM_DARWIN)
#   include <pthread.h>
#else
#   error "Unsupported platform"
#endif

namespace Crunch { namespace Concurrency { namespace Detail {

class CRUNCH_CONCURRENCY_API SystemEvent
{
public:
    SystemEvent(bool initialState = false);
    ~SystemEvent();

    void Set();
    void Reset();
    void Wait();

private:
#if defined (CRUNCH_PLATFORM_WIN32)
    HANDLE mEvent;
#else
    // TODO: more efficient implementation with futexes
    pthread_mutex_t mMutex;
    pthread_cond_t mCondition;
    volatile bool mState;
#endif
};

}}}

#endif
