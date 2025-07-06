/* -------------------------------------------

  Copyright (C) 2024-2025 Amlal EL Mahrouss, all rights reserved

------------------------------------------- */

#include <LibCompiler/CodeGen.h>
#include <LibCompiler/ErrorID.h>

/**
 * @file CodeGen.cc
 * @author amlal (amlal@nekernel.org)
 * @brief Assembler Kit
 * @version 0.1
 * @date 2024-01-27
 *
 * @copyright Copyright (c) 2024-2025 Amlal El Mahrouss
 *
 */

namespace LibCompiler {
///! @brief Compile for specific format (ELF, PEF, ZBIN)
Int32 AssemblyFactory::Compile(STLString sourceFile, const Int32& arch) noexcept {
  if (sourceFile.length() < 1) return LIBCOMPILER_UNIMPLEMENTED;

  if (!fMounted) return LIBCOMPILER_UNIMPLEMENTED;
  if (arch != fMounted->Arch()) return LIBCOMPILER_INVALID_ARCH;

  try {
    return this->fMounted->CompileToFormat(sourceFile, arch);
  } catch (std::exception& e) {
    return LIBCOMPILER_EXEC_ERROR;
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
}  // namespace LibCompiler
