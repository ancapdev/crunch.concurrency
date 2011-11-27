// Copyright (c) 2011, Christian Rorvik
// Distributed under the Simplified BSD License (See accompanying file LICENSE.txt)

#ifndef CRUNCH_CONCURRENCY_SCHEDULER_HPP
#define CRUNCH_CONCURRENCY_SCHEDULER_HPP

#include "crunch/base/novtable.hpp"
#include "crunch/base/enum_class.hpp"
#include "crunch/concurrency/waitable.hpp"

namespace Crunch { namespace Concurrency {
    
struct CRUNCH_NOVTABLE IThrottler
{
    virtual bool ShouldYield() const = 0;

    virtual ~IThrottler() {}
};

struct CRUNCH_NOVTABLE ISchedulerContext
{
    CRUNCH_ENUM_CLASS State
    {
        Idle,    ///< No more work to do until has work condition is signaled
        Working, ///< Busy working
        Polling  ///< Busy polling for work
    };

    // Run until throttler->ShouldYield() or idle. Needs to be polled for each work item. Enables implementation to
    // - Run N
    // - Run timed
    // - Run until IWaitable event
    // or a combination of such conditions
    virtual State Run(IThrottler const& throttler) = 0;

    virtual IWaitable& GetHasWorkCondition() = 0;

    virtual ~ISchedulerContext() {}
};

struct IScheduler
{
    virtual ISchedulerContext& GetContext() = 0;
};

}}

#endif
