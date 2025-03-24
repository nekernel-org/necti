/* -------------------------------------------

	Copyright (C) 2024 Amlal EL Mahrouss, all rights reserved

------------------------------------------- */

#include <LibCompiler/Defines.h>

/// @file dbg.cxx
/// @brief NE debugger.

LC_IMPORT_C int DebuggerPOSIX(int argc, char const* argv[]);

/// @brief Debugger entrypoint.
/// @return Status code of debugger.
int main(int argc, char const* argv[])
{
	return DebuggerPOSIX(argc, argv);
}
