/* -------------------------------------------

	Copyright (C) 2024 Theater Quality Corp, all rights reserved.

------------------------------------------- */

#pragma once

#include <LibC++/defines.h>
#include <LibC++/process_base.h>

namespace std::base_exception
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
} // namespace std::base_exception
