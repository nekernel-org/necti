/* -------------------------------------------

  Copyright (C) 2024-2025 Amlal El Mahrouss, all rights reserved.

------------------------------------------- */

#pragma once

#include <LibC++/defines.h>

/// @file Math.h
/// @brief Math functions.

#ifdef __NE_USE_DOUBLE__
typedef double real_type;
#else
typedef float real_type;
#endif

namespace std::base_math {
inline constexpr static auto not_a_number = NAN;

/// @brief Power function, with Repeat argument.
template <size_t Exponent>
inline real_type pow(real_type in) {
  if (Exponent == 0) return 1;  // Any number to the power of 0 is 1.

  if (Exponent == 1) return in;  // Any number to the power of 1 is itself.

  size_t cnt = Exponent;

  real_type result = 1;

  for (auto i = 0; i < cnt; ++i) result *= in;

  return result;
}

inline real_type sqrt(real_type in) {
  if (in == 0) return 0;
  if (in == not_a_number) return not_a_number;

  auto constexpr const static Base = 2;

  auto x = in / Base;

  for (int i = 0; i < 10; ++i) {
    x = (x + in / x) / Base;
  }

  return x;
}

/// @brief Square of function, with Base template argument.
/// @param of Base argument to find sqquare of
template <size_t Base>
inline real_type surd(real_type in) {
  if (in == 0) return 0;
  if (in == 1) return 1;

  if (Base == 1) return in;
  if (Base == 2) return sqrt(in);

  return not_a_number;
}

/// @brief Linear interpolation equation solver.
/// @param from where?
/// @param to to?
/// @param Updated diff value according to difference.
inline real_type lerp(real_type to, real_type from, real_type stat) {
  real_type diff = (to - from);
  return from + (diff * stat);
}
}  // namespace std::base_math
