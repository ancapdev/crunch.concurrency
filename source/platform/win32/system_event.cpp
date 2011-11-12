// Copyright (c) 2011, Christian Rorvik
// Distributed under the Simplified BSD License (See accompanying file LICENSE.txt)

#include "crunch/concurrency/detail/system_event.hpp"
#include "crunch/base/assert.hpp"

#include <windows.h>

namespace Crunch { namespace Concurrency { namespace Detail {

SystemEvent::SystemEvent(bool initialState)
    : mEvent(CreateEvent(NULL, TRUE, initialState ? TRUE : FALSE, NULL))
{}

SystemEvent::~SystemEvent()
{
    CloseHandle(mEvent);
}

void SystemEvent::Set()
{
    BOOL result = SetEvent(mEvent);
    CRUNCH_ASSERT_ALWAYS(result == TRUE);
}

void SystemEvent::Reset()
{
    BOOL result = ResetEvent(mEvent);
    CRUNCH_ASSERT_ALWAYS(result == TRUE);
}

void SystemEvent::Wait()
{
    DWORD result = WaitForSingleObject(mEvent, INFINITE);
    CRUNCH_ASSERT_ALWAYS(result == WAIT_OBJECT_0);
}


}}}
