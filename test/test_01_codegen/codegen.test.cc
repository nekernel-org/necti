// Copyright 2024-2025, Amlal El Mahrouss (amlal@nekernel.org)
// Licensed under the Apache License, Version 2.0 (See accompanying
// file LICENSE or copy at http://www.apache.org/licenses/LICENSE-2.0)
// Official repository: https://github.com/nekernel-org/nectar

/// @brief Codegen Unit test, from the C++ unit to the final executable.
/// @author Amlal El Mahrouss

#include <gtest/gtest.h>

TEST(CodegenTest, BasicCodegenTestGrep) {
  // Compile C++ source to assembly
  auto compile_result = std::system("pef-amd64-cxxdrv ../test_samples/sample.nc > /dev/null 2>&1");
  EXPECT_TRUE(compile_result == 0) << "C++ compiler driver failed to compile sample.cc";

  // Grep for expected entry point symbol in generated assembly
  auto grep_main = std::system("grep -q '__NECTAR_main' ../test_samples/sample.nc.masm");
  EXPECT_TRUE(grep_main == 0) << "Generated assembly missing expected entry point __NECTAR_main";

  // Grep for return instruction
  auto grep_ret = std::system("grep -q 'ret' ../test_samples/sample.nc.masm");
  EXPECT_TRUE(grep_ret == 0) << "Generated assembly missing return instruction";

  // Grep for 64-bit mode directive
  auto grep_bits64 = std::system("grep -q '%bits 64' ../test_samples/sample.nc.masm");
  EXPECT_TRUE(grep_bits64 == 0) << "Generated assembly missing 64-bit mode directive";
}

TEST(CodegenTest, BasicCodegenTestAssemble) {
  auto expr = std::system("asm -asm-x64 test_samples/sample.asm");
  EXPECT_TRUE(expr == 0) << "ASM Driver did not compile the easy ASM unit.";
}
