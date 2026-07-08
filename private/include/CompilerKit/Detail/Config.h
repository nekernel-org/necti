// Copyright 2024-2025, Amlal El Mahrouss (amlal@nekernel.org)
// Licensed under the Apache License, Version 2.0 (See accompanying
// file LICENSE or copy at http://www.apache.org/licenses/LICENSE-2.0)
// Official repository: https://github.com/ne-app-eu/ncc

#ifndef __COMPILERKIT_CONFIG_H__
#define __COMPILERKIT_CONFIG_H__

/// =========================================================== ///
/// @file detail/Config.h
/// @author Amlal El Mahrouss (ne-app)
/// @brief Basic defines and types for CompilerKit.
/// =========================================================== ///

#include <CompilerKit/Detail/PreConfig.h>

#ifdef __NCC_USE_TPROC__
#include <ocl/tproc.hpp>
#endif

namespace CompilerKit {

inline static constexpr int kBaseYear = 1900;
using STLString                       = std::string;

#ifdef __NCC_USE_TPROC__
using RopeString                      = ocl::tproc::crope;
#else
using RopeString = STLString;
#endif

inline STLString current_date() noexcept {
  auto time_data   = time(nullptr);
  auto time_struct = gmtime(&time_data);

  STLString fmt = std::to_string(kBaseYear + time_struct->tm_year);

  fmt += "-";
  fmt += std::to_string(time_struct->tm_mon + 1);
  fmt += "-";
  fmt += std::to_string(time_struct->tm_mday);

  return fmt;
}

inline bool to_str(char* str, Int32 limit, Int32 base) noexcept {
  if (limit == 0) return false;
  if (!str) return false;

  Int32 copy_limit = limit;
  Int32 cnt        = 0;
  Int32 ret        = base;

  while (limit != 1) {
    ret      = ret % 10;
    str[cnt] = ret;

    ++cnt;
    --limit;
    --ret;
  }

  str[copy_limit] = '\0';
  return true;
}

inline bool install_signal(Int32 signal, void (*handler)(int)) noexcept {
  if (handler == nullptr) return false;

  if (::signal(signal, handler) == SIG_ERR) {
    return false;
  }

  return true;
}

}  // namespace CompilerKit

#endif  // __COMPILERKIT_CONFIG_H__
