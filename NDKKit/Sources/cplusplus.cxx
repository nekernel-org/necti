/*
 *	========================================================
 *
 *	cplusplus
 * 	Copyright ZKA Technologies, all rights reserved.
 *
 * 	========================================================
 */

/// bugs: 0

#define __PK_USE_STRUCT_INSTEAD__ 1

#define kPrintF printf

#define kOk (0)

#define kSplashCxx() \
	kPrintF(kWhite "%s\n", "ZKA C++ Compiler Driver, (c) 2024 ZKA Technologies, all rights reserved.")

// import, @MLAutoRelease { ... }, fn foo() -> auto { ... }

#include <NDKKit/AsmKit/CPU/amd64.hxx>
#include <NDKKit/Parser.hxx>
#include <UUID.hxx>

#include <cctype>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

/* ZKA C++ driver */
/* This is part of NDK. */
/* (c) ZKA Technologies */

/// @author Amlal El Mahrouss (amlel)
/// @file cc.cxx
/// @brief Optimized C++ Compiler.
/// @todo Throw error for scoped inside scoped variables when they get referenced outside.
/// @todo Add class/struct/enum support.

/////////////////////

// ANSI ESCAPE CODES

/////////////////////

#define kBlank "\e[0;30m"
#define kRed   "\e[0;31m"
#define kWhite "\e[0;97m"

/////////////////////////////////////

// INTERNALS OF THE C COMPILER

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
		std::vector<CompilerKit::SyntaxLeafList> fSyntaxTreeList;
		std::vector<CompilerRegisterMap>	   kStackFrame;
		std::vector<CompilerStructMap>		   kStructMap;
		CompilerKit::SyntaxLeafList*			   fSyntaxTree{nullptr};
		std::unique_ptr<std::ofstream>		   fOutputAssembly;
		std::string							   fLastFile;
		std::string							   fLastError;
		bool								   fVerbose;
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
			std::cout << kRed << "[ cplusplus ] " << kWhite
					  << ((file == "cplusplus") ? "internal compiler error "
												: ("in file, " + file))
					  << kBlank << std::endl;
			std::cout << kRed << "[ cplusplus ] " << kWhite << reason << kBlank
					  << std::endl;

			kState.fLastFile = file;
		}
		else
		{
			std::cout << kRed << "[ cplusplus ] [ " << kState.fLastFile << " ] " << kWhite
					  << reason << kBlank << std::endl;
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
} // namespace detail

/////////////////////////////////////////////////////////////////////////////////////////

// Target architecture.
static int kMachine = CompilerKit::AssemblyFactory::kArchAMD64;

/////////////////////////////////////////

// ARGUMENTS REGISTERS (R8, R15)

/////////////////////////////////////////

static size_t								   kRegisterCnt		= kAsmRegisterLimit;
static size_t								   kStartUsable		= 8;
static size_t								   kUsableLimit		= 15;
static size_t								   kRegisterCounter = kStartUsable;
static std::vector<CompilerKit::CompilerKeyword> kKeywords;

/////////////////////////////////////////

// COMPILER PARSING UTILITIES/STATES.

/////////////////////////////////////////

static std::vector<std::string>		kFileList;
static CompilerKit::AssemblyFactory kFactory;
static bool							kInStruct	 = false;
static bool							kOnWhileLoop = false;
static bool							kOnForLoop	 = false;
static bool							kInBraces	 = false;
static size_t						kBracesCount = 0UL;

/* @brief C++ compiler backend for the ZKA C++ driver */
class CompilerBackendCPlusPlus final : public CompilerKit::CompilerBackend
{
public:
	explicit CompilerBackendCPlusPlus()	 = default;
	~CompilerBackendCPlusPlus() override = default;

	MPCC_COPY_DEFAULT(CompilerBackendCPlusPlus);

	bool Compile(const std::string& text, const char* file) override;

	const char* Language() override;
};

/// @internal compiler variables

static CompilerBackendCPlusPlus*		 kCompilerBackend = nullptr;
static std::vector<detail::CompilerType> kCompilerVariables;
static std::vector<std::string>			 kCompilerFunctions;

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
	"rcx",
	"rdx",
	"r8",
	"r9",
	"xmm8",
	"xmm9",
	"xmm10",
	"xmm11",
};

static size_t kLevelFunction = 0UL;

/// detail namespaces

const char* CompilerBackendCPlusPlus::Language()
{
	return "ZKA C++";
}

/////////////////////////////////////////////////////////////////////////////////////////

