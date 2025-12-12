/* ========================================

   Copyright (C) 2024-2025 Amlal El Mahrouss, licensed under the Apache 2.0 license.

   ======================================== */

/// @brief Codegen Unit test, from the C++ unit to the final executable.
/// @author Amlal El Mahrouss

#include <gtest/gtest.h>

TEST(CodegenTest, BasicCodegenTest) {
  auto expr = std::system("asm -asm-x64 test_samples/sample.asm");
  EXPECT_TRUE(expr == 0) << "ASM Driver did not compile the easy ASM unit.";
}
