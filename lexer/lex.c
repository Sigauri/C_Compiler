//////////////////////////////////
//								//
//		LEXICAL ANALYZER		//
//								//
//		 Author: Big_Jo			//
//								//
//////////////////////////////////

#include "stdio.h"
#include "lex.h"
#include "string.h"
#include "stdlib.h"
#include "ctype.h"
#include "lex_default.h"
#include "symboltable.h"



/*

TODOs:
	Rewrite these HUGE macro definitions as functions
	Clean up the mess with hashtables
	Comment everything properly

*/

static struct c_file
{
	FILE *fp;

	char *fname;
	char *dir;

	char *buf;
	
	int bytes_read;
};


struct c_lex_state c_lstate;
struct c_lex_state *saved_state;

/*
	Moves lookahead forward by x positions
	Handles EOF char if detected
	Writes x characters to c_lstate.lex_cur
*/
// #define MOVE_LOOKAHEAD(x)		 																\
// 	 																							\
// 	{																							\
// 																								\
// 		int move_current = x;																	\
// 		int move_next = 0;																		\
// 																								\
// 		if(GET_LOOKAHEAD_ADDR + x > BUFFER_END_FIRST && 										\
// 			GET_LOOKAHEAD_ADDR < BUFFER_END_FIRST)												\
// 		{																						\
// 			move_current = (GET_LOOKAHEAD_ADDR + x) - BUFFER_END_FIRST;							\
// 			getchar();																			\
// 		}																						\
// 		else if(GET_LOOKAHEAD_ADDR + x > BUFFER_END_SECOND) 									\
// 			move_current = BUFFER_END_SECOND - GET_LOOKAHEAD_ADDR;								\
// 																								\
// 																								\
// 		move_next = x - move_current;															\
// 																								\
// 		strncpy(c_lstate.lex_cur, GET_LOOKAHEAD_ADDR, move_current);							\
// 		c_lstate.lex_cur += move_current;														\
// 		c_lstate.lookahead += move_current;														\
// 								 																\
// 		if(GET_LOOKAHEAD == EOF) 																\
// 		{																						\
// 			if(GET_LOOKAHEAD_ADDR == BUFFER_END_FIRST											\
// 									||															\
// 			GET_LOOKAHEAD_ADDR == BUFFER_END_SECOND)											\
// 			{																					\
// 				buffer_reload();																\
// 																								\
// 				strncpy(c_lstate.lex_cur, GET_LOOKAHEAD_ADDR, move_next);						\
// 				c_lstate.lex_cur += move_next;													\
// 				c_lstate.lookahead += move_next;												\
// 			}																					\
// 			else 																				\
// 			{																					\
// 				c_lstate.eof_reached = 1;														\
// 				printf("%s\n", "EOF reached");													\
// 				getchar();																		\
// 			}																					\
// 		}																						\
// 																								\
// 	}


#define GET_LOOKAHEAD 		((*c_lstate.lookahead))
#define GET_LOOKAHEAD_ADDR  (c_lstate.lookahead)

#define SKIP_WS()					\
	while(isspace(GET_LOOKAHEAD))	\
	{								\
		if(GET_LOOKAHEAD == '\n')	\
			c_lstate.line_num++;	\
		c_lstate.lookahead+=1;		\
	}

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

//Clear(set to '\0') c_lstate.lex_base(current lexeme)
#define RS_LEX()																				\
	c_lstate.lex_cur = c_lstate.lex_base;														\
	memset(c_lstate.lex_base, '\0', c_lstate.lex_size);		










static void buffer_reload();
static struct c_file init_file(struct c_file *file, char *fname, char *dir);

