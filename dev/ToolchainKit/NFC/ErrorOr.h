/*
 *	========================================================
 *
 *	ToolchainKit
 * 	Copyright (C) 2024, EL Mahrouss Logic, all rights reserved.
 *
 * 	========================================================
 */

#pragma once

#include <ToolchainKit/Defines.h>
#include <ToolchainKit/NFC/Ref.h>

namespace ToolchainKit
{
	using ErrorT = UInt32;

	template <typename T>
	class ErrorOr final
	{
	public:
		ErrorOr()  = default;
		~ErrorOr() = default;

	public:
		explicit ErrorOr(Int32 err)
			: mId(err)
		{
		}

		explicit ErrorOr(nullPtr Null)
		{
		}

		explicit ErrorOr(T Class)
			: mRef(Class)
		{
		}

		ErrorOr& operator=(const ErrorOr&) = default;
		ErrorOr(const ErrorOr&)			   = default;

		Ref<T> Leak()
		{
			return mRef;
		}

		operator bool()
		{
			return mRef;
		}

	private:
		Ref<T> mRef;
		Int32  mId{0};
	};

	using ErrorOrAny = ErrorOr<voidPtr>;

} // namespace ToolchainKit
