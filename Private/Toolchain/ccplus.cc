/*
 *	========================================================
 *
 *	ccplus
 * 	Copyright Mahrouss Logic, all rights reserved.
 *
 * 	========================================================
 */

/// bugs: 1
/// Windows: Output name gets corrupt.

#define kPrintF printf
#define kSplashCxx()     \
  kPrintF(kWhite "%s\n", \
          "Mahrouss Visual C++ Compiler for HCore, Copyright Mahrouss Logic.")

#include <Compiler/cl-parser.hxx>
#include <CompilerKit/AsmKit/Arch/amd64.hpp>
#include <CompilerKit/ParserKit.hpp>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#define kOk 0

/* Mahrouss Logic C++ driver */
/* This is part of MP-UX C++ SDK. */
/* (c) Mahrouss Logic */

// @author Amlal El Mahrouss (amlel)
// @file cc.cc
// @brief Optimized C++ Compiler.

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
struct CompilerRegisterMap final {
  std::string fName;
  std::string fReg;
};

// \brief Offset based struct/class
struct CompilerStructMap final {
  std::string fName;
  std::string fReg;

  // offset counter
  std::size_t fOffsetsCnt;

  // offset array
  std::vector<std::pair<Int32, std::string>> fOffsets;
};

struct CompilerState final {
  std::vector<ParserKit::SyntaxLeafList> fSyntaxTreeList;
  std::vector<CompilerRegisterMap> kStackFrame;
  std::vector<CompilerStructMap> kStructMap;
  ParserKit::SyntaxLeafList* fSyntaxTree{nullptr};
  std::unique_ptr<std::ofstream> fOutputAssembly;
  std::string fLastFile;
  std::string fLastError;
  bool kVerbose;
};
}  // namespace detail

static detail::CompilerState kState;
static SizeType kErrorLimit = 100;

static Int32 kAcceptableErrors = 0;

namespace detail {
void print_error(std::string reason, std::string file) noexcept {
  if (reason[0] == '\n') reason.erase(0, 1);

  if (file.find(".pp") != std::string::npos) {
    file.erase(file.find(".pp"), 3);
  }

  if (kState.fLastFile != file) {
    std::cout << kRed << "[ ccplus ] " << kWhite
              << ((file == "ccplus") ? "internal compiler error "
                                     : ("in file, " + file))
              << kBlank << std::endl;
    std::cout << kRed << "[ ccplus ] " << kWhite << reason << kBlank
              << std::endl;

    kState.fLastFile = file;
  } else {
    std::cout << kRed << "[ ccplus ] [ " << kState.fLastFile << " ] " << kWhite
              << reason << kBlank << std::endl;
  }

  if (kAcceptableErrors > kErrorLimit) std::exit(3);

  ++kAcceptableErrors;
}

struct CompilerType {
  std::string fName;
  std::string fValue;
};
}  // namespace detail

/////////////////////////////////////////////////////////////////////////////////////////

// Target architecture.
static int kMachine = CompilerKit::AssemblyFactory::kArchAMD64;

/////////////////////////////////////////

// ARGUMENTS REGISTERS (R8, R15)

/////////////////////////////////////////

static size_t kRegisterCnt = kAsmRegisterLimit;
static size_t kStartUsable = 8;
static size_t kUsableLimit = 15;
static size_t kRegisterCounter = kStartUsable;
static std::string kRegisterPrefix = kAsmRegisterPrefix;
static std::vector<std::string> kKeywords;

/////////////////////////////////////////

// COMPILER PARSING UTILITIES/STATES.

/////////////////////////////////////////

static std::vector<std::string> kFileList;
static CompilerKit::AssemblyFactory kFactory;
static bool kInStruct = false;
static bool kOnWhileLoop = false;
static bool kOnForLoop = false;
static bool kInBraces = false;
static size_t kBracesCount = 0UL;

