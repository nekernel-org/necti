/* -------------------------------------------

  Copyright (C) 2024-2025 Amlal EL Mahrouss, all rights reserved

------------------------------------------- */

#pragma once

#include <CompilerKit/Defines.h>
#include <vector>

// @brief Open64x0 support.
// @file impl/64x0.h

#define CK_ASM_OPCODE(__NAME, __OPCODE, __FUNCT3, __FUNCT7) \
  {.fName = __NAME, .fOpcode = __OPCODE, .fFunct3 = __FUNCT3, .fFunct7 = __FUNCT7},

#define kAsmImmediate 0x01
#define kAsmRegToReg 0x02
#define kAsmSyscall 0x03
#define kAsmJump 0x04
#define kAsmNoArgs 0x00

typedef char    e64k_character_t;
typedef uint8_t e64k_num_t;

struct CpuOpcode64x0 {
  const e64k_character_t fName[32];
  e64k_num_t             fOpcode;
  e64k_num_t             fFunct3;
  e64k_num_t             fFunct7;
};

inline std::vector<CpuOpcode64x0> kOpcodes64x0 = {
    CK_ASM_OPCODE("nop", 0b0000000, 0b0000000, kAsmNoArgs)  // no-operation.
    CK_ASM_OPCODE("np", 0b0000000, 0b0000000, kAsmNoArgs)   // no-operation.
    CK_ASM_OPCODE("jlr", 0b1110011, 0b0000111,
                  kAsmJump)  // jump to linked return register
    CK_ASM_OPCODE("jrl", 0b1110011, 0b0001111,
                  kAsmJump)  // jump from return register.
    CK_ASM_OPCODE("mv", 0b0100011, 0b101, kAsmRegToReg) CK_ASM_OPCODE(
        "bg", 0b1100111, 0b111, kAsmRegToReg) CK_ASM_OPCODE("bl", 0b1100111, 0b011, kAsmRegToReg)
        CK_ASM_OPCODE("beq", 0b1100111, 0b000, kAsmRegToReg)
            CK_ASM_OPCODE("bne", 0b1100111, 0b001, kAsmRegToReg)
                CK_ASM_OPCODE("bge", 0b1100111, 0b101, kAsmRegToReg)
                    CK_ASM_OPCODE("ble", 0b1100111, 0b100, kAsmRegToReg)
                        CK_ASM_OPCODE("stw", 0b0001111, 0b100, kAsmImmediate)
                            CK_ASM_OPCODE("ldw", 0b0001111, 0b100, kAsmImmediate)
                                CK_ASM_OPCODE("lda", 0b0001111, 0b101, kAsmImmediate)
                                    CK_ASM_OPCODE("sta", 0b0001111, 0b001, kAsmImmediate)
    // add/sub without carry flag
    CK_ASM_OPCODE("add", 0b0101011, 0b100, kAsmImmediate)
        CK_ASM_OPCODE("sub", 0b0101011, 0b101, kAsmImmediate)
    // add/sub with carry flag
    CK_ASM_OPCODE("addc", 0b0101011, 0b110, kAsmImmediate) CK_ASM_OPCODE(
        "subc", 0b0101011, 0b111, kAsmImmediate) CK_ASM_OPCODE("sc", 0b1110011, 0b00, kAsmSyscall)};

// \brief 64x0 register prefix
// example: r32, r0
// r32 -> sp
// r0 -> hw zero

#define kAsmFloatZeroRegister 0
#define kAsmZeroRegister 0

#define kAsmRegisterPrefix "r"
#define kAsmRegisterLimit 30
#define kAsmPcRegister 17
#define kAsmCrRegister 18
#define kAsmSpRegister 5

/* return address register */
#define kAsmRetRegister 19

/////////////////////////////////////////////////////////////////////////////

// SYSTEM CALL/JUMP ADDRESSING

// | OPCODE | FUNCT3 | FUNCT7 |              OFF                 |

// IMMEDIATE ADDRESSING

// | OPCODE | FUNCT3 | FUNCT7 | REG      |  OFF     |
// | OPCODE | FUNCT3 | FUNCT7 | REG      |  OFF     |       REG      |
// | OPCODE | FUNCT3 | FUNCT7 | REG      |  REG     |       OFF      |

// REG TO REG ADDRESSING

// | OPCODE | FUNCT3 | FUNCT7 | REG      |  REG2    |

////////////////////////////////

// LOAD/CALL INTERRUPTS

// SET A HANDLER IN ADDRESS:
// DISABLE INTERRUPTS
// PROCESS INTERRUPT
// ENABLE INTERRUPTS

////////////////////////////////
