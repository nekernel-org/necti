/* -------------------------------------------

  Copyright (C) 2024-2025 Amlal EL Mahrouss, all rights reserved

------------------------------------------- */

#pragma once

#include <LibCompiler/CodeGen.h>

#define LC_COMPILER_FRONTEND : public LibCompiler::CompilerFrontendInterface

namespace LibCompiler {
inline static auto kInvalidFrontend = "?";

/// @brief Compiler backend, implements a frontend, such as C, C++...
/// See Toolchain, for some examples.
class CompilerFrontendInterface {
 public:
  explicit CompilerFrontendInterface() = default;
  virtual ~CompilerFrontendInterface() = default;

  LIBCOMPILER_COPY_DEFAULT(CompilerFrontendInterface);

  // NOTE: cast this to your user defined ast.
  typedef void* AstType;

  //! @brief Compile a syntax tree ouf of the text.
  //! Also takes the source file name for metadata.

  virtual bool Compile(std::string text, std::string file) = 0;

  //! @brief What language are we dealing with?
  virtual const char* Language() { return kInvalidFrontend; }

  virtual bool IsValid() { return strcmp(this->Language(), kInvalidFrontend) > 0; }
};

struct SyntaxLeafList;
struct SyntaxLeafList;
struct CompilerKeyword;

/// we want to do that because to separate keywords.
enum KeywordKind {
  kKeywordKindNamespace,
  kKeywordKindFunctionStart,
  kKeywordKindFunctionEnd,
  kKeywordKindVariable,
  kKeywordKindVariablePtr,
  kKeywordKindType,
  kKeywordKindTypePtr,
  kKeywordKindExpressionBegin,
  kKeywordKindExpressionEnd,
  kKeywordKindArgSeparator,
  kKeywordKindBodyStart,
  kKeywordKindBodyEnd,
  kKeywordKindClass,
  kKeywordKindPtrAccess,
  kKeywordKindAccess,
  kKeywordKindIf,
  kKeywordKindElse,
  kKeywordKindElseIf,
  kKeywordKindVariableAssign,
  kKeywordKindVariableDec,
  kKeywordKindVariableInc,
  kKeywordKindConstant,
  kKeywordKindTypedef,
  kKeywordKindEndInstr,
  kKeywordKindSpecifier,
  kKeywordKindInvalid,
  kKeywordKindReturn,
  kKeywordKindCommentInline,
  kKeywordKindCommentMultiLineStart,
  kKeywordKindCommentMultiLineEnd,
  kKeywordKindEq,
  kKeywordKindNotEq,
  kKeywordKindGreaterEq,
  kKeywordKindLessEq,
  kKeywordKindPtr,
};

/// \brief Compiler keyword information struct.
struct CompilerKeyword {
  std::string keyword_name{""};
  KeywordKind keyword_kind{kKeywordKindInvalid};
};

struct SyntaxLeafList final {
  struct SyntaxLeaf final {
    Int32 fUserType;

#ifdef LC_USE_STRUCTS
    CompilerKeyword fUserData;
#else
    std::string fUserData;
#endif

    std::string        fUserValue{""};
    struct SyntaxLeaf* fNext{nullptr};
  };

  std::vector<SyntaxLeaf> fLeafList;
  SizeType                fNumLeafs;

  size_t                   SizeOf() { return fNumLeafs; }
  std::vector<SyntaxLeaf>& Get() { return fLeafList; }
  SyntaxLeaf&              At(size_t index) { return fLeafList[index]; }
};

/// find the perfect matching word in a haystack.
/// \param haystack base string
/// \param needle the string we search for.
/// \return if we found it or not.
BOOL find_word(std::string haystack, std::string needle) noexcept;

/// find a word within strict conditions and returns a range of it.
/// \param haystack
/// \param needle
/// \return position of needle.
std::size_t find_word_range(std::string haystack, std::string needle) noexcept;
}  // namespace LibCompiler
