// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026, Amlal El Mahrouss (amlal@nekernel.org)
// Licensed under the Apache License, Version 2.0 (See accompanying
// file LICENSE or copy at http://www.apache.org/licenses/LICENSE-2.0)
// Official repository: https://github.com/ne-foss-org/nectar

#ifdef DK_MACH_DEBUGGER

#include <DebuggerKit/Mach.h>
#include <ThirdParty/Dialogs/Dialogs.h>

#ifdef DK_MACH_DEBUGGER
#include <DebuggerKit/Common.inl>

/// @internal
/// @brief Handles CTRL-C signal on debugger.
static Void dbgi_ctrlc_handler(std::int32_t _) {
  if (!kPID) {
    return;
  }

  kUserDebugger.Break();

  pfd::notify("Debugger Event", "Breakpoint hit!");

  kKeepRunning = false;
}

NECTAR_MODULE(DebuggerMach) {
  pfd::notify(
      "Debugger Event",
      "Userland Debugger\n(C) 2025 Amlal El Mahrouss, licensed under the Apache 2.0 license.");

  if (argc >= 3 && std::string(argv[1]) == "-p" && argv[2] != nullptr) {
    kPath = argv[2];
    kUserDebugger.SetPath(kPath);

    kStdOut << "[+] Image set to: " << kPath << "\n";
  } else {
    kStdOut << "usage: " << argv[0] << " -p <path>\n";
    kStdOut << "example: " << argv[0] << " -p </path/to/program>\n";

    return EXIT_FAILURE;
  }

  CompilerKit::install_signal(SIGINT, dbgi_ctrlc_handler);

  while (YES) {
    if (kKeepRunning) {
      continue;
    }

    CompilerKit::STLString cmd{};

    if (!std::getline(std::cin, cmd)) break;

    if (cmd == "c" || cmd == "cont" || cmd == "continue") {
      if (kUserDebugger.Continue()) {
        kKeepRunning = true;

        kStdOut << "[+] Continuing...\n";
        pfd::notify("Debugger Event", "Continuing...");
      }
    }

    if (cmd == "d" || cmd == "detach") kUserDebugger.Detach();

    if (cmd == "start") {
      kStdOut << "[?] Enter a argument to use: ";
      std::getline(std::cin, cmd);

      kUserDebugger.Attach(kPath, cmd, kPID);
    }

    if (cmd == "exit") {
      if (kPID > 0) kUserDebugger.Detach();

      break;
    }

    if (cmd == "break" || cmd == "b") {
      kStdOut << "[?] Enter a symbol to break on: ";

      std::getline(std::cin, cmd);

      if (kUserDebugger.BreakAt(cmd)) {
        pfd::notify("Debugger Event", "Add breakpoint at: " + cmd);
      }
    }
  }

  return EXIT_SUCCESS;
}
#endif

#endif
