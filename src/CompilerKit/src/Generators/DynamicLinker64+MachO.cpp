// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026, Amlal El Mahrouss (amlal@nekernel.org)
// Licensed under the Apache License, Version 2.0 (See accompanying
// file LICENSE or copy at http://www.apache.org/licenses/LICENSE-2.0)
// Official repository: https://github.com/ne-foss-org/nectar

/// @author Amlal El Mahrouss (amlal@nekernel.org)
/// @brief NeKernel.org 64-bit Mach-O Linker.
/// @version Last Rev: 2026
/// @note Outputs Mach-O executables with __TEXT and __DATA segments.

#ifdef CK_USE_MACHO_LINKER

#include <CompilerKit/AE.h>
#include <CompilerKit/CodeGenerator.h>
#include <CompilerKit/Detail/Config.h>
#include <CompilerKit/ErrorID.h>
#include <CompilerKit/MachO.h>
#include <CompilerKit/PEF.h>
#include <CompilerKit/UUID.h>
#include <CompilerKit/Utils/Compiler.h>

#define kLatestOSX (15)

#define kMachODefaultEntrypoint "_main"
#define kMachODefaultOutput {"a" kMachOExt}

#define kLinkerVersionStr "Nectar 64-Bit Linker (OS X Mach-O)"

#define kLinkerSplash() kStdOut << kLinkerVersionStr << kStdEndl

#define kConsoleOut        \
  (std::cout << kRed \
             << "ld: "  \
             << kReset)

static CompilerKit::STLString kOutput              = kMachODefaultOutput;
static cpu_type_t             kCpuType             = CPU_TYPE_X86_64;
static cpu_subtype_t          kCpuSubType          = CPU_SUBTYPE_X86_64_ALL;
static bool                   kFatBinaryEnable     = false;
static bool                   kStartFound          = false;
static bool                   kDuplicateSymbols    = false;
static bool                   kIsDylib             = false;
static Int64                  kMachODefaultStackSz = 0;

static CompilerKit::STLString kLinkerStart = kMachODefaultEntrypoint;

/* object code and list. */
static std::vector<CompilerKit::STLString>    kObjectList;
static std::vector<CompilerKit::Detail::Blob> kTextBytes;
static std::vector<CompilerKit::Detail::Blob> kDataBytes;

/* @brief symbol tables */
static std::vector<nlist_64>                    kSymbolTable;
static std::vector<char>                        kStringTable;
static std::map<CompilerKit::STLString, UInt64> kSymbolOffsets;

/// @brief Structure to hold section information from AE records
struct SectionInfo {
  CompilerKit::STLString name;
  UInt32                 kind;
  std::vector<char>      bytes;
  UInt64                 address;
  UInt64                 size;
};

using SectionInfoVec = std::vector<SectionInfo>;

