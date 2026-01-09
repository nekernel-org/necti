// Copyright 2024-2025, Amlal El Mahrouss (amlal@nekernel.org)
// Licensed under the Apache License, Version 2.0 (See accompanying
// file LICENSE or copy at http://www.apache.org/licenses/LICENSE-2.0)
// Official repository: https://github.com/nekernel-org/nectar

#ifndef LIBNECTAR_UTILITY_H
#define LIBNECTAR_UTILITY_H

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

#endif  // LIBNECTAR_UTILITY_H