/// @name Compile
/// @brief Generate MASM assembly from a C++ source.

/////////////////////////////////////////////////////////////////////////////////////////

bool CompilerBackendCPlusPlus::Compile(const std::string& text,
									   const char*		  file)
{
	if (text.empty())
		return false;

	// if (expr)
	// int name = expr;
	// expr;

	std::size_t														index = 0UL;
	std::vector<std::pair<CompilerKit::CompilerKeyword, std::size_t>> keywords_list;

	bool		found		 = false;
	static bool commentBlock = false;

	for (auto& keyword : kKeywords)
	{
		if (text.find(keyword.keyword_name) != std::string::npos)
		{
			switch (keyword.keyword_kind)
			{
			case CompilerKit::eKeywordKindCommentMultiLineStart: {
				commentBlock = true;
				return true;
			}
			case CompilerKit::eKeywordKindCommentMultiLineEnd: {
				commentBlock = false;
				break;
			}
			case CompilerKit::eKeywordKindCommentInline: {
				break;
			}
			default:
				break;
			}

			if (text[text.find(keyword.keyword_name) - 1] == '+' &&
				keyword.keyword_kind == CompilerKit::KeywordKind::eKeywordKindVariableAssign)
				continue;

			if (text[text.find(keyword.keyword_name) - 1] == '-' &&
				keyword.keyword_kind == CompilerKit::KeywordKind::eKeywordKindVariableAssign)
				continue;

			if (text[text.find(keyword.keyword_name) + 1] == '=' &&
				keyword.keyword_kind == CompilerKit::KeywordKind::eKeywordKindVariableAssign)
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
				detail::print_error("syntax error: " + text, file);
				return false;
			}
		}
	}

	for (auto& keyword : keywords_list)
	{
		auto syntax_tree = CompilerKit::SyntaxLeafList::SyntaxLeaf();

		switch (keyword.first.keyword_kind)
		{
		case CompilerKit::KeywordKind::eKeywordKindIf: {
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

							syntax_tree.fUserValue += "mov " + cRegisters[kRegisterMap.size() - 1] + ", " + valueOfVar + "\n";
							syntax_tree.fUserValue += "cmp " + cRegisters[kRegisterMap.size() - 1] + "," + cRegisters[indexRight + 1] + "\n";

							goto done_iterarting_on_if;
						}

						auto& valueOfVarOpposite = isdigit(left[0]) ? left : right;

						syntax_tree.fUserValue += "mov " + cRegisters[indexRight + 1] + ", " + valueOfVarOpposite + "\n";
						syntax_tree.fUserValue += "mov " + cRegisters[kRegisterMap.size() - 1] + ", " + valueOfVar + "\n";
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

				syntax_tree.fUserValue += "jge offset [rsp + 4]\n";
			}

			break;
		}
		case CompilerKit::KeywordKind::eKeywordKindFunctionStart: {
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

			syntax_tree.fUserValue = "export .code64 __MPCC_" + fnName + "\n";

			++kLevelFunction;
		}
		case CompilerKit::KeywordKind::eKeywordKindFunctionEnd: {
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
		case CompilerKit::KeywordKind::eKeywordKindEndInstr:
		case CompilerKit::KeywordKind::eKeywordKindVariableInc:
		case CompilerKit::KeywordKind::eKeywordKindVariableDec:
		case CompilerKit::KeywordKind::eKeywordKindVariableAssign: {
			std::string valueOfVar = "";

			if (keyword.first.keyword_kind == CompilerKit::KeywordKind::eKeywordKindVariableInc)
			{
				valueOfVar = text.substr(text.find("+=") + 2);
			}
			else if (keyword.first.keyword_kind == CompilerKit::KeywordKind::eKeywordKindVariableDec)
			{
				valueOfVar = text.substr(text.find("-=") + 2);
			}
			else if (keyword.first.keyword_kind == CompilerKit::KeywordKind::eKeywordKindVariableAssign)
			{
				valueOfVar = text.substr(text.find("=") + 1);
			}
			else if (keyword.first.keyword_kind == CompilerKit::KeywordKind::eKeywordKindEndInstr)
			{
				break;
			}

			while (valueOfVar.find(";") != std::string::npos &&
				   keyword.first.keyword_kind != CompilerKit::KeywordKind::eKeywordKindEndInstr)
			{
				valueOfVar.erase(valueOfVar.find(";"));
			}

			std::string varName = text;

			if (keyword.first.keyword_kind == CompilerKit::KeywordKind::eKeywordKindVariableInc)
			{
				varName.erase(varName.find("+="));
			}
			else if (keyword.first.keyword_kind == CompilerKit::KeywordKind::eKeywordKindVariableDec)
			{
				varName.erase(varName.find("-="));
			}
			else if (keyword.first.keyword_kind == CompilerKit::KeywordKind::eKeywordKindVariableAssign)
			{
				varName.erase(varName.find("="));
			}
			else if (keyword.first.keyword_kind == CompilerKit::KeywordKind::eKeywordKindEndInstr)
			{
				varName.erase(varName.find(";"));
			}

			static bool typeFound = false;

			for (auto& keyword : kKeywords)
			{
				if (keyword.keyword_kind == CompilerKit::eKeywordKindType)
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
				if (keyword.first.keyword_kind == CompilerKit::KeywordKind::eKeywordKindVariableInc)
				{
					detail::print_error("Can't increment variable when it's being created.", file);
				}
				else if (keyword.first.keyword_kind == CompilerKit::KeywordKind::eKeywordKindVariableDec)
				{
					detail::print_error("Can't decrement variable when it's being created.", file);
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
						syntax_tree.fUserValue = instr + cRegisters[kRegisterMap.size() - 1] + ", " + valueOfVar + "\n";
						goto done;
					}

					syntax_tree.fUserValue = instr + cRegisters[kRegisterMap.size() - 1] + ", " + valueOfVar + "\n";
					break;
				}

				if (((int)indexRight - 1) < 0)
				{
					syntax_tree.fUserValue = instr + cRegisters[kRegisterMap.size()] + ", " + valueOfVar + "\n";
				}

			done:
				kRegisterMap.push_back(varName);
			}
			else
			{
				if (kKeywords[keyword.second - 1].keyword_kind == CompilerKit::eKeywordKindType ||
					kKeywords[keyword.second - 1].keyword_kind == CompilerKit::eKeywordKindTypePtr)
				{
					syntax_tree.fUserValue = "\n";
					continue;
				}

				if (keyword.first.keyword_kind == CompilerKit::KeywordKind::eKeywordKindEndInstr)
				{
					syntax_tree.fUserValue = "\n";
					continue;
				}

				if (keyword.first.keyword_kind == CompilerKit::KeywordKind::eKeywordKindVariableInc)
				{
					instr = "add ";
				}
				else if (keyword.first.keyword_kind == CompilerKit::KeywordKind::eKeywordKindVariableDec)
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
					detail::print_error("Variable not declared: " + varErrCpy, file);
				}
			}

			break;
		}
		case CompilerKit::KeywordKind::eKeywordKindReturn: {
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

						syntax_tree.fUserValue = "mov rax, " + cRegisters[indxReg] + "\r\nret\n";
						break;
					}

					if (syntax_tree.fUserValue.empty())
					{
						detail::print_error("Variable not declared: " + subText, file);
					}
				}
				else
				{
					syntax_tree.fUserValue = "mov rax, " + subText + "\r\nret\n";
				}
			}
			else
			{
				syntax_tree.fUserValue = "mov rcx, " + subText + "\n";
				syntax_tree.fUserValue = "mov rax, rcx\r\nret\n";
			}

			break;
		}
		default:
			break;
		}

		syntax_tree.fUserData = keyword.first;
		kState.fSyntaxTree->fLeafList.emplace_back(syntax_tree);
	}

