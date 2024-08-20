/* -------------------------------------------

	Copyright ZKA Technologies

------------------------------------------- */

#pragma once

/// This file is an implementation of __throw* family of functions.

#include <exception>
#include <iostream>
#include <stdexcept>

namespace std
{
	inline void __throw_general(void)
	{
		throw std::runtime_error("NDK C++ Runtime error.");
	}

	inline void __throw_domain_error(const char* error)
	{
		std::cout << "NDK C++: Domain error: " << error << "\r";
		__throw_general();
	}
} // namespace std
