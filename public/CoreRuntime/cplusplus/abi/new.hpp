// SPDX-License-Identifier: Apache-2.0
// Copyright 2025-2026, Amlal El Mahrouss (amlal@nekernel.org)
// Licensed under the Apache License, Version 2.0 (see LICENSE file)
// Official repository: https://github.com/ne-foss/nectar

#pragma once

#include <CoreRuntime/cplusplus/defines.hpp>

namespace std {

struct nothrow_t final {
  explicit nothrow_t() = default;
  ~nothrow_t()         = default;
};

struct placement_t final {
  explicit placement_t() = default;
  ~placement_t()         = default;

  void*   __base{};
  int32_t __align{};
  size_t  __size{};
};

}  // namespace std

// AMLALE: Define the placement_t feature.
#ifndef __cpp_has_placement
#define __cpp_has_placement 1
#endif

// AMLALE: Define nothrow
#ifndef __cpp_has_nothrow
#define __cpp_has_nothrow 1
#endif

void* operator new(size_t);
void* operator new[](size_t);

/// \brief placement_t new and delete operators. Governs how the memory shall be placed.
/// \note This is a feature that shall be used wisely, failure to do so will produce Undefined Behaviors at runtime.
void* operator _placement_new(size_t);
void operator _placement_delete(void*);

/// \note This should NOT fail, failure to meet the conditions will cause the program's state to be aborted.
/// \brief Set the placement policy of future memory allocations.
template <class PlaceableType>
void set_placement_policy(const PlaceableType&) noexcept;

void* operator new(size_t, const nothrow_t&) noexcept;
void* operator new(size_t, void*) noexcept;
void* operator new[](size_t, const nothrow_t&) noexcept;
void* operator new[](size_t, void*) noexcept;

void operator delete(void*) noexcept;
void operator delete(void*, size_t) noexcept;

void operator delete[](void*) noexcept;
