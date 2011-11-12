// Copyright (c) 2011, Christian Rorvik
// Distributed under the Simplified BSD License (See accompanying file LICENSE.txt)

#include "./thread_data.hpp"

namespace Crunch { namespace Concurrency {

CRUNCH_THREAD_LOCAL Thread::Data* Thread::Data::tCurrent = NULL;

#if defined (CRUNCH_PLATFORM_WIN32)
DWORD WINAPI Thread::Data::EntryPoint(void* argument)
#else
void* Thread::Data::EntryPoint(void* argument)
#endif
{
    DataPtr data = reinterpret_cast<Data*>(argument)->self;
    data->self.reset();

    tCurrent = data.get();

    try
    {
        data->userEntryPoint();
    }
    catch (...)
    {
        std::terminate();
    }

    return 0;
}


}}
