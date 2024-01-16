/*
 *	========================================================
 *
 *	MP-UX C Compiler
 * 	Copyright 2024, Mahrouss Logic, all rights reserved.
 *
 * 	========================================================
 */

// @file main.d
// @brief CLI of the X64000 C/C++ compiler

module mpcc.main;

import mpcc.compiler;
import std.container.dlist;

///Authors: Amlal EL Mahrouss
///Examples: mpcc_summon_manual("foo");
void mpcc_summon_manual(string path)
{
	import core.stdc.stdlib;
    import std.string;
    import std.file;

    string base = "man ";
    base ~= "/usr/local/bin/man/";

    string extension = ".8";

	core.stdc.stdlib.system(toStringz(base ~ path ~ extension));
}

void main(string[] args) 
{
	import std.range, std.stdio;

    bool shared_library = false;
    bool compile_only = false;
    bool kernel_driver = false;
    string output_file = "a.out";
    size_t index = 0UL;

    string[255] args_list;

    foreach (arg ; args)
    {
        if (arg[0] == '-')
        {
            if (arg == "--version" ||
                arg == "-v")
            {
                writeln("mpcc: version 1.01, (c) Mahrouss Logic all rights reserved.");
                return;
            }
            else if (arg == "--dialect")
            {
                mpcc_summon_executable("/usr/local/bin/bin/cc --asm=masm --compiler=dalvik --dialect");
                return;
            }
            else if (arg == "--help" ||
                     arg == "-h")
            {
                writeln("mpcc: summoning manual entry for mpcc...");
                mpcc_summon_manual("mpcc");

                return;
            }
            else if (arg == "-c")
            {
                compile_only = true;
                continue;
            }
            else if (arg == "-kernel")
            {
                kernel_driver = true;
                continue;
            }
            else if (arg == "-shared")
            {
                output_file = "a.lib";
                shared_library = true;
                continue;
            }

            writeln("mpcc: unknown argument " ~ arg);
            return;
        }

        if (index == 0)
        {
            ++index;
            continue;
        }

        args_list[index] = arg;
        ++index;
    }

    if (args_list[1] == null)
    {
        writeln("mpcc: no input files.");
        return;
    }

    auto compiler = new CompileCommand();

    if (kernel_driver)
        compiler.compile(Platform.getKernelPath(), args_list, shared_library, output_file, compile_only);
    else
        compiler.compile(Platform.getIncludePath(), args_list, shared_library, output_file, compile_only);
}