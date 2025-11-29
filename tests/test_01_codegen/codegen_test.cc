/* ========================================

   Copyright (C) 2024-2025 Amlal El Mahrouss, Licensed under the Apache 2.0 license

   ======================================== */

/// @brief Codegen Unit test, from the C++ unit to the final executable.
/// @author Amlal El Mahrouss

#include <gtest/gtest.h>

TEST(CodegenTest, BasicCodegenTest) {
  /// compile asm
  {
    auto expr = std::system("asm sample/sample.asm");
    EXPECT_TRUE(expr == 0) << "ASM Driver did not compile the easy ASM unit.";
  }
}
