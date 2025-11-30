/* ========================================

  Copyright (C) 2025 Amlal El Mahrouss, Licensed under the Apache 2.0 license.

======================================== */

#ifdef DK_NEKERNEL_DEBUGGER

/// @author Amlal El Mahrouss
/// @brief Kernel Debugger Protocol

#include <CompilerKit/Config.h>
#include <DebuggerKit/Config.h>
#include <DebuggerKit/NeKernelContract.h>
#include <ThirdParty/Dialogs.h>

using namespace DebuggerKit::NeKernel;
using namespace DebuggerKit::NeKernel::Detail;

NeKernelContract::NeKernelContract() = default;

NeKernelContract::~NeKernelContract() = default;

Bool NeKernelContract::Attach(CompilerKit::STLString path, CompilerKit::STLString argv,
                              ProcessID& pid) noexcept {
  if (path.empty() || argv.empty()) return NO;

  m_socket = ::socket(AF_INET, SOCK_STREAM, 0);

  if (m_socket == -1) return NO;

  struct sockaddr_in server_addr;

  server_addr.sin_family = AF_INET;
  server_addr.sin_port   = htons(kDebugPort);

  if (::inet_pton(AF_INET, argv.c_str(), &server_addr.sin_addr) <= 0) return NO;

  auto ret = (::connect(m_socket, (struct sockaddr*) &server_addr, sizeof(server_addr)) == -1);

  if (ret) return NO;

  CompilerKit::STLString pkt = Detail::kDebugMagic;
  pkt += ";\r";

  ret = ::send(m_socket, pkt.data(), pkt.size(), 0) > 0;
  return ret;
}

Bool NeKernelContract::BreakAt(CompilerKit::STLString symbol) noexcept {
  CompilerKit::STLString pkt = Detail::kDebugMagic;
  pkt += ";SYM=";
  pkt += symbol;
  pkt += ";\r";

  if (pkt.size() > kDebugCmdLen) return NO;

  auto ret = ::send(m_socket, pkt.data(), pkt.size(), 0) > 0;
  return ret;
}

Bool NeKernelContract::Break() noexcept {
  CompilerKit::STLString pkt = Detail::kDebugMagic;
  pkt += ";BRK=1;\r";

  auto ret = ::send(m_socket, pkt.data(), pkt.size(), 0) > 0;
  return ret;
}

Bool NeKernelContract::Continue() noexcept {
  CompilerKit::STLString pkt = Detail::kDebugMagic;
  pkt += ";CONT=1;\r";

  auto ret = ::send(m_socket, pkt.data(), pkt.size(), 0) > 0;
  return ret;
  return NO;
}

Bool NeKernelContract::Detach() noexcept {
  CompilerKit::STLString pkt = Detail::kDebugMagic;
  pkt += ";DTCH=1;\r";

  auto ret = ::send(m_socket, pkt.data(), pkt.size(), 0) > 0;

  if (ret) ::close(m_socket);

  return ret;
}

#endif  // DK_NEKERNEL_DEBUGGER
