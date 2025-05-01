/*
 *	========================================================
 *
 *	bpp
 * 	Copyright (C) 2024-2025 Amlal El Mahrouss, all rights reserved.
 *
 * 	========================================================
 */

/// BUGS: 0

#include <LibCompiler/ErrorID.h>
#include <LibCompiler/Parser.h>
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <vector>

#define kMacroPrefix '#'

/// @author EL Mahrouss Amlal (amlel)
/// @file bpp.cxx
/// @brief Preprocessor.

typedef Int32 (*bpp_parser_fn_t)(LibCompiler::String& line, std::ifstream& hdr_file,
                                 std::ofstream& pp_out);

/////////////////////////////////////////////////////////////////////////////////////////

// @brief Preprocessor internal types.

/////////////////////////////////////////////////////////////////////////////////////////

namespace Detail {
enum {
  kInvalid = 0,
  kEqual   = 100,
  kGreaterEqThan,
  kLesserEqThan,
  kGreaterThan,
  kLesserThan,
  kNotEqual,
  kCount = 6,
};

struct bpp_macro_condition final {
  int32_t             fType;
  LibCompiler::String fTypeName;

  void Print() {
    std::cout << "type: " << fType << "\n";
    std::cout << "type_name: " << fTypeName << "\n";
  }
};

struct bpp_macro final {
  std::vector<LibCompiler::String> fArgs;
  LibCompiler::String              fName;
  LibCompiler::String              fValue;

  void Print() {
    std::cout << "name: " << fName << "\n";
    std::cout << "value: " << fValue << "\n";

    for (auto& arg : fArgs) {
      std::cout << "arg: " << arg << "\n";
    }
  }
};
}  // namespace Detail

static std::vector<LibCompiler::String> kFiles;
static std::vector<Detail::bpp_macro>   kMacros;
static std::vector<LibCompiler::String> kIncludes;

static LibCompiler::String kWorkingDir = "";

/////////////////////////////////////////////////////////////////////////////////////////

// @name bpp_parse_if_condition
// @brief parse #if condition

/////////////////////////////////////////////////////////////////////////////////////////

