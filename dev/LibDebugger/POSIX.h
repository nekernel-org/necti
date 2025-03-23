/***
	(C) 2025 Amlal El Mahrouss
 */

#pragma once

#ifdef _WIN32
#error Windows doesn't have a POSIX subsystem, please combine with windows instead.
#endif

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

namespace LibDebugger::POSIX
{
#ifdef __APPLE__
	typedef caddr_t CAddr;
#else
	typedef char* CAddr;
#endif

	/// \brief Debugger engine interface class in C++
	/// \author Amlal El Mahrouss
	class Debugger final
	{
	public:
		explicit Debugger() = default;
		~Debugger()			= default;

	public:
		Debugger& operator=(const Debugger&) = default;
		Debugger(const Debugger&)			 = default;

	public:
		bool Attach(pid_t pid)
		{
			if (ptrace(PTRACE_ATTACH, pid, nullptr, 0) == -1)
			{
				return false;
			}

			this->m_pid = pid;

			waitpid(m_pid, nullptr, 0);

			return true;
		}

		bool Break(CAddr addr)
		{
			uintptr_t original_data = ptrace(PTRACE_PEEKTEXT, m_pid, addr, 0);

			if (original_data == -1)
			{
				return false;
			}

			constexpr uint8_t kInt3x86 = 0xCC;

			uintptr_t data_with_int3 = (original_data & ~0xFF) | kInt3x86; // Insert INT3 (0xCC)

			if (ptrace(PTRACE_POKETEXT, m_pid, addr, data_with_int3) == -1)
			{
				return false;
			}

			m_breakpoints[reinterpret_cast<uintptr_t>(addr)] = original_data; // Store original data

			return true;
		}

		bool Continue()
		{
			if (ptrace(PTRACE_CONT, m_pid, nullptr, 0) == -1)
			{
				return false;
			}

			int status;
			waitpid(m_pid, &status, 0);

			if (WIFSTOPPED(status) && WSTOPSIG(status) == SIGTRAP)
			{
				std::cout << "[!] Breakpoint has been hit!" << std::endl;
			}

			return true;
		}

		bool Detach()
		{
			if (ptrace(PTRACE_DETACH, m_pid, nullptr, 0) == -1)
			{
				return false;
			}

			return true;
		}

		std::unordered_map<uintptr_t, uintptr_t>& Get()
		{
			return m_breakpoints;
		}

	private:
		pid_t									 m_pid;
		std::unordered_map<uintptr_t, uintptr_t> m_breakpoints;
	};
} // namespace LibDebugger::POSIX
