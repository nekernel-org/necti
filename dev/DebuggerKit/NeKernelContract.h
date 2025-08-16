
/***
  (C) 2025 Amlal El Mahrouss
 */

#ifndef DK_NEKERNEL_CONTRACT_H
#define DK_NEKERNEL_CONTRACT_H

#ifdef DK_NEKERNEL_DEBUGGER

#include <DebuggerKit/DebuggerContract.h>

namespace DebuggerKit::NeKernel {
class NeKernelContract;

namespace Detail {
  inline constexpr auto kDebugCmdLen  = 256U;
  inline constexpr auto kDebugPort    = 51820;
  inline constexpr auto kDebugMagic   = "VMK1.0.0;";
  inline constexpr auto kDebugVersion = 0x0100;
  typedef char          rt_debug_cmd[kDebugCmdLen];
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
}  // namespace DebuggerKit::NeKernel

#endif  // ifdef DK_NEKERNEL_DEBUGGER

#endif  // DK_NEKERNEL_CONTRACT_H