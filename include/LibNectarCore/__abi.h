// Copyright 2024-2025, Amlal El Mahrouss (amlal@nekernel.org)
// Licensed under the Apache License, Version 2.0 (See accompanying
// file LICENSE or copy at http://www.apache.org/licenses/LICENSE-2.0)
// Official repository: https://github.com/nekernel-org/nectar

#ifndef NECTAR_LIBNECTAR_ABI_H
#define NECTAR_LIBNECTAR_ABI_H

#include <LibNectarCore/base_process.h>
#include <LibNectarCore/defines.h>

__init_decl()

static constexpr int32_t __unreachable_code = 34;

inline void __compilerkit_unreachable(void) {
  signal(__unreachable_code);

  while (1);
}

__fini_decl()

#endif  // NECTAR_LIBNECTAR_ABI_H
