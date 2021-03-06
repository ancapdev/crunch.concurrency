// Copyright (c) 2011, Christian Rorvik
// Distributed under the Simplified BSD License (See accompanying file LICENSE.txt)

#include "crunch/concurrency/event.hpp"
#include "crunch/base/override.hpp"
#include "crunch/test/framework.hpp"

#include <cstdint>

namespace Crunch { namespace Concurrency {

BOOST_AUTO_TEST_SUITE(EventTests)

BOOST_AUTO_TEST_CASE(ConstructSetTest)
{
    Event e(true);
    BOOST_CHECK(e.IsSet());
    BOOST_CHECK(!e.IsOrderDependent());
}

BOOST_AUTO_TEST_CASE(ConstructUnsetTest)
{
    Event e(false);
    BOOST_CHECK(!e.IsSet());
    BOOST_CHECK(!e.IsOrderDependent());
}

BOOST_AUTO_TEST_CASE(StateChangeTest)
{
    Event e(true);
    BOOST_CHECK(e.IsSet());

    e.Reset();
    BOOST_CHECK(!e.IsSet());

    e.Set();
    BOOST_CHECK(e.IsSet());
}

BOOST_AUTO_TEST_CASE(AddWaiterToSetTest)
{
    Event e(true);
    volatile std::uint32_t wakeupCount = 0;

    // Add waiter to set event. Should return false and not execute callback.
    BOOST_CHECK(!e.AddWaiter([&] { wakeupCount++; }));
    BOOST_CHECK_EQUAL(wakeupCount, 0u);
}

BOOST_AUTO_TEST_CASE(AddWaiterToUnsetTest)
{
    Event e(false);
    volatile std::uint32_t wakeupCount = 0;

    auto waiter = Waiter::Create([&] { wakeupCount++; }, false);

    // Add waiter to unset event.
    BOOST_CHECK(e.AddWaiter(waiter));
    BOOST_CHECK_EQUAL(wakeupCount, 0u);

    // Remove waiter and set event.
    BOOST_CHECK(e.RemoveWaiter(waiter));
    e.Set();
    BOOST_CHECK_EQUAL(wakeupCount, 0u);

    // Add waiter and set event.
    e.Reset();
    BOOST_CHECK(e.AddWaiter(waiter));
    e.Set();
    BOOST_CHECK_EQUAL(wakeupCount, 1u);

    waiter->Destroy();
}

BOOST_AUTO_TEST_SUITE_END()

}}
