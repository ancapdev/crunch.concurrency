// Copyright (c) 2011, Christian Rorvik
// Distributed under the Simplified BSD License (See accompanying file LICENSE.txt)

#include "crunch/concurrency/thread.hpp"
#include "crunch/concurrency/exceptions.hpp"

#include "crunch/base/assert.hpp"

#include "../../thread_data.hpp"

#include <memory>

namespace Crunch { namespace Concurrency {

void Thread::Create(std::function<void ()>&& f)
{
    mData.reset(new Data(std::move(f)));
    mData->self = mData;

    pthread_t id;
    int result = pthread_create(&id, nullptr, &Data::EntryPoint, mData.get());
    if (result != 0)
    {
        mData->self.reset();
        mData.reset();
        throw ThreadResourceError();
    }
    mData->id = ThreadId(id);
}

void Thread::Detach()
{
    if (mData)
    {
        CRUNCH_ASSERT_ALWAYS(pthread_detach(mData->id.GetNative()) == 0);
        mData.reset();
    }
}

void Thread::Join()
{
    ThreadCancellationPoint();

    if (mData)
    {
        CRUNCH_ASSERT_ALWAYS(pthread_join(mData->id.GetNative(), nullptr) == 0);
        mData.reset();
    }
}

ThreadId GetThreadId()
{
    return ThreadId(pthread_self());
}

}}
