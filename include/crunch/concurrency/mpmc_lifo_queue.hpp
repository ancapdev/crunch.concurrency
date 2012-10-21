// Copyright (c) 2012, Christian Rorvik
// Distributed under the Simplified BSD License (See accompanying file LICENSE.txt)

#ifndef CRUNCH_CONCURRENCY_MPMC_LIFO_QUEUE_HPP
#define CRUNCH_CONCURRENCY_MPMC_LIFO_QUEUE_HPP

#include "crunch/concurrency/exponential_backoff.hpp"
#include "crunch/concurrency/mpmc_lifo_list.hpp"

#include <type_traits>
#include <utility>

namespace Crunch { namespace Concurrency {

namespace Detail
{
    template<typename T>
    struct MPMCLifoQueueNode
    {
        typedef typename std::aligned_storage<sizeof(T), std::alignment_of<T>::value>::type StorageType;

        StorageType storage;
        MPMCLifoQueueNode* next;
    };

    template<typename T>
    void SetNext(MPMCLifoQueueNode<T>& node, MPMCLifoQueueNode<T>* next)
    {
        node.next = next;
    }

    template<typename T>
    MPMCLifoQueueNode<T>* GetNext(MPMCLifoQueueNode<T> const& node)
    {
        return node.next;
    }
}

template<typename T, typename BackOffPolicy = ExponentialBackoff>
class MPMCLifoQueue
{
public:
    ~MPMCLifoQueue()
    {
        while (Node* node = mQueueNodes.Pop())
            reinterpret_cast<T&>(node->storage).~T();
    }

    template<typename TT>
    void Push(TT&& value)
    {
        Node* node = mFreeNodes.Pop();
        if (!node)
            node = new Node();

        new (&node->storage) T(std::forward<T>(value));
        node->next = nullptr;

        mQueueNodes.Push(node);
    }

    bool TryPop(T& outValue)
    {
        Node* node = mQueueNodes.Pop();
        if (!node)
            return false;

        T& value = reinterpret_cast<T&>(node->storage);
        outValue = std::move(value);
        value.~T();
        mFreeNodes.Push(node);
    }

private:
    typedef Detail::MPMCLifoQueueNode<T> Node;
    typedef MPMCLifoList<Node, BackOffPolicy, ReclamationPolicyNone> List;

    List mQueueNodes;
    List mFreeNodes;
};

}}

#endif
