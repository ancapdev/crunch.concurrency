// Copyright (c) 2011, Christian Rorvik
// Distributed under the Simplified BSD License (See accompanying file LICENSE.txt)

#include "crunch/concurrency/detail/system_event.hpp"
#include "crunch/base/assert.hpp"

namespace Crunch { namespace Concurrency { namespace Detail {

SystemEvent::SystemEvent(bool initialState)
    : mState(initialState)
{
    int mutexResult = pthread_mutex_init(&mMutex, nullptr);
    CRUNCH_ASSERT_ALWAYS(mutexResult == 0);

    int conditionResult = pthread_cond_init(&mCondition, nullptr);
    CRUNCH_ASSERT_ALWAYS(conditionResult == 0);
}

SystemEvent::~SystemEvent()
{
    pthread_cond_destroy(&mCondition);
    pthread_mutex_destroy(&mMutex);
}

void SystemEvent::Set()
{
    if (mState)
        return;

    int lockResult = pthread_mutex_lock(&mMutex);
    CRUNCH_ASSERT_ALWAYS(lockResult == 0);

    if (!mState)
    {
        mState = true;
        int broadcastResult = pthread_cond_broadcast(&mCondition);
        CRUNCH_ASSERT_ALWAYS(broadcastResult == 0);
    }

    int unlockResult = pthread_mutex_unlock(&mMutex);
    CRUNCH_ASSERT_ALWAYS(unlockResult == 0);
}

void SystemEvent::Reset()
{
    mState = false;
}

void SystemEvent::Wait()
{
    if (mState)
        return;

    int lockResult = pthread_mutex_lock(&mMutex);
    CRUNCH_ASSERT_ALWAYS(lockResult == 0);
    
    while (!mState)
    {
        int waitResult = pthread_cond_wait(&mCondition, &mMutex);
        CRUNCH_ASSERT_ALWAYS(waitResult == 0);
    }

    int unlockResult = pthread_mutex_unlock(&mMutex);
    CRUNCH_ASSERT_ALWAYS(unlockResult == 0);
}



}}}
