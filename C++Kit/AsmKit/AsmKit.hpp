/*
 *	========================================================
 *
 *	C++Kit
 * 	Copyright WestCo, all rights reserved.
 *
 * 	========================================================
 */

#pragma once

#include <CompilerKit/Compiler.hpp>
#include <C++Kit/Defines.hpp>
#include <C++Kit/StdKit/String.hpp>

namespace CxxKit
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
		// Example C++ -> Assembly -> AE object.
		virtual Int32 CompileToFormat(StringView& src, Int32 arch) = 0;

	};
	
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
			kArchARM64,
			kArchPowerPC,
			kArchARC,
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
