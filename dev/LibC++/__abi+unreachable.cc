/* -------------------------------------------

 Copyright (C) 2025 Amlal El Mahrouss, all rights reserved.

------------------------------------------- */

#include <LibC++/__abi.h>
#include <LibC++/base_process.h>

static const int32_t __unreachable_code = 34;

extern "C" void __libcompiler_unreachable(void) {
  std::base_process::signal(__unreachable_code);

  while (1);
}