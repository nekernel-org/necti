// Copyright 2025-2026, Amlal El Mahrouss (amlal@nekernel.org)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
// Official repository: https://github.com/nekernel-org/nectar

/// @author Amlal El Mahrouss

#include <CompilerKit/Detail/Config.h>
#include <gtest/gtest.h>

CK_IMPORT_C Int32 DynamicLinker64PEF(Int32 argc, Char** argv);

static Int32  kArgc{};
static Char** kArgv{};

Int32 main(Int32 argc, Char** argv) {
  ::testing::InitGoogleTest(&argc, argv);

  kArgc = argc;
  kArgv = argv;

  return RUN_ALL_TESTS();
}

TEST(LinkerRun, LinkerExitsCorrectly) {
  EXPECT_TRUE(kArgc > 1);
  EXPECT_TRUE(DynamicLinker64PEF(kArgc, kArgv) == 0) << "Linker invocation failed";
}
