#include "lex_default.h"
#include "stddef.h"

// Amount of punctuators consisting of multiple chars (e.g. ">>" ">=" ">>=")
#define MCHAR_PUNCT_QT 14

#define BUFFER_PAIR_SIZE (PAGE_SIZE*2)

// Max amount of variations of a punctuator can be continued.
// E.g. if we meet '<' character it can be continued: "<<", "<<=", "<=", "<:", "<%"
#define MCHAR_MAX_VARIATIONS 5	

// Max length of punctuators consisting of multiple chars(not counting the first character)
// The longest one is "%:%:"
#define MCHAR_LEN 4				

//Amount of entries in a hash table for punctuators with multiple characters.
#define MCHAR_PUNCT_MAX_CHAR '|'

//Amount of entries in a hash table for single char punctuators.
#define PUNCT_MAX_CHAR '~'

//First 14(MCHAR_PUNCT_QT) punctuators can have continuation.
#define PUNCTUATORS ">-<+/*%!&|^#.=[](){},;:~?"

#define BUFFER_END_FIRST (c_lstate.cur_file->buf + (PAGE_SIZE))
#define BUFFER_END_SECOND (c_lstate.cur_file->buf + (BUFFER_PAIR_SIZE+1))


// Token's position
struct pos_t
{
	// Offset into buffer
	unsigned int buff_offset;

	// Line number
	unsigned int l_number;

	// Offset from the start of the line
	unsigned int l_offset;
};


// All the infos to be associated with id/kwd
struct c_tok_name
{
	char *lexeme;

	// The token type
	char type;	

	union c_tok_name_union
	{
		struct c_tok_id
		{
			// Position where it first occured
			struct pos_t *first_pos;
			
			//Type of ID
			char *id_type;
		};

		// Keyword type
		int kwd_type;
	} tok_u;

};

struct c_lex_state
{
	//A file to lex
	struct c_file *cur_file;
	
	//How many times we used MOVE_LOOKAHEAD (remove later)
	int moved;

	// Hashtables with continuations for punctuators
	struct hash_table *ht_punct_cont;

	// Last token lexed(lookahead).
	struct c_token *current;

	//Default lexeme size
	size_t lex_size;

	//Base address of current lexeme
	char *lex_base;
	char *lex_cur;

	// The global symbol table.
	// All the default keywords and 
	// global variables are stored here 
	struct symbol_table *global_st;

	// 1 if EOF reached, 0 otherwise
	int eof_reached;

	// size of c_tok_name in bytes
	int c_tok_name_size;

	int line_num;

	char *lookahead;
};


enum c_kwd_type
{
	C_KWD_AUTO = 0,
	C_KWD_BREAK,
	C_KWD_CASE,
	C_KWD_CHAR,
	C_KWD_CONST,
	C_KWD_CONTINUE,
	C_KWD_DEFAULT,
	C_KWD_DO,
	C_KWD_DOUBLE,
	C_KWD_ELSE,
	C_KWD_ENUM,
	C_KWD_EXTERN,
	C_KWD_FLOAT,
	C_KWD_FOR,
	C_KWD_GOTO,
	C_KWD_IF,
	C_KWD_INLINE,
	C_KWD_INT,
	C_KWD_LONG,
	C_KWD_REGISTER,
	C_KWD_RESTRICT,
	C_KWD_RETURN,
	C_KWD_SHORT,
	C_KWD_SIGNED,
	C_KWD_SIZEOF,
	C_KWD_STATIC,
	C_KWD_STRUCT,
	C_KWD_SWITCH,
	C_KWD_TYPEDEF,
	C_KWD_UNION,
	C_KWD_UNSIGNED,
	C_KWD_VOID,
	C_KWD_VOLATILE, 
	C_KWD_WHILE
};


enum c_tok_type
{
	C_TOK_UNKNOWN,
	C_TOK_COMMENT,
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

	float value;
	
};

struct c_token *get_next_token();
void lstate_init(char *fname);

// Create and return an ID
struct c_tok_name *
c_tok_name_create_id(char *lexeme, struct pos_t *t_pos, 
					char *id_type);

// Create and return an KWD
struct c_tok_name *
c_tok_name_create_kwd(char *lexeme, int kwd_type);

void reset_state();
struct c_lex_state store_state();
