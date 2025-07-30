/* -------------------------------------------

  Copyright (C) 2024-2025 Amlal EL Mahrouss, all rights reserved

------------------------------------------- */

/// bugs: 0

/////////////////////////////////////////////////////////////////////////////////////////

// @file 32asm.cxx
// @author EL Mahrouss Amlal
// @brief 32x0 Assembler.

// REMINDER: when dealing with an undefined symbol use (string
// size):LinkerFindSymbol:(string) so that ld will look for it.

/////////////////////////////////////////////////////////////////////////////////////////

#ifndef __ASM_NEED_32x0__
#define __ASM_NEED_32x0__ 1
#endif

#include <CompilerKit/AE.h>
#include <CompilerKit/detail/32x0.h>
#include <CompilerKit/Frontend.h>
#include <CompilerKit/PEF.h>
#include <CompilerKit/utils/CompilerUtils.h>

/////////////////////////////////////////////////////////////////////////////////////////

// @brief 32x0 Assembler entrypoint, the program/module starts here.

/////////////////////////////////////////////////////////////////////////////////////////

LIBCOMPILER_MODULE(NEAssemblerMain32000) {
  CompilerKit::install_signal(SIGSEGV, Detail::drvi_crash_handler);
  return EXIT_SUCCESS;
}
