// Copyright (c) 2011, Christian Rorvik
// Distributed under the Simplified BSD License (See accompanying file LICENSE.txt)

#include "crunch/concurrency/yield.hpp"

#include <sched.h>

namespace Crunch { namespace Concurrency {

void ThreadYield()
{
    sched_yield();
}

}}
