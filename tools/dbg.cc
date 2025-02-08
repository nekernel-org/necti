/* -------------------------------------------

	Copyright (C) 2024 Amlal EL Mahrouss, all rights reserved

------------------------------------------- */

#include <LibCompiler/Defines.h>

/// @file ld64.cxx
/// @brief NE Linker for AE objects.

LC_IMPORT_C int DebuggerPOSIX(int argc, char const* argv[]);

int main(int argc, char const* argv[])
{
	if (argc < 1)
	{
		return 1;
	}

	return DebuggerPOSIX(argc, argv);
}
