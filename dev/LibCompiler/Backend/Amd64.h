/* -------------------------------------------

  Copyright (C) 2024-2025 Amlal EL Mahrous, all rights reserved

------------------------------------------- */

#pragma once

#include <LibCompiler/Defines.h>

// @brief AMD64 support.
// @file Backend/Amd64.h

#define kAsmOpcodeDecl(__NAME, __OPCODE) {.fName = __NAME, .fOpcode = __OPCODE},

typedef char     i64_character_t;
typedef uint8_t  i64_byte_t;
typedef uint16_t i64_hword_t;
typedef uint32_t i64_word_t;

#define kAsmRegisterPrefix "r"

struct CpuOpcodeAMD64 {
  std::string fName;
  i64_byte_t  fPrefixBytes[4];
  i64_hword_t fOpcode;
  i64_hword_t fModReg;
  i64_word_t  fDisplacment;
  i64_word_t  fImmediate;
};

/// these two are edge cases
#define kAsmIntOpcode 0xCC
#define kasmIntOpcodeAlt 0xCD

#define kAsmJumpOpcode 0x0F80
#define kJumpLimit 30
#define kJumpLimitStandard 0xE3
#define kJumpLimitStandardLimit 0xEB

inline std::vector<CpuOpcodeAMD64> kOpcodesAMD64 = {
    kAsmOpcodeDecl("int", 0xCD) kAsmOpcodeDecl("into", 0xCE) kAsmOpcodeDecl("intd", 0xF1)
        kAsmOpcodeDecl("int3", 0xC3) kAsmOpcodeDecl("iret", 0xCF) kAsmOpcodeDecl("retf", 0xCB)
            kAsmOpcodeDecl("retn", 0xC3) kAsmOpcodeDecl("ret", 0xC3) kAsmOpcodeDecl("sti", 0xfb)
                kAsmOpcodeDecl("cli", 0xfa) kAsmOpcodeDecl("hlt", 0xf4) kAsmOpcodeDecl("nop", 0x90)
                    kAsmOpcodeDecl("mov", 0x48) kAsmOpcodeDecl("call", 0xFF)};

#define kAsmRegisterLimit 16
