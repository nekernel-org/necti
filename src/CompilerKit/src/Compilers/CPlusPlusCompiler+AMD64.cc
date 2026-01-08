// Copyright 2024-2025, Amlal El Mahrouss (amlal@nekernel.org)
// Licensed under the Apache License, Version 2.0 (See accompanying
// file LICENSE or copy at http://www.apache.org/licenses/LICENSE-2.0)
// Official repository: https://github.com/nekernel-org/nectar

/// BUGS: 0

///////////////////////

// ANSI ESCAPE CODES //

///////////////////////

///////////////////////

// MACROS //

///////////////////////

#include <CompilerKit/AST.h>
#include <CompilerKit/Detail/AMD64.h>
#include <CompilerKit/PEF.h>
#include <CompilerKit/UUID.h>
#include <CompilerKit/Utilities/Compiler.h>
#include <csignal>
#include <cstdlib>
#include <filesystem>

/* NeKernel C++ Compiler Driver. */
/* This is part of the CompilerKit. */
/* (c) Amlal El Mahrouss 2024-2025 */

/// @author Amlal El Mahrouss (amlal@nekernel.org)
/// @file CPlusPlusCompilerAMD64.cc
/// @brief C++ Compiler Driver.

/////////////////////////////////////

// INTERNALS OF THE C++ COMPILER

/////////////////////////////////////

/// @internal
/// @brief Avoids relative_path which could discard parts of the original.
std::filesystem::path nectar_expand_home(const std::filesystem::path& input) {
  const std::string& raw = input.string();

  if (!raw.empty() && raw[0] == '~') {
    const char* home = std::getenv("HOME");
    if (!home) home = std::getenv("USERPROFILE");

    if (!home) throw std::runtime_error("Home directory not found in environment variables");

    return std::filesystem::path(home) / raw.substr(1);
  }

  return input;
}

/// \brief Register map, i.e ({foobar, rbp+48}, etc...)
struct CompilerRegisterMap final {
  CompilerKit::STLString fName{};
  CompilerKit::STLString fReg{};
};

/// \brief Offsets of struct and classes.
struct CompilerStructMap final {
  CompilerKit::STLString                                 fName{};
  CompilerKit::STLString                                 fReg{};
  std::vector<std::pair<UInt32, CompilerKit::STLString>> fOffsets;
};

/// \brief State machine of the compiler.
struct CompilerState final {
  std::vector<CompilerRegisterMap> fStackMapVector;
  std::vector<CompilerStructMap>   fStructMapVector;
  CompilerKit::STLString           fLastFile{};
  CompilerKit::STLString           fLastError{};
};

static CompilerState kState;

/// \brief Embed Scope of a class.
static Int32 kOnClassScope = 0;

/////////////////////////////////////////////////////////////////////////////////////////

// NEW DATA STRUCTURES FOR C++ SUPPORT

/////////////////////////////////////////////////////////////////////////////////////////

/// \brief Scope kind enumeration
enum class ScopeKind {
  kScopeGlobal,
  kScopeNamespace,
  kScopeClass,
  kScopeFunction,
};

/// \brief Compiler scope information
struct CompilerScope {
  ScopeKind              fKind{ScopeKind::kScopeGlobal};
  CompilerKit::STLString fName{};
  CompilerKit::STLString fMangledPrefix{};
};

/// \brief Variable location enumeration
enum class VarLocation {
  kRegister,
  kStack,
  kStackSpill,
};

/// \brief Extended variable information
struct VariableInfo {
  CompilerKit::STLString fName{};
  VarLocation            fLocation{VarLocation::kRegister};
  Int32                  fStackOffset{0};
  CompilerKit::STLString fRegister{};
  Int32                  fSize{8};
  bool                   fIsParameter{false};
  CompilerKit::STLString fTypeName{};
  UInt32                 fLastUsed{0};
};

/// \brief Complete compiler context
struct CompilerContext {
  std::vector<CompilerScope>     fScopeStack;
  std::vector<VariableInfo>      fVariables;
  std::vector<CompilerStructMap> fStructMapVector;
  CompilerKit::STLString         fLastFile{};
  CompilerKit::STLString         fLastError{};
  Int32                          fStackOffset{0};
  Int32                          fMaxStackUsed{0};
  UInt32                         fInstructionCounter{0};
};

/// \brief Global compiler context (replaces kState)
static CompilerContext kContext;

/////////////////////////////////////////////////////////////////////////////////////////

/// \brief Target architecture.
/// \note This shall never change.
static Int32 kMachine = CompilerKit::AssemblyFactory::kArchAMD64;

/////////////////////////////////////////

// ARGUMENT REGISTERS (R8, R15)

/////////////////////////////////////////

static std::vector<CompilerKit::SyntaxKeyword> kKeywords;

/////////////////////////////////////////

// COMPILER PARSING UTILITIES/STATES.

/////////////////////////////////////////

static CompilerKit::AssemblyFactory kAssembler;
static bool                         kInStruct    = false;
static bool                         kOnWhileLoop = false;
static bool                         kOnForLoop   = false;
static bool                         kInBraces    = false;
static size_t                       kBracesCount = 0UL;

/////////////////////////////////////////////////////////////////////////////////////////

// HELPER FUNCTION DECLARATIONS

/////////////////////////////////////////////////////////////////////////////////////////

// Scope management
static void nectar_push_scope(ScopeKind kind, const CompilerKit::STLString& name);
static void nectar_pop_scope();

// Name mangling
static std::vector<CompilerKit::STLString> nectar_extract_function_args(
    const CompilerKit::STLString& text);
static CompilerKit::STLString nectar_mangle_name(
    const CompilerKit::STLString& identifier, const std::vector<CompilerKit::STLString>& args = {});

// Stack frame management
static CompilerKit::STLString nectar_generate_prologue();
static CompilerKit::STLString nectar_generate_epilogue();
static Int32 nectar_allocate_stack_variable(const CompilerKit::STLString& var_name, Int32 size = 8);

// Register allocation
static CompilerKit::STLString nectar_allocate_register(const CompilerKit::STLString& var_name);
static CompilerKit::STLString nectar_spill_lru_variable();
static VariableInfo*          nectar_find_variable(const CompilerKit::STLString& var_name);
static CompilerKit::STLString nectar_get_variable_ref(const CompilerKit::STLString& var_name);

// Class/object management
static void                   nectar_add_class_member(const CompilerKit::STLString& class_name,
                                                     const CompilerKit::STLString& member_name, Int32 size);
static Int32                  nectar_get_class_size(const CompilerKit::STLString& class_name);
static CompilerKit::STLString nectar_generate_constructor_call(
    const CompilerKit::STLString& class_name, const CompilerKit::STLString& obj_name);
static CompilerKit::STLString nectar_generate_destructor_call(
    const CompilerKit::STLString& class_name, const CompilerKit::STLString& obj_name);

// PEF calling convention
static void nectar_process_function_parameters(const std::vector<CompilerKit::STLString>& args);

/////////////////////////////////////////////////////////////////////////////////////////

