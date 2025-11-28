/* ========================================

  Copyright (C) 2024-2025 Amlal El Mahrouss, Licensed under the Apache 2.0 license

======================================== */

/// bugs: 0

/////////////////////////////////////////////////////////////////////////////////////////

// @file 32asm.cc
// @author Amlal El Mahrouss
// @brief 32x0 Assembler.

// REMINDER: when dealing with an undefined symbol use (string
// size):LinkerFindSymbol:(string) so that ld will look for it.

/////////////////////////////////////////////////////////////////////////////////////////

#ifndef __ASM_NEED_32x0__
#define __ASM_NEED_32x0__ 1
#endif

#include <CompilerKit/AE.h>
#include <CompilerKit/AST.h>
#include <CompilerKit/PEF.h>
#include <CompilerKit/impl/32x0.h>
#include <CompilerKit/utils/CompilerUtils.h>

/////////////////////////////////////////////////////////////////////////////////////////

// @brief 32x0 Assembler entrypoint, the program/module starts here.

/////////////////////////////////////////////////////////////////////////////////////////

NECTI_MODULE(NEAssemblerMain32000) {
  CompilerKit::install_signal(SIGSEGV, Detail::drvi_crash_handler);
  return EXIT_SUCCESS;
}
