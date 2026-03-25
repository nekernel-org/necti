// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026, Amlal El Mahrouss (amlal@nekernel.org)
// Licensed under the Apache License, Version 2.0 (See accompanying
// file LICENSE or copy at http://www.apache.org/licenses/LICENSE-2.0)
// Official repository: https://github.com/ne-foss-org/nectar

#define kBlank "\e[0;30m"
#define kRed "\e[0;31m"
#define kWhite "\e[0;97m"

#define kStdOut (std::cout << kRed << "dbg: " << kWhite)

inline bool kKeepRunning = false;

#ifdef DK_NEKERNEL_DEBUGGER
inline DebuggerKit::NeKernel::NeSystemDebugger kKernelDebugger;
#else
#ifdef DK_MACH_DEBUGGER

inline DebuggerKit::POSIX::MachDebugger kUserDebugger;

#else

inline DebuggerKit::POSIX::POSIXDebugger kUserDebugger;

#endif
#endif

static DebuggerKit::ProcessID kPID           = 0L;
static DebuggerKit::CAddress  kActiveAddress = nullptr;
static CompilerKit::STLString kPath          = "";
