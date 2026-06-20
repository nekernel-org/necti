// Copyright 2024-2025, Amlal El Mahrouss (amlal@nekernel.org)
// Licensed under the Apache License, Version 2.0 (See accompanying
// file LICENSE or copy at http://www.apache.org/licenses/LICENSE-2.0)
// Official repository: https://github.com/ne-app-eu/ncc

#ifndef NECTAR_COMPILERKIT_UTILITIES_COMPILER_H
#define NECTAR_COMPILERKIT_UTILITIES_COMPILER_H

#include <CompilerKit/AST.h>
#include <CompilerKit/CodeGenerator.h>
#include <CompilerKit/Detail/Config.h>
#include <CompilerKit/ErrorID.h>
#include <ThirdParty/Dialogs/Dialogs.h>
#include <iostream>

#define kZero64Section ".zero64"
#define kCode64Section ".code64"
#define kData64Section ".data64"

#define kZero128Section ".zero128"
#define kCode128Section ".code128"
#define kData128Section ".data128"

#define kBlank "\e[0;0m"
#define kRed "\e[0;31m"
#define kWhite "\e[0;97m"
#define kYellow "\e[0;33m"
#define kReset kBlank

#define kStdOut (std::cout << kRed << "nectar: " << kReset)
#define kStdErr (std::cerr << kRed << "nectar: " << kReset)
#define kStdEndl std::endl
#define kPrintF kStdOut
#define kPrintErr kStdErr

inline static UInt32 kErrorLimit       = 0;
inline static UInt32 kAcceptableErrors = 0;
inline static bool   kVerbose          = false;
inline static bool   kOutputAsBinary   = false;
inline static bool   kNasmOutput       = false;

namespace CompilerKit::Detail {
/// @brief Blob structure
struct Blob final {
  std::vector<char> mBlob{};       // PEF code/bss/data blob.
  UIntPtr           mOffset{0UL};  // the offset of the PEF container header...

  explicit operator bool() { return mBlob.empty() && mOffset > 0UL; }
};

inline void print_error(STLString reason, STLString file) noexcept {
  if (reason[0] == '\n') reason.erase(0, 1);

  kStdErr << file << ": " << reason << kBlank << std::endl;

  ++kAcceptableErrors;
  if (kAcceptableErrors > kErrorLimit) std::exit(NECTAR_EXEC_ERROR);
}

inline void print_warning(STLString reason, STLString file) noexcept {
  if (reason[0] == '\n') reason.erase(0, 1);

  kStdOut << file << ": " << kYellow << reason << kBlank << std::endl;
}

/// @internal
/// @brief Handler for SIGSEGV signal.
inline void drvi_crash_handler(std::int32_t id) {
  CompilerKit::STLString verbose_header = "NECTAR CRASH REPORT - ";
  verbose_header += kDistVersion;
  verbose_header += " - ";
  verbose_header += CompilerKit::current_date();

  for (auto& ch : verbose_header) {
    std::cout << '=';
  }

  kStdOut << kStdEndl;
  kStdOut << verbose_header << kStdEndl;

  for (auto& ch : verbose_header) {
    std::cout << '=';
  }

  kStdOut << kStdEndl;

  kStdOut << "DATE: " << CompilerKit::current_date() << kStdEndl;
  kStdOut << "VERSION: " << kDistVersion << kStdEndl;
  kStdOut << "ERRNO: " << errno << kStdEndl;

  switch (id) {
    default: {
      kStdOut << "SIGNAL: (" << id << ")." << kBlank << kStdEndl;
      break;
    }
  }

  std::cout << kWhite;

  for (auto& ch : verbose_header) {
    std::cout << '=';
  }

  std::cout << std::endl;

  std::cout << verbose_header << std::endl;

  for (auto& ch : verbose_header) {
    std::cout << '=';
  }

  std::cout << std::endl;

  std::exit(NECTAR_EXEC_ERROR);
}
}  // namespace CompilerKit::Detail

#endif  // NECTAR_COMPILERKIT_UTILITIES_COMPILER_H
