/*
 *	========================================================
 *
 *	C++Kit
 * 	Copyright WestCo, all rights reserved.
 *
 * 	========================================================
 */

// @file ld.cxx
// @brief AE to PEF linker.
// Use this to compile to PEF compliant OS.

// README: Do not look up for anything with .text/.data/.page_zero!
// It will be loaded when program will start up!
// Unlike $$dynamic$$ these containers will be loaded before CUS will do its job.

#include <C++Kit/StdKit/ErrorID.hpp>

#include <fstream>
#include <iostream>
#include <uuid/uuid.h>

//! Portable Executable Format
#include <C++Kit/StdKit/PEF.hpp>

//! Advanced Executable Object Format
#include <C++Kit/StdKit/AE.hpp>

//! @brief standard PEF entry.
#define kPefStart "__start"

#define kToolVersion "MP-UX linker v1.14, (c) WestCo"

#define StringCompare(dst, src) strcmp(dst, src)

#define kPefNoCpu       0U
#define kPefNoSubCpu    0U

#define kWhite           "\e[0;97m"
#define kStdOut          (std::cout << kWhite)

#define kPefDefaultStart 0x8000000
#define kPefLinkerNumId  0x333D
#define kPefAbiId        "Container:Abi:MP-UX"

enum { kAbiMpUx = 0xDEAD1 };

std::ofstream& operator<<(std::ofstream& fp, CxxKit::PEFContainer& container)
{
    fp.write((char*)&container, sizeof(CxxKit::PEFContainer));
    return fp;
}

std::ofstream& operator<<(std::ofstream& fp, CxxKit::PEFCommandHeader& container)
{
    fp.write((char*)&container, sizeof(CxxKit::PEFCommandHeader));
    return fp;
}

static std::string kOutput = "a.out";

static Int32 kAbi = kAbiMpUx;
static Int32 kSubArch = kPefNoSubCpu;
static Int32 kArch = kPefNoCpu;
static Bool  kFatBinaryEnable = false;
static Bool kStartFound = false;
static Bool kDuplicateSymbols = false;

static const char* kLdDefineSymbol = ":ld:";
static const char* kLdDynamicSym = ":mld_reloc:";

static std::vector<char> kObjectBytes;
static std::vector<std::string> kObjectList;

