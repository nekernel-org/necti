// Copyright 2024-2025, Amlal El Mahrouss (amlal@nekernel.org)
// Licensed under the Apache License, Version 2.0 (See accompanying
// file LICENSE or copy at http://www.apache.org/licenses/LICENSE-2.0)
// Official repository: https://github.com/ne-foss/nectar

/// bugs: 0

/////////////////////////////////////////////////////////////////////////////////////////

// @file Assembler+32x0.cc
// @author Amlal El Mahrouss
// @brief 32x0 Assembler.

// REMINDER: when dealing with an undefined symbol use (string
// size):LinkerFindSymbol:(string) so that ld will look for it.

/////////////////////////////////////////////////////////////////////////////////////////

#ifndef __ASM_NEED_32x0__
#define __ASM_NEED_32x0__
#endif

#include <CompilerKit/AE.h>
#include <CompilerKit/AST.h>
#include <CompilerKit/Detail/32x0.h>
#include <CompilerKit/PEF.h>
#include <CompilerKit/Utils/Compiler.h>

/////////////////////////////////////////////////////////////////////////////////////////

// @brief 32x0 Assembler entrypoint, the program/module starts here.

/////////////////////////////////////////////////////////////////////////////////////////

NECTAR_MODULE(NEAssemblerMain32000) {
  CompilerKit::install_signal(SIGSEGV, CompilerKit::Detail::drvi_crash_handler);
  return EXIT_SUCCESS;
}
