/* -------------------------------------------

	Copyright ZKA Web Services Co

	FILE: Linker.cxx
	PURPOSE: C++ LINKER

------------------------------------------- */

/// @file Linker.cxx
/// @author Amlal EL Mahrouss (amlel)
/// @brief ZKA Linker.

/// Last Rev: Sat Feb 24 CET 2024

/// @note Do not look up for anything with .code64/.data64/.zero64!
/// It will be loaded when program will start up!

#include <ToolchainKit/NFC/ErrorID.h>

//! Assembler Kit
#include <ToolchainKit/AAL/Asm.h>

//! Preferred Executable Format
#include <ToolchainKit/NFC/PEF.h>
#include <ToolchainKit/UUID.h>
//! Dist version
#include <ToolchainKit/Version.h>

//! Advanced Executable Object Format
#include <ToolchainKit/NFC/AE.h>

#define kLinkerVersion "ZKA Linker Driver %s, (c) ZKA Web Services Co 2024, all rights reserved.\n"

#define StringCompare(DST, SRC) strcmp(DST, SRC)

#define kPefNoCpu	 0U
#define kPefNoSubCpu 0U

#define kWhite	"\e[0;97m"
#define kStdOut (std::cout << kWhite)

#define kLinkerDefaultOrigin kPefBaseOrigin
#define kLinkerId			 0x5046FF
#define kLinkerAbiContainer	 "Container:Abi:"

/// @brief PEF stack size symbol.
#define kLinkerStackSizeSymbol "SizeOfReserveStack"

enum
{
	eABIStart	= 0x1010, /* Invalid ABI start of ABI list. */
	eABINewOS	= 0x5046, /* PF (ZKA PEF ABI) */
	eABIInvalid = 0xFFFF,
};

static ToolchainKit::String kOutput			 = "";
static Int32	   kAbi				 = eABINewOS;
static Int32	   kSubArch			 = kPefNoSubCpu;
static Int32	   kArch			 = ToolchainKit::kPefArchInvalid;
static Bool		   kFatBinaryEnable	 = false;
static Bool		   kStartFound		 = false;
static Bool		   kDuplicateSymbols = false;
static Bool		   kVerbose			 = false;

/* link is to be found, mld is to be found at runtime. */
static const char* kLdDefineSymbol = ":UndefinedSymbol:";
static const char* kLdDynamicSym   = ":RuntimeSymbol:";

/* object code and list. */
static std::vector<ToolchainKit::String> kObjectList;
static std::vector<char>		kObjectBytes;

static uintptr_t kMIBCount = 8;

#define kPrintF			printf
#define kLinkerSplash() kPrintF(kWhite kLinkerVersion, kDistVersion)

/***
	@brief ZKA linker main
*/

