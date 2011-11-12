// Copyright (c) 2011, Christian Rorvik
// Distributed under the Simplified BSD License (See accompanying file LICENSE.txt)

#ifndef CRUNCH_CONCURRENCY_PLATFORM_WIN32_ATOMIC_OPS_X86_HPP
#define CRUNCH_CONCURRENCY_PLATFORM_WIN32_ATOMIC_OPS_X86_HPP

#include "crunch/base/assert.hpp"
#include "crunch/concurrency/fence.hpp"
#include "crunch/concurrency/memory_order.hpp"

#include <intrin.h>

namespace Crunch { namespace Concurrency { namespace Platform {

namespace Detail
{
    inline char AtomicLoad(char volatile const& src) { return src; }
    inline short AtomicLoad(short volatile const& src) { return src; }
    inline long AtomicLoad(long volatile const& src) { return src; }

    inline __int64 AtomicLoad(__int64 volatile const& src)
    {
#if defined (CRUNCH_ARCH_X86_64)
        return src;
#else
        __int64 result;
        __asm
        {
            mov eax, dword ptr [src]
            fild qword ptr [eax]
            fistp qword ptr [result]
        }
        return result;
#endif
    }

    inline __m128i AtomicLoad(__m128i volatile const& src)
    {
        return _mm_load_si128(const_cast<__m128i const*>(&src));;
    }

    inline void AtomicStore(char volatile& dst, char src) { dst = src; }
    inline void AtomicStore(short volatile& dst, short src) { dst = src; }
    inline void AtomicStore(long volatile& dst, long src) { dst = src; }

    inline void AtomicStore(__int64 volatile& dst, __int64 src)
    {
#if defined (CRUNCH_ARCH_X86_64)
        dst = src;
#else
        __asm
        {
            mov eax, dword ptr [dst]
            fild qword ptr [src]
            fistp qword ptr [eax]
        }
#endif
    };

    inline void AtomicStore(__m128i volatile& dst, __m128i src)
    {
        _mm_store_si128(const_cast<__m128i*>(&dst), src);
    }

    inline void AtomicStoreSeqCst(char volatile& dst, char src)
    {
        CRUNCH_MEMORY_FENCE();
        AtomicStore(dst, src);
        CRUNCH_MEMORY_FENCE();
    }

    inline void AtomicStoreSeqCst(short volatile& dst, short src)
    {
        CRUNCH_MEMORY_FENCE();
        AtomicStore(dst, src);
        CRUNCH_MEMORY_FENCE();
    }

    inline void AtomicStoreSeqCst(long volatile& dst, long src)
    {
        _InterlockedExchange(&dst, src);
    }

    inline void AtomicStoreSeqCst(__int64 volatile& dst, __int64 src)
    {
#if defined (CRUNCH_ARCH_X86_64)
        _InterlockedExchange64(&dst, src);
#else
        CRUNCH_MEMORY_FENCE();
        AtomicStore(dst, src);
        CRUNCH_MEMORY_FENCE();
#endif
    }

