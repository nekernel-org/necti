/*
 *	========================================================
 *
 *	C++Kit
 * 	Copyright Mahrouss Logic, all rights reserved.
 *
 * 	========================================================
 */

/////////////////////////////////////////////////////////////////////////////////////////

// @file masm.cxx
// @author Amlal El Mahrouss
// @brief MP-UX 64x0 Assembler.

// REMINDER: when dealing with an undefined symbol use (string size):ld:(string)
// so that ld will look for it.

/////////////////////////////////////////////////////////////////////////////////////////

#include <CompilerKit/AsmKit/Arch/64k.hpp>
#include <CompilerKit/ParserKit.hpp>
#include <CompilerKit/StdKit/PEF.hpp>
#include <CompilerKit/StdKit/AE.hpp>
#include <filesystem>
#include <iostream>
#include <fstream>

/////////////////////

// ANSI ESCAPE CODES

/////////////////////

#define kBlank  "\e[0;30m"
#define kRed    "\e[0;31m"
#define kWhite  "\e[0;97m"
#define kYellow "\e[0;33m"

#define kStdOut     (std::cout << kWhite)

static char         kOutputArch = CompilerKit::kPefArch64000;

//! base relocation address for every mp-ux app.
static UInt32       kErrorLimit = 10;
static UInt32       kAcceptableErrors = 0;

static std::size_t  kCounter = 1UL;

static bool         kVerbose = false;

static std::vector<char>     kBytes;
static CompilerKit::AERecordHeader kCurrentRecord{ .fName = "", .fKind = CompilerKit::kPefCode, .fSize = 0, .fOffset = 0 };

static std::vector<CompilerKit::AERecordHeader> kRecords;
static std::vector<std::string> kUndefinedSymbols;

static const std::string kUndefinedSymbol = ":ld:";
static const std::string kRelocSymbol = ":mld:";

// \brief forward decl.
static std::string masm_check_line(std::string& line, const std::string& file);
static bool masm_read_attributes(std::string& line);
static void masm_read_instruction(std::string& line, const std::string& file);

namespace detail
{
    void print_error(std::string reason, const std::string& file) noexcept
    {
        if (reason[0] == '\n')
            reason.erase(0, 1);

        kStdOut << kRed << "[ masm ] " << kWhite << ((file == "masm") ? "internal assembler error " : ("in file, " + file)) << kBlank << std::endl;
        kStdOut << kRed << "[ masm ] " << kWhite << reason << kBlank << std::endl;

        if (kAcceptableErrors > kErrorLimit)
            std::exit(3);

        ++kAcceptableErrors;
    }

    void print_warning(std::string reason, const std::string& file) noexcept
    {
        if (reason[0] == '\n')
            reason.erase(0, 1);

        if (!file.empty())
        {
            kStdOut << kYellow << "[ file ] " << kWhite << file << kBlank << std::endl;
        }

        kStdOut << kYellow << "[ masm ] " << kWhite << reason << kBlank << std::endl;
    }
}

// provide operator<< for AE

std::ofstream& operator<<(std::ofstream& fp, CompilerKit::AEHeader& container)
{
    fp.write((char*)&container, sizeof(CompilerKit::AEHeader));

    return fp;
}

std::ofstream& operator<<(std::ofstream& fp, CompilerKit::AERecordHeader& container)
{
    fp.write((char*)&container, sizeof(CompilerKit::AERecordHeader));

    return fp;
}

/////////////////////////////////////////////////////////////////////////////////////////

// @brief Main entrypoint.

