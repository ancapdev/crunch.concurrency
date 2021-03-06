// Copyright (c) 2011, Christian Rorvik
// Distributed under the Simplified BSD License (See accompanying file LICENSE.txt)

#include "crunch/concurrency/processor_topology.hpp"
#include "crunch/concurrency/processor_affinity.hpp"
#include "crunch/base/platform.hpp"
#include "crunch/base/bit_utility.hpp"

#if defined (CRUNCH_ARCH_X86)
#   include "crunch/base/arch/x86/cpuid.hpp"
#endif

#include <algorithm>

namespace Crunch { namespace Concurrency {

#if defined (CRUNCH_ARCH_X86)
namespace
{
    ProcessorTopology::ProcessorList EnumerateFromCpuidInitialApicId(std::uint32_t smtBits, std::uint32_t coreBits)
    {
        std::uint32_t allMask = ~std::uint32_t(0);
        std::uint32_t const smtMask = ~(allMask << smtBits);
        std::uint32_t const coreMask = (~(allMask << (coreBits + smtBits))) ^ smtMask;
        std::uint32_t const packageMask = allMask << (coreBits + smtBits);

        const ProcessorAffinity processAffinity = GetCurrentProcessAffinity();
        const std::uint32_t highest = processAffinity.GetHighestSetProcessor();

        ProcessorTopology::ProcessorList processors;

        for (std::uint32_t i = 0; i <= highest; ++i)
        {
            if (!processAffinity.IsSet(i))
                continue;

            ProcessorAffinity const oldAffinity = SetCurrentThreadAffinity(ProcessorAffinity(i));

            std::uint32_t const initialApicId = ExtractBits(QueryCpuid(1).ebx, 24, 31);
            std::uint32_t const threadId = initialApicId & smtMask;
            std::uint32_t const coreId = (initialApicId & coreMask) >> smtBits;
            std::uint32_t const packageId = (initialApicId & packageMask) >> (coreBits + smtBits);

            ProcessorTopology::Processor const processor = { i, threadId, coreId, packageId };
            processors.push_back(processor);

            SetCurrentThreadAffinity(oldAffinity);
        }

        return processors;
    }

    ProcessorTopology::ProcessorList EnumerateFromCpuidx2ApicId()
    {
        const ProcessorAffinity processAffinity = GetCurrentProcessAffinity();
        const std::uint32_t highest = processAffinity.GetHighestSetProcessor();

        ProcessorTopology::ProcessorList processors;

        for (std::uint32_t i = 0; ; ++i)
        {
            CpuidResult const res = QueryCpuid(11, i);
            if (ExtractBits(res.ebx, 0, 15) == 0)
                break;
        }

        for (std::uint32_t i = 0; i <= highest; ++i)
        {
            if (!processAffinity.IsSet(i))
                continue;

            ProcessorAffinity const oldAffinity = SetCurrentThreadAffinity(ProcessorAffinity(i));

            std::uint32_t const allMask = ~std::uint32_t(0);
            std::uint32_t const smtBits = ExtractBits(QueryCpuid(11).eax, 0, 4);
            std::uint32_t const smtMask = ~(allMask << smtBits);
            std::uint32_t const coreAndSmtBits = ExtractBits(QueryCpuid(11, 1).eax, 0, 4);
            std::uint32_t const coreMask = ~(allMask << coreAndSmtBits) ^ smtMask;
            std::uint32_t const packageMask = allMask << coreAndSmtBits;

            std::uint32_t const x2apic = QueryCpuid(11).edx;
            std::uint32_t const threadId = x2apic & smtMask;
            std::uint32_t const coreId = (x2apic & coreMask) >> smtBits;
            std::uint32_t const packageId = (x2apic & packageMask) >> (coreAndSmtBits);

            ProcessorTopology::Processor const processor = { i, threadId, coreId, packageId };
            processors.push_back(processor);

            SetCurrentThreadAffinity(oldAffinity);
        }

        return processors;
    }

    ProcessorTopology::ProcessorList EnumerateFromCpuidIntel()
    {
        // Reference: http://software.intel.com/en-us/articles/intel-64-architecture-processor-topology-enumeration/

        if (GetCpuidMaxFunction() >= 11 &&
            QueryCpuid(11).ebx != 0)
        {
            // Enumerate using leaf 11
            return EnumerateFromCpuidx2ApicId();
        }
        else if (GetCpuidMaxFunction() >= 4)
        {
            // Enumerate using leaf 1 and 4
            std::uint32_t const maxNumLogicalPerPackage = ExtractBits(QueryCpuid(1).ebx, 16, 23);
            std::uint32_t const maxNumPhysicalPerPackage = ExtractBits(QueryCpuid(4).eax, 26, 31) + 1;
            std::uint32_t const smtBits = Log2Floor(Pow2Ceil(maxNumLogicalPerPackage) / maxNumPhysicalPerPackage);
            std::uint32_t const coreBits = Log2Floor(maxNumPhysicalPerPackage);
            return EnumerateFromCpuidInitialApicId(smtBits, coreBits);
        }
        else
        {
            return ProcessorTopology::ProcessorList();
        }
    }

    ProcessorTopology::ProcessorList EnumerateFromCpuidAmd()
    {
        std::uint32_t const maxLogicalPerPackage = ExtractBits(QueryCpuid(1).ebx, 16, 23);
        if (QueryCpuid(CpuidFunction::HighestExtendedFunction).eax)
        {
            std::uint32_t const coreBits = ExtractBits(QueryCpuid(0x80000008ul).ecx, 12, 15);
            std::uint32_t const smtBits = (maxLogicalPerPackage >> coreBits) == 0 ? 0 : Log2Ceil(maxLogicalPerPackage >> coreBits);
            return EnumerateFromCpuidInitialApicId(smtBits, coreBits);
        }
        else
        {
            std::uint32_t const coreBits = Log2Ceil(maxLogicalPerPackage);
            return EnumerateFromCpuidInitialApicId(0, coreBits);
        }
    }

    ProcessorTopology::ProcessorList EnumerateFromCpuid()
    {
        // If HTT flag is not set, this is a single thread processor. Let fallback handle enumeration
        if ((QueryCpuid(1).edx & (1ul << 28)) == 0)
            return ProcessorTopology::ProcessorList();
        
        std::string const vendor = GetCpuidVendorId();
        if (vendor == "GenuineIntel")
            return EnumerateFromCpuidIntel();
        else if (vendor == "AuthenticAMD")
            return EnumerateFromCpuidAmd();
        else
            return ProcessorTopology::ProcessorList();
    }
}
#endif

ProcessorTopology::ProcessorTopology()
#if defined (CRUNCH_ARCH_X86)
    : mProcessors(EnumerateFromCpuid())
#endif
{
    // If we failed to enumerate in a system specific way, assume we have 1 core per system processor on a single package
    if (mProcessors.empty())
    {
        std::uint32_t numProcessors = GetSystemNumProcessors();
        for (std::uint32_t i = 0; i < numProcessors; ++i)
        {
            Processor const p = { i, 0, i, 0 };
            mProcessors.push_back(p);
        }
    }
}

}}
