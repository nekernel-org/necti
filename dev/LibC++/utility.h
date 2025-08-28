/* -------------------------------------------

 Copyright (C) 2024-2025 Amlal El Mahrouss, all rights reserved.

------------------------------------------- */

#ifndef LIBCXX_UTILITY_H
#define LIBCXX_UTILITY_H

namespace std {
/// @brief Forward object.
/// @tparam Args the object type.
/// @param arg the object.
/// @return object's rvalue
template <typename Args>
inline auto forward(Args& arg) -> Args&& {
  return static_cast<const Args&&>(arg);
}

/// @brief Move object.
/// @tparam Args the object type.
/// @param arg the object.
/// @return object's rvalue
template <typename Args>
inline auto move(Args&& arg) -> Args&& {
  return static_cast<Args&&>(arg);
}
}  // namespace std

#endif  // LIBCXX_UTILITY_H
