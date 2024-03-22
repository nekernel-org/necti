/* -------------------------------------------

    Copyright Mahrouss Logic

------------------------------------------- */

/// bugs: 0

/////////////////////////////////////////////////////////////////////////////////////////

// @file ppcasm.cxx
// @author Amlal El Mahrouss
// @brief PowerPC 64 Assembler.

// REMINDER: when dealing with an undefined symbol use (string
// size):LinkerFindSymbol:(string) so that ld will look for it.

/////////////////////////////////////////////////////////////////////////////////////////

#define __ASM_NEED_PPC64__ 1

#include <Headers/AsmKit/Arch/32x0.hpp>
#include <Headers/ParserKit.hpp>
#include <Headers/StdKit/AE.hpp>
#include <Headers/StdKit/PEF.hpp>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>
#include <memory>


/////////////////////

// ANSI ESCAPE CODES

/////////////////////

#define kBlank "\e[0;30m"
#define kRed "\e[0;31m"
#define kWhite "\e[0;97m"
#define kYellow "\e[0;33m"

#define kStdOut (std::cout << kWhite)
#define kStdErr (std::cout << kRed)

/////////////////////////////////////////////////////////////////////////////////////////

// @brief PowerPC 64 Assembler entrypoint, the program/module starts here.

/////////////////////////////////////////////////////////////////////////////////////////

MPCC_MODULE(HCoreAssemblerPowerPC64) {
  return 0;
}
