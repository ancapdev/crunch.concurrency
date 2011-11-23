// Copyright (c) 2011, Christian Rorvik
// Distributed under the Simplified BSD License (See accompanying file LICENSE.txt)

#ifndef CRUNCH_CONCURRENCY_WAIT_MODE_HPP
#define CRUNCH_CONCURRENCY_WAIT_MODE_HPP

#include <cstdint>

namespace Crunch { namespace Concurrency {

struct WaitMode
{
    WaitMode(std::uint32_t spinCount, bool runCooperative)
        : spinCount(spinCount)
        , runCooperative(runCooperative)
    {}

    static WaitMode Poll()
    {
        return WaitMode(0xfffffffful, false);
    }

    static WaitMode Block(std::uint32_t spinCount = 0)
    {
        return WaitMode(spinCount, false);
    }

    static WaitMode Run(std::uint32_t spinCount = 0)
    {
        return WaitMode(spinCount, true);
    }

    std::uint32_t spinCount;
    bool runCooperative;
};

}}

#endif
