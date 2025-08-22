/* -------------------------------------------

  Copyright (C) 2024-2025 Amlal El Mahrouss, all rights reserved.

------------------------------------------- */

#pragma once

#include <LibC++/defines.h>

/// @brief CRT exit, with exit code (!!! exits all threads. !!!)
/// @param code the exit code.
/// @return the return > 0 for non successful.
extern "C" int exit_(int code);

/// @brief CRT signal handler.
/// @param code the signal code.
extern "C" void signal_(int code);

/// @brief Standard C++ namespace
namespace std::base_process {
inline int signal(int code) {
  signal_(code);
  return -1;
}

extern "C" void (*__atexit_lst_ptr)(void);
extern "C" size_t __atexit_lst_cnt;

inline int exit(int code) {
  for (auto i = 0UL; i < __atexit_lst_cnt; ++i) {
    __atexit_lst_ptr();
  }

  exit_(code);
  return -1;
}
}  // namespace std::base_process
