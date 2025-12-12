/* ========================================

  Copyright (C) 2024-2025 Amlal El Mahrouss, licensed under the Apache 2.0 license

======================================== */

#include <CompilerKit/Detail/Config.h>

/// @file dbg.cc
/// @brief Nectar debugger.

CK_IMPORT_C Int32 DebuggerMachPOSIX(Int32 argc, Char const* argv[]);

/// @brief Debugger entrypoint.
/// @return Status code of debugger.
Int32 main(Int32 argc, Char const* argv[]) {
  return DebuggerMachPOSIX(argc, argv);
}
