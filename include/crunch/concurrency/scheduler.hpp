// Copyright (c) 2011, Christian Rorvik
// Distributed under the Simplified BSD License (See accompanying file LICENSE.txt)

#ifndef CRUNCH_CONCURRENCY_SCHEDULER_HPP
#define CRUNCH_CONCURRENCY_SCHEDULER_HPP

#include "crunch/concurrency/waitable.hpp"

namespace Crunch { namespace Concurrency {
    
struct ISchedulerContext
{
    enum State
    {
        STATE_IDLE,
        STATE_BUSY
    };

    virtual State RunUntil(IWaitable& condition) = 0;

    virtual State RunOne() = 0;

    virtual void RunAll()
    {
        while (RunOne() == STATE_BUSY) {}
    }

    virtual IWaitable& GetHasWorkCondition() = 0;
};

struct IScheduler
{
    virtual ISchedulerContext& GetContext() = 0;
};

}}

#endif
