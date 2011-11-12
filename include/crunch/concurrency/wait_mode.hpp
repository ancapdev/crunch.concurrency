// Copyright (c) 2011, Christian Rorvik
// Distributed under the Simplified BSD License (See accompanying file LICENSE.txt)

#ifndef CRUNCH_CONCURRENCY_WAIT_MODE_HPP
#define CRUNCH_CONCURRENCY_WAIT_MODE_HPP

#include "crunch/base/stdint.hpp"

namespace Crunch { namespace Concurrency {

struct WaitMode
{
    WaitMode(uint32 spinCount, bool runCooperative)
        : spinCount(spinCount)
        , runCooperative(runCooperative)
    {}

    static WaitMode Poll()
    {
        return WaitMode(0xfffffffful, false);
    }

    static WaitMode Block(uint32 spinCount = 0)
    {
        return WaitMode(spinCount, false);
    }

    static WaitMode Run(uint32 spinCount = 0)
    {
        return WaitMode(spinCount, true);
    }

    uint32 spinCount;
    bool runCooperative;
};

}}

#endif
