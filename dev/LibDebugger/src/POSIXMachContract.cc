/***
	(C) 2025 Amlal El Mahrouss
 */

#include <LibCompiler/Defines.h>
#include <Vendor/Dialogs.h>
#include <LibDebugger/POSIXMachContract.h>
#include <cstdint>
#include <string>

#ifndef _WIN32

static BOOL									 kKeepRunning = false;
static LibDebugger::POSIX::POSIXMachContract kDebugger;
static LibDebugger::ProcessID				 kPID			= 0L;
static LibDebugger::CAddress				 kActiveAddress = nullptr;

/// @internal
/// @brief Handles CTRL-C signal on debugger.
static void dbgi_ctrlc_handler(std::int32_t _)
{
	if (!kPID)
	{
		return;
	}

	auto list = kDebugger.Get();

	kDebugger.Break(kActiveAddress);

	pfd::notify("Debugger Event", "Breakpoint hit!");

	kKeepRunning = false;
}

LIBCOMPILER_MODULE(DebuggerMachPOSIX)
{
	pfd::notify("Debugger Event", "NeKernel Debugger\n(C) 2025 Amlal El Mahrouss, all rights reserved.");

	if (argc >= 3 && std::string(argv[1]) == "-p" &&
		argv[2] != nullptr)
	{
		kPID = std::stoi(argv[2]);
		kDebugger.Attach(kPID);
	}

	::signal(SIGINT, dbgi_ctrlc_handler);

	while (YES)
	{
		if (kKeepRunning)
		{
			continue;
		}

		std::string cmd;
		std::getline(std::cin, cmd);

		if (cmd == "c" ||
			cmd == "cont" ||
			cmd == "continue")
		{
			kDebugger.Continue();
			kKeepRunning = true;

			std::cout << "[+] Continuing...\n";
			pfd::notify("Debugger Event", "Continuing...");
		}

		if (cmd == "d" ||
			cmd == "detach")
			kDebugger.Detach();

		if (cmd == "attach" ||
			cmd == "pid" ||
			cmd == "a")
		{
			std::cout << "[?] Enter a PID to attach on: ";

			std::getline(std::cin, cmd);
			kPID = std::stoi(cmd.c_str());

			pfd::notify("Debugger Event", "Attach process: " + std::to_string(kPID));

			kDebugger.Attach(kPID);
		}

		if (cmd == "exit")
		{
			if (kPID > 0)
				kDebugger.Detach();

			break;
		}

#ifndef __APPLE__
		if (cmd == "break" ||
			cmd == "b")
		{
			std::cout << "[?] Enter an address/symbol to add a break on: ";

			std::getline(std::cin, cmd);

			auto addr = std::stoul(cmd.c_str(), nullptr, 16);

			try
			{
				pfd::notify("Debugger Event", "Add Breakpoint at: " + std::to_string(addr));
			}
			catch (...)
			{
				pfd::notify("Debugger Event", "Add Breakpoint at: " + cmd);
			}

			LibDebugger::CAddress breakpoint_addr = reinterpret_cast<LibDebugger::CAddress>(addr);

			if (breakpoint_addr)
			{
				kActiveAddress = breakpoint_addr;
				kDebugger.Break(kActiveAddress);
			}
		}
#endif // ifndef __APPLE__
	}

	return EXIT_SUCCESS;
}

#endif