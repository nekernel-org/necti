/* ========================================

  Copyright (C) 2024-2025 Amlal El Mahrouss, licensed under the Apache 2.0 license

======================================== */

#include <CompilerKit/CodeGenerator.h>

/**
 * @file AssemblyFactory.cc
 * @author Amlal El Mahrouss (amlal@nekernel.org)
 * @brief Nectar Assembly API
 * @version 0.0.3
 * @copyright Copyright (c) 2024-2025 Amlal El Mahrouss
 *
 */

namespace CompilerKit {
///! @brief Compile for specific format (ELF, PEF, ZBIN)
Int32 AssemblyFactory::Compile(STLString sourceFile, const Int32& arch) {
  if (sourceFile.length() < 1) return NECTI_UNIMPLEMENTED;

  if (!this->fMounted) return NECTI_UNIMPLEMENTED;
  if (arch != this->fMounted->Arch()) return NECTI_INVALID_ARCH;

  return this->fMounted->CompileToFormat(sourceFile, arch);
}

///! @brief mount assembly backend.
void AssemblyFactory::Mount(WeakRef<IAssembly> mountPtr) {
  if (mountPtr && !this->fMounted) {
    this->fMounted = mountPtr.Leak();
  }
}

///! @brief Unmount assembler.
WeakRef<IAssembly> AssemblyFactory::Unmount() noexcept {
  auto mount_prev = fMounted;

  if (this->fMounted) {
    this->fMounted = nullptr;
  }

  return WeakRef<IAssembly>{mount_prev};
}
}  // namespace CompilerKit
