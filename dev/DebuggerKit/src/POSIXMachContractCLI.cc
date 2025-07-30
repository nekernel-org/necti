/***
  DebuggerKit
  (C) 2025 Amlal El Mahrouss
  File: POSIXMachContract.cc
  Purpose: OS X/Darwin Debugger
*/

#ifdef LD_MACH_DEBUGGER

#include <CompilerKit/Defines.h>
#include <DebuggerKit/POSIXMachContract.h>
#include <ThirdParty/Dialogs.h>
#include <DebuggerKit/CommonCLI.inl>

/// @internal
/// @brief Handles CTRL-C signal on debugger.
static void dbgi_ctrlc_handler(std::int32_t _) {
  if (!kPID) {
    return;
  }

  kDebugger.Break();

  pfd::notify("Debugger Event", "Breakpoint hit!");

  kKeepRunning = false;
}

LIBCOMPILER_MODULE(DebuggerMachPOSIX) {
  pfd::notify("Debugger Event",
              "Userland Debugger\n(C) 2025 Amlal El Mahrouss, all rights reserved.");

  if (argc >= 3 && std::string(argv[1]) == "-p" && argv[2] != nullptr) {
    kPath = argv[2];
    kDebugger.SetPath(kPath);

    kStdOut << "[+] Image set to: " << kPath << "\n";
  } else {
    kStdOut << "usage: " << argv[0] << " -p <path>\n";
    kStdOut << "example: " << argv[0] << " -p /path/to/program\n";

    return EXIT_FAILURE;
  }

  CompilerKit::install_signal(SIGINT, dbgi_ctrlc_handler);

  while (YES) {
    if (kKeepRunning) {
      continue;
    }

    std::string cmd;
    if (!std::getline(std::cin, cmd)) break;

    if (cmd == "c" || cmd == "cont" || cmd == "continue") {
      if (kDebugger.Continue()) {
        kKeepRunning = true;

        kStdOut << "[+] Continuing...\n";

        pfd::notify("Debugger Event", "Continuing...");
      }
    }

    if (cmd == "d" || cmd == "detach") kDebugger.Detach();

    if (cmd == "start") {
      kStdOut << "[?] Enter a argument to use: ";
      std::getline(std::cin, cmd);

      kDebugger.Attach(kPath, cmd, kPID);
    }

    if (cmd == "exit") {
      if (kPID > 0) kDebugger.Detach();

      break;
    }

    if (cmd == "break" || cmd == "b") {
      kStdOut << "[?] Enter a symbol to break on: ";

      std::getline(std::cin, cmd);

      if (kDebugger.BreakAt(cmd)) {
        pfd::notify("Debugger Event", "Add BreakAt at: " + cmd);
      }
    }
  }

  return EXIT_SUCCESS;
}

#endif