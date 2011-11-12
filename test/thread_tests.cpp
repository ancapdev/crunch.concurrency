// Copyright (c) 2011, Christian Rorvik
// Distributed under the Simplified BSD License (See accompanying file LICENSE.txt)

#include "crunch/concurrency/thread.hpp"
#include "crunch/concurrency/atomic.hpp"
#include "crunch/test/framework.hpp"

namespace Crunch { namespace Concurrency {

BOOST_AUTO_TEST_SUITE(ThreadTests)

BOOST_AUTO_TEST_CASE(DefaultConstructTest)
{
    Thread t;
    BOOST_CHECK(!t.IsJoinable());
    BOOST_CHECK(t.GetId() == ThreadId());
}

BOOST_AUTO_TEST_CASE(RunTest)
{
    volatile bool ran = false;
    Thread t([&] { ran = true; });
    t.Join();
    BOOST_CHECK(ran);
}

BOOST_AUTO_TEST_CASE(MoveConstructTest)
{
    // Move construct
    Thread t([]{});
    Thread tt(std::move(t));
    BOOST_CHECK(!t.IsJoinable());
    BOOST_CHECK(tt.IsJoinable());
    tt.Join();
}

BOOST_AUTO_TEST_CASE(MoveAssignTest)
{
    Thread t([]{});
    Thread tt;
    tt = std::move(t);
    BOOST_CHECK(!t.IsJoinable());
    BOOST_CHECK(tt.IsJoinable());
    tt.Join();
}

BOOST_AUTO_TEST_CASE(SelfAssignTest)
{
    Thread t([]{});
    t = std::move(t);
    BOOST_CHECK(t.IsJoinable());
    t.Join();
}

BOOST_AUTO_TEST_SUITE_END()

}}
