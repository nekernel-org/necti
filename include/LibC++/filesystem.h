/* ========================================

  Copyright (C) 2024-2025 Amlal El Mahrouss, licensed under the Apache 2.0 license.

======================================== */

#ifndef __NECTAR_FS_H__
#define __NECTAR_FS_H__

#include <chrono>
#include <defines>

namespace std {
class path;
class filesystem_error;
class directory_entry;
class directory_iterator;
}  // namespace std

#ifndef __cpp_lib_filesystem
#define __cpp_lib_filesystem 201703L
#endif

#endif  // __NECTAR_FS_H__

