// Copyright (c) 2011, Christian Rorvik
// Distributed under the Simplified BSD License (See accompanying file LICENSE.txt)

#ifndef CRUNCH_CONCURRENCY_SEMAPHORE_HPP
#define CRUNCH_CONCURRENCY_SEMAPHORE_HPP

#include "crunch/base/override.hpp"
#include "crunch/base/stdint.hpp"
#include "crunch/concurrency/detail/waiter_list.hpp"

namespace Crunch { namespace Concurrency {

class Semaphore : public IWaitable
{
public:
    using IWaitable::AddWaiter;

    Semaphore(uint32 initialCount = 0);

    void Post();

    virtual void AddWaiter(Waiter* waiter) CRUNCH_OVERRIDE;
    virtual bool RemoveWaiter(Waiter* waiter) CRUNCH_OVERRIDE;
    virtual bool IsOrderDependent() const CRUNCH_OVERRIDE;

private:
    Atomic<int32> mCount;
    Detail::WaiterList mWaiters;
};

}}

#endif
