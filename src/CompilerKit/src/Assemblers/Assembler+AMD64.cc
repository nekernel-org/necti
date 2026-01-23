// Copyright 2024-2025, Amlal El Mahrouss (amlal@nekernel.org)
// Licensed under the Apache License, Version 2.0 (See accompanying
// file LICENSE or copy at http://www.apache.org/licenses/LICENSE-2.0)
// Official repository: https://github.com/nekernel-org/nectar

/////////////////////////////////////////////////////////////////////////////////////////

/// @file Assembler+AMD64.cc
/// @author Amlal El Mahrouss
/// @brief AMD64 Assembler.
/// REMINDER: when dealing with an undefined symbol use (string
/// size):LinkerFindSymbol:(string) so that ld will look for it.

/////////////////////////////////////////////////////////////////////////////////////////

/// bugs: 0

/// feature request: 1
/// Encode registers in mov, add, xor...

/////////////////////////////////////////////////////////////////////////////////////////

#ifndef __ASM_NEED_AMD64__
#define __ASM_NEED_AMD64__
#endif

#define kAssemblerPragmaSymStr "%"
#define kAssemblerPragmaSym '%'

#include <CompilerKit/AE.h>
#include <CompilerKit/AST.h>
#include <CompilerKit/Detail/AMD64.h>
#include <CompilerKit/PEF.h>
/////////////////////

// ANSI ESCAPE CODES

/////////////////////

#define kBlank "\e[0;30m"
#define kRed "\e[0;31m"
#define kWhite "\e[0;97m"
#define kYellow "\e[0;33m"

static char kOutputArch = CompilerKit::kPefArchAMD64;

static constexpr auto kIPAlignement = 0x1U;
static auto           kCounter      = 0x1UL;

static std::uintptr_t                                      kOrigin = kPefBaseOrigin;
static std::vector<std::pair<std::string, std::uintptr_t>> kOriginLabel;

/// @brief keep it simple by default.
static std::int32_t kRegisterBitWidth = 16U;

static std::vector<i64_byte_t> kAppBytes;

static CompilerKit::AERecordHeader kCurrentRecord{
    .fName = "", .fKind = CompilerKit::kPefCode, .fSize = 0, .fOffset = 0};

static std::vector<CompilerKit::AERecordHeader> kRecords;
static std::vector<std::string>                 kDefinedSymbols;
static std::vector<std::string>                 kUndefinedSymbols;

static const std::string kUndefinedSymbol = ":UndefinedSymbol:";

// \brief forward decl.
static bool asm_read_attributes(std::string line);

#include <CompilerKit/Utilities/Assembler.h>

/////////////////////////////////////////////////////////////////////////////////////////

// @brief AMD64 assembler entrypoint, the program/module starts here.

/////////////////////////////////////////////////////////////////////////////////////////

NECTAR_MODULE(AssemblerMainAMD64) {
  //////////////// CPU OPCODES BEGIN ////////////////

  CompilerKit::install_signal(SIGSEGV, CompilerKit::Detail::drvi_crash_handler);

  std::string opcodes_jump[kJumpLimit] = {"ja",  "jae",  "jb",  "jbe",  "jc",  "je",   "jg",  "jge",
                                          "jl",  "jle",  "jna", "jnae", "jnb", "jnbe", "jnc", "jne",
                                          "jng", "jnge", "jnl", "jnle", "jno", "jnp",  "jns", "jnz",
                                          "jo",  "jp",   "jpe", "jpo",  "js",  "jz"};

  for (i64_hword_t i = 0; i < kJumpLimit; i++) {
    CpuOpcodeAMD64 code{.fName   = opcodes_jump[i],
                        .fOpcode = static_cast<i64_hword_t>(kAsmJumpOpcode + i)};
    kOpcodesAMD64.push_back(code);
  }

  CpuOpcodeAMD64 code{.fName = "jcxz", .fOpcode = 0xE3};
  kOpcodesAMD64.push_back(code);

  for (i64_hword_t i = kJumpLimitStandard; i < kJumpLimitStandardLimit; i++) {
    CpuOpcodeAMD64 code{.fName = "jmp", .fOpcode = i};
    kOpcodesAMD64.push_back(code);
  }

  CpuOpcodeAMD64 lahf{.fName = "lahf", .fOpcode = 0x9F};
  kOpcodesAMD64.push_back(lahf);

  CpuOpcodeAMD64 lds{.fName = "lds", .fOpcode = 0xC5};
  kOpcodesAMD64.push_back(lds);

  CpuOpcodeAMD64 lea{.fName = "lea", .fOpcode = 0x8D};
  kOpcodesAMD64.push_back(lea);

  CpuOpcodeAMD64 nop{.fName = "nop", .fOpcode = 0x90};
  kOpcodesAMD64.push_back(nop);

  //////////////// CPU OPCODES END ////////////////

  for (size_t i = 1; i < argc; ++i) {
    if (argv[i][0] == '-') {
      if (strcmp(argv[i], "--version") == 0 || strcmp(argv[i], "-v") == 0) {
        kStdOut << "AssemblerAMD64: AMD64 Assembler Driver.\nAssemblerAMD64: Copyright (c) 2024-2026 "
                   "Amlal El Mahrouss\n";
        kStdOut << "AssemblerAMD64: This Software is part of the NeKernel project. (nekernel.org)\n";
        return 0;
      } else if (strcmp(argv[i], "-help") == 0) {
        kStdOut << "AssemblerAMD64: AMD64 Assembler Driver.\nAssemblerAMD64: Copyright (c) 2024-2026 "
                   "Amlal El Mahrouss\n";
        kStdOut << "AssemblerAMD64: This Software is part of the NeKernel project. (nekernel.org)\n";
        kStdOut << "--version: Print program version.\n";
        kStdOut << "--verbose: Print verbose output.\n";
        kStdOut << "--binary: Output as flat binary.\n";

        return 0;
      } else if (strcmp(argv[i], "--fbinary") == 0) {
        kOutputAsBinary = true;
        continue;
      } else if (strcmp(argv[i], "--fverbose") == 0) {
        kVerbose = true;
        continue;
      }

      kStdOut << "AssemblerAMD64: ignore " << argv[i] << "\n";
      continue;
    }

    if (!std::filesystem::exists(argv[i])) {
      kStdOut << "AssemblerAMD64: can't open: " << argv[i] << std::endl;
      goto asm_fail_exit;
    }

    std::string object_output(argv[i]);
    std::string asm_input(argv[i]);

    for (auto& ext : kAsmFileExts) {
      if (object_output.ends_with(ext)) {
        object_output.erase(object_output.find(ext), std::strlen(ext));
        break;
      }
    }

    object_output += kOutputAsBinary ? kBinaryFileExt : kObjectFileExt;

    std::ifstream file_ptr(argv[i]);
    std::ofstream file_ptr_out(object_output, std::ofstream::binary);

    kStdOut << "AssemblerAMD64: Assembling: " << argv[i] << "\n";

    if (file_ptr_out.bad()) {
      if (kVerbose) {
        kStdOut << "AssemblerAMD64: error: " << strerror(errno) << "\n";
      }

      return 1;
    }

    std::string line;

    CompilerKit::AEHeader hdr{0};

    memset(hdr.fPad, kAENullType, kAEPad);

    hdr.fMagic[0] = kAEMag0;
    hdr.fMagic[1] = kAEMag1;
    hdr.fMagic[2] = kAEMag2;
    hdr.fSize     = sizeof(CompilerKit::AEHeader);
    hdr.fArch     = kOutputArch;

    /////////////////////////////////////////////////////////////////////////////////////////

    // COMPILATION LOOP

    /////////////////////////////////////////////////////////////////////////////////////////

    CompilerKit::EncoderAMD64 asm64;

    if (kVerbose) {
      kStdOut << "Compiling: " + asm_input << "\n";
    }

    while (std::getline(file_ptr, line)) {
      try {
        if (auto ln = asm64.CheckLine(line, argv[i]); !ln.empty()) {
          CompilerKit::Detail::print_error(ln, argv[i]);
          continue;
        }

        asm_read_attributes(line);
        asm64.WriteLine(line, argv[i]);
      } catch (const std::exception& e) {
        if (kVerbose) {
          std::string what = e.what();
          CompilerKit::Detail::print_warning("exit because of: " + what, "CompilerKit");
        }

        try {
          std::filesystem::remove(object_output);
        } catch (...) {
        }

        goto asm_fail_exit;
      }
    }

    if (!kOutputAsBinary) {
      if (kVerbose) {
        kStdOut << "AssemblerAMD64: Writing object file...\n";
      }

      // this is the final step, write everything to the file.

      auto pos = file_ptr_out.tellp();

      hdr.fCount = kRecords.size() + kUndefinedSymbols.size();

      file_ptr_out << hdr;

      if (kRecords.empty()) {
        kStdErr << "AssemblerAMD64: At least one record is needed to write an object "
                   "file.\nAssemblerAMD64: Make one using `public_segment .code64 foo_bar`.\n";

        std::filesystem::remove(object_output);
        return 1;
      }

      kRecords[kRecords.size() - 1].fSize = kAppBytes.size();

      std::size_t record_count = 0UL;

      for (auto& rec : kRecords) {
        if (kVerbose) kStdOut << "AssemblerAMD64: Wrote record " << rec.fName << " to file...\n";

        rec.fFlags |= CompilerKit::kKindRelocationAtRuntime;
        rec.fOffset = record_count;
        ++record_count;

        file_ptr_out << rec;
      }

      // increment once again, so that we won't lie about the kUndefinedSymbols.
      ++record_count;

      for (auto& sym : kUndefinedSymbols) {
        CompilerKit::AERecordHeader _record_hdr{0};

        if (kVerbose) kStdOut << "AssemblerAMD64: Wrote symbol " << sym << " to file...\n";

        _record_hdr.fKind   = kAENullType;
        _record_hdr.fSize   = sym.size();
        _record_hdr.fOffset = record_count;

        ++record_count;

        memset(_record_hdr.fPad, kAENullType, kAEPad);
        memcpy(_record_hdr.fName, sym.c_str(), sym.size());

        file_ptr_out << _record_hdr;

        ++kCounter;
      }

      auto pos_end = file_ptr_out.tellp();

      file_ptr_out.seekp(pos);

      hdr.fStartCode = pos_end;
      hdr.fCodeSize  = kAppBytes.size();

      file_ptr_out << hdr;

      file_ptr_out.seekp(pos_end);
    } else {
      if (kVerbose) {
        kStdOut << "AssemblerAMD64: Write raw binary...\n";
      }
    }

    // byte from byte, we write this.
    for (auto& byte : kAppBytes) {
      file_ptr_out << reinterpret_cast<const char*>(&byte)[0];
    }

    if (kVerbose) kStdOut << "AssemblerAMD64: Wrote file with program in it.\n";

    file_ptr_out.flush();
    file_ptr_out.close();

    if (kVerbose) kStdOut << "AssemblerAMD64: Exit succeeded.\n";

    return 0;
  }

asm_fail_exit:

  if (kVerbose) kStdOut << "AssemblerAMD64: Exit failed.\n";

  return 1;
}

