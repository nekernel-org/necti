/* -------------------------------------------

  Copyright (C) 2024-2025 Amlal El Mahrouss, all rights reserved

  @file DynamicLinker64PEF.cc
  @brief: C++ 64-Bit PEF Linker for NeKernel.org

------------------------------------------- */

/// @author EL Mahrouss Amlal (amlal@nekernel.org)
/// @brief NeKernel.org 64-bit PEF Linker.
/// Last Rev: Sat Apr 19 CET 2025
/// @note Do not look up for anything with .code64/.data64/.zero64!
/// It will be loaded when the program loader will start the image.


#include <LibCompiler/Defines.h>
#include <LibCompiler/ErrorID.h>
#include <LibCompiler/CodeGen.h>
#include <LibCompiler/PEF.h>
#include <LibCompiler/UUID.h>
#include <LibCompiler/Version.h>
#include <LibCompiler/AE.h>
#include <LibCompiler/Util/CompilerUtils.h>

#define kLinkerVersionStr                                                           \
  "NeKernel.org 64-Bit Linker (Preferred Executable Format) %s, (c) Amlal El Mahrouss " \
  "2024-2025 "                                                                      \
  "all rights reserved.\n"

#define MemoryCopy(DST, SRC, SZ) memcpy(DST, SRC, SZ)
#define StringCompare(DST, SRC) strcmp(DST, SRC)

#define kPefNoCpu (0U)
#define kPefNoSubCpu (0U)

#define kLinkerDefaultOrigin kPefBaseOrigin
#define kLinkerId (0x5046FF)
#define kLinkerAbiContainer "__PEFContainer:ABI:"

#define kPrintF printf
#define kLinkerSplash() kConsoleOut << std::printf(kLinkerVersionStr, kDistVersion)

/// @brief PEF stack size symbol.
#define kLinkerStackSizeSymbol "__PEFSizeOfReserveStack"

#define kConsoleOut        \
  (std::cout << "\e[0;31m" \
             << "ld64: "   \
             << "\e[0;97m")

enum {
  kABITypeNull    = 0,
  kABITypeStart   = 0x1010, /* The start of ABI list. */
  kABITypeNE      = 0x5046, /* PF (NeKernel.org's PEF ABI) */
  kABITypeInvalid = 0xFFFF,
};

static LibCompiler::STLString kOutput           = "a" kPefExt;
static Int32                  kAbi              = kABITypeNE;
static Int32                  kSubArch          = kPefNoSubCpu;
static Int32                  kArch             = LibCompiler::kPefArchInvalid;
static Bool                   kFatBinaryEnable  = false;
static Bool                   kStartFound       = false;
static Bool                   kDuplicateSymbols = false;

/* ld64 is to be found, mld is to be found at runtime. */
static const Char* kLdDefineSymbol = ":UndefinedSymbol:";
static const Char* kLdDynamicSym   = ":RuntimeSymbol:";

/* object code and list. */
static std::vector<LibCompiler::STLString>    kObjectList;
static std::vector<Detail::DynamicLinkerBlob> kObjectBytes;

