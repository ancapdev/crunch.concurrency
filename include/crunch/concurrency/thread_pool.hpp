// Copyright (c) 2011, Christian Rorvik
// Distributed under the Simplified BSD License (See accompanying file LICENSE.txt)

#ifndef CRUNCH_CONCURRENCY_THREAD_POOL_HPP
#define CRUNCH_CONCURRENCY_THREAD_POOL_HPP

#include "crunch/base/noncopyable.hpp"
#include "crunch/base/result_of.hpp"
#include "crunch/concurrency/future.hpp"
#include "crunch/concurrency/promise.hpp"
#include "crunch/concurrency/thread.hpp"
#include "crunch/concurrency/detail/system_condition.hpp"
#include "crunch/concurrency/detail/system_mutex.hpp"

#include <cstdint>
#include <deque>
#include <vector>

namespace Crunch { namespace Concurrency {

/// Very simple thread pool for coarse grained tasks. Not designed for performance.
class ThreadPool : NonCopyable
{
public:
    ThreadPool(std::uint32_t maxThreadCount);
    ~ThreadPool();

    template<typename F>
    Future<typename ResultOf<F>::Type> Post(F f);

    std::uint32_t GetThreadCount() const;

private:
    template<typename F, typename R> struct Runner;

    typedef std::function<void ()> WorkItem;

    void AddWorkItem(WorkItem&& work);

    std::uint32_t const mMaxThreadCount;

    mutable Detail::SystemMutex mLock;
    volatile bool mStop;

    std::vector<Thread> mThreads;
    std::uint32_t mIdleThreadCount;

    std::deque<WorkItem> mWork;
    Detail::SystemCondition mHasWork;
};

template<typename F, typename R>
struct ThreadPool::Runner
{
    Runner(Promise<R>* p, F f) : p(p), f(f) {}

    void operator ()() const
    {
        p->SetValue(f());
        delete p;
    }

    Promise<R>* p;
    F f;
};

template<typename F>
struct ThreadPool::Runner<F, void>
{
    Runner(Promise<void>* p, F f) : p(p), f(f) {}

    void operator ()() const
    {
        f();
        p->SetValue();
        delete p;
    }

    Promise<void>* p;
    F f;
};

template<typename F>
Future<typename ResultOf<F>::Type> ThreadPool::Post(F f)
{
    // Would like to move promise, but can't capture in lambda by move
    auto promise = new Promise<typename ResultOf<F>::Type>();
    auto future = promise->GetFuture();
    AddWorkItem(Runner<F, typename ResultOf<F>::Type>(promise, f));
    return future;
}



}}

#endif
