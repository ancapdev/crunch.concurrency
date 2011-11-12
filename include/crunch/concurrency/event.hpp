// Copyright (c) 2011, Christian Rorvik
// Distributed under the Simplified BSD License (See accompanying file LICENSE.txt)

#ifndef CRUNCH_CONCURRENCY_EVENT_HPP
#define CRUNCH_CONCURRENCY_EVENT_HPP

#include "crunch/base/override.hpp"
#include "crunch/concurrency/detail/waiter_list.hpp"

namespace Crunch { namespace Concurrency {

class Event : public IWaitable
{
public:
    using IWaitable::AddWaiter;

    Event(bool initialState = false);

    // Locked with RemoveWaiter
    void Set();

    // Wait free
    void Reset();

    // Wait free
    bool IsSet() const;

    // Lock free
    virtual void AddWaiter(Waiter* waiter) CRUNCH_OVERRIDE;

    // Locked with RemoveWaiter and Set
    virtual bool RemoveWaiter(Waiter* waiter) CRUNCH_OVERRIDE;

    // Constant
    virtual bool IsOrderDependent() const CRUNCH_OVERRIDE;

private:
    static uint64 const EVENT_SET_BIT = Detail::WaiterList::USER_FLAG_BIT;
    Detail::WaiterList mWaiters;
};

inline void Event::Reset()
{
    mWaiters.And(~EVENT_SET_BIT);
}

inline bool Event::IsSet() const
{
    return (mWaiters.Load(MEMORY_ORDER_RELAXED) & EVENT_SET_BIT) != 0;
}

}}

#endif