///	@brief NE 64-bit Linker.
/// @note This linker is made for PEF executable, thus NE based OSes.
LIBCOMPILER_MODULE(DynamicLinker64PEF) {
  bool is_executable = true;

  ::signal(SIGSEGV, Detail::drvi_crash_handler);

  /**
   * @brief parse flags and trigger options.
   */
  for (size_t linker_arg = 1; linker_arg < argc; ++linker_arg) {
    if (StringCompare(argv[linker_arg], "-help") == 0) {
      kLinkerSplash();

      kConsoleOut << "-version: Show linker version.\n";
      kConsoleOut << "-help: Show linker help.\n";
      kConsoleOut << "-verbose: Enable linker trace.\n";
      kConsoleOut << "-dylib: Output as a Dynamic PEF.\n";
      kConsoleOut << "-fat: Output as a FAT PEF.\n";
      kConsoleOut << "-32k: Output as a 32x0 PEF.\n";
      kConsoleOut << "-64k: Output as a 64x0 PEF.\n";
      kConsoleOut << "-amd64: Output as a AMD64 PEF.\n";
      kConsoleOut << "-rv64: Output as a RISC-V PEF.\n";
      kConsoleOut << "-power64: Output as a POWER PEF.\n";
      kConsoleOut << "-arm64: Output as a ARM64 PEF.\n";
      kConsoleOut << "-output: Select the output file name.\n";

      return LIBCOMPILER_SUCCESS;
    } else if (StringCompare(argv[linker_arg], "-version") == 0) {
      kLinkerSplash();

      return LIBCOMPILER_SUCCESS;
    } else if (StringCompare(argv[linker_arg], "-fat") == 0) {
      kFatBinaryEnable = true;

      continue;
    } else if (StringCompare(argv[linker_arg], "-64k") == 0) {
      kArch = LibCompiler::kPefArch64000;

      continue;
    } else if (StringCompare(argv[linker_arg], "-amd64") == 0) {
      kArch = LibCompiler::kPefArchAMD64;

      continue;
    } else if (StringCompare(argv[linker_arg], "-32k") == 0) {
      kArch = LibCompiler::kPefArch32000;

      continue;
    } else if (StringCompare(argv[linker_arg], "-power64") == 0) {
      kArch = LibCompiler::kPefArchPowerPC;

      continue;
    } else if (StringCompare(argv[linker_arg], "-riscv64") == 0) {
      kArch = LibCompiler::kPefArchRISCV;

      continue;
    } else if (StringCompare(argv[linker_arg], "-arm64") == 0) {
      kArch = LibCompiler::kPefArchARM64;

      continue;
    } else if (StringCompare(argv[linker_arg], "-verbose") == 0) {
      kVerbose = true;

      continue;
    } else if (StringCompare(argv[linker_arg], "-dylib") == 0) {
      if (kOutput.empty()) {
        continue;
      }

      if (kOutput.find(kPefExt) != LibCompiler::STLString::npos)
        kOutput.erase(kOutput.find(kPefExt), strlen(kPefExt));

      kOutput += kPefDylibExt;

      is_executable = false;

      continue;
    } else if (StringCompare(argv[linker_arg], "-output") == 0) {
      if ((linker_arg + 1) > argc) continue;

      kOutput = argv[linker_arg + 1];
      ++linker_arg;

      continue;
    } else {
      if (argv[linker_arg][0] == '-') {
        kConsoleOut << "unknown flag: " << argv[linker_arg] << "\n";
        return EXIT_FAILURE;
      }

      kObjectList.emplace_back(argv[linker_arg]);

      continue;
    }
  }

  if (kOutput.empty()) {
    kConsoleOut << "no output filename set." << std::endl;
    return LIBCOMPILER_EXEC_ERROR;
  } else if (kObjectList.empty()) {
    kConsoleOut << "no input files." << std::endl;
    return LIBCOMPILER_EXEC_ERROR;
  } else {
    namespace FS = std::filesystem;

    // check for existing files, if they don't throw an error.
    for (auto& obj : kObjectList) {
      if (!FS::exists(obj)) {
        // if filesystem doesn't find file
        //          -> throw error.
        kConsoleOut << "no such file: " << obj << std::endl;
        return LIBCOMPILER_EXEC_ERROR;
      }
    }
  }

  // PEF expects a valid target architecture when outputing a binary.
  if (kArch == LibCompiler::kPefArchInvalid) {
    kConsoleOut << "no target architecture set, can't continue." << std::endl;
    return LIBCOMPILER_EXEC_ERROR;
  }

  LibCompiler::PEFContainer pef_container{};

  int32_t archs = kArch;

  pef_container.Count    = 0UL;
  pef_container.Kind     = is_executable ? LibCompiler::kPefKindExec : LibCompiler::kPefKindDylib;
  pef_container.SubCpu   = kSubArch;
  pef_container.Linker   = kLinkerId;  // Amlal El Mahrouss Linker
  pef_container.Abi      = kAbi;       // Multi-Processor UX ABI
  pef_container.Magic[0] = kPefMagic[kFatBinaryEnable ? 2 : 0];
  pef_container.Magic[1] = kPefMagic[1];
  pef_container.Magic[2] = kPefMagic[kFatBinaryEnable ? 0 : 2];
  pef_container.Magic[3] = kPefMagic[3];
  pef_container.Version  = kPefVersion;

  // specify the start address, can be 0x10000
  pef_container.Start    = kLinkerDefaultOrigin;
  pef_container.HdrSz    = sizeof(LibCompiler::PEFContainer);
  pef_container.Checksum = 0UL;

  std::ofstream output_fc(kOutput, std::ofstream::binary);

  if (output_fc.bad()) {
    if (kVerbose) {
      kConsoleOut << "error: " << strerror(errno) << "\n";
    }

    return LIBCOMPILER_FILE_NOT_FOUND;
  }

  //! Read AE to convert as PEF.

  std::vector<LibCompiler::PEFCommandHeader> command_headers;
  LibCompiler::Utils::AEReadableProtocol     reader_protocol{};

  for (const auto& objectFile : kObjectList) {
    if (!std::filesystem::exists(objectFile)) continue;

    LibCompiler::AEHeader hdr{};

    reader_protocol._Fp = std::ifstream(objectFile, std::ifstream::binary);
    reader_protocol._Fp >> hdr;

    if (hdr.fMagic[0] == kAEMag0 && hdr.fMagic[1] == kAEMag1 &&
        hdr.fSize == sizeof(LibCompiler::AEHeader)) {
      if (hdr.fArch != kArch) {
        if (kVerbose) kConsoleOut << "is this a FAT binary? : ";

        if (!kFatBinaryEnable) {
          if (kVerbose) kConsoleOut << "not a FAT binary.\n";

          kConsoleOut << "object " << objectFile
                      << " is a different kind of architecture and output isn't "
                         "treated as a FAT binary."
                      << std::endl;

          return LIBCOMPILER_FAT_ERROR;
        } else {
          if (kVerbose) {
            kConsoleOut << "Architecture matches what we expect.\n";
          }
        }
      }

      // append arch type to archs varaible.
      archs |= hdr.fArch;
      std::size_t cnt = hdr.fCount;

      if (kVerbose) kConsoleOut << "Object header found, record count: " << cnt << "\n";

      pef_container.Count = cnt;

      char_type* raw_ae_records = new char_type[cnt * sizeof(LibCompiler::AERecordHeader)];

      if (!raw_ae_records) {
        if (kVerbose) kConsoleOut << "Allocation failure for records of count: " << cnt << "\n";
      }

      memset(raw_ae_records, 0, cnt * sizeof(LibCompiler::AERecordHeader));

      auto* ae_records = reader_protocol.Read(raw_ae_records, cnt);

      auto org = kLinkerDefaultOrigin;

      for (size_t ae_record_index = 0; ae_record_index < cnt; ++ae_record_index) {
        LibCompiler::PEFCommandHeader command_header{0};
        std::size_t                   offset_of_obj = ae_records[ae_record_index].fOffset;

        MemoryCopy(command_header.Name, ae_records[ae_record_index].fName, kPefNameLen);

        LibCompiler::STLString cmd_hdr_name(command_header.Name);

        // check this header if it's any valid.
        if (cmd_hdr_name.find(kPefCode64) == LibCompiler::STLString::npos &&
            cmd_hdr_name.find(kPefData64) == LibCompiler::STLString::npos &&
            cmd_hdr_name.find(kPefZero64) == LibCompiler::STLString::npos) {
          if (cmd_hdr_name.find(kPefStart) == LibCompiler::STLString::npos &&
              *command_header.Name == 0) {
            if (cmd_hdr_name.find(kLdDefineSymbol) != LibCompiler::STLString::npos) {
              goto ld_mark_header;
            } else {
              continue;
            }
          }
        }

        if (cmd_hdr_name.find(kPefStart) != LibCompiler::STLString::npos &&
            cmd_hdr_name.find(kPefCode64) != LibCompiler::STLString::npos) {
          kStartFound = true;
        }

      ld_mark_header:
        command_header.Offset    = offset_of_obj;
        command_header.Kind      = ae_records[ae_record_index].fKind;
        command_header.Size      = ae_records[ae_record_index].fSize;
        command_header.Cpu       = hdr.fArch;
        command_header.VMAddress = org;  /// TODO:
        command_header.SubCpu    = hdr.fSubArch;

        org += command_header.Size;

        if (kVerbose) {
          kConsoleOut << "Record: " << ae_records[ae_record_index].fName << " is marked.\n";

          kConsoleOut << "Offset: " << command_header.Offset << "\n";
        }

        command_headers.emplace_back(command_header);
      }

      delete[] raw_ae_records;
      raw_ae_records = nullptr;

      std::vector<char> bytes;
      bytes.resize(hdr.fCodeSize);

      reader_protocol._Fp.seekg(std::streamsize(hdr.fStartCode));
      reader_protocol._Fp.read(bytes.data(), std::streamsize(hdr.fCodeSize));

      kObjectBytes.push_back({.mBlob = bytes, .mOffset = hdr.fStartCode});

      // Blob was written, close fp.

      reader_protocol._Fp.close();

      continue;
    }

    kConsoleOut << "Not an AE container: " << objectFile << std::endl;

    // don't continue, it is a fatal error.
    return LIBCOMPILER_EXEC_ERROR;
  }

  pef_container.Cpu = archs;

  output_fc << pef_container;

  if (kVerbose) {
    kConsoleOut << "Wrote container to: " << output_fc.tellp() << ".\n";
  }

  output_fc.seekp(std::streamsize(pef_container.HdrSz));

  std::vector<LibCompiler::STLString> not_found;
  std::vector<LibCompiler::STLString> symbols;

  // step 2: check for errors (multiple symbols, undefined ones)

  for (auto& command_hdr : command_headers) {
    // check if this symbol needs to be resolved.
    if (LibCompiler::STLString(command_hdr.Name).find(kLdDefineSymbol) !=
            LibCompiler::STLString::npos &&
        LibCompiler::STLString(command_hdr.Name).find(kLdDynamicSym) ==
            LibCompiler::STLString::npos) {
      if (kVerbose) kConsoleOut << "Found undefined symbol: " << command_hdr.Name << "\n";

      if (auto it = std::find(not_found.begin(), not_found.end(),
                              LibCompiler::STLString(command_hdr.Name));
          it == not_found.end()) {
        not_found.emplace_back(command_hdr.Name);
      }
    }

    symbols.emplace_back(command_hdr.Name);
  }

  // Now try to solve these symbols.

  for (size_t not_found_idx = 0; not_found_idx < command_headers.size(); ++not_found_idx) {
    if (const auto it = std::find(not_found.begin(), not_found.end(),
                                  LibCompiler::STLString(command_headers[not_found_idx].Name));
        it != not_found.end()) {
      LibCompiler::STLString symbol_imp = *it;

      if (symbol_imp.find(kLdDefineSymbol) == LibCompiler::STLString::npos) continue;

      // erase the lookup prefix.
      symbol_imp.erase(0, symbol_imp.find(kLdDefineSymbol) + strlen(kLdDefineSymbol));

      // demangle everything.
      while (symbol_imp.find('$') != LibCompiler::STLString::npos)
        symbol_imp.erase(symbol_imp.find('$'), 1);

      // the reason we do is because, this may not match the symbol, and we need
      // to look for other matching symbols.
      for (auto& command_hdr : command_headers) {
        if (LibCompiler::STLString(command_hdr.Name).find(symbol_imp) !=
                LibCompiler::STLString::npos &&
            LibCompiler::STLString(command_hdr.Name).find(kLdDefineSymbol) ==
                LibCompiler::STLString::npos) {
          LibCompiler::STLString undefined_symbol = command_hdr.Name;
          auto result_of_sym = undefined_symbol.substr(undefined_symbol.find(symbol_imp));

          for (int i = 0; result_of_sym[i] != 0; ++i) {
            if (result_of_sym[i] != symbol_imp[i]) goto ld_continue_search;
          }

          not_found.erase(it);

          if (kVerbose) kConsoleOut << "Found symbol: " << command_hdr.Name << "\n";

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
      kConsoleOut << "Undefined entrypoint: " << kPefStart
                  << ", you may have forget to link "
                     "against the C++ runtime library.\n";

    kConsoleOut << "Undefined entrypoint " << kPefStart << " for executable: " << kOutput << "\n";
  }

  // step 4: write all PEF commands.

  LibCompiler::PEFCommandHeader date_cmd_hdr{};

  time_t timestamp = time(nullptr);

  LibCompiler::STLString timeStampStr = "Container:BuildEpoch:";
  timeStampStr += std::to_string(timestamp);

  strncpy(date_cmd_hdr.Name, timeStampStr.c_str(), timeStampStr.size());

  date_cmd_hdr.Flags  = 0;
  date_cmd_hdr.Kind   = LibCompiler::kPefZero;
  date_cmd_hdr.Offset = output_fc.tellp();
  date_cmd_hdr.Size   = timeStampStr.size();

  command_headers.push_back(date_cmd_hdr);

  LibCompiler::PEFCommandHeader abi_cmd_hdr{};

  LibCompiler::STLString abi = kLinkerAbiContainer;

  switch (kArch) {
    case LibCompiler::kPefArchAMD64: {
      abi += "MSFT";
      break;
    }
    case LibCompiler::kPefArchPowerPC: {
      abi += "SYSV";
      break;
    }
    case LibCompiler::kPefArch32000:
    case LibCompiler::kPefArch64000: {
      abi += "_NEP";
      break;
    }
    default: {
      abi += "_IDK";
      break;
    }
  }

  MemoryCopy(abi_cmd_hdr.Name, abi.c_str(), abi.size());

  abi_cmd_hdr.Size   = abi.size();
  abi_cmd_hdr.Offset = output_fc.tellp();
  abi_cmd_hdr.Flags  = 0;
  abi_cmd_hdr.Kind   = LibCompiler::kPefLinkerID;

  command_headers.push_back(abi_cmd_hdr);

  LibCompiler::PEFCommandHeader stack_cmd_hdr{0};

  stack_cmd_hdr.Cpu    = kArch;
  stack_cmd_hdr.Flags  = 0;
  stack_cmd_hdr.Size   = sizeof(uintptr_t);
  stack_cmd_hdr.Offset = 0;

  MemoryCopy(stack_cmd_hdr.Name, kLinkerStackSizeSymbol, strlen(kLinkerStackSizeSymbol));

  command_headers.push_back(stack_cmd_hdr);

  LibCompiler::PEFCommandHeader uuid_cmd_hdr{};

  std::random_device rd;

  auto seedData = std::array<int, std::mt19937::state_size>{};
  std::generate(std::begin(seedData), std::end(seedData), std::ref(rd));
  std::seed_seq seq(std::begin(seedData), std::end(seedData));
  std::mt19937  generator(seq);

  auto        gen     = uuids::uuid_random_generator{generator};
  uuids::uuid id      = gen();
  auto        uuidStr = uuids::to_string(id);

  MemoryCopy(uuid_cmd_hdr.Name, "Container:GUID:4:", strlen("Container:GUID:4:"));
  MemoryCopy(uuid_cmd_hdr.Name + strlen("Container:GUID:4:"), uuidStr.c_str(), uuidStr.size());

  uuid_cmd_hdr.Size   = strlen(uuid_cmd_hdr.Name);
  uuid_cmd_hdr.Offset = output_fc.tellp();
  uuid_cmd_hdr.Flags  = LibCompiler::kPefLinkerID;
  uuid_cmd_hdr.Kind   = LibCompiler::kPefZero;

  command_headers.push_back(uuid_cmd_hdr);

  // prepare a symbol vector.
  std::vector<LibCompiler::STLString> undef_symbols;
  std::vector<LibCompiler::STLString> dupl_symbols;
  std::vector<LibCompiler::STLString> resolve_symbols;

  constexpr Int32 kPaddingOffset = 16;

  size_t previous_offset =
      (command_headers.size() * sizeof(LibCompiler::PEFCommandHeader)) + kPaddingOffset;

  LibCompiler::PEFCommandHeader end_exec_hdr;

  end_exec_hdr.Offset = output_fc.tellp();
  end_exec_hdr.Flags  = LibCompiler::kPefLinkerID;
  end_exec_hdr.Kind   = LibCompiler::kPefZero;

  MemoryCopy(end_exec_hdr.Name, "Container:Exec:END", strlen("Container:Exec:END"));

  end_exec_hdr.Size = strlen(end_exec_hdr.Name);

  command_headers.push_back(end_exec_hdr);

  // Finally write down the command headers.
  // And check for any duplications
  for (size_t commandHeaderIndex = 0UL; commandHeaderIndex < command_headers.size();
       ++commandHeaderIndex) {
    if (LibCompiler::STLString(command_headers[commandHeaderIndex].Name).find(kLdDefineSymbol) !=
            LibCompiler::STLString::npos &&
        LibCompiler::STLString(command_headers[commandHeaderIndex].Name).find(kLdDynamicSym) ==
            LibCompiler::STLString::npos) {
      // ignore :UndefinedSymbol: headers, they do not contain code.
      continue;
    }

    LibCompiler::STLString symbol_name = command_headers[commandHeaderIndex].Name;

    if (!symbol_name.empty()) {
      undef_symbols.emplace_back(symbol_name);
    }

    command_headers[commandHeaderIndex].Offset += previous_offset;
    previous_offset += command_headers[commandHeaderIndex].Size;

    LibCompiler::STLString name = command_headers[commandHeaderIndex].Name;

    /// so this is valid when we get to the entrypoint.
    /// it is always a code64 container. And should equal to kPefStart as well.
    /// this chunk of code updates the pef_container.Start with the updated offset.
    if (name.find(kPefStart) != LibCompiler::STLString::npos &&
        name.find(kPefCode64) != LibCompiler::STLString::npos) {
      pef_container.Start = command_headers[commandHeaderIndex].Offset;
      auto tellCurPos     = output_fc.tellp();

      output_fc.seekp(0);
      output_fc << pef_container;

      output_fc.seekp(tellCurPos);
    }

    if (kVerbose) {
      kConsoleOut << "Command name: " << name << "\n";
      kConsoleOut << "VMAddress of command content: " << command_headers[commandHeaderIndex].Offset
                  << "\n";
    }

    output_fc << command_headers[commandHeaderIndex];

    for (size_t sub_command_header_index = 0UL; sub_command_header_index < command_headers.size();
         ++sub_command_header_index) {
      if (sub_command_header_index == commandHeaderIndex) continue;

      if (LibCompiler::STLString(command_headers[sub_command_header_index].Name)
                  .find(kLdDefineSymbol) != LibCompiler::STLString::npos &&
          LibCompiler::STLString(command_headers[sub_command_header_index].Name)
                  .find(kLdDynamicSym) == LibCompiler::STLString::npos) {
        if (kVerbose) {
          kConsoleOut << "Ignoring :UndefinedSymbol: headers...\n";
        }

        // ignore :UndefinedSymbol: headers, they do not contain code.
        continue;
      }

      auto& command_hdr = command_headers[sub_command_header_index];

      if (command_hdr.Name == LibCompiler::STLString(command_headers[commandHeaderIndex].Name)) {
        if (std::find(dupl_symbols.cbegin(), dupl_symbols.cend(), command_hdr.Name) ==
            dupl_symbols.cend()) {
          dupl_symbols.emplace_back(command_hdr.Name);
        }

        if (kVerbose) kConsoleOut << "Found duplicate symbols of: " << command_hdr.Name << "\n";

        kDuplicateSymbols = true;
      }
    }
  }

  if (!dupl_symbols.empty()) {
    for (auto& symbol : dupl_symbols) {
      kConsoleOut << "Multiple symbols of: " << symbol << " detected, cannot continue.\n";
    }

    return LIBCOMPILER_EXEC_ERROR;
  }

  // step 2.5: write program bytes.

  for (auto& struct_of_blob : kObjectBytes) {
    output_fc.write(struct_of_blob.mBlob.data(), struct_of_blob.mBlob.size());
  }

  if (kVerbose) {
    kConsoleOut << "Wrote contents of: " << kOutput << "\n";
  }

  // step 3: check if we have those symbols

  std::vector<LibCompiler::STLString> unreferenced_symbols;

  for (auto& command_hdr : command_headers) {
    if (auto it =
            std::find(not_found.begin(), not_found.end(), LibCompiler::STLString(command_hdr.Name));
        it != not_found.end()) {
      unreferenced_symbols.emplace_back(command_hdr.Name);
    }
  }

  if (!unreferenced_symbols.empty()) {
    for (auto& unreferenced_symbol : unreferenced_symbols) {
      kConsoleOut << "Undefined symbol " << unreferenced_symbol << "\n";
    }

    return LIBCOMPILER_EXEC_ERROR;
  }

  if ((!kStartFound || kDuplicateSymbols) &&
      (std::filesystem::exists(kOutput) || !unreferenced_symbols.empty())) {
    if (kVerbose) {
      kConsoleOut << "File: " << kOutput << ", is corrupt, removing file...\n";
    }

    return LIBCOMPILER_EXEC_ERROR;
  }

  return LIBCOMPILER_SUCCESS;
}

// Last rev 13-1-24
