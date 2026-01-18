// Copyright 2024-2025, Amlal El Mahrouss (amlal@nekernel.org)
// Licensed under the Apache License, Version 2.0 (See accompanying
// file LICENSE or copy at http://www.apache.org/licenses/LICENSE-2.0)
// Official repository: https://github.com/nekernel-org/nectar

/// @author Amlal El Mahrouss (amlal@nekernel.org)
/// @brief NeKernel.org 64-bit Mach-O Linker.
/// Last Rev: 2026
/// @note Outputs Mach-O executables with __TEXT and __DATA segments.

#include <CompilerKit/AE.h>
#include <CompilerKit/CodeGenerator.h>
#include <CompilerKit/Detail/Config.h>
#include <CompilerKit/ErrorID.h>
#include <CompilerKit/MachO.h>
#include <CompilerKit/PEF.h>
#include <CompilerKit/UUID.h>
#include <CompilerKit/Utilities/Compiler.h>

#define kMachODefaultOutput \
  { "a.out" }

#define kLinkerVersionStr "Nectar 64-Bit Linker (Mach-O)"

#define kLinkerSplash() kStdOut << kLinkerVersionStr << kStdEndl

#define kConsoleOut        \
  (std::cout << "\e[0;31m" \
             << "mld64: "  \
             << "\e[0;97m")

static CompilerKit::STLString kOutput              = kMachODefaultOutput;
static cpu_type_t             kCpuType             = CPU_TYPE_X86_64;
static cpu_subtype_t          kCpuSubType          = CPU_SUBTYPE_X86_64_ALL;
static bool                   kFatBinaryEnable     = false;
static bool                   kStartFound          = false;
static bool                   kDuplicateSymbols    = false;
static bool                   kIsDylib             = false;
static Int64                  kMachODefaultStackSz = 0;

static CompilerKit::STLString kLinkerStart = "__ImageStart";

/* object code and list. */
static std::vector<CompilerKit::STLString>    kObjectList;
static std::vector<CompilerKit::Detail::Blob> kTextBytes;
static std::vector<CompilerKit::Detail::Blob> kDataBytes;

/* symbol table */
static std::vector<nlist_64>                    kSymbolTable;
static std::vector<Char>                        kStringTable;
static std::map<CompilerKit::STLString, UInt64> kSymbolOffsets;

/// @brief Structure to hold section information from AE records
struct SectionInfo {
  CompilerKit::STLString name;
  UInt32                 kind;
  std::vector<Char>      bytes;
  UInt64                 address;
  UInt64                 size;
};

/// @brief Extract clean symbol name from AE record name
/// AE format: ".code64$symbolname" or "symbolname.code64"
static CompilerKit::STLString ExtractSymbolName(const CompilerKit::STLString& aeName) {
  CompilerKit::STLString name = aeName;

  // Remove section prefixes/suffixes
  const Char* sections[] = {".code64", ".data64", ".zero64", "$"};

  for (const auto& sec : sections) {
    size_t pos;
    while ((pos = name.find(sec)) != CompilerKit::STLString::npos) {
      name.erase(pos, strlen(sec));
    }
  }

  // Trim whitespace
  while (!name.empty() && (name.front() == ' ' || name.front() == '\t')) {
    name.erase(0, 1);
  }
  while (!name.empty() && (name.back() == ' ' || name.back() == '\t')) {
    name.pop_back();
  }

  return name;
}

/// @brief Add a symbol to the symbol table
static UInt32 AddSymbol(const CompilerKit::STLString& name, uint8_t type, uint8_t sect,
                        UInt64 value) {
  // Add name to string table (offset 0 is reserved for empty string)
  if (kStringTable.empty()) {
    kStringTable.push_back('\0');  // First byte is null
  }

  UInt32 strOffset = static_cast<UInt32>(kStringTable.size());

  for (Char c : name) {
    kStringTable.push_back(c);
  }
  kStringTable.push_back('\0');

  // Create nlist_64 entry
  nlist_64 sym{};
  sym.n_un.n_strx = strOffset;
  sym.n_type      = type;
  sym.n_sect      = sect;
  sym.n_desc      = 0;
  sym.n_value     = value;

  kSymbolTable.push_back(sym);
  kSymbolOffsets[name] = value;

  return static_cast<UInt32>(kSymbolTable.size() - 1);
}

