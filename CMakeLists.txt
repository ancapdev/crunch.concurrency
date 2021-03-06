# Copyright (c) 2011-2013, Christian Rorvik
# Distributed under the Simplified BSD License (See accompanying file LICENSE.txt)

vpm_depend_self()

if(${CMAKE_SYSTEM_PROCESSOR} STREQUAL "x86" OR
   ${CMAKE_SYSTEM_PROCESSOR} STREQUAL "x86_64")
  if(APPLE)
    set(_atomicPlatform "linux")
  else()
    set(_atomicPlatform ${VPM_PLATFORM_NAME})
  endif()
  set(_platformFiles
    include/crunch/concurrency/platform/${_atomicPlatform}/atomic.hpp
    include/crunch/concurrency/platform/${_atomicPlatform}/atomic_ops_x86.hpp
    include/crunch/concurrency/platform/${_atomicPlatform}/atomic_storage.hpp
    include/crunch/concurrency/platform/${_atomicPlatform}/atomic_word.hpp)
endif()
    
vpm_add_library(crunch_concurrency_lib
  include/crunch/concurrency/api.hpp
  include/crunch/concurrency/atomic.hpp
  include/crunch/concurrency/constant_backoff.hpp
  include/crunch/concurrency/event.hpp
  include/crunch/concurrency/exceptions.hpp
  include/crunch/concurrency/exponential_backoff.hpp
  include/crunch/concurrency/fence.hpp
  include/crunch/concurrency/future.hpp
  include/crunch/concurrency/mpmc_lifo_list.hpp
  include/crunch/concurrency/mpmc_lifo_queue.hpp
  include/crunch/concurrency/lock_guard.hpp
  include/crunch/concurrency/memory_order.hpp
  include/crunch/concurrency/meta_scheduler.hpp
  include/crunch/concurrency/mutex.hpp
  include/crunch/concurrency/null_backoff.hpp
  include/crunch/concurrency/processor_affinity.hpp
  include/crunch/concurrency/processor_topology.hpp
  include/crunch/concurrency/promise.hpp
  include/crunch/concurrency/semaphore.hpp
  include/crunch/concurrency/scheduler.hpp
  include/crunch/concurrency/spin_barrier.hpp
  include/crunch/concurrency/thread.hpp
  include/crunch/concurrency/thread_local.hpp
  include/crunch/concurrency/thread_pool.hpp
  include/crunch/concurrency/versioned_data.hpp
  include/crunch/concurrency/wait_mode.hpp
  include/crunch/concurrency/waitable.hpp
  include/crunch/concurrency/waiter.hpp
  include/crunch/concurrency/waiter_utility.hpp
  include/crunch/concurrency/yield.hpp
  include/crunch/concurrency/detail/future_data.hpp
  include/crunch/concurrency/detail/system_condition.hpp
  include/crunch/concurrency/detail/system_event.hpp
  include/crunch/concurrency/detail/system_mutex.hpp
  include/crunch/concurrency/detail/system_semaphore.hpp
  include/crunch/concurrency/detail/waiter_list.hpp
  source/event.cpp
  source/exceptions.cpp
  source/future_data.cpp
  source/meta_scheduler.cpp
  source/mutex.cpp
  source/processor_affinity.cpp
  source/processor_topology.cpp
  source/semaphore.cpp
  source/thread.cpp
  source/thread_data.hpp
  source/thread_data.cpp
  source/thread_pool.cpp
  source/waiter.cpp
  source/waiter_list.cpp
  source/platform/${VPM_PLATFORM_NAME}/processor_affinity.cpp
  source/platform/${VPM_PLATFORM_NAME}/processor_topology.cpp
  source/platform/${VPM_PLATFORM_NAME}/system_condition.cpp
  source/platform/${VPM_PLATFORM_NAME}/system_event.cpp
  source/platform/${VPM_PLATFORM_NAME}/system_mutex.cpp
  source/platform/${VPM_PLATFORM_NAME}/system_semaphore.cpp
  source/platform/${VPM_PLATFORM_NAME}/thread.cpp
  source/platform/${VPM_PLATFORM_NAME}/yield.cpp
  ${_platformFiles})

target_link_libraries(crunch_concurrency_lib
  crunch_base_lib
  crunch_containers_lib)

if(UNIX)
  target_link_libraries(crunch_concurrency_lib
    pthread)
endif()

if(VPM_CURRENT_PACKAGE_IS_ROOT)
  # Add unit tests
  vpm_set_default_version(crunch.test master)
  vpm_depend(crunch.test)

  crunch_add_test(crunch_concurrency_test
    test/atomic_tests.cpp
    test/event_tests.cpp
    test/future_tests.cpp
    test/meta_scheduler_tests.cpp
    test/mpmc_lifo_list_tests.cpp
    test/mutex_tests.cpp
    test/processor_topology_tests.cpp
    test/semaphore_tests.cpp
    test/thread_pool_tests.cpp
    test/thread_tests.cpp)

  target_link_libraries(crunch_concurrency_test
    crunch_concurrency_lib)

  # Add benchmarks
  vpm_set_default_version(crunch.benchmarking master)
  vpm_depend(crunch.benchmarking)

  crunch_add_benchmark(crunch_concurrency_benchmark
    benchmark/atomic_benchmarks.cpp
    benchmark/event_benchmarks.cpp
    benchmark/meta_scheduler_benchmarks.cpp
    benchmark/mpmc_lifo_list_benchmarks.cpp)

  target_link_libraries(crunch_concurrency_benchmark
    crunch_concurrency_lib)
endif()
