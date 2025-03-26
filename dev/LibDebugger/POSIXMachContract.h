/***
	(C) 2025 Amlal El Mahrouss
 */

#pragma once

#ifdef _WIN32
#error Windows doesn't have a POSIX/Mach subsystem, please combine with windows instead.
#endif

/// @file POSIXMachContract.h
/// @brief POSIX/Mach debugger.

#include <LibDebugger/DebuggerContract.h>

#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/user.h>
#include <unistd.h>
#include <stdint.h>

#ifdef __APPLE__
#include <mach/mach.h>
#include <mach/mach_error.h>

#define PTRACE_ATTACH	PT_ATTACHEXC
#define PTRACE_DETACH	PT_DETACH
#define PTRACE_POKETEXT PT_WRITE_I
#define PTRACE_CONT		PT_CONTINUE
#define PTRACE_PEEKTEXT PT_READ_I
#endif

namespace LibDebugger::POSIX
{
	/// \brief POSIXMachContract engine interface class in C++
	/// \author Amlal El Mahrouss
	class POSIXMachContract final : public DebuggerContract
	{
	public:
		explicit POSIXMachContract()  = default;
		~POSIXMachContract() override = default;

	public:
		POSIXMachContract& operator=(const POSIXMachContract&) = default;
		POSIXMachContract(const POSIXMachContract&)			   = default;

	public:
		bool Attach(ProcessID pid) noexcept override
		{
#ifdef __APPLE__
			if (pid == 0)
				return false;

			this->m_pid = pid;
			return true;
#else

			if (ptrace(PTRACE_ATTACH, pid, nullptr, 0) == -1)
			{
				return false;
			}

			this->m_pid = pid;

			waitpid(m_pid, nullptr, 0);

			return true;
#endif
		}

		bool Break(CAddress addr) noexcept override
		{
#ifdef __APPLE__
			task_read_t task;
			task_for_pid(mach_task_self(), m_pid, &task);
			kern_return_t ret = task_suspend(task);

			return ret == KERN_SUCCESS;
#else
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
#endif
		}

		bool Continue() noexcept override
		{
#ifdef __APPLE__
			task_read_t task;
			task_for_pid(mach_task_self(), m_pid, &task);
			kern_return_t ret = task_resume(task);

			return ret == KERN_SUCCESS;
#else
			if (ptrace(PTRACE_CONT, m_pid, nullptr, 0) == -1)
			{

				return false;
			}

			int status;
			waitpid(m_pid, &status, 0);

			return WIFSTOPPED(status) && WSTOPSIG(status) == SIGTRAP;
#endif
		}

		bool Detach() noexcept override
		{
#ifdef __APPLE__
			this->Continue();

			task_read_t task;
			task_for_pid(mach_task_self(), m_pid, &task);

			kern_return_t kr = mach_port_deallocate(mach_task_self(), task);

			return kr = KERN_SUCCESS;
#else
			return ptrace(PTRACE_DETACH, m_pid, nullptr, 0) == -1;
#endif
		}

	private:
		ProcessID m_pid{0};
	};
} // namespace LibDebugger::POSIX
