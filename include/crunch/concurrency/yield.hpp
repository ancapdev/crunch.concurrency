// Copyright (c) 2011, Christian Rorvik
// Distributed under the Simplified BSD License (See accompanying file LICENSE.txt)

#ifndef CRUNCH_CONCURRENCY_YIELD_HPP
#define CRUNCH_CONCURRENCY_YIELD_HPP

#include "crunch/base/platform.hpp"
#include "crunch/base/duration.hpp"

#if defined (CRUNCH_ARCH_X86)
#   if defined (CRUNCH_COMPILER_MSVC) && defined (CRUNCH_ARCH_X86)
        extern "C" void _mm_pause();
#       pragma intrinsic(_mm_pause)
#       define CRUNCH_PAUSE() _mm_pause()
#   elif defined (CRUNCH_COMPILER_GCC)
#       define CRUNCH_PAUSE() __asm__ __volatile__("pause;");
#   endif
#endif

namespace Crunch { namespace Concurrency {

/// Yield scheduler to another thread
void ThreadYield();

/// Sleep the thread
void ThreadSleep(Duration duration);

#if defined (CRUNCH_ARCH_X86)
inline void Pause(int count)
{
    for (int i = 0; i < count; ++i)
        CRUNCH_PAUSE();
}
#endif

}}

#endif
