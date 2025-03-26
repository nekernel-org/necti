/***
	(C) 2025 Amlal El Mahrouss
 */

#pragma once

#include <iostream>
#include <unordered_map>

namespace LibDebugger
{
    typedef uint64_t ProcessID;
    typedef char* CAddress;

	/// \brief Debugger contract class in C++, as per the design states.
	/// \author Amlal El Mahrouss
	class DebuggerContract
	{
	public:
		explicit DebuggerContract() = default;
		virtual ~DebuggerContract()		  = default;

	public:
		DebuggerContract& operator=(const DebuggerContract&) = default;
		DebuggerContract(const DebuggerContract&)			 = default;

	public:
		virtual bool Attach(ProcessID pid) noexcept = 0;
		virtual bool Break(CAddress addr) noexcept = 0;
		virtual bool Continue() noexcept = 0;
		virtual bool Detach() noexcept = 0;

		virtual std::unordered_map<uintptr_t, uintptr_t>& Get()
		{
			return m_breakpoints;
		}

	protected:
		pid_t									 m_pid;
		std::unordered_map<uintptr_t, uintptr_t> m_breakpoints;
	};
} // namespace LibDebugger::POSIX
