/***
  DebuggerKit
  (C) 2025 Amlal El Mahrouss
  File: NeKernelContract.cc
  Purpose: NeKernel Debugger
*/

#ifdef LD_NEKERNEL_DEBUGGER

#include <CompilerKit/Defines.h>
#include <DebuggerKit/NeKernelContract.h>
#include <ThirdParty/Dialogs.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

constexpr static UInt16 kDebugPort = 51820;

using namespace DebuggerKit::NeKernel;

NeKernelContract::NeKernelContract() = default;

NeKernelContract::~NeKernelContract() = default;

BOOL NeKernelContract::Attach(CompilerKit::STLString path, CompilerKit::STLString argv,
                              ProcessID& pid) noexcept {
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

#endif  // LD_NEKERNEL_DEBUGGER