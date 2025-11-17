/* -------------------------------------------

  Copyright (C) 2024-2025 Amlal EL Mahrouss, Licensed under Apache 2.0

------------------------------------------- */

#include <CompilerKit/Defines.h>

/// @file ld64.cc
/// @brief NeCTI linker for AE objects.

CK_IMPORT_C Int32 DynamicLinker64PEF(Int32 argc, Char const* argv[]);

Int32 main(Int32 argc, Char const* argv[]) {
  return DynamicLinker64PEF(argc, argv);
}
