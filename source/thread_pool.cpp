// Copyright (c) 2011, Christian Rorvik
// Distributed under the Simplified BSD License (See accompanying file LICENSE.txt)

#include "crunch/concurrency/thread_pool.hpp"

namespace Crunch { namespace Concurrency {

ThreadPool::ThreadPool(std::uint32_t maxThreadCount)
    : mMaxThreadCount(maxThreadCount)
    , mStop(false)
    , mIdleThreadCount(0)
{}

ThreadPool::~ThreadPool()
{
    {
        Detail::SystemMutex::ScopedLock const lock(mLock);
        mStop = true;
        mHasWork.WakeAll();
    }

    std::for_each(mThreads.begin(), mThreads.end(), [] (Thread& t)
    {
        t.Join();
    });

    std::for_each(mWork.begin(), mWork.end(), [] (WorkItem const& workItem)
    {
        workItem();
    });
}

std::uint32_t ThreadPool::GetThreadCount() const
{
    Detail::SystemMutex::ScopedLock lock(mLock);
    return static_cast<std::uint32_t>(mThreads.size());
}

void ThreadPool::AddWorkItem(WorkItem&& work)
{
    Detail::SystemMutex::ScopedLock lock(mLock);

    mWork.push_back(std::move(work));

    if (mIdleThreadCount == 0 &&
        mThreads.size() < mMaxThreadCount)
    {
        mThreads.push_back(Thread([&]
        {
            while (!mStop)
            {
                mLock.Lock();
                mIdleThreadCount++;

                while (mWork.empty())
                {
                    mHasWork.Wait(mLock);
                    if (mStop)
                    {
                        mIdleThreadCount--;
                        mLock.Unlock();
                        return;
                    }
                }

                WorkItem workItem = std::move(mWork.front());
                mWork.pop_front();

                mIdleThreadCount--;
                mLock.Unlock();

                try
                {
                    workItem();
                }
                catch (...)
                {
                }
            }
        }));
    }
    else
    {
        mHasWork.WakeOne();
    }
}


}}