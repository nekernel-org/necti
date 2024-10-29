/* -------------------------------------------

	Copyright EL Mahrouss Logic

------------------------------------------- */

/// bugs: 0

/////////////////////////////////////////////////////////////////////////////////////////

// @file Assembler64x0.cxx
// @author Amlal EL Mahrouss
// @brief 64x000 Assembler.

// REMINDER: when dealing with an undefined symbol use (string
// size):LinkerFindSymbol:(string) so that ld will look for it.

/////////////////////////////////////////////////////////////////////////////////////////

#define __ASM_NEED_64x0__ 1

#include <ToolchainKit/AAL/CPU/64x0.h>
#include <ToolchainKit/Parser.h>
#include <ToolchainKit/NFC/AE.h>
#include <ToolchainKit/NFC/PEF.h>
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>

/////////////////////

// ANSI ESCAPE CODES

/////////////////////

#define kBlank	"\e[0;30m"
#define kRed	"\e[0;31m"
#define kWhite	"\e[0;97m"
#define kYellow "\e[0;33m"

#define kStdOut (std::cout << kWhite)
#define kStdErr (std::cout << kRed)

static char	   kOutputArch	   = ToolchainKit::kPefArch64000;
static Boolean kOutputAsBinary = false;

static UInt32 kErrorLimit		= 10;
static UInt32 kAcceptableErrors = 0;

constexpr auto c64x0IPAlignment = 0x4U;

static std::size_t kCounter = 1UL;

static std::uintptr_t									   kOrigin = kPefBaseOrigin;
static std::vector<std::pair<std::string, std::uintptr_t>> kOriginLabel;

static bool kVerbose = false;

static std::vector<e64k_num_t> kBytes;

static ToolchainKit::AERecordHeader kCurrentRecord{
	.fName = "", .fKind = ToolchainKit::kPefCode, .fSize = 0, .fOffset = 0};

static std::vector<ToolchainKit::AERecordHeader> kRecords;
static std::vector<std::string>			kUndefinedSymbols;

static const std::string kUndefinedSymbol = ":UndefinedSymbol:";
static const std::string kRelocSymbol	  = ":RuntimeSymbol:";

// \brief forward decl.
static bool asm_read_attributes(std::string& line);

namespace detail
{
	void print_error_asm(std::string reason, std::string file) noexcept
	{
		if (reason[0] == '\n')
			reason.erase(0, 1);

		kStdErr << kRed << "[ ToolchainKit ] " << kWhite
				<< ((file == "ToolchainKit") ? "InternalErrorException: "
									: ("FileException{ " + file + " }: "))
				<< kBlank << std::endl;
		kStdErr << kRed << "[ ToolchainKit ] " << kWhite << reason << kBlank << std::endl;

		if (kAcceptableErrors > kErrorLimit)
			std::exit(3);

		++kAcceptableErrors;
	}

	void print_warning_asm(std::string reason, std::string file) noexcept
	{
		if (reason[0] == '\n')
			reason.erase(0, 1);

		if (!file.empty())
		{
			kStdOut << kYellow << "[ ToolchainKit ] " << kWhite << file << kBlank << std::endl;
		}

		kStdOut << kYellow << "[ ToolchainKit ] " << kWhite << reason << kBlank << std::endl;
	}
} // namespace detail

/////////////////////////////////////////////////////////////////////////////////////////

// @brief 64x0 assembler entrypoint, the program/module starts here.

/////////////////////////////////////////////////////////////////////////////////////////

