// Copyright 2024-2025, Amlal El Mahrouss (amlal@nekernel.org)
// Licensed under the Apache License, Version 2.0 (See accompanying
// file LICENSE or copy at http://www.apache.org/licenses/LICENSE-2.0)
// Official repository: https://github.com/nekernel-org/nectar

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
