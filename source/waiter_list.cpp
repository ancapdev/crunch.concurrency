// Copyright (c) 2011, Christian Rorvik
// Distributed under the Simplified BSD License (See accompanying file LICENSE.txt)

#include "crunch/concurrency/detail/waiter_list.hpp"
#include "crunch/concurrency/waiter_utility.hpp"
#include "crunch/concurrency/yield.hpp"

namespace Crunch { namespace Concurrency { namespace Detail {

bool WaiterList::RemoveWaiter(Waiter* waiter)
{
    ExponentialBackoff backoff;
    uint64 head = Load(MEMORY_ORDER_RELAXED);
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
                uint64 const newHead = SetPointer(head, headPtr->next) + ABA_ADDEND;
                if (CompareAndSwap(newHead, head))
                    return true;
            }
            else
            {
                // Lock list from concurrent removal. Scan and remove waiter.
                if (CompareAndSwap((head | LOCK_BIT) + ABA_ADDEND, head))
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
