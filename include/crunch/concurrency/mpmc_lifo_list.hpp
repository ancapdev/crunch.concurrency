// Copyright (c) 2011, Christian Rorvik
// Distributed under the Simplified BSD License (See accompanying file LICENSE.txt)

#ifndef CRUNCH_CONCURRENCY_MPMC_LIFO_LIST_HPP
#define CRUNCH_CONCURRENCY_MPMC_LIFO_LIST_HPP

#include "crunch/base/stdint.hpp"
#include "crunch/concurrency/atomic.hpp"
#include "crunch/concurrency/exponential_backoff.hpp"

namespace Crunch { namespace Concurrency {

struct ReclamationPolicyNone;
struct ReclamationPolicyAccessViolationChecked;

/// The following functions must be available by ADL
///     void SetNext(T&, T*)
///     T* GetNext(T const&)
//     
// TODO: Memory reclamation policy
template<typename T, typename BackoffPolicy = ExponentialBackoff, typename ReclamationPolicy = ReclamationPolicyAccessViolationChecked>
class MPMCLifoList
{
public:
    MPMCLifoList()
        : mRoot(0, MEMORY_ORDER_RELEASE)
    {}
    
    void Push(T* node)
    {
        BackoffPolicy backoff;
        uint64 oldRoot = mRoot.Load(MEMORY_ORDER_RELAXED);
        uint64 const newRootPtrAndAddend = reinterpret_cast<uint64>(node) + ABA_ADDEND;

        for (;;)
        {
            // Set next pointer in node
            SetNext(*node, reinterpret_cast<T*>(oldRoot & PTR_MASK));

            // Try to update current root node
            uint64 const newRoot = newRootPtrAndAddend + (oldRoot & ABA_MASK);
            if (mRoot.CompareAndSwap(newRoot, oldRoot, MEMORY_ORDER_RELEASE))
                break;

            backoff.Pause();
        }
    }

    T* Pop()
    {
        BackoffPolicy backoff;
        uint64 oldRoot = mRoot.Load(MEMORY_ORDER_RELAXED);

        for (;;)
        {
            T* const oldRootPtr = reinterpret_cast<T*>(oldRoot & PTR_MASK);
            if (oldRootPtr == nullptr)
                return nullptr;

            uint64 const newRoot =
                reinterpret_cast<uint64>(GetNext(*oldRootPtr)) +
                (oldRoot & ABA_MASK) + ABA_ADDEND;
            
            if (mRoot.CompareAndSwap(newRoot, oldRoot, MEMORY_ORDER_RELEASE))
                return oldRootPtr;

            backoff.Pause();
        }
    }

private:
#if (CRUNCH_PTR_SIZE == 4)
    static uint64 const PTR_MASK   = 0x00000000ffffffffull;
    static uint64 const ABA_MASK   = 0xffffffff00000000ull;
    static uint64 const ABA_ADDEND = 0x0000000100000000ull;
#else
    // Assume 48 bit address space
    static uint64 const PTR_MASK   = 0x0000ffffffffffffull;
    static uint64 const ABA_MASK   = 0xffff000000000000ull;
    static uint64 const ABA_ADDEND = 0x0001000000000000ull;
#endif
    Atomic<uint64> mRoot;
};

}}
#endif
