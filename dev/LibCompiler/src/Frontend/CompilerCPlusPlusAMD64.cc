/*
 *	========================================================
 *
 *	CompilerCPlusPlusAMD64 CPlusPlus Compiler Driver
 * 	Copyright (C) 2024-2025 Amlal El Mahrouss, all rights reserved.
 *
 * 	========================================================
 */

/// BUGS: 1

#define kPrintF printf

#define kExitOK (EXIT_SUCCESS)
#define kExitNO (EXIT_FAILURE)

#include <LibCompiler/Backend/X64.h>
#include <LibCompiler/Frontend.h>
#include <LibCompiler/UUID.h>
#include <LibCompiler/Util/CompilerUtils.h>

/* NeKernel C++ Compiler Driver */
/* This is part of the LibCompiler. */
/* (c) Amlal El Mahrouss 2024-2025 */

/// @author EL Mahrouss Amlal (amlal@nekernel.org)
/// @file CPlusPlusCompilerAMD64.cxx
/// @brief Optimized C++ Compiler Driver.
/// @todo Throw error for scoped inside scoped variables when they get referenced outside.
/// @todo Add class/struct/enum support.

///////////////////////

// ANSI ESCAPE CODES //

///////////////////////

#define kBlank "\e[0;30m"
#define kRed "\e[0;31m"
#define kWhite "\e[0;97m"

/////////////////////////////////////

// INTERNALS OF THE C++ COMPILER

/////////////////////////////////////

/// @internal
namespace Detail {
std::filesystem::path expand_home(const std::filesystem::path& p) {
  if (!p.empty() && p.string()[0] == '~') {
    const char* home = std::getenv("HOME");  // For Unix-like systems

    if (!home) {
      home = std::getenv("USERPROFILE");  // For Windows
    }

    if (home) {
      return std::filesystem::path(home) / p.relative_path().string().substr(1);
    } else {
      throw std::runtime_error("Home directory not found in environment variables");
    }
  }
  return p;
}

struct CompilerRegisterMap final {
  LibCompiler::STLString fName;
  LibCompiler::STLString fReg;
};

/// \brief Offset based struct/class
struct CompilerStructMap final {
  LibCompiler::STLString                                 fName;
  LibCompiler::STLString                                 fReg;
  std::vector<std::pair<UInt32, LibCompiler::STLString>> fOffsets;
};

/// \brief Compiler state structure.
struct CompilerState final {
  std::vector<CompilerRegisterMap> fStackMapVector;
  std::vector<CompilerStructMap>   fStructMapVector;
  LibCompiler::STLString           fLastFile;
  LibCompiler::STLString           fLastError;
};
}  // namespace Detail

static Detail::CompilerState kState;

static Int32 kOnClassScope = 0;

/////////////////////////////////////////////////////////////////////////////////////////

// Target architecture.
static Int32 kMachine = LibCompiler::AssemblyFactory::kArchAMD64;

/////////////////////////////////////////

// ARGUMENTS REGISTERS (R8, R15)

/////////////////////////////////////////

static std::vector<LibCompiler::CompilerKeyword> kKeywords;

/////////////////////////////////////////

// COMPILER PARSING UTILITIES/STATES.

/////////////////////////////////////////

static LibCompiler::AssemblyFactory kFactory;
static Boolean                      kInStruct    = false;
static Boolean                      kOnWhileLoop = false;
static Boolean                      kOnForLoop   = false;
static Boolean                      kInBraces    = false;
static size_t                       kBracesCount = 0UL;

/* @brief C++ compiler backend for the NeKernel C++ driver */
class CompilerFrontendCPlusPlusAMD64 final LC_COMPILER_FRONTEND {
 public:
  explicit CompilerFrontendCPlusPlusAMD64()  = default;
  ~CompilerFrontendCPlusPlusAMD64() override = default;

  LIBCOMPILER_COPY_DEFAULT(CompilerFrontendCPlusPlusAMD64);

