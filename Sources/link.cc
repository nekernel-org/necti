/* -------------------------------------------

    Copyright Mahrouss Logic

------------------------------------------- */

/// BUGS: 0

/// Last Rev: Sat Feb 24 CET 2024

/// @file link.cc
/// @author Amlal El Mahrouss (amlel)
/// @brief Linker.

// README: Do not look up for anything with .code64/.data64/.page_zero!
// It will be loaded when program will start up!

#include <Headers/StdKit/ErrorID.hpp>

//! Assembler Kit
#include <Headers/AsmKit/AsmKit.hpp>

//! Portable Executable Format
#include <Headers/StdKit/PEF.hpp>
#include <Headers/UUID.hpp>
#include <filesystem>
#include <random>
#include <vector>

//! Advanced Executable Object Format
#include <Headers/StdKit/AE.hpp>

//! Dist version
#include <Version.hxx>

//! C++ I/O headers.
#include <fstream>
#include <iostream>

#define kLinkerVersion "Mahrouss Linker %s, (c) Mahrouss Logic 2024\n"

#define StringCompare(DST, SRC) strcmp(DST, SRC)

#define kPefNoCpu 0U
#define kPefNoSubCpu 0U

#define kWhite "\e[0;97m"
#define kStdOut (std::cout << kWhite)

#define kPefDeaultOrg kPefBaseOrigin
#define kPefLinkerNumId 0x5046FF
#define kPefAbiId "Container:Abi:"

enum { kStandardAbi = 0x5046 /* PF */ };

static std::string kOutput;
static Int32 kAbi = kStandardAbi;
static Int32 kSubArch = kPefNoSubCpu;
static Int32 kArch = CompilerKit::kPefArch64000;
static Bool kFatBinaryEnable = false;
static Bool kStartFound = false;
static Bool kDuplicateSymbols = false;
static Bool kVerbose = false;

/* link is to be found, mld is to be found at runtime. */
static const char *kLdDefineSymbol = ":UndefinedSymbol:";
static const char *kLdDynamicSym = ":RuntimeSymbol:";

/* object code and list. */
static std::vector<std::string> kObjectList;
static std::vector<char> kObjectBytes;

#define kPrintF printf
#define kSplashLink() kPrintF(kWhite kLinkerVersion, kDistVersion)

