/* -------------------------------------------

	Copyright ZKA Technologies

------------------------------------------- */

/// @brief provide support for Macros.hpp header.

#ifndef _CK_CL_HPP
#define _CK_CL_HPP

#define NDK_COPY_DELETE(KLASS)              \
	KLASS& operator=(const KLASS&) = delete; \
	KLASS(const KLASS&)			   = delete;

#define NDK_COPY_DEFAULT(KLASS)              \
	KLASS& operator=(const KLASS&) = default; \
	KLASS(const KLASS&)			   = default;

#define NDK_MOVE_DELETE(KLASS)         \
	KLASS& operator=(KLASS&&) = delete; \
	KLASS(KLASS&&)			  = delete;

#define NDK_MOVE_DEFAULT(KLASS)         \
	KLASS& operator=(KLASS&&) = default; \
	KLASS(KLASS&&)			  = default;

/// @note xxxx is the error placeholder, in hexadecimal.
#define NDK_ERROR_PREFIX_CXX "CXXxxxx"
#define NDK_ERROR_PREFIX_CL  "CLxxxx"
#define NDK_ERROR_PREFIX_ASM "ASMxxxx"

#endif /* ifndef _CK_CL_HPP */
