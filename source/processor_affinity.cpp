// Copyright (c) 2011, Christian Rorvik
// Distributed under the Simplified BSD License (See accompanying file LICENSE.txt)

#include "crunch/concurrency/processor_affinity.hpp"

#include <algorithm>

namespace Crunch { namespace Concurrency {

ProcessorAffinity::ProcessorAffinity()
{
}

ProcessorAffinity::ProcessorAffinity(std::uint32_t processorId)
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

void ProcessorAffinity::Set(std::uint32_t processorId)
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

void ProcessorAffinity::Clear(std::uint32_t processorId)
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

bool ProcessorAffinity::IsSet(std::uint32_t processorId) const
{
    return mProcessorMask.test(processorId);
}

bool ProcessorAffinity::IsEmpty() const
{
    return !mProcessorMask.any();
}

std::uint32_t ProcessorAffinity::GetHighestSetProcessor() const
{
    for (std::uint32_t i = MaxProcessorId; i != 0; --i)
        if (IsSet(i))
            return i;

    return 0;
}

}}
