/* ========================================

  Copyright (C) 2024-2025 Amlal El Mahrouss, licensed under the Apache 2.0 license

======================================== */

#include <CompilerKit/CodeGenerator.h>

/**
 * @file AssemblyFactory.cc
 * @author Amlal El Mahrouss (amlal@nekernel.org)
 * @brief Assembly API of Nectar
 * @version 0.0.3
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
  } catch (...) {
    return NECTI_EXEC_ERROR;
  }
}

///! @brief mount assembly backend.
void AssemblyFactory::Mount(WeakRef<IAssembly> mountPtr) noexcept {
  if (mountPtr) {
    fMounted = mountPtr.Leak();
  }
}

///! @brief Unmount assembler.
WeakRef<IAssembly> AssemblyFactory::Unmount() noexcept {
  auto mount_prev = fMounted;

  if (fMounted) {
    fMounted = nullptr;
  }

  return WeakRef<IAssembly>{mount_prev};
}
}  // namespace CompilerKit
