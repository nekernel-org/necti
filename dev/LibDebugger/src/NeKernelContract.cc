/***
  LibDebugger
  (C) 2025 Amlal El Mahrouss
  File: NeKernelContract.cc
  Purpose: NeKernel Debugger
*/

#ifdef LD_NEKERNEL_DEBUGGER

#include <LibCompiler/Defines.h>
#include <LibDebugger/NeKernelContract.h>
#include <Vendor/Dialogs.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

constexpr static UInt16 kDebugPort = 51820;

using namespace LibDebugger::NeKernel;

NeKernelContract::NeKernelContract() = default;

NeKernelContract::~NeKernelContract() = default;

BOOL NeKernelContract::Attach(LibCompiler::STLString path, LibCompiler::STLString argv,
                              ProcessID& pid) noexcept {
  return NO;
}

BOOL NeKernelContract::BreakAt(LibCompiler::STLString symbol) noexcept {
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