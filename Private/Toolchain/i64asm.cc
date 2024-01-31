/*
 *	========================================================
 *
 *	i64asm
 * 	Copyright Mahrouss Logic, all rights reserved.
 *
 * 	========================================================
 */

/// bugs: 0

/////////////////////////////////////////////////////////////////////////////////////////

// @file i64asm.cxx
// @author Amlal El Mahrouss
// @brief AMD64 Assembler.

// REMINDER: when dealing with an undefined symbol use (string
// size):LinkerFindSymbol:(string) so that ld will look for it.

/////////////////////////////////////////////////////////////////////////////////////////

#define __ASM_NEED_AMD64__ 1

#include <CompilerKit/AsmKit/Arch/amd64.hpp>
#include <CompilerKit/ParserKit.hpp>
#include <CompilerKit/StdKit/AE.hpp>
#include <CompilerKit/StdKit/PEF.hpp>
#include <filesystem>
#include <fstream>
#include <iostream>

/////////////////////

// ANSI ESCAPE CODES

/////////////////////

#define kBlank "\e[0;30m"
#define kRed "\e[0;31m"
#define kWhite "\e[0;97m"
#define kYellow "\e[0;33m"

#define kStdOut (std::cout << kWhite)
#define kStdErr (std::cout << kRed)

static char kOutputArch = CompilerKit::kPefArchAMD64;
static Boolean kOutputAsBinary = false;

static UInt32 kErrorLimit = 10;
static UInt32 kAcceptableErrors = 0;

static std::size_t kCounter = 1UL;

static std::uintptr_t kOrigin = kPefBaseOrigin;
static std::vector<std::pair<std::string, std::uintptr_t>> kOriginLabel;

static bool kVerbose = false;

static std::vector<e64_byte_t> kBytes;

static CompilerKit::AERecordHeader kCurrentRecord{
    .fName = "", .fKind = CompilerKit::kPefCode, .fSize = 0, .fOffset = 0};

static std::vector<CompilerKit::AERecordHeader> kRecords;
static std::vector<std::string> kUndefinedSymbols;

static const std::string kUndefinedSymbol = ":ld:";
static const std::string kRelocSymbol = ":mld:";

// \brief forward decl.
static bool asm_read_attributes(std::string &line);

namespace detail {
void print_error(std::string reason, const std::string &file) noexcept {
  if (reason[0] == '\n') reason.erase(0, 1);

  kStdErr << kRed << "[ i64asm ] " << kWhite
          << ((file == "i64asm") ? "internal assembler error "
                                 : ("in file, " + file))
          << kBlank << std::endl;
  kStdErr << kRed << "[ i64asm ] " << kWhite << reason << kBlank << std::endl;

  if (kAcceptableErrors > kErrorLimit) std::exit(3);

  ++kAcceptableErrors;
}

void print_warning(std::string reason, const std::string &file) noexcept {
  if (reason[0] == '\n') reason.erase(0, 1);

  if (!file.empty()) {
    kStdOut << kYellow << "[ file ] " << kWhite << file << kBlank << std::endl;
  }

  kStdOut << kYellow << "[ i64asm ] " << kWhite << reason << kBlank
          << std::endl;
}
}  // namespace detail

/////////////////////////////////////////////////////////////////////////////////////////

// @brief AMD64 assembler entrypoint, the program/module starts here.

/////////////////////////////////////////////////////////////////////////////////////////

