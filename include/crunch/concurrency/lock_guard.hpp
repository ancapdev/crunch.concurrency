// Copyright (c) 2011, Christian Rorvik
// Distributed under the Simplified BSD License (See accompanying file LICENSE.txt)

#ifndef CRUNCH_CONCURRENCY_LOCK_GUARD_HPP
#define CRUNCH_CONCURRENCY_LOCK_GUARD_HPP

#include "crunch/base/noncopyable.hpp"

namespace Crunch { namespace Concurrency {

template<typename LockableType>
class LockGuard : NonCopyable
{
public:
    LockGuard(LockableType& lockable)
        : mLockable(lockable)
    {
        mLockable.Lock();
    }

    ~LockGuard()
    {
        mLockable.Unlock();
    }

private:
    LockableType& mLockable;
};

}}

#endif