/* @brief C compiler backend for Mahrouss Logic C */
class CompilerBackendClang final : public ParserKit::CompilerBackend {
 public:
  explicit CompilerBackendClang() = default;
  ~CompilerBackendClang() override = default;

  MPCC_COPY_DEFAULT(CompilerBackendClang);

  bool Compile(const std::string& text, const char* file) override;

  const char* Language() override { return "Mahrouss C++, HCore target."; }
};

/// compiler variables

static CompilerBackendClang* kCompilerBackend = nullptr;
static std::vector<detail::CompilerType> kCompilerVariables;
static std::vector<std::string> kCompilerFunctions;

/// detail namespaces

namespace detail {
union number_cast final {
  number_cast(UInt64 raw) : raw(raw) {}

  char number[8];
  UInt64 raw;
};
}  // namespace detail

/////////////////////////////////////////////////////////////////////////////////////////

// @name Compile
// @brief Generate MASM from a C++ source.

/////////////////////////////////////////////////////////////////////////////////////////

bool CompilerBackendClang::Compile(const std::string& text, const char* file) {
  if (text.empty()) return false;

  // if (expr)
  // int name = expr;
  // expr;

  std::size_t index = 0UL;

  auto syntax_tree = ParserKit::SyntaxLeafList::SyntaxLeaf();

  syntax_tree.fUserData = text;
  kState.fSyntaxTree->fLeafList.emplace_back(syntax_tree);

  std::string text_cpy = text;

  std::vector<std::pair<std::string, std::size_t>> keywords_list;

  for (auto& keyword : kKeywords) {
    while (text_cpy.find(keyword) != std::string::npos) {
      keywords_list.emplace_back(std::make_pair(keyword, index));
      ++index;

      text_cpy.erase(text_cpy.find(keyword), keyword.size());
    }
  }

  // TODO: sort keywords

  for (auto& keyword : keywords_list) {
    syntax_tree.fUserData = keyword.first;
    kState.fSyntaxTree->fLeafList.emplace_back(syntax_tree);
  }

  return true;
}

/////////////////////////////////////////////////////////////////////////////////////////

/**
 * @brief C To Assembly mount-point.
 */

/////////////////////////////////////////////////////////////////////////////////////////

class AssemblyMountpointClang final : public CompilerKit::AssemblyMountpoint {
 public:
  explicit AssemblyMountpointClang() = default;
  ~AssemblyMountpointClang() override = default;

  MPCC_COPY_DEFAULT(AssemblyMountpointClang);

  [[maybe_unused]] static Int32 Arch() noexcept {
    return CompilerKit::AssemblyFactory::kArchAMD64;
  }

