// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026, Amlal El Mahrouss (amlal@nekernel.org)
// Licensed under the Apache License, Version 2.0 (see LICENSE file)
// Official repository: https://github.com/ne-foss-org/nectar

#ifndef CRK_UTILITY_HPP
#define CRK_UTILITY_HPP

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

#endif  // CRK_UTILITY_HPP
