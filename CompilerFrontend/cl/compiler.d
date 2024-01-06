/*
 *	========================================================
 *
 *	MP-UX C Compiler
 * 	Copyright Mahrouss Logic, all rights reserved.
 *
 * 	========================================================
 */

module mpcc.compiler;

///Authors: amlel
///Bugs: None
///Compiler Frontend

import std.stdio;

public void mcc_summon_executable(string path) 
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

            mcc_summon_executable("/usr/local/bin/bin/cpp --define __64x0__ 1 --define __LP64__ 1 --define __64000__ 1 --define __OPTIMIZED_C__ 1 " ~
                                 "--define __FILE__ " ~ file ~ " --define __DATE__ " ~ Clock.currTime(UTC()).toString() 
                                 ~ " " ~
                                " --working-dir ./ --include-dir " ~ includePath ~ " " ~ file);

            string changed;
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
                    changed ~= ch;
                else
                    ext ~= ch;
            }

            if (ext == ".cc")
            {
                mcc_summon_executable("/usr/local/bin/bin/ccplus --asm=masm -fmax-exceptions 20 --compiler=dolvik " ~
                file ~ ".pp");
            }
            else
            {
                mcc_summon_executable("/usr/local/bin/bin/cc --asm=masm -fmax-exceptions 20 --compiler=dolvik " ~
                file ~ ".pp");
            }

            changed ~= ".64x";

            mcc_summon_executable("/usr/local/bin/bin/masm -m64000 " ~ changed);
        }

        if (compile_only)
            return;

        string obj;

        foreach (file; files)
        {
            if (file.length == 0)
                    continue;

            string changed;

            foreach (ch; file)
            {
                if (ch == '.')
                    break;

                changed ~= ch;
            }

            changed ~= ".o ";

            obj ~= changed;
        }

        string shlib_enable = "";

        if (is_lib)
            shlib_enable = " -shared ";

        string output_object = shlib_enable;
        output_object ~= " -o ";
        output_object ~= output;

        mcc_summon_executable("/usr/local/bin/bin/ld -m64000 " ~
        obj ~ output_object);
    }
}