// Copyright (c) 2011, Christian Rorvik
// Distributed under the Simplified BSD License (See accompanying file LICENSE.txt)

#include "crunch/concurrency/processor_affinity.hpp"
#include "crunch/base/assert.hpp"

#include <windows.h>

namespace Crunch { namespace Concurrency {

namespace 
{
    DWORD_PTR MaskFromAffinity(ProcessorAffinity const& affinity)
    {
        DWORD_PTR mask = 0;

        uint32 highest = affinity.GetHighestSetProcessor();
        
        for (uint32 p = 0; p <= highest; ++p)
            if (affinity.IsSet(p))
                mask |= DWORD_PTR(1) << p;

        return mask;
    }

    ProcessorAffinity AffinityFromMask(DWORD_PTR mask)
    {
        ProcessorAffinity affinity;

        for (uint32 p = 0; p < (sizeof(DWORD_PTR) * 8); ++p)
        {
            if (mask & (DWORD_PTR(1) << p))
                affinity.Set(p);
        }

        return affinity;
    }
}

ProcessorAffinity SetThreadAffinity(HANDLE thread, ProcessorAffinity const& affinity)
{
    DWORD_PTR result = SetThreadAffinityMask(thread, MaskFromAffinity(affinity));
    CRUNCH_ASSERT_ALWAYS(result != 0);
    return AffinityFromMask(result);
}

void SetProcessAffinity(HANDLE process, ProcessorAffinity const& affinity)
{
    BOOL result = SetProcessAffinityMask(process, MaskFromAffinity(affinity));
    CRUNCH_ASSERT_ALWAYS(result == TRUE);
}

ProcessorAffinity GetProcessAffinity(HANDLE process)
{
    DWORD_PTR processMask;
    DWORD_PTR systemMask;
    BOOL result = GetProcessAffinityMask(process, &processMask, &systemMask);
    CRUNCH_ASSERT_ALWAYS(result == TRUE);
    return AffinityFromMask(processMask);
}

ProcessorAffinity SetCurrentThreadAffinity(ProcessorAffinity const& affinity)
{
    return SetThreadAffinity(GetCurrentThread(), affinity);
}

void SetCurrentProcessAffinity(ProcessorAffinity const& affinity)
{
    SetProcessAffinity(GetCurrentProcess(), affinity);
}

ProcessorAffinity GetCurrentProcessAffinity()
{
    return GetProcessAffinity(GetCurrentProcess());
}

}}
