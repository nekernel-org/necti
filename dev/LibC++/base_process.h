/* -------------------------------------------

  Copyright (C) 2024-2025 Amlal El Mahrouss, all rights reserved.

------------------------------------------- */

#pragma once

#include <LibC++/defines.h>

/// @brief CRT exit, with exit code (!!! exits all threads. !!!)
/// @param code the exit code.
/// @return the return > 0 for non successful.
extern "C" int exit_(int code);
extern "C" int signal_(int code);

/// @brief Standard C++ namespace
namespace std::base_process {
inline int signal(int code) {
  signal_(code);
  return -1;
}

inline int exit(int code) {
  exit_(code);
  return -1;
}
}  // namespace std::base_process
