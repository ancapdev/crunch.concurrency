// Copyright (c) 2011, Christian Rorvik
// Distributed under the Simplified BSD License (See accompanying file LICENSE.txt)

#ifndef CRUNCH_CONCURRENCY_PROCESSOR_AFFINITY_HPP
#define CRUNCH_CONCURRENCY_PROCESSOR_AFFINITY_HPP

#include "crunch/base/stdint.hpp"
#include "crunch/concurrency/thread.hpp"
#include "crunch/concurrency/processor_topology.hpp"

#include <bitset>

namespace Crunch { namespace Concurrency {

class ProcessorAffinity
{
public:
    ProcessorAffinity();
    ProcessorAffinity(uint32 processorId);
    ProcessorAffinity(ProcessorTopology::Processor const& processor);
    ProcessorAffinity(ProcessorTopology::ProcessorList const& processors);
    
    void Set(uint32 processorId);
    void Set(ProcessorTopology::Processor const& processor);
    void Set(ProcessorTopology::ProcessorList const& processors);

    void Clear(uint32 processorId);
    void Clear(ProcessorTopology::Processor const& processor);
    void Clear(ProcessorTopology::ProcessorList const& processors);

    bool IsSet(uint32 processorId) const;

    bool IsEmpty() const;

    uint32 GetHighestSetProcessor() const;

private:
    static const uint32 MaxProcessorId = 31;
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
