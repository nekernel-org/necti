/*
 *	========================================================
 *
 *	cc
 * 	Copyright WestCo, all rights reserved.
 *
 * 	========================================================
 */

#include <cstdio>
#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <C++Kit/AsmKit/Arch/NewCPU.hpp>
#include <C++Kit/ParserKit.hpp>

#define kOk 0

/* WestCo C driver */
/* This is part of MP-UX C SDK. */
/* (c) WestCo */

/////////////////////

// ANSI ESCAPE CODES

/////////////////////

#define kBlank "\e[0;30m"
#define kRed "\e[0;31m"
#define kWhite "\e[0;97m"

/////////////////////////////////////

// INTERNAL STUFF OF THE C COMPILER

/////////////////////////////////////

namespace detail
{
    struct CompilerRegisterMap
    {
        std::string fName;
        std::string fRegister;
    };

    struct CompilerState
    {
        std::vector<ParserKit::SyntaxLeafList> fSyntaxTreeList;
        std::vector<CompilerRegisterMap> kStackFrame;
        ParserKit::SyntaxLeafList* fSyntaxTree{ nullptr };
        std::unique_ptr<std::ofstream> fOutputAssembly;
        std::string fLastFile;
        std::string fLastError;
        bool kVerbose;
    };
}

static detail::CompilerState kState;
static SizeType kErrorLimit = 100;

static Int32 kAcceptableErrors = 0;

namespace detail
{
    void print_error(std::string reason, std::string file) noexcept
    {
        if (reason[0] == '\n')
            reason.erase(0, 1);

        if (file.find(".pp") != std::string::npos)
        {
            file.erase(file.find(".pp"), 3);
        }

        if (kState.fLastFile != file)
        {
            std::cout << kRed << "[ cc ] " << kWhite << ((file == "cc") ? "internal compiler error " : ("in file, " + file)) << kBlank << std::endl;
            std::cout << kRed << "[ cc ] " << kWhite << reason << kBlank << std::endl;

            kState.fLastFile = file;
        }
        else
        {
            std::cout << kRed << "[ cc ] [ " << kState.fLastFile <<  " ] " << kWhite << reason << kBlank << std::endl;
        }

        if (kAcceptableErrors > kErrorLimit)
            std::exit(3);

        ++kAcceptableErrors;
    }

    struct CompilerType
    {
        std::string fName;
        std::string fValue;
    };
}

/////////////////////////////////////////////////////////////////////////////////////////

// Target architecture.
static int kMachine = 0;

/////////////////////////////////////////

// REGISTERS ACCORDING TO USED ASSEMBLER

/////////////////////////////////////////

static size_t kRegisterCnt = kAsmRegisterLimit;
static size_t kStartUsable = 1;
static size_t kUsableLimit = 14;
static size_t kRegisterCounter = kStartUsable;
static std::string kRegisterPrefix = kAsmRegisterPrefix;

/////////////////////////////////////////

// COMPILER PARSING UTILITIES/STATES.

/////////////////////////////////////////

static std::vector<std::string> kFileList;
static CxxKit::AssemblyFactory kFactory;
static bool kInStruct = false;
static bool kOnWhileLoop = false;
static bool kOnForLoop = false;
static bool kInBraces = false;
static size_t kBracesCount = 0UL;

/* @brief C compiler backend for WestCo C */
class CompilerBackendClang final : public ParserKit::CompilerBackend
{
public:
    explicit CompilerBackendClang() = default;
    ~CompilerBackendClang() override = default;

    CXXKIT_COPY_DEFAULT(CompilerBackendClang);

    std::string Check(const char* text, const char* file);
    void Compile(const char* text, const char* file) override;

    const char* Language() override { return "C"; }

};

static CompilerBackendClang* kCompilerBackend = nullptr;
static std::vector<detail::CompilerType> kCompilerVariables;
static std::vector<std::string>          kCompilerFunctions;
static std::vector<detail::CompilerType> kCompilerTypes;

// @brief this hook code before the begin/end command.
static std::string kAddIfAnyBegin;
static std::string kAddIfAnyEnd;
static std::string kLatestVar;

