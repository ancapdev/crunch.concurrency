// Copyright (c) 2011, Christian Rorvik
// Distributed under the Simplified BSD License (See accompanying file LICENSE.txt)

#ifndef CRUNCH_CONCURRENCY_THREAD_HPP
#define CRUNCH_CONCURRENCY_THREAD_HPP

#include "crunch/base/platform.hpp"
#include "crunch/base/noncopyable.hpp"
#include "crunch/concurrency/api.hpp"

#if defined (CRUNCH_PLATFORM_WIN32)
#   include "crunch/base/platform/win32/wintypes.hpp"
#elif defined (CRUNCH_PLATFORM_LINUX)
#   include <pthread.h>
#else
#   error "Unsupported platform"
#endif

#include <functional>
#include <memory>

namespace Crunch { namespace Concurrency {


#if defined (CRUNCH_PLATFORM_WIN32)

class ThreadId
{
public:
    ThreadId() : mId(0) {}

    explicit ThreadId(DWORD id) : mId(id) {}

    bool operator == (ThreadId rhs) const { return mId == rhs.mId; }
    bool operator != (ThreadId rhs) const { return mId != rhs.mId; }

private:
    DWORD mId;
};

#else

class ThreadId
{
public:
    ThreadId() : mSet(false) {}

    explicit ThreadId(pthread_t id) : mId(id), mSet(true) {}

    bool operator == (ThreadId const& rhs) const
    {
        if (!mSet)
            return !rhs.mSet;
        else
            return pthread_equal(mId, rhs.mId);
    }

    bool operator != (ThreadId const& rhs) const
    {
        return !(*this == rhs);
    }

    pthread_t const& GetNative() const { return mId; }

private:
    pthread_t mId;
    bool mSet;
};

#endif


class Thread : NonCopyable
{
public:

#if defined (CRUNCH_PLATFORM_WIN32)
    typedef HANDLE PlatformHandleType;
#else
    typedef pthread_t PlatformHandleType;
#endif

    Thread() {}

    CRUNCH_CONCURRENCY_API ~Thread();

    template<typename F>
    explicit Thread(F f);

    CRUNCH_CONCURRENCY_API Thread(Thread&& rhs);

    CRUNCH_CONCURRENCY_API Thread& operator = (Thread&& rhs);

    CRUNCH_CONCURRENCY_API ThreadId GetId() const;

    CRUNCH_CONCURRENCY_API bool IsJoinable() const;

    CRUNCH_CONCURRENCY_API void Join();

    CRUNCH_CONCURRENCY_API void Detach();

    CRUNCH_CONCURRENCY_API void Cancel();

    CRUNCH_CONCURRENCY_API bool IsCancellationRequested() const;

    CRUNCH_CONCURRENCY_API PlatformHandleType GetPlatformHandle() const;

    CRUNCH_CONCURRENCY_API void operator () ();

private:
    friend CRUNCH_CONCURRENCY_API void SetThreadCancellationPolicy(bool);
    friend CRUNCH_CONCURRENCY_API void ThreadCancellationPoint();
    friend CRUNCH_CONCURRENCY_API bool IsThreadCancellationEnabled();
    friend CRUNCH_CONCURRENCY_API bool IsThreadCancellationRequested();

    struct Data;
    typedef std::shared_ptr<Data> DataPtr;

    CRUNCH_CONCURRENCY_API void Create(std::function<void ()>&& f);

    DataPtr mData;
};

CRUNCH_CONCURRENCY_API ThreadId GetThreadId();

CRUNCH_CONCURRENCY_API void SetThreadCancellationPolicy(bool enableCancellation);

CRUNCH_CONCURRENCY_API void ThreadCancellationPoint();

CRUNCH_CONCURRENCY_API bool IsThreadCancellationEnabled();

CRUNCH_CONCURRENCY_API bool IsThreadCancellationRequested();

template<bool EnableCancellation>
class ScopedThreadCancellationPolicy : NonCopyable
{
public:
    explicit ScopedThreadCancellationPolicy()
        : mPreviousState(IsThreadCancellationEnabled())
    {
        SetThreadCancellationPolicy(EnableCancellation);
    }

    ~ScopedThreadCancellationPolicy()
    {
        SetThreadCancellationPolicy(mPreviousState);
    }

private:
    bool mPreviousState;
};

typedef ScopedThreadCancellationPolicy<true> ScopedEnableThreadCancellation;
typedef ScopedThreadCancellationPolicy<false> ScopedDiableThreadCancellation;

template<typename F>
Thread::Thread(F f)
{
    Create(std::function<void ()>(f));
}

inline Thread::Thread(Thread&& rhs)
    : mData(std::move(rhs.mData))
{}

inline Thread& Thread::operator = (Thread&& rhs)
{
    mData = std::move(rhs.mData);
    return *this;
}

inline bool Thread::IsJoinable() const
{
    return !!mData;
}

inline void Thread::operator () ()
{
    Join();
}

}}

#endif
