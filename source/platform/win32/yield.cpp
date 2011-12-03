// Copyright (c) 2011, Christian Rorvik
// Distributed under the Simplified BSD License (See accompanying file LICENSE.txt)

#include "crunch/concurrency/yield.hpp"

#include <windows.h>

namespace Crunch { namespace Concurrency {

void ThreadYield()
{
    ::SwitchToThread();
}

void ThreadSleep(Duration duration)
{
    if (duration.IsNegative())
        return;

    ::Sleep(static_cast<DWORD>(duration.GetTotalMilliseconds()));
}

}}
