// Copyright (c) 2011, Christian Rorvik
// Distributed under the Simplified BSD License (See accompanying file LICENSE.txt)

#include "crunch/concurrency/processor_affinity.hpp"

#include <algorithm>

namespace Crunch { namespace Concurrency {

ProcessorAffinity::ProcessorAffinity()
{
}

ProcessorAffinity::ProcessorAffinity(uint32 processorId)
{
    Set(processorId);
}

ProcessorAffinity::ProcessorAffinity(ProcessorTopology::Processor const& processor)
{
    Set(processor);
}

ProcessorAffinity::ProcessorAffinity(ProcessorTopology::ProcessorList const& processors)
{
    Set(processors);
}

void ProcessorAffinity::Set(uint32 processorId)
{
    mProcessorMask.set(processorId);
}

void ProcessorAffinity::Set(ProcessorTopology::Processor const& processor)
{
    Set(processor.systemId);
}

void ProcessorAffinity::Set(ProcessorTopology::ProcessorList const& processors)
{
    std::for_each(processors.begin(), processors.end(), [&] (ProcessorTopology::Processor const& p) { Set(p); });
}

void ProcessorAffinity::Clear(uint32 processorId)
{
    mProcessorMask.reset(processorId);
}

void ProcessorAffinity::Clear(ProcessorTopology::Processor const& processor)
{
    mProcessorMask.reset(processor.systemId);
}

void ProcessorAffinity::Clear(ProcessorTopology::ProcessorList const& processors)
{
    std::for_each(processors.begin(), processors.end(), [&] (ProcessorTopology::Processor const& p) { Clear(p); });
}

bool ProcessorAffinity::IsSet(uint32 processorId) const
{
    return mProcessorMask.test(processorId);
}

bool ProcessorAffinity::IsEmpty() const
{
    return !mProcessorMask.any();
}

uint32 ProcessorAffinity::GetHighestSetProcessor() const
{
    for (uint32 i = MaxProcessorId; i != 0; --i)
        if (IsSet(i))
            return i;

    return 0;
}

}}
