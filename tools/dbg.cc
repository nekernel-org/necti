/***
	(C) 2025 Amlal El Mahrouss
 */

#include <LibDebugger/Debugger.h>

int main(int argc, char* argv[])
{
	LibDebugger::IDebugger debugger;
	pid_t				   pid = 0L;

	if (argc >= 3 && std::string(argv[1]) == "-p" &&
		argv[2] != nullptr)
	{
		pid = std::stoi(argv[2]);
		debugger.Attach(pid);
	}

	while (true)
	{
		std::string cmd;
		std::getline(std::cin, cmd);

		if (cmd == "c" ||
			cmd == "cont")
			debugger.ContinueExecution();

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
			debugger.Detach();
			break;
		}

		if (cmd == "break" ||
			cmd == "bp")
		{
			std::cout << "[?] Enter an address to add a breakpoint on: ";

			std::getline(std::cin, cmd);

			void* breakpoint_addr = reinterpret_cast<void*>(std::stoul(cmd.c_str(), nullptr, 16));

			if (breakpoint_addr)
				debugger.SetBreakpoint(breakpoint_addr);
		}
	}

	return 0;
}
