/*
 *	========================================================
 *
 *	C++Kit
 * 	Copyright Mahrouss Logic, all rights reserved.
 *
 * 	========================================================
 */

#pragma once

#include <CompilerKit/AsmKit/AsmKit.hpp>

namespace ParserKit
{
    using namespace CompilerKit;

    class CompilerBackend
    {
    public:
        explicit CompilerBackend() = default;
        virtual ~CompilerBackend() = default;

        CXXKIT_COPY_DEFAULT(CompilerBackend);

        // NOTE: cast this to your user defined ast.
        typedef void* AstType;

        //! @brief Compile a syntax tree ouf of the text.
        //! Also takes the source file name for metadata.
        
        virtual bool Compile(const std::string& text, const char* file) = 0;

        //! @brief What language are we dealing with?
        virtual const char* Language() { return "Generic Language"; }

    };

    struct SyntaxLeafList;
    struct SyntaxLeafList;

    struct SyntaxLeafList final
    {
        struct SyntaxLeaf final
        {
            Int32 fUserType;
            std::string fUserData;
            std::string fUserValue;
            struct SyntaxLeaf* fNext;
        };
        
        std::vector<SyntaxLeaf> fLeafList;
        SizeType fNumLeafs;

        size_t SizeOf() { return fNumLeafs; }
        std::vector<SyntaxLeaf>& Get() { return fLeafList; }
        SyntaxLeaf& At(size_t index) { return fLeafList[index]; }

    };

    /// find the perfect matching word in a haystack.
    /// \param haystack base string
    /// \param needle the string we search for.
    /// \return if we found it or not.
    inline bool find_word(const std::string& haystack,const std::string& needle) noexcept
    {
        auto index = haystack.find(needle);

        // check for needle validity.
        if (index == std::string::npos)
            return false;

        // declare lambda
        auto not_part_of_word = [&](int index){
            if (std::isspace(haystack[index]) || std::ispunct(haystack[index]))
                return true;

            if (index < 0 || index >= haystack.size())
                return true;

            return false;
        };

        return not_part_of_word(index - 1) &&
                not_part_of_word(index + needle.size());
    }

    /// find a word within strict conditions and returns a range of it.
    /// \param haystack
    /// \param needle
    /// \return position of needle.
    inline std::size_t find_word_range(const std::string& haystack, const std::string& needle) noexcept
    {
        auto index = haystack.find(needle);

        // check for needle validity.
        if (index == std::string::npos)
            return false;

        if (!isalnum((haystack[index + needle.size() + 1])) &&
                !isdigit(haystack[index + needle.size() + 1]) &&
                !isalnum((haystack[index - needle.size() - 1])) &&
                !isdigit(haystack[index - needle.size() - 1]))
        {
            return index;
        }

        return false;
    }
}