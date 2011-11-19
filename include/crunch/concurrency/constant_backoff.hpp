// Copyright (c) 2011, Christian Rorvik
// Distributed under the Simplified BSD License (See accompanying file LICENSE.txt)

#ifndef CRUNCH_CONCURRENCY_CONSTANT_BACKOFF_HPP
#define CRUNCH_CONCURRENCY_CONSTANT_BACKOFF_HPP

#include "crunch/concurrency/yield.hpp"

namespace Crunch { namespace Concurrency {

template<int PauseLimit>
class ConstantBackoffT
{
public:
    void Pause()
    {
        Concurrency::Pause(PauseLimit);
    }

    bool TryPause()
    {
        Pause();
        return true;
    }

    void Reset() {}
};

typedef ConstantBackoffT<1> ConstantBackoff;

}}

#endif
