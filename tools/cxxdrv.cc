/* -------------------------------------------

	Copyright (C) 2024-2025 Amlal EL Mahrous, all rights reserved

------------------------------------------- */

/// @file cxxdrv.cc
/// @brief NE C++ frontend compiler.

#include <LibCompiler/Defines.h>
#include <LibCompiler/ErrorID.h>
#include <LibCompiler/Version.h>
#include <iostream>
#include <cstring>
#include <vector>

LC_IMPORT_C int CompilerCPlusPlusAMD64(int argc, char const* argv[]);
LC_IMPORT_C int AssemblerMainAMD64(int argc, char const* argv[]);

int main(int argc, char const* argv[])
{
	std::vector<std::string> args_list_cxx;
	std::vector<std::string> args_list_asm;

	for (size_t index_arg = 0; index_arg < argc; ++index_arg)
	{
		if (strstr(argv[index_arg], ".cxx") ||
			strstr(argv[index_arg], ".cpp") ||
			strstr(argv[index_arg], ".cc") ||
			strstr(argv[index_arg], ".c++") ||
			strstr(argv[index_arg], ".C"))
		{
			std::string arg = argv[index_arg];

			arg += ".pp.masm";
			args_list_asm.push_back(arg);

			arg = argv[index_arg];
			arg += ".pp";

			args_list_cxx.push_back(arg);
		}
		else if (strstr(argv[index_arg], ".c"))
		{
			std::printf("cxxdrv: error: Not a C driver.\n");
			return EXIT_FAILURE;
		}
	}

	for (auto& cli : args_list_cxx)
	{
		const char* arr_cli[] = {argv[0], cli.data()};

		if (auto code = CompilerCPlusPlusAMD64(2, arr_cli);
			code > 0)
		{
			std::printf("cxxdrv: compiler exited with code %i.\n", code);

			return LIBCOMPILER_EXEC_ERROR;
		}
	}

	for (auto& cli : args_list_asm)
	{
		const char* arr_cli[] = {argv[0], cli.data()};

		if (auto code = AssemblerMainAMD64(2, arr_cli);
			code > 0)
		{
			std::printf("cxxdrv: assembler exited with code %i.\n", code);
		}
	}

	return LIBCOMPILER_EXEC_ERROR;
}
