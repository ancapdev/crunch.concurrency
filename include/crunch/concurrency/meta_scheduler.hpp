// Copyright (c) 2011, Christian Rorvik
// Distributed under the Simplified BSD License (See accompanying file LICENSE.txt)

#ifndef CRUNCH_CONCURRENCY_META_SCHEDULER_HPP
#define CRUNCH_CONCURRENCY_META_SCHEDULER_HPP

#include "crunch/base/stdint.hpp"
#include "crunch/concurrency/processor_affinity.hpp"
#include "crunch/concurrency/scheduler.hpp"
#include "crunch/concurrency/thread_local.hpp"
#include "crunch/concurrency/waitable.hpp"
#include "crunch/concurrency/detail/system_mutex.hpp"

#include <memory>
#include <vector>

namespace Crunch { namespace Concurrency {

// Very simple scheduler to manage processing resources and run multiple cooperative schedulers. E.g.,
// - task scheduler
// - io_service scheduler
// - fiber / coroutine scheduler
// TODO: Add suppor for threads that haven't joined the scheduler to call WaitForXXX
class MetaScheduler
{
public:
    class RunMode
    {
    public:
        static RunMode Disabled();
        static RunMode Some(uint32 count);
        static RunMode TimedMicroseconds(uint32 count);
        static RunMode All();

    private:
        enum Type
        {
            TYPE_DISABLED,
            TYPE_SOME,
            TYPE_TIMED,
            TYPE_ALL
        };

        RunMode(Type type, uint32 parameter);

        Type mType;
        uint32 mParameter;
    };

    typedef std::shared_ptr<IScheduler> SchedulerPtr;
    typedef std::vector<SchedulerPtr> SchedulerList;

    class Configuration
    {
    public:
        void AddGroup(uint32 id, RunMode defaultRunMode);
        void AddScheduler(uint32 groupId, SchedulerPtr const& scheduler);

    private:
        struct Group
        {
            Group(uint32 id, RunMode defaultRunMode);

            uint32 id;
            RunMode defaultRunMode;
            std::vector<SchedulerPtr> schedulers;
        };

        std::vector<Group> mGroups;
    };

    MetaScheduler(Configuration const& configuration);
    ~MetaScheduler();

    class MetaThreadConfig
    {
    public:
        MetaThreadConfig() : mSchedulerAffinity(0xfffffffful) {}

        void SetSchedulerAffinity(uint32 affinity) { mSchedulerAffinity = affinity; }
        uint32 GetSchedulerAffinity() const { return mSchedulerAffinity; }

        void SetProcessorAffinity(ProcessorAffinity const& affinity) { mProcessorAffinity = affinity; }
        ProcessorAffinity const& GetProcessorAffinity() const { return mProcessorAffinity; }

    private:
        uint32 mSchedulerAffinity; ///> Determines which schedulers run on this meta thread
        ProcessorAffinity mProcessorAffinity;
    };

    class MetaThreadHandle
    {
    };

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

    class MetaThread;
    typedef std::unique_ptr<MetaThread> MetaThreadPtr;
    typedef std::vector<MetaThreadPtr> MetaThreadList;

    class ContextImpl;
    typedef std::unique_ptr<ContextImpl> ContextPtr;
    typedef std::vector<ContextPtr> ContextList;

    Configuration mConfiguration;

    MetaThreadList mIdleMetaThreads;
    Detail::SystemMutex mMetaThreadsLock;

    ContextList mContexts;
    Detail::SystemMutex mContextsLock;

    static CRUNCH_THREAD_LOCAL ContextImpl* tCurrentContext;
};

}}

#endif