/* \brief C++ compiler backend for the NeKernel C++ driver */
class CompilerFrontendCPlusPlusAMD64 final CK_COMPILER_FRONTEND {
 public:
  explicit CompilerFrontendCPlusPlusAMD64()  = default;
  ~CompilerFrontendCPlusPlusAMD64() override = default;

  NECTAR_COPY_DEFAULT(CompilerFrontendCPlusPlusAMD64);

  /// \brief Parse C symbols and syntax.
  CompilerKit::SyntaxLeafList::SyntaxLeaf Compile(CompilerKit::STLString&       text,
                                                  const CompilerKit::STLString& file) override;

  /// \brief Contract language
  const char* Language() override;

 public:
  /// \brief Parse C++ namespaces and objects.
  /// \param CompilerKit::SyntaxLeafList::SyntaxLeaf the leaf to build upon.
  CompilerKit::SyntaxLeafList::SyntaxLeaf CompilePass2(CompilerKit::STLString&       text,
                                                       const CompilerKit::STLString& file,
                                                       CompilerKit::SyntaxLeafList::SyntaxLeaf&);
};

/// @internal compiler variables

static CompilerFrontendCPlusPlusAMD64* kFrontend = nullptr;

static std::vector<CompilerKit::STLString> kRegisterMap;

static std::vector<CompilerKit::STLString> kRegisterList = {
    "rbx", "rsi", "r10", "r11", "r12", "r13", "r14", "r15", "xmm12", "xmm13", "xmm14", "xmm15",
};

/// @brief The PEF calling convention (caller must save rax, rbp)
/// @note callee must return via **rax**.
static std::vector<CompilerKit::STLString> kRegisterConventionCallList = {
    "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
};

static std::size_t kFunctionEmbedLevel{};
static std::size_t kNamespaceEmbedLevel{};

/// detail namespaces

const char* CompilerFrontendCPlusPlusAMD64::Language() {
  return "Nectar C++";
}

static std::uintptr_t                                                 kOrigin = kPefBaseOrigin;
static std::vector<std::pair<CompilerKit::STLString, std::uintptr_t>> kOriginMap;

/////////////////////////////////////////////////////////////////////////////////////////

/// @name Compile
/// @brief Generate assembly from a C++ source.

/////////////////////////////////////////////////////////////////////////////////////////

