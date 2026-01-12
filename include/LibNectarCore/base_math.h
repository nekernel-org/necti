// Copyright 2024-2025, Amlal El Mahrouss (amlal@nekernel.org)
// Licensed under the Apache License, Version 2.0 (See accompanying
// file LICENSE or copy at http://www.apache.org/licenses/LICENSE-2.0)
// Official repository: https://github.com/nekernel-org/nectar

#ifndef NECTAR_LIBCXX_BASE_MATH_H
#define NECTAR_LIBCXX_BASE_MATH_H

#include <LibNectarCore/defines.h>

#ifndef NAN
#define NAN (__builtin_nanf(""))
#endif  // !NAN

/// @file base_math.h
/// @brief Base Mathematic functions.

#ifdef __LIBCXX_USE_DOUBLE__
typedef double real_type;
#else
typedef float real_type;
#endif

inline constexpr static auto not_a_number = NAN;

/// =========================================================== ///
/// @brief Power of Exponent function.
/// =========================================================== ///
template <size_t Exponent>
inline real_type pow(real_type in) {
  if (Exponent == 0) return 1;   // Any number to the power of 0 is 1.
  if (Exponent == 1) return in;  // Any number to the power of 1 is itself.

  real_type result = 1;

  for (auto i = 0UL; i < Exponent; ++i) result *= in;

  return result;
}

/// =========================================================== ///
/// @brief Square root function.
/// =========================================================== ///
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

/// =========================================================== ///
/// @brief Square of function, with Base template argument.
/// @param of Base argument to find the square of.
/// =========================================================== ///
template <size_t Base>
inline real_type surd(real_type in) {
  if (in == 0) return 0;
  if (in == 1) return 1;

  if (Base == 1) return in;
  if (Base == 2) return sqrt(in);

  return not_a_number;
}

/// =========================================================== ///
/// @brief Linear interpolation equation solver.
/// @param from where?
/// @param to to?
/// @param Updated diff value according to difference.
/// =========================================================== ///
inline real_type lerp(real_type to, real_type from, real_type stat) {
  real_type diff = (to - from);
  return from + (diff * stat);
}

using real_domain = double;

struct complex_domain final {
  double Re;
  double Im;
};

typename<class Result> using callable_type = Result (*)(size_t n, ...);

#ifdef __cpp_lib_base_math
#define __cpp_lib_base_math 1
#endif

#endif  // NECTAR_LIBCXX_BASE_MATH_H
