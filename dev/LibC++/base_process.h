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

extern "C" void (*__atexit_cdecl_ptr)(void);
extern "C" void (**__atexit_lst_ptr)(void);
extern "C" size_t __atexit_lst_cnt;

inline int32_t exit(const int32_t& code) {
  for (auto idx = 0UL; idx < __atexit_lst_cnt; ++idx) {
    __atexit_lst_ptr[idx]();
  }

  if (__atexit_cdecl_ptr) __atexit_cdecl_ptr();

  exit_(code);
  return -1;
}
}  // namespace std::base_process
