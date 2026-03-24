// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026, Amlal El Mahrouss (amlal@nekernel.org)
// Licensed under the Apache License, Version 2.0 (See accompanying
// file LICENSE or copy at http://www.apache.org/licenses/LICENSE-2.0)
// Official repository: https://github.com/ne-foss-org/nectar

/// @note This frontend is based on Nectar's AMD64 frontend.

/// BUGS: 0

#include <CompilerKit/AST.h>
#include <CompilerKit/UUID.h>
#include <CompilerKit/Utils/Compiler.h>

/* Nectar Compiler Driver. */
/* This is part of the CompilerKit. */
/* (c) Amlal El Mahrouss 2024-2026 */

/// @author Amlal El Mahrouss (amlal@nekernel.org)
/// @file NectarCompiler+PTX.cc
/// @brief NECTAR Compiler Driver (NVPTX).

/////////////////////////////////////

// INTERNALS OF THE NECTAR COMPILER

/////////////////////////////////////

/// @CompilerKit
/// @brief Avoids relative_path which could discard parts of the original.
static std::filesystem::path nectar_expand_home(const std::filesystem::path& input) {
  const std::string& raw = input.string();

  if (!raw.empty() && raw[0] == '~') {
    const char* home = std::getenv("HOME");
    if (!home) home = std::getenv("USERPROFILE");

    if (!home) throw std::runtime_error("Home directory not found in environment variables");

    return std::filesystem::path(home) / raw.substr(1);
  }

  return input;
}

/// \brief Register map, i.e ({foobar, %rd16}, etc...)
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

// NEW DATA STRUCTURES FOR NECTAR SUPPORT

/////////////////////////////////////////////////////////////////////////////////////////

/// \brief Scope kind enumeration
enum class ScopeKind {
  kScopeGlobal,
  kScopeNamespace,
  kScopeClass,
  kScopeFunction,
};

/// \brief Variable location enumeration
enum class VarLocation {
  kRegister,
  kStack,
  kStackSpill,
};

/// \brief Compiler scope information
struct CompilerScope {
  ScopeKind              fKind{ScopeKind::kScopeGlobal};
  CompilerKit::STLString fName{};
  CompilerKit::STLString fMangledPrefix{};
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
  bool                   fIsConstant{false};
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

/// @brief This function is for internal uses only, do not call it without a wrapper!
CK_IMPORT_C bool NectarCheckLine(CompilerKit::STLString& input);

/// \brief Global compiler context (replaces kState)
static CompilerContext kContext;

/////////////////////////////////////////////////////////////////////////////////////////

/// \brief Target architecture.
/// \note This shall never change.
static Int32 kMachine = CompilerKit::AssemblyFactory::kArchUnknown;

/////////////////////////////////////////

// ARGUMENT REGISTERS (PTX)

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

/// \brief PTX output support: track defined and external symbols
static std::set<CompilerKit::STLString> kDefinedSymbols;
static std::set<CompilerKit::STLString> kExternalSymbols;

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
static CompilerKit::STLString nectar_generate_function_header(
    const CompilerKit::STLString& mangled_name, const std::vector<CompilerKit::STLString>& args);
static CompilerKit::STLString nectar_generate_param_loads(
    const std::vector<CompilerKit::STLString>& args);
static Int32 nectar_allocate_stack_variable(const CompilerKit::STLString& var_name, Int32 size = 8,
                                            bool is_constant = false);

// Register allocation
static CompilerKit::STLString nectar_allocate_register(const CompilerKit::STLString& var_name);
static CompilerKit::STLString nectar_spill_lru_variable();
static VariableInfo*          nectar_find_variable(const CompilerKit::STLString& var_name);
static CompilerKit::STLString nectar_get_variable_ref(const CompilerKit::STLString& var_name,
                                                      bool                          lookup = false);

// Impl management
static void                   nectar_add_impl_member(const CompilerKit::STLString& class_name,
                                                     const CompilerKit::STLString& member_name, Int32 size);
static Int32                  nectar_get_impl_size(const CompilerKit::STLString& class_name);
static CompilerKit::STLString nectar_generate_constructor_call(
    const CompilerKit::STLString& class_name, const CompilerKit::STLString& obj_name);
static CompilerKit::STLString nectar_generate_destructor_call(
    const CompilerKit::STLString& class_name, const CompilerKit::STLString& obj_name);

// PTX calling convention
static void nectar_process_function_parameters(const std::vector<CompilerKit::STLString>& args);

/////////////////////////////////////////////////////////////////////////////////////////

/* \brief NECTAR compiler backend for the NeKernel NECTAR driver */
class CompilerFrontendNectarPTX final CK_COMPILER_FRONTEND {
 public:
  explicit CompilerFrontendNectarPTX()  = default;
  ~CompilerFrontendNectarPTX() override = default;

  NECTAR_COPY_DEFAULT(CompilerFrontendNectarPTX);

  /// \brief Parse Nectar symbols and syntax.
  CompilerKit::SyntaxLeafList::SyntaxLeaf Compile(CompilerKit::STLString&       text,
                                                  const CompilerKit::STLString& file) override;

  /// \brief Returns the language name.
  /// \return Language name.
  const char* Language() override;

