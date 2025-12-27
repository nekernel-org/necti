// Copyright 2024-2025, Amlal El Mahrouss (amlal@nekernel.org)
// Licensed under the Apache License, Version 2.0 (See accompanying
// file LICENSE or copy at http://www.apache.org/licenses/LICENSE-2.0)
// Official repository: https://github.com/nekernel-org/nectar

/// @brief Codegen Unit test, from the C++ unit to the final executable.
/// @author Amlal El Mahrouss

#include <gtest/gtest.h>

TEST(CodegenTest, BasicCodegenTest) {
  auto expr = std::system("asm -asm-x64 test_samples/sample.asm");
  EXPECT_TRUE(expr == 0) << "ASM Driver did not compile the easy ASM unit.";
}