/////////////////////////////////////////////////////////////////////////////////////////

// @brief Check for attributes
// returns true if any was found.

/////////////////////////////////////////////////////////////////////////////////////////

static bool asm_read_attributes(std::string line) {
  // extern_segment is the opposite of public_segment, it signals to the ld
  // that we need this symbol.
  if (CompilerKit::ast_find_needle(line, "extern_segment")) {
    if (kOutputAsBinary) {
      CompilerKit::Detail::print_error("Invalid directive in flat binary mode.", "CompilerKit");
      throw std::runtime_error("invalid_extern_segment_bin");
    }

    auto pos      = line.find("extern_segment");
    auto name_pos = pos + strlen("extern_segment") + 1;

    if (pos == std::string::npos || name_pos >= line.size()) {
      CompilerKit::Detail::print_error("Invalid extern_segment", "power-as");
      throw std::runtime_error("invalid_extern_segment");
    }

    auto name = line.substr(name_pos);

    if (name.size() == 0) {
      CompilerKit::Detail::print_error("Invalid extern_segment", "power-as");
      throw std::runtime_error("invalid_extern_segment");
    }

    std::string result = std::to_string(name.size());
    result += kUndefinedSymbol;

    // mangle this
    for (char& j : name) {
      if (j == ' ' || j == ',') j = '$';
    }

    result += name;

    if (name.find(kPefCode64) != std::string::npos) {
      // data is treated as code.
      kCurrentRecord.fKind = CompilerKit::kPefCode;
    } else if (name.find(kPefData64) != std::string::npos) {
      // no code will be executed from here.
      kCurrentRecord.fKind = CompilerKit::kPefData;
    } else if (name.find(kPefZero64) != std::string::npos) {
      // this is a bss section.
      kCurrentRecord.fKind = CompilerKit::kPefZero;
    }

    // now we can tell the code size of the previous kCurrentRecord.

    if (!kRecords.empty()) kRecords[kRecords.size() - 1].fSize = kAppBytes.size();

    memset(kCurrentRecord.fName, 0, kAESymbolLen);
    memcpy(kCurrentRecord.fName, result.c_str(), result.size());

    ++kCounter;

    memset(kCurrentRecord.fPad, kAENullType, kAEPad);

    kRecords.emplace_back(kCurrentRecord);

    return true;
  }
  // public_segment is a special keyword used by AssemblerAMD64 to tell the AE output stage to
  // mark this section as a header. it currently supports .code64, .data64 and
  // .zero64.
  else if (CompilerKit::ast_find_needle(line, "public_segment")) {
    if (kOutputAsBinary) {
      CompilerKit::Detail::print_error("Invalid directive in flat binary mode.", "CompilerKit");
      throw std::runtime_error("invalid_public_segment_bin");
    }

    auto res_sym_at = (line.find("public_segment") + strlen("public_segment") + 1);
    if (res_sym_at > line.size()) {
      CompilerKit::Detail::print_error("Invalid symbol for public_segment.", "CompilerKit");
      throw std::runtime_error("invalid_public_segment_symbol");
    }

    auto name = line.substr(res_sym_at);

    std::string name_copy = name;

    for (char& j : name) {
      if (j == ' ') j = '$';
    }

    if (std::find(kDefinedSymbols.begin(), kDefinedSymbols.end(), name) != kDefinedSymbols.end()) {
      CompilerKit::Detail::print_error("Symbol already defined.", "CompilerKit");
      throw std::runtime_error("invalid_public_segment_bin");
    }

    kDefinedSymbols.push_back(name);

    if (name.find(kPefCode64) != std::string::npos) {
      // data is treated as code.
      kCurrentRecord.fKind = CompilerKit::kPefCode;
    } else if (name.find(kPefData64) != std::string::npos) {
      // no code will be executed from here.
      kCurrentRecord.fKind = CompilerKit::kPefData;
    } else if (name.find(kPefZero64) != std::string::npos) {
      // this is a bss section.
      kCurrentRecord.fKind = CompilerKit::kPefZero;
    }

    while (name_copy.find(" ") != std::string::npos) name_copy.erase(name_copy.find(" "), 1);

    kOriginLabel.push_back(std::make_pair(name_copy, kOrigin));
    ++kOrigin;

    // now we can tell the code size of the previous kCurrentRecord.

    if (!kRecords.empty()) kRecords[kRecords.size() - 1].fSize = kAppBytes.size();

    memset(kCurrentRecord.fName, 0, kAESymbolLen);
    memcpy(kCurrentRecord.fName, name.c_str(), name.size());

    ++kCounter;

    memset(kCurrentRecord.fPad, kAENullType, kAEPad);

    kRecords.emplace_back(kCurrentRecord);

    return true;
  }

  return false;
}

