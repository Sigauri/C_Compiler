
#include "stdlib.h"	

#define DEFAULT_HT_SIZE 4098


struct node
{
	struct c_tok_name *data;

	struct node *next;
};

struct hash_table
{

	// Amount of elements currently being in use
	int in_use;

	// Amount of collisions
	int collisions;

	// Size of hash table's array
	int ht_arr_size;

	//Pointer to an array
	struct node *ht_arr[DEFAULT_HT_SIZE];
};


struct hash_table *ht_create();
void ht_destroy(struct hash_table **ht);

// Insert to_insert at ht_arr[id]'s first node
void ht_insert(struct hash_table *ht, unsigned int id, struct node *to_insert);

// Remove node with corresponding key at ht_arr[id]
void ht_remove(struct hash_table *ht, unsigned int id, struct node *to_remove);

// get a node with corresponding key at ht_arr[id]
struct node *ht_get(struct hash_table *ht, char *key);

// Compute hash code for str
unsigned long get_hash(unsigned char *str);


#define get_head(ht, id) ((ht->ht_arr[id]))

#define create_node(_node, _data)				\
	_node = calloc(1, sizeof(struct node));		\
	_node->data = _data;

#define insert_data(node, data) (node->data = data)

