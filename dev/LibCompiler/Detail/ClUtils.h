/* -------------------------------------------

  Copyright (C) 2024-2025 Amlal EL Mahrous, all rights reserved

------------------------------------------- */

#pragma once

#include <LibCompiler/AssemblyInterface.h>
#include <LibCompiler/ErrorID.h>
#include <LibCompiler/Parser.h>
#include <Vendor/Dialogs.h>

#define kZero64Section ".zero64"
#define kCode64Section ".code64"
#define kData64Section ".data64"

#define kZero128Section ".zero128"
#define kCode128Section ".code128"
#define kData128Section ".data128"

#define kBlank "\e[0;30m"
#define kRed "\e[0;31m"
#define kWhite "\e[0;97m"
#define kYellow "\e[0;33m"

#define kStdOut (std::cout << kWhite)
#define kStdErr (std::cout << kRed)

inline static UInt32 kErrorLimit       = 10;
inline static UInt32 kAcceptableErrors = 0;
inline static bool   kVerbose          = false;
inline static bool   kOutputAsBinary   = false;

namespace Detail {
inline void print_error(std::string reason, std::string file) noexcept {
  if (reason[0] == '\n') reason.erase(0, 1);

  kStdErr << kRed << "drv: " << kWhite << reason << kBlank << std::endl;

  if (kAcceptableErrors > kErrorLimit) std::exit(LIBCOMPILER_EXEC_ERROR);

  ++kAcceptableErrors;
}

inline void print_warning(std::string reason, std::string file) noexcept {
  if (reason[0] == '\n') reason.erase(0, 1);

  kStdOut << kYellow << "drv: " << kWhite << reason << kBlank << std::endl;
}

/// @internal
inline void segfault_handler(std::int32_t _) {
  pfd::notify("LibCompiler",
              "Driver just crashed, please report this to the developers.");
  std::exit(LIBCOMPILER_EXEC_ERROR);
}
}  // namespace Detail
