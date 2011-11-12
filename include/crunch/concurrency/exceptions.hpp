// Copyright (c) 2011, Christian Rorvik
// Distributed under the Simplified BSD License (See accompanying file LICENSE.txt)

#ifndef CRUNCH_CONCURRENCY_EXCEPTIONS_HPP
#define CRUNCH_CONCURRENCY_EXCEPTIONS_HPP

#include "crunch/base/exception.hpp"

namespace Crunch { namespace Concurrency {

class ThreadCanceled : public Exception
{
public:
    ThreadCanceled();
};

class ThreadResourceError : public Exception
{
public:
    ThreadResourceError();
};

}}

#endif