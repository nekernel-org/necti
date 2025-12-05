/* ========================================

 Copyright (C) 2024-2025 Amlal El Mahrouss, Licensed under the Apache 2.0 license.

======================================== */

#pragma once

#include <LibC++/defines.h>
#include <LibC++/base_process.h>

__init_decl()

static constexpr int32_t __unreachable_code = 34;

inline void __compilerkit_unreachable(void) {
  std::base_process::signal(__unreachable_code);

  while (1);
}

__fini_decl()