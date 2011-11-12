// Copyright (c) 2011, Christian Rorvik
// Distributed under the Simplified BSD License (See accompanying file LICENSE.txt)

#ifndef CRUNCH_CONCURRENCY_PLATFORM_LINUX_ATOMIC_TYPE_HPP
#define CRUNCH_CONCURRENCY_PLATFORM_LINUX_ATOMIC_TYPE_HPP

#include <cstdint>
#include <cstddef>

namespace Crunch { namespace Concurrency { namespace Platform {

template<std::size_t S> struct AtomicWordHelper;
template<> struct AtomicWordHelper<1> { typedef int8_t Type; };
template<> struct AtomicWordHelper<2> { typedef int16_t Type; };
template<> struct AtomicWordHelper<4> { typedef int32_t Type; };
template<> struct AtomicWordHelper<8> { typedef int64_t Type; };

static_assert(sizeof(AtomicWordHelper<1>::Type) == 1, "Unexpected AtomicWord size");
static_assert(sizeof(AtomicWordHelper<2>::Type) == 2, "Unexpected AtomicWord size");
static_assert(sizeof(AtomicWordHelper<4>::Type) == 4, "Unexpected AtomicWord size");
static_assert(sizeof(AtomicWordHelper<8>::Type) == 8, "Unexpected AtomicWord size");

#if (CRUNCH_PTR_SIZE == 8)
typedef long long AtomicWord16 __attribute__ ((__vector_size__(16), __may_alias__));
template<> struct AtomicWordHelper<16> { typedef AtomicWord16 Type; };
static_assert(sizeof(AtomicWordHelper<16>::Type) == 16, "Unexpected AtomicWord size");
#endif

template<typename T> struct AtomicWord : AtomicWordHelper<sizeof(T)> {};

}}}

#endif
