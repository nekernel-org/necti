/* -------------------------------------------

  Copyright (C) 2024-2025 Amlal EL Mahrouss, all rights reserved

------------------------------------------- */

#include <CompilerKit/Defines.h>

/// @file ld64.cxx
/// @brief NE Linker for AE objects.

CK_IMPORT_C int DynamicLinker64PEF(int argc, char const* argv[]);

int main(int argc, char const* argv[]) {
  return DynamicLinker64PEF(argc, argv);
}
