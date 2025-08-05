/***
  (C) 2025 Amlal El Mahrouss
 */

#pragma once

#ifdef DK_MACH_DEBUGGER

/// @file POSIXMachContract.h
/// @brief POSIX Mach debugger.

#include <CompilerKit/Defines.h>
#include <DebuggerKit/DebuggerContract.h>

#include <stdint.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/user.h>
#include <sys/wait.h>
#include <unistd.h>

#include <filesystem>
#include <iostream>

#include <dlfcn.h>
#include <mach-o/dyld.h>
#include <mach/mach.h>
#include <mach/mach_error.h>
#include <signal.h>

CK_IMPORT_C kern_return_t mach_vm_write(vm_map_t target_task, mach_vm_address_t address,
                                        vm_offset_t data, mach_msg_type_number_t dataCnt);

CK_IMPORT_C kern_return_t mach_vm_protect(vm_map_t target_task, mach_vm_address_t address,
                                          mach_vm_size_t size, boolean_t set_maximum,
                                          vm_prot_t new_protection);

#define PTRACE_ATTACH PT_ATTACHEXC
#define PTRACE_DETACH PT_DETACH
#define PTRACE_POKETEXT PT_WRITE_I
#define PTRACE_CONT PT_CONTINUE
#define PTRACE_PEEKTEXT PT_READ_I

namespace DebuggerKit::POSIX {
/// \brief POSIXMachContract engine interface class in C++
/// \author Amlal El Mahrouss
class POSIXMachContract : public DebuggerContract {
 public:
  explicit POSIXMachContract()  = default;
  ~POSIXMachContract() override = default;

 public:
  POSIXMachContract& operator=(const POSIXMachContract&) = default;
  POSIXMachContract(const POSIXMachContract&)            = default;

 public:
  BOOL Attach(std::string path, std::string argv, ProcessID& pid) noexcept override {
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
    m_pid  = pid;

    pid = this->m_pid;

    return true;
  }

  void SetPath(std::string path) noexcept {
    if (path.empty()) {
      return;
    }

    m_path = path;
  }

  BOOL BreakAt(std::string symbol) noexcept override {
    if (!m_path.empty() && std::filesystem::exists(m_path) &&
        std::filesystem::is_regular_file(m_path)) {
      auto handle = dlopen(m_path.c_str(), RTDK_LAZY);

      if (handle == nullptr) {
        return false;
      }

      auto addr = dlsym(handle, symbol.c_str());

      if (addr == nullptr) {
        return false;
      }

      task_read_t task;
      task_for_pid(mach_task_self(), m_pid, &task);

      uint32_t brk_inst = 0xD43E0000;

      mach_vm_protect(task, (mach_vm_address_t) addr, sizeof(uint32_t), false,
                      VM_PROT_READ | VM_PROT_WRITE | VM_PROT_EXECUTE);
      mach_vm_write(task, (mach_vm_address_t) addr, (vm_offset_t) &brk_inst, sizeof(addr));

      return true;
    }

    return false;
  }

  BOOL Break() noexcept override {
    task_read_t task;
    task_for_pid(mach_task_self(), m_pid, &task);

    kern_return_t ret = task_suspend(task);

    return ret == KERN_SUCCESS;
  }

  BOOL Continue() noexcept override {
    task_read_t task;
    task_for_pid(mach_task_self(), m_pid, &task);

    kern_return_t ret = task_resume(task);

    return ret == KERN_SUCCESS;
  }

  BOOL Detach() noexcept override {
    this->Continue();

    task_read_t task;
    task_for_pid(mach_task_self(), m_pid, &task);

    kern_return_t kr = mach_port_deallocate(mach_task_self(), task);

    return kr = KERN_SUCCESS;
  }

 private:
  ProcessID   m_pid{0};
  std::string m_path;
};
}  // namespace DebuggerKit::POSIX

#endif
