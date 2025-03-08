/***
	(C) 2025 Amlal El Mahrouss
 */

#include <iostream>
#include <unordered_map>

#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/user.h>
#include <unistd.h>
#include <stdint.h>

#ifdef __APPLE__
#define PTRACE_ATTACH	PT_ATTACHEXC
#define PTRACE_DETACH	PT_DETACH
#define PTRACE_POKETEXT PT_WRITE_I
#define PTRACE_CONT		PT_CONTINUE
#define PTRACE_PEEKTEXT PT_READ_I
#endif

namespace LibDebugger
{
#ifdef __APPLE__
	typedef caddr_t CAddr;
#else
	typedef char* CAddr;
#endif

	/// \brief Debugger engine interface class in C++
	/// \author Amlal El Mahrouss
	class IDebuggerEngine final
	{
	public:
		explicit IDebuggerEngine() = default;
		~IDebuggerEngine()		   = default;

	public:
		IDebuggerEngine& operator=(const IDebuggerEngine&) = default;
		IDebuggerEngine(const IDebuggerEngine&)			   = default;

	public:
		void Attach(pid_t pid)
		{
			if (ptrace(PTRACE_ATTACH, pid, nullptr, 0) == -1)
			{
				perror("dbg: Attach");
				return;
			}

			this->m_pid = pid;

			waitpid(m_pid, nullptr, 0);

			std::cout << "[+] Attached to process: " << m_pid << std::endl;
		}

		void SetBreakpoint(CAddr addr)
		{
			uintptr_t original_data = ptrace(PTRACE_PEEKTEXT, m_pid, addr, 0);

			if (original_data == -1)
			{
				perror("dbg: Peek");
				return;
			}

			uintptr_t data_with_int3 = (original_data & ~0xFF) | 0xCC; // Insert INT3 (0xCC)

			if (ptrace(PTRACE_POKETEXT, m_pid, addr, data_with_int3) == -1)
			{
				perror("dbg: Poke");
				return;
			}

			std::cout << "[+] Breakpoint set at: " << addr << std::endl;

			m_breakpoints[reinterpret_cast<uintptr_t>(addr)] = original_data; // Store original data
		}

		void ContinueExecution()
		{
			if (ptrace(PTRACE_CONT, m_pid, nullptr, 0) == -1)
			{
				perror("dbg: Cont");
				return;
			}

			int status;
			waitpid(m_pid, &status, 0);

			if (WIFSTOPPED(status) && WSTOPSIG(status) == SIGTRAP)
			{
				std::cout << "[!] Breakpoint hit." << std::endl;
			}
		}

		void Detach()
		{
			if (ptrace(PTRACE_DETACH, m_pid, nullptr, 0) == -1)
			{
				perror("dbg: Detach");
				return;
			}

			std::cout << "[-] Detached from process: " << m_pid << std::endl;
		}

		std::unordered_map<uintptr_t, uintptr_t>& Breakpoints()
		{
			return m_breakpoints;
		}

	private:
		pid_t									 m_pid;
		std::unordered_map<uintptr_t, uintptr_t> m_breakpoints;
	};
} // namespace LibDebugger
