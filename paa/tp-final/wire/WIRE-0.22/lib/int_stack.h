#ifndef _INT_STACK_H
#define _INT_STACK_H

#include <config.h>

#define MAX_STACK_SIZE 15

typedef struct{
	int data[MAX_STACK_SIZE];
	int pos;
} int_stack_t;

void int_stack_init(int_stack_t *s);
void int_stack_push(int_stack_t *s, int val);
int int_stack_pop(int_stack_t *s);
int int_stack_peek(int_stack_t *s);

#endif
