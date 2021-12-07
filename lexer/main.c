#include "stdio.h"
#include "string.h"
#include "lex.h"

#define DEBUG_INFO

#include "hashtable.h"
// #include "symboltable.h"
#include "stdlib.h"

int main()
{	
	struct hash_table *ht = ht_create(32);

	struct c_tok_name *kwd[34];

	for(int i = 0; i < 34; i++)
	{
		kwd[i] = c_tok_name_create_kwd(keywords[i], i);
	}
	

	unsigned long hash = 0;

	for(int i = 0; i < 32; i++)
	{
		ht_insert_str(&ht, kwd[i], kwd[i]->lexeme);
	}	

	for(int i = 0; i < 26; i++)
	{
		hash = ht->ht_hash_str(kwd[i]->lexeme);
		ht_remove(&ht, ht_find(ht, kwd[i]->lexeme, hash), hash);
	}


	for(int i = 0; i < 32; i++)
	{
		printf("%zu\n", ht->ht_arr[i]);
	}


	ht_state_print(ht);

	// Destroys hashtable and sets ht to NULL
	ht_destroy(&ht);
	printf("%zu\n", ht);




}