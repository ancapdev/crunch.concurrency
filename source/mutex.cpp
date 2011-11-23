// Copyright (c) 2011, Christian Rorvik
// Distributed under the Simplified BSD License (See accompanying file LICENSE.txt)

#include "crunch/base/assert.hpp"
#include "crunch/concurrency/exponential_backoff.hpp"
#include "crunch/concurrency/mutex.hpp"

namespace Crunch { namespace Concurrency {

Mutex::Mutex(std::uint32_t spinCount)
    : mWaiters(MUTEX_FREE_BIT)
    , mSpinCount(spinCount)
{}

void Mutex::Lock()
{
    std::uint64_t head = MUTEX_FREE_BIT;
    if (mWaiters.CompareAndSwap(0, head))
        return;
    
    std::uint32_t spinLeft = mSpinCount;
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
    std::uint64_t head = 0;
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
            std::uint64_t const newHead =
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

bool Mutex::AddWaiter(Waiter* waiter)
{
    std::uint64_t head = MUTEX_FREE_BIT;
    if (mWaiters.CompareAndSwap(0, head))
        return false;

    ExponentialBackoff backoff;
    for (;;)
    {
        if (head & MUTEX_FREE_BIT)
        {
            // Mutex is unlocked. Attempt to lock.
            CRUNCH_ASSERT(head == MUTEX_FREE_BIT);
            if (mWaiters.CompareAndSwap(0, head))
                return false;
        }
        else
        {
            // Mutex is locked, attempt to insert waiter
            waiter->next = Detail::WaiterList::GetPointer(head);
            std::uint64_t const newHead = Detail::WaiterList::SetPointer(head, waiter) + Detail::WaiterList::ABA_ADDEND;
            if (mWaiters.CompareAndSwap(newHead, head))
                return true;
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
