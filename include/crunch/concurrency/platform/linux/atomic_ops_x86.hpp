// Copyright (c) 2011, Christian Rorvik
// Distributed under the Simplified BSD License (See accompanying file LICENSE.txt)

#ifndef CRUNCH_CONCURRENCY_PLATFORM_LINUX_ATOMIC_OPS_X86_HPP
#define CRUNCH_CONCURRENCY_PLATFORM_LINUX_ATOMIC_OPS_X86_HPP

#include "crunch/base/assert.hpp"
#include "crunch/concurrency/fence.hpp"
#include "crunch/concurrency/memory_order.hpp"

namespace Crunch { namespace Concurrency { namespace Platform {

namespace Detail
{
    inline int8_t AtomicLoad(int8_t const volatile& src) { return src; }
    inline int16_t AtomicLoad(int16_t const volatile& src) { return src; }
    inline int32_t AtomicLoad(int32_t const volatile& src) { return src; }

    inline int64_t AtomicLoad(int64_t const volatile& src)
    {
#if defined (CRUNCH_ARCH_X86_64)
        return src;
#else
        int64_t_t result;

        __asm__ __volatile__ (
            "fildq %1\n\t"
            "fistpq %0"
            : "=m"(result) : "m"(src) : "memory");

        return result;
#endif
    }

#if defined (CRUNCH_ARCH_X86_64)
    inline AtomicWord16 AtomicLoad(AtomicWord16 const volatile& src)
    {
        return src;
    }
#endif

    inline void AtomicStore(int8_t volatile& dst, int8_t src) { dst = src; }
    inline void AtomicStore(int16_t volatile& dst, int16_t src) { dst = src; }
    inline void AtomicStore(int32_t volatile& dst, int32_t src) { dst = src; }

    inline void AtomicStore(int64_t volatile& dst, int64_t src)
    {
#if defined (CRUNCH_ARCH_X64)
        dst = src;
#else
        __asm__ __volatile__ (
            "fildq %1\n\t"
            "fistpq %0"
            : "=m"(dst) : "m"(src) : "memory");
#endif
    }

#if defined (CRUNCH_ARCH_X86_64)
    inline void AtomicStore(AtomicWord16 volatile& dst, AtomicWord16 const volatile& src)
    {
        dst = src;
    }
#endif

    inline void AtomicStoreSeqCst(int8_t volatile& dst, int8_t src)
    {
        CRUNCH_MEMORY_FENCE();
        AtomicStore(dst, src);
        CRUNCH_MEMORY_FENCE();
    }

    inline void AtomicStoreSeqCst(int16_t volatile& dst, int16_t src)
    {
        CRUNCH_MEMORY_FENCE();
        AtomicStore(dst, src);
        CRUNCH_MEMORY_FENCE();
    }

    inline void AtomicStoreSeqCst(int32_t volatile& dst, long src)
    {
        __sync_lock_test_and_set(&dst, src);
    }

    inline void AtomicStoreSeqCst(int64_t volatile& dst, int64_t src)
    {
#if defined (CRUNCH_ARCH_X86_64)
        __sync_lock_test_and_set(&dst, src);
#else
        CRUNCH_MEMORY_FENCE();
        AtomicStore(dst, src);
        CRUNCH_MEMORY_FENCE();
#endif
    }

