/*
 *	========================================================
 *
 *	MPCC
 * 	Copyright 2024, Mahrouss Logic, all rights reserved.
 *
 * 	========================================================
 */

#pragma once

#include <CompilerKit/Compiler.hpp>
#include <CompilerKit/Defines.hpp>
#include <CompilerKit/StdKit/String.hpp>

namespace CompilerKit
{
	//
	//	@brief Frontend to Assembly mountpoint.
	//
	class AssemblyMountpoint
	{
	public:
    	explicit AssemblyMountpoint() = default;
    	virtual ~AssemblyMountpoint() = default;

		CXXKIT_COPY_DEFAULT(AssemblyMountpoint);

		//@ brief compile to object file.
		// Example C++ -> MASM -> AE object.
		virtual Int32 CompileToFormat(StringView& src, Int32 arch) = 0;

	};
	
    /// @brief Simple assembly factory
    class AssemblyFactory final
    {
    public:
        explicit AssemblyFactory() = default;
        ~AssemblyFactory() = default;

		CXXKIT_COPY_DEFAULT(AssemblyFactory);

	public:
		enum
		{
			kArchAMD64,
			kArch32x0,
			kArch64x0,
			kArchRISCV,
			kArchUnknown,
		};

        Int32 Compile(StringView& sourceFile, const Int32& arch) noexcept;

		void Mount(AssemblyMountpoint* mountPtr) noexcept;
		AssemblyMountpoint* Unmount() noexcept;

	private:
		AssemblyMountpoint* fMounted{ nullptr };

    };

	class PlatformAssembler
	{
	public:
		explicit PlatformAssembler() = default;
		~PlatformAssembler() = default;

		CXXKIT_COPY_DEFAULT(PlatformAssembler);

		virtual std::string CheckLine(std::string &line, const std::string &file) = 0;
		virtual bool WriteLine(std::string &line, const std::string &file) = 0;
		virtual bool WriteNumber(const std::size_t &pos, std::string &from_what) = 0;

	};

#ifdef __ASM_NEED_64x0__

	class PlatformAssembler64x0 final : public PlatformAssembler
	{
	public:
		explicit PlatformAssembler64x0() = default;
		~PlatformAssembler64x0() = default;

		CXXKIT_COPY_DEFAULT(PlatformAssembler64x0);

		virtual std::string CheckLine(std::string &line, const std::string &file) override;
		virtual bool WriteLine(std::string &line, const std::string &file) override;
		virtual bool WriteNumber(const std::size_t& pos, std::string& from_what) override;

	};

#endif // __ASM_NEED_64x0__

#ifdef __ASM_NEED_32x0__

	class PlatformAssembler32x0 final : public PlatformAssembler
	{
	public:
		explicit PlatformAssembler32x0() = default;
		~PlatformAssembler32x0() = default;

		CXXKIT_COPY_DEFAULT(PlatformAssembler32x0);

		virtual std::string CheckLine(std::string &line, const std::string &file) override;
		virtual bool WriteLine(std::string &line, const std::string &file) override;
		virtual bool WriteNumber(const std::size_t& pos, std::string& from_what) override;

	};

#endif // __ASM_NEED_32x0__

	union NumberCast final
    {
        explicit NumberCast(UInt64 raw) : raw(raw) {}
		~NumberCast() { raw = 0; }

        char number[8];
        UInt64 raw;
    };
}

#ifdef __MODULE_NEED__
#	define MPCC_MODULE(name) int name(int argc, char** argv)
#else
#	define MPCC_MODULE(name) int main(int argc, char** argv)
#endif /* ifdef __MODULE_NEED__ */


