char get_current();
void move();

enum c_tok_type
{
	C_TOK_UNKNOWN,
	C_TOK_KEYWORD,
	C_TOK_IDENTIFIER,
	C_TOK_CONSTANT,
	C_TOK_STRING,
	C_TOK_PUNCTUATOR,
	C_TOK_DIRECTIVE,
	C_TOK_LESS,
	C_TOK_LESS_EQ,
	C_TOK_MORE,
	C_TOK_MORE_EQ,
	C_TOK_EQ,
	C_TOK_EQ_EQ,
	C_TOK_NOT,
	C_TOK_NOT_EQ,
	C_TOK_OPEN_PAREN,
	C_TOK_CLOSE_PAREN,
	C_TOK_OPEN_BRACE,
	C_TOK_CLOSE_BRACE,
	C_TOK_COMMA,
	C_TOK_SEMI,
	C_TOK_MINUS_MINUS,
	C_TOK_MINUS,
	C_TOK_MINUS_EQ,
	C_TOK_CHAR,
	C_TOK_SHIFT_R_EQ,
	C_TOK_SHIFT_L_EQ,
	C_TOK_SHIFT_R,
	C_TOK_SHIFT_L,
	C_TOK_PLUS_EQ,
	C_TOK_PLUS,
	C_TOK_PLUS_PLUS,
	C_TOK_OPEN_SQR_BRACKET,
	C_TOK_CLOSE_SQR_BRACKET,
	C_TOK_DOT,
	C_TOK_ONES_COMPLEMENT,
	C_TOK_COLON,
	C_TOK_ARROW,
	C_TOK_DIV_EQ,
	C_TOK_DIV,
	C_TOK_MUL_EQ,
	C_TOK_MUL,
	C_TOK_PERCENT_EQ,
	C_TOK_PERCENT,
	C_TOK_HASH,
	C_TOK_HASH_HASH,
	C_TOK_BIT_AND,
	C_TOK_AND,
	C_TOK_BIT_AND_EQ,
	C_TOK_OR,
	C_TOK_BIT_OR,
	C_TOK_BIT_OR_EQ,
	C_TOK_EXP,
	C_TOK_EXP_EQ,
	C_TOK_THREE_DOT,
	C_TOK_CONDITION

};

struct c_token
{
	enum c_tok_type ttype;
	void *attribute;

	union c_token_t
	{
		//for the numbers
		float value;

		//ID's and Keywords
		void *ht_pointer;

		char *lexeme;
	};
	
};

struct c_token *get_next_token();
void lstate_init(char *fname);
