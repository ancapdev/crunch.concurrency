// Copyright (c) 2011, Christian Rorvik
// Distributed under the Simplified BSD License (See accompanying file LICENSE.txt)

#ifndef CRUNCH_CONCURRENCY_MPMC_LIFO_LIST_HPP
#define CRUNCH_CONCURRENCY_MPMC_LIFO_LIST_HPP

#include "crunch/concurrency/atomic.hpp"
#include "crunch/concurrency/exponential_backoff.hpp"

#include <cstdint>

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
        std::uint64_t oldRoot = mRoot.Load(MEMORY_ORDER_RELAXED);
        std::uint64_t const newRootPtrAndAddend = reinterpret_cast<std::uint64_t>(node) + ABA_ADDEND;

        for (;;)
        {
            // Set next pointer in node
            SetNext(*node, reinterpret_cast<T*>(oldRoot & PTR_MASK));

            // Try to update current root node
            std::uint64_t const newRoot = newRootPtrAndAddend + (oldRoot & ABA_MASK);
            if (mRoot.CompareAndSwap(oldRoot, newRoot, MEMORY_ORDER_RELEASE))
                break;

            backoff.Pause();
        }
    }

    T* Pop()
    {
        BackoffPolicy backoff;
        std::uint64_t oldRoot = mRoot.Load(MEMORY_ORDER_RELAXED);

        for (;;)
        {
            T* const oldRootPtr = reinterpret_cast<T*>(oldRoot & PTR_MASK);
            if (oldRootPtr == nullptr)
                return nullptr;

            std::uint64_t const newRoot =
                reinterpret_cast<std::uint64_t>(GetNext(*oldRootPtr)) +
                (oldRoot & ABA_MASK) + ABA_ADDEND;
            
            if (mRoot.CompareAndSwap(oldRoot, newRoot, MEMORY_ORDER_RELEASE))
                return oldRootPtr;

            backoff.Pause();
        }
    }

private:
#if (CRUNCH_PTR_SIZE == 4)
    static std::uint64_t const PTR_MASK   = 0x00000000ffffffffull;
    static std::uint64_t const ABA_MASK   = 0xffffffff00000000ull;
    static std::uint64_t const ABA_ADDEND = 0x0000000100000000ull;
#else
    // Assume 48 bit address space
    static std::uint64_t const PTR_MASK   = 0x0000ffffffffffffull;
    static std::uint64_t const ABA_MASK   = 0xffff000000000000ull;
    static std::uint64_t const ABA_ADDEND = 0x0001000000000000ull;
#endif
    Atomic<std::uint64_t> mRoot;
};

}}
#endif
