// Copyright (c) 2011, Christian Rorvik
// Distributed under the Simplified BSD License (See accompanying file LICENSE.txt)

#ifndef CRUNCH_CONCURRENCY_PROCESSOR_AFFINITY_HPP
#define CRUNCH_CONCURRENCY_PROCESSOR_AFFINITY_HPP

#include "crunch/concurrency/api.hpp"
#include "crunch/concurrency/thread.hpp"
#include "crunch/concurrency/processor_topology.hpp"

#include <bitset>
#include <cstdint>

namespace Crunch { namespace Concurrency {

class ProcessorAffinity
{
public:
    CRUNCH_CONCURRENCY_API ProcessorAffinity();
    CRUNCH_CONCURRENCY_API ProcessorAffinity(std::uint32_t processorId);
    CRUNCH_CONCURRENCY_API ProcessorAffinity(ProcessorTopology::Processor const& processor);
    CRUNCH_CONCURRENCY_API ProcessorAffinity(ProcessorTopology::ProcessorList const& processors);
    
    CRUNCH_CONCURRENCY_API void Set(std::uint32_t processorId);
    CRUNCH_CONCURRENCY_API void Set(ProcessorTopology::Processor const& processor);
    CRUNCH_CONCURRENCY_API void Set(ProcessorTopology::ProcessorList const& processors);

    CRUNCH_CONCURRENCY_API void Clear(std::uint32_t processorId);
    CRUNCH_CONCURRENCY_API void Clear(ProcessorTopology::Processor const& processor);
    CRUNCH_CONCURRENCY_API void Clear(ProcessorTopology::ProcessorList const& processors);

    CRUNCH_CONCURRENCY_API bool IsSet(std::uint32_t processorId) const;

    CRUNCH_CONCURRENCY_API bool IsEmpty() const;

    CRUNCH_CONCURRENCY_API std::uint32_t GetHighestSetProcessor() const;

private:
    static const std::uint32_t MaxProcessorId = 31;
    typedef std::bitset<MaxProcessorId+1> MaskType;
    MaskType mProcessorMask;
};

CRUNCH_CONCURRENCY_API ProcessorAffinity operator ~ (ProcessorAffinity affinity);
CRUNCH_CONCURRENCY_API ProcessorAffinity operator | (ProcessorAffinity lhs, ProcessorAffinity rhs);
CRUNCH_CONCURRENCY_API ProcessorAffinity operator & (ProcessorAffinity lhs, ProcessorAffinity rhs);
CRUNCH_CONCURRENCY_API ProcessorAffinity operator ^ (ProcessorAffinity lhs, ProcessorAffinity rhs);
CRUNCH_CONCURRENCY_API ProcessorAffinity& operator |= (ProcessorAffinity& lhs, ProcessorAffinity rhs);
CRUNCH_CONCURRENCY_API ProcessorAffinity& operator &= (ProcessorAffinity& lhs, ProcessorAffinity rhs);
CRUNCH_CONCURRENCY_API ProcessorAffinity& operator ^= (ProcessorAffinity& lhs, ProcessorAffinity rhs);


/// \return Old affinity
/*
ProcessorAffinity SetThreadAffinity(ThreadId thread, ProcessorAffinity const& affinity);
void SetProcessAffinity(ProcessId process, ProcessorAffinity const& affinity);
ProcessorAffinity GetProcessAffinity(ProcessId process);
*/

CRUNCH_CONCURRENCY_API ProcessorAffinity SetCurrentThreadAffinity(ProcessorAffinity const& affinity);
CRUNCH_CONCURRENCY_API void SetCurrentProcessAffinity(ProcessorAffinity const& affinity);
CRUNCH_CONCURRENCY_API ProcessorAffinity GetCurrentProcessAffinity();

}}

#endif
