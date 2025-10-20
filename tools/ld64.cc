/* -------------------------------------------

  Copyright (C) 2024-2025 Amlal EL Mahrouss, all rights reserved

------------------------------------------- */

#include <CompilerKit/Defines.h>

/// @file ld64.cc
/// @brief NE Linker for AE objects.

CK_IMPORT_C Int32 DynamicLinker64PEF(Int32 argc, Char const* argv[]);

Int32 main(Int32 argc, Char const* argv[]) {
  return DynamicLinker64PEF(argc, argv);
}
