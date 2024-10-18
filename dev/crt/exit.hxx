/* -------------------------------------------

	Copyright ZKA Web Services Co.

------------------------------------------- */

#pragma once

/// @brief CRT exit, with exit code (!!! exits all threads. !!!)
/// @param code
/// @return
extern "C" int exit(int code);

/// @brief Standard C++ namespace
namespace std
{
	inline int exit(int code)
	{
		exit(code);
	}
} // namespace std
