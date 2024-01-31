/*
 *	========================================================
 *
 *	ld
 * 	Copyright Mahrouss Logic, all rights reserved.
 *
 * 	========================================================
 */

/// bugs: 0

// @file ld.cxx
// @author Amlal El Mahrouss (amlel)
// @brief 64x0 linker.

// README: Do not look up for anything with .text/.data/.page_zero!
// It will be loaded when program will start up!

#include <CompilerKit/StdKit/ErrorID.hpp>

//! Assembler Kit
#include <CompilerKit/AsmKit/AsmKit.hpp>

//! Portable Executable Format
#include <CompilerKit/StdKit/PEF.hpp>

//! Advanced Executable Object Format
#include <uuid/uuid.h>

#include <CompilerKit/StdKit/AE.hpp>
#include <fstream>
#include <iostream>

//! @brief standard PEF entry.
#define kPefStart "__start"

#define kToolVersion "ld v2.2.1, (c) Mahrouss Logic 2024"

#define StringCompare(DST, SRC) strcmp(DST, SRC)

#define kPefNoCpu 0U
#define kPefNoSubCpu 0U

#define kWhite "\e[0;97m"
#define kStdOut (std::cout << kWhite)

#define kPefDeaultOrg (uint64_t)0x10000
#define kPefLinkerNumId 0x5046FF
#define kPefAbiId "Container:Abi:64x0"

enum { kAbiMpUx = 0x5046 /* PF */ };

static std::string kOutput = "a" kPefExt;
static Int32 kAbi = kAbiMpUx;
static Int32 kSubArch = kPefNoSubCpu;
static Int32 kArch = CompilerKit::kPefArch64000;
static Bool kFatBinaryEnable = false;
static Bool kStartFound = false;
static Bool kDuplicateSymbols = false;
static Bool kVerbose = false;

/* ld is to be found, mld is to be found at runtime. */
static const char *kLdDefineSymbol = ":ld:";
static const char *kLdDynamicSym = ":mld:";

/* object code and list. */
static std::vector<std::string> kObjectList;
static std::vector<char> kObjectBytes;

