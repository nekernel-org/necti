/* -------------------------------------------

    Copyright Mahrouss Logic

    File: cl-parser.hxx
    Purpose: C++ Compiler parser

    Revision History:

    31/01/24: Added file (amlel)
    01/02/24: Add namespace and imports. (amlel)

------------------------------------------- */

#pragma once

#include <CompilerKit/AsmKit/AsmKit.hpp>
#include <CompilerKit/CompilerKit.hpp>
#include <CompilerKit/ParserKit.hpp>

namespace CompilerKit {
class CLParserExpression {
 private:
  StringView mExprView;

 public:
  CLParserExpression(const char* expr)
      : mExprView(StringBuilder::Construct(expr)) {}
  virtual ~CLParserExpression() = default;

  MPCC_COPY_DEFAULT(CLParserExpression);
};
}  // namespace CompilerKit
