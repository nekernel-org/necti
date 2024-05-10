/*
 *	========================================================
 *
 *	ccplus
 * 	Copyright SoftwareLabs, all rights reserved.
 *
 * 	========================================================
 */

/// bugs: 0

#define __PK_USE_STRUCT_INSTEAD__ 1

#define kPrintF printf

#define kSplashCxx() \
	kPrintF(kWhite "%s\n", "LightSpeed C++ Compiler, Copyright SoftwareLabs.")

#include <Headers/AsmKit/CPU/amd64.hpp>
#include <Headers/ParserKit.hpp>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#define kOk 0

/* SoftwareLabs C++ driver */
/* This is part of CodeTools C++ compiler. */
/* (c) SoftwareLabs */

// @author Amlal El Mahrouss (amlel)
// @file cc.cc
// @brief Optimized C++ Compiler.

/////////////////////

// ANSI ESCAPE CODES

/////////////////////

#define kBlank "\e[0;30m"
#define kRed   "\e[0;31m"
#define kWhite "\e[0;97m"

/////////////////////////////////////

// INTERNAL STUFF OF THE C COMPILER

/////////////////////////////////////

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
		std::vector<ParserKit::SyntaxLeafList> fSyntaxTreeList;
		std::vector<CompilerRegisterMap>	   kStackFrame;
		std::vector<CompilerStructMap>		   kStructMap;
		ParserKit::SyntaxLeafList*			   fSyntaxTree{nullptr};
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
			std::cout << kRed << "[ ccplus ] " << kWhite
					  << ((file == "ccplus") ? "internal compiler error "
											 : ("in file, " + file))
					  << kBlank << std::endl;
			std::cout << kRed << "[ ccplus ] " << kWhite << reason << kBlank
					  << std::endl;

			kState.fLastFile = file;
		}
		else
		{
			std::cout << kRed << "[ ccplus ] [ " << kState.fLastFile << " ] " << kWhite
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
static std::vector<ParserKit::CompilerKeyword> kKeywords;

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

/* @brief C++ compiler backend for SoftwareLabs C++ */
class CompilerBackendCPlusPlus final : public ParserKit::CompilerBackend
{
public:
	explicit CompilerBackendCPlusPlus()	 = default;
	~CompilerBackendCPlusPlus() override = default;

	MPCC_COPY_DEFAULT(CompilerBackendCPlusPlus);

	bool Compile(const std::string& text, const char* file) override;

	const char* Language() override;
};

/// compiler variables

static CompilerBackendCPlusPlus*		 kCompilerBackend = nullptr;
static std::vector<detail::CompilerType> kCompilerVariables;
static std::vector<std::string>			 kCompilerFunctions;

/// detail namespaces

namespace detail
{
	union number_cast final {
		number_cast(UInt64 raw)
			: raw(raw)
		{
		}

		char   number[8];
		UInt64 raw;
	};
} // namespace detail

const char* CompilerBackendCPlusPlus::Language()
{
	return "C++";
}

/////////////////////////////////////////////////////////////////////////////////////////

// @name Compile
// @brief Generate MASM from a C++ source.

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
	std::vector<std::pair<ParserKit::CompilerKeyword, std::size_t>> keywords_list;

	bool found = false;

	for (auto& keyword : kKeywords)
	{
		if (text.find(keyword.keyword_name) != std::string::npos)
		{
			keywords_list.emplace_back(std::make_pair(keyword, index));
			++index;

			found = true;
		}
	}

	if (!found)
	{
		detail::print_error("syntax error: " + text, file);
	}

	// TODO: sort keywords

	for (auto& keyword : keywords_list)
	{
		auto syntax_tree = ParserKit::SyntaxLeafList::SyntaxLeaf();

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

class AssemblyMountpointClang final : public CompilerKit::AssemblyInterface
{
public:
	explicit AssemblyMountpointClang()	= default;
	~AssemblyMountpointClang() override = default;

	MPCC_COPY_DEFAULT(AssemblyMountpointClang);

	[[maybe_unused]] static Int32 Arch() noexcept
	{
		return CompilerKit::AssemblyFactory::kArchAMD64;
	}

	Int32 CompileToFormat(std::string& src, Int32 arch) override
	{
		if (arch != AssemblyMountpointClang::Arch())
			return -1;

		if (kCompilerBackend == nullptr)
			return -1;

		/* @brief copy contents wihtout extension */
		std::string	  src_file = src.data();
		std::ifstream src_fp   = std::ifstream(src_file, std::ios::in);
		std::string	  dest;

		for (auto& ch : src_file)
		{
			if (ch == '.')
			{
				break;
			}

			dest += ch;
		}

		/* According to PEF ABI. */

		std::vector<const char*> exts = kAsmFileExts;
		dest += exts[3];

		kState.fOutputAssembly = std::make_unique<std::ofstream>(dest);

		auto fmt = CompilerKit::current_date();

		(*kState.fOutputAssembly) << "; Path: " << src_file << "\n";
		(*kState.fOutputAssembly)
			<< "; Language: CodeTools assembly. (Generated from C++)\n";
		(*kState.fOutputAssembly) << "; Date: " << fmt << "\n\n";
		(*kState.fOutputAssembly) << "#bits 64\n\n#org 0x1000000"
								  << "\n\n";

		ParserKit::SyntaxLeafList syntax;

		kState.fSyntaxTreeList.emplace_back(syntax);
		kState.fSyntaxTree =
			&kState.fSyntaxTreeList[kState.fSyntaxTreeList.size() - 1];

		std::string source;

		while (std::getline(src_fp, source))
		{
			// Compile into an object file.
			kCompilerBackend->Compile(source.c_str(), src.data());
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
	kPrintF("%s", "No help available, see:\r\n");
	kPrintF("%s", "www.el-mahrouss-logic.com/developer/newos/cplusplus\r\n");
}

/////////////////////////////////////////////////////////////////////////////////////////

#define kExtListCxx                          \
	{                                        \
		".cpp", ".cxx", ".cc", ".c++", ".cp" \
	}

MPCC_MODULE(CompilerCPlusPlus)
{
	bool skip = false;

	kKeywords.push_back({.keyword_name = "class", .keyword_kind = ParserKit::eKeywordKindClass});
	kKeywords.push_back({.keyword_name = "struct", .keyword_kind = ParserKit::eKeywordKindClass});
	kKeywords.push_back({.keyword_name = "namespace", .keyword_kind = ParserKit::eKeywordKindNamespace});
	kKeywords.push_back({.keyword_name = "typedef", .keyword_kind = ParserKit::eKeywordKindTypedef});
	kKeywords.push_back({.keyword_name = "using", .keyword_kind = ParserKit::eKeywordKindTypedef});
	kKeywords.push_back({.keyword_name = "}", .keyword_kind = ParserKit::eKeywordKindBodyStart});
	kKeywords.push_back({.keyword_name = "{", .keyword_kind = ParserKit::eKeywordKindBodyEnd});
	kKeywords.push_back({.keyword_name = "auto", .keyword_kind = ParserKit::eKeywordKindVariable});
	kKeywords.push_back({.keyword_name = "=", .keyword_kind = ParserKit::eKeywordKindVariableAssign});
	kKeywords.push_back({.keyword_name = "const", .keyword_kind = ParserKit::eKeywordKindConstant});
	kKeywords.push_back({.keyword_name = "->", .keyword_kind = ParserKit::eKeywordKindPtrAccess});
	kKeywords.push_back({.keyword_name = ".", .keyword_kind = ParserKit::eKeywordKindAccess});
	kKeywords.push_back({.keyword_name = ",", .keyword_kind = ParserKit::eKeywordKindArgSeparator});
	kKeywords.push_back({.keyword_name = ";", .keyword_kind = ParserKit::eKeywordKindEndInstr});
	kKeywords.push_back({.keyword_name = ":", .keyword_kind = ParserKit::eKeywordKindSpecifier});
	kKeywords.push_back({.keyword_name = "public:", .keyword_kind = ParserKit::eKeywordKindSpecifier});
	kKeywords.push_back({.keyword_name = "private:", .keyword_kind = ParserKit::eKeywordKindSpecifier});
	kKeywords.push_back({.keyword_name = "protected:", .keyword_kind = ParserKit::eKeywordKindSpecifier});
	kKeywords.push_back({.keyword_name = "final", .keyword_kind = ParserKit::eKeywordKindSpecifier});

	kFactory.Mount(new AssemblyMountpointClang());
	kCompilerBackend = new CompilerBackendCPlusPlus();

	for (auto index = 1UL; index < argc; ++index)
	{
		if (argv[index][0] == '-')
		{
			if (skip)
			{
				skip = false;
				continue;
			}

			if (strcmp(argv[index], "-v") == 0 ||
				strcmp(argv[index], "-version") == 0)
			{
				kSplashCxx();
				return kOk;
			}

			if (strcmp(argv[index], "-verbose") == 0)
			{
				kState.fVerbose = true;

				continue;
			}

			if (strcmp(argv[index], "-h") == 0 || strcmp(argv[index], "-help") == 0)
			{
				cxx_print_help();

				return kOk;
			}

			if (strcmp(argv[index], "-dialect") == 0)
			{
				if (kCompilerBackend)
					std::cout << kCompilerBackend->Language() << "\n";

				return kOk;
			}

			if (strcmp(argv[index], "-max-errors") == 0)
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

			detail::print_error(err, "ccplus");

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
				detail::print_error(argv_i + " is not a valid C++ source.\n", "ccplus");
			}

			return 1;
		}

		if (kFactory.Compile(argv_i, kMachine) != kOk)
			return -1;
	}

	return kOk;
}

// Last rev 8-1-24
