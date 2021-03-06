// Copyright (c) 2011, Christian Rorvik
// Distributed under the Simplified BSD License (See accompanying file LICENSE.txt)

#include "crunch/concurrency/meta_scheduler.hpp"

#include "crunch/base/assert.hpp"
#include "crunch/base/high_frequency_timer.hpp"
#include "crunch/base/inline.hpp"
#include "crunch/base/noncopyable.hpp"
#include "crunch/base/override.hpp"
#include "crunch/base/stack_alloc.hpp"
#include "crunch/concurrency/event.hpp"
#include "crunch/concurrency/yield.hpp"
#include "crunch/concurrency/detail/system_semaphore.hpp"

#include <algorithm>
#include <stdexcept>

namespace Crunch { namespace Concurrency {

namespace
{
    void WaitForImpl(Detail::SystemSemaphore& waitSemaphore, IWaitable& waitable, WaitMode waitMode)
    {
        if (waitable.AddWaiter([&] { waitSemaphore.Post(); }))
            waitSemaphore.SpinWait(waitMode.spinCount);
    }

    void WaitForAllImpl(Detail::SystemSemaphore& waitSemaphore, IWaitable** waitables, std::size_t count, WaitMode waitMode)
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
                if (waitables[i]->AddWaiter([&] { waitSemaphore.Post(); }))
                    addedCount++;

            for (std::size_t i = 0; i < addedCount; ++i)
                waitSemaphore.SpinWait(waitMode.spinCount);
        }
    }

    WaitForAnyResult WaitForAnyImpl(Detail::SystemSemaphore& waitSemaphore, IWaitable** waitables, std::size_t count, WaitMode waitMode)
    {
        auto poster = [&] { waitSemaphore.Post(); };
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
            waitSemaphore.SpinWait(waitMode.spinCount);

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
            waitSemaphore.SpinWait(waitMode.spinCount);

        // Destroy all waiters
        // TODO: Could bulk free. Callback shouldn't require destruction, so only list of waiters need to be reclaimed.
        for (std::size_t i = 0; i < addedCount; ++i)
            waiters[i]->Destroy();

        return signaled;
    }
}

RunMode::RunMode(Type type, std::uint32_t count, Duration duration)
    : mType(type)
    , mCount(count)
    , mDuration(duration)
{}

RunMode RunMode::Disabled()
{
    return RunMode(TYPE_DISABLED, 0, Duration::Zero);
}

RunMode RunMode::Some(std::uint32_t count)
{
    return RunMode(TYPE_SOME, count, Duration::Zero);
}

RunMode RunMode::Timed(Duration duration)
{
    return RunMode(TYPE_TIMED, 0, duration);
}

RunMode RunMode::All()
{
    return RunMode(TYPE_ALL, 0, Duration::Zero);
}

struct MetaScheduler::MetaThread
{
    ProcessorAffinity processorAffinity;
    std::map<std::uint32_t, RunMode> runModeOverrides;
};

MetaScheduler::SchedulerInfo::SchedulerInfo(SchedulerPtr const& scheduler, std::uint32_t id, RunMode defaultRunMode)
    : scheduler(scheduler)
    , id(id)
    , defaultRunMode(defaultRunMode)
{}

void MetaScheduler::Config::AddScheduler(SchedulerPtr const& scheduler, std::uint32_t id, RunMode defaultRunMode)
{
    auto it = std::find_if(mSchedulers.begin(), mSchedulers.end(), [=] (SchedulerInfo const& info) { return info.id == id; });
    CRUNCH_ASSERT_MSG_ALWAYS(it == mSchedulers.end(), "Scheduler with ID=%d already added", id);
    mSchedulers.push_back(SchedulerInfo(scheduler, id, defaultRunMode));
}

class MetaScheduler::ContextImpl : public MetaScheduler::Context, NonCopyable
{
public:
    ContextImpl(MetaScheduler& owner)
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

    MetaScheduler& GetOwner()
    {
        return mOwner;
    }

    void AddRef()
    {
        mRefCount++;
    }

