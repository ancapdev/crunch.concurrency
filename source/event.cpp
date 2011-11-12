// Copyright (c) 2011, Christian Rorvik
// Distributed under the Simplified BSD License (See accompanying file LICENSE.txt)

#include "crunch/concurrency/event.hpp"
#include "crunch/concurrency/yield.hpp"
#include "crunch/concurrency/waiter_utility.hpp"

namespace Crunch { namespace Concurrency {

Event::Event(bool initialState)
    : mWaiters(initialState ? EVENT_SET_BIT : 0)
{}

void Event::AddWaiter(Waiter* waiter)
{
    // ExponentialBackoff backoff;
    uint64 head = mWaiters.Load(MEMORY_ORDER_RELAXED);
    for (;;)
    {
        if (head & EVENT_SET_BIT)
        {
            waiter->Notify();
            return;
        }

        waiter->next = Detail::WaiterList::GetPointer(head);
        uint64 const newHead = Detail::WaiterList::SetPointer(head, waiter) + Detail::WaiterList::ABA_ADDEND;
        if (mWaiters.CompareAndSwap(newHead, head))
            return;

        // backoff.Pause();
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
    uint64 head = mWaiters.Load(MEMORY_ORDER_RELAXED);
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
                uint64 const newHead = (head | EVENT_SET_BIT) + Detail::WaiterList::ABA_ADDEND;
                if (mWaiters.CompareAndSwap(newHead, head))
                    return;
            }
            else
            {
                // Need to acquire lock on list and notify waiters
                uint64 const lockedHead =
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
