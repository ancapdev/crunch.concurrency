// Copyright (c) 2011, Christian Rorvik
// Distributed under the Simplified BSD License (See accompanying file LICENSE.txt)

#include "crunch/concurrency/constant_backoff.hpp"
#include "crunch/concurrency/event.hpp"
#include "crunch/concurrency/exponential_backoff.hpp"
#include "crunch/concurrency/waiter_utility.hpp"

namespace Crunch { namespace Concurrency {

Event::Event(bool initialState)
    : mWaiters(initialState ? EVENT_SET_BIT : 0)
{}

bool Event::AddWaiter(Waiter* waiter)
{
    ConstantBackoff backoff;
    std::uint64_t head = mWaiters.Load(MEMORY_ORDER_RELAXED);
    for (;;)
    {
        if (head & EVENT_SET_BIT)
            return false;

        waiter->next = Detail::WaiterList::GetPointer(head);
        std::uint64_t const newHead = Detail::WaiterList::SetPointer(head, waiter) + Detail::WaiterList::ABA_ADDEND;
        if (mWaiters.CompareAndSwap(newHead, head))
            return true;

        backoff.Pause();
    }
}

bool Event::RemoveWaiter(Waiter* waiter)
{
    return mWaiters.RemoveWaiter(waiter);
}

bool Event::IsOrderDependent() const
{
    return false;
}

void Event::Set()
{
    ExponentialBackoff backoff;
    std::uint64_t head = mWaiters.Load(MEMORY_ORDER_RELAXED);
    for (;;)
    {
        // If already set, return
        if (head & EVENT_SET_BIT)
            return;

        // Wait for list to be unlocked before notifying waiters
        if ((head & Detail::WaiterList::LOCK_BIT) == 0)
        {
            if ((head & Detail::WaiterList::PTR_MASK) == 0)
            {
                // No waiters to notify, only need to set state bit
                std::uint64_t const newHead = (head | EVENT_SET_BIT) + Detail::WaiterList::ABA_ADDEND;
                if (mWaiters.CompareAndSwap(newHead, head))
                    return;
            }
            else
            {
                // Need to acquire lock on list and notify waiters
                std::uint64_t const lockedHead =
                    ((head & ~Detail::WaiterList::PTR_MASK) |
                    Detail::WaiterList::LOCK_BIT | EVENT_SET_BIT) +
                    Detail::WaiterList::ABA_ADDEND;

                if (mWaiters.CompareAndSwap(lockedHead, head))
                {
                    NotifyAllWaiters(Detail::WaiterList::GetPointer(head));
                    mWaiters.Unlock();
                    return;
                }
            }
        }

        backoff.Pause();
    }
}

}}