// \brief algorithms and helpers.

namespace CompilerKit::Detail::algorithm {
// \brief authorize a brief set of characters.
static inline bool is_not_valid(char c) {
  if ((isalpha(c) || isdigit(c)) ||
      ((c == ' ') || (c == '\t') || (c == ',') || (c == '(') || (c == ')') || (c == '"') ||
       (c == '*') || (c == '\'') || (c == '[') || (c == ']') || (c == '+') || (c == '_') ||
       (c == ':') || (c == '@') || (c == '.') || (c == '#') || (c == '%') || (c == '~') ||
       (c == ';')))
    return false;

  return true;
}

bool is_valid_amd64(std::string str) {
  return std::find_if(str.begin(), str.end(), is_not_valid) == str.end();
}
}  // namespace CompilerKit::Detail::algorithm

/////////////////////////////////////////////////////////////////////////////////////////

// @brief Check for line (syntax check)

/////////////////////////////////////////////////////////////////////////////////////////

std::string CompilerKit::EncoderAMD64::CheckLine(std::string line, std::string file) {
  std::string err_str;

  if (line.empty() || CompilerKit::ast_find_needle(line, "extern_segment") ||
      CompilerKit::ast_find_needle(line, "public_segment") ||
      CompilerKit::ast_find_needle(line, kAssemblerPragmaSymStr) ||
      CompilerKit::ast_find_needle(line, ";") || line[0] == kAssemblerPragmaSym) {
    if (line.find(';') != std::string::npos) {
      line.erase(line.find(';'));
    } else {
      // now check the line for validity
      if (!CompilerKit::Detail::algorithm::is_valid_amd64(line)) {
        err_str = "Line contains non valid characters.\nhere -> ";
        err_str += line;
      }
    }

    return err_str;
  }

  // check for a valid instruction format.

  if (line.find(',') != std::string::npos) {
    if (line.find(',') + 1 == line.size()) {
      err_str += "\nInstruction lacks right register, here -> ";
      err_str += line.substr(line.find(','));

      return err_str;
    } else {
      bool nothing_on_right = true;

      if (line.find(',') + 1 > line.size()) {
        err_str += "\nInstruction not complete, here -> ";
        err_str += line;

        return err_str;
      }

      auto substr = line.substr(line.find(',') + 1);

      for (auto& ch : substr) {
        if (ch != ' ' && ch != '\t') {
          nothing_on_right = false;
        }
      }

      // this means we found nothing after that ',' .
      if (nothing_on_right) {
        err_str += "\nInstruction not complete, here -> ";
        err_str += line;

        return err_str;
      }
    }
  }
  for (auto& opcodeAMD64 : kOpcodesAMD64) {
    if (CompilerKit::ast_find_needle(line, opcodeAMD64.fName)) {
      return err_str;
    }
  }

  err_str += "\nUnrecognized instruction -> " + line;

  return err_str;
}

bool CompilerKit::EncoderAMD64::WriteNumber(const std::size_t& pos, std::string& jump_label) {
  if (!isdigit(jump_label[pos])) return false;

  switch (jump_label[pos + 1]) {
    case 'x': {
      auto res = strtol(jump_label.substr(pos + 2).c_str(), nullptr, 16);
      res += kOrigin;

      if (errno != 0) {
        return false;
      }

      CompilerKit::NumberCast64 num = CompilerKit::NumberCast64(res);

      for (char& i : num.number) {
        kAppBytes.push_back(i);
      }

      if (kVerbose) {
        kStdOut << "AssemblerAMD64: Found a base 16 number here: " << jump_label.substr(pos)
                << "\n";
      }

      return true;
    }
    case 'b': {
      auto res = strtol(jump_label.substr(pos + 2).c_str(), nullptr, 2);
      res += kOrigin;

      if (errno != 0) {
        return false;
      }

      CompilerKit::NumberCast64 num = CompilerKit::NumberCast64(res);

      if (kVerbose) {
        kStdOut << "AssemblerAMD64: Found a base 2 number here: " << jump_label.substr(pos) << "\n";
      }

      for (char& i : num.number) {
        kAppBytes.push_back(i);
      }

      return true;
    }
    case 'o': {
      auto res = strtol(jump_label.substr(pos + 2).c_str(), nullptr, 8);
      res += kOrigin;

      if (errno != 0) {
        return false;
      }

      CompilerKit::NumberCast64 num = CompilerKit::NumberCast64(res);

      if (kVerbose) {
        kStdOut << "AssemblerAMD64: Found a base 8 number here: " << jump_label.substr(pos) << "\n";
      }

      for (char& i : num.number) {
        kAppBytes.push_back(i);
      }

      return true;
    }
    default: {
      break;
    }
  }

  /// @note We assume base 10 here. As other cases have failed.
  auto res = strtol(jump_label.substr(pos + 1).c_str(), nullptr, 10);
  res += kOrigin;

  if (errno != 0) {
    return false;
  }

  CompilerKit::NumberCast64 num = CompilerKit::NumberCast64(res);

  for (char& i : num.number) {
    kAppBytes.push_back(i);
  }

  if (kVerbose) {
    kStdOut << "AssemblerAMD64: Found a base 10 number here: " << jump_label.substr(pos + 1) << "\n";
  }

  return true;
}

bool CompilerKit::EncoderAMD64::WriteNumber32(const std::size_t& pos, std::string& jump_label) {
  if (!isdigit(jump_label[pos])) return false;

  switch (jump_label[pos + 1]) {
    case 'x': {
      auto res = strtol(jump_label.substr(pos + 2).c_str(), nullptr, 16);
      res += kOrigin;

      if (errno != 0) {
        return false;
      }

      CompilerKit::NumberCast32 num = CompilerKit::NumberCast32(res);

      for (char& i : num.number) {
        kAppBytes.push_back(i);
      }

      if (kVerbose) {
        kStdOut << "AssemblerAMD64: Found a base 16 number here: " << jump_label.substr(pos)
                << "\n";
      }

      return true;
    }
    case 'b': {
      auto res = strtol(jump_label.substr(pos + 2).c_str(), nullptr, 2);
      res += kOrigin;

      if (errno != 0) {
        return false;
      }

      CompilerKit::NumberCast32 num = CompilerKit::NumberCast32(res);

      if (kVerbose) {
        kStdOut << "AssemblerAMD64: Found a base 2 number here: " << jump_label.substr(pos) << "\n";
      }

      for (char& i : num.number) {
        kAppBytes.push_back(i);
      }

      return true;
    }
    case 'o': {
      auto res = strtol(jump_label.substr(pos + 2).c_str(), nullptr, 8);
      res += kOrigin;

      if (errno != 0) {
        return false;
      }

      CompilerKit::NumberCast32 num = CompilerKit::NumberCast32(res);

      if (kVerbose) {
        kStdOut << "AssemblerAMD64: Found a base 8 number here: " << jump_label.substr(pos) << "\n";
      }

      for (char& i : num.number) {
        kAppBytes.push_back(i);
      }

      return true;
    }
    default: {
      break;
    }
  }

  auto res = strtol(jump_label.substr(pos + 2).c_str(), nullptr, 10);
  res += kOrigin;

  if (errno != 0) {
    return false;
  }

  CompilerKit::NumberCast32 num = CompilerKit::NumberCast32(res);

  for (char& i : num.number) {
    kAppBytes.push_back(i);
  }

  if (kVerbose) {
    kStdOut << "AssemblerAMD64: Found a base 10 number here: " << jump_label.substr(pos) << "\n";
  }

  return true;
}

