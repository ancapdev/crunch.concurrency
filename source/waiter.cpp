// Copyright (c) 2011, Christian Rorvik
// Distributed under the Simplified BSD License (See accompanying file LICENSE.txt)

#include "crunch/concurrency/waiter.hpp"
#include "crunch/concurrency/atomic.hpp"
#include "crunch/concurrency/detail/system_mutex.hpp"
#include "crunch/base/stdint.hpp"

#include <algorithm>
#include <cstdlib>
#include <vector>

namespace Crunch { namespace Concurrency {

namespace
{
    class WaiterAllocator
    {
    public:
        WaiterAllocator()
            : mFreeList(0)
        {}

        ~WaiterAllocator()
        {
            std::for_each(mAllocations.begin(), mAllocations.end(), [] (void* allocation) { std::free(allocation); });
        }

        void* Allocate()
        {
            uint64 head = mFreeList.Load(MEMORY_ORDER_RELAXED);
            for (;;)
            {
                Waiter* headPtr = GetPointer(head);
                if (headPtr == nullptr)
                {
                    Detail::SystemMutex::ScopedLock lock(mAllocationsLock);
                    void* allocation = std::malloc(sizeof(Waiter));
                    mAllocations.push_back(allocation);
                    return allocation;
                }
                else
                {
                    uint64 const newHead = SetPointer(head, headPtr->next) + ABA_ADDEND;
                    if (mFreeList.CompareAndSwap(newHead, head))
                        return headPtr;
                }
            }
        }

        void Free(void* allocation)
        {
            Waiter* node = reinterpret_cast<Waiter*>(allocation);
            uint64 head = mFreeList.Load(MEMORY_ORDER_RELAXED);
            for (;;)
            {
                node->next = GetPointer(head);
                uint64 const newHead = SetPointer(head, node) + ABA_ADDEND;
                if (mFreeList.CompareAndSwap(newHead, head))
                    return;
            }
        }

    private:
#if (CRUNCH_PTR_SIZE == 4)
        static uint64 const ABA_ADDEND = 4ull << 32;
        static uint64 const PTR_MASK = 0xffffffffull;
#else
        // Assume 48 bit address space
        static uint64 const ABA_ADDEND = 1ull << 48;
        static uint64 const PTR_MASK = ABA_ADDEND - 1;
#endif

        Waiter* GetPointer(uint64 ptrAndState)
        {
            return reinterpret_cast<Waiter*>(ptrAndState & PTR_MASK);
        }

        uint64 SetPointer(uint64 ptrAndState, Waiter* ptr)
        {
            CRUNCH_ASSERT((reinterpret_cast<uint64>(ptr) & ~PTR_MASK) == 0);
            return (ptrAndState & ~PTR_MASK) | reinterpret_cast<uint64>(ptr);
        }

        Atomic<uint64> mFreeList;
        Detail::SystemMutex mAllocationsLock;
        std::vector<void*> mAllocations;
    };

    WaiterAllocator gWaiterAllocator;
}

CRUNCH_THREAD_LOCAL Waiter* Waiter::tWaiterFreeList = 0;

void* Waiter::AllocateGlobal()
{
    return gWaiterAllocator.Allocate();
}

void Waiter::FreeGlobal(void* allocation)
{
    return gWaiterAllocator.Free(allocation);
}

}}