  LibCompiler::SyntaxLeafList::SyntaxLeaf Compile(const LibCompiler::STLString text,
                                                  LibCompiler::STLString       file) override;

  const char* Language() override;
};

/// @internal compiler variables

static CompilerFrontendCPlusPlusAMD64* kCompilerFrontend = nullptr;

static std::vector<LibCompiler::STLString> kRegisterMap;

static std::vector<LibCompiler::STLString> kRegisterList = {
    "rbx", "rsi", "r10", "r11", "r12", "r13", "r14", "r15", "xmm12", "xmm13", "xmm14", "xmm15",
};

/// @brief The PEF calling convention (caller must save rax, rbp)
/// @note callee must return via **rax**.
static std::vector<LibCompiler::STLString> kRegisterConventionCallList = {
    "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
};

static std::size_t kFunctionEmbedLevel = 0UL;

/// detail namespaces

const char* CompilerFrontendCPlusPlusAMD64::Language() {
  return "AMD64 C++";
}

static std::uintptr_t                                                 kOrigin = 0x1000000;
static std::vector<std::pair<LibCompiler::STLString, std::uintptr_t>> kOriginMap;

/////////////////////////////////////////////////////////////////////////////////////////

/// @name Compile
/// @brief Generate assembly from a C++ source.

/////////////////////////////////////////////////////////////////////////////////////////