TOOLCHAINKIT_MODULE(ZKALinkerMain)
{
	bool is_executable = true;

	/**
	 * @brief parse flags and such.
	 *
	 */
	for (size_t linker_arg = 1;	linker_arg < argc; ++linker_arg)
	{
		if (StringCompare(argv[linker_arg], "--link:?") == 0)
		{
			kLinkerSplash();

			kStdOut << "--link:ver: Show linker version.\n";
			kStdOut << "--link:?: Show linker help.\n";
			kStdOut << "--link:verbose: Enable linker trace.\n";
			kStdOut << "--link:dll: Output as a shared PEF.\n";
			kStdOut << "--link:fat: Output as a FAT PEF.\n";
			kStdOut << "--link:32k: Output as a 32x0 PEF.\n";
			kStdOut << "--link:64k: Output as a 64x0 PEF.\n";
			kStdOut << "--link:amd64: Output as a AMD64 PEF.\n";
			kStdOut << "--link:rv64: Output as a RISC-V PEF.\n";
			kStdOut << "--link:power64: Output as a POWER PEF.\n";
			kStdOut << "--link:arm64: Output as a ARM64 PEF.\n";
			kStdOut << "--link:output: Select the output file name.\n";

			return 0;
		}
		else if (StringCompare(argv[linker_arg], "--link:ver") == 0)
		{
			kLinkerSplash();
			return 0;
		}
		else if (StringCompare(argv[linker_arg], "--link:fat-binary") == 0)
		{
			kFatBinaryEnable = true;

			continue;
		}
		else if (StringCompare(argv[linker_arg], "--link:64k") == 0)
		{
			kArch = ToolchainKit::kPefArch64000;

			continue;
		}
		else if (StringCompare(argv[linker_arg], "--link:amd64") == 0)
		{
			kArch = ToolchainKit::kPefArchAMD64;

			continue;
		}
		else if (StringCompare(argv[linker_arg], "--link:32k") == 0)
		{
			kArch = ToolchainKit::kPefArch32000;

			continue;
		}
		else if (StringCompare(argv[linker_arg], "--link:power64") == 0)
		{
			kArch = ToolchainKit::kPefArchPowerPC;

			continue;
		}
		else if (StringCompare(argv[linker_arg], "--link:riscv64") == 0)
		{
			kArch = ToolchainKit::kPefArchRISCV;

			continue;
		}
		else if (StringCompare(argv[linker_arg], "--link:arm64") == 0)
		{
			kArch = ToolchainKit::kPefArchARM64;

			continue;
		}
		else if (StringCompare(argv[linker_arg], "--link:verbose") == 0)
		{
			kVerbose = true;

			continue;
		}
		else if (StringCompare(argv[linker_arg], "--link:dll") == 0)
		{
			if (kOutput.empty())
			{
				continue;
			}

			if (kOutput.find(kPefExt) != ToolchainKit::String::npos)
				kOutput.erase(kOutput.find(kPefExt), strlen(kPefExt));

			kOutput += kPefDylibExt;

			is_executable = false;

			continue;
		}
		else if (StringCompare(argv[linker_arg], "--link:output") == 0)
		{
			kOutput = argv[linker_arg + 1];
			++linker_arg;

			continue;
		}
		else
		{
			if (argv[linker_arg][0] == '-')
			{
				kStdOut << "link: unknown flag: " << argv[linker_arg] << "\n";
				continue;
			}

			kObjectList.emplace_back(argv[linker_arg]);

			continue;
		}
	}

	if (kOutput.empty())
	{
		kStdOut << "link: no output filename set." << std::endl;
		return TOOLCHAINKIT_EXEC_ERROR;
	}

	// sanity check.
	if (kObjectList.empty())
	{
		kStdOut << "link: no input files." << std::endl;
		return TOOLCHAINKIT_EXEC_ERROR;
	}
	else
	{
		namespace fs = std::filesystem;

		// check for existing files, if they don't throw an error.
		for (auto& obj : kObjectList)
		{
			if (!fs::exists(obj))
			{
				// if filesystem doesn't find file
				//          -> throw error.
				kStdOut << "link: no such file: " << obj << std::endl;
				return TOOLCHAINKIT_EXEC_ERROR;
			}
		}
	}

	// PEF expects a valid target architecture when outputing a binary.
	if (kArch == 0)
	{
		kStdOut << "link: no target architecture set, can't continue." << std::endl;
		return TOOLCHAINKIT_EXEC_ERROR;
	}

	ToolchainKit::PEFContainer pef_container{};

	int32_t archs = kArch;

	pef_container.Count	   = 0UL;
	pef_container.Kind	   = ToolchainKit::kPefKindExec;
	pef_container.SubCpu   = kSubArch;
	pef_container.Linker   = kLinkerId; // ZKA Web Services Co Linker
	pef_container.Abi	   = kAbi;		// Multi-Processor UX ABI
	pef_container.Magic[0] = kPefMagic[kFatBinaryEnable ? 2 : 0];
	pef_container.Magic[1] = kPefMagic[1];
	pef_container.Magic[2] = kPefMagic[kFatBinaryEnable ? 0 : 2];
	pef_container.Magic[3] = kPefMagic[3];
	pef_container.Version  = kPefVersion;

	// specify the start address, can be 0x10000
	pef_container.Start = kLinkerDefaultOrigin;
	pef_container.HdrSz = sizeof(ToolchainKit::PEFContainer);

	std::ofstream output_fc(kOutput, std::ofstream::binary);

	if (output_fc.bad())
	{
		if (kVerbose)
		{
			kStdOut << "link: error: " << strerror(errno) << "\n";
		}

		return TOOLCHAINKIT_FILE_NOT_FOUND;
	}

	//! Read AE to convert as PEF.

	std::vector<ToolchainKit::PEFCommandHeader> command_headers;
	ToolchainKit::Utils::AEReadableProtocol	   readProto{};

	for (const auto& i : kObjectList)
	{
		if (!std::filesystem::exists(i))
			continue;

		ToolchainKit::AEHeader hdr{};

		readProto.FP = std::ifstream(i, std::ifstream::binary);
		readProto.FP >> hdr;

		auto ae_header = hdr;

		if (ae_header.fMagic[0] == kAEMag0 && ae_header.fMagic[1] == kAEMag1 &&
			ae_header.fSize == sizeof(ToolchainKit::AEHeader))
		{
			if (ae_header.fArch != kArch)
			{
				if (kVerbose)
					kStdOut << "link: info: is this a FAT binary? : ";

				if (!kFatBinaryEnable)
				{
					if (kVerbose)
						kStdOut << "No.\n";

					kStdOut << "link: error: object " << i
							<< " is a different kind of architecture and output isn't "
							   "treated as a FAT binary."
							<< std::endl;

					std::remove(kOutput.c_str());
					return TOOLCHAINKIT_FAT_ERROR;
				}
				else
				{
					if (kVerbose)
					{
						kStdOut << "Architecture matches.\n";
					}
				}
			}

			// append arch type to archs varaible.
			archs |= ae_header.fArch;
			std::size_t cnt = ae_header.fCount;

			if (kVerbose)
				kStdOut << "link: object header found, record count: " << cnt << "\n";

			pef_container.Count = cnt;

			char_type* raw_ae_records =
				new char_type[cnt * sizeof(ToolchainKit::AERecordHeader)];
			memset(raw_ae_records, 0, cnt * sizeof(ToolchainKit::AERecordHeader));

			auto* ae_records = readProto.Read(raw_ae_records, cnt);
			for (size_t ae_record_index = 0; ae_record_index < cnt;
				 ++ae_record_index)
			{
				ToolchainKit::PEFCommandHeader command_header{0};
				size_t				  offsetOfData = ae_records[ae_record_index].fOffset + ae_header.fSize;

				memcpy(command_header.Name, ae_records[ae_record_index].fName,
					   kPefNameLen);

				ToolchainKit::String cmd_hdr_name(command_header.Name);

				// check this header if it's any valid.
				if (cmd_hdr_name.find(kPefCode64) ==
						ToolchainKit::String::npos &&
					cmd_hdr_name.find(kPefData64) ==
						ToolchainKit::String::npos &&
					cmd_hdr_name.find(kPefZero64) ==
						ToolchainKit::String::npos)
				{
					if (cmd_hdr_name.find(kPefStart) ==
							ToolchainKit::String::npos &&
						*command_header.Name == 0)
					{
						if (cmd_hdr_name.find(kLdDefineSymbol) !=
							ToolchainKit::String::npos)
						{
							goto ld_mark_header;
						}
						else
						{
							continue;
						}
					}
				}

				if (cmd_hdr_name.find(kPefStart) !=
						ToolchainKit::String::npos &&
					cmd_hdr_name.find(kPefCode64) !=
						ToolchainKit::String::npos)
				{
					kStartFound = true;
				}

			ld_mark_header:
				command_header.Offset = offsetOfData;
				command_header.Kind	  = ae_records[ae_record_index].fKind;
				command_header.Size	  = ae_records[ae_record_index].fSize;
				command_header.Cpu	  = ae_header.fArch;
				command_header.SubCpu = ae_header.fSubArch;

				if (kVerbose)
				{
					kStdOut << "link: object record: "
							<< ae_records[ae_record_index].fName << " was marked.\n";

					kStdOut << "link: object record offset: " << command_header.Offset << "\n";
				}

				command_headers.emplace_back(command_header);
			}

			delete[] raw_ae_records;

			std::vector<char> bytes;
			bytes.resize(ae_header.fCodeSize);

			// TODO: Port this for NeFS filesystems.

			readProto.FP.seekg(std::streamsize(ae_header.fStartCode));
			readProto.FP.read(bytes.data(), std::streamsize(ae_header.fCodeSize));

			for (auto& byte : bytes)
			{
				kObjectBytes.push_back(byte);
			}

			readProto.FP.close();

			continue;
		}

		kStdOut << "link: not an object: " << i << std::endl;
		std::remove(kOutput.c_str());

		// don't continue, it is a fatal error.
		return TOOLCHAINKIT_EXEC_ERROR;
	}

	pef_container.Cpu = archs;

	output_fc << pef_container;

	if (kVerbose)
	{
		kStdOut << "link: wrote container header.\n";
	}

	output_fc.seekp(std::streamsize(pef_container.HdrSz));

	std::vector<ToolchainKit::String> not_found;
	std::vector<ToolchainKit::String> symbols;

	// step 2: check for errors (multiple symbols, undefined ones)

	for (auto& command_hdr : command_headers)
	{
		// check if this symbol needs to be resolved.
		if (ToolchainKit::String(command_hdr.Name).find(kLdDefineSymbol) !=
				ToolchainKit::String::npos &&
			ToolchainKit::String(command_hdr.Name).find(kLdDynamicSym) == ToolchainKit::String::npos)
		{
			if (kVerbose)
				kStdOut << "link: found undefined symbol: " << command_hdr.Name << "\n";

			if (auto it = std::find(not_found.begin(), not_found.end(),
									ToolchainKit::String(command_hdr.Name));
				it == not_found.end())
			{
				not_found.emplace_back(command_hdr.Name);
			}
		}

		symbols.emplace_back(command_hdr.Name);
	}

	// Now try to solve these symbols.

	for (size_t not_found_idx = 0; not_found_idx < command_headers.size();
		 ++not_found_idx)
	{
		if (auto it = std::find(not_found.begin(), not_found.end(),
								ToolchainKit::String(command_headers[not_found_idx].Name));
			it != not_found.end())
		{
			ToolchainKit::String symbol_imp = *it;

			if (symbol_imp.find(kLdDefineSymbol) == ToolchainKit::String::npos)
				continue;

			// erase the lookup prefix.
			symbol_imp.erase(
				0, symbol_imp.find(kLdDefineSymbol) + strlen(kLdDefineSymbol));

			// demangle everything.
			while (symbol_imp.find('$') != ToolchainKit::String::npos)
				symbol_imp.erase(symbol_imp.find('$'), 1);

			// the reason we do is because, this may not match the symbol, and we need
			// to look for other matching symbols.
			for (auto& command_hdr : command_headers)
			{
				if (ToolchainKit::String(command_hdr.Name).find(symbol_imp) !=
						ToolchainKit::String::npos &&
					ToolchainKit::String(command_hdr.Name).find(kLdDefineSymbol) ==
						ToolchainKit::String::npos)
				{
					ToolchainKit::String undefined_symbol = command_hdr.Name;
					auto		result_of_sym =
						undefined_symbol.substr(undefined_symbol.find(symbol_imp));

					for (int i = 0; result_of_sym[i] != 0; ++i)
					{
						if (result_of_sym[i] != symbol_imp[i])
							goto ld_continue_search;
					}

					not_found.erase(it);

					if (kVerbose)
						kStdOut << "link: found symbol: " << command_hdr.Name << "\n";

					break;
				}
			}

		ld_continue_search:
			continue;
		}
	}

	// step 3: check for errors (recheck if we have those symbols.)

	if (!kStartFound && is_executable)
	{
		if (kVerbose)
			kStdOut
				<< "link: undefined entrypoint: " << kPefStart << ", you may have forget to link "
																  "against your compiler's runtime library.\n";

		kStdOut << "link: undefined entrypoint " << kPefStart
				<< " for executable: " << kOutput << "\n";
	}

	// step 4: write all PEF commands.

	ToolchainKit::PEFCommandHeader dateHeader{};

	time_t timestamp = time(nullptr);

	ToolchainKit::String timeStampStr = "Container:BuildEpoch:";
	timeStampStr += std::to_string(timestamp);

	strncpy(dateHeader.Name, timeStampStr.c_str(), timeStampStr.size());

	dateHeader.Flags  = 0;
	dateHeader.Kind	  = ToolchainKit::kPefZero;
	dateHeader.Offset = output_fc.tellp();
	dateHeader.Size	  = timeStampStr.size();

	command_headers.push_back(dateHeader);

	ToolchainKit::PEFCommandHeader abiHeader{};

	ToolchainKit::String abi = kLinkerAbiContainer;

	switch (kArch)
	{
	case ToolchainKit::kPefArchAMD64: {
		abi += "MSFT";
		break;
	}
	case ToolchainKit::kPefArchPowerPC: {
		abi += "SYSV";
		break;
	}
	case ToolchainKit::kPefArch32000:
	case ToolchainKit::kPefArch64000: {
		abi += "MHRA";
		break;
	}
	default: {
		abi += " IDK";
		break;
	}
	}

	memcpy(abiHeader.Name, abi.c_str(), abi.size());

	abiHeader.Size	 = abi.size();
	abiHeader.Offset = output_fc.tellp();
	abiHeader.Flags	 = 0;
	abiHeader.Kind	 = ToolchainKit::kPefLinkerID;

	command_headers.push_back(abiHeader);

	ToolchainKit::PEFCommandHeader stackHeader{0};

	stackHeader.Cpu	   = kArch;
	stackHeader.Flags  = 0;
	stackHeader.Size   = sizeof(uintptr_t);
	stackHeader.Offset = (kMIBCount * 1024 * 1024);
	memcpy(stackHeader.Name, kLinkerStackSizeSymbol, strlen(kLinkerStackSizeSymbol));

	command_headers.push_back(stackHeader);

	ToolchainKit::PEFCommandHeader uuidHeader{};

	std::random_device rd;

	auto seedData = std::array<int, std::mt19937::state_size>{};
	std::generate(std::begin(seedData), std::end(seedData), std::ref(rd));
	std::seed_seq seq(std::begin(seedData), std::end(seedData));
	std::mt19937  generator(seq);

	auto		gen		= uuids::uuid_random_generator{generator};
	uuids::uuid id		= gen();
	auto		uuidStr = uuids::to_string(id);

	memcpy(uuidHeader.Name, "Container:GUID:4:", strlen("Container:GUID:4:"));
	memcpy(uuidHeader.Name + strlen("Container:GUID:4:"), uuidStr.c_str(),
		   uuidStr.size());

	uuidHeader.Size	  = strlen(uuidHeader.Name);
	uuidHeader.Offset = output_fc.tellp();
	uuidHeader.Flags  = ToolchainKit::kPefLinkerID;
	uuidHeader.Kind	  = ToolchainKit::kPefZero;

	command_headers.push_back(uuidHeader);

	// prepare a symbol vector.
	std::vector<ToolchainKit::String> undef_symbols;
	std::vector<ToolchainKit::String> dupl_symbols;
	std::vector<ToolchainKit::String> resolve_symbols;

	constexpr Int32 cPaddingOffset = 16;

	size_t previous_offset = (command_headers.size() * sizeof(ToolchainKit::PEFCommandHeader)) + cPaddingOffset;

	// Finally write down the command headers.
	// And check for any duplications
	for (size_t commandHeaderIndex = 0UL;
		 commandHeaderIndex < command_headers.size(); ++commandHeaderIndex)
	{
		if (ToolchainKit::String(command_headers[commandHeaderIndex].Name)
					.find(kLdDefineSymbol) != ToolchainKit::String::npos &&
			ToolchainKit::String(command_headers[commandHeaderIndex].Name)
					.find(kLdDynamicSym) == ToolchainKit::String::npos)
		{
			// ignore :UndefinedSymbol: headers, they do not contain code.
			continue;
		}

		ToolchainKit::String symbol_name = command_headers[commandHeaderIndex].Name;

		if (!symbol_name.empty())
		{
			undef_symbols.emplace_back(symbol_name);
		}

		command_headers[commandHeaderIndex].Offset += previous_offset;
		previous_offset += command_headers[commandHeaderIndex].Size;

		ToolchainKit::String name = command_headers[commandHeaderIndex].Name;

		/// so this is valid when we get to the entrypoint.
		/// it is always a code64 container. And should equal to kPefStart as well.
		/// this chunk of code updates the pef_container.Start with the updated offset.
		if (name.find(kPefStart) != ToolchainKit::String::npos &&
			name.find(kPefCode64) != ToolchainKit::String::npos)
		{
			pef_container.Start = command_headers[commandHeaderIndex].Offset;
			auto tellCurPos		= output_fc.tellp();

			output_fc.seekp(0);
			output_fc << pef_container;

			output_fc.seekp(tellCurPos);
		}

		if (kVerbose)
		{
			kStdOut << "link: command header name: " << name << "\n";
			kStdOut << "link: real address of command header content: " << command_headers[commandHeaderIndex].Offset << "\n";
		}

		output_fc << command_headers[commandHeaderIndex];

		for (size_t sub_command_header_index = 0UL;
			 sub_command_header_index < command_headers.size();
			 ++sub_command_header_index)
		{
			if (sub_command_header_index == commandHeaderIndex)
				continue;

			if (ToolchainKit::String(command_headers[sub_command_header_index].Name)
						.find(kLdDefineSymbol) != ToolchainKit::String::npos &&
				ToolchainKit::String(command_headers[sub_command_header_index].Name)
						.find(kLdDynamicSym) == ToolchainKit::String::npos)
			{
				if (kVerbose)
				{
					kStdOut << "link: ignore :UndefinedSymbol: command header...\n";
				}

				// ignore :UndefinedSymbol: headers, they do not contain code.
				continue;
			}

			auto& command_hdr = command_headers[sub_command_header_index];

			if (command_hdr.Name ==
				ToolchainKit::String(command_headers[commandHeaderIndex].Name))
			{
				if (std::find(dupl_symbols.cbegin(), dupl_symbols.cend(),
							  command_hdr.Name) == dupl_symbols.cend())
				{
					dupl_symbols.emplace_back(command_hdr.Name);
				}

				if (kVerbose)
					kStdOut << "link: found duplicate symbol: " << command_hdr.Name
							<< "\n";

				kDuplicateSymbols = true;
			}
		}
	}

	if (!dupl_symbols.empty())
	{
		for (auto& symbol : dupl_symbols)
		{
			kStdOut << "link: multiple symbols of " << symbol << ".\n";
		}

		std::remove(kOutput.c_str());
		return TOOLCHAINKIT_EXEC_ERROR;
	}

	// step 2.5: write program bytes.

	for (auto byte : kObjectBytes)
	{
		output_fc << byte;
	}

	if (kVerbose)
		kStdOut << "link: wrote contents of: " << kOutput << "\n";

	// step 3: check if we have those symbols

	std::vector<ToolchainKit::String> unreferenced_symbols;

	for (auto& command_hdr : command_headers)
	{
		if (auto it = std::find(not_found.begin(), not_found.end(),
								ToolchainKit::String(command_hdr.Name));
			it != not_found.end())
		{
			unreferenced_symbols.emplace_back(command_hdr.Name);
		}
	}

	if (!unreferenced_symbols.empty())
	{
		for (auto& unreferenced_symbol : unreferenced_symbols)
		{
			kStdOut << "link: undefined symbol " << unreferenced_symbol << "\n";
		}
	}

	if (!kStartFound || kDuplicateSymbols && std::filesystem::exists(kOutput) ||
		!unreferenced_symbols.empty())
	{
		if (kVerbose)
			kStdOut << "link: file: " << kOutput
					<< ", is corrupt, removing file...\n";

		std::remove(kOutput.c_str());
		return TOOLCHAINKIT_EXEC_ERROR;
	}

	return 0;
}

// Last rev 13-1-24