MPCC_MODULE(MPUXAssemblerAMD64) {
  //////////////// CPU OPCODES BEGIN ////////////////

  std::string opcodes_jump[kJumpLimit] = {
      "ja",  "jae",  "jb",  "jbe",  "jc",  "je",  "jg",  "jge",  "jl",  "jle",
      "jna", "jnae", "jnb", "jnbe", "jnc", "jne", "jng", "jnge", "jnl", "jnle",
      "jno", "jnp",  "jns", "jnz",  "jo",  "jp",  "jpe", "jpo",  "js",  "jz"};

  for (e64_hword_t i = 0; i < kJumpLimit; i++) {
    CpuCodeAMD64 code{.fName = opcodes_jump[i],
                      .fOpcode = static_cast<e64_hword_t>(kAsmJumpOpcode + i)};
    kOpcodesAMD64.push_back(code);
  }

  CpuCodeAMD64 code{.fName = "jcxz", .fOpcode = 0xE3};
  kOpcodesAMD64.push_back(code);

  for (e64_hword_t i = kJumpLimitStandard; i < kJumpLimitStandardLimit; i++) {
    CpuCodeAMD64 code{.fName = "jmp", .fOpcode = i};
    kOpcodesAMD64.push_back(code);
  }

  CpuCodeAMD64 lahf{.fName = "lahf", .fOpcode = 0x9F};
  kOpcodesAMD64.push_back(lahf);

  CpuCodeAMD64 lds{.fName = "lds", .fOpcode = 0xC5};
  kOpcodesAMD64.push_back(lds);

  CpuCodeAMD64 lea{.fName = "lea", .fOpcode = 0x8D};
  kOpcodesAMD64.push_back(lea);

  CpuCodeAMD64 mov{.fName = "nop", .fOpcode = 0x90};
  kOpcodesAMD64.push_back(mov);

  //////////////// CPU OPCODES END ////////////////

  for (size_t i = 1; i < argc; ++i) {
    if (argv[i][0] == '-') {
      if (strcmp(argv[i], "-version") == 0 || strcmp(argv[i], "-v") == 0) {
        kStdOut << "i64asm: AMD64 Assembler.\ni64asm: v1.10\ni64asm: Copyright "
                   "(c) 2024 Mahrouss Logic.\n";
        return 0;
      } else if (strcmp(argv[i], "-h") == 0) {
        kStdOut << "i64asm: AMD64 Assembler.\ni64asm: Copyright (c) 2024 "
                   "Mahrouss Logic.\n";
        kStdOut << "-version: Print program version.\n";
        kStdOut << "-verbose: Print verbose output.\n";
        kStdOut << "-binary: Output as flat binary.\n";
        kStdOut << "-64xxx: Compile for a subset of the X64000.\n";

        return 0;
      } else if (strcmp(argv[i], "-binary") == 0) {
        kOutputAsBinary = true;
        continue;
      } else if (strcmp(argv[i], "-verbose") == 0) {
        kVerbose = true;
        continue;
      }

      kStdOut << "i64asm: ignore " << argv[i] << "\n";
      continue;
    }

    if (!std::filesystem::exists(argv[i])) {
      kStdOut << "i64asm: can't open: " << argv[i] << std::endl;
      goto asm_fail_exit;
    }

    std::string object_output(argv[i]);

    for (auto &ext : kAsmFileExts) {
      if (object_output.find(ext) != std::string::npos) {
        object_output.erase(object_output.find(ext), std::strlen(ext));
      }
    }

    object_output += kObjectFileExt;

    std::ifstream file_ptr(argv[i]);
    std::ofstream file_ptr_out(object_output, std::ofstream::binary);

    if (file_ptr_out.bad()) {
      if (kVerbose) {
        kStdOut << "i64asm: error: " << strerror(errno) << "\n";
      }
    }

    std::string line;

    CompilerKit::AEHeader hdr{0};

    memset(hdr.fPad, kAEInvalidOpcode, kAEPad);

    hdr.fMagic[0] = kAEMag0;
    hdr.fMagic[1] = kAEMag1;
    hdr.fSize = sizeof(CompilerKit::AEHeader);
    hdr.fArch = kOutputArch;

    /////////////////////////////////////////////////////////////////////////////////////////

    // COMPILATION LOOP

    /////////////////////////////////////////////////////////////////////////////////////////

    CompilerKit::PlatformAssemblerAMD64 asm64;

    while (std::getline(file_ptr, line)) {
      if (auto ln = asm64.CheckLine(line, argv[i]); !ln.empty()) {
        detail::print_error(ln, argv[i]);
        continue;
      }

      try {
        asm_read_attributes(line);
        asm64.WriteLine(line, argv[i]);
      } catch (const std::exception &e) {
        if (kVerbose) {
          std::string what = e.what();
          detail::print_warning("exit because of: " + what, "i64asm");
        }

        std::filesystem::remove(object_output);
        goto asm_fail_exit;
      }
    }

    if (!kOutputAsBinary) {
      if (kVerbose) {
        kStdOut << "i64asm: Writing object file...\n";
      }

      // this is the final step, write everything to the file.

      auto pos = file_ptr_out.tellp();

      hdr.fCount = kRecords.size() + kUndefinedSymbols.size();

      file_ptr_out << hdr;

      if (kRecords.empty()) {
        kStdErr << "i64asm: At least one record is needed to write an object "
                   "file.\ni64asm: Make one using `export .text foo_bar`.\n";

        std::filesystem::remove(object_output);
        return -1;
      }

      kRecords[kRecords.size() - 1].fSize = kBytes.size();

      std::size_t record_count = 0UL;

      for (auto &rec : kRecords) {
        if (kVerbose)
          kStdOut << "i64asm: Wrote record " << rec.fName << " to file...\n";

        rec.fFlags |= CompilerKit::kKindRelocationAtRuntime;
        rec.fOffset = record_count;
        ++record_count;

        file_ptr_out << rec;
      }

      // increment once again, so that we won't lie about the kUndefinedSymbols.
      ++record_count;

      for (auto &sym : kUndefinedSymbols) {
        CompilerKit::AERecordHeader _record_hdr{0};

        if (kVerbose)
          kStdOut << "i64asm: Wrote symbol " << sym << " to file...\n";

        _record_hdr.fKind = kAEInvalidOpcode;
        _record_hdr.fSize = sym.size();
        _record_hdr.fOffset = record_count;

        ++record_count;

        memset(_record_hdr.fPad, kAEInvalidOpcode, kAEPad);
        memcpy(_record_hdr.fName, sym.c_str(), sym.size());

        file_ptr_out << _record_hdr;

        ++kCounter;
      }

      auto pos_end = file_ptr_out.tellp();

      file_ptr_out.seekp(pos);

      hdr.fStartCode = pos_end;
      hdr.fCodeSize = kBytes.size();

      file_ptr_out << hdr;

      file_ptr_out.seekp(pos_end);
    } else {
      if (kVerbose) {
        kStdOut << "i64asm: Write raw binary...\n";
      }
    }

    // byte from byte, we write this.
    for (auto &byte : kBytes) {
      if (byte == 0) continue;

      if (byte == 0xFF) {
        byte = 0;
      }

      file_ptr_out << reinterpret_cast<const char *>(&byte)[0];
    }

    if (kVerbose) kStdOut << "i64asm: Wrote file with program in it.\n";

    file_ptr_out.flush();
    file_ptr_out.close();

    if (kVerbose) kStdOut << "i64asm: Exit succeeded.\n";

    return 0;
  }

asm_fail_exit:

  if (kVerbose) kStdOut << "i64asm: Exit failed.\n";

  return -1;
}