TOOLCHAINKIT_MODULE(AssemblerMain64x0)
{
	for (size_t i = 1; i < argc; ++i)
	{
		if (argv[i][0] == '-')
		{
			if (strcmp(argv[i], "--ver") == 0 || strcmp(argv[i], "--v") == 0)
			{
				kStdOut << "Assembler64x0: 64x0 Assembler.\nAssembler64x0: v1.10\nAssembler64x0: Copyright (c) "
						   "EL Mahrouss Logic.\n";
				return 0;
			}
			else if (strcmp(argv[i], "--h") == 0)
			{
				kStdOut << "Assembler64x0: 64x0 Assembler.\nAssembler64x0: Copyright (c) 2024 Mahrouss "
						   "Logic.\n";
				kStdOut << "--version: Print program version.\n";
				kStdOut << "--verbose: Print verbose output.\n";
				kStdOut << "--binary: Output as flat binary.\n";
				kStdOut << "--64xxx: Compile for a subset of the X64000.\n";

				return 0;
			}
			else if (strcmp(argv[i], "--binary") == 0)
			{
				kOutputAsBinary = true;
				continue;
			}
			else if (strcmp(argv[i], "--verbose") == 0)
			{
				kVerbose = true;
				continue;
			}

			kStdOut << "Assembler64x0: ignore " << argv[i] << "\n";
			continue;
		}

		if (!std::filesystem::exists(argv[i]))
		{
			kStdOut << "Assembler64x0: can't open: " << argv[i] << std::endl;
			goto asm_fail_exit;
		}

		std::string object_output(argv[i]);

		for (auto& ext : kAsmFileExts)
		{
			if (object_output.find(ext) != std::string::npos)
			{
				object_output.erase(object_output.find(ext), std::strlen(ext));
			}
		}

		object_output += kOutputAsBinary ? kBinaryFileExt : kObjectFileExt;

		std::ifstream file_ptr(argv[i]);
		std::ofstream file_ptr_out(object_output, std::ofstream::binary);

		if (file_ptr_out.bad())
		{
			if (kVerbose)
			{
				kStdOut << "Assembler64x0: error: " << strerror(errno) << "\n";
			}
		}

		std::string line;

		ToolchainKit::AEHeader hdr{0};

		memset(hdr.fPad, kAENullType, kAEPad);

		hdr.fMagic[0] = kAEMag0;
		hdr.fMagic[1] = kAEMag1;
		hdr.fSize	  = sizeof(ToolchainKit::AEHeader);
		hdr.fArch	  = kOutputArch;

		/////////////////////////////////////////////////////////////////////////////////////////

		// COMPILATION LOOP

		/////////////////////////////////////////////////////////////////////////////////////////

		ToolchainKit::Encoder64x0 asm64;

		while (std::getline(file_ptr, line))
		{
			if (auto ln = asm64.CheckLine(line, argv[i]); !ln.empty())
			{
				detail::print_error_asm(ln, argv[i]);
				continue;
			}

			try
			{
				asm_read_attributes(line);
				asm64.WriteLine(line, argv[i]);
			}
			catch (const std::exception& e)
			{
				if (kVerbose)
				{
					std::string what = e.what();
					detail::print_warning_asm("exit because of: " + what, "ToolchainKit");
				}

				std::filesystem::remove(object_output);
				goto asm_fail_exit;
			}
		}

		if (!kOutputAsBinary)
		{
			if (kVerbose)
			{
				kStdOut << "Assembler64x0: Writing object file...\n";
			}

			// this is the final step, write everything to the file.

			auto pos = file_ptr_out.tellp();

			hdr.fCount = kRecords.size() + kUndefinedSymbols.size();

			file_ptr_out << hdr;

			if (kRecords.empty())
			{
				kStdErr << "Assembler64x0: At least one record is needed to write an object "
						   "file.\nAssembler64x0: Make one using `public_segment .code64 foo_bar`.\n";

				std::filesystem::remove(object_output);
				return -1;
			}

			kRecords[kRecords.size() - 1].fSize = kBytes.size();

			std::size_t record_count = 0UL;

			for (auto& rec : kRecords)
			{
				if (kVerbose)
					kStdOut << "Assembler64x0: Wrote record " << rec.fName << " to file...\n";

				rec.fFlags |= ToolchainKit::kKindRelocationAtRuntime;
				rec.fOffset = record_count;
				++record_count;

				file_ptr_out << rec;
			}

			// increment once again, so that we won't lie about the kUndefinedSymbols.
			++record_count;

			for (auto& sym : kUndefinedSymbols)
			{
				ToolchainKit::AERecordHeader _record_hdr{0};

				if (kVerbose)
					kStdOut << "Assembler64x0: Wrote symbol " << sym << " to file...\n";

				_record_hdr.fKind	= kAENullType;
				_record_hdr.fSize	= sym.size();
				_record_hdr.fOffset = record_count;

				++record_count;

				memset(_record_hdr.fPad, kAENullType, kAEPad);
				memcpy(_record_hdr.fName, sym.c_str(), sym.size());

				file_ptr_out << _record_hdr;

				++kCounter;
			}

			auto pos_end = file_ptr_out.tellp();

			file_ptr_out.seekp(pos);

			hdr.fStartCode = pos_end;
			hdr.fCodeSize  = kBytes.size();

			file_ptr_out << hdr;

			file_ptr_out.seekp(pos_end);
		}
		else
		{
			if (kVerbose)
			{
				kStdOut << "Assembler64x0: Write raw binary...\n";
			}
		}

		// byte from byte, we write this.
		for (auto& byte : kBytes)
		{
			file_ptr_out.write(reinterpret_cast<const char*>(&byte), sizeof(byte));
		}

		if (kVerbose)
			kStdOut << "Assembler64x0: Wrote file with program in it.\n";

		file_ptr_out.flush();
		file_ptr_out.close();

		if (kVerbose)
			kStdOut << "Assembler64x0: Exit succeeded.\n";

		return 0;
	}

asm_fail_exit:

	if (kVerbose)
		kStdOut << "Assembler64x0: Exit failed.\n";

	return -1;
}

