// Copyright (c) 2011, Christian Rorvik
// Distributed under the Simplified BSD License (See accompanying file LICENSE.txt)

#ifndef CRUNCH_CONCURRENCY_THREAD_HPP
#define CRUNCH_CONCURRENCY_THREAD_HPP

#include "crunch/base/platform.hpp"
#include "crunch/base/noncopyable.hpp"

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

    ~Thread();

    template<typename F>
    explicit Thread(F f);

    Thread(Thread&& rhs);

    Thread& operator = (Thread&& rhs);

    ThreadId GetId() const;

    bool IsJoinable() const;

    void Join();

    void Detach();

    void Cancel();

    bool IsCancellationRequested() const;

    PlatformHandleType GetPlatformHandle() const;

    void operator () ();

private:
    friend void SetThreadCancellationPolicy(bool);
    friend void ThreadCancellationPoint();
    friend bool IsThreadCancellationEnabled();
    friend bool IsThreadCancellationRequested();

    struct Data;
    typedef std::shared_ptr<Data> DataPtr;

    void Create(std::function<void ()>&& f);

    DataPtr mData;
};

ThreadId GetThreadId();

void SetThreadCancellationPolicy(bool enableCancellation);

void ThreadCancellationPoint();

bool IsThreadCancellationEnabled();

bool IsThreadCancellationRequested();

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
