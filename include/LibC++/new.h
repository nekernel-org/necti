// Copyright 2024-2025, Amlal El Mahrouss (amlal@nekernel.org)
// Licensed under the Apache License, Version 2.0 (See accompanying
// file LICENSE or copy at http://www.apache.org/licenses/LICENSE-2.0)
// Official repository: https://github.com/nekernel-org/nectar

#ifndef NECTAR_LIBCXX_NEW_H
#define NECTAR_LIBCXX_NEW_H

#include <defines>

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
/// \note This is a feature that shall be used wisely, failure to do so will produce Undefined
/// Behaviors at runtime.
void* operator _placement_new(struct placement_t*);
void operator _placement_delete(struct placement_t*, void*);

/// \brief For all offsets within the base range and alignement 'align'
/// \brief Allocate offsets with respect to the `base` interval, apply alignement of `align` value.
/// Return `offsets` of length n as an aligned value within the domain of `base`.
using placeable_callable_type = void* /*offsets*/ (*) (void* base, size_t n, const int& align);

/// \note This should NOT fail, failure to meet the conditions will cause the program's state to be
/// aborted.
/// \brief Set the placement policy of future memory allocations.
template <class PlaceableCallable>
void set_placement_policy(const PlaceableCallable&) noexcept;

void* operator new(size_t, const nothrow_t&) noexcept;
void* operator new(size_t, void*) noexcept;
void* operator new[](size_t, const nothrow_t&) noexcept;
void* operator new[](size_t, void*) noexcept;

void operator delete(void*) noexcept;
void operator delete(void*, size_t) noexcept;

void operator delete[](void*) noexcept;

#endif  // NECTAR_LIBCXX_NEW_H
