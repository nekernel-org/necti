/* ========================================

  Copyright (C) 2024-2025 Amlal El Mahrouss, licensed under the Apache 2.0 license

======================================== */

/// @file cppdrv.cc
/// @brief Nectar frontend preprocessor.

#include <CompilerKit/Detail/Config.h>
#include <CompilerKit/ErrorID.h>

CK_IMPORT_C int CPlusPlusPreprocessorMain(int argc, char const* argv[]);

int main(int argc, char const* argv[]) {
  if (auto code = CPlusPlusPreprocessorMain(argc, argv); code > 0) {
    std::printf("cppdrv: preprocessor exited with code %i.\n", code);

    return NECTI_EXEC_ERROR;
  }

  return NECTI_SUCCESS;
}