LibCompiler::SyntaxLeafList::SyntaxLeaf CompilerFrontendCPlusPlusAMD64::Compile(
    LibCompiler::STLString text, LibCompiler::STLString file) {
  LibCompiler::SyntaxLeafList::SyntaxLeaf syntax_tree;

  if (text.length() < 1) return syntax_tree;

  std::size_t                                                       index = 0UL;
  std::vector<std::pair<LibCompiler::CompilerKeyword, std::size_t>> keywords_list;

  for (auto& keyword : kKeywords) {
    if (text.find(keyword.keyword_name) != std::string::npos) {
      switch (keyword.keyword_kind) {
        case LibCompiler::kKeywordKindCommentInline: {
          break;
        }
        default:
          break;
      }

      std::size_t pos = text.find(keyword.keyword_name);
      if (pos == std::string::npos) continue;

      // Safe guard: can't go before start of string
      if (pos > 0 && text[pos - 1] == '+' &&
          keyword.keyword_kind == LibCompiler::kKeywordKindVariableAssign)
        continue;

      if (pos > 0 && text[pos - 1] == '-' &&
          keyword.keyword_kind == LibCompiler::kKeywordKindVariableAssign)
        continue;

      // Safe guard: don't go out of range
      if ((pos + keyword.keyword_name.size()) < text.size() &&
          text[pos + keyword.keyword_name.size()] == '=' &&
          keyword.keyword_kind == LibCompiler::kKeywordKindVariableAssign)
        continue;

      keywords_list.emplace_back(std::make_pair(keyword, index));
      ++index;
    }
  }

  for (auto& keyword : keywords_list) {
    if (text.find(keyword.first.keyword_name) == LibCompiler::STLString::npos) continue;

    switch (keyword.first.keyword_kind) {
      case LibCompiler::KeywordKind::kKeywordKindClass: {
        ++kOnClassScope;
        break;
      }
      case LibCompiler::KeywordKind::kKeywordKindIf: {
        std::size_t keywordPos = text.find(keyword.first.keyword_name);
        std::size_t openParen  = text.find("(", keywordPos);
        std::size_t closeParen = text.find(")", openParen);

        if (keywordPos == LibCompiler::STLString::npos ||
            openParen == LibCompiler::STLString::npos ||
            closeParen == LibCompiler::STLString::npos || closeParen <= openParen) {
          Detail::print_error("Malformed if expression: " + text, file);
          break;
        }

        auto expr = text.substr(openParen + 1, closeParen - openParen - 1);

        if (expr.find(">=") != LibCompiler::STLString::npos) {
          auto left = text.substr(
              text.find(keyword.first.keyword_name) + keyword.first.keyword_name.size() + 2,
              expr.find("<=") + strlen("<="));
          auto right = text.substr(expr.find(">=") + strlen(">="), text.find(")") - 1);

          size_t i = right.size() - 1;

          if (i < 1) break;

          try {
            while (!std::isalnum(right[i])) {
              right.erase(i, 1);
              --i;
            }

            right.erase(0, i);
          } catch (...) {
            right.erase(0, i);
          }

          i = left.size() - 1;
          try {
            while (!std::isalnum(left[i])) {
              left.erase(i, 1);
              --i;
            }

            left.erase(0, i);
          } catch (...) {
            left.erase(0, i);
          }

          if (!isdigit(left[0]) || !isdigit(right[0])) {
            auto indexRight = 0UL;

            auto& valueOfVar = !isdigit(left[0]) ? left : right;

            if (!valueOfVar.empty()) {
              for (auto pairRight : kRegisterMap) {
                ++indexRight;

                if (pairRight != valueOfVar) {
                  auto& valueOfVarOpposite = isdigit(left[0]) ? left : right;

                  syntax_tree.fUserValue +=
                      "mov " + kRegisterList[indexRight + 1] + ", " + valueOfVarOpposite + "\n";
                  syntax_tree.fUserValue += "cmp " + kRegisterList[kRegisterMap.size() - 1] + "," +
                                            kRegisterList[indexRight + 1] + "\n";

                  goto done_iterarting_on_if;
                }

                auto& valueOfVarOpposite = isdigit(left[0]) ? left : right;

                syntax_tree.fUserValue +=
                    "mov " + kRegisterList[indexRight + 1] + ", " + valueOfVarOpposite + "\n";
                syntax_tree.fUserValue += "cmp " + kRegisterList[kRegisterMap.size() - 1] + ", " +
                                          kRegisterList[indexRight + 1] + "\n";

                break;
              }
            }
          }

        done_iterarting_on_if:

          LibCompiler::STLString fnName = text;
          fnName.erase(fnName.find(keyword.first.keyword_name));

          for (auto& ch : fnName) {
            if (ch == ' ') ch = '_';
          }

          syntax_tree.fUserValue +=
              "jge __OFFSET_ON_TRUE_LC\nsegment .code64 __OFFSET_ON_TRUE_LC:\n";
        }

        break;
      }
      case LibCompiler::KeywordKind::kKeywordKindFunctionStart: {
        for (auto& ch : text) {
          if (isdigit(ch)) {
            goto dont_accept;
          }
        }

        goto accept;

      dont_accept:
        break;

      accept:
        LibCompiler::STLString fnName      = text;
        size_t                 indexFnName = 0;

        // this one is for the type.
        for (auto& ch : text) {
          ++indexFnName;

          if (ch == '\t') break;
          if (ch == ' ') break;
        }

        fnName = text.substr(indexFnName);

        if (text.find("return ") != LibCompiler::STLString::npos) {
          text.erase(0, text.find("return "));
          break;
        }

        if (text.ends_with(";") && text.find("return") == LibCompiler::STLString::npos)
          goto lc_write_assembly;
        else if (text.size() <= indexFnName)
          Detail::print_error("Invalid function name: " + fnName, file);

        indexFnName = 0;

        for (auto& ch : fnName) {
          if (ch == ' ' || ch == '\t') {
            if (fnName[indexFnName - 1] != ')')
              Detail::print_error("Invalid function name: " + fnName, file);
          }

          ++indexFnName;
        }

        if (fnName.find("(") != LibCompiler::STLString::npos) {
          fnName.erase(fnName.find("("));
        }

        syntax_tree.fUserValue = "public_segment .code64 __LIBCOMPILER_" + fnName + "\n";
        ++kFunctionEmbedLevel;

        kOriginMap.push_back({"__LIBCOMPILER_" + fnName, kOrigin});

        break;

      lc_write_assembly:
        auto it =
            std::find_if(kOriginMap.begin(), kOriginMap.end(),
                         [&fnName](std::pair<LibCompiler::STLString, std::uintptr_t> pair) -> bool {
                           return fnName == pair.first;
                         });

        if (it != kOriginMap.end()) {
          std::stringstream ss;
          ss << std::hex << it->second;

          syntax_tree.fUserValue = "jmp " + ss.str() + "\n";
          kOrigin += 1UL;
        }
      }
      case LibCompiler::KeywordKind::kKeywordKindFunctionEnd: {
        if (kOnClassScope) --kOnClassScope;

        if (text.ends_with(";")) break;

        --kFunctionEmbedLevel;

        if (kRegisterMap.size() > kRegisterList.size()) {
          --kFunctionEmbedLevel;
        }

        if (kFunctionEmbedLevel < 1) kRegisterMap.clear();

        break;
      }
      case LibCompiler::KeywordKind::kKeywordKindEndInstr:
      case LibCompiler::KeywordKind::kKeywordKindVariableInc:
      case LibCompiler::KeywordKind::kKeywordKindVariableDec:
      case LibCompiler::KeywordKind::kKeywordKindVariableAssign: {
        LibCompiler::STLString valueOfVar = "";

        if (keyword.first.keyword_kind == LibCompiler::KeywordKind::kKeywordKindVariableInc) {
          valueOfVar = text.substr(text.find("+=") + 2);
        } else if (keyword.first.keyword_kind ==
                   LibCompiler::KeywordKind::kKeywordKindVariableDec) {
          valueOfVar = text.substr(text.find("-=") + 2);
        } else if (keyword.first.keyword_kind ==
                   LibCompiler::KeywordKind::kKeywordKindVariableAssign) {
          valueOfVar = text.substr(text.find("=") + 1);
        } else if (keyword.first.keyword_kind == LibCompiler::KeywordKind::kKeywordKindEndInstr) {
          break;
        }

        while (valueOfVar.find(";") != LibCompiler::STLString::npos &&
               keyword.first.keyword_kind != LibCompiler::KeywordKind::kKeywordKindEndInstr) {
          valueOfVar.erase(valueOfVar.find(";"));
        }

        LibCompiler::STLString varName = text;

        if (keyword.first.keyword_kind == LibCompiler::KeywordKind::kKeywordKindVariableInc) {
          varName.erase(varName.find("+="));
        } else if (keyword.first.keyword_kind ==
                   LibCompiler::KeywordKind::kKeywordKindVariableDec) {
          varName.erase(varName.find("-="));
        } else if (keyword.first.keyword_kind ==
                   LibCompiler::KeywordKind::kKeywordKindVariableAssign) {
          varName.erase(varName.find("="));
        } else if (keyword.first.keyword_kind == LibCompiler::KeywordKind::kKeywordKindEndInstr) {
          varName.erase(varName.find(";"));
        }

        static Boolean typeFound = false;

        for (auto& keyword : kKeywords) {
          if (keyword.keyword_kind == LibCompiler::kKeywordKindType) {
            if (text.find(keyword.keyword_name) != LibCompiler::STLString::npos) {
              if (text[text.find(keyword.keyword_name)] == ' ') {
                typeFound = false;
                continue;
              }

              typeFound = true;
            }
          }
        }

        LibCompiler::STLString instr = "mov ";

        std::vector<LibCompiler::STLString> newVars;

        if (typeFound &&
            keyword.first.keyword_kind != LibCompiler::KeywordKind::kKeywordKindVariableInc &&
            keyword.first.keyword_kind != LibCompiler::KeywordKind::kKeywordKindVariableDec) {
          if (kRegisterMap.size() > kRegisterList.size()) {
            ++kFunctionEmbedLevel;
          }

          while (varName.find(" ") != LibCompiler::STLString::npos) {
            varName.erase(varName.find(" "), 1);
          }

          while (varName.find("\t") != LibCompiler::STLString::npos) {
            varName.erase(varName.find("\t"), 1);
          }

          for (size_t i = 0; !isalnum(valueOfVar[i]); i++) {
            if (i > valueOfVar.size()) break;

            valueOfVar.erase(i, 1);
          }

          constexpr auto kTrueVal  = "true";
          constexpr auto kFalseVal = "false";

          if (valueOfVar == kTrueVal) {
            valueOfVar = "1";
          } else if (valueOfVar == kFalseVal) {
            valueOfVar = "0";
          }

          std::size_t indexRight = 0UL;

          for (auto pairRight : kRegisterMap) {
            ++indexRight;

            if (pairRight != valueOfVar) {
              if (valueOfVar[0] == '\"') {
                syntax_tree.fUserValue = "segment .data64 __LIBCOMPILER_LOCAL_VAR_" + varName +
                                         ": db " + valueOfVar + ", 0\n\n";
                syntax_tree.fUserValue += instr + kRegisterList[kRegisterMap.size() - 1] + ", " +
                                          "__LIBCOMPILER_LOCAL_VAR_" + varName + "\n";
                kOrigin += 1UL;
              } else {
                syntax_tree.fUserValue =
                    instr + kRegisterList[kRegisterMap.size() - 1] + ", " + valueOfVar + "\n";
                kOrigin += 1UL;
              }

              goto done;
            }
          }

          if (((int) indexRight - 1) < 0) {
            if (valueOfVar[0] == '\"') {
              syntax_tree.fUserValue = "segment .data64 __LIBCOMPILER_LOCAL_VAR_" + varName +
                                       ": db " + valueOfVar + ", 0\n";
              syntax_tree.fUserValue += instr + kRegisterList[kRegisterMap.size()] + ", " +
                                        "__LIBCOMPILER_LOCAL_VAR_" + varName + "\n";
              kOrigin += 1UL;
            } else {
              syntax_tree.fUserValue =
                  instr + kRegisterList[kRegisterMap.size()] + ", " + valueOfVar + "\n";
              kOrigin += 1UL;
            }

            goto done;
          }

          if (valueOfVar[0] != '\"' && valueOfVar[0] != '\'' && !isdigit(valueOfVar[0])) {
            for (auto pair : kRegisterMap) {
              if (pair == valueOfVar) goto done;
            }

            Detail::print_error("Variable not declared: " + varName, file);
            break;
          }

        done:
          for (auto& keyword : kKeywords) {
            if (keyword.keyword_kind == LibCompiler::kKeywordKindType &&
                varName.find(keyword.keyword_name) != LibCompiler::STLString::npos) {
              varName.erase(varName.find(keyword.keyword_name), keyword.keyword_name.size());
              break;
            }
          }

          newVars.push_back(varName);

          break;
        }

        kRegisterMap.insert(kRegisterMap.end(), newVars.begin(), newVars.end());

        if (keyword.second > 0 &&
                kKeywords[keyword.second - 1].keyword_kind == LibCompiler::kKeywordKindType ||
            kKeywords[keyword.second - 1].keyword_kind == LibCompiler::kKeywordKindTypePtr) {
          syntax_tree.fUserValue = "\n";
          continue;
        }

        if (keyword.first.keyword_kind == LibCompiler::KeywordKind::kKeywordKindEndInstr) {
          syntax_tree.fUserValue = "\n";
          continue;
        }

        if (keyword.first.keyword_kind == LibCompiler::KeywordKind::kKeywordKindVariableInc) {
          instr = "add ";
        } else if (keyword.first.keyword_kind ==
                   LibCompiler::KeywordKind::kKeywordKindVariableDec) {
          instr = "sub ";
        }

        LibCompiler::STLString varErrCpy = varName;

        while (varName.find(" ") != LibCompiler::STLString::npos) {
          varName.erase(varName.find(" "), 1);
        }

        while (varName.find("\t") != LibCompiler::STLString::npos) {
          varName.erase(varName.find("\t"), 1);
        }

        std::size_t indxReg = 0UL;

        for (size_t i = 0; !isalnum(valueOfVar[i]); i++) {
          if (i > valueOfVar.size()) break;

          valueOfVar.erase(i, 1);
        }

        while (valueOfVar.find(" ") != LibCompiler::STLString::npos) {
          valueOfVar.erase(valueOfVar.find(" "), 1);
        }

        while (valueOfVar.find("\t") != LibCompiler::STLString::npos) {
          valueOfVar.erase(valueOfVar.find("\t"), 1);
        }

        constexpr auto kTrueVal  = "true";
        constexpr auto kFalseVal = "false";

        /// interpet boolean values, since we're on C++

        if (valueOfVar == kTrueVal) {
          valueOfVar = "1";
        } else if (valueOfVar == kFalseVal) {
          valueOfVar = "0";
        }

        for (auto pair : kRegisterMap) {
          ++indxReg;

          if (pair != varName) continue;

          std::size_t indexRight = 0ul;

          for (auto pairRight : kRegisterMap) {
            ++indexRight;

            if (pairRight != varName) {
              syntax_tree.fUserValue =
                  instr + kRegisterList[kRegisterMap.size()] + ", " + valueOfVar + "\n";
              kOrigin += 1UL;
              continue;
            }

            syntax_tree.fUserValue =
                instr + kRegisterList[indexRight - 1] + ", " + valueOfVar + "\n";
            kOrigin += 1UL;
            break;
          }

          newVars.push_back(varName);
          break;
        }

        if (syntax_tree.fUserValue.empty()) {
          Detail::print_error("Variable not declared: " + varName, file);
        }

        kRegisterMap.insert(kRegisterMap.end(), newVars.begin(), newVars.end());

        break;
      }
      case LibCompiler::KeywordKind::kKeywordKindReturn: {
        try {
          auto                   pos     = text.find("return") + strlen("return") + 1;
          LibCompiler::STLString subText = text.substr(pos);
          subText                        = subText.erase(subText.find(";"));
          size_t indxReg                 = 0UL;

          if (subText[0] != '\"' && subText[0] != '\'') {
            if (!isdigit(subText[0])) {
              for (auto pair : kRegisterMap) {
                ++indxReg;

                if (pair != subText) continue;

                syntax_tree.fUserValue = "mov rax, " + kRegisterList[indxReg - 1] + "\nret\n";
                kOrigin += 1UL;

                break;
              }
            } else {
              syntax_tree.fUserValue = "mov rax, " + subText + "\nret\n";
              kOrigin += 1UL;

              break;
            }
          } else {
            syntax_tree.fUserValue = "__LIBCOMPILER_LOCAL_RETURN_STRING: db " + subText +
                                     ", 0\nmov rcx, __LIBCOMPILER_LOCAL_RETURN_STRING\n";
            syntax_tree.fUserValue += "mov rax, rcx\nret\n";
            kOrigin += 1UL;

            break;
          }

          if (syntax_tree.fUserValue.empty()) {
            if (subText.find("(") != LibCompiler::STLString::npos) {
              subText.erase(subText.find("("));

              auto it = std::find_if(
                  kOriginMap.begin(), kOriginMap.end(),
                  [&subText](std::pair<LibCompiler::STLString, std::uintptr_t> pair) -> bool {
                    return pair.first.find(subText) != LibCompiler::STLString::npos;
                  });

              if (it == kOriginMap.end())
                Detail::print_error("Invalid return value: " + subText, file);

              std::stringstream ss;
              ss << it->second;

              syntax_tree.fUserValue = "jmp " + ss.str() + "\nret\n";
              kOrigin += 1UL;
              break;
            }
          }

          break;
        } catch (...) {
          syntax_tree.fUserValue = "ret\n";
          kOrigin += 1UL;
        }
      }
      default: {
        continue;
      }
    }
  }

  return syntax_tree;
}

