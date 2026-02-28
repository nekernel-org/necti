// Copyright 2024-2026, Amlal El Mahrouss (amlal@nekernel.org)
// Licensed under the Apache License, Version 2.0 (See accompanying
// file LICENSE or copy at http://www.apache.org/licenses/LICENSE-2.0)
// Official repository: https://github.com/nekernel-org/nectar

/// @brief provide support for the CK headers.

#ifndef _NECTAR_MACROS_H_
#define _NECTAR_MACROS_H_

#define NECTAR_COPY_DELETE(KLASS)          \
  KLASS& operator=(const KLASS&) = delete; \
  KLASS(const KLASS&)            = delete;

#define NECTAR_COPY_DEFAULT(KLASS)          \
  KLASS& operator=(const KLASS&) = default; \
  KLASS(const KLASS&)            = default;

#define NECTAR_MOVE_DELETE(KLASS)     \
  KLASS& operator=(KLASS&&) = delete; \
  KLASS(KLASS&&)            = delete;

#define NECTAR_MOVE_DEFAULT(KLASS)     \
  KLASS& operator=(KLASS&&) = default; \
  KLASS(KLASS&&)            = default;

#endif /* ifndef _NECTAR_MACROS_H_ */
