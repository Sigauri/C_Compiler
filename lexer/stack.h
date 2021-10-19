#include "stddef.h"	

struct stack
{
	//The element on the top of the stack
	struct node *top;

	// The amount of objects added to stack 
	int count;
};



void *pop(struct stack *st);
void *peek();
struct stack *create_stack();
void push(void *value, struct stack *st);
// return 1 if empty, 0 otherwise
int is_empty();


struct node
{
	void *value;
	struct node *next;
	struct node *prev;
};



// Allocate, intialize and return a stack