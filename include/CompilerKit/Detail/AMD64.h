// Copyright 2024-2025, Amlal El Mahrouss (amlal@nekernel.org)
// Licensed under the Apache License, Version 2.0 (See accompanying
// file LICENSE or copy at http://www.apache.org/licenses/LICENSE-2.0)
// Official repository: https://github.com/nekernel-org/nectar

#ifndef NECTAR_COMPILERKIT_DETAIL_AMD64_H
#define NECTAR_COMPILERKIT_DETAIL_AMD64_H

#include <CompilerKit/Detail/Config.h>
#include <vector>

/// @brief AMD64 support.
/// @file Detail/AMD64.h

#define CK_ASM_OPCODE(__NAME, __OPCODE) {.fName = __NAME, .fOpcode = __OPCODE},

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

/// @brief Base opcodes for push/pop instructions
#define kAsmPushOpcode 0x50
#define kAsmPopOpcode 0x58

inline std::vector<CpuOpcodeAMD64> kOpcodesAMD64 = {
    CK_ASM_OPCODE("int", 0xCD) CK_ASM_OPCODE("into", 0xCE) CK_ASM_OPCODE("intd", 0xF1)
        CK_ASM_OPCODE("int3", 0xC3) CK_ASM_OPCODE("iret", 0xCF) CK_ASM_OPCODE("retf", 0xCB)
            CK_ASM_OPCODE("retn", 0xC3) CK_ASM_OPCODE("ret", 0xC3) CK_ASM_OPCODE("sti", 0xfb)
                CK_ASM_OPCODE("cli", 0xfa) CK_ASM_OPCODE("hlt", 0xf4) CK_ASM_OPCODE("nop", 0x90)
                    CK_ASM_OPCODE("mov", 0x48) CK_ASM_OPCODE("call", 0xFF)
                        CK_ASM_OPCODE("syscall", 0x0F) CK_ASM_OPCODE("xor", 0x48) CK_ASM_OPCODE(
                            "push", kAsmPushOpcode) CK_ASM_OPCODE("pop", kAsmPopOpcode)};

#define kAsmRegisterLimit 16

#endif  // NECTAR_COMPILERKIT_DETAIL_AMD64_H
