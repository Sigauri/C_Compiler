#include "stdio.h"
#include "lex.h"
#include "string.h"
#include "stdlib.h"
#include "stack.h"
#include "ctype.h"



static void buffer_reload();

// Amount of punctuators consisting of multiple chars (e.g. ">>" ">=" ">>=")
#define MCHAR_PUNCT_QT 14

// Page size
#define PAGE_SIZE 4096

#define BUFFER_PAIR_SIZE (PAGE_SIZE*2)

// Max amount of variations of a punctuator can be continued.
// E.g. if we meet '<' character it can be continued: "<<", "<<=", "<=", "<:", "<%"
#define MCHAR_MAX_VARIATIONS 5	

// Max length of punctuators consisting of multiple chars(not counting the first character)
// The longest one is "%:%:"
#define MCHAR_LEN 3				

//Amount of entries in a hash table for punctuators with multiple characters.
#define MCHAR_PUNCT_MAX_CHAR '|'

//Amount of entries in a hash table for single char punctuators.
#define PUNCT_MAX_CHAR '~'

//First 14(MCHAR_PUNCT_QT) punctuators can have continuation.
#define PUNCTUATORS ">-<+/*%!&|^#.=[](){},;:~?"

#define BUFFER_END_FIRST (c_lstate.cur_file->buf + (PAGE_SIZE - 4))
#define BUFFER_END_SECOND (c_lstate.cur_file->buf + (BUFFER_PAIR_SIZE - 4))



#define MOVE_LOOKAHEAD(x) 																	\
	do 																						\
	{																						\
		lexeme[i++] = GET_LOOKAHEAD;														\
		c_lstate.moved++;																	\
		int ch = GET_LOOKAHEAD;																\
		(c_lstate.lookahead += x);															\
																							\
		if(GET_LOOKAHEAD == EOF) 															\
		{																					\
			if(GET_LOOKAHEAD_ADDR == BUFFER_END_FIRST										\
									||														\
			GET_LOOKAHEAD_ADDR == BUFFER_END_SECOND)										\
			{																				\
				buffer_reload();															\
			}																				\
			else 																			\
			{																				\
				c_lstate.eof_reached = 1;													\
				printf("%s\n", "EOF reached");												\
			}																				\
		}																					\
	}while(0);

#define GET_LOOKAHEAD 		((*c_lstate.lookahead))
#define GET_PREV_LOOKAHEAD  ((*(c_lstate.lookahead-1)))
#define GET_NEXT_LOOKAHEAD  ((*(c_lstate.lookahead+1)))
#define GET_LOOKAHEAD_ADDR  (c_lstate.lookahead)


struct c_file
{
	FILE *fp;

	char *fname;
	char *dir;

	char *buf;
	
	int bytes_read;
};


struct c_lex_state
{
	//Atack of header files to lex
	struct stack *header_files;

	//A file to lex
	struct c_file *cur_file;
	
	//How many times we used MOVE_LOOKAHEAD (remove later)
	int moved;

	// 1 if EOF reached, 0 otherwise
	int eof_reached;
	char *lookahead;
}	c_lstate;



static struct c_file init_file(struct c_file *file, char *fname, char *dir)
{
	file->fname = fname;
	file->dir = dir;
	file->bytes_read = 0;
	file->buf = malloc(BUFFER_PAIR_SIZE);
	memset(file->buf, EOF, BUFFER_PAIR_SIZE);
}


static void buffer_reload()
{	
	//A new value for lookahead
	char *new_lookahead[2] = {c_lstate.cur_file->buf + PAGE_SIZE, c_lstate.cur_file->buf};
	
	//Base address of the buffer to be reloaded
	char *base_addr[2] = {c_lstate.cur_file->buf, c_lstate.cur_file->buf + PAGE_SIZE};

	int i = (GET_LOOKAHEAD_ADDR) - (c_lstate.cur_file->buf + PAGE_SIZE - 4);
	
	i = (i>>12) & 1;

	//Assign new_lookahead
	c_lstate.lookahead = new_lookahead[i];
	memset(base_addr[i], EOF, PAGE_SIZE);
	fread(base_addr[i], 1, PAGE_SIZE - 4, c_lstate.cur_file->fp);
	
	//Sentinel
	*(base_addr[i] + PAGE_SIZE-4) = EOF;
	printf("%s\n", "buffer reload");
	getchar();
}

