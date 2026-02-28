// Copyright 2024-2025, Amlal El Mahrouss (amlal@nekernel.org)
// Licensed under the Apache License, Version 2.0 (See accompanying
// file LICENSE or copy at http://www.apache.org/licenses/LICENSE-2.0)
// Official repository: https://github.com/nekernel-org/nectar

#include <CompilerKit/Detail/Config.h>

/// @file ld64.cc
/// @brief Nectar linker for AE objects.

CK_IMPORT_C Int32 DynamicLinker64MachO(Int32 argc, char const* argv[]);

Int32 main(Int32 argc, char const* argv[]) {
  return DynamicLinker64MachO(argc, argv);
}