static std::string cc_parse_function_call(std::string& _text)
{
    if (_text[0] == '(') {
        std::string substr;
        std::string args_buffer;
        std::string args;

        bool type_crossed = false;

        for (char substr_first_index: _text)
        {
            args_buffer += substr_first_index;

            if (substr_first_index == ';')
            {
                args_buffer = args_buffer.erase(0, args_buffer.find('('));
                args_buffer = args_buffer.erase(args_buffer.find(';'), 1);
                args_buffer = args_buffer.erase(args_buffer.find(')'), 1);
                args_buffer = args_buffer.erase(args_buffer.find('('), 1);

                if (!args_buffer.empty())
                    args += "\tpsh ";

                while (args_buffer.find(',') != std::string::npos)
                {
                    args_buffer.replace(args_buffer.find(','), 1, "\n\tpsh ");
                }

                args += args_buffer;
                args += "\n\tjlr __import ";
            }
        }

        return args;
    }

    return "";
}

#include <uuid/uuid.h>

namespace detail
{
    union number_type
    {
        number_type(UInt64 raw)
                : raw(raw)
        {}

        char number[8];
        UInt64 raw;
    };
}

// @name kAssignOpTypes
// @brief assignop types

std::vector<std::string> kAssignOpTypes = { "int", "float", 
"double", "long", 
"short", "unsigned" };

std::vector<std::string> kControlKeyword = { "if", "else", "while", "for", "struct", "enum"
                                            "union", "typedef"};

/////////////////////////////////////////////////////////////////////////////////////////

// @name Compile
// @brief Generate MASM from a C assignement.

/////////////////////////////////////////////////////////////////////////////////////////

void CompilerBackendClang::Compile(const char* text, const char* file)
{
    if (!strstr(text, "="))
        return;

    for (auto& key : kControlKeyword)
    {
        if (strstr(text, key.c_str()))
            return;
    }

    ParserKit::SyntaxLeafList::SyntaxLeaf leaf{};
    bool decl = false;

    std::string substr = text;

    for (auto& builtin: kAssignOpTypes)
    {
        if (strstr(text, builtin.c_str()))
        {
            decl = true;
            substr = strstr(text, builtin.c_str());

            if (substr.find(builtin) != std::string::npos)
            {
                substr.erase(substr.find(builtin), builtin.size());
            }

            break;
        }
    }
    
    if (substr.find(";") != std::string::npos)
    {
        substr.erase(substr.find(";"), 1);
    }

    if (substr.find(",") != std::string::npos)
    {
        substr.erase(substr.find(","), 1);
    }

    std::string val;

    if (substr.find('=') != std::string::npos)
    {
        val = substr.substr(substr.find('=') + 1);
        substr.erase(substr.find('='), 1);
    }

    if (substr.find(val) != std::string::npos)
    {
        substr.erase(substr.find(val), val.size());
    }

    while (substr.find(' ') != std::string::npos)
    {
        substr.erase(substr.find(' '), 1);
    }

    leaf.fUserValue += "stw ";

    std::string register_nc = "r";
    register_nc += std::to_string(kRegisterCounter);

    ++kRegisterCounter;

    leaf.fUserValue += register_nc + ",";
    leaf.fUserValue += substr + "," + val + "\n";


    kState.fSyntaxTree->fLeafList.push_back(leaf);
}

static bool kShouldHaveBraces = false;
static std::string kFnName;

