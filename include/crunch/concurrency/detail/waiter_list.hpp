// Copyright (c) 2011, Christian Rorvik
// Distributed under the Simplified BSD License (See accompanying file LICENSE.txt)

#ifndef CRUNCH_CONCURRENCY_DETAIL_WAITER_LIST_HPP
#define CRUNCH_CONCURRENCY_DETAIL_WAITER_LIST_HPP

#include "crunch/concurrency/api.hpp"
#include "crunch/concurrency/atomic.hpp"
#include "crunch/concurrency/waitable.hpp"

#include <cstdint>

namespace Crunch { namespace Concurrency { namespace Detail {

struct WaiterList : public Atomic<std::uint64_t>
{
#if (CRUNCH_PTR_SIZE == 4)
    static std::uint64_t const USER_FLAG_BIT = 1ull << 32;
    static std::uint64_t const LOCK_BIT = 2ull << 32;
    static std::uint64_t const FLAG_MASK = 3ull << 32;
    static std::uint64_t const ABA_ADDEND = 4ull << 32;
    static std::uint64_t const PTR_MASK = 0xffffffffull;
#else
    // Assume 48 bit address space and 4 byte alignment
    static std::uint64_t const USER_FLAG_BIT = 1;
    static std::uint64_t const LOCK_BIT = 2;
    static std::uint64_t const FLAG_MASK = 3;
    static std::uint64_t const ABA_ADDEND = 1ull << 48;
    static std::uint64_t const PTR_MASK = (ABA_ADDEND - 1) & ~FLAG_MASK;
#endif

    static Waiter* GetPointer(std::uint64_t ptrAndState)
    {
        return reinterpret_cast<Waiter*>(ptrAndState & PTR_MASK);
    }

    static std::uint64_t SetPointer(std::uint64_t ptrAndState, Waiter* ptr)
    {
        CRUNCH_ASSERT((reinterpret_cast<std::uint64_t>(ptr) & ~PTR_MASK) == 0);
        return (ptrAndState & ~PTR_MASK) | reinterpret_cast<std::uint64_t>(ptr);
    }

    WaiterList(std::uint64_t initialState = 0)
        : Atomic<std::uint64_t>(initialState, MEMORY_ORDER_RELAXED)
    {}

    void Unlock()
    {
        And(~LOCK_BIT, MEMORY_ORDER_RELEASE);
    }

    CRUNCH_CONCURRENCY_API bool RemoveWaiter(Waiter* waiter);
};

}}}

#endif