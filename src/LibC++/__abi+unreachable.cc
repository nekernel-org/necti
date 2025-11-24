/* ========================================

 Copyright (C) 2025 Amlal El Mahrouss, Licensed under the Apache 2.0 license.

======================================== */

#include <LibC++/__abi.h>
#include <LibC++/base_process.h>

static const int32_t __unreachable_code = 34;

extern "C" void __compilerkit_unreachable(void) {
  std::base_process::signal(__unreachable_code);

  while (1)
    ;
}