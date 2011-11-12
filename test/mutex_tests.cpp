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
    volatile bool acquired = false;
    m.AddWaiter([&] { acquired = true; });
    BOOST_CHECK(acquired);
    BOOST_CHECK(m.IsLocked());
    m.Unlock();
    BOOST_CHECK(!m.IsLocked());
}

BOOST_AUTO_TEST_CASE(AddWaiterToLockedTest)
{
    Mutex m;
    volatile bool acquired1 = false;
    volatile bool acquired2 = false;
    volatile bool acquired3 = false;
    m.AddWaiter([&] { acquired1 = true; });
    m.AddWaiter([&] { acquired2 = true; });
    m.AddWaiter([&] { acquired3 = true; });
    BOOST_CHECK(acquired1);
    BOOST_CHECK(!acquired2);
    BOOST_CHECK(!acquired3);
    BOOST_CHECK(m.IsLocked());
    m.Unlock();
    BOOST_CHECK(!acquired2);
    BOOST_CHECK(acquired3);
    BOOST_CHECK(m.IsLocked());
    m.Unlock();
    BOOST_CHECK(acquired2);
    BOOST_CHECK(m.IsLocked());
    m.Unlock();
    BOOST_CHECK(!m.IsLocked());
}

BOOST_AUTO_TEST_CASE(RemoveWaiterTest)
{
    Mutex m;
    volatile bool acquired1 = false;
    volatile bool acquired2 = false;

    auto waiter = Waiter::Create([&] { acquired2 = true; }, false);
    m.AddWaiter([&] { acquired1 = true; });
    m.AddWaiter(waiter);
    BOOST_CHECK(acquired1);
    BOOST_CHECK(!acquired2);
    BOOST_CHECK(m.IsLocked());
    m.RemoveWaiter(waiter);
    m.Unlock();
    BOOST_CHECK(!acquired2);
    BOOST_CHECK(!m.IsLocked());
    waiter->Destroy();
}

BOOST_AUTO_TEST_SUITE_END()

}}