/////////////////////////////////////////////////////////////////////////////////////////

// @brief Check for attributes
// returns true if any was found.

/////////////////////////////////////////////////////////////////////////////////////////

static bool asm_read_attributes(std::string& line)
{
	// extern_segment is the opposite of public_segment, it signals to the ld
	// that we need this symbol.
	if (ToolchainKit::find_word(line, "extern_segment"))
	{
		if (kOutputAsBinary)
		{
			detail::print_error_asm("Invalid extern_segment directive in flat binary mode.",
									"ToolchainKit");
			throw std::runtime_error("invalid_extern_segment_bin");
		}

		auto name = line.substr(line.find("extern_segment") + strlen("extern_segment"));

		/// sanity check to avoid stupid linker errors.
		if (name.size() == 0)
		{
			detail::print_error_asm("Invalid extern_segment", "power-as");
			throw std::runtime_error("invalid_extern_segment");
		}

		std::string result = std::to_string(name.size());
		result += kUndefinedSymbol;

		// mangle this
		for (char& j : name)
		{
			if (j == ' ' || j == ',')
				j = '$';
		}

		result += name;

		if (name.find(".code64") != std::string::npos)
		{
			// data is treated as code.
			kCurrentRecord.fKind = ToolchainKit::kPefCode;
		}
		else if (name.find(".data64") != std::string::npos)
		{
			// no code will be executed from here.
			kCurrentRecord.fKind = ToolchainKit::kPefData;
		}
		else if (name.find(".zero64") != std::string::npos)
		{
			// this is a bss section.
			kCurrentRecord.fKind = ToolchainKit::kPefZero;
		}

		// this is a special case for the start stub.
		// we want this so that ld can find it.

		if (name == kPefStart)
		{
			kCurrentRecord.fKind = ToolchainKit::kPefCode;
		}

		// now we can tell the code size of the previous kCurrentRecord.

		if (!kRecords.empty())
			kRecords[kRecords.size() - 1].fSize = kBytes.size();

		memset(kCurrentRecord.fName, 0, kAESymbolLen);
		memcpy(kCurrentRecord.fName, result.c_str(), result.size());

		++kCounter;

		memset(kCurrentRecord.fPad, kAENullType, kAEPad);

		kRecords.emplace_back(kCurrentRecord);

		return true;
	}
	// public_segment is a special keyword used by Assembler64x0 to tell the AE output stage to
	// mark this section as a header. it currently supports .code64, .data64.,
	// .zero64
	else if (ToolchainKit::find_word(line, "public_segment"))
	{
		if (kOutputAsBinary)
		{
			detail::print_error_asm("Invalid public_segment directive in flat binary mode.",
									"ToolchainKit");
			throw std::runtime_error("invalid_public_segment_bin");
		}

		auto name = line.substr(line.find("public_segment") + strlen("public_segment"));

		std::string name_copy = name;

		for (char& j : name)
		{
			if (j == ' ')
				j = '$';
		}

		if (name.find(".code64") != std::string::npos)
		{
			// data is treated as code.

			name_copy.erase(name_copy.find(".code64"), strlen(".code64"));
			kCurrentRecord.fKind = ToolchainKit::kPefCode;
		}
		else if (name.find(".data64") != std::string::npos)
		{
			// no code will be executed from here.

			name_copy.erase(name_copy.find(".data64"), strlen(".data64"));
			kCurrentRecord.fKind = ToolchainKit::kPefData;
		}
		else if (name.find(".zero64") != std::string::npos)
		{
			// this is a bss section.

			name_copy.erase(name_copy.find(".zero64"), strlen(".zero64"));
			kCurrentRecord.fKind = ToolchainKit::kPefZero;
		}

		// this is a special case for the start stub.
		// we want this so that ld can find it.

		if (name == kPefStart)
		{
			kCurrentRecord.fKind = ToolchainKit::kPefCode;
		}

		while (name_copy.find(" ") != std::string::npos)
			name_copy.erase(name_copy.find(" "), 1);

		kOriginLabel.push_back(std::make_pair(name_copy, kOrigin));
		++kOrigin;

		// now we can tell the code size of the previous kCurrentRecord.

		if (!kRecords.empty())
			kRecords[kRecords.size() - 1].fSize = kBytes.size();

		memset(kCurrentRecord.fName, 0, kAESymbolLen);
		memcpy(kCurrentRecord.fName, name.c_str(), name.size());

		++kCounter;

		memset(kCurrentRecord.fPad, kAENullType, kAEPad);

		kRecords.emplace_back(kCurrentRecord);

		return true;
	}

	return false;
}

