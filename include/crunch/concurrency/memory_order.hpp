// Copyright (c) 2011, Christian Rorvik
// Distributed under the Simplified BSD License (See accompanying file LICENSE.txt)

#ifndef CRUNCH_CONCURRENCY_MEMORY_ORDER_HPP
#define CRUNCH_CONCURRENCY_MEMORY_ORDER_HPP

namespace Crunch { namespace Concurrency {

enum MemoryOrder
{
    MEMORY_ORDER_RELAXED = 0,
    MEMORY_ORDER_ACQUIRE = 1,
    MEMORY_ORDER_RELEASE = 2,
    MEMORY_ORDER_ACQ_REL = 3, // MEMORY_ORDER_ACQUIRE | MEMORY_ORDER_RELEASE,
    MEMORY_ORDER_SEQ_CST = 7, // MEMORY_ORDER_ACQUIRE | 4,
    MEMORY_ORDER_CONSUME = 8
};

}}

#endif