/***
	(C) 2025 Amlal El Mahrouss
 */

#include <LibCompiler/Defines.h>
#include <Vendor/Dialogs.h>
#include <LibDebugger/POSIX.h>
#include <cstdint>
#include <string>

#ifndef _WIN32

static bool					 kKeepRunning = false;
LibDebugger::POSIX::Debugger kDebugger;

static void DebuggerCtrlCHandler(std::int32_t _)
{
	auto list = kDebugger.Get();

	LibDebugger::POSIX::CAddr addr = (LibDebugger::POSIX::CAddr)list[list.size() - 1];

	if (!addr)
	{
		pfd::notify("Debugger Event", "Invalid breakpoint at: " + std::to_string(list[list.size() - 1]));
		return;
	}

	kDebugger.Break(addr);

	pfd::notify("Debugger Event", "Breakpoint at: " + std::to_string(list[list.size() - 1]));

	kKeepRunning = false;
}

LIBCOMPILER_MODULE(DebuggerPOSIX)
{
	if (argc < 1)
	{
		return EXIT_FAILURE;
	}

	pid_t pid = 0L;

	if (argc >= 3 && std::string(argv[1]) == "-p" &&
		argv[2] != nullptr)
	{
		pid = std::stoi(argv[2]);
		kDebugger.Attach(pid);
	}

	::signal(SIGINT, DebuggerCtrlCHandler);

	while (YES)
	{
		if (kKeepRunning)
		{
			continue;
		}

		std::string cmd;
		std::getline(std::cin, cmd);

		if (cmd == "c" ||
			cmd == "cont")
		{
			kDebugger.Continue();
			kKeepRunning = true;
		}

		if (cmd == "d" ||
			cmd == "detach")
			kDebugger.Detach();

		if (cmd == "attach")
		{
			std::cout << "[?] Enter a PID to attach on: ";

			std::getline(std::cin, cmd);
			pid = std::stoi(cmd.c_str());

			pfd::notify("Debugger Event", "Attach process: " + std::to_string(pid));

			kDebugger.Attach(pid);
		}

		if (cmd == "exit")
		{
			if (pid > 0)
				kDebugger.Detach();

			break;
		}

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

			LibDebugger::POSIX::CAddr breakpoint_addr = reinterpret_cast<LibDebugger::POSIX::CAddr>(addr);

			if (breakpoint_addr)
				kDebugger.Break(breakpoint_addr);
		}
	}

	return EXIT_SUCCESS;
}

#endif