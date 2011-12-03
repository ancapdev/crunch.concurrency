// Copyright (c) 2011, Christian Rorvik
// Distributed under the Simplified BSD License (See accompanying file LICENSE.txt)

#include "crunch/base/override.hpp"
#include "crunch/concurrency/event.hpp"
#include "crunch/concurrency/meta_scheduler.hpp"
#include "crunch/concurrency/thread.hpp"
#include "crunch/concurrency/yield.hpp"
#include "crunch/test/framework.hpp"

#include <stdexcept>

namespace Crunch { namespace Concurrency {

BOOST_AUTO_TEST_SUITE(MetaSchedulerTests)

BOOST_AUTO_TEST_CASE(WaitTest)
{
    struct NullWaitable : IWaitable
    {
        virtual bool AddWaiter(Waiter*) { return false; }
        virtual bool RemoveWaiter(Waiter*) { return false; }
        virtual bool IsOrderDependent() const { return false; }
    };

    MetaScheduler::Config configuration;
    MetaScheduler ms(configuration);
    MetaScheduler::Context& context = ms.AcquireContext();

    NullWaitable nullWaitable;
    IWaitable* waitables[] = { &nullWaitable };

    WaitFor(nullWaitable, WaitMode::Block());
    WaitForAll(waitables, 1, WaitMode::Block());
    WaitForAny(waitables, 1, WaitMode::Block());

    context.Release();
}

BOOST_AUTO_TEST_CASE(RunTest)
{
    struct TestScheduler : IScheduler
    {
        struct Context : ISchedulerContext
        {
            virtual State Run(IThrottler&) CRUNCH_OVERRIDE
            {
                return State::Idle;
            }

            virtual bool CanReEnter() CRUNCH_OVERRIDE
            {
                return false;
            }

            virtual IWaitable& GetHasWorkCondition() CRUNCH_OVERRIDE
            {
                return mHasWork;
            }

            Event mHasWork;
        };

        virtual bool CanOrphan() CRUNCH_OVERRIDE
        {
            return false;
        }

        virtual ISchedulerContext& GetContext() CRUNCH_OVERRIDE
        {
            return mContext;
        }

        Context mContext;
    };

    MetaScheduler::Config config;
    config.AddScheduler(MetaScheduler::SchedulerPtr(new TestScheduler()), 0, RunMode::All());

    MetaScheduler ms(config);

    MetaScheduler::MetaThreadConfig mtConfig;
    mtConfig.SetProcessorAffinity(ProcessorAffinity(0));
    MetaScheduler::MetaThreadHandle mtHandle = ms.CreateMetaThread(mtConfig);
    (void)mtHandle;

    Event doneEvent;
    Thread expireThread([&]
    {
        ThreadSleep(Duration::Seconds(2));
        doneEvent.Set();
    });

    MetaScheduler::Context& msContext = ms.AcquireContext();
    msContext.Run(doneEvent);
    msContext.Release();
    expireThread.Join();
}

BOOST_AUTO_TEST_SUITE_END()

}}
