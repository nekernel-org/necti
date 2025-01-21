/* -------------------------------------------

	Copyright (C) 2024 Amlal EL Mahrouss, all rights reserved

------------------------------------------- */

/// @brief provide support for Macros.hpp header.

#ifndef _MACROS_H_
#define _MACROS_H_

#define LIBCOMPILER_COPY_DELETE(KLASS)       \
	KLASS& operator=(const KLASS&) = delete; \
	KLASS(const KLASS&)			   = delete;

#define LIBCOMPILER_COPY_DEFAULT(KLASS)       \
	KLASS& operator=(const KLASS&) = default; \
	KLASS(const KLASS&)			   = default;

#define LIBCOMPILER_MOVE_DELETE(KLASS)  \
	KLASS& operator=(KLASS&&) = delete; \
	KLASS(KLASS&&)			  = delete;

#define LIBCOMPILER_MOVE_DEFAULT(KLASS)  \
	KLASS& operator=(KLASS&&) = default; \
	KLASS(KLASS&&)			  = default;

/// @note xxxx is the error placeholder, in hexadecimal.
#define LIBCOMPILER_ERROR_PREFIX_CXX "CXXxxxx"
#define LIBCOMPILER_ERROR_PREFIX_CL	 "CLxxxx"
#define LIBCOMPILER_ERROR_PREFIX_ASM "ASMxxxx"

#endif /* ifndef _MACROS_H_ */
