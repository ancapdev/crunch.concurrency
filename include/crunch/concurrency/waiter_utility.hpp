// Copyright (c) 2011, Christian Rorvik
// Distributed under the Simplified BSD License (See accompanying file LICENSE.txt)

#ifndef CRUNCH_CONCURRENCY_WAITER_UTILITY_HPP
#define CRUNCH_CONCURRENCY_WAITER_UTILITY_HPP

#include "crunch/concurrency/waiter.hpp"

namespace Crunch { namespace Concurrency {

/// Remove waiter from list if wait is guaranteed to not be at head
/// \param head Start of list
/// \param waiter The waiter to remove
/// \return true if removed, otherwise false
inline bool RemoveWaiterFromListNotAtHead(Waiter* head, Waiter* waiter)
{
    CRUNCH_ASSERT(head != waiter);

    Waiter* current = head;
    while (current->next != nullptr)
    {
        if (current->next == waiter)
        {
            current->next = waiter->next;
            return true;
        }
        current = current->next;
    }
    return false;
}

/// Remove waiter from list
/// \param head Start of list
/// \param waiter The waiter to remove
/// \return New head
inline Waiter* RemoveWaiterFromList(Waiter* head, Waiter* waiter)
{
    if (head == waiter)
        return head->next;

    RemoveWaiterFromListNotAtHead(head, waiter);
    return head;
}

/// Notify all waiters in a list that their waitable is ready
inline void NotifyAllWaiters(Waiter* head)
{
    Waiter* current = head;
    while (current != nullptr)
    {
        // current may be invalid after Notify, so cache next before calling
        Waiter* next = current->next;
        current->Notify();
        current = next;
    }
}

}}

#endif