  Int32 CompileToFormat(std::string& src, Int32 arch) override {
    if (arch != AssemblyMountpointClang::Arch()) return -1;

    if (kCompilerBackend == nullptr) return -1;

    /* @brief copy contents wihtout extension */
    std::string src_file = src.data();
    std::ifstream src_fp = std::ifstream(src_file, std::ios::in);
    std::string dest;

    for (auto& ch : src_file) {
      if (ch == '.') {
        break;
      }

      dest += ch;
    }

    /* According to PEF abi. */

    std::vector<const char*> exts = kAsmFileExts;
    dest += exts[3];

    kState.fOutputAssembly = std::make_unique<std::ofstream>(dest);

    auto fmt = CompilerKit::current_date();

    (*kState.fOutputAssembly) << "# Path: " << src_file << "\n";
    (*kState.fOutputAssembly)
        << "# Language: AMD64 HCore Assembly (Generated from C++)\n";
    (*kState.fOutputAssembly) << "# Build Date: " << fmt << "\n\n";

    ParserKit::SyntaxLeafList syntax;

    kState.fSyntaxTreeList.emplace_back(syntax);
    kState.fSyntaxTree =
        &kState.fSyntaxTreeList[kState.fSyntaxTreeList.size() - 1];

    std::string source;

    while (std::getline(src_fp, source)) {
      // compile into AST.
      kCompilerBackend->Compile(source.c_str(), src.data());
    }

    if (kAcceptableErrors > 0) return -1;

    std::vector<std::string> lines;

    // \brief compiler scope type.
    struct scope_type {
      std::vector<std::string> vals;
      std::size_t reg_cnt{kStartUsable};
      std::size_t id{0};

      bool operator==(const scope_type& typ) const { return typ.id == id; }
    };

    std::vector<scope_type> scope;
    scope.emplace_back();

    bool found_type = false;
    bool is_pointer = false;
    bool found_expr = false;
    bool found_func = false;
    bool is_access_field = false;

    std::string type;

    std::string this_ref;

    for (auto& leaf : kState.fSyntaxTree->fLeafList) {
      if (leaf.fUserData == "{") {
        scope.emplace_back();
      }

      if (leaf.fUserData == "}") {
        is_access_field = false;
        this_ref = "";

        scope.pop_back();

        if (scope.size() == 0) {
          kRegisterCounter = kStartUsable;
        }
      }

      if (leaf.fUserData == "int" || leaf.fUserData == "long" ||
          leaf.fUserData == "unsigned" || leaf.fUserData == "short" ||
          leaf.fUserData == "char" || leaf.fUserData == "struct" ||
          leaf.fUserData == "class") {
        type += leaf.fUserData;
        found_type = true;
      }

      if (leaf.fUserData == "(") {
        if (found_type) {
          found_expr = true;
          found_type = false;
          is_pointer = false;
        }
      }

      if (leaf.fUserData == ";") {
        is_access_field = false;
      }

      if (leaf.fUserData.find(" this") != std::string::npos) {
        leaf.fUserData.replace(leaf.fUserData.find(" this"), strlen(" this"),
                               "r15");
        is_access_field = true;
      }

      if (leaf.fUserData.find(" *this") != std::string::npos) {
        leaf.fUserData.replace(leaf.fUserData.find(" *this"), strlen(" *this"),
                               "r15");
        is_access_field = true;
      }

      if (leaf.fUserData.find("->") != std::string::npos) {
        leaf.fUserData.replace(leaf.fUserData.find("->"), strlen("->"), ", ");

        is_access_field = true;
      }

      if (leaf.fUserData == ")") {
        if (found_expr) {
          found_expr = false;
          is_pointer = false;
        } else {
          leaf.fUserValue = "export .text _CZZ_MAHR_MANGLE";

          for (auto& line : lines) {
            if (line.find(type) != std::string::npos &&
                line.find("(") != std::string::npos) {
              auto fn_name = line.substr(line.find(type), line.find("("));

              while (fn_name.find(' ') != std::string::npos)
                fn_name.replace(fn_name.find(' '), 1, "@");

              leaf.fUserValue += fn_name;

              break;
            }
          }

          leaf.fUserValue += "\n";

          found_func = true;
        }
      }

      if (leaf.fUserData == ",") {
        if (is_pointer) {
          is_pointer = false;
        }

        auto& front = scope.front();

        std::string reg = "r";
        reg += std::to_string(front.reg_cnt);
        ++front.reg_cnt;

        front.vals.push_back(reg);
      }

      if (leaf.fUserData == "*" || leaf.fUserData == "&") {
        if (found_type && !found_expr) is_pointer = true;
      }

      if (leaf.fUserData == "=") {
        if (found_type) {
          auto& front = scope.front();

          std::string reg = "r";
          reg += std::to_string(front.reg_cnt);
          ++front.reg_cnt;

          front.vals.push_back(reg);

          leaf.fUserValue = !is_pointer ? "mov %s, %s1\n" : "lea %s, %s1\n";

          for (auto& ln : lines) {
            if (ln.find(leaf.fUserData) != std::string::npos &&
                ln.find(";") != std::string::npos) {
              auto val =
                  ln.substr(ln.find(leaf.fUserData) + leaf.fUserData.size());

              if (val.find(";") != std::string::npos)
                val.erase(val.find(";"), 1);

              while (val.find(" ") != std::string::npos)
                val.erase(val.find(" "), 1);

              if (val == this_ref) val = "r15";

              if (this_ref.empty()) this_ref = val;

              if (isalnum(val[0]) && !isdigit(val[0]) &&
                  ln.find("r6") == std::string::npos)
                val.insert(0, "import ");

              leaf.fUserValue.replace(leaf.fUserValue.find("%s1"),
                                      strlen("%s1"), val);
            }
          }

          leaf.fUserValue.replace(leaf.fUserValue.find("%s"), strlen("%s"),
                                  reg);

          found_type = false;
        } else {
          leaf.fUserValue = !is_pointer ? "mov %s, %s1\n" : "lea 0(%s), %s1\n";

          for (auto& ln : lines) {
            if (ln.find(leaf.fUserData) != std::string::npos &&
                ln.find(";") != std::string::npos) {
              std::string nm;
              for (auto i = ln.find('=') + 1; i < ln.size(); ++i) {
                if (ln[i] == ';') break;

                nm.push_back(ln[i]);
              }

              if (!nm.empty()) {
                if (nm == this_ref) nm = "r15";

                if (this_ref.empty()) this_ref = nm;

                leaf.fUserValue.replace(leaf.fUserValue.find("%s1"),
                                        strlen("%s1"), nm);
                break;
              }
            }
          }

          auto& front = scope.front();

          std::string reg = "r";
          reg += std::to_string(front.reg_cnt);
          ++front.reg_cnt;

          leaf.fUserValue.replace(leaf.fUserValue.find("%s"), strlen("%s"),
                                  reg);

          front.vals.push_back(reg);

          if (is_pointer) {
            is_pointer = false;
          }
        }
      }

      if (leaf.fUserData == "return") {
        leaf.fUserValue = "mov rax, %s\nret";

        if (!lines.empty()) {
          for (auto& ln : lines) {
            if (ln.find(leaf.fUserData) != std::string::npos &&
                ln.find(";") != std::string::npos) {
              auto val =
                  ln.substr(ln.find(leaf.fUserData) + leaf.fUserData.size());
              val.erase(val.find(";"), 1);

              std::string val_reg;
              std::size_t reg_cnt = kRegisterCounter;

              for (int i = ln.find(leaf.fUserData) + leaf.fUserData.size();
                   i < ln.size(); ++i) {
                try {
                  if (ln[i] == ',' || ln[i] == '+' || ln[i] == '/' ||
                      ln[i] == '-' || ln[i] == '*' || ln[i] == '|' ||
                      ln[i] == '&' || ln[i] == '&' || ln[i] == '|' ||
                      ln[i] == ';') {
                    val.replace(val.find(val_reg), val_reg.size(),
                                "r" + std::to_string(reg_cnt));
                    val_reg.clear();
                    ++reg_cnt;

                    continue;
                  }
                } catch (...) {
                }

                if (isalnum(ln[i])) val_reg += ln[i];
              }

              while (val.find(" ") != std::string::npos)
                val.erase(val.find(" "), 1);

              leaf.fUserValue.replace(leaf.fUserValue.find("%s"), strlen("%s"),
                                      val);
            }
          }
        } else {
          leaf.fUserValue.replace(leaf.fUserValue.find("%s"), strlen("%s"),
                                  "0");
        }

        continue;
      }

      lines.emplace_back(leaf.fUserData);
    }

    for (auto& leaf : kState.fSyntaxTree->fLeafList) {
      (*kState.fOutputAssembly) << leaf.fUserValue;
    }

    kState.fSyntaxTree = nullptr;

    kState.fOutputAssembly->flush();
    kState.fOutputAssembly.reset();

    return kOk;
  }
};

