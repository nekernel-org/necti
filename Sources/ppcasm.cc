/* -------------------------------------------

    Copyright Zeta Electronics Corporation

------------------------------------------- */

/////////////////////////////////////////////////////////////////////////////////////////

/// @file ppcasm.cxx
/// @author Amlal EL Mahrouss
/// @brief POWER Assembler.

/// REMINDER: when dealing with an undefined symbol use (string
/// size):LinkerFindSymbol:(string) so that li will look for it.

/////////////////////////////////////////////////////////////////////////////////////////

#define __ASM_NEED_PPC__ 1

#include <Comm/StdKit/ErrorID.hpp>
#include <Comm/AsmKit/CPU/ppc.hpp>
#include <Comm/StdKit/PEF.hpp>
#include <Comm/ParserKit.hpp>
#include <Comm/StdKit/AE.hpp>
#include <Comm/Version.hxx>
#include <filesystem>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <vector>

/////////////////////

// ANSI ESCAPE CODES

/////////////////////

#define kBlank "\e[0;30m"
#define kRed "\e[0;31m"
#define kWhite "\e[0;97m"
#define kYellow "\e[0;33m"

#define kStdOut (std::cout << kWhite)
#define kStdErr (std::cout << kRed)

static CharType kOutputArch = CompilerKit::kPefArchPowerPC;
static Boolean kOutputAsBinary = false;

constexpr auto cPowerIPAlignment = 0x4U;

static UInt32 kErrorLimit = 10;
static UInt32 kAcceptableErrors = 0;

static std::size_t kCounter = 1UL;

static std::uintptr_t kOrigin = kPefBaseOrigin;
static std::vector<std::pair<std::string, std::uintptr_t>> kOriginLabel;

static bool kVerbose = false;

static std::vector<uint8_t> kBytes;

static CompilerKit::AERecordHeader kCurrentRecord{
    .fName = "", .fKind = CompilerKit::kPefCode, .fSize = 0, .fOffset = 0};

static std::vector<CompilerKit::AERecordHeader> kRecords;
static std::vector<std::string> kUndefinedSymbols;

static const std::string kUndefinedSymbol = ":UndefinedSymbol:";
static const std::string kRelocSymbol = ":RuntimeSymbol:";

// \brief forward decl.
static bool asm_read_attributes(std::string &line);

namespace detail {
void print_error(std::string reason, const std::string &file) noexcept {
  if (reason[0] == '\n') reason.erase(0, 1);

  kStdErr << kRed << "[ ppcasm ] " << kWhite
          << ((file == "ppcasm") ? "internal assembler error "
                                 : ("in file, " + file))
          << kBlank << std::endl;
  kStdErr << kRed << "[ ppcasm ] " << kWhite << reason << kBlank << std::endl;

  if (kAcceptableErrors > kErrorLimit) std::exit(3);

  ++kAcceptableErrors;
}

void print_warning(std::string reason, const std::string &file) noexcept {
  if (reason[0] == '\n') reason.erase(0, 1);

  if (!file.empty()) {
    kStdOut << kYellow << "[ file ] " << kWhite << file << kBlank << std::endl;
  }

  kStdOut << kYellow << "[ ppcasm ] " << kWhite << reason << kBlank
          << std::endl;
}
}  // namespace detail

/// Do not move it on top! it uses the assembler detail namespace!
#include <asmutils.hxx>

/////////////////////////////////////////////////////////////////////////////////////////

/// @brief POWER assembler entrypoint, the program/module starts here.

/////////////////////////////////////////////////////////////////////////////////////////

