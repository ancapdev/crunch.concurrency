// Copyright (c) 2011, Christian Rorvik
// Distributed under the Simplified BSD License (See accompanying file LICENSE.txt)

#include "crunch/concurrency/waiter.hpp"
#include "crunch/concurrency/atomic.hpp"
#include "crunch/concurrency/detail/system_mutex.hpp"

#include <algorithm>
#include <cstdint>
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
            std::uint64_t head = mFreeList.Load(MEMORY_ORDER_RELAXED);
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
                    std::uint64_t const newHead = SetPointer(head, headPtr->next) + ABA_ADDEND;
                    if (mFreeList.CompareAndSwap(head, newHead))
                        return headPtr;
                }
            }
        }

        void Free(void* allocation)
        {
            Waiter* node = reinterpret_cast<Waiter*>(allocation);
            std::uint64_t head = mFreeList.Load(MEMORY_ORDER_RELAXED);
            for (;;)
            {
                node->next = GetPointer(head);
                std::uint64_t const newHead = SetPointer(head, node) + ABA_ADDEND;
                if (mFreeList.CompareAndSwap(head, newHead))
                    return;
            }
        }

    private:
#if (CRUNCH_PTR_SIZE == 4)
        static std::uint64_t const ABA_ADDEND = 4ull << 32;
        static std::uint64_t const PTR_MASK = 0xffffffffull;
#else
        // Assume 48 bit address space
        static std::uint64_t const ABA_ADDEND = 1ull << 48;
        static std::uint64_t const PTR_MASK = ABA_ADDEND - 1;
#endif

        Waiter* GetPointer(std::uint64_t ptrAndState)
        {
            return reinterpret_cast<Waiter*>(ptrAndState & PTR_MASK);
        }

        std::uint64_t SetPointer(std::uint64_t ptrAndState, Waiter* ptr)
        {
            CRUNCH_ASSERT((reinterpret_cast<std::uint64_t>(ptr) & ~PTR_MASK) == 0);
            return (ptrAndState & ~PTR_MASK) | reinterpret_cast<std::uint64_t>(ptr);
        }

        Atomic<std::uint64_t> mFreeList;
        Detail::SystemMutex mAllocationsLock;
        std::vector<void*> mAllocations;
    };

    WaiterAllocator gWaiterAllocator;

    CRUNCH_THREAD_LOCAL Waiter* tWaiterFreeList = nullptr;
}

void* Waiter::AllocateGlobal()
{
    return gWaiterAllocator.Allocate();
}

void Waiter::FreeGlobal(void* allocation)
{
    return gWaiterAllocator.Free(allocation);
}

void* Waiter::Allocate()
{
    Waiter* localFree = tWaiterFreeList;
    if (localFree)
    {
        tWaiterFreeList = localFree->next;
        return localFree;
    }
    return AllocateGlobal();
}

void Waiter::Free(void* allocation)
{
    // TODO: free back to global list
    Waiter* node = reinterpret_cast<Waiter*>(allocation);
    node->next = tWaiterFreeList;
    tWaiterFreeList = node;
}

}}