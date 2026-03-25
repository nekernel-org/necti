// Copyright 2024-2025, Amlal El Mahrouss (amlal@nekernel.org)
// Licensed under the Apache License, Version 2.0 (See accompanying
// file LICENSE or copy at http://www.apache.org/licenses/LICENSE-2.0)
// Official repository: https://github.com/ne-foss-org/nectar

#ifndef NECTAR_DEBUGGERKIT_MACHCONTRACT_H
#define NECTAR_DEBUGGERKIT_MACHCONTRACT_H

#ifdef DK_MACH_DEBUGGER

/// @file POSIXDebugger.h
/// @brief POSIX Mach debugger.

#include <DebuggerKit/IDebugger.h>
#include <sys/ptrace.h>
#include <filesystem>
#include <vector>

#define PTRACE_ATTACH PT_ATTACH
#define PTRACE_DETACH PT_DETACH
#define PTRACE_POKETEXT PT_WRITE_I
#define PTRACE_CONT PT_CONTINUE
#define PTRACE_PEEKTEXT PT_READ_I

namespace DebuggerKit::POSIX {
/// =========================================================== ///
/// \brief POSIXDebugger engine class in C++
/// \author Amlal El Mahrouss
/// =========================================================== ///
class POSIXDebugger DK_DEBUGGER_CONTRACT {
 public:
  explicit POSIXDebugger()  = default;
  ~POSIXDebugger() override = default;

 public:
  POSIXDebugger& operator=(const POSIXDebugger&) = default;
  POSIXDebugger(const POSIXDebugger&)            = default;

 public:
  bool Attach(const CompilerKit::STLString& path, const CompilerKit::STLString& argv,
              ProcessID& pid) noexcept override {
    pid = fork();

    if (pid == 0) {
      if (argv.empty()) {
        ptrace(PT_TRACE_ME, 0, nullptr, 0);
        kill(getpid(), SIGSTOP);
      }

      std::vector<char*> argv_arr;

      argv_arr.push_back(const_cast<char*>(path.c_str()));
      argv_arr.push_back(const_cast<char*>(argv.c_str()));
      argv_arr.push_back(nullptr);

      execv(path.c_str(), argv_arr.data());

      _exit(1);
    }

    m_path = path;
    mPid   = pid;

    pid = this->mPid;

    return true;
  }

  void SetPath(const CompilerKit::STLString& path) noexcept {
    if (path.empty()) {
      return;
    }

    m_path = path;
  }

  bool BreakAt(const CompilerKit::STLString& symbol) noexcept override {
    if (!m_path.empty() && std::filesystem::exists(m_path) &&
        std::filesystem::is_regular_file(m_path)) {
      auto handle = dlopen(m_path.c_str(), RTLD_LAZY);

      if (handle == nullptr) {
        return false;
      }

      auto addr = dlsym(handle, symbol.c_str());

      if (addr == nullptr) {
        return false;
      }

      return true;
    }

    return false;
  }

  bool Break() noexcept override { return false; }

  bool Continue() noexcept override { return false; }

  bool Detach() noexcept override {
    this->Continue();
    return false;
  }

 private:
  ProcessID              mPid{0};
  CompilerKit::STLString m_path;
};
}  // namespace DebuggerKit::POSIX

#endif  // DK_MACH_DEBUGGER

#endif  // NECTAR_DEBUGGERKIT_MACHCONTRACT_H
