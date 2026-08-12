// Copyright 2024-2025, Amlal El Mahrouss (amlal@nekernel.org)
// Licensed under the Apache License, Version 2.0 (See accompanying
// file LICENSE or copy at http://www.apache.org/licenses/LICENSE-2.0)
// Official repository: https://github.com/ne-app-eu/ncc

#ifndef NCC_COMPILERKIT_UTILITIES_ASSEMBLER_H
#define NCC_COMPILERKIT_UTILITIES_ASSEMBLER_H

#include <CompilerKit/AST.h>
#include <CompilerKit/CodeGenerator.h>
#include <CompilerKit/Utils/Compiler.h>

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
      auto         res = strtol(lineBuffer.substr(pos + 2).c_str(), nullptr, 16);
      NumberCast32 numOffset(strtol(lineBuffer.substr(pos + 2).c_str(), nullptr, 16));

      if (kVerbose) {
        kStdOut << "asm: found a base 16 number here: " << lineBuffer.substr(pos) << "\n";
      }

      return numOffset;
    }
    case 'b': {
      auto         res = strtol(lineBuffer.substr(pos + 2).c_str(), nullptr, 2);
      NumberCast32 numOffset(strtol(lineBuffer.substr(pos + 2).c_str(), nullptr, 2));

      if (kVerbose) {
        kStdOut << "asm: found a base 2 number here:" << lineBuffer.substr(pos) << "\n";
      }

      return numOffset;
    }
    case 'o': {
      auto         res = strtol(lineBuffer.substr(pos + 2).c_str(), nullptr, 8);
      NumberCast32 numOffset(strtol(lineBuffer.substr(pos + 2).c_str(), nullptr, 8));

      if (kVerbose) {
        kStdOut << "asm: found a base 8 number here:" << lineBuffer.substr(pos) << "\n";
      }

      return numOffset;
    }
    default: {
      auto         res = strtol(lineBuffer.substr(pos + 2).c_str(), nullptr, 10);
      NumberCast32 numOffset(strtol(lineBuffer.substr(pos + 2).c_str(), nullptr, 10));

      if (kVerbose) {
        kStdOut << "asm: found a base 10 number here:" << lineBuffer.substr(pos) << kStdEndl;
      }

      return numOffset;
    }
  }
}
}  // namespace CompilerKit

#endif  // NCC_COMPILERKIT_UTILITIES_ASSEMBLER_H
