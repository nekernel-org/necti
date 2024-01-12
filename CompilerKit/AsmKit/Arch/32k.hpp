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

// @brief 32x0 support.
// @file Arch/64k.hpp

#define kAsmOpcodeDecl(__NAME, __OPCODE, __FUNCT3, __FUNCT7) \
    { .fName = __NAME, .fOpcode = __OPCODE, .fFunct3 = __FUNCT3, .fFunct7 = __FUNCT7 },



// placeholder for funct7/funct7-rs2
#define kAsmImmediate 0x01
#define kAsmSyscall 0x02
#define kAsmJump 0x03
#define kAsmNoArgs 0x04

#define kAsmByte  0
#define kAsmHWord 1
#define kAsmWord  2

struct CpuCode32x0
{
    const char fName[16];
    char fOpcode;
    char fSize;
    char fFunct3;
    char fFunct7;
};

#define kAsmWordStr  ".word"
#define kAsmHWordStr ".half"
#define kAsmByteStr  ".bbyte"

inline std::vector<CpuCode32x0> kOpcodes32x0 = {
        kAsmOpcodeDecl("nop", 0b0100011, 0b0000000, kAsmNoArgs) // nothing to do.
        kAsmOpcodeDecl("jmp", 0b1110011, 0b0000011, kAsmJump) // jump to branch
        kAsmOpcodeDecl("move", 0b0100011, 0b101, kAsmImmediate)
        kAsmOpcodeDecl("push", 0b0111011, 0b0, kAsmImmediate) // push to sp
        kAsmOpcodeDecl("pop", 0b0111011, 0b1, kAsmImmediate) // pop from sp.
        kAsmOpcodeDecl("int", 0b0111111, 0b0, kAsmSyscall) // raise interrupt
};

// \brief 64x0 register prefix
// example: r32, r0
// r32 -> sp
// r0 -> hw zero

#define kAsmRegisterPrefix "r"
#define kAsmRegisterLimit  16
#define kAsmPcRegister     17
#define kAsmCrRegister     18
#define kAsmSpRegister     5

/* return address register */
#define kAsmRetRegister    19

/////////////////////////////////////////////////////////////////////////////

// SYSTEM CALL ADDRESSING

// | OPCODE | FUNCT3 | FUNCT7 |              OFF                 |

// IMMEDIATE ADDRESSING

// | OPCODE | FUNCT3 | FUNCT7 | REG      |  OFF     |
// | OPCODE | FUNCT3 | FUNCT7 | REG      |  OFF     |       REG      |
// | OPCODE | FUNCT3 | FUNCT7 | REG      |  REG     |       OFF      |

// REG TO REG ADDRESSING

// | OPCODE | FUNCT3 | FUNCT7 | REG      |  REG2    |

////////////////////////////////

// LOAD/CALL INTERRUPTS

// SET A HANDLER IN ADDRESS: TODO: find one
// DISABLE INTERRUPTS
// PROCESS INTERRUPT
// ENABLE INTERRUPTS

////////////////////////////////