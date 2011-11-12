// Copyright (c) 2011, Christian Rorvik
// Distributed under the Simplified BSD License (See accompanying file LICENSE.txt)

#include "crunch/concurrency/semaphore.hpp"
#include "crunch/test/framework.hpp"

namespace Crunch { namespace Concurrency {

BOOST_AUTO_TEST_SUITE(SemaphoreTests)

BOOST_AUTO_TEST_CASE(InitialReadyThenWaitTest)
{
    Semaphore s(1);
    volatile bool acquired = false;
    s.AddWaiter([&] { acquired = true; });
    BOOST_CHECK(acquired);
}

BOOST_AUTO_TEST_CASE(WaitThenPostTest)
{
    Semaphore s(0);
    volatile bool acquired = false;
    s.AddWaiter([&] { acquired = true; });
    BOOST_CHECK(!acquired);
    s.Post();
    BOOST_CHECK(acquired);
}

BOOST_AUTO_TEST_CASE(PostThenWaitTest)
{
    Semaphore s(0);
    volatile bool acquired = false;
    s.Post();
    s.AddWaiter([&] { acquired = true; });
    BOOST_CHECK(acquired);
}

BOOST_AUTO_TEST_CASE(RemoveWaitFromEmptyTest)
{
    Semaphore s(0);
    auto waiter = Waiter::Create([]{}, false);
    BOOST_CHECK(!s.RemoveWaiter(waiter));
    waiter->Destroy();
}

BOOST_AUTO_TEST_CASE(RemoveWaiterTest)
{
    Semaphore s(0);
    auto waiter = Waiter::Create([]{}, false);
    s.AddWaiter(waiter);
    BOOST_CHECK(s.RemoveWaiter(waiter));
    waiter->Destroy();
}

BOOST_AUTO_TEST_SUITE_END()

}}
