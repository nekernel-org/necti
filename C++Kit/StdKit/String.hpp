/*
 *	========================================================
 *
 *	CxxKit
 * 	Copyright Western Company, all rights reserved.
 *
 * 	========================================================
 */

#pragma once

#include <C++Kit/Defines.hpp>
#include <C++Kit/StdKit/ErrorOr.hpp>

namespace CxxKit
{
    class StringView final
    {
      public:
        StringView() = delete;

        explicit StringView(SizeType Sz) : m_Sz(Sz)
        {
            
        }

        ~StringView() = default;

        CXXKIT_COPY_DEFAULT(StringView);

        CharType *Data();
        const CharType *CData() const;
        SizeType Length() const;

        bool operator==(const CharType *rhs) const;
        bool operator!=(const CharType *rhs) const;

        bool operator==(const StringView &rhs) const;
        bool operator!=(const StringView &rhs) const;

        StringView &operator+=(const CharType *rhs);
        StringView &operator+=(const StringView &rhs);

        operator bool()
        {
            return m_Data.empty() == false;
        }

        bool operator!()
        {
            return m_Data.empty() == true;
        }

      private:
        std::basic_string<char> m_Data{""};
        SizeType m_Sz{0};
        SizeType m_Cur{0};

        friend class StringBuilder;

    };

    struct StringBuilder final
    {
        static StringView Construct(const CharType *data);
        static const char* FromInt(const char *fmt, int n);
        static const char* FromBool(const char *fmt, bool n);
        static const char* Format(const char *fmt, const char* from);
        static bool Equals(const char *lhs, const char *rhs);

    };
} // namespace CxxKit
