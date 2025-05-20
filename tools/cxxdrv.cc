/* -------------------------------------------

  Copyright (C) 2024-2025 Amlal EL Mahrous, all rights reserved

------------------------------------------- */

/// @file cxxdrv.cc
/// @brief NE C++ frontend compiler.

#include <LibCompiler/Defines.h>
#include <LibCompiler/ErrorID.h>
#include <LibCompiler/Version.h>
#include <cstring>

LC_IMPORT_C int CompilerCPlusPlusAMD64(int argc, char const* argv[]);

int main(int argc, char const* argv[]) {
  return CompilerCPlusPlusAMD64(argc, argv);
}
