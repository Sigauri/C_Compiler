#include "hashtable.h"
#include "lex.h"
#include "string.h"
#include "stdio.h"

//djb2 hash function
unsigned long
get_hash(unsigned char *str)
{
    unsigned long hash = 5381;
    int c;

    while (c = *str++)
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash;
}


// Get node by key
struct node *ht_get(struct hash_table *ht, char *key)
{
	unsigned int hash = get_hash(key)%DEFAULT_HT_SIZE;
	struct node *header = get_head(ht, hash);
	struct node *cur = header;

	if(!cur) return NULL;
	
	for(int i = 0; i < DEFAULT_HT_SIZE; i++)
	{
		if(strcmp(cur->data->lexeme, key) != 0)
		{
			cur = cur->next;
			continue;
		}
		break;
	}
	return cur;
}

void ht_insert(struct hash_table *ht, unsigned int id, struct node *to_insert)
{
	to_insert->next = get_head(ht, id);
	ht->ht_arr[id] = to_insert;
}

void ht_remove(struct hash_table *ht, unsigned int id, struct node *to_remove)
{
	struct node **cur = &get_head(ht, id);

	while(*cur != to_remove) cur = &(*cur)->next;

	*cur = to_remove->next;

	free(to_remove);
}

struct hash_table *ht_create()
{
	struct hash_table *ht = calloc(1, sizeof(struct hash_table));
	return ht;
}



// Free all the memory allocated for the ht
void ht_destroy(struct hash_table **ht)
{
	struct node *cur_head;
	struct node *cur_node;
	for(int i = 0; i < DEFAULT_HT_SIZE-1; i++)
	{
		cur_head = (*ht)->ht_arr[i];
		if(cur_head)
		{
			cur_node = cur_head;
			while(cur_node)
			{
				struct node *cur_next = cur_node->next;
				struct c_tok_name *cur_name = cur_node->data;
				// free(cur_name->lexeme);
				switch(cur_name->type)
				{
					case C_UNION_ID:
					{
						free(cur_name->tok_u.first_pos);
						free(cur_name->tok_u.id_type);
						break;
					}

				}
				free(cur_node);
				cur_node = cur_next;
			}
		}
	}

	free(*ht);
	*ht = NULL;
}