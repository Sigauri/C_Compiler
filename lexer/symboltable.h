#include "hashtable.h"

struct symbol_table
{
	// Hash table of this st
	struct hash_table *ht;

	// If null - this is a global st
	struct symbol_table *prev;
};

void st_destroy(struct symbol_table **st);
struct symbol_table *st_create();
struct c_tok_name *st_get(struct symbol_table *st, char *key);
void st_add(struct symbol_table *st, struct c_tok_name *tok_name);