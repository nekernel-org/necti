/* -------------------------------------------

	Copyright ZKA Technologies

------------------------------------------- */

#ifndef __NDK_DEFINES_HXX__
#define __NDK_DEFINES_HXX__

#ifndef Yes
#define Yes true
#endif // ifndef Yes

#ifndef No
#define No false
#endif // ifndef No

#define SizeType size_t

#define VoidPtr void*
#define voidPtr VoidPtr

#define UIntPtr uintptr_t

#define Int64  int64_t
#define UInt64 uint64_t

#define Int32  int
#define UInt32 unsigned

#define Bool bool

#define Int16  int16_t
#define UInt16 uint16_t

#define Int8  int8_t
#define UInt8 uint8_t

#define CharType char
#define Boolean	 bool

#include <cstdint>
#include <cassert>
#include <cstring>
#include <new>
#include <iostream>
#include <utility>
#include <cctype>
#include <cstdio>
#include <fstream>
#include <memory>
#include <string>
#include <filesystem>
#include <vector>

#define nullPtr std::nullptr_t

#define MUST_PASS(E) assert(E)

#ifndef __FORCE_STRLEN
#define __FORCE_STRLEN 1

#define string_length(len) strlen(len)
#endif

#ifndef __FORCE_MEMCPY
#define __FORCE_MEMCPY 1

#define rt_copy_memory(dst, src, len) memcpy(dst, src, len)
#endif

#define NDK_COPY_DELETE(KLASS)               \
	KLASS& operator=(const KLASS&) = delete; \
	KLASS(const KLASS&)			   = delete;

#define NDK_COPY_DEFAULT(KLASS)               \
	KLASS& operator=(const KLASS&) = default; \
	KLASS(const KLASS&)			   = default;

#define NDK_MOVE_DELETE(KLASS)          \
	KLASS& operator=(KLASS&&) = delete; \
	KLASS(KLASS&&)			  = delete;

#define NDK_MOVE_DEFAULT(KLASS)          \
	KLASS& operator=(KLASS&&) = default; \
	KLASS(KLASS&&)			  = default;

#include <ctime>
#include <fstream>
#include <string>
#include <vector>

namespace NDK
{
	inline constexpr int cBaseYear = 1900;

	typedef std::string String;

	inline String current_date() noexcept
	{
		auto time_data	 = time(nullptr);
		auto time_struct = gmtime(&time_data);

		String fmt = std::to_string(cBaseYear + time_struct->tm_year);
		
		fmt += "-";
		fmt += std::to_string(time_struct->tm_mon + 1);
		fmt += "-";
		fmt += std::to_string(time_struct->tm_mday);

		return fmt;
	}

	inline bool to_str(CharType* str, Int32 limit, Int32 base) noexcept
	{
		if (limit == 0)
			return false;

		Int32 copy_limit = limit;
		Int32 cnt		 = 0;
		Int32 ret		 = base;

		while (limit != 1)
		{
			ret		 = ret % 10;
			str[cnt] = ret;

			++cnt;
			--limit;
			--ret;
		}

		str[copy_limit] = '\0';
		return true;
	}

	using String = std::basic_string<CharType>;
} // namespace NDK

#define PACKED __attribute__((packed))

typedef char char_type;

#define kObjectFileExt ".obj"
#define kBinaryFileExt ".bin"

#define kAsmFileExts                                        \
	{                                                       \
		".64x", ".32x", ".masm", ".s", ".S", ".asm", ".x64" \
	}

#define kAsmFileExtsMax 7

#define NDK_MODULE(name) extern "C" int name(int argc, char** argv)

#ifdef MSVC
#pragma scalar_storage_order big-endian
#endif // ifdef MSVC

#endif /* ifndef __NDK_DEFINES_HXX__ */