    MetaThreadPtr AcquireMetaThread(IWaitable& interrupt)
    {
        volatile bool stop = false;
        Detail::SystemSemaphore waiterDone(0);

        auto waiter = Waiter::Create([&]
        {
            stop = true;
            mOwner.mIdleMetaThreadAvailable.WakeAll();
            waiterDone.Post();
        }, false);

        MetaThreadPtr metaThread;

        if (interrupt.AddWaiter(waiter))
        {
            {
                Detail::SystemMutex::ScopedLock const lock(mOwner.mIdleMetaThreadsLock);
                while (!stop && mOwner.mIdleMetaThreads.empty())
                    mOwner.mIdleMetaThreadAvailable.Wait(mOwner.mIdleMetaThreadsLock);

                if (!mOwner.mIdleMetaThreads.empty())
                {
                    metaThread = std::move(mOwner.mIdleMetaThreads.back());
                    mOwner.mIdleMetaThreads.pop_back();
                }
            }

            if (!interrupt.RemoveWaiter(waiter))
                waiterDone.Wait();
        }

        waiter->Destroy();
        return metaThread;
    }

    void ReleaseMetaThread(MetaThreadPtr&& metaThread)
    {
        Detail::SystemMutex::ScopedLock const lock(mOwner.mIdleMetaThreadsLock);
        mOwner.mIdleMetaThreads.push_back(std::move(metaThread));
        mOwner.mIdleMetaThreadAvailable.WakeOne();
    }

    struct SchedulerState : NonCopyable
    {
        typedef ISchedulerContext::State (*RunFunction)(ISchedulerContext&, RunMode runMode, volatile bool& stop);

        static ISchedulerContext::State RunAll(ISchedulerContext& schedulerContext, RunMode, volatile bool& stop)
        {
            struct Throttler : IThrottler, NonCopyable
            {
                Throttler(volatile bool& stop) : mStop(stop) {}

                virtual bool ShouldYield() CRUNCH_OVERRIDE
                {
                    return mStop;
                }

                volatile bool& mStop;
            };

            Throttler throttler(stop);
            return schedulerContext.Run(throttler);
        }

        static ISchedulerContext::State RunSome(ISchedulerContext& schedulerContext, RunMode runMode, volatile bool& stop)
        {
            struct Throttler : IThrottler, NonCopyable
            {
                Throttler(volatile bool& stop, std::uint32_t count) : mStop(stop), mCount(count) {}

                virtual bool ShouldYield() CRUNCH_OVERRIDE
                { 
                    if (mStop || mCount == 0)
                        return true;

                    mCount--;
                    return false;
                }

                volatile bool& mStop;
                std::uint32_t mCount;
            };

            Throttler throttler(stop, runMode.mCount);
            return schedulerContext.Run(throttler);
        }

        static ISchedulerContext::State RunTimed(ISchedulerContext& schedulerContext, RunMode runMode, volatile bool& stop)
        {
            struct Throttler : IThrottler, NonCopyable
            {
                Throttler(volatile bool& stop, Duration duration) : mStop(stop), mStart(mTimer.Sample()), mMaxDuration(duration) {}

                virtual bool ShouldYield() CRUNCH_OVERRIDE
                {
                    return mStop || mTimer.GetElapsedTime(mStart, mTimer.Sample()) > mMaxDuration;
                }

                volatile bool& mStop;
                HighFrequencyTimer mTimer;
                HighFrequencyTimer::SampleType mStart;
                Duration mMaxDuration;
            };

            Throttler throttler(stop, runMode.mDuration);
            return schedulerContext.Run(throttler);
        }

        static RunFunction RunFunctionFromRunMode(RunMode runMode)
        {
            switch (runMode.mType)
            {
            case RunMode::TYPE_ALL: return &SchedulerState::RunAll;
            case RunMode::TYPE_SOME: return &SchedulerState::RunSome;
            case RunMode::TYPE_TIMED: return &SchedulerState::RunTimed;
            default:
                throw std::runtime_error("Invalid run mode");
            }
        }

        SchedulerState(ISchedulerContext* context, RunMode runMode, std::function<void ()> const& notifyReady)
            : context(context)
            , lastState(ISchedulerContext::State::Working)
            , hasWorkCondition(&context->GetHasWorkCondition())
            , runMode(runMode)
            , runner(RunFunctionFromRunMode(runMode))
            , notifyReady(notifyReady)
        {
            hasWorkWaiter = Waiter::Create(notifyReady, false);
        }

