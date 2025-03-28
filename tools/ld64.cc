/* -------------------------------------------

	Copyright (C) 2024-2025 Amlal EL Mahrous, all rights reserved

------------------------------------------- */

#include <LibCompiler/Defines.h>

/// @file ld64.cxx
/// @brief NE Linker for AE objects.

LC_IMPORT_C int DynamicLinker64PEF(int argc, char const* argv[]);

int main(int argc, char const* argv[])
{
	if (argc < 1)
	{
		return 1;
	}

	return DynamicLinker64PEF(argc, argv);
}
