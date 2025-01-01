/*
 *	========================================================
 *
 *	LibCompiler
 * 	Copyright (C) 2024 Theater Quality Corp, all rights reserved.
 *
 * 	========================================================
 */

/**
 * @file String.cxx
 * @author Amlal (amlal@el-mahrouss-logic.com)
 * @brief C++ string manipulation API.
 * @version 0.2
 * @date 2024-01-23
 *
 * @copyright Copyright (c) Theater Quality Corp.
 *
 */

#include <LibCompiler/NFC/String.h>

namespace LibCompiler
{
	CharType* StringView::Data()
	{
		return m_Data;
	}

	const CharType* StringView::CData() const
	{
		return m_Data;
	}

	SizeType StringView::Length() const
	{
		return strlen(m_Data);
	}

	bool StringView::operator==(const StringView& rhs) const
	{
		if (rhs.Length() != Length())
			return false;

		for (SizeType index = 0; index < Length(); ++index)
		{
			if (rhs.m_Data[index] != m_Data[index])
				return false;
		}

		return true;
	}

	bool StringView::operator==(const CharType* rhs) const
	{
		if (string_length(rhs) != Length())
			return false;

		for (SizeType index = 0; index < string_length(rhs); ++index)
		{
			if (rhs[index] != m_Data[index])
				return false;
		}

		return true;
	}

	bool StringView::operator!=(const StringView& rhs) const
	{
		if (rhs.Length() != Length())
			return false;

		for (SizeType index = 0; index < rhs.Length(); ++index)
		{
			if (rhs.m_Data[index] == m_Data[index])
				return false;
		}

		return true;
	}

	bool StringView::operator!=(const CharType* rhs) const
	{
		if (string_length(rhs) != Length())
			return false;

		for (SizeType index = 0; index < string_length(rhs); ++index)
		{
			if (rhs[index] == m_Data[index])
				return false;
		}

		return true;
	}

	StringView StringBuilder::Construct(const CharType* data)
	{
		if (!data || *data == 0)
			return StringView(0);

		StringView view(strlen(data));
		view += data;

		return view;
	}

	const char* StringBuilder::FromInt(const char* fmt, int i)
	{
		if (!fmt)
			return ("-1");

		auto  ret_len = 8 + string_length(fmt);
		char* ret	  = new char[ret_len];

		if (!ret)
			return ("-1");

		memset(ret, 0, ret_len);

		CharType result[sizeof(int64_t)];

		if (!to_str(result, sizeof(int64_t), i))
		{
			delete[] ret;
			return ("-1");
		}

		const auto fmt_len = string_length(fmt);
		const auto res_len = string_length(result);

		for (SizeType idx = 0; idx < fmt_len; ++idx)
		{
			if (fmt[idx] == '%')
			{
				SizeType result_cnt = idx;

				for (auto y_idx = 0; y_idx < res_len; ++y_idx)
				{
					ret[y_idx] = result[result_cnt];
					++result_cnt;
				}

				break;
			}

			ret[idx] = fmt[idx];
		}

		return ret; /* Copy that ret into a buffer, Alloca allocates to the stack */
	}

	const char* StringBuilder::FromBool(const char* fmt, bool i)
	{
		if (!fmt)
			return ("?");

		const char* boolean_expr = i ? "true" : "false";
		char*		ret			 = new char[i ? 4 : 5 + string_length(fmt)];

		if (!ret)
			return ("?");

		const auto fmt_len = string_length(fmt);
		const auto res_len = string_length(boolean_expr);

		for (SizeType idx = 0; idx < fmt_len; ++idx)
		{
			if (fmt[idx] == '%')
			{
				SizeType result_cnt = idx;

				for (auto y_idx = idx; y_idx < res_len; ++y_idx)
				{
					ret[result_cnt] = boolean_expr[y_idx];
					++result_cnt;
				}

				break;
			}

			ret[idx] = fmt[idx];
		}

		return ret;
	}

	bool StringBuilder::Equals(const char* lhs, const char* rhs)
	{
		if (string_length(rhs) != string_length(lhs))
			return false;

		for (SizeType index = 0; index < string_length(rhs); ++index)
		{
			if (rhs[index] != lhs[index])
				return false;
		}

		return true;
	}

	const char* StringBuilder::Format(const char* fmt, const char* fmtRight)
	{
		if (!fmt || !fmtRight)
			return ("?");

		char* ret = new char[string_length(fmtRight) + string_length(fmtRight)];
		if (!ret)
			return ("?");

		for (SizeType idx = 0; idx < string_length(fmt); ++idx)
		{
			if (fmt[idx] == '%')
			{
				SizeType result_cnt = idx;

				for (SizeType y_idx = 0; y_idx < string_length(fmtRight); ++y_idx)
				{
					ret[result_cnt] = fmtRight[y_idx];
					++result_cnt;
				}

				break;
			}

			ret[idx] = fmt[idx];
		}

		return ret;
	}

	StringView& StringView::operator+=(const CharType* rhs)
	{
		if (strlen(rhs) > this->m_Sz)
		{
			throw std::runtime_error("out_of_bounds: StringView");
		}

		memcpy(this->m_Data + this->m_Cur, rhs, strlen(rhs));
		this->m_Cur += strlen(rhs);

		return *this;
	}

	StringView& StringView::operator+=(const StringView& rhs)
	{
		if (rhs.m_Cur > this->m_Sz)
		{
			throw std::runtime_error("out_of_bounds: StringView");
		}

		memcpy(this->m_Data + this->m_Cur, rhs.CData(), strlen(rhs.CData()));
		this->m_Cur += strlen(rhs.CData());

		return *this;
	}
} // namespace LibCompiler
