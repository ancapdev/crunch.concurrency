// Copyright (c) 2011, Christian Rorvik
// Distributed under the Simplified BSD License (See accompanying file LICENSE.txt)

#include "crunch/concurrency/detail/future_data.hpp"

namespace Crunch { namespace Concurrency { namespace Detail {

void FutureDataBase::Destroy()
{
    delete this;
}

}}}
