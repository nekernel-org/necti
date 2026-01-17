// Copyright 2026, Amlal El Mahrouss (amlal@nekernel.org)
// Licensed under the Apache License, Version 2.0 (See accompanying
// file LICENSE or copy at http://www.apache.org/licenses/LICENSE-2.0)
// Official repository: https://github.com/nekernel-org/nectar

#ifndef _NECTAR_MACHO_H_
#define _NECTAR_MACHO_H_

#include <CompilerKit/Detail/Config.h>

#include <mach-o/ldsyms.h>
#include <mach-o/loader.h>
#include <mach-o/nlist.h>

namespace CompilerKit {
namespace MachO {

  /// @brief Mach-O segment names
  constexpr const char* kSegmentText     = "__TEXT";
  constexpr const char* kSegmentData     = "__DATA";
  constexpr const char* kSegmentPageZero = "__PAGEZERO";

  /// @brief Mach-O section names
  constexpr const char* kSectionText = "__text";
  constexpr const char* kSectionData = "__data";
  constexpr const char* kSectionBss  = "__bss";

  /// @brief Default base address for Mach-O executables
  constexpr uint64_t kDefaultBaseAddress = 0x100000000ULL;

  /// @brief Page size for alignment
  constexpr uint64_t kPageSize = 0x4000ULL;  // 16KB for arm64, also works for x86_64

  /// @brief Section alignment (2^4 = 16 bytes)
  constexpr uint32_t kSectionAlign = 4;

  /// @brief Helper to align a value to page boundary
  inline uint64_t AlignToPage(uint64_t value) { return (value + kPageSize - 1) & ~(kPageSize - 1); }

  /// @brief Helper to copy segment/section name safely
  inline void CopySegmentName(char* dest, const char* src) {
    std::memset(dest, 0, 16);
    std::strncpy(dest, src, 16);
  }

}  // namespace MachO
}  // namespace CompilerKit

#endif  // ifndef _NECTAR_MACHO_H_