std::string CompilerBackendClang::Check(const char* text, const char* file)
{
    std::string err_str;
    std::string ln = text;

    if (ln.empty())
    {
        return err_str;
    }

    bool non_ascii_found = false;

    for (int i = 0; i < ln.size(); ++i) {
        if (isalnum(ln[i]))
        {
            non_ascii_found = true;
            break;
        }
    }

    if (kShouldHaveBraces &&
        ln.find('{') != std::string::npos) {
        kShouldHaveBraces = false;
    }

    if (!non_ascii_found)
        return err_str;

    size_t string_index = 1UL;

    if (ln.find('\'') != std::string::npos)
    {
        string_index = ln.find('\'') + 1;

        for (; string_index < ln.size(); ++string_index)
        {
            if (ln[string_index] == '\'')
            {
                if (ln[string_index + 1] != ';')
                {
                    ln.erase(string_index, 1);
                }

                return err_str;
            }
        }
    }
    else if (ln.find('"') != std::string::npos)
    {
        string_index = ln.find('"') + 1;

        for (; string_index < ln.size(); ++string_index)
        {
            if (ln[string_index] == '"')
            {
                if (ln[string_index + 1] != ';')
                {
                    ln.erase(string_index, 1);
                }
                else
                {
                    break;
                }
            }
        }
    }
    else if (ln.find('"') == std::string::npos &&
            ln.find('\'') == std::string::npos)
    {
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

        for (auto& forbidden : forbidden_words)
        {
            if (ParserKit::find_word(ln, forbidden))
            {
                err_str += "\nForbidden character detected: ";
                err_str += forbidden;

                return err_str;
            }
        }
    }

    struct CompilerVariableRange final
    {
        std::string fBegin;
        std::string fEnd;
    };

    const std::vector<CompilerVariableRange> variables_list = {
        { .fBegin = "static ", .fEnd = "="}, 
        { .fBegin = "=", .fEnd = ";"},
        { .fBegin = "if(",  .fEnd = "="},
        { .fBegin = "if (", .fEnd = "="},
        { .fBegin = "if(",  .fEnd = "<"},
        { .fBegin = "if (", .fEnd = "<"},
        { .fBegin = "if(",  .fEnd = ">"},
        { .fBegin = "if (", .fEnd = ">"},
        { .fBegin = "if(",  .fEnd = ")"},
        { .fBegin = "if (", .fEnd = ")"},

        { .fBegin = "else(",  .fEnd = "="},
        { .fBegin = "else (", .fEnd = "="},
        { .fBegin = "else(",  .fEnd = "<"},
        { .fBegin = "else (", .fEnd = "<"},
        { .fBegin = "else(",  .fEnd = ">"},
        { .fBegin = "else (", .fEnd = ">"},
        { .fBegin = "else(",  .fEnd = ")"},
        { .fBegin = "else (", .fEnd = ")"},
    };

    for (auto& variable : variables_list)
    {
        if (ln.find(variable.fBegin) != std::string::npos)
        {
            string_index = ln.find(variable.fBegin) + variable.fBegin.size();

            while (ln[string_index] == ' ')
                ++string_index;

            std::string keyword;

            for (; string_index < ln.size(); ++string_index)
            {
                if (ln[string_index] == variable.fEnd[0])
                {
                    std::string varname = "";

                    for (size_t index_keyword = ln.find(' '); ln[index_keyword] != variable.fBegin[0]; 
                        ++index_keyword)
                    {
                        if (ln[index_keyword] == ' ')
                        {
                            continue;
                        }

                        if (isdigit(ln[index_keyword]))
                        {
                            goto cc_next_loop;
                        }

                        varname += ln[index_keyword];
                    }
                    
                    if (varname.find(' ') != std::string::npos)
                    {
                        varname.erase(0, varname.find(' '));

                        if (variable.fBegin == "extern")
                        {
                            varname.erase(0, varname.find(' '));
                        }
                    }

                    std::string reg = kAsmRegisterPrefix;
                    reg += std::to_string(kRegisterCounter);
                    
                    kCompilerVariables.push_back({ .fValue = varname });
                    goto cc_check_done;
                }

                keyword.push_back(ln[string_index]);
            }
            
            goto cc_next_loop;

        cc_check_done:

            // skip digit value.
            if (isdigit(keyword[0]) ||
                keyword[0] == '"')
            {
                goto cc_next_loop;
            }

            while (keyword.find(' ') != std::string::npos)
                keyword.erase(keyword.find(' '), 1);

            for (auto& var : kCompilerVariables)
            {
                if (var.fValue.find(keyword) != std::string::npos)
                {
                    err_str.clear();
                    goto cc_next;
                }
            }

            for (auto& fn : kCompilerFunctions)
            {
                if (fn.find(keyword[0]) != std::string::npos)
                {
                    auto where_begin = fn.find(keyword[0]);
                    auto keyword_begin = 0UL;
                    auto failed = false;

                    for (; where_begin < keyword.size(); ++where_begin)
                    {
                        if (fn[where_begin] == '(' &&
                            keyword[keyword_begin] == '(')
                            break;

                        if (fn[where_begin] != keyword[keyword_begin])
                        {
                            failed = true;
                            break;
                        }

                        ++keyword_begin;
                    }

                    if (!failed)
                    {
                        err_str.clear();
                        goto cc_next;
                    }
                    else
                    {
                        continue;
                    }
                }
            }

cc_error_value:
            if (keyword.find("->") != std::string::npos)
                return err_str;

            if (keyword.find(".") != std::string::npos)
                return err_str;

            
            if (isalnum(keyword[0]))
                err_str += "\nUndefined value: " + keyword;

            return err_str;
        }

cc_next_loop:
        continue;
    }

cc_next:

    // extern doesnt declare anything, it imports a variable.
    // so that's why it's not declare upper.
    if (ParserKit::find_word(ln, "extern"))
    {
        auto substr = ln.substr(ln.find("extern") + strlen("extern"));
        kCompilerVariables.push_back({ .fValue = substr });
    }

    if (kShouldHaveBraces &&
        ln.find('{') == std::string::npos)
    {
        err_str += "Missing '{' for function ";
        err_str += kFnName;
        err_str += "\n";

        kShouldHaveBraces = false;
        kFnName.clear();
    }
    else if (kShouldHaveBraces &&
             ln.find('{') != std::string::npos)
    {
        kShouldHaveBraces = false;
        kFnName.clear();
    }

    bool type_not_found = true;

    if (ln.find('\'') != std::string::npos)
    {
        ln.replace(ln.find('\''), 3, "0");
    }

    auto first = ln.find('"');
    if (first != std::string::npos)
    {
        auto second = 0UL;
        bool found_second_quote = false;

        for (size_t i = first + 1; i < ln.size(); ++i)
        {
            if (ln[i] == '\"')
            {
                found_second_quote = true;
                second = i;

                break;
            }
        }

        if (!found_second_quote)
        {
            err_str += "Missing terminating \".";
            err_str += " here -> " + ln.substr(ln.find('"'), second);
        }
    }

    if (ln.find(')') != std::string::npos &&
        ln.find(';') == std::string::npos)
    {
        if (ln.find('{') == std::string::npos)
        {
            kFnName = ln;
            kShouldHaveBraces = true;

            goto skip_braces_check;
        }
        else if (ln.find('{') != std::string::npos)
        {
            kShouldHaveBraces = false;
        }
    }

skip_braces_check:

    for (auto& key : kCompilerTypes)
    {
        if (ParserKit::find_word(ln, key.fName))
        {
            if (isdigit(ln[ln.find(key.fName) + key.fName.size() + 1]))
            {
                err_str += "\nNumber cannot be set for ";
                err_str += key.fName;
                err_str += "'s name. here -> ";
                err_str += ln;
            }

            if (ln.find(key.fName) == 0 ||
                ln[ln.find(key.fName) - 1] == ' ' ||
                ln[ln.find(key.fName) - 1] == '\t')
            {
                type_not_found = false;

                if (ln[ln.find(key.fName) + key.fName.size()] != ' ')
                {
                    type_not_found = true;

                    if (ln[ln.find(key.fName) + key.fName.size()] == '\t')
                        type_not_found = false;

                    goto next;
                }
                else if (ln[ln.find(key.fName) + key.fName.size()] != '\t')
                {
                    type_not_found = true;

                    if (ln[ln.find(key.fName) + key.fName.size()] == ' ')
                        type_not_found = false;

                }
            }

next:

            if (key.fName != "struct" ||
                key.fName != "enum" ||
                key.fName != "union")
            {
                if (ln.find(';') == std::string::npos)
                {
                    if (ln.find('(') != std::string::npos)
                    {
                        if (ln.find('=') == std::string::npos)
                            continue;
                    }

                    err_str += "\nMissing ';', here -> ";
                    err_str += ln;
                }
                else
                {
                    continue;
                }

                if (ln.find('=') != std::string::npos)
                {
                    if (ln.find('(') != std::string::npos)
                    {
                        if (ln.find(')') == std::string::npos)
                        {
                            err_str += "\nMissing ')', after '(' here -> ";
                            err_str += ln.substr(ln.find('('));
                        }
                    }
                }
            }
        }
    }

    if (kInBraces &&
             ln.find("struct") != std::string::npos &&
            ln.find("union") != std::string::npos &&
            ln.find("enum") != std::string::npos &&
            ln.find('=') != std::string::npos)
    {
        if (ln.find(';') == std::string::npos)
        {
            err_str += "\nMissing ';' after struct/union/enum declaration, here -> ";
            err_str += ln;
        }
    }

    if (ln.find(';') != std::string::npos &&
        ln.find("for") == std::string::npos)
    {
        if (ln.find(';') + 1 != ln.size())
        {
            for (int i = 0; i < ln.substr(ln.find(';') + 1).size(); ++i)
            {
                if ((ln.substr(ln.find(';') + 1)[i] != ' ') ||
                    (ln.substr(ln.find(';') + 1)[i] != '\t'))
                {
                    if (auto err = this->Check(ln.substr(ln.find(';') + 1).c_str(), file);
                        !err.empty())
                    {
                        err_str += "\nUnexpected text after ';' -> ";
                        err_str += ln.substr(ln.find(';'));
                        err_str += err;
                    }
                }
            }
        }
    }

    if (ln.find('(') != std::string::npos)
    {
        if (ln.find(';') == std::string::npos &&
            !ParserKit::find_word(ln, "|") &&
            !ParserKit::find_word(ln, "||") &&
            !ParserKit::find_word(ln, "&") &&
            !ParserKit::find_word(ln, "&&") &&
            !ParserKit::find_word(ln, "~"))
        {
            bool found_func = false;
            size_t i = ln.find('(');
            std::vector<char> opens;
            std::vector<char> closes;

            for (; i < ln.size(); ++i)
            {
                if (ln[i] == ')')
                {
                    closes.push_back(1);
                }

                if (ln[i] == '(')
                {
                    opens.push_back(1);
                }
            }

            if (closes.size() != opens.size())
                err_str += "Unterminated (), here -> " + ln;

            bool space_found = false;

            for (int i = 0; i < ln.size(); ++i)
            {
                if (ln[i] == ')' &&
                    !space_found)
                {
                    space_found = true;
                    continue;
                }

                if (space_found)
                {
                    if (ln[i] == ' ' &&
                        isalnum(ln[i+1]))
                    {
                        err_str += "\nBad function format here -> ";
                        err_str += ln;
                    }
                }
            }
        }

        if (ln.find('(') < 1)
        {
            err_str += "\nMissing identifier before '(' here -> ";
            err_str += ln;
        }
        else
        {
            if (type_not_found &&
                ln.find(';') == std::string::npos &&
                ln.find("if") == std::string::npos &&
                ln.find("while") == std::string::npos &&
                ln.find("for") == std::string::npos &&
                ln.find("static") == std::string::npos &&
                ln.find("inline") == std::string::npos &&
                ln.find("|") == std::string::npos &&
                ln.find("&") == std::string::npos)
            {
                err_str += "\n Missing ';' or type, here -> ";
                err_str += ln;
            }
        }

        if (ln.find(')') == std::string::npos)
        {
            err_str += "\nMissing ')', after '(' here -> ";
            err_str += ln.substr(ln.find('('));
        }
    }
    else
    {
        if (ln.find("for") != std::string::npos ||
            ln.find("while") != std::string::npos)
        {
            err_str += "\nMissing '(', after \"for\", here -> ";
            err_str += ln;
        }
    }

    if (ln.find('}') != std::string::npos &&
        !kInBraces)
    {
        if (!kInStruct &&
            ln.find(';') == std::string::npos)
        {
            err_str += "\nMismatched '}', here -> ";
            err_str += ln;
        }
    }

    if (!ln.empty())
    {
        if (ln.find(';') == std::string::npos &&
            ln.find("struct") == std::string::npos &&
            ln.find("enum") == std::string::npos &&
            ln.find("union") == std::string::npos &&
            ln.find("for") == std::string::npos &&
            ln.find("while") == std::string::npos &&
            ln.find('{') == std::string::npos &&
            ln.find('}') == std::string::npos &&
            ln.find(')') == std::string::npos &&
            ln.find('(') == std::string::npos &&
            ln.find(',') == std::string::npos &&
            ln.find("typedef") == std::string::npos)
        {
            if (ln.size() <= 2)
                return err_str;

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

class AssemblyMountpointClang final : public CxxKit::AssemblyMountpoint
{
public:
    explicit AssemblyMountpointClang() = default;
    ~AssemblyMountpointClang() override = default;

    CXXKIT_COPY_DEFAULT(AssemblyMountpointClang);

    [[maybe_unused]] static Int32 Arch() noexcept { return CxxKit::AssemblyFactory::kArchRISCV; }

    Int32 CompileToFormat(CxxKit::StringView& src, Int32 arch) override
    {
        if (arch != AssemblyMountpointClang::Arch())
            return -1;

        if (kCompilerBackend == nullptr)
            return -1;

        /* @brief copy contents wihtout extension */
        std::string src_file = src.CData();
        std::ifstream src_fp = std::ifstream(src_file, std::ios::in);
        std::string dest;

        for (auto& ch : src_file)
        {
            if (ch == '.')
            {
                break;
            }

            dest += ch;
        }

        /* According to pef abi. */
        dest += kAsmFileExt;

        kState.fOutputAssembly = std::make_unique<std::ofstream>(dest);

        auto fmt = CxxKit::current_date();

        (*kState.fOutputAssembly) << "# Path: " << src_file << "\n";
        (*kState.fOutputAssembly) << "# Language: MP-UX Assembly\n";
        (*kState.fOutputAssembly) << "# Build Date: " << fmt << "\n\n";

        ParserKit::SyntaxLeafList syntax;

        kState.fSyntaxTreeList.push_back(syntax);
        kState.fSyntaxTree = &kState.fSyntaxTreeList[kState.fSyntaxTreeList.size() - 1];

        std::string line_src;

        while (std::getline(src_fp, line_src))
        {
            if (auto err = kCompilerBackend->Check(line_src.c_str(), src.CData());
                err.empty())
            {
                kCompilerBackend->Compile(line_src.c_str(), src.CData());
            }
            else
            {
                detail::print_error(err, src.CData());
            }
        }

        if (kAcceptableErrors > 0)
            return -1;

        for (auto& leaf : kState.fSyntaxTree->fLeafList)
        {
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
#define kSplashCxx() kPrintF(kWhite "%s\n", "X64000 C compiler, v1.13, (c) WestCo")

static void cc_print_help()
{
    kSplashCxx();
    kPrintF(kWhite "--asm={MACHINE}: %s\n", "Compile to a specific assembler syntax. (masm)");
    kPrintF(kWhite "--compiler={COMPILER}: %s\n", "Select compiler engine (builtin -> dolvik).");
}

/////////////////////////////////////////////////////////////////////////////////////////

#define kExt ".c"

int main(int argc, char** argv)
{
    kCompilerTypes.push_back({ .fName = "void", .fValue = "void" });
    kCompilerTypes.push_back({ .fName = "char", .fValue = "byte" });
    kCompilerTypes.push_back({ .fName = "short", .fValue = "hword" });
    kCompilerTypes.push_back({ .fName = "int", .fValue = "dword" });
    kCompilerTypes.push_back({ .fName = "long", .fValue = "qword" });

    bool skip = false;

    for (auto index = 1UL; index < argc; ++index)
    {
        if (skip)
        {
            skip = false;
            continue;
        }

        if (argv[index][0] == '-')
        {
            if (strcmp(argv[index], "-v") == 0 ||
                strcmp(argv[index], "--version") == 0)
            {
                kSplashCxx();
                return kOk;
            }

            if (strcmp(argv[index], "--verbose") == 0)
            {
                kState.kVerbose = true;

                continue;
            }

            if (strcmp(argv[index], "-h") == 0 ||
                strcmp(argv[index], "--help") == 0)
            {
                cc_print_help();

                return kOk;
            }

            if (strcmp(argv[index], "--dialect") == 0)
            {
                if (kCompilerBackend)
                    std::cout << kCompilerBackend->Language() << "\n";

                return kOk;
            }

            if (strcmp(argv[index], "--asm=masm") == 0)
            {
                delete kFactory.Unmount();

                kFactory.Mount(new AssemblyMountpointClang());
                kMachine = CxxKit::AssemblyFactory::kArchRISCV;

                continue;
            }

            if (strcmp(argv[index], "--compiler=dolvik") == 0)
            {
                if (!kCompilerBackend)
                    kCompilerBackend = new CompilerBackendClang();

                continue;
            }

            if (strcmp(argv[index], "-fmax-exceptions") == 0)
            {
                try
                {
                    kErrorLimit = std::strtol(argv[index + 1], nullptr, 10);
                }
                    // catch anything here
                catch (...)
                {
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

        CxxKit::StringView srcFile = CxxKit::StringBuilder::Construct(argv[index]);

        if (strstr(argv[index], kExt) == nullptr)
        {
            if (kState.kVerbose)
            {
                std::cerr << argv[index] << " is not a valid C line_src.\n";
            }

            return -1;
        }

        if (kFactory.Compile(srcFile, kMachine) != kOk)
            return -1;
    }

    return kOk;
}