/////////////////////////////////////////////////////////////////////////////////////////

static void cxx_print_help() { kSplashCxx(); }

/////////////////////////////////////////////////////////////////////////////////////////

#define kExtListCxx \
  { ".cpp", ".cxx", ".cc", ".c++" }

MPCC_MODULE(CompilerCPlusPlus64x0) {
  kKeywords.emplace_back("auto");
  kKeywords.emplace_back("else");
  kKeywords.emplace_back("break");
  kKeywords.emplace_back("switch");
  kKeywords.emplace_back("enum");
  kKeywords.emplace_back("register");
  kKeywords.emplace_back("do");
  kKeywords.emplace_back("return");
  kKeywords.emplace_back("if");
  kKeywords.emplace_back("default");
  kKeywords.emplace_back("struct");
  kKeywords.emplace_back("_Packed");
  kKeywords.emplace_back("extern");
  kKeywords.emplace_back("volatile");
  kKeywords.emplace_back("static");
  kKeywords.emplace_back("for");
  kKeywords.emplace_back("class");
  kKeywords.emplace_back("{");
  kKeywords.emplace_back("}");
  kKeywords.emplace_back("(");
  kKeywords.emplace_back(")");
  kKeywords.emplace_back("char");
  kKeywords.emplace_back("int");
  kKeywords.emplace_back("short");
  kKeywords.emplace_back("long");
  kKeywords.emplace_back("float");
  kKeywords.emplace_back("double");
  kKeywords.emplace_back("unsigned");
  kKeywords.emplace_back("__attribute__");
  kKeywords.emplace_back("namespace");
  kKeywords.emplace_back("while");
  kKeywords.emplace_back("sizeof");
  kKeywords.emplace_back("private");
  kKeywords.emplace_back("->");
  kKeywords.emplace_back(".");
  kKeywords.emplace_back("::");
  kKeywords.emplace_back("*");
  kKeywords.emplace_back("+");
  kKeywords.emplace_back("-");
  kKeywords.emplace_back("/");
  kKeywords.emplace_back("=");
  kKeywords.emplace_back("==");
  kKeywords.emplace_back("!=");
  kKeywords.emplace_back(">=");
  kKeywords.emplace_back("<=");
  kKeywords.emplace_back(">");
  kKeywords.emplace_back("<");
  kKeywords.emplace_back(":");
  kKeywords.emplace_back(",");
  kKeywords.emplace_back(";");
  kKeywords.emplace_back("&");
  kKeywords.emplace_back("public");
  kKeywords.emplace_back("protected");

  bool skip = false;

  kFactory.Mount(new AssemblyMountpointClang());
  kCompilerBackend = new CompilerBackendClang();

  for (auto index = 1UL; index < argc; ++index) {
    if (argv[index][0] == '-') {
      if (skip) {
        skip = false;
        continue;
      }

      if (strcmp(argv[index], "-v") == 0 ||
          strcmp(argv[index], "--version") == 0) {
        kSplashCxx();
        return kOk;
      }

      if (strcmp(argv[index], "-verbose") == 0) {
        kState.kVerbose = true;

        continue;
      }

      if (strcmp(argv[index], "-h") == 0 ||
          strcmp(argv[index], "--help") == 0) {
        cxx_print_help();

        return kOk;
      }

      if (strcmp(argv[index], "-pdialect") == 0) {
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

      std::string err = "Unknown option: ";
      err += argv[index];

      detail::print_error(err, "ccplus");

      continue;
    }

    kFileList.emplace_back(argv[index]);

    std::string argv_i = argv[index];

    std::vector exts = kExtListCxx;
    bool found = false;

    for (std::string ext : exts) {
      if (argv_i.find(ext) != std::string::npos) {
        found = true;
        break;
      }
    }

    if (!found) {
      if (kState.kVerbose) {
        std::cerr << argv[index] << " is not a valid C++ source.\n";
      }
    }

    if (kFactory.Compile(argv_i, kMachine) != kOk) return -1;
  }

  return kOk;
}

// Last rev 8-1-24
