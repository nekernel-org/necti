// Copyright 2024-2025, Amlal El Mahrouss (amlal@nekernel.org)
// Licensed under the Apache License, Version 2.0 (See accompanying
// file LICENSE or copy at http://www.apache.org/licenses/LICENSE-2.0)
// Official repository: https://github.com/ne-app-eu/ncc

#ifndef NCC_COMPILERKIT_ERRORID_H
#define NCC_COMPILERKIT_ERRORID_H

#include <CompilerKit/Detail/Config.h>

/// =========================================================== ///
/// @file ErrorID.h
/// @author Amlal El Mahrouss
/// @brief Error IDs for CompilerKit.
/// =========================================================== ///

#define NCC_SUCCESS 0
#define NCC_EXEC_ERROR -30
#define NCC_FILE_NOT_FOUND -31
#define NCC_DIR_NOT_FOUND -32
#define NCC_FILE_EXISTS -33
#define NCC_TOO_LONG -34
#define NCC_INVALID_DATA -35
#define NCC_UNIMPLEMENTED -36
#define NCC_FAT_ERROR -37
#define NCC_INVALID_ARCH -38

#endif  // NCC_COMPILERKIT_ERRORID_H
