// Copyright (c) 2011, Christian Rorvik
// Distributed under the Simplified BSD License (See accompanying file LICENSE.txt)

#include "crunch/base/assert.hpp"
#include "crunch/concurrency/mutex.hpp"
#include "crunch/concurrency/yield.hpp"

namespace Crunch { namespace Concurrency {

Mutex::Mutex(uint32 spinCount)
    : mWaiters(MUTEX_FREE_BIT)
    , mSpinCount(spinCount)
{}

void Mutex::Lock()
{
    uint64 head = MUTEX_FREE_BIT;
    if (mWaiters.CompareAndSwap(0, head))
        return;
    
    uint32 spinLeft = mSpinCount;
    while (spinLeft--)
    {
        head = mWaiters.Load(MEMORY_ORDER_RELAXED);
        if (head == MUTEX_FREE_BIT)
        {
            if (mWaiters.CompareAndSwap(0, head))
                return;
        }
        CRUNCH_PAUSE();
    }

    WaitFor(*this);
}

void Mutex::Unlock()
{
    uint64 head = 0;
    if (mWaiters.CompareAndSwap(MUTEX_FREE_BIT, head))
        return;

    ExponentialBackoff backoff;
    for (;;)
    {
        CRUNCH_ASSERT_MSG((head & MUTEX_FREE_BIT) == 0, "Attempting to release unlocked mutex");

        Waiter* headPtr = Detail::WaiterList::GetPointer(head);
        if (headPtr == nullptr)
        {
            CRUNCH_ASSERT((head & Detail::WaiterList::LOCK_BIT) == 0);

            // No waiters, attempt to set free bit.
            if (mWaiters.CompareAndSwap(MUTEX_FREE_BIT, head))
                return;
        }
        else if (head & Detail::WaiterList::LOCK_BIT)
        {
            // Wait for lock release.
            head = mWaiters.Load(MEMORY_ORDER_RELAXED);
        }
        else 
        {
            // Try to pop first waiter off list and signal
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

bool Mutex::IsLocked() const
{
    return mWaiters.Load(MEMORY_ORDER_RELAXED) != MUTEX_FREE_BIT;
}

void Mutex::AddWaiter(Waiter* waiter)
{
    uint64 head = MUTEX_FREE_BIT;
    if (mWaiters.CompareAndSwap(0, head))
    {
        waiter->Notify();
        return;
    }

    ExponentialBackoff backoff;
    for (;;)
    {
        if (head & MUTEX_FREE_BIT)
        {
            // Mutex is unlocked. Attempt to lock.
            CRUNCH_ASSERT(head == MUTEX_FREE_BIT);
            if (mWaiters.CompareAndSwap(0, head))
            {
                waiter->Notify();
                return;
            }
        }
        else
        {
            // Mutex is locked, attempt to insert waiter
            waiter->next = Detail::WaiterList::GetPointer(head);
            uint64 const newHead = Detail::WaiterList::SetPointer(head, waiter) + Detail::WaiterList::ABA_ADDEND;
            if (mWaiters.CompareAndSwap(newHead, head))
                return;
        }

        backoff.Pause();
    }
}

bool Mutex::RemoveWaiter(Waiter* waiter)
{
    return mWaiters.RemoveWaiter(waiter);
}

bool Mutex::IsOrderDependent() const
{
    return true;
}

}}
