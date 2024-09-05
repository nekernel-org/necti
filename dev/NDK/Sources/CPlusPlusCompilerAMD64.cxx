/*
 *	========================================================
 *
 *	c++-drv
 * 	Copyright ZKA Technologies, all rights reserved.
 *
 * 	========================================================
 */

/// BUGS: 1

#define __PK_USE_STRUCT_INSTEAD__ 1

#define kPrintF printf

#define kOk (0)

#define kSplashCxx() \
	kPrintF(kWhite "%s\n", "ZKA C++ Compiler Driver, (c) 2024 ZKA Technologies, all rights reserved.")

// extern_segment, @MLAutoRelease { ... }, fn foo() -> auto { ... }

#include <NDK/Asm/CPU/amd64.hxx>
#include <NDK/Parser.hxx>
#include <UUID.hxx>

#include <cctype>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

/* ZKA C++ Compiler driver */
/* This is part of the NDK. */
/* (c) ZKA Technologies */

/// @author Amlal El Mahrouss (amlel)
/// @file CPlusPlusCompilerAMD64.cxx
/// @brief Optimized C++ Compiler Driver.
/// @todo Throw error for scoped inside scoped variables when they get referenced outside.
/// @todo Add class/struct/enum support.

///////////////////////

// ANSI ESCAPE CODES //

///////////////////////

#define kBlank "\e[0;30m"
#define kRed   "\e[0;31m"
#define kWhite "\e[0;97m"

/////////////////////////////////////

// INTERNALS OF THE C++ COMPILER

/////////////////////////////////////

/// @internal
namespace detail
{
	struct CompilerRegisterMap final
	{
		std::string fName;
		std::string fReg;
	};

	// \brief Offset based struct/class
	struct CompilerStructMap final
	{
		std::string fName;
		std::string fReg;

		// offset counter
		std::size_t fOffsetsCnt;

		// offset array
		std::vector<std::pair<Int32, std::string>> fOffsets;
	};

	struct CompilerState final
	{
		std::vector<CompilerRegisterMap> kStackFrame;
		std::vector<CompilerStructMap>	 kStructMap;
		NDK::SyntaxLeafList*			 fSyntaxTree{nullptr};
		std::unique_ptr<std::ofstream>	 fOutputAssembly;
		std::string						 fLastFile;
		std::string						 fLastError;
		bool							 fVerbose;
	};
} // namespace detail

static detail::CompilerState kState;
static SizeType				 kErrorLimit = 100;

static Int32 kAcceptableErrors = 0;

namespace detail
{
	/// @brief prints an error into stdout.
	/// @param reason the reason of the error.
	/// @param file where does it originate from?
	void print_error_asm(std::string reason, std::string file) noexcept;

	struct CompilerType
	{
		std::string fName;
		std::string fValue;
	};
} // namespace detail

/////////////////////////////////////////////////////////////////////////////////////////

// Target architecture.
static int kMachine = NDK::AssemblyFactory::kArchAMD64;

/////////////////////////////////////////

// ARGUMENTS REGISTERS (R8, R15)

/////////////////////////////////////////

static size_t							 kRegisterCnt	  = kAsmRegisterLimit;
static size_t							 kStartUsable	  = 8;
static size_t							 kUsableLimit	  = 15;
static size_t							 kRegisterCounter = kStartUsable;
static std::vector<NDK::CompilerKeyword> kKeywords;

/////////////////////////////////////////

// COMPILER PARSING UTILITIES/STATES.

/////////////////////////////////////////

static std::vector<std::string> kFileList;
static NDK::AssemblyFactory		kFactory;
static bool						kInStruct	 = false;
static bool						kOnWhileLoop = false;
static bool						kOnForLoop	 = false;
static bool						kInBraces	 = false;
static size_t					kBracesCount = 0UL;

/* @brief C++ compiler backend for the ZKA C++ driver */
class CompilerBackendCPlusPlus final : public NDK::CompilerBackend
{
public:
	explicit CompilerBackendCPlusPlus()	 = default;
	~CompilerBackendCPlusPlus() override = default;

