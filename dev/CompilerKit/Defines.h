/* -------------------------------------------

  Copyright (C) 2024-2025 Amlal EL Mahrouss, all rights reserved

------------------------------------------- */

#ifndef __NECTI_DEFINES_H__
#define __NECTI_DEFINES_H__

#ifndef Yes
#define Yes true
#endif  // ifndef Yes

#ifndef No
#define No false
#endif  // ifndef No

#ifndef YES
#define YES true
#endif  // ifndef YES

#ifndef NO
#define NO false
#endif  // ifndef NO

#ifndef BOOL
#define BOOL bool
#endif  // ifndef BOOL

#define SizeType size_t

#define VoidPtr void*
#define voidPtr VoidPtr

#define UIntPtr uintptr_t

#define Int64 int64_t
#define UInt64 uint64_t

#define Int32 int
#define UInt32 unsigned

#define Bool bool

#define Int16 int16_t
#define UInt16 uint16_t

#define Int8 int8_t
#define UInt8 uint8_t

#define Char char
#define Boolean bool

#include <signal.h>
#include <unistd.h>
#include <cassert>
#include <cctype>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <new>
#include <string>
#include <utility>
#include <vector>

#define nullPtr std::nullptr_t

#define MUST_PASS(E) assert(E)

#ifndef __FORCE_STRLEN
#define __FORCE_STRLEN 1

#define string_length(len) strlen(len)
#endif

#ifndef __FORCE_MEMCPY
#define __FORCE_MEMCPY 1

#define rt_copy_memory(dst, src, len) memcpy(dst, src, len)
#endif

#define ATTRIBUTE(X) __attribute__((X))
#define PACKED ATTRIBUTE(packed)

#define kObjectFileExt ".obj"
#define kBinaryFileExt ".bin"

#define kAsmFileExts \
  { ".64x", ".32x", ".masm", ".s", ".S", ".asm", ".x64" }

#define kAsmFileExtsMax (7U)

#define NECTI_MODULE(name) extern "C" int name(int argc, char** argv)

#ifdef MSVC
#pragma scalar_storage_order big - endian
#endif  // ifdef MSVC

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

#define CK_IMPORT_C extern "C"
#define CK_IMPORT extern
namespace CompilerKit {
inline constexpr int kBaseYear = 1900;

typedef std::string STLString;

inline STLString current_date() noexcept {
  auto time_data   = time(nullptr);
  auto time_struct = gmtime(&time_data);

  STLString fmt = std::to_string(kBaseYear + time_struct->tm_year);

  fmt += "-";
  fmt += std::to_string(time_struct->tm_mon + 1);
  fmt += "-";
  fmt += std::to_string(time_struct->tm_mday);

  return fmt;
}

inline bool to_str(Char* str, Int32 limit, Int32 base) noexcept {
  if (limit == 0) return false;

  Int32 copy_limit = limit;
  Int32 cnt        = 0;
  Int32 ret        = base;

  while (limit != 1) {
    ret      = ret % 10;
    str[cnt] = ret;

    ++cnt;
    --limit;
    --ret;
  }

  str[copy_limit] = '\0';
  return true;
}

inline bool install_signal(Int32 signal, void (*handler)(int)) noexcept {
  if (handler == nullptr) return false;

  if (::signal(signal, handler) == SIG_ERR) {
    return false;
  }

  return true;
}
}  // namespace CompilerKit

#endif /* ifndef __NECTI_DEFINES_H__ */
