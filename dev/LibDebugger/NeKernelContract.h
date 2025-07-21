
/***
  (C) 2025 Amlal El Mahrouss
 */

#ifndef LD_NEKERNEL_CONTRACT_H
#define LD_NEKERNEL_CONTRACT_H

#ifdef LD_NEKERNEL_DEBUGGER

#include <LibDebugger/DebuggerContract.h>

namespace LibDebugger::NeKernel {
class NeKernelContract;

namespace Detail {
  inline constexpr size_t kDebugCmdLen = 256U;
  typedef char            rt_debug_cmd[kDebugCmdLen];
}  // namespace Detail

class NeKernelContract : public DebuggerContract {
 public:
  NeKernelContract();
  ~NeKernelContract() override;

 public:
  NeKernelContract& operator=(const NeKernelContract&) = default;
  NeKernelContract(const NeKernelContract&)            = default;

  // Override additional methods from DebuggerContract

 public:
  bool Attach(std::string path, std::string arg_v, ProcessID& pid) noexcept override;
  bool BreakAt(std::string symbol) noexcept override;
  bool Break() noexcept override;
  bool Continue() noexcept override;
  bool Detach() noexcept override;

 private:
  std::string m_kernel_path;
  int64_t     m_socket{0};
};
}  // namespace LibDebugger::NeKernel

#endif  // ifdef LD_NEKERNEL_DEBUGGER

#endif  // LD_NEKERNEL_CONTRACT_H