bool CompilerKit::EncoderAMD64::WriteNumber16(const std::size_t& pos, std::string& jump_label) {
  if (!isdigit(jump_label[pos])) return false;

  switch (jump_label[pos + 1]) {
    case 'x': {
      if (auto res = strtol(jump_label.substr(pos + 2).c_str(), nullptr, 16); !res) {
        if (errno != 0) {
          CompilerKit::Detail::print_error("Invalid hex number: " + jump_label, "CompilerKit");
          throw std::runtime_error("invalid_hex");
        }
      }

      CompilerKit::NumberCast16 num =
          CompilerKit::NumberCast16(strtol(jump_label.substr(pos + 2).c_str(), nullptr, 16));

      for (char& i : num.number) {
        kAppBytes.push_back(i);
      }

      if (kVerbose) {
        kStdOut << "AssemblerAMD64: Found a base 16 number here: " << jump_label.substr(pos)
                << "\n";
      }

      return true;
    }
    case 'b': {
      if (auto res = strtol(jump_label.substr(pos + 2).c_str(), nullptr, 2); !res) {
        if (errno != 0) {
          CompilerKit::Detail::print_error("Invalid binary number: " + jump_label, "CompilerKit");
          throw std::runtime_error("invalid_bin");
        }
      }

      CompilerKit::NumberCast16 num =
          CompilerKit::NumberCast16(strtol(jump_label.substr(pos + 2).c_str(), nullptr, 2));

      if (kVerbose) {
        kStdOut << "AssemblerAMD64: Found a base 2 number here: " << jump_label.substr(pos) << "\n";
      }

      for (char& i : num.number) {
        kAppBytes.push_back(i);
      }

      return true;
    }
    case 'o': {
      if (auto res = strtol(jump_label.substr(pos + 2).c_str(), nullptr, 8); !res) {
        if (errno != 0) {
          CompilerKit::Detail::print_error("Invalid octal number: " + jump_label, "CompilerKit");
          throw std::runtime_error("invalid_octal");
        }
      }

      CompilerKit::NumberCast16 num =
          CompilerKit::NumberCast16(strtol(jump_label.substr(pos + 2).c_str(), nullptr, 8));

      if (kVerbose) {
        kStdOut << "AssemblerAMD64: Found a base 8 number here: " << jump_label.substr(pos) << "\n";
      }

      for (char& i : num.number) {
        kAppBytes.push_back(i);
      }

      return true;
    }
    default: {
      break;
    }
  }

  /* check for errno and stuff like that */
  if (auto res = strtol(jump_label.substr(pos).c_str(), nullptr, 10); !res) {
    if (errno != 0) {
      return false;
    }
  }

  CompilerKit::NumberCast16 num =
      CompilerKit::NumberCast16(strtol(jump_label.substr(pos).c_str(), nullptr, 10));

  for (char& i : num.number) {
    kAppBytes.push_back(i);
  }

  if (kVerbose) {
    kStdOut << "AssemblerAMD64: Found a base 10 number here: " << jump_label.substr(pos) << "\n";
  }

  return true;
}

bool CompilerKit::EncoderAMD64::WriteNumber8(const std::size_t& pos, std::string& jump_label) {
  if (!isdigit(jump_label[pos])) return false;

  switch (jump_label[pos + 1]) {
    case 'x': {
      auto res = strtol(jump_label.substr(pos + 2).c_str(), nullptr, 16);
      res += kOrigin;

      if (errno != 0) {
        return false;
      }

      CompilerKit::NumberCast8 num = CompilerKit::NumberCast8(res);

      kAppBytes.push_back(num.number);

      if (kVerbose) {
        kStdOut << "AssemblerAMD64: Found a base 16 number here: " << jump_label.substr(pos)
                << "\n";
      }

      return true;
    }
    case 'b': {
      auto res = strtol(jump_label.substr(pos + 2).c_str(), nullptr, 2);
      res += kOrigin;

      if (errno != 0) {
        return false;
      }

      CompilerKit::NumberCast8 num = CompilerKit::NumberCast8(res);

      if (kVerbose) {
        kStdOut << "AssemblerAMD64: Found a base 2 number here: " << jump_label.substr(pos) << "\n";
      }

      kAppBytes.push_back(num.number);

      return true;
    }
    case 'o': {
      auto res = strtol(jump_label.substr(pos + 2).c_str(), nullptr, 8);
      res += kOrigin;

      if (errno != 0) {
        return false;
      }

      CompilerKit::NumberCast8 num = CompilerKit::NumberCast8(res);

      if (kVerbose) {
        kStdOut << "AssemblerAMD64: Found a base 8 number here: " << jump_label.substr(pos) << "\n";
      }

      kAppBytes.push_back(num.number);

      return true;
    }
    default: {
      break;
    }
  }

  auto res = strtol(jump_label.substr(pos).c_str(), nullptr, 10);
  res += kOrigin;

  if (errno != 0) {
    return false;
  }

  CompilerKit::NumberCast8 num = CompilerKit::NumberCast8(res);

  kAppBytes.push_back(num.number);

  if (kVerbose) {
    kStdOut << "AssemblerAMD64: Found a base 10 number here: " << jump_label.substr(pos) << "\n";
  }

  return true;
}

/////////////////////////////////////////////////////////////////////////////////////////

// @brief Read and write an instruction to the output array.

/////////////////////////////////////////////////////////////////////////////////////////

