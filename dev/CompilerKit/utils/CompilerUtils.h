/* -------------------------------------------

  Copyright (C) 2024-2025 Amlal EL Mahrouss, all rights reserved

------------------------------------------- */

#pragma once

#include <CompilerKit/Compiler.h>
#include <CompilerKit/ErrorID.h>
#include <CompilerKit/Frontend.h>
#include <CompilerKit/Version.h>
#include <ThirdParty/Dialogs.h>
#include <iostream>

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
/// @brief Linker specific blob metadata structure
struct DynamicLinkerBlob final {
  std::vector<Char> mBlob{};       // PEF code/bss/data blob.
  UIntPtr           mOffset{0UL};  // the offset of the PEF container header...
};

inline void print_error(std::string reason, std::string file) noexcept {
  if (reason[0] == '\n') reason.erase(0, 1);

  kStdErr << reason << kBlank << std::endl;

  if (kAcceptableErrors > kErrorLimit) std::exit(NECTI_EXEC_ERROR);

  ++kAcceptableErrors;
}

inline void print_warning(std::string reason, std::string file) noexcept {
  if (reason[0] == '\n') reason.erase(0, 1);

  kStdOut << kYellow << reason << kBlank << std::endl;
}

/// @internal
/// @brief Handler for SIGSEGV signal.
inline void drvi_crash_handler(std::int32_t id) {
  CompilerKit::STLString verbose_header = "LIBCOMPILER CRASH REPORT - ";
  verbose_header += kDistVersion;
  verbose_header += " - ";
  verbose_header += CompilerKit::current_date();

  for (auto& ch : verbose_header) {
    std::cout << '=';
  }

  std::cout << std::endl;

  std::cout << verbose_header << std::endl;

  for (auto& ch : verbose_header) {
    std::cout << '=';
  }

  std::cout << std::endl;

  kStdOut << "DATE: " << CompilerKit::current_date() << std::endl;
  kStdOut << "VERSION: " << kDistVersion << std::endl;
  kStdOut << "ERRNO: " << errno << std::endl;
  kStdOut << "ERRNO(STRING): " << strerror(errno) << std::endl;

  switch (id) {
    case SIGSEGV: {
      kStdOut << "SIGNAL: Segmentation Fault." << kBlank << std::endl;
      break;
    }
    case SIGABRT: {
      kStdOut << "SIGNAL: Aborted." << kBlank << std::endl;
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

  std::exit(NECTI_EXEC_ERROR);
}
}  // namespace Detail
