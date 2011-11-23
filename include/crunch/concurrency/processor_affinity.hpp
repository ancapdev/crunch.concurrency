// Copyright (c) 2011, Christian Rorvik
// Distributed under the Simplified BSD License (See accompanying file LICENSE.txt)

#ifndef CRUNCH_CONCURRENCY_PROCESSOR_AFFINITY_HPP
#define CRUNCH_CONCURRENCY_PROCESSOR_AFFINITY_HPP

#include "crunch/concurrency/thread.hpp"
#include "crunch/concurrency/processor_topology.hpp"

#include <bitset>
#include <cstdint>

namespace Crunch { namespace Concurrency {

class ProcessorAffinity
{
public:
    ProcessorAffinity();
    ProcessorAffinity(std::uint32_t processorId);
    ProcessorAffinity(ProcessorTopology::Processor const& processor);
    ProcessorAffinity(ProcessorTopology::ProcessorList const& processors);
    
    void Set(std::uint32_t processorId);
    void Set(ProcessorTopology::Processor const& processor);
    void Set(ProcessorTopology::ProcessorList const& processors);

    void Clear(std::uint32_t processorId);
    void Clear(ProcessorTopology::Processor const& processor);
    void Clear(ProcessorTopology::ProcessorList const& processors);

    bool IsSet(std::uint32_t processorId) const;

    bool IsEmpty() const;

    std::uint32_t GetHighestSetProcessor() const;

private:
    static const std::uint32_t MaxProcessorId = 31;
    typedef std::bitset<MaxProcessorId+1> MaskType;
    MaskType mProcessorMask;
};

ProcessorAffinity operator ~ (ProcessorAffinity affinity);
ProcessorAffinity operator | (ProcessorAffinity lhs, ProcessorAffinity rhs);
ProcessorAffinity operator & (ProcessorAffinity lhs, ProcessorAffinity rhs);
ProcessorAffinity operator ^ (ProcessorAffinity lhs, ProcessorAffinity rhs);
ProcessorAffinity& operator |= (ProcessorAffinity& lhs, ProcessorAffinity rhs);
ProcessorAffinity& operator &= (ProcessorAffinity& lhs, ProcessorAffinity rhs);
ProcessorAffinity& operator ^= (ProcessorAffinity& lhs, ProcessorAffinity rhs);


/// \return Old affinity
/*
ProcessorAffinity SetThreadAffinity(ThreadId thread, ProcessorAffinity const& affinity);
void SetProcessAffinity(ProcessId process, ProcessorAffinity const& affinity);
ProcessorAffinity GetProcessAffinity(ProcessId process);
*/

ProcessorAffinity SetCurrentThreadAffinity(ProcessorAffinity const& affinity);
void SetCurrentProcessAffinity(ProcessorAffinity const& affinity);
ProcessorAffinity GetCurrentProcessAffinity();

}}

#endif
