// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026, Amlal El Mahrouss (amlal@nekernel.org)
// Licensed under the Apache License, Version 2.0 (See accompanying
// file LICENSE or copy at http://www.apache.org/licenses/LICENSE-2.0)
// Official repository: https://github.com/ne-foss/nectar

#include <CompilerKit/CodeGenerator.h>

/**
 * @file AssemblyFactory.cpp
 * @author Amlal El Mahrouss (amlal@nekernel.org)
 * @brief Nectar Code Generation API
 * @version 0.0.3
 * @copyright Copyright (c) 2024-2026 Amlal El Mahrouss
 *
 */

namespace CompilerKit {

///! @brief Compile for specific format (ELF, PEF, AE)
Int32 AssemblyFactory::Compile(STLString source_file, const Int32& arch) {
  if (source_file.length() == 0) return NECTAR_UNIMPLEMENTED;

  if (!this->fMounted) return NECTAR_UNIMPLEMENTED;
  if (arch != this->fMounted->Arch()) return NECTAR_INVALID_ARCH;

  if (!std::filesystem::is_regular_file(source_file)) return NECTAR_UNIMPLEMENTED;

  auto compiled_unit = source_file + ".ignore";

  try {
    std::filesystem::copy(source_file, compiled_unit);
    auto ret = this->fMounted->CompileToFormat(compiled_unit, arch);

    std::filesystem::remove(compiled_unit);
    return ret;
  } catch (...) {
    std::filesystem::remove(compiled_unit);
  }

  return NECTAR_INVALID_DATA;
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
