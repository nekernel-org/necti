/* -------------------------------------------

  Copyright (C) 2024-2025 Amlal EL Mahrouss, all rights reserved

------------------------------------------- */

/// @file cxxdrv.cc
/// @brief NE C++ frontend compiler.

#include <CompilerKit/Defines.h>
#include <CompilerKit/ErrorID.h>
#include <CompilerKit/Version.h>
#include <CompilerKit/utils/CompilerUtils.h>
#include <CompilerKit/utils/DylibHelpers.h>

#ifdef __APPLE__
static auto kPath = "/usr/local/lib/libCompilerKit.dylib";
#else
static auto kPath = "/usr/lib/libCompilerKit.so";
#endif

static auto kSymbol = "CompilerCPlusPlusAMD64";

Int32 main(Int32 argc, Char const* argv[]) {
  CompilerKitDylibTraits dylib;
  dylib(kPath, kSymbol);

  CompilerKitEntrypoint entrypoint_cxx = (CompilerKitEntrypoint) dylib.fEntrypoint;

  if (!entrypoint_cxx) {
    kStdOut;
    std::printf("error: Could not find entrypoint in %s: %s\n", kPath, dlerror());

    return EXIT_FAILURE;
  }

  auto ret = (entrypoint_cxx(argc, argv) == NECTI_SUCCESS) ? EXIT_SUCCESS : EXIT_FAILURE;

  return ret;
}
