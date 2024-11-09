/* -------------------------------------------

	Copyright (C) 2024, Amlal EL Mahrouss, all rights reserved

------------------------------------------- */

#include <ToolchainKit/Defines.h>

/// @file ld64.cxx
/// @brief ZKA Linker for AE objects.

TK_IMPORT_C int DynamicLinker64PEF(int argc, char const* argv[]);

int main(int argc, char const* argv[])
{
	if (argc < 1)
	{
		return 1;
	}

	return DynamicLinker64PEF(argc, argv);
}
