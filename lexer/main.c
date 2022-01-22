#include "stdio.h"
#include "string.h"
#include "lex.h"

#define DEBUG_INFO

#include "symboltable.h"
#include "stdlib.h"

int main()
{	
	// struct hash_table *ht = ht_create(32);
	lstate_init("test.c");
	get_next_token();
	struct c_token *tkn = get_next_token();
	printf("%zu\n", tkn->ttype);
}