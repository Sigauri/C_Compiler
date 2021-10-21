#include "stack.h"
#include "stdlib.h"
#include "string.h"
#include "stdio.h"

void push(void *value, struct stack *st)
{

	int node_size = sizeof(struct node);

	if(!st->top)
	{
		st->top = malloc(node_size);
		st->top->prev = NULL;
		st->top->next = NULL;
		st->top->value = value;
		return;
	}

	st->top->next = malloc(node_size);
	st->top->next->prev = st->top;
	st->top->next->value = value;

	st->top = st->top->next;
	st->count++;

}

void *pop(struct stack *st)
{
	void **tmp = &st->top;
	
	if(!(*tmp)) return NULL;

	if(st->top->prev) st->top = st->top->prev;
	free(*tmp);
	*tmp = NULL;
	return (void*)st->top;


}

void *peek(struct stack *st)
{
	return st->top;
}

int is_empty(struct stack *st)
{
	if(!st->count) return 0;
	return 1;
}






struct stack *create_stack()
{
	
	int to_allocate = sizeof(struct stack);
	struct stack *st = malloc(to_allocate);

	memset(st, 0, to_allocate);
	return st;
}