/////////////////////////////////////////////////////////////////////////////////////////

/**
 * @brief C++ assembler class.
 */

/////////////////////////////////////////////////////////////////////////////////////////

class AssemblyCPlusPlusInterfaceAMD64 final LC_ASSEMBLY_INTERFACE {
 public:
  explicit AssemblyCPlusPlusInterfaceAMD64()  = default;
  ~AssemblyCPlusPlusInterfaceAMD64() override = default;

  LIBCOMPILER_COPY_DEFAULT(AssemblyCPlusPlusInterfaceAMD64);

  UInt32 Arch() noexcept override { return LibCompiler::AssemblyFactory::kArchAMD64; }

  Int32 CompileToFormat(LibCompiler::STLString src, Int32 arch) override {
    if (kCompilerFrontend == nullptr) return kExitNO;

    LibCompiler::STLString dest = src;
    dest += ".pp.masm";

    std::ofstream out_fp(dest);
    std::ifstream src_fp = std::ifstream(src + ".pp");

    LibCompiler::STLString line_source;

    out_fp << "#bits 64\n";
    out_fp << "#org " << kOrigin << "\n\n";

    while (std::getline(src_fp, line_source)) {
      out_fp << kCompilerFrontend->Compile(line_source, src).fUserValue;
    }

    return kExitOK;
  }
};

