// Copyright (c) 2011, Christian Rorvik
// Distributed under the Simplified BSD License (See accompanying file LICENSE.txt)

#include "crunch/concurrency/meta_scheduler.hpp"

#include "crunch/base/assert.hpp"
#include "crunch/base/inline.hpp"
#include "crunch/base/noncopyable.hpp"
#include "crunch/base/override.hpp"
#include "crunch/base/stack_alloc.hpp"
#include "crunch/concurrency/event.hpp"
#include "crunch/concurrency/yield.hpp"
#include "crunch/concurrency/detail/system_semaphore.hpp"

#include <algorithm>

namespace Crunch { namespace Concurrency {

MetaScheduler::RunMode::RunMode(Type type, std::uint32_t count, Duration duration)
    : mType(type)
    , mCount(count)
    , mDuration(duration)
{}

MetaScheduler::RunMode MetaScheduler::RunMode::Disabled()
{
    return RunMode(TYPE_DISABLED, 0, Duration::Zero);
}

MetaScheduler::RunMode MetaScheduler::RunMode::Some(std::uint32_t count)
{
    return RunMode(TYPE_SOME, count, Duration::Zero);
}

MetaScheduler::RunMode MetaScheduler::RunMode::Timed(Duration duration)
{
    return RunMode(TYPE_TIMED, 0, duration);
}

MetaScheduler::RunMode MetaScheduler::RunMode::All()
{
    return RunMode(TYPE_ALL, 0, Duration::Zero);
}

MetaScheduler::Configuration::Group::Group(std::uint32_t id, RunMode defaultRunMode)
    : id(id)
    , defaultRunMode(defaultRunMode)
{}

void MetaScheduler::Configuration::AddGroup(std::uint32_t id, RunMode defaultRunMode)
{
    CRUNCH_ASSERT_MSG_ALWAYS(std::find_if(mGroups.begin(), mGroups.end(), [=] (Group const& group) { return group.id == id; }) == mGroups.end(), "Scheduler group already exists");
    mGroups.push_back(Group(id, defaultRunMode));
}

void MetaScheduler::Configuration::AddScheduler(std::uint32_t groupId, SchedulerPtr const& scheduler)
{
    auto it = std::find_if(mGroups.begin(), mGroups.end(), [=] (Group const& group) { return group.id == groupId; });
    CRUNCH_ASSERT_MSG_ALWAYS(it != mGroups.end(), "Invalid scheduler group");
    it->schedulers.push_back(scheduler);
}

class MetaScheduler::ContextImpl : public MetaScheduler::Context, NonCopyable
{
public:
    ContextImpl(MetaScheduler* owner)
        : mOwner(owner)
        , mWaitSemaphore(0)
        , mRefCount(1)
    {
        auto waiter = Waiter::Create([&] { mWaitSemaphore.Post(); }, false);
        mWaiterDestroyer = [=] { waiter->Destroy(); };
        mWaiter = waiter;
    }

    ~ContextImpl()
    {
        mWaiterDestroyer();
    }

    MetaScheduler* GetOwner()
    {
        return mOwner;
    }

    void AddRef()
    {
        mRefCount++;
    }

    void Run(IWaitable& /*until*/)
    {
        // while not done
        //     Acquire idle meta thread
        //     Affinitize to meta thread processor affinity
        //     Set meta thread in TLS context
        //     Add schedulers for meta thread
        // 1:  Run schedulers until idle
        //         If blocked (from within scheduler -- can't switch meta threads at this point)
        //             If supported, re-enter scheduler until idle or not blocked or done
        //             Release meta thread (put in high demand idle list to increase chance of re-acquiring)
        //             Yield until not blocked or done
        //             Wait for same meta thread to become available (actually.. must be able to migrate meta thread at this point or might starve)
        //         If only 1 scheduler active
        //             Run scheduler until done or meta thread required by other blocked threads
        //     Release meta thread
        //     Yield until not-idle or done
        //     Attempt to re-acquire same meta thread
        //         If successful
        //             Goto 1:
        //         Else
        //             Notify schedulers of meta thread move

        /*
        volatile bool done = false;
        auto doneWaiter = MakeWaiter([&] {done = true;});
        until.AddWaiter(&doneWaiter);
        while (!done)
        {
        }
        */
    }

    CRUNCH_ALWAYS_INLINE void Release()
    {
        if (--mRefCount == 0)
        {
            delete this;
            MetaScheduler::tCurrentContext = nullptr;
        }
    }

    CRUNCH_ALWAYS_INLINE void WaitFor(IWaitable& waitable, WaitMode waitMode)
    {
        // TODO: Let active scheduler handle the wait if it can
        // TODO: Keep in mind if active scheduler is a fiber scheduler we might come back on a different system thread.. (and this thread might be used for other things.. i.e. waiter must be stack local)
        if (waitable.AddWaiter(mWaiter))
            mWaitSemaphore.SpinWait(waitMode.spinCount);
    }

