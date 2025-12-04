/* ========================================

  Copyright (C) 2025 Amlal El Mahrouss, Licensed under the Apache 2.0 license.

======================================== */

#pragma once

#include <DebuggerKit/detail/Config.h>
#include <unordered_map>

#define DK_DEBUGGER_CONTRACT : public ::DebuggerKit::IDebuggerContract

namespace DebuggerKit {
class IDebuggerContract;

/// =========================================================== ///
/// \brief Process ID
/// =========================================================== ///
typedef uint64_t ProcessID;

/// =========================================================== ///
/// \brief Address type, a la BSD.
/// =========================================================== ///
typedef char* CAddress;

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

  virtual std::unordered_map<uintptr_t, uintptr_t>& Get() { return m_breakpoints; }

 protected:
  ProcessID                                m_pid{(ProcessID) ~0};
  std::unordered_map<uintptr_t, uintptr_t> m_breakpoints;
};
}  // namespace DebuggerKit
