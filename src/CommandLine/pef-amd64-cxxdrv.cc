/* ========================================

  Copyright (C) 2024-2025 Amlal El Mahrouss, licensed under the Apache 2.0 license

======================================== */

/// @file pef-amd64-cxxdrv.cc
/// @brief Nectar C++ frontend compiler for AMD64.

#include <CompilerKit/Detail/Config.h>
#include <CompilerKit/ErrorID.h>
#include <CompilerKit/Utilities/Compiler.h>
#include <CompilerKit/Utilities/DLL.h>

#ifdef __APPLE__
static auto kPath = "/usr/local/lib/libCompilerKit.dylib";
#else
static auto kPath = "/usr/lib/libCompilerKit.so";
#endif

static auto kSymbol = "CompilerCPlusPlusAMD64";

Int32 main(Int32 argc, Char const* argv[]) {
  CompilerKit::DLLLoader dylib;
  dylib(kPath, kSymbol);

  CompilerKit::DLLLoader::EntryT entrypoint_cxx =
      reinterpret_cast<CompilerKit::DLLLoader::EntryT>(dylib.fEntrypoint);

  if (!entrypoint_cxx) {
    kStdOut;
    std::printf("error: Could not find entrypoint in %s: %s\n", kPath, dlerror());

    return EXIT_FAILURE;
  }

  return (entrypoint_cxx(argc, argv) == NECTI_SUCCESS) ? EXIT_SUCCESS : EXIT_FAILURE;
}
