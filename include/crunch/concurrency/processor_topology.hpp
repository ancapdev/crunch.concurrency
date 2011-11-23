// Copyright (c) 2011, Christian Rorvik
// Distributed under the Simplified BSD License (See accompanying file LICENSE.txt)

#ifndef CRUNCH_CONCURRENCY_PROCESSOR_TOPOLOGY_HPP
#define CRUNCH_CONCURRENCY_PROCESSOR_TOPOLOGY_HPP

#include <cstdint>
#include <vector>

namespace Crunch { namespace Concurrency {

std::uint32_t GetSystemNumProcessors();

class ProcessorTopology
{
public:
    struct Processor
    {
        std::uint32_t systemId;  ///> Identifier used by the system for this processor. E.g., for processor affinity.
        std::uint32_t threadId;  ///> Thread ID within the the core
        std::uint32_t coreId;    ///> Core ID within the package
        std::uint32_t packageId; ///> Package ID within the system
    };

    typedef std::vector<Processor> ProcessorList;

    ProcessorTopology();

    ProcessorList const& GetProcessors() const { return mProcessors; }

    ProcessorList GetProcessorsOnCore(std::uint32_t coreId);
    ProcessorList GetProcessorsOnPackage(std::uint32_t packageId);

private:
    ProcessorList mProcessors;
};

}}

#endif

