/* -------------------------------------------

  Copyright (C) 2024-2025 Amlal EL Mahrouss, all rights reserved

------------------------------------------- */

/// @file cxxdrv.cc
/// @brief NE C++ frontend compiler.

#include <LibCompiler/Defines.h>
#include <LibCompiler/ErrorID.h>
#include <LibCompiler/Util/CompilerUtils.h>
#include <LibCompiler/Util/DylibHelpers.h>
#include <LibCompiler/Version.h>

static auto kPath = "/usr/local/lib/libCompiler.dylib";
static auto kSymbol = "CompilerCPlusPlusAMD64";

Int32 main(Int32 argc, Char const* argv[]) {
  LibCompilerDylib handler = dlopen(kPath, RTLD_LAZY | RTLD_GLOBAL);

  if (!handler) {
    kStdOut;
    std::printf("error: Could not load dylib in %s: %s\n", kPath, dlerror());

    return EXIT_FAILURE;
  }

  LibCompilerEntrypoint entrypoint_cxx =
      (LibCompilerEntrypoint) dlsym(handler, kSymbol);

  if (!entrypoint_cxx) {
    kStdOut;
    std::printf("error: Could not find entrypoint in %s: %s\n", kPath, dlerror());
    dlclose(handler);

    return EXIT_FAILURE;
  }

  auto ret = (entrypoint_cxx(argc, argv) == LIBCOMPILER_SUCCESS) ? EXIT_SUCCESS : EXIT_FAILURE;

  dlclose(handler);

  return ret;
}
