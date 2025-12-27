/* ========================================

  Copyright (C) 2025 Amlal El Mahrouss, licensed under the Apache 2.0 license.

======================================== */

#define kBlank "\e[0;30m"
#define kRed "\e[0;31m"
#define kWhite "\e[0;97m"

#define kStdOut (std::cout << kRed << "dbg: " << kWhite)

inline bool kKeepRunning = false;

#ifdef DK_NEKERNEL_DEBUGGER
inline DebuggerKit::NeKernel::NeKernelContract kKernelDebugger;
#else
inline DebuggerKit::POSIX::POSIXMachContract kUserDebugger;
#endif

static DebuggerKit::ProcessID kPID           = 0L;
static DebuggerKit::CAddress  kActiveAddress = nullptr;
static CompilerKit::STLString kPath          = "";
