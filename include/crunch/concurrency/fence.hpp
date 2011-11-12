// Copyright (c) 2011, Christian Rorvik
// Distributed under the Simplified BSD License (See accompanying file LICENSE.txt)

#ifndef CRUNCH_CONCURRENCY_FENCE_HPP
#define CRUNCH_CONCURRENCY_FENCE_HPP

#include "crunch/base/platform.hpp"

#if defined (CRUNCH_PLATFORM_WIN32)
    extern "C" void _ReadWriteBarrier();
#   pragma intrinsic(_ReadWriteBarrier)
#   define CRUNCH_COMPILER_FENCE() _ReadWriteBarrier()
#   if defined (CRUNCH_ARCH_X86_32)
#        define CRUNCH_MEMORY_FENCE() do { long barrier; __asm { xchg barrier, eax } } while(0)
#   elif defined (CRUNCH_ARCH_X86_64)
#       define CRUNCH_MEMORY_FENCE() __faststorefence()
#   else
#       include <Windows.h>
#       define CRUNCH_MEMORY_FENCE() MemoryBarrier()
#   endif
#elif defined (CRUNCH_PLATFORM_LINUX)
#   define CRUNCH_COMPILER_FENCE() asm volatile ("" : : : "memory")
#   define CRUNCH_MEMORY_FENCE() __sync_synchronize()
#else
#   error "Unsupported platform"
#endif

#endif
