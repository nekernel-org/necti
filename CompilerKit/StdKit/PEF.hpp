/*
 *	========================================================
 *
 *	MPCC
 * 	Copyright Mahrouss Logic, all rights reserved.
 *
 * 	========================================================
 */

#pragma once

#include <CompilerKit/Defines.hpp>

// @file PEF.hpp
// @brief Preferred Executable Format

#define kPefMagic    "PEF"
#define kPefMagicFat "FEP"

#define kPefExt ".out"
#define kPefDylibExt ".lib"
#define kPefObjectExt ".o"
#define kPefDebugExt ".dbg"

#define kPefMagicLen 3

#define kPefVersion 2
#define kPefNameLen 64

#define kPefBaseOrigin 0

namespace CompilerKit
{
    enum
    {
        kPefArchIntel86S = 100,
        kPefArchAMD64,
        kPefArchRISCV,
	    kPefArch64000, /* Advanced RISC architecture. */
        kPefArch32000,
        kPefArchInvalid = 0xFF,
    };

    enum
    {
        kPefKindExec = 1, /* .o/.pef/<none>  */
        kPefKindSharedObject = 2, /* .lib */
        kPefKindObject = 4, /* .obj */
        kPefKindDwarf = 5, /* .dsym */
    };

    /* PEF container */
    typedef struct PEFContainer final
    {
        CharType Magic[kPefMagicLen];
        UInt32 Linker;
        UInt32 Version;
        UInt32 Kind;
        UInt32 Abi;
        UInt32 Cpu;
        UInt32 SubCpu; /* Cpu specific information */
        UIntPtr Start; /* Origin of code */
        SizeType HdrSz; /* Size of header */
        SizeType Count; /* container header count */
    } __attribute__((packed)) PEFContainer;

    /* First PEFCommandHeader starts after PEFContainer */
    /* Last container is __exec_end */

    /* PEF executable section and commands. */

    typedef struct PEFCommandHeader final
    {
        CharType Name[kPefNameLen]; /* container name */
        UInt32 Flags; /* container flags */
        UInt16 Kind; /* container kind */
        UIntPtr Offset; /* file offset */
        SizeType Size; /* file size */
    } __attribute__((packed)) PEFCommandHeader;

    enum
    {
        kPefCode = 0xC,
        kPefData = 0xD,
        kPefZero = 0xE,
	    kPefLinkerID = 0x1,
    };
}

inline std::ofstream& operator<<(std::ofstream& fp, CompilerKit::PEFContainer& container)
{
    fp.write((char*)&container, sizeof(CompilerKit::PEFContainer));
    return fp;
}

inline std::ofstream& operator<<(std::ofstream& fp, CompilerKit::PEFCommandHeader& container)
{
    fp.write((char*)&container, sizeof(CompilerKit::PEFCommandHeader));
    return fp;
}
