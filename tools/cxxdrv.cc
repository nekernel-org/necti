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
  if (auto code = CompilerCPlusPlusAMD64(argc, argv); code > 0) {
    std::printf("cxxdrv: compiler exited with code %i.\n", code);
    return LIBCOMPILER_EXEC_ERROR;
  }

  return LIBCOMPILER_SUCCESS;
}
