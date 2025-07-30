/***
  DebuggerKit
  (C) 2025 Amlal El Mahrouss
  File: NeKernelContract.cc
  Purpose: NeKernel Debugger CLI.
*/

#ifdef LD_NEKERNEL_DEBUGGER

#include <CompilerKit/Defines.h>
#include <DebuggerKit/NeKernelContract.h>
#include <ThirdParty/Dialogs.h>
#include <string>

#include <DebuggerKit/CommonCLI.inl>

using namespace DebuggerKit::NeKernel;

static void dbgi_ctrlc_handler(std::int32_t _) {
  if (!kPID || kPath.empty()) {
    return;
  }

  kKernelDebugger.Break();

  pfd::notify("Debugger Event", "Breakpoint hit!");

  kKeepRunning = false;
}

LIBCOMPILER_MODULE(DebuggerNeKernel) {
  pfd::notify("Debugger Event",
              "NeKernel Debugger\n(C) 2025 Amlal El Mahrouss and NeKernel.org contributors, all rights reserved.");

  if (argc >= 5 && std::string(argv[1]) == "-k" && argv[2] != nullptr &&
      std::string(argv[3]) == "-ip" && argv[4] != nullptr) {
    kPath = argv[2];
    kPath += ":";
    kPath += argv[4];

    kStdOut << "[+] KIP (Kernel:IP) set to: " << kPath << "\n";

    CompilerKit::install_signal(SIGINT, dbgi_ctrlc_handler);

    kKernelDebugger.Attach(kPath, "", kPID);
    kKernelDebugger.BreakAt("$HANDOVER_START");

    while (YES) {
      if (kKeepRunning) {
        continue;
      }

      std::string cmd;
      if (!std::getline(std::cin, cmd)) break;

      if (cmd == "c" || cmd == "cont" || cmd == "continue") {
        if (kKernelDebugger.Continue()) {
          kKeepRunning = true;

          kStdOut << "[+] Continuing...\n";

          pfd::notify("Debugger Event", "Continuing...");
        }
      }

      if (cmd == "d" || cmd == "detach") kKernelDebugger.Detach();

      if (cmd == "start") {
        kStdOut << "[?] Enter a argument to use: ";
        std::getline(std::cin, cmd);

        kKernelDebugger.Attach(kPath, cmd, kPID);
      }

      if (cmd == "exit") {
        if (kPID > 0) kKernelDebugger.Detach();

        break;
      }

      if (cmd == "break" || cmd == "b") {
        kStdOut << "[?] Enter a symbol to break on: ";

        std::getline(std::cin, cmd);

        if (kKernelDebugger.BreakAt(cmd)) {
          pfd::notify("Debugger Event", "Add BreakAt at: " + cmd);
        }
      }
    }

    return EXIT_SUCCESS;
  }

  kStdOut << "usage: " << argv[0] << " -k <kernel_path> -ip <ip4>\n";
  kStdOut << "example: " << argv[0] << " -k /path/to/ne_kernel -ip 127.0.0.1\n";

  return EXIT_FAILURE;
}

#endif  // LD_NEKERNEL_DEBUGGER