CompilerKit::SyntaxLeafList::SyntaxLeaf CompilerFrontendCPlusPlusAMD64::Compile(
    CompilerKit::STLString& text, const CompilerKit::STLString& file) {
  CompilerKit::SyntaxLeafList::SyntaxLeaf syntax_tree;

  if (text.empty()) return syntax_tree;

  std::size_t                                                     index{};
  std::vector<std::pair<CompilerKit::SyntaxKeyword, std::size_t>> keywords_list;

  for (auto& keyword : kKeywords) {
    if (text.find(keyword.fKeywordName) != std::string::npos) {
      switch (keyword.fKeywordKind) {
        case CompilerKit::KeywordKind::kKeywordKindCommentInline: {
          break;
        }
        default:
          break;
      }

      std::size_t pos = text.find(keyword.fKeywordName);
      if (pos == std::string::npos) continue;

      // can't go before start of string
      if (pos > 0 && text[pos - 1] == '+' &&
          keyword.fKeywordKind == CompilerKit::KeywordKind::kKeywordKindVariableAssign)
        continue;

      if (pos > 0 && text[pos - 1] == '-' &&
          keyword.fKeywordKind == CompilerKit::KeywordKind::kKeywordKindVariableAssign)
        continue;

      // don't go out of range
      if ((pos + keyword.fKeywordName.size()) < text.size() &&
          text[pos + keyword.fKeywordName.size()] == '=' &&
          keyword.fKeywordKind == CompilerKit::KeywordKind::kKeywordKindVariableAssign)
        continue;

      keywords_list.emplace_back(std::make_pair(keyword, index));
      ++index;
    }
  }

  for (auto& keyword : keywords_list) {
    if (text.find(keyword.first.fKeywordName) == CompilerKit::STLString::npos) continue;

    switch (keyword.first.fKeywordKind) {
      case CompilerKit::KeywordKind::kKeywordKindClass: {
        ++kOnClassScope;
        auto symbol_name_cls = text.substr(text.find(keyword.first.fKeywordName));

        for (auto& key : symbol_name_cls) {
          if (key == ' ') key = '_';
          if (key == ':') key = '$';
        }

        syntax_tree.fUserValue += "public_segment .data64 __NECTAR_" + symbol_name_cls + "\n";

        break;
      }
      case CompilerKit::KeywordKind::kKeywordKindIf: {
        std::size_t keywordPos = text.find(keyword.first.fKeywordName);
        std::size_t openParen  = text.find("(", keywordPos);
        std::size_t closeParen = text.find(")", openParen);

        if (keywordPos == CompilerKit::STLString::npos ||
            openParen == CompilerKit::STLString::npos ||
            closeParen == CompilerKit::STLString::npos || closeParen <= openParen) {
          CompilerKit::Detail::print_error("Malformed if expression: " + text, file);
          break;
        }

        auto expr = text.substr(openParen + 1, closeParen - openParen - 1);

        if (expr.find(">=") != CompilerKit::STLString::npos) {
          auto left = text.substr(
              text.find(keyword.first.fKeywordName) + keyword.first.fKeywordName.size() + 2,
              expr.find("<=") + strlen("<="));
          auto right = text.substr(expr.find(">=") + strlen(">="), text.find(")") - 1);

          // Trim whitespace
          while (!right.empty() && (right.back() == ' ' || right.back() == '\t')) {
            right.pop_back();
          }
          while (!right.empty() && (right.front() == ' ' || right.front() == '\t')) {
            right.erase(0, 1);
          }

          while (!left.empty() && (left.back() == ' ' || left.back() == '\t')) {
            left.pop_back();
          }
          while (!left.empty() && (left.front() == ' ' || left.front() == '\t')) {
            left.erase(0, 1);
          }

          if ((!left.empty() && !isdigit(left[0])) || (!right.empty() && !isdigit(right[0]))) {
            auto indexRight = 0UL;

            auto& valueOfVar = (!left.empty() && !isdigit(left[0])) ? left : right;

            if (!valueOfVar.empty()) {
              for (auto pairRight : kRegisterMap) {
                ++indexRight;

                CompilerKit::STLString instr = "mov ";

                if (pairRight != valueOfVar) {
                  auto& valueOfVarOpposite = (!left.empty() && isdigit(left[0])) ? left : right;

                  syntax_tree.fUserValue +=
                      instr + kRegisterList[indexRight + 1] + ", " + valueOfVarOpposite + "\n";
                  syntax_tree.fUserValue += "cmp " + kRegisterList[kRegisterMap.size() - 1] + "," +
                                            kRegisterList[indexRight + 1] + "\n";

                  goto lc_done_iterarting_on_if;
                }

                auto& valueOfVarOpposite = (!left.empty() && isdigit(left[0])) ? left : right;

                syntax_tree.fUserValue +=
                    instr + kRegisterList[indexRight + 1] + ", " + valueOfVarOpposite + "\n";
                syntax_tree.fUserValue += "cmp " + kRegisterList[kRegisterMap.size() - 1] + ", " +
                                          kRegisterList[indexRight + 1] + "\n";

                break;
              }
            }
          }

        lc_done_iterarting_on_if:

          CompilerKit::STLString symbol_name_fn = text;

          symbol_name_fn.erase(symbol_name_fn.find(keyword.first.fKeywordName));

          for (auto& ch : symbol_name_fn) {
            if (ch == ' ') ch = '_';
          }

          syntax_tree.fUserValue +=
              "jge __OFFSET_ON_TRUE_LC\nsegment .code64 __OFFSET_ON_TRUE_LC:\n";
        }

        break;
      }
      case CompilerKit::KeywordKind::kKeywordKindFunctionStart: {
        for (auto& ch : text) {
          if (isdigit(ch)) {
            goto dont_accept_func;
          }
        }

        goto accept_func;

      dont_accept_func:
        break;

      accept_func: {
        CompilerKit::STLString symbol_name_fn = text;
        size_t                 indexFnName    = 0;

        // this one is for the type.
        for (auto& ch : text) {
          ++indexFnName;

          if (ch == '\t') break;
          if (ch == ' ') break;
        }

        symbol_name_fn = text.substr(indexFnName);

        if (text.find("return ") != CompilerKit::STLString::npos) {
          text.erase(0, text.find("return "));
          break;
        }

        // Check if this is a function call (ends with ;)
        if (text.ends_with(";") && text.find("return") == CompilerKit::STLString::npos) {
          // Handle function call/jump
          auto it = std::find_if(
              kOriginMap.begin(), kOriginMap.end(),
              [&symbol_name_fn](std::pair<CompilerKit::STLString, std::uintptr_t> pair) -> bool {
                return symbol_name_fn.find(pair.first) != CompilerKit::STLString::npos;
              });

          if (it != kOriginMap.end()) {
            std::stringstream ss;
            ss << std::hex << it->second;

            syntax_tree.fUserValue += "jmp " + ss.str() + "\n";
            kOrigin += 1UL;
          }
          break;
        }

        indexFnName = 0;

        // Extract clean function name
        CompilerKit::STLString cleanFnName = symbol_name_fn;
        if (cleanFnName.find("(") != CompilerKit::STLString::npos) {
          cleanFnName = cleanFnName.substr(0, cleanFnName.find("("));
        }

        // Remove whitespace/tabs
        while (!cleanFnName.empty() && (cleanFnName.back() == ' ' || cleanFnName.back() == '\t')) {
          cleanFnName.pop_back();
        }
        while (!cleanFnName.empty() &&
               (cleanFnName.front() == ' ' || cleanFnName.front() == '\t')) {
          cleanFnName.erase(0, 1);
        }

        // Extract function arguments
        auto args = nectar_extract_function_args(text);

        // Generate mangled name
        auto mangled_name = nectar_mangle_name(cleanFnName, args);

        // Generate function label and prologue
        syntax_tree.fUserValue += "public_segment .code64 " + mangled_name + "\n";
        syntax_tree.fUserValue += nectar_generate_prologue();

        // Initialize function-local state
        kContext.fVariables.clear();
        kContext.fStackOffset  = 0;
        kContext.fMaxStackUsed = 0;

        // Process function parameters
        nectar_process_function_parameters(args);

        // Push function scope
        nectar_push_scope(ScopeKind::kScopeFunction, cleanFnName);

        ++kFunctionEmbedLevel;

        kOriginMap.push_back({mangled_name, kOrigin});
        kOrigin += 2UL;  // Account for prologue instructions

        break;
      }
      }
      case CompilerKit::KeywordKind::kKeywordKindFunctionEnd: {
        if (kOnClassScope) --kOnClassScope;

        if (text.ends_with(";")) break;

        --kFunctionEmbedLevel;

        if (kRegisterMap.size() > kRegisterList.size()) {
          --kFunctionEmbedLevel;
        }

        // Pop function scope
        nectar_pop_scope();

        // Clear function-local state
        if (kFunctionEmbedLevel < 1) {
          kRegisterMap.clear();
          kContext.fVariables.clear();
        }

        break;
      }
      case CompilerKit::KeywordKind::kKeywordKindEndInstr:
      case CompilerKit::KeywordKind::kKeywordKindVariableInc:
      case CompilerKit::KeywordKind::kKeywordKindVariableDec:
      case CompilerKit::KeywordKind::kKeywordKindVariableAssign: {
        CompilerKit::STLString valueOfVar = "";

        if (keyword.first.fKeywordKind == CompilerKit::KeywordKind::kKeywordKindVariableInc) {
          valueOfVar = text.substr(text.find("+=") + 2);
        } else if (keyword.first.fKeywordKind ==
                   CompilerKit::KeywordKind::kKeywordKindVariableDec) {
          valueOfVar = text.substr(text.find("-=") + 2);
        } else if (keyword.first.fKeywordKind ==
                   CompilerKit::KeywordKind::kKeywordKindVariableAssign) {
          valueOfVar = text.substr(text.find("=") + 1);
        } else if (keyword.first.fKeywordKind == CompilerKit::KeywordKind::kKeywordKindEndInstr) {
          break;
        }

        while (valueOfVar.find(";") != CompilerKit::STLString::npos &&
               keyword.first.fKeywordKind != CompilerKit::KeywordKind::kKeywordKindEndInstr) {
          valueOfVar.erase(valueOfVar.find(";"));
        }

        CompilerKit::STLString varName = text;

        if (keyword.first.fKeywordKind == CompilerKit::KeywordKind::kKeywordKindVariableInc) {
          varName.erase(varName.find("+="));
        } else if (keyword.first.fKeywordKind ==
                   CompilerKit::KeywordKind::kKeywordKindVariableDec) {
          varName.erase(varName.find("-="));
        } else if (keyword.first.fKeywordKind ==
                   CompilerKit::KeywordKind::kKeywordKindVariableAssign) {
          varName.erase(varName.find("="));
        } else if (keyword.first.fKeywordKind == CompilerKit::KeywordKind::kKeywordKindEndInstr) {
          varName.erase(varName.find(";"));
        }

        static bool typeFound = false;

        for (auto& keyword : kKeywords) {
          if (keyword.fKeywordKind == CompilerKit::KeywordKind::kKeywordKindType) {
            if (text.find(keyword.fKeywordName) != CompilerKit::STLString::npos) {
              if (text[text.find(keyword.fKeywordName)] == ' ') {
                typeFound = false;
                continue;
              }

              typeFound = true;
            }
          }
        }

        CompilerKit::STLString instr = "mov ";

        std::vector<CompilerKit::STLString> newVars;

        if (typeFound &&
            keyword.first.fKeywordKind != CompilerKit::KeywordKind::kKeywordKindVariableInc &&
            keyword.first.fKeywordKind != CompilerKit::KeywordKind::kKeywordKindVariableDec) {
          if (kRegisterMap.size() > kRegisterList.size()) {
            ++kFunctionEmbedLevel;
          }

          while (varName.find(" ") != CompilerKit::STLString::npos) {
            varName.erase(varName.find(" "), 1);
          }

          while (varName.find("\t") != CompilerKit::STLString::npos) {
            varName.erase(varName.find("\t"), 1);
          }

          // Remove whitespace only (keep operators and quotes)
          while (!valueOfVar.empty() && (valueOfVar[0] == ' ' || valueOfVar[0] == '\t')) {
            valueOfVar.erase(0, 1);
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
                syntax_tree.fUserValue += "segment .data64 __NECTAR_LOCAL_VAR_" + varName + ": db " +
                                         valueOfVar + ", 0\n\n";
                syntax_tree.fUserValue += instr + kRegisterList[kRegisterMap.size() - 1] + ", " +
                                          "__NECTAR_LOCAL_VAR_" + varName + "\n";
                kOrigin += 1UL;
              } else {
                syntax_tree.fUserValue +=
                    instr + kRegisterList[kRegisterMap.size() - 1] + ", " + valueOfVar + "\n";
                kOrigin += 1UL;
              }

              goto done;
            }
          }

          if (((int) indexRight - 1) < 0) {
            if (valueOfVar[0] == '\"') {
              syntax_tree.fUserValue +=
                  "segment .data64 __NECTAR_LOCAL_VAR_" + varName + ": db " + valueOfVar + ", 0\n";
              syntax_tree.fUserValue += instr + kRegisterList[kRegisterMap.size()] + ", " +
                                        "__NECTAR_LOCAL_VAR_" + varName + "\n";
              kOrigin += 1UL;
            } else {
              auto mangled = valueOfVar;

              if (mangled.find("(") != std::string::npos) {
                auto ret = mangled.erase(mangled.find("("));
                mangled  = "__NECTAR_";
                mangled += ret;

                syntax_tree.fUserValue += "jmp " + mangled + "\n";
                syntax_tree.fUserValue +=
                    instr + " rax, " + kRegisterList[kRegisterMap.size()] + "\n";

                goto done;
              }

              syntax_tree.fUserValue +=
                  instr + kRegisterList[kRegisterMap.size()] + ", " + mangled + "\n";
              kOrigin += 1UL;
            }

            goto done;
          }

          if (!valueOfVar.empty() && valueOfVar[0] != '\"' && valueOfVar[0] != '\'' &&
              !isdigit(valueOfVar[0])) {
            for (auto pair : kRegisterMap) {
              if (pair == valueOfVar) goto done;
            }
          }

        done:
          for (auto& keyword : kKeywords) {
            if (keyword.fKeywordKind == CompilerKit::KeywordKind::kKeywordKindType &&
                varName.find(keyword.fKeywordName) != CompilerKit::STLString::npos) {
              varName.erase(varName.find(keyword.fKeywordName), keyword.fKeywordName.size());
              break;
            }
          }

          newVars.push_back(varName);

          break;
        }

        kRegisterMap.insert(kRegisterMap.end(), newVars.begin(), newVars.end());

        if (keyword.second > 0 && kKeywords[keyword.second - 1].fKeywordKind ==
                                      CompilerKit::KeywordKind::kKeywordKindType ||
            kKeywords[keyword.second - 1].fKeywordKind ==
                CompilerKit::KeywordKind::kKeywordKindTypePtr) {
          syntax_tree.fUserValue += "\n";
          continue;
        }

        if (keyword.first.fKeywordKind == CompilerKit::KeywordKind::kKeywordKindEndInstr) {
          syntax_tree.fUserValue += "\n";
          continue;
        }

        if (keyword.first.fKeywordKind == CompilerKit::KeywordKind::kKeywordKindVariableInc) {
          instr = "add ";
        } else if (keyword.first.fKeywordKind ==
                   CompilerKit::KeywordKind::kKeywordKindVariableDec) {
          instr = "sub ";
        }

        CompilerKit::STLString varErrCpy = varName;

        while (varName.find(" ") != CompilerKit::STLString::npos) {
          varName.erase(varName.find(" "), 1);
        }

        while (varName.find("\t") != CompilerKit::STLString::npos) {
          varName.erase(varName.find("\t"), 1);
        }

        std::size_t indxReg = 0UL;

        while (!valueOfVar.empty() && (valueOfVar[0] == ' ' || valueOfVar[0] == '\t')) {
          valueOfVar.erase(0, 1);
        }

        while (valueOfVar.find(" ") != CompilerKit::STLString::npos) {
          valueOfVar.erase(valueOfVar.find(" "), 1);
        }

        while (valueOfVar.find("\t") != CompilerKit::STLString::npos) {
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

        if (CompilerKit::STLString::npos == varName.find("*"))
            CompilerKit::Detail::print_error("Type not declared: " + varName, file);

        varName = varName.substr(varName.find("*") + 1);

        for (auto pair : kRegisterMap) {
          ++indxReg;

          if (pair != varName) continue;

          std::size_t indexRight = 0ul;

          for (auto pairRight : kRegisterMap) {
            ++indexRight;

            if (pairRight != varName) {
              syntax_tree.fUserValue +=
                  instr + kRegisterList[kRegisterMap.size()] + ", " + valueOfVar + "\n";
              kOrigin += 1UL;
              continue;
            }

            syntax_tree.fUserValue +=
                instr + kRegisterList[indexRight - 1] + ", " + valueOfVar + "\n";
            kOrigin += 1UL;
            break;
          }

          newVars.push_back(varName);
          break;
        }

        if (syntax_tree.fUserValue.empty()) {
          CompilerKit::Detail::print_error("Variable not declared: " + varName, file);
        }

        kRegisterMap.insert(kRegisterMap.end(), newVars.begin(), newVars.end());

        break;
      }
      case CompilerKit::KeywordKind::kKeywordKindReturn: {
        try {
          auto                   pos     = text.find("return") + strlen("return") + 1;
          CompilerKit::STLString subText = text.substr(pos);
          if (subText.find(";") == CompilerKit::STLString::npos) break;

          subText        = subText.erase(subText.find(";"));
          size_t indxReg = 0UL;

          // Trim whitespace
          while (!subText.empty() && (subText.front() == ' ' || subText.front() == '\t')) {
            subText.erase(0, 1);
          }
          while (!subText.empty() && (subText.back() == ' ' || subText.back() == '\t')) {
            subText.pop_back();
          }

          if (!subText.empty() && subText[0] != '\"' && subText[0] != '\'') {
            if (!isdigit(subText[0])) {
              for (auto pair : kRegisterMap) {
                ++indxReg;

                syntax_tree.fUserValue += "mov rax, " + kRegisterList[indxReg - 1] + "\n" +
                                         nectar_generate_epilogue() + "ret\n";
                kOrigin += 3UL;  // mov + epilogue (2) + ret

                break;
              }
            } else {
              syntax_tree.fUserValue +=
                  "mov rax, " + subText + "\n" + nectar_generate_epilogue() + "ret\n";
              kOrigin += 3UL;

              break;
            }
          } else if (!subText.empty()) {
            syntax_tree.fUserValue += "__NECTAR_LOCAL_RETURN_STRING: db " + subText +
                                     ", 0\nmov rcx, __NECTAR_LOCAL_RETURN_STRING\n";
            syntax_tree.fUserValue += "mov rax, rcx\n" + nectar_generate_epilogue() + "ret\n";
            kOrigin += 4UL;  // mov rcx + mov rax + epilogue (2) + ret

            break;
          }

          if (syntax_tree.fUserValue.empty()) {
            if (subText.find("(") != CompilerKit::STLString::npos) {
              // Check for namespace resolution
              if (subText.find("::") != CompilerKit::STLString::npos) {
                auto colonPos = subText.find("::");
                auto nsName = subText.substr(0, colonPos);
                auto funcPart = subText.substr(colonPos + 2);

                // Trim
                while (!nsName.empty() && (nsName.front() == ' ' || nsName.front() == '\t')) {
                  nsName.erase(0, 1);
                }
                while (!nsName.empty() && (nsName.back() == ' ' || nsName.back() == '\t')) {
                  nsName.pop_back();
                }

                // Extract function name
                auto funcName = funcPart;
                if (funcName.find("(") != CompilerKit::STLString::npos) {
                  funcName = funcName.substr(0, funcName.find("("));
                }

                // Trim
                while (!funcName.empty() && (funcName.front() == ' ' || funcName.front() == '\t')) {
                  funcName.erase(0, 1);
                }
                while (!funcName.empty() && (funcName.back() == ' ' || funcName.back() == '\t')) {
                  funcName.pop_back();
                }

                // Generate mangled name
                nectar_push_scope(ScopeKind::kScopeNamespace, nsName);
                auto mangled = nectar_mangle_name(funcName);
                nectar_pop_scope();

                syntax_tree.fUserValue +=
                    "call " + mangled + "\n" + nectar_generate_epilogue() + "ret\n";
                kOrigin += 3UL;
                break;
              } else {
                // Regular function call
                subText.erase(subText.find("("));

                auto it = std::find_if(
                    kOriginMap.begin(), kOriginMap.end(),
                    [&subText](std::pair<CompilerKit::STLString, std::uintptr_t> pair) -> bool {
                      return pair.first.find(subText) != CompilerKit::STLString::npos;
                    });

                if (it != kOriginMap.end()) {
                  std::stringstream ss;
                  ss << std::hex << it->second;

                  syntax_tree.fUserValue +=
                      "call " + ss.str() + "\n" + nectar_generate_epilogue() + "ret\n";
                  kOrigin += 3UL;
                  break;
                }
              }
            }
          }

          syntax_tree.fUserValue += nectar_generate_epilogue() + "ret\n";
          kOrigin += 2UL;

          break;
        } catch (...) {
          syntax_tree.fUserValue += nectar_generate_epilogue() + "ret\n";
          kOrigin += 2UL;

          break;
        }
      }
      default: {
        continue;
      }
    }
  }

  return this->CompilePass2(text, file, syntax_tree);
}

