/* -------------------------------------------

	Copyright (C) 2024 Theater Quality Incorporated., all rights reserved.

------------------------------------------- */

#pragma once

/// @brief CRT exit, with exit code (!!! exits all threads. !!!)
/// @param code the exit code.
/// @return the return > 0 for non successful.
extern "C" int exit(int code);

/// @brief Standard C++ namespace
namespace std::process_base
{
	inline int exit(int code)
	{
		exit(code);
	}
} // namespace std::process_base