bool CompilerKit::EncoderAMD64::WriteLine(std::string line, std::string file) {
  if (CompilerKit::ast_find_needle(line, "public_segment ")) return true;

  struct RegMapAMD64 {
    CompilerKit::STLString fName;
    i64_byte_t             fModRM;
  };

  std::vector<RegMapAMD64> kRegisterList{
      {.fName = "ax", .fModRM = 0x0}, {.fName = "cx", .fModRM = 1},
      {.fName = "dx", .fModRM = 0x2}, {.fName = "bx", .fModRM = 3},
      {.fName = "sp", .fModRM = 0x4}, {.fName = "bp", .fModRM = 5},
      {.fName = "si", .fModRM = 0x6}, {.fName = "di", .fModRM = 7},
  };

  bool foundInstruction = false;

  for (auto& opcodeAMD64 : kOpcodesAMD64) {
    // strict check here
    if (CompilerKit::ast_find_needle(line, opcodeAMD64.fName) &&
        CompilerKit::Detail::algorithm::is_valid_amd64(line)) {
      foundInstruction = true;
      std::string name(opcodeAMD64.fName);

      /// Move instruction handler.
      if (line.find(name) != std::string::npos) {
        if (name == "mov" || name == "xor") {
          std::string substr = line.substr(line.find(name) + name.size());

          uint64_t bits = kRegisterBitWidth;

          if (substr.find(",") == std::string::npos) {
            CompilerKit::Detail::print_error("Syntax error: missing right operand.", "CompilerKit");
            throw std::runtime_error("syntax_err");
          }

          /// Handle [reg+n] or [reg-n] memory addressing for any register
          if (substr.find('[') != std::string::npos) {
            // Parse the memory operand
            auto bracketStart = substr.find('[');
            auto bracketEnd   = substr.find(']');

            if (bracketStart == std::string::npos || bracketEnd == std::string::npos) {
              CompilerKit::Detail::print_error("Syntax error: malformed memory operand.", file);
              throw std::runtime_error("syntax_err");
            }

            std::string memOperand = substr.substr(bracketStart + 1, bracketEnd - bracketStart - 1);

            // Register lookup table
            struct RegInfo {
              const char* name;
              i64_byte_t  code;
            };

            RegInfo regs64[] = {{"rax", 0}, {"rcx", 1}, {"rdx", 2}, {"rbx", 3},
                                {"rsp", 4}, {"rbp", 5}, {"rsi", 6}, {"rdi", 7}};

            // Find base register in memory operand
            i64_byte_t baseReg   = 0;
            bool       foundBase = false;

            for (auto& reg : regs64) {
              if (memOperand.find(reg.name) != std::string::npos) {
                baseReg   = reg.code;
                foundBase = true;
                break;
              }
            }

            if (!foundBase) {
              CompilerKit::Detail::print_error("Invalid base register in memory operand.", file);
              throw std::runtime_error("invalid_base_reg");
            }

            bool isRbp = (baseReg == 5);
            bool isRsp = (baseReg == 4);

            // Parse displacement
            int32_t displacement = 0;
            bool    hasDisp      = false;

            auto plusPos  = memOperand.find('+');
            auto minusPos = memOperand.find('-');

            if (plusPos != std::string::npos) {
              std::string dispStr = memOperand.substr(plusPos + 1);
              displacement        = static_cast<int32_t>(strtol(dispStr.c_str(), nullptr, 0));
              hasDisp             = true;
            } else if (minusPos != std::string::npos) {
              std::string dispStr = memOperand.substr(minusPos + 1);
              displacement        = -static_cast<int32_t>(strtol(dispStr.c_str(), nullptr, 0));
              hasDisp             = true;
            }

            // Determine if destination is memory or register
            auto commaPos     = substr.find(',');
            bool destIsMemory = bracketStart < commaPos;

            // Find register in the other operand
            std::string otherOperand;
            if (destIsMemory) {
              otherOperand = substr.substr(commaPos + 1);
            } else {
              otherOperand = substr.substr(0, commaPos);
            }

            // Remove whitespace
            while (!otherOperand.empty() && (otherOperand[0] == ' ' || otherOperand[0] == '\t')) {
              otherOperand.erase(0, 1);
            }

            // Check for register in other operand
            i64_byte_t regCode     = 0;
            bool       foundReg    = false;
            bool       isImmediate = false;
            int64_t    immValue    = 0;

            for (auto& reg : regs64) {
              if (otherOperand.find(reg.name) != std::string::npos) {
                regCode  = reg.code;
                foundReg = true;
                break;
              }
            }

            if (!foundReg) {
              // Check if it's an immediate value
              std::string immStr = otherOperand;
              while (!immStr.empty() && (immStr[0] == ' ' || immStr[0] == '\t')) {
                immStr.erase(0, 1);
              }
              if (!immStr.empty() && (isdigit(immStr[0]) || immStr[0] == '-')) {
                isImmediate = true;
                immValue    = strtol(immStr.c_str(), nullptr, 0);
              }
            }

            // Determine mod field based on displacement size
            // mod=00: [reg] no displacement (except rbp which requires disp8)
            // mod=01: [reg+disp8]
            // mod=10: [reg+disp32]
            i64_byte_t mod = 0;
            if (!hasDisp && displacement == 0) {
              // [rbp] requires disp8 with 0, can't use mod=00 (it means RIP-relative)
              mod = isRbp ? 0x01 : 0x00;
            } else if (displacement >= -128 && displacement <= 127) {
              mod = 0x01;  // 8-bit displacement
            } else {
              mod = 0x02;  // 32-bit displacement
            }

            if (destIsMemory) {
              if (foundReg) {
                // mov [reg+n], reg
                kAppBytes.emplace_back(0x48);  // REX.W
                kAppBytes.emplace_back(0x89);  // MOV r/m64, r64

                // ModR/M: mod | reg << 3 | r/m
                i64_byte_t modrm = (mod << 6) | (regCode << 3) | baseReg;
                kAppBytes.emplace_back(modrm);

                // RSP needs SIB byte
                if (isRsp) {
                  kAppBytes.emplace_back(0x24);  // SIB: scale=0, index=4(none), base=4(rsp)
                }
              } else if (isImmediate) {
                // mov qword [reg+n], imm32
                kAppBytes.emplace_back(0x48);  // REX.W
                kAppBytes.emplace_back(0xC7);  // MOV r/m64, imm32

                // ModR/M: mod | 0 << 3 | r/m (reg field is 0 for this opcode)
                i64_byte_t modrm = (mod << 6) | (0 << 3) | baseReg;
                kAppBytes.emplace_back(modrm);

                // RSP needs SIB byte
                if (isRsp) {
                  kAppBytes.emplace_back(0x24);
                }
              } else {
                CompilerKit::Detail::print_error("Invalid source operand for mov to memory.", file);
                throw std::runtime_error("invalid_operand");
              }
            } else {
              // mov reg, [reg+n]
              kAppBytes.emplace_back(0x48);  // REX.W
              kAppBytes.emplace_back(0x8B);  // MOV r64, r/m64

              // ModR/M: mod | reg << 3 | r/m
              i64_byte_t modrm = (mod << 6) | (regCode << 3) | baseReg;
              kAppBytes.emplace_back(modrm);

              // RSP needs SIB byte
              if (isRsp) {
                kAppBytes.emplace_back(0x24);
              }
            }

            // Write displacement
            if (mod == 0x01) {
              // 8-bit displacement
              kAppBytes.emplace_back(static_cast<i64_byte_t>(displacement & 0xFF));
            } else if (mod == 0x02) {
              // 32-bit displacement
              kAppBytes.emplace_back(static_cast<i64_byte_t>(displacement & 0xFF));
              kAppBytes.emplace_back(static_cast<i64_byte_t>((displacement >> 8) & 0xFF));
              kAppBytes.emplace_back(static_cast<i64_byte_t>((displacement >> 16) & 0xFF));
              kAppBytes.emplace_back(static_cast<i64_byte_t>((displacement >> 24) & 0xFF));
            } else if (isRbp) {
              // rbp with mod=00 still needs disp8=0
              kAppBytes.emplace_back(0x00);
            }

            // Write immediate if present
            if (destIsMemory && isImmediate) {
              kAppBytes.emplace_back(static_cast<i64_byte_t>(immValue & 0xFF));
              kAppBytes.emplace_back(static_cast<i64_byte_t>((immValue >> 8) & 0xFF));
              kAppBytes.emplace_back(static_cast<i64_byte_t>((immValue >> 16) & 0xFF));
              kAppBytes.emplace_back(static_cast<i64_byte_t>((immValue >> 24) & 0xFF));
            }

            break;
          }

          bool onlyOneReg = true;

          std::vector<RegMapAMD64> currentRegList;

          for (auto& reg : kRegisterList) {
            std::string registerName;

            if (bits == 32)
              registerName.push_back('e');
            else if (bits == 64)
              registerName.push_back('r');
            else {
              CompilerKit::Detail::print_error("Invalid size for register, current bit width is: " +
                                                   std::to_string(kRegisterBitWidth),
                                               file);
              throw std::runtime_error("invalid_reg_size");
            }

            registerName += reg.fName;

            while (line.find(registerName) != std::string::npos) {
              line.erase(line.find(registerName), registerName.size());

              if (bits == 16) {
                if (registerName[0] == 'r') {
                  CompilerKit::Detail::print_error(
                      "Invalid size for register, current bit width is: " +
                          std::to_string(kRegisterBitWidth),
                      file);
                  throw std::runtime_error("invalid_reg_size");
                }
              }

              currentRegList.push_back({.fName = registerName, .fModRM = reg.fModRM});
            }
          }

          if (currentRegList.size() > 1) onlyOneReg = false;

          bool hasRBasedRegs = false;

          if (!onlyOneReg) {
            /// very tricky to understand.
            /// but this checks for a r8 through r15 register.
            if (currentRegList[0].fName[0] == 'r' || currentRegList[1].fName[0] == 'r') {
              if (isdigit(currentRegList[0].fName[1]) && isdigit(currentRegList[1].fName[1])) {
                kAppBytes.emplace_back(0x4d);
                hasRBasedRegs = true;
              } else if (isdigit(currentRegList[0].fName[1]) ||
                         isdigit(currentRegList[1].fName[1])) {
                kAppBytes.emplace_back(0x4c);
                hasRBasedRegs = true;
              }
            }
          }

          if (name == "mov") {
            if (bits == 64 || bits == 32) {
              if (!hasRBasedRegs && bits >= 32) {
                kAppBytes.emplace_back(opcodeAMD64.fOpcode);
              } else if (hasRBasedRegs && bits == 32) {
                CompilerKit::Detail::print_error("Invalid combination of operands and registers.",
                                                 "CompilerKit");
                throw std::runtime_error("comb_op_reg");
              }

              if (!onlyOneReg) kAppBytes.emplace_back(0x89);
            } else if (bits == 16) {
              if (hasRBasedRegs) {
                CompilerKit::Detail::print_error("Invalid combination of operands and registers.",
                                                 "CompilerKit");
                throw std::runtime_error("comb_op_reg");
              } else {
                kAppBytes.emplace_back(0x66);
                kAppBytes.emplace_back(0x89);
              }
            }
          } else {
            if (!hasRBasedRegs && bits >= 32) {
              kAppBytes.emplace_back(opcodeAMD64.fOpcode);
            }

            kAppBytes.emplace_back(0x31);
          }

          if (onlyOneReg) {
            auto num = GetNumber32(line, ",");

            auto modrm = (0x3 << 6 | currentRegList[0].fModRM);

            kAppBytes.emplace_back(0xC7);  // prefixed before placing the modrm and then the number.
            kAppBytes.emplace_back(modrm);

            if (name != "xor") {
              kAppBytes.emplace_back(num.number[0]);
              kAppBytes.emplace_back(num.number[1]);
              kAppBytes.emplace_back(num.number[2]);
              kAppBytes.emplace_back(num.number[3]);
            }

            break;
          }

          if (currentRegList[1].fName[0] == 'r' && currentRegList[0].fName[0] == 'e') {
            CompilerKit::Detail::print_error("Invalid combination of operands and registers.",
                                             "CompilerKit");
            throw std::runtime_error("comb_op_reg");
          }

          if (currentRegList[0].fName[0] == 'r' && currentRegList[1].fName[0] == 'e') {
            CompilerKit::Detail::print_error("Invalid combination of operands and registers.",
                                             "CompilerKit");
            throw std::runtime_error("comb_op_reg");
          }

          if (bits == 16) {
            if (currentRegList[0].fName[0] == 'r' || currentRegList[0].fName[0] == 'e') {
              CompilerKit::Detail::print_error("Invalid combination of operands and registers.",
                                               "CompilerKit");
              throw std::runtime_error("comb_op_reg");
            }

            if (currentRegList[1].fName[0] == 'r' || currentRegList[1].fName[0] == 'e') {
              CompilerKit::Detail::print_error("Invalid combination of operands and registers.",
                                               "CompilerKit");
              throw std::runtime_error("comb_op_reg");
            }
          } else {
            if (currentRegList[0].fName[0] != 'r' || currentRegList[0].fName[0] == 'e') {
              CompilerKit::Detail::print_error("Invalid combination of operands and registers.",
                                               "CompilerKit");
              throw std::runtime_error("comb_op_reg");
            }

            if (currentRegList[1].fName[0] != 'r' || currentRegList[1].fName[0] == 'e') {
              CompilerKit::Detail::print_error("Invalid combination of operands and registers.",
                                               "CompilerKit");
              throw std::runtime_error("comb_op_reg");
            }
          }

          /// encode register using the modrm encoding.

          auto modrm = (0x3 << 6 | currentRegList[1].fModRM << 3 | currentRegList[0].fModRM);

          kAppBytes.emplace_back(modrm);

          break;
        }

        /// Compare instruction handler.
        if (name == "cmp") {
          std::string substr = line.substr(line.find(name) + name.size());

          if (substr.find(",") == std::string::npos) {
            CompilerKit::Detail::print_error("Syntax error: missing right operand.", "CompilerKit");
            throw std::runtime_error("syntax_err");
          }

          // Register lookup table
          struct RegInfo {
            const char* name;
            i64_byte_t  code;
          };

          RegInfo regs64[] = {{"rax", 0}, {"rcx", 1}, {"rdx", 2}, {"rbx", 3},
                              {"rsp", 4}, {"rbp", 5}, {"rsi", 6}, {"rdi", 7}};

          /// Handle [reg+n] memory addressing
          if (substr.find('[') != std::string::npos) {
            auto bracketStart = substr.find('[');
            auto bracketEnd   = substr.find(']');

            if (bracketEnd == std::string::npos) {
              CompilerKit::Detail::print_error("Syntax error: malformed memory operand.", file);
              throw std::runtime_error("syntax_err");
            }

            std::string memOperand = substr.substr(bracketStart + 1, bracketEnd - bracketStart - 1);

            // Find base register
            i64_byte_t baseReg   = 0;
            bool       foundBase = false;

            for (auto& reg : regs64) {
              if (memOperand.find(reg.name) != std::string::npos) {
                baseReg   = reg.code;
                foundBase = true;
                break;
              }
            }

            if (!foundBase) {
              CompilerKit::Detail::print_error("Invalid base register in memory operand.", file);
              throw std::runtime_error("invalid_base_reg");
            }

            bool isRbp = (baseReg == 5);
            bool isRsp = (baseReg == 4);

            // Parse displacement
            int32_t displacement = 0;
            bool    hasDisp      = false;

            auto plusPos  = memOperand.find('+');
            auto minusPos = memOperand.find('-');

            if (plusPos != std::string::npos) {
              std::string dispStr = memOperand.substr(plusPos + 1);
              displacement        = static_cast<int32_t>(strtol(dispStr.c_str(), nullptr, 0));
              hasDisp             = true;
            } else if (minusPos != std::string::npos) {
              std::string dispStr = memOperand.substr(minusPos + 1);
              displacement        = -static_cast<int32_t>(strtol(dispStr.c_str(), nullptr, 0));
              hasDisp             = true;
            }

            auto commaPos     = substr.find(',');
            bool destIsMemory = bracketStart < commaPos;

            std::string otherOperand;
            if (destIsMemory) {
              otherOperand = substr.substr(commaPos + 1);
            } else {
              otherOperand = substr.substr(0, commaPos);
            }

            while (!otherOperand.empty() && (otherOperand[0] == ' ' || otherOperand[0] == '\t')) {
              otherOperand.erase(0, 1);
            }

            i64_byte_t regCode     = 0;
            bool       foundReg    = false;
            bool       isImmediate = false;
            int64_t    immValue    = 0;

            for (auto& reg : regs64) {
              if (otherOperand.find(reg.name) != std::string::npos) {
                regCode  = reg.code;
                foundReg = true;
                break;
              }
            }

            if (!foundReg) {
              std::string immStr = otherOperand;
              while (!immStr.empty() && (immStr[0] == ' ' || immStr[0] == '\t')) {
                immStr.erase(0, 1);
              }
              if (!immStr.empty() && (isdigit(immStr[0]) || immStr[0] == '-')) {
                isImmediate = true;
                immValue    = strtol(immStr.c_str(), nullptr, 0);
              }
            }

            // Determine mod field
            i64_byte_t mod = 0;
            if (!hasDisp && displacement == 0) {
              mod = isRbp ? 0x01 : 0x00;
            } else if (displacement >= -128 && displacement <= 127) {
              mod = 0x01;
            } else {
              mod = 0x02;
            }

            if (destIsMemory) {
              if (foundReg) {
                // cmp [reg+n], reg
                kAppBytes.emplace_back(0x48);  // REX.W
                kAppBytes.emplace_back(0x39);  // CMP r/m64, r64

                i64_byte_t modrm = (mod << 6) | (regCode << 3) | baseReg;
                kAppBytes.emplace_back(modrm);

                if (isRsp) {
                  kAppBytes.emplace_back(0x24);
                }
              } else if (isImmediate) {
                // cmp qword [reg+n], imm32
                kAppBytes.emplace_back(0x48);  // REX.W
                kAppBytes.emplace_back(0x81);  // CMP r/m64, imm32

                // reg field = 7 for CMP
                i64_byte_t modrm = (mod << 6) | (7 << 3) | baseReg;
                kAppBytes.emplace_back(modrm);

                if (isRsp) {
                  kAppBytes.emplace_back(0x24);
                }
              }
            } else {
              // cmp reg, [reg+n]
              kAppBytes.emplace_back(0x48);  // REX.W
              kAppBytes.emplace_back(0x3B);  // CMP r64, r/m64

              i64_byte_t modrm = (mod << 6) | (regCode << 3) | baseReg;
              kAppBytes.emplace_back(modrm);

              if (isRsp) {
                kAppBytes.emplace_back(0x24);
              }
            }

            // Write displacement
            if (mod == 0x01) {
              kAppBytes.emplace_back(static_cast<i64_byte_t>(displacement & 0xFF));
            } else if (mod == 0x02) {
              kAppBytes.emplace_back(static_cast<i64_byte_t>(displacement & 0xFF));
              kAppBytes.emplace_back(static_cast<i64_byte_t>((displacement >> 8) & 0xFF));
              kAppBytes.emplace_back(static_cast<i64_byte_t>((displacement >> 16) & 0xFF));
              kAppBytes.emplace_back(static_cast<i64_byte_t>((displacement >> 24) & 0xFF));
            } else if (isRbp) {
              kAppBytes.emplace_back(0x00);
            }

            // Write immediate
            if (destIsMemory && isImmediate) {
              kAppBytes.emplace_back(static_cast<i64_byte_t>(immValue & 0xFF));
              kAppBytes.emplace_back(static_cast<i64_byte_t>((immValue >> 8) & 0xFF));
              kAppBytes.emplace_back(static_cast<i64_byte_t>((immValue >> 16) & 0xFF));
              kAppBytes.emplace_back(static_cast<i64_byte_t>((immValue >> 24) & 0xFF));
            }

            break;
          }

          // Handle register-to-register and register-to-immediate
          i64_byte_t reg1Code    = 0;
          i64_byte_t reg2Code    = 0;
          bool       foundReg1   = false;
          bool       foundReg2   = false;
          bool       isImmediate = false;
          int64_t    immValue    = 0;

          auto        commaPos     = substr.find(',');
          std::string leftOperand  = substr.substr(0, commaPos);
          std::string rightOperand = substr.substr(commaPos + 1);

          while (!leftOperand.empty() && (leftOperand[0] == ' ' || leftOperand[0] == '\t')) {
            leftOperand.erase(0, 1);
          }
          while (!rightOperand.empty() && (rightOperand[0] == ' ' || rightOperand[0] == '\t')) {
            rightOperand.erase(0, 1);
          }

          for (auto& reg : regs64) {
            if (leftOperand.find(reg.name) != std::string::npos) {
              reg1Code  = reg.code;
              foundReg1 = true;
              break;
            }
          }

          for (auto& reg : regs64) {
            if (rightOperand.find(reg.name) != std::string::npos) {
              reg2Code  = reg.code;
              foundReg2 = true;
              break;
            }
          }

          if (!foundReg2) {
            if (!rightOperand.empty() && (isdigit(rightOperand[0]) || rightOperand[0] == '-')) {
              isImmediate = true;
              immValue    = strtol(rightOperand.c_str(), nullptr, 0);
            }
          }

          if (foundReg1 && foundReg2) {
            // cmp reg1, reg2
            kAppBytes.emplace_back(0x48);  // REX.W
            kAppBytes.emplace_back(0x39);  // CMP r/m64, r64

            i64_byte_t modrm = (0x3 << 6) | (reg2Code << 3) | reg1Code;
            kAppBytes.emplace_back(modrm);
          } else if (foundReg1 && isImmediate) {
            // cmp reg, imm
            kAppBytes.emplace_back(0x48);  // REX.W
            kAppBytes.emplace_back(0x81);  // CMP r/m64, imm32

            // reg field = 7 for CMP
            i64_byte_t modrm = (0x3 << 6) | (7 << 3) | reg1Code;
            kAppBytes.emplace_back(modrm);

            kAppBytes.emplace_back(static_cast<i64_byte_t>(immValue & 0xFF));
            kAppBytes.emplace_back(static_cast<i64_byte_t>((immValue >> 8) & 0xFF));
            kAppBytes.emplace_back(static_cast<i64_byte_t>((immValue >> 16) & 0xFF));
            kAppBytes.emplace_back(static_cast<i64_byte_t>((immValue >> 24) & 0xFF));
          } else {
            CompilerKit::Detail::print_error("Invalid operands for cmp instruction.", file);
            throw std::runtime_error("invalid_cmp_operands");
          }

          break;
        }

        /// LEA instruction handler.
        if (name == "lea") {
          std::string substr = line.substr(line.find(name) + name.size());

          // Remove leading whitespace
          while (!substr.empty() && (substr[0] == ' ' || substr[0] == '\t')) {
            substr.erase(0, 1);
          }

          if (substr.find(",") == std::string::npos || substr.find('[') == std::string::npos) {
            CompilerKit::Detail::print_error("Syntax error: lea requires reg, [mem] format.", file);
            throw std::runtime_error("syntax_err");
          }

          // Register lookup table
          struct RegInfo {
            const char* name;
            i64_byte_t  code;
          };

          RegInfo regs64[] = {{"rax", 0}, {"rcx", 1}, {"rdx", 2}, {"rbx", 3},
                              {"rsp", 4}, {"rbp", 5}, {"rsi", 6}, {"rdi", 7}};

          auto        commaPos    = substr.find(',');
          std::string destOperand = substr.substr(0, commaPos);
          std::string srcOperand  = substr.substr(commaPos + 1);

          // Remove whitespace
          while (!destOperand.empty() && (destOperand[0] == ' ' || destOperand[0] == '\t')) {
            destOperand.erase(0, 1);
          }
          while (!srcOperand.empty() && (srcOperand[0] == ' ' || srcOperand[0] == '\t')) {
            srcOperand.erase(0, 1);
          }

          // Find destination register
          i64_byte_t destReg   = 0;
          bool       foundDest = false;

          for (auto& reg : regs64) {
            if (destOperand.find(reg.name) != std::string::npos) {
              destReg   = reg.code;
              foundDest = true;
              break;
            }
          }

          if (!foundDest) {
            CompilerKit::Detail::print_error("Invalid destination register for lea.", file);
            throw std::runtime_error("invalid_dest_reg");
          }

          // Parse memory operand [base+disp] or [base-disp]
          auto bracketStart = srcOperand.find('[');
          auto bracketEnd   = srcOperand.find(']');

          if (bracketStart == std::string::npos || bracketEnd == std::string::npos) {
            CompilerKit::Detail::print_error("Syntax error: malformed memory operand for lea.",
                                             file);
            throw std::runtime_error("syntax_err");
          }

          std::string memOperand =
              srcOperand.substr(bracketStart + 1, bracketEnd - bracketStart - 1);

          // Find base register
          i64_byte_t baseReg   = 0;
          bool       foundBase = false;

          for (auto& reg : regs64) {
            if (memOperand.find(reg.name) != std::string::npos) {
              baseReg   = reg.code;
              foundBase = true;
              break;
            }
          }

          if (!foundBase) {
            CompilerKit::Detail::print_error("Invalid base register in memory operand for lea.",
                                             file);
            throw std::runtime_error("invalid_base_reg");
          }

          bool    isRbp        = (baseReg == 5);
          bool    isRsp        = (baseReg == 4);
          int32_t displacement = 0;
          bool    hasDisp      = false;

          // Look for +/- displacement
          auto plusPos  = memOperand.find('+');
          auto minusPos = memOperand.find('-');

          if (plusPos != std::string::npos) {
            std::string dispStr = memOperand.substr(plusPos + 1);
            displacement        = static_cast<int32_t>(strtol(dispStr.c_str(), nullptr, 0));
            hasDisp             = true;
          } else if (minusPos != std::string::npos) {
            std::string dispStr = memOperand.substr(minusPos + 1);
            displacement        = -static_cast<int32_t>(strtol(dispStr.c_str(), nullptr, 0));
            hasDisp             = true;
          }

          // Determine mod field
          i64_byte_t mod = 0x00;
          if (hasDisp || isRbp) {
            if (displacement >= -128 && displacement <= 127) {
              mod = 0x01;  // 8-bit displacement
            } else {
              mod = 0x02;  // 32-bit displacement
            }
          }

          // Emit REX.W prefix for 64-bit
          kAppBytes.emplace_back(0x48);

          // Emit LEA opcode
          kAppBytes.emplace_back(0x8D);

          // Emit ModR/M byte
          i64_byte_t modrm = (mod << 6) | (destReg << 3) | baseReg;
          kAppBytes.emplace_back(modrm);

          // RSP needs SIB byte
          if (isRsp) {
            kAppBytes.emplace_back(0x24);
          }

          // Emit displacement
          if (mod == 0x01) {
            kAppBytes.emplace_back(static_cast<i64_byte_t>(displacement & 0xFF));
          } else if (mod == 0x02) {
            kAppBytes.emplace_back(static_cast<i64_byte_t>(displacement & 0xFF));
            kAppBytes.emplace_back(static_cast<i64_byte_t>((displacement >> 8) & 0xFF));
            kAppBytes.emplace_back(static_cast<i64_byte_t>((displacement >> 16) & 0xFF));
            kAppBytes.emplace_back(static_cast<i64_byte_t>((displacement >> 24) & 0xFF));
          } else if (isRbp) {
            // RBP with no displacement needs [rbp+0]
            kAppBytes.emplace_back(0x00);
          }

          break;
        }

        /// Push instruction handler.
        if (name == "push" || name == "pop") {
          std::string substr = line.substr(line.find(name) + name.size());

          // Remove leading whitespace
          while (!substr.empty() && (substr[0] == ' ' || substr[0] == '\t')) {
            substr.erase(0, 1);
          }

          i64_byte_t baseOpcode = (name == "push") ? kAsmPushOpcode : kAsmPopOpcode;
          bool       found      = false;

          // Check for extended registers r8-r15
          if (substr.size() >= 2 && substr[0] == 'r' && isdigit(substr[1])) {
            int regNum = 0;

            if (substr.size() >= 3 && isdigit(substr[2])) {
              regNum = (substr[1] - '0') * 10 + (substr[2] - '0');
            } else {
              regNum = substr[1] - '0';
            }

            if (regNum >= 8 && regNum <= 15) {
              // REX.B prefix for r8-r15
              kAppBytes.emplace_back(0x41);
              kAppBytes.emplace_back(baseOpcode + (regNum - 8));
              found = true;
            }
          }

          // Check for standard 64-bit registers rax-rdi
          if (!found) {
            struct RegPushPop {
              const char* name;
              i64_byte_t  offset;
            };

            RegPushPop regs[] = {{"rax", 0}, {"rcx", 1}, {"rdx", 2}, {"rbx", 3},
                                 {"rsp", 4}, {"rbp", 5}, {"rsi", 6}, {"rdi", 7}};

            for (auto& reg : regs) {
              if (substr.find(reg.name) != std::string::npos) {
                kAppBytes.emplace_back(baseOpcode + reg.offset);
                found = true;
                break;
              }
            }
          }

          if (!found) {
            if (isnumber(substr[0])) {
              kAppBytes.emplace_back(name == "push" ? 0x68 : 0x8F);

              // push imm always takes a 32-bit immediate (sign-extended in 64-bit mode)
              // Parse the immediate value without adding kOrigin
              long imm = 0;
              if (substr.size() > 2 && substr[0] == '0' && substr[1] == 'x') {
                imm = strtol(substr.c_str() + 2, nullptr, 16);
              } else if (substr.size() > 2 && substr[0] == '0' && substr[1] == 'b') {
                imm = strtol(substr.c_str() + 2, nullptr, 2);
              } else if (substr.size() > 2 && substr[0] == '0' && substr[1] == 'o') {
                imm = strtol(substr.c_str() + 2, nullptr, 8);
              } else {
                imm = strtol(substr.c_str(), nullptr, 10);
              }

              CompilerKit::NumberCast32 num(imm);
              if (kRegisterBitWidth == 64 || kRegisterBitWidth == 32) {
                kAppBytes.emplace_back(num.number[0]);
                kAppBytes.emplace_back(num.number[1]);
                kAppBytes.emplace_back(num.number[2]);
                kAppBytes.emplace_back(num.number[3]);
              } else if (kRegisterBitWidth == 16) {
                kAppBytes.emplace_back(num.number[0]);
                kAppBytes.emplace_back(num.number[1]);
              }

              break;
            }

            CompilerKit::Detail::print_error("Invalid operand for " + name + ": " + substr,
                                             "CompilerKit");
            throw std::runtime_error("invalid_push_pop_operand");
          }

          break;
        }
      }

      if (name == "int" || name == "into" || name == "intd") {
        kAppBytes.emplace_back(opcodeAMD64.fOpcode);
        this->WriteNumber8(line.find(name) + name.size() + 1, line);

        break;
      } else if (name == "jmp" || name == "call") {
        kAppBytes.emplace_back(opcodeAMD64.fOpcode);

        if (kRegisterBitWidth == 64) {
          this->WriteNumber(line.find(name) + name.size() + 1, line);
        } else {
          this->WriteNumber32(line.find(name) + name.size() + 1, line);
        }
        break;
      }

      if (name == "syscall") {
        kAppBytes.emplace_back(opcodeAMD64.fOpcode);
        kAppBytes.emplace_back(0x05);
        break;
      } else {
        kAppBytes.emplace_back(opcodeAMD64.fOpcode);

        break;
      }
    }
  }

  if (line[0] == kAssemblerPragmaSym) {
    if (foundInstruction) {
      CompilerKit::Detail::print_error("Syntax error: " + line, file);
      throw std::runtime_error("syntax_err");
    }

    if (line.find("bits 64") != std::string::npos) {
      kRegisterBitWidth = 64U;
    } else if (line.find("bits 32") != std::string::npos) {
      kRegisterBitWidth = 32U;
    } else if (line.find("bits 16") != std::string::npos) {
      kRegisterBitWidth = 16U;
    }

    if (auto org_pos = line.find("org"); org_pos != std::string::npos) {
      auto value_pos = org_pos + strlen("org") + 1;

      if (value_pos >= line.size()) {
        CompilerKit::Detail::print_error("Invalid org directive", "CompilerKit");
        throw std::runtime_error("invalid_org");
      }

      size_t base[] = {10, 16, 2, 8};

      for (size_t i = 0; i < 4; i++) {
        if (kOrigin = strtol(line.substr(value_pos).c_str(), nullptr, base[i]); kOrigin) {
          if (errno != 0) {
            continue;
          } else {
            if (kVerbose) {
              kStdOut << "AssemblerAMD64: Origin Set: " << kOrigin << std::endl;
            }

            break;
          }
        }
      }
    }
  }
  /// write a dword
  else if (auto pos = line.find(".dword"); pos != std::string::npos) {
    this->WriteNumber32(pos + strlen(".dword") + 1, line);
  }
  /// write a long
  else if (auto pos = line.find(".long"); pos != std::string::npos) {
    this->WriteNumber(pos + strlen(".long") + 1, line);
  }
  /// write a 16-bit number
  else if (auto pos = line.find(".word"); pos != std::string::npos) {
    this->WriteNumber16(pos + strlen(".word") + 1, line);
  }

  kOrigin += kIPAlignement;

  return true;
}

// Last rev 13-1-24
