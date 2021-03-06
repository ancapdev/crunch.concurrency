// Copyright (c) 2011, Christian Rorvik
// Distributed under the Simplified BSD License (See accompanying file LICENSE.txt)

#include "crunch/concurrency/atomic.hpp"
#include "crunch/test/framework.hpp"

#include <boost/mpl/list.hpp>

#include <cstdint>

namespace Crunch { namespace Concurrency {

typedef boost::mpl::list<
    std::int16_t,
    std::uint16_t,
    std::int32_t,
    std::uint32_t,
    std::int64_t,
    std::uint64_t> AtomicTypes;

typedef boost::mpl::list<
#if defined (CRUNCH_ARCH_X86_64)
    std::int64_t,
    std::uint64_t,
#endif
    std::int32_t,
    std::uint32_t> AtomicSwapTypes;

typedef AtomicSwapTypes AtomicAddTypes;

typedef boost::mpl::list<
    std::int16_t,
    std::uint16_t,
    std::int32_t,
    std::uint32_t,
    std::int64_t,
    std::uint64_t> AtomicIncrementTypes;

typedef boost::mpl::list<
    std::int8_t,
    std::uint8_t,
    std::int16_t,
    std::uint16_t,
    std::int32_t,
    std::uint32_t,
    std::int64_t,
    std::uint64_t> AtomicAndTypes;

typedef AtomicAndTypes AtomicOrTypes;
typedef AtomicAndTypes AtomicXorTypes;

BOOST_AUTO_TEST_SUITE(AtomicTests)

BOOST_AUTO_TEST_CASE_TEMPLATE(LoadTest, T, AtomicTypes)
{
    T v = 123;
    Atomic<T> a;
    std::memcpy(&a, &v, sizeof(T));
    BOOST_CHECK_EQUAL(a.Load(), v);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(StoreTest, T, AtomicTypes)
{
    Atomic<T> a;
    a.Store(123);
    T v;
    std::memcpy(&v, &a, sizeof(T));
    BOOST_CHECK_EQUAL(v, T(123));
}

BOOST_AUTO_TEST_CASE_TEMPLATE(SwapTest, T, AtomicSwapTypes)
{
    Atomic<T> value = 102;
    T const old = value.Swap(T(123));
    BOOST_CHECK_EQUAL(old, T(102));
    BOOST_CHECK_EQUAL(value, T(123));
}

BOOST_AUTO_TEST_CASE_TEMPLATE(CompareAndSwapSucceedTest, T, AtomicTypes)
{
    Atomic<T> value = 0;

    T cmp = 0;
    bool const swapped = value.CompareAndSwap(cmp, 123);
    BOOST_CHECK(swapped);
    BOOST_CHECK_EQUAL(cmp, T(0));
    BOOST_CHECK_EQUAL(value, T(123));
}

BOOST_AUTO_TEST_CASE_TEMPLATE(CompareAndSwapFailTest, T, AtomicTypes)
{
    Atomic<T> value = 0;
 
    T cmp = 123;
    bool const swapped = value.CompareAndSwap(cmp, 0);
    BOOST_CHECK(!swapped);
    BOOST_CHECK_EQUAL(cmp, T(0));
    BOOST_CHECK_EQUAL(value, T(0));
}

BOOST_AUTO_TEST_CASE_TEMPLATE(AddTest, T, AtomicAddTypes)
{
    Atomic<T> value = 0;
   
    T old = value.Add(T(123));
    BOOST_CHECK_EQUAL(old, T(0));
    BOOST_CHECK_EQUAL(value, T(123));
}

BOOST_AUTO_TEST_CASE_TEMPLATE(IncrementTest, T, AtomicIncrementTypes)
{
    Atomic<T> value = 0;

    T old = value.Increment();
    BOOST_CHECK_EQUAL(old, T(0));
    BOOST_CHECK_EQUAL(value, T(1));
}

BOOST_AUTO_TEST_CASE_TEMPLATE(DecrementTest, T, AtomicIncrementTypes)
{
    Atomic<T> value = 1;

    T old = value.Decrement();
    BOOST_CHECK_EQUAL(old, T(1));
    BOOST_CHECK_EQUAL(value, T(0));
}

BOOST_AUTO_TEST_CASE_TEMPLATE(AndTest, T, AtomicAndTypes)
{
    Atomic<T> value = 0x48;

    T old = value.And(T(0x42));
    BOOST_CHECK_EQUAL(old, T(0x48));
    BOOST_CHECK_EQUAL(value, T(0x40));
}

BOOST_AUTO_TEST_CASE_TEMPLATE(OrTest, T, AtomicOrTypes)
{
    Atomic<T> value = 0x40;

    T old = value.Or(T(0x08));
    BOOST_CHECK_EQUAL(old, T(0x40));
    BOOST_CHECK_EQUAL(value, T(0x48));
}

BOOST_AUTO_TEST_CASE_TEMPLATE(XorTest, T, AtomicXorTypes)
{
    Atomic<T> value = 0x48;

    T old = value.Xor(T(0x28));
    BOOST_CHECK_EQUAL(old, T(0x48));
    BOOST_CHECK_EQUAL(value, T(0x60));
}

BOOST_AUTO_TEST_SUITE_END()

}}
