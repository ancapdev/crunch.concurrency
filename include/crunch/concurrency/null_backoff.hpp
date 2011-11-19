// Copyright (c) 2011, Christian Rorvik
// Distributed under the Simplified BSD License (See accompanying file LICENSE.txt)

#ifndef CRUNCH_CONCURRENCY_NULL_BACKOFF_HPP
#define CRUNCH_CONCURRENCY_NULL_BACKOFF_HPP

namespace Crunch { namespace Concurrency {

class NullBackoff
{
public:
    void Pause() {}
    bool TryPause() { return true; }
    void Reset() {}
};

}}

#endif
