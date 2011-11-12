// Copyright (c) 2011, Christian Rorvik
// Distributed under the Simplified BSD License (See accompanying file LICENSE.txt)

#ifndef CRUNCH_CONCURRENCY_PROCESSOR_TOPOLOGY_HPP
#define CRUNCH_CONCURRENCY_PROCESSOR_TOPOLOGY_HPP

#include "crunch/base/stdint.hpp"

#include <vector>

namespace Crunch { namespace Concurrency {

uint32 GetSystemNumProcessors();

class ProcessorTopology
{
public:
    struct Processor
    {
        uint32 systemId;  ///> Identifier used by the system for this processor. E.g., for processor affinity.
        uint32 threadId;  ///> Thread ID within the the core
        uint32 coreId;    ///> Core ID within the package
        uint32 packageId; ///> Package ID within the system
    };

    typedef std::vector<Processor> ProcessorList;

    ProcessorTopology();

    ProcessorList const& GetProcessors() const { return mProcessors; }

    ProcessorList GetProcessorsOnCore(uint32 coreId);
    ProcessorList GetProcessorsOnPackage(uint32 packageId);

private:
    ProcessorList mProcessors;
};

}}

#endif

