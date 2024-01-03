/*
 *	========================================================
 *
 *	CxxKit
 * 	Copyright Western Company, all rights reserved.
 *
 * 	========================================================
 */

#include "String.hpp"
#include <utility>

namespace CxxKit
{
    CharType* StringView::Data()
    {
        return m_Data.data();
    }

    const CharType* StringView::CData() const
    {
        return m_Data.c_str();
    }

    SizeType StringView::Length() const
    {
        return m_Data.size();
    }

    bool StringView::operator==(const StringView &rhs) const
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

    bool StringView::operator==(const CharType *rhs) const
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

    bool StringView::operator!=(const StringView &rhs) const
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

    bool StringView::operator!=(const CharType *rhs) const
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

    StringView StringBuilder::Construct(const CharType *data)
    {
        if (!data ||
            *data == 0)
            return StringView(0);

        StringView view(strlen(data));
        view += data;

		return view;
    }

    const char* StringBuilder::FromInt(const char *fmt, int i)
    {
        if (!fmt)
            return ("-1");

        char *ret = new char[8 + string_length(fmt)];

        if (!ret)
            return ("-1");

        CharType result[8];
        if (!to_str(result, sizeof(int), i))
        {
            delete[] ret;
            return ("-1");
        }

        const auto fmt_len = string_length(fmt);
        const auto res_len = string_length(result);

        for (SizeType idx = 0; idx < fmt_len; ++idx)
        {
            if (fmt[idx] == '%') {
                SizeType result_cnt = idx;

                for (auto y_idx = idx; y_idx < res_len; ++y_idx) {
                    ret[result_cnt] = result[y_idx];
                    ++result_cnt;
                }

                break;
            }

            ret[idx] = fmt[idx];
        }

        return ret; /* Copy that ret into a buffer, Alloca allocates to the stack */
    }

    const char* StringBuilder::FromBool(const char *fmt, bool i)
    {
        if (!fmt)
            return ("?");

        const char *boolean_expr = i ? "true" : "false";
        char *ret = new char[i ? 4 : 5 + string_length(fmt)];

        if (!ret)
            return ("?");

        const auto fmt_len = string_length(fmt);
        const auto res_len = string_length(boolean_expr);

        for (SizeType idx = 0; idx < fmt_len; ++idx)
        {
            if (fmt[idx] == '%') {
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

    bool StringBuilder::Equals(const char *lhs, const char *rhs)
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

    const char *StringBuilder::Format(const char *fmt, const char *fmt2)
    {
        if (!fmt || !fmt2)
            return ("?");

        char *ret = new char[string_length(fmt2) + string_length(fmt2)];
        if (!ret)
            return ("?");

        for (SizeType idx = 0; idx < string_length(fmt); ++idx)
        {
            if (fmt[idx] == '%') {
                SizeType result_cnt = idx;
                for (SizeType y_idx = 0; y_idx < string_length(fmt2); ++y_idx)
                {
                    ret[result_cnt] = fmt2[y_idx];
                    ++result_cnt;
                }

                break;
            }

            ret[idx] = fmt[idx];
        }

        return ret;
    }

    StringView &StringView::operator+=(const CharType *rhs)
    {
        this->m_Data += rhs;
        this->m_Cur = this->m_Data.size();
        this->m_Sz = this->m_Data.size();

        return *this;
    }

    StringView &StringView::operator+=(const StringView &rhs)
    {
        this->m_Data += rhs.CData();
        this->m_Cur = this->m_Data.size();
        this->m_Sz = this->m_Data.size();

        return *this;
    }
} // namespace CxxKit
