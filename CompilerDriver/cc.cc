/*
 *	========================================================
 *
 *	cc
 * 	Copyright Western Company, all rights reserved.
 *
 * 	========================================================
 */

#include <cstdio>
#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <uuid/uuid.h>
#include <C++Kit/AsmKit/Arch/64k.hpp>
#include <C++Kit/ParserKit.hpp>

#define kOk 0

/* Optimized C driver */
/* This is part of MP-UX C SDK. */
/* (c) Western Company */

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
static size_t kStartUsable = 6;
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

/* @brief C compiler backend for Optimized C */
class CompilerBackendClang final : public ParserKit::CompilerBackend
{
public:
    explicit CompilerBackendClang() = default;
    ~CompilerBackendClang() override = default;

    CXXKIT_COPY_DEFAULT(CompilerBackendClang);

    std::string Check(const char* text, const char* file);
    bool Compile(const std::string& text, const char* file) override;

    const char* Language() override { return "Optimized 64x0 C"; }

};

static CompilerBackendClang* kCompilerBackend = nullptr;
static std::vector<detail::CompilerType> kCompilerVariables;
static std::vector<std::string>          kCompilerFunctions;
static std::vector<detail::CompilerType> kCompilerTypes;

// @brief this hook code before the begin/end command.
static std::string kAddIfAnyBegin;
static std::string kAddIfAnyEnd;
static std::string kLatestVar;

// \brief parse a function call
static std::string cc_parse_function_call(std::string& _text)
{
    if (_text[0] == '(') 
    {
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
                args += "\n\tjb __import ";
            }
        }

        return args;
    }

    return "";
}

namespace detail
{
    union number_cast
    {
        number_cast(UInt64 raw)
                : raw(raw)
        {}

        char number[8];
        UInt64 raw;
    };
}

/////////////////////////////////////////////////////////////////////////////////////////

// @name Compile
// @brief Generate MASM from a C assignement.

/////////////////////////////////////////////////////////////////////////////////////////

