/* -------------------------------------------

    Copyright SoftwareLabs

------------------------------------------- */

#include <Headers/AsmKit/AsmKit.hpp>
#include <Headers/StdKit/ErrorID.hpp>

/**
 * @file AsmKit.cc
 * @author Amlal El Mahrouss (amlal@mahrouss.com)
 * @brief Assembler Kit
 * @version 0.1
 * @date 2024-01-27
 *
 * @copyright Copyright (c) 2024, SoftwareLabs
 *
 */

#include <iostream>

//! @file AsmKit.cpp
//! @brief AssemblyKit source implementation.

namespace CompilerKit {
//! @brief Compile for specific format (ELF, PEF, ZBIN)
Int32 AssemblyFactory::Compile(std::string& sourceFile,
                               const Int32& arch) noexcept {
  if (sourceFile.length() < 1 || !fMounted) return MPCC_UNIMPLEMENTED;

  return fMounted->CompileToFormat(sourceFile, arch);
}

//! @brief mount assembly backend.
void AssemblyFactory::Mount(AssemblyInterface* mountPtr) noexcept {
  if (mountPtr) {
    fMounted = mountPtr;
  }
}

AssemblyInterface* AssemblyFactory::Unmount() noexcept {
  auto mount_prev = fMounted;

  if (mount_prev) {
    fMounted = nullptr;
  }

  return mount_prev;
}
}  // namespace CompilerKit
