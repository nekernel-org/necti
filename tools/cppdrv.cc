/* -------------------------------------------

  Copyright (C) 2024-2025 Amlal EL Mahrouss, all rights reserved

------------------------------------------- */

/// @file cxxdrv.cc
/// @brief NE frontend preprocessor.

#include <CompilerKit/Defines.h>
#include <CompilerKit/ErrorID.h>
#include <CompilerKit/Version.h>
#include <cstring>
#include <iostream>
#include <vector>

LC_IMPORT_C int CPlusPlusPreprocessorMain(int argc, char const* argv[]);

int main(int argc, char const* argv[]) {
  if (auto code = CPlusPlusPreprocessorMain(2, argv); code > 0) {
    std::printf("cppdrv: preprocessor exited with code %i.\n", code);

    return LIBCOMPILER_EXEC_ERROR;
  }

  return LIBCOMPILER_SUCCESS;
}