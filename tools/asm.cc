/* -------------------------------------------

  Copyright (C) 2024-2025 Amlal EL Mahrouss, all rights reserved

------------------------------------------- */

/// @file asm.cc
/// @brief Assembler frontend.

#include <CompilerKit/Defines.h>
#include <CompilerKit/Version.h>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>

CK_IMPORT_C int AssemblerMainPower64(int argc, char const* argv[]);
CK_IMPORT_C int AssemblerMainARM64(int argc, char const* argv[]);
CK_IMPORT_C int AssemblerMain64x0(int argc, char const* argv[]);
CK_IMPORT_C int AssemblerMainAMD64(int argc, char const* argv[]);

enum AsmKind : Int32 {
  kInvalidAssembler = 0,
  kX64Assembler     = 100,
  k64X0Assembler,
  kPOWER64Assembler,
  kARM64Assembler,
  kAssemblerCount,
};

int main(int argc, char const* argv[]) {
  std::vector<const char*> arg_vec_cstr;
  arg_vec_cstr.push_back(argv[0]);

  const Int32 kInvalidAssembler = -1;
  Int32       asm_type          = kInvalidAssembler;

  for (size_t index_arg = 1; index_arg < argc; ++index_arg) {
    if (strstr(argv[index_arg], "-asm:h")) {
      std::printf("asm: Frontend Assembler (64x0, power64, arm64, x64).\n");
      std::printf("asm: Version: %s, Release: %s.\n", kDistVersion, kDistRelease);
      std::printf(
          "asm: Designed by Amlal El Mahrouss, Copyright (C) 2024-2025 Amlal El Mahrouss, all "
          "rights reserved.\n");
      std::printf(
          "CompilerKit: Designed by Amlal El Mahrouss, Copyright (C) 2024-2025 Amlal El Mahrouss, "
          "all rights reserved.\n");

      return 0;
    } else if (strstr(argv[index_arg], "-asm:x64")) {
      asm_type = kX64Assembler;
    } else if (strstr(argv[index_arg], "-asm:aarch64")) {
      asm_type = kARM64Assembler;
    } else if (strstr(argv[index_arg], "-asm:64x0")) {
      asm_type = k64X0Assembler;
    } else if (strstr(argv[index_arg], "-asm:power64")) {
      asm_type = kPOWER64Assembler;
    } else {
      arg_vec_cstr.push_back(argv[index_arg]);
    }
  }

  switch (asm_type) {
    case kPOWER64Assembler: {
      if (int32_t code = AssemblerMainPower64(arg_vec_cstr.size(), arg_vec_cstr.data()); code) {
        std::printf("asm: frontend exited with code %i.\n", code);
        return code;
      }
      break;
    }
    case k64X0Assembler: {
      if (int32_t code = AssemblerMain64x0(arg_vec_cstr.size(), arg_vec_cstr.data()); code) {
        std::printf("asm: frontend exited with code %i.\n", code);
        return code;
      }
      break;
    }
    case kARM64Assembler: {
      if (int32_t code = AssemblerMainARM64(arg_vec_cstr.size(), arg_vec_cstr.data()); code) {
        std::printf("asm: frontend exited with code %i.\n", code);
        return code;
      }
      break;
    }
    case kX64Assembler: {
      if (int32_t code = AssemblerMainAMD64(arg_vec_cstr.size(), arg_vec_cstr.data()); code) {
        std::printf("asm: frontend exited with code %i.\n", code);
        return code;
      }
      break;
    }
    default: {
      return EXIT_FAILURE;
    }
  }

  return EXIT_SUCCESS;
}
