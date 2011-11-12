// Copyright (c) 2011, Christian Rorvik
// Distributed under the Simplified BSD License (See accompanying file LICENSE.txt)

#ifndef CRUNCH_CONCURRENCY_PLATFORM_WIN32_ATOMIC_STORAGE_HPP
#define CRUNCH_CONCURRENCY_PLATFORM_WIN32_ATOMIC_STORAGE_HPP

#include "crunch/base/align.hpp"

#include <emmintrin.h>

namespace Crunch { namespace Concurrency { namespace Platform {

template<typename T> struct AtomicStorage;
template<> struct AtomicStorage<char> { char bits; };
template<> struct CRUNCH_ALIGN_PREFIX(2) AtomicStorage<short> { short bits; } CRUNCH_ALIGN_POSTFIX(2);
template<> struct CRUNCH_ALIGN_PREFIX(4) AtomicStorage<long> { long bits; } CRUNCH_ALIGN_POSTFIX(4);
template<> struct CRUNCH_ALIGN_PREFIX(8) AtomicStorage<__int64> { __int64 bits; } CRUNCH_ALIGN_POSTFIX(8);
template<> struct CRUNCH_ALIGN_PREFIX(16) AtomicStorage<__m128i> { __m128i bits; } CRUNCH_ALIGN_POSTFIX(16);

static_assert(sizeof(AtomicStorage<char>) == 1, "Unexpected AtomicStorage size");
static_assert(sizeof(AtomicStorage<short>) == 2, "Unexpected AtomicStorage size");
static_assert(sizeof(AtomicStorage<long>) == 4, "Unexpected AtomicStorage size");
static_assert(sizeof(AtomicStorage<__int64>) == 8, "Unexpected AtomicStorage size");
static_assert(sizeof(AtomicStorage<__m128i>) == 16, "Unexpected AtomicStorage size");

}}}

#endif