
/*
 *	========================================================
 *
 *	MP-UX C Compiler
 * 	Copyright WestCo, all rights reserved.
 *
 * 	========================================================
 */

module cl.dparse;

//Parse D syntax, from a line to AST.

struct ast_type
{
    string    p_keyword;
    ast_type* p_prev;
    ast_type* p_next;
}

