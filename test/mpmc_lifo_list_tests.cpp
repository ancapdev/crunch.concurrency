// Copyright (c) 2011, Christian Rorvik
// Distributed under the Simplified BSD License (See accompanying file LICENSE.txt)

#include "crunch/concurrency/mpmc_lifo_list.hpp"
#include "crunch/test/framework.hpp"

namespace Crunch { namespace Concurrency {

namespace
{
    struct TestNode
    {
        TestNode* next;
    };

    void SetNext(TestNode& node, TestNode* next)
    {
        node.next = next;
    }

    TestNode* GetNext(TestNode& node)
    {
        return node.next;
    }
}

BOOST_AUTO_TEST_SUITE(MPMCLifeListTests)

BOOST_AUTO_TEST_CASE(InitialStateTest)
{
    MPMCLifoList<TestNode> list;
    BOOST_CHECK_EQUAL(list.Pop(), (TestNode*)0);
}

BOOST_AUTO_TEST_CASE(SinglePushPopoTest)
{
    MPMCLifoList<TestNode> list;

    TestNode n0;
    list.Push(&n0);
    TestNode* p0 = list.Pop();
    BOOST_CHECK_EQUAL(p0, &n0);
}

BOOST_AUTO_TEST_CASE(MultiPushPopTest)
{
    MPMCLifoList<TestNode> list;

    TestNode n[5];
    for (int i = 0; i < 5; ++i)
        list.Push(n + i);

    BOOST_CHECK_EQUAL(list.Pop(), n + 4);
    BOOST_CHECK_EQUAL(list.Pop(), n + 3);
    BOOST_CHECK_EQUAL(list.Pop(), n + 2);
    BOOST_CHECK_EQUAL(list.Pop(), n + 1);
    BOOST_CHECK_EQUAL(list.Pop(), n + 0);
    BOOST_CHECK_EQUAL(list.Pop(), (TestNode*)0);
}


BOOST_AUTO_TEST_SUITE_END()

}}