void move_lookahead(unsigned int x)
{
																								
		int move_current = x;																	
		int move_next = 0;																		
																								
		if(GET_LOOKAHEAD_ADDR + x > BUFFER_END_FIRST && 										
			GET_LOOKAHEAD_ADDR < BUFFER_END_FIRST)												
		{																						
			move_current = (GET_LOOKAHEAD_ADDR + x) - BUFFER_END_FIRST;							
			getchar();																			
		}																					
		else if(GET_LOOKAHEAD_ADDR + x > BUFFER_END_SECOND) 								
			move_current = BUFFER_END_SECOND - GET_LOOKAHEAD_ADDR;								
																								
																								
		move_next = x - move_current;															
																								
		strncpy(c_lstate.lex_cur, GET_LOOKAHEAD_ADDR, move_current);							
		c_lstate.lex_cur += move_current;														
		c_lstate.lookahead += move_current;														
								 																
		if(GET_LOOKAHEAD == EOF) 																
		{																						
			if(GET_LOOKAHEAD_ADDR == BUFFER_END_FIRST											
									||															
			GET_LOOKAHEAD_ADDR == BUFFER_END_SECOND)											
			{																					
				buffer_reload();																
																								
				strncpy(c_lstate.lex_cur, GET_LOOKAHEAD_ADDR, move_next);						
				c_lstate.lex_cur += move_next;													
				c_lstate.lookahead += move_next;												
			}																					
			else 																				
			{																					
				c_lstate.eof_reached = 1;														
				printf("%s\n", "EOF reached");													
				getchar();																		
			}																					
		}																							
}

// create and return id/kwd
struct c_tok_name *
c_tok_name_create_id(char *lexeme, struct pos_t *t_pos, 
					char *id_type)
{
	struct c_tok_name *result = malloc(sizeof(struct c_tok_name));
	result->lexeme = lexeme;
	result->type = C_TOK_IDENTIFIER;
	result->tok_u.id_type = id_type;
	result->tok_u.first_pos = t_pos;
	return result;
}

struct c_tok_name *c_tok_name_create_kwd(char *lexeme, int kwd_type)
{
	struct c_tok_name *result = malloc(sizeof(struct c_tok_name));
	result->tok_u.kwd_type = kwd_type;
	result->lexeme = lexeme;
	result->type = C_TOK_KEYWORD;
	return result;
}

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

	#define ht_state_print(ht)										\
	{																\
		printf("\n\n%s%zu\n", "In Use: ", ht->in_use);				\
		printf("%s%zu\n", "Collisions: ", ht->collisions);			\
		printf("%s%zu\n", "HT Size: ", ht->ht_size);				\
		printf("%s%zu\n", "ht_hash_str: ", ht->ht_hash_str);		\
		printf("%s%zu\n\n", "ht_hash_int: ", ht->ht_hash_int);		\
	}	

struct c_lex_state store_state()
{
	saved_state = malloc(sizeof(struct c_lex_state));
	saved_state->cur_file = malloc(sizeof(struct c_file));
	saved_state->cur_file->buf = malloc(BUFFER_PAIR_SIZE + 2);
	memcpy(saved_state->cur_file->buf, c_lstate.cur_file->buf, BUFFER_PAIR_SIZE + 2);
	saved_state->eof_reached = c_lstate.eof_reached;
	saved_state->moved = c_lstate.moved;
	saved_state->lex_size = c_lstate.lex_size;
	saved_state->c_tok_name_size = c_lstate.c_tok_name_size;
	saved_state->line_num = c_lstate.line_num;
	saved_state->lookahead = c_lstate.lookahead;

}

void reset_state()
{
	c_lstate.cur_file = saved_state->cur_file;
	c_lstate.moved = saved_state->moved;
	c_lstate.lex_size = saved_state->lex_size;
	c_lstate.eof_reached = saved_state->eof_reached;
	c_lstate.c_tok_name_size = saved_state->c_tok_name_size;
	c_lstate.line_num = saved_state->line_num;
	c_lstate.lookahead = saved_state->lookahead;
}

