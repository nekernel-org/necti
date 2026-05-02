// Copyright 2024-2025, Amlal El Mahrouss (amlal@nekernel.org)
// Licensed under the Apache License, Version 2.0 (See accompanying
// file LICENSE or copy at http://www.apache.org/licenses/LICENSE-2.0)
// Official repository: https://github.com/ne-foss-org/nectar

#include <CompilerKit/Detail/Config.h>

/// @file kdbg.cc
/// @brief NeSystem debugger.

CK_IMPORT_C Int32 DebuggerNeKernel(Int32 argc, char const* argv[]);

/// @brief Debugger entrypoint.
/// @return Status code of debugger.
Int32 main(Int32 argc, char const* argv[]) {
  return DebuggerNeKernel(argc, argv);
}
