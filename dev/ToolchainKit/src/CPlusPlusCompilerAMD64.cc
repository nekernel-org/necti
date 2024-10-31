/*
 *	========================================================
 *
 *	c++-drv
 * 	Copyright EL Mahrouss Logic, all rights reserved.
 *
 * 	========================================================
 */

/// BUGS: 1

#define __PK_USE_STRUCT_INSTEAD__ 1

#define kPrintF printf

#define kExitOK (0)

#define kSplashCxx() \
	kPrintF(kWhite "%s\n", "ZKA C++ Compiler Driver, (c) 2024 EL Mahrouss Logic, all rights reserved.")

// extern_segment, @MLAutoRelease { ... }, fn foo() -> auto { ... }

#include <ToolchainKit/AAL/CPU/amd64.h>
#include <ToolchainKit/Parser.h>
#include <ToolchainKit/UUID.h>

/* ZKA C++ Compiler driver */
/* This is part of the ToolchainKit. */
/* (c) EL Mahrouss Logic */

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
		ToolchainKit::SyntaxLeafList*			 fSyntaxTree{nullptr};
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
static int kMachine = ToolchainKit::AssemblyFactory::kArchAMD64;

/////////////////////////////////////////

// ARGUMENTS REGISTERS (R8, R15)

/////////////////////////////////////////

static size_t							 kRegisterCnt	  = kAsmRegisterLimit;
static size_t							 kStartUsable	  = 8;
static size_t							 kUsableLimit	  = 15;
static size_t							 kRegisterCounter = kStartUsable;
static std::vector<ToolchainKit::CompilerKeyword> kKeywords;

/////////////////////////////////////////

// COMPILER PARSING UTILITIES/STATES.

/////////////////////////////////////////

static std::vector<std::string> kFileList;
static ToolchainKit::AssemblyFactory		kFactory;
static bool						kInStruct	 = false;
static bool						kOnWhileLoop = false;
static bool						kOnForLoop	 = false;
static bool						kInBraces	 = false;
static size_t					kBracesCount = 0UL;

/* @brief C++ compiler backend for the ZKA C++ driver */
class CompilerFrontendCPlusPlus final : public ToolchainKit::ICompilerFrontend
{
public:
	explicit CompilerFrontendCPlusPlus()  = default;
	~CompilerFrontendCPlusPlus() override = default;

	TOOLCHAINKIT_COPY_DEFAULT(CompilerFrontendCPlusPlus);

	bool Compile(const std::string text, const std::string file) override;

	const char* Language() override;
};

/// @internal compiler variables

static CompilerFrontendCPlusPlus* kCompilerFrontend = nullptr;

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

const char* CompilerFrontendCPlusPlus::Language()
{
	return "ZKA C++";
}

/////////////////////////////////////////////////////////////////////////////////////////

/// @name Compile
/// @brief Generate assembly from a C++ source.

/////////////////////////////////////////////////////////////////////////////////////////

