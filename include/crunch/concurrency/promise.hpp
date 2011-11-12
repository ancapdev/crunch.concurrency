// Copyright (c) 2011, Christian Rorvik
// Distributed under the Simplified BSD License (See accompanying file LICENSE.txt)

#ifndef CRUNCH_CONCURRENCY_PROMISE_HPP
#define CRUNCH_CONCURRENCY_PROMISE_HPP

#include "crunch/base/intrusive_ptr.hpp"
#include "crunch/concurrency/future.hpp"

#include <exception>
#include <utility>

namespace Crunch { namespace Concurrency {

template<typename T>
class Promise
{
public:
    Promise()
        : mData(new DataType())
    {}

    Promise(Promise&& rhs)
        : mData(std::move(rhs))
    {}
                
    Promise& operator= (Promise&& rhs)
    {
        mData = std::move(rhs.mData);
        return *this;
    }

    void SetValue(T const& value)
    {
        mData->Set(value);
    }

    void SetValue(T&& value)
    {
        mData->Set(value);
    }

    void SetException(std::exception_ptr const& exception)
    {
        mData->SetException(exception);
    }

    Future<T> GetFuture()
    {
        return Future<T>(mData);
    }

private:
    typedef Detail::FutureData<T> DataType;
    typedef IntrusivePtr<DataType> DataPtr;

    DataPtr mData;
};

template<>
class Promise<void>
{
};

template<typename T>
class Promise<T&>
{
};

}}

#endif
