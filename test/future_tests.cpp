// Copyright (c) 2011, Christian Rorvik
// Distributed under the Simplified BSD License (See accompanying file LICENSE.txt)

#include "crunch/concurrency/future.hpp"
#include "crunch/concurrency/promise.hpp"
#include "crunch/test/framework.hpp"

#include <stdexcept>

namespace Crunch { namespace Concurrency {

BOOST_AUTO_TEST_SUITE(FutureTests)

BOOST_AUTO_TEST_CASE(GetValueTest)
{
    Promise<int> p;
    Future<int> f = p.GetFuture();

    BOOST_CHECK(!f.HasValue());
    BOOST_CHECK(!f.HasException());
    BOOST_CHECK(!f.IsReady());

    p.SetValue(123);
    BOOST_CHECK(f.IsReady());
    BOOST_CHECK(f.HasValue());
    BOOST_CHECK(!f.HasException());
    BOOST_CHECK_EQUAL(f.Get(), 123);
}

BOOST_AUTO_TEST_CASE(ExceptionTest)
{
    Promise<int> p;
    Future<int> f = p.GetFuture();
    p.SetException(std::copy_exception(std::runtime_error("test")));

    BOOST_CHECK(!f.HasValue());
    BOOST_CHECK(f.HasException());
    BOOST_CHECK_THROW(f.Get(), std::runtime_error);
}

BOOST_AUTO_TEST_SUITE_END()

}}
