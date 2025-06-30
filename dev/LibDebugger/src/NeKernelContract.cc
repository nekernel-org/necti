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
#include <string>

using namespace LibDebugger::NeKernel;

NeKernelContract::NeKernelContract() = default;

NeKernelContract::~NeKernelContract() = default;

bool NeKernelContract::Attach(std::string path, std::string argv, ProcessID& pid) noexcept {
  return false;
}

bool NeKernelContract::Breakpoint(std::string symbol) noexcept {
  return false;
}

bool NeKernelContract::Break() noexcept {
  return false;
}

bool NeKernelContract::Continue() noexcept {
  return false;
}

bool NeKernelContract::Detach() noexcept {
  return false;
}

#endif  // LD_NEKERNEL_DEBUGGER