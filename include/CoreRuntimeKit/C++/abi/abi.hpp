// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026, Amlal El Mahrouss (amlal@nekernel.org)
// Licensed under the Apache License, Version 2.0 (see LICENSE file)
// Official repository: https://github.com/ne-foss-org/nectar

#pragma once

#include <base_process>
#include <defines>

__init_decl()

    static constexpr int32_t __unreachable_code = 34;

inline void __compilerkit_unreachable(void) {
  std::base_process::signal(__unreachable_code);
  while (true);
}

__fini_decl()
