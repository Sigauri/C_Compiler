#include "stdio.h"
#include "string.h"
#include "lex.h"

int main()
{
	lstate_init("test.c");

	struct c_token *tok;
	tok = get_next_token();

	printf("%zu\n", tok);

}