_MpccOkay:
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////

/**
 * @brief C To Assembly mount-point.
 */

/////////////////////////////////////////////////////////////////////////////////////////

class AssemblyCPlusPlusInterface final : public CompilerKit::AssemblyInterface
{
public:
	explicit AssemblyCPlusPlusInterface()  = default;
	~AssemblyCPlusPlusInterface() override = default;

	MPCC_COPY_DEFAULT(AssemblyCPlusPlusInterface);

	[[maybe_unused]]
	static Int32 Arch() noexcept
	{
		return CompilerKit::AssemblyFactory::kArchAMD64;
	}

	Int32 CompileToFormat(std::string& src, Int32 arch) override
	{
		if (arch != AssemblyCPlusPlusInterface::Arch())
			return -1;

		if (kCompilerBackend == nullptr)
			return -1;

		/* @brief copy contents wihtout extension */
		std::string	  src_file = src.data();
		std::ifstream src_fp   = std::ifstream(src_file, std::ios::in);

		const char* cExts[] = kAsmFileExts;

		std::string dest = src_file;
		dest += cExts[4];

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

		auto fmt = CompilerKit::current_date();

		(*kState.fOutputAssembly) << "; Path: " << src_file << "\n";
		(*kState.fOutputAssembly)
			<< "; Language: AMD64 assembly. (Generated from C++)\n";
		(*kState.fOutputAssembly) << "; Date: " << fmt << "\n";
		(*kState.fOutputAssembly) << "#bits 64\n#org 0x1000000"
								  << "\n";

		CompilerKit::SyntaxLeafList syntax;

		kState.fSyntaxTreeList.emplace_back(syntax);
		kState.fSyntaxTree =
			&kState.fSyntaxTreeList[kState.fSyntaxTreeList.size() - 1];

		std::string source;

		while (std::getline(src_fp, source))
		{
			// Compile into an object file.
			kCompilerBackend->Compile(source.c_str(), src.c_str());
		}

		for (auto& ast : kState.fSyntaxTree->fLeafList)
		{
			(*kState.fOutputAssembly) << ast.fUserValue;
		}

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
	kPrintF("%s", "www.zeta.com/developer/cplusplus\n");
}

/////////////////////////////////////////////////////////////////////////////////////////

#define kExtListCxx                          \
	{                                        \
		".cpp", ".cxx", ".cc", ".c++", ".cp" \
	}

NDK_MODULE(CompilerCPlusPlus)
{
	bool skip = false;

	kKeywords.push_back({.keyword_name = "if", .keyword_kind = CompilerKit::eKeywordKindIf});
	kKeywords.push_back({.keyword_name = "else", .keyword_kind = CompilerKit::eKeywordKindElse});
	kKeywords.push_back({.keyword_name = "else if", .keyword_kind = CompilerKit::eKeywordKindElseIf});

	kKeywords.push_back({.keyword_name = "class", .keyword_kind = CompilerKit::eKeywordKindClass});
	kKeywords.push_back({.keyword_name = "struct", .keyword_kind = CompilerKit::eKeywordKindClass});
	kKeywords.push_back({.keyword_name = "namespace", .keyword_kind = CompilerKit::eKeywordKindNamespace});
	kKeywords.push_back({.keyword_name = "typedef", .keyword_kind = CompilerKit::eKeywordKindTypedef});
	kKeywords.push_back({.keyword_name = "using", .keyword_kind = CompilerKit::eKeywordKindTypedef});
	kKeywords.push_back({.keyword_name = "{", .keyword_kind = CompilerKit::eKeywordKindBodyStart});
	kKeywords.push_back({.keyword_name = "}", .keyword_kind = CompilerKit::eKeywordKindBodyEnd});
	kKeywords.push_back({.keyword_name = "auto", .keyword_kind = CompilerKit::eKeywordKindVariable});
	kKeywords.push_back({.keyword_name = "int", .keyword_kind = CompilerKit::eKeywordKindType});
	kKeywords.push_back({.keyword_name = "bool", .keyword_kind = CompilerKit::eKeywordKindType});
	kKeywords.push_back({.keyword_name = "unsigned", .keyword_kind = CompilerKit::eKeywordKindType});
	kKeywords.push_back({.keyword_name = "short", .keyword_kind = CompilerKit::eKeywordKindType});
	kKeywords.push_back({.keyword_name = "char", .keyword_kind = CompilerKit::eKeywordKindType});
	kKeywords.push_back({.keyword_name = "long", .keyword_kind = CompilerKit::eKeywordKindType});
	kKeywords.push_back({.keyword_name = "float", .keyword_kind = CompilerKit::eKeywordKindType});
	kKeywords.push_back({.keyword_name = "double", .keyword_kind = CompilerKit::eKeywordKindType});
	kKeywords.push_back({.keyword_name = "void", .keyword_kind = CompilerKit::eKeywordKindType});

	kKeywords.push_back({.keyword_name = "auto*", .keyword_kind = CompilerKit::eKeywordKindVariablePtr});
	kKeywords.push_back({.keyword_name = "int*", .keyword_kind = CompilerKit::eKeywordKindTypePtr});
	kKeywords.push_back({.keyword_name = "bool*", .keyword_kind = CompilerKit::eKeywordKindTypePtr});
	kKeywords.push_back({.keyword_name = "unsigned*", .keyword_kind = CompilerKit::eKeywordKindTypePtr});
	kKeywords.push_back({.keyword_name = "short*", .keyword_kind = CompilerKit::eKeywordKindTypePtr});
	kKeywords.push_back({.keyword_name = "char*", .keyword_kind = CompilerKit::eKeywordKindTypePtr});
	kKeywords.push_back({.keyword_name = "long*", .keyword_kind = CompilerKit::eKeywordKindTypePtr});
	kKeywords.push_back({.keyword_name = "float*", .keyword_kind = CompilerKit::eKeywordKindTypePtr});
	kKeywords.push_back({.keyword_name = "double*", .keyword_kind = CompilerKit::eKeywordKindTypePtr});
	kKeywords.push_back({.keyword_name = "void*", .keyword_kind = CompilerKit::eKeywordKindTypePtr});

	kKeywords.push_back({.keyword_name = "(", .keyword_kind = CompilerKit::eKeywordKindFunctionStart});
	kKeywords.push_back({.keyword_name = ")", .keyword_kind = CompilerKit::eKeywordKindFunctionEnd});
	kKeywords.push_back({.keyword_name = "=", .keyword_kind = CompilerKit::eKeywordKindVariableAssign});
	kKeywords.push_back({.keyword_name = "+=", .keyword_kind = CompilerKit::eKeywordKindVariableInc});
	kKeywords.push_back({.keyword_name = "-=", .keyword_kind = CompilerKit::eKeywordKindVariableDec});
	kKeywords.push_back({.keyword_name = "const", .keyword_kind = CompilerKit::eKeywordKindConstant});
	kKeywords.push_back({.keyword_name = "*", .keyword_kind = CompilerKit::eKeywordKindPtr});
	kKeywords.push_back({.keyword_name = "->", .keyword_kind = CompilerKit::eKeywordKindPtrAccess});
	kKeywords.push_back({.keyword_name = ".", .keyword_kind = CompilerKit::eKeywordKindAccess});
	kKeywords.push_back({.keyword_name = ",", .keyword_kind = CompilerKit::eKeywordKindArgSeparator});
	kKeywords.push_back({.keyword_name = ";", .keyword_kind = CompilerKit::eKeywordKindEndInstr});
	kKeywords.push_back({.keyword_name = ":", .keyword_kind = CompilerKit::eKeywordKindSpecifier});
	kKeywords.push_back({.keyword_name = "public:", .keyword_kind = CompilerKit::eKeywordKindSpecifier});
	kKeywords.push_back({.keyword_name = "private:", .keyword_kind = CompilerKit::eKeywordKindSpecifier});
	kKeywords.push_back({.keyword_name = "protected:", .keyword_kind = CompilerKit::eKeywordKindSpecifier});
	kKeywords.push_back({.keyword_name = "final", .keyword_kind = CompilerKit::eKeywordKindSpecifier});
	kKeywords.push_back({.keyword_name = "return", .keyword_kind = CompilerKit::eKeywordKindReturn});
	kKeywords.push_back({.keyword_name = "/*", .keyword_kind = CompilerKit::eKeywordKindCommentMultiLineStart});
	kKeywords.push_back({.keyword_name = "*/", .keyword_kind = CompilerKit::eKeywordKindCommentMultiLineStart});
	kKeywords.push_back({.keyword_name = "//", .keyword_kind = CompilerKit::eKeywordKindCommentInline});
	kKeywords.push_back({.keyword_name = "==", .keyword_kind = CompilerKit::eKeywordKindEq});
	kKeywords.push_back({.keyword_name = "!=", .keyword_kind = CompilerKit::eKeywordKindNotEq});
	kKeywords.push_back({.keyword_name = ">=", .keyword_kind = CompilerKit::eKeywordKindGreaterEq});
	kKeywords.push_back({.keyword_name = "<=", .keyword_kind = CompilerKit::eKeywordKindLessEq});

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

			if (strcmp(argv[index], "/v") == 0 ||
				strcmp(argv[index], "/version") == 0)
			{
				kSplashCxx();
				return kOk;
			}

			if (strcmp(argv[index], "/verbose") == 0)
			{
				kState.fVerbose = true;

				continue;
			}

			if (strcmp(argv[index], "/h") == 0 || strcmp(argv[index], "/help") == 0)
			{
				cxx_print_help();

				return kOk;
			}

			if (strcmp(argv[index], "/dialect") == 0)
			{
				if (kCompilerBackend)
					std::cout << kCompilerBackend->Language() << "\n";

				return kOk;
			}

			if (strcmp(argv[index], "/max-errors") == 0)
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

			detail::print_error(err, "cplusplus");

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
				detail::print_error(argv_i + " is not a valid C++ source.\n", "cplusplus");
			}

			return 1;
		}

		if (kFactory.Compile(argv_i, kMachine) != kOk)
			return -1;
	}

	return kOk;
}

// Last rev 8-1-24
//
