#include "stdio.h"
#include "lexer/lex.h"
#include "stdarg.h"
struct c_token *current = NULL;

// Synchronizing tokens
#define SYNC_TOKENS_SIZE 2
const int SYNC_TOKENS[SYNC_TOKENS_SIZE] = {C_TOK_OPEN_BRACE, C_TOK_SEMI};
extern struct c_lex_state c_lstate;


int a = 0;

void primary_expression();
void expression();
void postfix_expression();
void cast_expression();
void unary_expression();
void mul_expression();
void cond_expression();
void additive_expression();
void assignment_expression();
void error(char *msg, ...)
{	
	a = 1;
	printf("%s\n", "error");
	// char buffer[512];
	// va_list args;
	// va_start(args, msg);
	// vsnprintf(buffer, sizeof(buffer), msg, args);
	// printf(buffer);
	
	// while(1)
	// {
	// 	current = get_next_token();
	// 	for(int i = 0; i < SYNC_TOKENS_SIZE; i++)
	// 	{
	// 		if(current->ttype == SYNC_TOKENS[i] ||
	// 			current->ttype == C_TOK_UNKNOWN) return;
	// 	}
	// }
}

void expression()
{
	assignment_expression();
}

void primary_expression()
{
	if( current->ttype != C_TOK_IDENTIFIER  &&
		current->ttype != C_TOK_STRING		&&
		current->ttype != C_TOK_CONSTANT	
	  )
	{
		//error
		if(current->ttype == C_TOK_OPEN_PAREN)
		{
			current = get_next_token();
			expression();
			// current = get_next_token();
			if(current->ttype == C_TOK_CLOSE_PAREN) return;
			else error("Error at line %d: expression expected\n", c_lstate.line_num);
		}
		
		error("Error at line %d: expression expected\n", c_lstate.line_num);
	}
}


void unary_expression()
{

	switch(current->ttype)
	{
		case C_TOK_PLUS_PLUS:
		{
			current = get_next_token();
			unary_expression();
			break;
		}
		case C_TOK_MINUS_MINUS:
		{
			current = get_next_token();
			unary_expression();
			break;
		}
		case C_TOK_BIT_AND: case C_TOK_MUL:
		case C_TOK_PLUS: case C_TOK_MINUS:
		case C_TOK_ONES_COMPLEMENT: case C_TOK_NOT:
		{
			current = get_next_token();
			cast_expression();
			break;
		}
		default:
		{
			postfix_expression();
			break;
		}
		//sizeof
		//Alignof

	}

}

void cast_expression()
{
	unary_expression();
}

void mul_expression()
{
	int out = 0;
	cast_expression();
	while(!out)
	{
		switch(current->ttype)
		{
			case C_TOK_MUL:
			{
				current = get_next_token();
				cast_expression();
				continue;
				break;
			}

			case C_TOK_DIV:
			{
				current = get_next_token();
				cast_expression();
				continue;			
				break;
			}

			case C_TOK_PERCENT:
			{
				current = get_next_token();
				cast_expression();
				continue;
				break;
			}

			default:
			{
				out = 1;
			}
		}
	}
	
}

void additive_expression()
{
	int out = 0;
	mul_expression();
	while(!out)
	{
		switch(current->ttype)
		{
			case C_TOK_PLUS:
			{
				current = get_next_token();
				mul_expression();
				continue;
				break;
			}

			case C_TOK_MINUS:
			{
				current = get_next_token();
				mul_expression();
				continue;			
				break;
			}

			default:
			{
				out = 1;
			}
		}
	}
	
}

void postfix_expression()
{

	primary_expression();
	int out = 0;
	while(!out)
	{

		current = get_next_token();
		switch(current->ttype)
		{
			case C_TOK_OPEN_SQR_BRACKET:
			{
				current = get_next_token();
				expression();
				if(current->ttype != C_TOK_CLOSE_SQR_BRACKET)
					error("error at line %d: expected postfix expression\n", c_lstate.line_num);
				continue;
				break;
			}
			case C_TOK_OPEN_PAREN:
			{
				//argument-expression-list
				break;

			}
			case C_TOK_DOT:
			{
				current = get_next_token();
				if(current->ttype != C_TOK_IDENTIFIER) 
					error("error at line %d: expected identifier\n", c_lstate.line_num);
				continue;
			}
			case C_TOK_ARROW:
			{
				current = get_next_token();
				if(current->ttype != C_TOK_IDENTIFIER) 
					error("error at line %d: expected identifier\n", c_lstate.line_num);
				continue;				
			}
			case C_TOK_PLUS_PLUS: case C_TOK_MINUS_MINUS: continue;

			default:
			{
				out = 1;
				continue;
			}
		}
	}

}