MPCC_MODULE(MPUXLinker) {
  bool is_executable = true;

  /**
   * @brief parse flags and such.
   *
   */
  for (size_t i = 1; i < argc; ++i) {
    if (StringCompare(argv[i], "-h") == 0) {
      kStdOut << kToolVersion << "\n";
      kStdOut << "-version: Show program version.\n";
      kStdOut << "-verbose: Enable program trace.\n";
      kStdOut << "-shared: Output as a shared PEF.\n";
      kStdOut << "-fat-bin: Output as FAT PEF.\n";
      kStdOut << "-32x0: Output as 32x0 PEF.\n";
      kStdOut << "-64x0: Output as 64x0 PEF.\n";
      kStdOut << "-output-file: Select output file name.\n";

      return 0;
    } else if (StringCompare(argv[i], "-version") == 0) {
      kStdOut << kToolVersion << std::endl;

      return 0;
    } else if (StringCompare(argv[i], "-fat-bin") == 0) {
      kFatBinaryEnable = true;

      continue;
    } else if (StringCompare(argv[i], "-64x0") == 0) {
      kArch = CompilerKit::kPefArch64000;

      continue;
    } else if (StringCompare(argv[i], "-32x0") == 0) {
      kArch = CompilerKit::kPefArch32000;

      continue;
    } else if (StringCompare(argv[i], "-verbose") == 0) {
      kVerbose = true;

      continue;
    } else if (StringCompare(argv[i], "-shared") == 0) {
      if (kOutput.find(kPefExt) != std::string::npos)
        kOutput.erase(kOutput.find(kPefExt), strlen(kPefExt));

      kOutput += kPefDylibExt;

      is_executable = false;

      continue;
    } else if (StringCompare(argv[i], "-output-file") == 0) {
      kOutput = argv[i + 1];
      ++i;

      continue;
    } else {
      if (argv[i][0] == '-') {
        kStdOut << "ld: unknown flag: " << argv[i] << "\n";
        return -CXXKIT_EXEC_ERROR;
      }

      kObjectList.emplace_back(argv[i]);

      continue;
    }
  }

  // sanity check.
  if (kObjectList.empty()) {
    kStdOut << "ld: no input files." << std::endl;
    return CXXKIT_EXEC_ERROR;
  } else {
    // check for existing files, if they don't throw an error.
    for (auto &obj : kObjectList) {
      if (!std::filesystem::exists(obj)) {
        // if filesystem doesn't find file
        //          -> throw error.
        kStdOut << "ld: no such file: " << obj << std::endl;
        return CXXKIT_EXEC_ERROR;
      }
    }
  }

  // PEF expects a valid architecture when outputing a binary.
  if (kArch == 0) {
    kStdOut << "ld: no target architecture set, can't continue." << std::endl;
    return CXXKIT_EXEC_ERROR;
  }

  CompilerKit::PEFContainer pef_container{};

  int32_t archs = kArch;

  pef_container.Count = 0UL;
  pef_container.Kind = CompilerKit::kPefKindExec;
  pef_container.SubCpu = kSubArch;
  pef_container.Linker = kPefLinkerNumId;  // Mahrouss Logic Linker
  pef_container.Abi = kAbi;                // Multi-Processor UX ABI
  pef_container.Magic[0] = kPefMagic[kFatBinaryEnable ? 2 : 0];
  pef_container.Magic[1] = kPefMagic[1];
  pef_container.Magic[2] = kPefMagic[kFatBinaryEnable ? 0 : 2];
  pef_container.Version = kPefVersion;

  // specify the start address, can be 0x10000
  pef_container.Start = kPefDeaultOrg;
  pef_container.HdrSz = sizeof(CompilerKit::PEFContainer);

  std::ofstream output_fc(kOutput, std::ofstream::binary);

  if (output_fc.bad()) {
    if (kVerbose) {
      kStdOut << "ld: error: " << strerror(errno) << "\n";
    }

    return -CXXKIT_FILE_NOT_FOUND;
  }

  //! Read AE to convert as PEF.

  std::vector<CompilerKit::PEFCommandHeader> pef_command_hdrs;
  CompilerKit::Utils::AEReadableProtocol readProto{};

  for (const auto &i : kObjectList) {
    if (!std::filesystem::exists(i)) continue;

    CompilerKit::AEHeader hdr{};

    readProto.FP = std::ifstream(i, std::ifstream::binary);
    readProto.FP >> hdr;

    auto ae_header = hdr;

    if (ae_header.fMagic[0] == kAEMag0 && ae_header.fMagic[1] == kAEMag1 &&
        ae_header.fSize == sizeof(CompilerKit::AEHeader)) {
      if (ae_header.fArch != kArch) {
        if (kVerbose) kStdOut << "ld: info: is a fat binary? : ";

        if (!kFatBinaryEnable) {
          if (kVerbose) kStdOut << "no.\n";

          kStdOut << "ld: error: object " << i
                  << " is a different kind of architecture and output isn't "
                     "treated as FAT binary."
                  << std::endl;

          std::remove(kOutput.c_str());
          return -CXXKIT_FAT_ERROR;
        } else {
          if (kVerbose) {
            kStdOut << "yes.\n";
          }
        }
      }

      // append arch type to archs varaible.
      archs |= ae_header.fArch;
      std::size_t cnt = ae_header.fCount;

      if (kVerbose)
        kStdOut << "ld: object header found, record count: " << cnt << "\n";

      pef_container.Count = cnt;

      char_type *raw_ae_records =
          new char[cnt * sizeof(CompilerKit::AERecordHeader)];
      memset(raw_ae_records, 0, cnt * sizeof(CompilerKit::AERecordHeader));

      auto *ae_records = readProto.Read(raw_ae_records, cnt);

      for (size_t ae_record_index = 0; ae_record_index < cnt;
           ++ae_record_index) {
        CompilerKit::PEFCommandHeader command_header{0};

        memcpy(command_header.Name, ae_records[ae_record_index].fName,
               kPefNameLen);

        // check this header if it's any valid.
        if (std::string(command_header.Name).find(".text") ==
                std::string::npos &&
            std::string(command_header.Name).find(".data") ==
                std::string::npos &&
            std::string(command_header.Name).find(".page_zero") ==
                std::string::npos) {
          if (std::string(command_header.Name).find(kPefStart) ==
                  std::string::npos &&
              *command_header.Name == 0) {
            if (std::string(command_header.Name).find(kLdDefineSymbol) !=
                std::string::npos) {
              goto ld_mark_header;
            } else {
              continue;
            }
          }
        }

        if (std::string(command_header.Name).find(kPefStart) !=
                std::string::npos &&
            std::string(command_header.Name).find(".text") !=
                std::string::npos) {
          kStartFound = true;
          pef_container.Start = ae_records[ae_record_index].fOffset;
        }

      ld_mark_header:
        command_header.Offset = ae_records[ae_record_index].fOffset;
        command_header.Kind = ae_records[ae_record_index].fKind;
        command_header.Size = ae_records[ae_record_index].fSize;

        if (kVerbose)
          kStdOut << "ld: object record: " << ae_records[ae_record_index].fName
                  << " was marked.\n";

        pef_command_hdrs.emplace_back(command_header);
      }

      delete[] raw_ae_records;

      std::vector<char> bytes;
      bytes.resize(ae_header.fCodeSize);

      readProto.FP.seekg(std::streamsize(ae_header.fStartCode));
      readProto.FP.read(bytes.data(), std::streamsize(ae_header.fCodeSize));

      for (auto &byte : bytes) {
        kObjectBytes.push_back(byte);
      }

      readProto.FP.close();

      continue;
    }

    kStdOut << "ld: not an object: " << i << std::endl;
    std::remove(kOutput.c_str());

    // don't continue, it is a fatal error.
    return -CXXKIT_EXEC_ERROR;
  }

  pef_container.Cpu = archs;

  output_fc << pef_container;

  if (kVerbose) {
    kStdOut << "ld: pef: wrote container header.\n";
  }

  output_fc.seekp(std::streamsize(pef_container.HdrSz));

  std::vector<std::string> not_found;
  std::vector<std::string> symbols;

  // step 2: check for errors (multiple symbols, undefined ones)

  for (auto &pef_command_hdr : pef_command_hdrs) {
    // check if this symbol needs to be resolved.
    if (std::string(pef_command_hdr.Name).find(kLdDefineSymbol) !=
            std::string::npos &&
        std::string(pef_command_hdr.Name).find(kLdDynamicSym) ==
            std::string::npos) {
      if (kVerbose)
        kStdOut << "ld: found undefined symbol: " << pef_command_hdr.Name
                << "\n";

      if (auto it = std::find(not_found.begin(), not_found.end(),
                              std::string(pef_command_hdr.Name));
          it == not_found.end()) {
        not_found.emplace_back(pef_command_hdr.Name);
      }
    }

    symbols.emplace_back(pef_command_hdr.Name);
  }

  // Now try to solve these symbols.

  for (size_t not_found_idx = 0; not_found_idx < pef_command_hdrs.size();
       ++not_found_idx) {
    if (auto it = std::find(not_found.begin(), not_found.end(),
                            std::string(pef_command_hdrs[not_found_idx].Name));
        it != not_found.end()) {
      std::string symbol_imp = *it;

      if (symbol_imp.find(kLdDefineSymbol) == std::string::npos) continue;

      // erase the lookup prefix.
      symbol_imp.erase(
          0, symbol_imp.find(kLdDefineSymbol) + strlen(kLdDefineSymbol));

      // demangle everything.
      while (symbol_imp.find('$') != std::string::npos)
        symbol_imp.erase(symbol_imp.find('$'), 1);

      // the reason we do is because, this may not match the symbol, and we need
      // to look for other matching symbols.
      for (auto &pef_command_hdr : pef_command_hdrs) {
        if (std::string(pef_command_hdr.Name).find(symbol_imp) !=
                std::string::npos &&
            std::string(pef_command_hdr.Name).find(kLdDefineSymbol) ==
                std::string::npos) {
          std::string undefined_symbol = pef_command_hdr.Name;
          auto result_of_sym =
              undefined_symbol.substr(undefined_symbol.find(symbol_imp));

          for (int i = 0; result_of_sym[i] != 0; ++i) {
            if (result_of_sym[i] != symbol_imp[i]) goto ld_continue_search;
          }

          not_found.erase(it);

          if (kVerbose)
            kStdOut << "ld: found symbol: " << pef_command_hdr.Name << "\n";

          break;
        }
      }

    ld_continue_search:
      continue;
    }
  }

  // step 3: check for errors (recheck if we have those symbols.)

  if (!kStartFound && is_executable) {
    if (kVerbose)
      kStdOut << "ld: undefined symbol: __start, you may have forget to link "
                 "against your runtime library.\n";

    kStdOut << "ld: undefined entrypoint " << kPefStart << " for executable "
            << kOutput << "\n";
  }

  // step 4: write some pef commands.

  CompilerKit::PEFCommandHeader date_header{};

  time_t timestamp = time(nullptr);

  std::string timestamp_str = "ContainerDate:";
  timestamp_str += std::to_string(timestamp);

  strcpy(date_header.Name, timestamp_str.c_str());

  date_header.Flags = 0;
  date_header.Kind = CompilerKit::kPefData;
  date_header.Offset = output_fc.tellp();
  date_header.Size = timestamp_str.size();

  output_fc << date_header;

  CompilerKit::PEFCommandHeader abi_header{};

  memcpy(abi_header.Name, kPefAbiId, strlen(kPefAbiId));

  abi_header.Size = strlen(kPefAbiId);
  abi_header.Offset = output_fc.tellp();
  abi_header.Flags = 0;
  abi_header.Kind = CompilerKit::kPefLinkerID;

  output_fc << abi_header;

  CompilerKit::PEFCommandHeader uuid_header{};

  uuid_t uuid{0};
  uuid_generate_random(uuid);

  memcpy(uuid_header.Name, "UUID_TYPE:4:", strlen("UUID_TYPE:4:"));
  memcpy(uuid_header.Name + strlen("UUID_TYPE:4:"), uuid, 16);

  uuid_header.Size = 16;
  uuid_header.Offset = output_fc.tellp();
  uuid_header.Flags = 0;
  uuid_header.Kind = 0;

  output_fc << uuid_header;

  // prepare a symbol vector.
  std::vector<std::string> undefined_symbols;
  std::vector<std::string> duplicate_symbols;
  std::vector<std::string> symbols_to_resolve;

  // Finally write down the command headers.
  // And check for any duplications
  for (size_t cmd_hdr = 0UL; cmd_hdr < pef_command_hdrs.size(); ++cmd_hdr) {
    if (std::string(pef_command_hdrs[cmd_hdr].Name).find(kLdDefineSymbol) !=
            std::string::npos &&
        std::string(pef_command_hdrs[cmd_hdr].Name).find(kLdDynamicSym) ==
            std::string::npos) {
      // ignore :ld: headers, they do not contain code.
      continue;
    }

    std::string sym_name = pef_command_hdrs[cmd_hdr].Name;

    if (!sym_name.empty()) {
      undefined_symbols.emplace_back(sym_name);
    }

    output_fc << pef_command_hdrs[cmd_hdr];

    for (size_t cmd_hdr_sub = 0UL; cmd_hdr_sub < pef_command_hdrs.size();
         ++cmd_hdr_sub) {
      if (cmd_hdr_sub == cmd_hdr) continue;

      if (std::string(pef_command_hdrs[cmd_hdr_sub].Name)
                  .find(kLdDefineSymbol) != std::string::npos &&
          std::string(pef_command_hdrs[cmd_hdr_sub].Name).find(kLdDynamicSym) ==
              std::string::npos) {
        if (kVerbose) {
          kStdOut << "ld: ignore :ld: command header...\n";
        }

        // ignore :ld: headers, they do not contain code.
        continue;
      }

      auto &pef_command_hdr = pef_command_hdrs[cmd_hdr_sub];

      if (pef_command_hdr.Name == std::string(pef_command_hdrs[cmd_hdr].Name)) {
        if (std::find(duplicate_symbols.cbegin(), duplicate_symbols.cend(),
                      pef_command_hdr.Name) == duplicate_symbols.cend()) {
          duplicate_symbols.emplace_back(pef_command_hdr.Name);
        }

        if (kVerbose)
          kStdOut << "ld: found duplicate symbol: " << pef_command_hdr.Name
                  << "\n";

        kDuplicateSymbols = true;
      }
    }
  }

  if (!duplicate_symbols.empty()) {
    for (auto &symbol : duplicate_symbols) {
      kStdOut << "ld: multiple symbols of " << symbol << ".\n";
    }

    std::remove(kOutput.c_str());
    return -CXXKIT_EXEC_ERROR;
  }

  // step 2.5: write program bytes.

  for (auto byte : kObjectBytes) {
    output_fc << byte;
  }

  if (kVerbose) kStdOut << "ld: wrote code for: " << kOutput << "\n";

  // step 3: check if we have those symbols

  std::vector<std::string> unreferenced_symbols;

  for (auto &pef_command_hdr : pef_command_hdrs) {
    if (auto it = std::find(not_found.begin(), not_found.end(),
                            std::string(pef_command_hdr.Name));
        it != not_found.end()) {
      unreferenced_symbols.emplace_back(pef_command_hdr.Name);
    }
  }

  if (!unreferenced_symbols.empty()) {
    for (auto &unreferenced_symbol : unreferenced_symbols) {
      kStdOut << "ld: undefined symbol " << unreferenced_symbol << "\n";
    }
  }

  if (!kStartFound || kDuplicateSymbols && std::filesystem::exists(kOutput) ||
      !unreferenced_symbols.empty()) {
    if (kVerbose)
      kStdOut << "ld: code for: " << kOutput
              << ", is corrupt, removing file...\n";

    std::remove(kOutput.c_str());
    return -CXXKIT_EXEC_ERROR;
  }

  return 0;
}

// Last rev 13-1-24
