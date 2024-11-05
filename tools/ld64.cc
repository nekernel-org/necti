/* -------------------------------------------

	Copyright Amlal EL Mahrouss

------------------------------------------- */

#include <ToolchainKit/Defines.h>

/// @file ld64.cxx
/// @brief ZKA Linker for AE objects.

TK_IMPORT_C int Linker64Main(int argc, char const* argv[]);

int main(int argc, char const* argv[])
{
	if (argc < 1)
	{
		return 1;
	}

	return Linker64Main(argc, argv);
}
