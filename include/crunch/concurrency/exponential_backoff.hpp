// Copyright (c) 2011, Christian Rorvik
// Distributed under the Simplified BSD License (See accompanying file LICENSE.txt)

#ifndef CRUNCH_CONCURRENCY_EXPONENTIAL_BACKOFF_HPP
#define CRUNCH_CONCURRENCY_EXPONENTIAL_BACKOFF_HPP

#include "crunch/concurrency/yield.hpp"

namespace Crunch { namespace Concurrency {

template<int PauseLimit>
class ExponentialBackoffT
{
public:
    ExponentialBackoffT() : mCount(1) {}

    void Pause()
    {
        if (mCount <= mLimit)
        {
            Concurrency::Pause(mCount);
            mCount *= 2;
        }
        else
        {
            ThreadYield();
        }
    }

    bool TryPause()
    {
        if (mCount <= mLimit)
        {
            Concurrency::Pause(mCount);
            mCount *= 2;
            return true;
        }
        else
        {
            return false;
        }
    }
    
    void Reset()
    {
        mCount = 1;
    }

private:
    int mCount;
    static int const mLimit = PauseLimit;
};

typedef ExponentialBackoffT<16> ExponentialBackoff;

}}

#endif
