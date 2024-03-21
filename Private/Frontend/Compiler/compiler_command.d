/*
 *	========================================================
 *
 *	MultiProcessor C Compiler
 * 	Copyright Mahrouss Logic, all rights reserved.
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
        return "/C/SDK/Public/Kits/";
    }

    public static string getKernelPath()
    {
        return "/C/SDK/Private/Kits/";
    }
}

public class CompileCommandAMD64
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

            string input = "bpp";

            string[] arr_macros = CompilerMacroHelpers.getStandardMacros();

            foreach (string macro_name; arr_macros)
            {
                input ~= " -define ";
                input ~= macro_name;
                input ~= "1 ";
                input ~= " ";
            }

            input ~= "-define __FILE__ " ~ file;
            input ~= " ";
            input ~= "-define __DATE__ " ~ Clock.currTime(UTC()).toString();
            input ~= " ";
            input ~= "-working-dir ./ -include-dir " ~ includePath ~ " " ~ file;

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

            mpcc_summon_executable("ccplus -fmax-exceptions 10 " ~
            file ~ ".pp");

            assembly_source ~= ".s";

            mpcc_summon_executable("i64asm " ~ assembly_source);
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

            object_source ~= ".obj";

            obj ~= object_source;
        }

        string shlib_enable = "";

        if (is_lib)
            shlib_enable = " -shared ";

        string output_object = shlib_enable;
        output_object ~= " -o ";
        output_object ~= output;

        mpcc_summon_executable("link -amd64 " ~ obj ~ output_object);
    }
}
