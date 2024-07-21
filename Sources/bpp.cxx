/*
 *	========================================================
 *
 *	bpp
 * 	Copyright ZKA Technologies, all rights reserved.
 *
 * 	========================================================
 */

/// BUGS: 0

#include <Headers/ParserKit.hpp>
#include <Headers/StdKit/ErrorID.hpp>
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>

#define kMacroPrefix '%'

/// @author Amlal El Mahrouss (amlel)
/// @file bpp.cxx
/// @brief Preprocessor.

typedef Int32 (*bpp_parser_fn_t)(std::string &line, std::ifstream &hdr_file,
                                 std::ofstream &pp_out);

/////////////////////////////////////////////////////////////////////////////////////////

// @brief Preprocessor internal types.

/////////////////////////////////////////////////////////////////////////////////////////

namespace details {
enum {
  kEqual,
  kGreaterEqThan,
  kLesserEqThan,
  kGreaterThan,
  kLesserThan,
  kNotEqual,
};

struct bpp_macro_condition final {
  int32_t fType;
  std::string fTypeName;
};

struct bpp_macro final {
  std::vector<std::string> fArgs;
  std::string fName;
  std::string fValue;
};

class bpp_pragma final {
 public:
  explicit bpp_pragma() = default;
  ~bpp_pragma() = default;

  MPCC_COPY_DEFAULT(bpp_pragma);

  std::string fMacroName;
  bpp_parser_fn_t fParse;
};
}  // namespace details

static std::vector<std::string> kFiles;
static std::vector<details::bpp_macro> kMacros;
static std::vector<std::string> kIncludes;

static std::string kWorkingDir;

static std::vector<std::string> kKeywords = {
    "include", "if",     "pragma", "def",     "elif",
    "ifdef",   "ifndef", "else",   "warning", "error"};

#define kKeywordCxxCnt kKeywords.size()

/////////////////////////////////////////////////////////////////////////////////////////

// @name bpp_parse_if_condition
// @brief parse #if condition

/////////////////////////////////////////////////////////////////////////////////////////

