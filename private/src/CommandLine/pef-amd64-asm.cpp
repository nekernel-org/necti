// Copyright 2024-2025, Amlal El Mahrouss (amlal@nekernel.org)
// Licensed under the Apache License, Version 2.0 (See accompanying
// file LICENSE or copy at http://www.apache.org/licenses/LICENSE-2.0)
// Official repository: https://github.com/ne-foss-org/nectar

/// @file pef-amd64-asm.cc
/// @brief Nectar C++ frontend compiler for AMD64.

#include <CompilerKit/Detail/Config.h>
#include <CompilerKit/ErrorID.h>
#include <CompilerKit/Utils/Compiler.h>
#include <CompilerKit/Utils/DLL.h>

#ifdef __APPLE__
static auto kPath = "/usr/local/lib/libCompilerKit.dylib";
#else
static auto kPath = "/usr/lib/libCompilerKit.so";
#endif

static auto kSymbol = "AssemblerMainAMD64";

Int32 main(Int32 argc, char const* argv[]) {
  CompilerKit::ModuleLoader dylib;
  dylib(kPath, kSymbol);

  CompilerKit::ModuleLoader::EntryT entrypoint_cxx =
      reinterpret_cast<CompilerKit::ModuleLoader::EntryT>(dylib.fEntrypoint);

  if (!entrypoint_cxx) {
    kStdOut;
    std::printf("error: Could not find entrypoint in %s: %s\n", kPath, dlerror());

    return EXIT_FAILURE;
  }

  return (entrypoint_cxx(argc, argv) == NECTAR_SUCCESS) ? EXIT_SUCCESS : EXIT_FAILURE;
}
