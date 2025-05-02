/***
  (C) 2025 Amlal El Mahrouss
 */

#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>

namespace LibDebugger {
class DebuggerContract;

/// \brief Process ID
typedef uint64_t ProcessID;

/// \brief Address type, a la BSD.
typedef char* CAddress;

/// \brief Debugger contract class in C++, as per the design states.
/// \author Amlal El Mahrouss
class DebuggerContract {
 public:
  explicit DebuggerContract() = default;
  virtual ~DebuggerContract() = default;

 public:
  DebuggerContract& operator=(const DebuggerContract&) = default;
  DebuggerContract(const DebuggerContract&)            = default;

 public:
  virtual bool Attach(std::string path, std::string argv, ProcessID& pid) noexcept = 0;
  virtual bool Breakpoint(std::string symbol) noexcept                             = 0;
  virtual bool Break() noexcept                                                    = 0;
  virtual bool Continue() noexcept                                                 = 0;
  virtual bool Detach() noexcept                                                   = 0;

  virtual std::unordered_map<uintptr_t, uintptr_t>& Get() { return m_breakpoints; }

 protected:
  ProcessID                                m_pid;
  std::unordered_map<uintptr_t, uintptr_t> m_breakpoints;
};
}  // namespace LibDebugger
