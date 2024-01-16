/*
 *	========================================================
 *
 *	MPCC
 * 	Copyright 2024, Mahrouss Logic, all rights reserved.
 *
 * 	========================================================
 */

#ifndef __CXXKIT_DEFINES_HPP__
#define __CXXKIT_DEFINES_HPP__

#ifndef Yes
#define Yes true
#endif // ifndef Yes

#ifndef No
#define No false
#endif // ifndef No

#include <stdint.h>

#define SizeType size_t

#define VoidPtr void*
#define voidPtr VoidPtr

#define UIntPtr uintptr_t

#define Int64 int64_t
#define UInt64 uint64_t

#define Int32 int32_t
#define UInt32 uint32_t

#define Bool bool

#define Int16 int16_t
#define UInt16 uint16_t

#define Int8 int8_t
#define UInt8 uint8_t

#define CharType char
#define Boolean bool

#include <new>
#include <cstring>
#include <cassert>

#define nullPtr std::nullptr_t

#define MUST_PASS(E) assert(E)

#ifndef __FORCE_STRLEN
#	define __FORCE_STRLEN 1

#	define string_length(len) strlen(len)
#endif

#ifndef __FORCE_MEMCPY
#	define __FORCE_MEMCPY 1

#	define rt_copy_memory(dst, src, len) memcpy(dst, src, len)
#endif

#define CXXKIT_COPY_DELETE(KLASS)                                                                                         \
    KLASS &operator=(const KLASS &) = delete;                                                                          \
    KLASS(const KLASS &) = delete;


#define CXXKIT_COPY_DEFAULT(KLASS)                                                                                        \
    KLASS &operator=(const KLASS &) = default;                                                                         \
    KLASS(const KLASS &) = default;


#define CXXKIT_MOVE_DELETE(KLASS)                                                                                         \
    KLASS &operator=(KLASS &&) = delete;                                                                               \
    KLASS(KLASS &&) = delete;


#define CXXKIT_MOVE_DEFAULT(KLASS)                                                                                        \
    KLASS &operator=(KLASS &&) = default;                                                                              \
    KLASS(KLASS &&) = default;



#include <ctime>
#include <string>
#include <fstream>

namespace CompilerKit
{
    inline constexpr int BASE_YEAR = 1900;

    inline std::string current_date()
    {
		auto time_data = time(nullptr);
		auto time_struct = gmtime(&time_data);

        std::string fmt = std::to_string(BASE_YEAR + time_struct->tm_year);
        fmt += "-";
        fmt += std::to_string(time_struct->tm_mon + 1);
        fmt += "-";
        fmt += std::to_string(time_struct->tm_mday);

        return fmt;
    }

	inline bool to_str(CharType *str, Int32 limit, Int32 base)
    {
        if (limit == 0)
            return false;

        Int32 copy_limit = limit;
        Int32 cnt = 0;
        Int32 ret = base;

        while (limit != 1) {
            ret = ret % 10;
            str[cnt] = ret;

            ++cnt;
            --limit;
            --ret;
        }

        str[copy_limit] = '\0';
        return true;
    }
}

typedef char char_type;

#define kObjectFileExt     ".o"
#define kAsmFileExt64x0    ".64x"

#endif /* ifndef __CXXKIT_DEFINES_HPP__ */
