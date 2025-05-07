/* -------------------------------------------

  Copyright (C) 2024-2025 Amlal EL Mahrous, all rights reserved

------------------------------------------- */

#include <LibCompiler/AssemblyInterface.h>
#include <LibCompiler/ErrorID.h>

/**
 * @file AssemblyFactory.cxx
 * @author amlal (amlal@nekernel.org)
 * @brief Assembler Kit
 * @version 0.1
 * @date 2024-01-27
 *
 * @copyright Copyright (c) 2024-2025 Amlal El Mahrouss
 *
 */

//! @file Asm.cpp
//! @brief AssemblyKit source implementation.

namespace LibCompiler {
///! @brief Compile for specific format (ELF, PEF, ZBIN)
Int32 AssemblyFactory::Compile(std::string sourceFile, const Int32& arch) noexcept {
  if (sourceFile.length() < 1 || !fMounted) return LIBCOMPILER_UNIMPLEMENTED;

  if (arch != fMounted->Arch()) return LIBCOMPILER_INVALID_ARCH;

  return fMounted->CompileToFormat(sourceFile, arch);
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

  if (mount_prev) {
    fMounted = nullptr;
  }

  return mount_prev;
}
}  // namespace LibCompiler
