/* -------------------------------------------

	Copyright ZKA Web Services Co

------------------------------------------- */

/// @brief provide support for Macros.hpp header.

#ifndef _MACROS_HXX_
#define _MACROS_HXX_

#define TOOLCHAINKIT_COPY_DELETE(KLASS)               \
	KLASS& operator=(const KLASS&) = delete; \
	KLASS(const KLASS&)			   = delete;

#define TOOLCHAINKIT_COPY_DEFAULT(KLASS)               \
	KLASS& operator=(const KLASS&) = default; \
	KLASS(const KLASS&)			   = default;

#define TOOLCHAINKIT_MOVE_DELETE(KLASS)          \
	KLASS& operator=(KLASS&&) = delete; \
	KLASS(KLASS&&)			  = delete;

#define TOOLCHAINKIT_MOVE_DEFAULT(KLASS)          \
	KLASS& operator=(KLASS&&) = default; \
	KLASS(KLASS&&)			  = default;

/// @note xxxx is the error placeholder, in hexadecimal.
#define TOOLCHAINKIT_ERROR_PREFIX_CXX "CXXxxxx"
#define TOOLCHAINKIT_ERROR_PREFIX_CL	 "CLxxxx"
#define TOOLCHAINKIT_ERROR_PREFIX_ASM "ASMxxxx"

#endif /* ifndef _MACROS_HXX_ */