/////////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char** argv)
{
    for (size_t i = 1; i < argc; ++i)
    {
        if (argv[i][0] == '-')
        {
            if (strcmp(argv[i], "-v") == 0)
            {
                kStdOut << "64asm: The MP-UX 64x0 Assembler.\n64asm: v1.10\n64asm: Copyright (c) 2023 Mahrouss Logic.\n";
                return 0;
            }

            if (strcmp(argv[i], "-h") == 0)
            {
                kStdOut << "64asm: The MP-UX 64x0 Assembler.\n64asm: Copyright (c) 2023 Mahrouss Logic.\n";
                kStdOut << "-v: Print program version.\n";
                kStdOut << "-verbose: Print verbose output.\n";
                kStdOut << "-m64000: Compile for the X64000 instruction set.\n";

                return 0;
            }
            else if (strcmp(argv[i], "-verbose") == 0)
            {
                kVerbose = true;
                continue;
            }

            kStdOut << "64asm: ignore " << argv[i] << "\n";
            continue;
        }

        if (!std::filesystem::exists(argv[i]))
            continue;

        std::string object_output(argv[i]);

        if (object_output.find(kAsmFileExt64x0) != std::string::npos)
        {
            object_output.erase(object_output.find(kAsmFileExt64x0), std::size(kAsmFileExt64x0));
        }

        object_output += kObjectFileExt;

        std::ifstream file_ptr(argv[i]);
        std::ofstream file_ptr_out(object_output,
                                   std::ofstream::binary);

        if (file_ptr_out.bad())
        {
            if (kVerbose)
            {
                kStdOut << "64asm: error: " << strerror(errno) << "\n";
            }
        }

        std::string line;

        CompilerKit::AEHeader hdr{ 0 };

        memset(hdr.fPad, kAEInvalidOpcode, kAEPad);

        hdr.fMagic[0] = kAEMag0;
        hdr.fMagic[1] = kAEMag1;
        hdr.fSize = sizeof(CompilerKit::AEHeader);
        hdr.fArch = kOutputArch;

        /////////////////////////////////////////////////////////////////////////////////////////

        // COMPILATION LOOP

        /////////////////////////////////////////////////////////////////////////////////////////

        while (std::getline(file_ptr, line))
        {
            if (auto ln = masm_check_line(line, argv[i]);
                !ln.empty())
            {
                detail::print_error(ln, argv[i]);
                continue;
            }

            try
            {
                masm_read_attributes(line);
                masm_read_instruction(line, argv[i]);
            }
            catch(const std::exception& e)
            {
                if (kVerbose)
                {
                    std::string what = e.what();
                    detail::print_warning("exit because of: " + what, "masm");
                }

                std::filesystem::remove(object_output);
                goto masm_fail_exit;
            }
            
        }

        if (kVerbose)
            kStdOut << "64asm: writing to file...\n";

        // this is the final step, write everything to the file.

        auto pos = file_ptr_out.tellp();

        hdr.fCount = kRecords.size() + kUndefinedSymbols.size();

        file_ptr_out << hdr;

        if (kRecords.empty())
        {
            std::filesystem::remove(object_output);
            return -1;
        }

        kRecords[kRecords.size() - 1].fSize = kBytes.size();

        std::size_t record_count = 0UL;

        for (auto& rec : kRecords)
        {
            if (kVerbose)
                kStdOut << "64asm: wrote record " << rec.fName << " to file...\n";

            rec.fFlags |= CompilerKit::kKindRelocationAtRuntime;
            rec.fOffset = record_count;
            ++record_count;

            file_ptr_out << rec;
        }

        // increment once again, so that we won't lie about the kUndefinedSymbols.
        ++record_count;

        for (auto& sym : kUndefinedSymbols)
        {
            CompilerKit::AERecordHeader _record_hdr{ 0 };

            if (kVerbose)
                kStdOut << "64asm: wrote symbol " << sym << " to file...\n";

            _record_hdr.fKind = kAEInvalidOpcode;
            _record_hdr.fSize = sym.size();
            _record_hdr.fOffset = record_count;

            ++record_count;

            memset(_record_hdr.fPad, kAEInvalidOpcode, kAEPad);
            memcpy(_record_hdr.fName, sym.c_str(), sym.size());

            file_ptr_out << _record_hdr;

            ++kCounter;
        }

        auto pos_end = file_ptr_out.tellp();

        file_ptr_out.seekp(pos);

        hdr.fStartCode = pos_end;
        hdr.fCodeSize = kBytes.size();

        file_ptr_out << hdr;

        file_ptr_out.seekp(pos_end);

        // byte from byte, we write this.
        for (auto& byte : kBytes)
        {
            file_ptr_out.write(reinterpret_cast<const char *>(&byte), sizeof(byte));
        }

        if (kVerbose)
            kStdOut << "64asm: wrote program bytes to file...\n";

        file_ptr_out.flush();
        file_ptr_out.close();
    
        if (kVerbose)
            kStdOut << "64asm: exit succeeded with code 0.\n";

        return 0;
    }

masm_fail_exit:

    if (kVerbose)
        kStdOut << "64asm: exit failed with code -1.\n";

    return -1;
}

