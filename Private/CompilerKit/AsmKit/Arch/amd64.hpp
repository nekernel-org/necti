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

// @brief 64x0 support.
// @file Arch/64x0.hpp

#define kAsmOpcodeDecl(__NAME, __OPCODE) {.fName = __NAME, .fOpcode = __OPCODE},

typedef char e64k_character_t;
typedef uint8_t e64_byte_t;
typedef uint16_t e64_hword_t;
typedef uint32_t e64k_word_t;

struct CpuCodeAMD64 {
  std::string fName;
  e64_byte_t fPrefixBytes[4];
  e64_hword_t fOpcode;
  e64_hword_t fModReg;
  e64k_word_t fDisplacment;
  e64k_word_t fImmediate;
};

/// these two are edge cases
#define kAsmIntOpcode 0xCC
#define kasmIntOpcodeAlt 0xCD

#define kAsmJumpOpcode 0x0F80
#define kJumpLimit 30
#define kJumpLimitStandard 0xE3
#define kJumpLimitStandardLimit 0xEB

inline std::vector<CpuCodeAMD64> kOpcodesAMD64 = {
    kAsmOpcodeDecl("int", 0xCD) kAsmOpcodeDecl("into", 0xCE) kAsmOpcodeDecl(
        "intd", 0xF1) kAsmOpcodeDecl("int3", 0xC3)

        kAsmOpcodeDecl("iret", 0xCF) kAsmOpcodeDecl("retf", 0xCB)
            kAsmOpcodeDecl("retn", 0xC3) kAsmOpcodeDecl("sti", 0xfb)
                kAsmOpcodeDecl("cli", 0xfa)

                    kAsmOpcodeDecl("nop", 0x90)

                        kAsmOpcodeDecl("mov eax", 0xb8) kAsmOpcodeDecl(
                            "mov ecx", 0xb9) kAsmOpcodeDecl("mov edx", 0xba)
                            kAsmOpcodeDecl("mov ebx", 0xbb) kAsmOpcodeDecl(
                                "mov esp", 0xbc) kAsmOpcodeDecl("mov ebp", 0xbd)
                                kAsmOpcodeDecl("mov esi", 0xbe)

                                    kAsmOpcodeDecl("jmp", 0xE9)
                                        kAsmOpcodeDecl("call", 0xE9)

                                            kAsmOpcodeDecl("mov", 0x00)};

// \brief 64x0 register prefix
// example: r32, r0
// r32 -> sp
// r0 -> hw zero

#define kAsmFloatZeroRegister -1
#define kAsmZeroRegister -1

#define kAsmRegisterPrefix "r"
#define kAsmRegisterLimit 16
#define kAsmPcRegister 8
#define kAsmCrRegister -1
#define kAsmSpRegister 9

/* return address register */
#define kAsmRetRegister 0
