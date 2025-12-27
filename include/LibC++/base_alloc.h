// Copyright 2024-2025, Amlal El Mahrouss (amlal@nekernel.org)
// Licensed under the Apache License, Version 2.0 (See accompanying
// file LICENSE or copy at http://www.apache.org/licenses/LICENSE-2.0)
// Official repository: https://github.com/nekernel-org/nectar

#ifndef NECTAR_LIBCXX_BASE_ALLOC_H
#define NECTAR_LIBCXX_BASE_ALLOC_H

#include <LibC++/defines.h>

namespace std::base_alloc {
/// @brief allocate a new class.
/// @tparam KindClass the class type to allocate.
template <class KindClass, typename... Args>
inline KindClass* allocate(Args&&... args) {
  return new KindClass(forward(args)...);
}

/// @brief allocate a new class.
/// @note aborts on error.
/// @tparam KindClass the class type to allocate.
template <class KindClass, typename... Args>
inline KindClass* allocate_nothrow(Args&&... args) noexcept {
  return allocate(forward(args)...);
}

/// @brief free a class.
/// @tparam KindClass the class type to allocate.
template <class KindClass>
inline void release(KindClass ptr) {
  if (!ptr) return;

  delete ptr;
}

/// @brief destroy and free a class.
/// @note aborts on error.
/// @tparam KindClass the class type to allocate.
template <class KindClass>
inline void release_nothrow(KindClass ptr) noexcept {
  release(ptr);
}
}  // namespace std::base_alloc

#endif  // NECTAR_LIBCXX_BASE_ALLOC_H
