/* -------------------------------------------

Copyright (C) 2024-2025 Amlal EL Mahrous, all rights reserved

------------------------------------------- */

#pragma once

#include <LibCompiler/Defines.h>
#include <stdint.h>

/// @brief ARM64 encoding support.
/// @file Backend/Aarch64.h

struct CpuOpcodeArm64;

/// @brief ARM64 opcode header.
struct PACKED CpuOpcodeArm64_Data final {
  uint32_t fOpcode : 10;  // Bits 31–22: Opcode for operation
  uint32_t fRm : 5;       // Bits 21–16: Source register Rm
  uint32_t fShamt : 6;    // Bits 15–10: Shift amount
  uint32_t fRn : 5;       // Bits 9–5: Source register Rn
  uint32_t fRd : 5;       // Bits 4–0: Destination register Rd
};

typedef struct {
  uint32_t opcode : 6;   // Bits 31–26: Branch opcode
  int32_t  offset : 26;  // Bits 25–0: Signed offset (branch target)
} PACKED CpuOpcodeArm64_Branch;

typedef struct {
  uint32_t size : 2;     // Bits 31–30: Size of the data
  uint32_t opcode : 7;   // Bits 29–23: Opcode for load/store
  uint32_t offset : 12;  // Bits 22–10: Offset
  uint32_t rn : 5;       // Bits 9–5: Base address register Rn
  uint32_t rt : 5;       // Bits 4–0: Target/source register Rt
} PACKED CpuOpcodeArm64_LoadStore;

#define kAsmRegisterLimit (30)
#define kAsmRegisterPrefix "x"
#define kOpcodeARM64Count (1000)
