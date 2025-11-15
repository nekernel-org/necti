/* -------------------------------------------

  Copyright (C) 2024-2025 Amlal EL Mahrouss, all rights reserved

------------------------------------------- */

/// @brief provide support for Macros.h header.

#ifndef _NECTI_MACROS_H_
#define _NECTI_MACROS_H_

#define NECTI_COPY_DELETE(KLASS)           \
  KLASS& operator=(const KLASS&) = delete; \
  KLASS(const KLASS&)            = delete;

#define NECTI_COPY_DEFAULT(KLASS)           \
  KLASS& operator=(const KLASS&) = default; \
  KLASS(const KLASS&)            = default;

#define NECTI_MOVE_DELETE(KLASS)      \
  KLASS& operator=(KLASS&&) = delete; \
  KLASS(KLASS&&)            = delete;

#define NECTI_MOVE_DEFAULT(KLASS)      \
  KLASS& operator=(KLASS&&) = default; \
  KLASS(KLASS&&)            = default;


#endif /* ifndef _NECTI_MACROS_H_ */