/// \brief Parse C++ namespaces and objects.
/// \param CompilerKit::SyntaxLeafList::SyntaxLeaf the leaf to build upon.
CompilerKit::SyntaxLeafList::SyntaxLeaf CompilerFrontendCPlusPlusAMD64::CompilePass2(
    CompilerKit::STLString& text, const CompilerKit::STLString& file,
    CompilerKit::SyntaxLeafList::SyntaxLeaf& syntax_tree) {
  // Handle namespace entry
  if (text.find("namespace") != CompilerKit::STLString::npos &&
      text.find("{") != CompilerKit::STLString::npos) {
    auto nsPos = text.find("namespace") + strlen("namespace");
    auto bracePos = text.find("{");

    auto nsName = text.substr(nsPos, bracePos - nsPos);

    // Trim whitespace
    while (!nsName.empty() && (nsName.front() == ' ' || nsName.front() == '\t')) {
      nsName.erase(0, 1);
    }
    while (!nsName.empty() && (nsName.back() == ' ' || nsName.back() == '\t')) {
      nsName.pop_back();
    }

    if (!nsName.empty()) {
      nectar_push_scope(ScopeKind::kScopeNamespace, nsName);
      ++kNamespaceEmbedLevel;
    }
  }

  // Handle namespace exit
  if (text.find("}") != CompilerKit::STLString::npos && kNamespaceEmbedLevel > 0) {
    if (text.find("//") != CompilerKit::STLString::npos &&
        text.find("namespace") != CompilerKit::STLString::npos) {
      --kNamespaceEmbedLevel;
      nectar_pop_scope();
    }
  }

  // Handle namespace resolution (::) - only in non-assignment, non-declaration contexts
  if (text.find("::") != CompilerKit::STLString::npos) {
    auto colonPos = text.find("::");

    // Extract namespace and function name
    auto nsName = text.substr(0, colonPos);
    auto restOfText = text.substr(colonPos + 2);

    // Trim namespace name
    while (!nsName.empty() && (nsName.front() == ' ' || nsName.front() == '\t')) {
      nsName.erase(0, 1);
    }
    while (!nsName.empty() && (nsName.back() == ' ' || nsName.back() == '\t')) {
      nsName.pop_back();
    }

    // Extract function name (everything before '(' if exists)
    auto funcName = restOfText;
    if (funcName.find("(") != CompilerKit::STLString::npos) {
      funcName = funcName.substr(0, funcName.find("("));
    }
    if (funcName.find(";") != CompilerKit::STLString::npos) {
      funcName = funcName.substr(0, funcName.find(";"));
    }

    // Trim function name
    while (!funcName.empty() && (funcName.front() == ' ' || funcName.front() == '\t')) {
      funcName.erase(0, 1);
    }
    while (!funcName.empty() && (funcName.back() == ' ' || funcName.back() == '\t')) {
      funcName.pop_back();
    }

    // Generate mangled name for the call
    nectar_push_scope(ScopeKind::kScopeNamespace, nsName);
    auto mangled = nectar_mangle_name(funcName);
    nectar_pop_scope();

    // Only generate call if in return context or explicit call
    if (text.find("return") != CompilerKit::STLString::npos) {
      // This will be handled by the return handler, just update the text to use mangled name
      // Store this for use by the return handler
    } else {
      syntax_tree.fUserValue += "call " + mangled + "\n";
      kOrigin += 1UL;
    }
  }

  // Handle class entry
  if ((text.find("class") != CompilerKit::STLString::npos ||
       text.find("struct") != CompilerKit::STLString::npos) &&
      text.find("{") != CompilerKit::STLString::npos) {
    CompilerKit::STLString keyword = text.find("class") != CompilerKit::STLString::npos ? "class" : "struct";
    auto classPos = text.find(keyword) + keyword.length();
    auto bracePos = text.find("{");

    auto className = text.substr(classPos, bracePos - classPos);

    // Trim whitespace
    while (!className.empty() && (className.front() == ' ' || className.front() == '\t')) {
      className.erase(0, 1);
    }
    while (!className.empty() && (className.back() == ' ' || className.back() == '\t')) {
      className.pop_back();
    }

    if (!className.empty()) {
      nectar_push_scope(ScopeKind::kScopeClass, className);
      ++kOnClassScope;
    }
  }

  // Handle class exit
  if (text.find("}") != CompilerKit::STLString::npos && kOnClassScope > 0) {
    if (text.find(";") != CompilerKit::STLString::npos) {
      --kOnClassScope;
      nectar_pop_scope();
    }
  }

  return syntax_tree;
}

