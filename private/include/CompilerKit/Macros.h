// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026, Amlal El Mahrouss (amlal@nekernel.org)
// Licensed under the Apache License, Version 2.0 (See accompanying
// file LICENSE or copy at http://www.apache.org/licenses/LICENSE-2.0)
// Official repository: https://github.com/ne-app-eu/ncc

/// @brief provide support for the CK headers.

#ifndef _NCC_MACROS_H_
#define _NCC_MACROS_H_

#define NCC_COPY_DELETE(KLASS)          \
  KLASS& operator=(const KLASS&) = delete; \
  KLASS(const KLASS&)            = delete;

#define NCC_COPY_DEFAULT(KLASS)          \
  KLASS& operator=(const KLASS&) = default; \
  KLASS(const KLASS&)            = default;

#define NCC_MOVE_DELETE(KLASS)     \
  KLASS& operator=(KLASS&&) = delete; \
  KLASS(KLASS&&)            = delete;

#define NCC_MOVE_DEFAULT(KLASS)     \
  KLASS& operator=(KLASS&&) = default; \
  KLASS(KLASS&&)            = default;

#endif /* ifndef _NCC_MACROS_H_ */
