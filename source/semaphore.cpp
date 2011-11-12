// Copyright (c) 2011, Christian Rorvik
// Distributed under the Simplified BSD License (See accompanying file LICENSE.txt)

#include "crunch/concurrency/semaphore.hpp"
#include "crunch/concurrency/yield.hpp"

namespace Crunch { namespace Concurrency {

Semaphore::Semaphore(uint32 initialCount)
    : mCount(initialCount)
    , mWaiters(0)
{}

void Semaphore::Post()
{
    if (mCount.Increment() < 0)
    {
        ExponentialBackoff backoff;
        uint64 head = mWaiters.Load(MEMORY_ORDER_RELAXED);

        for (;;)
        {
            Waiter* headPtr = Detail::WaiterList::GetPointer(head);

            if (headPtr == nullptr)
                return;

            if ((head & Detail::WaiterList::LOCK_BIT) == 0)
            {
                uint64 const newHead =
                    Detail::WaiterList::SetPointer(head, headPtr->next) +
                    Detail::WaiterList::ABA_ADDEND;

                if (mWaiters.CompareAndSwap(newHead, head))
                {
                    headPtr->Notify();
                    return;
                }
            }

            backoff.Pause();
        }
    }
}

void Semaphore::AddWaiter(Waiter* waiter)
{
    if (mCount.Decrement() > 0)
    {
        waiter->Notify();
    }
    else
    {
        ExponentialBackoff backoff;
        uint64 head = mWaiters.Load(MEMORY_ORDER_RELAXED);
        for (;;)
        {
            waiter->next = Detail::WaiterList::GetPointer(head);
            uint64 const newHead =
                Detail::WaiterList::SetPointer(head, waiter) +
                Detail::WaiterList::ABA_ADDEND;

            if (mWaiters.CompareAndSwap(newHead, head))
                return;

            backoff.Pause();
        }
    }
}

bool Semaphore::RemoveWaiter(Waiter* waiter)
{
    return mWaiters.RemoveWaiter(waiter);
}

bool Semaphore::IsOrderDependent() const
{
    return true;
}

}}
