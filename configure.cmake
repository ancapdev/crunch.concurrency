# Copyright (c) 2011-2013, Christian Rorvik
# Distributed under the Simplified BSD License (See accompanying file LICENSE.txt)

vpm_set_default_versions(
  crunch.base master
  crunch.containers master)

vpm_depend(
  crunch.base
  crunch.containers)

vpm_include_directories(${CMAKE_CURRENT_LIST_DIR}/include)