    inline void AtomicStoreSeqCst(AtomicWord16 volatile& dst, AtomicWord16 src)
    {
        CRUNCH_MEMORY_FENCE();
        AtomicStore(dst, src);
        CRUNCH_MEMORY_FENCE();
    }
}

template<typename T>
inline T AtomicLoad(T volatile& src, MemoryOrder ordering = MEMORY_ORDER_SEQ_CST)
{
    CRUNCH_ASSERT_ALIGNMENT(&src, sizeof(T));

    if (ordering == MEMORY_ORDER_SEQ_CST)
        CRUNCH_COMPILER_FENCE();

    T result = Detail::AtomicLoad(src);

    if (ordering & MEMORY_ORDER_ACQUIRE)
        CRUNCH_COMPILER_FENCE();

    return result;
}


template<typename T>
inline void AtomicStore(T volatile& dst, T src, MemoryOrder ordering = MEMORY_ORDER_SEQ_CST)
{
    CRUNCH_ASSERT_ALIGNMENT(&dst, sizeof(T));

    if (ordering == MEMORY_ORDER_SEQ_CST)
    {
        Detail::AtomicStoreSeqCst(dst, src);
    }
    else
    {
        if (ordering & MEMORY_ORDER_RELEASE)
            CRUNCH_COMPILER_FENCE();

        Detail::AtomicStore(dst, src);
    }
}

inline int32_t AtomicSwap(int32_t volatile& dst, int32_t src, MemoryOrder = MEMORY_ORDER_SEQ_CST)
{
    CRUNCH_ASSERT_ALIGNMENT(&dst, 4);
    return __sync_lock_test_and_set(&dst, src);
}

#if defined (CRUNCH_ARCH_X86_64)
inline int64_t AtomicSwap(int64_t volatile& dst, int64_t src, MemoryOrder = MEMORY_ORDER_SEQ_CST)
{
    CRUNCH_ASSERT_ALIGNMENT(&dst, 8);
    return __sync_lock_test_and_set(&dst, src);
}
#endif

inline bool AtomicCompareAndSwap(int16_t volatile& dst, int16_t src, int16_t& cmp, MemoryOrder = MEMORY_ORDER_SEQ_CST)
{
    CRUNCH_ASSERT_ALIGNMENT(&dst, 2);
    // TODO: Comparison is redundant, but intrinsics don't expose both comparison result and old value
    //       Consider implementing with inline assembly
    const int16_t cmp_ = cmp;
    cmp = __sync_val_compare_and_swap(&dst, cmp, src);
    return cmp == cmp_;
}

inline bool AtomicCompareAndSwap(int32_t volatile& dst, int32_t src, int32_t& cmp, MemoryOrder = MEMORY_ORDER_SEQ_CST)
{
    CRUNCH_ASSERT_ALIGNMENT(&dst, 4);
    // TODO: Comparison is redundant, but intrinsics don't expose both comparison result and old value
    //       Consider implementing with inline assembly
    const int32_t cmp_ = cmp;
    cmp = __sync_val_compare_and_swap(&dst, cmp, src);
    return cmp == cmp_;
}

inline bool AtomicCompareAndSwap(int64_t volatile& dst, int64_t src, int64_t& cmp, MemoryOrder = MEMORY_ORDER_SEQ_CST)
{
    CRUNCH_ASSERT_ALIGNMENT(&dst, 8);
    // TODO: Comparison is redundant, but intrinsics don't expose both comparison result and old value
    //       Consider implementing with inline assembly
    const int64_t cmp_ = cmp;
    cmp = __sync_val_compare_and_swap(&dst, cmp, src);
    return cmp == cmp_;
}

#if defined (CRUNCH_ARCH_X86_64)
inline bool AtomicCompareAndSwap(AtomicWord16 volatile& dst, AtomicWord16 src, AtomicWord16& cmp, MemoryOrder = MEMORY_ORDER_SEQ_CST)
{
    CRUNCH_ASSERT_ALIGNMENT(&dst, 16);
    // TODO: Comparison is redundant, but intrinsics don't expose both comparison result and old value
    //       Consider implementing with inline assembly

    union Conversion
    {
        AtomicWord16 v;
        __int128_t i;
    };

    Conversion cmp_;
    cmp_.v = cmp;

    Conversion src_;
    src_.v = src;
    
    Conversion cmp2_;
    cmp2_.i = __sync_val_compare_and_swap(reinterpret_cast<__int128_t volatile*>(&dst), cmp_.i, src_.i);
    cmp = cmp2_.v;
    return cmp2_.i == cmp_.i;
}
#endif

inline int16_t AtomicIncrement(int16_t volatile& addend, MemoryOrder = MEMORY_ORDER_SEQ_CST)
{
    CRUNCH_ASSERT_ALIGNMENT(&addend, 2);
    return __sync_fetch_and_add(&addend, 1);
}

inline int32_t AtomicIncrement(int32_t volatile& addend, MemoryOrder = MEMORY_ORDER_SEQ_CST)
{
    CRUNCH_ASSERT_ALIGNMENT(&addend, 4);
    return __sync_fetch_and_add(&addend, 1);
}

#if defined (CRUNCH_ARCH_X86_64)
inline int64_t AtomicIncrement(int64_t volatile& addend, MemoryOrder = MEMORY_ORDER_SEQ_CST)
{
    CRUNCH_ASSERT_ALIGNMENT(&addend, 8);
    return __sync_fetch_and_add(&addend, 1);
}
#endif

inline int16_t AtomicDecrement(int16_t volatile& addend, MemoryOrder = MEMORY_ORDER_SEQ_CST)
{
    CRUNCH_ASSERT_ALIGNMENT(&addend, 2);
    return __sync_fetch_and_sub(&addend, 1);
}

inline int32_t AtomicDecrement(int32_t volatile& addend, MemoryOrder = MEMORY_ORDER_SEQ_CST)
{
    CRUNCH_ASSERT_ALIGNMENT(&addend, 4);
    return __sync_fetch_and_sub(&addend, 1);
}

#if defined (CRUNCH_ARCH_X86_64)
inline int64_t AtomicDecrement(int64_t volatile& addend, MemoryOrder = MEMORY_ORDER_SEQ_CST)
{
    CRUNCH_ASSERT_ALIGNMENT(&addend, 8);
    return __sync_fetch_and_sub(&addend, 1);
}
#endif

inline int32_t AtomicAdd(int32_t volatile& addend, int32_t value, MemoryOrder = MEMORY_ORDER_SEQ_CST)
{
    CRUNCH_ASSERT_ALIGNMENT(&addend, 4);
    return __sync_fetch_and_add(&addend, value);
}

#if defined (CRUNCH_ARCH_X86_64)
inline int64_t AtomicAdd(int64_t volatile& addend, int64_t value, MemoryOrder = MEMORY_ORDER_SEQ_CST)
{
    CRUNCH_ASSERT_ALIGNMENT(&addend, 8);
    return __sync_fetch_and_add(&addend, value);
}
#endif

inline int32_t AtomicSub(int32_t volatile& addend, int32_t value, MemoryOrder = MEMORY_ORDER_SEQ_CST)
{
    CRUNCH_ASSERT_ALIGNMENT(&addend, 4);
    return __sync_fetch_and_sub(&addend, value);
}

#if defined (CRUNCH_ARCH_X86_64)
inline int64_t AtomicSub(int64_t volatile& addend, int64_t value, MemoryOrder = MEMORY_ORDER_SEQ_CST)
{
    CRUNCH_ASSERT_ALIGNMENT(&addend, 8);
    return __sync_fetch_and_sub(&addend, value);
}
#endif

inline int8_t AtomicAnd(int8_t volatile& value, int8_t mask, MemoryOrder = MEMORY_ORDER_SEQ_CST)
{
    return __sync_fetch_and_and(&value, mask);
}

inline int16_t AtomicAnd(int16_t volatile& value, int16_t mask, MemoryOrder = MEMORY_ORDER_SEQ_CST)
{
    CRUNCH_ASSERT_ALIGNMENT(&value, 2);
    return __sync_fetch_and_and(&value, mask);
}

inline int32_t AtomicAnd(int32_t volatile& value, int32_t mask, MemoryOrder = MEMORY_ORDER_SEQ_CST)
{
    CRUNCH_ASSERT_ALIGNMENT(&value, 4);
    return __sync_fetch_and_and(&value, mask);
}

#if defined (CRUNCH_ARCH_X86_64)
inline int64_t AtomicAnd(int64_t volatile& value, int64_t mask, MemoryOrder = MEMORY_ORDER_SEQ_CST)
{
    CRUNCH_ASSERT_ALIGNMENT(&value, 8);
    return __sync_fetch_and_and(&value, mask);
}
#endif

inline int8_t AtomicOr(int8_t volatile& value, int8_t mask, MemoryOrder = MEMORY_ORDER_SEQ_CST)
{
    return __sync_fetch_and_or(&value, mask);
}

inline int16_t AtomicOr(int16_t volatile& value, int16_t mask, MemoryOrder = MEMORY_ORDER_SEQ_CST)
{
    CRUNCH_ASSERT_ALIGNMENT(&value, 2);
    return __sync_fetch_and_or(&value, mask);
}

inline int32_t AtomicOr(int32_t volatile& value, int32_t mask, MemoryOrder = MEMORY_ORDER_SEQ_CST)
{
    CRUNCH_ASSERT_ALIGNMENT(&value, 4);
    return __sync_fetch_and_or(&value, mask);
}

#if defined (CRUNCH_ARCH_X86_64)
inline int64_t AtomicOr(int64_t volatile& value, int64_t mask, MemoryOrder = MEMORY_ORDER_SEQ_CST)
{
    CRUNCH_ASSERT_ALIGNMENT(&value, 8);
    return __sync_fetch_and_or(&value, mask);
}
#endif

inline int8_t AtomicXor(int8_t volatile& value, int8_t mask, MemoryOrder = MEMORY_ORDER_SEQ_CST)
{
    return __sync_fetch_and_xor(&value, mask);
}

inline int16_t AtomicXor(int16_t volatile& value, int16_t mask, MemoryOrder = MEMORY_ORDER_SEQ_CST)
{
    CRUNCH_ASSERT_ALIGNMENT(&value, 2);
    return __sync_fetch_and_xor(&value, mask);
}

inline int32_t AtomicXor(int32_t volatile& value, int32_t mask, MemoryOrder = MEMORY_ORDER_SEQ_CST)
{
    CRUNCH_ASSERT_ALIGNMENT(&value, 4);
    return __sync_fetch_and_xor(&value, mask);
}

#if defined (CRUNCH_ARCH_X86_64)
inline int64_t AtomicXor(int64_t volatile& value, int64_t mask, MemoryOrder = MEMORY_ORDER_SEQ_CST)
{
    CRUNCH_ASSERT_ALIGNMENT(&value, 8);
    return __sync_fetch_and_xor(&value, mask);
}
#endif


}}}

#endif
