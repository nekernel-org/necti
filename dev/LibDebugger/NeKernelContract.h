
/***
	(C) 2025 Amlal El Mahrouss
 */

#ifndef LD_NEKERNEL_CONTRACT_H
#define LD_NEKERNEL_CONTRACT_H

#include <LibDebugger/DebuggerContract.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

#define kDebugUnboundPort 0x0FEED

#define kDebugMag0 'K'
#define kDebugMag1 'D'
#define kDebugMag2 'B'
#define kDebugMag3 'G'

#define kDebugSourceFile 23
#define kDebugLine		 33
#define kDebugTeam		 43
#define kDebugEOP		 49

namespace LibDebugger::NeKernel
{
    class NeKernelContract;

	namespace Detail
	{
        class NeKernelPortHeader;

		inline constexpr size_t kDebugTypeLen = 256U;

		typedef char rt_debug_type[kDebugTypeLen];

		class NeKernelPortHeader final
		{
		public:
			int16_t fPort;
			int16_t fPortBsy;
		};
	} // namespace Detail

	class NeKernelContract : public DebuggerContract
	{
	public:
		NeKernelContract();
		virtual ~NeKernelContract();

		// Override additional methods from DebuggerContract
		virtual bool Attach(std::string path, std::string argv, ProcessID& pid) noexcept override;
		virtual bool Breakpoint(std::string symbol) noexcept override;
		virtual bool Break() noexcept override;
		virtual bool Continue() noexcept override;
		virtual bool Detach() noexcept override;

	private:
		std::string m_ip_address;
		std::string m_port;
	};
} // namespace LibDebugger::NeKernel

#endif // LD_NEKERNEL_CONTRACT_H