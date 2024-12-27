/* -------------------------------------------

	Copyright (C) 2024 Theater Quality Inc, all rights reserved

------------------------------------------- */

#include <LibCompiler/AAL/AssemblyInterface.h>
#include <LibCompiler/NFC/ErrorID.h>

/**
 * @file AssemblyFactory.cxx
 * @author amlal (amlal@zka.com)
 * @brief Assembler Kit
 * @version 0.1
 * @date 2024-01-27
 *
 * @copyright Copyright (c) 2024 Theater Quality Inc
 *
 */

#include <iostream>

//! @file Asm.cpp
//! @brief AssemblyKit source implementation.

namespace LibCompiler
{
	///! @brief Compile for specific format (ELF, PEF, ZBIN)
	Int32 AssemblyFactory::Compile(std::string& sourceFile,
								   const Int32& arch) noexcept
	{
		if (sourceFile.length() < 1 || !fMounted)
			return TOOLCHAINKIT_UNIMPLEMENTED;

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
} // namespace LibCompiler
