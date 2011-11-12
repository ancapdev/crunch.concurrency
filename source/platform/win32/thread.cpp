// Copyright (c) 2011, Christian Rorvik
// Distributed under the Simplified BSD License (See accompanying file LICENSE.txt)

#include "crunch/concurrency/thread.hpp"
#include "crunch/concurrency/exceptions.hpp"

#include "crunch/base/assert.hpp"

#include "../../thread_data.hpp"

#include <memory>

#include <windows.h>

namespace Crunch { namespace Concurrency {

void Thread::Create(std::function<void ()>&& f)
{
    mData.reset(new Data(std::move(f)));
    mData->self = mData;

    DWORD id;
    mData->handle = ::CreateThread(NULL, 0, &Data::EntryPoint, mData.get(), 0, &id);
    if (mData->handle == NULL)
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
        CloseHandle(mData->handle);
        mData.reset();
    }
}

void Thread::Join()
{
    ThreadCancellationPoint();

    if (mData)
    {
        CRUNCH_ASSERT_ALWAYS(WaitForSingleObject(mData->handle, INFINITE) == WAIT_OBJECT_0);
        CloseHandle(mData->handle);
        mData.reset();
    }
}

ThreadId GetThreadId()
{
    return ThreadId(::GetCurrentThreadId());
}

}}