// A table with continuations for each punctuator that can be continued.
static unsigned char mchar_punct[MCHAR_PUNCT_QT] [MCHAR_MAX_VARIATIONS] [MCHAR_LEN] = 
				{
					">=", ">", "=", "", "", 		// '>'
					">", "-", "=", "", "",			// '-'
					"<=", "<", "=", ":", "%",		// '<'
					"+", "=", "", "", "",			// '+'
					"=", "", "", "", "",			// '/'
					"=", "", "", "","",				// '*'
					":%:", ":", "=", "", "",		// '%'
					"=", "", "", "", "",			// '!'
					"&", "=", "", "", "",			// '&'
					"|", "=", "", "", "",			// '|'
					"=", "", "", "", "",			// '^'
					"#", "", "", "", "",			// "#"
					"..", "", "", "", "",			// '.'
					"=", "", "", "", ""				// '='
				};

static unsigned char mchar_ttype[MCHAR_PUNCT_QT][MCHAR_MAX_VARIATIONS] = 
				{
					C_TOK_SHIFT_R_EQ, C_TOK_SHIFT_R, C_TOK_MORE_EQ, C_TOK_UNKNOWN, C_TOK_UNKNOWN,
					C_TOK_ARROW, C_TOK_MINUS_MINUS, C_TOK_MINUS_EQ, C_TOK_UNKNOWN, C_TOK_UNKNOWN,
					C_TOK_SHIFT_L_EQ, C_TOK_SHIFT_L, C_TOK_LESS_EQ, C_TOK_OPEN_SQR_BRACKET, C_TOK_OPEN_BRACE,
					C_TOK_PLUS_PLUS, C_TOK_PLUS_EQ, C_TOK_UNKNOWN, C_TOK_UNKNOWN, C_TOK_UNKNOWN,
					C_TOK_DIV_EQ, C_TOK_UNKNOWN, C_TOK_UNKNOWN, C_TOK_UNKNOWN, C_TOK_UNKNOWN,
					C_TOK_MINUS_EQ, C_TOK_UNKNOWN, C_TOK_UNKNOWN, C_TOK_UNKNOWN, C_TOK_UNKNOWN,
					C_TOK_HASH_HASH, C_TOK_HASH, C_TOK_PERCENT_EQ, C_TOK_UNKNOWN, C_TOK_UNKNOWN,
					C_TOK_NOT_EQ, C_TOK_UNKNOWN, C_TOK_UNKNOWN, C_TOK_UNKNOWN, C_TOK_UNKNOWN,
					C_TOK_AND, C_TOK_BIT_AND_EQ, C_TOK_UNKNOWN, C_TOK_UNKNOWN, C_TOK_UNKNOWN,
					C_TOK_OR, C_TOK_BIT_OR_EQ, C_TOK_UNKNOWN, C_TOK_UNKNOWN, C_TOK_UNKNOWN,
					C_TOK_EXP_EQ, C_TOK_UNKNOWN, C_TOK_UNKNOWN, C_TOK_UNKNOWN, C_TOK_UNKNOWN,
					C_TOK_HASH_HASH, C_TOK_UNKNOWN, C_TOK_UNKNOWN, C_TOK_UNKNOWN, C_TOK_UNKNOWN,
					C_TOK_THREE_DOT, C_TOK_UNKNOWN, C_TOK_UNKNOWN, C_TOK_UNKNOWN, C_TOK_UNKNOWN,
					C_TOK_EQ_EQ, C_TOK_UNKNOWN, C_TOK_UNKNOWN, C_TOK_UNKNOWN, C_TOK_UNKNOWN
				};


