// Copyright 2024-2025, Amlal El Mahrouss (amlal@nekernel.org)
// Licensed under the Apache License, Version 2.0 (See accompanying
// file LICENSE or copy at http://www.apache.org/licenses/LICENSE-2.0)
// Official repository: https://github.com/ne-foss-org/nectar

#ifndef COMPILERKIT_CONFIG_PRECONFIG_H
#define COMPILERKIT_CONFIG_PRECONFIG_H

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

#include <signal.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>
#include <cassert>
#include <filesystem>
#include <fstream>
#include <string>

#ifdef __linux__
#define isnumber isdigit
#endif

#define ToString(X) Stringify(X)
#define Stringify(X) #X

#define kDistRelease ToString(kDistReleaseBranch)

#define kDistVersion "v0.1.3-compilerkit"
#define kDistVersionBCD 0x0103

#define ToString(X) Stringify(X)
#define Stringify(X) #X

#define kDistRelease ToString(kDistReleaseBranch)

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

#define NECTAR_MODULE(name) extern "C" int name(int argc, char** argv)

#ifdef MSVC
#pragma scalar_storage_order big - endian
#endif  // ifdef MSVC

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

#define CK_IMPORT_CXX extern "C++"
#define CK_IMPORT_C extern "C"
#define CK_IMPORT extern

#endif  // COMPILERKIT_CONFIG_PRECONFIG_H
