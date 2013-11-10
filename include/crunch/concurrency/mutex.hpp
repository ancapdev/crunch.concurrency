// Copyright (c) 2011, Christian Rorvik
// Distributed under the Simplified BSD License (See accompanying file LICENSE.txt)

#ifndef CRUNCH_CONCURRENCY_MUTEX_HPP
#define CRUNCH_CONCURRENCY_MUTEX_HPP

#include "crunch/base/override.hpp"
#include "crunch/concurrency/api.hpp"
#include "crunch/concurrency/detail/waiter_list.hpp"

#include <cstdint>

namespace Crunch { namespace Concurrency {

// Simple non-fair mutex. Will give lock to waiters in LIFO order.
class Mutex : public IWaitable
{
public:
    using IWaitable::AddWaiter;

    CRUNCH_CONCURRENCY_API Mutex(std::uint32_t spinCount = 0);

    CRUNCH_CONCURRENCY_API void Lock();
    
    CRUNCH_CONCURRENCY_API void Unlock();

    CRUNCH_CONCURRENCY_API bool IsLocked() const;

    CRUNCH_CONCURRENCY_API CRUNCH_MUST_CHECK_RESULT virtual bool AddWaiter(Waiter* waiter) CRUNCH_OVERRIDE;
    CRUNCH_CONCURRENCY_API CRUNCH_MUST_CHECK_RESULT virtual bool RemoveWaiter(Waiter* waiter) CRUNCH_OVERRIDE;
    CRUNCH_CONCURRENCY_API virtual bool IsOrderDependent() const CRUNCH_OVERRIDE;

private:
    // Set when free rather than locked so we don't have to strip off
    // locked bit when building waiter list
    static std::uint64_t const MUTEX_FREE_BIT = Detail::WaiterList::USER_FLAG_BIT;

    Detail::WaiterList mWaiters;
    std::uint32_t mSpinCount;
};

}}

#endif
