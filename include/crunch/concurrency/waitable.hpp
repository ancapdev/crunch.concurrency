// Copyright (c) 2011, Christian Rorvik
// Distributed under the Simplified BSD License (See accompanying file LICENSE.txt)

#ifndef CRUNCH_CONCURRENCY_WAITABLE_HPP
#define CRUNCH_CONCURRENCY_WAITABLE_HPP

#include "crunch/base/novtable.hpp"
#include "crunch/base/must_check_result.hpp"
#include "crunch/concurrency/waiter.hpp"
#include "crunch/concurrency/wait_mode.hpp"
#include "crunch/containers/small_vector.hpp"

namespace Crunch { namespace Concurrency {

struct CRUNCH_NOVTABLE IWaitable
{
    template<typename F>
    CRUNCH_MUST_CHECK_RESULT bool AddWaiter(F callback)
    {
        auto waiter = Waiter::Create(callback, true);
        if (AddWaiter(waiter))
            return true;
        waiter->Destroy();
        return false;
    }

    template<typename F>
    CRUNCH_MUST_CHECK_RESULT bool AddWaiter(Waiter::Typed<F>* waiter)
    {
        return AddWaiter(static_cast<Waiter*>(waiter));
    }

    /// \return true if added, false otherwise
    CRUNCH_MUST_CHECK_RESULT virtual bool AddWaiter(Waiter* waiter) = 0;

    /// \return true if removed, false otherwise.
    CRUNCH_MUST_CHECK_RESULT virtual bool RemoveWaiter(Waiter* waiter) = 0;

    virtual bool IsOrderDependent() const = 0;

    virtual ~IWaitable() { }
};

/// List of signaled waitables
typedef Containers::SmallVector<IWaitable*, 16> WaitForAnyResult;

void WaitFor(IWaitable& waitable, WaitMode waitMode = WaitMode::Run());
void WaitForAll(IWaitable** waitables, std::size_t count, WaitMode waitMode = WaitMode::Run());
WaitForAnyResult WaitForAny(IWaitable** waitables, std::size_t count, WaitMode waitMode = WaitMode::Run());

}}

#endif
