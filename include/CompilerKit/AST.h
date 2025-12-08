/* ========================================

  Copyright (C) 2024-2025 Amlal El Mahrouss, Licensed under the Apache 2.0 license

======================================== */

#pragma once

#include <CompilerKit/CodeGenerator.h>
#include <vector>
#include "CompilerKit/Detail/Config.h"

#define CK_COMPILER_FRONTEND : public ::CompilerKit::ICompilerFrontend

namespace CompilerKit {
inline static auto kInvalidFrontend = "(null)";

struct SyntaxLeafList;
struct SyntaxLeafList;
struct SyntaxKeyword;

/// =========================================================== ///
/// @note we want to do that to separate keywords.
/// =========================================================== ///

enum KeywordKind {
  kKeywordKindReserved  = 0,
  kKeywordKindNamespace = 100,
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
  kKeywordKindClassStart,
  kKeywordKindClassEnd,
  kKeywordKindMethodStart,
  kKeywordKindMethodEnd,
  kKeywordKindCount = kKeywordKindPtr - kKeywordKindNamespace + 1,
};

/// =========================================================== ///
/// \brief Compiler keyword information struct.
/// =========================================================== ///
struct SyntaxKeyword {
  SyntaxKeyword(STLString name, KeywordKind kind) : fKeywordName(name), fKeywordKind(kind) {}

  STLString   fKeywordName{""};
  KeywordKind fKeywordKind{kKeywordKindInvalid};
};

struct SyntaxLeafList final {
  struct SyntaxLeaf final {
    Int32         fUserType{0U};
    SyntaxKeyword fUserData{"", kKeywordKindInvalid};

    STLString          fUserValue{""};
    struct SyntaxLeaf* fNext{nullptr};
  };

  using ArrayType = std::vector<SyntaxLeaf>;

  ArrayType fLeafList;
  SizeType  fNumLeafs{0};

  SizeType    NumLeafs() { return fNumLeafs; }
  ArrayType&  Get() { return fLeafList; }
  SyntaxLeaf& At(const SizeType& index) { return fLeafList[index]; }
};

/// =========================================================== ///
/// find the perfect matching word in a haystack.
/// \param haystack base string
/// \param needle the string we search for.
/// \return if we found it or not.
/// =========================================================== ///
bool ast_find_needle(STLString haystack, STLString needle) noexcept;

/// =========================================================== ///
/// find a word within strict conditions and returns a range of it.
/// \param haystack
/// \param needle
/// \return position of needle.
/// =========================================================== ///
SizeType ast_find_needle_range(STLString haystack, STLString needle) noexcept;

/// =========================================================== ///
/// @brief Compiler backend, implements a frontend, such as C, C++...
/// See Toolchain, for some examples.
/// =========================================================== ///
class ICompilerFrontend {
 public:
  explicit ICompilerFrontend() = default;
  virtual ~ICompilerFrontend() = default;

  NECTI_COPY_DEFAULT(ICompilerFrontend)

  /// =========================================================== ///
  /// NOTE: cast this to your user defined ast.
  /// =========================================================== ///
  using AstType = VoidPtr;

  /// =========================================================== ///
  //! @brief Compile a syntax tree ouf of the text.
  //! Also takes the source file name for metadata.
  /// =========================================================== ///
  virtual SyntaxLeafList::SyntaxLeaf Compile(STLString text, STLString file) = 0;

  /// =========================================================== ///
  //! @brief What language are we dealing with?
  /// =========================================================== ///
  virtual const char* Language();

  /// =========================================================== ///
  /// @brief Checks if language is a valid frontend.
  /// =========================================================== ///
  virtual bool IsValid();
};
}  // namespace CompilerKit

#include <CompilerKit/AST.inl>