/// @brief Extract clean symbol name from AE record name
/// AE format: ".code64$symbolname" or "symbolname.code64"
static CompilerKit::STLString macho_extract_symbol_name(const CompilerKit::STLString& aeName) {
  CompilerKit::STLString name = aeName;

  // Remove section prefixes/suffixes
  const char* sections[] = {".code64", ".data64", ".zero64", "$"};

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
static UInt32 macho_add_symbol(const CompilerKit::STLString& name, uint8_t type, uint8_t sect,
                               UInt64 value) {
  // Add name to string table (offset 0 is reserved for empty string)
  if (kStringTable.empty()) {
    kStringTable.push_back('\0');  // First byte is null
  }

  UInt32 strOffset = static_cast<UInt32>(kStringTable.size());

  for (const char& c : name) {
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

/// @brief Nectar 64-bit Mach-O Linker.
/// @note This linker outputs Mach-O executables for macOS/iOS.
NECTAR_MODULE(DynamicLinker64MachO) {
  CompilerKit::install_signal(SIGSEGV, CompilerKit::Detail::drvi_crash_handler);

  /**
   * @brief parse flags and trigger options.
   */
  for (size_t linker_arg{1}; linker_arg < argc; ++linker_arg) {
    if (std::strcmp(argv[linker_arg], "--help") == 0 ||
   std::strcmp(argv[linker_arg], "-h") == 0) {
      kLinkerSplash();

      kConsoleOut << "--version: Show linker version.\n";
      kConsoleOut << "--help: Show linker help.\n";
      kConsoleOut << "--verbose: Enable linker trace.\n";
      kConsoleOut << "-fdylib: Output as a Dynamic Library.\n";
      kConsoleOut << "-ffat: Output as a FAT binary.\n";
      kConsoleOut << "-famd64: Output as an x86_64 Mach-O.\n";
      kConsoleOut << "-farm64: Output as an ARM64 Mach-O.\n";
      kConsoleOut << "--output: Select the output file name.\n";
      kConsoleOut << "-fstart: Specify entry point symbol.\n";

      return NECTAR_SUCCESS;
    } else if (std::strcmp(argv[linker_arg], "--version") == 0 ||
  std::strcmp(argv[linker_arg], "-v") == 0) {
      kLinkerSplash();

      return NECTAR_SUCCESS;
    } else if (std::strcmp(argv[linker_arg], "-ffat") == 0) {
      kFatBinaryEnable = true;

      continue;
    } else if (std::strcmp(argv[linker_arg], "-famd64") == 0) {
      kCpuType    = CPU_TYPE_X86_64;
      kCpuSubType = CPU_SUBTYPE_X86_64_ALL;

      continue;
    } else if (std::strcmp(argv[linker_arg], "-farm64") == 0) {
      kCpuType    = CPU_TYPE_ARM64;
      kCpuSubType = CPU_SUBTYPE_ARM64_ALL;

      continue;
    } else if (std::strcmp(argv[linker_arg], "-fstart") == 0) {
      if (argv[linker_arg + 1] == nullptr || argv[linker_arg + 1][0] == '-') continue;

      kLinkerStart = argv[linker_arg + 1];
      linker_arg += 1;

      continue;
    } else if (std::strcmp(argv[linker_arg], "--verbose") == 0) {
      kVerbose = true;

      continue;
    } else if (std::strcmp(argv[linker_arg], "-fdylib") == 0) {
      kIsDylib = true;

      if (kOutput.find(kMachOExt) != CompilerKit::STLString::npos) {
        kOutput.erase(kOutput.find(kMachOExt), strlen(kMachOExt));
        kOutput += kMachODylibExt;
      }

      continue;
    } else if (std::strcmp(argv[linker_arg], "--output") == 0) {
      if ((linker_arg + 1) > argc) continue;

      kOutput = argv[linker_arg + 1];
      ++linker_arg;

      continue;
    } else {
      if (argv[linker_arg][0] == '-') {
        kConsoleOut << "unknown option: " << argv[linker_arg] << "\n";
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

  SectionInfoVec                         sections;
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

      char* raw_ae_records = new char[cnt * sizeof(CompilerKit::AERecordHeader)];

      if (!raw_ae_records) {
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
        CompilerKit::STLString symbolName = macho_extract_symbol_name(section.name);

        if (!symbolName.empty()) {
          // Determine section number (1 = __text, 2 = __data)
          uint8_t sectNum = 0;
          if (section.kind & CompilerKit::kPefCode) {
            sectNum = 1;  // __text section
          } else if (section.kind & CompilerKit::kPefData) {
            sectNum = 2;  // __data section
          } else if (section.kind & CompilerKit::kPefZero) {
            sectNum = 3;  // __bss section
          }

          // N_EXT = external, N_SECT = defined in section
          uint8_t symType = N_EXT | N_SECT;

          macho_add_symbol(symbolName, symType, sectNum, ae_records[ae_record_index].fOffset);
        }

        sections.push_back(section);
      }

      // Look up entry point from symbol table
      auto entryIt = kSymbolOffsets.find(kLinkerStart);

      if (entryIt != kSymbolOffsets.end()) {
        entryCommand.entryoff = entryIt->second;
        kStartFound           = true;
      }

      delete[] raw_ae_records;
      raw_ae_records = nullptr;

      // Read the actual code bytes
      std::vector<char> bytes;
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
    kConsoleOut << "undefined entrypoint " << kLinkerStart << " for executable: " << kOutput
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
  UInt32 dylinkerCmdSize = (13 + 1 + 7) & ~7;           // "/usr/lib/dyld" + padding to 8-byte align

  sizeOfCmds = pageZeroSize + textSegCmdSize + dataSegCmdSize + buildCmdSize + uuidCmdSize +
               symtabCmdSize + dysymtabCmdSize + linkeditCmdSize;

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

  output_fc.write(reinterpret_cast<const char*>(&header), sizeof(header));

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

  output_fc.write(reinterpret_cast<const char*>(&pageZeroSegment), sizeof(pageZeroSegment));

  build_version_command build = {.cmd      = LC_BUILD_VERSION,
                                 .cmdsize  = sizeof(build_version_command),
                                 .platform = PLATFORM_MACOS,
                                 .minos    = (kLatestOSX << 16),  // macOS 11.0
                                 .sdk      = (kLatestOSX << 16),  // macOS 11.0
                                 .ntools   = 0};

  output_fc.write(reinterpret_cast<const char*>(&build), sizeof(build));

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

  output_fc.write(reinterpret_cast<const char*>(&textSegment), sizeof(textSegment));

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

  output_fc.write(reinterpret_cast<const char*>(&textSection), sizeof(textSection));

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
    output_fc.write(reinterpret_cast<const char*>(&dataSegment), sizeof(dataSegment));

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
    output_fc.write(reinterpret_cast<const char*>(&dataSection), sizeof(dataSection));

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

  output_fc.write(reinterpret_cast<const char*>(&linkeditSegment), sizeof(linkeditSegment));

  // Write LC_LOAD_DYLINKER command
  constexpr const char* dyldPath = "/usr/lib/dyld";
  std::vector<char>     dylinkerCmd(dylinkerCmdSize, 0);
  dylinker_command*     dylinker = reinterpret_cast<dylinker_command*>(dylinkerCmd.data());
  dylinker->cmd                  = LC_LOAD_DYLINKER;
  dylinker->cmdsize              = dylinkerCmdSize;
  dylinker->name.offset          = sizeof(dylinker_command);
  std::memcpy(dylinkerCmd.data() + sizeof(dylinker_command), dyldPath, strlen(dyldPath) + 1);

  output_fc.write(dylinkerCmd.data(), dylinkerCmd.size());

  // Write LC_MAIN entry point command (executables only)
  if (!kIsDylib) {
    entryCommand.cmd     = LC_MAIN;
    entryCommand.cmdsize = sizeof(entry_point_command);
    // entryoff is relative to __TEXT segment file offset
    entryCommand.entryoff = textFileOffset + entryCommand.entryoff;

    output_fc.write(reinterpret_cast<const char*>(&entryCommand), sizeof(entryCommand));
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

  output_fc.write(reinterpret_cast<const char*>(&uuidCmd), sizeof(uuidCmd));

  // Write LC_SYMTAB command
  symtab_command symtabCmd{};
  symtabCmd.cmd     = LC_SYMTAB;
  symtabCmd.cmdsize = sizeof(symtab_command);
  symtabCmd.symoff  = static_cast<UInt32>(symtabFileOffset);
  symtabCmd.nsyms   = static_cast<UInt32>(kSymbolTable.size());
  symtabCmd.stroff  = static_cast<UInt32>(strtabFileOffset);
  symtabCmd.strsize = static_cast<UInt32>(kStringTable.size());

  output_fc.write(reinterpret_cast<const char*>(&symtabCmd), sizeof(symtabCmd));

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

  output_fc.write(reinterpret_cast<const char*>(&dysymtabCmd), sizeof(dysymtabCmd));

  // Pad to text section offset
  UInt64 currentPos = output_fc.tellp();
  UInt64 padding    = textFileOffset - currentPos;

  if (padding > 0) {
    std::vector<char> zeros(padding, 0);
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
    std::vector<char> zeros(padding, 0);
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
    std::vector<char> zeros(padding, 0);
    output_fc.write(zeros.data(), zeros.size());
  }

  // Write symbol table (nlist_64 entries)
  for (auto& sym : kSymbolTable) {
    output_fc.write(reinterpret_cast<const char*>(&sym), sizeof(nlist_64));
  }

  // Write string table
  output_fc.write(kStringTable.data(), kStringTable.size());

  output_fc.flush();

  return NECTAR_SUCCESS;
}

// Last rev - 2026

#endif  // ifdef CK_USE_MACHO_LINKER
