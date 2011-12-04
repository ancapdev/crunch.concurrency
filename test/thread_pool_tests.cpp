// Copyright (c) 2011, Christian Rorvik
// Distributed under the Simplified BSD License (See accompanying file LICENSE.txt)

#include "crunch/concurrency/thread_pool.hpp"

#include "crunch/test/framework.hpp"

namespace Crunch { namespace Concurrency {

BOOST_AUTO_TEST_SUITE(ThreadPoolTests)

BOOST_AUTO_TEST_CASE(PostOneTest)
{
    ThreadPool tp(1);
    Future<int> f = tp.Post([]{ return 123; });
    BOOST_CHECK_EQUAL(f.Get(), 123);
}

BOOST_AUTO_TEST_CASE(SpawnMaxTest)
{
    ThreadPool tp(3);
    std::vector<Future<void>> results;

    for (int i = 0; i < 10000; ++i)
        results.push_back(tp.Post([]{}));

    std::for_each(results.begin(), results.end(), [] (Future<void>& result)
    {
        result.Wait();
    });

    BOOST_CHECK_LE(tp.GetThreadCount(), 3u);
}

BOOST_AUTO_TEST_SUITE_END()

}}

