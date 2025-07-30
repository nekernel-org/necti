/***
  DebuggerKit
  (C) 2025 Amlal El Mahrouss
  File: CommonCLI.inl
  Purpose: Common Debugger symbols.
*/

#include <cstdint>
#include <iostream>
#include <string>

#define kBlank "\e[0;30m"
#define kRed "\e[0;31m"
#define kWhite "\e[0;97m"

#define kStdOut (std::cout << kRed << "dbg: " << kWhite)

static BOOL kKeepRunning = false;

#ifdef LD_NEKERNEL_DEBUGGER
static DebuggerKit::NeKernel::NeKernelContract kKernelDebugger;
#else
static DebuggerKit::POSIX::POSIXMachContract kDebugger;
#endif

static DebuggerKit::ProcessID kPID           = 0L;
static DebuggerKit::CAddress  kActiveAddress = nullptr;
static std::string            kPath          = "";
