/*
 * ========================================================
 *
 *      ToolchainKit
 *      Copyright ZKA Web Services Co, all rights reserved.
 *
 * ========================================================
 */

#pragma once

#include <ToolchainKit/Defines.h>

#define kAEMag0 'A'
#define kAEMag1 'E'

#define kAESymbolLen (255)
#define kAEPad		 (8)
#define kAEMagLen	 (2)
#define kAENullType	 (0x00)

// Advanced Executable File Format for MetroLink.
// Reloctable by offset is the default strategy.
// You can also relocate at runtime but that's up to the operating system
// loader.

namespace ToolchainKit
{
	// @brief Advanced Executable Header
	// One thing to keep in mind.
	// This object format, is reloctable.
	typedef struct AEHeader final
	{
		CharType fMagic[kAEMagLen];
		CharType fArch;
		CharType fSubArch;
		SizeType fCount;
		CharType fSize;
		SizeType fStartCode;
		SizeType fCodeSize;
		CharType fPad[kAEPad];
	} PACKED AEHeader, *AEHeaderPtr;

	// @brief Advanced Executable Record.
	// Could be data, code or bss.
	// fKind must be filled with PEF fields.

	typedef struct AERecordHeader final
	{
		CharType fName[kAESymbolLen];
		SizeType fKind;
		SizeType fSize;
		SizeType fFlags;
		UIntPtr	 fOffset;
		CharType fPad[kAEPad];
	} PACKED AERecordHeader, *AERecordHeaderPtr;

	enum
	{
		kKindRelocationByOffset	 = 0x23f,
		kKindRelocationAtRuntime = 0x34f,
	};
} // namespace ToolchainKit

// provide operator<< for AE

inline std::ofstream& operator<<(std::ofstream& fp, ToolchainKit::AEHeader& container)
{
	fp.write((char*)&container, sizeof(ToolchainKit::AEHeader));

	return fp;
}

inline std::ofstream& operator<<(std::ofstream&				   fp,
								 ToolchainKit::AERecordHeader& container)
{
	fp.write((char*)&container, sizeof(ToolchainKit::AERecordHeader));

	return fp;
}

inline std::ifstream& operator>>(std::ifstream& fp, ToolchainKit::AEHeader& container)
{
	fp.read((char*)&container, sizeof(ToolchainKit::AEHeader));
	return fp;
}

inline std::ifstream& operator>>(std::ifstream&				   fp,
								 ToolchainKit::AERecordHeader& container)
{
	fp.read((char*)&container, sizeof(ToolchainKit::AERecordHeader));
	return fp;
}

namespace ToolchainKit::Utils
{
	/**
	 * @brief AE Reader protocol
	 *
	 */
	class AEReadableProtocol final
	{
	public:
		std::ifstream FP;

	public:
		explicit AEReadableProtocol() = default;
		~AEReadableProtocol()		  = default;

		TOOLCHAINKIT_COPY_DELETE(AEReadableProtocol);

		/**
		 * @brief Read AE Record headers.
		 *
		 * @param raw the containing buffer
		 * @param sz it's size (1 = one AERecordHeader, 2 two AERecordHeader(s))
		 * @return AERecordHeaderPtr
		 */
		AERecordHeaderPtr Read(char* raw, std::size_t sz)
		{
			if (!raw)
				return nullptr;

			return this->_Read<AERecordHeader>(raw, sz * sizeof(AERecordHeader));
		}

	private:
		/**
		 * @brief Implementation of Read for raw classes.
		 *
		 * @tparam TypeClass The class to read.
		 * @param raw the buffer
		 * @param sz the size
		 * @return TypeClass* the returning class.
		 */
		template <typename TypeClass>
		TypeClass* _Read(char* raw, std::size_t sz)
		{
			FP.read(raw, std::streamsize(sz));
			return reinterpret_cast<TypeClass*>(raw);
		}
	};
} // namespace ToolchainKit::Utils
