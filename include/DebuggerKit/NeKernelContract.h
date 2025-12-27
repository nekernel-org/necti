// Copyright 2024-2025, Amlal El Mahrouss (amlal@nekernel.org)
// Licensed under the Apache License, Version 2.0 (See accompanying
// file LICENSE or copy at http://www.apache.org/licenses/LICENSE-2.0)
// Official repository: https://github.com/nekernel-org/nectar

#ifndef DK_NEKERNEL_CONTRACT_H
#define DK_NEKERNEL_CONTRACT_H

/// @brief NeKernel Debugging Protocol
/// @author Amlal El Mahrouss

#ifdef DK_NEKERNEL_DEBUGGER

#include <CompilerKit/Detail/Config.h>
#include <DebuggerKit/DebuggerContract.h>

namespace DebuggerKit::NeKernel {
class NeKernelContract;

/// =========================================================== ///
/// \brief NeKernel Debugger Contract
/// \author Amlal El Mahrouss
/// =========================================================== ///
class NeKernelContract final DK_DEBUGGER_CONTRACT {
 public:
  NeKernelContract();
  virtual ~NeKernelContract() override;

 public:
  NeKernelContract& operator=(const NeKernelContract&) = default;
  NeKernelContract(const NeKernelContract&)            = default;

 public:
  bool Attach(CompilerKit::STLString path, CompilerKit::STLString arg_v,
              ProcessID& pid) noexcept override;
  bool BreakAt(CompilerKit::STLString symbol) noexcept override;
  bool Break() noexcept override;
  bool Continue() noexcept override;
  bool Detach() noexcept override;

 private:
  CompilerKit::STLString m_kernel_path{};
  Detail::dk_socket_type m_socket{0};
};
}  // namespace DebuggerKit::NeKernel

#endif  // ifdef DK_NEKERNEL_DEBUGGER

#endif  // DK_NEKERNEL_CONTRACT_H
