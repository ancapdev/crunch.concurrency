// Copyright (c) 2011, Christian Rorvik
// Distributed under the Simplified BSD License (See accompanying file LICENSE.txt)

#ifndef CRUNCH_CONCURRENCY_THREAD_LOCAL_HPP
#define CRUNCH_CONCURRENCY_THREAD_LOCAL_HPP

#include "crunch/base/platform.hpp"

#if defined (CRUNCH_COMPILER_MSVC)
#   define CRUNCH_THREAD_LOCAL __declspec(thread)
#elif defined (CRUNCH_COMPILER_GCC)
#   define CRUNCH_THREAD_LOCAL __thread
#else
#   error "Unsupport compiler"
#endif

#endif