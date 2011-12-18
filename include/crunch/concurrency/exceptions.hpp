// Copyright (c) 2011, Christian Rorvik
// Distributed under the Simplified BSD License (See accompanying file LICENSE.txt)

#ifndef CRUNCH_CONCURRENCY_EXCEPTIONS_HPP
#define CRUNCH_CONCURRENCY_EXCEPTIONS_HPP

#include "crunch/base/exception.hpp"
#include "crunch/concurrency/api.hpp"

namespace Crunch { namespace Concurrency {

class CRUNCH_CONCURRENCY_API ThreadCanceled : public Exception
{
public:
    ThreadCanceled();
};

class CRUNCH_CONCURRENCY_API ThreadResourceError : public Exception
{
public:
    ThreadResourceError();
};

}}

#endif