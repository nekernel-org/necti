/* -------------------------------------------

	Copyright ZKA Technologies

------------------------------------------- */

#pragma once

#include <ndkdll/Asm/Asm.hxx>
#include <vector>

namespace NDK
{
	inline auto cInvalidLang = "?";

	/// @brief Compiler backend, implements a frontend, such as C, C++...
	/// See Toolchain, for some examples.
	class CompilerBackend
	{
	public:
		explicit CompilerBackend() = default;
		virtual ~CompilerBackend() = default;

		NDK_COPY_DEFAULT(CompilerBackend);

		// NOTE: cast this to your user defined ast.
		typedef void* AstType;

		//! @brief Compile a syntax tree ouf of the text.
		//! Also takes the source file name for metadata.

		virtual bool Compile(const std::string& text, const char* file) = 0;

		//! @brief What language are we dealing with?
		virtual const char* Language()
		{
			return cInvalidLang;
		}

		virtual bool IsValid()
		{
			return strcmp(this->Language(), cInvalidLang);
		}
	};

	struct SyntaxLeafList;
	struct SyntaxLeafList;
	struct CompilerKeyword;

	/// we want to do that because to separate keywords.
	enum KeywordKind
	{
		eKeywordKindNamespace,
		eKeywordKindFunctionStart,
		eKeywordKindFunctionEnd,
		eKeywordKindVariable,
		eKeywordKindVariablePtr,
		eKeywordKindType,
		eKeywordKindTypePtr,
		eKeywordKindExpressionBegin,
		eKeywordKindExpressionEnd,
		eKeywordKindArgSeparator,
		eKeywordKindBodyStart,
		eKeywordKindBodyEnd,
		eKeywordKindClass,
		eKeywordKindPtrAccess,
		eKeywordKindAccess,
		eKeywordKindIf,
		eKeywordKindElse,
		eKeywordKindElseIf,
		eKeywordKindVariableAssign,
		eKeywordKindVariableDec,
		eKeywordKindVariableInc,
		eKeywordKindConstant,
		eKeywordKindTypedef,
		eKeywordKindEndInstr,
		eKeywordKindSpecifier,
		eKeywordKindInvalid,
		eKeywordKindReturn,
		eKeywordKindCommentInline,
		eKeywordKindCommentMultiLineStart,
		eKeywordKindCommentMultiLineEnd,
		eKeywordKindEq,
		eKeywordKindNotEq,
		eKeywordKindGreaterEq,
		eKeywordKindLessEq,
		eKeywordKindPtr,
	};

	/// \brief Compiler keyword information struct.
	struct CompilerKeyword
	{
		std::string keyword_name;
		KeywordKind keyword_kind = eKeywordKindInvalid;
	};
	struct SyntaxLeafList final
	{
		struct SyntaxLeaf final
		{
			Int32 fUserType;
#ifdef __PK_USE_STRUCT_INSTEAD__
			CompilerKeyword fUserData;
#else
			std::string fUserData;
#endif

			std::string		   fUserValue;
			struct SyntaxLeaf* fNext;
		};

		std::vector<SyntaxLeaf> fLeafList;
		SizeType				fNumLeafs;

		size_t SizeOf()
		{
			return fNumLeafs;
		}
		std::vector<SyntaxLeaf>& Get()
		{
			return fLeafList;
		}
		SyntaxLeaf& At(size_t index)
		{
			return fLeafList[index];
		}
	};

	/// find the perfect matching word in a haystack.
	/// \param haystack base string
	/// \param needle the string we search for.
	/// \return if we found it or not.
	inline bool find_word(const std::string& haystack,
						  const std::string& needle) noexcept
	{
		auto index = haystack.find(needle);

		// check for needle validity.
		if (index == std::string::npos)
			return false;

		// declare lambda
		auto not_part_of_word = [&](int index) {
			if (std::isspace(haystack[index]) || std::ispunct(haystack[index]))
				return true;

			if (index < 0 || index >= haystack.size())
				return true;

			return false;
		};

		return not_part_of_word(index - 1) && not_part_of_word(index + needle.size());
	}

	/// find a word within strict conditions and returns a range of it.
	/// \param haystack
	/// \param needle
	/// \return position of needle.
	inline std::size_t find_word_range(const std::string& haystack,
									   const std::string& needle) noexcept
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
} // namespace NDK