// A hash table which contains continuations of punctuators
// Intitialized from the table above(mchar_punct)
static char ht_mchar_punct[MCHAR_PUNCT_MAX_CHAR + 1][MCHAR_MAX_VARIATIONS][MCHAR_LEN];
static unsigned char ht_mchar_ttype[MCHAR_PUNCT_MAX_CHAR + 1][MCHAR_MAX_VARIATIONS];
static unsigned char ht_punct_ttype[PUNCT_MAX_CHAR];

void lstate_init(char *fname)
{
	c_lstate.cur_file = malloc(sizeof(struct c_file));
	

	int fname_length = strlen(fname);
	char *dir;
	char *file_name;

	// Extract file name
	for(int i = fname_length; i >= 0; i--)
	{
		
		if(fname[i] == '/') 
		{
			dir = malloc(i + 1);
			file_name = malloc(fname_length - i);

			memcpy(file_name, (fname + i + 1), fname_length - i);
			memcpy(dir, fname, i+1);

			break;
		}	
	}

	init_file(c_lstate.cur_file, file_name, dir);
	
	c_lstate.moved = 0;
	c_lstate.eof_reached = 0;

	c_lstate.cur_file->fp = fopen(fname, "r+");
	fread(c_lstate.cur_file->buf, 1, PAGE_SIZE - 4, c_lstate.cur_file->fp);
	fread(c_lstate.cur_file->buf + PAGE_SIZE, 1, PAGE_SIZE - 4, c_lstate.cur_file->fp);

	c_lstate.cur_file->buf[PAGE_SIZE - 4] = EOF;
	c_lstate.cur_file->buf[BUFFER_PAIR_SIZE - 4] = EOF;

	c_lstate.header_files = create_stack();
	c_lstate.lookahead = c_lstate.cur_file->buf;


	ht_punct_ttype['>'] = C_TOK_MORE;
	ht_punct_ttype['-'] = C_TOK_MINUS;
	ht_punct_ttype['<'] = C_TOK_LESS;
	ht_punct_ttype['+'] = C_TOK_PLUS;
	ht_punct_ttype['/'] = C_TOK_DIV;
	ht_punct_ttype['*'] = C_TOK_MUL;
	ht_punct_ttype['%'] = C_TOK_PERCENT;
	ht_punct_ttype['!'] = C_TOK_NOT;
	ht_punct_ttype['&'] = C_TOK_AND;
	ht_punct_ttype['|'] = C_TOK_BIT_OR;
	ht_punct_ttype['^'] = C_TOK_EXP;
	ht_punct_ttype['#'] = C_TOK_HASH;
	ht_punct_ttype['.'] = C_TOK_DOT;
	ht_punct_ttype['='] = C_TOK_EQ;
	ht_punct_ttype['['] = C_TOK_OPEN_SQR_BRACKET;
	ht_punct_ttype[']'] = C_TOK_CLOSE_SQR_BRACKET;
	ht_punct_ttype['('] = C_TOK_OPEN_PAREN;
	ht_punct_ttype[')'] = C_TOK_CLOSE_PAREN;
	ht_punct_ttype['{'] = C_TOK_OPEN_BRACE;
	ht_punct_ttype['}'] = C_TOK_CLOSE_BRACE;
	ht_punct_ttype[','] = C_TOK_COMMA;
	ht_punct_ttype[';'] = C_TOK_SEMI;
	ht_punct_ttype[':'] = C_TOK_COLON;
	ht_punct_ttype['~'] = C_TOK_ONES_COMPLEMENT;
	ht_punct_ttype['?'] = C_TOK_CONDITION;


	
	char punctuators[] = PUNCTUATORS;

	for(int i = 0; i < MCHAR_PUNCT_QT; i++)
	{
		int ch = 0;

		ch = punctuators[i];
		for(int j = 0; j < MCHAR_MAX_VARIATIONS; j++)
		{
			strcpy(ht_mchar_punct[ch][j], mchar_punct[i][j]);
			ht_mchar_ttype[ch][j] = mchar_ttype[i][j];
		}
			
	}
}


struct c_token *get_next_token()
{
	int lookahead = 0;

	int ttype = C_TOK_UNKNOWN;
	float value = 0;

	struct c_token *result = malloc(sizeof(struct c_token));
	memset(result, 0, sizeof(struct c_token));

