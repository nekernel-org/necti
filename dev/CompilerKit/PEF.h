/* -------------------------------------------

  Copyright (C) 2024-2025 Amlal EL Mahrouss, all rights reserved

------------------------------------------- */

#pragma once

#include <CompilerKit/Defines.h>

// @file PEF.h
// @brief Preferred Executable Format

#define kPefMagic "Open"
#define kPefMagicFat "nepO"

#define kPefExt ".exec"
#define kPefDylibExt ".dylib"
#define kPefLibExt ".lib"
#define kPefObjectExt ".obj"
#define kPefDebugExt ".dbg"
#define kPefDriverExt ".sys"

#define kPefZero128 ".zero128"
#define kPefCode128 ".code128"
#define kPefData128 ".data128"

#define kPefZero64 ".zero64"
#define kPefCode64 ".code64"
#define kPefData64 ".data64"

/* @note counting the \0 at the end */
#define kPefMagicLen (5)

#define kPefVersion (0x0500)
#define kPefNameLen (255)

#define kPefBaseOrigin (0x40000000)

/* @note this doesn't have to be __ImageStart only, any C initialization stub will do. */
#define kPefStart "__ImageStart"

namespace CompilerKit {
/* @brief Architecture type.  */
enum {
  kPefArchIntel86S,
  kPefArchAMD64,
  kPefArchRISCV,
  kPefArch64000, /* 64x0 RISC architecture. */
  kPefArch32000,
  kPefArchPowerPC, /* 64-bit POWER architecture. */
  kPefArchARM64,
  kPefArchCount   = (kPefArchARM64 - kPefArchIntel86S) + 1,
  kPefArchInvalid = 0xFF,
};

/* @brief Architecture vendor. */
enum {
  kPefSubArchGeneric = 0,
  kPefSubArchAMD     = 200,
  kPefSubArchIntel,
  kPefSubArchARM,
  kPefSubArchIBM,
};

enum {
  kPefKindInvalid = 0,
  kPefKindExec    = 1, /* .exec */
  kPefKindDylib   = 2, /* .dylib */
  kPefKindObject  = 4, /* .obj */
  kPefKindDebug   = 5, /* .dbg */
  kPefKindDriver  = 6,
  kPefKindCount,
};

/* PEF container information  */
typedef struct PEFContainer final {
  Char     Magic[kPefMagicLen];
  UInt32   Linker; /* Linker used to link executable */
  UInt32   Version;
  UInt32   Kind;
  UInt32   Abi;
  UInt32   Cpu;
  UInt32   SubCpu;   /* Cpu specific information */
  UIntPtr  Start;    /* Origin of code */
  SizeType HdrSz;    /* Size of header */
  SizeType Count;    /* container header count */
  UInt32   Checksum; /* Whole binary checksum */
} PACKED PEFContainer, *PEFContainerPtr;

/* First PEFCommandHeader starts after PEFContainer */
/* Last container is __exec_end */

/* PEF executable section and commands. */

/* @brief Command Header, a la Mach-O, designed with FAT binaries and virtual memory in mind.  */
typedef struct PEFCommandHeader final {
  Char     Name[kPefNameLen]; /* container name */
  UInt32   Cpu;               /* container cpu */
  UInt32   SubCpu;            /* container sub-cpu */
  UInt32   Flags;             /* container flags */
  UInt16   Kind;              /* container kind */
  UIntPtr  Offset;            /* File offset */
  SizeType OffsetSize;
  UIntPtr  VirtualAddress; /* Virtual Address */
  SizeType VirtualSize;    /* Virtual Size */
} PACKED PEFCommandHeader, *PEFCommandHeaderPtr;

enum {
  kPefInvalid  = 0x0,
  kPefCode     = 0xC,
  kPefData     = 0xD,
  kPefZero     = 0xE,
  kPefLinkerID = 0x1,
  kPefCount    = 4,
};
}  // namespace CompilerKit

inline std::ofstream& operator<<(std::ofstream& fp, CompilerKit::PEFContainer& container) {
  fp.write((char*) &container, sizeof(CompilerKit::PEFContainer));
  return fp;
}

inline std::ofstream& operator<<(std::ofstream& fp, CompilerKit::PEFCommandHeader& container) {
  fp.write((char*) &container, sizeof(CompilerKit::PEFCommandHeader));
  return fp;
}

inline std::ifstream& operator>>(std::ifstream& fp, CompilerKit::PEFContainer& container) {
  fp.read((char*) &container, sizeof(CompilerKit::PEFContainer));
  return fp;
}

inline std::ifstream& operator>>(std::ifstream& fp, CompilerKit::PEFCommandHeader& container) {
  fp.read((char*) &container, sizeof(CompilerKit::PEFCommandHeader));
  return fp;
}
