// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026, Amlal El Mahrouss (amlal@nekernel.org)
// Licensed under the Apache License, Version 2.0 (See accompanying
// file LICENSE or copy at http://www.apache.org/licenses/LICENSE-2.0)
// Official repository: https://github.com/ne-foss/nectar

/// @brief Codegen Unit test, from the C++ unit to the final executable.
/// @author Amlal El Mahrouss

#include <gtest/gtest.h>

TEST(CodegenTest, BasicCodegenTestGrep) {
  // Compile C++ source to assembly
  auto compile_result =
      std::system("pef-amd64-necdrv ../../../../snippets/test_snippets/inner.nc > /dev/null 2>&1");
  EXPECT_TRUE(compile_result == 0) << "C++ compiler driver failed to compile sample.cc";
}

TEST(CodegenTest, BasicCodegenTestAssemble) {
  auto expr =
      std::system("pef-amd64-asm ../../../../snippets/test_snippets/inner.masm > /dev/null 2>&1");
  EXPECT_TRUE(expr == 0) << "ASM Driver did not compile the easy ASM unit.";
}
