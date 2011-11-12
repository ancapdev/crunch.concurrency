// Copyright (c) 2011, Christian Rorvik
// Distributed under the Simplified BSD License (See accompanying file LICENSE.txt)

#include "crunch/concurrency/thread.hpp"
#include "crunch/concurrency/exceptions.hpp"
#include "./thread_data.hpp"

namespace Crunch { namespace Concurrency {

Thread::~Thread()
{
    if (IsJoinable())
    {
        Cancel();
        Detach();
    }
}

void Thread::Cancel()
{
    if (mData)
        mData->cancellationRequested = true;
}

bool Thread::IsCancellationRequested() const
{
    return mData && mData->cancellationRequested && !mData->canceled;
}

ThreadId Thread::GetId() const
{
    if (mData)
        return mData->id;
    else
        return ThreadId();
}

void SetThreadCancellationPolicy(bool enableCancellation)
{
    if (Thread::Data::tCurrent)
        Thread::Data::tCurrent->cancellationEnabled = enableCancellation;
} 

void ThreadCancellationPoint()
{
    if (Thread::Data::tCurrent &&
        Thread::Data::tCurrent->cancellationRequested &&
        !Thread::Data::tCurrent->canceled)
    {
        Thread::Data::tCurrent->canceled = true;
        throw ThreadCanceled();
    }
}

bool IsThreadCancellationEnabled()
{
    return Thread::Data::tCurrent && Thread::Data::tCurrent->cancellationEnabled;
}

bool IsThreadCancellationRequested()
{
    return Thread::Data::tCurrent && Thread::Data::tCurrent->cancellationRequested;
}

}}

