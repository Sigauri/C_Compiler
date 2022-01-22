#include "hashtable.h"
#include "lex.h"
#include "string.h"
#include "stdio.h"
#include <assert.h>


/* 

TODOs:

-Make it work for statically allocated keys and values too ???

-Iterating through whole ht->ht_arr when copying or destroying 
 a hash table can take too much time. Maybe create a list of all
 occupied entries?

-Passing double pointer to those functions??

-Not sure about how good the hash functions are. Do some tests?  

*/

// djb2 hash function
static unsigned long
default_hash_str(unsigned char *str)
{
    unsigned long hash = 5381;
    int c;

    while (c = *str++)
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash;
}


static unsigned long default_hash_int(long long i_key)
{
	return (i_key*(i_key + 3));
}

//  Size of struct node in bytes
unsigned int node_size;

struct hash_table *ht_create(unsigned int ht_size)
{
	struct hash_table *ht = calloc(1, sizeof(struct hash_table));
	assert(ht);

	// Round to next power of two
	// or set to default size if 0 passed 
	
	ht->ht_size = 1;
	node_size = sizeof(struct node);

	if(ht_size > 0)
		while(ht->ht_size < ht_size) ht->ht_size <<= 1;
	else ht->ht_size = DEFAULT_HT_SIZE;

	ht->ht_arr = calloc(ht->ht_size, node_size);

	assert(ht->ht_arr);

	ht->ht_hash_str = default_hash_str;
	ht->ht_hash_int = default_hash_int;

	ht->max_load_factor = DEFAULT_MAX_LOAD_FACTOR;
	ht->min_load_factor = DEFAULT_MIN_LOAD_FACTOR;

	
	return ht;
}

// Insert nd_after after nd_before
#define insert_after(nd_before, nd_after)							\
	do 																\
	{																\
		struct node *nd = nd_before->next;							\
		nd_before->next = nd_after;									\
		nd_after->next = nd;										\
																	\
	}while(0)




// TODOs - Count collisions, 
// Copys entrys from one ht to another. Hash values are computed again.
// if rm is 1, the old hashtable is removed
// Returns 1 on success - 0 otherwise. 
int ht_copy(struct hash_table **ptr_ht_dest, struct hash_table **ptr_ht_src, int rm)
{

	struct node *to_copy;
	unsigned long hash;

	struct hash_table *ht_src = *ptr_ht_src;
	struct hash_table *ht_dest = *ptr_ht_dest;
	
	if(!ht_dest || !ht_src) return 0;

	for(int i = 0; i < ht_src->ht_size; i++)
	{

		to_copy = ht_src->ht_arr[i];
		while(to_copy)
		{
			hash = to_copy->hash;
			// Computing id to which the node goes 
			unsigned int id = hash % ht_dest->ht_size;

			struct node *to_insert = to_copy;

			/*  
				If rm >= 1, put nodes from ht_src to ht_dest
				without copying them, beacause ht_src will be freed.
				
				If rm <= 0, ht_src will not be removed, hence we have
				to copy the nodes, so that both hash tables don't 
				point to same nodes.
			*/
			if(rm) 
			{
				to_insert = malloc(node_size);
				assert(to_insert);
				memcpy(to_insert, to_copy, node_size);
			}

			if(ht_dest->ht_arr[id]) ht_dest->collisions++;
			ht_dest->ht_arr[id] = to_insert;
			to_copy = to_copy->next;
			ht_dest->in_use++;

		}

	}

	ht_dest->ht_hash_str = ht_src->ht_hash_str;
	ht_dest->ht_hash_int = ht_src->ht_hash_int;

	// If rm, free ht_src
	if(rm)
	{
		free(ht_src->ht_arr);
		free(ht_src);
	}


	return 1;
}


