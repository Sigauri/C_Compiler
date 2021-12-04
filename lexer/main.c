#include "stdio.h"
#include "string.h"
#include "lex.h"

#define DEBUG_INFO

#include "hashtable.h"
// #include "symboltable.h"
#include "stdlib.h"

int main()
{	
	struct hash_table *ht = ht_create(0);

	struct c_tok_name *kwd[34];

	for(int i = 0; i < 34; i++)
	{
		kwd[i] = c_tok_name_create_kwd(keywords[i], i);
	}

	void *ptr1 = malloc(24);
	void *ptr2 = malloc(24);
	void *ptr3 = malloc(24);
	void *ptr4 = malloc(24);
	void *ptr5 = malloc(24);
	void *ptr6 = malloc(24);
	void *ptr7 = malloc(24);

	*((size_t *)ptr1) = 12;	
	*((size_t *)ptr2) = 11;	
	*((size_t *)ptr3) = 13;	
	char *l = "auto";
	
	
	printf("%zu\n", ht);

	for(int i = 0; i < 13; i++)
	{
		ht_insert(&ht, kwd[i], kwd[i]->lexeme, KEY_TYPE_STR);
	}

	float load_factor = (float)ht->in_use / ht->ht_size;
	printf("%f\n", load_factor);
	printf("%zu\n", ht->in_use);
	printf("%zu\n", ht->collisions);
	printf("%zu\n", ht->ht_size);
	// printf("%s\n", (ht->ht_arr[ht->ht_hash_str(kwd[0]->lexeme) % ht->ht_size])->key);

	// printf("%zu\n", ht->ht_size);
	// ht_state_print((*ht));

	// printf("%zu\n", ht->ht_arr[30]);
	// printf("%zu\n", ht->ht_arr[30]->next);

	// hash = ht->ht_hash("Key");
	// printf("%zu\n", hash);

	// hash = ht->ht_hash("Keys");
	// printf("%zu\n", hash);

	// hash = ht->ht_hash("Keyl");
	// printf("%zu\n", hash);

	// hash = ht->ht_hash("KeyH");
	// printf("%zu\n", hash);

	// hash = ht->ht_hash("KsdySx");
	// printf("%zu\n", hash);

	// hash = ht->ht_hash("SeySi");
	// printf("%zu\n", hash);



}