/////////////////////////////////////////////////////////////////////////////////////////

// @brief Check for attributes
// returns true if any was found.

/////////////////////////////////////////////////////////////////////////////////////////

static bool asm_read_attributes(std::string &line) {
  // import is the opposite of export, it signals to the ld
  // that we need this symbol.
  if (ParserKit::find_word(line, "import ")) {
    if (kOutputAsBinary) {
      detail::print_error("invalid import directive in flat binary mode.",
                          "i64asm");
      throw std::runtime_error("invalid_import_bin");
    }

    auto name = line.substr(line.find("import ") + strlen("import "));

    std::string result = std::to_string(name.size());
    result += kUndefinedSymbol;

    // mangle this
    for (char &j : name) {
      if (j == ' ' || j == ',') j = '$';
    }

    result += name;

    if (name.find(".text") != std::string::npos) {
      // data is treated as code.
      kCurrentRecord.fKind = CompilerKit::kPefCode;
    } else if (name.find(".data") != std::string::npos) {
      // no code will be executed from here.
      kCurrentRecord.fKind = CompilerKit::kPefData;
    } else if (name.find(".page_zero") != std::string::npos) {
      // this is a bss section.
      kCurrentRecord.fKind = CompilerKit::kPefZero;
    }

    // this is a special case for the start stub.
    // we want this so that ld can find it.

    if (name == "__start") {
      kCurrentRecord.fKind = CompilerKit::kPefCode;
    }

    // now we can tell the code size of the previous kCurrentRecord.

    if (!kRecords.empty()) kRecords[kRecords.size() - 1].fSize = kBytes.size();

    memset(kCurrentRecord.fName, 0, kAESymbolLen);
    memcpy(kCurrentRecord.fName, result.c_str(), result.size());

    ++kCounter;

    memset(kCurrentRecord.fPad, kAEInvalidOpcode, kAEPad);

    kRecords.emplace_back(kCurrentRecord);

    return true;
  }
  // export is a special keyword used by i64asm to tell the AE output stage to
  // mark this section as a header. it currently supports .text, .data.,
  // page_zero
  else if (ParserKit::find_word(line, "export ")) {
    if (kOutputAsBinary) {
      detail::print_error("invalid export directive in flat binary mode.",
                          "i64asm");
      throw std::runtime_error("invalid_export_bin");
    }

    auto name = line.substr(line.find("export ") + strlen("export "));

    std::string name_copy = name;

    for (char &j : name) {
      if (j == ' ') j = '$';
    }

    if (name.find(".text") != std::string::npos) {
      // data is treated as code.

      name_copy.erase(name_copy.find(".text"), strlen(".text"));
      kCurrentRecord.fKind = CompilerKit::kPefCode;
    } else if (name.find(".data") != std::string::npos) {
      // no code will be executed from here.

      name_copy.erase(name_copy.find(".data"), strlen(".data"));
      kCurrentRecord.fKind = CompilerKit::kPefData;
    } else if (name.find(".page_zero") != std::string::npos) {
      // this is a bss section.

      name_copy.erase(name_copy.find(".page_zero"), strlen(".page_zero"));
      kCurrentRecord.fKind = CompilerKit::kPefZero;
    }

    // this is a special case for the start stub.
    // we want this so that ld can find it.

    if (name == "__start") {
      kCurrentRecord.fKind = CompilerKit::kPefCode;
    }

    while (name_copy.find(" ") != std::string::npos)
      name_copy.erase(name_copy.find(" "), 1);

    kOriginLabel.push_back(std::make_pair(name_copy, kOrigin));
    ++kOrigin;

    // now we can tell the code size of the previous kCurrentRecord.

    if (!kRecords.empty()) kRecords[kRecords.size() - 1].fSize = kBytes.size();

    memset(kCurrentRecord.fName, 0, kAESymbolLen);
    memcpy(kCurrentRecord.fName, name.c_str(), name.size());

    ++kCounter;

    memset(kCurrentRecord.fPad, kAEInvalidOpcode, kAEPad);

    kRecords.emplace_back(kCurrentRecord);

    return true;
  }

  return false;
}

