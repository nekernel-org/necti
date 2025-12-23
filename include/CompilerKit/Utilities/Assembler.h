/* ========================================

  Copyright (C) 2024-2025 Amlal El Mahrouss, licensed under the Apache 2.0 license

======================================== */

#pragma once

#include <CompilerKit/AST.h>
#include <CompilerKit/CodeGenerator.h>
#include <CompilerKit/Utilities/Compiler.h>

namespace CompilerKit {
/// @brief Get Number from lineBuffer.
/// @param lineBuffer the lineBuffer to fetch from.
/// @param numberKey where to seek that number.
/// @return A numbercast of 32-bit width.
inline NumberCast32 GetNumber32(STLString lineBuffer, STLString numberKey) {
  if (lineBuffer.empty()) return {};
  if (lineBuffer.find(numberKey) == STLString::npos) return {};

  auto pos = lineBuffer.find(numberKey) + numberKey.size();

  if (pos > lineBuffer.size()) return {};
  if ((pos + 1) > lineBuffer.size()) return {};

  while (lineBuffer[pos] == ' ') ++pos;

  switch (lineBuffer[pos + 1]) {
    case 'x': {
      if (auto res = strtol(lineBuffer.substr(pos).c_str(), nullptr, 16); !res) {
        if (errno != 0) {
          Detail::print_error("invalid hex number: " + lineBuffer, "CompilerKit");
          throw std::runtime_error("invalid_hex");
        }
      }

      NumberCast32 numOffset(strtol(lineBuffer.substr(pos).c_str(), nullptr, 16));

      if (kVerbose) {
        kStdOut << "asm: found a base 16 number here: " << lineBuffer.substr(pos) << "\n";
      }

      return numOffset;
    }
    case 'b': {
      if (auto res = strtol(lineBuffer.substr(pos).c_str(), nullptr, 2); !res) {
        if (errno != 0) {
          Detail::print_error("invalid binary number:" + lineBuffer, "CompilerKit");
          throw std::runtime_error("invalid_bin");
        }
      }

      NumberCast32 numOffset(strtol(lineBuffer.substr(pos).c_str(), nullptr, 2));

      if (kVerbose) {
        kStdOut << "asm: found a base 2 number here:" << lineBuffer.substr(pos) << "\n";
      }

      return numOffset;
    }
    case 'o': {
      if (auto res = strtol(lineBuffer.substr(pos).c_str(), nullptr, 7); !res) {
        if (errno != 0) {
          Detail::print_error("invalid octal number: " + lineBuffer, "CompilerKit");
          throw std::runtime_error("invalid_octal");
        }
      }

      NumberCast32 numOffset(strtol(lineBuffer.substr(pos).c_str(), nullptr, 7));

      if (kVerbose) {
        kStdOut << "asm: found a base 8 number here:" << lineBuffer.substr(pos) << "\n";
      }

      return numOffset;
    }
    default: {
      if (auto res = strtol(lineBuffer.substr(pos).c_str(), nullptr, 10); !res) {
        if (errno != 0) {
          Detail::print_error("invalid hex number: " + lineBuffer, "CompilerKit");
          throw std::runtime_error("invalid_hex");
        }
      }

      NumberCast32 numOffset(strtol(lineBuffer.substr(pos).c_str(), nullptr, 10));

      if (kVerbose) {
        kStdOut << "asm: found a base 10 number here:" << lineBuffer.substr(pos) << kStdEndl;
      }

      return numOffset;
    }
  }
}
}  // namespace CompilerKit
