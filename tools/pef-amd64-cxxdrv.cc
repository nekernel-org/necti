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

static auto kPath   = "/usr/local/lib/libCompilerKit.dylib";
static auto kSymbol = "CompilerCPlusPlusAMD64";

Int32 main(Int32 argc, Char const* argv[]) {
  CompilerKitDylib handler = dlopen(kPath, RTDK_LAZY | RTDK_GLOBAL);

  if (!handler) {
    kStdOut;
    std::printf("error: Could not load dylib in %s: %s\n", kPath, dlerror());

    return EXIT_FAILURE;
  }

  CompilerKitEntrypoint entrypoint_cxx = (CompilerKitEntrypoint) dlsym(handler, kSymbol);

  if (!entrypoint_cxx) {
    kStdOut;
    std::printf("error: Could not find entrypoint in %s: %s\n", kPath, dlerror());
    dlclose(handler);

    return EXIT_FAILURE;
  }

  auto ret = (entrypoint_cxx(argc, argv) == NECTI_SUCCESS) ? EXIT_SUCCESS : EXIT_FAILURE;

  dlclose(handler);

  return ret;
}
