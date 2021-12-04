#include "stdio.h"
#include "symboltable.h"
#include "lex.h"

struct symbol_table *st_current = NULL;

// Create symbol table and set up st
struct symbol_table *st_create()
{
	struct symbol_table *st = malloc(sizeof(struct symbol_table));
	
	st->prev = st_current;
	st_current = st;

	st->ht = ht_create();
	return st;

}

// Destroy st
void st_destroy(struct symbol_table **st)
{
	// ptr to ptr to st->ht
	struct hash_table **ht_ptr = &((*st)->ht);
	
	ht_destroy(ht_ptr);
	
	// Free the st itself
	free(*st);
	*st = NULL;
}

void st_add(struct symbol_table *st, struct c_tok_name *tok_name)
{
	struct node *nd;
	create_node(nd, tok_name);
	unsigned int hash = get_hash(tok_name->lexeme);
	ht_insert(st->ht, hash, nd);
}

struct c_tok_name *st_get(struct symbol_table *st, char *key)
{
	struct node *nd = ht_get(st->ht, key);
	if(!nd) return NULL;
	return nd->data;
}