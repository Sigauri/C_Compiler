#include "stdio.h"
#include "string.h"
#include "lex.h"
#include "hashtable.h"
#include "stdlib.h"


int main()
{

	struct hash_table *ht = malloc(sizeof(struct hash_table));

	struct node *nd[6];
	unsigned int hash[6];
	int node_size = sizeof(struct node);
	for(int i = 0; i < 6; i++)
	{
		nd[i] = malloc(node_size);
		nd[i]->data = c_tok_name_create(keywords[i], NULL, NULL, C_KWD_CONTINUE);
		hash[i] = get_hash(keywords[i])%DEFAULT_HT_SIZE;
	}

	ht_insert(ht, 0, nd[0]);
	ht_insert(ht, 0, nd[1]);
	ht_insert(ht, 0, nd[2]);
	ht_insert(ht, 0, nd[3]);
	
	printf("%s\n", get_head(ht, 0)->data->lexeme);
	printf("%s\n", get_head(ht, 0)->next->data->lexeme);
	printf("%s\n", get_head(ht, 0)->next->next->data->lexeme);

	ht_remove(ht, 0, nd[3]);

	printf("%s\n", get_head(ht, 0)->data->lexeme);
	printf("%s\n", get_head(ht, 0)->next->data->lexeme);
	printf("%s\n", get_head(ht, 0)->next->next->data->lexeme);

	// for(int i = 0; i < DEFAULT_HT_SIZE; i++)
	// {
	// 	printf("%zu\n", i);
	// 	printf("%zu\n", ht->ht_arr[i]);
	// 	ht->ht_arr[i] = NULL;
	// }
	ht_destroy(&ht);

}