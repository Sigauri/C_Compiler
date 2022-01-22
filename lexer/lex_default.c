#include "lex_default.h"
#include "lex.h"
char keywords[NUMBER_KWD][LEN_KWD] = 
{
	"auto",
	"break",
	"case",
	"char",
	"const",
	"continue",
	"default",
	"do",
	"double",
	"else",
	"enum",
	"extern",
	"float",
	"for",
	"goto",
	"if",
	"inline",
	"int",
	"long",
	"register",
	"restrict",
	"return",
	"short",
	"signed",
	"sizeof",
	"static",
	"struct",
	"switch",
	"typedef",
	"union",
	"unsigned",
	"void",
	"volatile",
	"while"
};

char continuations[MCHAR_PUNCT_QT][24] = {  
											"> = >=\0", "- = >\0", "< = <= : %\0", "+ =\0", "=\0", "=\0", "= : :%:\0",
											"=\0", "&\0", "| =\0", "=\0", "#\0", "..\0", "="
										 };