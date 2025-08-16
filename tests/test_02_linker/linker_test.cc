/* -------------------------------------------

   Copyright (C) 2024-2025 Amlal EL Mahrouss, all rights reserved

   ------------------------------------------- */


/// @brief Linker Unit test, from the C++ unit to the final executable.
/// @author Amlal El Mahrouss

#include <gtest/gtest.h>

TEST(LinkerTest, BasicLinkTest)
{
  /// @note this is the driver, it will look for a .cc.pp (.pp stands for pre-processed)
  auto expr = std::system("pef-amd64-cxxdrv sample/sample.cc");
  EXPECT_TRUE(expr == 0) << "C++ Driver did not compile the easy C++ unit.";

  expr = std::system("asm -asm:x64 sample/sample.cc.pp.masm");
  EXPECT_TRUE(expr == 0) << "Assembler did not assemble the easy asm unit.";

  expr = std::system("ld64 -amd64 sample/sample.cc.pp.obj -start __NECTI_main -output main.exec");
  EXPECT_TRUE(expr == 0) << "Linker did not link the easy object.";
}
