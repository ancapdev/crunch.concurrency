// Copyright (c) 2011, Christian Rorvik
// Distributed under the Simplified BSD License (See accompanying file LICENSE.txt)

#ifndef CRUNCH_CONCURRENCY_EVENT_HPP
#define CRUNCH_CONCURRENCY_EVENT_HPP

#include "crunch/base/override.hpp"
#include "crunch/concurrency/api.hpp"
#include "crunch/concurrency/detail/waiter_list.hpp"

namespace Crunch { namespace Concurrency {

class Event : public IWaitable
{
public:
    using IWaitable::AddWaiter;

    CRUNCH_CONCURRENCY_API Event(bool initialState = false);

    // Locked with RemoveWaiter
    CRUNCH_CONCURRENCY_API void Set();

    // Wait free
    void Reset();

    // Wait free
    bool IsSet() const;

    // Lock free
    CRUNCH_CONCURRENCY_API virtual bool AddWaiter(Waiter* waiter) CRUNCH_OVERRIDE;

    // Locked with RemoveWaiter and Set
    CRUNCH_CONCURRENCY_API virtual bool RemoveWaiter(Waiter* waiter) CRUNCH_OVERRIDE;

    // Constant
    CRUNCH_CONCURRENCY_API virtual bool IsOrderDependent() const CRUNCH_OVERRIDE;

private:
    static std::uint64_t const EVENT_SET_BIT = Detail::WaiterList::USER_FLAG_BIT;
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
