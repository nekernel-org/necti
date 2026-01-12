// Copyright 2024-2025, Amlal El Mahrouss (amlal@nekernel.org)
// Licensed under the Apache License, Version 2.0 (See accompanying
// file LICENSE or copy at http://www.apache.org/licenses/LICENSE-2.0)
// Official repository: https://github.com/nekernel-org/nectar

#ifndef NECTAR_LIBCXX_BASE_EXCEPTION_H
#define NECTAR_LIBCXX_BASE_EXCEPTION_H

#include <LibNectarCore/__abi.h>
#include <LibNectarCore/base_process.h>
#include <LibNectarCore/defines.h>

/// @author Amlal El Mahrouss (amlal@nekernel.org)

inline constexpr int __terminate_id = 33;

/// @note This function is internal, don't call it.
extern void __unwind_object_list();

inline void __throw_general(const char* what) {
  __unwind_object_list();
  exit(__terminate_id);
}

inline void __throw_domain_error(const char* what) {
  __throw_general(what);
  __builtin_unreachable();  // prevent from continuing.
}

inline void __throw_bad_array_new_length(const char* what) {
  __throw_general(what);
  __builtin_unreachable();  // prevent from continuing.
}

#endif  // NECTAR_LIBCXX_BASE_EXCEPTION_H
