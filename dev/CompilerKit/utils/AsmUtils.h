/* -------------------------------------------

  Copyright (C) 2024-2025 Amlal EL Mahrouss, all rights reserved

------------------------------------------- */

#pragma once

#include <CompilerKit/CodeGen.h>
#include <CompilerKit/Frontend.h>

#include <CompilerKit/utils/CompilerUtils.h>

using namespace CompilerKit;

/// @brief Get Number from lineBuffer.
/// @param lineBuffer the lineBuffer to fetch from.
/// @param numberKey where to seek that number.
/// @return
static NumberCast32 GetNumber32(std::string lineBuffer, std::string numberKey) {
  auto pos = lineBuffer.find(numberKey) + numberKey.size();

  while (lineBuffer[pos] == ' ') {
    ++pos;
  }

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
        kStdOut << "asm: found a base 10 number here:" << lineBuffer.substr(pos) << "\n";
      }

      return numOffset;
    }
  }
}
