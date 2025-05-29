/***
  LibDebugger
  (C) 2025 Amlal El Mahrouss
  File: NeKernelContract.cc
  Purpose: NeKernel Debugger CLI.
*/

#ifdef LD_NEKERNEL_DEBUGGER

#include <LibCompiler/Defines.h>
#include <LibDebugger/NeKernelContract.h>
#include <Vendor/Dialogs.h>

#include <cstdint>
#include <iostream>
#include <string>

#include <LibDebugger/CommonCLI.inl>

using namespace LibDebugger::NeKernel;

LIBCOMPILER_MODULE(DebuggerNeKernel) {
  pfd::notify("Debugger Event",
              "Kernel Debugger\n(C) 2025 Amlal El Mahrouss, all rights reserved.");

  if (argc >= 3 && std::string(argv[1]) == "-k" && argv[2] != nullptr) {
    kPath = argv[2];
    kStdOut << "[+] Kernel (ne_kernel) set to: " << kPath << "\n";

    kKernelDebugger.Attach(kPath, nullptr, kPID);
    kKernelDebugger.Breakpoint("$HANDOVER_START");

    return EXIT_SUCCESS;
  }

  kStdOut << "Usage: " << argv[0] << " -k <kernel_path>\n";
  kStdOut << "Example: " << argv[0] << " -k /path/to/ne_kernel\n";

  return EXIT_FAILURE;
}

#endif  // LD_NEKERNEL_DEBUGGER