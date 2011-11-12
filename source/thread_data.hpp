// Copyright (c) 2011, Christian Rorvik
// Distributed under the Simplified BSD License (See accompanying file LICENSE.txt)

#ifndef CRUNCH_CONCURRENCY_SOURCE_THREAD_DATA_HPP
#define CRUNCH_CONCURRENCY_SOURCE_THREAD_DATA_HPP

#include "crunch/concurrency/thread.hpp"
#include "crunch/concurrency/thread_local.hpp"

#include <memory>

#if defined (CRUNCH_PLATFORM_WIN32)
#   include <windows.h>
#endif

namespace Crunch { namespace Concurrency {

struct Thread::Data
{
    Data(std::function<void ()>&& f)
        : self()
        , id()
#if defined (CRUNCH_PLATFORM_WIN32)
        , handle(NULL)
#endif
        , userEntryPoint(std::move(f))
        , cancellationRequested(false)
        , cancellationEnabled(true)
        , canceled(false)
    {}

#if defined (CRUNCH_PLATFORM_WIN32)
    static DWORD WINAPI EntryPoint(void* argument);
#else
    static void* EntryPoint(void* argument);
#endif

    DataPtr self;
    ThreadId id;
#if defined (CRUNCH_PLATFORM_WIN32)
    HANDLE handle;
#endif
    std::function<void ()> userEntryPoint;
    bool cancellationRequested;
    bool cancellationEnabled;
    bool canceled;

    static CRUNCH_THREAD_LOCAL Thread::Data* tCurrent;
};

}}

#endif