int32_t bpp_parse_if_condition(Detail::bpp_macro_condition& cond, Detail::bpp_macro& macro,
                               bool& inactive_code, bool& defined, LibCompiler::String& macro_str) {
  if (cond.fType == Detail::kEqual) {
    auto substr_macro = macro_str.substr(macro_str.find(macro.fName) + macro.fName.size());

    if (substr_macro.find(macro.fValue) != LibCompiler::String::npos) {
      if (macro.fValue == "0") {
        defined       = false;
        inactive_code = true;

        return 1;
      }

      defined       = true;
      inactive_code = false;

      return 1;
    }
  } else if (cond.fType == Detail::kNotEqual) {
    auto substr_macro = macro_str.substr(macro_str.find(macro.fName) + macro.fName.size());

    if (substr_macro.find(macro.fName) != LibCompiler::String::npos) {
      if (substr_macro.find(macro.fValue) != LibCompiler::String::npos) {
        defined       = false;
        inactive_code = true;

        return 1;
      }

      defined       = true;
      inactive_code = false;

      return 1;
    }

    return 0;
  }

  auto substr_macro = macro_str.substr(macro_str.find(macro.fName) + macro.fName.size());

  LibCompiler::String number;

  for (auto& macro_num : kMacros) {
    if (substr_macro.find(macro_num.fName) != LibCompiler::String::npos) {
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

  if (cond.fType == Detail::kGreaterThan) {
    if (lhs < rhs) {
      defined       = true;
      inactive_code = false;

      return 1;
    }

    return 0;
  }

  if (cond.fType == Detail::kGreaterEqThan) {
    if (lhs <= rhs) {
      defined       = true;
      inactive_code = false;

      return 1;
    }

    return 0;
  }

  if (cond.fType == Detail::kLesserEqThan) {
    if (lhs >= rhs) {
      defined       = true;
      inactive_code = false;

      return 1;
    }

    return 0;
  }

  if (cond.fType == Detail::kLesserThan) {
    if (lhs > rhs) {
      defined       = true;
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

std::vector<LibCompiler::String> kAllIncludes;

/////////////////////////////////////////////////////////////////////////////////////////

// @name bpp_parse_file
// @brief parse file to preprocess it.

/////////////////////////////////////////////////////////////////////////////////////////

void bpp_parse_file(std::ifstream& hdr_file, std::ofstream& pp_out) {
  LibCompiler::String hdr_line;
  LibCompiler::String line_after_include;

  bool inactive_code = false;
  bool defined       = false;

  try {
    while (std::getline(hdr_file, hdr_line)) {
      if (inactive_code) {
        if (hdr_line.find("#endif") == LibCompiler::String::npos) {
          continue;
        } else if (hdr_line[0] == kMacroPrefix &&
                   hdr_line.find("#endif") != LibCompiler::String::npos) {
          inactive_code = false;
        }

        if (hdr_line.find("*/") != LibCompiler::String::npos) {
          continue;
        }
      }

      if (hdr_line.find("--/") != LibCompiler::String::npos) {
        hdr_line.erase(hdr_line.find("--/"));
      }

      if (hdr_line.find("--*") != LibCompiler::String::npos) {
        inactive_code = true;
        // get rid of comment.
        hdr_line.erase(hdr_line.find("--*"));
      }

      /// BPP 'brief' documentation.
      if (hdr_line.find("@brief") != LibCompiler::String::npos) {
        hdr_line.erase(hdr_line.find("@brief"));

        // TODO: Write an <file_name>.html or append to it.
      }

      if (hdr_line[0] == kMacroPrefix && hdr_line.find("endif") != LibCompiler::String::npos) {
        if (!defined && inactive_code) {
          inactive_code = false;
          defined       = false;

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
        if (LibCompiler::find_word(hdr_line, macro.fName)) {
          if (hdr_line.substr(hdr_line.find(macro.fName)).find(macro.fName + '(') !=
              LibCompiler::String::npos) {
            if (!macro.fArgs.empty()) {
              LibCompiler::String              symbol_val = macro.fValue;
              std::vector<LibCompiler::String> args;

              size_t x_arg_indx = 0;

              LibCompiler::String line_after_define = hdr_line;
              LibCompiler::String str_arg;

              if (line_after_define.find("(") != LibCompiler::String::npos) {
                line_after_define.erase(0, line_after_define.find("(") + 1);

                for (auto& subc : line_after_define) {
                  if (subc == ' ' || subc == '\t') continue;

                  if (subc == ',' || subc == ')') {
                    if (str_arg.empty()) continue;

                    args.push_back(str_arg);

                    str_arg.clear();

                    continue;
                  }

                  str_arg.push_back(subc);
                }
              }

              for (auto arg : macro.fArgs) {
                if (symbol_val.find(macro.fArgs[x_arg_indx]) != LibCompiler::String::npos) {
                  symbol_val.replace(symbol_val.find(macro.fArgs[x_arg_indx]),
                                     macro.fArgs[x_arg_indx].size(), args[x_arg_indx]);
                  ++x_arg_indx;
                } else {
                  throw std::runtime_error("cppdrv: Internal error.");
                }
              }

              auto len = macro.fName.size();
              len += symbol_val.size();
              len += 2;  // ( and )

              hdr_line.erase(hdr_line.find(")"), 1);

              hdr_line.replace(hdr_line.find(hdr_line.substr(hdr_line.find(macro.fName + '('))),
                               len, symbol_val);
            } else {
              auto value = macro.fValue;

              hdr_line.replace(hdr_line.find(macro.fName), macro.fName.size(), value);
            }
          }
        }
      }

      if (hdr_line[0] == kMacroPrefix && hdr_line.find("define ") != LibCompiler::String::npos) {
        auto line_after_define = hdr_line.substr(hdr_line.find("define ") + strlen("define "));

        LibCompiler::String macro_value;
        LibCompiler::String macro_key;

        std::size_t pos = 0UL;

        std::vector<LibCompiler::String> args;
        bool                             on_args = false;

        for (auto& ch : line_after_define) {
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

        LibCompiler::String str;

        if (line_after_define.find("(") != LibCompiler::String::npos) {
          line_after_define.erase(0, line_after_define.find("(") + 1);

          for (auto& subc : line_after_define) {
            if (subc == ',' || subc == ')') {
              if (str.empty()) continue;

              args.push_back(str);

              str.clear();

              continue;
            }

            str.push_back(subc);
          }
        }

        Detail::bpp_macro macro;

        macro.fArgs  = args;
        macro.fName  = macro_key;
        macro.fValue = macro_value;

        kMacros.emplace_back(macro);

        continue;
      }

      if (hdr_line[0] != kMacroPrefix) {
        if (inactive_code) {
          continue;
        }

        pp_out << hdr_line << std::endl;

        continue;
      }

      if (hdr_line[0] == kMacroPrefix && hdr_line.find("ifndef") != LibCompiler::String::npos) {
        auto line_after_ifndef = hdr_line.substr(hdr_line.find("ifndef") + strlen("ifndef") + 1);
        LibCompiler::String macro;

        for (auto& ch : line_after_ifndef) {
          if (ch == ' ') {
            break;
          }

          macro += ch;
        }

        if (macro == "0") {
          defined       = true;
          inactive_code = false;
          continue;
        }

        if (macro == "1") {
          defined       = false;
          inactive_code = true;

          continue;
        }

        bool found = false;

        defined       = true;
        inactive_code = false;

        for (auto& macro_ref : kMacros) {
          if (hdr_line.find(macro_ref.fName) != LibCompiler::String::npos) {
            found = true;
            break;
          }
        }

        if (found) {
          defined       = false;
          inactive_code = true;

          continue;
        }
      } else if (hdr_line[0] == kMacroPrefix &&
                 hdr_line.find("else") != LibCompiler::String::npos) {
        if (!defined && inactive_code) {
          inactive_code = false;
          defined       = true;

          continue;
        } else {
          defined       = false;
          inactive_code = true;

          continue;
        }
      } else if (hdr_line[0] == kMacroPrefix &&
                 hdr_line.find("ifdef") != LibCompiler::String::npos) {
        auto line_after_ifdef = hdr_line.substr(hdr_line.find("ifdef") + strlen("ifdef") + 1);
        LibCompiler::String macro;

        for (auto& ch : line_after_ifdef) {
          if (ch == ' ') {
            break;
          }

          macro += ch;
        }

        if (macro == "0") {
          defined       = false;
          inactive_code = true;

          continue;
        }

        if (macro == "1") {
          defined       = true;
          inactive_code = false;

          continue;
        }

        defined       = false;
        inactive_code = true;

        for (auto& macro_ref : kMacros) {
          if (hdr_line.find(macro_ref.fName) != LibCompiler::String::npos) {
            defined       = true;
            inactive_code = false;

            break;
          }
        }
      } else if (hdr_line[0] == kMacroPrefix && hdr_line.find("if") != LibCompiler::String::npos) {
        inactive_code = true;

        std::vector<Detail::bpp_macro_condition> bpp_macro_condition_list = {
            {
                .fType     = Detail::kEqual,
                .fTypeName = "==",
            },
            {
                .fType     = Detail::kNotEqual,
                .fTypeName = "!=",
            },
            {
                .fType     = Detail::kLesserThan,
                .fTypeName = "<",
            },
            {
                .fType     = Detail::kGreaterThan,
                .fTypeName = ">",
            },
            {
                .fType     = Detail::kLesserEqThan,
                .fTypeName = "<=",
            },
            {
                .fType     = Detail::kGreaterEqThan,
                .fTypeName = ">=",
            },
        };

        int32_t good_to_go = 0;

        for (auto& macro_condition : bpp_macro_condition_list) {
          if (hdr_line.find(macro_condition.fTypeName) != LibCompiler::String::npos) {
            for (auto& found_macro : kMacros) {
              if (hdr_line.find(found_macro.fName) != LibCompiler::String::npos) {
                good_to_go = bpp_parse_if_condition(macro_condition, found_macro, inactive_code,
                                                    defined, hdr_line);

                break;
              }
            }
          }
        }

        if (good_to_go) continue;

        auto                line_after_if = hdr_line.substr(hdr_line.find("if") + strlen("if") + 1);
        LibCompiler::String macro;

        for (auto& ch : line_after_if) {
          if (ch == ' ') {
            break;
          }

          macro += ch;
        }

        if (macro == "0") {
          defined       = false;
          inactive_code = true;
          continue;
        }

        if (macro == "1") {
          defined       = true;
          inactive_code = false;

          continue;
        }

        // last try, is it defined to be one?
        for (auto& macro_ref : kMacros) {
          if (macro_ref.fName.find(macro) != LibCompiler::String::npos && macro_ref.fValue == "1") {
            inactive_code = false;
            defined       = true;

            break;
          }
        }
      } else if (hdr_line[0] == kMacroPrefix &&
                 hdr_line.find("warning") != LibCompiler::String::npos) {
        auto line_after_warning = hdr_line.substr(hdr_line.find("warning") + strlen("warning") + 1);
        LibCompiler::String message;

        for (auto& ch : line_after_warning) {
          if (ch == '\r' || ch == '\n') {
            break;
          }

          message += ch;
        }

        std::cout << "warn: " << message << std::endl;
      } else if (hdr_line[0] == kMacroPrefix &&
                 hdr_line.find("error") != LibCompiler::String::npos) {
        auto line_after_warning = hdr_line.substr(hdr_line.find("error") + strlen("error") + 1);
        LibCompiler::String message;

        for (auto& ch : line_after_warning) {
          if (ch == '\r' || ch == '\n') {
            break;
          }

          message += ch;
        }

        throw std::runtime_error("error: " + message);
      } else if (hdr_line[0] == kMacroPrefix &&
                 hdr_line.find("include ") != LibCompiler::String::npos) {
        line_after_include = hdr_line.substr(hdr_line.find("include ") + strlen("include "));

      kIncludeFile:
        auto it = std::find(kAllIncludes.cbegin(), kAllIncludes.cend(), line_after_include);

        if (it != kAllIncludes.cend()) {
          continue;
        }

        LibCompiler::String path;

        kAllIncludes.push_back(line_after_include);

        bool enable    = false;
        bool not_local = false;

        for (auto& ch : line_after_include) {
          if (ch == ' ') continue;

          if (ch == '<') {
            not_local = true;
            enable    = true;

            continue;
          }

          if (ch == '\"') {
            not_local = false;
            enable    = true;
            continue;
          }

          if (enable) {
            path += ch;
          }
        }

        if (not_local) {
          bool open = false;

          if (path.ends_with('>')) {
            path.erase(path.find('>'));
          }

          if (path.ends_with('"')) {
            path.erase(path.find('"'));
          }

          for (auto& include : kIncludes) {
            LibCompiler::String header_path = include;
            header_path.push_back('/');
            header_path += path;

            std::ifstream header(header_path);

            if (!header.is_open()) continue;

            open = true;

            bpp_parse_file(header, pp_out);

            break;
          }

          if (!open) {
            throw std::runtime_error("cppdrv: no such include file: " + path);
          }
        } else {
          std::ifstream header(path);

          if (!header.is_open()) throw std::runtime_error("cppdrv: no such include file: " + path);

          bpp_parse_file(header, pp_out);
        }
      } else {
        std::cerr << ("cppdrv: unknown pre-processor directive, " + hdr_line) << "\n";
        continue;
      }
    }
  } catch (std::out_of_range& oor) {
    return;
  }
}

/////////////////////////////////////////////////////////////////////////////////////////

// @brief main entrypoint of app.

/////////////////////////////////////////////////////////////////////////////////////////

LIBCOMPILER_MODULE(CPlusPlusPreprocessorMain) {
  try {
    bool skip        = false;
    bool double_skip = false;

    Detail::bpp_macro macro_1;

    macro_1.fName  = "__true";
    macro_1.fValue = "1";

    kMacros.push_back(macro_1);

    Detail::bpp_macro macro_unreachable;

    macro_unreachable.fName  = "__unreachable";
    macro_unreachable.fValue = "__libcompiler_unreachable";

    kMacros.push_back(macro_unreachable);

    Detail::bpp_macro macro_unused;

    macro_unreachable.fName  = "__unused";
    macro_unreachable.fValue = "__libcompiler_unused";

    kMacros.push_back(macro_unused);

    Detail::bpp_macro macro_0;

    macro_0.fName  = "__false";
    macro_0.fValue = "0";

    kMacros.push_back(macro_0);

    Detail::bpp_macro macro_zka;

    macro_zka.fName  = "__LIBCOMPILER__";
    macro_zka.fValue = "1";

    kMacros.push_back(macro_zka);

    Detail::bpp_macro macro_cxx;

    macro_cxx.fName  = "__cplusplus";
    macro_cxx.fValue = "202302L";

    kMacros.push_back(macro_cxx);

    Detail::bpp_macro macro_size_t;
    macro_size_t.fName  = "__SIZE_TYPE__";
    macro_size_t.fValue = "unsigned long long int";

    kMacros.push_back(macro_size_t);

    macro_size_t.fName  = "__UINT32_TYPE__";
    macro_size_t.fValue = "unsigned int";

    kMacros.push_back(macro_size_t);

    macro_size_t.fName  = "__UINTPTR_TYPE__";
    macro_size_t.fValue = "unsigned long long int";

    kMacros.push_back(macro_size_t);

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

      if (argv[index][0] == '-') {
        if (strcmp(argv[index], "--cppdrv:ver") == 0) {
          printf("%s\n",
                 "NeKernel Preprocessor Driver v1.11, (c) Amlal El Mahrouss 2024-2025 all rights "
                 "reserved.");

          return LIBCOMPILER_SUCCESS;
        }

        if (strcmp(argv[index], "--cppdrv:?") == 0) {
          printf("%s\n",
                 "NeKernel Preprocessor Driver v1.11, (c) Amlal El Mahrouss 2024-2025 all rights "
                 "reserved.");
          printf("%s\n", "--cppdrv:working-dir <path>: set directory to working path.");
          printf("%s\n", "--cppdrv:include-dir <path>: add directory to include path.");
          printf("%s\n", "--cppdrv:def <name> <value>: define a macro.");
          printf("%s\n", "--cppdrv:ver: print the version.");
          printf("%s\n", "--cppdrv:?: show help (this current command).");

          return LIBCOMPILER_SUCCESS;
        }

        if (strcmp(argv[index], "--cppdrv:include-dir") == 0) {
          LibCompiler::String inc = argv[index + 1];

          skip = true;

          kIncludes.push_back(inc);
        }

        if (strcmp(argv[index], "--cppdrv:working-dir") == 0) {
          LibCompiler::String inc = argv[index + 1];
          skip                    = true;
          kWorkingDir             = inc;
        }

        if (strcmp(argv[index], "--cppdrv:def") == 0 && argv[index + 1] != nullptr &&
            argv[index + 2] != nullptr) {
          LibCompiler::String macro_key = argv[index + 1];

          LibCompiler::String macro_value;
          bool                is_string = false;

          for (int argv_find_len = 0; argv_find_len < strlen(argv[index]); ++argv_find_len) {
            if (!isdigit(argv[index][argv_find_len])) {
              is_string = true;
              macro_value += "\"";

              break;
            }
          }

          macro_value += argv[index + 2];

          if (is_string) macro_value += "\"";

          Detail::bpp_macro macro;
          macro.fName  = macro_key;
          macro.fValue = macro_value;

          kMacros.push_back(macro);

          double_skip = true;
        }

        continue;
      }

      kFiles.emplace_back(argv[index]);
    }

    if (kFiles.empty()) return LIBCOMPILER_EXEC_ERROR;

    for (auto& file : kFiles) {
      if (!std::filesystem::exists(file)) continue;

      std::ifstream file_descriptor(file);
      std::ofstream file_descriptor_pp(file + ".pp");

      bpp_parse_file(file_descriptor, file_descriptor_pp);
    }

    return LIBCOMPILER_SUCCESS;
  } catch (const std::runtime_error& e) {
    std::cout << e.what() << '\n';
  }

  return LIBCOMPILER_EXEC_ERROR;
}

// Last rev 8-1-24