/////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////

#define kExtListCxx \
  { ".cpp", ".cxx", ".cc", ".c++", ".cp" }

LIBCOMPILER_MODULE(CompilerCPlusPlusAMD64) {
  Boolean skip = false;

  kKeywords.emplace_back("if", LibCompiler::kKeywordKindIf);
  kKeywords.emplace_back("else", LibCompiler::kKeywordKindElse);
  kKeywords.emplace_back("else if", LibCompiler::kKeywordKindElseIf);

  kKeywords.emplace_back("class", LibCompiler::kKeywordKindClass);
  kKeywords.emplace_back("struct", LibCompiler::kKeywordKindClass);
  kKeywords.emplace_back("namespace", LibCompiler::kKeywordKindNamespace);
  kKeywords.emplace_back("typedef", LibCompiler::kKeywordKindTypedef);
  kKeywords.emplace_back("using", LibCompiler::kKeywordKindTypedef);
  kKeywords.emplace_back("{", LibCompiler::kKeywordKindBodyStart);
  kKeywords.emplace_back("}", LibCompiler::kKeywordKindBodyEnd);
  kKeywords.emplace_back("auto", LibCompiler::kKeywordKindVariable);
  kKeywords.emplace_back("int", LibCompiler::kKeywordKindType);
  kKeywords.emplace_back("bool", LibCompiler::kKeywordKindType);
  kKeywords.emplace_back("unsigned", LibCompiler::kKeywordKindType);
  kKeywords.emplace_back("short", LibCompiler::kKeywordKindType);
  kKeywords.emplace_back("char", LibCompiler::kKeywordKindType);
  kKeywords.emplace_back("long", LibCompiler::kKeywordKindType);
  kKeywords.emplace_back("float", LibCompiler::kKeywordKindType);
  kKeywords.emplace_back("double", LibCompiler::kKeywordKindType);
  kKeywords.emplace_back("void", LibCompiler::kKeywordKindType);

  kKeywords.emplace_back("auto*", LibCompiler::kKeywordKindVariablePtr);
  kKeywords.emplace_back("int*", LibCompiler::kKeywordKindTypePtr);
  kKeywords.emplace_back("bool*", LibCompiler::kKeywordKindTypePtr);
  kKeywords.emplace_back("unsigned*", LibCompiler::kKeywordKindTypePtr);
  kKeywords.emplace_back("short*", LibCompiler::kKeywordKindTypePtr);
  kKeywords.emplace_back("char*", LibCompiler::kKeywordKindTypePtr);
  kKeywords.emplace_back("long*", LibCompiler::kKeywordKindTypePtr);
  kKeywords.emplace_back("float*", LibCompiler::kKeywordKindTypePtr);
  kKeywords.emplace_back("double*", LibCompiler::kKeywordKindTypePtr);
  kKeywords.emplace_back("void*", LibCompiler::kKeywordKindTypePtr);

  kKeywords.emplace_back("(", LibCompiler::kKeywordKindFunctionStart);
  kKeywords.emplace_back(")", LibCompiler::kKeywordKindFunctionEnd);
  kKeywords.emplace_back("=", LibCompiler::kKeywordKindVariableAssign);
  kKeywords.emplace_back("+=", LibCompiler::kKeywordKindVariableInc);
  kKeywords.emplace_back("-=", LibCompiler::kKeywordKindVariableDec);
  kKeywords.emplace_back("const", LibCompiler::kKeywordKindConstant);
  kKeywords.emplace_back("*", LibCompiler::kKeywordKindPtr);
  kKeywords.emplace_back("->", LibCompiler::kKeywordKindPtrAccess);
  kKeywords.emplace_back(".", LibCompiler::kKeywordKindAccess);
  kKeywords.emplace_back(",", LibCompiler::kKeywordKindArgSeparator);
  kKeywords.emplace_back(";", LibCompiler::kKeywordKindEndInstr);
  kKeywords.emplace_back(":", LibCompiler::kKeywordKindSpecifier);
  kKeywords.emplace_back("public:", LibCompiler::kKeywordKindSpecifier);
  kKeywords.emplace_back("private:", LibCompiler::kKeywordKindSpecifier);
  kKeywords.emplace_back("protected:", LibCompiler::kKeywordKindSpecifier);
  kKeywords.emplace_back("final", LibCompiler::kKeywordKindSpecifier);
  kKeywords.emplace_back("return", LibCompiler::kKeywordKindReturn);
  kKeywords.emplace_back("/*", LibCompiler::kKeywordKindCommentMultiLineStart);
  kKeywords.emplace_back("*/", LibCompiler::kKeywordKindCommentMultiLineEnd);
  kKeywords.emplace_back("//", LibCompiler::kKeywordKindCommentInline);
  kKeywords.emplace_back("==", LibCompiler::kKeywordKindEq);
  kKeywords.emplace_back("!=", LibCompiler::kKeywordKindNotEq);
  kKeywords.emplace_back(">=", LibCompiler::kKeywordKindGreaterEq);
  kKeywords.emplace_back("<=", LibCompiler::kKeywordKindLessEq);

  kErrorLimit = 0;

  kCompilerFrontend = new CompilerFrontendCPlusPlusAMD64();
  kFactory.Mount(new AssemblyCPlusPlusInterfaceAMD64());

  LibCompiler::install_signal(SIGSEGV, Detail::drvi_crash_handler);

  for (auto index = 1UL; index < argc; ++index) {
    if (!argv[index]) break;

    if (argv[index][0] == '-') {
      if (skip) {
        skip = false;
        continue;
      }

      if (strcmp(argv[index], "-cxx-verbose") == 0) {
        kVerbose = true;

        continue;
      }

      if (strcmp(argv[index], "-cxx-dialect") == 0) {
        if (kCompilerFrontend) std::cout << kCompilerFrontend->Language() << "\n";

        return LIBCOMPILER_SUCCESS;
      }

      if (strcmp(argv[index], "-cxx-max-err") == 0) {
        try {
          kErrorLimit = std::strtol(argv[index + 1], nullptr, 10);
        }
        // catch anything here
        catch (...) {
          kErrorLimit = 0;
        }

        skip = true;

        continue;
      }

      LibCompiler::STLString err = "Unknown option: ";
      err += argv[index];

      Detail::print_error(err, "cxxdrv");

      continue;
    }

    LibCompiler::STLString argv_i = argv[index];

    std::vector<LibCompiler::STLString> exts = kExtListCxx;

    for (LibCompiler::STLString ext : exts) {
      if (argv_i.ends_with(ext)) {
        if (kFactory.Compile(argv_i, kMachine) != kExitOK) {
          return LIBCOMPILER_INVALID_DATA;
        }

        break;
      }
    }
  }

  kFactory.Unmount();

  return LIBCOMPILER_SUCCESS;
}

//
// Last rev 23-5-25
//
