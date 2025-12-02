/* ========================================

  Copyright (C) 2025, Amlal El Mahrouss, licensed under the Apache 2.0 license.

======================================== */

#pragma once

#include <LibC++/defines.h>

namespace std {
struct nothrow_t final {
  explicit nothrow_t() = default;
  ~nothrow_t() = default;
};

struct placement_t final {
  explicit placement_t() = default;
  ~placement_t() = default;

  void*   __base{};
  int32_t __align{};
  size_t  __size{};
};
}  // namespace std

#ifndef __has_placement
#define placement
#endif

void* operator new(size_t);
void* operator new[](size_t);

void* operator new(size_t, const std::nothrow_t&) noexcept;
void* operator new(size_t, void*) noexcept;
void* operator new[](size_t, const std::nothrow_t&) noexcept;
void* operator new[](size_t, void*) noexcept;

void operator delete(void*) noexcept;
void operator delete(void*, size_t) noexcept;

void operator delete[](void*) noexcept;
