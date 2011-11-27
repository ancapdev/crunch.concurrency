// Copyright (c) 2011, Christian Rorvik
// Distributed under the Simplified BSD License (See accompanying file LICENSE.txt)

#include "crunch/concurrency/detail/waiter_list.hpp"
#include "crunch/concurrency/exponential_backoff.hpp"
#include "crunch/concurrency/waiter_utility.hpp"

namespace Crunch { namespace Concurrency { namespace Detail {

bool WaiterList::RemoveWaiter(Waiter* waiter)
{
    ExponentialBackoff backoff;
    std::uint64_t head = Load(MEMORY_ORDER_RELAXED);
    for (;;)
    {
        // If empty, nothing to remove
        if ((head & PTR_MASK) == 0)
            return false;

        // If another remove is in progress, wait for completion
        if (head & LOCK_BIT)
        {
            head = Load(MEMORY_ORDER_RELAXED);
        }
        else
        {
            Waiter* const headPtr = GetPointer(head);
            if (headPtr == waiter)
            {
                // If waiter is at head, do simple pop
                std::uint64_t const newHead = SetPointer(head, headPtr->next) + ABA_ADDEND;
                if (CompareAndSwap(head, newHead))
                    return true;
            }
            else
            {
                // Lock list from concurrent removal. Scan and remove waiter.
                if (CompareAndSwap(head, (head | LOCK_BIT) + ABA_ADDEND))
                {
                    bool removed = RemoveWaiterFromListNotAtHead(headPtr, waiter);
                    And(~LOCK_BIT, MEMORY_ORDER_RELEASE);
                    return removed;
                }
            }
        }

        backoff.Pause();
    }
}

}}}
