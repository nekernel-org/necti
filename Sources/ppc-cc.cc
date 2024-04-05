/*
 *	========================================================
 *
 *	cc
 * 	Copyright Mahrouss Logic, all rights reserved.
 *
 * 	========================================================
 */

/// BUGS: ?
/// TODO: 

#include <Headers/AsmKit/Arch/powerpc.hpp>
#include <Headers/ParserKit.hpp>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>
#include <utility>
#include <vector>
#include <memory>
#include <filesystem>

#include <random>
#include <Headers/UUID.hpp>

#define kOk 0

/// TODO: support structures, else if, else, ., ->

/* C driver */
/* This is part of MultiProcessor C SDK. */
/* (c) Mahrouss Logic */

/// @author Amlal El Mahrouss (amlel)
/// @file cc.cc
/// @brief PowerPC C Compiler.

/////////////////////

// ANSI ESCAPE CODES

/////////////////////

#define kBlank "\e[0;30m"
#define kRed "\e[0;31m"
#define kWhite "\e[0;97m"

/////////////////////////////////////

// INTERNAL STUFF OF THE C COMPILER

/////////////////////////////////////

namespace detail {
// \brief name to register struct.
struct CompilerRegisterMap final {
  std::string fName;
  std::string fReg;
};

// \brief Map for C structs
// \author amlel
struct CompilerStructMap final {
  // 'my_foo'
  std::string fName;

  // if instance: stores a valid register.
  std::string fReg;

  // offset count
  std::size_t fOffsetsCnt;

  // offset array.
  std::vector<std::pair<Int32, std::string>> fOffsets;
};

struct CompilerState final {
  std::vector<ParserKit::SyntaxLeafList> fSyntaxTreeList;
  std::vector<CompilerRegisterMap> kStackFrame;
  std::vector<CompilerStructMap> kStructMap;
  ParserKit::SyntaxLeafList *fSyntaxTree{nullptr};
  std::unique_ptr<std::ofstream> fOutputAssembly;
  std::string fLastFile;
  std::string fLastError;
  bool fVerbose;
};
}  // namespace detail

static detail::CompilerState kState;
static SizeType kErrorLimit = 100;
static std::string kIfFunction = "";
static Int32 kAcceptableErrors = 0;

namespace detail {
void print_error(std::string reason, std::string file) noexcept {
  if (reason[0] == '\n') reason.erase(0, 1);

  if (file.find(".pp") != std::string::npos) {
    file.erase(file.find(".pp"), 3);
  }

  if (kState.fLastFile != file) {
    std::cout << kRed << "[ cc ] " << kWhite
              << ((file == "cc") ? "internal compiler error "
                                   : ("in file, " + file))
              << kBlank << std::endl;
    std::cout << kRed << "[ cc ] " << kWhite << reason << kBlank << std::endl;

    kState.fLastFile = file;
  } else {
    std::cout << kRed << "[ cc ] [ " << kState.fLastFile << " ] " << kWhite
              << reason << kBlank << std::endl;
  }

  if (kAcceptableErrors > kErrorLimit) std::exit(3);

  ++kAcceptableErrors;
}

struct CompilerType final {
  std::string fName;
  std::string fValue;
};
}  // namespace detail

/////////////////////////////////////////////////////////////////////////////////////////

// Target architecture.
static int kMachine = 0;

/////////////////////////////////////////

// REGISTERS ACCORDING TO USED ASSEMBLER

/////////////////////////////////////////

static size_t kRegisterCnt = kAsmRegisterLimit;
static size_t kStartUsable = 2;
static size_t kUsableLimit = 15;
static size_t kRegisterCounter = kStartUsable;
static std::string kRegisterPrefix = kAsmRegisterPrefix;

/////////////////////////////////////////

// COMPILER PARSING UTILITIES/STATES.

/////////////////////////////////////////

static std::vector<std::string> kFileList;
static CompilerKit::AssemblyFactory kFactory;
static bool kInStruct = false;
static bool kOnWhileLoop = false;
static bool kOnForLoop = false;
static bool kInBraces = false;
static bool kIfFound = false;
static size_t kBracesCount = 0UL;

/* @brief C compiler backend for C */
class CompilerBackendCLang final : public ParserKit::CompilerBackend {
 public:
  explicit CompilerBackendCLang() = default;
  ~CompilerBackendCLang() override = default;

  MPCC_COPY_DEFAULT(CompilerBackendCLang);

  std::string Check(const char *text, const char *file);
  bool Compile(const std::string &text, const char *file) override;

  const char *Language() override { return "PowerPC C"; }
};

static CompilerBackendCLang *kCompilerBackend = nullptr;
static std::vector<detail::CompilerType> kCompilerVariables;
static std::vector<std::string> kCompilerFunctions;
static std::vector<detail::CompilerType> kCompilerTypes;