MPCC_MODULE(NewOSLinker) {
  bool is_executable = true;

  /**
   * @brief parse flags and such.
   *
   */
  for (size_t i = 1; i < argc; ++i) {
    if (StringCompare(argv[i], "-h") == 0) {
      kSplashLink();
      kStdOut << "-version: Show program version.\n";
      kStdOut << "-verbose: Enable program trace.\n";
      kStdOut << "-shared: Output as a shared PEF.\n";
      kStdOut << "-fat-bin: Output as FAT PEF.\n";
      kStdOut << "-32x0: Output as 32x0 PEF.\n";
      kStdOut << "-64x0: Output as 64x0 PEF.\n";
      kStdOut << "-amd64: Output as AMD64 PEF.\n";
      kStdOut << "-rv64: Output as RISC-V 64 PEF.\n";
      kStdOut << "-ppc64: Output as POWER 64 PEF.\n";
      kStdOut << "-output-file: Select output file name.\n";

      return 0;
    } else if (StringCompare(argv[i], "-v") == 0) {
      kSplashLink();
      return 0;
    } else if (StringCompare(argv[i], "-fat-bin") == 0) {
      kFatBinaryEnable = true;

      continue;
    } else if (StringCompare(argv[i], "-64x0") == 0) {
      kArch = CompilerKit::kPefArch64000;

      continue;
    } else if (StringCompare(argv[i], "-amd64") == 0) {
      kArch = CompilerKit::kPefArchAMD64;

      continue;
    } else if (StringCompare(argv[i], "-32x0") == 0) {
      kArch = CompilerKit::kPefArch32000;

      continue;
    } else if (StringCompare(argv[i], "-ppc64") == 0) {
      kArch = CompilerKit::kPefArchPowerPC;

      continue;
    } else if (StringCompare(argv[i], "-verbose") == 0) {
      kVerbose = true;

      continue;
    } else if (StringCompare(argv[i], "-shared") == 0) {
      if (kOutput.empty()) {
        continue;
      }

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
        kStdOut << "link: unknown flag: " << argv[i] << "\n";
        return -MPCC_EXEC_ERROR;
      }

      kObjectList.emplace_back(argv[i]);

      continue;
    }
  }

  if (kOutput.empty()) {
    kStdOut << "link: no output filename set." << std::endl;
    return MPCC_EXEC_ERROR;
  }

  // sanity check.
  if (kObjectList.empty()) {
    kStdOut << "link: no input files." << std::endl;
    return MPCC_EXEC_ERROR;
  } else {
    // check for existing files, if they don't throw an error.
    for (auto &obj : kObjectList) {
      if (!std::filesystem::exists(obj)) {
        // if filesystem doesn't find file
        //          -> throw error.
        kStdOut << "link: no such file: " << obj << std::endl;
        return MPCC_EXEC_ERROR;
      }
    }
  }

  // PEF expects a valid architecture when outputing a binary.
  if (kArch == 0) {
    kStdOut << "link: no target architecture set, can't continue." << std::endl;
    return MPCC_EXEC_ERROR;
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
  pef_container.Magic[3] = kPefMagic[3];
  pef_container.Version = kPefVersion;

  // specify the start address, can be 0x10000
  pef_container.Start = kPefDeaultOrg;
  pef_container.HdrSz = sizeof(CompilerKit::PEFContainer);

  std::ofstream outputFc(kOutput, std::ofstream::binary);

  if (outputFc.bad()) {
    if (kVerbose) {
      kStdOut << "link: error: " << strerror(errno) << "\n";
    }

    return -MPCC_FILE_NOT_FOUND;
  }

  //! Read AE to convert as PEF.

  std::vector<CompilerKit::PEFCommandHeader> commandHdrsList;
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
        if (kVerbose) kStdOut << "link: info: is this a FAT binary? : ";

        if (!kFatBinaryEnable) {
          if (kVerbose) kStdOut << "No.\n";

          kStdOut << "link: error: object " << i
                  << " is a different kind of architecture and output isn't "
                     "treated as a FAT binary."
                  << std::endl;

          std::remove(kOutput.c_str());
          return -MPCC_FAT_ERROR;
        } else {
          if (kVerbose) {
            kStdOut << "Yes.\n";
          }
        }
      }

      // append arch type to archs varaible.
      archs |= ae_header.fArch;
      std::size_t cnt = ae_header.fCount;

      if (kVerbose)
        kStdOut << "link: object header found, record count: " << cnt << "\n";

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
        if (std::string(command_header.Name).find(".code64") ==
                std::string::npos &&
            std::string(command_header.Name).find(".data64") ==
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
            std::string(command_header.Name).find(".code64") !=
                std::string::npos) {
          kStartFound = true;
          pef_container.Start = ae_records[ae_record_index].fOffset;
        }

      ld_mark_header:
        command_header.Offset = ae_records[ae_record_index].fOffset;
        command_header.Kind = ae_records[ae_record_index].fKind;
        command_header.Size = ae_records[ae_record_index].fSize;
        command_header.Cpu = ae_header.fArch;
        command_header.SubCpu = ae_header.fSubArch;

        if (kVerbose)
          kStdOut << "link: object record: "
                  << ae_records[ae_record_index].fName << " was marked.\n";

        commandHdrsList.emplace_back(command_header);
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

    kStdOut << "link: not an object: " << i << std::endl;
    std::remove(kOutput.c_str());

    // don't continue, it is a fatal error.
    return -MPCC_EXEC_ERROR;
  }

  pef_container.Cpu = archs;

  outputFc << pef_container;

  if (kVerbose) {
    kStdOut << "link: pef: wrote container header.\n";
  }

  outputFc.seekp(std::streamsize(pef_container.HdrSz));

  std::vector<std::string> not_found;
  std::vector<std::string> symbols;

  // step 2: check for errors (multiple symbols, undefined ones)

  for (auto &commandHdr : commandHdrsList) {
    // check if this symbol needs to be resolved.
    if (std::string(commandHdr.Name).find(kLdDefineSymbol) !=
            std::string::npos &&
        std::string(commandHdr.Name).find(kLdDynamicSym) == std::string::npos) {
      if (kVerbose)
        kStdOut << "link: found undefined symbol: " << commandHdr.Name << "\n";

      if (auto it = std::find(not_found.begin(), not_found.end(),
                              std::string(commandHdr.Name));
          it == not_found.end()) {
        not_found.emplace_back(commandHdr.Name);
      }
    }

    symbols.emplace_back(commandHdr.Name);
  }

  // Now try to solve these symbols.

  for (size_t not_found_idx = 0; not_found_idx < commandHdrsList.size();
       ++not_found_idx) {
    if (auto it = std::find(not_found.begin(), not_found.end(),
                            std::string(commandHdrsList[not_found_idx].Name));
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
      for (auto &commandHdr : commandHdrsList) {
        if (std::string(commandHdr.Name).find(symbol_imp) !=
                std::string::npos &&
            std::string(commandHdr.Name).find(kLdDefineSymbol) ==
                std::string::npos) {
          std::string undefined_symbol = commandHdr.Name;
          auto result_of_sym =
              undefined_symbol.substr(undefined_symbol.find(symbol_imp));

          for (int i = 0; result_of_sym[i] != 0; ++i) {
            if (result_of_sym[i] != symbol_imp[i]) goto ld_continue_search;
          }

          not_found.erase(it);

          if (kVerbose)
            kStdOut << "link: found symbol: " << commandHdr.Name << "\n";

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
      kStdOut
          << "link: undefined entrypoint: __start, you may have forget to link "
             "against your compiler's runtime library.\n";

    kStdOut << "link: undefined entrypoint " << kPefStart
            << " for executable: " << kOutput << "\n";
  }

  // step 4: write some pef commands.

  CompilerKit::PEFCommandHeader dateHeader{};

  time_t timestamp = time(nullptr);

  std::string timeStampStr = "Container:BuildEpoch:";
  timeStampStr += std::to_string(timestamp);

  strcpy(dateHeader.Name, timeStampStr.c_str());

  dateHeader.Flags = 0;
  dateHeader.Kind = CompilerKit::kPefZero;
  dateHeader.Offset = outputFc.tellp();
  dateHeader.Size = timeStampStr.size();

  outputFc << dateHeader;

  CompilerKit::PEFCommandHeader abiHeader{};

  std::string abi = kPefAbiId;

  switch (kArch) {
    case CompilerKit::kPefArchAMD64: {
      abi += "MS*T";
      break;
    }
    case CompilerKit::kPefArchPowerPC: {
      abi += "MHRP";
      break;
    }
    case CompilerKit::kPefArch32000:
    case CompilerKit::kPefArch64000: {
      abi += "MHRC";
      break;
    }
    default: {
      abi += " IDK";
      break;
    }
  }

  memcpy(abiHeader.Name, abi.c_str(), abi.size());

  abiHeader.Size = abi.size();
  abiHeader.Offset = outputFc.tellp();
  abiHeader.Flags = 0;
  abiHeader.Kind = CompilerKit::kPefLinkerID;

  outputFc << abiHeader;

  CompilerKit::PEFCommandHeader uuidHeader{};

  std::random_device rd;

  auto seedData = std::array<int, std::mt19937::state_size>{};
  std::generate(std::begin(seedData), std::end(seedData), std::ref(rd));
  std::seed_seq seq(std::begin(seedData), std::end(seedData));
  std::mt19937 generator(seq);

  auto gen = uuids::uuid_random_generator{generator};
  uuids::uuid id = gen();
  auto uuidStr = uuids::to_string(id);

  memcpy(uuidHeader.Name, "Container:GUID:4:", strlen("Container:GUID:4:"));
  memcpy(uuidHeader.Name + strlen("Container:GUID:4:"), uuidStr.c_str(),
         uuidStr.size());

  uuidHeader.Size = 16;
  uuidHeader.Offset = outputFc.tellp();
  uuidHeader.Flags = 0;
  uuidHeader.Kind = CompilerKit::kPefZero;

  outputFc << uuidHeader;

  // prepare a symbol vector.
  std::vector<std::string> undefSymbols;
  std::vector<std::string> duplSymbols;
  std::vector<std::string> resolveSymbols;

  // Finally write down the command headers.
  // And check for any duplications
  for (size_t commandHeaderIndex = 0UL;
       commandHeaderIndex < commandHdrsList.size(); ++commandHeaderIndex) {
    if (std::string(commandHdrsList[commandHeaderIndex].Name)
                .find(kLdDefineSymbol) != std::string::npos &&
        std::string(commandHdrsList[commandHeaderIndex].Name)
                .find(kLdDynamicSym) == std::string::npos) {
      // ignore :UndefinedSymbol: headers, they do not contain code.
      continue;
    }

    std::string symbolName = commandHdrsList[commandHeaderIndex].Name;

    if (!symbolName.empty()) {
      undefSymbols.emplace_back(symbolName);
    }

    outputFc << commandHdrsList[commandHeaderIndex];

    for (size_t subCommandHeaderIndex = 0UL;
         subCommandHeaderIndex < commandHdrsList.size();
         ++subCommandHeaderIndex) {
      if (subCommandHeaderIndex == commandHeaderIndex) continue;

      if (std::string(commandHdrsList[subCommandHeaderIndex].Name)
                  .find(kLdDefineSymbol) != std::string::npos &&
          std::string(commandHdrsList[subCommandHeaderIndex].Name)
                  .find(kLdDynamicSym) == std::string::npos) {
        if (kVerbose) {
          kStdOut << "link: ignore :UndefinedSymbol: command header...\n";
        }

        // ignore :UndefinedSymbol: headers, they do not contain code.
        continue;
      }

      auto &commandHdr = commandHdrsList[subCommandHeaderIndex];

      if (commandHdr.Name ==
          std::string(commandHdrsList[commandHeaderIndex].Name)) {
        if (std::find(duplSymbols.cbegin(), duplSymbols.cend(),
                      commandHdr.Name) == duplSymbols.cend()) {
          duplSymbols.emplace_back(commandHdr.Name);
        }

        if (kVerbose)
          kStdOut << "link: found duplicate symbol: " << commandHdr.Name
                  << "\n";

        kDuplicateSymbols = true;
      }
    }
  }

  if (!duplSymbols.empty()) {
    for (auto &symbol : duplSymbols) {
      kStdOut << "link: multiple symbols of " << symbol << ".\n";
    }

    std::remove(kOutput.c_str());
    return -MPCC_EXEC_ERROR;
  }

  // step 2.5: write program bytes.

  for (auto byte : kObjectBytes) {
    outputFc << byte;
  }

  if (kVerbose) kStdOut << "link: wrote code for: " << kOutput << "\n";

  // step 3: check if we have those symbols

  std::vector<std::string> unrefSyms;

  for (auto &commandHdr : commandHdrsList) {
    if (auto it = std::find(not_found.begin(), not_found.end(),
                            std::string(commandHdr.Name));
        it != not_found.end()) {
      unrefSyms.emplace_back(commandHdr.Name);
    }
  }

  if (!unrefSyms.empty()) {
    for (auto &unreferenced_symbol : unrefSyms) {
      kStdOut << "link: undefined symbol " << unreferenced_symbol << "\n";
    }
  }

  if (!kStartFound || kDuplicateSymbols && std::filesystem::exists(kOutput) ||
      !unrefSyms.empty()) {
    if (kVerbose)
      kStdOut << "link: file: " << kOutput
              << ", is corrupt, removing file...\n";

    std::remove(kOutput.c_str());
    return -MPCC_EXEC_ERROR;
  }

  return 0;
}

// Last rev 13-1-24
