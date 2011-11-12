// Copyright (c) 2011, Christian Rorvik
// Distributed under the Simplified BSD License (See accompanying file LICENSE.txt)

#include "crunch/concurrency/processor_topology.hpp"

#include <windows.h>

namespace Crunch { namespace Concurrency {

uint32 GetSystemNumProcessors()
{
    SYSTEM_INFO info;
    GetSystemInfo(&info);
    return info.dwNumberOfProcessors;
}

}}
