// Copyright 2024-2025, Amlal El Mahrouss (amlal@nekernel.org)
// Licensed under the Apache License, Version 2.0 (See accompanying
// file LICENSE or copy at http://www.apache.org/licenses/LICENSE-2.0)
// Official repository: https://github.com/nekernel-org/nectar

/// @file cppdrv.cc
/// @brief Nectar frontend preprocessor.

#include <CompilerKit/Detail/Config.h>
#include <CompilerKit/ErrorID.h>

CK_IMPORT_C int GenericPreprocessorMain(int argc, char const* argv[]);

int main(int argc, char const* argv[]) {
  if (auto code = GenericPreprocessorMain(argc, argv); code > 0) {
    std::printf("cppdrv: preprocessor exited with code %i.\n", code);

    return NECTAR_EXEC_ERROR;
  }

  return NECTAR_SUCCESS;
}
