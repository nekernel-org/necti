// Copyright 2024-2025, Amlal El Mahrouss (amlal@nekernel.org)
// Licensed under the Apache License, Version 2.0 (See accompanying
// file LICENSE or copy at http://www.apache.org/licenses/LICENSE-2.0)
// Official repository: https://github.com/ne-foss-org/nectar

#ifndef NECTAR_COMPILERKIT_ERRORID_H
#define NECTAR_COMPILERKIT_ERRORID_H

#include <CompilerKit/Detail/Config.h>

/// =========================================================== ///
/// @file ErrorID.h
/// @author Amlal El Mahrouss
/// @brief Error IDs for CompilerKit.
/// =========================================================== ///

#define NECTAR_SUCCESS 0
#define NECTAR_EXEC_ERROR -30
#define NECTAR_FILE_NOT_FOUND -31
#define NECTAR_DIR_NOT_FOUND -32
#define NECTAR_FILE_EXISTS -33
#define NECTAR_TOO_LONG -34
#define NECTAR_INVALID_DATA -35
#define NECTAR_UNIMPLEMENTED -36
#define NECTAR_FAT_ERROR -37
#define NECTAR_INVALID_ARCH -38

#endif  // NECTAR_COMPILERKIT_ERRORID_H
