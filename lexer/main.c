#include "stdio.h"
#include "string.h"
#include "lex.h"
#include "stdlib.h"
int main()
{

	lstate_init("test.c");

	struct c_token *tok;
	while(1)
	{
		tok = get_next_token();
	}
}