bool CompilerFrontendCPlusPlus::Compile(const std::string text,
										const std::string file)
{
	if (text.empty())
		return false;

	std::size_t												  index = 0UL;
	std::vector<std::pair<ToolchainKit::CompilerKeyword, std::size_t>> keywords_list;

	bool		found		 = false;
	static bool commentBlock = false;

	for (auto& keyword : kKeywords)
	{
		if (text.find(keyword.keyword_name) != std::string::npos)
		{
			switch (keyword.keyword_kind)
			{
			case ToolchainKit::eKeywordKindCommentMultiLineStart: {
				commentBlock = true;
				return true;
			}
			case ToolchainKit::eKeywordKindCommentMultiLineEnd: {
				commentBlock = false;
				break;
			}
			case ToolchainKit::eKeywordKindCommentInline: {
				break;
			}
			default:
				break;
			}

			if (text[text.find(keyword.keyword_name) - 1] == '+' &&
				keyword.keyword_kind == ToolchainKit::KeywordKind::eKeywordKindVariableAssign)
				continue;

			if (text[text.find(keyword.keyword_name) - 1] == '-' &&
				keyword.keyword_kind == ToolchainKit::KeywordKind::eKeywordKindVariableAssign)
				continue;

			if (text[text.find(keyword.keyword_name) + 1] == '=' &&
				keyword.keyword_kind == ToolchainKit::KeywordKind::eKeywordKindVariableAssign)
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
		auto syntax_tree = ToolchainKit::SyntaxLeafList::SyntaxLeaf();

		switch (keyword.first.keyword_kind)
		{
		case ToolchainKit::KeywordKind::eKeywordKindIf: {
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
		case ToolchainKit::KeywordKind::eKeywordKindFunctionStart: {
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

			syntax_tree.fUserValue = "public_segment .code64 __TOOLCHAINKIT_" + fnName + "\n";

			++kLevelFunction;
		}
		case ToolchainKit::KeywordKind::eKeywordKindFunctionEnd: {
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
		case ToolchainKit::KeywordKind::eKeywordKindEndInstr:
		case ToolchainKit::KeywordKind::eKeywordKindVariableInc:
		case ToolchainKit::KeywordKind::eKeywordKindVariableDec:
		case ToolchainKit::KeywordKind::eKeywordKindVariableAssign: {
			std::string valueOfVar = "";

			if (keyword.first.keyword_kind == ToolchainKit::KeywordKind::eKeywordKindVariableInc)
			{
				valueOfVar = text.substr(text.find("+=") + 2);
			}
			else if (keyword.first.keyword_kind == ToolchainKit::KeywordKind::eKeywordKindVariableDec)
			{
				valueOfVar = text.substr(text.find("-=") + 2);
			}
			else if (keyword.first.keyword_kind == ToolchainKit::KeywordKind::eKeywordKindVariableAssign)
			{
				valueOfVar = text.substr(text.find("=") + 1);
			}
			else if (keyword.first.keyword_kind == ToolchainKit::KeywordKind::eKeywordKindEndInstr)
			{
				break;
			}

			while (valueOfVar.find(";") != std::string::npos &&
				   keyword.first.keyword_kind != ToolchainKit::KeywordKind::eKeywordKindEndInstr)
			{
				valueOfVar.erase(valueOfVar.find(";"));
			}

			std::string varName = text;

			if (keyword.first.keyword_kind == ToolchainKit::KeywordKind::eKeywordKindVariableInc)
			{
				varName.erase(varName.find("+="));
			}
			else if (keyword.first.keyword_kind == ToolchainKit::KeywordKind::eKeywordKindVariableDec)
			{
				varName.erase(varName.find("-="));
			}
			else if (keyword.first.keyword_kind == ToolchainKit::KeywordKind::eKeywordKindVariableAssign)
			{
				varName.erase(varName.find("="));
			}
			else if (keyword.first.keyword_kind == ToolchainKit::KeywordKind::eKeywordKindEndInstr)
			{
				varName.erase(varName.find(";"));
			}

			static bool typeFound = false;

			for (auto& keyword : kKeywords)
			{
				if (keyword.keyword_kind == ToolchainKit::eKeywordKindType)
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

			if (typeFound && keyword.first.keyword_kind != ToolchainKit::KeywordKind::eKeywordKindVariableInc &&
				keyword.first.keyword_kind != ToolchainKit::KeywordKind::eKeywordKindVariableDec)
			{
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

							syntax_tree.fUserValue = "segment .data64 __TOOLCHAINKIT_LOCAL_VAR_" + varName + ": db " + valueOfVar + ", 0\n\n";
							syntax_tree.fUserValue += instr + cRegisters[kRegisterMap.size() - 1] + ", " + "__TOOLCHAINKIT_LOCAL_VAR_" + varName + "\n";
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

						syntax_tree.fUserValue = "segment .data64 __TOOLCHAINKIT_LOCAL_VAR_" + varName + ": db " + valueOfVar + ", 0\n";
						syntax_tree.fUserValue += instr + cRegisters[kRegisterMap.size()] + ", " + "__TOOLCHAINKIT_LOCAL_VAR_" + varName + "\n";
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
					if (keyword.keyword_kind == ToolchainKit::eKeywordKindType &&
						varName.find(keyword.keyword_name) != std::string::npos)
					{
						varName.erase(varName.find(keyword.keyword_name), keyword.keyword_name.size());
						break;
					}
				}

				kRegisterMap.push_back(varName);

				break;
			}

			if (kKeywords[keyword.second - 1].keyword_kind == ToolchainKit::eKeywordKindType ||
				kKeywords[keyword.second - 1].keyword_kind == ToolchainKit::eKeywordKindTypePtr)
			{
				syntax_tree.fUserValue = "\n";
				continue;
			}

			if (keyword.first.keyword_kind == ToolchainKit::KeywordKind::eKeywordKindEndInstr)
			{
				syntax_tree.fUserValue = "\n";
				continue;
			}

			if (keyword.first.keyword_kind == ToolchainKit::KeywordKind::eKeywordKindVariableInc)
			{
				instr = "add ";
			}
			else if (keyword.first.keyword_kind == ToolchainKit::KeywordKind::eKeywordKindVariableDec)
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

			break;
		}
		case ToolchainKit::KeywordKind::eKeywordKindReturn: {
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
				syntax_tree.fUserValue = "__TOOLCHAINKIT_LOCAL_RETURN_STRING: db " + subText + ", 0\nmov rcx, __TOOLCHAINKIT_LOCAL_RETURN_STRING\n";
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
 * @brief C++ assembler class.
 */

/////////////////////////////////////////////////////////////////////////////////////////

class AssemblyCPlusPlusInterface final : public ToolchainKit::AssemblyInterface
{
public:
	explicit AssemblyCPlusPlusInterface()  = default;
	~AssemblyCPlusPlusInterface() override = default;

	TOOLCHAINKIT_COPY_DEFAULT(AssemblyCPlusPlusInterface);

	[[maybe_unused]] static Int32 Arch() noexcept
	{
		return ToolchainKit::AssemblyFactory::kArchAMD64;
	}

	Int32 CompileToFormat(std::string& src, Int32 arch) override
	{
		if (arch != AssemblyCPlusPlusInterface::Arch())
			return 1;

		if (kCompilerFrontend == nullptr)
			return 1;

		/* @brief copy contents wihtout extension */
		std::string	  src_file = src;
		std::ifstream src_fp   = std::ifstream(src_file, std::ios::in);

		const char* cExts[] = kAsmFileExts;

		std::string dest = src_file;
		dest += cExts[2];

		if (dest.empty())
		{
			dest = "CXX-ToolchainKit-";

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

		auto fmt = ToolchainKit::current_date();

		(*kState.fOutputAssembly) << "; Path: " << src_file << "\n";
		(*kState.fOutputAssembly)
			<< "; Language: AMD64 assembly. (Generated from C++)\n";
		(*kState.fOutputAssembly) << "; Date: " << fmt << "\n";
		(*kState.fOutputAssembly) << "#bits 64\n#org 0x1000000"
								  << "\n";

		kState.fSyntaxTree = new ToolchainKit::SyntaxLeafList();

		// ===================================
		// Parse source file.
		// ===================================

		std::string line_source;

		while (std::getline(src_fp, line_source))
		{
			kCompilerFrontend->Compile(line_source, src);
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
			return 1;

		return kExitOK;
	}
};

/////////////////////////////////////////////////////////////////////////////////////////

static void cxx_print_help()
{
	kSplashCxx();
	kPrintF("%s", "No help available, see:\n");
	kPrintF("%s", "www.zws.zka.com/help/c++lang\n");
}

/////////////////////////////////////////////////////////////////////////////////////////

#define kExtListCxx                          \
	{                                        \
		".cpp", ".cxx", ".cc", ".c++", ".cp" \
	}

TOOLCHAINKIT_MODULE(CompilerCPlusPlusX8664)
{
	bool skip = false;

	kKeywords.push_back({.keyword_name = "if", .keyword_kind = ToolchainKit::eKeywordKindIf});
	kKeywords.push_back({.keyword_name = "else", .keyword_kind = ToolchainKit::eKeywordKindElse});
	kKeywords.push_back({.keyword_name = "else if", .keyword_kind = ToolchainKit::eKeywordKindElseIf});

	kKeywords.push_back({.keyword_name = "class", .keyword_kind = ToolchainKit::eKeywordKindClass});
	kKeywords.push_back({.keyword_name = "struct", .keyword_kind = ToolchainKit::eKeywordKindClass});
	kKeywords.push_back({.keyword_name = "namespace", .keyword_kind = ToolchainKit::eKeywordKindNamespace});
	kKeywords.push_back({.keyword_name = "typedef", .keyword_kind = ToolchainKit::eKeywordKindTypedef});
	kKeywords.push_back({.keyword_name = "using", .keyword_kind = ToolchainKit::eKeywordKindTypedef});
	kKeywords.push_back({.keyword_name = "{", .keyword_kind = ToolchainKit::eKeywordKindBodyStart});
	kKeywords.push_back({.keyword_name = "}", .keyword_kind = ToolchainKit::eKeywordKindBodyEnd});
	kKeywords.push_back({.keyword_name = "auto", .keyword_kind = ToolchainKit::eKeywordKindVariable});
	kKeywords.push_back({.keyword_name = "int", .keyword_kind = ToolchainKit::eKeywordKindType});
	kKeywords.push_back({.keyword_name = "bool", .keyword_kind = ToolchainKit::eKeywordKindType});
	kKeywords.push_back({.keyword_name = "unsigned", .keyword_kind = ToolchainKit::eKeywordKindType});
	kKeywords.push_back({.keyword_name = "short", .keyword_kind = ToolchainKit::eKeywordKindType});
	kKeywords.push_back({.keyword_name = "char", .keyword_kind = ToolchainKit::eKeywordKindType});
	kKeywords.push_back({.keyword_name = "long", .keyword_kind = ToolchainKit::eKeywordKindType});
	kKeywords.push_back({.keyword_name = "float", .keyword_kind = ToolchainKit::eKeywordKindType});
	kKeywords.push_back({.keyword_name = "double", .keyword_kind = ToolchainKit::eKeywordKindType});
	kKeywords.push_back({.keyword_name = "void", .keyword_kind = ToolchainKit::eKeywordKindType});

	kKeywords.push_back({.keyword_name = "auto*", .keyword_kind = ToolchainKit::eKeywordKindVariablePtr});
	kKeywords.push_back({.keyword_name = "int*", .keyword_kind = ToolchainKit::eKeywordKindTypePtr});
	kKeywords.push_back({.keyword_name = "bool*", .keyword_kind = ToolchainKit::eKeywordKindTypePtr});
	kKeywords.push_back({.keyword_name = "unsigned*", .keyword_kind = ToolchainKit::eKeywordKindTypePtr});
	kKeywords.push_back({.keyword_name = "short*", .keyword_kind = ToolchainKit::eKeywordKindTypePtr});
	kKeywords.push_back({.keyword_name = "char*", .keyword_kind = ToolchainKit::eKeywordKindTypePtr});
	kKeywords.push_back({.keyword_name = "long*", .keyword_kind = ToolchainKit::eKeywordKindTypePtr});
	kKeywords.push_back({.keyword_name = "float*", .keyword_kind = ToolchainKit::eKeywordKindTypePtr});
	kKeywords.push_back({.keyword_name = "double*", .keyword_kind = ToolchainKit::eKeywordKindTypePtr});
	kKeywords.push_back({.keyword_name = "void*", .keyword_kind = ToolchainKit::eKeywordKindTypePtr});

	kKeywords.push_back({.keyword_name = "(", .keyword_kind = ToolchainKit::eKeywordKindFunctionStart});
	kKeywords.push_back({.keyword_name = ")", .keyword_kind = ToolchainKit::eKeywordKindFunctionEnd});
	kKeywords.push_back({.keyword_name = "=", .keyword_kind = ToolchainKit::eKeywordKindVariableAssign});
	kKeywords.push_back({.keyword_name = "+=", .keyword_kind = ToolchainKit::eKeywordKindVariableInc});
	kKeywords.push_back({.keyword_name = "-=", .keyword_kind = ToolchainKit::eKeywordKindVariableDec});
	kKeywords.push_back({.keyword_name = "const", .keyword_kind = ToolchainKit::eKeywordKindConstant});
	kKeywords.push_back({.keyword_name = "*", .keyword_kind = ToolchainKit::eKeywordKindPtr});
	kKeywords.push_back({.keyword_name = "->", .keyword_kind = ToolchainKit::eKeywordKindPtrAccess});
	kKeywords.push_back({.keyword_name = ".", .keyword_kind = ToolchainKit::eKeywordKindAccess});
	kKeywords.push_back({.keyword_name = ",", .keyword_kind = ToolchainKit::eKeywordKindArgSeparator});
	kKeywords.push_back({.keyword_name = ";", .keyword_kind = ToolchainKit::eKeywordKindEndInstr});
	kKeywords.push_back({.keyword_name = ":", .keyword_kind = ToolchainKit::eKeywordKindSpecifier});
	kKeywords.push_back({.keyword_name = "public:", .keyword_kind = ToolchainKit::eKeywordKindSpecifier});
	kKeywords.push_back({.keyword_name = "private:", .keyword_kind = ToolchainKit::eKeywordKindSpecifier});
	kKeywords.push_back({.keyword_name = "protected:", .keyword_kind = ToolchainKit::eKeywordKindSpecifier});
	kKeywords.push_back({.keyword_name = "final", .keyword_kind = ToolchainKit::eKeywordKindSpecifier});
	kKeywords.push_back({.keyword_name = "return", .keyword_kind = ToolchainKit::eKeywordKindReturn});
	kKeywords.push_back({.keyword_name = "--*", .keyword_kind = ToolchainKit::eKeywordKindCommentMultiLineStart});
	kKeywords.push_back({.keyword_name = "*/", .keyword_kind = ToolchainKit::eKeywordKindCommentMultiLineStart});
	kKeywords.push_back({.keyword_name = "--/", .keyword_kind = ToolchainKit::eKeywordKindCommentInline});
	kKeywords.push_back({.keyword_name = "==", .keyword_kind = ToolchainKit::eKeywordKindEq});
	kKeywords.push_back({.keyword_name = "!=", .keyword_kind = ToolchainKit::eKeywordKindNotEq});
	kKeywords.push_back({.keyword_name = ">=", .keyword_kind = ToolchainKit::eKeywordKindGreaterEq});
	kKeywords.push_back({.keyword_name = "<=", .keyword_kind = ToolchainKit::eKeywordKindLessEq});

	kFactory.Mount(new AssemblyCPlusPlusInterface());
	kCompilerFrontend = new CompilerFrontendCPlusPlus();

	for (auto index = 1UL; index < argc; ++index)
	{
		if (argv[index][0] == '-')
		{
			if (skip)
			{
				skip = false;
				continue;
			}

			if (strcmp(argv[index], "--cl:version") == 0)
			{
				kSplashCxx();
				return kExitOK;
			}

			if (strcmp(argv[index], "--cl:verbose") == 0)
			{
				kState.fVerbose = true;

				continue;
			}

			if (strcmp(argv[index], "--cl:h") == 0)
			{
				cxx_print_help();

				return kExitOK;
			}

			if (strcmp(argv[index], "--cl:c++-dialect") == 0)
			{
				if (kCompilerFrontend)
					std::cout << kCompilerFrontend->Language() << "\n";

				return kExitOK;
			}

			if (strcmp(argv[index], "--cl:max-err") == 0)
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

		std::cout << "CPlusPlusCompilerAMD64: building: " << argv[index] << std::endl;

		if (kFactory.Compile(argv_i, kMachine) != kExitOK)
			return 1;
	}

	return kExitOK;
}

// Last rev 8-1-24
//
