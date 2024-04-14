/* -------------------------------------------

    Copyright Mahrouss Logic

------------------------------------------- */

//! provide support for CompilerKit.hpp header.

#ifndef _CK_CL_HPP
#define _CK_CL_HPP

#define MPCC_COPY_DELETE(KLASS)             \
  KLASS &operator=(const KLASS &) = delete; \
  KLASS(const KLASS &) = delete;

#define MPCC_COPY_DEFAULT(KLASS)             \
  KLASS &operator=(const KLASS &) = default; \
  KLASS(const KLASS &) = default;

#define MPCC_MOVE_DELETE(KLASS)        \
  KLASS &operator=(KLASS &&) = delete; \
  KLASS(KLASS &&) = delete;

#define MPCC_MOVE_DEFAULT(KLASS)        \
  KLASS &operator=(KLASS &&) = default; \
  KLASS(KLASS &&) = default;

#endif /* ifndef _CK_CL_HPP */
