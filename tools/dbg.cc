/***
	(C) 2025 Amlal El Mahrouss
 */

#include <iostream>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/user.h>
#include <unistd.h>
#include <unordered_map>
#include <stdint.h>

namespace Dbg
{
	/// \brief Debug Interface class in C++
	/// \author Amlal El Mahrouss
	class Interface final
	{
	public:
		Interface()	 = default;
		~Interface() = default;

		Interface& operator=(const Interface&) = default;
		Interface(const Interface&)			   = default;

		void Attach(pid_t pid)
		{
			this->pid = pid;

			if (ptrace(PTRACE_ATTACH, this->pid, nullptr, nullptr) == -1)
			{
				perror("DBG: Attach");
				exit(1);
			}

			waitpid(pid, nullptr, 0);
			std::cout << "[+] Attached to process: " << pid << std::endl;
		}

		void SetBreakpoint(void* addr)
		{
			long original_data = ptrace(PTRACE_PEEKTEXT, pid, addr, nullptr);
			if (original_data == -1)
			{
				perror("DBG: Peek");
				exit(1);
			}

			long data_with_int3 = (original_data & ~0xFF) | 0xCC; // Insert INT3 (0xCC)
			if (ptrace(PTRACE_POKETEXT, pid, addr, data_with_int3) == -1)
			{
				perror("DBG: Poke");
				exit(1);
			}

			std::cout << "[+] Breakpoint set at: " << addr << std::endl;
			breakpoints[reinterpret_cast<uintptr_t>(addr)] = original_data; // Store original data
		}

		void ContinueExecution()
		{
			if (ptrace(PTRACE_CONT, pid, nullptr, nullptr) == -1)
			{
				perror("DBG: Cont");
				exit(1);
			}

			int status;
			waitpid(pid, &status, 0);

			if (WIFSTOPPED(status) && WSTOPSIG(status) == SIGTRAP)
			{
				std::cout << "[!] Breakpoint hit!" << std::endl;
			}
		}

		void Detach()
		{
			if (ptrace(PTRACE_DETACH, pid, nullptr, nullptr) == -1)
			{
				perror("DBG: Detach");
				exit(1);
			}

			std::cout << "[-] Detached from process: " << pid << std::endl;
		}

		std::unordered_map<uintptr_t, long>& Breakpoints()
		{
			return breakpoints;
		}

	private:
		pid_t								pid;
		std::unordered_map<uintptr_t, long> breakpoints;
	};
} // namespace Dbg

int main(int argc, char* argv[])
{
	if (!argv[1])
	{
		std::cout << "[?] Attach to process using it's PID.\n";
		return EXIT_FAILURE;
	}

	pid_t pid = std::stoi(argv[1]);

	Dbg::Interface debugger;
	debugger.Attach(pid);

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
			pid = std::stoi(argv[1]);
			debugger.Attach(pid);
		}

		if (cmd == "exit")
		{
			std::exit(0);
		}

		if (cmd == "b" ||
			cmd == "break")
		{
			std::cout << "[?] Enter an address to break on: ";

			std::getline(std::cin, cmd);
			void* breakpoint_addr = reinterpret_cast<void*>(std::stoul(cmd.c_str(), nullptr, 16));

			if (breakpoint_addr)
				debugger.SetBreakpoint(breakpoint_addr);
		}
	}

	return 0;
}