    CRUNCH_ALWAYS_INLINE void WaitForAll(IWaitable** waitables, std::size_t count, WaitMode waitMode)
    {
        IWaitable** unordered = CRUNCH_STACK_ALLOC_T(IWaitable*, count);
        IWaitable** ordered = CRUNCH_STACK_ALLOC_T(IWaitable*, count);
        std::size_t orderedCount = 0;
        std::size_t unorderedCount = 0;

        for (std::size_t i = 0; i < count; ++i)
        {
            if (waitables[i]->IsOrderDependent())
                ordered[orderedCount++] = waitables[i];
            else
                unordered[unorderedCount++] = waitables[i];
        }

        if (orderedCount != 0)
        {
            std::sort(ordered, ordered + orderedCount);

            // Order dependent doesn't imply fair, so we need to wait for one at a time
            for (std::size_t i = 0; i < orderedCount; ++i)
                WaitFor(*ordered[i], waitMode);
        }

        if (unorderedCount != 0)
        {
            std::size_t addedCount = 0;
            for (std::size_t i = 0; i < unorderedCount; ++i)
                if (waitables[i]->AddWaiter([&] { mWaitSemaphore.Post(); }))
                    addedCount++;

            for (std::size_t i = 0; i < addedCount; ++i)
                mWaitSemaphore.SpinWait(waitMode.spinCount);
        }
    }

    WaitForAnyResult WaitForAny(IWaitable** waitables, std::size_t count, WaitMode waitMode)
    {
        auto poster = [&] { mWaitSemaphore.Post(); };
        typedef Waiter::Typed<decltype(poster)> WaiterType;

        WaiterType** waiters = CRUNCH_STACK_ALLOC_T(WaiterType*, count);

        WaitForAnyResult signaled;

        std::size_t addedCount = 0;
        for (std::size_t i = 0; i < count; ++i)
        {
            waiters[i] = Waiter::Create(poster, false);
            if (!waitables[i]->AddWaiter(waiters[i]))
            {
                waiters[i]->Destroy();
                signaled.push_back(waitables[i]);
                break;
            }
            addedCount++;
        }

        // If no waitable synchronously ready, wait for one to become ready
        if (addedCount == count)
            mWaitSemaphore.SpinWait(waitMode.spinCount);

        // Try to remove waiters for all waitables
        for (std::size_t i = 0; i < addedCount; ++i)
        {
            if (!waitables[i]->RemoveWaiter(waiters[i]))
                signaled.push_back(waitables[i]);
        }

        // Wait for any waiters that weren't removed. They should be in-flight callbacks.
        // We've already waited for 1 either in SpinWait or by synchronous ready, so start count from 1
        CRUNCH_ASSERT(signaled.size() >= 1);
        for (std::size_t i = 1; i < signaled.size(); ++i)
            mWaitSemaphore.SpinWait(waitMode.spinCount);

        // Destroy all waiters
        // TODO: Could bulk free. Callback shouldn't require destruction, so only list of waiters need to be reclaimed.
        for (std::size_t i = 0; i < addedCount; ++i)
            waiters[i]->Destroy();

        return signaled;
    }

private:
    MetaScheduler* mOwner;
    std::uint32_t mRefCount;
    Detail::SystemSemaphore mWaitSemaphore;
    Waiter* mWaiter;
    std::function<void ()> mWaiterDestroyer;
};

void MetaScheduler::Context::Run(IWaitable& until)
{
    static_cast<ContextImpl*>(this)->Run(until);
}

void MetaScheduler::Context::Release()
{
    static_cast<ContextImpl*>(this)->Release();
}

CRUNCH_THREAD_LOCAL MetaScheduler::ContextImpl* MetaScheduler::tCurrentContext = NULL;

class MetaScheduler::MetaThread
{
public:
    MetaThread(MetaThreadConfig const& config)
        : mConfig(config)
    {}

private:
    MetaThreadConfig mConfig;
};

MetaScheduler::MetaScheduler(const Configuration& configuration)
    : mConfiguration(configuration)
{}

MetaScheduler::~MetaScheduler()
{}

MetaScheduler::MetaThreadHandle MetaScheduler::CreateMetaThread(MetaThreadConfig const& config)
{
    Detail::SystemMutex::ScopedLock lock(mMetaThreadsLock);

    MetaThreadPtr mt(new MetaThread(config));
    mIdleMetaThreads.push_back(std::move(mt));
    return MetaThreadHandle();
}

MetaScheduler::Context& MetaScheduler::AcquireContext()
{
    if (tCurrentContext == nullptr)
    {
        tCurrentContext = new ContextImpl(this);
    }
    else
    {
        CRUNCH_ASSERT_MSG(tCurrentContext->GetOwner() == this, "Context belongs to different MetaScheduler instance");
        tCurrentContext->AddRef();
    }

    return *tCurrentContext;
}

void WaitFor(IWaitable& waitable, WaitMode waitMode)
{
    CRUNCH_ASSERT_MSG(MetaScheduler::tCurrentContext != nullptr, "No context on current thread");
    MetaScheduler::tCurrentContext->WaitFor(waitable, waitMode);
}

void WaitForAll(IWaitable** waitables, std::size_t count, WaitMode waitMode)
{
    CRUNCH_ASSERT_MSG(MetaScheduler::tCurrentContext != nullptr, "No context on current thread");
    MetaScheduler::tCurrentContext->WaitForAll(waitables, count, waitMode);
}

WaitForAnyResult WaitForAny(IWaitable** waitables, std::size_t count, WaitMode waitMode)
{
    CRUNCH_ASSERT_MSG(MetaScheduler::tCurrentContext != nullptr, "No context on current thread");
    return MetaScheduler::tCurrentContext->WaitForAny(waitables, count, waitMode);
}

}}
