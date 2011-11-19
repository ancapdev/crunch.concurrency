// Copyright (c) 2011, Christian Rorvik
// Distributed under the Simplified BSD License (See accompanying file LICENSE.txt)

#include "crunch/concurrency/mutex.hpp"
#include "crunch/test/framework.hpp"

namespace Crunch { namespace Concurrency {

BOOST_AUTO_TEST_SUITE(MutexTests)

BOOST_AUTO_TEST_CASE(ConstructTest)
{
    Mutex m;
    BOOST_CHECK(!m.IsLocked());
}

BOOST_AUTO_TEST_CASE(AddWaiterToUnlockedTest)
{
    Mutex m;
    volatile bool called = false;
    BOOST_CHECK(!m.AddWaiter([&] { called = true; }));
    BOOST_CHECK(!called);
    BOOST_CHECK(m.IsLocked());
    m.Unlock();
    BOOST_CHECK(!m.IsLocked());
}

BOOST_AUTO_TEST_CASE(AddWaiterToLockedTest)
{
    Mutex m;
    volatile bool called1 = false;
    volatile bool called2 = false;
    volatile bool called3 = false;
    BOOST_CHECK(!m.AddWaiter([&] { called1 = true; }));
    BOOST_CHECK(m.AddWaiter([&] { called2 = true; }));
    BOOST_CHECK(m.AddWaiter([&] { called3 = true; }));
    BOOST_CHECK(!called1);
    BOOST_CHECK(!called2);
    BOOST_CHECK(!called3);
    BOOST_CHECK(m.IsLocked());
    m.Unlock();
    BOOST_CHECK(!called1);
    BOOST_CHECK(!called2);
    BOOST_CHECK(called3);
    BOOST_CHECK(m.IsLocked());
    m.Unlock();
    BOOST_CHECK(called2);
    BOOST_CHECK(m.IsLocked());
    m.Unlock();
    BOOST_CHECK(!m.IsLocked());
}

BOOST_AUTO_TEST_CASE(RemoveWaiterTest)
{
    Mutex m;
    volatile bool called1 = false;
    volatile bool called2 = false;

    auto waiter = Waiter::Create([&] { called2 = true; }, false);
    BOOST_CHECK(!m.AddWaiter([&] { called1 = true; }));
    BOOST_CHECK(m.AddWaiter(waiter));
    BOOST_CHECK(!called1);
    BOOST_CHECK(!called2);
    BOOST_CHECK(m.IsLocked());
    m.RemoveWaiter(waiter);
    m.Unlock();
    BOOST_CHECK(!called2);
    BOOST_CHECK(!m.IsLocked());
    waiter->Destroy();
}

BOOST_AUTO_TEST_SUITE_END()

}}
