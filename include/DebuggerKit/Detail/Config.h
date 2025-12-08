/* ========================================

  Copyright (C) 2025 Amlal El Mahrouss, Licensed under the Apache 2.0 license.

======================================== */

#pragma once

/// =========================================================== ///
/// @author Amlal El Mahrouss
/// =========================================================== ///

#include <CompilerKit/Detail/Config.h>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>

#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/user.h>
#include <sys/wait.h>
#include <unistd.h>

#include <dlfcn.h>

#ifdef __APPLE__
#include <mach-o/dyld.h>
#include <mach/mach.h>
#include <mach/mach_error.h>
#endif

#ifndef kDistRelease

#define kDistVersion "v0.0.7-debuggerkit"
#define kDistVersionBCD 0x0001

#define ToString(X) Stringify(X)
#define Stringify(X) #X

#define kDistRelease ToString(kDistReleaseBranch)

#endif  // !kDistRelease

namespace DebuggerKit {
/// =========================================================== ///
/// \brief Process ID
/// =========================================================== ///
typedef uint64_t ProcessID;

/// =========================================================== ///
/// \brief Address type, a la BSD.
/// =========================================================== ///
typedef char* CAddress;

namespace Detail {
  constexpr auto     kDebugCmdLen  = 256U;
  constexpr auto     kDebugPort    = 51820;
  constexpr auto     kDebugMagic   = "NE1.0.0;";
  constexpr uint16_t kDebugVersion = 0x0100;
  constexpr auto     kDebugDelim   = ';';
  constexpr auto     kDebugEnd     = '\r';
  using dk_socket_type             = int64_t;
}  // namespace Detail
}  // namespace DebuggerKit