/* -------------------------------------------

  Copyright (C) 2024-2025 Amlal El Mahrouss, all rights reserved.

------------------------------------------- */

#pragma once

#include <LibC++/defines.h>
#include <LibC++/base_process.h>

namespace std::base_exception {
inline void __throw_general(void) {
  base_process::exit(33);
}

inline void __throw_domain_error(const char* error) {
  __throw_general();
  __builtin_unreachable();  // prevent from continuing.
}

inline void __throw_bad_array_new_length(void) {
  __throw_general();
  __builtin_unreachable();  // prevent from continuing.
}
}  // namespace std::base_exception
