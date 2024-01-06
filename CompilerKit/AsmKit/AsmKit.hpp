/*
 *	========================================================
 *
 *	C++Kit
 * 	Copyright Mahrouss Logic, all rights reserved.
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
}