/////////////////////////////////////////////////////////////////////////////////////////

// HELPER FUNCTION IMPLEMENTATIONS

/////////////////////////////////////////////////////////////////////////////////////////

/// \brief Push a new scope onto the scope stack
static void nectar_push_scope(ScopeKind kind, const CompilerKit::STLString& name) {
  CompilerScope scope;
  scope.fKind = kind;
  scope.fName = name;

  // Build mangled prefix based on current scope stack
  for (const auto& s : kContext.fScopeStack) {
    if (s.fKind == ScopeKind::kScopeNamespace) {
      scope.fMangledPrefix += "N_" + s.fName + "_";
    } else if (s.fKind == ScopeKind::kScopeClass) {
      scope.fMangledPrefix += "C_" + s.fName + "_";
    }
  }

  kContext.fScopeStack.push_back(scope);
}

/// \brief Pop the current scope from the scope stack
static void nectar_pop_scope() {
  if (!kContext.fScopeStack.empty()) {
    kContext.fScopeStack.pop_back();
  }
}

/// \brief Extract function arguments from function declaration
static std::vector<CompilerKit::STLString> nectar_extract_function_args(
    const CompilerKit::STLString& text) {
  std::vector<CompilerKit::STLString> args;

  auto openParen  = text.find("(");
  auto closeParen = text.find(")");

  if (openParen == CompilerKit::STLString::npos || closeParen == CompilerKit::STLString::npos ||
      closeParen <= openParen) {
    return args;
  }

  auto argsText = text.substr(openParen + 1, closeParen - openParen - 1);

  // Trim whitespace
  while (!argsText.empty() && (argsText.front() == ' ' || argsText.front() == '\t')) {
    argsText.erase(0, 1);
  }
  while (!argsText.empty() && (argsText.back() == ' ' || argsText.back() == '\t')) {
    argsText.pop_back();
  }

  if (argsText.empty() || argsText == "void") {
    return args;
  }

  // Simple comma-separated parsing
  std::size_t pos = 0;
  while (pos < argsText.size()) {
    auto commaPos = argsText.find(",", pos);
    if (commaPos == CompilerKit::STLString::npos) {
      commaPos = argsText.size();
    }

    auto arg = argsText.substr(pos, commaPos - pos);

    // Extract type name (skip variable name)
    std::size_t lastSpace = arg.rfind(' ');
    if (lastSpace != CompilerKit::STLString::npos) {
      arg = arg.substr(0, lastSpace);
    }

    // Trim
    while (!arg.empty() && (arg.front() == ' ' || arg.front() == '\t')) {
      arg.erase(0, 1);
    }
    while (!arg.empty() && (arg.back() == ' ' || arg.back() == '\t')) {
      arg.pop_back();
    }

    if (!arg.empty()) {
      args.push_back(arg);
    }

    pos = commaPos + 1;
  }

  return args;
}