// \brief algorithms and helpers.

namespace detail::algorithm
{
	// \brief authorize a brief set of characters.
	static inline bool is_not_alnum_space(char c)
	{
		return !(isalpha(c) || isdigit(c) || (c == ' ') || (c == '\t') ||
				 (c == ',') || (c == '(') || (c == ')') || (c == '"') ||
				 (c == '\'') || (c == '[') || (c == ']') || (c == '+') ||
				 (c == '_') || (c == ':') || (c == '@') || (c == '.'));
	}

	bool is_valid_64x0(const std::string& str)
	{
		return std::find_if(str.begin(), str.end(), is_not_alnum_space) == str.end();
	}
} // namespace detail::algorithm

/////////////////////////////////////////////////////////////////////////////////////////

// @brief Check for line (syntax check)

/////////////////////////////////////////////////////////////////////////////////////////

std::string ToolchainKit::Encoder64x0::CheckLine(std::string&	   line,
										const std::string& file)
{
	std::string err_str;

	if (line.empty() || ToolchainKit::find_word(line, "extern_segment") ||
		ToolchainKit::find_word(line, "public_segment") ||
		line.find('#') != std::string::npos || ToolchainKit::find_word(line, ";"))
	{
		if (line.find('#') != std::string::npos)
		{
			line.erase(line.find('#'));
		}
		else if (line.find(';') != std::string::npos)
		{
			line.erase(line.find(';'));
		}
		else
		{
			// now check the line for validity
			if (!detail::algorithm::is_valid_64x0(line))
			{
				err_str = "Line contains non alphanumeric characters.\nhere -> ";
				err_str += line;
			}
		}

		return err_str;
	}

	if (!detail::algorithm::is_valid_64x0(line))
	{
		err_str = "Line contains non alphanumeric characters.\nhere -> ";
		err_str += line;

		return err_str;
	}

	// check for a valid instruction format.

	if (line.find(',') != std::string::npos)
	{
		if (line.find(',') + 1 == line.size())
		{
			err_str += "\nInstruction lacks right register, here -> ";
			err_str += line.substr(line.find(','));

			return err_str;
		}
		else
		{
			bool nothing_on_right = true;

			if (line.find(',') + 1 > line.size())
			{
				err_str += "\nInstruction not complete, here -> ";
				err_str += line;

				return err_str;
			}

			auto substr = line.substr(line.find(',') + 1);

			for (auto& ch : substr)
			{
				if (ch != ' ' && ch != '\t')
				{
					nothing_on_right = false;
				}
			}

			// this means we found nothing after that ',' .
			if (nothing_on_right)
			{
				err_str += "\nInstruction not complete, here -> ";
				err_str += line;

				return err_str;
			}
		}
	}

	// these do take an argument.
	std::vector<std::string> operands_inst = {"stw", "ldw", "lda", "sta"};

	// these don't.
	std::vector<std::string> filter_inst = {"jlr", "jrl", "int"};

	for (auto& opcode64x0 : kOpcodes64x0)
	{
		if (line.find(opcode64x0.fName) != std::string::npos)
		{
			if (opcode64x0.fFunct7 == kAsmNoArgs)
				return err_str;

			for (auto& op : operands_inst)
			{
				// if only the instruction was found.
				if (line == op)
				{
					err_str += "\nMalformed ";
					err_str += op;
					err_str += " instruction, here -> ";
					err_str += line;
				}
			}

			// if it is like that -> addr1, 0x0
			if (auto it = std::find(filter_inst.begin(), filter_inst.end(),
									opcode64x0.fName);
				it == filter_inst.cend())
			{
				if (ToolchainKit::find_word(line, opcode64x0.fName))
				{
					if (!isspace(line[line.find(opcode64x0.fName) +
									  strlen(opcode64x0.fName)]))
					{
						err_str += "\nMissing space between ";
						err_str += opcode64x0.fName;
						err_str += " and operands.\nhere -> ";
						err_str += line;
					}
				}
			}

			return err_str;
		}
	}

	err_str += "Unrecognized instruction: " + line;

	return err_str;
}

