prog: parser.c lexer/lex.c lexer/lex_default.c lexer/hashtable.c
	gcc parser.c lexer/lex.c lexer/lex_default.c lexer/hashtable.c -o prog -g 