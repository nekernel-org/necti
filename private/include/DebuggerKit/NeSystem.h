// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026, Amlal El Mahrouss (amlal@nekernel.org)
// Licensed under the Apache License, Version 2.0 (See accompanying
// file LICENSE or copy at http://www.apache.org/licenses/LICENSE-2.0)
// Official repository: https://github.com/ne-foss-org/nectar

#ifndef DK_NEKERNEL_CONTRACT_H
#define DK_NEKERNEL_CONTRACT_H

/// @brief NeSystem Debugging Protocol
/// @author Amlal El Mahrouss

#ifdef DK_NEKERNEL_DEBUGGER

#include <CompilerKit/Detail/Config.h>
#include <DebuggerKit/IDebugger.h>

namespace DebuggerKit::NeSystem {
class NeSystemDebugger;

/// =========================================================== ///
/// \brief NeSystem Debugger Contract
/// \author Amlal El Mahrouss
/// =========================================================== ///
class NeSystemDebugger DK_DEBUGGER_CONTRACT {
 public:
  NeSystemDebugger();
  virtual ~NeSystemDebugger() override;

 public:
  NeSystemDebugger& operator=(const NeSystemDebugger&) = default;
  NeSystemDebugger(const NeSystemDebugger&)            = default;

 public:
  bool Attach(const CompilerKit::STLString& path, const CompilerKit::STLString& arg_v,
              ProcessID& pid) noexcept override;
  bool BreakAt(const CompilerKit::STLString& symbol) noexcept override;
  bool Break() noexcept override;
  bool Continue() noexcept override;
  bool Detach() noexcept override;

 private:
  CompilerKit::STLString m_kernel_path{};
  Detail::dk_socket_type m_socket{0};
};
}  // namespace DebuggerKit::NeSystem

#endif  // ifdef DK_NEKERNEL_DEBUGGER

#endif  // DK_NEKERNEL_CONTRACT_H
