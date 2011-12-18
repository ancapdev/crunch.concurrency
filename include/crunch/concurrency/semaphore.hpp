// Copyright (c) 2011, Christian Rorvik
// Distributed under the Simplified BSD License (See accompanying file LICENSE.txt)

#ifndef CRUNCH_CONCURRENCY_SEMAPHORE_HPP
#define CRUNCH_CONCURRENCY_SEMAPHORE_HPP

#include "crunch/base/override.hpp"
#include "crunch/concurrency/api.hpp"
#include "crunch/concurrency/detail/waiter_list.hpp"

#include <cstdint>

namespace Crunch { namespace Concurrency {

class Semaphore : public IWaitable
{
public:
    using IWaitable::AddWaiter;

    CRUNCH_CONCURRENCY_API Semaphore(std::uint32_t initialCount = 0);

    CRUNCH_CONCURRENCY_API void Post();

    CRUNCH_CONCURRENCY_API virtual bool AddWaiter(Waiter* waiter) CRUNCH_OVERRIDE;
    CRUNCH_CONCURRENCY_API virtual bool RemoveWaiter(Waiter* waiter) CRUNCH_OVERRIDE;
    CRUNCH_CONCURRENCY_API virtual bool IsOrderDependent() const CRUNCH_OVERRIDE;

private:
    Atomic<std::int32_t> mCount;
    Detail::WaiterList mWaiters;
};

}}

#endif
