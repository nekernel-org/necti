module Frontend.Compiler.compiler_macro_helpers;

class CompilerMacroHelpers
{
    static public string[] getStandardMacros()
    {
        string[] macros = [ "__64x0__", "__mpux__" ];
        return macros;
    }
}