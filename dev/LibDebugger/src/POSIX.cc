/***
	(C) 2025 Amlal El Mahrouss
 */

#include <LibCompiler/Defines.h>
#include <LibDebugger/POSIX.h>

#ifndef _WIN32

LIBCOMPILER_MODULE(DebuggerPOSIX)
{
	if (argc < 1)
	{
		return EXIT_FAILURE;
	}

	LibDebugger::POSIX::Debugger debugger;
	pid_t						 pid = 0L;

	if (argc >= 3 && std::string(argv[1]) == "-p" &&
		argv[2] != nullptr)
	{
		pid = std::stoi(argv[2]);
		debugger.Attach(pid);
	}

	while (YES)
	{
		std::string cmd;
		std::getline(std::cin, cmd);

		if (cmd == "c" ||
			cmd == "cont")
			debugger.Continue();

		if (cmd == "d" ||
			cmd == "detach")
			debugger.Detach();

		if (cmd == "attach")
		{
			std::cout << "[?] Enter a PID to attach on: ";

			std::getline(std::cin, cmd);
			pid = std::stoi(cmd.c_str());

			debugger.Attach(pid);
		}

		if (cmd == "exit")
		{
			if (pid > 0)
				debugger.Detach();

			break;
		}

		if (cmd == "break" ||
			cmd == "b")
		{
			std::cout << "[?] Enter an address to add a breakpoint on: ";

			std::getline(std::cin, cmd);

			LibDebugger::POSIX::CAddr breakpoint_addr = reinterpret_cast<LibDebugger::POSIX::CAddr>(std::stoul(cmd.c_str(), nullptr, 16));

			if (breakpoint_addr)
				debugger.Break(breakpoint_addr);
		}
	}

	return EXIT_SUCCESS;
}

#endif