/////////////////////////////////////////////////////////////////////////////////////////

// @brief Check for attributes
// returns true if any was found.

/////////////////////////////////////////////////////////////////////////////////////////

static bool masm_read_attributes(std::string& line)
{
    // import is the opposite of export, it signals to the ld
    // that we need this symbol.
    if (ParserKit::find_word(line, "import "))
    {
        auto name = line.substr(line.find("import ") + strlen("import "));

        std::string result = std::to_string(name.size());
        result += kUndefinedSymbol;

        // mangle this
        for (char & j : name)
        {
            if (j == ' ' ||
                j == ',')
                j = '$';

        }

        result += name;

        if (name.find(".text") != std::string::npos)
        {
            // data is treated as code.
            kCurrentRecord.fKind = CompilerKit::kPefCode;
        }
        else if (name.find(".data") != std::string::npos)
        {
            // no code will be executed from here.
            kCurrentRecord.fKind = CompilerKit::kPefData;
        }
        else if (name.find(".page_zero") != std::string::npos)
        {
            // this is a bss section.
            kCurrentRecord.fKind = CompilerKit::kPefZero;
        }

        // this is a special case for the start stub.
        // we want this so that ld can find it.

        if (name == "__start")
        {
            kCurrentRecord.fKind = CompilerKit::kPefCode;
        }

        // now we can tell the code size of the previous kCurrentRecord.

        if (!kRecords.empty())
            kRecords[kRecords.size() - 1].fSize = kBytes.size();

        memset(kCurrentRecord.fName, 0, kAESymbolLen);
        memcpy(kCurrentRecord.fName, result.c_str(), result.size());

        ++kCounter;

        memset(kCurrentRecord.fPad, kAEInvalidOpcode, kAEPad);

        kRecords.emplace_back(kCurrentRecord);

        return true;
    }

    // export is a special keyword used by masm to tell the AE output stage to mark this section as a header.
    // it currently supports .text, .data., page_zero
    if (ParserKit::find_word(line, "export "))
    {
        auto name = line.substr(line.find("export ") + strlen("export "));

        for (char& j : name)
        {
            if (j == ' ')
                j = '$';

        }

        if (name.find(',') != std::string::npos)
            name.erase(name.find(','));

        if (name.find(".text") != std::string::npos)
        {
            // data is treated as code.
            kCurrentRecord.fKind = CompilerKit::kPefCode;
        }
        else if (name.find(".data") != std::string::npos)
        {
            // no code will be executed from here.
            kCurrentRecord.fKind = CompilerKit::kPefData;
        }
        else if (name.find(".page_zero") != std::string::npos)
        {
            // this is a bss section.
            kCurrentRecord.fKind = CompilerKit::kPefZero;
        }

        // this is a special case for the start stub.
        // we want this so that ld can find it.

        if (name == "__start")
        {
            kCurrentRecord.fKind = CompilerKit::kPefCode;
        }

        // now we can tell the code size of the previous kCurrentRecord.

        if (!kRecords.empty())
            kRecords[kRecords.size() - 1].fSize = kBytes.size();

        memset(kCurrentRecord.fName, 0, kAESymbolLen);
        memcpy(kCurrentRecord.fName, name.c_str(), name.size());

        ++kCounter;

        memset(kCurrentRecord.fPad, kAEInvalidOpcode, kAEPad);

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
        return !(isalpha(c) || isdigit(c) || (c == ' ') || (c == '\t') || (c == ',') ||
                (c == '(') || (c == ')') || (c == '"') || (c == '\'') || (c == '[') || (c == ']')
                || (c == '+') || (c == '_'));
    }

    bool is_valid(const std::string &str)
    {
        if (ParserKit::find_word(str, "export ") ||
                ParserKit::find_word(str, "import "))
            return true;

        return find_if(str.begin(), str.end(), is_not_alnum_space) == str.end();
    }
}

