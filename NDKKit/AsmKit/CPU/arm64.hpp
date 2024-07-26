/* -------------------------------------------

Copyright ZKA Technologies

------------------------------------------- */

#pragma once

#include <NDKKit/Defines.hpp>

/// @brief ARM64 encoding support.
/// @file CPU/arm64.hpp

struct CpuOpcodeArm64;

/// @brief ARM64 opcode header.
struct CpuOpcodeArm64 final
{
	uint8_t fOpcode; // opcode
	uint8_t fRegisterLeft; // left register index
	uint8_t fRegisterRight; // right register index
	bool fRegisterLeftHooked;
	bool fRegisterRightHooked;
	uint32_t fImmediateValue; // immediate 32-bit value
	bool fImmediateValueHooked;
};
