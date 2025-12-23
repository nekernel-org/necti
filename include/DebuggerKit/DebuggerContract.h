/* ========================================

  Copyright (C) 2025 Amlal El Mahrouss, licensed under the Apache 2.0 license.

======================================== */

#pragma once

#include <DebuggerKit/Detail/Config.h>
#include <unordered_map>

#define DK_DEBUGGER_CONTRACT : public ::DebuggerKit::IDebuggerContract

namespace DebuggerKit {
class IDebuggerContract;

/// =========================================================== ///
/// \brief Debugger contract class in C++, as per the design states.
/// \author Amlal El Mahrouss
/// =========================================================== ///
class IDebuggerContract {
 public:
  explicit IDebuggerContract() = default;
  virtual ~IDebuggerContract() = default;

 public:
  IDebuggerContract& operator=(const IDebuggerContract&) = default;
  IDebuggerContract(const IDebuggerContract&)            = default;

 public:
  virtual bool Attach(std::string path, std::string argv, ProcessID& pid) noexcept = 0;
  virtual bool BreakAt(std::string symbol) noexcept                                = 0;
  virtual bool Break() noexcept                                                    = 0;
  virtual bool Continue() noexcept                                                 = 0;
  virtual bool Detach() noexcept                                                   = 0;

  using BreakpointMap = std::unordered_map<uintptr_t, uintptr_t>;

  virtual BreakpointMap& Get() { return mBreakpoints; }

 protected:
  ProcessID     mPid{(ProcessID) ~0};
  BreakpointMap mBreakpoints;
};
}  // namespace DebuggerKit