MPCC_MODULE(NewOSAssemblerPowerPC) {
  for (size_t i = 1; i < argc; ++i) {
    if (argv[i][0] == '-') {
      if (strcmp(argv[i], "-version") == 0 || strcmp(argv[i], "-v") == 0) {
		kStdOut << "ppcasm: POWER64/XEN Assembler.\nppcasm: " << kDistVersion << "\nppcasm: "
                   "Copyright (c) "
                   "2024 Zeta Electronics Corporation.\n";
        return 0;
      } else if (strcmp(argv[i], "-h") == 0) {
		kStdOut << "ppcasm: POWER64/XEN Assembler.\nppcasm: Copyright (c) 2024 "
                   "Zeta Electronics Corporation.\n";
        kStdOut << "-version: Print program version.\n";
        kStdOut << "-verbose: Print verbose output.\n";
        kStdOut << "-binary: Output as flat binary.\n";

        return 0;
      } else if (strcmp(argv[i], "-binary") == 0) {
        kOutputAsBinary = true;
        continue;
      } else if (strcmp(argv[i], "-verbose") == 0) {
        kVerbose = true;
        continue;
      }

      kStdOut << "ppcasm: ignore " << argv[i] << "\n";
      continue;
    }

    if (!std::filesystem::exists(argv[i])) {
      kStdOut << "ppcasm: can't open: " << argv[i] << std::endl;
      goto asm_fail_exit;
    }

    std::string object_output(argv[i]);

    for (auto &ext : kAsmFileExts) {
      if (object_output.find(ext) != std::string::npos) {
        object_output.erase(object_output.find(ext), std::strlen(ext));
      }
    }

    object_output += kOutputAsBinary ? kBinaryFileExt : kObjectFileExt;

    std::ifstream file_ptr(argv[i]);
    std::ofstream file_ptr_out(object_output, std::ofstream::binary);

    if (file_ptr_out.bad()) {
      if (kVerbose) {
        kStdOut << "ppcasm: error: " << strerror(errno) << "\n";
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

    CompilerKit::EncoderPowerPC asm64;

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
          detail::print_warning("exit because of: " + what, "ppcasm");
        }

        std::filesystem::remove(object_output);
        goto asm_fail_exit;
      }
    }

    if (!kOutputAsBinary) {
      if (kVerbose) {
        kStdOut << "ppcasm: Writing object file...\n";
      }

      // this is the final step, write everything to the file.

      auto pos = file_ptr_out.tellp();

      hdr.fCount = kRecords.size() + kUndefinedSymbols.size();

      file_ptr_out << hdr;

      if (kRecords.empty()) {
        kStdErr << "ppcasm: At least one record is needed to write an object "
                   "file.\nppcasm: Make one using `export .code64 foo_bar`.\n";

        std::filesystem::remove(object_output);
        return -1;
      }

      kRecords[kRecords.size() - 1].fSize = kBytes.size();

      std::size_t record_count = 0UL;

      for (auto &record_hdr : kRecords) {
        record_hdr.fFlags |= CompilerKit::kKindRelocationAtRuntime;
        record_hdr.fOffset = record_count;
        ++record_count;

        file_ptr_out << record_hdr;

        if (kVerbose)
          kStdOut << "ppcasm: Wrote record " << record_hdr.fName << "...\n";
      }

      // increment once again, so that we won't lie about the kUndefinedSymbols.
      ++record_count;

      for (auto &sym : kUndefinedSymbols) {
        CompilerKit::AERecordHeader undefined_sym{0};

        if (kVerbose)
          kStdOut << "ppcasm: Wrote symbol " << sym << " to file...\n";

        undefined_sym.fKind = kAEInvalidOpcode;
        undefined_sym.fSize = sym.size();
        undefined_sym.fOffset = record_count;

        ++record_count;

        memset(undefined_sym.fPad, kAEInvalidOpcode, kAEPad);
        memcpy(undefined_sym.fName, sym.c_str(), sym.size());

        file_ptr_out << undefined_sym;

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
        kStdOut << "ppcasm: Write raw binary...\n";
      }
    }

    // byte from byte, we write this.
    for (auto &byte : kBytes) {
      file_ptr_out.write(reinterpret_cast<const char *>(&byte), sizeof(byte));
    }

    if (kVerbose) kStdOut << "ppcasm: Wrote file with program in it.\n";

    file_ptr_out.flush();
    file_ptr_out.close();

    if (kVerbose) kStdOut << "ppcasm: Exit succeeded.\n";

    return 0;
  }

