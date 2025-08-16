/***
  DebuggerKit
  (C) 2025 Amlal El Mahrouss
  File: NeKernelContract.cc
  Purpose: NeKernel Debugger
*/

#pragma once

#ifdef DEBUGGERKIT_POSIX
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>
#else
#error !!! DebuggerKit needs a networking backend !!!
#endif