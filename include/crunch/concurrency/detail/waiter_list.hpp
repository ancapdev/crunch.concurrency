// Copyright (c) 2011, Christian Rorvik
// Distributed under the Simplified BSD License (See accompanying file LICENSE.txt)

#ifndef CRUNCH_CONCURRENCY_DETAIL_WAITER_LIST_HPP
#define CRUNCH_CONCURRENCY_DETAIL_WAITER_LIST_HPP

#include "crunch/base/stdint.hpp"
#include "crunch/concurrency/atomic.hpp"
#include "crunch/concurrency/waitable.hpp"

namespace Crunch { namespace Concurrency { namespace Detail {

struct WaiterList : public Atomic<uint64>
{
#if (CRUNCH_PTR_SIZE == 4)
    static uint64 const USER_FLAG_BIT = 1ull << 32;
    static uint64 const LOCK_BIT = 2ull << 32;
    static uint64 const FLAG_MASK = 3ull << 32;
    static uint64 const ABA_ADDEND = 4ull << 32;
    static uint64 const PTR_MASK = 0xffffffffull;
#else
    // Assume 48 bit address space and 4 byte alignment
    static uint64 const USER_FLAG_BIT = 1;
    static uint64 const LOCK_BIT = 2;
    static uint64 const FLAG_MASK = 3;
    static uint64 const ABA_ADDEND = 1ull << 48;
    static uint64 const PTR_MASK = (ABA_ADDEND - 1) & ~FLAG_MASK;
#endif

    static Waiter* GetPointer(uint64 ptrAndState)
    {
        return reinterpret_cast<Waiter*>(ptrAndState & PTR_MASK);
    }

    static uint64 SetPointer(uint64 ptrAndState, Waiter* ptr)
    {
        CRUNCH_ASSERT((reinterpret_cast<uint64>(ptr) & ~PTR_MASK) == 0);
        return (ptrAndState & ~PTR_MASK) | reinterpret_cast<uint64>(ptr);
    }

    WaiterList(uint64 initialState = 0)
        : Atomic<uint64>(initialState, MEMORY_ORDER_RELAXED)
    {}

    void Unlock()
    {
        And(~LOCK_BIT, MEMORY_ORDER_RELEASE);
    }

    bool RemoveWaiter(Waiter* waiter);
};

}}}

#endif