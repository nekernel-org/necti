/* ========================================

  Copyright (C) 2024-2025 Amlal El Mahrouss, Licensed under the Apache 2.0 license

======================================== */

#ifndef __COMPILERKIT_PRECONFIG_H__
#define __COMPILERKIT_PRECONFIG_H__

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

#define SizeType size_t

#define VoidPtr void*
#define voidPtr VoidPtr

#define UIntPtr uintptr_t

#define Int64 int64_t
#define UInt64 uint64_t

#define Int32 int
#define UInt32 unsigned

#define Int16 int16_t
#define UInt16 uint16_t

#define Int8 int8_t
#define UInt8 uint8_t

#define Char char

#include <signal.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>
#include <cassert>
#include <string>

#define kDistVersion "v0.0.7-compilerkit"
#define kDistVersionBCD 0x0002

#define ToString(X) Stringify(X)
#define Stringify(X) #X

#define kDistRelease ToString(kDistReleaseBranch)

#ifndef kDistRelease

#define kDistVersion "v0.0.7-compilerkit"
#define kDistVersionBCD 0x0002

#define ToString(X) Stringify(X)
#define Stringify(X) #X

#define kDistRelease ToString(kDistReleaseBranch)

#endif  // !kDistRelease

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

#define kAsmFileExts {".64x", ".32x", ".masm", ".s", ".S", ".asm", ".x64"}

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

#define CK_IMPORT_CXX extern "C++"
#define CK_IMPORT_C extern "C"
#define CK_IMPORT extern

#endif  // __COMPILERKIT_PRECONFIG_H__