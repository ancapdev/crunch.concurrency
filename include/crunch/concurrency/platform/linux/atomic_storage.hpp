// Copyright (c) 2011, Christian Rorvik
// Distributed under the Simplified BSD License (See accompanying file LICENSE.txt)

#ifndef CRUNCH_CONCURRENCY_PLATFORM_LINUX_ATOMIC_STORAGE_HPP
#define CRUNCH_CONCURRENCY_PLATFORM_LINUX_ATOMIC_STORAGE_HPP

#include "crunch/base/align.hpp"
#include "crunch/concurrency/platform/linux/atomic_word.hpp"

namespace Crunch { namespace Concurrency { namespace Platform {

template<typename T> struct AtomicStorage;
template<> struct AtomicStorage<int8_t> { int8_t bits; };
template<> struct CRUNCH_ALIGN_PREFIX(2) AtomicStorage<int16_t> { int16_t bits; } CRUNCH_ALIGN_POSTFIX(2);
template<> struct CRUNCH_ALIGN_PREFIX(4) AtomicStorage<int32_t> { int32_t bits; } CRUNCH_ALIGN_POSTFIX(4);
template<> struct CRUNCH_ALIGN_PREFIX(8) AtomicStorage<int64_t> { int64_t bits; } CRUNCH_ALIGN_POSTFIX(8);

static_assert(sizeof(AtomicStorage<int8_t>) == 1, "Unexpected AtomicStorage size");
static_assert(sizeof(AtomicStorage<int16_t>) == 2, "Unexpected AtomicStorage size");
static_assert(sizeof(AtomicStorage<int32_t>) == 4, "Unexpected AtomicStorage size");
static_assert(sizeof(AtomicStorage<int64_t>) == 8, "Unexpected AtomicStorage size");

#if (CRUNCH_PTR_SIZE == 8)
template<> struct CRUNCH_ALIGN_PREFIX(16) AtomicStorage<AtomicWord16> { AtomicWord16 bits; } CRUNCH_ALIGN_POSTFIX(16);
static_assert(sizeof(AtomicStorage<AtomicWord16>) == 16, "Unexpected AtomicStorage size");
#endif

}}}

#endif