//Initialize lexer state
void lstate_init(char *fname)
{
	c_lstate.cur_file = malloc(sizeof(struct c_file));
	c_lstate.line_num = 1;

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
	
	c_lstate.c_tok_name_size = sizeof(struct c_tok_name);

	c_lstate.moved = 0;
	c_lstate.eof_reached = 0;

	c_lstate.lex_size = 128;
	c_lstate.lex_base = malloc(c_lstate.lex_size);
	memset(c_lstate.lex_base, '\0', c_lstate.lex_size);
	c_lstate.lex_cur = c_lstate.lex_base;

	c_lstate.cur_file->fp = fopen(fname, "r+");

	fread(c_lstate.cur_file->buf, 1, PAGE_SIZE, c_lstate.cur_file->fp);
	fread(c_lstate.cur_file->buf + (PAGE_SIZE+1), 1, PAGE_SIZE, c_lstate.cur_file->fp);

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

	struct symbol_table *prev = NULL;
	st_create(c_lstate.global_st, prev, 64);
	
	for(int i = 0; i < NUMBER_KWD; i++)
	{
		struct c_tok_name *t_name = c_tok_name_create_kwd(keywords[i], i);
		st_add(c_lstate.global_st, t_name, keywords[i]);
	}

	ht_state_print(c_lstate.global_st->ht);
}

/*

*/

//

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
		//If multiple-line, first end char is '*' and second - '\'
		if(GET_NEXT_LOOKAHEAD(1) == '/') first_end_char = '\n';
		else if(GET_NEXT_LOOKAHEAD(1) == '*')
		{
			first_end_char = '*';
			second_end_char = '/';
		}
		else break;
	
		move_lookahead(2);
		do
		{
			//Move lookahead until first end symbol met
			do
			{

				move_lookahead(1);
			}while(GET_LOOKAHEAD != first_end_char);

			//If multiple string comment
			if(second_end_char)
			{
				move_lookahead(1);
				if(GET_LOOKAHEAD == second_end_char) continue;
			}
		}while(0);
		
		move_lookahead(1);	
		
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
				move_lookahead(1);
				is_float = 1;
				continue;
			}
			else if(GET_LOOKAHEAD > 47 && GET_LOOKAHEAD < 58)
			{
				move_lookahead(1);
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
		
		move_lookahead(1);
		while(isalpha(GET_LOOKAHEAD) || GET_LOOKAHEAD == '_' || isdigit(GET_LOOKAHEAD))
		{
			move_lookahead(1);
			
		}

		ttype = C_TOK_IDENTIFIER;
		
		/*  
			We have to check if the current 
			symbol table contains this id/kwd,
			and if it does, get it and set the ttype.

			If its not there, add it or not will be parser's task.
		*/
		struct c_tok_name *value = NULL;
		st_find(c_lstate.global_st, value, c_lstate.lex_base);

		// If we found an entry in a Symbol Table
		if(value)
		{	
			ttype = value->type;
		}

	}
	//Handle a punctuator
	else if(ispunct(GET_LOOKAHEAD))
	{
		int ch = GET_LOOKAHEAD;
		int prev = 0;

		//Handle string literals and character constants
		if(ch == '"' || ch == '\'')
		{

			move_lookahead(1);

			do
			{
				prev = GET_LOOKAHEAD;
				move_lookahead(1);	
			}while(prev != ch);
				

			if(prev == '"') ttype = C_TOK_STRING;
			else if(prev == '\'') ttype = C_TOK_CHAR; 
		}
		//Handle punctuators/operators
		else 
		{
			ch = GET_LOOKAHEAD;

			
			move_lookahead(1);
			

			char str[4];
			memset(str, '\0', 4);
			BUF_CPY(str, GET_LOOKAHEAD_ADDR, 3);	

			ttype = ht_punct_ttype[ch];
			for(int i = 0; i < MCHAR_MAX_VARIATIONS; i++)
			{
				if(ht_mchar_punct[ch][i][0] == '\0') break;

				if(strncmp(str, ht_mchar_punct[ch][i], strlen(ht_mchar_punct[ch][i])) == 0)
				{
					ttype = ht_mchar_ttype[ch][i];
					move_lookahead(strlen(ht_mchar_punct[ch][i]));
					break;
				}

			}
				
			
		}


	}
	else if(GET_LOOKAHEAD == EOF)
	{
		c_lstate.lookahead = c_lstate.lookahead - 1;
		move_lookahead(1);
	}
	printf("%s\n", c_lstate.lex_base);
	RS_LEX();

	result->ttype = ttype;
	return result;
}


	