/* -------------------------------------------

	Copyright (C) 2024 Amlal EL Mahrouss, all rights reserved

------------------------------------------- */

/// @file asm.cxx
/// @brief Assembler frontend.

#include <LibCompiler/Defines.h>
#include <LibCompiler/Version.h>
#include <iostream>
#include <cstring>
#include <string>
#include <vector>

LC_IMPORT_C int AssemblerMainPower64(int argc, char const* argv[]);
LC_IMPORT_C int AssemblerMainARM64(int argc, char const* argv[]);
LC_IMPORT_C int AssemblerMain64x0(int argc, char const* argv[]);
LC_IMPORT_C int AssemblerMainAMD64(int argc, char const* argv[]);

int main(int argc, char const* argv[])
{
	std::vector<const char*> arg_vec_cstr;
	arg_vec_cstr.push_back(argv[0]);

	enum
	{
		kX64Assembler,
		k64X0Assembler,
		kPOWER64Assembler,
		kARM64Assembler,
		kInvalidAssembler
	} asm_type = kInvalidAssembler;

	for (size_t index_arg = 1; index_arg < argc; ++index_arg)
	{
		if (strstr(argv[index_arg], "--asm:h"))
		{
			std::printf("asm.exe: Frontend Assembler (64x0, power64, arm64, x64).\n");
			std::printf("asm.exe: Version: %s, Release: %s.\n", kDistVersion, kDistRelease);
			std::printf("asm.exe: Designed by Amlal EL Mahrouss., Copyright (C) 2024-2025 Amlal EL Mahrouss, all rights reserved.\n");
			std::printf("libCCDyn.dylib: Designed by Amlal EL Mahrouss, Copyright (C) 2024-2025 Amlal EL Mahrouss, all rights reserved.\n");

			return 0;
		}
		else if (strstr(argv[index_arg], "--asm:x64"))
		{
			asm_type = kX64Assembler;
		}
		else if (strstr(argv[index_arg], "--asm:aarch64"))
		{
			asm_type = kARM64Assembler;
		}
		else if (strstr(argv[index_arg], "--asm:64x0"))
		{
			asm_type = k64X0Assembler;
		}
		else if (strstr(argv[index_arg], "--asm:power64"))
		{
			asm_type = kPOWER64Assembler;
		}
		else
		{
			arg_vec_cstr.push_back(argv[index_arg]);
		}
	}

	switch (asm_type)
	{
	case kPOWER64Assembler: {
		if (int32_t code = AssemblerMainPower64(arg_vec_cstr.size(), arg_vec_cstr.data()); code)
		{
			std::printf("asm.exe: frontend exited with code %i.\n", code);
			return code;
		}
		break;
	}
	case k64X0Assembler: {
		if (int32_t code = AssemblerMain64x0(arg_vec_cstr.size(), arg_vec_cstr.data()); code)
		{
			std::printf("asm.exe: frontend exited with code %i.\n", code);
			return code;
		}
		break;
	}
	case kARM64Assembler: {
		if (int32_t code = AssemblerMainARM64(arg_vec_cstr.size(), arg_vec_cstr.data()); code)
		{
			std::printf("asm.exe: frontend exited with code %i.\n", code);
			return code;
		}
		break;
	}
	case kX64Assembler: {
		if (int32_t code = AssemblerMainAMD64(arg_vec_cstr.size(), arg_vec_cstr.data()); code)
		{
			std::printf("asm.exe: frontend exited with code %i.\n", code);
			return code;
		}
		break;
	}
	default: {
		return 1;
	}
	}

	return 0;
}