namespace detail {
union number_cast final {
 public:
  number_cast(UInt64 _Raw) : _Raw(_Raw) {}

 public:
  char _Num[8];
  UInt64 _Raw;
};

union double_cast final {
 public:
  double_cast(float _Raw) : _Raw(_Raw) {}

 public:
  char _Sign;
  char _Lh[8];
  char _Rh[23];

  float _Raw;
};
}  // namespace detail

/////////////////////////////////////////////////////////////////////////////////////////

// @name Compile
// @brief Generate MASM from a C assignement.

/////////////////////////////////////////////////////////////////////////////////////////

bool CompilerBackendCLang::Compile(const std::string &text, const char *file) {
  std::string textBuffer = text;

  bool typeFound = false;
  bool fnFound = false;

  // setup generator.
  std::random_device rd;

  auto seed_data = std::array<int, std::mt19937::state_size> {};
  std::generate(std::begin(seed_data), std::end(seed_data), std::ref(rd));
  std::seed_seq seq(std::begin(seed_data), std::end(seed_data));
  std::mt19937 generator(seq);

  // start parsing
  for (size_t text_index = 0; text_index < textBuffer.size(); ++text_index) {
    auto syntaxLeaf = ParserKit::SyntaxLeafList::SyntaxLeaf();

    auto gen = uuids::uuid_random_generator{generator};
    uuids::uuid out = gen();

    detail::number_cast time_off = (UInt64)out.as_bytes().data();

    if (!typeFound) {
      auto substr = textBuffer.substr(text_index);
      std::string match_type;

      for (size_t y = 0; y < substr.size(); ++y) {
        if (substr[y] == ' ') {
          while (match_type.find(' ') != std::string::npos) {
            match_type.erase(match_type.find(' '));
          }

          for (auto &clType : kCompilerTypes) {
            if (clType.fName == match_type) {
              match_type.clear();

              std::string buf;

              buf += clType.fValue;
              buf += ' ';

              if (substr.find('=') != std::string::npos) {
                break;
              }

              if (textBuffer.find('(') != std::string::npos) {
                syntaxLeaf.fUserValue = buf;

                kState.fSyntaxTree->fLeafList.push_back(syntaxLeaf);
              }

              typeFound = true;
              break;
            }
          }

          break;
        }

        match_type += substr[y];
      }
    }

    if (textBuffer[text_index] == '{') {
      if (kInStruct) {
        continue;
      }

      kInBraces = true;
      ++kBracesCount;

      kState.fSyntaxTree->fLeafList.push_back(syntaxLeaf);
    }

    // return keyword handler
    if (textBuffer[text_index] == 'r') {
      std::string return_keyword;
      return_keyword += "return";

      std::size_t index = 0UL;

      std::string value;

      for (size_t return_index = text_index; return_index < textBuffer.size();
           ++return_index) {
        if (textBuffer[return_index] != return_keyword[index]) {
          for (size_t value_index = return_index; value_index < textBuffer.size();
               ++value_index) {
            if (textBuffer[value_index] == ';') break;

            value += textBuffer[value_index];
          }

          break;
        }

        ++index;
      }

      if (index == return_keyword.size()) {
        if (!value.empty()) {
          if (value.find('(') != std::string::npos) {
            value.erase(value.find('('));
          }

          if (!isdigit(value[value.find('(') + 2])) {
            std::string tmp = value;
            bool reg_to_reg = false;

            value.clear();

            value += " import";
            value += tmp;
          }

          syntaxLeaf.fUserValue = "\tldw r19, ";

          // make it pretty.
          if (value.find('\t') != std::string::npos)
            value.erase(value.find('\t'), 1);

          syntaxLeaf.fUserValue += value + "\n";
        }

        syntaxLeaf.fUserValue += "\tjlr";

        kState.fSyntaxTree->fLeafList.push_back(syntaxLeaf);

        break;
      }
    }

    if (textBuffer[text_index] == 'i' && textBuffer[text_index+1] == 'f') {
      auto expr = textBuffer.substr(text_index + 2);
      textBuffer.erase(text_index, 2);

      if (expr.find("{") != std::string::npos) {
        expr.erase(expr.find("{"));
      }

      if (expr.find("(") != std::string::npos)
        expr.erase(expr.find("("));
      
      if (expr.find(")") != std::string::npos)
        expr.erase(expr.find(")"));
        
      kIfFunction = "__MPCC_IF_PROC_";
      kIfFunction += std::to_string(time_off._Raw);

      syntaxLeaf.fUserValue = "\tlda r12, import ";
      syntaxLeaf.fUserValue += kIfFunction + "\n\t#r12 = Code to jump on, r11 right cond, r10 left cond.\n\tbeq r10, r11, r12\ndword export .code64 " + kIfFunction + "\n";
      kState.fSyntaxTree->fLeafList.push_back(syntaxLeaf);

      kIfFound = true;
    }

    // Parse expressions and instructions here.
    // what does this mean?
    // we encounter an assignment, or we reached the end of an expression.
    if (textBuffer[text_index] == '=' || textBuffer[text_index] == ';') {
      if (fnFound) continue;
      if (kIfFound) continue;

      if (textBuffer[text_index] == ';' && kInStruct) continue;

      if (textBuffer.find("typedef ") != std::string::npos) continue;

      if (textBuffer[text_index] == '=' && kInStruct) {
        detail::print_error("assignement of value in struct " + textBuffer, file);
        continue;
      }

      if (textBuffer[text_index] == ';' && kInStruct) {
        bool space_found_ = false;
        std::string sym;

        for (auto &ch : textBuffer) {
          if (ch == ' ') {
            space_found_ = true;
          }

          if (ch == ';') break;

          if (space_found_) sym.push_back(ch);
        }

        kState.kStructMap[kState.kStructMap.size() - 1].fOffsets.push_back(
            std::make_pair(
                kState.kStructMap[kState.kStructMap.size() - 1].fOffsetsCnt + 4,
                sym));

        kState.kStructMap[kState.kStructMap.size() - 1].fOffsetsCnt =
            kState.kStructMap[kState.kStructMap.size() - 1].fOffsetsCnt + 4;

        continue;
      }

      if (textBuffer[text_index] == '=' && kInStruct) {
        continue;
      }

      if (textBuffer[text_index + 1] == '=' || textBuffer[text_index - 1] == '!' ||
          textBuffer[text_index - 1] == '<' || textBuffer[text_index - 1] == '>') {
        continue;
      }

      std::string substr;

      if (textBuffer.find('=') != std::string::npos && kInBraces &&
        !kIfFound) {
        if (textBuffer.find("*") != std::string::npos) {
          if (textBuffer.find("=") > textBuffer.find("*"))
            substr += "\tlda ";
          else
            substr += "\tldw ";
        } else {
          substr += "\tldw ";
        }
      } else if (textBuffer.find('=') != std::string::npos && !kInBraces) {
        substr += "stw export .data64 ";
      }

      int first_encountered = 0;

      std::string str_name;

      for (size_t text_index_2 = 0; text_index_2 < textBuffer.size();
           ++text_index_2) {
        if (textBuffer[text_index_2] == '\"') {
          ++text_index_2;

          // want to add this, so that the parser recognizes that this is a
          // string.
          substr += '"';

          for (; text_index_2 < textBuffer.size(); ++text_index_2) {
            if (textBuffer[text_index_2] == '\"') break;

            substr += textBuffer[text_index_2];
          }
        }

        if (textBuffer[text_index_2] == '{' || textBuffer[text_index_2] == '}') continue;

        if (textBuffer[text_index_2] == ';') {
          break;
        }

        if (textBuffer[text_index_2] == ' ' || textBuffer[text_index_2] == '\t') {
          if (first_encountered != 2) {
            if (textBuffer[text_index] != '=' &&
                substr.find("export .data64") == std::string::npos && !kInStruct)
              substr += "export .data64 ";
          }

          ++first_encountered;

          continue;
        }

        if (textBuffer[text_index_2] == '=') {
          if (!kInBraces) {
            substr.replace(substr.find("export .data64"), strlen("export .data64"),
                           "export .page_zero ");
          }

          substr += ",";
          continue;
        }

        substr += textBuffer[text_index_2];
      }

      for (auto &clType : kCompilerTypes) {
        if (substr.find(clType.fName) != std::string::npos) {
          if (substr.find(clType.fName) > substr.find('"')) continue;

          substr.erase(substr.find(clType.fName), clType.fName.size());
        } else if (substr.find(clType.fValue) != std::string::npos) {
          if (substr.find(clType.fValue) > substr.find('"')) continue;

          if (clType.fName == "const") continue;

          substr.erase(substr.find(clType.fValue), clType.fValue.size());
        }
      }

      if (substr.find("extern") != std::string::npos) {
        substr.replace(substr.find("extern"), strlen("extern"), "import ");

        if (substr.find("export .data64") != std::string::npos)
          substr.erase(substr.find("export .data64"), strlen("export .data64"));
      }

      auto var_to_find =
          std::find_if(kCompilerVariables.cbegin(), kCompilerVariables.cend(),
                       [&](detail::CompilerType type) {
                         return type.fName.find(substr) != std::string::npos;
                       });

      if (kRegisterCounter == 5 || kRegisterCounter == 6) ++kRegisterCounter;

      std::string reg = kAsmRegisterPrefix;
      reg += std::to_string(kRegisterCounter);

      if (var_to_find == kCompilerVariables.cend()) {
        ++kRegisterCounter;

        kState.kStackFrame.push_back({.fName = substr, .fReg = reg});
        kCompilerVariables.push_back({.fName = substr});
      }

      syntaxLeaf.fUserValue += substr;
      kState.fSyntaxTree->fLeafList.push_back(syntaxLeaf);

      if (textBuffer[text_index] == '=') break;
    }

    // function handler.

    if (textBuffer[text_index] == '(' && !fnFound && !kIfFound) {
      std::string substr;
      std::string args_buffer;
      std::string args;

      bool type_crossed = false;

      for (size_t idx = textBuffer.find('(') + 1; idx < textBuffer.size(); ++idx) {
        if (textBuffer[idx] == ',') continue;

        if (textBuffer[idx] == ' ') continue;

        if (textBuffer[idx] == ')') break;
      }

      for (char substr_first_index : textBuffer) {
        if (substr_first_index != ',')
          args_buffer += substr_first_index;
        else
          args_buffer += '$';

        if (substr_first_index == ';') {
          args_buffer = args_buffer.erase(0, args_buffer.find('('));
          args_buffer = args_buffer.erase(args_buffer.find(';'), 1);
          args_buffer = args_buffer.erase(args_buffer.find(')'), 1);
          args_buffer = args_buffer.erase(args_buffer.find('('), 1);

          if (!args_buffer.empty()) args += "\tldw r6, ";

          std::string register_type;
          std::size_t index = 7UL;

          while (args_buffer.find("$") != std::string::npos) {
            register_type = kRegisterPrefix;
            register_type += std::to_string(index);

            ++index;

            args_buffer.replace(args_buffer.find('$'), 1,
                                "\n\tldw " + register_type + ",");
          }

          args += args_buffer;
          args += "\n\tlda r19, ";
        }
      }

      for (char _text_i : textBuffer) {
        if (_text_i == '\t' || _text_i == ' ') {
          if (!type_crossed) {
            substr.clear();
            type_crossed = true;
          }

          continue;
        }

        if (_text_i == '(') break;

        substr += _text_i;
      }

      if (kInBraces) {
        syntaxLeaf.fUserValue = args;
        syntaxLeaf.fUserValue += substr;
        syntaxLeaf.fUserValue += "\n\tjrl\n";

        kState.fSyntaxTree->fLeafList.push_back(syntaxLeaf);

        fnFound = true;
      } else {
        syntaxLeaf.fUserValue.clear();

        syntaxLeaf.fUserValue += "export .code64 ";

        syntaxLeaf.fUserValue += substr;
        syntaxLeaf.fUserValue += "\n";

        kState.fSyntaxTree->fLeafList.push_back(syntaxLeaf);

        fnFound = true;
      }

      kCompilerFunctions.push_back(textBuffer);
    }

    if (textBuffer[text_index] == '-' && textBuffer[text_index + 1] == '-') {
      textBuffer = textBuffer.replace(textBuffer.find("--"), strlen("--"), "");

      for (int _text_i = 0; _text_i < textBuffer.size(); ++_text_i) {
        if (textBuffer[_text_i] == '\t' || textBuffer[_text_i] == ' ')
          textBuffer.erase(_text_i, 1);
      }

      syntaxLeaf.fUserValue += "dec ";
      syntaxLeaf.fUserValue += textBuffer;

      kState.fSyntaxTree->fLeafList.push_back(syntaxLeaf);
      break;
    }

    if (textBuffer[text_index] == '}') {
      kRegisterCounter = kStartUsable;

      --kBracesCount;

      if (kBracesCount < 1) {
        kInBraces = false;
        kBracesCount = 0;
      }

      if (kIfFound)
        kIfFound = false;

      if (kInStruct) kInStruct = false;

      kState.fSyntaxTree->fLeafList.push_back(syntaxLeaf);
    }

    syntaxLeaf.fUserValue.clear();
  }

  auto syntaxLeaf = ParserKit::SyntaxLeafList::SyntaxLeaf();
  syntaxLeaf.fUserValue = "\n";
  kState.fSyntaxTree->fLeafList.push_back(syntaxLeaf);

  return true;
}

