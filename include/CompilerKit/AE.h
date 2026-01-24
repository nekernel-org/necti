// Copyright 2024-2025, Amlal El Mahrouss (amlal@nekernel.org)
// Licensed under the Apache License, Version 2.0 (See accompanying
// file LICENSE or copy at http://www.apache.org/licenses/LICENSE-2.0)
// Official repository: https://github.com/nekernel-org/nectar

#ifndef NECTAR_COMPILERKIT_AE_H
#define NECTAR_COMPILERKIT_AE_H

#include <CompilerKit/Detail/Config.h>
#include <fstream>

#define kAEIdentVersion (0x0123)

#define kAEMag0 'A'
#define kAEMag1 'E'
#define kAEMag2 'O'

#define kAESymbolLen (256)
#define kAEPad (8)
#define kAEMagLen (3)
#define kAENullType (0x00)

/// @author Amlal El Mahrouss

/// @brief
// Advanced Executable File Format for ld64.
// Reloctable by offset is the default strategy.
// You can also relocate at runtime but that's up to the operating system loader.

namespace CompilerKit {

// @brief Advanced Executable Header
// One thing to keep in mind.
// This object format, is reloctable.
typedef struct AEHeader final {
  Char     fMagic[kAEMagLen] = {};
  UInt16   fVersion{kAEIdentVersion};
  Char     fArch{};
  Char     fSubArch{};
  SizeType fCount{};
  Char     fSize{};
  SizeType fStartCode{};
  SizeType fCodeSize{};
  Char     fPad[kAEPad] = {};
} PACKED AEHeader, *AEHeaderPtr;

// @brief Advanced Executable Record.
// Could be data, code or bss.
// fKind must be filled with PEF fields.

typedef struct AERecordHeader final {
  Char     fName[kAESymbolLen];
  SizeType fKind;
  SizeType fSize;
  SizeType fFlags;
  UIntPtr  fOffset;
  Char     fPad[kAEPad];
} PACKED AERecordHeader, *AERecordHeaderPtr;

enum {
  kKindImportSymbol = 0x356,
  kKindExportSymbol = 0x237,
};

enum {
  kKindRelocationByOffset  = 0x23f,
  kKindRelocationAtRuntime = 0x34f,
};
}  // namespace CompilerKit

// provide operator<< for AE

namespace Operators {
inline std::ofstream& operator<<(std::ofstream& fp, CompilerKit::AEHeader& container) {
  fp.write((char*) &container, sizeof(CompilerKit::AEHeader));
  return fp;
}

inline std::ofstream& operator<<(std::ofstream& fp, CompilerKit::AERecordHeader& container) {
  fp.write((char*) &container, sizeof(CompilerKit::AERecordHeader));
  return fp;
}

inline std::ifstream& operator>>(std::ifstream& fp, CompilerKit::AEHeader& container) {
  fp.read((char*) &container, sizeof(CompilerKit::AEHeader));
  return fp;
}

inline std::ifstream& operator>>(std::ifstream& fp, CompilerKit::AERecordHeader& container) {
  fp.read((char*) &container, sizeof(CompilerKit::AERecordHeader));
  return fp;
}
}  // namespace Operators

#ifndef __CK_NO_USING_OPERATORS__
using namespace Operators;
#endif

namespace CompilerKit::Utils {
/**
 * @brief AE Reader protocol
 *
 */
class AEReadableProtocol final {
 public:
  std::ifstream fFilePtr;

 public:
  explicit AEReadableProtocol() = default;
  ~AEReadableProtocol()         = default;

  NECTAR_COPY_DELETE(AEReadableProtocol)

  /**
   * @brief Reads the AE Record headers.
   *
   * @param raw the containing buffer
   * @param sz it's size (1 = one AERecordHeader, 2 two AERecordHeader(s))
   * @return AERecordHeaderPtr
   */
  AERecordHeaderPtr Read(char* raw, std::size_t sz) {
    if (!raw) return nullptr;
    return this->Read_<AERecordHeader>(raw, sz * sizeof(AERecordHeader));
  }

 private:
  /**
   * @brief Implementation of Read for raw classes.
   *
   * @tparam TypeClass The class to read.
   * @param raw the buffer
   * @param sz the size
   * @return TypeClass* the returning class.
   */
  template <typename TypeClass>
  TypeClass* Read_(char* raw, std::size_t sz) {
    fFilePtr.read(raw, std::streamsize(sz));
    return reinterpret_cast<TypeClass*>(raw);
  }
};

}  // namespace CompilerKit::Utils

#endif /* ifndef NECTAR_COMPILERKIT_AE_H */
