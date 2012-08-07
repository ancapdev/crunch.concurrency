// Copyright (c) 2011, Christian Rorvik
// Distributed under the Simplified BSD License (See accompanying file LICENSE.txt)

#include "crunch/concurrency/processor_affinity.hpp"

namespace Crunch { namespace Concurrency {

void SetProcessAffinity(pid_t process, ProcessorAffinity const& affinity)
{
}

ProcessorAffinity GetProcessAffinity(pid_t process)
{
}

ProcessorAffinity SetThreadAffinity(pid_t thread, ProcessorAffinity const& affinity)
{
}

ProcessorAffinity SetCurrentThreadAffinity(ProcessorAffinity const& affinity)
{
}

void SetCurrentProcessAffinity(ProcessorAffinity const& affinity)
{
}

ProcessorAffinity GetCurrentProcessAffinity()
{
}

}}
