/* ========================================

  Copyright (C) 2024-2025 Amlal El Mahrouss, Licensed under the Apache 2.0 license

======================================== */

/// @file asm.cc
/// @brief Assembler frontend.

#include <CompilerKit/Detail/Config.h>
#include <cstring>
#include <vector>

CK_IMPORT_C Int32 AssemblerMainPower64(Int32 argc, Char const* argv[]);
CK_IMPORT_C Int32 AssemblerMainARM64(Int32 argc, Char const* argv[]);
CK_IMPORT_C Int32 AssemblerMain64x0(Int32 argc, Char const* argv[]);
CK_IMPORT_C Int32 AssemblerMainAMD64(Int32 argc, Char const* argv[]);

enum AsmKind : Int32 {
  kInvalidAssembler = 0,
  kX64Assembler     = 100,
  k64X0Assembler,
  kPOWER64Assembler,
  kARM64Assembler,
  kAssemblerCount = kARM64Assembler - kX64Assembler + 1,
};

Int32 main(Int32 argc, Char const* argv[]) {
  std::vector<const Char*> arg_vec_cstr;
  arg_vec_cstr.push_back(argv[0]);

  const Int32 kInvalidAssembler = -1;
  Int32       asm_type          = kInvalidAssembler;

  for (size_t index_arg = 1; index_arg < argc; ++index_arg) {
    if (strcmp(argv[index_arg], "-asm-h") == 0) {
      std::printf("asm: Frontend Assembler (64x0, power64, arm64, x64).\n");
      std::printf("asm: Version: %s, Release: %s.\n", kDistVersion, kDistRelease);
      std::printf(
          "asm: Designed by Amlal El Mahrouss, Copyright (C) 2024-2025 Amlal El Mahrouss, all "
          "rights reserved.\n");
      std::printf(
          "CompilerKit: Designed by Amlal El Mahrouss, Copyright (C) 2024-2025 Amlal El Mahrouss, "
          "Licensed under the Apache 2.0 license.\n");

      return 0;
    } else if (strcmp(argv[index_arg], "-asm-x64") == 0) {
      asm_type = kX64Assembler;
    } else if (strcmp(argv[index_arg], "-asm-aarch64") == 0) {
      asm_type = kARM64Assembler;
    } else if (strcmp(argv[index_arg], "-asm-64x0") == 0) {
      asm_type = k64X0Assembler;
    } else if (strcmp(argv[index_arg], "-asm-power64") == 0) {
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
