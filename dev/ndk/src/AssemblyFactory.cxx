/* -------------------------------------------

	Copyright ZKA Technologies

------------------------------------------- */

#include <ndk/Asm/Asm.hxx>
#include <ndk/NFC/ErrorID.hxx>

/**
 * @file AssemblyFactory.cxx
 * @author amlal (amlal@zeta.com)
 * @brief Assembler Kit
 * @version 0.1
 * @date 2024-01-27
 *
 * @copyright Copyright (c) 2024, ZKA Technologies
 *
 */

#include <iostream>

//! @file Asm.cpp
//! @brief AssemblyKit source implementation.

namespace NDK
{
	///! @brief Compile for specific format (ELF, PEF, ZBIN)
	Int32 AssemblyFactory::Compile(std::string& sourceFile,
								   const Int32& arch) noexcept
	{
		if (sourceFile.length() < 1 || !fMounted)
			return NDK_UNIMPLEMENTED;

		return fMounted->CompileToFormat(sourceFile, arch);
	}

	///! @brief mount assembly backend.
	void AssemblyFactory::Mount(AssemblyInterface* mountPtr) noexcept
	{
		if (mountPtr)
		{
			fMounted = mountPtr;
		}
	}

	///! @brief Unmount assembler.
	AssemblyInterface* AssemblyFactory::Unmount() noexcept
	{
		auto mount_prev = fMounted;

		if (mount_prev)
		{
			fMounted = nullptr;
		}

		return mount_prev;
	}
} // namespace NDK