// \brief algorithms and helpers.

namespace detail::algorithm {
// \brief authorize a brief set of characters.
static inline bool is_not_alnum_space(char c) {
  return !(isalpha(c) || isdigit(c) || (c == ' ') || (c == '\t') ||
           (c == ',') || (c == '(') || (c == ')') || (c == '"') ||
           (c == '\'') || (c == '[') || (c == ']') || (c == '+') ||
           (c == '_') || (c == ':') || (c == '@') || (c == '.'));
}

bool is_valid(const std::string &str) {
  return find_if(str.begin(), str.end(), is_not_alnum_space) == str.end();
}
}  // namespace detail::algorithm

/////////////////////////////////////////////////////////////////////////////////////////

// @brief Check for line (syntax check)

/////////////////////////////////////////////////////////////////////////////////////////

std::string CompilerKit::PlatformAssemblerAMD64::CheckLine(
    std::string &line, const std::string &file) {
  std::string err_str;

  if (line.empty() || ParserKit::find_word(line, "import") ||
      ParserKit::find_word(line, "export") || ParserKit::find_word(line, "#") ||
      ParserKit::find_word(line, ";")) {
    if (line.find('#') != std::string::npos) {
      line.erase(line.find('#'));
    } else if (line.find(';') != std::string::npos) {
      line.erase(line.find(';'));
    } else {
      // now check the line for validity
      if (!detail::algorithm::is_valid(line)) {
        err_str = "Line contains non alphanumeric characters.\nhere -> ";
        err_str += line;
      }
    }

    return err_str;
  }

  if (!detail::algorithm::is_valid(line)) {
    err_str = "Line contains non alphanumeric characters.\nhere -> ";
    err_str += line;

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

      for (auto &ch : substr) {
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

  return err_str;
}

bool CompilerKit::PlatformAssemblerAMD64::WriteNumber(const std::size_t &pos,
                                                      std::string &jump_label) {
  if (!isdigit(jump_label[pos])) return false;

  switch (jump_label[pos + 1]) {
    case 'x': {
      if (auto res = strtoq(jump_label.substr(pos + 2).c_str(), nullptr, 16);
          !res) {
        if (errno != 0) {
          detail::print_error("invalid hex number: " + jump_label, "i64asm");
          throw std::runtime_error("invalid_hex");
        }
      }

      CompilerKit::NumberCast64 num = CompilerKit::NumberCast64(
          strtoq(jump_label.substr(pos + 2).c_str(), nullptr, 16));

      for (char &i : num.number) {
        if (i == 0) i = 0xFF;

        kBytes.push_back(i);
      }

      if (kVerbose) {
        kStdOut << "i64asm: found a base 16 number here: "
                << jump_label.substr(pos) << "\n";
      }

      return true;
    }
    case 'b': {
      if (auto res = strtoq(jump_label.substr(pos + 2).c_str(), nullptr, 2);
          !res) {
        if (errno != 0) {
          detail::print_error("invalid binary number: " + jump_label, "i64asm");
          throw std::runtime_error("invalid_bin");
        }
      }

      CompilerKit::NumberCast64 num = CompilerKit::NumberCast64(
          strtoq(jump_label.substr(pos + 2).c_str(), nullptr, 2));

      if (kVerbose) {
        kStdOut << "i64asm: found a base 2 number here: "
                << jump_label.substr(pos) << "\n";
      }

      for (char &i : num.number) {
        if (i == 0) i = 0xFF;

        kBytes.push_back(i);
      }

      return true;
    }
    case 'o': {
      if (auto res = strtoq(jump_label.substr(pos + 2).c_str(), nullptr, 7);
          !res) {
        if (errno != 0) {
          detail::print_error("invalid octal number: " + jump_label, "i64asm");
          throw std::runtime_error("invalid_octal");
        }
      }

      CompilerKit::NumberCast64 num = CompilerKit::NumberCast64(
          strtoq(jump_label.substr(pos + 2).c_str(), nullptr, 7));

      if (kVerbose) {
        kStdOut << "i64asm: found a base 8 number here: "
                << jump_label.substr(pos) << "\n";
      }

      for (char &i : num.number) {
        if (i == 0) i = 0xFF;

        kBytes.push_back(i);
      }

      return true;
    }
    default: {
      break;
    }
  }

  /* check for errno and stuff like that */
  if (auto res = strtoq(jump_label.substr(pos).c_str(), nullptr, 10); !res) {
    if (errno != 0) {
      return false;
    }
  }

  CompilerKit::NumberCast64 num = CompilerKit::NumberCast64(
      strtoq(jump_label.substr(pos).c_str(), nullptr, 10));

  for (char &i : num.number) {
    if (i == 0) i = 0xFF;

    kBytes.push_back(i);
  }

  if (kVerbose) {
    kStdOut << "i64asm: found a base 10 number here: " << jump_label.substr(pos)
            << "\n";
  }

  return true;
}

bool CompilerKit::PlatformAssemblerAMD64::WriteNumber32(
    const std::size_t &pos, std::string &jump_label) {
  if (!isdigit(jump_label[pos])) return false;

  switch (jump_label[pos + 1]) {
    case 'x': {
      if (auto res = strtoq(jump_label.substr(pos + 2).c_str(), nullptr, 16);
          !res) {
        if (errno != 0) {
          detail::print_error("invalid hex number: " + jump_label, "i64asm");
          throw std::runtime_error("invalid_hex");
        }
      }

      CompilerKit::NumberCast32 num = CompilerKit::NumberCast32(
          strtoq(jump_label.substr(pos + 2).c_str(), nullptr, 16));

      for (char &i : num.number) {
        if (i == 0) i = 0xFF;

        kBytes.push_back(i);
      }

      if (kVerbose) {
        kStdOut << "i64asm: found a base 16 number here: "
                << jump_label.substr(pos) << "\n";
      }

      return true;
    }
    case 'b': {
      if (auto res = strtoq(jump_label.substr(pos + 2).c_str(), nullptr, 2);
          !res) {
        if (errno != 0) {
          detail::print_error("invalid binary number: " + jump_label, "i64asm");
          throw std::runtime_error("invalid_bin");
        }
      }

      CompilerKit::NumberCast32 num = CompilerKit::NumberCast32(
          strtoq(jump_label.substr(pos + 2).c_str(), nullptr, 2));

      if (kVerbose) {
        kStdOut << "i64asm: found a base 2 number here: "
                << jump_label.substr(pos) << "\n";
      }

      for (char &i : num.number) {
        if (i == 0) i = 0xFF;

        kBytes.push_back(i);
      }

      return true;
    }
    case 'o': {
      if (auto res = strtoq(jump_label.substr(pos + 2).c_str(), nullptr, 7);
          !res) {
        if (errno != 0) {
          detail::print_error("invalid octal number: " + jump_label, "i64asm");
          throw std::runtime_error("invalid_octal");
        }
      }

      CompilerKit::NumberCast32 num = CompilerKit::NumberCast32(
          strtoq(jump_label.substr(pos + 2).c_str(), nullptr, 7));

      if (kVerbose) {
        kStdOut << "i64asm: found a base 8 number here: "
                << jump_label.substr(pos) << "\n";
      }

      for (char &i : num.number) {
        if (i == 0) i = 0xFF;

        kBytes.push_back(i);
      }

      return true;
    }
    default: {
      break;
    }
  }

  /* check for errno and stuff like that */
  if (auto res = strtoq(jump_label.substr(pos).c_str(), nullptr, 10); !res) {
    if (errno != 0) {
      return false;
    }
  }

  CompilerKit::NumberCast32 num = CompilerKit::NumberCast32(
      strtoq(jump_label.substr(pos).c_str(), nullptr, 10));

  for (char &i : num.number) {
    if (i == 0) i = 0xFF;

    kBytes.push_back(i);
  }

  if (kVerbose) {
    kStdOut << "i64asm: found a base 10 number here: " << jump_label.substr(pos)
            << "\n";
  }

  return true;
}

bool CompilerKit::PlatformAssemblerAMD64::WriteNumber16(
    const std::size_t &pos, std::string &jump_label) {
  if (!isdigit(jump_label[pos])) return false;

  switch (jump_label[pos + 1]) {
    case 'x': {
      if (auto res = strtoq(jump_label.substr(pos + 2).c_str(), nullptr, 16);
          !res) {
        if (errno != 0) {
          detail::print_error("invalid hex number: " + jump_label, "i64asm");
          throw std::runtime_error("invalid_hex");
        }
      }

      CompilerKit::NumberCast16 num = CompilerKit::NumberCast16(
          strtoq(jump_label.substr(pos + 2).c_str(), nullptr, 16));

      for (char &i : num.number) {
        if (i == 0) i = 0xFF;

        kBytes.push_back(i);
      }

      if (kVerbose) {
        kStdOut << "i64asm: found a base 16 number here: "
                << jump_label.substr(pos) << "\n";
      }

      return true;
    }
    case 'b': {
      if (auto res = strtoq(jump_label.substr(pos + 2).c_str(), nullptr, 2);
          !res) {
        if (errno != 0) {
          detail::print_error("invalid binary number: " + jump_label, "i64asm");
          throw std::runtime_error("invalid_bin");
        }
      }

      CompilerKit::NumberCast16 num = CompilerKit::NumberCast16(
          strtoq(jump_label.substr(pos + 2).c_str(), nullptr, 2));

      if (kVerbose) {
        kStdOut << "i64asm: found a base 2 number here: "
                << jump_label.substr(pos) << "\n";
      }

      for (char &i : num.number) {
        if (i == 0) i = 0xFF;

        kBytes.push_back(i);
      }

      return true;
    }
    case 'o': {
      if (auto res = strtoq(jump_label.substr(pos + 2).c_str(), nullptr, 7);
          !res) {
        if (errno != 0) {
          detail::print_error("invalid octal number: " + jump_label, "i64asm");
          throw std::runtime_error("invalid_octal");
        }
      }

      CompilerKit::NumberCast16 num = CompilerKit::NumberCast16(
          strtoq(jump_label.substr(pos + 2).c_str(), nullptr, 7));

      if (kVerbose) {
        kStdOut << "i64asm: found a base 8 number here: "
                << jump_label.substr(pos) << "\n";
      }

      for (char &i : num.number) {
        if (i == 0) i = 0xFF;

        kBytes.push_back(i);
      }

      return true;
    }
    default: {
      break;
    }
  }

  /* check for errno and stuff like that */
  if (auto res = strtoq(jump_label.substr(pos).c_str(), nullptr, 10); !res) {
    if (errno != 0) {
      return false;
    }
  }

  CompilerKit::NumberCast16 num = CompilerKit::NumberCast16(
      strtoq(jump_label.substr(pos).c_str(), nullptr, 10));

  for (char &i : num.number) {
    if (i == 0) i = 0xFF;

    kBytes.push_back(i);
  }

  if (kVerbose) {
    kStdOut << "i64asm: found a base 10 number here: " << jump_label.substr(pos)
            << "\n";
  }

  return true;
}

bool CompilerKit::PlatformAssemblerAMD64::WriteNumber8(
    const std::size_t &pos, std::string &jump_label) {
  if (!isdigit(jump_label[pos])) return false;

  switch (jump_label[pos + 1]) {
    case 'x': {
      if (auto res = strtoq(jump_label.substr(pos + 2).c_str(), nullptr, 16);
          !res) {
        if (errno != 0) {
          detail::print_error("invalid hex number: " + jump_label, "i64asm");
          throw std::runtime_error("invalid_hex");
        }
      }

      CompilerKit::NumberCast8 num = CompilerKit::NumberCast8(
          strtoq(jump_label.substr(pos + 2).c_str(), nullptr, 16));

      kBytes.push_back(num.number);

      if (kVerbose) {
        kStdOut << "i64asm: found a base 16 number here: "
                << jump_label.substr(pos) << "\n";
      }

      return true;
    }
    case 'b': {
      if (auto res = strtoq(jump_label.substr(pos + 2).c_str(), nullptr, 2);
          !res) {
        if (errno != 0) {
          detail::print_error("invalid binary number: " + jump_label, "i64asm");
          throw std::runtime_error("invalid_bin");
        }
      }

      CompilerKit::NumberCast8 num = CompilerKit::NumberCast8(
          strtoq(jump_label.substr(pos + 2).c_str(), nullptr, 2));

      if (kVerbose) {
        kStdOut << "i64asm: found a base 2 number here: "
                << jump_label.substr(pos) << "\n";
      }

      kBytes.push_back(num.number);

      return true;
    }
    case 'o': {
      if (auto res = strtoq(jump_label.substr(pos + 2).c_str(), nullptr, 7);
          !res) {
        if (errno != 0) {
          detail::print_error("invalid octal number: " + jump_label, "i64asm");
          throw std::runtime_error("invalid_octal");
        }
      }

      CompilerKit::NumberCast8 num = CompilerKit::NumberCast8(
          strtoq(jump_label.substr(pos + 2).c_str(), nullptr, 7));

      if (kVerbose) {
        kStdOut << "i64asm: found a base 8 number here: "
                << jump_label.substr(pos) << "\n";
      }

      kBytes.push_back(num.number);

      return true;
    }
    default: {
      break;
    }
  }

  /* check for errno and stuff like that */
  if (auto res = strtoq(jump_label.substr(pos).c_str(), nullptr, 10); !res) {
    if (errno != 0) {
      return false;
    }
  }

  CompilerKit::NumberCast8 num = CompilerKit::NumberCast8(
      strtoq(jump_label.substr(pos).c_str(), nullptr, 10));

  kBytes.push_back(num.number);

  if (kVerbose) {
    kStdOut << "i64asm: found a base 10 number here: " << jump_label.substr(pos)
            << "\n";
  }

  return true;
}

/////////////////////////////////////////////////////////////////////////////////////////

// @brief Read and write an instruction to the output array.

/////////////////////////////////////////////////////////////////////////////////////////

bool CompilerKit::PlatformAssemblerAMD64::WriteLine(std::string &line,
                                                    const std::string &file) {
  if (ParserKit::find_word(line, "export ")) return true;

  for (auto &opcodeAMD64 : kOpcodesAMD64) {
    // strict check here
    if (ParserKit::find_word(line, opcodeAMD64.fName) &&
        detail::algorithm::is_valid(line)) {
      std::string name(opcodeAMD64.fName);

      if (name.find("mov") != std::string::npos) {
        struct RegMapAMD64 {
          std::string fName;
          e64_byte_t fModRM;
        };

        std::vector<RegMapAMD64> regs{
            {.fName = "ax", .fModRM = 0}, {.fName = "cx", .fModRM = 1},
            {.fName = "dx", .fModRM = 2}, {.fName = "bx", .fModRM = 3},
            {.fName = "sp", .fModRM = 4}, {.fName = "bp", .fModRM = 5},
            {.fName = "si", .fModRM = 6}, {.fName = "di", .fModRM = 7},
        };

        std::string substr = line.substr(line.find(name) + name.size());

        uint64_t bits = 16;

        if (substr.find(",") == std::string::npos) {
          detail::print_error("Invalid combination of operands and registers.",
                              "i64asm");
          throw std::runtime_error("comb_op_reg");
        }

        bool found = false;

        for (auto &reg : regs) {
          if (line.find(reg.fName) != std::string::npos) {
            if (!found) {
              if (line.substr(line.find(reg.fName) - 1)[0] == 'r') {
                bits = 64;

                kBytes.emplace_back(0x48);
                kBytes.emplace_back(0x89);
                kBytes.emplace_back(0x00);
              } else {
                detail::print_error(
                    "Invalid combination of registers, each 64-bit register "
                    "must start with 'r'.",
                    "i64asm");
                throw std::runtime_error("comb_op_reg");
              }

              found = true;

              kBytes.push_back(0xc0 + reg.fModRM);

              continue;
            }
          }
        }

        if (bits == 64)
          this->WriteNumber32(line.find(name) + name.size() + 2, line);
        else if (bits == 16)
          this->WriteNumber16(line.find(name) + name.size() + 2, line);
        else {
          detail::print_error("Invalid combination of operands and registers.",
                              "i64asm");
          throw std::runtime_error("comb_op_reg");
        }

        break;
      } else if (name == "int" || name == "into" || name == "intd") {
        kBytes.emplace_back(opcodeAMD64.fOpcode);
        this->WriteNumber8(line.find(name) + name.size() + 1, line);

        break;
      } else if (name == "jmp" || name == "call") {
        kBytes.emplace_back(opcodeAMD64.fOpcode);
        this->WriteNumber32(line.find(name) + name.size() + 1, line);

        break;
      } else {
        kBytes.emplace_back(0);
        kBytes.emplace_back(opcodeAMD64.fOpcode);

        break;
      }
    }
  }

  if (line.find("db") != std::string::npos) {
    this->WriteNumber(line.find("db") + strlen("db") + 1, line);
  }
  else if (line.find("org ") != std::string::npos) {
    size_t base[] = {10, 16, 2, 7};

    for (size_t i = 0; i < 4; i++) {
      if (kOrigin = strtol(
              (line.substr(line.find("org ") + strlen("org ") + 1)).c_str(),
              nullptr, base[i]);
          kOrigin) {
        if (errno != 0) {
          continue;
        } else {
          if (kVerbose) {
            kStdOut << "i64asm: set-origin: " << kOrigin << std::endl;
          }
        }
      }
    }
  }

  return true;
}

// Last rev 13-1-24