bool CompilerBackendClang::Compile(const std::string& text, const char* file)
{
    std::string _text = text;

    auto syntax_tree = ParserKit::SyntaxLeafList::SyntaxLeaf();
    bool type_found = false;
    bool function_found = false;

    // start parsing
    for (size_t text_index = 0; text_index < _text.size(); ++text_index)
    {
        uuid_t out{0};

        uuid_generate_random(out);
        detail::number_cast time_off = (UInt64)out;

        if (!type_found)
        {
            auto substr = _text.substr(text_index);
            std::string match_type;

            for (size_t y = 0; y < substr.size(); ++y)
            {
                if (substr[y] == ' ')
                {
                    while (match_type.find(' ') != std::string::npos) {
                        match_type.erase(match_type.find(' '));
                    }

                    for (auto& clType : kCompilerTypes)
                    {
                        if (clType.fName == match_type)
                        {
                            match_type.clear();

                            std::string buf;

                            buf += clType.fValue;
                            buf += ' ';

                            if (clType.fName == "struct" ||
                                clType.fName == "union")
                            {
                                for (size_t a = y + 1; a < substr.size(); a++)
                                {
                                    if (substr[a] == ' ')
                                    {
                                        break;
                                    }

                                    if (substr[a] == '\n')
                                        break;

                                    buf += substr[a];
                                }
                            }

                            if (substr.find('=') != std::string::npos)
                            {
                                break;
                            }

                            if (_text.find('(') != std::string::npos)
                            {
                                syntax_tree.fUserValue = buf;

                                kState.fSyntaxTree->fLeafList.push_back(syntax_tree);
                            }

                            type_found = true;
                            break;
                        }
                    }

                    break;
                }

                match_type += substr[y];
            }
        }

        if (_text[text_index] == '{')
        {
            if (kInStruct)
            {
                continue;
            }

            kInBraces = true;
            ++kBracesCount;

            if (kOnWhileLoop ||
                kOnForLoop)
            {
                syntax_tree.fUserValue = "void __export .text _L";
                syntax_tree.fUserValue += std::to_string(kBracesCount) + "_" + std::to_string(time_off.raw);
            }

            kState.fSyntaxTree->fLeafList.push_back(syntax_tree);
        }

        // return keyword handler
        if (_text[text_index] == 'r')
        {
            std::string return_keyword;
            return_keyword += "return";

            std::size_t index = 0UL;

            std::string value;

            for (size_t return_index = text_index; return_index < _text.size(); ++return_index)
            {
                if (_text[return_index] != return_keyword[index])
                {
                    for (size_t value_index = return_index; value_index < _text.size(); ++value_index)
                    {
                        if (_text[value_index] == ';')
                            break;

                        value += _text[value_index];
                    }

                    break;
                }

                ++index;
            }

            if (index == return_keyword.size())
            {
                if (!value.empty())
                {
                    if (value.find('(') != std::string::npos)
                    {
                        value.erase(value.find('('));
                    }

                    if (!isdigit(value[value.find('(') + 2]))
                    {
                        std::string tmp = value;
                        bool reg_to_reg = false;
                        
                        value.clear();

                        value += " __import";
                        value += tmp;
                    }
                    
                    syntax_tree.fUserValue = "\tldw r19, ";

                    // make it pretty.
                    if (value.find('\t') != std::string::npos)
                        value.erase(value.find('\t'), 1);

                    syntax_tree.fUserValue += value + "\n";
                }

                syntax_tree.fUserValue += "\tjlr";

                kState.fSyntaxTree->fLeafList.push_back(syntax_tree);

                break;
            }
        }

        if (_text[text_index] == 'i' &&
            _text[text_index + 1] == 'f')
        {
            std::string format = "ldw r15, %s\nldw r16, %s2\n";
            std::string expr = format;

            if (ParserKit::find_word(_text, "=="))
            {
                expr += "\nbeq";
            }
            
            if (ParserKit::find_word(_text, "!="))
            {
                expr += "\nbneq";
            }
            
            if (ParserKit::find_word(_text, ">="))
            {
                expr += "\nbge";
            }
            else if (ParserKit::find_word(_text, ">"))
            {
                expr += "\nbg";
            }

            if (ParserKit::find_word(_text, "<="))
            {
                expr += "\nble";
            }
            else if (ParserKit::find_word(_text, "<"))
            {
                expr += "\nbl";
            }

            std::string substr = expr;

            std::string buf;

            for (size_t text_index_2 = (_text.find("if") + std::string("if").size()); text_index_2 < _text.size(); ++text_index_2)
            {
                if (_text[text_index_2] == ';')
                {
                    buf.clear();

                    for (size_t text_index_3 = text_index_2 + 1; text_index_3 < _text.size(); text_index_3++)
                    {
                        if (_text[text_index_3] == '{')
                            continue;

                        if (_text[text_index_3] == '}')
                            continue;

                        if (_text[text_index_3] == ' ')
                            continue;

                        if (_text[text_index_3] == '=')
                            continue;

                        if (_text[text_index_3] == '<' &&
                            _text[text_index_3+1] == '=' ||
                            _text[text_index_3] == '=' &&
                            _text[text_index_3+1] == '=' ||
                            _text[text_index_3] == '>' &&
                            _text[text_index_3+1] == '=' ||
                            _text[text_index_3] == '>' ||
                            _text[text_index_3] == '<' &&
                            _text[text_index_3+1] == '=' ||
                            _text[text_index_3] == '!')
                        {
                            buf += ", ";
                            continue;
                        }
                        else if (_text[text_index_3] == '=')
                        {
                            continue;
                        }

                        buf += _text[text_index_3];
                    }

                    break;
                }

                if (_text[text_index_2] == '{')
                    continue;

                if (_text[text_index_2] == '}')
                    continue;

                if (_text[text_index_2] == '<' &&
                    _text[text_index_2+1] == '=' ||
                    _text[text_index_2] == '=' &&
                    _text[text_index_2+1] == '=' ||
                    _text[text_index_2] == '>' &&
                    _text[text_index_2+1] == '=' ||
                    _text[text_index_2] == '>' ||
                    _text[text_index_2] == '<' &&
                    _text[text_index_2+1] == '=' ||
                    _text[text_index_2] == '!')
                {
                    buf += ", ";
                    continue;
                }
                else if (_text[text_index_2] == '=')
                {
                    continue;
                }

                buf += _text[text_index_2];
            }

            if (buf.find(",") == std::string::npos &&
                buf.find("(") != std::string::npos &&
                buf.find(")") != std::string::npos )
            {

                std::string cond = buf.substr(buf.find("(") + 1, buf.find(")") - 1);
                cond.erase(cond.find("("));

                std::string cond2 = buf.substr(buf.find("(") + 1, buf.find(")") - 1);
                cond2.erase(cond2.find(")"));

                substr.replace(substr.find("%s"), 2, cond);
                substr.replace(substr.find("%s2"), 3, cond2);

                buf.replace(buf.find(cond), cond.size(), "r15");
                buf.replace(buf.find(cond2), cond2.size(), "r16");

                substr += buf;

                syntax_tree.fUserValue = substr + "\n";

                kState.fSyntaxTree->fLeafList.push_back(syntax_tree);

                break;
            }
            else
            {
                continue;
            }

            // dealing with pointer
            if (buf.find("*") != std::string::npos)
            {
                buf.erase(buf.find("*"), 1);
            }

            std::string cond = buf.substr(buf.find("(") + 1, buf.find(",") - 1);
            cond.erase(cond.find(","));

            std::string cond2 = buf.substr(buf.find(",") + 1, buf.find(")") - 1);
            cond2.erase(cond2.find(")"));

            substr.replace(substr.find("%s"), 2, cond);
            substr.replace(substr.find("%s2"), 3, cond2);
            
            buf.replace(buf.find(cond), cond.size(), "r15");
            buf.replace(buf.find(cond2), cond2.size(), "r16");

            substr += buf;

            syntax_tree.fUserValue = substr + "\n";

            kState.fSyntaxTree->fLeafList.push_back(syntax_tree);

            break;
        }

        // Parse expressions and instructions here.
        // what does this mean?
        // we encounter an assignment, or we reached the end of an expression.
        if (_text[text_index] == '=' ||
            _text[text_index] == ';')
        {
            if (function_found)
                continue;

            if (_text[text_index] == ';' &&
                kInStruct)
                continue;

            if (_text.find("typedef ") != std::string::npos)
                continue;

            if (_text[text_index] == '=' &&
                kInStruct)
            {
                continue;
            }

            if (_text[text_index+1] == '=' ||
                _text[text_index-1] == '!' ||
                _text[text_index-1] == '<' ||
                _text[text_index-1] == '>')
            {
                continue;
            }

            std::string substr;

            if (_text.find('=') != std::string::npos &&
                kInBraces)
            {
                if (_text.find("*") != std::string::npos)
                {
                    if (_text.find("=") > _text.find("*"))
                        substr += "\tlda ";
                    else
                        substr += "\tldw ";
                }
                else
                {
                    substr += "\tldw ";
                }
            }
            else if (_text.find('=') != std::string::npos &&
                !kInBraces)
            {
                substr += "stw __export .data ";
            }

            int first_encountered = 0;

            std::string str_name;

            for (size_t text_index_2 = 0; text_index_2 < _text.size(); ++text_index_2)
            {
                if (_text[text_index_2] == '\"')
                {
                    ++text_index_2;

                    // want to add this, so that the parser recognizes that this is a string.
                    substr += '"';

                    for (; text_index_2 < _text.size(); ++text_index_2)
                    {
                        if (_text[text_index_2] == '\"')
                            break;

                        kLatestVar += _text[text_index_2];
                        substr += _text[text_index_2];
                    }
                }

                if (_text[text_index_2] == '{' ||
                    _text[text_index_2] == '}')
                    continue;

                if (_text[text_index_2] == ';')
                {
                    break;
                }

                if (_text[text_index_2] == ' ' ||
                    _text[text_index_2] == '\t')
                {
                    if (first_encountered != 2)
                    {
                        if (_text[text_index] != '=' &&
                            substr.find("__export .data") == std::string::npos &&
                            !kInStruct &&
                            _text.find("struct") == std::string::npos &&
                            _text.find("extern") == std::string::npos &&
                             _text.find("union") == std::string::npos &&
                             _text.find("typedef") == std::string::npos)
                            substr += "__export .data ";
                    }

                    ++first_encountered;

                    continue;
                }

                if (_text[text_index_2] == '=')
                {
                    if (!kInBraces)
                    {
                        substr.replace(substr.find("__export .data"), strlen("__export .data"), "__export .page_zero ");
                    }

                    substr += ",";
                    continue;
                }

                kLatestVar += _text[text_index_2];
                substr += _text[text_index_2];
            }

            for (auto& clType : kCompilerTypes)
            {
                if (substr.find(clType.fName) != std::string::npos)
                {
                    if (substr.find(clType.fName) > substr.find('"'))
                        continue;

                    substr.erase(substr.find(clType.fName), clType.fName.size());
                }
                else if (substr.find(clType.fValue) != std::string::npos)
                {
                    if (substr.find(clType.fValue) > substr.find('"'))
                        continue;

                    if (clType.fName == "const")
                        continue;

                    substr.erase(substr.find(clType.fValue), clType.fValue.size());
                }
            }

            if (substr.find("struct") != std::string::npos)
            {
                substr.replace(substr.find("struct"), strlen("struct"), "ldw ");
                substr += ", 0";
            }

            if (substr.find("union") != std::string::npos)
            {
                substr.replace(substr.find("union"), strlen("union"), "ldw ");
                substr += ", 0";
            }

            if (substr.find("static") != std::string::npos)
            {
                substr.replace(substr.find("static"), strlen("static"), "__export .data ");
            }
            else if (substr.find("extern") != std::string::npos)
            {
                substr.replace(substr.find("extern"), strlen("extern"), "__import ");

                if (substr.find("__export .data") != std::string::npos)
                    substr.erase(substr.find("__export .data"), strlen("__export .data"));
            }
            
            auto var_to_find = std::find_if(kCompilerVariables.cbegin(), kCompilerVariables.cend(), [&](detail::CompilerType type) {
                return type.fName.find(substr) != std::string::npos;
            });

            std::string reg = kAsmRegisterPrefix;
            reg += std::to_string(kRegisterCounter);

            if (var_to_find == kCompilerVariables.cend())
            {
                ++kRegisterCounter;

                kState.kStackFrame.push_back({ .fName = substr, .fRegister = reg });
                kCompilerVariables.push_back({ .fName = substr });
            }
            
            syntax_tree.fUserValue += substr;
            kState.fSyntaxTree->fLeafList.push_back(syntax_tree);

            if (_text[text_index] == '=')
                break;
        }

        // function handler.

        if (_text[text_index] == '(' &&
            !function_found)
        {
            std::string substr;
            std::string args_buffer;
            std::string args;

            bool type_crossed = false;

            for (size_t idx = _text.find('(') + 1; idx < _text.size(); ++idx)
            {
                if (_text[idx] == ',')
                    continue;

                if (_text[idx] == ' ')
                    continue;

                if (_text[idx] == ')')
                    break;
            }

            for (char substr_first_index : _text)
            {
                args_buffer += substr_first_index;

                if (substr_first_index == ';')
                {
                    args_buffer = args_buffer.erase(0, args_buffer.find('('));
                    args_buffer = args_buffer.erase(args_buffer.find(';'), 1);
                    args_buffer = args_buffer.erase(args_buffer.find(')'), 1);
                    args_buffer = args_buffer.erase(args_buffer.find('('), 1);

                    if (!args_buffer.empty())
                        args += "\tldw r6, ";

                    std::size_t index = 0UL;

                    while (ParserKit::find_word(args_buffer, ","))
                    {
                        std::string register_type = kRegisterPrefix;
                        register_type += std::to_string(index);

                        args_buffer.replace(args_buffer.find(','), 1, "\n\tldw " + register_type + ",");
                    }

                    args += args_buffer;
                    args += "\n\tjb __import ";
                }
            }

            for (char _text_i : _text)
            {
                if (_text_i == '\t' ||
                    _text_i == ' ')
                {
                    if (!type_crossed)
                    {
                        substr.clear();
                        type_crossed = true;
                    }

                    continue;
                }

                if (_text_i == '(')
                    break;

                substr += _text_i;
            }
            
            if (kInBraces)
            {
                syntax_tree.fUserValue = args;

                syntax_tree.fUserValue += substr;

                kState.fSyntaxTree->fLeafList.push_back(syntax_tree);

                function_found = true;
            }
            else
            {
                syntax_tree.fUserValue.clear();

                syntax_tree.fUserValue += "__export .text ";

                syntax_tree.fUserValue += substr;
                syntax_tree.fUserValue += "\n";

                kState.fSyntaxTree->fLeafList.push_back(syntax_tree);

                function_found = true;
            }

            kCompilerFunctions.push_back(_text);
        }

        if (_text[text_index] == 's')
        {
            if (_text.find("struct") != text_index)
                continue;

            if (_text.find(";") == std::string::npos)
                kInStruct = true;
        }

        if (_text[text_index] == 'u')
        {
            if (_text.find("union") != text_index)
                continue;

            if (_text.find(";") == std::string::npos)
                kInStruct = true;
        }

        if (_text[text_index] == 'e')
        {
            if (_text.find("enum") != text_index)
                continue;

            if (_text.find(";") == std::string::npos)
                kInStruct = true;
        }

        if (_text[text_index] == '-' &&
            _text[text_index+1] == '-')
        {
            _text = _text.replace(_text.find("--"), strlen("--"), "");

            for (int _text_i = 0; _text_i < _text.size(); ++_text_i)
            {
                if (_text[_text_i] == '\t' ||
                    _text[_text_i] == ' ')
                    _text.erase(_text_i, 1);
            }

            syntax_tree.fUserValue += "dec ";
            syntax_tree.fUserValue += _text;

            kState.fSyntaxTree->fLeafList.push_back(syntax_tree);
            break;
        }

        // while loop
        if (_text[text_index] == 'w')
        {
            if (_text.find("while") == std::string::npos)
                continue;

            if (_text.find("while") != text_index)
                continue;

            syntax_tree.fUserValue = "jrl [r32+0x04]";

            std::string symbol_loop = "_loop_while_";
            symbol_loop += std::to_string(time_off.raw);
            symbol_loop += " ";

            syntax_tree.fUserValue = "beq ";
            syntax_tree.fUserValue += kState.kStackFrame[kState.kStackFrame.size() - 2].fRegister;
            syntax_tree.fUserValue += ",";
            syntax_tree.fUserValue += kState.kStackFrame[kState.kStackFrame.size() - 1].fRegister;
            syntax_tree.fUserValue += ", __end%s\njb __continue%s\n__export .text __end%s\njlr\nvoid __export .text __continue%s\njb _L";
            syntax_tree.fUserValue += std::to_string(kBracesCount + 1) + "_" + std::to_string(time_off.raw);

            while (syntax_tree.fUserValue.find("%s") != std::string::npos)
            {
                syntax_tree.fUserValue.replace(syntax_tree.fUserValue.find("%s"), strlen("%s"), symbol_loop);
            }

            kState.fSyntaxTree->fLeafList.push_back(syntax_tree);

            kOnWhileLoop = true;

            break;
        }

        if (_text[text_index] == 'f')
        {
            if (_text.find("for") == std::string::npos)
                continue;

            if (_text.find("for") != text_index)
                continue;

            syntax_tree.fUserValue = "jrl [r32+0x1]\n";

            // actually set registers now.

            auto expr = _text.substr(_text.find("for") + strlen("for"));

            kLatestVar.clear();

            kState.fSyntaxTree->fLeafList.push_back(syntax_tree);

            kOnForLoop = true;
            break;
        }

        if (_text[text_index] == '+' &&
            _text[text_index+1] == '+')
        {
            _text = _text.replace(_text.find("++"), strlen("++"), "");

            for (int _text_i = 0; _text_i < _text.size(); ++_text_i)
            {
                if (_text[_text_i] == '\t' ||
                    _text[_text_i] == ' ')
                    _text.erase(_text_i, 1);
            }

            syntax_tree.fUserValue += "add ";
            syntax_tree.fUserValue += _text;

            if (syntax_tree.fUserValue.find(";") != std::string::npos)
                syntax_tree.fUserValue.erase(syntax_tree.fUserValue.find(";"), 1);

            kState.fSyntaxTree->fLeafList.push_back(syntax_tree);
        }

        if (_text[text_index] == '}')
        {
            kRegisterCounter = kStartUsable;

            --kBracesCount;

            if (kBracesCount < 1)
            {
                kInBraces = false;
                kBracesCount = 0;
            }

            if (kInStruct)
                kInStruct = false;

            if (!kInBraces)
            {
                syntax_tree.fUserValue += kAddIfAnyEnd;

                kAddIfAnyEnd.clear();

                kState.fSyntaxTree->fLeafList.push_back(syntax_tree);
            }
            else
            {
                if (kOnWhileLoop ||
                    kOnForLoop)
                {
                    if (kOnForLoop)
                        kOnForLoop = false;

                    if (kOnWhileLoop)
                        kOnWhileLoop = false;

                    std::string symbol_loop = "_loop_for_";
                    symbol_loop += std::to_string(time_off.raw);
                    symbol_loop += " ";

                    syntax_tree.fUserValue = "beq ";
                    syntax_tree.fUserValue += kState.kStackFrame[kState.kStackFrame.size() - 2].fRegister;
                    syntax_tree.fUserValue += ",";
                    syntax_tree.fUserValue += kState.kStackFrame[kState.kStackFrame.size() - 1].fRegister;
                    syntax_tree.fUserValue += ", __end%s\njb __continue%s\n__export .text __end%s\njlr\nvoid __export .text __continue%s\njb _L";
                    syntax_tree.fUserValue += std::to_string(kBracesCount + 1) + "_" + std::to_string(time_off.raw);

                    while (syntax_tree.fUserValue.find("%s") != std::string::npos)
                    {
                        syntax_tree.fUserValue.replace(syntax_tree.fUserValue.find("%s"), strlen("%s"), symbol_loop);
                    }

                    kState.fSyntaxTree->fLeafList.push_back(syntax_tree);
                }
                else
                {
                    kState.fSyntaxTree->fLeafList.push_back(syntax_tree);
                }
            }
        }

        syntax_tree.fUserValue.clear();
    }

    syntax_tree.fUserValue = "\n";
    kState.fSyntaxTree->fLeafList.push_back(syntax_tree);

    return true;
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
                ln.find("&") == std::string::npos &&
                ln.find("(") == std::string::npos &&
                ln.find(")") == std::string::npos)
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
        dest += kAsmFileExt64x0;

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

        std::vector<std::string> keywords = { "ldw", "stw", "lda", "sta", "add", "dec", "mv"};

        for (auto& leaf : kState.fSyntaxTree->fLeafList)
        {
            for (auto& keyword : keywords)
            {
                if (ParserKit::find_word(leaf.fUserValue, keyword))
                {
                    std::size_t cnt = 0UL;

                    for (auto & reg : kState.kStackFrame)
                    {
                        std::string needle;

                        for (size_t i = 0; i < reg.fName.size(); i++)
                        {
                            if (reg.fName[i] == ' ')
                            {
                                ++i;

                                for (; i < reg.fName.size(); i++)
                                {
                                    if (reg.fName[i] == ',')
                                    {
                                        break;
                                    }

                                    if (reg.fName[i] == ' ')
                                        continue;

                                    needle += reg.fName[i];
                                }

                                break;
                            }
                        }

                        if (ParserKit::find_word(leaf.fUserValue, needle))
                        {
                            leaf.fUserValue.replace(leaf.fUserValue.find(needle),
                                                    needle.size(), reg.fRegister);

                            if (leaf.fUserValue.find("__import") != std::string::npos)
                            {
                                if (leaf.fUserValue.find("__import") < leaf.fUserValue.find(needle))
                                {
                                    leaf.fUserValue.erase(leaf.fUserValue.find("__import"), strlen("__import"));
                                }
                            }

                            ++cnt;
                        }
                    }

                    if (cnt > 1 && keyword != "mv" && keyword != "add" && keyword != "dec")
                    {
                        leaf.fUserValue.replace(leaf.fUserValue.find(keyword), keyword.size(), "mv");
                    }
                }
            }
        }

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
#define kSplashCxx() kPrintF(kWhite "%s\n", "cc, v1.14, (c) Western Company")

static void cc_print_help()
{
    kSplashCxx();

    kPrintF(kWhite "--asm={MACHINE}: %s\n", "Compile with a specific syntax. (64x0, 32x0)");
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

            if (strcmp(argv[index], "-verbose") == 0)
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