static bool kShouldHaveBraces = false;
static std::string kFnName;

std::string CompilerBackendCLang::Check(const char *text, const char *file) {
  std::string err_str;
  std::string ln = text;

  if (ln.empty()) {
    return err_str;
  }

  bool non_ascii_found = false;

  for (int i = 0; i < ln.size(); ++i) {
    if (isalnum(ln[i])) {
      non_ascii_found = true;
      break;
    }
  }

  if (kShouldHaveBraces && ln.find('{') != std::string::npos) {
    kShouldHaveBraces = false;
  }

  if (!non_ascii_found) return err_str;

  size_t string_index = 1UL;

  if (ln.find('\'') != std::string::npos) {
    string_index = ln.find('\'') + 1;

    for (; string_index < ln.size(); ++string_index) {
      if (ln[string_index] == '\'') {
        if (ln[string_index + 1] != ';') {
          ln.erase(string_index, 1);
        }

        return err_str;
      }
    }
  } else if (ln.find('"') != std::string::npos) {
    string_index = ln.find('"') + 1;

    for (; string_index < ln.size(); ++string_index) {
      if (ln[string_index] == '"') {
        if (ln[string_index + 1] != ';') {
          ln.erase(string_index, 1);
        } else {
          break;
        }
      }
    }
  } else if (ln.find('"') == std::string::npos &&
             ln.find('\'') == std::string::npos) {
    std::vector<std::string> forbidden_words;

    forbidden_words.push_back("\\");
    forbidden_words.push_back("?");
    forbidden_words.push_back("@");
    forbidden_words.push_back("~");
    forbidden_words.push_back("::");
    forbidden_words.push_back("/*");
    forbidden_words.push_back("*/");

    // add them to avoid stupid mistakes.
    forbidden_words.push_back("namespace");
    forbidden_words.push_back("class");
    forbidden_words.push_back("extern \"C\"");

    for (auto &forbidden : forbidden_words) {
      if (ln.find(forbidden) != std::string::npos) {
        err_str += "\nForbidden character detected: ";
        err_str += forbidden;

        return err_str;
      }
    }
  }

  struct CompilerVariableRange final {
    std::string fBegin;
    std::string fEnd;
  };

  const std::vector<CompilerVariableRange> variables_list = {
      {.fBegin = "static ", .fEnd = "="}, {.fBegin = "=", .fEnd = ";"},
      {.fBegin = "if(", .fEnd = "="},     {.fBegin = "if (", .fEnd = "="},
      {.fBegin = "if(", .fEnd = "<"},     {.fBegin = "if (", .fEnd = "<"},
      {.fBegin = "if(", .fEnd = ">"},     {.fBegin = "if (", .fEnd = ">"},
      {.fBegin = "if(", .fEnd = ")"},     {.fBegin = "if (", .fEnd = ")"},

      {.fBegin = "else(", .fEnd = "="},   {.fBegin = "else (", .fEnd = "="},
      {.fBegin = "else(", .fEnd = "<"},   {.fBegin = "else (", .fEnd = "<"},
      {.fBegin = "else(", .fEnd = ">"},   {.fBegin = "else (", .fEnd = ">"},
      {.fBegin = "else(", .fEnd = ")"},   {.fBegin = "else (", .fEnd = ")"},
  };

  for (auto &variable : variables_list) {
    if (ln.find(variable.fBegin) != std::string::npos) {
      string_index = ln.find(variable.fBegin) + variable.fBegin.size();

      while (ln[string_index] == ' ') ++string_index;

      std::string keyword;

      for (; string_index < ln.size(); ++string_index) {
        if (ln[string_index] == variable.fEnd[0]) {
          std::string varname = "";

          for (size_t index_keyword = ln.find(' ');
               ln[index_keyword] != variable.fBegin[0]; ++index_keyword) {
            if (ln[index_keyword] == ' ') {
              continue;
            }

            if (isdigit(ln[index_keyword])) {
              goto cc_next_loop;
            }

            varname += ln[index_keyword];
          }

          if (varname.find(' ') != std::string::npos) {
            varname.erase(0, varname.find(' '));

            if (variable.fBegin == "extern") {
              varname.erase(0, varname.find(' '));
            }
          }

          if (kRegisterCounter == 5 || kRegisterCounter == 6)
            ++kRegisterCounter;

          std::string reg = kAsmRegisterPrefix;
          reg += std::to_string(kRegisterCounter);

          kCompilerVariables.push_back({.fValue = varname});
          goto cc_check_done;
        }

        keyword.push_back(ln[string_index]);
      }

      goto cc_next_loop;

    cc_check_done:

      // skip digit value.
      if (isdigit(keyword[0]) || keyword[0] == '"') {
        goto cc_next_loop;
      }

      while (keyword.find(' ') != std::string::npos)
        keyword.erase(keyword.find(' '), 1);

      for (auto &var : kCompilerVariables) {
        if (var.fValue.find(keyword) != std::string::npos) {
          err_str.clear();
          goto cc_next;
        }
      }

      for (auto &fn : kCompilerFunctions) {
        if (fn.find(keyword[0]) != std::string::npos) {
          auto where_begin = fn.find(keyword[0]);
          auto keyword_begin = 0UL;
          auto failed = false;

          for (; where_begin < keyword.size(); ++where_begin) {
            if (fn[where_begin] == '(' && keyword[keyword_begin] == '(') break;

            if (fn[where_begin] != keyword[keyword_begin]) {
              failed = true;
              break;
            }

            ++keyword_begin;
          }

          if (!failed) {
            err_str.clear();
            goto cc_next;
          } else {
            continue;
          }
        }
      }

    cc_error_value:
      if (keyword.find("->") != std::string::npos) return err_str;

      if (keyword.find(".") != std::string::npos) return err_str;

      if (isalnum(keyword[0])) err_str += "\nUndefined value: " + keyword;

      return err_str;
    }

  cc_next_loop:
    continue;
  }

cc_next:

  // extern does not declare anything, it imports a variable.
  // so that's why it's not declare upper.
  if (ParserKit::find_word(ln, "extern")) {
    auto substr = ln.substr(ln.find("extern") + strlen("extern"));
    kCompilerVariables.push_back({.fValue = substr});
  }

  if (kShouldHaveBraces && ln.find('{') == std::string::npos) {
    err_str += "Missing '{' for function ";
    err_str += kFnName;
    err_str += "\n";

    kShouldHaveBraces = false;
    kFnName.clear();
  } else if (kShouldHaveBraces && ln.find('{') != std::string::npos) {
    kShouldHaveBraces = false;
    kFnName.clear();
  }

  bool type_not_found = true;

  if (ln.find('\'') != std::string::npos) {
    ln.replace(ln.find('\''), 3, "0");
  }

  auto first = ln.find('"');
  if (first != std::string::npos) {
    auto second = 0UL;
    bool found_second_quote = false;

    for (size_t i = first + 1; i < ln.size(); ++i) {
      if (ln[i] == '\"') {
        found_second_quote = true;
        second = i;

        break;
      }
    }

    if (!found_second_quote) {
      err_str += "Missing terminating \".";
      err_str += " here -> " + ln.substr(ln.find('"'), second);
    }
  }

  if (ln.find(')') != std::string::npos && ln.find(';') == std::string::npos) {
    if (ln.find('{') == std::string::npos) {
      kFnName = ln;
      kShouldHaveBraces = true;

      goto skip_braces_check;
    } else if (ln.find('{') != std::string::npos) {
      kShouldHaveBraces = false;
    }
  }

skip_braces_check:

  for (auto &key : kCompilerTypes) {
    if (ParserKit::find_word(ln, key.fName)) {
      if (isdigit(ln[ln.find(key.fName) + key.fName.size() + 1])) {
        err_str += "\nNumber cannot be set for ";
        err_str += key.fName;
        err_str += "'s name. here -> ";
        err_str += ln;
      }

      if (ln.find(key.fName) == 0 || ln[ln.find(key.fName) - 1] == ' ' ||
          ln[ln.find(key.fName) - 1] == '\t') {
        type_not_found = false;

        if (ln[ln.find(key.fName) + key.fName.size()] != ' ') {
          type_not_found = true;

          if (ln[ln.find(key.fName) + key.fName.size()] == '\t')
            type_not_found = false;

          goto next;
        } else if (ln[ln.find(key.fName) + key.fName.size()] != '\t') {
          type_not_found = true;

          if (ln[ln.find(key.fName) + key.fName.size()] == ' ')
            type_not_found = false;
        }
      }

    next:

      if (ln.find(';') == std::string::npos) {
        if (ln.find('(') != std::string::npos) {
          if (ln.find('=') == std::string::npos) continue;
        }

        err_str += "\nMissing ';', here -> ";
        err_str += ln;
      } else {
        continue;
      }

      if (ln.find('=') != std::string::npos) {
        if (ln.find('(') != std::string::npos) {
          if (ln.find(')') == std::string::npos) {
            err_str += "\nMissing ')', after '(' here -> ";
            err_str += ln.substr(ln.find('('));
          }
        }
      }
    }
  }

  if (kInBraces && ln.find("struct") != std::string::npos &&
      ln.find("union") != std::string::npos &&
      ln.find("enum") != std::string::npos &&
      ln.find('=') != std::string::npos) {
    if (ln.find(';') == std::string::npos) {
      err_str += "\nMissing ';' after struct/union/enum declaration, here -> ";
      err_str += ln;
    }
  }

  if (ln.find(';') != std::string::npos &&
      ln.find("for") == std::string::npos) {
    if (ln.find(';') + 1 != ln.size()) {
      for (int i = 0; i < ln.substr(ln.find(';') + 1).size(); ++i) {
        if ((ln.substr(ln.find(';') + 1)[i] != ' ') ||
            (ln.substr(ln.find(';') + 1)[i] != '\t')) {
          if (auto err = this->Check(ln.substr(ln.find(';') + 1).c_str(), file);
              !err.empty()) {
            err_str += "\nUnexpected text after ';' -> ";
            err_str += ln.substr(ln.find(';'));
            err_str += err;
          }
        }
      }
    }
  }

  if (ln.find('(') != std::string::npos) {
    if (ln.find(';') == std::string::npos && !ParserKit::find_word(ln, "|") &&
        !ParserKit::find_word(ln, "||") && !ParserKit::find_word(ln, "&") &&
        !ParserKit::find_word(ln, "&&") && !ParserKit::find_word(ln, "~")) {
      bool found_func = false;
      size_t i = ln.find('(');
      std::vector<char> opens;
      std::vector<char> closes;

      for (; i < ln.size(); ++i) {
        if (ln[i] == ')') {
          closes.push_back(1);
        }

        if (ln[i] == '(') {
          opens.push_back(1);
        }
      }

      if (closes.size() != opens.size())
        err_str += "Unterminated (), here -> " + ln;

      bool space_found = false;

      for (int i = 0; i < ln.size(); ++i) {
        if (ln[i] == ')' && !space_found) {
          space_found = true;
          continue;
        }

        if (space_found) {
          if (ln[i] == ' ' && isalnum(ln[i + 1])) {
            err_str += "\nBad function format here -> ";
            err_str += ln;
          }
        }
      }
    }

    if (ln.find('(') < 1) {
      err_str += "\nMissing identifier before '(' here -> ";
      err_str += ln;
    } else {
      if (type_not_found && ln.find(';') == std::string::npos &&
          ln.find("if") == std::string::npos &&
          ln.find("|") == std::string::npos &&
          ln.find("&") == std::string::npos &&
          ln.find("(") == std::string::npos &&
          ln.find(")") == std::string::npos) {
        err_str += "\n Missing ';' or type, here -> ";
        err_str += ln;
      }
    }

    if (ln.find(')') == std::string::npos) {
      err_str += "\nMissing ')', after '(' here -> ";
      err_str += ln.substr(ln.find('('));
    }
  } else {
    if (ln.find("for") != std::string::npos ||
        ln.find("while") != std::string::npos) {
      err_str += "\nMissing '(', after \"for\", here -> ";
      err_str += ln;
    }
  }

  if (ln.find('}') != std::string::npos && !kInBraces) {
    if (!kInStruct && ln.find(';') == std::string::npos) {
      err_str += "\nMismatched '}', here -> ";
      err_str += ln;
    }
  }

  if (!ln.empty()) {
    if (ln.find(';') == std::string::npos &&
        ln.find('{') == std::string::npos &&
        ln.find('}') == std::string::npos &&
        ln.find(')') == std::string::npos &&
        ln.find('(') == std::string::npos &&
        ln.find(',') == std::string::npos) {
      if (ln.size() <= 2) return err_str;

      err_str += "\nMissing ';', here -> ";
      err_str += ln;
    }
  }

  return err_str;
}

