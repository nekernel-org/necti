/* -------------------------------------------

  Copyright (C) 2024-2025 Amlal EL Mahrouss, all rights reserved

------------------------------------------- */

#pragma once

#include <CompilerKit/Defines.h>

// @brief AMD64 support.
// @file detail/X64.h

#define LC_ASM_OPCODE(__NAME, __OPCODE) {.fName = __NAME, .fOpcode = __OPCODE},

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
    LC_ASM_OPCODE("int", 0xCD) LC_ASM_OPCODE("into", 0xCE) LC_ASM_OPCODE("intd", 0xF1)
        LC_ASM_OPCODE("int3", 0xC3) LC_ASM_OPCODE("iret", 0xCF) LC_ASM_OPCODE("retf", 0xCB)
            LC_ASM_OPCODE("retn", 0xC3) LC_ASM_OPCODE("ret", 0xC3) LC_ASM_OPCODE("sti", 0xfb)
                LC_ASM_OPCODE("cli", 0xfa) LC_ASM_OPCODE("hlt", 0xf4) LC_ASM_OPCODE("nop", 0x90)
                    LC_ASM_OPCODE("mov", 0x48) LC_ASM_OPCODE("call", 0xFF)
                        LC_ASM_OPCODE("syscall", 0x0F) LC_ASM_OPCODE("xor", 0x48)};

#define kAsmRegisterLimit 16
