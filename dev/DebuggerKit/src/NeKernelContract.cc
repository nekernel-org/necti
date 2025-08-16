/***
  DebuggerKit
  (C) 2025 Amlal El Mahrouss
  File: NeKernelContract.cc
  Purpose: NeKernel Debugger
*/

#ifdef DK_NEKERNEL_DEBUGGER

/// @author Amlal El Mahrouss
/// @brief Kernel Debugger Protocol

#include <CompilerKit/Defines.h>
#include <DebuggerKit/NeKernelContract.h>
#include <ThirdParty/Dialogs.h>

#include <DebuggerKit/Platform.h>

using namespace DebuggerKit::NeKernel;

NeKernelContract::NeKernelContract() = default;

NeKernelContract::~NeKernelContract() = default;

BOOL NeKernelContract::Attach(CompilerKit::STLString path, CompilerKit::STLString argv,
                              ProcessID& pid) noexcept {
  if (path.empty() || argv.empty()) return NO;
  return NO;
}

BOOL NeKernelContract::BreakAt(CompilerKit::STLString symbol) noexcept {
  return NO;
}

BOOL NeKernelContract::Break() noexcept {
  return NO;
}

BOOL NeKernelContract::Continue() noexcept {
  return NO;
}

BOOL NeKernelContract::Detach() noexcept {
  return NO;
}

#endif  // DK_NEKERNEL_DEBUGGER