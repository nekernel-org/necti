/* ========================================

  Copyright (C) 2024-2025 Amlal El Mahrouss, Licensed under the Apache 2.0 license

======================================== */

#include <CompilerKit/Defines.h>

/// @file kdbg.cc
/// @brief NeKernel debugger.

CK_IMPORT_C Int32 DebuggerNeKernel(Int32 argc, Char const* argv[]);

/// @brief Debugger entrypoint.
/// @return Status code of debugger.
Int32 main(Int32 argc, Char const* argv[]) {
  return DebuggerNeKernel(argc, argv);
}