        ~SchedulerState()
        {
            hasWorkWaiter->Destroy();
        }

        ISchedulerContext* context;
        volatile ISchedulerContext::State lastState;
        IWaitable* hasWorkCondition;
        Waiter::Typed<std::function<void ()>>* hasWorkWaiter;
        RunMode runMode;
        RunFunction runner;
        std::function<void ()> notifyReady;
    };

    void Run(IWaitable& until)
    {
        MetaThreadPtr metaThread = AcquireMetaThread(until);
        if (!metaThread)
            return;

        Detail::SystemMutex stateLock;
        Detail::SystemCondition stateChanged;
        volatile bool stop = false;
        volatile std::size_t activeCount = 0;

        if (!until.AddWaiter([&]
        {
            Detail::SystemMutex::ScopedLock const lock(stateLock);
            stop = true;
            if (activeCount == 0)
                stateChanged.WakeAll();
        }))
        {
            return;
        }

        ProcessorAffinity oldAffinity;
        if (!metaThread->processorAffinity.IsEmpty())
            oldAffinity = SetCurrentThreadAffinity(metaThread->processorAffinity);

        std::vector<std::unique_ptr<SchedulerState>> schedulers;
        for (auto it = mOwner.mSchedulers.begin(); it != mOwner.mSchedulers.end(); ++it)
        {
            auto const runModeOverrideIt = metaThread->runModeOverrides.find(it->id);
            RunMode const runMode = runModeOverrideIt != metaThread->runModeOverrides.end() ? runModeOverrideIt->second : it->defaultRunMode;
            if (runMode.mType != runMode.TYPE_DISABLED)
            {
                std::size_t index = schedulers.size();
                schedulers.push_back(std::unique_ptr<SchedulerState>(new SchedulerState(&it->scheduler->GetContext(), runMode, [&, index]
                {
                    Detail::SystemMutex::ScopedLock const lock(stateLock);
                    if (activeCount++ == 0)
                        stateChanged.WakeAll();

                    schedulers[index]->lastState = ISchedulerContext::State::Working;
                })));
            }
        }

        std::size_t pollingCount = 0;
        activeCount = schedulers.size();
        std::vector<SchedulerState*> activeSchedulers;
        std::vector<SchedulerState*> idleSchedulers;

        // Start with all schedulers active
        std::for_each(schedulers.begin(), schedulers.end(), [&] (std::unique_ptr<SchedulerState> const& schedulerState)
        {
            activeSchedulers.push_back(schedulerState.get());
        });

        for (;;)
        {
            if (stop)
                goto stopped;

            if (activeCount != activeSchedulers.size())
            {
                // One of the idle schedulers has been signaled ready
                Detail::SystemMutex::ScopedLock const lock(stateLock);
                for (auto it = idleSchedulers.begin(); it != idleSchedulers.end();)
                {
                    if ((*it)->lastState != ISchedulerContext::State::Idle)
                    {
                        activeSchedulers.push_back(*it);
                        it = idleSchedulers.erase(it);
                    }
                    else
                    {
                        ++it;
                    }
                }
                activeCount = activeSchedulers.size();
            }

            for (auto it = activeSchedulers.begin(); it != activeSchedulers.end();)
            {
                if (stop)
                    goto stopped;

                SchedulerState* ss = *it;
                ISchedulerContext::State const state = ss->runner(*ss->context, ss->runMode, stop);

                if (state == ISchedulerContext::State::Idle)
                {
                    if (ss->lastState == ISchedulerContext::State::Polling)
                        pollingCount--;

                    Detail::SystemMutex::ScopedLock const lock(stateLock);
                    if (ss->hasWorkCondition->AddWaiter(ss->hasWorkWaiter))
                    {
                        ss->lastState = ISchedulerContext::State::Idle;
                        idleSchedulers.push_back(ss);
                        it = activeSchedulers.erase(it);
                        activeCount--;
                    }
                    else
                    {
                        ss->lastState = ISchedulerContext::State::Working;
                        ++it;
                    }
                }
                else
                {
                    if (state != ss->lastState)
                    {
                        if (state == ISchedulerContext::State::Polling)
                            pollingCount++;
                        else
                            pollingCount--;

                        ss->lastState = state;
                    }
                    ++it;
                }
            }

            if (activeSchedulers.empty())
            {
                // No active schedulers, go idle.
                // TODO: IWaitable until must also notify state changed
                Detail::SystemMutex::ScopedLock const lock(stateLock);
                while (activeCount == 0)
                {
                    if (stop)
                        goto stopped;

                    stateChanged.Wait(stateLock);
                }
            }

            if (activeSchedulers.size() == pollingCount)
            {
                // All active schedulers are busy polling for work. Yield resources.
                Pause(100);
                ThreadYield();
            }
        }

stopped:
        // TODO: remove all waiters on idle schedulers
        {
            Detail::SystemMutex::ScopedLock lock(stateLock);
            std::for_each(idleSchedulers.begin(), idleSchedulers.end(), [&] (SchedulerState* ss)
            {
                if (!ss->hasWorkCondition->RemoveWaiter(ss->hasWorkWaiter))
                {
                    // Failed to remove waiter. Must wait until callback has finished
                    while (ss->lastState == ISchedulerContext::State::Idle)
                        stateChanged.Wait(stateLock);
                }
            });
        }

        if (!oldAffinity.IsEmpty())
            SetCurrentThreadAffinity(oldAffinity);

        ReleaseMetaThread(std::move(metaThread));
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
        WaitForAllImpl(mWaitSemaphore, waitables, count, waitMode);
    }