bool ToolchainKit::Encoder64x0::WriteNumber(const std::size_t& pos,
								   std::string&		  jump_label)
{
	if (!isdigit(jump_label[pos]))
		return false;

	switch (jump_label[pos + 1])
	{
	case 'x': {
		if (auto res = strtol(jump_label.substr(pos + 2).c_str(), nullptr, 16);
			!res)
		{
			if (errno != 0)
			{
				detail::print_error_asm("invalid hex number: " + jump_label, "ToolchainKit");
				throw std::runtime_error("invalid_hex_number");
			}
		}

		ToolchainKit::NumberCast64 num(
			strtol(jump_label.substr(pos + 2).c_str(), nullptr, 16));

		for (char& i : num.number)
		{
			kBytes.push_back(i);
		}

		if (kVerbose)
		{
			kStdOut << "Assembler64x0: found a base 16 number here: "
					<< jump_label.substr(pos) << "\n";
		}

		return true;
	}
	case 'b': {
		if (auto res = strtol(jump_label.substr(pos + 2).c_str(), nullptr, 2);
			!res)
		{
			if (errno != 0)
			{
				detail::print_error_asm("invalid binary number: " + jump_label, "ToolchainKit");
				throw std::runtime_error("invalid_bin");
			}
		}

		ToolchainKit::NumberCast64 num(
			strtol(jump_label.substr(pos + 2).c_str(), nullptr, 2));

		if (kVerbose)
		{
			kStdOut << "Assembler64x0: found a base 2 number here: "
					<< jump_label.substr(pos) << "\n";
		}

		for (char& i : num.number)
		{
			kBytes.push_back(i);
		}

		return true;
	}
	case 'o': {
		if (auto res = strtol(jump_label.substr(pos + 2).c_str(), nullptr, 7);
			!res)
		{
			if (errno != 0)
			{
				detail::print_error_asm("invalid octal number: " + jump_label, "ToolchainKit");
				throw std::runtime_error("invalid_octal");
			}
		}

		ToolchainKit::NumberCast64 num(
			strtol(jump_label.substr(pos + 2).c_str(), nullptr, 7));

		if (kVerbose)
		{
			kStdOut << "Assembler64x0: found a base 8 number here: "
					<< jump_label.substr(pos) << "\n";
		}

		for (char& i : num.number)
		{
			kBytes.push_back(i);
		}

		return true;
	}
	default: {
		break;
	}
	}

	/* check for errno and stuff like that */
	if (auto res = strtol(jump_label.substr(pos).c_str(), nullptr, 10); !res)
	{
		if (errno != 0)
		{
			return false;
		}
	}

	ToolchainKit::NumberCast64 num(
		strtol(jump_label.substr(pos).c_str(), nullptr, 10));

	for (char& i : num.number)
	{
		kBytes.push_back(i);
	}

	if (kVerbose)
	{
		kStdOut << "Assembler64x0: found a base 10 number here: " << jump_label.substr(pos)
				<< "\n";
	}

	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////

// @brief Read and write an instruction to the output array.

/////////////////////////////////////////////////////////////////////////////////////////

bool ToolchainKit::Encoder64x0::WriteLine(std::string&		line,
								 const std::string& file)
{
	if (ToolchainKit::find_word(line, "public_segment "))
		return true;

	for (auto& opcode64x0 : kOpcodes64x0)
	{
		// strict check here
		if (ToolchainKit::find_word(line, opcode64x0.fName) &&
			detail::algorithm::is_valid_64x0(line))
		{
			std::string name(opcode64x0.fName);
			std::string jump_label, cpy_jump_label;

			kBytes.emplace_back(opcode64x0.fOpcode);
			kBytes.emplace_back(opcode64x0.fFunct3);
			kBytes.emplace_back(opcode64x0.fFunct7);

			// check funct7 type.
			switch (opcode64x0.fFunct7)
			{
			// reg to reg means register to register transfer operation.
			case kAsmRegToReg:
			case kAsmImmediate: {
				// \brief how many registers we found.
				std::size_t found_some = 0UL;

				for (size_t line_index = 0UL; line_index < line.size();
					 line_index++)
				{
					if (line[line_index] == kAsmRegisterPrefix[0] &&
						isdigit(line[line_index + 1]))
					{
						std::string register_syntax = kAsmRegisterPrefix;
						register_syntax += line[line_index + 1];

						if (isdigit(line[line_index + 2]))
							register_syntax += line[line_index + 2];

						std::string reg_str;
						reg_str += line[line_index + 1];

						if (isdigit(line[line_index + 2]))
							reg_str += line[line_index + 2];

						// it ranges from r0 to r19
						// something like r190 doesn't exist in the instruction set.
						if (kOutputArch == ToolchainKit::kPefArch64000)
						{
							if (isdigit(line[line_index + 3]) &&
								isdigit(line[line_index + 2]))
							{
								reg_str += line[line_index + 3];
								detail::print_error_asm(
									"invalid register index, r" + reg_str +
										"\nnote: The 64x0 accepts registers from r0 to r20.",
									file);
								throw std::runtime_error("invalid_register_index");
							}
						}

						// finally cast to a size_t
						std::size_t reg_index = strtol(reg_str.c_str(), nullptr, 10);

						if (reg_index > kAsmRegisterLimit)
						{
							detail::print_error_asm("invalid register index, r" + reg_str,
													file);
							throw std::runtime_error("invalid_register_index");
						}

						kBytes.emplace_back(reg_index);
						++found_some;

						if (kVerbose)
						{
							kStdOut << "Assembler64x0: Register found: " << register_syntax << "\n";
							kStdOut << "Assembler64x0: Register amount in instruction: "
									<< found_some << "\n";
						}
					}
				}

				// we're not in immediate addressing, reg to reg.
				if (opcode64x0.fFunct7 != kAsmImmediate)
				{
					// remember! register to register!
					if (found_some == 1)
					{
						detail::print_error_asm(
							"Too few registers.\ntip: each Assembler64x0 register "
							"starts with 'r'.\nline: " +
								line,
							file);
						throw std::runtime_error("not_a_register");
					}
				}

				if (found_some < 1 && name != "ldw" && name != "lda" &&
					name != "stw")
				{
					detail::print_error_asm(
						"invalid combination of opcode and registers.\nline: " + line,
						file);
					throw std::runtime_error("invalid_comb_op_reg");
				}
				else if (found_some == 1 && name == "add")
				{
					detail::print_error_asm(
						"invalid combination of opcode and registers.\nline: " + line,
						file);
					throw std::runtime_error("invalid_comb_op_reg");
				}
				else if (found_some == 1 && name == "sub")
				{
					detail::print_error_asm(
						"invalid combination of opcode and registers.\nline: " + line,
						file);
					throw std::runtime_error("invalid_comb_op_reg");
				}

				if (found_some > 0 && name == "pop")
				{
					detail::print_error_asm(
						"invalid combination for opcode 'pop'.\ntip: it expects "
						"nothing.\nline: " +
							line,
						file);
					throw std::runtime_error("invalid_comb_op_pop");
				}
			}
			default:
				break;
			}

			// try to fetch a number from the name
			if (name == "stw" || name == "ldw" || name == "lda" || name == "sta")
			{
				auto where_string = name;

				// if we load something, we'd need it's symbol/literal
				if (name == "stw" || name == "sta" || name == "ldw" || name == "lda" ||
					name == "sta")
					where_string = ",";

				jump_label = line;

				auto found_sym = false;

				while (jump_label.find(where_string) != std::string::npos)
				{
					jump_label = jump_label.substr(jump_label.find(where_string) +
												   where_string.size());

					while (jump_label.find(" ") != std::string::npos)
					{
						jump_label.erase(jump_label.find(" "), 1);
					}

					if (jump_label[0] != kAsmRegisterPrefix[0] &&
						!isdigit(jump_label[1]))
					{
						if (found_sym)
						{
							detail::print_error_asm(
								"invalid combination of opcode and operands.\nhere -> " +
									jump_label,
								file);
							throw std::runtime_error("invalid_comb_op_ops");
						}
						else
						{
							// death trap installed.
							found_sym = true;
						}
					}
				}

				cpy_jump_label = jump_label;

				// replace any spaces with $
				if (jump_label[0] == ' ')
				{
					while (jump_label.find(' ') != std::string::npos)
					{
						if (isalnum(jump_label[0]) || isdigit(jump_label[0]))
							break;

						jump_label.erase(jump_label.find(' '), 1);
					}
				}

				if (!this->WriteNumber(0, jump_label))
				{
					// sta expects this: sta 0x000000, r0
					if (name == "sta")
					{
						detail::print_error_asm(
							"invalid combination of opcode and operands.\nHere ->" + line,
							file);
						throw std::runtime_error("invalid_comb_op_ops");
					}
				}
				else
				{
					if (name == "sta" &&
						cpy_jump_label.find("extern_segment ") != std::string::npos)
					{
						detail::print_error_asm("invalid usage extern_segment on 'sta', here: " + line,
												file);
						throw std::runtime_error("invalid_sta_usage");
					}
				}

				goto asm_write_label;
			}

			// This is the case where we jump to a label, it is also used as a goto.
			if (name == "lda" || name == "sta")
			{
			asm_write_label:
				if (cpy_jump_label.find('\n') != std::string::npos)
					cpy_jump_label.erase(cpy_jump_label.find('\n'), 1);

				if (cpy_jump_label.find("extern_segment") != std::string::npos)
				{
					cpy_jump_label.erase(cpy_jump_label.find("extern_segment"), strlen("extern_segment"));

					if (name == "sta")
					{
						detail::print_error_asm("extern_segment is not allowed on a sta operation.",
												file);
						throw std::runtime_error("extern_segment_sta_op");
					}
					else
					{
						goto asm_end_label_cpy;
					}
				}

				if (name == "lda" || name == "sta")
				{
					for (auto& label : kOriginLabel)
					{
						if (cpy_jump_label == label.first)
						{
							if (kVerbose)
							{
								kStdOut << "Assembler64x0: Replace label " << cpy_jump_label
										<< " to address: " << label.second << std::endl;
							}

							ToolchainKit::NumberCast64 num(label.second);

							for (auto& num : num.number)
							{
								kBytes.push_back(num);
							}

							goto asm_end_label_cpy;
						}
					}

					if (cpy_jump_label[0] == '0')
					{
						switch (cpy_jump_label[1])
						{
						case 'x':
						case 'o':
						case 'b':
							if (this->WriteNumber(0, cpy_jump_label))
								goto asm_end_label_cpy;

							break;
						default:
							break;
						}

						if (isdigit(cpy_jump_label[0]))
						{
							if (this->WriteNumber(0, cpy_jump_label))
								goto asm_end_label_cpy;

							break;
						}
					}
				}

				if (cpy_jump_label.size() < 1)
				{
					detail::print_error_asm("label is empty, can't jump on it.", file);
					throw std::runtime_error("label_empty");
				}

				/// don't go any further if:
				/// load word (ldw) or store word. (stw)

				if (name == "ldw" || name == "stw")
					break;

				auto mld_reloc_str = std::to_string(cpy_jump_label.size());
				mld_reloc_str += kUndefinedSymbol;
				mld_reloc_str += cpy_jump_label;

				bool ignore_back_slash = false;

				for (auto& reloc_chr : mld_reloc_str)
				{
					if (reloc_chr == '\\')
					{
						ignore_back_slash = true;
						continue;
					}

					if (ignore_back_slash)
					{
						ignore_back_slash = false;
						continue;
					}

					kBytes.push_back(reloc_chr);
				}

				kBytes.push_back('\0');
				goto asm_end_label_cpy;
			}

		asm_end_label_cpy:
			kOrigin += c64x0IPAlignment;

			break;
		}
	}

	return true;
}

// Last rev 13-1-24