int main(int argc, char** argv)
{
	bool is_executable = true;
	
    for (size_t i = 1; i < argc; ++i)
    {
        if (StringCompare(argv[i], "-h") == 0)
        {
            kStdOut << kToolVersion << "\n";
            kStdOut << "-v: Print program version.\n";
            kStdOut << "-shared: Output as a shared library.\n";
            kStdOut << "-m64000: Link for the X64000 instruction set.\n";
            kStdOut << "-m68000: Link for the NXP 68000 instruction set.\n";
            kStdOut << "-mppc64: Link for the PowerPC instruction set.\n";
            kStdOut << "--fat-binary: Output as FAT PEF.\n";
            kStdOut << "-o: Select output filename.\n";	

            // bye
            return 0;
        }
        else if (StringCompare(argv[i], "-v") == 0 ||
            StringCompare(argv[i], "--version") == 0)
        {
            kStdOut << kToolVersion << std::endl;

            // bye :D
            return 0;
        }
        //
        //  we continue our way if these conditions are met.
        //  they are not files and are just flags.
        //  don't forget the 'continue' after your code.
        //
        else if (StringCompare(argv[i], "-m64000") == 0)
        {
            kArch = CxxKit::kPefArchARC;
        
            continue;
        }
        else if (StringCompare(argv[i], "--fat-binary") == 0)
        {
            kFatBinaryEnable = true;
        
            continue;
        }
        else if (StringCompare(argv[i], "-shared") == 0)
        {
            if (kOutput.find(".out") != std::string::npos)
                kOutput.erase(kOutput.find(".out"), strlen(".out"));

            kOutput += ".lib";

            is_executable = false;

            continue;
        }
        else if (StringCompare(argv[i], "-o") == 0)
        {
            kOutput = argv[i+1];
            ++i;

            continue;
        }
        else
        {
            kObjectList.emplace_back(argv[i]);

            continue;
        }

        kStdOut << "ld: ignore: " << argv[i] << "\n";
    }
    
    // sanity check.
    if (kObjectList.empty())
    {
        kStdOut << "ld: no input files." << std::endl;
        return CXXKIT_EXEC_ERROR;
    }

    if (kArch == 0)
    {
        kStdOut << "ld: no architecture set, can't continue." << std::endl;
        return CXXKIT_EXEC_ERROR;
    }

    CxxKit::PEFContainer pef_container{};

    pef_container.Count = 0UL;
    pef_container.Kind = CxxKit::kPefKindExec;
    pef_container.SubCpu = kSubArch;
    pef_container.Cpu = kArch;
    pef_container.Linker = kPefLinkerNumId; // WestCo Linker
    pef_container.Abi = kAbi; // Multi-Processor UNIX ABI
    pef_container.Magic[0] = kPefMagic[kFatBinaryEnable ? 2 : 0];
    pef_container.Magic[1] = kPefMagic[1];
    pef_container.Magic[2] = kPefMagic[kFatBinaryEnable ? 0 : 2];
    pef_container.Version = kPefVersion;

    // specify the start address.
    pef_container.Start = kPefDefaultStart;
    pef_container.HdrSz = sizeof(CxxKit::PEFContainer);

    std::ofstream output_fc(kOutput, std::ofstream::binary);
    output_fc << pef_container;

    //! Read AE to convert as PEF.

    std::vector<CxxKit::PEFCommandHeader> pef_command_hdrs;

    for (const auto& i : kObjectList)
    {
        if (!std::filesystem::exists(i))
            continue;

        CxxKit::AEHeader hdr{};
        
        std::ifstream input_object(i, std::ifstream::binary);

        input_object.read((char*)&hdr, sizeof(CxxKit::AEHeader));
    
        auto ae_header = hdr;
        
        if (ae_header.fArch != kArch)
        {
            if (!kFatBinaryEnable)
            {
                kStdOut << "ld: error: object " << i << " is a different kind of architecture and output isn't treated as FAT binary." << std::endl;

                std::remove(kOutput.c_str());
                return -CXXKIT_FAT_ERROR;
            }
        }

        if (ae_header.fMagic[0] == kAEMag0 &&
            ae_header.fMagic[1] == kAEMag1 &&
	        ae_header.fSize == sizeof(CxxKit::AEHeader))
        {
            std::size_t cnt = ae_header.fCount;

            pef_container.Count = cnt;

            char* raw_ae_records = new char[cnt * sizeof(CxxKit::AERecordHeader)];
            memset(raw_ae_records, 0, cnt * sizeof(CxxKit::AERecordHeader));

            input_object.read(raw_ae_records, std::streamsize(cnt * sizeof(CxxKit::AERecordHeader)));

            auto* ae_records = (CxxKit::AERecordHeader*)raw_ae_records;

            for (size_t ae_record_index = 0; ae_record_index < cnt; ++ae_record_index)
            {
				CxxKit::PEFCommandHeader command_header{ 0 };

				memcpy(command_header.Name, ae_records[ae_record_index].fName, kPefNameLen);

                // check this header if it's any valid.
                if (std::string(command_header.Name).find(".text") == std::string::npos &&
                    std::string(command_header.Name).find(".data") == std::string::npos &&
                    std::string(command_header.Name).find(".page_zero") == std::string::npos)
                {
                    if (std::string(command_header.Name).find(kPefStart) == std::string::npos &&
                        *command_header.Name == 0)
                    {
                        if (std::string(command_header.Name).find(kLdDefineSymbol) != std::string::npos)
                        {
                            goto ld_mark_header;
                        }
                        else
                        {
                            continue;
                        }
                    }
                }

                if (std::string(command_header.Name).find(kPefStart) != std::string::npos &&
                    std::string(command_header.Name).find(".text") != std::string::npos)
                {
                    kStartFound = true;
                    pef_container.Start = ae_records[ae_record_index].fOffset;
                }

ld_mark_header:
				command_header.Offset = ae_records[ae_record_index].fOffset;
				command_header.Kind = ae_records[ae_record_index].fKind;
				command_header.Size = ae_records[ae_record_index].fSize;

                pef_command_hdrs.emplace_back(command_header);
            }

            delete[] raw_ae_records;

            std::vector<char> bytes;
            bytes.resize(ae_header.fCodeSize);

            input_object.seekg(ae_header.fStartCode);
            input_object.read(bytes.data(), ae_header.fCodeSize);

            for (auto& byte : bytes)
            {
                kObjectBytes.push_back(byte);
            }

            continue;  
        }

        kStdOut << "ld: not an object " << i << std::endl;
        std::remove(kOutput.c_str());

        // don't continue, it is a fatal error.
        return -CXXKIT_EXEC_ERROR;
    }

    output_fc.seekp(std::streamsize(pef_container.HdrSz));

    std::vector<std::string> not_found;
    std::vector<std::string> symbols;

    // step 2: check for errors (multiple symbols, undefined ones)

    for (auto & pef_command_hdr : pef_command_hdrs)
    {
        // check if this symbol needs to be resolved.
        if (std::string(pef_command_hdr.Name).find(kLdDefineSymbol) !=
            std::string::npos &&
            std::string(pef_command_hdr.Name).find(kLdDynamicSym) ==
            std::string::npos)
        {
            if (auto it = std::find(not_found.begin(), not_found.end(), std::string(pef_command_hdr.Name));
                    it == not_found.end())
            {
                not_found.emplace_back(pef_command_hdr.Name);
            }
        }

        symbols.emplace_back(pef_command_hdr.Name);
    }

    // Now try to solve these symbols.

    for (size_t not_found_idx = 0; not_found_idx < pef_command_hdrs.size(); ++not_found_idx)
    {
        if (auto it = std::find(not_found.begin(), not_found.end(), std::string(pef_command_hdrs[not_found_idx].Name));
                it != not_found.end())
        {
            std::string symbol_imp = *it;

            if (symbol_imp.find(kLdDefineSymbol) == std::string::npos)
                continue;

            // erase the lookup prefix.
            symbol_imp.erase(0, symbol_imp.find(kLdDefineSymbol) + strlen(kLdDefineSymbol));

            // demangle everything.
            while (symbol_imp.find('$') != std::string::npos)
                symbol_imp.erase(symbol_imp.find('$'), 1);

            // the reason we do is because, this may not match the symbol, and we need
            // to look for other matching symbols.
            for (auto& pef_command_hdr : pef_command_hdrs)
            {
                if (std::string(pef_command_hdr.Name).find(symbol_imp) != std::string::npos &&
                    std::string(pef_command_hdr.Name).find(kLdDefineSymbol) == std::string::npos)
                {
                    std::string undefined_symbol = pef_command_hdr.Name;
                    auto result_of_sym = undefined_symbol.substr(undefined_symbol.find(symbol_imp));

                    for (int i = 0; result_of_sym[i] != 0; ++i)
                    {
                        if (result_of_sym[i] != symbol_imp[i])
                            goto ld_continue_search;

                    }

                    not_found.erase(it);

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
        kStdOut << "ld: undefined entrypoint " << kPefStart << " for executable " << kOutput << "\n";
    }

    // step 4: write some pef commands.

    CxxKit::PEFCommandHeader date_header{};

    time_t timestamp = time(nullptr);

    std::string timestamp_str = "ContainerDate:";
    timestamp_str += std::to_string(timestamp);

    strcpy(date_header.Name, timestamp_str.c_str());

    date_header.Flags = 0;
    date_header.Kind = CxxKit::kPefData;
    date_header.Offset = output_fc.tellp();
    date_header.Size = timestamp_str.size();

    output_fc << date_header;

    CxxKit::PEFCommandHeader abi_header{};

    memcpy(abi_header.Name, kPefAbiId, strlen(kPefAbiId));

    abi_header.Size = strlen(kPefAbiId);
    abi_header.Offset = output_fc.tellp();
    abi_header.Flags = 0;
    abi_header.Kind = CxxKit::kPefLinkerID;

    output_fc << abi_header;

    CxxKit::PEFCommandHeader uuid_header{};

    uuid_t uuid{ 0 };
    uuid_generate_random(uuid);

    memcpy(uuid_header.Name, "UUID_TYPE:4:", strlen("UUID_TYPE:4:"));
    memcpy(uuid_header.Name + strlen("UUID_TYPE:4:"), uuid,  16);

    uuid_header.Size = 16;
    uuid_header.Offset = output_fc.tellp();
    uuid_header.Flags = 0;
    uuid_header.Kind = 0;

    output_fc << uuid_header;

    // prepare a symbol vector.
    std::vector<std::string> undefined_symbols;
    std::vector<std::string> duplicate_symbols;

    // Finally write down the command headers.
    // And check for any duplications
    for (size_t cmd_hdr = 0UL; cmd_hdr < pef_command_hdrs.size(); ++cmd_hdr)
    {
        if (std::string(pef_command_hdrs[cmd_hdr].Name).find(kLdDefineSymbol) !=
            std::string::npos &&
            std::string(pef_command_hdrs[cmd_hdr].Name).find(kLdDynamicSym) ==
            std::string::npos)
        {
            // ignore :ld: headers, they do not contain code.
            continue;
        }

        std::string sym_name = pef_command_hdrs[cmd_hdr].Name;

        if (!sym_name.empty())
        {
            undefined_symbols.emplace_back(sym_name);
        }

        output_fc << pef_command_hdrs[cmd_hdr];

        for (size_t cmd_hdr_sub = 0UL; cmd_hdr_sub < pef_command_hdrs.size(); ++cmd_hdr_sub)
        {
            if (cmd_hdr_sub == cmd_hdr)
                continue;

            if (std::string(pef_command_hdrs[cmd_hdr_sub].Name).find(kLdDefineSymbol) !=
                std::string::npos &&
                std::string(pef_command_hdrs[cmd_hdr_sub].Name).find(kLdDynamicSym) ==
                std::string::npos)
            {
                // ignore :ld: headers, they do not contain code.
                continue;
            }

            auto& pef_command_hdr = pef_command_hdrs[cmd_hdr_sub];

            if (pef_command_hdr.Name == std::string(pef_command_hdrs[cmd_hdr].Name))
            {
                if (std::find(duplicate_symbols.cbegin(), duplicate_symbols.cend(), pef_command_hdr.Name) == duplicate_symbols.cend())
                {
                    duplicate_symbols.push_back(pef_command_hdr.Name);
                }

                kDuplicateSymbols = true;
            }
        }
    }

    if (!duplicate_symbols.empty())
    {
        for (auto& symbol : duplicate_symbols)
        {
            kStdOut << "ld: multiple symbols of " << symbol << ".\n";
        }

        std::remove(kOutput.c_str());
        return -CXXKIT_EXEC_ERROR;
    }
    
    // step 2.5: write program bytes.

    for (auto byte : kObjectBytes)
    {
        output_fc << byte;
    }

    // step 3: check if we have those symbols

    std::vector<std::string> unreferenced_symbols;

    for (auto & pef_command_hdr : pef_command_hdrs)
    {
        if (auto it = std::find(not_found.begin(), not_found.end(), std::string(pef_command_hdr.Name));
                it != not_found.end())
        {
            unreferenced_symbols.emplace_back(pef_command_hdr.Name);
        }
    }

    if (!unreferenced_symbols.empty())
    {
        for (auto& unreferenced_symbol : unreferenced_symbols)
        {
            kStdOut << "ld: undefined symbol " << unreferenced_symbol << "\n";
        }
    }

    if (!kStartFound ||
        kDuplicateSymbols &&
        std::filesystem::exists(kOutput) ||
        !unreferenced_symbols.empty())
    {
        std::remove(kOutput.c_str());
        return -CXXKIT_EXEC_ERROR;
    }

    return 0;
}

// Last rev 28-12-23