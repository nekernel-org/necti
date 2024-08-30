/* -------------------------------------------

	Copyright ZKA Technologies

------------------------------------------- */

/// @file Linker.cxx
/// @brief ZKA C++ frontend for ZKA OS.

#include <ndkdll/Version.hxx>
#include <iostream>
#include <cstring>
#include <vector>

extern "C" int ZKAAssemblerMain64000(int argc, char const* argv[]);

int main(int argc, char const* argv[])
{
	for (size_t index_arg = 0; index_arg < argc; ++index_arg)
	{
		if (strstr(argv[index_arg], "/?"))
		{
			std::printf("asm.exe: Frontend Assembler (64x0, POWER64, AMD64).\n");
			std::printf("asm.exe: Version: %s, Release: %s.\n", kDistVersion, kDistRelease);
			std::printf("asm.exe: Designed by Amlal El Mahrouss, Copyright ZKA Technologies.\n");
			std::printf("libndk.dylib/ndk.dll: Designed by Amlal El Mahrouss, Copyright ZKA Technologies.\n");

			return 0;
		}
	}

	if (int32_t code = ZKAAssemblerMain64000(argc, argv); code)
	{
		std::printf("asm.exe: frontend exited with code %i.\n", code);
		return code;
	}

	return 0;
}