/// \brief Mangle a function or method name according to Nectar mangling scheme
static CompilerKit::STLString nectar_mangle_name(const CompilerKit::STLString& identifier,
                                                const std::vector<CompilerKit::STLString>& args) {
  CompilerKit::STLString mangled = "__NECTAR_";

  // Add scope chain
  for (const auto& scope : kContext.fScopeStack) {
    if (scope.fKind == ScopeKind::kScopeNamespace) {
      mangled += "N_" + scope.fName + "_";
    } else if (scope.fKind == ScopeKind::kScopeClass) {
      mangled += "C_" + scope.fName + "_";
    }
  }

  // Check if it's a constructor or destructor
  if (!kContext.fScopeStack.empty() &&
      kContext.fScopeStack.back().fKind == ScopeKind::kScopeClass) {
    if (identifier == kContext.fScopeStack.back().fName) {
      mangled += "CTOR";
      return mangled;
    } else if (identifier == "~" + kContext.fScopeStack.back().fName) {
      mangled += "DTOR";
      return mangled;
    }
  }

  // Check if we're in a class scope for member functions
  bool inClass = false;
  for (const auto& scope : kContext.fScopeStack) {
    if (scope.fKind == ScopeKind::kScopeClass) {
      inClass = true;
      break;
    }
  }

  if (inClass) {
    mangled += "M_" + identifier;
  } else {
    mangled += "F_" + identifier;
  }

  // Add argument types if provided
  if (!args.empty()) {
    mangled += "_A" + std::to_string(args.size());
    for (const auto& arg : args) {
      mangled += "_" + arg;
    }
  }

  return mangled;
}

/// \brief Generate function prologue
static CompilerKit::STLString nectar_generate_prologue() {
  return "push rbp\nmov rbp, rsp\n";
}

/// \brief Generate function epilogue
static CompilerKit::STLString nectar_generate_epilogue() {
  return "mov rsp, rbp\npop rbp\n";
}

/// \brief Allocate a variable on the stack
static Int32 nectar_allocate_stack_variable(const CompilerKit::STLString& var_name, Int32 size) {
  kContext.fStackOffset -= size;
  kContext.fMaxStackUsed = std::min(kContext.fStackOffset, kContext.fMaxStackUsed);

  VariableInfo varInfo;
  varInfo.fName        = var_name;
  varInfo.fLocation    = VarLocation::kStack;
  varInfo.fStackOffset = kContext.fStackOffset;
  varInfo.fSize        = size;
  varInfo.fLastUsed    = kContext.fInstructionCounter;
  kContext.fVariables.push_back(varInfo);

  return kContext.fStackOffset;
}

/// \brief Find a variable by name
static VariableInfo* nectar_find_variable(const CompilerKit::STLString& var_name) {
  for (auto& var : kContext.fVariables) {
    if (var.fName == var_name) {
      return &var;
    }
  }
  return nullptr;
}

/// \brief Get variable reference (register or stack location)
static CompilerKit::STLString nectar_get_variable_ref(const CompilerKit::STLString& var_name) {
  auto* varInfo = nectar_find_variable(var_name);
  if (!varInfo) {
    return "";
  }

  varInfo->fLastUsed = kContext.fInstructionCounter;

  if (varInfo->fLocation == VarLocation::kRegister) {
    return varInfo->fRegister;
  } else {
    // Stack or spilled
    return "qword [rbp" + std::to_string(varInfo->fStackOffset) + "]";
  }
}

