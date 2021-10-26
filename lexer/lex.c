

//////////////////////////////////
//								//
//		LEXICAL ANALYZER		//
//								//
//		Author: Big_Jo			//
//								//
//////////////////////////////////								

#include "stdio.h"
#include "lex.h"
#include "string.h"
#include "stdlib.h"
#include "stack.h"
#include "ctype.h"

static struct c_file
{
	FILE *fp;

	char *fname;
	char *dir;

	char *buf;
	
	int bytes_read;
};


struct c_lex_state c_lstate;

/*
	Moves lookahead character forward by x positions
	Handles EOF char if detected
	Forms a lexeme so we have it at the end of get_next_token
*/
#define MOVE_LOOKAHEAD(x)		 																\
	 																							\
	{																							\
																								\
		int move_current = x;																	\
		int move_next = 0;																		\
																								\
		if(GET_LOOKAHEAD_ADDR + x > BUFFER_END_FIRST && 										\
			GET_LOOKAHEAD_ADDR < BUFFER_END_FIRST)												\
		{																						\
			move_current = (GET_LOOKAHEAD_ADDR + x) - BUFFER_END_FIRST;							\
			getchar();																			\
		}																						\
		else if(GET_LOOKAHEAD_ADDR + x > BUFFER_END_SECOND) 									\
			move_current = BUFFER_END_SECOND - GET_LOOKAHEAD_ADDR;								\
																								\
																								\
		move_next = x - move_current;															\
																								\
		strncpy(c_lstate.lex_cur, GET_LOOKAHEAD_ADDR, move_current);							\
		c_lstate.lex_cur += move_current;														\
		c_lstate.lookahead += move_current;														\
								 																\
		if(GET_LOOKAHEAD == EOF) 																\
		{																						\
			if(GET_LOOKAHEAD_ADDR == BUFFER_END_FIRST											\
									||															\
			GET_LOOKAHEAD_ADDR == BUFFER_END_SECOND)											\
			{																					\
				buffer_reload();																\
																								\
				strncpy(c_lstate.lex_cur, GET_LOOKAHEAD_ADDR, move_next);						\
				c_lstate.lex_cur += move_next;													\
				c_lstate.lookahead += move_next;												\
			}																					\
			else 																				\
			{																					\
				c_lstate.eof_reached = 1;														\
				printf("%s\n", "EOF reached");													\
				getchar();																		\
			}																					\
		}																						\
																								\
	}


#define SKIP_WS()					\
	while(isspace(GET_LOOKAHEAD))	\
	{								\
		c_lstate.lookahead+=1;		\
	}

//Incorrectly when GET_LOOKAHEAD_ADDR is start/end of the buffer
//Delete all and commit suicide
#define GET_LOOKAHEAD 		((*c_lstate.lookahead))

#define GET_LOOKAHEAD_ADDR  (c_lstate.lookahead)

//Set ch to the value of lookahead + offset
//Check if we cross the buffer's boundary
static inline int GET_NEXT_LOOKAHEAD(int offset)
{
		int ch = *(GET_LOOKAHEAD_ADDR + offset);													
		if(GET_LOOKAHEAD_ADDR < BUFFER_END_FIRST && 											
			(GET_LOOKAHEAD_ADDR + offset) > BUFFER_END_FIRST)									
		{																						
			ch = *(GET_LOOKAHEAD_ADDR + offset + 1);												
		}																						
		else if(GET_LOOKAHEAD_ADDR + offset > BUFFER_END_SECOND)								
		{																						
			int remainder = BUFFER_END_SECOND - GET_LOOKAHEAD_ADDR;								
			int to_move = offset - remainder;													
			ch = *(c_lstate.cur_file->buf + to_move);											
		}
		return ch;																						
}






//Copy to_copy bytes from src to dest, check if reach the end of the buffer
#define BUF_CPY(dest, src, to_copy)																\
	{																							\
		int copy_current = to_copy;																\
		int copy_next = 0;																		\
																								\
		int buff_offset = 0;																	\
		char *buff = c_lstate.cur_file->buf;													\
																								\
		if(src < (buff + PAGE_SIZE) && src + to_copy > (buff + PAGE_SIZE))						\
		{																						\
			buff_offset = PAGE_SIZE;															\
			copy_current = (buff + buff_offset) - src; 											\
		}																						\
		else if(src + to_copy > buff + BUFFER_PAIR_SIZE)										\
			copy_current = (buff + BUFFER_PAIR_SIZE) - src;										\
																								\
		copy_next = to_copy - copy_current;														\
																								\
		strncpy(dest, src, copy_current); 														\
		strncpy(dest + copy_current, buff + buff_offset, copy_next); 							\
	}

//Clear(set to '\0') the string array containing current lexeme(c_lstate.lex_cur)
#define RS_LEX()																				\
	c_lstate.lex_cur = c_lstate.lex_base;														\
	memset(c_lstate.lex_base, '\0', c_lstate.lex_size);											

static void buffer_reload();
static struct c_file init_file(struct c_file *file, char *fname, char *dir);




static struct c_file init_file(struct c_file *file, char *fname, char *dir)
{
	file->fname = fname;
	file->dir = dir;
	file->bytes_read = 0;
	// + 2 for 2 sentinels
	file->buf = malloc(BUFFER_PAIR_SIZE + 2);
	memset(file->buf, EOF, BUFFER_PAIR_SIZE + 2);
}