		char lexeme[64];
		memset(lexeme, 0, 64);
		int i = 0;

	//Skip whitespace characters
	while(isspace(GET_LOOKAHEAD)) 
		MOVE_LOOKAHEAD(1);

	char *lookahead_old = GET_LOOKAHEAD_ADDR;

	if(GET_LOOKAHEAD == '/')
	{
		if(GET_NEXT_LOOKAHEAD == '/')
		{
			MOVE_LOOKAHEAD(1);
			while(GET_LOOKAHEAD != '\n')
			{
				MOVE_LOOKAHEAD(1);
			}
			while(isspace(GET_LOOKAHEAD)) 
			MOVE_LOOKAHEAD(1);
		}

	}

	//Handle a digit
	if(isdigit(GET_LOOKAHEAD))
	{
		int is_float = 0;
		
		while(1)
		{	
			if(GET_LOOKAHEAD == '.' && !is_float)
			{
				MOVE_LOOKAHEAD(1);
				is_float = 1;
				continue;
			}
			else if(GET_LOOKAHEAD > 47 && GET_LOOKAHEAD < 58)
			{
				MOVE_LOOKAHEAD(1);
				continue;
			}


			break;
		}
		
		
		//A string that contains the number
		char number[32];
		memset(number, '\0', 31);

		//Copy the string from file buffer
		int bytes_to_copy = GET_LOOKAHEAD_ADDR - lookahead_old;
		memcpy(number, lookahead_old, bytes_to_copy);

		result->value = atof(number);
		ttype = C_TOK_CONSTANT;
		
	}
	//Handle an Identifier
	else if(isalpha(GET_LOOKAHEAD) || GET_LOOKAHEAD == '_')
	{
		
		MOVE_LOOKAHEAD(1);
		while(isalpha(GET_LOOKAHEAD) || GET_LOOKAHEAD == '_' || isdigit(GET_LOOKAHEAD))
		{
			MOVE_LOOKAHEAD(1);
			
		}
		// int lexeme_size = GET_LOOKAHEAD_ADDR - lookahead_old;
		// lexeme = malloc(lexeme_size);

		// memcpy(lexeme, lookahead_old, lexeme_size);
		// lexeme[lexeme_size] = '\0';
		
		// printf("%s\n", lexeme);

		//look in a hash table


		ttype = C_TOK_IDENTIFIER;
	}
	//Handle a Directive
	else if(GET_LOOKAHEAD == '#')
	{
		//Handle a directive 
	}
	//Handle a punctuator
	else if(ispunct(GET_LOOKAHEAD))
	{
		int ch = GET_LOOKAHEAD;

		if(ch == '"' || ch == '\'')
		{

			MOVE_LOOKAHEAD(1);

			do
			{
				MOVE_LOOKAHEAD(1);	
			}while(GET_PREV_LOOKAHEAD != ch);
				

			if(GET_PREV_LOOKAHEAD == '"') ttype = C_TOK_STRING;
			else if(GET_PREV_LOOKAHEAD == '\'') ttype = C_TOK_CHAR; 
		}
		else 
		{
			ch = GET_LOOKAHEAD;
			MOVE_LOOKAHEAD(1);
			

			char str[4];
			memset(str, '\0', 4);
			memcpy(str, GET_LOOKAHEAD_ADDR, 3);
			

			for(int i = 0; i < MCHAR_MAX_VARIATIONS; i++)
			{
				if(ht_mchar_punct[ch][i][0] == '\0') break;

				if(strncmp(str, ht_mchar_punct[ch][i], strlen(ht_mchar_punct[ch][i])) == 0)
				{
					ttype = ht_mchar_ttype[ch][i];
					
					for (int i = 0; i < strlen(ht_mchar_punct[ch][i]); i++)
					MOVE_LOOKAHEAD(1);
					break;
				}

			}

			// If not a multiple char punctuator,
			// take ttype for single char punctuators
			if(ttype == C_TOK_UNKNOWN)
				ttype = ht_punct_ttype[ch];
			
		}


	}

	result->ttype = ttype;
	printf("lexeme: %s\n", lexeme);


	
	return result;
}


	