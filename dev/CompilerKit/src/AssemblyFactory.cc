/* -------------------------------------------

  Copyright (C) 2024-2025 Amlal EL Mahrouss, all rights reserved

------------------------------------------- */

#include <CompilerKit/Compiler.h>
#include <CompilerKit/ErrorID.h>

/**
 * @file AssemblyFactory.cc
 * @author Amlal El Mahrouss (amlal@nekernel.org)
 * @brief Compiler API of NeCTI
 * @version 0.0.2
 *
 * @copyright Copyright (c) 2024-2025 Amlal El Mahrouss
 *
 */

namespace CompilerKit {
///! @brief Compile for specific format (ELF, PEF, ZBIN)
Int32 AssemblyFactory::Compile(STLString sourceFile, const Int32& arch) noexcept {
  if (sourceFile.length() < 1) return NECTI_UNIMPLEMENTED;

  if (!fMounted) return NECTI_UNIMPLEMENTED;
  if (arch != fMounted->Arch()) return NECTI_INVALID_ARCH;

  try {
    return this->fMounted->CompileToFormat(sourceFile, arch);
  } catch (std::exception& e) {
    return NECTI_EXEC_ERROR;
  }
}

///! @brief mount assembly backend.
void AssemblyFactory::Mount(AssemblyInterface* mountPtr) noexcept {
  if (mountPtr) {
    fMounted = mountPtr;
  }
}

///! @brief Unmount assembler.
AssemblyInterface* AssemblyFactory::Unmount() noexcept {
  auto mount_prev = fMounted;

  if (fMounted) {
    fMounted = nullptr;
  }

  return mount_prev;
}
}  // namespace CompilerKit