    WaitForAnyResult WaitForAny(IWaitable** waitables, std::size_t count, WaitMode waitMode)
    {
        return WaitForAnyImpl(mWaitSemaphore, waitables, count, waitMode);
    }

private:
    MetaScheduler& mOwner;
    std::uint32_t mRefCount;
    Detail::SystemSemaphore mWaitSemaphore;
    Waiter* mWaiter;
    std::function<void ()> mWaiterDestroyer;

    // Detail::SystemMutex mStateLock;
    // Detail::SystemSemaphore mStateChanged;
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

MetaScheduler::MetaScheduler(const Config& config)
    : mSchedulers(config.mSchedulers)
{}

MetaScheduler::~MetaScheduler()
{}

MetaScheduler::MetaThreadHandle MetaScheduler::CreateMetaThread(MetaThreadConfig const& config)
{
    // TODO: signal any threads waiting for idle meta threads

    Detail::SystemMutex::ScopedLock lock(mIdleMetaThreadsLock);
    MetaThreadPtr mt(new MetaThread());
    mt->processorAffinity = config.mProcessorAffinity;
    mt->runModeOverrides = config.mRunModeOverrides;
    mIdleMetaThreads.push_back(std::move(mt));
    return MetaThreadHandle();
}

MetaScheduler::Context& MetaScheduler::AcquireContext()
{
    if (tCurrentContext == nullptr)
    {
        tCurrentContext = new ContextImpl(*this);
    }
    else
    {
        CRUNCH_ASSERT_MSG(&tCurrentContext->GetOwner() == this, "Context belongs to different MetaScheduler instance");
        tCurrentContext->AddRef();
    }

    return *tCurrentContext;
}

void WaitFor(IWaitable& waitable, WaitMode waitMode)
{
    if (MetaScheduler::tCurrentContext)
    {
        MetaScheduler::tCurrentContext->WaitFor(waitable, waitMode);
    }
    else
    {
        Detail::SystemSemaphore waitSemaphore(0);
        WaitForImpl(waitSemaphore, waitable, waitMode);
    }
}

void WaitForAll(IWaitable** waitables, std::size_t count, WaitMode waitMode)
{
    if (MetaScheduler::tCurrentContext)
    {
        MetaScheduler::tCurrentContext->WaitForAll(waitables, count, waitMode);
    }
    else
    {
        Detail::SystemSemaphore waitSemaphore(0);
        WaitForAllImpl(waitSemaphore, waitables, count, waitMode);
    }
}

WaitForAnyResult WaitForAny(IWaitable** waitables, std::size_t count, WaitMode waitMode)
{
    if (MetaScheduler::tCurrentContext)
    {
        return MetaScheduler::tCurrentContext->WaitForAny(waitables, count, waitMode);
    }
    else
    {
        Detail::SystemSemaphore waitSemaphore(0);
        return WaitForAnyImpl(waitSemaphore, waitables, count, waitMode);
    }
}

}}
