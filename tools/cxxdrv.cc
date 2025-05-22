/* -------------------------------------------

  Copyright (C) 2024-2025 Amlal EL Mahrous, all rights reserved

------------------------------------------- */

/// @file cxxdrv.cc
/// @brief NE C++ frontend compiler.

#include <LibCompiler/Defines.h>
#include <LibCompiler/ErrorID.h>
#include <LibCompiler/Version.h>
#include <cstring>

LC_IMPORT_C Int32 CompilerCPlusPlusAMD64(Int32 argc, CharType const* argv[]);

Int32 main(Int32 argc, CharType const* argv[]) {
  auto ret = CompilerCPlusPlusAMD64(argc, argv);

  return (ret == LIBCOMPILER_SUCCESS) ? EXIT_SUCCESS : EXIT_FAILURE;
}
