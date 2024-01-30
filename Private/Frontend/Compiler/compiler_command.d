/*
 *	========================================================
 *
 *	MP-UX C Compiler
 * 	Copyright 2024, Mahrouss Logic, all rights reserved.
 *
 * 	========================================================
 */

module Frontend.Compiler.compiler_command;

///Authors: amlel
///Bugs: None
///Compiler Frontend

import std.stdio;
import Frontend.Compiler.compiler_macro_helpers;

public void mpcc_summon_executable(string path)
{
	import core.stdc.stdlib;
    import std.string;

	system(toStringz(path));
}

public class Platform
{
    public static string getIncludePath()
    {
	    import core.stdc.stdlib;
        import std.string;
        import std.conv;
        import std.path;

        string pathHome = expandTilde("~");
        pathHome ~= "/mp-ux/libc/";

        return pathHome;
    }

    public static string getKernelPath()
    {
	    import core.stdc.stdlib;
        import std.string;
        import std.conv;
        import std.path;

        string pathHome = expandTilde("~");
        pathHome ~= "/mp-ux/mp-ux/";

        return pathHome;
    }
}

public class CompileCommand
{
    public void compile(string includePath, string[] files, bool is_lib, string output, bool compile_only)
    {
        import std.string;
        import std.algorithm;

        foreach (file; files)
        {
            if (file.length == 0)
                continue;

            import std.datetime;

            string input = "/usr/local/bin/bin/cpp";

            string[] arr_macros = CompilerMacroHelpers.getStandardMacros();

            foreach (string macro_name; arr_macros)
            {
                input ~= " --define ";
                input ~= macro_name;
                input ~= "1 ";
                input ~= " ";
            }

            input ~= "--define __FILE__ " ~ file;
            input ~= " ";
            input ~= "--define __DATE__ " ~ Clock.currTime(UTC()).toString();
            input ~= " ";
            input ~= "--working-dir ./ --include-dir " ~ includePath ~ " " ~ file;

            mpcc_summon_executable(input);

            string assembly_source;
            string ext;
            bool ext_now = false;

            foreach (ch; file)
            {
                if (ch == '.')
                {
                    ext_now = true;
                    break;
                }

                if (!ext_now)
                    assembly_source ~= ch;
                else
                    ext ~= ch;
            }

            mpcc_summon_executable("/usr/local/bin/bin/ccplus --asm=masm -fmax-exceptions 10 --compiler=vanhalen " ~
            file ~ ".pp");

            assembly_source ~= ".64x";

            mpcc_summon_executable("/usr/local/bin/bin/64asm " ~ assembly_source);
        }

        if (compile_only)
            return;

        string obj;

        foreach (file; files)
        {
            if (file.length == 0)
                    continue;

            string object_source;

            foreach (ch; file)
            {
                if (ch == '.')
                    break;

                object_source ~= ch;
            }

            object_source ~= ".o";

            obj ~= object_source;
        }

        string shlib_enable = "";

        if (is_lib)
            shlib_enable = " -shared ";

        string output_object = shlib_enable;
        output_object ~= " -o ";
        output_object ~= output;

        mpcc_summon_executable("/usr/local/bin/bin/ld " ~ obj ~ output_object);
    }
}
