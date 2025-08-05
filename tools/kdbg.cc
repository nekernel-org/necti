/* -------------------------------------------

  Copyright (C) 2024-2025 Amlal EL Mahrouss, all rights reserved

------------------------------------------- */

#include <CompilerKit/Defines.h>

/// @file kdbg.cxx
/// @brief NeKernel debugger.

CK_IMPORT_C Int32 DebuggerNeKernel(Int32 argc, Char const* argv[]);

/// @brief Debugger entrypoint.
/// @return Status code of debugger.
Int32 main(Int32 argc, Char const* argv[]) {
  return DebuggerNeKernel(argc, argv);
}
