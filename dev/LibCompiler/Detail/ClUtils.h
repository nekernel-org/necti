/* -------------------------------------------

  Copyright (C) 2024-2025 Amlal EL Mahrous, all rights reserved

------------------------------------------- */

#pragma once

#include <LibCompiler/AssemblyInterface.h>
#include <LibCompiler/Parser.h>

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
inline static bool kVerbose = false;
inline static bool kOutputAsBinary = false;

namespace Detail {
inline void print_error(std::string reason, std::string file) noexcept {
  if (reason[0] == '\n') reason.erase(0, 1);

  kStdErr << kRed << "[ asm ] " << kWhite
          << ((file == "LibCompiler") ? "InternalErrorException: "
                                      : ("FileException{ " + file + " }: "))
          << kBlank << std::endl;
  kStdErr << kRed << "[ asm ] " << kWhite << reason << kBlank << std::endl;

  if (kAcceptableErrors > kErrorLimit) std::exit(3);

  ++kAcceptableErrors;
}

inline void print_warning(std::string reason, std::string file) noexcept {
  if (reason[0] == '\n') reason.erase(0, 1);

  if (!file.empty()) {
    kStdOut << kYellow << "[ asm ] " << kWhite << file << kBlank << std::endl;
  }

  kStdOut << kYellow << "[ asm ] " << kWhite << reason << kBlank << std::endl;
}
}  // namespace Detail