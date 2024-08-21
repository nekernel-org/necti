/* -------------------------------------------

	Copyright ZKA Technologies

------------------------------------------- */

/// @file Linker.cxx
/// @brief ZKA Linker frontend for AE objects.

#include <iostream>
#include <cstring>
#include <vector>

extern "C" int CompilerCPlusPlusX8664(int argc, char const* argv[]);
extern "C" int NewOSAssemblerAMD64(int argc, char const* argv[]);

int main(int argc, char const* argv[])
{
	if (auto code = CompilerCPlusPlusX8664(argc, argv); code)
	{
		std::printf("cl.exe: frontend exited with code %i.\n", code);
        return 1;
	}
	else
	{
		std::vector<std::string> args_list;

		for (size_t index_arg = 0; index_arg < argc; ++index_arg)
		{
			if (strstr(argv[index_arg], ".cxx") ||
				strstr(argv[index_arg], ".cpp") ||
				strstr(argv[index_arg], ".cc") ||
				strstr(argv[index_arg], ".c++") ||
				strstr(argv[index_arg], ".C"))
			{
                std::string arg = argv[index_arg];
                arg += ".masm";
                args_list.push_back(arg);
			}
		}

		for (auto &cli : args_list)
		{
			const char* arr_cli[] = { argv[0], cli.data() };

			if (auto code = NewOSAssemblerAMD64(2, arr_cli); code)
			{
				std::printf("cl.exe: assembler exited with code %i.", code);
			}
		}

	}

	return 0;
}
