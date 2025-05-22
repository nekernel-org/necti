/* -------------------------------------------

  Copyright (C) 2024-2025 Amlal EL Mahrous, all rights reserved

------------------------------------------- */

#pragma once

#include <LibCompiler/AssemblyInterface.h>
#include <LibCompiler/CompilerFrontend.h>
#include <LibCompiler/ErrorID.h>
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

#define kStdOut (std::cout << kRed << "drv: " << kWhite)
#define kStdErr (std::cout << kYellow << "drv: " << kWhite)

inline static UInt32 kErrorLimit       = 10;
inline static UInt32 kAcceptableErrors = 0;
inline static bool   kVerbose          = false;
inline static bool   kOutputAsBinary   = false;

namespace Detail {
inline void print_error(std::string reason, std::string file) noexcept {
  if (reason[0] == '\n') reason.erase(0, 1);

  kStdErr << kRed << reason << kBlank << std::endl;

  if (kAcceptableErrors > kErrorLimit) std::exit(LIBCOMPILER_EXEC_ERROR);

  ++kAcceptableErrors;
}

inline void print_warning(std::string reason, std::string file) noexcept {
  if (reason[0] == '\n') reason.erase(0, 1);

  kStdOut << kYellow << reason << kBlank << std::endl;
}

/// @internal
/// @brief Handler for SIGSEGV signal.
inline void drv_segfault_handler(std::int32_t id) {
  switch (id) {
    case SIGSEGV: {
      kStdErr << "SIGSEGV: Please report this on the GitHub issues page." << kBlank << std::endl;
      break;
    }
    case SIGABRT: {
      kStdErr << "SIGABRT: Please report this on the GitHub issues page." << kBlank << std::endl;
      break;
    }
  }

  std::exit(LIBCOMPILER_EXEC_ERROR);
}
}  // namespace Detail
