/* -------------------------------------------

  Copyright (C) 2024-2025 Amlal EL Mahrouss, Licensed under Apache 2.0

------------------------------------------- */

#include <CompilerKit/Defines.h>

/// @file dbg.cc
/// @brief NE debugger.

CK_IMPORT_C Int32 DebuggerMachPOSIX(Int32 argc, Char const* argv[]);

/// @brief Debugger entrypoint.
/// @return Status code of debugger.
Int32 main(Int32 argc, Char const* argv[]) {
  return DebuggerMachPOSIX(argc, argv);
}