/// \brief Allocate a register for a variable
static CompilerKit::STLString nectar_allocate_register(const CompilerKit::STLString& var_name) {
  // Check if variable already has a register
  auto* existing = nectar_find_variable(var_name);
  if (existing && existing->fLocation == VarLocation::kRegister) {
    return existing->fRegister;
  }

  // Find a free register
  for (const auto& reg : kRegisterList) {
    bool inUse = false;
    for (const auto& var : kContext.fVariables) {
      if (var.fLocation == VarLocation::kRegister && var.fRegister == reg) {
        inUse = true;
        break;
      }
    }

    if (!inUse) {
      // Allocate this register
      if (existing) {
        existing->fLocation = VarLocation::kRegister;
        existing->fRegister = reg;
        existing->fLastUsed = kContext.fInstructionCounter;
      } else {
        VariableInfo varInfo;
        varInfo.fName     = var_name;
        varInfo.fLocation = VarLocation::kRegister;
        varInfo.fRegister = reg;
        varInfo.fLastUsed = kContext.fInstructionCounter;
        kContext.fVariables.push_back(varInfo);
      }
      return reg;
    }
  }

  // No free register
  return "";
}

/// \brief Spill the least recently used variable to stack
static CompilerKit::STLString nectar_spill_lru_variable() {
  CompilerKit::STLString spillCode;

  // Find LRU variable in register (that's not a parameter)
  VariableInfo* lruVar      = nullptr;
  UInt32        minLastUsed = UINT32_MAX;

  for (auto& var : kContext.fVariables) {
    if (var.fLocation == VarLocation::kRegister && !var.fIsParameter &&
        var.fLastUsed < minLastUsed) {
      lruVar      = &var;
      minLastUsed = var.fLastUsed;
    }
  }

  if (!lruVar) {
    return "";  // No variable to spill
  }

  // Allocate stack space
  kContext.fStackOffset -= lruVar->fSize;
  kContext.fMaxStackUsed = std::min(kContext.fStackOffset, kContext.fMaxStackUsed);

  // Generate spill code
  spillCode =
      "mov qword [rbp" + std::to_string(kContext.fStackOffset) + "], " + lruVar->fRegister + "\n";

  // Update variable info
  lruVar->fLocation    = VarLocation::kStackSpill;
  lruVar->fStackOffset = kContext.fStackOffset;
  auto spilledReg      = lruVar->fRegister;
  lruVar->fRegister    = "";

  return spillCode;
}

/// \brief Add a class member to the struct map
static void nectar_add_class_member(const CompilerKit::STLString& class_name,
                                   const CompilerKit::STLString& member_name, Int32 size) {
  // Find or create struct map entry
  CompilerStructMap* structMap = nullptr;
  for (auto& sm : kContext.fStructMapVector) {
    if (sm.fName == class_name) {
      structMap = &sm;
      break;
    }
  }

  if (!structMap) {
    CompilerStructMap newMap;
    newMap.fName = class_name;
    kContext.fStructMapVector.push_back(newMap);
    structMap = &kContext.fStructMapVector.back();
  }

  // Calculate offset
  UInt32 offset = 0;
  if (!structMap->fOffsets.empty()) {
    offset = structMap->fOffsets.back().first + 8;  // Assume 8-byte members for now
  }

  structMap->fOffsets.emplace_back(offset, member_name);
}

/// \brief Get the size of a class
static Int32 nectar_get_class_size(const CompilerKit::STLString& class_name) {
  for (const auto& sm : kContext.fStructMapVector) {
    if (sm.fName == class_name) {
      if (sm.fOffsets.empty()) {
        return 0;
      }
      return sm.fOffsets.back().first + 8;  // Last offset + size
    }
  }
  return 0;
}

/// \brief Generate constructor call
static CompilerKit::STLString nectar_generate_constructor_call(
    const CompilerKit::STLString& class_name, const CompilerKit::STLString& obj_name) {
  auto size   = nectar_get_class_size(class_name);
  auto offset = nectar_allocate_stack_variable(obj_name, size == 0 ? 8 : size);

  nectar_push_scope(ScopeKind::kScopeClass, class_name);
  auto ctor_mangled = nectar_mangle_name(class_name);
  nectar_pop_scope();

  CompilerKit::STLString code;
  code += "lea r8, [rbp" + std::to_string(offset) + "]\n";
  code += "call " + ctor_mangled + "\n";
  return code;
}

/// \brief Generate destructor call
static CompilerKit::STLString nectar_generate_destructor_call(
    const CompilerKit::STLString& class_name, const CompilerKit::STLString& obj_name) {
  auto* varInfo = nectar_find_variable(obj_name);
  if (!varInfo) {
    return "";
  }

  nectar_push_scope(ScopeKind::kScopeClass, class_name);
  auto dtor_mangled = nectar_mangle_name("~" + class_name);
  nectar_pop_scope();

  CompilerKit::STLString code;
  if (varInfo->fLocation == VarLocation::kStack || varInfo->fLocation == VarLocation::kStackSpill) {
    code += "lea r8, [rbp" + std::to_string(varInfo->fStackOffset) + "]\n";
  } else {
    code += "mov r8, " + varInfo->fRegister + "\n";
  }
  code += "call " + dtor_mangled + "\n";
  return code;
}

/// \brief Process function parameters per PEF calling convention
static void nectar_process_function_parameters(const std::vector<CompilerKit::STLString>& args) {
  for (size_t i = 0; i < args.size() && i < 8; ++i) {
    VariableInfo param;
    param.fName        = "arg" + std::to_string(i);
    param.fLocation    = VarLocation::kRegister;
    param.fRegister    = kRegisterConventionCallList[i];
    param.fIsParameter = true;
    param.fTypeName    = args[i];
    param.fLastUsed    = kContext.fInstructionCounter;
    kContext.fVariables.push_back(param);
  }

  // Args beyond r15 go on stack
  for (size_t i = 8; i < args.size(); ++i) {
    Int32        offset = 16 + (i - 8) * 8;
    VariableInfo param;
    param.fName        = "arg" + std::to_string(i);
    param.fLocation    = VarLocation::kStack;
    param.fStackOffset = offset;  // Positive (before rbp)
    param.fIsParameter = true;
    param.fTypeName    = args[i];
    param.fLastUsed    = kContext.fInstructionCounter;
    kContext.fVariables.push_back(param);
  }
}

/////////////////////////////////////////////////////////////////////////////////////////

/**
 * @brief C++ assembler class.
 */

/////////////////////////////////////////////////////////////////////////////////////////

class AssemblyCPlusPlusInterfaceAMD64 final CK_ASSEMBLY_INTERFACE {
 public:
  explicit AssemblyCPlusPlusInterfaceAMD64()  = default;
  ~AssemblyCPlusPlusInterfaceAMD64() override = default;

  NECTAR_COPY_DEFAULT(AssemblyCPlusPlusInterfaceAMD64);

  UInt32 Arch() noexcept override { return CompilerKit::AssemblyFactory::kArchAMD64; }