asm_fail_exit:

  if (kVerbose) kStdOut << "ppcasm: Exit failed.\n";

  return MPCC_EXEC_ERROR;
}

/////////////////////////////////////////////////////////////////////////////////////////

// @brief Check for attributes
// returns true if any was found.

/////////////////////////////////////////////////////////////////////////////////////////

static bool asm_read_attributes(std::string &line) {
  // import is the opposite of export, it signals to the li
  // that we need this symbol.
  if (ParserKit::find_word(line, "import")) {
    if (kOutputAsBinary) {
      detail::print_error("Invalid import directive in flat binary mode.",
                          "ppcasm");
      throw std::runtime_error("invalid_import_bin");
    }

    auto name = line.substr(line.find("import") + strlen("import") + 1);

    if (name.size() == 0) {
      detail::print_error("Invalid import", "ppcasm");
      throw std::runtime_error("invalid_import");
    }

    std::string result = std::to_string(name.size());
    result += kUndefinedSymbol;

    // mangle this
    for (char &j : name) {
      if (j == ' ' || j == ',') j = '$';
    }

    result += name;

    if (name.find(".code64") != std::string::npos) {
      // data is treated as code.
      kCurrentRecord.fKind = CompilerKit::kPefCode;
    } else if (name.find(".data64") != std::string::npos) {
      // no code will be executed from here.
      kCurrentRecord.fKind = CompilerKit::kPefData;
    } else if (name.find(".zero64") != std::string::npos) {
      // this is a bss section.
      kCurrentRecord.fKind = CompilerKit::kPefZero;
    }

    // this is a special case for the start stub.
    // we want this so that li can find it.

    if (name == kPefStart) {
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
  // export is a special keyword used by ppcasm to tell the AE output stage to
  // mark this section as a header. it currently supports .code64, .data64.,
  // .zero64
  else if (ParserKit::find_word(line, "export")) {
    if (kOutputAsBinary) {
      detail::print_error("Invalid export directive in flat binary mode.",
                          "ppcasm");
      throw std::runtime_error("invalid_export_bin");
    }

    auto name = line.substr(line.find("export") + strlen("export"));

    std::string name_copy = name;

    for (char &j : name) {
      if (j == ' ') j = '$';
    }

    if (name.find(".code64") != std::string::npos) {
      // data is treated as code.

      name_copy.erase(name_copy.find(".code64"), strlen(".code64"));
      kCurrentRecord.fKind = CompilerKit::kPefCode;
    } else if (name.find(".data64") != std::string::npos) {
      // no code will be executed from here.

      name_copy.erase(name_copy.find(".data64"), strlen(".data64"));
      kCurrentRecord.fKind = CompilerKit::kPefData;
    } else if (name.find(".zero64") != std::string::npos) {
      // this is a bss section.

      name_copy.erase(name_copy.find(".zero64"), strlen(".zero64"));
      kCurrentRecord.fKind = CompilerKit::kPefZero;
    }

    // this is a special case for the start stub.
    // we want this so that li can find it.

    if (name == kPefStart) {
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
  return std::find_if(str.begin(), str.end(), is_not_alnum_space) == str.end();
}
}  // namespace detail::algorithm

/////////////////////////////////////////////////////////////////////////////////////////

// @brief Check for line (syntax check)

/////////////////////////////////////////////////////////////////////////////////////////

std::string CompilerKit::EncoderPowerPC::CheckLine(std::string &line,
                                                   const std::string &file) {
  std::string err_str;

  if (line.empty() || ParserKit::find_word(line, "import") ||
      ParserKit::find_word(line, "export") ||
      line.find('#') != std::string::npos || ParserKit::find_word(line, ";")) {
    if (line.find('#') != std::string::npos) {
      line.erase(line.find('#'));
    } else if (line.find(';') != std::string::npos) {
      line.erase(line.find(';'));
    } else {
      /// does the line contains valid input?
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

  // these do take an argument.
  std::vector<std::string> operands_inst = {"stw", "li"};

  // these don't.
  std::vector<std::string> filter_inst = {"blr", "bl", "sc"};

  for (auto &opcodePPC : kOpcodesPowerPC) {
    if (ParserKit::find_word(line, opcodePPC.name)) {
      for (auto &op : operands_inst) {
        // if only the instruction was found.
        if (line == op) {
          err_str += "\nMalformed ";
          err_str += op;
          err_str += " instruction, here -> ";
          err_str += line;
        }
      }

      // if it is like that -> addr1, 0x0
      if (auto it =
              std::find(filter_inst.begin(), filter_inst.end(), opcodePPC.name);
          it == filter_inst.cend()) {
        if (ParserKit::find_word(line, opcodePPC.name)) {
          if (!isspace(
                  line[line.find(opcodePPC.name) + strlen(opcodePPC.name)])) {
            err_str += "\nMissing space between ";
            err_str += opcodePPC.name;
            err_str += " and operands.\nhere -> ";
            err_str += line;
          }
        }
      }

      return err_str;
    }
  }

  err_str += "Unrecognized instruction: " + line;

  return err_str;
}

bool CompilerKit::EncoderPowerPC::WriteNumber(const std::size_t &pos,
                                              std::string &jump_label) {
  if (!isdigit(jump_label[pos])) return false;

  switch (jump_label[pos + 1]) {
    case 'x': {
      if (auto res = strtol(jump_label.substr(pos + 2).c_str(), nullptr, 16);
          !res) {
        if (errno != 0) {
          detail::print_error("invalid hex number: " + jump_label, "ppcasm");
          throw std::runtime_error("invalid_hex");
        }
      }

      CompilerKit::NumberCast64 num(
          strtol(jump_label.substr(pos + 2).c_str(), nullptr, 16));

      for (char &i : num.number) {
        kBytes.push_back(i);
      }

      if (kVerbose) {
        kStdOut << "ppcasm: found a base 16 number here: "
                << jump_label.substr(pos) << "\n";
      }

      return true;
    }
    case 'b': {
      if (auto res = strtol(jump_label.substr(pos + 2).c_str(), nullptr, 2);
          !res) {
        if (errno != 0) {
          detail::print_error("invalid binary number: " + jump_label, "ppcasm");
          throw std::runtime_error("invalid_bin");
        }
      }

      CompilerKit::NumberCast64 num(
          strtol(jump_label.substr(pos + 2).c_str(), nullptr, 2));

      if (kVerbose) {
        kStdOut << "ppcasm: found a base 2 number here: "
                << jump_label.substr(pos) << "\n";
      }

      for (char &i : num.number) {
        kBytes.push_back(i);
      }

      return true;
    }
    case 'o': {
      if (auto res = strtol(jump_label.substr(pos + 2).c_str(), nullptr, 7);
          !res) {
        if (errno != 0) {
          detail::print_error("invalid octal number: " + jump_label, "ppcasm");
          throw std::runtime_error("invalid_octal");
        }
      }

      CompilerKit::NumberCast64 num(
          strtol(jump_label.substr(pos + 2).c_str(), nullptr, 7));

      if (kVerbose) {
        kStdOut << "ppcasm: found a base 8 number here: "
                << jump_label.substr(pos) << "\n";
      }

      for (char &i : num.number) {
        kBytes.push_back(i);
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

  CompilerKit::NumberCast64 num(
      strtol(jump_label.substr(pos).c_str(), nullptr, 10));

  for (char &i : num.number) {
    kBytes.push_back(i);
  }

  if (kVerbose) {
    kStdOut << "ppcasm: found a base 10 number here: " << jump_label.substr(pos)
            << "\n";
  }

  return true;
}

/////////////////////////////////////////////////////////////////////////////////////////

/// @brief Read and write an instruction to the output array.

/////////////////////////////////////////////////////////////////////////////////////////

bool CompilerKit::EncoderPowerPC::WriteLine(std::string &line,
                                            const std::string &file) {
  if (ParserKit::find_word(line, "export")) return true;
  if (!detail::algorithm::is_valid(line)) return true;

  for (auto &opcodePPC : kOpcodesPowerPC) {
    // strict check here
    if (ParserKit::find_word(line, opcodePPC.name)) {
      std::string name(opcodePPC.name);
      std::string jump_label, cpy_jump_label;
      std::vector<size_t> found_registers_index;

      // check funct7 type.
      switch (opcodePPC.ops->type) {
        default: {
          NumberCast32 num(opcodePPC.opcode);

          for (auto ch : num.number) {
            kBytes.emplace_back(ch);
          }
          break;
        }
        case BADDR:
        case PCREL: {
          auto num = GetNumber32(line, name);

          kBytes.emplace_back(num.number[0]);
          kBytes.emplace_back(num.number[1]);
          kBytes.emplace_back(num.number[2]);
          kBytes.emplace_back(0x48);

          break;
        }
        /// General purpose, float, vector operations. Everything that involve
        /// registers.
        case G0REG:
        case FREG:
        case VREG:
        case GREG: {
          // \brief how many registers we found.
          std::size_t found_some_count = 0UL;
          std::size_t register_count = 0UL;
          std::string opcodeName = opcodePPC.name;
          std::size_t register_sum = 0;

          NumberCast64 num(opcodePPC.opcode);

          for (size_t line_index = 0UL; line_index < line.size();
               line_index++) {
            if (line[line_index] == kAsmRegisterPrefix[0] &&
                isdigit(line[line_index + 1])) {
              std::string register_syntax = kAsmRegisterPrefix;
              register_syntax += line[line_index + 1];

              if (isdigit(line[line_index + 2]))
                register_syntax += line[line_index + 2];

              std::string reg_str;
              reg_str += line[line_index + 1];

              if (isdigit(line[line_index + 2]))
                reg_str += line[line_index + 2];

              // it ranges from r0 to r19
              // something like r190 doesn't exist in the instruction set.
              if (isdigit(line[line_index + 3]) &&
                  isdigit(line[line_index + 2])) {
                reg_str += line[line_index + 3];
                detail::print_error(
                    "invalid register index, r" + reg_str +
                        "\nnote: The POWER accepts registers from r0 to r32.",
                    file);
                throw std::runtime_error("invalid_register_index");
              }

              // finally cast to a size_t
              std::size_t reg_index = strtol(reg_str.c_str(), nullptr, 10);

              if (reg_index > kAsmRegisterLimit) {
                detail::print_error("invalid register index, r" + reg_str,
                                    file);
                throw std::runtime_error("invalid_register_index");
              }

              if (opcodeName == "li") {
                char numIndex = 0;

                for (size_t i = 0; i != reg_index; i++) {
                  numIndex += 0x20;
                }

                auto num = GetNumber32(line, reg_str);

                kBytes.push_back(num.number[0]);
                kBytes.push_back(num.number[1]);
                kBytes.push_back(numIndex);
                kBytes.push_back(0x38);

                // check if bigger than two.
                for (size_t i = 2; i < 4; i++) {
                  if (num.number[i] > 0) {
                    detail::print_warning("number overflow on li operation.",
                                          file);
                    break;
                  }
                }

                break;
              }

              if ((opcodeName[0] == 's' && opcodeName[1] == 't')) {
                if (register_sum == 0) {
                  for (size_t indexReg = 0UL; indexReg < reg_index;
                       ++indexReg) {
                    register_sum += 0x20;
                  }
                } else {
                  register_sum += reg_index;
                }
              }

              if (opcodeName == "mr") {
                switch (register_count) {
                  case 0: {
                    kBytes.push_back(0x78);

                    char numIndex = 0x3;

                    for (size_t i = 0; i != reg_index; i++) {
                      numIndex += 0x8;
                    }

                    kBytes.push_back(numIndex);

                    break;
                  }
                  case 1: {
                    char numIndex = 0x1;

                    for (size_t i = 0; i != reg_index; i++) {
                      numIndex += 0x20;
                    }

                    for (size_t i = 0; i != reg_index; i++) {
                      kBytes[kBytes.size() - 1] += 0x8;
                    }

                    kBytes[kBytes.size() - 1] -= 0x8;

                    kBytes.push_back(numIndex);

                    if (reg_index >= 10 && reg_index < 20)
                      kBytes.push_back(0x7d);
                    else if (reg_index >= 20 && reg_index < 30)
                      kBytes.push_back(0x7e);
                    else if (reg_index >= 30)
                      kBytes.push_back(0x7f);
                    else
                      kBytes.push_back(0x7c);

                    break;
                  }
                  default:
                    break;
                }

                ++register_count;
                ++found_some_count;
              }

              if (opcodeName == "addi") {
                if (found_some_count == 2 || found_some_count == 0)
                  kBytes.emplace_back(reg_index);
                else if (found_some_count == 1)
                  kBytes.emplace_back(0x00);

                ++found_some_count;

                if (found_some_count > 3) {
                  detail::print_error("Too much registers. -> " + line, file);
                  throw std::runtime_error("too_much_regs");
                }
              }

              if (opcodeName.find("cmp") != std::string::npos) {
                ++found_some_count;

                if (found_some_count > 3) {
                  detail::print_error("Too much registers. -> " + line, file);
                  throw std::runtime_error("too_much_regs");
                }
              }

              if (opcodeName.find("mf") != std::string::npos ||
                  opcodeName.find("mt") != std::string::npos) {
                char numIndex = 0;

                for (size_t i = 0; i != reg_index; i++) {
                  numIndex += 0x20;
                }

                num.number[2] += numIndex;

                ++found_some_count;

                if (found_some_count > 1) {
                  detail::print_error("Too much registers. -> " + line, file);
                  throw std::runtime_error("too_much_regs");
                }

                if (kVerbose) {
                  kStdOut << "ppcasm: Found register: " << register_syntax
                          << "\n";
                  kStdOut << "ppcasm: Amount of registers in instruction: "
                          << found_some_count << "\n";
                }

                if (reg_index >= 10 && reg_index < 20)
                  num.number[3] = 0x7d;
                else if (reg_index >= 20 && reg_index < 30)
                  num.number[3] = 0x7e;
                else if (reg_index >= 30)
                  num.number[3] = 0x7f;
                else
                  num.number[3] = 0x7c;

                for (auto ch : num.number) {
                  kBytes.emplace_back(ch);
                }
              }

              found_registers_index.push_back(reg_index);
            }
          }

          if (opcodeName == "addi") {
            kBytes.emplace_back(0x38);
          }

          if (opcodeName.find("cmp") != std::string::npos) {
              char rightReg = 0x0;

              for (size_t i = 0; i != found_registers_index[1]; i++) {
                rightReg += 0x08;
              }

              kBytes.emplace_back(0x00);
              kBytes.emplace_back(rightReg);
              kBytes.emplace_back(found_registers_index[0]);
              kBytes.emplace_back(0x7c);
          }

          if ((opcodeName[0] == 's' && opcodeName[1] == 't')) {
            size_t offset = 0UL;

            if (line.find('+') != std::string::npos) {
              auto number = GetNumber32(line.substr(line.find("+")), "+");
              offset = number.raw;
            }

            kBytes.push_back(offset);
            kBytes.push_back(0x00);
            kBytes.push_back(register_sum);

            kBytes.emplace_back(0x90);
          }

          if (opcodeName == "mr") {
            if (register_count == 1) {
              detail::print_error("Too few registers. -> " + line, file);
              throw std::runtime_error("too_few_registers");
            }
          }

          // we're not in immediate addressing, reg to reg.
          if (opcodePPC.ops->type != GREG) {
            // remember! register to register!
            if (found_some_count == 1) {
              detail::print_error(
                  "Unrecognized register found.\ntip: each ppcasm register "
                  "starts with 'r'.\nline: " +
                      line,
                  file);

              throw std::runtime_error("not_a_register");
            }
          }

          if (found_some_count < 1 && name[0] != 'l' && name[0] != 's') {
            detail::print_error(
                "invalid combination of opcode and registers.\nline: " + line,
                file);
            throw std::runtime_error("invalid_comb_op_reg");
          }

          break;
        }
      }

      kOrigin += cPowerIPAlignment;
      break;
    }
  }

  return true;
}

// Last rev 13-1-24
