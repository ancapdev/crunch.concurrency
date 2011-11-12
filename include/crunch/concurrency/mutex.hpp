// Copyright (c) 2011, Christian Rorvik
// Distributed under the Simplified BSD License (See accompanying file LICENSE.txt)

#ifndef CRUNCH_CONCURRENCY_MUTEX_HPP
#define CRUNCH_CONCURRENCY_MUTEX_HPP

#include "crunch/base/override.hpp"
#include "crunch/base/stdint.hpp"
#include "crunch/concurrency/detail/waiter_list.hpp"

namespace Crunch { namespace Concurrency {

// Simple non-fair mutex. Will give lock to waiters in LIFO order.
class Mutex : public IWaitable
{
public:
    using IWaitable::AddWaiter;

    Mutex(uint32 spinCount = 0);

    void Lock();
    
    void Unlock();

    bool IsLocked() const;

    virtual void AddWaiter(Waiter* waiter) CRUNCH_OVERRIDE;
    virtual bool RemoveWaiter(Waiter* waiter) CRUNCH_OVERRIDE;
    virtual bool IsOrderDependent() const CRUNCH_OVERRIDE;

private:
    // Set when free rather than locked so we don't have to strip off
    // locked bit when building waiter list
    static uint64 const MUTEX_FREE_BIT = Detail::WaiterList::USER_FLAG_BIT;

    Detail::WaiterList mWaiters;
    uint32 mSpinCount;
};

}}

#endif
