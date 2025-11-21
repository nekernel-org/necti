/* ========================================

  Copyright (C) 2025, Amlal El Mahrouss, licensed under the Apache 2.0 license.

======================================== */

#pragma once

#include <LibC++/defines.h>

namespace std {
struct placement_new;

/// =========================================================
/// @brief Disambugate non-throwing allocation functions.
/// =========================================================
struct nothrow_t {
  explicit nothrow_t() = default;
};

/// =========================================================
/// @brief Placement new metadata.
/// =========================================================
struct placement_new final {
  void*   __base{};
  int32_t __align{};
  size_t  __size{};
};

using placement_new_t = placement_new;
}  // namespace std

void* operator new(size_t);
void* operator new[](size_t);

void* operator new(size_t, const std::nothrow_t&) noexcept;
void* operator new(size_t, void*) noexcept;
void* operator new[](size_t, const std::nothrow_t&) noexcept;
void* operator new[](size_t, void*) noexcept;

void operator delete(void*) noexcept;
void operator delete(void*, size_t) noexcept;

void operator delete[](void*) noexcept;