void shift_expression()
{
	int out = 0;
	additive_expression();
	while(!out)
	{
		switch(current->ttype)
		{
			case C_TOK_SHIFT_L:
			{
				current = get_next_token();
				additive_expression();
				continue;
				break;
			}

			case C_TOK_SHIFT_R:
			{
				current = get_next_token();
				additive_expression();
				continue;			
				break;
			}

			default:
			{
				out = 1;
			}
		}
	}	
}

void relationl_expression()
{
	int out = 0;
	shift_expression();
	while(!out)
	{
		switch(current->ttype)
		{
			case C_TOK_LESS:
			{
				current = get_next_token();
				shift_expression();
				continue;
				break;
			}

			case C_TOK_MORE:
			{
				current = get_next_token();
				shift_expression();
				continue;			
				break;
			}

			case C_TOK_LESS_EQ:
			{
				current = get_next_token();
				shift_expression();
				continue;			
				break;				
			}

			case C_TOK_MORE_EQ:
			{
				current = get_next_token();
				shift_expression();
				continue;			
				break;				
			}

			default:
			{
				out = 1;
			}
		}
	}		
}

void eq_expression()
{
	int out = 0;
	relationl_expression();
	while(!out)
	{
		switch(current->ttype)
		{
			case C_TOK_EQ_EQ:
			{
				current = get_next_token();
				relationl_expression();
				continue;
				break;
			}

			case C_TOK_NOT_EQ:
			{
				current = get_next_token();
				relationl_expression();
				continue;			
				break;
			}

			default:
			{
				out = 1;
			}
		}
	}		
}

void and_expression()
{
	int out = 0;
	eq_expression();
	while(!out)
	{
		switch(current->ttype)
		{
			case C_TOK_BIT_AND:
			{
				current = get_next_token();
				eq_expression();
				continue;
				break;
			}

			default:
			{
				out = 1;
			}
		}
	}		
}

void excl_or_expression()
{
	and_expression();
	while(current->ttype == C_TOK_EXP)
	{
		current = get_next_token();
		and_expression();		
	}
}

void incl_or_expression()
{
	excl_or_expression();
	while(current->ttype == C_TOK_BIT_OR)
	{
		current = get_next_token();
		excl_or_expression();
	}
}

void logical_and_expression()
{
	incl_or_expression();
	while(current->ttype == C_TOK_AND)
	{
		current = get_next_token();
		incl_or_expression();
	}

}

void logical_or_expression()
{
	logical_and_expression();
	while(current->ttype == C_TOK_OR)
	{
		current = get_next_token();
		logical_and_expression();
	}

}


void cond_expression()
{
	logical_or_expression();
	while(current->ttype == C_TOK_CONDITION)
	{
		current = get_next_token();
		expression();
		if(current->ttype != C_TOK_COLON)
			error("Error in line %d: symbol ':' expected in conditional expression", c_lstate.line_num);
		current = get_next_token();
		logical_or_expression();
	}

}

void assignment_expression()
{
	store_state();
	unary_expression();
	// current = get_next_token();
	if( current->ttype != C_TOK_EQ && 
		current->ttype != C_TOK_MUL_EQ &&
		current->ttype != C_TOK_DIV_EQ &&
		current->ttype != C_TOK_PERCENT_EQ &&
		current->ttype != C_TOK_PLUS_EQ )
		error("Error at line %n: expected assignment operator", c_lstate.line_num);
	current = get_next_token();
	if(a)
	{
		printf("%s\n", "error detected");
		reset_state();
		cond_expression();
		a = 0;
		return;
	}
	assignment_expression();

	printf("%d\n", a);

}

	
	

	


int main()
{
	lstate_init("lexer/test.c");
	current = get_next_token();
	assignment_expression();
	return 0;
}