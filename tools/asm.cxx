/* -------------------------------------------

	Copyright ZKA Technologies

------------------------------------------- */

/// @file Linker.cxx
/// @brief ZKA C++ frontend for ZKA OS.

#include <ndk/Version.hxx>
#include <iostream>
#include <cstring>
#include <string>
#include <vector>

extern "C" int ZKAAssemblerMainPowerPC(int argc, char const* argv[]);
extern "C" int ZKAAssemblerMain64000(int argc, char const* argv[]);
extern "C" int ZKAAssemblerMainAMD64(int argc, char const* argv[]);

int main(int argc, char const* argv[])
{
	std::vector<const char*> arg_vec_cstr;
	arg_vec_cstr.push_back(argv[0]);

	enum
	{
		eX64Assembler,
		e64X0Assembler,
		ePOWER64Assembler,
		eInvalidAssembler
	} cAsm = eInvalidAssembler;

	for (size_t index_arg = 1; index_arg < argc; ++index_arg)
	{
		if (strstr(argv[index_arg], "/?"))
		{
			std::printf("asm.exe: Frontend Assembler (64x0, POWER64, AMD64).\n");
			std::printf("asm.exe: Version: %s, Release: %s.\n", kDistVersion, kDistRelease);
			std::printf("asm.exe: Designed by Amlal El Mahrouss, Copyright ZKA Technologies.\n");
			std::printf("libndk.so/ndk.dll: Designed by Amlal El Mahrouss, Copyright ZKA Technologies.\n");

			return 0;
		}
		else if (strstr(argv[index_arg], "/Asm:x64"))
		{
			cAsm = eX64Assembler;
		}
		else if (strstr(argv[index_arg], "/Asm:64x0"))
		{
			cAsm = e64X0Assembler;
		}
		else if (strstr(argv[index_arg], "/Asm:POWER64"))
		{
			cAsm = ePOWER64Assembler;
		}
		else
		{
			arg_vec_cstr.push_back(argv[index_arg]);
		}
	}

	switch (cAsm)
	{
	case ePOWER64Assembler: {
		if (int32_t code = ZKAAssemblerMainPowerPC(arg_vec_cstr.size(), arg_vec_cstr.data()); code)
		{
			std::printf("asm.exe: frontend exited with code %i.\n", code);
			return code;
		}
		break;
	}
	case e64X0Assembler: {
		if (int32_t code = ZKAAssemblerMain64000(arg_vec_cstr.size(), arg_vec_cstr.data()); code)
		{
			std::printf("asm.exe: frontend exited with code %i.\n", code);
			return code;
		}
		break;
	}
	case eX64Assembler: {
		if (int32_t code = ZKAAssemblerMainAMD64(arg_vec_cstr.size(), arg_vec_cstr.data()); code)
		{
			std::printf("asm.exe: frontend exited with code %i.\n", code);
			return code;
		}
		break;
	}
	default: {
		return -1;
	}
	}

	return 0;
}
