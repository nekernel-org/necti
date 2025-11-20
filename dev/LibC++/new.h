/* -------------------------------------------

  Copyright (C) 2025, Amlal El Mahrouss, licensed under the Apache 2.0 license.

------------------------------------------- */

#pragma once

#include <LibC++/defines.h>

struct __placement_new_info;

void* operator new(size_t);
void* operator new[](size_t);

void operator delete(void*) noexcept;
void operator delete(void*, unsigned long) noexcept;

void operator delete[](void*) noexcept;

/// =========================================================
/// @brief Placement new information structure
/// =========================================================
struct __placement_new_info {
  void* __base;
  int __align;
  long long __size;
};