/////////////////////////////////////////////////////////////////////////////////////////

// @brief Check for line (syntax check)

/////////////////////////////////////////////////////////////////////////////////////////

static std::string masm_check_line(std::string& line, const std::string& file)
{
    (void)file;

    std::string err_str;

    while (line.find('\t') != std::string::npos)
        line.erase(line.find('\t'), 1);

    if (line.empty() ||
        ParserKit::find_word(line, "import ") ||
        ParserKit::find_word(line, "export ") ||
        ParserKit::find_word(line, "#") ||
        ParserKit::find_word(line, ";") ||
        ParserKit::find_word(line, "layout"))
    {
        if (line.find('#') != std::string::npos)
        {
            line.erase(line.find('#'));
        }

        if (line.find(';') != std::string::npos)
        {
            line.erase(line.find(';'));
        }

        return err_str;
    }

    if (!detail::algorithm::is_valid(line))
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
            err_str += "\ninstruction lacks right register, here -> ";
            err_str += line.substr(line.find(','));

            return err_str;
        }
        else
        {
            bool nothing_on_right = true;

            if (line.find(',') + 1 > line.size())
            {
                err_str += "\ninstruction not complete, here -> ";
                err_str += line;

                return err_str;
            }

            auto substr = line.substr(line.find(',') + 1);

            for (auto& ch : substr)
            {
                if (ch != ' ' &&
                    ch != '\t')
                {
                    nothing_on_right = false;
                }
            }

            // this means we found nothing after that ',' .
            if (nothing_on_right)
            {
                err_str += "\ninstruction not complete, here -> ";
                err_str += line;

                return err_str;
            }
        }
    }

    // these do take an argument.
    std::vector<std::string> operands_inst = { "jb", "psh", "stw", "ldw", "lda", "sta" };

    // these don't.
    std::vector<std::string> filter_inst = { "jlr", "jrl", "syscall" };

    for (auto& opcode64x0 : kOpcodes64x0)
    {
        if (line.find(opcode64x0.fName) != std::string::npos)
        {
            for (auto& op : operands_inst)
            {
                // if only instruction found.
                if (line == op)
                {
                    err_str += "\nmalformed ";
                    err_str += op;
                    err_str += " instruction, here -> ";
                    err_str += line;
                }
            }

            // if it is like that -> addr1, 0x0
            if (auto it = std::find(filter_inst.begin(), filter_inst.end(), opcode64x0.fName);
                it == filter_inst.cend())
            {
                if (!isspace(line[line.find(opcode64x0.fName) + strlen(opcode64x0.fName)]))
                {
                    err_str += "\nmissing space between ";
                    err_str += opcode64x0.fName;
                    err_str += " and operands.\nhere -> ";
                    err_str += line;
                }
            }

            return err_str;
        }
    }

    err_str += "unknown syntax: ";
    err_str += line;

    return err_str;
}

/////////////////////////////////////////////////////////////////////////////////////////

// @brief internal namespace

/////////////////////////////////////////////////////////////////////////////////////////

namespace detail
{
    union number_cast
    {
        explicit number_cast(UInt64 raw)
                : raw(raw)
        {}

        char number[8];
        UInt64 raw;
    };
}

