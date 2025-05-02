/* -------------------------------------------

  Copyright (C) 2024-2025 Amlal EL Mahrous, all rights reserved

------------------------------------------- */

#include <LibCompiler/Defines.h>

/// @file dbg.cxx
/// @brief NE debugger.

LC_IMPORT_C int DebuggerMachPOSIX(int argc, char const* argv[]);

/// @brief Debugger entrypoint.
/// @return Status code of debugger.
int main(int argc, char const* argv[]) {
  return DebuggerMachPOSIX(argc, argv);
}
