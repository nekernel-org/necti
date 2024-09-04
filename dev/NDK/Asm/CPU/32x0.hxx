/* -------------------------------------------

	Copyright ZKA Technologies

------------------------------------------- */

#pragma once

#include <NDK/Defines.hxx>

// @brief 32x0 support.
// @file CPU/32x0.hpp

#define kAsmOpcodeDecl(__NAME, __OPCODE, __FUNCT3, __FUNCT7) \
	{.fName	  = __NAME,                                      \
	 .fOpcode = __OPCODE,                                    \
	 .fFunct3 = __FUNCT3,                                    \
	 .fFunct7 = __FUNCT7},

#define kAsmImmediate 0x01
#define kAsmSyscall	  0x02
#define kAsmJump	  0x03
#define kAsmNoArgs	  0x04

#define kAsmByte  0
#define kAsmHWord 1
#define kAsmWord  2

struct CpuCode32x0
{
	const char fName[32];
	uint8_t	   fOpcode;
	uint8_t	   fSize;
	uint8_t	   fFunct3;
	uint8_t	   fFunct7;
};

#define kAsmDWordStr ".dword" /* 64 bit */
#define kAsmWordStr	 ".word"  /* 32-bit */
#define kAsmHWordStr ".half"  /* 16-bit */
#define kAsmByteStr	 ".byte"  /* 8-bit */

inline std::vector<CpuCode32x0> kOpcodes32x0 = {
	kAsmOpcodeDecl("nop", 0b0100011, 0b000, kAsmNoArgs)	   // nothing to do. (1C)
	kAsmOpcodeDecl("jmp", 0b1110011, 0b001, kAsmJump)	   // jump to branch (2C)
	kAsmOpcodeDecl("mov", 0b0100011, 0b101, kAsmImmediate) // move registers (3C)
	kAsmOpcodeDecl("psh", 0b0111011, 0b000, kAsmImmediate) // push to sp (2C)
	kAsmOpcodeDecl("pop", 0b0111011, 0b001, kAsmImmediate) // pop from sp. (1C)
	kAsmOpcodeDecl("lea", 0b0111011, 0b010,
				   kAsmImmediate) // setup stack and call, store address to CR (1C).
	kAsmOpcodeDecl("ret", 0b0111011, 0b110,
				   kAsmImmediate)						 // return from procedure (2C).
	kAsmOpcodeDecl("uc", 0b0111111, 0b000, kAsmSyscall)	 // user call (1C)
	kAsmOpcodeDecl("kc", 0b0111111, 0b001, kAsmSyscall)	 // kernel call (1C)
	kAsmOpcodeDecl("int", 0b0111111, 0b010, kAsmSyscall) // raise interrupt (1C)
};

// \brief 64x0 register prefix
// example: r32, r0
// r32 -> sp
// r0 -> hw zero

#define kAsmRegisterPrefix "r"
#define kAsmRegisterLimit  16
#define kAsmPcRegister	   17
#define kAsmCrRegister	   18
#define kAsmSpRegister	   5

/* return address register */
#define kAsmRetRegister 19

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
