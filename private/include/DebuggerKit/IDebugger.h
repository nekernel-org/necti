// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026, Amlal El Mahrouss (amlal@nekernel.org)
// Licensed under the Apache License, Version 2.0 (See accompanying
// file LICENSE or copy at http://www.apache.org/licenses/LICENSE-2.0)
// Official repository: https://github.com/ne-foss-org/nectar

#ifndef NECTAR_DEBUGGERKIT_DEBUGGERCONTRACT_H
#define NECTAR_DEBUGGERKIT_DEBUGGERCONTRACT_H

#include <DebuggerKit/Detail/Config.h>
#include <unordered_map>

#define DK_DEBUGGER_CONTRACT : public ::DebuggerKit::IDebugger

namespace DebuggerKit {
class IDebugger;

/// =========================================================== ///
/// \brief Debugger contract class in C++, as per the design states.
/// \author Amlal El Mahrouss
/// =========================================================== ///
class IDebugger {
 public:
  explicit IDebugger() = default;
  virtual ~IDebugger() = default;

 public:
  IDebugger& operator=(const IDebugger&) = default;
  IDebugger(const IDebugger&)            = default;

 public:
  virtual bool Attach(const CompilerKit::STLString& path, const CompilerKit::STLString& argv,
                      ProcessID& pid) noexcept                        = 0;
  virtual bool BreakAt(const CompilerKit::STLString& symbol) noexcept = 0;
  virtual bool Break() noexcept                                       = 0;
  virtual bool Continue() noexcept                                    = 0;
  virtual bool Detach() noexcept                                      = 0;

  using BreakpointMap = std::unordered_map<uintptr_t, uintptr_t>;

  virtual BreakpointMap& Leak() { return mBreakpoints; }

 protected:
  ProcessID     mPid{(ProcessID) ~0};
  BreakpointMap mBreakpoints;
};
}  // namespace DebuggerKit

#endif  // NECTAR_DEBUGGERKIT_DEBUGGERCONTRACT_H
