// Copyright (c) 2011, Christian Rorvik
// Distributed under the Simplified BSD License (See accompanying file LICENSE.txt)

#ifndef CRUNCH_CONCURRENCY_PLATFORM_WIN32_ATOMIC_TYPE_HPP
#define CRUNCH_CONCURRENCY_PLATFORM_WIN32_ATOMIC_TYPE_HPP

#include <emmintrin.h>

#include <cstddef>

namespace Crunch { namespace Concurrency { namespace Platform {

template<std::size_t S> struct AtomicWordHelper;
template<> struct AtomicWordHelper<1> { typedef char Type; };
template<> struct AtomicWordHelper<2> { typedef short Type; };
template<> struct AtomicWordHelper<4> { typedef long Type; };
template<> struct AtomicWordHelper<8> { typedef __int64 Type; };
template<> struct AtomicWordHelper<16> { typedef __m128i Type; };

static_assert(sizeof(AtomicWordHelper<1>::Type) == 1, "Unexpected AtomicWord size");
static_assert(sizeof(AtomicWordHelper<2>::Type) == 2, "Unexpected AtomicWord size");
static_assert(sizeof(AtomicWordHelper<4>::Type) == 4, "Unexpected AtomicWord size");
static_assert(sizeof(AtomicWordHelper<8>::Type) == 8, "Unexpected AtomicWord size");
static_assert(sizeof(AtomicWordHelper<16>::Type) == 16, "Unexpected AtomicWord size");

template<typename T> struct AtomicWord : AtomicWordHelper<sizeof(T)> {};

}}}

#endif