    inline void AtomicStoreSeqCst(__m128i volatile& dst, __m128i src)
    {
        CRUNCH_MEMORY_FENCE();
        AtomicStore(dst, src);
        CRUNCH_MEMORY_FENCE();
    }

#if defined (CRUNCH_ARCH_X86_32)
    template<typename Op>
    inline __int64 FallbackAtomicOp(__int64 volatile& value, Op op)
    {
        __int64 oldValue = value;
        for (;;)
        {
            __int64 const newValue = op(oldValue);
            __int64 currentValue = _InterlockedCompareExchange64(&value, newValue, oldValue);
            if (currentValue == oldValue)
                return oldValue;

            _mm_pause();
            oldValue = currentValue;
        }
    }
#endif
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

inline long AtomicSwap(long volatile& dst, long src, MemoryOrder = MEMORY_ORDER_SEQ_CST)
{
    CRUNCH_ASSERT_ALIGNMENT(&dst, 4);
    return _InterlockedExchange(&dst, src);
}


inline __int64 AtomicSwap(__int64 volatile& dst, __int64 src, MemoryOrder = MEMORY_ORDER_SEQ_CST)
{
    CRUNCH_ASSERT_ALIGNMENT(&dst, 8);
#if defined (CRUNCH_ARCH_X86_64)
    return _InterlockedExchange64(&dst, src);
#else
    return Detail::FallbackAtomicOp(dst, [=] (__int64) { return src; });
#endif
}


inline bool AtomicCompareAndSwap(short volatile& dst, short src, short& cmp, MemoryOrder = MEMORY_ORDER_SEQ_CST)
{
    CRUNCH_ASSERT_ALIGNMENT(&dst, 2);
    short cmp_ = cmp;
    cmp = _InterlockedCompareExchange16(&dst, src, cmp);
    return cmp_ == cmp;
}

inline bool AtomicCompareAndSwap(long volatile& dst, long src, long& cmp, MemoryOrder = MEMORY_ORDER_SEQ_CST)
{
    CRUNCH_ASSERT_ALIGNMENT(&dst, 4);
    long cmp_ = cmp;
    cmp = _InterlockedCompareExchange(&dst, src, cmp);
    return cmp_ == cmp;
}

inline bool AtomicCompareAndSwap(__int64 volatile& dst, __int64 src, __int64& cmp, MemoryOrder = MEMORY_ORDER_SEQ_CST)
{
    CRUNCH_ASSERT_ALIGNMENT(&dst, 8);
    __int64 cmp_ = cmp;
    cmp = _InterlockedCompareExchange64(&dst, src, cmp);
    return cmp_ == cmp;
}

#if defined (CRUNCH_ARCH_X86_64)
inline bool AtomicCompareAndSwap(__m128i volatile& dst, __m128i src, __m128i& cmp, MemoryOrder = MEMORY_ORDER_SEQ_CST)
{
    CRUNCH_ASSERT_ALIGNMENT(&dst, 16);

    return _InterlockedCompareExchange128(
        dst.m128i_i64,
        src.m128i_i64[1],
        src.m128i_i64[0],
        cmp.m128i_i64) != 0;
}
#endif

inline short AtomicIncrement(short volatile& addend, MemoryOrder = MEMORY_ORDER_SEQ_CST)
{
    CRUNCH_ASSERT_ALIGNMENT(&addend, 2);
    return _InterlockedIncrement16(&addend) - 1;
}

inline long AtomicIncrement(long volatile& addend, MemoryOrder = MEMORY_ORDER_SEQ_CST)
{
    CRUNCH_ASSERT_ALIGNMENT(&addend, 4);
    return _InterlockedIncrement(&addend) - 1;
}


inline __int64 AtomicIncrement(__int64 volatile& addend, MemoryOrder = MEMORY_ORDER_SEQ_CST)
{
    CRUNCH_ASSERT_ALIGNMENT(&addend, 8);
#if defined (CRUNCH_ARCH_X86_64)
    return _InterlockedIncrement64(&addend) - 1;
#else
    return Detail::FallbackAtomicOp(addend, [] (__int64 x) { return x + 1; });
#endif
}

inline short AtomicDecrement(short volatile& addend, MemoryOrder = MEMORY_ORDER_SEQ_CST)
{
    CRUNCH_ASSERT_ALIGNMENT(&addend, 2);
    return _InterlockedDecrement16(&addend) + 1;
}

inline long AtomicDecrement(long volatile& addend, MemoryOrder = MEMORY_ORDER_SEQ_CST)
{
    CRUNCH_ASSERT_ALIGNMENT(&addend, 4);
    return _InterlockedDecrement(&addend) + 1;
}


inline __int64 AtomicDecrement(__int64 volatile& addend, MemoryOrder = MEMORY_ORDER_SEQ_CST)
{
    CRUNCH_ASSERT_ALIGNMENT(&addend, 8);
#if defined (CRUNCH_ARCH_X86_64)
    return _InterlockedDecrement64(&addend) + 1;
#else
    return Detail::FallbackAtomicOp(addend, [] (__int64 x) { return x - 1; });
#endif
}

inline long AtomicAdd(long volatile& addend, long value, MemoryOrder = MEMORY_ORDER_SEQ_CST)
{
    CRUNCH_ASSERT_ALIGNMENT(&addend, 4);
    return _InterlockedExchangeAdd(&addend, value);
}

inline __int64 AtomicAdd(__int64 volatile& addend, __int64 value, MemoryOrder = MEMORY_ORDER_SEQ_CST)
{
    CRUNCH_ASSERT_ALIGNMENT(&addend, 8);
#if defined (CRUNCH_ARCH_X86_64)
    return _InterlockedExchangeAdd64(&addend, value);
#else
    return Detail::FallbackAtomicOp(addend, [=] (__int64 x) { return x + value; });
#endif
}

inline long AtomicSub(long volatile& addend, long value, MemoryOrder = MEMORY_ORDER_SEQ_CST)
{
    CRUNCH_ASSERT_ALIGNMENT(&addend, 4);
    return _InterlockedExchangeAdd(&addend, -value);
}


inline __int64 AtomicSub(__int64 volatile& addend, __int64 value, MemoryOrder = MEMORY_ORDER_SEQ_CST)
{
    CRUNCH_ASSERT_ALIGNMENT(&addend, 8);
#if defined (CRUNCH_ARCH_X86_64)
    return _InterlockedExchangeAdd64(&addend, -value);
#else
    return Detail::FallbackAtomicOp(addend, [=] (__int64 x) { return x - value; });
#endif
}

inline char AtomicAnd(char volatile& value, char mask, MemoryOrder = MEMORY_ORDER_SEQ_CST)
{
    return _InterlockedAnd8(&value, mask);
}

inline short AtomicAnd(short volatile& value, short mask, MemoryOrder = MEMORY_ORDER_SEQ_CST)
{
    CRUNCH_ASSERT_ALIGNMENT(&value, 2);
    return _InterlockedAnd16(&value, mask);
}

inline long AtomicAnd(long volatile& value, long mask, MemoryOrder = MEMORY_ORDER_SEQ_CST)
{
    CRUNCH_ASSERT_ALIGNMENT(&value, 4);
    return _InterlockedAnd(&value, mask);
}

inline __int64 AtomicAnd(__int64 volatile& value, __int64 mask, MemoryOrder = MEMORY_ORDER_SEQ_CST)
{
    CRUNCH_ASSERT_ALIGNMENT(&value, 8);
#if defined (CRUNCH_ARCH_X86_64)
    return _InterlockedAnd64(&value, mask);
#else
    return Detail::FallbackAtomicOp(value, [=] (__int64 x) { return x & mask; });
#endif
}


inline char AtomicOr(char volatile& value, char mask, MemoryOrder = MEMORY_ORDER_SEQ_CST)
{
    return _InterlockedOr8(&value, mask);
}

inline short AtomicOr(short volatile& value, short mask, MemoryOrder = MEMORY_ORDER_SEQ_CST)
{
    CRUNCH_ASSERT_ALIGNMENT(&value, 2);
    return _InterlockedOr16(&value, mask);
}

inline long AtomicOr(long volatile& value, long mask, MemoryOrder = MEMORY_ORDER_SEQ_CST)
{
    CRUNCH_ASSERT_ALIGNMENT(&value, 4);
    return _InterlockedOr(&value, mask);
}


inline __int64 AtomicOr(__int64 volatile& value, __int64 mask, MemoryOrder = MEMORY_ORDER_SEQ_CST)
{
    CRUNCH_ASSERT_ALIGNMENT(&value, 8);
#if defined (CRUNCH_ARCH_X86_64)
    return _InterlockedOr64(&value, mask);
#else
    return Detail::FallbackAtomicOp(value, [=] (__int64 x) { return x | mask; });
#endif
}

inline char AtomicXor(char volatile& value, char mask, MemoryOrder = MEMORY_ORDER_SEQ_CST)
{
    return _InterlockedXor8(&value, mask);
}

inline short AtomicXor(short volatile& value, short mask, MemoryOrder = MEMORY_ORDER_SEQ_CST)
{
    CRUNCH_ASSERT_ALIGNMENT(&value, 2);
    return _InterlockedXor16(&value, mask);
}

inline long AtomicXor(long volatile& value, long mask, MemoryOrder = MEMORY_ORDER_SEQ_CST)
{
    CRUNCH_ASSERT_ALIGNMENT(&value, 4);
    return _InterlockedXor(&value, mask);
}

inline __int64 AtomicXor(__int64 volatile& value, __int64 mask, MemoryOrder = MEMORY_ORDER_SEQ_CST)
{
    CRUNCH_ASSERT_ALIGNMENT(&value, 8);
#if defined (CRUNCH_ARCH_X86_64)
    return _InterlockedXor64(&value, mask);
#else
    return Detail::FallbackAtomicOp(value, [=] (__int64 x) { return x ^ mask; });
#endif
}

}}}

#endif