/////////////////////////////////////////////////////////////////////////////////////////

/**
 * @brief C To Assembly mount-point.
 */

/////////////////////////////////////////////////////////////////////////////////////////

class AssemblyMountpointCLang final : public CompilerKit::AssemblyInterface {
 public:
  explicit AssemblyMountpointCLang() = default;
  ~AssemblyMountpointCLang() override = default;

  MPCC_COPY_DEFAULT(AssemblyMountpointCLang);

  [[maybe_unused]] static Int32 Arch() noexcept {
    return CompilerKit::AssemblyFactory::kArchPowerPC;
  }

  Int32 CompileToFormat(std::string &src, Int32 arch) override {
    if (arch != AssemblyMountpointCLang::Arch()) return -1;

    if (kCompilerBackend == nullptr) return -1;

    /* @brief copy contents wihtout extension */
    std::string src_file = src.data();
    std::ifstream src_fp = std::ifstream(src_file, std::ios::in);
    std::string dest;

    for (auto &ch : src_file) {
      if (ch == '.') {
        break;
      }

      dest += ch;
    }

    /* According to pef abi. */
    std::vector<const char *> exts = kAsmFileExts;
    dest += exts[4];

    kState.fOutputAssembly = std::make_unique<std::ofstream>(dest);

    auto fmt = CompilerKit::current_date();

    (*kState.fOutputAssembly) << "# Path: " << src_file << "\n";
    (*kState.fOutputAssembly)
        << "# Language: PowerPC Assembly (Generated from ANSI C)\n";
    (*kState.fOutputAssembly) << "# Build Date: " << fmt << "\n\n";

    ParserKit::SyntaxLeafList syntax;

    kState.fSyntaxTreeList.push_back(syntax);
    kState.fSyntaxTree =
        &kState.fSyntaxTreeList[kState.fSyntaxTreeList.size() - 1];

    std::string line_src;

    while (std::getline(src_fp, line_src)) {
      if (auto err = kCompilerBackend->Check(line_src.c_str(), src.data());
          err.empty()) {
        kCompilerBackend->Compile(line_src, src.data());
      } else {
        detail::print_error(err, src.data());
      }
    }

    if (kAcceptableErrors > 0) return -1;

    std::vector<std::string> keywords = {"ldw", "stw", "lda", "sta",
                                         "add", "dec", "mv"};

    ///
    /// Replace, optimize, fix assembly output.
    ///

    for (auto &leaf : kState.fSyntaxTree->fLeafList) {
      std::vector<std::string> access_keywords = {"->", "."};

      for (auto &access_ident : access_keywords) {
        if (ParserKit::find_word(leaf.fUserValue, access_ident)) {
          for (auto &struc : kState.kStructMap) {
            /// TODO:
          }
        }
      }

      for (auto &keyword : keywords) {
        if (ParserKit::find_word(leaf.fUserValue, keyword)) {
          std::size_t cnt = 0UL;

          for (auto &reg : kState.kStackFrame) {
            std::string needle;

            for (size_t i = 0; i < reg.fName.size(); i++) {
              if (reg.fName[i] == ' ') {
                ++i;

                for (; i < reg.fName.size(); i++) {
                  if (reg.fName[i] == ',') {
                    break;
                  }

                  if (reg.fName[i] == ' ') continue;

                  needle += reg.fName[i];
                }

                break;
              }
            }

            if (ParserKit::find_word(leaf.fUserValue, needle)) {
              if (leaf.fUserValue.find("import " + needle) != std::string::npos) {
                std::string range = "import " + needle;
                leaf.fUserValue.replace(leaf.fUserValue.find("import " + needle), range.size(), needle);
              }

              if (leaf.fUserValue.find("ldw r6") != std::string::npos) {
                std::string::difference_type countComma = std::count(
                    leaf.fUserValue.begin(), leaf.fUserValue.end(), ',');

                if (countComma == 1) {
                  leaf.fUserValue.replace(leaf.fUserValue.find("ldw"),
                                          strlen("ldw"), "mv");
                }
              }

              leaf.fUserValue.replace(leaf.fUserValue.find(needle),
                                      needle.size(), reg.fReg);

              ++cnt;
            }
          }

          if (cnt > 1 && keyword != "mv" && keyword != "add" &&
              keyword != "dec") {
            leaf.fUserValue.replace(leaf.fUserValue.find(keyword),
                                    keyword.size(), "mv");
          }
        }
      }
    }

    for (auto &leaf : kState.fSyntaxTree->fLeafList) {
      (*kState.fOutputAssembly) << leaf.fUserValue;
    }

    kState.fSyntaxTree = nullptr;

    kState.fOutputAssembly->flush();
    kState.fOutputAssembly.reset();

    return kOk;
  }
};

