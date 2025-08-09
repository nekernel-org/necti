/* -------------------------------------------

  Copyright (C) 2024-2025 Amlal El Mahrouss, all rights reserved.

------------------------------------------- */

#pragma once

#include <LibC++/__abi.h>
#include <LibC++/base_process.h>
#include <LibC++/defines.h>
#include <iostream>

/// @author Amlal El Mahrouss (amlal@nekernel.org)

namespace std::base_exception::abi {
inline constexpr int __terminate_id = 33;

/// @note This function is internal, don't call it.
extern void __unwind_object_list();

inline void __throw_general(const char* what) {
  std::cout << "LibC++: Unwinding exception of kind: " << what << ", aborting here..." << std::endl;
  __unwind_object_list();
  base_process::exit(__terminate_id);
}

inline void __throw_domain_error(const char* what) {
  __throw_general(what);
  __builtin_unreachable();  // prevent from continuing.
}

inline void __throw_bad_array_new_length(const char* what) {
  __throw_general(what);
  __builtin_unreachable();  // prevent from continuing.
}
}  // namespace std::base_exception::abi