  Int32 CompileToFormat(CompilerKit::STLString src, Int32 arch) override {
    if (kFrontend == nullptr) return EXIT_FAILURE;

    CompilerKit::STLString dest = src;
    dest += ".masm";

    std::ofstream out_fp(dest);
    std::ifstream src_fp = std::ifstream(src);

    CompilerKit::STLString line_source;

    std::stringstream ss;
    ss << std::hex << kOrigin;

    out_fp << "%bits 64\n";
    out_fp << "%org 0x" << ss.str() << "\n\n";

    while (std::getline(src_fp, line_source)) {
      out_fp << kFrontend->Compile(line_source, src).fUserValue;
    }

    return EXIT_SUCCESS;
  }
};

/////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////

#define kExtListCxx {".cpp", ".cc", ".cc", ".c++", ".cp", ".ipp", ".i++"}

NECTAR_MODULE(CompilerCPlusPlusAMD64) {
  bool skip = false;

  kKeywords.emplace_back("if", CompilerKit::KeywordKind::kKeywordKindIf);
  kKeywords.emplace_back("else", CompilerKit::KeywordKind::kKeywordKindElse);
  kKeywords.emplace_back("else if", CompilerKit::KeywordKind::kKeywordKindElseIf);

  kKeywords.emplace_back("class", CompilerKit::KeywordKind::kKeywordKindClass);
  kKeywords.emplace_back("struct", CompilerKit::KeywordKind::kKeywordKindClass);
  kKeywords.emplace_back("namespace", CompilerKit::KeywordKind::kKeywordKindNamespace);
  kKeywords.emplace_back("typedef", CompilerKit::KeywordKind::kKeywordKindTypedef);
  kKeywords.emplace_back("using", CompilerKit::KeywordKind::kKeywordKindTypedef);
  kKeywords.emplace_back("{", CompilerKit::KeywordKind::kKeywordKindBodyStart);
  kKeywords.emplace_back("}", CompilerKit::KeywordKind::kKeywordKindBodyEnd);
  kKeywords.emplace_back("auto", CompilerKit::KeywordKind::kKeywordKindType);
  kKeywords.emplace_back("int", CompilerKit::KeywordKind::kKeywordKindType);
  kKeywords.emplace_back("bool", CompilerKit::KeywordKind::kKeywordKindType);
  kKeywords.emplace_back("unsigned", CompilerKit::KeywordKind::kKeywordKindType);
  kKeywords.emplace_back("short", CompilerKit::KeywordKind::kKeywordKindType);
  kKeywords.emplace_back("char", CompilerKit::KeywordKind::kKeywordKindType);
  kKeywords.emplace_back("long", CompilerKit::KeywordKind::kKeywordKindType);
  kKeywords.emplace_back("float", CompilerKit::KeywordKind::kKeywordKindType);
  kKeywords.emplace_back("double", CompilerKit::KeywordKind::kKeywordKindType);
  kKeywords.emplace_back("void", CompilerKit::KeywordKind::kKeywordKindType);

  kKeywords.emplace_back("*", CompilerKit::KeywordKind::kKeywordKindTypePtr);

  kKeywords.emplace_back("(", CompilerKit::KeywordKind::kKeywordKindFunctionStart);
  kKeywords.emplace_back(")", CompilerKit::KeywordKind::kKeywordKindFunctionEnd);
  kKeywords.emplace_back("=", CompilerKit::KeywordKind::kKeywordKindVariableAssign);
  kKeywords.emplace_back("+=", CompilerKit::KeywordKind::kKeywordKindVariableInc);
  kKeywords.emplace_back("-=", CompilerKit::KeywordKind::kKeywordKindVariableDec);
  kKeywords.emplace_back("const", CompilerKit::KeywordKind::kKeywordKindConstant);
  kKeywords.emplace_back("*", CompilerKit::KeywordKind::kKeywordKindPtr);
  kKeywords.emplace_back("->", CompilerKit::KeywordKind::kKeywordKindPtrAccess);
  kKeywords.emplace_back(".", CompilerKit::KeywordKind::kKeywordKindAccess);
  kKeywords.emplace_back(",", CompilerKit::KeywordKind::kKeywordKindArgSeparator);
  kKeywords.emplace_back(";", CompilerKit::KeywordKind::kKeywordKindEndInstr);
  kKeywords.emplace_back(":", CompilerKit::KeywordKind::kKeywordKindSpecifier);
  kKeywords.emplace_back("public:", CompilerKit::KeywordKind::kKeywordKindSpecifier);
  kKeywords.emplace_back("private:", CompilerKit::KeywordKind::kKeywordKindSpecifier);
  kKeywords.emplace_back("protected:", CompilerKit::KeywordKind::kKeywordKindSpecifier);
  kKeywords.emplace_back("final", CompilerKit::KeywordKind::kKeywordKindSpecifier);
  kKeywords.emplace_back("return", CompilerKit::KeywordKind::kKeywordKindReturn);
  kKeywords.emplace_back("/*", CompilerKit::KeywordKind::kKeywordKindCommentMultiLineStart);
  kKeywords.emplace_back("*/", CompilerKit::KeywordKind::kKeywordKindCommentMultiLineEnd);
  kKeywords.emplace_back("//", CompilerKit::KeywordKind::kKeywordKindCommentInline);
  kKeywords.emplace_back("==", CompilerKit::KeywordKind::kKeywordKindEq);
  kKeywords.emplace_back("!=", CompilerKit::KeywordKind::kKeywordKindNotEq);
  kKeywords.emplace_back(">=", CompilerKit::KeywordKind::kKeywordKindGreaterEq);
  kKeywords.emplace_back("<=", CompilerKit::KeywordKind::kKeywordKindLessEq);

  kErrorLimit = 0;

  kFrontend = new CompilerFrontendCPlusPlusAMD64();

  CompilerKit::StrongRef<AssemblyCPlusPlusInterfaceAMD64> mntPnt{
      new AssemblyCPlusPlusInterfaceAMD64()};
  kAssembler.Mount({mntPnt.Leak()});

  CompilerKit::install_signal(SIGSEGV, CompilerKit::Detail::drvi_crash_handler);

  // Ensure cleanup on exit
  std::atexit([]() {
    delete kFrontend;
    kFrontend = nullptr;
  });

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
        if (kFrontend) std::cout << kFrontend->Language() << "\n";

        return NECTAR_SUCCESS;
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

      CompilerKit::STLString err = "Unknown option: ";
      err += argv[index];

      CompilerKit::Detail::print_error(err, "cxxdrv");

      continue;
    }

    CompilerKit::STLString argv_i = argv[index];

    std::vector<CompilerKit::STLString> exts = kExtListCxx;

    for (CompilerKit::STLString ext : exts) {
      if (argv_i.ends_with(ext)) {
        if (kAssembler.Compile(argv_i, kMachine) != EXIT_SUCCESS) {
          return NECTAR_INVALID_DATA;
        }

        break;
      }
    }
  }

  kAssembler.Unmount();

  return NECTAR_SUCCESS;
}

//
// Last rev 25-8-7
//
