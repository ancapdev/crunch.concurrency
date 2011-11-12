// Copyright (c) 2011, Christian Rorvik
// Distributed under the Simplified BSD License (See accompanying file LICENSE.txt)

#ifndef CRUNCH_CONCURRENCY_FUTURE_HPP
#define CRUNCH_CONCURRENCY_FUTURE_HPP

#include "crunch/base/override.hpp"
#include "crunch/base/intrusive_ptr.hpp"
#include "crunch/concurrency/waitable.hpp"
#include "crunch/concurrency/detail/future_data.hpp"

#include <utility>

namespace Crunch { namespace Concurrency {

// Similar to std::shared_future
template<typename T>
class Future : public IWaitable
{
public:
    typedef Detail::FutureData<T> DataType;
    typedef IntrusivePtr<DataType> DataPtr;


    // For internal use
    Future(DataPtr const& data)
        : mData(data)
    {}

    Future(DataPtr&& data)
        : mData(std::move(data))
    {}

    bool IsReady() const
    {
        return mData->IsReady();
    }

    bool HasValue() const
    {
        return mData->HasValue();
    }

    bool HasException() const
    {
        return mData->HasException();
    }

    typename DataType::GetReturnType Get() const
    {
        return mData->Get();
    }

    void Wait() const
    {
        mData->Wait();
    }

    //
    // IWaitable
    //
    using IWaitable::AddWaiter;

    virtual void AddWaiter(Waiter* waiter) CRUNCH_OVERRIDE
    {
        mData->AddWaiter(waiter);
    }

    virtual bool RemoveWaiter(Waiter* waiter) CRUNCH_OVERRIDE
    {
        return mData->RemoveWaiter(waiter);
    }

    virtual bool IsOrderDependent() const CRUNCH_OVERRIDE
    {
        return false;
    }

private:

    DataPtr mData;
};

template<typename T>
class Future<T&>
{
};

}}

#endif
