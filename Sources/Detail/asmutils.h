#pragma once

using namespace CompilerKit;

/// @brief Get Number from line.
/// @param line
/// @param name
/// @return
static NumberCast32 GetNumber32(std::string line, std::string name) {
  auto pos = line.find(name) + name.size();

  if (line.find(",") != std::string::npos) line.erase(line.find(","), 1);

  while (line[pos] == ' ') ++pos;

  switch (line[pos + 1]) {
    case 'x': {
      if (auto res = strtol(line.substr(pos).c_str(), nullptr, 16); !res) {
        if (errno != 0) {
          detail::print_error("invalid hex number: " + line, "ppcasm");
          throw std::runtime_error("invalid_hex");
        }
      }

      NumberCast32 numOffset(strtol(line.substr(pos).c_str(), nullptr, 16));

      if (kVerbose) {
        kStdOut << "ppcasm: found a base 16 number here: " << line.substr(pos)
                << "\n";
      }

      return numOffset;
    }
    case 'b': {
      if (auto res = strtol(line.substr(pos).c_str(), nullptr, 2); !res) {
        if (errno != 0) {
          detail::print_error("invalid binary number:" + line, "ppcasm");
          throw std::runtime_error("invalid_bin");
        }
      }

      NumberCast32 numOffset(strtol(line.substr(pos).c_str(), nullptr, 2));

      if (kVerbose) {
        kStdOut << "ppcasm: found a base 2 number here:" << line.substr(pos)
                << "\n";
      }

      return numOffset;
    }
    case 'o': {
      if (auto res = strtol(line.substr(pos).c_str(), nullptr, 7); !res) {
        if (errno != 0) {
          detail::print_error("invalid octal number: " + line, "ppcasm");
          throw std::runtime_error("invalid_octal");
        }
      }

      NumberCast32 numOffset(strtol(line.substr(pos).c_str(), nullptr, 7));

      if (kVerbose) {
        kStdOut << "ppcasm: found a base 8 number here:" << line.substr(pos)
                << "\n";
      }

      return numOffset;
    }
    default: {
      if (auto res = strtol(line.substr(pos).c_str(), nullptr, 10); !res) {
        if (errno != 0) {
          detail::print_error("invalid hex number: " + line, "ppcasm");
          throw std::runtime_error("invalid_hex");
        }
      }

      NumberCast32 numOffset(strtol(line.substr(pos).c_str(), nullptr, 10));

      if (kVerbose) {
        kStdOut << "ppcasm: found a base 10 number here:" << line.substr(pos)
                << "\n";
      }

      return numOffset;
    }
  }
}