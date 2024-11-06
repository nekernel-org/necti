/* -------------------------------------------

	Copyright (C) 2024, Amlal EL Mahrouss, all rights reserved

------------------------------------------- */

/// bugs: 0

/////////////////////////////////////////////////////////////////////////////////////////

// @file 32asm.cxx
// @author Amlal EL Mahrouss
// @brief 32x0 Assembler.

// REMINDER: when dealing with an undefined symbol use (string
// size):LinkerFindSymbol:(string) so that ld will look for it.

/////////////////////////////////////////////////////////////////////////////////////////

#define __ASM_NEED_32x0__ 1

#include <ToolchainKit/AAL/CPU/32x0.h>
#include <ToolchainKit/Parser.h>
#include <ToolchainKit/NFC/AE.h>
#include <ToolchainKit/NFC/PEF.h>

/////////////////////

// ANSI ESCAPE CODES

/////////////////////

#define kBlank	"\e[0;30m"
#define kRed	"\e[0;31m"
#define kWhite	"\e[0;97m"
#define kYellow "\e[0;33m"

#define kStdOut (std::cout << kWhite)
#define kStdErr (std::cout << kRed)

/////////////////////////////////////////////////////////////////////////////////////////

// @brief 32x0 Assembler entrypoint, the program/module starts here.

/////////////////////////////////////////////////////////////////////////////////////////

TOOLCHAINKIT_MODULE(ZKAAssemblerMain32000)
{
	return 0;
}
