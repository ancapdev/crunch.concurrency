// Copyright (c) 2011, Christian Rorvik
// Distributed under the Simplified BSD License (See accompanying file LICENSE.txt)

#ifndef CRUNCH_CONCURRENCY_VERSIONED_DATA_HPP
#define CRUNCH_CONCURRENCY_VERSIONED_DATA_HPP

#include "crunch/concurrency/detail/system_mutex.hpp"

#include <cstdint>

namespace Crunch { namespace Concurrency {

template<typename T>
class VersionedData
{
public:
    VersionedData(std::uint32_t initialVersion = 0)
        : mVersion(initialVersion)
    {}

    bool HasChanged(std::uint32_t lastSeenVersion)
    {
        return lastSeenVersion != mVersion;
    }

    template<typename F>
    void Update(F f)
    {
        Detail::SystemMutex::ScopedLock const lock(mLock);
        f(mData);
        mVersion++;
    }

    template<typename F>
    void ReadIfDifferent(std::uint32_t& localVersion, F f) const
    {
        if (localVersion != mVersion)
        {
            Detail::SystemMutex::ScopedLock const lock(mLock);
            f(mData);
            localVersion = mVersion;
        }
    }

private:
    mutable Detail::SystemMutex mLock;
    volatile std::uint32_t mVersion;
    T mData;
};

}}

#endif