/*
	Reloads buffer and moves lookahead to the other when needed.
	Will ONLY work with GET_LOOKAHEAD_ADDR = PAGE_SIZE or BUFFER_PAIR_SIZE
*/
static void buffer_reload()
{	
	// printf("%s\n", "buffer reload");
	//A new value for lookahead
	char *new_lookahead[2] = {c_lstate.cur_file->buf + PAGE_SIZE + 1, c_lstate.cur_file->buf};
	
	//Base address of the buffer to be reloaded
	char *base_addr[2] = {c_lstate.cur_file->buf, c_lstate.cur_file->buf + PAGE_SIZE + 1};

	//index for base_addr  and new_lookahead
	int i = (GET_LOOKAHEAD_ADDR) - (c_lstate.cur_file->buf + PAGE_SIZE);
	
	//If this is more than 0, we want it to be 1
	//This is always odd number
	i = i & 1;

	//Assign new_lookahead
	c_lstate.lookahead = new_lookahead[i];

	/*
	If fread() won't fill the whole buffer,
	the empty part will contain EOFs(-1), 
	so we can detect the end of the file
	*/
	memset(base_addr[i], EOF, PAGE_SIZE);
	fread(base_addr[i], 1, PAGE_SIZE, c_lstate.cur_file->fp);
	// getchar();
}

// A table with continuations for each punctuator that can be continued(needed to init ht_mchar_punct) .
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

//A table with ttypes for multiple char punctuators(needed to init ht_mchar_ttype) 
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
static char ht_mchar_punct[MCHAR_PUNCT_MAX_CHAR + 1][MCHAR_MAX_VARIATIONS][MCHAR_LEN];

// A hash table which contains types of multiple char punctuators
static unsigned char ht_mchar_ttype[MCHAR_PUNCT_MAX_CHAR + 1][MCHAR_MAX_VARIATIONS];

// A hash table which contains types of one char punctuators
static unsigned char ht_punct_ttype[PUNCT_MAX_CHAR];


//Initialize lexer state
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

	c_lstate.lex_size = 128;
	c_lstate.lex_base = malloc(c_lstate.lex_size);
	memset(c_lstate.lex_base, '\0', c_lstate.lex_size);
	c_lstate.lex_cur = c_lstate.lex_base;

	c_lstate.cur_file->fp = fopen(fname, "r+");

	fread(c_lstate.cur_file->buf, 1, PAGE_SIZE, c_lstate.cur_file->fp);
	fread(c_lstate.cur_file->buf + (PAGE_SIZE+1), 1, PAGE_SIZE, c_lstate.cur_file->fp);

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
	int ttype = C_TOK_UNKNOWN;	
	float value = 0;

	struct c_token *result = malloc(sizeof(struct c_token));

	//Skip whitespace characters
	SKIP_WS();

	//Handle Comments 
	while(GET_LOOKAHEAD == '/')
	{
		//Characters that indicate end of the comment
		int first_end_char = 0;
		int second_end_char = 0;

		//If one-line comment, then first end char is '\n',
		//If multiple-line first end char is '*' and second - '\'
		if(GET_NEXT_LOOKAHEAD(1) == '/') first_end_char = '\n';
		else if(GET_NEXT_LOOKAHEAD(1) == '*')
		{
			first_end_char = '*';
			second_end_char = '/';
		}
		else break;
	
		MOVE_LOOKAHEAD(2);
		do
		{
			//Move lookahead until first ene symbol met
			do
			{

				MOVE_LOOKAHEAD(1);
			}while(GET_LOOKAHEAD != first_end_char);

			//If multiple string comment
			if(second_end_char)
			{
				MOVE_LOOKAHEAD(1);
				if(GET_LOOKAHEAD == second_end_char) continue;
			}
		}while(0);
		
		MOVE_LOOKAHEAD(1);
		printf("%s\n", c_lstate.lex_base);
		
		RS_LEX();
		
		SKIP_WS();

	}
	
	SKIP_WS();
	
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
		int prev = 0;

		if(ch == '"' || ch == '\'')
		{

			MOVE_LOOKAHEAD(1);

			do
			{
				prev = GET_LOOKAHEAD;
				MOVE_LOOKAHEAD(1);	
			}while(prev != ch);
				

			if(prev == '"') ttype = C_TOK_STRING;
			else if(prev == '\'') ttype = C_TOK_CHAR; 
		}
		else 
		{
			ch = GET_LOOKAHEAD;

			
			MOVE_LOOKAHEAD(1);
			

			char str[4];
			memset(str, '\0', 4);
			BUF_CPY(str, GET_LOOKAHEAD_ADDR, 3);	

			for(int i = 0; i < MCHAR_MAX_VARIATIONS; i++)
			{
				if(ht_mchar_punct[ch][i][0] == '\0') break;

				if(strncmp(str, ht_mchar_punct[ch][i], strlen(ht_mchar_punct[ch][i])) == 0)
				{
					ttype = ht_mchar_ttype[ch][i];
					MOVE_LOOKAHEAD(strlen(ht_mchar_punct[ch][i]));
					break;
				}

			}

			// If not a multiple char punctuator,
			// take ttype for single char punctuators
			if(ttype == C_TOK_UNKNOWN)
				ttype = ht_punct_ttype[ch];
			
		}


	}
	else if(GET_LOOKAHEAD == EOF)
	{
		c_lstate.lookahead = c_lstate.lookahead - 1;
		MOVE_LOOKAHEAD(1);
	}
	printf("%s\n", c_lstate.lex_base);
	RS_LEX();
	
	result->ttype = ttype;
	return result;
}


	