static bool masm_write_number(const std::size_t& pos, std::string& jump_label)
{
    if (!isdigit(jump_label[pos]))
        return false;

    switch (jump_label[pos+1])
    {
        case 'x':
        {
            if (auto res = strtoq(jump_label.substr(pos + 2).c_str(),
                                  nullptr, 16);
                    !res)
            {
                if (errno != 0)
                {
                    detail::print_error("invalid hex number: " + jump_label, "masm");
                    throw std::runtime_error("invalid_hex");
                }
            }

            detail::number_cast num(strtoq(jump_label.substr(pos + 2).c_str(),
                                   nullptr, 16));

            for (char& i : num.number)
            {
                kBytes.push_back(i);
            }

            if (kVerbose)
            {
                kStdOut << "64asm: found a base 16 number here: " << jump_label.substr(pos) << "\n";
            }

            return true;
        }
        case 'b':
        {
            if (auto res = strtoq(jump_label.substr(pos + 2).c_str(),
                                  nullptr, 2);
                !res)
            {
                if (errno != 0)
                {
                    detail::print_error("invalid binary number: " + jump_label, "masm");
                    throw std::runtime_error("invalid_bin");
                }
            }

            detail::number_cast num(strtoq(jump_label.substr(pos + 2).c_str(),
                                   nullptr, 2));

            if (kVerbose)
            {
                kStdOut << "64asm: found a base 2 number here: " << jump_label.substr(pos) << "\n";
            }

            for (char& i : num.number)
            {
                kBytes.push_back(i);
            }

            return true;
        }
        case 'o':
        {
            if (auto res = strtoq(jump_label.substr(pos + 2).c_str(),
                                  nullptr, 7);
                !res)
            {
                if (errno != 0)
                {
                    detail::print_error("invalid octal number: " + jump_label, "masm");
                    throw std::runtime_error("invalid_octal");
                }
            }

            detail::number_cast num(strtoq(jump_label.substr(pos + 2).c_str(),
                                   nullptr, 7));

            if (kVerbose)
            {
                kStdOut << "64asm: found a base 8 number here: " << jump_label.substr(pos) << "\n";
            }

            for (char& i : num.number)
            {
                kBytes.push_back(i);
            }

            return true;
        }
        default:
        {
            break;
        }
    }

    /* check for errno and stuff like that */
    if (auto res = strtoq(jump_label.substr(pos).c_str(),
                          nullptr, 10);
            !res)
    {
        if (errno != 0)
        {
            return false;
        }
    }

    detail::number_cast num(strtoq(jump_label.substr(pos).c_str(),
                                        nullptr, 10));

    for (char& i : num.number)
    {
        kBytes.push_back(i);
    }

    if (kVerbose)
    {
        kStdOut << "64asm: found a base 10 number here: " << jump_label.substr(pos) << "\n";
    }

    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////

// @brief Read and write an instruction to the output array.

/////////////////////////////////////////////////////////////////////////////////////////

static void masm_read_instruction(std::string& line, const std::string& file)
{
    if (ParserKit::find_word(line, "export "))
        return;

    for (auto& opcode64x0 : kOpcodes64x0)
    {
        // strict check here
        if (ParserKit::find_word(line, opcode64x0.fName) &&
            detail::algorithm::is_valid(line))
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
                case kAsmImmediate:
                {
                    // \brief how many registers we found.
                    std::size_t found_some = 0UL;

                    for (size_t line_index = 0UL; line_index < line.size(); line_index++)
                    {
                        if (line[line_index] == 'r' &&
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
                            if (kOutputArch == CompilerKit::kPefArch64000)
                            {
                                if (isdigit(line[line_index + 3]) &&
                                    isdigit(line[line_index + 2]))
                                {
                                    reg_str += line[line_index + 3];
                                    detail::print_error("invalid register index, r" + reg_str + "\nnote: 64x0 accepts registers from r0 to r20.", file);
                                    throw std::runtime_error("invalid_register_index");
                                }
                            }

                            // finally cast to a size_t
                            std::size_t reg_index = strtoq(
                                reg_str.c_str(),
                                nullptr,
                                10);

                            if (reg_index > kAsmRegisterLimit)
                            {
                                detail::print_error("invalid register index, r" + reg_str, file);
                                throw std::runtime_error("invalid_register_index");
                            }

                            kBytes.emplace_back(reg_index);
                            ++found_some;

                            if (kVerbose)
                            {
                                if (kOutputArch == CompilerKit::kPefArch64000)
                                    kStdOut << "64asm: 64x0 register found: " << register_syntax << "\n";
                                else
                                    kStdOut << "64asm: register found: " << register_syntax << "\n";

                                kStdOut << "64asm: Number of registers: " << found_some << "\n";
                            }
                        }
                    }
                    
                    // we're not in immediate addressing, reg to reg.
                    if (opcode64x0.fFunct7 != kAsmImmediate)
                    {
                        // remember! register to register!
                        if (found_some == 1)
                        {
                            detail::print_error("unrecognized register found.\ntip: each masm register starts with 'r'.\nline: " + line, file);
                            throw std::runtime_error("not_a_register");
                        }
                    }

                    if (found_some < 1 &&
                        name != "psh" &&
                        name != "ldw" &&
                        name != "lda" &&
                        name != "stw" &&
                        name != "jb")
                    {
                        detail::print_error("invalid combination of opcode and registers.\nline: " + line, file);
                        throw std::runtime_error("invalid_comb_op_reg");
                    }
                    else if (found_some == 1 &&
                            name == "add" )
                    {
                        detail::print_error("invalid combination of opcode and registers.\nline: " + line, file);
                        throw std::runtime_error("invalid_comb_op_reg");
                    }
                    else if (found_some == 1 &&
                            name == "dec")
                    {
                        detail::print_error("invalid combination of opcode and registers.\nline: " + line, file);
                        throw std::runtime_error("invalid_comb_op_reg");
                    }

                    if (found_some > 0 &&
                        name == "pop")
                    {
                        detail::print_error("invalid combination for opcode 'pop'.\ntip: it expects nothing.\nline: " + line, file);
                        throw std::runtime_error("invalid_comb_op_pop");
                    }
                }
                default:
                    break;

            }

            // try to fetch a number from the name
            if (name == "psh" ||
                name == "jb" ||
                name == "stw" ||
                name == "ldw" ||
                name == "lda" ||
                name == "sta")
            {
                auto where_string = name;

                // if we load something, we'd need it's symbol/literal
                if (name == "stw" ||
                    name == "ldw" ||
                    name == "lda" ||
                    name == "sta")
                    where_string = ",";

                jump_label = line.substr(line.find(where_string) + where_string.size());
                cpy_jump_label = jump_label;

                // replace any spaces with $
                if (jump_label[0] == ' ')
                {
                    while (jump_label.find(' ') != std::string::npos)
                    {
                        if (isalnum(jump_label[0]) ||
                            isdigit(jump_label[0]))
                            break;

                        jump_label.erase(jump_label.find(' '), 1);
                    }
                }

                if (!masm_write_number(0, jump_label))
                {
                    // sta expects this: sta 0x000000, r0
                    if (name == "sta")
                    {
                        detail::print_error("invalid combination of opcode and operands.\nhere ->" + line, file);
                        throw std::runtime_error("invalid_comb_op_ops");
                    }
                    
                    goto masm_write_label;
                }
                else
                {
                    if (name == "sta" &&
                        cpy_jump_label.find("import ") != std::string::npos)
                    {
                        detail::print_error("invalid usage import on 'sta', here: " + line, file);
                        throw std::runtime_error("invalid_sta_usage");
                    }
                }
            }

            // This is the case where we jump to a label, it is also used as a goto.
            if (name == "jb")
            {
masm_write_label:
                if (cpy_jump_label.find('\n') != std::string::npos)
                    cpy_jump_label.erase(cpy_jump_label.find('\n'), 1);

                if (cpy_jump_label.find("import ") == std::string::npos &&
                    name == "psh" ||
                    cpy_jump_label.find("import ") == std::string::npos &&
                    name == "jb")
                {
                    detail::print_error("import not found on jump label, please add one.", file);
                    throw std::runtime_error("import_jmp_lbl");
                }
                else if (cpy_jump_label.find("import ") != std::string::npos)
                { 
                    if (name == "sta")
                    {
                        detail::print_error("import is not allowed on a sta operation.", file);
                        throw std::runtime_error("import_sta_op");
                    }

                    cpy_jump_label.erase(cpy_jump_label.find("import "), strlen("import "));
                }

                while (cpy_jump_label.find(' ') != std::string::npos)
                {
                    cpy_jump_label.erase(cpy_jump_label.find(' '), 1);
                }

                if (cpy_jump_label.size() < 1)
                {
                    detail::print_error("label is empty, can't jump on it.", file);
                    throw std::runtime_error("label_empty");
                }

                auto mld_reloc_str = std::to_string(cpy_jump_label.size());
                mld_reloc_str += kRelocSymbol;
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
            }

            kBytes.push_back('\0');
            break;
        }
    }
}

// Last rev 8-1-24