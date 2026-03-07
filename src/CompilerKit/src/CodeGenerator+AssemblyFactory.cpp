// Copyright 2024-2026, Amlal El Mahrouss (amlal@nekernel.org)
// Licensed under the Apache License, Version 2.0 (See accompanying
// file LICENSE or copy at http://www.apache.org/licenses/LICENSE-2.0)
// Official repository: https://github.com/ne-foss-org/nectar

#include <CompilerKit/CodeGenerator.h>

/**
 * @file AssemblyFactory.cc
 * @author Amlal El Mahrouss (amlal@nekernel.org)
 * @brief Nectar Code Generation API
 * @version 0.0.3
 * @copyright Copyright (c) 2024-2026 Amlal El Mahrouss
 *
 */

namespace CompilerKit {
///! @brief Compile for specific format (ELF, PEF, AE)
Int32 AssemblyFactory::Compile(STLString sourceFile, const Int32& arch) {
  if (sourceFile.length() == 0) return NECTAR_UNIMPLEMENTED;

  if (!this->fMounted) return NECTAR_UNIMPLEMENTED;
  if (arch != this->fMounted->Arch()) return NECTAR_INVALID_ARCH;

  return this->fMounted->CompileToFormat(sourceFile, arch);
}

///! @brief mount assembly backend.
void AssemblyFactory::Mount(WeakRef<IAssembly> mount_ptr) {
  if (mount_ptr.Leak() && !this->fMounted) {
    this->fMounted = mount_ptr.Leak();
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
