/* -------------------------------------------

	Copyright ZKA Web Services Co.

------------------------------------------- */

#pragma once

#include <LibC++/defines.hxx>
#include <LibC++/exit.hxx>

namespace std
{
	inline void __throw_general(void)
	{
		exit(33);
	}

	inline void __throw_domain_error(const char* error)
	{
		__throw_general();
		__builtin_unreachable(); // prevent from continuing.
	}

	inline void __throw_bad_array_new_length(void)
	{
		__throw_general();
		__builtin_unreachable(); // prevent from continuing.
	}
} // namespace std
