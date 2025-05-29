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

#include <LibCompiler/AE.h>
#include <LibCompiler/Backend/32x0.h>
#include <LibCompiler/Frontend.h>
#include <LibCompiler/PEF.h>
#include <LibCompiler/Util/CompilerUtils.h>

/////////////////////////////////////////////////////////////////////////////////////////

// @brief 32x0 Assembler entrypoint, the program/module starts here.

/////////////////////////////////////////////////////////////////////////////////////////

LIBCOMPILER_MODULE(NEAssemblerMain32000) {
  LibCompiler::install_signal(SIGSEGV, Detail::drvi_crash_handler);
  return EXIT_SUCCESS;
}
