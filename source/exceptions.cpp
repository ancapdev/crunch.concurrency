// Copyright (c) 2011, Christian Rorvik
// Distributed under the Simplified BSD License (See accompanying file LICENSE.txt)

#include "crunch/concurrency/exceptions.hpp"

namespace Crunch { namespace Concurrency {

ThreadCanceled::ThreadCanceled()
    : Exception("Thread canceled")
{}

ThreadResourceError::ThreadResourceError()
    : Exception("Thread resource error")
{}

}}
