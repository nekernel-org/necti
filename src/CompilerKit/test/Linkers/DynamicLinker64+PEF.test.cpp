// SPDX-License-Identifier: Apache-2.0
// Copyright 2025-2026, Amlal El Mahrouss (amlal@nekernel.org)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
// Official repository: https://github.com/ne-foss-org/nectar

/// @author Amlal El Mahrouss

#include <CompilerKit/Detail/Config.h>
#include <CompilerKit/ErrorID.h>
#include <CompilerKit/Utilities/Compiler.h>
#include <CompilerKit/Utilities/DLL.h>

#include <gtest/gtest.h>

static Int32  argc{};
static char** argv{};

Int32 main(Int32 _argc, char** _argv) {
  ::testing::InitGoogleTest(&_argc, _argv);

  argc = _argc;
  argv = _argv;

  return RUN_ALL_TESTS();
}

#ifdef __APPLE__
static auto kPath = "/usr/local/lib/libCompilerKit.dylib";
#else
static auto kPath = "/usr/lib/libCompilerKit.so";
#endif

static auto kSymbol = "DynamicLinker64PEF";

TEST(LinkerRunMachO, LinkerExitsCorrectly) {
  CompilerKit::ModuleLoader dylib;
  dylib(kPath, kSymbol);

  CompilerKit::ModuleLoader::EntryT entrypoint_cxx =
      reinterpret_cast<CompilerKit::ModuleLoader::EntryT>(dylib.fEntrypoint);

  if (!entrypoint_cxx) {
    kStdOut;
    std::printf("error: Could not find entrypoint in %s: %s\n", kPath, dlerror());

    return;
  }

  auto ret =(entrypoint_cxx(argc, const_cast<const char**>(argv)) == NECTAR_SUCCESS) ? EXIT_SUCCESS : EXIT_FAILURE;
  EXPECT_TRUE(ret == EXIT_SUCCESS);
}