///	@brief Nectar 64-bit Mach-O Linker.
/// @note This linker outputs Mach-O executables for macOS/iOS.
NECTAR_MODULE(DynamicLinker64MachO) {
  CompilerKit::install_signal(SIGSEGV, CompilerKit::Detail::drvi_crash_handler);

  /**
   * @brief parse flags and trigger options.
   */
  for (size_t linker_arg = 1; linker_arg < argc; ++linker_arg) {
    if (std::strcmp(argv[linker_arg], "-help") == 0) {
      kLinkerSplash();

      kConsoleOut << "-version: Show linker version.\n";
      kConsoleOut << "-help: Show linker help.\n";
      kConsoleOut << "-verbose: Enable linker trace.\n";
      kConsoleOut << "-dylib: Output as a Dynamic Library.\n";
      kConsoleOut << "-fat: Output as a FAT binary.\n";
      kConsoleOut << "-amd64: Output as an x86_64 Mach-O.\n";
      kConsoleOut << "-arm64: Output as an ARM64 Mach-O.\n";
      kConsoleOut << "-output: Select the output file name.\n";
      kConsoleOut << "-start: Specify entry point symbol.\n";

      return NECTAR_SUCCESS;
    } else if (std::strcmp(argv[linker_arg], "-version") == 0) {
      kLinkerSplash();

      return NECTAR_SUCCESS;
    } else if (std::strcmp(argv[linker_arg], "-fat") == 0) {
      kFatBinaryEnable = true;

      continue;
    } else if (std::strcmp(argv[linker_arg], "-amd64") == 0) {
      kCpuType    = CPU_TYPE_X86_64;
      kCpuSubType = CPU_SUBTYPE_X86_64_ALL;

      continue;
    } else if (std::strcmp(argv[linker_arg], "-arm64") == 0) {
      kCpuType    = CPU_TYPE_ARM64;
      kCpuSubType = CPU_SUBTYPE_ARM64_ALL;

      continue;
    } else if (std::strcmp(argv[linker_arg], "-start") == 0) {
      if (argv[linker_arg + 1] == nullptr || argv[linker_arg + 1][0] == '-') continue;

      kLinkerStart = argv[linker_arg + 1];
      linker_arg += 1;

      continue;
    } else if (std::strcmp(argv[linker_arg], "-verbose") == 0) {
      kVerbose = true;

      continue;
    } else if (std::strcmp(argv[linker_arg], "-dylib") == 0) {
      kIsDylib = true;

      if (kOutput.find(".out") != CompilerKit::STLString::npos) {
        kOutput.erase(kOutput.find(".out"), strlen(".out"));
        kOutput += ".dylib";
      }

      continue;
    } else if (std::strcmp(argv[linker_arg], "-output") == 0) {
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
    return NECTAR_EXEC_ERROR;
  } else if (kObjectList.empty()) {
    kConsoleOut << "no input files." << std::endl;
    return NECTAR_EXEC_ERROR;
  } else {
    namespace FS = std::filesystem;

    // check for existing files, if they don't throw an error.
    for (auto& obj : kObjectList) {
      if (!FS::exists(obj)) {
        kConsoleOut << "no such file: " << obj << std::endl;
        return NECTAR_EXEC_ERROR;
      }
    }
  }

  std::vector<SectionInfo>               sections;
  CompilerKit::Utils::AEReadableProtocol reader_protocol{};

  entry_point_command entryCommand{};
  entryCommand.stacksize = kMachODefaultStackSz;

  // Collect all text and data from AE object files
  for (const auto& objectFile : kObjectList) {
    if (!std::filesystem::exists(objectFile)) continue;

    CompilerKit::AEHeader hdr{};

    reader_protocol.fFilePtr = std::ifstream(objectFile, std::ifstream::binary);
    reader_protocol.fFilePtr >> hdr;

    if (hdr.fMagic[0] == kAEMag0 && hdr.fMagic[1] == kAEMag1 &&
        hdr.fSize == sizeof(CompilerKit::AEHeader) && hdr.fMagic[2] == kAEMag2) {
      std::size_t cnt = hdr.fCount;

      if (kVerbose) kConsoleOut << "header found, record count: " << cnt << "\n";

      Char* raw_ae_records = new Char[cnt * sizeof(CompilerKit::AERecordHeader)];

      if (!raw_ae_records) {
        if (kVerbose) kConsoleOut << "allocation failed for records of count: " << cnt << "\n";
        return NECTAR_EXEC_ERROR;
      }

      std::memset(raw_ae_records, 0, cnt * sizeof(CompilerKit::AERecordHeader));

      auto* ae_records = reader_protocol.Read(raw_ae_records, cnt);

      for (size_t ae_record_index = 0; ae_record_index < cnt; ++ae_record_index) {
        SectionInfo section;
        section.name = ae_records[ae_record_index].fName;
        section.kind = ae_records[ae_record_index].fKind;
        section.size = ae_records[ae_record_index].fSize;

        // Extract clean symbol name and add to symbol table
        CompilerKit::STLString symbolName = ExtractSymbolName(section.name);

        if (!symbolName.empty()) {
          // Determine section number (1 = __text, 2 = __data)
          uint8_t sectNum = 0;
          if (section.kind == CompilerKit::kPefCode) {
            sectNum = 1;  // __text section
          } else if (section.kind == CompilerKit::kPefData) {
            sectNum = 2;  // __data section
          }

          // N_EXT = external, N_SECT = defined in section
          uint8_t symType = N_EXT | N_SECT;

          AddSymbol(symbolName, symType, sectNum, ae_records[ae_record_index].fOffset);

          if (kVerbose) {
            kConsoleOut << "Added symbol: " << symbolName
                        << " at offset: " << ae_records[ae_record_index].fOffset << "\n";
          }
        }

        sections.push_back(section);
      }

      // Look up entry point from symbol table
      auto entryIt = kSymbolOffsets.find(kLinkerStart);
      if (entryIt != kSymbolOffsets.end()) {
        entryCommand.entryoff = entryIt->second;
        kStartFound           = true;

        if (kVerbose) {
          kConsoleOut << "Found entry point " << kLinkerStart << " at offset: " << entryIt->second
                      << "\n";
        }
      }

      delete[] raw_ae_records;

      // Read the actual code bytes
      std::vector<Char> bytes;
      bytes.resize(hdr.fCodeSize);

      reader_protocol.fFilePtr.seekg(std::streamsize(hdr.fStartCode));
      reader_protocol.fFilePtr.read(bytes.data(), std::streamsize(hdr.fCodeSize));

      // Separate code and data based on section kind
      for (auto& section : sections) {
        if (section.kind == CompilerKit::kPefCode) {
          kTextBytes.push_back({.mBlob = bytes, .mOffset = 0});
        } else if (section.kind == CompilerKit::kPefData) {
          kDataBytes.push_back({.mBlob = bytes, .mOffset = 0});
        }
      }

      reader_protocol.fFilePtr.close();
      continue;
    }

    kConsoleOut << "not an object container: " << objectFile << std::endl;
    return NECTAR_EXEC_ERROR;
  }

  // Check for entry point in executables
  if (!kStartFound && !kIsDylib) {
    kConsoleOut << "Undefined entrypoint " << kLinkerStart << " for executable: " << kOutput
                << "\n";
  }

  // Calculate sizes
  UInt64 textSize = 0;
  UInt64 dataSize = 0;

  for (auto& blob : kTextBytes) {
    textSize += blob.mBlob.size();
  }

  for (auto& blob : kDataBytes) {
    dataSize += blob.mBlob.size();
  }

  // Open output file
  std::ofstream output_fc(kOutput, std::ofstream::binary);

  if (output_fc.bad()) {
    if (kVerbose) {
      kConsoleOut << "error: " << strerror(errno) << "\n";
    }
    return NECTAR_FILE_NOT_FOUND;
  }

  using namespace CompilerKit::MachO;

  UInt32 numCommands = 8;  // __PAGEZERO, LC_BUILD_VERSION, __TEXT, __LINKEDIT, LC_LOAD_DYLINKER,
                           // LC_UUID, LC_SYMTAB, LC_DYSYMTAB

  if (!kIsDylib) {
    numCommands += 1;  // LC_MAIN
  }

  UInt32 dataSegCmdSize =
      kDataBytes.size() > 0 ? sizeof(segment_command_64) + sizeof(section_64) : 0;

  if (dataSegCmdSize > 0) ++numCommands;  // __DATA segment

  UInt32 sizeOfCmds      = 0;
  UInt32 headerSize      = sizeof(mach_header_64);
  UInt32 pageZeroSize    = sizeof(segment_command_64);
  UInt32 textSegCmdSize  = sizeof(segment_command_64) + sizeof(section_64);
  UInt32 buildCmdSize    = sizeof(build_version_command);
  UInt32 mainCmdSize     = sizeof(entry_point_command);
  UInt32 uuidCmdSize     = sizeof(uuid_command);
  UInt32 symtabCmdSize   = sizeof(symtab_command);
  UInt32 dysymtabCmdSize = sizeof(dysymtab_command);
  UInt32 linkeditCmdSize = sizeof(segment_command_64);  // No sections
  UInt32 dylinkerCmdSize =
      (sizeof(dylinker_command) + 13 + 1 + 7) & ~7;  // "/usr/lib/dyld" + padding to 8-byte align

  sizeOfCmds = pageZeroSize + textSegCmdSize + dataSegCmdSize + buildCmdSize + uuidCmdSize +
               symtabCmdSize + dysymtabCmdSize + linkeditCmdSize + dylinkerCmdSize;

  if (!kIsDylib) sizeOfCmds += mainCmdSize;

  UInt64 headerAndCmdsSize = headerSize + sizeOfCmds;
  UInt64 textFileOffset    = AlignToPage(headerAndCmdsSize);
  UInt64 textVMAddr        = kDefaultBaseAddress;
  UInt64 textSegmentSize   = AlignToPage(textSize > 0 ? textSize : kPageSize);
  UInt64 textVMSize        = textFileOffset + textSegmentSize;  // __TEXT includes header

  UInt64 dataFileOffset  = textFileOffset + textSegmentSize;
  UInt64 dataVMAddr      = textVMAddr + textVMSize;
  UInt64 dataSegmentSize = dataSize > 0 ? AlignToPage(dataSize) : 0;  // 0 if no data

  // __LINKEDIT segment comes after data segment (or __TEXT if no data)
  UInt64 linkeditFileOffset =
      dataSegmentSize > 0 ? dataFileOffset + dataSegmentSize : textFileOffset + textSegmentSize;
  UInt64 linkeditVMAddr =
      dataSegmentSize > 0 ? dataVMAddr + dataSegmentSize : textVMAddr + textVMSize;
  UInt64 symtabFileOffset = linkeditFileOffset;
  UInt64 strtabFileOffset = symtabFileOffset + (kSymbolTable.size() * sizeof(nlist_64));
  UInt64 linkeditFileSize = (kSymbolTable.size() * sizeof(nlist_64)) + kStringTable.size();
  UInt64 linkeditVMSize   = AlignToPage(linkeditFileSize > 0 ? linkeditFileSize : 1);

  // Write Mach-O header
  mach_header_64 header{};

  header.magic      = MH_MAGIC_64;
  header.cputype    = kCpuType;
  header.cpusubtype = kCpuSubType;
  header.filetype   = kIsDylib ? MH_DYLIB : MH_EXECUTE;
  header.ncmds      = numCommands;
  header.sizeofcmds = sizeOfCmds;
  header.flags      = MH_NOUNDEFS | MH_DYLDLINK | MH_TWOLEVEL | MH_PIE;
  header.reserved   = 0;

  output_fc.write(reinterpret_cast<const Char*>(&header), sizeof(header));

  if (kVerbose) {
    kConsoleOut << "Wrote Mach-O header, ncmds: " << numCommands << "\n";
  }

  segment_command_64 pageZeroSegment{};
  pageZeroSegment.cmd     = LC_SEGMENT_64;
  pageZeroSegment.cmdsize = sizeof(segment_command_64);
  CopySegmentName(pageZeroSegment.segname, kSegmentPageZero);
  pageZeroSegment.vmaddr   = 0;
  pageZeroSegment.vmsize   = 0x100000000;
  pageZeroSegment.fileoff  = 0;
  pageZeroSegment.filesize = 0;
  pageZeroSegment.maxprot  = 0;
  pageZeroSegment.initprot = 0;
  pageZeroSegment.nsects   = 0;
  pageZeroSegment.flags    = 0;

  output_fc.write(reinterpret_cast<const Char*>(&pageZeroSegment), sizeof(pageZeroSegment));

  build_version_command build = {.cmd      = LC_BUILD_VERSION,
                                 .cmdsize  = sizeof(build_version_command),
                                 .platform = PLATFORM_MACOS,
                                 .minos    = (11 << 16),  // macOS 11.0
                                 .sdk      = (11 << 16),  // macOS 11.0
                                 .ntools   = 0};

  output_fc.write(reinterpret_cast<const Char*>(&build), sizeof(build));

  if (kVerbose) {
    kConsoleOut << "Wrote LC_BUILD_VERSION, platform: macOS, minos: 11.0, sdk: 11.0\n";
  }

  // Write __TEXT segment command
  segment_command_64 textSegment{};
  textSegment.cmd     = LC_SEGMENT_64;
  textSegment.cmdsize = sizeof(segment_command_64) + sizeof(section_64);  // 1 section
  CopySegmentName(textSegment.segname, kSegmentText);
  textSegment.vmaddr  = textVMAddr;
  textSegment.vmsize  = textVMSize;  // Header + code (page-aligned)
  textSegment.fileoff = 0;           // Must include Mach-O header
  textSegment.filesize =
      dataSegmentSize > 0 ? dataFileOffset : linkeditFileOffset;  // Extend to next segment
  textSegment.maxprot  = VM_PROT_READ | VM_PROT_EXECUTE;
  textSegment.initprot = VM_PROT_READ | VM_PROT_EXECUTE;
  textSegment.nsects   = 1;
  textSegment.flags    = 0;

  output_fc.write(reinterpret_cast<const Char*>(&textSegment), sizeof(textSegment));

  // Write __text section header
  section_64 textSection{};
  CopySegmentName(textSection.sectname, kSectionText);
  CopySegmentName(textSection.segname, kSegmentText);
  textSection.addr      = textVMAddr + textFileOffset;  // Section is at offset within segment
  textSection.size      = textSize;
  textSection.offset    = static_cast<UInt32>(textFileOffset);
  textSection.align     = kSectionAlign;
  textSection.reloff    = 0;
  textSection.nreloc    = 0;
  textSection.flags     = S_ATTR_PURE_INSTRUCTIONS | S_ATTR_SOME_INSTRUCTIONS;
  textSection.reserved1 = 0;
  textSection.reserved2 = 0;
  textSection.reserved3 = 0;

  output_fc.write(reinterpret_cast<const Char*>(&textSection), sizeof(textSection));

  if (kVerbose) {
    kConsoleOut << "Wrote __TEXT segment, vmaddr: 0x" << std::hex << textVMAddr << std::dec << "\n";
    kConsoleOut << "  __text section, size: " << textSize << " bytes\n";
  }

  // Write __DATA segment command
  segment_command_64 dataSegment{};
  dataSegment.cmd     = LC_SEGMENT_64;
  dataSegment.cmdsize = sizeof(segment_command_64) + sizeof(section_64);  // 1 section
  CopySegmentName(dataSegment.segname, kSegmentData);
  dataSegment.vmaddr   = dataVMAddr;
  dataSegment.vmsize   = dataSegmentSize;
  dataSegment.fileoff  = dataFileOffset;
  dataSegment.filesize = dataSize;
  dataSegment.maxprot  = VM_PROT_READ | VM_PROT_WRITE;
  dataSegment.initprot = VM_PROT_READ | VM_PROT_WRITE;
  dataSegment.nsects   = 1;
  dataSegment.flags    = 0;

  if (dataSegCmdSize > 0)
    output_fc.write(reinterpret_cast<const Char*>(&dataSegment), sizeof(dataSegment));

  // Write __data section header
  section_64 dataSection{};
  CopySegmentName(dataSection.sectname, kSectionData);
  CopySegmentName(dataSection.segname, kSegmentData);
  dataSection.addr      = dataVMAddr;
  dataSection.size      = dataSize;
  dataSection.offset    = static_cast<UInt32>(dataFileOffset);
  dataSection.align     = kSectionAlign;
  dataSection.reloff    = 0;
  dataSection.nreloc    = 0;
  dataSection.flags     = 0;
  dataSection.reserved1 = 0;
  dataSection.reserved2 = 0;
  dataSection.reserved3 = 0;

  if (dataSegCmdSize > 0)
    output_fc.write(reinterpret_cast<const Char*>(&dataSection), sizeof(dataSection));

  if (kVerbose) {
    kConsoleOut << "Wrote __DATA segment, vmaddr: 0x" << std::hex << dataVMAddr << std::dec << "\n";
    kConsoleOut << "  __data section, size: " << dataSize << " bytes\n";
  }

  // Write __LINKEDIT segment command (contains symbol/string tables)
  segment_command_64 linkeditSegment{};
  linkeditSegment.cmd     = LC_SEGMENT_64;
  linkeditSegment.cmdsize = sizeof(segment_command_64);  // No sections
  CopySegmentName(linkeditSegment.segname, "__LINKEDIT");
  linkeditSegment.vmaddr   = linkeditVMAddr;
  linkeditSegment.vmsize   = linkeditVMSize;
  linkeditSegment.fileoff  = linkeditFileOffset;
  linkeditSegment.filesize = linkeditFileSize;
  linkeditSegment.maxprot  = VM_PROT_READ;
  linkeditSegment.initprot = VM_PROT_READ;
  linkeditSegment.nsects   = 0;
  linkeditSegment.flags    = 0;

  output_fc.write(reinterpret_cast<const Char*>(&linkeditSegment), sizeof(linkeditSegment));

  if (kVerbose) {
    kConsoleOut << "Wrote __LINKEDIT segment, vmaddr: 0x" << std::hex << linkeditVMAddr << std::dec
                << ", fileoff: " << linkeditFileOffset << ", filesize: " << linkeditFileSize
                << "\n";
  }

  // Write LC_LOAD_DYLINKER command
  const Char*       dyldPath = "/usr/lib/dyld";
  std::vector<Char> dylinkerCmd(dylinkerCmdSize, 0);
  dylinker_command* dylinker = reinterpret_cast<dylinker_command*>(dylinkerCmd.data());
  dylinker->cmd              = LC_LOAD_DYLINKER;
  dylinker->cmdsize          = dylinkerCmdSize;
  dylinker->name.offset      = sizeof(dylinker_command);
  std::memcpy(dylinkerCmd.data() + sizeof(dylinker_command), dyldPath, strlen(dyldPath) + 1);

  output_fc.write(dylinkerCmd.data(), dylinkerCmd.size());

  if (kVerbose) {
    kConsoleOut << "Wrote LC_LOAD_DYLINKER: " << dyldPath << "\n";
  }

  // Write LC_MAIN entry point command (executables only)
  if (!kIsDylib) {
    entryCommand.cmd     = LC_MAIN;
    entryCommand.cmdsize = sizeof(entry_point_command);
    // entryoff is relative to __TEXT segment file offset
    entryCommand.entryoff = textFileOffset + entryCommand.entryoff;

    output_fc.write(reinterpret_cast<const Char*>(&entryCommand), sizeof(entryCommand));

    if (kVerbose) {
      kConsoleOut << "Wrote LC_MAIN, entryoff: 0x" << std::hex << entryCommand.entryoff << std::dec
                  << ", stacksize: " << entryCommand.stacksize << "\n";
    }
  }

  // Write LC_UUID command
  uuid_command uuidCmd{};
  uuidCmd.cmd     = LC_UUID;
  uuidCmd.cmdsize = sizeof(uuid_command);

  // Generate a random UUID (version 4)
  std::random_device           rd;
  std::mt19937                 gen(rd());
  uuids::uuid_random_generator uuidGen(gen);
  uuids::uuid                  generatedUuid = uuidGen();
  auto                         uuidBytes     = generatedUuid.as_bytes();
  std::memcpy(uuidCmd.uuid, uuidBytes.data(), 16);

  output_fc.write(reinterpret_cast<const Char*>(&uuidCmd), sizeof(uuidCmd));

  if (kVerbose) {
    kConsoleOut << "Wrote LC_UUID\n";
  }

  // Write LC_SYMTAB command
  symtab_command symtabCmd{};
  symtabCmd.cmd     = LC_SYMTAB;
  symtabCmd.cmdsize = sizeof(symtab_command);
  symtabCmd.symoff  = static_cast<UInt32>(symtabFileOffset);
  symtabCmd.nsyms   = static_cast<UInt32>(kSymbolTable.size());
  symtabCmd.stroff  = static_cast<UInt32>(strtabFileOffset);
  symtabCmd.strsize = static_cast<UInt32>(kStringTable.size());

  output_fc.write(reinterpret_cast<const Char*>(&symtabCmd), sizeof(symtabCmd));

  if (kVerbose) {
    kConsoleOut << "Wrote LC_SYMTAB, nsyms: " << symtabCmd.nsyms
                << ", strsize: " << symtabCmd.strsize << "\n";
  }

  // Write LC_DYSYMTAB command
  dysymtab_command dysymtabCmd{};
  std::memset(&dysymtabCmd, 0, sizeof(dysymtabCmd));
  dysymtabCmd.cmd     = LC_DYSYMTAB;
  dysymtabCmd.cmdsize = sizeof(dysymtab_command);

  // All symbols are local for now
  dysymtabCmd.ilocalsym = 0;
  dysymtabCmd.nlocalsym = static_cast<UInt32>(kSymbolTable.size());

  // External symbols start after locals
  dysymtabCmd.iextdefsym = static_cast<UInt32>(kSymbolTable.size());
  dysymtabCmd.nextdefsym = 0;

  // Undefined symbols
  dysymtabCmd.iundefsym = static_cast<UInt32>(kSymbolTable.size());
  dysymtabCmd.nundefsym = 0;

  output_fc.write(reinterpret_cast<const Char*>(&dysymtabCmd), sizeof(dysymtabCmd));

  if (kVerbose) {
    kConsoleOut << "Wrote LC_DYSYMTAB\n";
  }

  // Pad to text section offset
  UInt64 currentPos = output_fc.tellp();
  UInt64 padding    = textFileOffset - currentPos;

  if (padding > 0) {
    std::vector<Char> zeros(padding, 0);
    output_fc.write(zeros.data(), zeros.size());
  }

  // Write __text content
  for (auto& blob : kTextBytes) {
    output_fc.write(blob.mBlob.data(), blob.mBlob.size());
  }

  // Pad to data section offset
  currentPos = output_fc.tellp();
  padding    = dataFileOffset - currentPos;

  if (padding > 0) {
    std::vector<Char> zeros(padding, 0);
    output_fc.write(zeros.data(), zeros.size());
  }

  // Write __data content
  for (auto& blob : kDataBytes) {
    output_fc.write(blob.mBlob.data(), blob.mBlob.size());
  }

  // Pad to symbol table offset
  currentPos = output_fc.tellp();
  padding    = symtabFileOffset - currentPos;

  if (padding > 0) {
    std::vector<Char> zeros(padding, 0);
    output_fc.write(zeros.data(), zeros.size());
  }

  // Write symbol table (nlist_64 entries)
  for (auto& sym : kSymbolTable) {
    output_fc.write(reinterpret_cast<const Char*>(&sym), sizeof(nlist_64));
  }

  if (kVerbose) {
    kConsoleOut << "Wrote symbol table, " << kSymbolTable.size() << " entries\n";
  }

  // Write string table
  output_fc.write(kStringTable.data(), kStringTable.size());

  if (kVerbose) {
    kConsoleOut << "Wrote string table, " << kStringTable.size() << " bytes\n";
  }

  output_fc.flush();
  output_fc.close();

  if (kVerbose) {
    kConsoleOut << "Wrote Mach-O binary: " << kOutput << "\n";
  }

  return NECTAR_SUCCESS;
}

// Last rev - 2026
