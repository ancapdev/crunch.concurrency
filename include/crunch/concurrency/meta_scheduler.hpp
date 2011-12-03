// Copyright (c) 2011, Christian Rorvik
// Distributed under the Simplified BSD License (See accompanying file LICENSE.txt)

#ifndef CRUNCH_CONCURRENCY_META_SCHEDULER_HPP
#define CRUNCH_CONCURRENCY_META_SCHEDULER_HPP

#include "crunch/base/duration.hpp"
#include "crunch/concurrency/processor_affinity.hpp"
#include "crunch/concurrency/scheduler.hpp"
#include "crunch/concurrency/thread_local.hpp"
#include "crunch/concurrency/waitable.hpp"
#include "crunch/concurrency/detail/system_condition.hpp"
#include "crunch/concurrency/detail/system_mutex.hpp"

#include <cstdint>
#include <map>
#include <memory>
#include <vector>

namespace Crunch { namespace Concurrency {

class RunMode
{
public:
    static RunMode Disabled();
    static RunMode Some(std::uint32_t count);
    static RunMode Timed(Duration duration);
    static RunMode All();

private:
    // TODO: remove and create proper API
    friend class MetaScheduler;

    enum Type
    {
        TYPE_DISABLED,
        TYPE_SOME,
        TYPE_TIMED,
        TYPE_ALL
    };

    RunMode(Type type, std::uint32_t count, Duration duration);

    Type mType;
    std::uint32_t mCount;
    Duration mDuration;
};

// Very simple scheduler to manage processing resources and run multiple cooperative schedulers. E.g.,
// - task scheduler
// - io_service scheduler
// - fiber / coroutine scheduler
// TODO: Add support for threads that haven't joined the scheduler to call WaitForXXX
class MetaScheduler
{
public:
    class Config;
    class MetaThreadConfig;
    class MetaThreadHandle {};
    typedef std::shared_ptr<IScheduler> SchedulerPtr;

    MetaScheduler(Config const& config);
    ~MetaScheduler();

    MetaThreadHandle CreateMetaThread(MetaThreadConfig const& config);

    class Context
    {
    public:
        void Run(IWaitable& until);
        void Release();
    };

    Context& AcquireContext();

private:
    friend void WaitFor(IWaitable&, WaitMode);
    friend void WaitForAll(IWaitable**, std::size_t, WaitMode);
    friend WaitForAnyResult WaitForAny(IWaitable**, std::size_t, WaitMode);

    friend class Config;

    struct MetaThread;
    typedef std::unique_ptr<MetaThread> MetaThreadPtr;
    typedef std::vector<MetaThreadPtr> MetaThreadList;

    class ContextImpl;
    typedef std::unique_ptr<ContextImpl> ContextPtr;
    typedef std::vector<ContextPtr> ContextList;

    Detail::SystemMutex mIdleMetaThreadsLock;
    MetaThreadList mIdleMetaThreads;
    Detail::SystemCondition mIdleMetaThreadAvailable;

    Detail::SystemMutex mContextsLock;
    ContextList mContexts;

    struct SchedulerInfo
    {
        SchedulerInfo(SchedulerPtr const& scheduler, std::uint32_t id, RunMode defaultRunMode);

        SchedulerPtr scheduler;
        std::uint32_t id;
        RunMode defaultRunMode;
    };

    typedef std::vector<SchedulerInfo> SchedulerInfoList;
    SchedulerInfoList mSchedulers;

    static CRUNCH_THREAD_LOCAL ContextImpl* tCurrentContext;
};


class MetaScheduler::Config
{
public:
    void AddScheduler(SchedulerPtr const& scheduler, std::uint32_t id, RunMode defaultRunMode);

private:
    friend class MetaScheduler;

    std::vector<SchedulerInfo> mSchedulers;
};

class MetaScheduler::MetaThreadConfig
{
public:
    void SetRunModeOverride(std::uint32_t schedulerId, RunMode runMode);
    void SetProcessorAffinity(ProcessorAffinity const& affinity) { mProcessorAffinity = affinity; }

private:
    friend class MetaScheduler;

    ProcessorAffinity mProcessorAffinity;
    std::map<std::uint32_t, RunMode> mRunModeOverrides;
};

}}

#endif
