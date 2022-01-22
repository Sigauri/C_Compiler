#include "hashtable.h"

#define ST_DEFAULT_SIZE 32

struct symbol_table
{

	// Pointer to previous symbol table
	struct symbol_table *prev;

	// Pointer to its hash table
	struct hash_table *ht;
};

#define st_destroy(st) ({ht_destroy(&(st->ht)); free(st);})

#define st_create(st, prev, size)								\
	do 															\
	{															\
		unsigned int actual_size = size;						\
																\
		if(actual_size <= 0) actual_size = ST_DEFAULT_SIZE;		\
																\
		st = malloc(sizeof(struct symbol_table));				\
		st->prev = prev;										\
		st->ht = ht_create(actual_size);						\
	}while(0);

#define st_remove(st, key)										\
	do 															\
	{															\
		struct hash_table **ht = &st->ht;						\
		unsigned long hash = (*ht)->ht_hash_str(key);			\
		void *value = ht_find(*ht, key, hash);					\
		ht_remove(ht, value, hash);								\
	}while(0);													

#define st_find(st, value, key)									\
	do 															\
	{															\
		struct hash_table **ht = &(st->ht);						\
		unsigned long hash = (*ht)->ht_hash_str(key);			\
		value = ht_find(*ht, key, hash);						\
	}while(0);

#define st_add(st, value, key)									\
	do 															\
	{															\
		struct hash_table **ht = &(st->ht);						\
		unsigned long hash = (*ht)->ht_hash_str(key);			\
		ht_insert_str(ht, (void*)value, key);					\
	}while(0);