int32_t bpp_parse_if_condition(details::bpp_macro_condition &cond,
                               details::bpp_macro &macro, bool &inactive_code,
                               bool &defined, std::string &macro_str) {
  if (cond.fType == details::kEqual) {
    auto substr_macro =
        macro_str.substr(macro_str.find(macro.fName) + macro.fName.size());

    if (substr_macro.find(macro.fValue) != std::string::npos) {
      if (macro.fValue == "0") {
        defined = false;
        inactive_code = true;

        return 1;
      }

      defined = true;
      inactive_code = false;

      return 1;
    }
  } else if (cond.fType == details::kNotEqual) {
    auto substr_macro =
        macro_str.substr(macro_str.find(macro.fName) + macro.fName.size());

    if (substr_macro.find(macro.fName) != std::string::npos) {
      if (substr_macro.find(macro.fValue) != std::string::npos) {
        defined = false;
        inactive_code = true;

        return 1;
      }

      defined = true;
      inactive_code = false;

      return 1;
    }

    return 0;
  }

  auto substr_macro =
      macro_str.substr(macro_str.find(macro.fName) + macro.fName.size());

  std::string number;

  for (auto &macro_num : kMacros) {
    if (substr_macro.find(macro_num.fName) != std::string::npos) {
      for (size_t i = 0; i < macro_num.fName.size(); ++i) {
        if (isdigit(macro_num.fValue[i])) {
          number += macro_num.fValue[i];
        } else {
          number.clear();
          break;
        }
      }

      break;
    }
  }

  size_t y = 2;

  /* last try */
  for (; y < macro_str.size(); y++) {
    if (isdigit(macro_str[y])) {
      for (size_t x = y; x < macro_str.size(); x++) {
        if (macro_str[x] == ' ') break;

        number += macro_str[x];
      }

      break;
    }
  }

  size_t rhs = atol(macro.fValue.c_str());
  size_t lhs = atol(number.c_str());

  if (lhs == 0) {
    number.clear();
    ++y;

    for (; y < macro_str.size(); y++) {
      if (isdigit(macro_str[y])) {
        for (size_t x = y; x < macro_str.size(); x++) {
          if (macro_str[x] == ' ') break;

          number += macro_str[x];
        }

        break;
      }
    }

    lhs = atol(number.c_str());
  }

  if (cond.fType == details::kGreaterThan) {
    if (lhs < rhs) {
      defined = true;
      inactive_code = false;

      return 1;
    }

    return 0;
  }

  if (cond.fType == details::kGreaterEqThan) {
    if (lhs <= rhs) {
      defined = true;
      inactive_code = false;

      return 1;
    }

    return 0;
  }

  if (cond.fType == details::kLesserEqThan) {
    if (lhs >= rhs) {
      defined = true;
      inactive_code = false;

      return 1;
    }

    return 0;
  }

  if (cond.fType == details::kLesserThan) {
    if (lhs > rhs) {
      defined = true;
      inactive_code = false;

      return 1;
    }

    return 0;
  }

  return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

// @brief stores every included file here.

/////////////////////////////////////////////////////////////////////////////////////////

std::vector<std::string> kAllIncludes;

/////////////////////////////////////////////////////////////////////////////////////////

// @name bpp_parse_file
// @brief parse file to preprocess it.

/////////////////////////////////////////////////////////////////////////////////////////

void bpp_parse_file(std::ifstream &hdr_file, std::ofstream &pp_out) {
  std::string hdr_line;
  std::string line_after_include;

  bool inactive_code = false;
  bool defined = false;

  try {
    while (std::getline(hdr_file, hdr_line)) {
      /// BPP Documentation.
      if (hdr_line.find("@bdoc") != std::string::npos) {
        hdr_line.erase(hdr_line.find("@bdoc"));
      }

      if (hdr_line[0] == kMacroPrefix &&
          hdr_line.find("endif") != std::string::npos) {
        if (!defined && inactive_code) {
          inactive_code = false;
          defined = false;

          continue;
        }

        continue;
      }

      if (!defined && inactive_code) {
        continue;
      }

      if (defined && inactive_code) {
        continue;
      }

      for (auto macro : kMacros) {
        if (ParserKit::find_word(hdr_line, macro.fName) &&
            hdr_line.find("%def") == std::string::npos) {
          auto value = macro.fValue;

          hdr_line.replace(hdr_line.find(macro.fName), macro.fName.size(),
                           value);
        }
      }

      if (hdr_line[0] == kMacroPrefix &&
          hdr_line.find("def ") != std::string::npos) {
        auto line_after_define =
            hdr_line.substr(hdr_line.find("def ") + strlen("def "));

        std::string macro_value;
        std::string macro_key;

        std::size_t pos = 0UL;

        std::vector<std::string> args;
        bool on_args = false;

        for (auto &ch : line_after_define) {
          ++pos;

          if (ch == '(') {
            on_args = true;
            continue;
          }

          if (ch == ')') {
            on_args = false;
            continue;
          }

          if (ch == '\\') continue;

          if (on_args) continue;

          if (ch == ' ') {
            for (size_t i = pos; i < line_after_define.size(); i++) {
              macro_value += line_after_define[i];
            }

            break;
          }

          macro_key += ch;
        }

        std::vector<std::string> dupls;
        std::string str;

        line_after_define.erase(0, line_after_define.find("(") + 1);

        for (auto &subc : line_after_define) {
          if (subc == ',' || subc == ')') {
            if (str.empty()) continue;

            dupls.push_back(str);
            args.push_back(str);

            str.clear();

            continue;
          }

          if (isalnum(subc)) str.push_back(subc);
        }

        for (auto &dupl : dupls) {
          std::size_t cnt = 0;

          for (auto &arg : args) {
            if (dupl == arg) ++cnt;
          }

          if (cnt > 1) {
            auto it = std::find(args.begin(), args.end(), dupl);

            while (it != args.end()) {
              args.erase(it);
              it = std::find(args.begin(), args.end(), dupl);
            }
          }
        }

        details::bpp_macro macro;

        macro.fArgs = args;
        macro.fName = macro_key;
        macro.fValue = macro_value;

        kMacros.emplace_back(macro);

        continue;
      }

      if (hdr_line[0] != kMacroPrefix) {
        if (inactive_code) {
          continue;
        }

        for (auto &macro : kMacros) {
          if (hdr_line.find(macro.fName) != std::string::npos) {
            std::vector<std::string> arg_values;

            if (macro.fArgs.size() > 0) {
              for (size_t i = 0; i < hdr_line.size(); ++i) {
                if (hdr_line[i] == '(') {
                  std::string tmp_arg;

                  for (size_t x = i; x < hdr_line.size(); x++) {
                    if (hdr_line[x] == ')') break;

                    if (hdr_line[x] == ' ') continue;

                    if (hdr_line[i] == '\\') continue;

                    if (hdr_line[x] == ',') {
                      arg_values.push_back(tmp_arg);
                      tmp_arg.clear();
                      continue;
                    }

                    tmp_arg += hdr_line[x];
                  }

                  break;
                }
              }

              std::string symbol;

              for (char i : macro.fValue) {
                if (i == '(') break;

                if (i == '\\') continue;

                symbol += i;
              }

              hdr_line.replace(hdr_line.find(macro.fName), macro.fName.size(),
                               symbol);

              size_t x_arg_indx = 0;

              for (size_t i = hdr_line.find(macro.fValue); i < hdr_line.size();
                   ++i) {
                if (hdr_line.find(macro.fArgs[x_arg_indx]) == i) {
                  hdr_line.replace(i, macro.fArgs[x_arg_indx].size(),
                                   arg_values[x_arg_indx]);
                  ++x_arg_indx;
                }
              }
            } else {
              std::string symbol;

              for (size_t i = 0; i < macro.fValue.size(); i++) {
                if (macro.fValue[i] == ' ') continue;

                if (macro.fValue[i] == '\\') continue;

                symbol += macro.fValue[i];
              }

              hdr_line.replace(hdr_line.find(macro.fName), macro.fName.size(),
                               symbol);
            }

            break;
          }
        }

        pp_out << hdr_line << std::endl;

        continue;
      }

      if (hdr_line[0] == kMacroPrefix &&
          hdr_line.find("ifndef") != std::string::npos) {
        auto line_after_ifndef =
            hdr_line.substr(hdr_line.find("ifndef") + strlen("ifndef") + 1);
        std::string macro;

        for (auto &ch : line_after_ifndef) {
          if (ch == ' ') {
            break;
          }

          macro += ch;
        }

        if (macro == "0") {
          defined = true;
          inactive_code = false;
          continue;
        }

        if (macro == "1") {
          defined = false;
          inactive_code = true;

          continue;
        }

        bool found = false;

        defined = true;
        inactive_code = false;

        for (auto &macro_ref : kMacros) {
          if (hdr_line.find(macro_ref.fName) != std::string::npos) {
            found = true;
            break;
          }
        }

        if (found) {
          defined = false;
          inactive_code = true;

          continue;
        }
      } else if (hdr_line[0] == kMacroPrefix &&
                 hdr_line.find("else") != std::string::npos) {
        if (!defined && inactive_code) {
          inactive_code = false;
          defined = true;

          continue;
        } else {
          defined = false;
          inactive_code = true;

          continue;
        }
      } else if (hdr_line[0] == kMacroPrefix &&
                 hdr_line.find("ifdef") != std::string::npos) {
        auto line_after_ifdef =
            hdr_line.substr(hdr_line.find("ifdef") + strlen("ifdef") + 1);
        std::string macro;

        for (auto &ch : line_after_ifdef) {
          if (ch == ' ') {
            break;
          }

          macro += ch;
        }

        if (macro == "0") {
          defined = false;
          inactive_code = true;

          continue;
        }

        if (macro == "1") {
          defined = true;
          inactive_code = false;

          continue;
        }

        defined = false;
        inactive_code = true;

        for (auto &macro_ref : kMacros) {
          if (hdr_line.find(macro_ref.fName) != std::string::npos) {
            defined = true;
            inactive_code = false;

            break;
          }
        }
      } else if (hdr_line[0] == kMacroPrefix &&
                 hdr_line.find("pragma") != std::string::npos) {
        line_after_include = hdr_line.substr(hdr_line.find("pragma once"));

        // search for this file
        auto it = std::find(kAllIncludes.cbegin(), kAllIncludes.cend(),
                            line_after_include);

        if (it == kAllIncludes.cend()) {
          goto kIncludeFile;
        }
      } else if (hdr_line[0] == kMacroPrefix &&
                 hdr_line.find("if") != std::string::npos) {
        inactive_code = true;

        std::vector<details::bpp_macro_condition> bpp_macro_condition_list = {
            {
                .fType = details::kEqual,
                .fTypeName = "==",
            },
            {
                .fType = details::kNotEqual,
                .fTypeName = "!=",
            },
            {
                .fType = details::kLesserThan,
                .fTypeName = "<",
            },
            {
                .fType = details::kGreaterThan,
                .fTypeName = ">",
            },
            {
                .fType = details::kLesserEqThan,
                .fTypeName = "<=",
            },
            {
                .fType = details::kGreaterEqThan,
                .fTypeName = ">=",
            },
        };

        int32_t good_to_go = 0;

        for (auto &macro_condition : bpp_macro_condition_list) {
          if (hdr_line.find(macro_condition.fTypeName) != std::string::npos) {
            for (auto &found_macro : kMacros) {
              if (hdr_line.find(found_macro.fName) != std::string::npos) {
                good_to_go =
                    bpp_parse_if_condition(macro_condition, found_macro,
                                           inactive_code, defined, hdr_line);

                break;
              }
            }
          }
        }

        if (good_to_go) continue;

        auto line_after_if =
            hdr_line.substr(hdr_line.find("if") + strlen("if") + 1);
        std::string macro;

        for (auto &ch : line_after_if) {
          if (ch == ' ') {
            break;
          }

          macro += ch;
        }

        if (macro == "0") {
          defined = false;
          inactive_code = true;
          continue;
        }

        if (macro == "1") {
          defined = true;
          inactive_code = false;

          continue;
        }

        // last try, is it defined to be one?
        for (auto &macro_ref : kMacros) {
          if (macro_ref.fName.find(macro) != std::string::npos &&
              macro_ref.fValue == "1") {
            inactive_code = false;
            defined = true;

            break;
          }
        }
      } else if (hdr_line[0] == kMacroPrefix &&
                 hdr_line.find("warning") != std::string::npos) {
        auto line_after_warning =
            hdr_line.substr(hdr_line.find("warning") + strlen("warning") + 1);
        std::string message;

        for (auto &ch : line_after_warning) {
          if (ch == '\r' || ch == '\n') {
            break;
          }

          message += ch;
        }

        std::cout << "Warning: " << message << std::endl;
      } else if (hdr_line[0] == kMacroPrefix &&
                 hdr_line.find("error") != std::string::npos) {
        auto line_after_warning =
            hdr_line.substr(hdr_line.find("error") + strlen("error") + 1);
        std::string message;

        for (auto &ch : line_after_warning) {
          if (ch == '\r' || ch == '\n') {
            break;
          }

          message += ch;
        }

        throw std::runtime_error("Error: " + message);
      } else if (hdr_line[0] == kMacroPrefix &&
                 hdr_line.find("inc ") != std::string::npos) {
        line_after_include =
            hdr_line.substr(hdr_line.find("inc ") + strlen("inc "));

      kIncludeFile:
        auto it = std::find(kAllIncludes.cbegin(), kAllIncludes.cend(),
                            line_after_include);

        if (it != kAllIncludes.cend()) {
          continue;
        }

        std::string path;

        kAllIncludes.push_back(line_after_include);

        bool enable = false;
        bool not_local = false;

        for (auto &ch : line_after_include) {
          if (ch == ' ') continue;

          if (ch == '<') {
            not_local = true;
            enable = true;

            continue;
          }

          if (ch == '\'') {
            enable = true;
            continue;
          }

          if (enable) {
            if (not_local) {
              if (ch == '>') break;
            } else {
              if (ch == '\'') {
                break;
              }
            }

            path += ch;
          }
        }

        if (not_local) {
          bool open = false;

          for (auto &include : kIncludes) {
            std::string header_path = include;
            header_path.push_back('/');
            header_path += path;

            std::ifstream header(header_path);

            if (!header.is_open()) continue;

            open = true;

            bpp_parse_file(header, pp_out);

            break;
          }

          if (!open) {
            throw std::runtime_error("bpp: no such include file: " + path);
          }
        } else {
          std::ifstream header(kWorkingDir + path);

          if (!header.is_open())
            throw std::runtime_error("bpp: no such include file: " + path);

          bpp_parse_file(header, pp_out);
        }
      } else {
        std::cerr << ("bpp: unknown pre-processor directive, " + hdr_line)
                  << "\n";
        continue;
      }
    }
  } catch (std::out_of_range &oor) {
    return;
  }
}

/////////////////////////////////////////////////////////////////////////////////////////

// @brief main entrypoint of app.

/////////////////////////////////////////////////////////////////////////////////////////

NDK_MODULE(NewOSPreprocessor) {
  try {
    bool skip = false;
    bool double_skip = false;

    details::bpp_macro macro_1;
    macro_1.fName = "__true";
    macro_1.fValue = "1";

    kMacros.push_back(macro_1);

    details::bpp_macro macro_0;
    macro_0.fName = "__false";
    macro_0.fValue = "0";

    kMacros.push_back(macro_0);

    details::bpp_macro macro_hcore;
    macro_hcore.fName = "__MAHROUSS__";
    macro_hcore.fValue = "1";

    kMacros.push_back(macro_hcore);

    for (auto index = 1UL; index < argc; ++index) {
      if (skip) {
        skip = false;
        continue;
      }

      if (double_skip) {
        ++index;
        double_skip = false;
        continue;
      }

      if (argv[index][0] == '/') {
        if (strcmp(argv[index], "/version") == 0) {
          printf("%s\n", "bpp v1.11, (c) ZKA Technologies");
          return 0;
        }

        if (strcmp(argv[index], "/help") == 0) {
          printf("%s\n", "ZKA Preprocessor Driver v1.11, (c) ZKA Technologies");
          printf("%s\n", "/working-dir <path>: set directory to working path.");
          printf("%s\n", "/include-dir <path>: add directory to include path.");
          printf("%s\n", "/def <name> <value>: def macro.");
          printf("%s\n", "/version: print the version.");
          printf("%s\n", "/help: show help.");

          return 0;
        }

        if (strcmp(argv[index], "/include-dir") == 0) {
          std::string inc = argv[index + 1];

          skip = true;

          kIncludes.push_back(inc);
        }

        if (strcmp(argv[index], "/working-dir") == 0) {
          std::string inc = argv[index + 1];
          skip = true;
          kWorkingDir = inc;
        }

        if (strcmp(argv[index], "/def") == 0 && argv[index + 1] != nullptr &&
            argv[index + 2] != nullptr) {
          std::string macro_key = argv[index + 1];

          std::string macro_value;
          bool is_string = false;

          for (int argv_find_len = 0; argv_find_len < strlen(argv[index]);
               ++argv_find_len) {
            if (!isdigit(argv[index][argv_find_len])) {
              is_string = true;
              macro_value += "\"";

              break;
            }
          }

          macro_value += argv[index + 2];

          if (is_string) macro_value += "\"";

          details::bpp_macro macro;
          macro.fName = macro_key;
          macro.fValue = macro_value;

          kMacros.push_back(macro);

          double_skip = true;
        }

        continue;
      }

      kFiles.emplace_back(argv[index]);
    }

    if (kFiles.empty()) return MPCC_EXEC_ERROR;

    for (auto &file : kFiles) {
      if (!std::filesystem::exists(file)) continue;

      std::ifstream file_descriptor(file);
      std::ofstream file_descriptor_pp(file + ".pp");

      bpp_parse_file(file_descriptor, file_descriptor_pp);
    }

    return 0;
  } catch (const std::runtime_error &e) {
    std::cout << e.what() << '\n';
  }

  return 1;
}

// Last rev 8-1-24
