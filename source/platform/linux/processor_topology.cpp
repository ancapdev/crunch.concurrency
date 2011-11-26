// Copyright (c) 2011, Christian Rorvik
// Distributed under the Simplified BSD License (See accompanying file LICENSE.txt)

#include "crunch/concurrency/processor_topology.hpp"

#include <unistd.h>

namespace Crunch { namespace Concurrency {

std::uint32_t GetSystemNumProcessors()
{
    long numProcessors = sysconf(_SC_NPROCESSORS_ONLN);
    if (numProcessors > 0)
        return static_cast<std::uint32_t>(numProcessors);
    else
        return 1;
}

}}

