// Copyright (c) 2011, Christian Rorvik
// Distributed under the Simplified BSD License (See accompanying file LICENSE.txt)

#include "crunch/concurrency/meta_scheduler.hpp"
#include "crunch/test/framework.hpp"

namespace Crunch { namespace Concurrency {

BOOST_AUTO_TEST_SUITE(MetaSchedulerTests)

BOOST_AUTO_TEST_CASE(WaitTest)
{
    struct NullWaitable : IWaitable
    {
        virtual void AddWaiter(Waiter* waiter) { waiter->Notify(); }
        virtual bool RemoveWaiter(Waiter*) { return false; }
        virtual bool IsOrderDependent() const { return false; }
    };

    MetaScheduler::Configuration const configuration;
    MetaScheduler ms(configuration);
    MetaScheduler::Context& context = ms.AcquireContext();

    NullWaitable nullWaitable;
    IWaitable* waitables[] = { &nullWaitable };

    WaitFor(nullWaitable, WaitMode::Block());
    WaitForAll(waitables, 1, WaitMode::Block());
    WaitForAny(waitables, 1, WaitMode::Block());

    context.Release();
}


BOOST_AUTO_TEST_SUITE_END()

}}
