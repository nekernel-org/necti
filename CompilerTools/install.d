/*
 *	========================================================
 *
 *	MP-UX C Compiler
 * 	Copyright Mahrouss Logic, all rights reserved.
 *
 * 	========================================================
 */

module mpcc.install;

///Author: amlel
///This helps install the mp-ux toolchain.
class InstallFactory
{
    this(string where, string from)
    {
        import std;
        import std.file;

        try 
        {
            from.copy(where);
        } 
        catch(Exception e) 
        { 
            writeln("install.d: error: ", e.msg);
        }
    }
}

void main(string[] args)
{
    auto factory = new InstallFactory(args[1], args[2]);
}