 public:
  /// \brief Parse NECTAR namespaces and Impls.
  /// \param CompilerKit::SyntaxLeafList::SyntaxLeaf the leaf to build upon.
  CompilerKit::SyntaxLeafList::SyntaxLeaf CompileLayout(CompilerKit::STLString&       text,
                                                        const CompilerKit::STLString& file,
                                                        CompilerKit::SyntaxLeafList::SyntaxLeaf&);
};

/// @internal compiler variables

static CompilerFrontendNectarPTX* kFrontend = nullptr;

static constexpr const char* kPtxRetReg  = "%rd0";
static constexpr const char* kPtxTmpRegA = "%rd1";
static constexpr const char* kPtxTmpRegB = "%rd2";
static constexpr const char* kPtxThisReg = "%rd3";
static constexpr const char* kPtxPredReg = "%p1";

static std::vector<CompilerKit::STLString> kRegisterList = {
    "%rd16", "%rd17", "%rd18", "%rd19", "%rd20", "%rd21", "%rd22", "%rd23", "%rd24", "%rd25",
    "%rd26", "%rd27", "%rd28", "%rd29", "%rd30", "%rd31", "%rd32", "%rd33", "%rd34", "%rd35",
    "%rd36", "%rd37", "%rd38", "%rd39", "%rd40", "%rd41", "%rd42", "%rd43", "%rd44", "%rd45",
    "%rd46", "%rd47", "%rd48", "%rd49", "%rd50", "%rd51", "%rd52", "%rd53", "%rd54", "%rd55",
    "%rd56", "%rd57", "%rd58", "%rd59", "%rd60", "%rd61", "%rd62", "%rd63",
};

/// @brief NVPTX calling convention (params loaded into registers).
static std::vector<CompilerKit::STLString> kRegisterConventionCallList = {
    "%rd8", "%rd9", "%rd10", "%rd11", "%rd12", "%rd13", "%rd14", "%rd15",
};

static std::size_t            kFunctionEmbedLevel{};
static CompilerKit::STLString kCurrentIfSymbol{};
static CompilerKit::STLString kCurrentReturnAddress{};
static bool                   kCurrentIfCondition{false};

/// detail namespaces

const char* CompilerFrontendNectarPTX::Language() {
  return "Parallel Nectar (NVPTX)";
}

static std::uintptr_t                                                 kOrigin = 0;
static std::vector<std::pair<CompilerKit::STLString, std::uintptr_t>> kOriginMap;

/////////////////////////////////////////////////////////////////////////////////////////

/// @name Compile
/// @brief Generate assembly from a NECTAR source.

/////////////////////////////////////////////////////////////////////////////////////////

static auto nectar_get_impl_member(const CompilerKit::STLString& class_name,
                                   const CompilerKit::STLString& member_name) -> CompilerStructMap {
  // Find or create struct map entry
  for (auto& sm : kContext.fStructMapVector) {
    if (sm.fName == class_name) {
      return sm;
    }
  }

  return {};
}

CompilerKit::SyntaxLeafList::SyntaxLeaf CompilerFrontendNectarPTX::Compile(
    CompilerKit::STLString& text, const CompilerKit::STLString& file) {
  CompilerKit::SyntaxLeafList::SyntaxLeaf syntax_tree;
  CompilerKit::STLString                  syntax_rem_buffer;

  if (!NectarCheckLine(text)) return syntax_tree;

  std::size_t                                                     index{};
  std::vector<std::pair<CompilerKit::SyntaxKeyword, std::size_t>> keywords_list;

  for (auto& keyword : kKeywords) {
    if (text.find(keyword.fKeywordName) != std::string::npos) {
      switch (keyword.fKeywordKind) {
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
      case CompilerKit::KeywordKind::kKeywordKindImpl: {
        ++kOnClassScope;
        break;
      }
      case CompilerKit::KeywordKind::kKeywordKindIf: {
        kCurrentIfCondition = true;

        std::size_t keywordPos = text.find(keyword.first.fKeywordName);
        std::size_t openParen  = text.find("(");
        std::size_t closeParen = text.find("):");

        if (keywordPos == CompilerKit::STLString::npos ||
            openParen == CompilerKit::STLString::npos ||
            closeParen == CompilerKit::STLString::npos || closeParen <= openParen) {
          CompilerKit::Detail::print_error("Malformed if expression: " + text, file);
          break;
        }

        auto left = text.substr(openParen + 1, closeParen - openParen - 1);

        while (left.find(" ") != CompilerKit::STLString::npos) {
          left.erase(left.find(" "), 1);
        }

        std::vector<std::pair<CompilerKit::STLString, CompilerKit::STLString>> operators = {
            {"!==", "ne"}, {"===", "eq"}, {">", "lt"}, {"<", "gt"}, {">=", "lte"}, {"<=", "gte"},
        };

        for (auto& op : operators) {
          if (left.find(op.first) == CompilerKit::STLString::npos) continue;

          auto right = left.substr(left.find(op.first) + op.first.size());

          if (auto res = right.find(":"); res != CompilerKit::STLString::npos) right.erase(res);

          auto tmp = left.substr(0, left.find(op.first));

          while (tmp.find(" ") != CompilerKit::STLString::npos) tmp.erase(tmp.find(" "), 1);

          while (right.find(" ") != CompilerKit::STLString::npos) right.erase(right.find(" "), 1);

          if (auto var = nectar_find_variable(tmp); var) {
            syntax_tree.fUserValue +=
                "mov.u64 " + CompilerKit::STLString{kPtxTmpRegA} + ", " + var->fRegister + ";\n";
            delete var;
          } else {
            if (!isnumber(tmp[0])) {
              CompilerKit::Detail::print_warning("Variable not found, treating as symbol: " + tmp,
                                                 file);
            }

            syntax_tree.fUserValue +=
                "mov.u64 " + CompilerKit::STLString{kPtxTmpRegA} + ", " + tmp + ";\n";
          }

          if (auto var = nectar_find_variable(right); var) {
            syntax_tree.fUserValue +=
                "mov.u64 " + CompilerKit::STLString{kPtxTmpRegB} + ", " + var->fRegister + ";\n";
            delete var;
          }

          else {
            if (!isnumber(right[0])) {
              CompilerKit::Detail::print_warning("Variable not found, treating as symbol: " + right,
                                                 file);
            }

            syntax_tree.fUserValue +=
                "mov.u64 " + CompilerKit::STLString{kPtxTmpRegB} + ", " + right + ";\n";
          }

          syntax_tree.fUserValue += "setp." + op.second + ".s64 " +
                                    CompilerKit::STLString{kPtxPredReg} + ", " +
                                    CompilerKit::STLString{kPtxTmpRegA} + ", " +
                                    CompilerKit::STLString{kPtxTmpRegB} + ";\n";
          syntax_tree.fUserValue += "@" + CompilerKit::STLString{kPtxPredReg} + " bra __ret_" +
                                    std::to_string(kOrigin) + "_" + kCurrentIfSymbol + ";\n";

          kCurrentIfSymbol = std::to_string(kOrigin) + "_" + kCurrentIfSymbol;

          ++kOrigin;
        }

        break;
      }
      case CompilerKit::KeywordKind::kKeywordKindImplInit: {
        if (text.find(":= ") == CompilerKit::STLString::npos)
          CompilerKit::Detail::print_error("Invalid invokation of Init.", file);

        auto res = text.substr(text.find(":= ") + strlen(":= "));

        if (auto tmp = res.find("{}"); tmp) {
          if (tmp == CompilerKit::STLString::npos) {
            break;
          }

          res.erase(tmp);
        }

        syntax_tree.fUserValue += "call __NECTAR_M_" + res + ";\n";
        break;
      }
      case CompilerKit::KeywordKind::kKeywordKindFunctionStart: {
        for (auto& ch : text) {
          if (isnumber(ch)) {
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
        if (text.ends_with(");")) {
          // Handle function call/jump
          auto it = std::find_if(
              kOriginMap.begin(), kOriginMap.end(),
              [&symbol_name_fn](std::pair<CompilerKit::STLString, std::uintptr_t> pair) -> bool {
                return symbol_name_fn.find(pair.first) != CompilerKit::STLString::npos;
              });

          if (it != kOriginMap.end()) {
            syntax_tree.fUserValue += "call " + it->first + ";\n";
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

        while (mangled_name.find(" ") != CompilerKit::STLString::npos) {
          mangled_name.erase(mangled_name.find(" "), 1);
        }

        // Track defined symbol for PTX extern resolution
        kDefinedSymbols.insert(mangled_name);

        if (mangled_name.starts_with("__NECTAR") == false) {
          mangled_name = "_" + mangled_name;
        }

        syntax_tree.fUserValue += nectar_generate_function_header(mangled_name, args);
        syntax_tree.fUserValue += nectar_generate_prologue();

        // Initialize function-local state
        kContext.fVariables.clear();
        kContext.fStackOffset  = 0;
        kContext.fMaxStackUsed = 0;

        // Process function parameters
        syntax_tree.fUserValue += nectar_generate_param_loads(args);
        nectar_process_function_parameters(args);

        // Push function scope
        nectar_push_scope(ScopeKind::kScopeFunction, cleanFnName);

        ++kFunctionEmbedLevel;

        kOriginMap.push_back({mangled_name, kOrigin});
        ++kOrigin;

        break;
      }
      }
      case CompilerKit::KeywordKind::kKeywordKindFunctionEnd: {
        if (kOnClassScope) --kOnClassScope;

        if (text.find("{") != CompilerKit::STLString::npos) break;

        if (text.ends_with(";")) break;

        if (kFunctionEmbedLevel) {
          --kFunctionEmbedLevel;
        }

        // Pop function scope
        nectar_pop_scope();

        break;
      }
      case CompilerKit::KeywordKind::kKeywordKindBodyEnd: {
        if (kFunctionEmbedLevel) {
          --kFunctionEmbedLevel;
          nectar_pop_scope();
          syntax_tree.fUserValue += "}\n";
        }

        break;
      }
      case CompilerKit::KeywordKind::kKeywordKindDelete: {
        if (auto pos = syntax_tree.fUserValue.find(keyword.first.fKeywordName);
            pos != CompilerKit::STLString::npos) {
          syntax_tree.fUserValue.replace(pos, keyword.first.fKeywordName.size(),
                                         "__operator_delete");
        }
        continue;
      }
      case CompilerKit::KeywordKind::kKeywordKindNew: {
        if (auto pos = syntax_tree.fUserValue.find(keyword.first.fKeywordName);
            pos != CompilerKit::STLString::npos) {
          syntax_tree.fUserValue.replace(pos, keyword.first.fKeywordName.size(), "__operator_new");
        }

        continue;
      }
      case CompilerKit::KeywordKind::kKeywordKindAccess:
      case CompilerKit::KeywordKind::kKeywordKindFunctionAccess:
      case CompilerKit::KeywordKind::kKeywordKindAccessChecked: {
        if (text.find("return ") != CompilerKit::STLString::npos) {
          break;
        }

        if (text.find("if ") != CompilerKit::STLString::npos) {
          break;
        }

        if (text.find("const ") != CompilerKit::STLString::npos) {
          break;
        }

        if (text.find("let ") != CompilerKit::STLString::npos) {
          break;
        }

        if (text.find("):") != CompilerKit::STLString::npos) {
          break;
        }

        CompilerKit::STLString valueOfVar =
            text.substr(text.find(keyword.first.fKeywordName) + keyword.first.fKeywordName.size());

        CompilerKit::STLString args;

        if (valueOfVar.find("{") != CompilerKit::STLString::npos) {
          break;
        }

        if (CompilerKit::KeywordKind::kKeywordKindFunctionAccess == keyword.first.fKeywordKind)
          args = text.substr(text.find(keyword.first.fKeywordName));
        else
          args = valueOfVar.substr(valueOfVar.find("(") + 1);

        auto nameVar = text.substr(0, text.find(keyword.first.fKeywordName));

        while (nameVar.find(" ") != CompilerKit::STLString::npos) {
          nameVar.erase(nameVar.find(" "), 1);
        }

        while (nameVar.find("\t") != CompilerKit::STLString::npos) {
          nameVar.erase(nameVar.find("\t"), 1);
        }

        auto method = text.substr(0, text.find(keyword.first.fKeywordName));

        if (method.find("let ") != CompilerKit::STLString::npos) {
          method.erase(0, method.find("let ") + strlen("let "));
        } else if (method.find("const ") != CompilerKit::STLString::npos) {
          method.erase(0, method.find("const ") + strlen("const "));
        }

        if (method.find(":=") != CompilerKit::STLString::npos) {
          method.erase(0, method.find(":=") + strlen(":="));
        }

        while (method.find(" ") != CompilerKit::STLString::npos) {
          method.erase(method.find(" "), 1);
        }

        if (!nectar_get_variable_ref(nameVar).empty())
          syntax_tree.fUserValue += "mov.u64 " + CompilerKit::STLString{kPtxThisReg} + ", " +
                                    nectar_get_variable_ref(nameVar) + ";\n";

        if (CompilerKit::KeywordKind::kKeywordKindFunctionAccess != keyword.first.fKeywordKind)
          method = valueOfVar.erase(valueOfVar.find("("));

        valueOfVar += "\n";

        CompilerKit::STLString arg;
        auto                   index = 8;

        CompilerKit::STLString buf;

        for (auto& ch : args) {
          if (ch == ',' || ch == ')') {
            if (index <= 15) {
              auto val = nectar_get_variable_ref(arg);

              if (val.empty()) {
                val = arg;

                while (val.find(" ") != CompilerKit::STLString::npos) {
                  val.erase(val.find(" "), 1);
                }

                if (!isnumber(val[0])) {
                  val = "0x0";
                }
              }

              if (!arg.empty()) buf += "mov.u64 %rd" + std::to_string(index) + ", " + val + ";\n";

              arg.clear();
              ++index;
            }

            continue;
          }

          arg += ch;
        }

        if (!nectar_get_variable_ref(nameVar).empty()) {
          syntax_tree.fUserValue += buf;
          syntax_tree.fUserValue += "call.uni ";
          syntax_tree.fUserValue +=
              (keyword.first.fKeywordName.ends_with('>') ? nectar_get_variable_ref(nameVar)
                                                         : nectar_get_variable_ref(nameVar)) +
              method + ";\n";
        } else {
          auto res = buf;
          if (method.starts_with("__NECTAR") == false)
            res += "call _" + method + ";\n";
          else
            res += "call " + method + ";\n";

          res += syntax_rem_buffer;

          syntax_tree.fUserValue += res;
        }

        break;
      }
      case CompilerKit::KeywordKind::kKeywordKindEndLine:
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
          valueOfVar = text.substr(text.find(keyword.first.fKeywordName) +
                                   keyword.first.fKeywordName.size());
        } else if (keyword.first.fKeywordKind == CompilerKit::KeywordKind::kKeywordKindEndLine) {
          break;
        }

        if (valueOfVar.empty()) {
          CompilerKit::Detail::print_error("Undefined Right-Value for variable", file);
        }

        while (valueOfVar.find(";") != CompilerKit::STLString::npos &&
               keyword.first.fKeywordKind != CompilerKit::KeywordKind::kKeywordKindEndLine) {
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
          varName.erase(varName.find(keyword.first.fKeywordName));
        } else if (keyword.first.fKeywordKind == CompilerKit::KeywordKind::kKeywordKindEndLine) {
          varName.erase(varName.find(";"));
        }

        static bool typeFound = false;

        for (auto& keyword : kKeywords) {
          if (keyword.fKeywordKind == CompilerKit::KeywordKind::kKeywordKindVariable) {
            if (text.find(keyword.fKeywordName) != CompilerKit::STLString::npos) {
              if (text[text.find(keyword.fKeywordName)] == ' ') {
                typeFound = false;
                continue;
              }

              typeFound = true;
            }
          }
        }

        CompilerKit::STLString instr = "mov.u64 ";

        std::vector<CompilerKit::STLString> newVars;

        if (typeFound &&
            keyword.first.fKeywordKind != CompilerKit::KeywordKind::kKeywordKindVariableInc &&
            keyword.first.fKeywordKind != CompilerKit::KeywordKind::kKeywordKindVariableDec) {
          // Remove whitespace only (keep operators and quotes)
          while (!valueOfVar.empty() && (valueOfVar[0] == ' ' || valueOfVar[0] == '\t')) {
            valueOfVar.erase(0, 1);
          }
        }

        if (keyword.second > 0 && kKeywords[keyword.second - 1].fKeywordKind ==
                                      CompilerKit::KeywordKind::kKeywordKindVariable) {
          syntax_tree.fUserValue += "\n";
          continue;
        }

        if (keyword.first.fKeywordKind == CompilerKit::KeywordKind::kKeywordKindEndLine) {
          syntax_tree.fUserValue += "\n";
          continue;
        }

        if (keyword.first.fKeywordKind == CompilerKit::KeywordKind::kKeywordKindVariableInc) {
          instr = "add.u64 ";
        } else if (keyword.first.fKeywordKind ==
                   CompilerKit::KeywordKind::kKeywordKindVariableDec) {
          instr = "sub.u64 ";
        }

        CompilerKit::STLString varErrCpy = varName;

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

        auto pos = 0;

        if (varName.find("let ") != CompilerKit::STLString::npos) {
          pos     = varName.find("let ");
          varName = varName.substr(pos + std::string{"let "}.size());
        }

        while (varName.find(" ") != CompilerKit::STLString::npos) {
          varName.erase(varName.find(" "), 1);
        }

        while (varName.find("\t") != CompilerKit::STLString::npos) {
          varName.erase(varName.find("\t"), 1);
        }

        nectar_allocate_stack_variable(varName, 8,
                                       text.find("const ") != CompilerKit::STLString::npos);

        CompilerKit::STLString mangled;

        if (valueOfVar.find(".") != CompilerKit::STLString::npos) {
          valueOfVar.erase(0, valueOfVar.find(".") + strlen("."));

          mangled = "__NECTAR_SM_";
        }

        if (valueOfVar.find("->") != CompilerKit::STLString::npos) {
          valueOfVar.erase(0, valueOfVar.find("->") + strlen("->"));
          mangled = "__NECTAR_RM_";
        }

        if (valueOfVar.find(")") != CompilerKit::STLString::npos) {
          if (valueOfVar.find("(") != CompilerKit::STLString::npos)
            valueOfVar.erase(valueOfVar.find("("));

          if (!valueOfVar.empty()) {
            // Track as potential external symbol for PTX
            kExternalSymbols.insert(mangled + valueOfVar);

            syntax_tree.fUserValue += "call " + mangled + valueOfVar + ";\n";
            syntax_tree.fUserValue += instr + nectar_get_variable_ref(varName) + ", " +
                                      CompilerKit::STLString{kPtxRetReg} + ";\n";
          }

          break;
        }

        if (valueOfVar.ends_with("{}"))
          valueOfVar = CompilerKit::STLString{kPtxRetReg};  // impl init returns back to return reg.

        syntax_tree.fUserValue +=
            instr + nectar_get_variable_ref(varName) + ", " + valueOfVar + ";\n";

        break;
      }
      case CompilerKit::KeywordKind::kKeywordKindExport: {
        auto tmp = text;

        if (tmp.find(";") != CompilerKit::STLString::npos) tmp.erase(tmp.find(";"));

        while (tmp.find(" ") != CompilerKit::STLString::npos) {
          tmp.erase(tmp.find(" "), 1);
        }

        syntax_tree.fUserValue +=
            ".visible .func _" +
            tmp.substr(tmp.find(keyword.first.fKeywordName) + keyword.first.fKeywordName.size()) +
            ";\n";

        break;
      }
      case CompilerKit::KeywordKind::kKeywordKindImport: {
        auto tmp = text;

        if (tmp.find(";") != CompilerKit::STLString::npos) tmp.erase(tmp.find(";"));

        while (tmp.find(" ") != CompilerKit::STLString::npos) {
          tmp.erase(tmp.find(" "), 1);
        }

        syntax_tree.fUserValue +=
            ".extern .func _" +
            tmp.substr(tmp.find(keyword.first.fKeywordName) + keyword.first.fKeywordName.size()) +
            ";\n";

        break;
      }
      case CompilerKit::KeywordKind::kKeywordKindExtern: {
        auto tmp = text;

        if (tmp.find(";") != CompilerKit::STLString::npos) tmp.erase(tmp.find(";"));

        while (tmp.find(" ") != CompilerKit::STLString::npos) {
          tmp.erase(tmp.find(" "), 1);
        }

        syntax_tree.fUserValue +=
            ".extern .func _" +
            tmp.substr(tmp.find(keyword.first.fKeywordName) + keyword.first.fKeywordName.size()) +
            ";\n";

        break;
      }
      case CompilerKit::KeywordKind::kKeywordKindReturn: {
        try {
          auto pos = text.find("return");

          if (pos == CompilerKit::STLString::npos) {
            syntax_tree.fUserValue += nectar_generate_epilogue();
            syntax_tree.fUserValue += "ret;\n";
            ++kOrigin;
            break;
          }

          pos += std::string("return").size() + 1;

          CompilerKit::STLString subText = text.substr(pos);

          subText = subText.erase(subText.find(";"));

          // Extract and set up call arguments before erasing them
          if (subText.find("):") != CompilerKit::STLString::npos) {
            auto argStart = subText.find("(") + 1;
            auto argEnd   = subText.find("):");

            if (argEnd != CompilerKit::STLString::npos && argEnd > argStart) {
              auto argsStr = subText.substr(argStart, argEnd - argStart);
              auto regIdx  = 8;

              CompilerKit::STLString currentArg;
              for (std::size_t i = 0; i <= argsStr.size(); ++i) {
                if (i == argsStr.size() || argsStr[i] == ',') {
                  while (!currentArg.empty() && currentArg[0] == ' ') currentArg.erase(0, 1);
                  while (!currentArg.empty() && currentArg.back() == ' ') currentArg.pop_back();

                  if (!currentArg.empty() && regIdx <= 15) {
                    auto val = nectar_get_variable_ref(currentArg);
                    if (val.empty()) val = currentArg;

                    syntax_tree.fUserValue +=
                        "mov.u64 %rd" + std::to_string(regIdx) + ", " + val + ";\n";
                    ++regIdx;
                  }

                  currentArg.clear();
                } else {
                  currentArg += argsStr[i];
                }
              }
            }

            subText.erase(subText.find("("));
          }

          auto ref = nectar_get_variable_ref(subText);

          if (ref.empty() == false)
            syntax_tree.fUserValue +=
                "mov.u64 " + CompilerKit::STLString{kPtxRetReg} + ", " + ref + ";\n";

          if (subText.starts_with("'") || isnumber(subText[0]))
            syntax_tree.fUserValue +=
                "mov.u64 " + CompilerKit::STLString{kPtxRetReg} + ", " + subText + ";\n";
          else if (text.find("(") != CompilerKit::STLString::npos &&
                   text.find(");") != CompilerKit::STLString::npos) {
            // Track as potential external symbol for PTX.

            subText.erase(subText.find("("));

            for (const auto& keyword : kKeywords) {
              if (keyword.fKeywordName == subText)
                CompilerKit::Detail::print_error("A nectar keyword cannot be used there.", file);
            }

            kExternalSymbols.insert(subText);

            syntax_tree.fUserValue += "call " + subText + ";\n";
          }

          syntax_tree.fUserValue += nectar_generate_epilogue() + "ret;\n";
          ++kOrigin;
        } catch (...) {
          syntax_tree.fUserValue += nectar_generate_epilogue() + "ret;\n";
          ++kOrigin;
        }

        if (kCurrentIfCondition) {
          syntax_tree.fUserValue += "__ret_" + kCurrentIfSymbol + ":\n";

          kCurrentIfSymbol.clear();
          kCurrentIfCondition = false;
        }
      }
      default: {
        continue;
      }
    }
  }

  return this->CompileLayout(text, file, syntax_tree);
}

/// \brief Parse NECTAR Impls.
/// \param CompilerKit::SyntaxLeafList::SyntaxLeaf the leaf to build upon.
CompilerKit::SyntaxLeafList::SyntaxLeaf CompilerFrontendNectarPTX::CompileLayout(
    CompilerKit::STLString& text, const CompilerKit::STLString& file,
    CompilerKit::SyntaxLeafList::SyntaxLeaf& syntax_tree) {
  if ((text.find("impl") != CompilerKit::STLString::npos)) {
    CompilerKit::STLString keyword  = "impl";
    auto                   classPos = text.find(keyword) + keyword.length();
    auto                   bracePos = text.find("{");

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

    syntax_tree.fUserValue += "// HINT: " + className + "\n";
  }

  // Handle class exit
  if (text.find("};") != CompilerKit::STLString::npos) {
    --kOnClassScope;
    nectar_pop_scope();

    syntax_tree.fUserValue += "// HINT: END NAMESPACE\n";
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
      scope.fMangledPrefix += "N_" + s.fName;
    } else if (s.fKind == ScopeKind::kScopeClass) {
      scope.fMangledPrefix += "C_" + s.fName;
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
      mangled += "N_" + scope.fName;
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

  CompilerKit::STLString identifierCopy = identifier;

  if (auto pos = identifierCopy.find("let "); pos != CompilerKit::STLString::npos) {
    identifierCopy = identifierCopy.substr(pos + 3);
  } else if (auto pos = identifierCopy.find("const "); pos != CompilerKit::STLString::npos) {
    identifierCopy = identifierCopy.substr(pos + 5);
  }

  while (auto pos = identifierCopy.find(" ")) {
    if (pos == CompilerKit::STLString::npos) break;
    identifierCopy.erase(pos, 1);
  }

  if (inClass) {
    mangled += "M_" + identifierCopy;
  } else {
    return identifierCopy;
  }

  return mangled;
}

/// \brief Generate function prologue
static CompilerKit::STLString nectar_generate_prologue() {
  CompilerKit::STLString prologue;
  prologue += "  .reg .u64 %rd<64>;\n";
  prologue += "  .reg .pred %p<8>;\n";
  return prologue;
}

/// \brief Generate function epilogue
static CompilerKit::STLString nectar_generate_epilogue() {
  return "";
}

/// \brief Generate PTX function header
static CompilerKit::STLString nectar_generate_function_header(
    const CompilerKit::STLString& mangled_name, const std::vector<CompilerKit::STLString>& args) {
  CompilerKit::STLString header = ".visible .func " + mangled_name + "(";

  for (size_t i = 0; i < args.size(); ++i) {
    header += ".param .u64 __arg" + std::to_string(i);
    if (i + 1 < args.size()) header += ", ";
  }

  header += ")\n{\n";
  return header;
}

/// \brief Emit PTX parameter loads into registers
static CompilerKit::STLString nectar_generate_param_loads(
    const std::vector<CompilerKit::STLString>& args) {
  CompilerKit::STLString loads;

  for (size_t i = 0; i < args.size(); ++i) {
    CompilerKit::STLString reg;
    if (i < kRegisterConventionCallList.size()) {
      reg = kRegisterConventionCallList[i];
    } else {
      auto idx = i - kRegisterConventionCallList.size();
      if (idx < kRegisterList.size()) reg = kRegisterList[idx];
    }

    if (!reg.empty()) {
      loads += "  ld.param.u64 " + reg + ", [__arg" + std::to_string(i) + "];\n";
    }
  }

  return loads;
}

/// \brief Allocate a variable on the stack
static Int32 nectar_allocate_stack_variable(const CompilerKit::STLString& var_name, Int32 size,
                                            bool is_constant) {
  if (auto var = nectar_find_variable(var_name); var) {
    if (var->fIsConstant)
      CompilerKit::Detail::print_error(
          "Variable " + var_name.substr(var_name.find("const") + strlen("const")) + " is constant.",
          "CompilerKit");

    if (var->fStackOffset > 0)
      CompilerKit::Detail::print_error("Variable " + var_name + " is already defined.",
                                       "CompilerKit");

    delete var;
  }

  VariableInfo varInfo;
  varInfo.fName        = var_name;
  varInfo.fLocation    = VarLocation::kRegister;
  varInfo.fStackOffset = 0;
  varInfo.fSize        = size;
  varInfo.fLastUsed    = kContext.fInstructionCounter;
  varInfo.fIsConstant  = is_constant;
  varInfo.fRegister    = nectar_allocate_register(var_name);
  kContext.fVariables.push_back(varInfo);

  return 0;
}

/// \brief Find a variable by name
static VariableInfo* nectar_find_variable(const CompilerKit::STLString& var_name) {
  for (auto& var : kContext.fVariables) {
    if (var.fName == var_name) {
      return new VariableInfo(var);
    }
  }
  return nullptr;
}

/// \brief Get variable reference (register or stack location)
static CompilerKit::STLString nectar_get_variable_ref(const CompilerKit::STLString& var_name,
                                                      bool                          lookup) {
  auto* varInfo = nectar_find_variable(var_name);

  if (!varInfo || var_name.empty() || !isnumber(var_name[0])) {
    if (!isnumber(var_name[0]) && lookup)
      CompilerKit::Detail::print_error("Variable " + var_name + " not found.", "CompilerKit");
  }

  if (!varInfo) {
    return "";
  }

  if (varInfo->fIsConstant) {
    CompilerKit::Detail::print_error("Invalid use of constant " +
                                         var_name.substr(var_name.find("const") + strlen("const")) +
                                         " as variable.",
                                     "CompilerKit");
    return "0";
  }

  varInfo->fLastUsed = kContext.fInstructionCounter;

  if (varInfo->fLocation == VarLocation::kRegister) {
    auto reg = varInfo->fRegister;
    delete varInfo;
    return reg;
  }

  // PTX backend keeps locals in registers; no stack references.
  auto reg = varInfo->fRegister;
  delete varInfo;
  return reg;

  return "";
}

/// \brief Allocate a register for a variable
static CompilerKit::STLString nectar_allocate_register(const CompilerKit::STLString& var_name) {
  // Check if variable already has a register
  VariableInfo* existing = nullptr;

  for (auto& var : kContext.fVariables) {
    if (var.fName == var_name) {
      existing = &var;
      break;
    }
  }

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
        if (existing->fIsConstant) {
          CompilerKit::Detail::print_error("Invalid use of constant " + var_name + " as variable.",
                                           "CompilerKit");
          return "0";
        }

        existing->fLocation = VarLocation::kRegister;
        existing->fRegister = reg;
        existing->fLastUsed = kContext.fInstructionCounter;
      } else {
        VariableInfo varInfo;
        varInfo.fName       = var_name;
        varInfo.fLocation   = VarLocation::kRegister;
        varInfo.fRegister   = reg;
        varInfo.fLastUsed   = kContext.fInstructionCounter;
        varInfo.fIsConstant = false;

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
  return "";
}

/// \brief Add a class member to the struct map
static void nectar_add_impl_member(const CompilerKit::STLString& class_name,
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
static Int32 nectar_get_impl_size(const CompilerKit::STLString& class_name) {
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
  auto size   = nectar_get_impl_size(class_name);
  auto offset = nectar_allocate_stack_variable(
      obj_name, size == 0 ? 8 : size, obj_name.find("_const_") != CompilerKit::STLString::npos);

  nectar_push_scope(ScopeKind::kScopeClass, class_name);
  auto ctor_mangled = nectar_mangle_name(class_name);
  nectar_pop_scope();

  CompilerKit::STLString code;
  auto                   objReg = nectar_allocate_register(obj_name);
  if (!objReg.empty()) {
    code += "mov.u64 " + CompilerKit::STLString{kPtxThisReg} + ", " + objReg + ";\n";
  }
  code += "call " + ctor_mangled + ";\n";
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
  code += "mov.u64 " + CompilerKit::STLString{kPtxThisReg} + ", " + varInfo->fRegister + ";\n";

  delete varInfo;

  code += "call " + dtor_mangled + ";\n";
  return code;
}

/// \brief Process function parameters per PTX calling convention.
/// \note Assumes args are already extracted.
static void nectar_process_function_parameters(const std::vector<CompilerKit::STLString>& args) {
  for (size_t i = 0; i < args.size(); ++i) {
    VariableInfo param;
    param.fName     = "arg" + std::to_string(i);
    param.fLocation = VarLocation::kRegister;
    if (i < kRegisterConventionCallList.size()) {
      param.fRegister = kRegisterConventionCallList[i];
    } else {
      auto idx = i - kRegisterConventionCallList.size();
      if (idx < kRegisterList.size()) {
        param.fRegister = kRegisterList[idx];
      }
    }
    param.fIsParameter = true;
    param.fTypeName    = args[i];
    param.fLastUsed    = kContext.fInstructionCounter;
    kContext.fVariables.push_back(param);
  }
}

/////////////////////////////////////////////////////////////////////////////////////////

/**
 * @brief NECTAR assembler class.
 */

/////////////////////////////////////////////////////////////////////////////////////////

#define kExtListCxx {".nc", ".pp.nc"}

class AssemblyNectarInterfacePTX final CK_ASSEMBLY_INTERFACE {
 public:
  explicit AssemblyNectarInterfacePTX()  = default;
  ~AssemblyNectarInterfacePTX() override = default;

  NECTAR_COPY_DEFAULT(AssemblyNectarInterfacePTX);

  UInt32 Arch() noexcept override { return CompilerKit::AssemblyFactory::kArchUnknown; }

  Int32 CompileToFormat(CompilerKit::STLString src, Int32 arch) override {
    if (kFrontend == nullptr) return EXIT_FAILURE;

    CompilerKit::STLString              dest = src;
    std::vector<CompilerKit::STLString> ext  = kExtListCxx;

    dest.erase(dest.find(ext[0]));

    dest += ".ptx";

    std::ofstream out_fp(dest);
    std::ifstream src_fp = std::ifstream(src);

    CompilerKit::STLString line_source;

    // Clear symbol tracking sets for this compilation unit
    kDefinedSymbols.clear();
    kExternalSymbols.clear();

    // First pass: compile all lines and collect symbols
    CompilerKit::STLString compiledCode;
    std::size_t            lastRes{};
    std::string            prevRes;
    std::string            nextRes;

    while (std::getline(src_fp, line_source)) {
      auto res = kFrontend->Compile(line_source, src);
      if (kAcceptableErrors > 0) return EXIT_FAILURE;

      if (res.fPlaceType == CompilerKit::SyntaxLeafList::SyntaxLeaf::kPlaceBefore) {
        compiledCode.insert(compiledCode.find(prevRes), res.fUserValue, 0, res.fUserValue.size());
      } else if (res.fPlaceType == CompilerKit::SyntaxLeafList::SyntaxLeaf::kPlaceAfter) {
        nextRes = res.fUserValue;
        continue;
      } else {
        compiledCode += res.fUserValue;
        if (!nextRes.empty()) {
          compiledCode += nextRes;
          nextRes.clear();
        }
      }

      lastRes = res.fUserValue.size();
      prevRes = res.fUserValue;
    }

    // Output header
    out_fp << ".version 7.0\n";
    out_fp << ".target sm_50\n";
    out_fp << ".address_size 64\n\n";

    // Emit extern declarations for undefined symbols
    for (const auto& sym : kExternalSymbols) {
      if (kDefinedSymbols.find(sym) == kDefinedSymbols.end() && !sym.empty()) {
        out_fp << ".extern .func " << sym << ";\n";
      }
    }
    if (!kExternalSymbols.empty()) {
      out_fp << "\n";
    }

    // Output compiled code
    out_fp << compiledCode;

    return EXIT_SUCCESS;
  }
};

/////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////

NECTAR_MODULE(CompilerNectarPTX) {
  bool skip = false;

  kKeywords.emplace_back("impl", CompilerKit::KeywordKind::kKeywordKindImpl);
  kKeywords.emplace_back("trait", CompilerKit::KeywordKind::kKeywordKindTrait);
  kKeywords.emplace_back("{", CompilerKit::KeywordKind::kKeywordKindBodyStart);
  kKeywords.emplace_back("}", CompilerKit::KeywordKind::kKeywordKindBodyEnd);
  kKeywords.emplace_back("{}", CompilerKit::KeywordKind::kKeywordKindImplInit);
  kKeywords.emplace_back("(", CompilerKit::KeywordKind::kKeywordKindFunctionStart);
  kKeywords.emplace_back(")", CompilerKit::KeywordKind::kKeywordKindFunctionEnd);
  kKeywords.emplace_back(":=", CompilerKit::KeywordKind::kKeywordKindVariableAssign);
  kKeywords.emplace_back("+=", CompilerKit::KeywordKind::kKeywordKindVariableInc);
  kKeywords.emplace_back("-=", CompilerKit::KeywordKind::kKeywordKindVariableDec);
  kKeywords.emplace_back("const", CompilerKit::KeywordKind::kKeywordKindVariable);
  kKeywords.emplace_back("let", CompilerKit::KeywordKind::kKeywordKindVariable);
  kKeywords.emplace_back("new", CompilerKit::KeywordKind::kKeywordKindNew);
  kKeywords.emplace_back("delete", CompilerKit::KeywordKind::kKeywordKindDelete);
  kKeywords.emplace_back(".", CompilerKit::KeywordKind::kKeywordKindAccess);
  kKeywords.emplace_back("->", CompilerKit::KeywordKind::kKeywordKindAccessChecked);
  kKeywords.emplace_back("(", CompilerKit::KeywordKind::kKeywordKindFunctionAccess);
  kKeywords.emplace_back(";", CompilerKit::KeywordKind::kKeywordKindEndLine);
  kKeywords.emplace_back(":==", CompilerKit::KeywordKind::kKeywordKindVariableEquals);
  kKeywords.emplace_back("return", CompilerKit::KeywordKind::kKeywordKindReturn);
  kKeywords.emplace_back("extern", CompilerKit::KeywordKind::kKeywordKindExtern);
  kKeywords.emplace_back("import", CompilerKit::KeywordKind::kKeywordKindImport);
  kKeywords.emplace_back("export", CompilerKit::KeywordKind::kKeywordKindExport);

  kKeywords.emplace_back("if", CompilerKit::KeywordKind::kKeywordKindIf);

  kErrorLimit = 0;

  kFrontend = new CompilerFrontendNectarPTX();

  CompilerKit::StrongRef<AssemblyNectarInterfacePTX> mntPnt{new AssemblyNectarInterfacePTX()};
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

      if (strcmp(argv[index], "-fverbose") == 0) {
        kVerbose = true;
        continue;
      }

      if (strcmp(argv[index], "-fprint-dialect") == 0) {
        if (kFrontend) std::cout << kFrontend->Language() << "\n";

        return NECTAR_SUCCESS;
      }

      CompilerKit::STLString err = "Unknown option: ";
      err += argv[index];

      CompilerKit::Detail::print_error(err, "frontend");

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
