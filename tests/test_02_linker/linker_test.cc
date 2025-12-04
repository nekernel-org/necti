/* ========================================

   Copyright (C) 2024-2025 Amlal El Mahrouss, Licensed under the Apache 2.0 license

   ======================================== */

/// @brief Linker Unit test, from the C++ unit to the final executable.
/// @author Amlal El Mahrouss

#include <gtest/gtest.h>

/// c++
TEST(LinkerTest, BasicLinkTest) {
  auto expr = std::system("pef-amd64-cxxdrv test_samples/sample.cc");
  EXPECT_TRUE(expr == 0) << "C++ Driver did not compile the easy C++ unit.";
}

/// assemble
TEST(LinkerTest, BasicLinkTest2) {
  auto expr = std::system("asm -asm:x64 test_samples/sample.cc.pp.masm");
  EXPECT_TRUE(expr == 0) << "Assembler did not assemble the easy asm unit.";
}

/// link
TEST(LinkerTest, BasicLinkTest3) {
  auto expr =
      std::system("ld64 -amd64 test_samples/sample.cc.pp.obj -start __NECTI_main -output main.exec");
  EXPECT_TRUE(expr == 0) << "Linker did not link the easy object.";
}