	NDK_COPY_DEFAULT(CompilerBackendCPlusPlus);

	bool Compile(const std::string text, const std::string file) override;

	const char* Language() override;
};

/// @internal compiler variables

static CompilerBackendCPlusPlus* kCompilerBackend = nullptr;

static std::vector<std::string> kRegisterMap;

static std::vector<std::string> cRegisters = {
	"rbx",
	"rsi",
	"r10",
	"r11",
	"r12",
	"r13",
	"r14",
	"r15",
	"xmm12",
	"xmm13",
	"xmm14",
	"xmm15",
};

/// @brief The PEF calling convention (caller must save rax, rbp)
/// @note callee must return via **rax**.
static std::vector<std::string> cRegistersCall = {
	"r8",
	"r9",
	"r10",
	"r11",
	"r12",
	"r13",
	"r14",
	"r15",
};

static size_t kLevelFunction = 0UL;

/// detail namespaces

const char* CompilerBackendCPlusPlus::Language()
{
	return "ZKA C++";
}

/////////////////////////////////////////////////////////////////////////////////////////

/// @name Compile
/// @brief Generate assembly from a C++ source.

/////////////////////////////////////////////////////////////////////////////////////////

bool CompilerBackendCPlusPlus::Compile(const std::string text,
									   const std::string file)
{
	if (text.empty())
		return false;

	std::size_t												  index = 0UL;
	std::vector<std::pair<NDK::CompilerKeyword, std::size_t>> keywords_list;

	bool		found		 = false;
	static bool commentBlock = false;

	for (auto& keyword : kKeywords)
	{
		if (text.find(keyword.keyword_name) != std::string::npos)
		{
			switch (keyword.keyword_kind)
			{
			case NDK::eKeywordKindCommentMultiLineStart: {
				commentBlock = true;
				return true;
			}
			case NDK::eKeywordKindCommentMultiLineEnd: {
				commentBlock = false;
				break;
			}
			case NDK::eKeywordKindCommentInline: {
				break;
			}
			default:
				break;
			}

			if (text[text.find(keyword.keyword_name) - 1] == '+' &&
				keyword.keyword_kind == NDK::KeywordKind::eKeywordKindVariableAssign)
				continue;

			if (text[text.find(keyword.keyword_name) - 1] == '-' &&
				keyword.keyword_kind == NDK::KeywordKind::eKeywordKindVariableAssign)
				continue;

			if (text[text.find(keyword.keyword_name) + 1] == '=' &&
				keyword.keyword_kind == NDK::KeywordKind::eKeywordKindVariableAssign)
				continue;

			keywords_list.emplace_back(std::make_pair(keyword, index));
			++index;

			found = true;
		}
	}

	if (!found && !commentBlock)
	{
		for (size_t i = 0; i < text.size(); i++)
		{
			if (isalnum(text[i]))
			{
				detail::print_error_asm("syntax error: " + text, file);
				return false;
			}
		}
	}

	for (auto& keyword : keywords_list)
	{
		auto syntax_tree = NDK::SyntaxLeafList::SyntaxLeaf();

		switch (keyword.first.keyword_kind)
		{
		case NDK::KeywordKind::eKeywordKindIf: {
			auto expr = text.substr(text.find(keyword.first.keyword_name) + keyword.first.keyword_name.size() + 1, text.find(")") - 1);

			if (expr.find(">=") != std::string::npos)
			{
				auto left  = text.substr(text.find(keyword.first.keyword_name) + keyword.first.keyword_name.size() + 2, expr.find("<=") + strlen("<="));
				auto right = text.substr(expr.find(">=") + strlen(">="), text.find(")") - 1);

				size_t i = right.size() - 1;

				try
				{
					while (!std::isalnum(right[i]))
					{
						right.erase(i, 1);
						--i;
					}

					right.erase(0, i);
				}
				catch (...)
				{
					right.erase(0, i);
				}

				i = left.size() - 1;
				try
				{
					while (!std::isalnum(left[i]))
					{
						left.erase(i, 1);
						--i;
					}

					left.erase(0, i);
				}
				catch (...)
				{
					left.erase(0, i);
				}

				if (!isdigit(left[0]) ||
					!isdigit(right[0]))
				{
					auto indexRight = 0UL;

					auto& valueOfVar = !isdigit(left[0]) ? left : right;

					for (auto pairRight : kRegisterMap)
					{
						++indexRight;

						if (pairRight != valueOfVar)
						{

							auto& valueOfVarOpposite = isdigit(left[0]) ? left : right;

							syntax_tree.fUserValue += "mov " + cRegisters[indexRight + 1] + ", " + valueOfVarOpposite + "\n";
							syntax_tree.fUserValue += "cmp " + cRegisters[kRegisterMap.size() - 1] + "," + cRegisters[indexRight + 1] + "\n";

							goto done_iterarting_on_if;
						}

						auto& valueOfVarOpposite = isdigit(left[0]) ? left : right;

						syntax_tree.fUserValue += "mov " + cRegisters[indexRight + 1] + ", " + valueOfVarOpposite + "\n";
						syntax_tree.fUserValue += "cmp " + cRegisters[kRegisterMap.size() - 1] + ", " + cRegisters[indexRight + 1] + "\n";

						break;
					}
				}

			done_iterarting_on_if:

				std::string fnName = text;
				fnName.erase(fnName.find(keyword.first.keyword_name));

				for (auto& ch : fnName)
				{
					if (ch == ' ')
						ch = '_';
				}

				syntax_tree.fUserValue += "jge __OFFSET_ON_TRUE_NDK\nsegment .code64 __OFFSET_ON_TRUE_NDK:\n";
			}

			break;
		}
		case NDK::KeywordKind::eKeywordKindFunctionStart: {
			if (text.ends_with(";"))
			{
				break;
			}

			for (auto& ch : text)
			{
				if (isdigit(ch))
				{
					goto dont_accept;
				}
			}

			goto accept;

		dont_accept:
			return true;

		accept:
			std::string fnName = text;

			for (auto& ch : fnName)
			{
				if (ch == ' ')
					ch = '_';
			}

			syntax_tree.fUserValue = "public_segment .code64 __NDK_" + fnName + "\n";

			++kLevelFunction;
		}
		case NDK::KeywordKind::eKeywordKindFunctionEnd: {
			if (text.ends_with(";"))
				break;

			--kLevelFunction;

			if (kRegisterMap.size() > cRegisters.size())
			{
				--kLevelFunction;
			}

			if (kLevelFunction < 1)
				kRegisterMap.clear();
			break;
		}
		case NDK::KeywordKind::eKeywordKindEndInstr:
		case NDK::KeywordKind::eKeywordKindVariableInc:
		case NDK::KeywordKind::eKeywordKindVariableDec:
		case NDK::KeywordKind::eKeywordKindVariableAssign: {
			std::string valueOfVar = "";

			if (keyword.first.keyword_kind == NDK::KeywordKind::eKeywordKindVariableInc)
			{
				valueOfVar = text.substr(text.find("+=") + 2);
			}
			else if (keyword.first.keyword_kind == NDK::KeywordKind::eKeywordKindVariableDec)
			{
				valueOfVar = text.substr(text.find("-=") + 2);
			}
			else if (keyword.first.keyword_kind == NDK::KeywordKind::eKeywordKindVariableAssign)
			{
				valueOfVar = text.substr(text.find("=") + 1);
			}
			else if (keyword.first.keyword_kind == NDK::KeywordKind::eKeywordKindEndInstr)
			{
				break;
			}

			while (valueOfVar.find(";") != std::string::npos &&
				   keyword.first.keyword_kind != NDK::KeywordKind::eKeywordKindEndInstr)
			{
				valueOfVar.erase(valueOfVar.find(";"));
			}

			std::string varName = text;

			if (keyword.first.keyword_kind == NDK::KeywordKind::eKeywordKindVariableInc)
			{
				varName.erase(varName.find("+="));
			}
			else if (keyword.first.keyword_kind == NDK::KeywordKind::eKeywordKindVariableDec)
			{
				varName.erase(varName.find("-="));
			}
			else if (keyword.first.keyword_kind == NDK::KeywordKind::eKeywordKindVariableAssign)
			{
				varName.erase(varName.find("="));
			}
			else if (keyword.first.keyword_kind == NDK::KeywordKind::eKeywordKindEndInstr)
			{
				varName.erase(varName.find(";"));
			}

			static bool typeFound = false;

			for (auto& keyword : kKeywords)
			{
				if (keyword.keyword_kind == NDK::eKeywordKindType)
				{
					if (text.find(keyword.keyword_name) != std::string::npos)
					{
						if (text[text.find(keyword.keyword_name)] == ' ')
						{
							typeFound = false;
							continue;
						}

						typeFound = true;
					}
				}
			}

			std::string instr = "mov ";

			if (typeFound)
			{
				if (keyword.first.keyword_kind == NDK::KeywordKind::eKeywordKindVariableInc)
				{
					detail::print_error_asm("Can't increment variable when it's being created.", file);
				}
				else if (keyword.first.keyword_kind == NDK::KeywordKind::eKeywordKindVariableDec)
				{
					detail::print_error_asm("Can't decrement variable when it's being created.", file);
				}

				if (kRegisterMap.size() > cRegisters.size())
				{
					++kLevelFunction;
				}

				while (varName.find(" ") != std::string::npos)
				{
					varName.erase(varName.find(" "), 1);
				}

				while (varName.find("\t") != std::string::npos)
				{
					varName.erase(varName.find("\t"), 1);
				}

				for (size_t i = 0; !isalnum(valueOfVar[i]); i++)
				{
					if (i > valueOfVar.size())
						break;

					valueOfVar.erase(i, 1);
				}

				constexpr auto cTrueVal	 = "true";
				constexpr auto cFalseVal = "false";

				if (valueOfVar == cTrueVal)
				{
					valueOfVar = "1";
				}
				else if (valueOfVar == cFalseVal)
				{
					valueOfVar = "0";
				}

				std::size_t indexRight = 0UL;

				for (auto pairRight : kRegisterMap)
				{
					++indexRight;

					if (pairRight != valueOfVar)
					{
						if (valueOfVar[0] == '\"')
						{

							syntax_tree.fUserValue = "segment .data64 __NDK_LOCAL_VAR_" + varName + ": db " + valueOfVar + ", 0\n\n";
							syntax_tree.fUserValue += instr + cRegisters[kRegisterMap.size() - 1] + ", " + "__NDK_LOCAL_VAR_" + varName + "\n";
						}
						else
						{
							syntax_tree.fUserValue = instr + cRegisters[kRegisterMap.size() - 1] + ", " + valueOfVar + "\n";
						}

						goto done;
					}
				}

				if (((int)indexRight - 1) < 0)
				{
					if (valueOfVar[0] == '\"')
					{

						syntax_tree.fUserValue = "segment .data64 __NDK_LOCAL_VAR_" + varName + ": db " + valueOfVar + ", 0\n";
						syntax_tree.fUserValue += instr + cRegisters[kRegisterMap.size()] + ", " + "__NDK_LOCAL_VAR_" + varName + "\n";
					}
					else
					{
						syntax_tree.fUserValue = instr + cRegisters[kRegisterMap.size()] + ", " + valueOfVar + "\n";
					}

					goto done;
				}

				if (valueOfVar[0] != '\"' &&
					valueOfVar[0] != '\'' &&
					!isdigit(valueOfVar[0]))
				{
					for (auto pair : kRegisterMap)
					{
						if (pair == valueOfVar)
							goto done;
					}

					detail::print_error_asm("Variable not declared: " + varName, file);
					return false;
				}

			done:
				for (auto& keyword : kKeywords)
				{
					if (keyword.keyword_kind == NDK::eKeywordKindType &&
						varName.find(keyword.keyword_name) != std::string::npos)
					{
						varName.erase(varName.find(keyword.keyword_name), keyword.keyword_name.size());
						break;
					}
				}

				kRegisterMap.push_back(varName);
			}
			else
			{
				if (kKeywords[keyword.second - 1].keyword_kind == NDK::eKeywordKindType ||
					kKeywords[keyword.second - 1].keyword_kind == NDK::eKeywordKindTypePtr)
				{
					syntax_tree.fUserValue = "\n";
					continue;
				}

				if (keyword.first.keyword_kind == NDK::KeywordKind::eKeywordKindEndInstr)
				{
					syntax_tree.fUserValue = "\n";
					continue;
				}

				if (keyword.first.keyword_kind == NDK::KeywordKind::eKeywordKindVariableInc)
				{
					instr = "add ";
				}
				else if (keyword.first.keyword_kind == NDK::KeywordKind::eKeywordKindVariableDec)
				{
					instr = "sub ";
				}

				std::string varErrCpy = varName;

				while (varName.find(" ") != std::string::npos)
				{
					varName.erase(varName.find(" "), 1);
				}

				while (varName.find("\t") != std::string::npos)
				{
					varName.erase(varName.find("\t"), 1);
				}

				std::size_t indxReg = 0UL;

				for (size_t i = 0; !isalnum(valueOfVar[i]); i++)
				{
					if (i > valueOfVar.size())
						break;

					valueOfVar.erase(i, 1);
				}

				while (valueOfVar.find(" ") != std::string::npos)
				{
					valueOfVar.erase(valueOfVar.find(" "), 1);
				}

				while (valueOfVar.find("\t") != std::string::npos)
				{
					valueOfVar.erase(valueOfVar.find("\t"), 1);
				}

				constexpr auto cTrueVal	 = "true";
				constexpr auto cFalseVal = "false";

				/// interpet boolean values, since we're on C++

				if (valueOfVar == cTrueVal)
				{
					valueOfVar = "1";
				}
				else if (valueOfVar == cFalseVal)
				{
					valueOfVar = "0";
				}

				for (auto pair : kRegisterMap)
				{
					++indxReg;

					if (pair != varName)
						continue;

					std::size_t indexRight = 0ul;

					for (auto pairRight : kRegisterMap)
					{
						++indexRight;

						if (pairRight != varName)
						{
							syntax_tree.fUserValue = instr + cRegisters[kRegisterMap.size()] + ", " + valueOfVar + "\n";
							continue;
						}

						syntax_tree.fUserValue = instr + cRegisters[indexRight - 1] + ", " + valueOfVar + "\n";
						break;
					}

					break;
				}

				if (syntax_tree.fUserValue.empty())
				{
					detail::print_error_asm("Variable not declared: " + varErrCpy, file);
				}
			}

			break;
		}
		case NDK::KeywordKind::eKeywordKindReturn: {
			auto		pos		= text.find("return") + strlen("return") + 1;
			std::string subText = text.substr(pos);
			subText				= subText.erase(subText.find(";"));
			size_t indxReg		= 0UL;

			if (subText[0] != '\"' &&
				subText[0] != '\'')
			{
				if (!isdigit(subText[0]))
				{
					for (auto pair : kRegisterMap)
					{
						++indxReg;

						if (pair != subText)
							continue;

						syntax_tree.fUserValue = "mov rax, " + cRegisters[indxReg - 1] + "\r\nret\n";
						break;
					}

					if (syntax_tree.fUserValue.empty())
					{
						detail::print_error_asm("Variable not declared: " + subText, file);
					}
				}
				else
				{
					syntax_tree.fUserValue = "mov rax, " + subText + "\r\nret\n";
				}
			}
			else
			{
				syntax_tree.fUserValue = "__NDK_LOCAL_RETURN_STRING: db " + subText + ", 0\nmov rcx, __NDK_LOCAL_RETURN_STRING\n";
				syntax_tree.fUserValue += "mov rax, rcx\r\nret\n";
			}

			break;
		}
		default:
			break;
		}

		syntax_tree.fUserData = keyword.first;
		kState.fSyntaxTree->fLeafList.push_back(syntax_tree);
	}

ndk_compile_ok:
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////

/**
 * @brief C To Assembly mount-point.
 */

/////////////////////////////////////////////////////////////////////////////////////////

class AssemblyCPlusPlusInterface final : public NDK::AssemblyInterface
{
public:
	explicit AssemblyCPlusPlusInterface()  = default;
	~AssemblyCPlusPlusInterface() override = default;

	NDK_COPY_DEFAULT(AssemblyCPlusPlusInterface);

	[[maybe_unused]] static Int32 Arch() noexcept
	{
		return NDK::AssemblyFactory::kArchAMD64;
	}

	Int32 CompileToFormat(std::string& src, Int32 arch) override
	{
		if (arch != AssemblyCPlusPlusInterface::Arch())
			return -1;

		if (kCompilerBackend == nullptr)
			return -1;

		/* @brief copy contents wihtout extension */
		std::string	  src_file = src;
		std::ifstream src_fp   = std::ifstream(src_file, std::ios::in);

		const char* cExts[] = kAsmFileExts;

		std::string dest = src_file;
		dest += cExts[2];

		if (dest.empty())
		{
			dest = "CXX-NDK-";

			std::random_device rd;
			auto			   seed_data = std::array<int, std::mt19937::state_size>{};

			std::generate(std::begin(seed_data), std::end(seed_data), std::ref(rd));

			std::seed_seq seq(std::begin(seed_data), std::end(seed_data));
			std::mt19937  generator(seq);

			auto gen = uuids::uuid_random_generator(generator);

			auto id = gen();
			dest += uuids::to_string(id);
		}

		kState.fOutputAssembly = std::make_unique<std::ofstream>(dest);

		auto fmt = NDK::current_date();

		(*kState.fOutputAssembly) << "; Path: " << src_file << "\n";
		(*kState.fOutputAssembly)
			<< "; Language: AMD64 assembly. (Generated from C++)\n";
		(*kState.fOutputAssembly) << "; Date: " << fmt << "\n";
		(*kState.fOutputAssembly) << "#bits 64\n#org 0x1000000"
								  << "\n";

		kState.fSyntaxTree = new NDK::SyntaxLeafList();

		// ===================================
		// Parse source file.
		// ===================================

		std::string line_source;

		while (std::getline(src_fp, line_source))
		{
			kCompilerBackend->Compile(line_source, src);
		}

		for (auto& ast_generated : kState.fSyntaxTree->fLeafList)
		{
			(*kState.fOutputAssembly) << ast_generated.fUserValue;
		}

		kState.fOutputAssembly->flush();
		kState.fOutputAssembly->close();

		delete kState.fSyntaxTree;
		kState.fSyntaxTree = nullptr;

		if (kAcceptableErrors > 0)
			return -1;

		return kOk;
	}
};

/////////////////////////////////////////////////////////////////////////////////////////

static void cxx_print_help()
{
	kSplashCxx();
	kPrintF("%s", "No help available, see:\n");
	kPrintF("%s", "www.zeta.com/developer/c++-drv\n");
}

/////////////////////////////////////////////////////////////////////////////////////////

#define kExtListCxx                          \
	{                                        \
		".cpp", ".cxx", ".cc", ".c++", ".cp" \
	}

NDK_MODULE(CompilerCPlusPlusX8664)
{
	bool skip = false;

	kKeywords.push_back({.keyword_name = "if", .keyword_kind = NDK::eKeywordKindIf});
	kKeywords.push_back({.keyword_name = "else", .keyword_kind = NDK::eKeywordKindElse});
	kKeywords.push_back({.keyword_name = "else if", .keyword_kind = NDK::eKeywordKindElseIf});

	kKeywords.push_back({.keyword_name = "class", .keyword_kind = NDK::eKeywordKindClass});
	kKeywords.push_back({.keyword_name = "struct", .keyword_kind = NDK::eKeywordKindClass});
	kKeywords.push_back({.keyword_name = "namespace", .keyword_kind = NDK::eKeywordKindNamespace});
	kKeywords.push_back({.keyword_name = "typedef", .keyword_kind = NDK::eKeywordKindTypedef});
	kKeywords.push_back({.keyword_name = "using", .keyword_kind = NDK::eKeywordKindTypedef});
	kKeywords.push_back({.keyword_name = "{", .keyword_kind = NDK::eKeywordKindBodyStart});
	kKeywords.push_back({.keyword_name = "}", .keyword_kind = NDK::eKeywordKindBodyEnd});
	kKeywords.push_back({.keyword_name = "auto", .keyword_kind = NDK::eKeywordKindVariable});
	kKeywords.push_back({.keyword_name = "int", .keyword_kind = NDK::eKeywordKindType});
	kKeywords.push_back({.keyword_name = "bool", .keyword_kind = NDK::eKeywordKindType});
	kKeywords.push_back({.keyword_name = "unsigned", .keyword_kind = NDK::eKeywordKindType});
	kKeywords.push_back({.keyword_name = "short", .keyword_kind = NDK::eKeywordKindType});
	kKeywords.push_back({.keyword_name = "char", .keyword_kind = NDK::eKeywordKindType});
	kKeywords.push_back({.keyword_name = "long", .keyword_kind = NDK::eKeywordKindType});
	kKeywords.push_back({.keyword_name = "float", .keyword_kind = NDK::eKeywordKindType});
	kKeywords.push_back({.keyword_name = "double", .keyword_kind = NDK::eKeywordKindType});
	kKeywords.push_back({.keyword_name = "void", .keyword_kind = NDK::eKeywordKindType});

	kKeywords.push_back({.keyword_name = "auto*", .keyword_kind = NDK::eKeywordKindVariablePtr});
	kKeywords.push_back({.keyword_name = "int*", .keyword_kind = NDK::eKeywordKindTypePtr});
	kKeywords.push_back({.keyword_name = "bool*", .keyword_kind = NDK::eKeywordKindTypePtr});
	kKeywords.push_back({.keyword_name = "unsigned*", .keyword_kind = NDK::eKeywordKindTypePtr});
	kKeywords.push_back({.keyword_name = "short*", .keyword_kind = NDK::eKeywordKindTypePtr});
	kKeywords.push_back({.keyword_name = "char*", .keyword_kind = NDK::eKeywordKindTypePtr});
	kKeywords.push_back({.keyword_name = "long*", .keyword_kind = NDK::eKeywordKindTypePtr});
	kKeywords.push_back({.keyword_name = "float*", .keyword_kind = NDK::eKeywordKindTypePtr});
	kKeywords.push_back({.keyword_name = "double*", .keyword_kind = NDK::eKeywordKindTypePtr});
	kKeywords.push_back({.keyword_name = "void*", .keyword_kind = NDK::eKeywordKindTypePtr});

	kKeywords.push_back({.keyword_name = "(", .keyword_kind = NDK::eKeywordKindFunctionStart});
	kKeywords.push_back({.keyword_name = ")", .keyword_kind = NDK::eKeywordKindFunctionEnd});
	kKeywords.push_back({.keyword_name = "=", .keyword_kind = NDK::eKeywordKindVariableAssign});
	kKeywords.push_back({.keyword_name = "+=", .keyword_kind = NDK::eKeywordKindVariableInc});
	kKeywords.push_back({.keyword_name = "-=", .keyword_kind = NDK::eKeywordKindVariableDec});
	kKeywords.push_back({.keyword_name = "const", .keyword_kind = NDK::eKeywordKindConstant});
	kKeywords.push_back({.keyword_name = "*", .keyword_kind = NDK::eKeywordKindPtr});
	kKeywords.push_back({.keyword_name = "->", .keyword_kind = NDK::eKeywordKindPtrAccess});
	kKeywords.push_back({.keyword_name = ".", .keyword_kind = NDK::eKeywordKindAccess});
	kKeywords.push_back({.keyword_name = ",", .keyword_kind = NDK::eKeywordKindArgSeparator});
	kKeywords.push_back({.keyword_name = ";", .keyword_kind = NDK::eKeywordKindEndInstr});
	kKeywords.push_back({.keyword_name = ":", .keyword_kind = NDK::eKeywordKindSpecifier});
	kKeywords.push_back({.keyword_name = "public:", .keyword_kind = NDK::eKeywordKindSpecifier});
	kKeywords.push_back({.keyword_name = "private:", .keyword_kind = NDK::eKeywordKindSpecifier});
	kKeywords.push_back({.keyword_name = "protected:", .keyword_kind = NDK::eKeywordKindSpecifier});
	kKeywords.push_back({.keyword_name = "final", .keyword_kind = NDK::eKeywordKindSpecifier});
	kKeywords.push_back({.keyword_name = "return", .keyword_kind = NDK::eKeywordKindReturn});
	kKeywords.push_back({.keyword_name = "/*", .keyword_kind = NDK::eKeywordKindCommentMultiLineStart});
	kKeywords.push_back({.keyword_name = "*/", .keyword_kind = NDK::eKeywordKindCommentMultiLineStart});
	kKeywords.push_back({.keyword_name = "//", .keyword_kind = NDK::eKeywordKindCommentInline});
	kKeywords.push_back({.keyword_name = "==", .keyword_kind = NDK::eKeywordKindEq});
	kKeywords.push_back({.keyword_name = "!=", .keyword_kind = NDK::eKeywordKindNotEq});
	kKeywords.push_back({.keyword_name = ">=", .keyword_kind = NDK::eKeywordKindGreaterEq});
	kKeywords.push_back({.keyword_name = "<=", .keyword_kind = NDK::eKeywordKindLessEq});

	kFactory.Mount(new AssemblyCPlusPlusInterface());
	kCompilerBackend = new CompilerBackendCPlusPlus();

	for (auto index = 1UL; index < argc; ++index)
	{
		if (argv[index][0] == '/')
		{
			if (skip)
			{
				skip = false;
				continue;
			}

			if (strcmp(argv[index], "/Ver") == 0)
			{
				kSplashCxx();
				return kOk;
			}

			if (strcmp(argv[index], "/Verbose") == 0)
			{
				kState.fVerbose = true;

				continue;
			}

			if (strcmp(argv[index], "/?") == 0)
			{
				cxx_print_help();

				return kOk;
			}

			if (strcmp(argv[index], "/GetC++") == 0)
			{
				if (kCompilerBackend)
					std::cout << kCompilerBackend->Language() << "\n";

				return kOk;
			}

			if (strcmp(argv[index], "/MaxErr") == 0)
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

			std::string err = "Unknown option: ";
			err += argv[index];

			detail::print_error_asm(err, "c++-drv");

			continue;
		}

		kFileList.emplace_back(argv[index]);

		std::string argv_i = argv[index];

		std::vector exts  = kExtListCxx;
		bool		found = false;

		for (std::string ext : exts)
		{
			if (argv_i.find(ext) != std::string::npos)
			{
				found = true;
				break;
			}
		}

		if (!found)
		{
			if (kState.fVerbose)
			{
				detail::print_error_asm(argv_i + " is not a valid C++ source.\n", "c++-drv");
			}

			return 1;
		}

		std::cout << "c++drv: building: " << argv[index] << std::endl;

		if (kFactory.Compile(argv_i, kMachine) != kOk)
			return -1;
	}

	return kOk;
}

// Last rev 8-1-24
//