// If option is 0 - new size will be ht->ht_size * 2,
// if 1, ht->ht_size / 2
// Returns pointer to resized hash table on success, NULL - otherwise
static struct hash_table *ht_resize(struct hash_table **ptr_ht, int option)
{
	struct hash_table *ht = *ptr_ht;
	if(!ht) return NULL;


	// Shifting(mul/div by 2) ht_size to left/right according to option
	unsigned long ht_new_size = 0;
	if(option) ht_new_size = ht->ht_size >> 1;
	else ht_new_size = ht->ht_size << 1;

	struct hash_table *new_ht = ht_create(ht_new_size);

	ht_copy(&new_ht, ptr_ht, 1);
	*ptr_ht = new_ht;


	// printf("%zu\n", *ptr_ht);

	return new_ht;

}

static void destroy_node(struct node *nd)
{
	free(nd->key);
	free(nd->value);
	free(nd);
}


static struct node *create_node(void *value, void *key, 
						 unsigned long hash, struct node *next)
{
	struct node *new_node = calloc(1, node_size);
	assert(new_node);

	new_node->value = value;
	new_node->key = key;
	new_node->next = next;
	new_node->hash = hash;
	return new_node;
}





// Insert value into ht
// Resize ht if load factor is too high
struct node *ht_insert(struct hash_table **ptr_ht, void *value, void *key, unsigned long hash)
{
	unsigned long id = hash % (*ptr_ht)->ht_size;;
	struct node *to_insert;
	
	if(!key || !value || !(*ptr_ht) ) return NULL;

	if((*ptr_ht)->ht_arr[id]) (*ptr_ht)->collisions++;
	(*ptr_ht)->ht_arr[id] = create_node(value, key, hash, (*ptr_ht)->ht_arr[id]);


	// Check if load factor >= ht->max_load_factor
	// and resize if needed.
	float load_factor = (float)(*ptr_ht)->in_use / (*ptr_ht)->ht_size;
	if(load_factor >= (*ptr_ht)->max_load_factor) ht_resize(ptr_ht, 0);

	(*ptr_ht)->in_use++;
	return (*ptr_ht)->ht_arr[id];
}



// Return an item whos key == key, NULL if not found
void *ht_find(struct hash_table *ht, void *key, unsigned long hash)
{
	unsigned long id = hash % ht->ht_size;

	if(!ht || !key) return NULL;

	struct node *result = ht->ht_arr[id];

	while(result && strcmp(result->key, key) != 0)
		result = result->next;

	if(result) return result->value;
	return NULL;
}



int ht_remove(struct hash_table **ht, void *value, unsigned long hash)
{
	unsigned long id = hash % (*ht)->ht_size;

	if(!(*ht) || !value) return 0;
	
	struct node **to_compare = &((*ht)->ht_arr[id]);

	while(to_compare && (*to_compare)->value != value)
		*to_compare = (*to_compare)->next;

	if(!(*to_compare)) return 0;

	struct node *tmp = (*to_compare)->next;
	free(*to_compare);
	*to_compare = tmp;

	float load_factor = (float)(*ht)->in_use / (*ht)->ht_size;
	// printf("%f\n", load_factor);
	if(load_factor <= (*ht)->min_load_factor)
	{
		// 1 because we want to reduce
		ht_resize(ht, 1);
	}	

	(*ht)->in_use--;

	return 1;
}


/* 
	Destroy a hash table
	Frees storage allocated for HT
	and all its nodes, but doesn't free keys
	or values from those nodes, because other 
	Hashtables may still be using them.

*/
void ht_destroy(struct hash_table **ht)
{
	if(!(*ht)) return;

	struct node *current = (*ht)->ht_arr[0];

	for(int i = 0; i < (*ht)->ht_size; i++)
	{
		struct node *to_remove = (*ht)->ht_arr[i];
		while(to_remove)
		{
			struct node *tmp = to_remove->next;
			free(to_remove);
			to_remove = tmp;
		}
	}

	free((*ht)->ht_arr);
	free(*ht);
	*ht = NULL;
}