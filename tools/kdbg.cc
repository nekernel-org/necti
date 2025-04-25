/* -------------------------------------------

	Copyright (C) 2024-2025 Amlal EL Mahrous, all rights reserved

------------------------------------------- */

#include <LibCompiler/Defines.h>

/// @file kdbg.cxx
/// @brief NeKernel debugger.

LC_IMPORT_C int DebuggerNeKernel(int argc, char const* argv[]);

/// @brief Debugger entrypoint.
/// @return Status code of debugger.
int main(int argc, char const* argv[])
{
	return DebuggerNeKernel(argc, argv);
}
