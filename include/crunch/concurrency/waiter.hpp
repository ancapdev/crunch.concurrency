// Copyright (c) 2011, Christian Rorvik
// Distributed under the Simplified BSD License (See accompanying file LICENSE.txt)

#ifndef CRUNCH_CONCURRENCY_WAITER_HPP
#define CRUNCH_CONCURRENCY_WAITER_HPP

#include "crunch/base/assert.hpp"
#include "crunch/base/noncopyable.hpp"
#include "crunch/concurrency/thread_local.hpp"

#include <memory>
#include <type_traits>

namespace Crunch { namespace Concurrency {

class Waiter : NonCopyable
{
public:
    template<typename F>
    class Typed;

    template<typename F>
    static Typed<F>* Create(F callback, bool destroyOnNotify);

    void Notify();

    Waiter* next;

private:
    template<typename F> friend class Typed;

    typedef void (*Callback)(Waiter*);
    typedef std::aligned_storage<16, 8>::type StorageType;

    Waiter(Callback callback);
    ~Waiter() {}

    static void* Allocate();
    static void* AllocateGlobal();
    static void Free(void* allocation);
    static void FreeGlobal(void* allocation);

    Callback mCallback;
    StorageType mStorage;

    static CRUNCH_THREAD_LOCAL Waiter* tWaiterFreeList;
};

template<typename F>
class Waiter::Typed : public Waiter
{
public:
    void Destroy();

private:
    friend class Waiter;

    Typed(Callback callback) : Waiter(callback) {}

    static void Invoke(Waiter* waiter);
    static void Destroy(Waiter* waiter);
    static void InvokeAndDestroy(Waiter* waiter);
};

template<typename F>
Waiter::Typed<F>* Waiter::Create(F callback, bool destroyOnNotify)
{
    Waiter::Typed<F>* waiter = new (Allocate()) Waiter::Typed<F>(destroyOnNotify ? &Waiter::Typed<F>::InvokeAndDestroy : &Waiter::Typed<F>::Invoke);

    if (sizeof(F) > sizeof(StorageType))
        *reinterpret_cast<F**>(&waiter->mStorage) = new F(std::move(callback));
    else
        new (&waiter->mStorage) F(std::move(callback));

    return waiter;
}

template<typename F>
void Waiter::Typed<F>::Destroy()
{
    Destroy(this);
}

template<typename F>
void Waiter::Typed<F>::Invoke(Waiter* waiter)
{
    if (sizeof(F) > sizeof(StorageType))
    {
        (**reinterpret_cast<F**>(&waiter->mStorage))();
    }
    else
        (*reinterpret_cast<F*>(&waiter->mStorage))();
}

template<typename F>
void Waiter::Typed<F>::Destroy(Waiter* waiter)
{
    if (sizeof(F) > sizeof(StorageType))
        delete *reinterpret_cast<F**>(&waiter->mStorage);
    else
        reinterpret_cast<F*>(&waiter->mStorage)->~F();

    Free(waiter);
}

template<typename F>
void Waiter::Typed<F>::InvokeAndDestroy(Waiter* waiter)
{
    Invoke(waiter);
    Destroy(waiter);
}

inline void Waiter::Notify()
{
    mCallback(this);
}

inline Waiter::Waiter(Callback callback)
    : mCallback(callback)
{}

inline void* Waiter::Allocate()
{
    Waiter* localFree = tWaiterFreeList;
    if (localFree)
    {
        tWaiterFreeList = localFree->next;
        return localFree;
    }
    return AllocateGlobal();
}

inline void Waiter::Free(void* allocation)
{
    // TODO: free back to global list
    Waiter* node = reinterpret_cast<Waiter*>(allocation);
    node->next = tWaiterFreeList;
    tWaiterFreeList = node;
}

}}

#endif
