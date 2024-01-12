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
// @file Arch/64k.hpp

#define kAsmOpcodeDecl(__NAME, __OPCODE, __FUNCT3, __FUNCT7) \
    { .fName = __NAME, .fOpcode = __OPCODE, .fFunct3 = __FUNCT3, .fFunct7 = __FUNCT7 },


// placeholder for funct7/funct7-rs2
#define kAsmImmediate 0x01
#define kAsmRegToReg 0x02
#define kAsmSyscall 0x03
#define kAsmJump 0x04
#define kAsmNoArgs 0x05

struct CpuCode64x0
{
    const char fName[16];
    char fOpcode;
    char fFunct3;
    char fFunct7;
};

inline std::vector<CpuCode64x0> kOpcodes64x0 = {
        kAsmOpcodeDecl("np", 0b0100011, 0b0000000, kAsmNoArgs) // no-operation.
        kAsmOpcodeDecl("jb", 0b1110011, 0b0000011, kAsmJump) // jump to branch
        kAsmOpcodeDecl("jlr", 0b1110011, 0b0000111, kAsmJump) // jump to linked return register
        kAsmOpcodeDecl("jrl", 0b1110011, 0b0001111, kAsmJump) // jump from return register.
        kAsmOpcodeDecl("mv", 0b0100011, 0b101, kAsmRegToReg)
        kAsmOpcodeDecl("bg", 0b1100111, 0b111, kAsmRegToReg)
        kAsmOpcodeDecl("bl", 0b1100111, 0b011, kAsmRegToReg)
        kAsmOpcodeDecl("beq", 0b1100111, 0b000, kAsmRegToReg)
        kAsmOpcodeDecl("bne", 0b1100111, 0b001, kAsmRegToReg)
        kAsmOpcodeDecl("bge", 0b1100111, 0b101, kAsmRegToReg)
        kAsmOpcodeDecl("ble", 0b1100111, 0b100, kAsmRegToReg)
        kAsmOpcodeDecl("stw", 0b0001111, 0b100, kAsmImmediate)
        kAsmOpcodeDecl("ldw", 0b0001111, 0b100, kAsmImmediate)
        kAsmOpcodeDecl("lda", 0b0001111, 0b101, kAsmImmediate)
        kAsmOpcodeDecl("sta", 0b0001111, 0b001, kAsmImmediate)
        kAsmOpcodeDecl("add", 0b0101011, 0b100, kAsmImmediate)
        kAsmOpcodeDecl("dec", 0b0101011, 0b101, kAsmImmediate)
        kAsmOpcodeDecl("int", 0b1110011, 0b00, kAsmSyscall)
        kAsmOpcodeDecl("syscall", 0b1110011, 0b00, kAsmSyscall)
        kAsmOpcodeDecl("pha", 0b1110011, 0b00, kAsmNoArgs)
        kAsmOpcodeDecl("pla", 0b1110011, 0b01, kAsmNoArgs)
};

// \brief 64x0 register prefix
// example: r32, r0
// r32 -> sp
// r0 -> hw zero

#define kAsmFloatRegisterPrefix "f"
#define kAsmFloatRegisterLimit  10

#define kAsmRegisterPrefix "r"
#define kAsmRegisterLimit  20
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

// SET A HANDLER IN ADDRESS:
// DISABLE INTERRUPTS
// PROCESS INTERRUPT
// ENABLE INTERRUPTS

////////////////////////////////