// Copyright 2024-2025, Amlal El Mahrouss (amlal@nekernel.org)
// Licensed under the Apache License, Version 2.0 (See accompanying
// file LICENSE or copy at http://www.apache.org/licenses/LICENSE-2.0)
// Official repository: https://github.com/nekernel-org/nectar

#ifndef NECTAR_COMPILERKIT_AST_H
#define NECTAR_COMPILERKIT_AST_H

#include <CompilerKit/CodeGenerator.h>
#include <vector>

#define CK_COMPILER_FRONTEND : public ::CompilerKit::ICompilerFrontend

namespace CompilerKit {
inline static constexpr auto kInvalidFrontend = "(null)";

struct SyntaxLeafList;
struct SyntaxLeafList;
struct SyntaxKeyword;

/// =========================================================== ///
/// @note we want to do that to separate keywords.
/// =========================================================== ///

/// \brief The type of keyword we are dealing with.
enum struct KeywordKind {
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
  SyntaxKeyword(const STLString& name, KeywordKind kind) : fKeywordName(name), fKeywordKind(kind) {}
  ~SyntaxKeyword() = default;
  SyntaxKeyword()  = delete;

  NECTAR_COPY_DEFAULT(SyntaxKeyword);

  STLString   fKeywordName{};
  KeywordKind fKeywordKind{KeywordKind::kKeywordKindInvalid};
};

struct SyntaxLeafList final {
  struct SyntaxLeaf;

  struct SyntaxLeaf {
    using Ptr            = SyntaxLeaf*;
    using Reference      = SyntaxLeaf&;
    using ConstReference = const SyntaxLeaf&;

    /// \brief User data type.
    Int32 fUserType{};

    /// \brief User data buffer.
    SyntaxKeyword fUserData{{}, KeywordKind::kKeywordKindInvalid};

    /// \brief User data value
    STLString fUserValue{};

    /// \brief Next user data on list.
    Ptr fNext{nullptr};
  };

  using ArrayType = std::vector<SyntaxLeaf>;

  ArrayType fLeafList;
  SizeType  fNumLeafs{0};

 public:
  const SizeType&  NumLeafs() { return fNumLeafs; }
  const SizeType&  NumLeafs() const { return fNumLeafs; }
  const ArrayType& Get() const { return fLeafList; }
  ArrayType&       Get() { return fLeafList; }
  // \brief You can't get a reference of a leaf in a const context.
  const SyntaxLeaf& At(const SizeType& index) const = delete;
  // \breif Grab leaf from index.
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

  NECTAR_COPY_DEFAULT(ICompilerFrontend)

  /// =========================================================== ///
  /// NOTE: cast this to your user defined ast.
  /// =========================================================== ///
  using AstType = VoidPtr;

  /// =========================================================== ///
  //! @brief Compile a syntax tree ouf of the text.
  //! Also takes the source file name for metadata.
  /// =========================================================== ///
  virtual SyntaxLeafList::SyntaxLeaf Compile(STLString& text, const STLString& file) { return {}; }

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

#endif  // NECTAR_COMPILERKIT_AST_H
