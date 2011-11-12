// Copyright (c) 2011, Christian Rorvik
// Distributed under the Simplified BSD License (See accompanying file LICENSE.txt)

#ifndef CRUNCH_CONCURRENCY_PLATFORM_LINUX_ATOMIC_HPP
#define CRUNCH_CONCURRENCY_PLATFORM_LINUX_ATOMIC_HPP

#include "crunch/base/platform.hpp"

#include "crunch/concurrency/platform/linux/atomic_word.hpp"
#include "crunch/concurrency/platform/linux/atomic_storage.hpp"

#if defined (CRUNCH_ARCH_X86)
#   include "crunch/concurrency/platform/linux/atomic_ops_x86.hpp"
#endif

#endif
