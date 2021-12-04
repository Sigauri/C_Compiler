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

-Passing double pointer to those functions? Right thing to do?

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
			// Check what type the key value is and compute hash value for it
			switch(to_copy->key_type)
			{
				case KEY_TYPE_INT:
				{
					hash = ht_dest->ht_hash_int(*((long long *)to_copy->key));
					break;
				}
				case KEY_TYPE_STR:
				{
					hash = ht_dest->ht_hash_str(to_copy->key);
					break;
				}
			}

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
		}

	}



	ht_dest->in_use = ht_src->in_use;
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
	unsigned long ht_new_size = ht->ht_size <<= 1;
	if(option) ht_new_size = ht->ht_size >>= 1;
	
	struct hash_table *new_ht = ht_create(ht_new_size);

	ht_copy(&new_ht, ptr_ht, 1);
	*ptr_ht = new_ht;


	printf("%zu\n", *ptr_ht);

	return new_ht;

}

static void destroy_node(struct node *nd)
{
	free(nd->key);
	free(nd->value);
	free(nd);
}


static struct node *create_node(void *value, void *key, 
				unsigned char key_type, struct node *next)
{
	struct node *new_node = calloc(1, node_size);
	assert(new_node);

	new_node->value = value;
	new_node->key = key;
	new_node->key_type = key_type;
	new_node->next = next;

	return new_node;
}


// Insert value into ht
// Resize ht if load factor is too high
struct node *ht_insert(struct hash_table **ptr_ht, void *value, void *key, unsigned char key_type)
{
	unsigned long hash;
	unsigned long id;
	struct node *to_insert;
	struct hash_table *ht = *ptr_ht;
	
	if(!key || !value || !ht ) return NULL;

	switch(key_type)
	{
		case KEY_TYPE_INT:
		{
			hash = ht->ht_hash_int(*((long long *)key));
			break;
		}
		case KEY_TYPE_STR:
		{
			hash = ht->ht_hash_str(key);
			break;
		}
		
	}

	id = hash % ht->ht_size;

	if(ht->ht_arr[id]) ht->collisions++;
	ht->ht_arr[id] = create_node(value, key, key_type, ht->ht_arr[id]);


	// Check if load factor >= ht->max_load_factor
	// and resize if needed.
	float load_factor = (float)ht->in_use / ht->ht_size;
	if(load_factor >= ht->max_load_factor) ht_resize(ptr_ht, 0);

	(*ptr_ht)->in_use++;
	return (*ptr_ht)->ht_arr[id];
}


int ht_remove(struct hash_table **ht, void *key, unsigned char key_type)
{
	unsigned long hash;
	unsigned long id;

	if(!(*ht) || !key) return 0;

	switch(key_type)
	{
		case KEY_TYPE_STR:
		{
			hash = (*ht)->ht_hash_str(key);	
			break;
		}
		case KEY_TYPE_INT:
		{

			break;
		}

	}


	id = hash % ht->ht_size;

	struct node *to_compare = ht->ht_arr[id];


}