/////////////////////////////////////////////////////////////////////////////////////////

#define kPrintF printf
#define kSplashCxx() kPrintF(kWhite "%s\n", "cc, v1.15, (c) Mahrouss Logic")

static void cc_print_help() {
  kSplashCxx();
}

/////////////////////////////////////////////////////////////////////////////////////////

#define kExt ".c"

MPCC_MODULE(NewOSCompilerCLangPowerPC) {
  kCompilerTypes.push_back({.fName = "void", .fValue = "void"});
  kCompilerTypes.push_back({.fName = "char", .fValue = "byte"});
  kCompilerTypes.push_back({.fName = "short", .fValue = "hword"});
  kCompilerTypes.push_back({.fName = "int", .fValue = "dword"});
  kCompilerTypes.push_back({.fName = "long", .fValue = "qword"});
  kCompilerTypes.push_back({.fName = "*", .fValue = "offset"});

  bool skip = false;

  kFactory.Mount(new AssemblyMountpointCLang());
  kMachine = CompilerKit::AssemblyFactory::kArchPowerPC;
  kCompilerBackend = new CompilerBackendCLang();

  for (auto index = 1UL; index < argc; ++index) {
    if (skip) {
      skip = false;
      continue;
    }

    if (argv[index][0] == '-') {
      if (strcmp(argv[index], "-v") == 0 ||
          strcmp(argv[index], "-version") == 0) {
        kSplashCxx();
        return kOk;
      }

      if (strcmp(argv[index], "-verbose") == 0) {
        kState.fVerbose = true;

        continue;
      }

      if (strcmp(argv[index], "-h") == 0 ||
          strcmp(argv[index], "-help") == 0) {
        cc_print_help();

        return kOk;
      }

      if (strcmp(argv[index], "-dialect") == 0) {
        if (kCompilerBackend) std::cout << kCompilerBackend->Language() << "\n";

        return kOk;
      }

      if (strcmp(argv[index], "-fmax-exceptions") == 0) {
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

      std::string err = "Unknown command: ";
      err += argv[index];

      detail::print_error(err, "cc");

      continue;
    }

    kFileList.emplace_back(argv[index]);

    std::string srcFile = argv[index];

    if (strstr(argv[index], kExt) == nullptr) {
      if (kState.fVerbose) {
        detail::print_error(srcFile + " is not a valid C source.\n", "cc");
      }

      return 1;
    }

    if (kFactory.Compile(srcFile, kMachine) != kOk) return -1;
  }

  return kOk;
}

// Last rev 8-1-24
